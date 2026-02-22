//
//  BTTestView.swift
//  jumbohack_2026
//
//  Created by John Cha on 2/22/26.
//



import SwiftUI

struct BTTestView: View {
    @EnvironmentObject var appState: AppState
    
    @State private var isConnected: Bool = false
    @StateObject private var bleManager = SimpleBLEManager()

    // NEW: auth request simulation state
    @StateObject private var authManager = AuthRequestManager()
    @State private var lastDecision: String = "None"
    
    var body: some View {
        GeometryReader { geo in
            ZStack {
                AppColors.blackBackground
                    .ignoresSafeArea()
                
                VStack(spacing: 0) {
                    Button("Hello World") {
                        print("Fuck you stupid hoero")
                    }
                    
                    Button("Test Notification") {
                        bleManager.triggerNotification()
                    }
                    
                    
                }
                .sheet(isPresented: $appState.showAuthView){ Text("NOTIFICATION CLICKED!!")
                }
            }
            .frame(height: 800)
        }
    }
}

struct BTTestView_Previews: PreviewProvider {
    static var previews: some View {
        BTTestView()
            .previewDevice("iPhone 14 Pro")
            .preferredColorScheme(.dark)
    }
}
