//
//  NSObject+EFF_AudioDeviceManager.m
//  effervescence-app
//
//  Created by Nerrons on 30/3/20.
//  Copyright © 2020 nerrons.
//  Copyright © 2016-2018 Kyle Neideck
//

// Self Include
#import "EFF_AudioDeviceManager.h"

// Local Includes
#import "EFF_Types.h"
#import "EFF_Utils.h"
#import "EFF_AudioDevice.h"
//#import "EFFDeviceControlSync.h"
//#import "EFFOutputDeviceMenuSection.h"
//#import "EFFOutputVolumeMenuItem.h"
//#import "EFFPlayThrough.h"
//#import "EFFXPCProtocols.h"

// PublicUtility Includes
#import "CAAtomic.h"
#import "CAAutoDisposer.h"
#import "CAHALAudioSystemObject.h"


#pragma clang assume_nonnull begin


@implementation EFFAudioDeviceManager {
    // This ivar is a pointer so that EFFBackgroundMusicDevice's constructor doesn't get called
    // during [EFFAudioDeviceManager alloc] when the ivars are initialised. It queries the HAL for
    // EFFDevice's AudioObject ID, which might throw a CAException, most likely because EFFDevice
    // isn't installed.
    //
    // That would be the only way for [EFFAudioDeviceManager alloc] to throw a CAException, so we
    // could wrap that call in a try/catch block instead, but it would make the code a bit
    // confusing.
    EFFSoundDevice* effSoundDevicePtr;
    EFFAudioDevice outputDevice;
    
//    EFFDeviceControlSync deviceControlSync;
//    EFFPlayThrough playThrough;
//    EFFPlayThrough playThrough_UISounds;

    // A connection to EFFXPCHelper so we can send it the ID of the output device.
//    NSXPCConnection* __nullable EFFXPCHelperConnection;

//    EFFOutputVolumeMenuItem* __nullable outputVolumeMenuItem;
//    EFFOutputDeviceMenuSection* __nullable outputDeviceMenuSection;

    NSRecursiveLock* stateLock;
}


#pragma mark Construction/Destruction

- (instancetype) init {
    if ((self = [super init])) {
        stateLock = [NSRecursiveLock new];
//        EFFXPCHelperConnection = nil;
//        outputVolumeMenuItem = nil;
//        outputDeviceMenuSection = nil;
        outputDevice = kAudioObjectUnknown;

        try {
            effSoundDevicePtr = new EFFSoundDevice;
        } catch (const CAException& e) {
            LogError("EFFAudioDeviceManager::init: EFFSoundDevice not found. (%d)", e.GetError());
            self = nil;
            return self;
        }
    }
    
    return self;
}

- (void) dealloc {
    @try {
        [stateLock lock];

        if (effSoundDevicePtr) {
            delete effSoundDevicePtr;
            effSoundDevicePtr = nullptr;
        }
    } @finally {
        [stateLock unlock];
    }
}


#pragma mark Systemwide Default Device

// Note that there are two different "default" output devices on OS X: "output" and "system output". See
// kAudioHardwarePropertyDefaultSystemOutputDevice in AudioHardware.h.

- (NSError* __nullable) setEFFSoundDeviceAsOSDefault {
    try {
        // Intentionally avoid taking stateLock before making calls to the HAL. See
        // startPlayThroughSync.
        CAMemoryBarrier();
        effSoundDevicePtr->SetAsOSDefault();
    } catch (const CAException& e) {
        EFFLogExceptionIn("EFFAudioDeviceManager::setEFFDeviceAsOSDefault", e);
        return [NSError errorWithDomain:@kEFFAppBundleID code:e.GetError() userInfo:nil];
    }

    return nil;
}

