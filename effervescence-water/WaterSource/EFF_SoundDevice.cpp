//
//  EFF_SoundDevice.cpp
//  effervescence-app
//
//  Created by Nerrons on 30/3/20.
//  Copyright © 2020 nerrons.
//  Copyright © 2016-2019 Kyle Neideck
//  Copyright © 2017 Andrew Tonner
//

// Self Include
#include "EFF_SoundDevice.h"

// Local Includes
#include "EFF_Types.h"
#include "EFF_Utils.h"

// PublicUtility Includes
#include "CADebugMacros.h"
#include "CAHALAudioSystemObject.h"
#include "CACFArray.h"
#include "CACFDictionary.h"

// STL Includes
#include <map>


// Easier getter and setter to os default audio devices
namespace {
AudioObjectID    GetOSDefaultMainDevice(CAHALAudioSystemObject audioSystem)
{
    return audioSystem.GetDefaultAudioDevice(false, false);
}
AudioObjectID    GetOSDefaultSystemDevice(CAHALAudioSystemObject audioSystem)
{
    return audioSystem.GetDefaultAudioDevice(false, true);
}
void    SetOSDefaultMainDevice(CAHALAudioSystemObject audioSystem, AudioObjectID newDevice)
{
    audioSystem.SetDefaultAudioDevice(false, false, newDevice);
}
void    SetOSDefaultSystemDevice(CAHALAudioSystemObject audioSystem, AudioObjectID newDevice)
{
    audioSystem.SetDefaultAudioDevice(false, true, newDevice);
}
}

#pragma clang assume_nonnull begin


#pragma mark Construction/Destruction

EFFSoundDevice::EFFSoundDevice()
:
    EFFAudioDevice(CFSTR(kEFFDeviceUID)),
    mEFFSystemDevice(CFSTR(kEFFDeviceUID_UISounds))
{
    if((GetEFFMainDeviceID() == kAudioObjectUnknown) || (mEFFSystemDevice == kAudioObjectUnknown))
    {
        LogError("EFFSoundDevice::EFFSoundDevice: Error getting EFFDevice ID");
        Throw(CAException(kAudioHardwareIllegalOperationError));
    }
};

EFFSoundDevice::~EFFSoundDevice()
{
}


#pragma mark Systemwide Default Device

void EFFSoundDevice::SetAsOSDefault()
{
    DebugMsg("EFFSoundDevice::SetAsOSDefault: Setting the system's default audio device to EFFDevice");

    CAHALAudioSystemObject audioSystem;

    // AudioDeviceID defaultMainDevice = audioSystem.GetDefaultAudioDevice(/* inIsInput */ false, /* inIsSystem */ false);
    AudioDeviceID defaultMainDevice = GetOSDefaultMainDevice(audioSystem);
    // AudioDeviceID defaultSystemDevice = audioSystem.GetDefaultAudioDevice(false, true);
    AudioDeviceID defaultSystemDevice = GetOSDefaultSystemDevice(audioSystem);

    // If the default system device is the same as the default device, also change default system device
    // to our system device.
    if(defaultSystemDevice == defaultMainDevice)
    {
        // audioSystem.SetDefaultAudioDevice(/* inIsInput */ false, /* inIsSystem */ true, /* inNewDefaultDevice */ mEFFSystemDevice);
        SetOSDefaultSystemDevice(audioSystem, mEFFSystemDevice);
    }

    // audioSystem.SetDefaultAudioDevice(false, false, GetEFFMainDeviceID());
    SetOSDefaultMainDevice(audioSystem, GetEFFMainDeviceID());
}

void EFFSoundDevice::UnsetAsOSDefault(AudioDeviceID inOutputDeviceID)
{
    CAHALAudioSystemObject audioSystem;

    // Set EFFApp's output device as OS X's default output device.
    // bool EFFMainDeviceIsDefault = (audioSystem.GetDefaultAudioDevice(false, false) == GetEFFMainDeviceID());
    bool EFFMainDeviceIsDefault = GetOSDefaultMainDevice(audioSystem) == GetEFFMainDeviceID();

    if(EFFMainDeviceIsDefault)
    {
        DebugMsg("EFFSoundDevice::UnsetAsOSDefault: Setting the system's default output "
                 "device back to device %d", inOutputDeviceID);

        // audioSystem.SetDefaultAudioDevice(false, false, inOutputDeviceID);
        SetOSDefaultMainDevice(audioSystem, inOutputDeviceID);
    }

    // Set EFFApp's output device as OS X's default system output device.
    // bool EFFDeviceIsSystemDefault = (audioSystem.GetDefaultAudioDevice(false, true) == mEFFSystemDevice);
    bool EFFSystemDeviceIsDefault = GetOSDefaultSystemDevice(audioSystem) == mEFFSystemDevice;

    // If we changed the default system output device to EFFDevice, which we only do if it's set to
    // the same device as the default output device, change it back to the previous device.
    if(EFFSystemDeviceIsDefault)
    {
        DebugMsg("EFFSoundDevice::UnsetAsOSDefault: Setting the system's default system "
                 "output device back to device %d", inOutputDeviceID);

        // audioSystem.SetDefaultAudioDevice(false, true, inOutputDeviceID);
        SetOSDefaultSystemDevice(audioSystem, inOutputDeviceID);
    }
}

#pragma clang assume_nonnull end
