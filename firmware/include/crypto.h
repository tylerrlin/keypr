#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Generate a new P-256 keypair
// private_key: 32 bytes out
// public_key_x: 32 bytes out (x coordinate)
// public_key_y: 32 bytes out (y coordinate)
bool crypto_generate_keypair(
    uint8_t *private_key,
    uint8_t *public_key_x,
    uint8_t *public_key_y
);

// Sign data with ECDSA P-256 / SHA-256
// Returns DER-encoded signature in sig_buf
// sig_len is set to actual signature length (max 72 bytes)
bool crypto_sign(
    const uint8_t *private_key,
    const uint8_t *data,
    size_t         data_len,
    uint8_t       *sig_buf,
    size_t        *sig_len
);

// SHA-256 hash
bool crypto_sha256(
    const uint8_t *data,
    size_t         len,
    uint8_t       *hash_out  // 32 bytes
);

// Fill buffer with cryptographically secure random bytes
bool crypto_random(uint8_t *buf, size_t len);

// Build the authenticator data structure per WebAuthn spec
// §6.1 Authenticator Data
size_t crypto_build_auth_data(
    const uint8_t *rp_id_hash,          // 32 bytes
    bool           user_present,
    bool           user_verified,
    bool           include_attested_cred,
    uint32_t       sign_count,
    const uint8_t *credential_id,       // CREDENTIAL_ID_LEN bytes, or NULL
    const uint8_t *pub_key_x,           // 32 bytes, or NULL
    const uint8_t *pub_key_y,           // 32 bytes, or NULL
    uint8_t       *out_buf,
    size_t         out_buf_max
);
