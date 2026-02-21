#include "crypto.h"
#include "ctap2_types.h"
#include <string.h>
#include <Arduino.h>  // for esp_random()

// mbedTLS includes - same as before, available under Arduino ESP32
#include "mbedtls/ecdsa.h"
#include "mbedtls/ecp.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/sha256.h"

// Rest of the file is identical to the original crypto.c
// Copy everything from crypto.c below this line unchanged
// (the mbedTLS API is the same under Arduino ESP32)

static mbedtls_entropy_context  entropy;
static mbedtls_ctr_drbg_context ctr_drbg;
static bool rng_initialized = false;

static void ensure_rng(void) {
    if (rng_initialized) return;
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    const char *pers = "fido2_esp32s3";
    mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                           (const unsigned char *)pers, strlen(pers));
    rng_initialized = true;
}

bool crypto_random(uint8_t *buf, size_t len) {
    ensure_rng();
    return mbedtls_ctr_drbg_random(&ctr_drbg, buf, len) == 0;
}

bool crypto_sha256(const uint8_t *data, size_t len, uint8_t *hash_out) {
    // Arduino ESP32 mbedTLS uses _ret variants that actually return int
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts_ret(&ctx, 0);
    mbedtls_sha256_update_ret(&ctx, data, len);
    mbedtls_sha256_finish_ret(&ctx, hash_out);
    mbedtls_sha256_free(&ctx);
    return true;
}

bool crypto_generate_keypair(uint8_t *private_key, uint8_t *pub_x, uint8_t *pub_y) {
    ensure_rng();
    mbedtls_ecdsa_context ctx;
    mbedtls_ecdsa_init(&ctx);

    int ret = mbedtls_ecdsa_genkey(&ctx, MBEDTLS_ECP_DP_SECP256R1,
                                   mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret) {
        Serial.printf("[CRYPTO] Key generation failed: -0x%04x\n", -ret);
        mbedtls_ecdsa_free(&ctx);
        return false;
    }

    // Arduino ESP32 mbedTLS exposes struct members directly - no MBEDTLS_PRIVATE
    size_t privlen = mbedtls_mpi_size(&ctx.d);
    memset(private_key, 0, PRIVATE_KEY_LEN);
    mbedtls_mpi_write_binary(&ctx.d,
                             private_key + (PRIVATE_KEY_LEN - privlen),
                             privlen);

    uint8_t pub_buf[65];
    size_t pub_len;
    mbedtls_ecp_point_write_binary(&ctx.grp, &ctx.Q,
                                    MBEDTLS_ECP_PF_UNCOMPRESSED,
                                    &pub_len, pub_buf, sizeof(pub_buf));
    memcpy(pub_x, pub_buf + 1,  32);
    memcpy(pub_y, pub_buf + 33, 32);

    mbedtls_ecdsa_free(&ctx);
    return true;
}

bool crypto_sign(const uint8_t *private_key, const uint8_t *data, size_t data_len,
                 uint8_t *sig_buf, size_t *sig_len) {
    ensure_rng();

    uint8_t hash[32];
    if (!crypto_sha256(data, data_len, hash)) return false;

    mbedtls_ecdsa_context ctx;
    mbedtls_ecdsa_init(&ctx);
    // Direct member access - no MBEDTLS_PRIVATE
    mbedtls_ecp_group_load(&ctx.grp, MBEDTLS_ECP_DP_SECP256R1);
    mbedtls_mpi_read_binary(&ctx.d, private_key, PRIVATE_KEY_LEN);

    // Arduino mbedTLS ecdsa_write_signature takes size_t* for sig_len, no max_sig_len param
    int ret = mbedtls_ecdsa_write_signature(
        &ctx, MBEDTLS_MD_SHA256,
        hash, sizeof(hash),
        sig_buf, sig_len,
        mbedtls_ctr_drbg_random, &ctr_drbg
    );
    mbedtls_ecdsa_free(&ctx);

    if (ret) {
        Serial.printf("[CRYPTO] ECDSA sign failed: -0x%04x\n", -ret);
        return false;
    }
    return true;
}

size_t crypto_build_auth_data(
    const uint8_t *rp_id_hash,
    bool user_present,
    bool user_verified,
    bool include_attested_cred,
    uint32_t sign_count,
    const uint8_t *credential_id,
    const uint8_t *pub_key_x,
    const uint8_t *pub_key_y,
    uint8_t *out_buf,
    size_t out_buf_max)
{
    size_t offset = 0;

    memcpy(out_buf + offset, rp_id_hash, 32);
    offset += 32;

    uint8_t flags = 0;
    if (user_present)          flags |= 0x01;
    if (user_verified)         flags |= 0x04;
    if (include_attested_cred) flags |= 0x40;
    out_buf[offset++] = flags;

    out_buf[offset++] = (sign_count >> 24) & 0xFF;
    out_buf[offset++] = (sign_count >> 16) & 0xFF;
    out_buf[offset++] = (sign_count >>  8) & 0xFF;
    out_buf[offset++] = (sign_count      ) & 0xFF;

    if (include_attested_cred && credential_id && pub_key_x && pub_key_y) {
        memset(out_buf + offset, 0, AAGUID_LEN);
        offset += AAGUID_LEN;

        out_buf[offset++] = 0x00;
        out_buf[offset++] = CREDENTIAL_ID_LEN;

        memcpy(out_buf + offset, credential_id, CREDENTIAL_ID_LEN);
        offset += CREDENTIAL_ID_LEN;

        // COSE_Key CBOR encoding
        out_buf[offset++] = 0xA5;
        out_buf[offset++] = 0x01; out_buf[offset++] = 0x02;
        out_buf[offset++] = 0x03; out_buf[offset++] = 0x26;
        out_buf[offset++] = 0x20; out_buf[offset++] = 0x01;
        out_buf[offset++] = 0x21; out_buf[offset++] = 0x58; out_buf[offset++] = 0x20;
        memcpy(out_buf + offset, pub_key_x, 32); offset += 32;
        out_buf[offset++] = 0x22; out_buf[offset++] = 0x58; out_buf[offset++] = 0x20;
        memcpy(out_buf + offset, pub_key_y, 32); offset += 32;
    }

    return offset;
}
