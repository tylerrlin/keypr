#ifndef __BLE_PHONE__
#define __BLE_PHONE__

#include "ble_context.h"
#include "ble_keypr.h"
#include <stdbool.h>
#include <Arduino.h>
#include <NimBLEDevice.h>

extern void ble_send(const char* message);

#endif