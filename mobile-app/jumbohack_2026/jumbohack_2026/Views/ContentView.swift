//
//  ContentView.swift
//  jumnohack_2026
//
//  Created by John Cha on 2/21/26.
//

import SwiftUI

struct ContentView: View {
    // Placeholder boolean for BLE connection
    @State private var isConnected: Bool = false
    @StateObject private var bleManager = SimpleBLEManager()

    var body: some View {
        GeometryReader { geo in
            AppColors.blackBackground
                .ignoresSafeArea()
            VStack(spacing: 0) {
                // Top 15% gold section
                ZStack{
                    AppColors.goldMainText
                        .ignoresSafeArea()
                    Text("keypr")
                        .font(.largeTitle)
                        .fontWeight(.bold)
                        .foregroundColor(.black) // logo/text color in gold rectangle
                }
                .frame(height: geo.size.height * 0.15) // 15% of screen height

                // Bottom 85% black section
                ZStack(alignment: .topLeading) {
                    AppColors.blackBackground
                        .ignoresSafeArea()
                    
                }
                .frame(height: geo.size.height * 0.85)
            }
        }
    }

    // Computed property for the descriptive message
    private var connectionMessage: String {
        if isConnected {
            return "Your phone is connected to the hardware device via BLE."
        } else {
            return "No hardware device detected. Please ensure your device is on and in range."
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
            .previewDevice("iPhone 14 Pro") // optional: pick device
            .preferredColorScheme(.dark)    // optional: dark/light mode
    }
}
