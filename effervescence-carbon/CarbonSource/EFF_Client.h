//
//  EFF_Client.h
//  effervescence-core
//
//  Created by Nerrons on 25/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

#ifndef EFF_Client_h
#define EFF_Client_h

// PublicUtility Includes
#include "CACFString.h"

// System Includes
#include <CoreAudio/AudioServerPlugIn.h>


#pragma clang assume_nonnull begin

//==================================================================================================
//    EFF_Client
//
//  Client meaning a client (of the host) of the EFFDevice, i.e. an app registered with the HAL,
//  generally so it can do IO at some point.
//==================================================================================================

class EFF_Client
{


#pragma mark Construction/Destruction

public:
                                EFF_Client() = default;
                                EFF_Client(const AudioServerPlugInClientInfo* inClientInfo);
                                ~EFF_Client() = default;
                                EFF_Client(const EFF_Client& inClient)
                                    { Copy(inClient); };
                                EFF_Client& operator=(const EFF_Client& inClient)
                                    { Copy(inClient); return *this; }
    
private:
    void                        Copy(const EFF_Client& inClient);
    

#pragma mark Implementation

public:
    // These fields are duplicated from AudioServerPlugInClientInfo (except the mBundleID CFStringRef is
    // wrapped in a CACFString here).
    UInt32                      mClientID;
    pid_t                       mProcessID;
    Boolean                     mIsNativeEndian = true;
    CACFString                  mBundleID;
    
    // Becomes true when the client triggers the plugin host to call StartIO or to begin
    // kAudioServerPlugInIOOperationThread, and false again on StopIO or when
    // kAudioServerPlugInIOOperationThread ends
    bool                        mDoingIO = false;

    // True if EFFApp has set this client as belonging to the music player app
    bool                        mIsMusicPlayer = false;

    // The client's volume relative to other clients. In the range [0.0, 4.0], defaults to 1.0 (unchanged).
    // mRelativeVolumeCurve is applied to this value when it's set.
    Float32                     mRelativeVolume = 1.0;

    // The client's pan position, in the range [-100, 100] where -100 is left and 100 is right
    SInt32                      mPanPosition = 0;
    
};

#pragma clang assume_nonnull end

#endif /* EFF_Client_h */
