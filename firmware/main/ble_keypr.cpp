#include "ble_keypr.h"
#include "ble_context.h"
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <Preferences.h>
#define EXIT_UNIMPLEMENTED 4


extern Preferences prefs;

void ble_connected_callback(uint8_t *macaddr)
{
    Serial.println("[BLE - DEBUG] Connection Callback");
}

void ble_disconnected_callback(void)
{
    Serial.println("[BLE - DEBUG] Disconnection Callback");
}

void ble_connection_callback(bool connected, uint8_t *macaddr)
{
    if (connected) {
        ble_connected_callback(macaddr);
    } else {
        ble_disconnected_callback();
    }
}

bool auth_addr(uint8_t ty, uint8_t *vals)
{
    uint8_t real_ty;
    prefs.getBytes("mac_type", &real_ty, sizeof(real_ty));
    if (ty != real_ty) {
        return false;
    }

    uint8_t real_vals[6];
    prefs.getBytes("mac_val", real_vals,
                   sizeof(real_vals) / sizeof(uint8_t));

    for (int i = 0; i < 6; i++) {
        Serial.printf("%2x <- val | real_val -> %2x\n", vals[i], real_vals[i]);
    }
    for (int i = 0; i < 6; i++) {
        if (vals[i] != real_vals[i]) {
            return false;
        }
    }

    return true;
}
