#include "ctap2.h"
#include "ctap2_types.h"
#include "crypto.h"
#include "storage.h"
#include "ble_context.h"
#include <string.h>
#include <Arduino.h>      // replaces esp_log.h - use Serial.printf instead
#include <Preferences.h>

static Preferences ble_prefs;
static bool ble_prefs_opened = false;

static void ensure_ble_prefs() {
    if (!ble_prefs_opened) {
        ble_prefs.begin("ble", true);  // true = read-only, same namespace as ble_context.cpp
        ble_prefs_opened = true;
    }
}

// ─── NOTE ON CBOR ─────────────────────────────────────────────────────────────
// For a production build, use tinycbor or cn-cbor.
// For the hackathon, this implements minimal hand-rolled CBOR parsing/encoding
// sufficient for the CTAP2 commands we need. A proper CBOR lib is strongly
// recommended for production.

static const char *TAG = "CTAP2";

// GPIO for user presence button (connect a button between GPIO0 and GND)
#define UP_BUTTON_GPIO GPIO_NUM_0
// LED for visual feedback (built-in LED on most devkit boards)
#define STATUS_LED_GPIO 48

// ─── Minimal CBOR helpers ─────────────────────────────────────────────────────

// Write a CBOR unsigned int
static size_t cbor_write_uint(uint8_t *buf, uint64_t val) {
    if (val <= 23) { buf[0] = (uint8_t)val; return 1; }
    if (val <= 0xFF) { buf[0] = 0x18; buf[1] = (uint8_t)val; return 2; }
    if (val <= 0xFFFF) { buf[0] = 0x19; buf[1] = val>>8; buf[2] = val&0xFF; return 3; }
    return 0;
}

// Write CBOR bytes
static size_t cbor_write_bytes(uint8_t *buf, const uint8_t *data, size_t len) {
    size_t hdr = 0;
    if (len <= 23) { buf[0] = 0x40 | len; hdr = 1; }
    else if (len <= 0xFF) { buf[0] = 0x58; buf[1] = (uint8_t)len; hdr = 2; }
    else { buf[0] = 0x59; buf[1] = len>>8; buf[2] = len&0xFF; hdr = 3; }
    memcpy(buf + hdr, data, len);
    return hdr + len;
}

// Write CBOR text string
static size_t cbor_write_text(uint8_t *buf, const char *str) {
    size_t len = strlen(str);
    size_t hdr = 0;
    if (len <= 23) { buf[0] = 0x60 | len; hdr = 1; }
    else if (len <= 0xFF) { buf[0] = 0x78; buf[1] = (uint8_t)len; hdr = 2; }
    else { buf[0] = 0x79; buf[1] = len>>8; buf[2] = len&0xFF; hdr = 3; }
    memcpy(buf + hdr, str, len);
    return hdr + len;
}

// Read CBOR map key (positive uint)
static uint8_t cbor_read_uint_key(const uint8_t *buf, size_t *consumed) {
    *consumed = 1;
    uint8_t v = buf[0] & 0x1F;
    if (v <= 23) return v;
    if (v == 24) { *consumed = 2; return buf[1]; }
    return 0xFF;
}

// Skip a CBOR value, return bytes consumed
static size_t cbor_skip(const uint8_t *buf, size_t max) {
    if (max == 0) return 0;
    uint8_t mt = (buf[0] >> 5) & 7;
    uint8_t ai = buf[0] & 0x1F;

    uint64_t extra = 0;
    size_t hdr = 1;
    if (ai == 24) { extra = buf[1]; hdr = 2; }
    else if (ai == 25) { extra = ((uint64_t)buf[1]<<8)|buf[2]; hdr = 3; }
    else if (ai == 26) { extra = ((uint64_t)buf[1]<<24)|((uint64_t)buf[2]<<16)|((uint64_t)buf[3]<<8)|buf[4]; hdr = 5; }
    else if (ai <= 23) { extra = ai; }

    switch (mt) {
        case 0: case 1: return hdr;                          // uint, negint
        case 2: case 3: return hdr + (size_t)extra;          // bytes, text
        case 4: {                                             // array
            size_t total = hdr;
            for (uint64_t i = 0; i < extra; i++) total += cbor_skip(buf + total, max - total);
            return total;
        }
        case 5: {                                             // map
            size_t total = hdr;
            for (uint64_t i = 0; i < extra * 2; i++) total += cbor_skip(buf + total, max - total);
            return total;
        }
        default: return 1;
    }
}

