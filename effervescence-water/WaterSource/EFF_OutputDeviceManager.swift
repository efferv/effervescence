//
//  EFF_OutputDeviceManager.swift
//  effervescence-water
//
//  Created by Nerrons on 4/4/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

import Foundation
import CoreAudio


class OutputDeviceManager: NSObject {
    var audioDeviceManager: EFFAudioDeviceManager!
    var audioSystem: EFF_HALAudioSystemObject!
    
    var outputDevice: AudioObjectID
    
    init(_ audioDeviceManager: EFFAudioDeviceManager) {
        self.audioDeviceManager = audioDeviceManager
        self.audioSystem = EFF_HALAudioSystemObject()
        outputDevice = audioSystem.getFirstAudioDevice()
    }
    
    func start() -> Void {
        
        //var effMainDevice = audioDeviceManager.effSoundPtr
        //audioSystem.setOSDefaultMainDeviceWithID(effMainDevice)
    }
}
