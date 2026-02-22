#include "ble_context.h"
#include <NimBLEDevice.h>
#include <Arduino.h>

static bool phone_connected = false;
static ble_context_callback_t user_callback = NULL;
static NimBLEServer *pServer = nullptr;

#define BLE_DEVICE_NAME "FIDO2-AuthNodeJack"

class ConnectionCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) override {
        phone_connected = true;
        Serial.println("[BLE] Phone connected - auth ARMED");
        if (user_callback) user_callback(true);
        // Don't restart advertising - we only want one connection
    }

    void onDisconnect(NimBLEServer *pServer) override {
        phone_connected = false;
        Serial.println("[BLE] Phone disconnected - auth DISARMED");
        if (user_callback) user_callback(false);
        // Restart advertising so phone can reconnect
        NimBLEDevice::startAdvertising();
    }
};

void ble_context_init(void) {
    NimBLEDevice::init(BLE_DEVICE_NAME);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);  // max power for range

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ConnectionCallbacks());

    // Create the service and characteristic
    NimBLEService *pService = pServer->createService(BLE_SERVICE_UUID);
    NimBLECharacteristic *pChar = pService->createCharacteristic(
        BLE_CHAR_UUID,
        NIMBLE_PROPERTY::READ
    );
    pChar->setValue("FIDO2-AuthNode");
    pService->start();

    // Set up advertising
    NimBLEAdvertising *pAdv = NimBLEDevice::getAdvertising();
    pAdv->addServiceUUID(BLE_SERVICE_UUID);
    pAdv->setScanResponse(true);
    pAdv->setMinPreferred(0x06);
    NimBLEDevice::startAdvertising();

    Serial.println("[BLE] Advertising as 'FIDO2-AuthNode'");
}

bool ble_context_is_phone_connected(void) {
    return phone_connected;
}

bool ble_context_is_phone_nearby(int8_t rssi_threshold_dbm) {
    if (!phone_connected || pServer == nullptr) return false;
    if (pServer->getConnectedCount() == 0) return false;

    // Get RSSI of first connected client
    uint16_t conn_handle = pServer->getPeerInfo(0).getConnHandle();
    int8_t rssi = NimBLEDevice::getClientByID(conn_handle) ?
                  NimBLEDevice::getClientByID(conn_handle)->getRssi() : -127;

    Serial.printf("[BLE] RSSI: %d dBm (threshold: %d)\n", rssi, rssi_threshold_dbm);
    return rssi >= rssi_threshold_dbm;
}

void ble_context_set_callback(ble_context_callback_t cb) {
    user_callback = cb;
}