// Read bytes from CBOR bytes value
static const uint8_t *cbor_read_bytes(const uint8_t *buf, size_t *out_len, size_t *consumed) {
    uint8_t ai = buf[0] & 0x1F;
    size_t hdr = 1;
    size_t len = 0;
    if (ai <= 23) { len = ai; hdr = 1; }
    else if (ai == 24) { len = buf[1]; hdr = 2; }
    else if (ai == 25) { len = ((size_t)buf[1]<<8)|buf[2]; hdr = 3; }
    *out_len = len;
    *consumed = hdr + len;
    return buf + hdr;
}

// Read text from CBOR text value
static const uint8_t *cbor_read_text(const uint8_t *buf, size_t *out_len, size_t *consumed) {
    return cbor_read_bytes(buf, out_len, consumed); // same encoding, different major type
}

// ─── User Presence ────────────────────────────────────────────────────────────
bool ctap2_wait_for_user_presence(uint32_t timeout_ms) {


    ensure_ble_prefs();
    bool trusted = ble_prefs.getBool("t_connected", false);
    bool connected = ble_prefs.getBool("has_trusted", false);
    if (!connected || !trusted) {
        return false;
    }
    return true;
    pinMode(UP_BUTTON_GPIO, INPUT_PULLUP);
    pinMode(STATUS_LED_GPIO, OUTPUT);

    uint32_t elapsed = 0;
    Serial.println("[CTAP2] Waiting for user presence (press button)...");
    while (elapsed < timeout_ms) {
        digitalWrite(STATUS_LED_GPIO, (elapsed / 200) % 2);
        if (digitalRead(UP_BUTTON_GPIO) == LOW) {  // active low
            digitalWrite(STATUS_LED_GPIO, HIGH);
            delay(100);
            digitalWrite(STATUS_LED_GPIO, LOW);
            Serial.println("[CTAP2] User presence confirmed");
            return true;
        }
        delay(50);
        elapsed += 50;
    }
    Serial.println("[CTAP2] User presence timeout");
    digitalWrite(STATUS_LED_GPIO, LOW);
    return false;
}

// ─── getInfo ──────────────────────────────────────────────────────────────────
uint8_t ctap2_get_info(uint8_t *out_buf, size_t *out_len) {
    uint8_t *p = out_buf;
    *p++ = CTAP2_OK;  // status

    // CBOR map with 4 entries
    *p++ = 0xA4;

    // key 1: versions = ["FIDO_2_0"]
    p += cbor_write_uint(p, 1);
    *p++ = 0x81;  // array(1)
    p += cbor_write_text(p, "FIDO_2_0");

    // key 2: extensions = [] (none for now)
    p += cbor_write_uint(p, 2);
    *p++ = 0x80;  // array(0)

    // key 3: aaguid (16 bytes of zeros)
    p += cbor_write_uint(p, 3);
    uint8_t aaguid[16] = {0};
    p += cbor_write_bytes(p, aaguid, 16);

    // key 4: options = { "rk": true, "up": true, "uv": false }
    p += cbor_write_uint(p, 4);
    *p++ = 0xA3;       // map(3)
    p += cbor_write_text(p, "rk"); *p++ = 0xF5;  // true
    p += cbor_write_text(p, "up"); *p++ = 0xF5;  // true
    p += cbor_write_text(p, "uv"); *p++ = 0xF4;  // false

    *out_len = p - out_buf;
    return CTAP2_OK;
}

