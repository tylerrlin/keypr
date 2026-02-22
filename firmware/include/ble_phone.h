#ifndef __BLE_PHONE__
#define __BLE_PHONE__

#include "ble_context.h"
#include "ble_keypr.h"
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <stdbool.h>


extern bool ble_send(char *message);
extern bool ble_wait_for_response(char *buf, int timeout);

#endif