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
                    
                        
                    Text("TESTING VIEW")
                        .font(.custom("KodeMono-Regular", size: 36))
                        .fontWeight(.bold)
                        .foregroundColor(AppColors.goldMainText)
                        .padding(.top, 25)
                    
                    Rectangle()
                        .fill(AppColors.blueAccent)
                        .frame(height: 2)
                        .padding(.top, 10)
                        .padding(.horizontal, 25)
                    
                    
                    Button("Test Notification") {
                        bleManager.triggerNotification()
                    }
                    
                    
                }
                .sheet(isPresented: $appState.showAuthView) {
                    Text("This **should** be working??")
                }
            }
            .frame(height: .infinity)
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
