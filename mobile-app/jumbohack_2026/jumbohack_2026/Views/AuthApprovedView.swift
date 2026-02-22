//
//  AuthApprovedView.swift
//  jumbohack_2026
//
//  Created by Elizabeth Ngai on 2/22/26.
//

import SwiftUI

struct AuthApprovedView: View {

    // Hardcoded for now, change to read from backend
    let deviceName: String = "keypr"
    let connectionStatus: String = "connected"
    let authStatus: String = "approved"

    // Optional: call this when user taps disconnect
    var onDisconnect: (() -> Void)? = nil

    var body: some View {
        GeometryReader { geo in
            VStack(spacing: 0) {

                Spacer(minLength: geo.size.height * 0.05)

                // Approve lock
                Image("icon_lockOpen")
                    .resizable()
                    .scaledToFit()
                    .frame(height: geo.size.height * 0.22)

                // APPROVED!
                Text("APPROVED!")
                    .font(.custom("KodeMono-Regular_Bold", size: 40))
                    .foregroundStyle(AppColors.blueAccent)
                    .padding(.top, 10)

                Spacer(minLength: geo.size.height * 0.05)

                // Current status block
                VStack(alignment: .leading, spacing: 12) {

                    Text("current status")
                        .font(.custom("KodeMono-Regular_Medium", size: 28))
                        .foregroundStyle(AppColors.goldMainText)

                    Rectangle()
                        .fill(Color.white.opacity(0.6))
                        .frame(height: 1)

                    statusRow(left: deviceName, right: connectionStatus, rightColor: AppColors.blueAccent)
                    statusRow(left: "authentication", right: authStatus, rightColor: AppColors.blueAccent)
                }
                .padding(.horizontal, 28)

                Spacer()

                // Disconnect button: "disconnect " in KodeMono, "keypr" in KyivTypeSans
                Button {
                    onDisconnect?()
                } label: {
                    (
                        Text("disconnect ")
                            .font(.custom("KodeMono-Regular_Medium", size: 22))
                        +
                        Text("keypr")
                            .font(.custom("KyivTypeSans-Medium-", size: 22))
                    )
                    .foregroundStyle(AppColors.blueAccent)
                    .frame(maxWidth: .infinity)
                    .frame(height: 56)
                    .background(Color.white.opacity(0.90))
                }
                .buttonStyle(.plain)
                .padding(.horizontal, 28)

                Spacer(minLength: geo.size.height * 0.06)
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)
        }
    }

    private func statusRow(left: String, right: String, rightColor: Color) -> some View {
        HStack {
            Text(left)
                .font(.custom("KodeMono-Regular", size: 20))
                .foregroundStyle(Color.white.opacity(0.85))

            Spacer()

            Text(right)
                .font(.custom("KodeMono-Regular_Medium", size: 20))
                .foregroundStyle(rightColor)
        }
    }
}
