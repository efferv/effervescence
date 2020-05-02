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
//
// The rule of thumb is that the views shouldn't directly call C-Layer stuff.
// This means the diaphragm is susceptible to get big quickly
//
// Notes on audio processing:
// I soon need to decide on a way to process audio. I see two options:
//
// 1) Record whatever's captured by the driver.
// + (Probably) less demanding to code with more margins for error
// + More creative effects to allow a flattened, evenly-distributed experience
// - (Very) Bad I/O performance
//
// 2) Process real-time with AU
// + Smooth performance, since most heavy operations are handled by OS
// - Requires more rigorous coding == buggy
// - Need to find ways to store past audio fragments if want to flatten
//
// So I'm going to take the AU route. Flattening can be a later problem to
// solve. Pretty sure it can incorporate audio files quite smoothly


import Foundation

class Diaphragm: ObservableObject {

    var audioDeviceManager: EFFAudioDeviceManager
    var hal: EFF_HALAudioSystemObject

    @Published var outputDeviceIDs: [AudioObjectID]
    @Published var outputDeviceNames: [String]
    @Published var oldOutputDeviceID: AudioObjectID
    
    init() {
        audioDeviceManager = EFFAudioDeviceManager()
        hal = EFF_HALAudioSystemObject()
        outputDeviceIDs = []
        outputDeviceNames = []
        oldOutputDeviceID = hal.getOSDefaultMainDeviceID()

        refreshOutputDevices()
        setEFFAsDefaultDevice()
    }

    func setEFFAsDefaultDevice() {
        let effDeviceID = audioDeviceManager.getEFFMainDeviceID()

        if oldOutputDeviceID != effDeviceID {
            audioDeviceManager.setEFFSoundDeviceAsOSDefault()
            audioDeviceManager.setOutputDeviceWithID(oldOutputDeviceID, revertOnFailure: false)
        } else {
            // We should try to infer what's the best output device here, OR prompt the user to select.
        }
    }

    func unsetEFFAsDefaultDevice() {
        audioDeviceManager.unsetEFFSoundDeviceAsOSDefault()
    }

    func refreshOutputDeviceIDs() {
        // get all available audio devices
        let numAudioDevices = hal.getNumberAudioDevices()
        let audioDeviceIDsPtr = UnsafeMutablePointer<AudioObjectID>.allocate(capacity: numAudioDevices)
        defer { free(audioDeviceIDsPtr) }
        hal.getAudioDevices(withNumber: numAudioDevices, outAudioDevices: audioDeviceIDsPtr)
        let audioDeviceIDs = [AudioObjectID](UnsafeBufferPointer(start: audioDeviceIDsPtr,
                                                                 count: numAudioDevices))

        // filter out the output devices
        outputDeviceIDs = audioDeviceIDs.filter { audioDeviceManager.canBeOutputDevice($0) }
    }
    
    func translateIDToName(fromArray: [AudioObjectID]) -> [String] {
        var names = [String]()
        for id in fromArray {
            names.append(audioDeviceManager.getNameFromDeviceID(id))
        }
        return names
    }

    func refreshOutputDevices() {
        refreshOutputDeviceIDs()
        outputDeviceNames = translateIDToName(fromArray: outputDeviceIDs)
    }
    
    func setOutputDevice(id: AudioObjectID) {
        audioDeviceManager.setOutputDeviceWithID(id, revertOnFailure: true)
    }
}
