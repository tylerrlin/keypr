#include "storage.h"
#include "ctap2_types.h"
#include <string.h>
#include <Arduino.h>
#include <Preferences.h>  // Arduino ESP32 NVS wrapper - easier than raw NVS API

// Using Arduino's Preferences library instead of raw nvs.h
// It wraps NVS with a cleaner API and works identically under the hood

static Preferences prefs;
static bool initialized = false;

#define NAMESPACE "fido2"

bool storage_init(void) {
    initialized = prefs.begin(NAMESPACE, false);  // false = read/write
    if (!initialized) {
        Serial.println("[STORAGE] Failed to open NVS namespace");
        return false;
    }
    Serial.printf("[STORAGE] Ready, %d credentials stored\n", storage_count());
    return true;
}

static void make_key(int idx, char *key_buf) {
    snprintf(key_buf, 16, "cred_%02d", idx);
}

bool storage_store_credential(const fido2_credential_t *cred) {
    if (!initialized) return false;

    for (int i = 0; i < MAX_CREDENTIALS; i++) {
        char key[16];
        make_key(i, key);

        // Check if slot is free
        if (prefs.getBytesLength(key) == 0) {
            size_t written = prefs.putBytes(key, cred, sizeof(fido2_credential_t));
            Serial.printf("[STORAGE] Credential stored in slot %d\n", i);
            return written == sizeof(fido2_credential_t);
        }

        // Check if existing slot is invalid
        fido2_credential_t existing;
        prefs.getBytes(key, &existing, sizeof(existing));
        if (!existing.valid) {
            size_t written = prefs.putBytes(key, cred, sizeof(fido2_credential_t));
            return written == sizeof(fido2_credential_t);
        }
    }

    Serial.println("[STORAGE] Credential store full!");
    return false;
}

bool storage_find_credential(const uint8_t *rp_id_hash, const uint8_t *credential_id,
                              fido2_credential_t *cred_out) {
    if (!initialized) return false;

    for (int i = 0; i < MAX_CREDENTIALS; i++) {
        char key[16];
        make_key(i, key);
        if (prefs.getBytesLength(key) == 0) continue;

        fido2_credential_t cred;
        prefs.getBytes(key, &cred, sizeof(cred));

        if (cred.valid &&
            memcmp(cred.rp_id_hash, rp_id_hash, 32) == 0 &&
            memcmp(cred.id, credential_id, CREDENTIAL_ID_LEN) == 0) {
            memcpy(cred_out, &cred, sizeof(fido2_credential_t));
            return true;
        }
    }
    return false;
}

int storage_find_credentials_for_rp(const uint8_t *rp_id_hash,
                                     fido2_credential_t *creds_out, int max_creds) {
    if (!initialized) return 0;

    int found = 0;
    for (int i = 0; i < MAX_CREDENTIALS && found < max_creds; i++) {
        char key[16];
        make_key(i, key);
        if (prefs.getBytesLength(key) == 0) continue;

        fido2_credential_t cred;
        prefs.getBytes(key, &cred, sizeof(cred));

        if (cred.valid && memcmp(cred.rp_id_hash, rp_id_hash, 32) == 0) {
            memcpy(&creds_out[found++], &cred, sizeof(fido2_credential_t));
        }
    }
    return found;
}

bool storage_update_sign_count(const uint8_t *credential_id, uint32_t new_count) {
    if (!initialized) return false;

    for (int i = 0; i < MAX_CREDENTIALS; i++) {
        char key[16];
        make_key(i, key);
        if (prefs.getBytesLength(key) == 0) continue;

        fido2_credential_t cred;
        prefs.getBytes(key, &cred, sizeof(cred));

        if (cred.valid && memcmp(cred.id, credential_id, CREDENTIAL_ID_LEN) == 0) {
            cred.sign_count = new_count;
            prefs.putBytes(key, &cred, sizeof(cred));
            return true;
        }
    }
    return false;
}

bool storage_reset(void) {
    if (!initialized) return false;
    prefs.clear();
    Serial.println("[STORAGE] All credentials wiped");
    return true;
}

int storage_count(void) {
    if (!initialized) return 0;
    int count = 0;
    for (int i = 0; i < MAX_CREDENTIALS; i++) {
        char key[16];
        make_key(i, key);
        if (prefs.getBytesLength(key) == 0) continue;
        fido2_credential_t cred;
        prefs.getBytes(key, &cred, sizeof(cred));
        if (cred.valid) count++;
    }
    return count;
}
