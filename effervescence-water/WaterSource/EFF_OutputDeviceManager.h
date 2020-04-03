//
//  EFF_PreferredOutputDevices.h
//  effervescence-water
//
//  Created by Nerrons on 3/4/20.
//  Copyright © 2020 nerrons.
//  Copyright © 2018 Kyle Neideck
//
//  Tries to change EFFApp's output device when the user plugs in or unplugs an audio device, in the
//  same way macOS would change its default device if Background Music wasn't running.
//
//  For example, if you plug in some USB headphones, make them your default device and then unplug
//  them, macOS will change its default device to the previous default device. Then, if you plug
//  them back in, macOS will make them the default device again.
//
//  This class isn't always able to figure out what macOS would do, in which case it leaves EFFApp's
//  output device as it is.
//

#ifndef EFF_OutputDeviceManager_h
#define EFF_OutputDeviceManager_h

// Local Includes
#import "EFF_AudioDeviceManager.h"

// System Includes
#import <CoreAudio/AudioHardwareBase.h>
#import <Foundation/Foundation.h>


#pragma clang assume_nonnull begin

@interface EFFOutputDeviceManager : NSObject

/*!
 @discussion Starts responding to device connections/disconnections immediately.
    Stops if/when the instance is deallocated.
 */
- (instancetype) initWithDevices:(EFFAudioDeviceManager*)devices;
//                    userDefaults:(EFFUserDefaults*)userDefaults;


/*!
 @abstract Decide the new output device when the user plugs/unplugs a device.
 @returns The most preferred device currently connected,
    or if there's none, the current output device,
    or if it's disconnected, an arbitrary device,
    or if no device can be connected or HAL returned an error, kAudioObjectUnknown.
 */
- (AudioObjectID) findPreferredDevice;

- (void) userChangedOutputDeviceTo:(AudioObjectID)device;

@end


#pragma clang assume_nonnull end

#endif /* EFF_OutputDeviceManager_h */

