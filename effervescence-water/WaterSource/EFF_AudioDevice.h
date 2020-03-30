//
//  EFF_AudioDevice.hpp
//  effervescence-app
//
//  Created by Nerrons on 30/3/20.
//  Copyright © 2020 nerrons.
//  Copyright © 2017, 2020 Kyle Neideck
//
//  An HAL audio device. Note that this class's only state is the AudioObjectID of the device.
//

#ifndef EFF_AudioDevice_h
#define EFF_AudioDevice_h

// PublicUtility Includes
#include "CAHALAudioDevice.h"


class EFFAudioDevice
:
    public CAHALAudioDevice
{
    
    
#pragma mark Construction/Destruction

public:
                        EFFAudioDevice(AudioObjectID inAudioDevice);
    /*!
     Creates a EFFAudioDevice with the Audio Object ID of the device whose UID is inUID or, if no
     such device is found, kAudioObjectUnknown.

     @throws CAException If the HAL returns an error when queried for the device's ID.
     @see kAudioPlugInPropertyTranslateUIDToDevice in AudioHardwareBase.h.
     */
                        EFFAudioDevice(CFStringRef inUID);
                        EFFAudioDevice(const CAHALAudioDevice& inDevice);
    virtual             ~EFFAudioDevice();
    
#if defined(__OBJC__)

    // Hack/workaround for Objective-C classes so we don't have to use pointers for instance
    // variables.
                        EFFAudioDevice() : EFFAudioDevice(kAudioObjectUnknown) { }

#endif /* defined(__OBJC__) */

                        operator AudioObjectID() const { return GetObjectID(); }


#pragma mark Accessor
    
    /*!
     @return True if this device is EFFDevice. (Specifically, the main instance of EFFDevice, not
             the instance used for UI sounds.)
     @throws CAException If the HAL returns an error when queried.
     */
    bool                IsEFFMainDevice()       const { return IsEFFDevice(false); };
    /*!
     @return True if this device is either the main instance of EFFDevice (the device named
             "Background Music") or the instance used for UI sounds (the device named "Background
             Music (UI Sounds)").
     @throws CAException If the HAL returns an error when queried.
     */
    bool                IsEFFDeviceInstance()   const { return IsEFFDevice(true); };
        
    /*!
     @return True if this device can be set as the output device in EFFApp.
     @throws CAException If the HAL returns an error when queried.
     */
    bool                CanBeOutputDeviceInEFFApp() const;


#pragma mark Available Controls

    bool                HasSettableMasterVolume(AudioObjectPropertyScope inScope) const;
    bool                HasSettableVirtualMasterVolume(AudioObjectPropertyScope inScope) const;
    bool                HasSettableMasterMute(AudioObjectPropertyScope inScope) const;
    
    
#pragma mark Control Values Accessors

    void                CopyMuteFrom(const EFFAudioDevice inDevice,
                                     AudioObjectPropertyScope inScope);
    void                CopyVolumeFrom(const EFFAudioDevice inDevice,
                                       AudioObjectPropertyScope inScope);

    bool                SetMasterVolumeScalar(AudioObjectPropertyScope inScope, Float32 inVolume);
    
    bool                GetVirtualMasterVolumeScalar(AudioObjectPropertyScope inScope,
                                                     Float32& outVirtualMasterVolume) const;
    bool                SetVirtualMasterVolumeScalar(AudioObjectPropertyScope inScope,
                                                     Float32 inVolume);

    bool                GetVirtualMasterBalance(AudioObjectPropertyScope inScope,
                                                Float32& outVirtualMasterBalance) const;
    
    
#pragma mark Implementation

private:
    bool                IsEFFDevice(bool inIncludingUISoundsInstance) const;

    static OSStatus     AHSGetPropertyData(AudioObjectID inObjectID,
                                           const AudioObjectPropertyAddress* inAddress,
                                           UInt32* ioDataSize,
                                           void* outData);
    static OSStatus     AHSSetPropertyData(AudioObjectID inObjectID,
                                           const AudioObjectPropertyAddress* inAddress,
                                           UInt32 inDataSize,
                                           const void* inData);

};

#endif /* EFF_AudioDevice_h */
