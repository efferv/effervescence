//
//  EFF_Clients.cpp
//  effervescence-driver
//
//  Created by Nerrons on 26/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

// Self Include
#include "EFF_Clients.h"

// Local Includes
#include "EFF_Types.h"
#include "EFF_PlugIn.h"

// PublicUtility Includes
#include "CAException.h"
#include "CACFDictionary.h"
#include "CADispatchQueue.h"


#pragma mark Construction/Destruction

EFF_Clients::EFF_Clients(AudioObjectID inOwnerDeviceID,
                         EFF_TaskQueue* inTaskQueue)
:
    mOwnerDeviceID(inOwnerDeviceID),
    mClientMap(inTaskQueue)
{
    mRelativeVolumeCurve.AddRange(kAppRelativeVolumeMinRawValue,
                                  kAppRelativeVolumeMaxRawValue,
                                  kAppRelativeVolumeMinDbValue,
                                  kAppRelativeVolumeMaxDbValue);
}


#pragma mark Add/Remove Clients

void    EFF_Clients::AddClient(EFF_Client inClient)
{
    CAMutex::Locker theLocker(mMutex);

    // Check whether this is the music player's client
    bool pidMatchesMusicPlayerProperty =
        (mMusicPlayerProcessIDProperty != 0 && inClient.mProcessID == mMusicPlayerProcessIDProperty);
    bool bundleIDMatchesMusicPlayerProperty =
        (mMusicPlayerBundleIDProperty != "" &&
         inClient.mBundleID.IsValid() &&
         inClient.mBundleID == mMusicPlayerBundleIDProperty);
    inClient.mIsMusicPlayer = (pidMatchesMusicPlayerProperty || bundleIDMatchesMusicPlayerProperty);

    if(inClient.mIsMusicPlayer)
    {
        DebugMsg("EFF_Clients::AddClient: Adding music player client. mClientID = %u", inClient.mClientID);
    }

    mClientMap.AddClient(inClient);

    // If we're adding EFFApp, update our local copy of its client ID
    if(inClient.mBundleID.IsValid() && inClient.mBundleID == kEFFAppBundleID)
    {
        mEFFAppClientID = inClient.mClientID;
    }
}

void    EFF_Clients::RemoveClient(const UInt32 inClientID)
{
    CAMutex::Locker theLocker(mMutex);
    
    EFF_Client theRemovedClient = mClientMap.RemoveClient(inClientID);
    
    // If we're removing EFFApp, clear our local copy of its client ID
    if(theRemovedClient.mClientID == mEFFAppClientID)
    {
        mEFFAppClientID = -1;
    }
}


#pragma mark IO Status

// Start IO for inClientID and update stats.
// Return true if no other clients were running IO before this one started, which means the device should start IO
bool    EFF_Clients::StartIONonRT(UInt32 inClientID)
{
    CAMutex::Locker theLocker(mMutex);
    // get client from inClientID
    EFF_Client theClient;
    bool didFindClient = mClientMap.GetClientNonRT(inClientID, &theClient);
    ThrowIf(!didFindClient,
            EFF_InvalidClientException(),
            "EFF_Clients::StartIO: Cannot start IO for client that was never added");

    bool didStartIO = false; // the value to be returned
    bool sendIsRunningNotification = false;
    bool sendIsRunningSomewhereOtherThanEFFAppNotification = false;

    if(!theClient.mDoingIO)
    {
        // Make sure we can start
        ThrowIf(mStartCount == UINT64_MAX,
                CAException(kAudioHardwareIllegalOperationError),
                "EFF_Clients::StartIO: failed to start because the ref count was maxxed out already");

        DebugMsg("EFF_Clients::StartIO: Client %u (%s, %d) starting IO",
                 inClientID,
                 CFStringGetCStringPtr(theClient.mBundleID.GetCFString(), kCFStringEncodingUTF8),
                 theClient.mProcessID);
        
        mClientMap.StartIONonRT(inClientID);
        
        mStartCount++;
        
        // Update mStartCountExcludingEFFApp
        if(!IsEFFApp(inClientID))
        {
            ThrowIf(mStartCountExcludingEFFApp == UINT64_MAX,
                    CAException(kAudioHardwareIllegalOperationError),
                    "EFF_Clients::StartIO: failed to start because mStartCountExcludingEFFApp was maxxed out already");

            mStartCountExcludingEFFApp++;
            
            if(mStartCountExcludingEFFApp == 1)
            {
                sendIsRunningSomewhereOtherThanEFFAppNotification = true;
            }
        }
        
        didStartIO = (mStartCount == 1);
        sendIsRunningNotification = didStartIO;
    }
    
    Assert(mStartCountExcludingEFFApp == mStartCount - 1 || mStartCountExcludingEFFApp == mStartCount,
           "mStartCount and mStartCountExcludingEFFApp are out of sync");
    
    SendIORunningNotifications(sendIsRunningNotification,
                               sendIsRunningSomewhereOtherThanEFFAppNotification);

    return didStartIO;
}

