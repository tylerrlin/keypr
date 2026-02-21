#pragma once
#include <stdint.h>
#include <stddef.h>
#include "ctap2_types.h"

// Initialize Adafruit TinyUSB HID stack
void usb_hid_init(void);

// Must be called in Arduino loop() - processes pending USB tasks
void usb_hid_task(void);

// Send CTAPHID error on a channel
void ctaphid_send_error(uint32_t cid, uint8_t err);

// CTAPHID response builders
void ctaphid_send_init_response(uint32_t cid, const uint8_t *nonce);
void ctaphid_send_cbor_response(uint32_t cid, const uint8_t *cbor_data, size_t cbor_len);
