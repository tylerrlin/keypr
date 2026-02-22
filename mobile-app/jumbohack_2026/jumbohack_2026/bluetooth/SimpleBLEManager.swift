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
    func debug() {
        print("DEBUG: literally a sanity check")
    }
    
        
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
        print("Scan stopped for \(scanCooldown) seconds")
    }
    


        
        // CBCentralManagerDelegate
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        if central.state == .poweredOn {
            startScan()
        }
    }
        
    func centralManager(_ central: CBCentralManager,
                        didDiscover peripheral: CBPeripheral,
                        advertisementData: [String: Any],
                        rssi RSSI: NSNumber) {
        guard !isDeviceFound else { return }
        print("A MESSAGE HAS BEEN FOUND!")
//        guard let data = advertisementData[CBAdvertisementDataManufacturerDataKey] as? Data,
//              !data.isEmpty else {
//            print("RETURNING HERE")
//            return }
        
        isDeviceFound = true
        stopScan()
        
        if let localName = advertisementData[CBAdvertisementDataLocalNameKey] as? String,
           let byte = UInt8(localName, radix: 16) {
            handleMessage(Data([byte]))
        }
        
//        handleMessage(data)
        
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

    
    func handleMessage(_ data: Data) {
        guard let messageType = DeviceMessage(rawValue: data[0]) else {
            print("Unknown message type: \(data[0])")
            return
        }
//        triggerNotification()
        switch messageType {
        case .empty:
            break
        case .pairingRequest:
            print("HAVE TO IMPLEMENT APPSTATE! AppState.shared.showPairingView = true ")
        case .pairingAccept, .pairingReject:
            print("Unexpected message on phone side: \(messageType)")
        case .authRequest:
            print("DEBUG: triggerNotification()")
        case .authDataChunk:
            break // TODO
        case .authGranted, .authDenied:
            print("Unexpected message on phone side: \(messageType)")
        }
    }
    
    func triggerNotification() {
        print("DEBUG: triggerNotification() called")
        let content = UNMutableNotificationContent()

        content.title = "Are you trying to log in?"
        content.body = "Tap here to confirm or deny"
        content.sound = .default

        let request = UNNotificationRequest(
            identifier: UUID().uuidString,
            content: content,
            trigger: nil
        )

        DispatchQueue.main.async {
            UNUserNotificationCenter.current().add(request) { error in
                if let error = error {
                    print("Notification error: \(error)")
                } else {
                    print("Notification scheduled successfully")
                }
            }
        }
        
    }
}

enum DeviceMessage: UInt8 {
    case empty          = 0x00
    
    // Pairing
    case pairingRequest = 0x10  // Device -> Phone
    case pairingAccept  = 0x20  // Phone -> Device
    case pairingReject  = 0x21  // Phone -> Device
    
    // Authentication
    case authRequest    = 0x30  // Device -> Phone (next byte = site name length, followed by chunked name)
    case authDataChunk  = 0x31  // Device -> Phone (delivers x bytes)
    case authGranted    = 0x40  // Phone -> Device
    case authDenied     = 0x41  // Phone -> Device
}
