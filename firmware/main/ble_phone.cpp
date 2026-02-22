#include "ble_phone.h"
#include <string.h>
#include <Preferences.h>

extern Preferences prefs;
extern NimBLECharacteristic* pCharacteristic;

class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    // called when phone writes to ESP32
    void onWrite(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc) override {
        std::string message = pCharacteristic->getValue();
        // Serial.print("[BLE] Received: ");
        // Serial.print(message);

        prefs.putUShort("msg_size", message.length());
        prefs.putBytes("msg", message.c_str(), message.length());
        prefs.putBool("newMsg", true);
    }

    void onRead(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc) override {
        Serial.println("[BLE] Client read characteristic");
    }
};

// will silently fail if connected is false
bool ble_send(char* message) {

    bool connected = prefs.getBool("t_connected", false);
    if (!connected) {
        return false;
    }
    pCharacteristic->setValue(message);
    pCharacteristic->indicate();   
    return true;
}

bool ble_wait_for_response(char *buf, int timeout) {
    int clock = 0; 
    while (!prefs.getBool("newMsg", false) && clock < timeout) {
        delay(100);
        Serial.print("1");
        clock += 100;
    } 
    Serial.print("\n");

    if (!prefs.getBool("newMsg", false)) {
        return false;
    } else {
        uint16_t size = prefs.getUShort("msg_size", 0);
        if (size == 0) {
            return false;
        }

        int read = prefs.getBytes("msg", buf, size);
        if (read == 0) {
            return false;
        }
    }

    prefs.putBool("newMsg", true);
    return true;
}