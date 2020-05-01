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
        List(diaphragm.getOutputDeviceNames(), id: \.self) { name in
            Text(name)
        }
            .frame(maxWidth: .infinity, maxHeight: .infinity)
    }
}


struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
