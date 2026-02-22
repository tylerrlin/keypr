#include "ble_keypr.h"
#include "ble_context.h"
#include <Arduino.h>
#include <NimBLEDevice.h>
#define EXIT_UNIMPLEMENTED 4

const uint8_t vdmacaddr[6] = {0x8b, 0xef, 0xce, 0x02, 0x50, 0x3c};

// // INIT process for device connecting for the first time
// // Should perform some authentication procedure and establish future
// authentication (i.e. set MAC address required) void ble_first_connect(void) {
//     exit(EXIT_UNIMPLEMENTED);
// }

bool isSameMac(uint8_t *add) {

  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", add[5], add[4], add[3],
                add[2], add[1], add[0]);

  for (int i = 0; i < 6; i++) {
    if (vdmacaddr[i] != add[i]) {
      return false;
    }
  }

  return true;
}

void ble_connected_callback(uint8_t *macaddr) {
  Serial.println("[BLE - DEBUG] Connection Callback");
//  ! bonded happens AFTER this callback but this SHOULD be called before st we authenticate before pairing
  bool auth = isSameMac(macaddr);

  if (!auth) {
    Serial.println("[BLE - DEBUG] WRONG MAC ADDR #notvdawg");
    // NimBLEDevice::getServer()->disconnect(desc->conn_handle);
  }

  // Upon connection,
  //
  //  authenticate mac address,
  //
  //  if differs disconnect device,
  //
  //  if authenticated, continue
}

void ble_disconnected_callback(void) {
  Serial.println("[BLE - DEBUG] Disconnection Callback");
}

void ble_connection_callback(bool connected, uint8_t *macaddr) {
  if (connected) {
    ble_connected_callback(macaddr);
  } else {
    ble_disconnected_callback();
  }
}

bool ble_device_authenticated(void) { exit(EXIT_UNIMPLEMENTED); }
