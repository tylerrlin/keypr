//
//  CustomTabBar.swift
//  jumbohack_2026
//
//  Created by Elizabeth Ngai on 2/21/26.
//

import SwiftUI

struct CustomTabBar: View {
    @Binding var selected: AppTab

    // swap these for your AppColors when ready
    private let bg = Color(red: 0.10, green: 0.10, blue: 0.10)     // dark
    private let gold = Color(red: 0.90, green: 0.70, blue: 0.00)   // ~E5B300
    private let labelSelected = Color.white.opacity(0.9)
    private let labelUnselected = Color.white.opacity(0.55)
    private let dividerBlue = Color(red: 0.31, green: 0.53, blue: 0.64)

    var body: some View {
        VStack(spacing: 0) {
            Rectangle()
                .fill(dividerBlue)
                .frame(height: 2)

            HStack {
                ForEach(AppTab.allCases) { tab in
                    Button {
                        selected = tab
                    } label: {
                        VStack(spacing: 6) {
                            Image(tab.iconAssetName)
                                .renderingMode(.template)
                                .resizable()
                                .scaledToFit()
                                .frame(width: 34, height: 34)
                                .foregroundStyle(gold)

                            Text(tab.title)
                                .font(.system(size: 14, design: .monospaced))
                                .foregroundStyle(selected == tab ? labelSelected : labelUnselected)
                        }
                        .frame(maxWidth: .infinity)
                        .contentShape(Rectangle())
                    }
                    .buttonStyle(.plain)
                }
            }
            .padding(.top, 10)
            .padding(.bottom, 18)
            .padding(.horizontal, 24)
            .background(bg)
        }
    }
}
