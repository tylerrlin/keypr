//
//  jumnohack_2026App.swift
//  jumnohack_2026
//
//  Created by John Cha on 2/21/26.
//

import SwiftUI
import UserNotifications
#if canImport(UIKit)
import UIKit
#endif


//CHANGE THE WINDOW GROUP IN THE BODY TO TEST FEATURES
@main
struct jumnohack_2026App: App {
    
    let notificationDelegate = NotificationDelegate()
       
    init() {
        UNUserNotificationCenter.current().delegate = notificationDelegate
        requestNotificationPermission()
    }

    var body: some Scene {
        WindowGroup {
//            ContentView()
//            AuthTestingView()
            BTTestView()
                .environmentObject(AppState.shared)
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

class NotificationDelegate: NSObject, UNUserNotificationCenterDelegate {
    
    func userNotificationCenter(_ center: UNUserNotificationCenter,
                                willPresent notification: UNNotification,
                                withCompletionHandler completionHandler:
                                @escaping (UNNotificationPresentationOptions) -> Void) {
        DispatchQueue.main.async {
                AppState.shared.showAuthView = true
        }
        completionHandler([.banner, .sound])
        
    }
}
