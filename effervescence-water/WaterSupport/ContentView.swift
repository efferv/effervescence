//
//  ContentView.swift
//  effervescence-app
//
//  Created by Nerrons on 29/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

import SwiftUI

struct ContentView: View {
    @EnvironmentObject var diaphragm: Diaphragm

    var body: some View {
        // Compute the initial value of the picked state so it matches the pre-decided EFF output device
        OutputSwitchView(picked: diaphragm.outputDeviceIDs.firstIndex(of: diaphragm.oldOutputDeviceID) ?? 0)
            .frame(maxWidth: .infinity, maxHeight: .infinity)
    }
}


struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
