//
//  Untitled.swift
//  jumbohack_2026
//
//  Created by John Cha on 2/21/26.
//

// This is a proof of concept thing
// Currently, this provides a class such that it searches for Jack Burton's phone

import Foundation
import CoreBluetooth
import Combine
import UserNotifications

class SimpleBLEManager: NSObject, ObservableObject, CBCentralManagerDelegate {
    
    @Published var isDeviceFound = false
    
    private var centralManager: CBCentralManager!
    
    override init() {
          super.init()
          // This enables background restoration
          centralManager = CBCentralManager(
              delegate: self,
              queue: nil,
              options: [CBCentralManagerOptionRestoreIdentifierKey: "SimpleBLEManagerRestoreKey"]
          )
      }
    
    // Replace with your device's advertised Service UUID
    private let targetServiceUUID = CBUUID(string: "b1eb0868-cf7c-44a5-a917-33a90e31fda9")
    
//    override init() {
//        super.init()
//        centralManager = CBCentralManager(delegate: self, queue: nil)
//    }
    
    func startScan() {
            let serviceUUIDs = [targetServiceUUID]
            centralManager.scanForPeripherals(withServices: serviceUUIDs, options: [CBCentralManagerScanOptionAllowDuplicatesKey: true])
        }
        
        // CBCentralManagerDelegate
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        if central.state == .poweredOn {
            startScan()
        }
    }
        
    func centralManager(_ central: CBCentralManager,
                        didDiscover peripheral: CBPeripheral,
                        advertisementData: [String : Any],
                        rssi RSSI: NSNumber) {
        guard !isDeviceFound else { return }
        isDeviceFound = true
        central.stopScan()
        // you can trigger app logic here
        
//        trigger notification here
        triggerNotification()
    }
    
    // Restoration delegate
    func centralManager(_ central: CBCentralManager,
                        willRestoreState dict: [String : Any]) {
        print("Restored BLE state: \(dict)")
        // iOS wakes your app in background and gives you peripherals discovered
    }
    
//    For when Jack's UUID is found
    func triggerNotification() {
        let content = UNMutableNotificationContent()
        content.title = "Passkey Device Found"
        content.body = "Your BLE device is nearby."
        content.sound = .default
        
        let request = UNNotificationRequest(
            identifier: UUID().uuidString,
            content: content,
            trigger: nil // immediate
        )
        
        UNUserNotificationCenter.current().add(request)
    }
}
