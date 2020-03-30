//
//  EFF_SoundDevice.h
//  effervescence-app
//
//  Created by Nerrons on 30/3/20.
//  Copyright © 2020 nerrons.
//  Copyright © 2017 Kyle Neideck
//
//  The interface to EFFDevice, the main virtual device published by EFFDriver, and the second
//  instance of that device, which handles UI-related audio. In most cases, users of this class
//  should be able to think of it as representing a single device.
//
//  EFFDevice is the device that appears as "Background Music" in programs that list the output
//  devices, e.g. System Preferences. It receives the system's audio, processes it and sends it to
//  EFFApp by publishing an input stream. EFFApp then plays the audio on the user's real output
//  device.
//
//  See EFFDriver/EFFDriver/EFF_Device.h.
//
// This class is called SoundDevice because it appears in the Sound panel of the preferences.

#ifndef EFF_SoundDevice_h
#define EFF_SoundDevice_h

// Superclass Includes
#include "EFF_AudioDevice.h"

// Local Includes
#include "EFF_Types.h"

// PublicUtility Includes
#include "CACFString.h"

// STL Includes
#include <vector>


#pragma clang assume_nonnull begin

class EFFSoundDevice
:
    public EFFAudioDevice
{
    

#pragma mark Construction/Destruction

public:
    /*!
     @throws CAException If EFFDevice is not found or the HAL returns an error when queried for
        EFFDevice's current Audio Object ID.
     */
                        EFFSoundDevice();
    virtual             ~EFFSoundDevice();
    

#pragma mark Accessor

    AudioObjectID       GetEFFMainDeviceID() const { return GetObjectID(); }
    
#pragma mark OS-wide Default Device

public:
    /*!
     Set EFFDevice as the default audio device for all processes.

     @throws CAException If the HAL responds with an error.
     */
    void                SetAsOSDefault();
    /*!
     Replace EFFDevice as the default device with the output device.

     @throws CAException If the HAL responds with an error.
     */
    void                UnsetAsOSDefault(AudioDeviceID inOutputDeviceID);

    
#pragma mark App Volumes
    
// TODO

    
#pragma mark Audible State
    
// TODO
    
    
#pragma mark Music Player
    
// TODO


#pragma mark UI Sounds Instance

public:
    /*! @return The instance of EFFDevice that handles UI sounds. */
    EFFAudioDevice      GetEFFSystemDeviceInstance() { return mEFFSystemDevice; }

private:
    /*! The instance of EFFDevice that handles UI sounds. */
    EFFAudioDevice      mEFFSystemDevice;

};

#pragma clang assume_nonnull end

#endif /* EFF_SoundDevice_h */
