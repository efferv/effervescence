//
//  NSObject+EFF_HALAudioSystemObject.h
//  effervescence-water
//
//  Created by Nerrons on 4/4/20.
//  Copyright © 2020 nerrons. All rights reserved.
//


// System Includes
#import <CoreAudio/AudioHardwareBase.h>
#import <Foundation/Foundation.h>

#pragma clang assume_nonnull begin

@interface EFF_HALAudioSystemObject : NSObject

// Accessors
- (void) getAudioDevicesWithNumber:(UInt32)ioNumberAudioDevices
                   outAudioDevices:(AudioObjectID*)outAudioDevices;
- (AudioObjectID) getFirstAudioDevice;

// Default devices
- (AudioObjectID) getOSDefaultMainDevice;
- (AudioObjectID) getOSDefaultSystemDevice;
- (void) setOSDefaultMainDeviceWithID:(AudioObjectID)mainDeviceID;
- (void) setOSDefaultSystemDeviceWithID:(AudioObjectID)systemDeviceID;

@end

#pragma clang assume_nonnull end
