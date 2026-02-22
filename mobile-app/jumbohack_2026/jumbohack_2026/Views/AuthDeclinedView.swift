//
//  AuthDeclinedView.swift
//  jumbohack_2026
//
//  Created by Elizabeth Ngai on 2/22/26.
//

import SwiftUI

struct AuthDeclinedView: View {

    // Hardcoded for now, change to read from backend
    let deviceName: String = "keypr"
    let connectionStatus: String = "not connected"
    let authStatus: String = "declined"
    let onDismiss: () -> Void

    // Optional: call this when user taps disconnect
    var onDisconnect: (() -> Void)? = nil

    var body: some View {
        GeometryReader { geo in
            VStack(spacing: 0) {

                // Approve lock
                Image("icon_lockClosed_white")
                    .resizable()
                    .scaledToFit()
                    .frame(height: geo.size.height * 0.22)
                    .padding(.top, 60)

                // DECLINED
                Text("DECLINED")
                    .font(.custom("KodeMono-Regular_Bold", size: 40))
                    .foregroundStyle(AppColors.red)
                    .padding(.top, 10)
                    .padding(.bottom, 30)

                // Current status block
                VStack(alignment: .leading, spacing: 12) {

                    Text("current status")
                        .font(.custom("KodeMono-Regular_Medium", size: 28))
                        .foregroundStyle(AppColors.goldMainText)

                    Rectangle()
                        .fill(Color.white.opacity(0.6))
                        .frame(height: 1)

                    statusRow(
                        left: deviceName,
                        right: connectionStatus,
                        rightColor: AppColors.greyAccent
                    )

                    statusRow(
                        left: "authentication",
                        right: authStatus,
                        rightColor: AppColors.red
                    )
                }
                .padding(.horizontal, 28)

                Spacer()

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

