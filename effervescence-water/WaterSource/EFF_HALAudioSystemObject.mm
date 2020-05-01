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

- (NSInteger) getNumberAudioDevices {
    return audioSystemObject.GetNumberAudioDevices();
}

- (void) getAudioDevicesWithNumber:(NSInteger)inNumberAudioDevices
                   outAudioDevices:(AudioObjectID*)outAudioDevices {
    UInt32 ioNumberAudioDevices = (UInt32)inNumberAudioDevices;
    audioSystemObject.GetAudioDevices(ioNumberAudioDevices, outAudioDevices);
}

- (AudioObjectID) getFirstAudioDevice {
    return audioSystemObject.GetAudioDeviceAtIndex(0);
}

- (AudioObjectID) getOSDefaultMainDeviceID {
    return audioSystemObject.GetDefaultAudioDevice(false, false);
}

- (AudioObjectID) getOSDefaultSystemDeviceID {
    return audioSystemObject.GetDefaultAudioDevice(false, true);
}

- (void) setOSDefaultMainDeviceWithID:(AudioObjectID)mainDeviceID {
    audioSystemObject.SetDefaultAudioDevice(false, false, mainDeviceID);
}

- (void) setOSDefaultSystemDeviceWithID:(AudioObjectID)systemDeviceID {
    audioSystemObject.SetDefaultAudioDevice(false, true, systemDeviceID);
}


@end
