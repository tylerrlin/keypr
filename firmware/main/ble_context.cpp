#include "ble_context.h"
#include "ble_keypr.h"
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <Preferences.h>
#include <assert.h>
#include <esp_system.h>

static ble_context_callback_t user_callback = ble_connection_callback;
static NimBLEServer *pServer = nullptr;
static bool phone_connected = false;
NimBLECharacteristic *pCharacteristic;


#define BLE_DEVICE_NAME "FIDO2-AuthNode-VDAWG"

Preferences prefs;

class ConnectionCallbacks : public NimBLEServerCallbacks
{
    void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) override
    {
        Serial.println("[BLE] Connected. Starting security.");

        // uint8_t ty;
        // uint8_t vals[6];
        // prefs.getBytes("mac_type", &ty, 1);
        // prefs.getBytes("mac_val", vals, 6);
        // Serial.printf(
        //     "[DEBUG] [1/4] [t_connected %d] [has_trusted %d] [mac_type "
        //     "%2x] [mac_val %2x:%2x:%2x:%2x:%2x:%2x]\n",
        //     prefs.getBool("t_connected", false),
        //     prefs.getBool("has_trusted", false), ty, vals[5], vals[4], vals[3],
        //     vals[2], vals[1], vals[0]);

        // Serial.printf(
        //     "[DEBUG] [CONNECT MAC ADDR] [mac_val %2x:%2x:%2x:%2x:%2x:%2x]\n", 
        //     desc->peer_id_addr.val[5], desc->peer_id_addr.val[4], desc->peer_id_addr.val[3],
        //     desc->peer_id_addr.val[2], desc->peer_id_addr.val[1], desc->peer_id_addr.val[0]);

        // ! when vedant attempt reconnect, went to auth failed abort, shouldve
        // recognized pairing!
        // ble_gap_security_initiate(desc->conn_handle);
        phone_connected = true;
        prefs.putBool("t_connected", true);

        NimBLEDevice::startSecurity(desc->conn_handle);

        // prefs.getBytes("mac_type", &ty, 1);
        // prefs.getBytes("mac_val", vals, 6);
        // Serial.printf(
        //     "[DEBUG] [2/4] [t_connected %d] [has_trusted %d] [mac_type "
        //     "%2x] [mac_val %2x:%2x:%2x:%2x:%2x:%2x]\n",
        //     prefs.getBool("t_connected", false),
        //     prefs.getBool("has_trusted", false), ty, vals[5], vals[4], vals[3],
        //     vals[2], vals[1], vals[0]);
    }

    void onAuthenticationComplete(ble_gap_conn_desc *desc) override
    {
        if (!desc->sec_state.bonded) {
            Serial.println("[BLE] Bonding failed. Disconnecting.");
            NimBLEDevice::getServer()->disconnect(desc->conn_handle);
            return;
        }

        if (prefs.getBool("has_trusted",
                          false)) { // TODO CHECK MATCH TRUSTED ADDR
            if (!auth_addr(desc->peer_id_addr.type, desc->peer_id_addr.val)) {
                bool save = prefs.getBool("t_connected", false);
                Serial.println(
                    "[BLE] Unknown device attempted connection - rejecting");
                NimBLEDevice::deleteBond(desc->peer_id_addr);
                NimBLEDevice::getServer()->disconnect(desc->conn_handle);
                prefs.putBool("t_connected", save);
                return;
            }
        } else { // PLACE TRUSTED ADDR
            Serial.println("[DEBUG] SETTING HAS TRUSTED TO 1");
            prefs.putBool("has_trusted", true);
            prefs.putBytes("mac_type", &desc->peer_id_addr.type, 1);
            prefs.putBytes("mac_val", desc->peer_id_addr.val, 6);
        }

        Serial.println("[BLE] Bonded successfully ✅");
        if (user_callback) {
            user_callback(true, desc->peer_id_addr.val);
        }
    }


    void onDisconnect(NimBLEServer *pServer) override
    {

        
        uint8_t ty;
        uint8_t vals[6];
        prefs.getBytes("mac_type", &ty, 1);
        prefs.getBytes("mac_val", vals, 6);
        Serial.printf(
            "[DEBUG] [3/4] [t_connected %d] [has_trusted %d] [mac_type "
            "%2x] [mac_val %2x:%2x:%2x:%2x:%2x:%2x]\n",
            prefs.getBool("t_connected", false),
            prefs.getBool("has_trusted", false), ty, vals[5], vals[4], vals[3],
            vals[2], vals[1], vals[0]);
        
        Serial.println("[BLE] Phone disconnected - auth DISARMED");
        prefs.putBool("t_connected", false);
        phone_connected = false;

        prefs.getBytes("mac_type", &ty, 1);
        prefs.getBytes("mac_val", vals, 6);
        Serial.printf(
            "[DEBUG] [4/4] [t_connected %d] [has_trusted %d] [mac_type "
            "%2x] [mac_val %2x:%2x:%2x:%2x:%2x:%2x]\n",
            prefs.getBool("t_connected", false),
            prefs.getBool("has_trusted", false), ty, vals[5], vals[4], vals[3],
            vals[2], vals[1], vals[0]);

        NimBLEDevice::startAdvertising();
    }
};

void ble_context_init(void)
{
    prefs.begin("ble", false);

    ///////TODO comment this out if you dare
    prefs.clear();
    Serial.println("[NVS] Wiped!");
    ///////

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
    pCharacteristic =
        pService->createCharacteristic(BLE_CHAR_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
    pCharacteristic->setValue("FIDO2-AuthNode-VDAWG");
    pService->start();

    // Set up advertising
    NimBLEAdvertising *pAdv = NimBLEDevice::getAdvertising();
    pAdv->addServiceUUID(BLE_SERVICE_UUID);
    pAdv->setScanResponse(true);
    pAdv->setMinPreferred(0x06);
    NimBLEDevice::startAdvertising();

    Serial.println("[BLE] Advertising as 'FIDO2-AuthNode-VDAWG'");
}

bool ble_context_is_phone_connected(void)
{
    return prefs.getBool("t_connected", false);
}

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
