//
//  NSObject+EFF_HALAudioSystemObject.m
//  effervescence-water
//
//  Created by Nerrons on 4/4/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

// Local Includes
#import "EFF_HALAudioSystemObject.h"

// PublicUtility Includes
#import "CAHALAudioSystemObject.h"


@implementation EFF_HALAudioSystemObject {
    CAHALAudioSystemObject audioSystemObject;
}

- (void) getAudioDevicesWithNumber:(UInt32)ioNumberAudioDevices
                   outAudioDevices:(AudioObjectID*)outAudioDevices {
    audioSystemObject.GetAudioDevices(ioNumberAudioDevices, outAudioDevices);
}

- (AudioObjectID) getFirstAudioDevice {
    return audioSystemObject.GetAudioDeviceAtIndex(0);
}

- (AudioObjectID) getOSDefaultMainDevice {
    return audioSystemObject.GetDefaultAudioDevice(false, false);
}

- (AudioObjectID) getOSDefaultSystemDevice {
    return audioSystemObject.GetDefaultAudioDevice(false, true);
}

- (void) setOSDefaultMainDeviceWithID:(AudioObjectID)mainDeviceID {
    audioSystemObject.SetDefaultAudioDevice(false, false, mainDeviceID);
}

- (void) setOSDefaultSystemDeviceWithID:(AudioObjectID)systemDeviceID {
    audioSystemObject.SetDefaultAudioDevice(false, true, systemDeviceID);
}


@end
