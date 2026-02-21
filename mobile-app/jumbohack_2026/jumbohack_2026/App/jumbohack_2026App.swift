//
//  jumnohack_2026App.swift
//  jumnohack_2026
//
//  Created by John Cha on 2/21/26.
//

import SwiftUI
import UserNotifications

@main
struct jumnohack_2026App: App {
    init() {
        requestNotificationPermission()
    }
    
    var body: some Scene {
        WindowGroup {
            ContentView()
        }
    }
    
    func requestNotificationPermission() {
        UNUserNotificationCenter.current().requestAuthorization(options: [.alert, .sound]) { granted, error in
            if granted {
                print("Notifications allowed")
            } else {
                print("Notifications denied")
            }
        }
    }
    
}
