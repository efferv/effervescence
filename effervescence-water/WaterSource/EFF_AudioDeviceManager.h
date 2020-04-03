//
//  NSObject+EFF_AudioDeviceManager.h
//  effervescence-app
//
//  Created by Nerrons on 30/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

#if defined(__cplusplus)

// Local Includes
#import "EFF_SoundDevice.h"

// PublicUtility Includes
#import "CAHALAudioDevice.h"

#endif /* defined(__cplusplus) */

// System Includes
#import <Foundation/Foundation.h>
#import <CoreAudio/AudioHardwareBase.h>


// Forward Declarations
@class EFFOutputVolumeMenuItem;
@class EFFOutputDeviceMenuSection;


#pragma clang assume_nonnull begin

static const int kEFFErrorCode_OutputDeviceNotFound = 1;
static const int kEFFErrorCode_ReturningEarly       = 2;

@interface EFFAudioDeviceManager : NSObject

// Returns nil if EFFDevice isn't installed.
- (instancetype) init;

// To be implemented later.
// Set the EFFOutputVolumeMenuItem to be notified when the output device is changed.
// - (void) setOutputVolumeMenuItem:(EFFOutputVolumeMenuItem*)item;
// Set the EFFOutputDeviceMenuSection to be notified when the output device is changed.
// - (void) setOutputDeviceMenuSection:(EFFOutputDeviceMenuSection*)menuSection;

// Set EFFDevice as the default audio device for all processes
- (NSError* __nullable) setEFFSoundDeviceAsOSDefault;
// Replace EFFDevice as the default device with the output device
- (NSError* __nullable) unsetEFFSoundDeviceAsOSDefault;

#ifdef __cplusplus
// The virtual device published by EFFDriver.
- (EFFSoundDevice) effSoundDevicePtr;

// The device EFFApp will play audio through, making it, from the user's perspective, the system's
// default output device.
- (CAHALAudioDevice) outputDevice;
#endif

- (BOOL) isOutputDevice:(AudioObjectID)deviceID;
- (BOOL) isOutputDataSource:(UInt32)dataSourceID;

/*!
 @abstract Set the real output device that EFFApp uses.
 @returns NSError if the output device couldn't be changed. If revertOnFailure is true in that case,
    this method will attempt to set the output device back to the original device. If it fails to
    revert, an additional error will be included in the error's userInfo with the key "revertError".
 @discussion Both errors' codes will be the code of the exception that caused the failure, if any, generally one
    of the error constants from AudioHardwareBase.h.
    Blocks while the old device stops IO (if there was one).
 */
- (NSError* __nullable) setOutputDeviceWithID:(AudioObjectID)deviceID
                              revertOnFailure:(BOOL)revertOnFailure;

/*!
 @abstract Set the real output device and its data source.
 @discussion See kAudioDevicePropertyDataSource in AudioHardware.h.
    See setOutputDeviceWithID:dataSourceID:revertOnFailure:
 */
- (NSError* __nullable) setOutputDeviceWithID:(AudioObjectID)deviceID
                                 dataSourceID:(UInt32)dataSourceID
                              revertOnFailure:(BOOL)revertOnFailure;

// *** PlayThrough is currently not needed ***
/*
// Start playthrough synchronously. Blocks until IO has started on the output device and playthrough
// is running. See EFFPlayThrough.
//
// Returns one of the error codes defined by this class or EFFPlayThrough, or an AudioHardware error
// code received from the HAL.
- (OSStatus) startPlayThroughSync:(BOOL)forUISoundsDevice;
 */

// *** XPC is currently not needed **
// /*!
//  @abstract Send the ID of the new output device to XPCHelper when the output device changed.
//  */
// - (void) setEFFXPCHelperConnection:(NSXPCConnection* __nullable)connection;

@end

#pragma clang assume_nonnull end