// ─── makeCredential ───────────────────────────────────────────────────────────
uint8_t ctap2_make_credential(const uint8_t *cbor, size_t len,
                               uint8_t *out_buf, size_t *out_len) {
    const uint8_t *client_data_hash = NULL;
    size_t cdh_len = 0;
    const uint8_t *rp_id = NULL;
    size_t rp_id_len = 0;
    const uint8_t *user_id = NULL;
    size_t user_id_len = 0;
    const char *user_name = NULL;
    size_t user_name_len = 0;

    ensure_ble_prefs();
    bool trusted = ble_prefs.getBool("t_connected", false);
    bool has_trusted = ble_prefs.getBool("has_trusted", false);
    if (!has_trusted || !trusted) {
        out_buf[0] = CTAP2_ERR_OPERATION_DENIED;
        *out_len = 1;
        return CTAP2_ERR_OPERATION_DENIED;
    }

    if ((cbor[0] & 0xE0) != 0xA0) return CTAP2_ERR_CBOR_UNEXPECTED_TYPE;
    uint8_t map_len = cbor[0] & 0x1F;
    size_t pos = 1;

    for (int i = 0; i < map_len && pos < len; i++) {
        size_t consumed;
        uint8_t key = cbor_read_uint_key(cbor + pos, &consumed);
        pos += consumed;

        switch (key) {
            case 1: {  // clientDataHash
                client_data_hash = cbor_read_bytes(cbor + pos, &cdh_len, &consumed);
                pos += consumed;
                break;
            }
            case 2: {  // rp { id: "...", name: "..." }
                if ((cbor[pos] & 0xE0) == 0xA0) {
                    uint8_t rp_map_len = cbor[pos] & 0x1F;
                    pos++;
                    for (int j = 0; j < rp_map_len; j++) {
                        size_t key_len;
                        const uint8_t *k = cbor_read_text(cbor + pos, &key_len, &consumed);
                        pos += consumed;
                        if (key_len == 2 && memcmp(k, "id", 2) == 0) {
                            rp_id = cbor_read_text(cbor + pos, &rp_id_len, &consumed);
                            pos += consumed;
                        } else {
                            pos += cbor_skip(cbor + pos, len - pos);
                        }
                    }
                } else {
                    pos += cbor_skip(cbor + pos, len - pos);
                }
                break;
            }
            case 3: {  // user { id: bytes, name: "..." }
                if ((cbor[pos] & 0xE0) == 0xA0) {
                    uint8_t u_map_len = cbor[pos] & 0x1F;
                    pos++;
                    for (int j = 0; j < u_map_len; j++) {
                        size_t key_len;
                        const uint8_t *k = cbor_read_text(cbor + pos, &key_len, &consumed);
                        pos += consumed;
                        if (key_len == 2 && memcmp(k, "id", 2) == 0) {
                            user_id = cbor_read_bytes(cbor + pos, &user_id_len, &consumed);
                            pos += consumed;
                        } else if (key_len == 4 && memcmp(k, "name", 4) == 0) {
                            user_name = (const char *)cbor_read_text(cbor + pos, &user_name_len, &consumed);
                            pos += consumed;
                        } else {
                            pos += cbor_skip(cbor + pos, len - pos);
                        }
                    }
                } else {
                    pos += cbor_skip(cbor + pos, len - pos);
                }
                break;
            }
            default:
                pos += cbor_skip(cbor + pos, len - pos);
                break;
        }
    }

    if (!client_data_hash || !rp_id || cdh_len != 32) {
        Serial.println("[CTAP2] makeCredential: missing required params");
        return CTAP2_ERR_MISSING_PARAMETER;
    }

    if (!ctap2_wait_for_user_presence(10000)) {
        return CTAP2_ERR_ACTION_TIMEOUT;
    }

    // Generate credential
    fido2_credential_t cred = {0};
    cred.valid = true;
    cred.sign_count = 0;

    uint8_t pub_x[32], pub_y[32];
    if (!crypto_random(cred.id, CREDENTIAL_ID_LEN)) return CTAP2_ERR_OTHER;
    if (!crypto_generate_keypair(cred.private_key, pub_x, pub_y)) return CTAP2_ERR_OTHER;
    if (!crypto_sha256(rp_id, rp_id_len, cred.rp_id_hash)) return CTAP2_ERR_OTHER;

    if (user_id && user_id_len <= MAX_USER_ID_LEN) {
        memcpy(cred.user_id, user_id, user_id_len);
        cred.user_id_len = user_id_len;
    }
    if (user_name && user_name_len < MAX_USER_NAME_LEN) {
        memcpy(cred.user_name, user_name, user_name_len);
    }

    if (!storage_store_credential(&cred)) return CTAP2_ERR_KEY_STORE_FULL;

    // Build authenticator data
    uint8_t auth_data[512];
    size_t auth_data_len = crypto_build_auth_data(
        cred.rp_id_hash, true, false, true,
        0, cred.id, pub_x, pub_y,
        auth_data, sizeof(auth_data)
    );

    // Sign: authenticatorData || clientDataHash
    uint8_t to_sign[512 + 32];
    memcpy(to_sign, auth_data, auth_data_len);
    memcpy(to_sign + auth_data_len, client_data_hash, 32);

    uint8_t sig[72];
    size_t sig_len = 0;
    if (!crypto_sign(cred.private_key, to_sign, auth_data_len + 32, sig, &sig_len)) {
        return CTAP2_ERR_OTHER;
    }

    // Build attestation object response - MUST use text keys per WebAuthn spec
    uint8_t *p = out_buf;
    *p++ = CTAP2_OK;
    *p++ = 0xA3;  // map(3)

    // "fmt": "none"
    p += cbor_write_text(p, "fmt");
    p += cbor_write_text(p, "none");

    // "attStmt": {} (empty map for "none" attestation)
    p += cbor_write_text(p, "attStmt");
    *p++ = 0xA0;


    // "authData": bytes
    p += cbor_write_text(p, "authData");
    p += cbor_write_bytes(p, auth_data, auth_data_len);

    *out_len = p - out_buf;
    Serial.printf("[CTAP2] makeCredential success for RP: %.*s\n", (int)rp_id_len, rp_id);
    return CTAP2_OK;
}

