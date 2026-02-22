#include "ble_phone.h"
#include <string.h>
#include <Preferences.h>

extern Preferences prefs;
extern NimBLECharacteristic* pCharacteristic;

class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc) override {
        std::string message = pCharacteristic->getValue();
        Serial.print("[BLE] Received: ");
        //Serial.print(message);
    }

    void onRead(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc) override {
        Serial.println("[BLE] Client read characteristic");
    }
};

// will silently fail if connected is false
void ble_send(const char* message) {

    bool connected = prefs.getBool("t_connected", false);
    if (connected) {
        pCharacteristic->setValue(message);
        pCharacteristic->indicate();   
    }
}

/*

void ble_wait_for_response(char *buf, int timeout) {
    int clock = 0; 
    while (prefs.getBool("newMessage", false) && clock < timeout) {
        delay(100);
    } 
    
}

*/ 