// Stop IO for inClientID and update stats.
// Return true if we stopped IO entirely (i.e. there are no clients still running IO)
bool    EFF_Clients::StopIONonRT(UInt32 inClientID)
{
    CAMutex::Locker theLocker(mMutex);

    // get client from inClientID
    EFF_Client theClient;
    bool didFindClient = mClientMap.GetClientNonRT(inClientID, &theClient);
    ThrowIf(!didFindClient,
            EFF_InvalidClientException(),
            "EFF_Clients::StopIO: Cannot stop IO for client that was never added");
    
    bool didStopIO = false; // the value to be returned
    bool sendIsRunningNotification = false;
    bool sendIsRunningSomewhereOtherThanEFFAppNotification = false;
    
    if(theClient.mDoingIO)
    {
        DebugMsg("EFF_Clients::StopIO: Client %u (%s, %d) stopping IO",
                 inClientID,
                 CFStringGetCStringPtr(theClient.mBundleID.GetCFString(), kCFStringEncodingUTF8),
                 theClient.mProcessID);
        
        mClientMap.StopIONonRT(inClientID);
        
        ThrowIf(mStartCount <= 0,
                CAException(kAudioHardwareIllegalOperationError),
                "EFF_Clients::StopIO: Underflowed mStartCount");
        
        mStartCount--;
        
        // Update mStartCountExcludingEFFApp
        if(!IsEFFApp(inClientID))
        {
            ThrowIf(mStartCountExcludingEFFApp <= 0,
                    CAException(kAudioHardwareIllegalOperationError),
                    "EFF_Clients::StopIO: Underflowed mStartCountExcludingEFFApp");
            
            mStartCountExcludingEFFApp--;
            
            if(mStartCountExcludingEFFApp == 0)
            {
                sendIsRunningSomewhereOtherThanEFFAppNotification = true;
            }
        }

        didStopIO = (mStartCount == 0);
        sendIsRunningNotification = didStopIO;
    }
    
    Assert(mStartCountExcludingEFFApp == mStartCount - 1 || mStartCountExcludingEFFApp == mStartCount,
           "mStartCount and mStartCountExcludingEFFApp are out of sync");
    
    SendIORunningNotifications(sendIsRunningNotification, sendIsRunningSomewhereOtherThanEFFAppNotification);
    
    return didStopIO;
}

// Sends PropertiesChanged notifications for kAudioDevicePropertyDeviceIsRunning and
// kAudioDeviceCustomPropertyDeviceIsRunningSomewhereOtherThanEFFApp
void    EFF_Clients::SendIORunningNotifications(bool sendIsRunningNotification,
                                                bool sendIsRunningSomewhereOtherThanEFFAppNotification)
