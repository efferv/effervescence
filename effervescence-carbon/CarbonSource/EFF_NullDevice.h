//
//  EFF_NullDevice.h
//  effervescence-core
//
//  Created by Nerrons on 25/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

#ifndef __EFF_NullDevice_h
#define __EFF_NullDevice_h

// SuperClass Includes
#include "EFF_AbstractDevice.h"

// Local Includes
#include "EFF_Types.h"
#include "EFF_Stream.h"

// PublicUtility Includes
#include "CAMutex.h"

// System Includes
#include <pthread.h>


#pragma clang assume_nonnull begin

class EFF_NullDevice
:
    public EFF_AbstractDevice
{

    
#pragma mark Construction/Destruction

public:
    static EFF_NullDevice&      GetInstance();
    virtual void                Activate();
    virtual void                Deactivate();

protected:
                                EFF_NullDevice();
    virtual                     ~EFF_NullDevice();
    
private:
    static void                 StaticInitializer();
    void                        SendDeviceIsAlivePropertyNotifications();

    
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
                                                const void* inQualifierData,
                                                UInt32 inDataSize,
                                                const void* inData);

    
#pragma mark IO Operations

public:
    void                        StartIO(UInt32 inClientID);
    void                        StopIO(UInt32 inClientID);

    void                        GetZeroTimeStamp(Float64& outSampleTime,
                                                 UInt64& outHostTime,
                                                 UInt64& outSeed);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
    void                        WillDoIOOperation(UInt32 inOperationID,
                                                  bool& outWillDo,
                                                  bool& outWillDoInPlace) const;
    void                        BeginIOOperation(UInt32 inOperationID,
                                                 UInt32 inIOBufferFrameSize,
                                                 const AudioServerPlugInIOCycleInfo& inIOCycleInfo,
                                                 UInt32 inClientID) { /* No-op */ };
    void                        DoIOOperation(AudioObjectID inStreamObjectID,
                                              UInt32 inClientID,
                                              UInt32 inOperationID,
                                              UInt32 inIOBufferFrameSize,
                                              const AudioServerPlugInIOCycleInfo& inIOCycleInfo,
                                              void* ioMainBuffer,
                                              void* __nullable ioSecondaryBuffer);
    void                        EndIOOperation(UInt32 inOperationID,
                                               UInt32 inIOBufferFrameSize,
                                               const AudioServerPlugInIOCycleInfo& inIOCycleInfo,
                                               UInt32 inClientID) { /* No-op */ };


#pragma mark Implementation

public:
    CFStringRef                 CopyDeviceUID() const
                                    { return CFSTR(kEFFNullDeviceUID); };
    void                        AddClient(const AudioServerPlugInClientInfo* inClientInfo)
                                    { /* No-op */ };
    void                        RemoveClient(const AudioServerPlugInClientInfo* inClientInfo)
                                    { /* No-op */ };
    void                        PerformConfigChange(UInt64 inChangeAction,
                                                    void* __nullable inChangeInfo)
                                    { /* No-op */ };
    void                        AbortConfigChange(UInt64 inChangeAction,
                                                  void* __nullable inChangeInfo)
                                    { /* No-op */ };

#pragma clang diagnostic pop

private:
    static pthread_once_t       sStaticInitializer;
    static EFF_NullDevice*      sInstance;

    #define kNullDeviceName             "Effervescence Null Device"
    #define kNullDeviceManufacturerName "Effervescence contributors"

    CAMutex                     mStateMutex;
    CAMutex                     mIOMutex;

    EFF_Stream                  mStream;

    UInt32                      mClientsDoingIO    = 0;

    Float64                     mHostTicksPerFrame = 0.0;
    UInt64                      mNumberTimeStamps  = 0;
    UInt64                      mAnchorHostTime    = 0;

};

#pragma clang assume_nonnull end

#endif /* __EFF_NullDevice_h */
