//
//  Diaphragm.swift
//  effervescence-water
//
//  Created by Nerrons on 30/4/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

// Diaphragm is a big, flat layer which provides a universal Swift API for everything
// that involves ObjC and C++ audio-related code
//
// Also, maybe should move all c code to carbon and provide interface there
// instead of including many things in the bridging header
// No... carbon is just the driver. Hierarchy should look like this:
//
//
// * SwiftUI    --\
// * Diaphragm  ---- Water
// * C-Layer    --/
//
// * Driver     ---- Carbon

import Foundation

class Diaphragm: ObservableObject {
    
    @Published var updates = 0

    var audioDeviceManager: EFFAudioDeviceManager
    var hal: EFF_HALAudioSystemObject
    
    init() {
        audioDeviceManager = EFFAudioDeviceManager()
        hal = EFF_HALAudioSystemObject()
    }

    func setEFFAsOutputDevice() {
        print(getOutputDeviceNames())

        let oldOutputDeviceID = hal.getOSDefaultMainDeviceID()
        let effDeviceID = audioDeviceManager.getEFFMainDeviceID();
        
        if oldOutputDeviceID != effDeviceID {
            audioDeviceManager.setEFFSoundDeviceAsOSDefault()
            audioDeviceManager.setOutputDeviceWithID(oldOutputDeviceID, revertOnFailure: false)
        } else {
            // We should try to infer what's the best output device here, OR prompt the user to select.
        }
    }

    func unsetEFFAsOutputDevice() {
        audioDeviceManager.unsetEFFSoundDeviceAsOSDefault()
    }

    func getOutputDeviceIDs() -> [AudioObjectID] {
        // get all available audio devices
        let numAudioDevices = hal.getNumberAudioDevices()
        let audioDeviceIDListPtr = UnsafeMutablePointer<AudioObjectID>.allocate(capacity: numAudioDevices)
        defer { free(audioDeviceIDListPtr) }
        hal.getAudioDevices(withNumber: numAudioDevices, outAudioDevices: audioDeviceIDListPtr)
        let audioDeviceIDList = [AudioObjectID](UnsafeBufferPointer(start: audioDeviceIDListPtr,
                                                                    count: numAudioDevices))

        // filter out the output devices
        let outputDeviceIDList = audioDeviceIDList.filter { audioDeviceManager.canBeOutputDevice($0) }
        return outputDeviceIDList
    }
    
    func translateIDToName(fromArray: [AudioObjectID]) -> [String] {
        var names = [String]()
        for id in fromArray {
            names.append(audioDeviceManager.getNameFromDeviceID(id))
        }
        return names
    }

    func getOutputDeviceNames() -> [String] {
        return translateIDToName(fromArray: getOutputDeviceIDs())
    }
}
