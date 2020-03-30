//
//  EFF_NullDevice.cpp
//  effervescence-driver
//
//  Created by Nerrons on 25/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

// Self Include
#include "EFF_NullDevice.h"

// Local Includes
#include "EFF_PlugIn.h"

// PublicUtility Includes
#include "CADebugMacros.h"
#include "CAException.h"
#include "CAPropertyAddress.h"
#include "CADispatchQueue.h"
#include "CAHostTimeBase.h"

#pragma clang assume_nonnull begin

static const Float64 kSampleRate = 44100.0;
static const UInt32 kZeroTimeStampPeriod = 10000;  // Arbitrary.


#pragma mark Construction/Destruction

pthread_once_t     EFF_NullDevice::sStaticInitializer   = PTHREAD_ONCE_INIT;
EFF_NullDevice*    EFF_NullDevice::sInstance            = nullptr;

EFF_NullDevice&    EFF_NullDevice::GetInstance()
{
    pthread_once(&sStaticInitializer, StaticInitializer);
    return *sInstance;
}

void    EFF_NullDevice::StaticInitializer()
{
    try
    {
        sInstance = new EFF_NullDevice;
        // Note that we leave the device inactive initially. EFFApp will activate it when needed.
    }
    catch(...)
    {
        DebugMsg("EFF_NullDevice::StaticInitializer: Failed to create the device");
        delete sInstance;
        sInstance = nullptr;
    }
}

EFF_NullDevice::EFF_NullDevice()
:
    EFF_AbstractDevice(kObjectID_Device_Null, kAudioObjectPlugInObject),
    mStateMutex("Null Device State"),
    mIOMutex("Null Device IO"),
    mStream(kObjectID_Stream_Null, kObjectID_Device_Null, false, kSampleRate)
{
}

EFF_NullDevice::~EFF_NullDevice()
{
}

void    EFF_NullDevice::Activate()
{
    CAMutex::Locker theStateLocker(mStateMutex);

    if(!IsActive())
    {
        // Call the super-class, which just marks the object as active.
        EFF_AbstractDevice::Activate();

        // Calculate the number of host clock ticks per frame for this device's clock.
        mHostTicksPerFrame = CAHostTimeBase::GetFrequency() / kSampleRate;

        SendDeviceIsAlivePropertyNotifications();
    }
}

void    EFF_NullDevice::Deactivate()
{
    CAMutex::Locker theStateLocker(mStateMutex);

    if(IsActive())
    {
        CAMutex::Locker theIOLocker(mIOMutex);

        // Mark the object inactive by calling the super-class.
        EFF_AbstractDevice::Deactivate();

        SendDeviceIsAlivePropertyNotifications();
    }
}

void    EFF_NullDevice::SendDeviceIsAlivePropertyNotifications()
{
    CADispatchQueue::GetGlobalSerialQueue().Dispatch(false, ^{
        AudioObjectPropertyAddress theChangedProperties[] = {
            CAPropertyAddress(kAudioDevicePropertyDeviceIsAlive)
        };

        EFF_PlugIn::Host_PropertiesChanged(GetObjectID(), 1, theChangedProperties);
    });
}


#pragma mark Property Operations

bool    EFF_NullDevice::HasProperty(AudioObjectID inObjectID,
                                    pid_t inClientPID,
                                    const AudioObjectPropertyAddress& inAddress)
const
{
    if(inObjectID == mStream.GetObjectID())
    {
        return mStream.HasProperty(inObjectID, inClientPID, inAddress);
    }

    bool theAnswer = false;
    switch(inAddress.mSelector)
    {
        case kAudioDevicePropertyDeviceCanBeDefaultDevice:
        case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
            theAnswer = true;
            break;

        default:
            theAnswer = EFF_AbstractDevice::HasProperty(inObjectID, inClientPID, inAddress);
            break;
    };
    return theAnswer;
}

bool    EFF_NullDevice::IsPropertySettable(AudioObjectID inObjectID,
                                           pid_t inClientPID,
                                           const AudioObjectPropertyAddress& inAddress)
const
{
    // Forward stream properties.
    if(inObjectID == mStream.GetObjectID())
    {
        return mStream.IsPropertySettable(inObjectID, inClientPID, inAddress);
    }

    bool theAnswer = false;

    switch(inAddress.mSelector)
    {
        default:
            theAnswer = EFF_AbstractDevice::IsPropertySettable(inObjectID, inClientPID, inAddress);
            break;
    };

    return theAnswer;
}

