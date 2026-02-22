#include "ble_context.h"
#include "ble_keypr.h"
#include <Arduino.h>
#include <NimBLEDevice.h>

static bool phone_connected = false;
static ble_context_callback_t user_callback = ble_connection_callback;
static NimBLEServer *pServer = nullptr;
static bool trusted_device_enrolled = false;

#define BLE_DEVICE_NAME "FIDO2-AuthNode-VDAWG"

class ConnectionCallbacks : public NimBLEServerCallbacks
{
    // void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) override
    // {
    //     phone_connected = true;
    //     Serial.println("[BLE] Phone connected - auth ARMED");
    //     // pServer->startEncryption(connInfo.getHandle(),
    //     //                          BLE_SM_PAIR_AUTHREQ_BOND);
    //     ble_gap_security_initiate(desc->conn_handle);

    //     // TODO check every 5 seconds for bonding
    //     // TODO
    //     if (!desc->sec_state.bonded) {

    //         if (trusted_device_enrolled) {
    //             Serial.println("[BLE - DEBUG] Auth failed - abort connection");
    //             pServer->disconnect(desc->conn_handle);
    //         } else {
    //             Serial.println(
    //                 "[BLE - DEBUG] Phone not bonded - init bond procedure");
    //             NimBLEDevice::startSecurity(desc->conn_handle);
    //             delay(500);
    //         }
    //     }

    //     // TODO: store mac address in non voltaile
    //     if (!desc->sec_state.bonded) {
    //         Serial.println("[BLE - DEBUG] Paring failed, john is sad </3");
    //     }

    //     if (user_callback) {
    //         user_callback(true, desc->peer_id_addr.val);
    //     }

    //     Serial.println("[BLE] Bonded device authenticated - auth ARMED");

    //     // Don't restart advertising - we only want one connection
    // }

    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) override {
        Serial.println("[BLE] Connected. Starting security.");

        // ! when vedant attempt reconnect, went to auth failed abort, shouldve recognized pairing!
        if (!desc->sec_state.bonded) {
            if (trusted_device_enrolled) {
                Serial.println("[BLE - DEBUG] Auth failed - abort connection");
                pServer->disconnect(desc->conn_handle);
            } else {
                ble_gap_security_initiate(desc->conn_handle);
                NimBLEDevice::startSecurity(desc->conn_handle);
            }
        } 


        if (user_callback) {
            user_callback(true, desc->peer_id_addr.val);
        }
    }

    void onAuthenticationComplete(ble_gap_conn_desc* desc) override {
        if (!desc->sec_state.bonded) {
            Serial.println("[BLE] Bonding failed. Disconnecting.");
            NimBLEDevice::getServer()->disconnect(desc->conn_handle);
            return;
        }

        Serial.println("[BLE] Bonded successfully ✅");
        trusted_device_enrolled = true;

    }

    void onDisconnect(NimBLEServer *pServer) override
    {
        phone_connected = false;
        Serial.println("[BLE] Phone disconnected - auth DISARMED");
        // if (user_callback)
        //     user_callback(false, NULL);
        // Restart advertising so phone can reconnect
        NimBLEDevice::startAdvertising();
    }
};

void ble_context_init(void)
{
    NimBLEDevice::init(BLE_DEVICE_NAME);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); // max power for range

    NimBLEDevice::setSecurityAuth(true, false, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

    NimBLEDevice::setSecurityInitKey(BLE_SM_PAIR_KEY_DIST_ENC);
    NimBLEDevice::setSecurityRespKey(BLE_SM_PAIR_KEY_DIST_ENC);

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ConnectionCallbacks());

    // Create the service and characteristic
    NimBLEService *pService = pServer->createService(BLE_SERVICE_UUID);
    NimBLECharacteristic *pChar =
        pService->createCharacteristic(BLE_CHAR_UUID, NIMBLE_PROPERTY::READ);
    pChar->setValue("FIDO2-AuthNode-VDAWG");
    pService->start();

    // Set up advertising
    NimBLEAdvertising *pAdv = NimBLEDevice::getAdvertising();
    pAdv->addServiceUUID(BLE_SERVICE_UUID);
    pAdv->setScanResponse(true);
    pAdv->setMinPreferred(0x06);
    NimBLEDevice::startAdvertising();

    Serial.println("[BLE] Advertising as 'FIDO2-AuthNode-VDAWG'");
}

bool ble_context_is_phone_connected(void) { return phone_connected; }

bool ble_context_is_phone_nearby(int8_t rssi_threshold_dbm)
{
    if (!phone_connected || pServer == nullptr)
        return false;
    if (pServer->getConnectedCount() == 0)
        return false;

    // Get RSSI of first connected client
    uint16_t conn_handle = pServer->getPeerInfo(0).getConnHandle();
    int8_t rssi = NimBLEDevice::getClientByID(conn_handle)
                      ? NimBLEDevice::getClientByID(conn_handle)->getRssi()
                      : -127;

    Serial.printf("[BLE] RSSI: %d dBm (threshold: %d)\n", rssi,
                  rssi_threshold_dbm);
    return rssi >= rssi_threshold_dbm;
}

// void ble_context_set_callback(ble_context_callback_t cb) {
//     user_callback = cb;
// }
