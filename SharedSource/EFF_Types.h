//
//  EFF_Types.h
//  effervescence-core
//
//  Created by Nerrons on 24/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

#ifndef EFF_Types_h
#define EFF_Types_h

// System Includes
#include <CoreAudio/AudioServerPlugIn.h>

// STL Includes
#if defined(__cplusplus)
#include <stdexcept>
#endif


#pragma mark Project URLs

static const char* const kEFFProjectURL = "https://github.com/nerrons/effervescence";
static const char* const kEFFIssueTrackerURL = "https://github.com/nerrons/effervescence/issues";


#pragma mark IDs

// TODO: Change these and the other defines to const strings?
#define kEFFDriverBundleID           "com.nerrons.effervescence.driver"
#define kEFFAppBundleID              "com.nerrons.effervescence.app"
#define kEFFXPCHelperBundleID        "com.nerrons.effervescence.xpchelper"

#define kEFFDeviceUID                "EFFDevice"
#define kEFFDeviceModelUID           "EFFDeviceModelUID"
#define kEFFDeviceUID_UISounds       "EFFDevice_UISounds"
#define kEFFDeviceModelUID_UISounds  "EFFDeviceModelUID_UISounds"
#define kEFFNullDeviceUID            "EFFNullDevice"
#define kEFFNullDeviceModelUID       "EFFNullDeviceModelUID"

// The object IDs for the audio objects this driver implements.
//
// EFFDevice always publishes this fixed set of objects (except when EFFDevice's volume or mute
// controls are disabled). We might need to change that at some point, but so far it hasn't caused
// any problems and it makes the driver much simpler.

enum
{
    kObjectID_PlugIn                            = kAudioObjectPlugInObject,
    // EFFDevice
    kObjectID_Device                            = 2,   // Belongs to kObjectID_PlugIn
    kObjectID_Stream_Input                      = 3,   // Belongs to kObjectID_Device
    kObjectID_Stream_Output                     = 4,   // Belongs to kObjectID_Device
    kObjectID_Volume_Output_Master              = 5,   // Belongs to kObjectID_Device
    kObjectID_Mute_Output_Master                = 6,   // Belongs to kObjectID_Device
    // Null Device
    kObjectID_Device_Null                       = 7,   // Belongs to kObjectID_PlugIn
    kObjectID_Stream_Null                       = 8,   // Belongs to kObjectID_Device_Null
    // EFFDevice for UI sounds
    kObjectID_Device_UI_Sounds                  = 9,   // Belongs to kObjectID_PlugIn
    kObjectID_Stream_Input_UI_Sounds            = 10,  // Belongs to kObjectID_Device_UI_Sounds
    kObjectID_Stream_Output_UI_Sounds           = 11,  // Belongs to kObjectID_Device_UI_Sounds
    kObjectID_Volume_Output_Master_UI_Sounds    = 12,  // Belongs to kObjectID_Device_UI_Sounds
};

// AudioObjectPropertyElement docs: "Elements are numbered sequentially where 0 represents the
// master element."
static const AudioObjectPropertyElement kMasterChannel = kAudioObjectPropertyElementMaster;


#pragma mark EFFPlugIn Custom Properties

enum
{
    // A CFBoolean. True if the null device is enabled. Settable, false by default.
    kAudioPlugInCustomPropertyNullDeviceActive = 'nuld'
};


#pragma mark EFFDevice Custom Properties

enum
{
    // TODO: Combine the two music player properties
    
    // The process ID of the music player as a CFNumber. Setting this property will also clear the value of
    // kAudioDeviceCustomPropertyMusicPlayerBundleID. We use 0 to mean unset.
    //
    // There is currently no way for a client to tell whether the process it has set as the music player is a
    // client of the EFFDevice.
    kAudioDeviceCustomPropertyMusicPlayerProcessID                    = 'mppi',
    // The music player's bundle ID as a CFString (UTF8), or the empty string if it's unset/null. Setting this
    // property will also clear the value of kAudioDeviceCustomPropertyMusicPlayerProcessID.
    kAudioDeviceCustomPropertyMusicPlayerBundleID                     = 'mpbi',
    // A CFNumber that specifies whether the device is silent, playing only music (i.e. the client set as the
    // music player is the only client playing audio) or audible. See enum values below. This property is only
    // updated after the audible state has been different for kDeviceAudibleStateMinChangedFramesForUpdate
    // consecutive frames. (To avoid excessive CPU use if for some reason the audible state starts changing
    // very often.)
    kAudioDeviceCustomPropertyDeviceAudibleState                      = 'daud',
    // A CFBoolean similar to kAudioDevicePropertyDeviceIsRunning except it ignores whether IO is running for
    // EFFApp. This is so EFFApp knows when it can stop doing IO to save CPU.
    kAudioDeviceCustomPropertyDeviceIsRunningSomewhereOtherThanEFFApp = 'runo',
    // A CFArray of CFDictionaries that each contain an app's pid, bundle ID and volume relative to other
    // running apps. See the dictionary keys below for more info.
    //
    // Getting this property will only return apps with volumes other than the default. Setting this property
    // will add new app volumes or replace existing ones, but there's currently no way to delete an app from
    // the internal collection.
    kAudioDeviceCustomPropertyAppVolumes                              = 'apvs',
    // A CFArray of CFBooleans indicating which of EFFDevice's controls are enabled. All controls are enabled
    // by default. This property is settable. See the array indices below for more info.
    kAudioDeviceCustomPropertyEnabledOutputControls                   = 'bgct'
};

