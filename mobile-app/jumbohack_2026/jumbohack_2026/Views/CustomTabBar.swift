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
        VStack(spacing: 10) {

            // blue divider line above the tab bar
            Rectangle()
                .fill(AppColors.blueAccent)
                .frame(height: 2)
                .padding(.horizontal, 25)

            HStack (spacing: 25) {
                
                tabButton(
                    tab: .status,
                    label: "status",
                    imageName: "icon_status",
                    iconSize: 60
                )

                Spacer()

                tabButton(
                    tab: .history,
                    label: "history",
                    imageName: "icon_history",
                    iconSize: 55
                )

                Spacer()

                tabButton(
                    tab: .devices,
                    label: "devices",
                    imageName: "icon_devices",
                    iconSize: 80
                )
            }
            
            .padding(.horizontal, 35)
            .padding(.bottom, 8)
        }
        .frame(maxWidth: .infinity)
        .background(AppColors.blackBackground)
    }

    private func tabButton(
        tab: AppTab,
        label: String,
        imageName: String,
        iconSize: CGFloat
    ) -> some View {

        Button {
            selected = tab
        } label: {

            VStack(spacing: 6) {

                // Fixed icon container (KEY FIX)
                ZStack {
                    Image(imageName)
                        .renderingMode(.original)
                        .resizable()
                        .scaledToFit()
                        .frame(width: iconSize, height: iconSize)
                }
                .frame(height: 64)

                Text(label)
                    .font(.custom("KodeMono-Regular", size: 15))
                    .foregroundStyle(
                        selected == tab
                        ? AppColors.blueAccent
                        : Color.white.opacity(0.85)
                    )
            }
            .frame(maxWidth: .infinity)
        }
        .buttonStyle(.plain)
    }
}
