#include <Arduino.h>
#include "usb_hid.h"
#include "ble_context.h"
#include "storage.h"
#include "ctap2.h"


#define STATUS_LED_GPIO 2
#define UP_BUTTON_GPIO  0


static void on_ble_context_changed(bool connected) {
    digitalWrite(STATUS_LED_GPIO, connected ? HIGH : LOW);
}

void setup() {
    fido2_hid_early_init();  // USB starts here - must be first

    Serial.begin(115200);
    // DO NOT wait for Serial in OTG mode - it will block forever
    // while (!Serial) <- DELETE THIS
    delay(500);

    Serial.println("[MAIN] booting...");

    if (!storage_init()) {
        Serial.println("[MAIN] Storage init failed!");
    }

    ble_context_set_callback(on_ble_context_changed);
    ble_context_init();

    usb_hid_init();

    Serial.println("[MAIN] ready");
}

void loop() {
    //Serial.println("hello world!");
    usb_hid_task();
    delay(1);
}