// ─── getAssertion ─────────────────────────────────────────────────────────────
uint8_t ctap2_get_assertion(const uint8_t *cbor, size_t len,
                             uint8_t *out_buf, size_t *out_len) {

    // ★ BLE CONTEXT CHECK ★ 
    // TODO come back and uncomment for real thing
    // This is the key innovation: refuse to sign if phone isn't BLE-connected
    // if (!ble_context_is_phone_connected()) {
    //     Serial.printf(TAG, "getAssertion BLOCKED: phone not in BLE range");
    //     out_buf[0] = CTAP2_ERR_OPERATION_DENIED;
    //     *out_len = 1;
    //     return CTAP2_ERR_OPERATION_DENIED;
    // }
    ensure_ble_prefs();
    bool trusted = ble_prefs.getBool("t_connected", false);
    bool connected = ble_prefs.getBool("has_trusted", false);
    if (!connected || !trusted) {
        return CTAP2_ERR_OPERATION_DENIED;
    }

    

    const uint8_t *client_data_hash = NULL;
    size_t cdh_len = 0;
    const uint8_t *rp_id = NULL;
    size_t rp_id_len = 0;
    const uint8_t *allow_credential_id = NULL;
    size_t allow_cred_id_len = 0;

    if ((cbor[0] & 0xE0) != 0xA0) return CTAP2_ERR_CBOR_UNEXPECTED_TYPE;
    uint8_t map_len = cbor[0] & 0x1F;
    size_t pos = 1;

    for (int i = 0; i < map_len && pos < len; i++) {
        size_t consumed;
        uint8_t key = cbor_read_uint_key(cbor + pos, &consumed);
        pos += consumed;

        switch (key) {
            case 1: { // rpId
                rp_id = cbor_read_text(cbor + pos, &rp_id_len, &consumed);
                pos += consumed;
                break;
            }
            case 2: { // clientDataHash
                client_data_hash = cbor_read_bytes(cbor + pos, &cdh_len, &consumed);
                pos += consumed;
                break;
            }
            case 3: { // allowList - array of PublicKeyCredentialDescriptors
                if ((cbor[pos] & 0xE0) == 0x80) {
                    uint8_t arr_len = cbor[pos] & 0x1F;
                    pos++;
                    for (int j = 0; j < arr_len; j++) {
                        // Each entry is map { type: "public-key", id: bytes }
                        if ((cbor[pos] & 0xE0) == 0xA0) {
                            uint8_t entry_map_len = cbor[pos] & 0x1F;
                            pos++;
                            for (int k = 0; k < entry_map_len; k++) {
                                size_t key_len;
                                const uint8_t *mk = cbor_read_text(cbor + pos, &key_len, &consumed);
                                pos += consumed;
                                if (key_len == 2 && memcmp(mk, "id", 2) == 0) {
                                    allow_credential_id = cbor_read_bytes(cbor + pos, &allow_cred_id_len, &consumed);
                                    pos += consumed;
                                } else {
                                    pos += cbor_skip(cbor + pos, len - pos);
                                }
                            }
                        } else {
                            pos += cbor_skip(cbor + pos, len - pos);
                        }
                    }
                } else {
                    pos += cbor_skip(cbor + pos, len - pos);
                }
                break;
            }
            default:
                pos += cbor_skip(cbor + pos, len - pos);
                break;
        }
    }

    if (!client_data_hash || !rp_id || cdh_len != 32) {
        return CTAP2_ERR_MISSING_PARAMETER;
    }

    uint8_t rp_id_hash[32];
    if (!crypto_sha256(rp_id, rp_id_len, rp_id_hash)) return CTAP2_ERR_OTHER;

    // Find matching credential
    fido2_credential_t cred;
    bool found = false;

    if (allow_credential_id && allow_cred_id_len == CREDENTIAL_ID_LEN) {
        found = storage_find_credential(rp_id_hash, allow_credential_id, &cred);
    } else {
        // No allowList - find first credential for this RP
        found = storage_find_credentials_for_rp(rp_id_hash, &cred, 1) > 0;
    }

    if (!found) {
        Serial.printf(TAG, "No credential found for RP: %.*s", (int)rp_id_len, rp_id);
        return CTAP2_ERR_NO_CREDENTIALS;
    }

    // Wait for user presence
    if (!ctap2_wait_for_user_presence(10000)) {
        return CTAP2_ERR_ACTION_TIMEOUT;
    }

    cred.sign_count++;
    storage_update_sign_count(cred.id, cred.sign_count);

    // Build authenticator data (no attested cred data for assertions)
    uint8_t auth_data[37];  // exactly 37 bytes: 32 + 1 + 4
    size_t auth_data_len = crypto_build_auth_data(
        rp_id_hash, true, false, false,
        cred.sign_count, NULL, NULL, NULL,
        auth_data, sizeof(auth_data)
    );

    // Sign: authenticatorData || clientDataHash
    uint8_t to_sign[37 + 32];
    memcpy(to_sign, auth_data, auth_data_len);
    memcpy(to_sign + auth_data_len, client_data_hash, 32);

    uint8_t sig[72];
    size_t sig_len = 0;
    if (!crypto_sign(cred.private_key, to_sign, auth_data_len + 32, sig, &sig_len)) {
        return CTAP2_ERR_OTHER;
    }

    // Build CBOR response
    // { 1: credential, 2: authData, 3: signature, 4: user }
    uint8_t *p = out_buf;
    *p++ = CTAP2_OK;
    *p++ = 0xA4;  // map(4)

    // 1: credential { type: "public-key", id: bytes }
    p += cbor_write_uint(p, 1);
    *p++ = 0xA2;

    p += cbor_write_text(p, "id");
    p += cbor_write_bytes(p, cred.id, CREDENTIAL_ID_LEN);
    p += cbor_write_text(p, "type");
    p += cbor_write_text(p, "public-key");

    // 2: authData
    p += cbor_write_uint(p, 2);
    p += cbor_write_bytes(p, auth_data, auth_data_len);

    // 3: signature
    p += cbor_write_uint(p, 3);
    p += cbor_write_bytes(p, sig, sig_len);

    // 4: user { id: bytes, name: text }
    p += cbor_write_uint(p, 4);
    *p++ = 0xA2;
    p += cbor_write_text(p, "id");
    p += cbor_write_bytes(p, cred.user_id, cred.user_id_len);
    p += cbor_write_text(p, "name");
    p += cbor_write_text(p, cred.user_name);

    *out_len = p - out_buf;
    Serial.printf("getAssertion success, sign_count=%lu", (unsigned long)cred.sign_count);
    return CTAP2_OK;
}

