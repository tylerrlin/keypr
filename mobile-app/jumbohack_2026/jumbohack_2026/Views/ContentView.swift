//
//  ContentView.swift
//  jumnohack_2026
//
//  Created by John Cha on 2/21/26.
//

import SwiftUI

struct ContentView: View {
    @EnvironmentObject var appState: AppState
    
    @State private var isConnected: Bool = false
    @StateObject private var bleManager = SimpleBLEManager()
    
    @State private var selectedTab: AppTab = .status

    var body: some View {
        GeometryReader { geo in
            ZStack {
                AppColors.blackBackground.ignoresSafeArea()

                VStack(spacing: 10) {

                    // ===== HEADER (fixed height) =====
                    VStack(spacing: 10) {
                        Text("keypr")
                            .font(.custom("KyivTypeSans-Medium-", size: 36))
                            .fontWeight(.bold)
                            .foregroundColor(AppColors.goldMainText)
                            .padding(.top, 4)

                        Rectangle()
                            .fill(AppColors.blueAccent)
                            .frame(height: 2)
                            .padding(.horizontal, 25)
                    }
                    .frame(height: geo.size.height * 0.10)  // keeps header visible
//                    .padding(.top, -30)

                    // ===== CONTENT (fills remaining space) =====
                    ZStack {
                        switch selectedTab {
                        case .status:
                            HomeView()
                        case .history:
                            HistoryView()
                        case .devices:
                            DevicesView(
                                keyprs: ["keypr"],
                                computers: ["E's MacBook Pro"]
                            )
                        }
                    }
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
                    .clipped()

                    // ===== TAB BAR (fixed height) =====
                    CustomTabBar(selected: $selectedTab)
                        .frame(height: geo.size.height * 0.13)
                }
            }
        }
    }
}
