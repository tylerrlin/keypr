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

                    
                Text("keypr")
                    .font(.custom("KyivTypeSans-Heavy3", size: 48))
                    .fontWeight(.bold)
                    .foregroundColor(AppColors.goldMainText) // logo/text color in gold rectangle
                
                Divider()
                    .frame(height: 2)
                    .overlay(AppColors.blueAccent)// Inserts a horizontal line
                    .padding(.horizontal, 23)
                    .padding(.top, )
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
//        ContentView.printFonts()
    }
}
