//
//  EFF_WrappedAudioEngine.cpp
//  effervescence-driver
//
//  Created by Nerrons on 26/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//
//  Copyright © 2016 Kyle Neideck

// Self Include
#include "EFF_WrappedAudioEngine.h"


// TODO: Register to be notified when the IO Registry values for these change so we can cache them

UInt64    EFF_WrappedAudioEngine::GetSampleRate() const
{
    return 0;
}

kern_return_t EFF_WrappedAudioEngine::SetSampleRate(Float64 inNewSampleRate)
{
    #pragma unused (inNewSampleRate)
    
    return 0;
}

UInt32 EFF_WrappedAudioEngine::GetSampleBufferFrameSize() const
{
    return 0;
}
