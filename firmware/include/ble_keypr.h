#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

void ble_connection_callback(bool connected, uint8_t *macaddr);
bool auth_addr(uint8_t ty, uint8_t *vals);