// The number of silent/audible frames before EFFDriver will change kAudioDeviceCustomPropertyDeviceAudibleState
#define kDeviceAudibleStateMinChangedFramesForUpdate (2 << 11)

enum EFFDeviceAudibleState : SInt32
{
    // kAudioDeviceCustomPropertyDeviceAudibleState values
    //
    // No audio is playing on the device's streams (regardless of whether IO is running or not)
    kEFFDeviceIsSilent              = 'silt',
    // The client whose bundle ID matches the current value of kCustomAudioDevicePropertyMusicPlayerBundleID is the
    // only audible client
    kEFFDeviceIsSilentExceptMusic   = 'olym',
    kEFFDeviceIsAudible             = 'audi'
};

// kAudioDeviceCustomPropertyAppVolumes keys
//
// A CFNumber<SInt32> between kAppRelativeVolumeMinRawValue and kAppRelativeVolumeMaxRawValue. A value greater than
// the midpoint increases the client's volume and a value less than the midpoint decreases it. A volume curve is
// applied to kEFFAppVolumesKey_RelativeVolume when it's first set and then each of the app's samples are multiplied
// by it.
#define kEFFAppVolumesKey_RelativeVolume    "rvol"
// A CFNumber<SInt32> between kAppPanLeftRawValue and kAppPanRightRawValue. A negative value has a higher proportion
// of left channel, and a positive value has a higher proportion of right channel.
#define kEFFAppVolumesKey_PanPosition       "ppos"
// The app's pid as a CFNumber. May be omitted if kEFFAppVolumesKey_BundleID is present.
#define kEFFAppVolumesKey_ProcessID         "pid"
// The app's bundle ID as a CFString. May be omitted if kEFFAppVolumesKey_ProcessID is present.
#define kEFFAppVolumesKey_BundleID          "bid"

// Volume curve range for app volumes
#define kAppRelativeVolumeMaxRawValue   100
#define kAppRelativeVolumeMinRawValue   0
#define kAppRelativeVolumeMinDbValue    -96.0f
#define kAppRelativeVolumeMaxDbValue    0.0f

// Pan position values
#define kAppPanLeftRawValue   -100
#define kAppPanCenterRawValue 0
#define kAppPanRightRawValue  100

// kAudioDeviceCustomPropertyEnabledOutputControls indices
enum
{
    // True if EFFDevice's master output volume control is enabled.
    kEFFEnabledOutputControlsIndex_Volume = 0,
    // True if EFFDevice's master output mute control is enabled.
    kEFFEnabledOutputControlsIndex_Mute   = 1
};


#pragma mark EFFDevice Custom Property Addresses

// For convenience.

static const AudioObjectPropertyAddress kEFFMusicPlayerProcessIDAddress = {
    kAudioDeviceCustomPropertyMusicPlayerProcessID,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster
};

static const AudioObjectPropertyAddress kEFFMusicPlayerBundleIDAddress = {
    kAudioDeviceCustomPropertyMusicPlayerBundleID,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster
};

static const AudioObjectPropertyAddress kEFFAudibleStateAddress = {
    kAudioDeviceCustomPropertyDeviceAudibleState,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster
};

static const AudioObjectPropertyAddress kEFFRunningSomewhereOtherThanEFFAppAddress = {
    kAudioDeviceCustomPropertyDeviceIsRunningSomewhereOtherThanEFFApp,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster
};

static const AudioObjectPropertyAddress kEFFAppVolumesAddress = {
    kAudioDeviceCustomPropertyAppVolumes,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster
};

static const AudioObjectPropertyAddress kEFFEnabledOutputControlsAddress = {
    kAudioDeviceCustomPropertyEnabledOutputControls,
    kAudioObjectPropertyScopeOutput,
    kAudioObjectPropertyElementMaster
};


#pragma mark XPC Return Codes

enum {
    kEFFXPC_Success,
    kEFFXPC_MessageFailure,
    kEFFXPC_Timeout,
    kEFFXPC_EFFAppStateError,
    kEFFXPC_HardwareError,
    kEFFXPC_ReturningEarlyError,
    kEFFXPC_InternalError
};


#pragma mark Exceptions

#if defined(__cplusplus)

class EFF_InvalidClientException : public std::runtime_error {
public:
    EFF_InvalidClientException() : std::runtime_error("InvalidClient") { }
};

class EFF_InvalidClientPIDException : public std::runtime_error {
public:
    EFF_InvalidClientPIDException() : std::runtime_error("InvalidClientPID") { }
};

class EFF_InvalidClientRelativeVolumeException : public std::runtime_error {
public:
    EFF_InvalidClientRelativeVolumeException() : std::runtime_error("InvalidClientRelativeVolume") { }
};

class EFF_InvalidClientPanPositionException : public std::runtime_error {
public:
    EFF_InvalidClientPanPositionException() : std::runtime_error("InvalidClientPanPosition") { }
};

class EFF_DeviceNotSetException : public std::runtime_error {
public:
    EFF_DeviceNotSetException() : std::runtime_error("DeviceNotSet") { }
};

#endif

// Assume we've failed to start the output device if it isn't running IO after this timeout expires.
//
// Currently set to 30s because some devices, e.g. AirPlay, can legitimately take that long to start.
//
// TODO: Should we have a timeout at all? Is there a notification we can subscribe to that will tell us whether the
//       device is still making progress? Should we regularly poll mOutputDevice.IsAlive() while we're waiting to
//       check it's still responsive?
static const UInt64 kStartIOTimeoutNsec = 30 * NSEC_PER_SEC;


#endif /* EFF_Types_h */
