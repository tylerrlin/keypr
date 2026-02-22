//
//  RootView.swift
//  jumbohack_2026
//
//  Created by Elizabeth Ngai on 2/21/26.
//

import SwiftUI

struct RootView: View {
    @State private var selected: AppTab = .status

    var body: some View {
        ZStack(alignment: .bottom) {

            // Main screen content
            Group {
                switch selected {
                case .status:
                    NavigationStack { Home() }   // your existing Home view
                case .history:
                    NavigationStack { HistoryView() }
                case .devices:
                    NavigationStack { DevicesView() }
                }
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)

            // Bottom navbar
            CustomTabBar(selected: $selected)
        }
        .ignoresSafeArea(edges: .bottom)
    }
}
