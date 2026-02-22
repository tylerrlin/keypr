//
//  jumnohack_2026App.swift
//  jumnohack_2026
//
//  Created by John Cha on 2/21/26.
//

import SwiftUI
import UserNotifications
import UIKit

@main
struct jumnohack_2026App: App {
    
    let notificationDelegate = NotificationDelegate()
       
    init() {
        UNUserNotificationCenter.current().delegate = notificationDelegate
        requestNotificationPermission()
        print("FONTSFONTSFONTS")
        printFonts()
        print("All fonts should have been printed by now")
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
    
    func printFonts() {
        for family in UIFont.familyNames.sorted() {
            let names = UIFont.fontNames(forFamilyName: family)
            print("Family: \(family) Font names: \(names)")
        }
    }
    
}

class NotificationDelegate: NSObject, UNUserNotificationCenterDelegate {
    
    func userNotificationCenter(_ center: UNUserNotificationCenter,
                                willPresent notification: UNNotification,
                                withCompletionHandler completionHandler:
                                @escaping (UNNotificationPresentationOptions) -> Void) {
        
        completionHandler([.banner, .sound])
        
    }
}
