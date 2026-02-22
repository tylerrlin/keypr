//
//  AuthRequestManager.swift
//  jumbohack_2026
//
//  Created by Elizabeth Ngai on 2/21/26.
//

import SwiftUI

@MainActor
final class AuthRequestManager: ObservableObject {

    struct Request: Identifiable {
        let id = UUID()
        let appName: String
    }

    @Published var activeRequest: Request? = nil

    func showRequest(appName: String) {
        activeRequest = Request(appName: appName)
    }

    func clearRequest() {
        activeRequest = nil
    }
}
