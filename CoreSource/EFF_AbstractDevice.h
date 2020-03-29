//
//  EFF_AbstractDevice.h
//  effervescence-driver
//
//  Created by Nerrons on 24/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

#ifndef EFF_AbstractDevice_h
#define EFF_AbstractDevice_h

// SuperClass Includes
#include "EFF_Object.h"


#pragma clang assume_nonnull begin

class EFF_AbstractDevice
:
    public EFF_Object
{


#pragma mark Construction/Destruction

protected:
                                EFF_AbstractDevice(AudioObjectID inObjectID,
                                                   AudioObjectID inOwnerObjectID);
    virtual                     ~EFF_AbstractDevice();


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
                                                    void* outData) const;
    

#pragma mark IO Operations

public:
    virtual void                StartIO(UInt32 inClientID) = 0;
    virtual void                StopIO(UInt32 inClientID) = 0;

    virtual void                GetZeroTimeStamp(Float64& outSampleTime,
                                                 UInt64& outHostTime,
                                                 UInt64& outSeed) = 0;

    virtual void                WillDoIOOperation(UInt32 inOperationID,
                                                  bool& outWillDo,
                                                  bool& outWillDoInPlace) const = 0;
    virtual void                BeginIOOperation(UInt32 inOperationID,
                                                 UInt32 inIOBufferFrameSize,
                                                 const AudioServerPlugInIOCycleInfo& inIOCycleInfo,
                                                 UInt32 inClientID) = 0;
    virtual void                DoIOOperation(AudioObjectID inStreamObjectID,
                                              UInt32 inClientID,
                                              UInt32 inOperationID,
                                              UInt32 inIOBufferFrameSize,
                                              const AudioServerPlugInIOCycleInfo& inIOCycleInfo,
                                              void* ioMainBuffer,
                                              void* __nullable ioSecondaryBuffer) = 0;
    virtual void                EndIOOperation(UInt32 inOperationID,
                                               UInt32 inIOBufferFrameSize,
                                               const AudioServerPlugInIOCycleInfo& inIOCycleInfo,
                                               UInt32 inClientID) = 0;

    
#pragma mark Implementation

public:
    virtual CFStringRef         CopyDeviceUID() const = 0;
    virtual void                AddClient(const AudioServerPlugInClientInfo* inClientInfo) = 0;
    virtual void                RemoveClient(const AudioServerPlugInClientInfo* inClientInfo) = 0;
    virtual void                PerformConfigChange(UInt64 inChangeAction,
                                                    void* __nullable inChangeInfo) = 0;
    virtual void                AbortConfigChange(UInt64 inChangeAction,
                                                  void* __nullable inChangeInfo) = 0;

};

#pragma clang assume_nonnull end

#endif /* EFF_AbstractDevice_h */
