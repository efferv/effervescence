//
//  EFF_ClientMap.h
//  effervescence-core
//
//  Created by Nerrons on 25/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

#ifndef EFF_ClientMap_h
#define EFF_ClientMap_h


// Local Includes
#include "EFF_Client.h"
#include "EFF_TaskQueue.h"

// PublicUtility Includes
#include "CAMutex.h"
#include "CACFString.h"
#include "CACFArray.h"
#include "CAVolumeCurve.h"

// STL Includes
#include <map>
#include <vector>
#include <functional>


// Forward Declarations
class EFF_ClientTasks;


#pragma clang assume_nonnull begin
//==================================================================================================
//    EFF_ClientMap
//
//  This class stores the clients (EFF_Client) that have been registered with EFFDevice by the HAL.
//  It also maintains maps from clients' PIDs and bundle IDs to the clients. When a client is
//  removed by the HAL we add it to a map of past clients to keep track of settings specific to that
//  client. (Currently only the client's volume.)
//
//  Since the maps are read from during IO, this class has to to be real-time safe when accessing
//  them. So each map has an identical "shadow" map, which we use to buffer update
//  To update the clients we lock the shadow maps, modify them, have EFF_TaskQueue's real-time
//  thread swap them with the main maps, and then repeat the modification to keep both sets of maps
//  identical. We have to swap the maps on a real-time thread so we can take the main maps' lock
//  without risking priority inversion, but this way the actual work doesn't need to be real-time
//  safe.
//
//  Methods that only read from the maps and are called on non-real-time threads will just read
//  from the shadow maps because it's easier.
//
//  Methods whose names end with "RT" and "NonRT" can only safely be called from real-time and
//  non-real-time threads respectively. (Methods with neither are most likely non-RT.)
//==================================================================================================

class EFF_ClientMap
{
    friend class EFF_ClientTasks;
    typedef std::vector<EFF_Client*> EFF_ClientPtrList;
    
#pragma mark Construction/Destruction
    
public:
                                EFF_ClientMap(EFF_TaskQueue* inTaskQueue)
                                :
                                    mTaskQueue(inTaskQueue),
                                    mMapsMutex("Maps mutex"),
                                    mShadowMapsMutex("Shadow maps mutex") { };
    

#pragma mark API

    void                        AddClient(EFF_Client inClient);
    EFF_Client                  RemoveClient(UInt32 inClientID);
    
    // These methods are functionally identical except that GetClientRT must only be called from real-time threads
    // and GetClientNonRT must only be called from non-real-time threads. Both return true if a client was found.
    bool                        GetClientRT(UInt32 inClientID, EFF_Client* outClient) const;
    bool                        GetClientNonRT(UInt32 inClientID, EFF_Client* outClient) const;
    std::vector<EFF_Client>     GetClientsByPID(pid_t inPID) const;
    
    // Set the isMusicPlayer flag for each client. (True if the client has the given bundle ID/PID, false otherwise.)
    void                        UpdateMusicPlayerFlags(pid_t inMusicPlayerPID);
    void                        UpdateMusicPlayerFlags(CACFString inMusicPlayerBundleID);

    // Copies the current and past clients into an array in the format expected for
    // kAudioDeviceCustomPropertyAppVolumes. (Except that CACFArray and CACFDictionary are used instead
    // of unwrapped CFArray and CFDictionary refs.)
    CACFArray                   CopyClientRelativeVolumesAsAppVolumes(CAVolumeCurve inVolumeCurve) const;
    
    // Using the template function hits LLVM Bug 23987
    // TODO Switch to template function
    
    // Returns true if a client for the key was found and its relative volume changed.
    //template <typename T>
    //bool                        SetClientsRelativeVolume(T _Null_unspecified searchKey, Float32 inRelativeVolume);
    //
    //template <typename T>
    //bool                        SetClientsPanPosition(T _Null_unspecified searchKey, SInt32 inPanPosition);
    
