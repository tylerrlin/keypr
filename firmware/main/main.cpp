#include <Arduino.h>
#include "usb_hid.h"
#include "ble_context.h"
#include "storage.h"
#include "ctap2.h"

#define STATUS_LED_GPIO 2
#define UP_BUTTON_GPIO  0

// static void on_ble_context_changed(bool connected) {
//     digitalWrite(STATUS_LED_GPIO, connected ? HIGH : LOW);
//     Serial.printf("[MAIN] BLE context: %s\n", connected ? "ARMED ✓" : "DISARMED ✗");
// }

void setup() {
    Serial.begin(115200);
    // Give serial a moment to connect (optional, remove for production)
    delay(500);

    Serial.println("[MAIN] ESP32-S3 FIDO2 Key booting...");

    // LED + button
    pinMode(STATUS_LED_GPIO, OUTPUT);
    digitalWrite(STATUS_LED_GPIO, LOW);
    pinMode(UP_BUTTON_GPIO, INPUT_PULLUP);

    // Boot blink
    for (int i = 0; i < 3; i++) {
        digitalWrite(STATUS_LED_GPIO, HIGH); delay(100);
        digitalWrite(STATUS_LED_GPIO, LOW);  delay(100);
    }

    // Init storage (NVS)
    if (!storage_init()) {
        Serial.println("[MAIN] Storage init failed!");
        // Don't halt - continue without persistent storage for now
    }

    // Init BLE proximity context
    // ble_context_set_callback(on_ble_context_changed);
    ble_context_init();

    // // Init USB HID - do this last
    // // On Arduino framework with native USB, USB starts automatically
    // // but we need TinyUSB configured before any HID traffic
    // usb_hid_init();

    Serial.println("");
    Serial.println("[MAIN] ════════════════════════════════");
    Serial.println("[MAIN]   FIDO2 Key ready");
    Serial.println("[MAIN]   LED OFF = auth BLOCKED");
    Serial.println("[MAIN]   LED ON  = auth ARMED");
    Serial.println("[MAIN]   BLINK   = waiting for button");
    Serial.printf("[MAIN]   Credentials stored: %d\n", storage_count());
    Serial.println("[MAIN] ════════════════════════════════");
}

void loop() {
    delay(1);
}
