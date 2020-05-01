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

    var diaphragm = Diaphragm()

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Initialize audio related stuff
        diaphragm.setEFFAsOutputDevice()

        // Create the SwiftUI view that provides the window contents.
        let contentView = ContentView().environmentObject(diaphragm)

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
        // Undo all audio-related operations
        diaphragm.unsetEFFAsOutputDevice()
    }
}


