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
    
    // Replace with your device's advertised Service UUID
    private let targetServiceUUID = CBUUID(string: "b1eb0868-cf7c-44a5-a917-33a90e31fda9`")
    
    override init() {
        super.init()
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }
    
    func startScan() {
        guard centralManager.state == .poweredOn else {
            print("Bluetooth not ready")
            return
        }
        
        isDeviceFound = false
        
        centralManager.scanForPeripherals(
            withServices: [targetServiceUUID],
            options: nil
        )
        
        print("Scanning started...")
    }
    
    // Required delegate
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        if central.state == .poweredOn {
            print("Bluetooth is ON")
        } else {
            print("Bluetooth not available")
        }
    }
    
    // Device discovered
    func centralManager(_ central: CBCentralManager,
                        didDiscover peripheral: CBPeripheral,
                        advertisementData: [String : Any],
                        rssi RSSI: NSNumber) {
        
        print("Device found!")
        
        DispatchQueue.main.async {
            self.isDeviceFound = true
        }
        
        centralManager.stopScan()
    }
}
