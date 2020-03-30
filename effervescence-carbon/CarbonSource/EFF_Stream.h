//
//  EFF_Stream.h
//  effervescence-core
//
//  Created by Nerrons on 25/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

#ifndef EFF_Stream_h
#define EFF_Stream_h

// SuperClass Includes
#include "EFF_Object.h"

// PublicUtility Includes
#include "CAMutex.h"

// System Includes
#include <CoreAudio/AudioHardwareBase.h>


#pragma clang assume_nonnull begin

class EFF_Stream
:
    public EFF_Object
{

    
#pragma mark Construction/Destruction

public:
                                EFF_Stream(AudioObjectID inObjectID,
                                           AudioObjectID inOwnerDeviceID,
                                           bool inIsInput,
                                           Float64 inSampleRate,
                                           UInt32 inStartingChannel = 1);
    virtual                     ~EFF_Stream();

#pragma mark Property Operations
    
public:
    bool                        HasProperty(AudioObjectID inObjectID,
                                            pid_t inClientPID,
                                            const AudioObjectPropertyAddress& inAddress) const;
    bool                        IsPropertySettable(AudioObjectID inObjectID,
                                                   pid_t inClientPID,
                                                   const AudioObjectPropertyAddress& inAddress) const;
    UInt32                      GetPropertyDataSize(AudioObjectID inObjectID,
                                                    pid_t inClientPID,
                                                    const AudioObjectPropertyAddress& inAddress,
                                                    UInt32 inQualifierDataSize,
                                                    const void* __nullable inQualifierData) const;
    void                        GetPropertyData(AudioObjectID inObjectID,
                                                pid_t inClientPID,
                                                const AudioObjectPropertyAddress& inAddress,
                                                UInt32 inQualifierDataSize,
                                                const void* __nullable inQualifierData,
                                                UInt32 inDataSize,
                                                UInt32& outDataSize,
                                                void* outData) const;
    void                        SetPropertyData(AudioObjectID inObjectID,
                                                pid_t inClientPID,
                                                const AudioObjectPropertyAddress& inAddress,
                                                UInt32 inQualifierDataSize,
                                                const void* __nullable inQualifierData,
                                                UInt32 inDataSize,
                                                const void* inData);
    
#pragma mark Implementation

public:
    // This is only called by EFFDevice and not by this stream itself, because the device will
    // make the decision to set the sample rates for both streams at once
    void                        SetSampleRate(Float64 inSampleRate);

private:
    CAMutex                     mStateMutex;

    bool                        mIsInput;
    Float64                     mSampleRate;
    
    /*! True if the stream is enabled and doing IO. See kAudioStreamPropertyIsActive. */
    bool                        mIsStreamActive;

    /*!
     The absolute channel number for the first channel in the stream. For example, if a device has
     two output streams with two channels each, then the starting channel number for the first
     stream is 1 and the starting channel number for the second stream is 3. See
     kAudioStreamPropertyStartingChannel.
     */
    UInt32                      mStartingChannel;

};

#pragma clang assume_nonnull end

#endif /* EFF_Stream_h */