UInt32    EFF_NullDevice::GetPropertyDataSize(AudioObjectID inObjectID,
                                            pid_t inClientPID,
                                            const AudioObjectPropertyAddress& inAddress,
                                            UInt32 inQualifierDataSize,
                                            const void* __nullable inQualifierData)
const
{
    // Forward stream properties.
    if(inObjectID == mStream.GetObjectID())
    {
        return mStream.GetPropertyDataSize(inObjectID,
                                           inClientPID,
                                           inAddress,
                                           inQualifierDataSize,
                                           inQualifierData);
    }

    UInt32 theAnswer = 0;

    switch(inAddress.mSelector)
    {
        case kAudioDevicePropertyStreams:
            theAnswer = 1 * sizeof(AudioObjectID);
            break;

        case kAudioDevicePropertyAvailableNominalSampleRates:
            theAnswer = 1 * sizeof(AudioValueRange);
            break;

        default:
            theAnswer = EFF_AbstractDevice::GetPropertyDataSize(inObjectID,
                                                                inClientPID,
                                                                inAddress,
                                                                inQualifierDataSize,
                                                                inQualifierData);
            break;
    };

    return theAnswer;
}

void    EFF_NullDevice::GetPropertyData(AudioObjectID inObjectID,
                                        pid_t inClientPID,
                                        const AudioObjectPropertyAddress& inAddress,
                                        UInt32 inQualifierDataSize,
                                        const void* __nullable inQualifierData,
                                        UInt32 inDataSize,
                                        UInt32& outDataSize,
                                        void* outData)
