#pragma once
#include <stdint.h>
#include <stddef.h>
#include "ctap2_types.h"

// Process a full CTAP2 CBOR command buffer
// Returns response in out_buf, length in out_len
// First byte of out_buf is always the CTAP status code
uint8_t ctap2_process(
    const uint8_t *in_buf,
    size_t         in_len,
    uint8_t       *out_buf,
    size_t        *out_len
);

// Individual command handlers
uint8_t ctap2_get_info(uint8_t *out_buf, size_t *out_len);
uint8_t ctap2_make_credential(const uint8_t *cbor, size_t len, uint8_t *out_buf, size_t *out_len);
uint8_t ctap2_get_assertion(const uint8_t *cbor, size_t len, uint8_t *out_buf, size_t *out_len);

// User presence - blocks until button pressed or timeout
// Returns true if user confirmed
bool ctap2_wait_for_user_presence(uint32_t timeout_ms);