    // Returns true if a client for PID inAppPID was found and its relative volume changed.
    bool                        SetClientsRelativeVolume(pid_t inAppPID, Float32 inRelativeVolume);
    // Returns true if a client for bundle ID inAppBundleID was found and its relative volume changed.
    bool                        SetClientsRelativeVolume(CACFString inAppBundleID, Float32 inRelativeVolume);
    
    // Returns true if a client for PID inAppPID was found and its pan position changed.
    bool                        SetClientsPanPosition(pid_t inAppPID, SInt32 inPanPosition);
    // Returns true if a client for bundle ID inAppBundleID was found and its pan position changed.
    bool                        SetClientsPanPosition(CACFString inAppBundleID, SInt32 inPanPosition);
    
    void                        StartIONonRT(UInt32 inClientID) { UpdateClientIOStateNonRT(inClientID, true ); }
    void                        StopIONonRT(UInt32 inClientID)  { UpdateClientIOStateNonRT(inClientID, false); }

    
#pragma mark Implementation

private:
    void                        AddClientToShadowMaps(EFF_Client inClient);
    static bool                 GetClient(const std::map<UInt32, EFF_Client>& inClientMap,
                                          UInt32 inClientID,
                                          EFF_Client* outClient);
    void                        UpdateMusicPlayerFlagsInShadowMaps(std::function<bool(EFF_Client)> inIsMusicPlayerTest);
    void                        CopyClientIntoAppVolumesArray(EFF_Client inClient,
                                                              CAVolumeCurve inVolumeCurve,
                                                              CACFArray& ioAppVolumes) const;
    void                        UpdateClientIOStateNonRT(UInt32 inClientID, bool inDoingIO);
    
    // Has a real-time thread call SwapInShadowMapsRT. (Synchronously queues the call as a task on mTaskQueue.)
    // The shadow maps mutex must be locked when calling this method.
    void                        SwapInShadowMaps();
    // Note that this method is called by EFF_TaskQueue through the EFF_ClientTasks interface.
    // The shadow maps mutex must be locked when calling this method.
    void                        SwapInShadowMapsRT();
    
    // Client lookup for PID inAppPID
    std::vector<EFF_Client*> * _Nullable            GetClients(pid_t inAppPid);
    // Client lookup for bundle ID inAppBundleID
    std::vector<EFF_Client*> * _Nullable            GetClients(CACFString inAppBundleID);
    

#pragma mark Members

    EFF_TaskQueue*                                  mTaskQueue;
    
    // Must be held to access mClientMap or mClientMapByPID. Code that runs while holding this mutex needs
    // to be real-time safe. Should probably not be held for most operations on mClientMapByBundleID because,
    // as far as I can tell, code that works with CFStrings is unlikely to be real-time safe.
    CAMutex                                         mMapsMutex;
    // Should only be locked by non-real-time threads. Should not be released until the maps have been
    // made identical to their shadow maps.
    CAMutex                                         mShadowMapsMutex;
    
    // The clients currently registered with EFFDevice. Indexed by client ID.
    std::map<UInt32, EFF_Client>                    mClientMap;
    // We keep this in sync with mClientMap so it can be modified outside of real-time safe sections and
    // then swapped in on a real-time thread, which is safe.
    std::map<UInt32, EFF_Client>                    mClientMapShadow;
    
    // These maps hold lists of pointers to clients in mClientMap/mClientMapShadow. Lists because a process
    // can have multiple clients and clients can have the same bundle ID.
    std::map<pid_t, EFF_ClientPtrList>              mClientMapByPID;
    std::map<pid_t, EFF_ClientPtrList>              mClientMapByPIDShadow;
    
    std::map<CACFString, EFF_ClientPtrList>         mClientMapByBundleID;
    std::map<CACFString, EFF_ClientPtrList>         mClientMapByBundleIDShadow;
    
    // Clients are added to mPastClientMap so we can restore settings specific to them if they get
    // added again.
    std::map<CACFString, EFF_Client>                mPastClientMap;
};

#pragma clang assume_nonnull end

#endif /* EFF_ClientMap_h */
