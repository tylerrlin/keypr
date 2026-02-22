#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <ble_keypr_structures.h>

// returns true if device can be authenticated and false otherwise
bool ble_device_authenticated(void);

void ble_connection_callback(bool connected, uint8_t *macaddr);