- (NSError* __nullable) unsetEFFSoundDeviceAsOSDefault {
    // Copy the devices so we can call the HAL without holding stateLock. See startPlayThroughSync.
    EFFSoundDevice* effSoundDevicePtrCopy;
    AudioDeviceID outputDeviceID;

    @try {
        [stateLock lock];
        effSoundDevicePtrCopy = effSoundDevicePtr;
        outputDeviceID = outputDevice.GetObjectID();
    } @finally {
        [stateLock unlock];
    }

    if (outputDeviceID == kAudioObjectUnknown) {
        return [NSError errorWithDomain:@kEFFAppBundleID
                                   code:kEFFErrorCode_OutputDeviceNotFound
                               userInfo:nil];
    }

    try {
        effSoundDevicePtrCopy->UnsetAsOSDefault(outputDeviceID);
    } catch (const CAException& e) {
        EFFLogExceptionIn("EFFAudioDeviceManager::unsetEFFDeviceAsOSDefault", e);
        return [NSError errorWithDomain:@kEFFAppBundleID code:e.GetError() userInfo:nil];
    }
    
    return nil;
}


#pragma mark Accessors

- (EFFSoundDevice) effSoundDevicePtr {
    return *effSoundDevicePtr;
}

- (CAHALAudioDevice) outputDevice {
    return outputDevice;
}

- (BOOL) isOutputDevice:(AudioObjectID)deviceID {
    @try {
        [stateLock lock];
        return deviceID == outputDevice.GetObjectID();
    } @finally {
        [stateLock lock];
    }
}

- (BOOL) isOutputDataSource:(UInt32)dataSourceID {
    BOOL isOutputDataSource = NO;
    
    @try {
        [stateLock lock];
        try {
            AudioObjectPropertyScope scope = kAudioDevicePropertyScopeOutput;
            UInt32 channel = 0;
            isOutputDataSource = (outputDevice.HasDataSourceControl(scope, channel) &&
                                  (dataSourceID == outputDevice.GetCurrentDataSourceID(scope, channel)));
        } catch (const CAException& e) {
            EFFLogException(e);
        }
    } @finally {
        [stateLock unlock];
    }
    
    return isOutputDataSource;
}


#pragma mark Output Device

- (NSError* __nullable) setOutputDeviceWithID:(AudioObjectID)deviceID
                              revertOnFailure:(BOOL)revertOnFailure {
    return [self setOutputDeviceWithIDImpl:deviceID
                              dataSourceID:nil
                           revertOnFailure:revertOnFailure];
}

- (NSError* __nullable) setOutputDeviceWithID:(AudioObjectID)deviceID
                                 dataSourceID:(UInt32)dataSourceID
                              revertOnFailure:(BOOL)revertOnFailure {
    return [self setOutputDeviceWithIDImpl:deviceID
                              dataSourceID:&dataSourceID
                           revertOnFailure:revertOnFailure];
}

- (NSError* __nullable) setOutputDeviceWithIDImpl:(AudioObjectID)newDeviceID
                                     dataSourceID:(UInt32* __nullable)dataSourceID
                                  revertOnFailure:(BOOL)revertOnFailure {
    DebugMsg("EFFAudioDeviceManager::setOutputDeviceWithIDImpl: Setting output device. newDeviceID=%u",
             newDeviceID);
    
    @try {
        [stateLock lock];

        AudioDeviceID currentDeviceID = outputDevice.GetObjectID();  // (Doesn't throw.)

        try {
            [self setOutputDeviceWithIDImpl:newDeviceID
                               dataSourceID:dataSourceID
                            currentDeviceID:currentDeviceID];
        } catch (const CAException& e) {
            EFFAssert(e.GetError() != kAudioHardwareNoError,
                      "CAException with kAudioHardwareNoError");
            
            return [self failedToSetOutputDevice:newDeviceID
                                       errorCode:e.GetError()
                                        revertTo:(revertOnFailure ? &currentDeviceID : nullptr)];
        } catch (...) {
            return [self failedToSetOutputDevice:newDeviceID
                                       errorCode:kAudioHardwareUnspecifiedError
                                        revertTo:(revertOnFailure ? &currentDeviceID : nullptr)];
        }

        // Tell other classes and EFFXPCHelper that we changed the output device.
        [self propagateOutputDeviceChange];
    } @finally {
        [stateLock unlock];
    }

    return nil;
}

