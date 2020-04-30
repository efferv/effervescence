//
//  Diaphragm.swift
//  effervescence-water
//
//  Created by Nerrons on 30/4/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

// Diaphragm is a big, flat layer which provides a universal Swift API for everything
// that involves ObjC and C++ audio-related code

import Foundation

class Diaphragm {
    var audioDeviceManager: EFFAudioDeviceManager!
    var hal: EFF_HALAudioSystemObject
    
    init() {
        audioDeviceManager = EFFAudioDeviceManager()
        hal = EFF_HALAudioSystemObject()
    }

    func setEFFAsOutputDevice() {
        let oldOutputDevice = hal.getOSDefaultMainDevice()
        audioDeviceManager.setEFFSoundDeviceAsOSDefault()
        audioDeviceManager.setOutputDeviceWithID(oldOutputDevice, revertOnFailure: false)
    }

    func unsetEFFAsOutputDevice() {
        audioDeviceManager.unsetEFFSoundDeviceAsOSDefault()
    }
}
