//
//  Untitled.swift
//  jumbohack_2026
//
//  Created by John Cha on 2/21/26.
//

// This is a proof of concept thing
// Currently, this provides a class such that it searches for Jack Burton's phone

//TODO: match up the notification logic to views, send out a confirm/neglect
//TODO: how to discriminate between the different types of signals from the UUID
//TODO: update cooldown logic; right now it's just 10 seconds

import Foundation
import CoreBluetooth
import Combine
import UserNotifications


class SimpleBLEManager: NSObject, ObservableObject, CBCentralManagerDelegate {
    
    @Published var isDeviceFound = false
    @Published var discoveredDeviceNames: [String] = []
    
    private var centralManager: CBCentralManager!
    private var isScanning = false
    
//    Add a cool down to allow for continuous prompting
    private let scanCooldown: TimeInterval = 20 // seconds
    
    override init() {
          super.init()
          // This enables background restoration
          centralManager = CBCentralManager(
              delegate: self,
              queue: nil,
              options: [CBCentralManagerOptionRestoreIdentifierKey: "SimpleBLEManagerRestoreKey"]
          )
      }
    
    // targetService UUID is the hardcoded UUID in a git-ignored file
    private let targetServiceUUID = CBUUID(string: hardcoded_UUID)
    

    
    func startScan() {
        // 2️⃣ Guard to prevent overlapping scans
        guard centralManager.state == .poweredOn, !isScanning else { return }
        print("Starting scan...")
        let serviceUUIDs = [targetServiceUUID]
        centralManager.scanForPeripherals(
            withServices: serviceUUIDs,
            options: [CBCentralManagerScanOptionAllowDuplicatesKey: true]
        )
        isScanning = true
    }
    
    func stopScan() {
        centralManager.stopScan()
        isScanning = false
        print("Scan stopped for 10 seconds")
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
        stopScan()
        // you can trigger app logic here
        
        //  trigger notification here
        triggerNotification()
        
                
        // 4️⃣ Cool-down: reset isDeviceFound and restart scan after 10s
        DispatchQueue.main.asyncAfter(deadline: .now() + scanCooldown) { [weak self] in
            self?.isDeviceFound = false
            self?.startScan()
        }
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
        content.title = "Are you trying to log in?"
        content.body = "Tap here to confirm or deny"
        content.sound = .default
        
        let request = UNNotificationRequest(
            identifier: UUID().uuidString,
            content: content,
            trigger: nil // immediate
        )
        
        UNUserNotificationCenter.current().add(request)
    }
}