const
{
    // Forward stream properties.
    if(inObjectID == mStream.GetObjectID())
    {
        return mStream.GetPropertyData(inObjectID,
                                       inClientPID,
                                       inAddress,
                                       inQualifierDataSize,
                                       inQualifierData,
                                       inDataSize,
                                       outDataSize,
                                       outData);
    }

    // See EFF_Device::Device_GetPropertyData for more information about these properties.
    switch(inAddress.mSelector)
    {
        case kAudioObjectPropertyName:
            ThrowIf(inDataSize < sizeof(AudioObjectID),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "EFF_NullDevice::GetPropertyData: not enough space for the return value of "
                    "kAudioObjectPropertyName for the device");
            *reinterpret_cast<CFStringRef*>(outData) = CFSTR(kNullDeviceName);
            outDataSize = sizeof(CFStringRef);
            break;

        case kAudioObjectPropertyManufacturer:
            ThrowIf(inDataSize < sizeof(AudioObjectID),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "EFF_NullDevice::GetPropertyData: not enough space for the return value of "
                    "kAudioObjectPropertyManufacturer for the device");
            *reinterpret_cast<CFStringRef*>(outData) = CFSTR(kNullDeviceManufacturerName);
            outDataSize = sizeof(CFStringRef);
            break;

        case kAudioDevicePropertyDeviceUID:
            ThrowIf(inDataSize < sizeof(AudioObjectID),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "EFF_NullDevice::GetPropertyData: not enough space for the return value of "
                    "kAudioDevicePropertyDeviceUID for the device");
            *reinterpret_cast<CFStringRef*>(outData) = CFSTR(kEFFNullDeviceUID);
            outDataSize = sizeof(CFStringRef);
            break;

        case kAudioDevicePropertyModelUID:
            ThrowIf(inDataSize < sizeof(AudioObjectID),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "EFF_NullDevice::GetPropertyData: not enough space for the return value of "
                    "kAudioDevicePropertyModelUID for the device");
            *reinterpret_cast<CFStringRef*>(outData) = CFSTR(kEFFNullDeviceModelUID);
            outDataSize = sizeof(CFStringRef);
            break;

        case kAudioDevicePropertyDeviceIsAlive:
            ThrowIf(inDataSize < sizeof(UInt32),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "EFF_NullDevice::GetPropertyData: not enough space for the return value of "
                    "kAudioDevicePropertyDeviceIsAlive for the device");
            *reinterpret_cast<UInt32*>(outData) = IsActive() ? 1 : 0;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioDevicePropertyDeviceIsRunning:
            {
                ThrowIf(inDataSize < sizeof(UInt32),
                        CAException(kAudioHardwareBadPropertySizeError),
                        "EFF_NullDevice::GetPropertyData: not enough space for the return value of "
                        "kAudioDevicePropertyDeviceIsRunning for the device");
                CAMutex::Locker theStateLocker(mStateMutex);
                // 1 means the device is running, i.e. doing IO.
                *reinterpret_cast<UInt32*>(outData) = (mClientsDoingIO > 0) ? 1 : 0;
                outDataSize = sizeof(UInt32);
            }
            break;

        case kAudioDevicePropertyStreams:
            if(inDataSize >= sizeof(AudioObjectID) &&
               (inAddress.mScope == kAudioObjectPropertyScopeGlobal ||
                inAddress.mScope == kAudioObjectPropertyScopeOutput))
            {
                // Return the ID of this device's stream.
                reinterpret_cast<AudioObjectID*>(outData)[0] = kObjectID_Stream_Null;
                // Report how much we wrote.
                outDataSize = 1 * sizeof(AudioObjectID);
            }
            else
            {
                // Return nothing if we don't have a stream of the given scope or there's no room
                // for the response.
                outDataSize = 0;
            }
            break;

        case kAudioDevicePropertyNominalSampleRate:
            ThrowIf(inDataSize < sizeof(Float64),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "EFF_NullDevice::GetPropertyData: not enough space for the return value of "
                    "kAudioDevicePropertyNominalSampleRate for the device");
            *reinterpret_cast<Float64*>(outData) = kSampleRate;
            outDataSize = sizeof(Float64);
            break;

        case kAudioDevicePropertyAvailableNominalSampleRates:
            // Check we were given space to return something.
            if((inDataSize / sizeof(AudioValueRange)) >= 1)
            {
                // This device doesn't support changing the sample rate.
                reinterpret_cast<AudioValueRange*>(outData)[0].mMinimum = kSampleRate;
                reinterpret_cast<AudioValueRange*>(outData)[0].mMaximum = kSampleRate;
                outDataSize = sizeof(AudioValueRange);
            }
            else
            {
                outDataSize = 0;
            }
            break;

        case kAudioDevicePropertyZeroTimeStampPeriod:
            ThrowIf(inDataSize < sizeof(UInt32),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "EFF_NullDevice::GetPropertyData: not enough space for the return value of "
                    "kAudioDevicePropertyZeroTimeStampPeriod for the device");
            *reinterpret_cast<UInt32*>(outData) = kZeroTimeStampPeriod;
            outDataSize = sizeof(UInt32);
            break;

        default:
            EFF_AbstractDevice::GetPropertyData(inObjectID,
                                                inClientPID,
                                                inAddress,
                                                inQualifierDataSize,
                                                inQualifierData,
                                                inDataSize,
                                                outDataSize,
                                                outData);
            break;
    };
}

void    EFF_NullDevice::SetPropertyData(AudioObjectID inObjectID,
                                        pid_t inClientPID,
                                        const AudioObjectPropertyAddress& inAddress,
                                        UInt32 inQualifierDataSize,
                                        const void* inQualifierData,
                                        UInt32 inDataSize,
                                        const void* inData)
{
    // This device doesn't have any settable properties, so just pass stream properties along.
    if(inObjectID == mStream.GetObjectID())
    {
        mStream.SetPropertyData(inObjectID,
                                inClientPID,
                                inAddress,
                                inQualifierDataSize,
                                inQualifierData,
                                inDataSize,
                                inData);
    }
    else if(inObjectID == GetObjectID())
    {
        EFF_AbstractDevice::SetPropertyData(inObjectID,
                                            inClientPID,
                                            inAddress,
                                            inQualifierDataSize,
                                            inQualifierData,
                                            inDataSize,
                                            inData);
    }
    else
    {
        Throw(CAException(kAudioHardwareBadObjectError));
    }
}


#pragma mark IO Operations

