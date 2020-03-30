//
//  EFF_Object.cpp
//  effervescence-driver
//
//  Created by Nerrons on 24/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

// Self Include
#include "EFF_Object.h"

// PublicUtility Includes
#include "CADebugMacros.h"
#include "CAException.h"

//==================================================================================================
#pragma mark -
#pragma mark EFF_Object
//==================================================================================================

#pragma mark Construction/Destruction

EFF_Object::EFF_Object(AudioObjectID inObjectID,
                       AudioClassID inClassID,
                       AudioClassID inBaseClassID,
                       AudioObjectID inOwnerObjectID)
:
    mObjectID(inObjectID),
    mClassID(inClassID),
    mBaseClassID(inBaseClassID),
    mOwnerObjectID(inOwnerObjectID),
    mIsActive(false)
{
}

void    EFF_Object::Activate()
{
    mIsActive = true;
}

void    EFF_Object::Deactivate()
{
    mIsActive = false;
}

EFF_Object::~EFF_Object()
{
}

#pragma mark Property Operations

bool    EFF_Object::HasProperty(AudioObjectID inObjectID,
                                pid_t inClientPID,
                                const AudioObjectPropertyAddress& inAddress)
const
{
    #pragma unused(inObjectID, inClientPID)
    
    bool theAnswer = false;
    switch(inAddress.mSelector)
    {
        case kAudioObjectPropertyBaseClass:
        case kAudioObjectPropertyClass:
        case kAudioObjectPropertyOwner:
        case kAudioObjectPropertyOwnedObjects:
            theAnswer = true;
            break;
    };
    return theAnswer;
}

bool    EFF_Object::IsPropertySettable(AudioObjectID inObjectID,
                                       pid_t inClientPID,
                                       const AudioObjectPropertyAddress& inAddress)
const
{
    #pragma unused(inObjectID, inClientPID)
    
    bool theAnswer = false;
    switch(inAddress.mSelector)
    {
        case kAudioObjectPropertyBaseClass:
        case kAudioObjectPropertyClass:
        case kAudioObjectPropertyOwner:
        case kAudioObjectPropertyOwnedObjects:
            theAnswer = false;
            break;
        
        default:
            Throw(CAException(kAudioHardwareUnknownPropertyError));
    };
    return theAnswer;
}

UInt32    EFF_Object::GetPropertyDataSize(AudioObjectID inObjectID,
                                          pid_t inClientPID,
                                          const AudioObjectPropertyAddress& inAddress,
                                          UInt32 inQualifierDataSize,
                                          const void* inQualifierData)
const
{
    #pragma unused(inObjectID, inClientPID, inQualifierDataSize, inQualifierData)
    
    UInt32 theAnswer = 0;
    switch(inAddress.mSelector)
    {
        case kAudioObjectPropertyBaseClass:
        case kAudioObjectPropertyClass:
            theAnswer = sizeof(AudioClassID);
            break;
            
        case kAudioObjectPropertyOwner:
            theAnswer = sizeof(AudioObjectID);
            break;
            
        case kAudioObjectPropertyOwnedObjects:
            theAnswer = 0;
            break;
            
        default:
            Throw(CAException(kAudioHardwareUnknownPropertyError));
    };
    return theAnswer;
}

void    EFF_Object::GetPropertyData(AudioObjectID inObjectID,
                                    pid_t inClientPID,
                                    const AudioObjectPropertyAddress& inAddress,
                                    UInt32 inQualifierDataSize,
                                    const void* inQualifierData,
                                    UInt32 inDataSize,
                                    UInt32& outDataSize,
                                    void* outData)
const
{
    #pragma unused(inObjectID, inClientPID, inQualifierDataSize, inQualifierData)
    
    switch(inAddress.mSelector)
    {
        case kAudioObjectPropertyBaseClass:
            //    This is the AudioClassID of the base class of this object. This is an invariant.
            ThrowIf(inDataSize < sizeof(AudioClassID),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "EFF_Object::GetPropertyData: not enough space for the return value of kAudioObjectPropertyBaseClass");
            *reinterpret_cast<AudioClassID*>(outData) = mBaseClassID;
            outDataSize = sizeof(AudioClassID);
            break;
            
        case kAudioObjectPropertyClass:
            //    This is the AudioClassID of the class of this object. This is an invariant.
            ThrowIf(inDataSize < sizeof(AudioClassID),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "EFF_Object::GetPropertyData: not enough space for the return value of kAudioObjectPropertyClass");
            *reinterpret_cast<AudioClassID*>(outData) = mClassID;
            outDataSize = sizeof(AudioClassID);
            break;
            
        case kAudioObjectPropertyOwner:
            //    The AudioObjectID of the object that owns this object. This is an invariant.
            ThrowIf(inDataSize < sizeof(AudioObjectID),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "EFF_Object::GetPropertyData: not enough space for the return value of kAudioObjectPropertyOwner");
            *reinterpret_cast<AudioClassID*>(outData) = mOwnerObjectID;
            outDataSize = sizeof(AudioObjectID);
            break;
            
        case kAudioObjectPropertyOwnedObjects:
            //    This is an array of AudioObjectIDs for the objects owned by this object. By default,
            //    objects don't own any other objects. This is an invariant by default, but an object
            //    that can contain other objects will likely need to do some synchronization to access
            //    this property.
            outDataSize = 0;
            break;
        
        default:
            Throw(CAException(kAudioHardwareUnknownPropertyError));
    };
}

void    EFF_Object::SetPropertyData(AudioObjectID inObjectID,
                                    pid_t inClientPID,
                                    const AudioObjectPropertyAddress& inAddress,
                                    UInt32 inQualifierDataSize,
                                    const void* inQualifierData,
                                    UInt32 inDataSize,
                                    const void* inData)
{
    #pragma unused(inObjectID, inClientPID, inQualifierDataSize, inQualifierData, inDataSize, inData)
    
    switch(inAddress.mSelector)
    {
        default:
            Throw(CAException(kAudioHardwareUnknownPropertyError));
    };
}
