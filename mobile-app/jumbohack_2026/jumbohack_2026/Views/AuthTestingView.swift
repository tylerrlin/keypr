//
//  AuthTestingView.swift
//  jumbohack_2026
//
//  Created by John Cha on 2/22/26.
//

//This is a file to more easily test the auth functionality and all of that via the simulator

// to activate, set this as the active view under jumbohack_2026App, under the App directory


import SwiftUI

struct AuthTestingView: View {
    // Your existing BLE-related state
    @State private var isConnected: Bool = false
    @StateObject private var bleManager = SimpleBLEManager()

    // NEW: auth request simulation state
    @StateObject private var authManager = AuthRequestManager()
    @State private var lastDecision: String = "None"
    
    func sim_req_button() -> some View {
        ZStack(alignment: .topLeading) {
            AppColors.blackBackground.ignoresSafeArea()

            VStack(alignment: .leading, spacing: 12) {
                Text("THIS IS A TESTING SCREEN")
                Text(connectionMessage)
                    .foregroundStyle(.white.opacity(0.85))
                    .font(.system(size: 14, design: .monospaced))

                Button("Simulate Auth Request") {
                    authManager.showRequest(appName: "keypr")
                }
                .buttonStyle(.borderedProminent)

                Text("Last decision: \(lastDecision)")
                    .foregroundStyle(.white.opacity(0.85))
                    .font(.system(size: 14, design: .monospaced))
            }
            .padding(20)
        }
        
    }

    var body: some View {
        GeometryReader { geo in
            VStack(spacing: 0) {

                // Top 15% gold section
                ZStack {
                    AppColors.goldMainText.ignoresSafeArea()

                    VStack(spacing: 10) {
                        Text("keypr")
                            .font(.custom("KyivTypeSans-Heavy3", size: 48))
                            .fontWeight(.bold)
                            .foregroundColor(.black)

                        Divider()
                            .frame(height: 2)
                            .overlay(AppColors.blueAccent)
                            .padding(.horizontal, 23)
                    }
                }
                .frame(height: geo.size.height * 0.15)

                sim_req_button()
                .frame(height: geo.size.height * 0.85)
                
            }
        }
        .ignoresSafeArea()

        // ✅ Show AuthNotifView “on its own” when request appears
        .fullScreenCover(item: $authManager.activeRequest) { request in
            NavigationStack {
                AuthNotifView(
                    appName: request.appName,
                    onAccept: {
                        lastDecision = "Accepted"
                        // simulate: dismiss after a beat so you can see the next page if you add it
                        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                            authManager.clearRequest()
                        }
                    },
                    onDecline: {
                        lastDecision = "Declined"
                        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                            authManager.clearRequest()
                        }
                    }
                )
            }
        }

        // ✅ Optional: auto simulation after 2 seconds
        .onAppear {
            DispatchQueue.main.asyncAfter(deadline: .now() + 2) {
                authManager.showRequest(appName: "keypr")
            }
        }
    }

    // Computed property for the descriptive message
    private var connectionMessage: String {
        isConnected
        ? "Your phone is connected to the hardware device via BLE."
        : "No hardware device detected. Please ensure your device is on and in range."
    }
}


struct AuthTestingView_Previews: PreviewProvider {
    static var previews: some View {
        AuthTestingView()
            .previewDevice("iPhone 14 Pro")
            .preferredColorScheme(.dark)
    }
}