void    EFF_NullDevice::StartIO(UInt32 inClientID)
{
    #pragma unused (inClientID)

    CAMutex::Locker theStateLocker(mStateMutex);

    if(mClientsDoingIO == 0)
    {
        // Reset the clock.
        mNumberTimeStamps = 0;
        mAnchorHostTime = CAHostTimeBase::GetTheCurrentTime();

        // Send notifications.
        DebugMsg("EFF_NullDevice::StartIO: Sending kAudioDevicePropertyDeviceIsRunning");
        CADispatchQueue::GetGlobalSerialQueue().Dispatch(false, ^{
            AudioObjectPropertyAddress theChangedProperty[] = {
                CAPropertyAddress(kAudioDevicePropertyDeviceIsRunning)
            };
            EFF_PlugIn::Host_PropertiesChanged(kObjectID_Device_Null, 1, theChangedProperty);
        });
    }

    mClientsDoingIO++;
}

void    EFF_NullDevice::StopIO(UInt32 inClientID)
{
    #pragma unused (inClientID)

    CAMutex::Locker theStateLocker(mStateMutex);

    ThrowIf(mClientsDoingIO == 0,
            CAException(kAudioHardwareIllegalOperationError),
            "EFF_NullDevice::StopIO: Underflowed mClientsDoingIO");

    mClientsDoingIO--;

    if(mClientsDoingIO == 0)
    {
        // Send notifications.
        DebugMsg("EFF_NullDevice::StopIO: Sending kAudioDevicePropertyDeviceIsRunning");
        CADispatchQueue::GetGlobalSerialQueue().Dispatch(false, ^{
            AudioObjectPropertyAddress theChangedProperty[] = {
                CAPropertyAddress(kAudioDevicePropertyDeviceIsRunning)
            };
            EFF_PlugIn::Host_PropertiesChanged(kObjectID_Device_Null, 1, theChangedProperty);
        });
    }
}

void    EFF_NullDevice::GetZeroTimeStamp(Float64& outSampleTime,
                                         UInt64& outHostTime,
                                         UInt64& outSeed)
{
    CAMutex::Locker theIOLocker(mIOMutex);

    // Not sure whether there's actually any point to implementing this. The documentation says that
    // clockless devices don't need to, but if the device doesn't have
    // kAudioDevicePropertyZeroTimeStampPeriod the HAL seems to reject it. So we give it a simple
    // clock similar to the loopback clock in EFF_Device.
    UInt64 theCurrentHostTime = CAHostTimeBase::GetTheCurrentTime();

    // Calculate the next host time.
    Float64 theHostTicksPerPeriod = mHostTicksPerFrame * static_cast<Float64>(kZeroTimeStampPeriod);
    Float64 theHostTickOffset = static_cast<Float64>(mNumberTimeStamps + 1) * theHostTicksPerPeriod;
    UInt64 theNextHostTime = mAnchorHostTime + static_cast<UInt64>(theHostTickOffset);

    // Go to the next period if the next host time is less than the current time.
    if(theNextHostTime <= theCurrentHostTime)
    {
        mNumberTimeStamps++;
    }

    Float64 theHostTicksSinceAnchor =
        (static_cast<Float64>(mNumberTimeStamps) * theHostTicksPerPeriod);

    // Set the return values.
    outSampleTime = mNumberTimeStamps * kZeroTimeStampPeriod;
    outHostTime = static_cast<UInt64>(mAnchorHostTime + theHostTicksSinceAnchor);
    outSeed = 1;
}

void    EFF_NullDevice::WillDoIOOperation(UInt32 inOperationID,
                                          bool& outWillDo,
                                          bool& outWillDoInPlace) const
{
    switch(inOperationID)
    {
        case kAudioServerPlugInIOOperationWriteMix:
            outWillDo = true;
            outWillDoInPlace = true;
            break;

        default:
            outWillDo = false;
            outWillDoInPlace = true;
            break;
            
    };
}

void    EFF_NullDevice::DoIOOperation(AudioObjectID inStreamObjectID,
                                      UInt32 inClientID,
                                      UInt32 inOperationID,
                                      UInt32 inIOBufferFrameSize,
                                      const AudioServerPlugInIOCycleInfo& inIOCycleInfo,
                                      void* ioMainBuffer,
                                      void* __nullable ioSecondaryBuffer)
{
    #pragma unused (inStreamObjectID, inClientID, inOperationID, inIOCycleInfo, inIOBufferFrameSize)
    #pragma unused (ioMainBuffer, ioSecondaryBuffer)
    // Ignore the audio data.
}

#pragma clang assume_nonnull end

