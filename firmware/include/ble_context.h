#pragma once
#include <stdbool.h>
#include <stdint.h> 
#include "ble_keypr_structures.h"

// Initialize BLE peripheral
// Advertises as "KEYPR" with a known service UUID
void ble_context_init(void);

// Returns true if a phone is currently BLE-connected
// This is the core context check - called before get_assertion
bool ble_context_is_phone_connected(void);

// Optional: RSSI-based proximity (phone must be within ~1m)
// Returns true if connected AND RSSI > threshold
bool ble_context_is_phone_nearby(int8_t rssi_threshold_dbm);

// Register a callback for connect/disconnect events
// ! Type changed from: typedef void (*ble_context_callback_t)(bool connected);
typedef void (*ble_context_callback_t)(bool connected, uint8_t *macaddr);
void ble_context_set_callback(ble_context_callback_t cb);

// BLE Service/Characteristic UUIDs
// These are what your phone app connects to
#define BLE_SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
