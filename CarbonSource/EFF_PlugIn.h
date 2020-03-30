//
//  EFF_Plugin.h
//  effervescence-core
//
//  Created by Nerrons on 29/3/20.
//  Copyright © 2020 nerrons.
//  Copyright © 2016 Kyle Neideck
//  Copyright (C) 2013 Apple Inc. All Rights Reserved.
//
//  Based largely on SA_PlugIn.h from Apple's SimpleAudioDriver Plug-In sample code.
//  https://developer.apple.com/library/mac/samplecode/AudioDriverExamples
//

#ifndef EFF_Plugin_h
#define EFF_Plugin_h

// SuperClass Includes
#include "EFF_Object.h"

// Local Includes
#include "EFF_Types.h"

// PublicUtility Includes
#include "CAMutex.h"


class EFF_PlugIn
:
    public EFF_Object
{


#pragma mark Construction/Destruction
    
public:
    static EFF_PlugIn&              GetInstance();

protected:
                                    EFF_PlugIn();
    virtual                         ~EFF_PlugIn();
    virtual void                    Deactivate();
    
private:
    static void                     StaticInitializer();


#pragma mark Host Access
    
public:
    static void                     SetHost(AudioServerPlugInHostRef inHost)
                                        { sHost = inHost; }

    static void                     Host_PropertiesChanged(AudioObjectID inObjectID,
                                                           UInt32 inNumberAddresses,
                                                           const AudioObjectPropertyAddress inAddresses[])
                                        { if(sHost != NULL) {
                                            sHost->PropertiesChanged(sHost,
                                                                     inObjectID,
                                                                     inNumberAddresses,
                                                                     inAddresses); } }

    static void                     Host_RequestDeviceConfigurationChange(AudioObjectID inDeviceObjectID,
                                                                          UInt64 inChangeAction,
                                                                          void* inChangeInfo)
                                        { if(sHost != NULL) {
                                            sHost->RequestDeviceConfigurationChange(sHost,
                                                                                    inDeviceObjectID,
                                                                                    inChangeAction,
                                                                                    inChangeInfo); } }

#pragma mark Property Operations
    
public:
    virtual bool                    HasProperty(AudioObjectID inObjectID,
                                                pid_t inClientPID,
                                                const AudioObjectPropertyAddress& inAddress) const;
    virtual bool                    IsPropertySettable(AudioObjectID inObjectID,
                                                       pid_t inClientPID,
                                                       const AudioObjectPropertyAddress& inAddress) const;
    virtual UInt32                  GetPropertyDataSize(AudioObjectID inObjectID,
                                                        pid_t inClientPID,
                                                        const AudioObjectPropertyAddress& inAddress,
                                                        UInt32 inQualifierDataSize,
                                                        const void* inQualifierData) const;
    virtual void                    GetPropertyData(AudioObjectID inObjectID,
                                                    pid_t inClientPID,
                                                    const AudioObjectPropertyAddress& inAddress,
                                                    UInt32 inQualifierDataSize,
                                                    const void* inQualifierData,
                                                    UInt32 inDataSize,
                                                    UInt32& outDataSize,
                                                    void* outData) const;
    virtual void                    SetPropertyData(AudioObjectID inObjectID,
                                                    pid_t inClientPID,
                                                    const AudioObjectPropertyAddress& inAddress,
                                                    UInt32 inQualifierDataSize,
                                                    const void* inQualifierData,
                                                    UInt32 inDataSize,
                                                    const void* inData);
    
    
#pragma mark Implementation
    
public:
    const CFStringRef               GetBundleID() const { return CFSTR(kEFFDriverBundleID); }
    
private:
    CAMutex                         mMutex;
    
    static pthread_once_t           sStaticInitializer;
    static EFF_PlugIn*              sInstance;
    static AudioServerPlugInHostRef sHost;

};


#endif /* EFF_Plugin_h */
