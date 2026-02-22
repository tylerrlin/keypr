//
//  CustomTabBar.swift
//  jumbohack_2026
//
//  Created by Elizabeth Ngai on 2/21/26.
//

import SwiftUI

struct CustomTabBar: View {
    @Binding var selected: AppTab

    var body: some View {
        HStack {
            // status
            Button {
                selected = .status
            } label: {
                VStack {
                    Image("icon_status")
                    Text("status")
                }
            }

            Spacer()

            Button {
                selected = .history
            } label: {
                VStack {
                    Image("icon_history")
                    Text("history")
                }
            }

            Spacer()

            Button {
                selected = .devices
            } label: {
                VStack {
                    Image("icon_devices")
                    Text("devices")
                }
            }
        }
        .padding(.horizontal, 28)
        .padding(.top, 10)
        .padding(.bottom, 18)
    }
}
