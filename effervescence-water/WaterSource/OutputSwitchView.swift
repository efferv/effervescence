//
//  OutputSwitchView.swift
//  effervescence-water
//
//  Created by Nerrons on 1/5/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

import SwiftUI
import Combine

struct OutputSwitchView: View {
    @EnvironmentObject var diaphragm: Diaphragm

    @State var picked: Int

    var body: some View {
        Form {
            Picker(selection: $picked, label: Text("Select an output device")) {
                ForEach(0 ..< diaphragm.outputDeviceNames.count) { i in
                    Text(self.diaphragm.outputDeviceNames[i]).tag(i)
                }
            }.onReceive(Just(self.picked)) { p in
                print(self.diaphragm.outputDeviceNames[self.picked])
                self.diaphragm.setOutputDevice(id: self.diaphragm.outputDeviceIDs[self.picked])
            }
        }
    }
}

struct OutputSwitchView_Previews: PreviewProvider {
    static var previews: some View {
        OutputSwitchView(picked: 0).environmentObject(Diaphragm())
    }
}
