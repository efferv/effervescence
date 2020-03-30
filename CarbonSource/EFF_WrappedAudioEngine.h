//
//  EFF_WrappedAudioEngine.h
//  effervescence-core
//
//  Created by Nerrons on 26/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//
//  Copyright © 2016 Kyle Neideck
//
//  The plan for this is to allow devices with IOAudioEngine drivers to be used as the output device
//  directly from EFFDriver, rather than going through EFFApp. That way we get roughly the same CPU
//  usage and latency as normal, and don't need to worry about pausing EFFApp's IO when no clients
//  are doing IO. It also lets EFFDriver mostly continue working without EFFApp running. I've written
//  a very experimental version that mostly works but the code needs a lot of clean up so I haven't
//  added it to this project yet.
//

#ifndef EFF_WrappedAudioEngine_h
#define EFF_WrappedAudioEngine_h


#include <CoreAudio/CoreAudioTypes.h>
#include <mach/kern_return.h>


class EFF_WrappedAudioEngine
{
    
public:
    UInt64          GetSampleRate() const;
    kern_return_t   SetSampleRate(Float64 inNewSampleRate);
    UInt32          GetSampleBufferFrameSize() const;
    
};

#endif /* EFF_WrappedAudioEngine_h */