// ─── Dispatcher ───────────────────────────────────────────────────────────────
uint8_t ctap2_process(const uint8_t *in_buf, size_t in_len,
                       uint8_t *out_buf, size_t *out_len) {
    if (in_len == 0) {
        out_buf[0] = CTAP2_ERR_INVALID_LENGTH;
        *out_len = 1;
        return CTAP2_ERR_INVALID_LENGTH;
    }

    uint8_t cmd = in_buf[0];
    Serial.printf("CTAP2 command: 0x%02x", cmd);

    switch (cmd) {
        case CTAP2_CMD_GET_INFO:
            return ctap2_get_info(out_buf, out_len);

        case CTAP2_CMD_MAKE_CREDENTIAL:
            return ctap2_make_credential(in_buf + 1, in_len - 1, out_buf, out_len);

        case CTAP2_CMD_GET_ASSERTION:
            return ctap2_get_assertion(in_buf + 1, in_len - 1, out_buf, out_len);

        case CTAP2_CMD_RESET:
            storage_reset();
            out_buf[0] = CTAP2_OK;
            *out_len = 1;
            return CTAP2_OK;

        default:
            Serial.printf("Unknown command: 0x%02x", cmd);
            out_buf[0] = CTAP2_ERR_INVALID_COMMAND;
            *out_len = 1;
            return CTAP2_ERR_INVALID_COMMAND;
    }
}