const
{
    if(sendIsRunningNotification || sendIsRunningSomewhereOtherThanEFFAppNotification)
    {
        CADispatchQueue::GetGlobalSerialQueue().Dispatch(false, ^{
            AudioObjectPropertyAddress theChangedProperties[2];
            UInt32 theNotificationCount = 0;

            if(sendIsRunningNotification)
            {
                DebugMsg("EFF_Clients::SendIORunningNotifications: Sending kAudioDevicePropertyDeviceIsRunning");
                theChangedProperties[0] = { kAudioDevicePropertyDeviceIsRunning, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
                theNotificationCount++;
            }

            if(sendIsRunningSomewhereOtherThanEFFAppNotification)
            {
                DebugMsg("EFF_Clients::SendIORunningNotifications: Sending kAudioDeviceCustomPropertyDeviceIsRunningSomewhereOtherThanEFFApp");
                theChangedProperties[theNotificationCount] = kEFFRunningSomewhereOtherThanEFFAppAddress;
                theNotificationCount++;
            }

            EFF_PlugIn::Host_PropertiesChanged(mOwnerDeviceID, theNotificationCount, theChangedProperties);
        });
    }
}

#pragma mark Music Player

bool    EFF_Clients::SetMusicPlayer(const pid_t inPID)
{
    ThrowIf(inPID < 0,
            EFF_InvalidClientPIDException(),
            "EFF_Clients::SetMusicPlayer: Invalid music player PID");

    CAMutex::Locker theLocker(mMutex);
    
    if(mMusicPlayerProcessIDProperty == inPID)
    {
        // We're not changing the properties, so return false
        return false;
    }
    
    mMusicPlayerProcessIDProperty = inPID;
    // Unset the bundle ID property
    mMusicPlayerBundleIDProperty = "";
    
    DebugMsg("EFF_Clients::SetMusicPlayer: Setting music player by PID. inPID=%d", inPID);
    
    // Update the clients' mIsMusicPlayer fields
    mClientMap.UpdateMusicPlayerFlags(inPID);
    
    return true;
}

bool    EFF_Clients::SetMusicPlayer(const CACFString inBundleID)
{
    Assert(inBundleID.IsValid(), "EFF_Clients::SetMusicPlayer: Invalid CACFString given as bundle ID");
    
    CAMutex::Locker theLocker(mMutex);
    
    if(mMusicPlayerBundleIDProperty == inBundleID)
    {
        // We're not changing the properties, so return false
        return false;
    }

    mMusicPlayerBundleIDProperty = inBundleID;
    // Unset the PID property
    mMusicPlayerProcessIDProperty = 0;
    
    DebugMsg("EFF_Clients::SetMusicPlayer: Setting music player by bundle ID. inBundleID=%s",
             CFStringGetCStringPtr(inBundleID.GetCFString(), kCFStringEncodingUTF8));
    
    // Update the clients' mIsMusicPlayer fields
    mClientMap.UpdateMusicPlayerFlags(inBundleID);
    
    return true;
}

bool    EFF_Clients::IsMusicPlayerRT(const UInt32 inClientID)
const
{
    EFF_Client theClient;
    bool didGetClient = mClientMap.GetClientRT(inClientID, &theClient);
    return didGetClient && theClient.mIsMusicPlayer;
}

#pragma mark App Volumes

// not sure if we should differentiate "client not found" and "client doesn't have custom volume"
Float32 EFF_Clients::GetClientRelativeVolumeRT(UInt32 inClientID)
const
{
    EFF_Client theClient;
    bool didGetClient = mClientMap.GetClientRT(inClientID, &theClient);
    return (didGetClient ? theClient.mRelativeVolume : 1.0f);
}

// same for this one, see GetClientRelativeVolumeRT(UInt32)
SInt32 EFF_Clients::GetClientPanPositionRT(UInt32 inClientID)
const
{
    EFF_Client theClient;
    bool didGetClient = mClientMap.GetClientRT(inClientID, &theClient);
    return (didGetClient ? theClient.mPanPosition : kAppPanCenterRawValue);
}

bool    EFF_Clients::SetClientsRelativeVolumes(const CACFArray inAppVolumes)
{
    bool didChangeAppVolumes = false;
    
    // Each element in appVolumes is a CFDictionary containing the process id and/or bundle id of an app, and its
    // new relative volume
    for(UInt32 i = 0; i < inAppVolumes.GetNumberItems(); i++)
    {
        CACFDictionary theAppVolume(false);
        inAppVolumes.GetCACFDictionary(i, theAppVolume);
        
        // Get the app's PID from the dict
        pid_t theAppPID;
        bool didFindPID = theAppVolume.GetSInt32(CFSTR(kEFFAppVolumesKey_ProcessID), theAppPID);
        
        // Get the app's bundle ID from the dict
        CACFString theAppBundleID;
        theAppBundleID.DontAllowRelease();
        theAppVolume.GetCACFString(CFSTR(kEFFAppVolumesKey_BundleID), theAppBundleID);
        
        ThrowIf(!didFindPID && !theAppBundleID.IsValid(),
                EFF_InvalidClientRelativeVolumeException(),
                "EFF_Clients::SetClientsRelativeVolumes: App volume was sent without PID or bundle ID for app");
        
        bool didGetVolume;
        {
            SInt32 theRawRelativeVolume;
            didGetVolume = theAppVolume.GetSInt32(CFSTR(kEFFAppVolumesKey_RelativeVolume), theRawRelativeVolume);
            
            if (didGetVolume) {
                ThrowIf(didGetVolume &&
                        (theRawRelativeVolume < kAppRelativeVolumeMinRawValue ||
                         theRawRelativeVolume > kAppRelativeVolumeMaxRawValue),
                        EFF_InvalidClientRelativeVolumeException(),
                        "EFF_Clients::SetClientsRelativeVolumes: Relative volume for app out of valid range");
                
                // Apply the volume curve to the raw volume
                //
                // mRelativeVolumeCurve uses the default kPow2Over1Curve transfer function, so we also multiply by 4 to
                // keep the middle volume equal to 1 (meaning apps' volumes are unchanged by default).
                Float32 theRelativeVolume = mRelativeVolumeCurve.ConvertRawToScalar(theRawRelativeVolume) * 4;

                // Try to update the client's volume, first by PID and then by bundle ID. Always try
                // both because apps can have multiple clients.
                if(mClientMap.SetClientsRelativeVolume(theAppPID, theRelativeVolume))
                {
                    didChangeAppVolumes = true;
                }

                if(mClientMap.SetClientsRelativeVolume(theAppBundleID, theRelativeVolume))
                {
                    didChangeAppVolumes = true;
                }

                // TODO: If the app isn't currently a client, we should add it to the past clients
                //       map, or update its past volume if it's already in there.
            }
        }
        
        bool didGetPanPosition;
        {
            SInt32 thePanPosition;
            didGetPanPosition = theAppVolume.GetSInt32(CFSTR(kEFFAppVolumesKey_PanPosition), thePanPosition);
            if (didGetPanPosition) {
                ThrowIf(didGetPanPosition && (thePanPosition < kAppPanLeftRawValue || thePanPosition > kAppPanRightRawValue),
                                              EFF_InvalidClientPanPositionException(),
                                              "EFF_Clients::SetClientsRelativeVolumes: Pan position for app out of valid range");
                
                if(mClientMap.SetClientsPanPosition(theAppPID, thePanPosition))
                {
                    didChangeAppVolumes = true;
                }

                if(mClientMap.SetClientsPanPosition(theAppBundleID, thePanPosition))
                {
                    didChangeAppVolumes = true;
                }

                // TODO: If the app isn't currently a client, we should add it to the past clients
                //       map, or update its past pan position if it's already in there.
            }
        }
        
        ThrowIf(!didGetVolume && !didGetPanPosition,
                EFF_InvalidClientRelativeVolumeException(),
                "EFF_Clients::SetClientsRelativeVolumes: No volume or pan position in request");
    }
    
    return didChangeAppVolumes;
}

