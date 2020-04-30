//
//  AppDelegate.swift
//  effervescence-app
//
//  Created by Nerrons on 29/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//
//  effervescence-water handles sound processing as well as UI.
//  effervescence-carbon's only job is to collect waves through the device.

import Cocoa
import SwiftUI

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {

    var window: NSWindow!

    var audioDeviceManager: EFFAudioDeviceManager!

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // I want to make a Swift class called Diaphragm which will provide
        // a universal Swift API to all SwiftUI classes
        audioDeviceManager = EFFAudioDeviceManager()
        let hal = EFF_HALAudioSystemObject()
        let oldOutputDevice = hal.getOSDefaultMainDevice()
        audioDeviceManager.setEFFSoundDeviceAsOSDefault()
        audioDeviceManager.setOutputDeviceWithID(oldOutputDevice, revertOnFailure: false)

        // Create the SwiftUI view that provides the window contents.
        let contentView = ContentView()

        // Create the window and set the content view. 
        window = NSWindow(
            contentRect: NSRect(x: 0, y: 0, width: 480, height: 300),
            styleMask: [.titled, .closable, .miniaturizable, .resizable, .fullSizeContentView],
            backing: .buffered, defer: false)
        window.center()
        window.setFrameAutosaveName("Main Window")
        window.contentView = NSHostingView(rootView: contentView)
        window.makeKeyAndOrderFront(nil)
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        audioDeviceManager.unsetEFFSoundDeviceAsOSDefault()
    }
}