// Throws CAException.
- (void) setOutputDeviceWithIDImpl:(AudioObjectID)newDeviceID
                      dataSourceID:(UInt32* __nullable)dataSourceID
                   currentDeviceID:(AudioObjectID)currentDeviceID {
    if (newDeviceID != currentDeviceID) {
        EFFAudioDevice newOutputDevice(newDeviceID);
//        [self setOutputDeviceForPlaythroughAndControlSync:newOutputDevice];
        outputDevice = newOutputDevice;
    }

    // Set the output device to use the new data source.
    if (dataSourceID) {
        // TODO: If this fails, ideally we'd still start playthrough and return an error, but not
        //       revert the device. It would probably be a bit awkward, though.
        [self setDataSource:*dataSourceID device:outputDevice];
    }

//    if (newDeviceID != currentDeviceID) {
//        // We successfully changed to the new device. Start playthrough on it, since audio might be
//        // playing. (If we only changed the data source, playthrough will already be running if it
//        // needs to be.)
//        playThrough.Start();
//        playThrough_UISounds.Start();
//        // But stop playthrough if audio isn't playing, since it uses CPU.
//        playThrough.StopIfIdle();
//        playThrough_UISounds.StopIfIdle();
//    }

    CFStringRef outputDeviceUID = outputDevice.CopyDeviceUID();
    DebugMsg("EFFAudioDeviceManager::setOutputDeviceWithIDImpl: Set output device to %s (%d)",
             CFStringGetCStringPtr(outputDeviceUID, kCFStringEncodingUTF8),
             outputDevice.GetObjectID());
    CFRelease(outputDeviceUID);
}

- (void) setDataSource:(UInt32)dataSourceID device:(EFFAudioDevice&)device {
    EFFLogAndSwallowExceptions("EFFAudioDeviceManager::setDataSource", ([&] {
        AudioObjectPropertyScope scope = kAudioObjectPropertyScopeOutput;
        UInt32 channel = 0;

        if (device.DataSourceControlIsSettable(scope, channel)) {
            DebugMsg("EFFAudioDeviceManager::setOutputDeviceWithID: Setting dataSourceID=%u",
                     dataSourceID);
            device.SetCurrentDataSourceByID(scope, channel, dataSourceID);
        }
    }));
}

- (void) propagateOutputDeviceChange {
    // Tell EFFXPCHelper that the output device has changed.
//    [self sendOutputDeviceToEFFXPCHelper];

    // Update the menu item for the volume of the output device.
//    [outputVolumeMenuItem outputDeviceDidChange];
//    [outputDeviceMenuSection outputDeviceDidChange];
}

- (NSError*) failedToSetOutputDevice:(AudioDeviceID)deviceID
                           errorCode:(OSStatus)errorCode
                            revertTo:(AudioDeviceID*)revertTo {
    // Using LogWarning from PublicUtility instead of NSLog here crashes from a bad access. Not sure why.
    // TODO: Possibly caused by a bug in CADebugMacros.cpp. See commit ab9d4cd.
    NSLog(@"EFFAudioDeviceManager::failedToSetOutputDevice: Couldn't set device with ID %u as output device. "
          "%s%d. %@",
          deviceID,
          "Error: ", errorCode,
          (revertTo ? [NSString stringWithFormat:@"Will attempt to revert to the previous device. "
                                                  "Previous device ID: %u.", *revertTo] : @""));

    NSDictionary* __nullable info = nil;

    if (revertTo) {
        // Try to reactivate the original device listener and playthrough. (Sorry about the mutual recursion.)
        NSError* __nullable revertError = [self setOutputDeviceWithID:*revertTo revertOnFailure:NO];
        
        if (revertError) {
            info = @{ @"revertError": (NSError*)revertError };
        }
    } else {
        // TODO: Handle this error better in callers. Maybe show an error dialog and try to set the original
        //       default device as the output device.
        NSLog(@"EFFAudioDeviceManager::failedToSetOutputDevice: Failed to revert to the previous device.");
    }
    
    return [NSError errorWithDomain:@kEFFAppBundleID code:errorCode userInfo:info];
}

@end

#pragma clang assume_nonnull end
