#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "ctap2_types.h"

// Initialize NVS storage
bool storage_init(void);

// Store a new credential
bool storage_store_credential(const fido2_credential_t *cred);

// Find credential by RP ID hash and credential ID
// Returns true and fills cred if found
bool storage_find_credential(
    const uint8_t *rp_id_hash,
    const uint8_t *credential_id,
    fido2_credential_t *cred_out
);

// Find all credentials for an RP (for get_assertion allowList=empty case)
// Returns count of credentials found
int storage_find_credentials_for_rp(
    const uint8_t      *rp_id_hash,
    fido2_credential_t *creds_out,
    int                 max_creds
);

// Update sign counter after successful assertion
bool storage_update_sign_count(
    const uint8_t *credential_id,
    uint32_t       new_count
);

// Wipe all credentials (CTAP2 reset command)
bool storage_reset(void);

// How many credentials are stored
int storage_count(void);
