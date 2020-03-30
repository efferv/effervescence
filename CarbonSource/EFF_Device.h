//
//  EFF_Device.h
//  effervescence-core
//
//  Created by Nerrons on 27/3/20.
//  Copyright © 2020 nerrons.
//  Copyright © 2016, 2017, 2019 Kyle Neideck
//  Copyright © 2019 Gordon Childs
//

#ifndef EFF_Device_h
#define EFF_Device_h


// SuperClass Includes
#include "EFF_AbstractDevice.h"

// Local Includes
#include "EFF_Types.h"
#include "EFF_WrappedAudioEngine.h"
#include "EFF_Clients.h"
#include "EFF_TaskQueue.h"
#include "EFF_AudibleState.h"
#include "EFF_Stream.h"
#include "EFF_VolumeControl.h"
#include "EFF_MuteControl.h"

// PublicUtility Includes
#include "CAMutex.h"
#include "CAVolumeCurve.h"
#include "CARingBuffer.h"

// System Includes
#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>

class EFF_Device
:
    public EFF_AbstractDevice
{


#pragma mark Construction/Destruction
    
public:
    static EFF_Device&          GetInstance();
    static EFF_Device&          GetUISoundsInstance();

protected:
                                EFF_Device(AudioObjectID inObjectID,
                                           const CFStringRef __nonnull inDeviceName,
                                           const CFStringRef __nonnull inDeviceUID,
                                           const CFStringRef __nonnull inDeviceModelUID,
                                           AudioObjectID inInputStreamID,
                                           AudioObjectID inOutputStreamID,
                                           AudioObjectID inOutputVolumeControlID,
                                           AudioObjectID inOutputMuteControlID);
    virtual                     ~EFF_Device();

    virtual void                Activate();
    virtual void                Deactivate();
    
private:
    void                        InitLoopback();
    static void                 StaticInitializer();
    
    
#pragma mark Property Operations
    
public:
    virtual bool                HasProperty(AudioObjectID inObjectID,
                                            pid_t inClientPID,
                                            const AudioObjectPropertyAddress& inAddress) const;
    virtual bool                IsPropertySettable(AudioObjectID inObjectID,
                                                   pid_t inClientPID,
                                                   const AudioObjectPropertyAddress& inAddress) const;
    virtual UInt32              GetPropertyDataSize(AudioObjectID inObjectID,
                                                    pid_t inClientPID,
                                                    const AudioObjectPropertyAddress& inAddress,
                                                    UInt32 inQualifierDataSize,
                                                    const void* __nullable inQualifierData) const;
    virtual void                GetPropertyData(AudioObjectID inObjectID,
                                                pid_t inClientPID,
                                                const AudioObjectPropertyAddress& inAddress,
                                                UInt32 inQualifierDataSize,
                                                const void* __nullable inQualifierData,
                                                UInt32 inDataSize,
                                                UInt32& outDataSize,
                                                void* __nonnull outData) const;
    virtual void                SetPropertyData(AudioObjectID inObjectID,
                                                pid_t inClientPID,
                                                const AudioObjectPropertyAddress& inAddress,
                                                UInt32 inQualifierDataSize,
                                                const void* __nullable inQualifierData,
                                                UInt32 inDataSize,
                                                const void* __nonnull inData);
    
    
#pragma mark Device Property Operations
// these are for the EFFDevice itself, not the owned objects
    
private:
    bool                        Device_HasProperty(AudioObjectID inObjectID,
                                                   pid_t inClientPID,
                                                   const AudioObjectPropertyAddress& inAddress) const;
    bool                        Device_IsPropertySettable(AudioObjectID inObjectID,
                                                          pid_t inClientPID,
                                                          const AudioObjectPropertyAddress& inAddress) const;
    UInt32                      Device_GetPropertyDataSize(AudioObjectID inObjectID,
                                                           pid_t inClientPID,
                                                           const AudioObjectPropertyAddress& inAddress,
                                                           UInt32 inQualifierDataSize,
                                                           const void* __nullable inQualifierData) const;
    void                        Device_GetPropertyData(AudioObjectID inObjectID,
                                                       pid_t inClientPID,
                                                       const AudioObjectPropertyAddress& inAddress,
                                                       UInt32 inQualifierDataSize,
                                                       const void* __nullable inQualifierData,
                                                       UInt32 inDataSize,
                                                       UInt32& outDataSize,
                                                       void* __nonnull outData) const;
    void                        Device_SetPropertyData(AudioObjectID inObjectID,
                                                       pid_t inClientPID,
                                                       const AudioObjectPropertyAddress& inAddress,
                                                       UInt32 inQualifierDataSize,
                                                       const void* __nullable inQualifierData,
                                                       UInt32 inDataSize,
                                                       const void* __nonnull inData);

    
#pragma mark IO Operations
    
public:
    void                        StartIO(UInt32 inClientID);
    void                        StopIO(UInt32 inClientID);

    void                        GetZeroTimeStamp(Float64& outSampleTime,
                                                 UInt64& outHostTime,
                                                 UInt64& outSeed);
    
    void                        WillDoIOOperation(UInt32 inOperationID,
                                                  bool& outWillDo,
                                                  bool& outWillDoInPlace) const;
    void                        BeginIOOperation(UInt32 inOperationID,
                                                 UInt32 inIOBufferFrameSize,
                                                 const AudioServerPlugInIOCycleInfo& inIOCycleInfo,
                                                 UInt32 inClientID);
    /*!
     @discussion All operations take IO lock.
        For each type of kAudioServerPlugInIOOperation{...}, we do:
        ReadInput: Call ReadInputData() to copy from mLoopbackRingBuffer to ioMainBuffer
        ProcessOutput: For inClientID, update audible state for that client and apply relative volume
        ProcessMix: The device applies its own volume
        WriteMix: Update audible state for the mix; copy data from ioMainBuffer to mLoopbackRingBuffer
     */
    void                        DoIOOperation(AudioObjectID inStreamObjectID,
                                              UInt32 inClientID,
                                              UInt32 inOperationID,
                                              UInt32 inIOBufferFrameSize,
                                              const AudioServerPlugInIOCycleInfo& inIOCycleInfo,
                                              void* __nonnull ioMainBuffer,
                                              void* __nullable ioSecondaryBuffer);
    void                        EndIOOperation(UInt32 inOperationID,
                                               UInt32 inIOBufferFrameSize,
                                               const AudioServerPlugInIOCycleInfo& inIOCycleInfo,
                                               UInt32 inClientID);

private:
    /*!
     @abstract Copy data in mLoopbackRingBuffer at inSampleTime to outBuffer
     @throws CAException(kAudioHardwareIllegalOperationError)
     @throws CAException(kAudioHardwareUnspecifiedError)
     */
    void                        ReadInputData(UInt32 inIOBufferFrameSize,
                                              Float64 inSampleTime,
                                              void* __nonnull outBuffer);
    /*!
     @abstract Copy data in inBuffer at inSampleTime to mLoopbackRingBuffer
     @throws CAException
     */
    void                        WriteOutputData(UInt32 inIOBufferFrameSize,
                                                Float64 inSampleTime,
                                                const void* __nonnull inBuffer);
    /*!
     @abstract Applies volume and panning settings to a buffer with two channels.
     */
    void                        ApplyClientRelativeVolume(UInt32 inClientID,
                                                          UInt32 inIOBufferFrameSize,
                                                          void* __nonnull inBuffer) const;
    

#pragma mark Accessors

public:
    /*!
     @abstract Request to change enable or disable the device's volume and/or mute controls.
     @discussion This function is async because it has to ask the host to stop IO for the device
        before the controls can be enabled/disabled.
        See EFF_Device::PerformConfigChange and RequestDeviceConfigurationChange in AudioServerPlugIn.h.
     */
    void                        RequestEnabledControls(bool inVolumeEnabled, bool inMuteEnabled);

    Float64                     GetSampleRate() const;
    /*!
     @abstract Request to change the sample rate of the device.
     @discussion This function is async because it has to ask the host to stop IO for the device
        before the sample rate can be changed.
        See EFF_Device::PerformConfigChange and RequestDeviceConfigurationChange in AudioServerPlugIn.h.
     */
    void                        RequestSampleRate(Float64 inRequestedSampleRate);

private:
    /*!
     @return The AudioObject that has the ID inObjectID and belongs to this device.
     @throws CAException(kAudioHardwareBadObjectError) if there is no such AudioObject.
     */
    const EFF_Object&           GetOwnedObjectByID(AudioObjectID inObjectID) const;
    EFF_Object&                 GetOwnedObjectByID(AudioObjectID inObjectID);
    
    /*! @return The number of Audio Objects belonging to this device, e.g. streams and controls. */
    UInt32                      GetNumberOfSubObjects() const;
    /*! @return The number of Audio Objects with output scope belonging to this device. */
    UInt32                      GetNumberOfOutputSubObjects() const;
    /*!
     @return The number of control Audio Objects with output scope belonging to this device, e.g.
             output volume and mute controls.
     */
    UInt32                      GetNumberOfOutputControls() const;
    
    /*!
     Enable or disable the device's volume and/or mute controls.

     Private because (after initialisation) this can only be called after asking the host to stop IO
     for the device. See EFF_Device::RequestEnabledControls, EFF_Device::PerformConfigChange and
     RequestDeviceConfigurationChange in AudioServerPlugIn.h.
     */
    void                        SetEnabledControls(bool inVolumeEnabled, bool inMuteEnabled);
    /*!
     Set the device's sample rate.

     Private because (after initialisation) this can only be called after asking the host to stop IO
     for the device. See EFF_Device::RequestEnabledControls, EFF_Device::PerformConfigChange and
     RequestDeviceConfigurationChange in AudioServerPlugIn.h.

     @param inNewSampleRate The sample rate.
     @param force If true, set the sample rate on the device even if it's already inNewSampleRate.
     @throws CAException if inNewSampleRate < 1 or if applying the sample rate to one of the streams fails.
     */
    void                        SetSampleRate(Float64 inNewSampleRate, bool force = false);
    
    /*! @return True if inObjectID is the ID of one of this device's streams. */
    inline bool                 IsStreamID(AudioObjectID inObjectID) const noexcept;
    
    
#pragma mark Hardware Accessors
    
private:
    void                        _HW_Open();
    void                        _HW_Close();
    kern_return_t               _HW_StartIO();
    void                        _HW_StopIO();
    Float64                     _HW_GetSampleRate() const;
    kern_return_t               _HW_SetSampleRate(Float64 inNewSampleRate);
    UInt32                      _HW_GetRingBufferFrameSize() const;
    
    
#pragma mark Implementation
    
public:
    CFStringRef __nonnull       CopyDeviceUID() const { return mDeviceUID; }
    void                        AddClient(const AudioServerPlugInClientInfo* __nonnull inClientInfo);
    void                        RemoveClient(const AudioServerPlugInClientInfo* __nonnull inClientInfo);
    /*!
     Apply a change requested with EFF_PlugIn::Host_RequestDeviceConfigurationChange. See
     PerformDeviceConfigurationChange in AudioServerPlugIn.h.
     */
    void                        PerformConfigChange(UInt64 inChangeAction, void* __nullable inChangeInfo);
    /*! Cancel a change requested with EFF_PlugIn::Host_RequestDeviceConfigurationChange. */
    void                        AbortConfigChange(UInt64 inChangeAction, void* __nullable inChangeInfo);

private:
    static pthread_once_t               sStaticInitializer;
    static EFF_Device* __nonnull        sInstance;
    static EFF_Device* __nonnull        sUISoundsInstance;
    
    #define kDeviceName                 "Effervescence Device"
    #define kDeviceName_UISounds        "Effervescence Device (UI Sounds)"
    #define kDeviceManufacturerName     "Effervescence contributors"

    const CFStringRef __nonnull         mDeviceName;
    const CFStringRef __nonnull         mDeviceUID;
    const CFStringRef __nonnull         mDeviceModelUID;
    
    enum
    {
        // The number of global/output sub-objects varies because the controls can be disabled.
                                        kNumberOfInputSubObjects            = 1,

                                        kNumberOfStreams                    = 2,
                                        kNumberOfInputStreams               = 1,
                                        kNumberOfOutputStreams              = 1
    };

    CAMutex                             mStateMutex;
    CAMutex                             mIOMutex;
    
    const Float64                       kSampleRateDefault = 44100.0;
    // Before we can change sample rate, the host has to stop the device. The new sample rate is
    // stored here while it does. Like the shadow maps in EFF_ClientMap
    Float64                             mPendingSampleRate = kSampleRateDefault;
    
    EFF_WrappedAudioEngine* __nullable  mWrappedAudioEngine;
    
    EFF_TaskQueue                       mTaskQueue;
    
    EFF_Clients                         mClients;
    
    #define kLoopbackRingBufferFrameSize    16384
    Float64                             mLoopbackSampleRate;
    CARingBuffer                        mLoopbackRingBuffer;
    
    // TODO: a comment explaining why we need a clock for loopback-only mode
    // sample time := frames passed from hardware initialization to now == numberTimeStamps * kLoopbackRingBufferFrameSize
    // host time := ticks passed from genesis to now == (anchorHostTime +
    //                                                   numberTimeStamps * kLoopbackRingBufferFrameSize * hostTicksPerFrame)
    // Side note: What GetZeroTimeStamp() does is return the timestamp of the next "rewind" of the ring buffer
    //     in both sample time and host time formats
    struct {
        Float64                         hostTicksPerFrame = 0.0;    // Totally depends on the host
        UInt64                          numberTimeStamps  = 0;      // # of sample times passed since _HW_StartIO
        UInt64                          anchorHostTime    = 0;      // The host time when IO started in hardware
    }                                   mLoopbackTime;
    
    EFF_Stream                          mInputStream;
    EFF_Stream                          mOutputStream;

    EFF_AudibleState                    mAudibleState;
    
    enum class ChangeAction : UInt64
    {
        SetSampleRate,
        SetEnabledControls
    };
    
    EFF_VolumeControl                   mVolumeControl;
    EFF_MuteControl                     mMuteControl;
    bool                                mPendingOutputVolumeControlEnabled = true;
    bool                                mPendingOutputMuteControlEnabled   = true;

};


#endif /* EFF_Device_h */
