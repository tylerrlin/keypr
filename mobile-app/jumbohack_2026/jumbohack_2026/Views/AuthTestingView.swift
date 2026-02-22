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
    @State private var lastDecision: String = "None"
    @State private var showApproved: Bool = false
    @State private var showDeclined: Bool = false

    var body: some View {
        ZStack {
            AppColors.blackBackground.ignoresSafeArea()

            // PRIORITY ORDER MATTERS
            if showApproved {

                AuthApprovedView(
                    onDisconnect: {
                        showApproved = false
                    }
                )

            } else if showDeclined {

                AuthDeclinedView(
                    onDismiss: {
                        showDeclined = false
                    }
                )

            } else {

                AuthNotifView(
                    appName: "keypr",

                    onAccept: {
                        lastDecision = "Accepted"
                        showApproved = true
                    },

                    onDecline: {
                        lastDecision = "Declined"
                        showDeclined = true
                    }
                )

            }
        }
    }
}
