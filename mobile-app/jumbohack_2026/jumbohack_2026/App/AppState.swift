//
//  AppState.swift
//  jumbohack_2026
//
//  Created by John Cha on 2/22/26.
//

//Used for the Auth notification logic
import SwiftUI
import Combine

class AppState: ObservableObject {
    static let shared = AppState()
    @Published var showAuthView = false
    @Published var showPairingView = false
}
