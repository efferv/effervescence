//
//  EFF_AudibleState.h
//  effervescence-core
//
//  Created by Nerrons on 26/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//
//  Copyright © 2016, 2017 Kyle Neideck
//
//  Inspects a stream of audio data and reports whether it's silent, silent except for the user's
//  music player, or audible.
//
//  See kAudioDeviceCustomPropertyDeviceAudibleState and the EFFDeviceAudibleState enum in
//  EFF_Types.h for more info.
//
//  Not thread-safe.
//

#ifndef EFF_AudibleState_h
#define EFF_AudibleState_h

// Local Includes
#include "EFF_Types.h"

// System Includes
#include <MacTypes.h>


#pragma clang assume_nonnull begin

class EFF_AudibleState
{

public:
                                EFF_AudibleState();

    /*!
     @return The current audible state of the device, to be used as the value of the
             kAudioDeviceCustomPropertyDeviceAudibleState property.
     */
    EFFDeviceAudibleState       GetState() const noexcept;

    /*! Set the audible state back to kEFFDeviceIsSilent and ignore all previous IO. */
    void                        Reset() noexcept;
    
    /*!
     Read an audio buffer sent by a single device client (i.e. a process playing audio) and update
     the audible state. The update will only affect the return value of GetState after the next
     call to UpdateWithMixedIO, when all IO for the cycle has been read.
     
     Called within 

     Real-time safe. Not thread safe.
     */
    void                        UpdateWithClientIO(bool inClientIsMusicPlayer,
                                                   UInt32 inIOBufferFrameSize,
                                                   Float64 inOutputSampleTime,
                                                   const Float32* inBuffer);
    
    /*!
     Read a fully mixed audio buffer and update the audible state. All client (unmixed) buffers for
     the same cycle must be read with UpdateWithClientIO before calling this function.

     Real-time safe. Not thread safe.

     @return True if the audible state changed.
     */
    bool                        UpdateWithMixedIO(UInt32 inIOBufferFrameSize,
                                                  Float64 inOutputSampleTime,
                                                  const Float32* inBuffer);
    
private:
    bool                        RecalculateState(Float64 inEndFrameSampleTime);

    static bool                 BufferIsAudible(UInt32 inIOBufferFrameSize,
                                                const Float32* inBuffer);

    EFFDeviceAudibleState       mState;

    // TODO: figure out what these exactly are and give appropriate names
    struct
    {
        Float64                 latestAudibleNonMusic;
        Float64                 latestSilent;
        Float64                 latestAudibleMusic;
        Float64                 latestSilentMusic;
    }                           mSampleTimes;

};

#pragma clang assume_nonnull end


#endif /* EFF_AudibleState_h */
