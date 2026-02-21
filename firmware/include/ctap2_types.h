#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// ─── CTAP2 Command Codes ────────────────────────────────────────────────────
#define CTAP2_CMD_MAKE_CREDENTIAL    0x01
#define CTAP2_CMD_GET_ASSERTION      0x02
#define CTAP2_CMD_GET_INFO           0x04
#define CTAP2_CMD_CLIENT_PIN         0x06
#define CTAP2_CMD_RESET              0x07

// ─── CTAP2 Status Codes ─────────────────────────────────────────────────────
#define CTAP2_OK                     0x00
#define CTAP2_ERR_INVALID_COMMAND    0x01
#define CTAP2_ERR_INVALID_PARAMETER  0x02
#define CTAP2_ERR_INVALID_LENGTH     0x03
#define CTAP2_ERR_INVALID_SEQ        0x04
#define CTAP2_ERR_TIMEOUT            0x05
#define CTAP2_ERR_CHANNEL_BUSY       0x06
#define CTAP2_ERR_LOCK_REQUIRED      0x0A
#define CTAP2_ERR_INVALID_CHANNEL    0x0B
#define CTAP2_ERR_CBOR_UNEXPECTED_TYPE   0x11
#define CTAP2_ERR_INVALID_CBOR       0x12
#define CTAP2_ERR_MISSING_PARAMETER  0x14
#define CTAP2_ERR_LIMIT_EXCEEDED     0x15
#define CTAP2_ERR_UNSUPPORTED_EXTENSION  0x16
#define CTAP2_ERR_CREDENTIAL_EXCLUDED    0x19
#define CTAP2_ERR_PROCESSING         0x21
#define CTAP2_ERR_INVALID_CREDENTIAL 0x22
#define CTAP2_ERR_USER_ACTION_PENDING    0x23
#define CTAP2_ERR_OPERATION_PENDING  0x24
#define CTAP2_ERR_NO_OPERATIONS      0x25
#define CTAP2_ERR_UNSUPPORTED_ALGORITHM  0x26
#define CTAP2_ERR_OPERATION_DENIED   0x27  // <-- what we return when BLE not present
#define CTAP2_ERR_KEY_STORE_FULL     0x28
#define CTAP2_ERR_NOT_BUSY           0x29
#define CTAP2_ERR_NO_OPERATION_PENDING   0x2A
#define CTAP2_ERR_UNSUPPORTED_OPTION 0x2B
#define CTAP2_ERR_INVALID_OPTION     0x2C
#define CTAP2_ERR_KEEPALIVE_CANCEL   0x2D
#define CTAP2_ERR_NO_CREDENTIALS     0x2E
#define CTAP2_ERR_USER_ACTION_TIMEOUT    0x2F
#define CTAP2_ERR_NOT_ALLOWED        0x30
#define CTAP2_ERR_PIN_INVALID        0x31
#define CTAP2_ERR_PIN_BLOCKED        0x32
#define CTAP2_ERR_PIN_AUTH_INVALID   0x33
#define CTAP2_ERR_PIN_AUTH_BLOCKED   0x34
#define CTAP2_ERR_PIN_NOT_SET        0x35
#define CTAP2_ERR_PIN_REQUIRED       0x36
#define CTAP2_ERR_PIN_POLICY_VIOLATION   0x37
#define CTAP2_ERR_PIN_TOKEN_EXPIRED  0x38
#define CTAP2_ERR_REQUEST_TOO_LARGE  0x39
#define CTAP2_ERR_ACTION_TIMEOUT     0x3A
#define CTAP2_ERR_UP_REQUIRED        0x3B
#define CTAP2_ERR_OTHER              0x7F
#define CTAP2_ERR_SPEC_LAST          0xDF
#define CTAP2_ERR_EXTENSION_FIRST    0xE0
#define CTAP2_ERR_EXTENSION_LAST     0xEF
#define CTAP2_ERR_VENDOR_FIRST       0xF0
#define CTAP2_ERR_VENDOR_LAST        0xFF

// ─── HID/CTAPHID Constants ───────────────────────────────────────────────────
#define CTAPHID_BROADCAST_CID        0xFFFFFFFF
#define CTAPHID_CMD_MSG              0x83
#define CTAPHID_CMD_CBOR             0x90
#define CTAPHID_CMD_INIT             0x86
#define CTAPHID_CMD_PING             0x81
#define CTAPHID_CMD_CANCEL           0x91
#define CTAPHID_CMD_ERROR            0xBF
#define CTAPHID_CMD_KEEPALIVE        0xBB
#define CTAPHID_CMD_WINK             0x88
#define CTAPHID_CMD_LOCK             0x84

#define HID_PACKET_SIZE              64

// ─── Crypto Constants ────────────────────────────────────────────────────────
#define COSE_ALG_ES256               -7   // ECDSA w/ SHA-256 over P-256
#define CREDENTIAL_ID_LEN            32
#define PRIVATE_KEY_LEN              32
#define PUBLIC_KEY_LEN               64   // uncompressed, x+y
#define AAGUID_LEN                   16
#define MAX_CREDENTIALS              10
#define MAX_RP_ID_LEN                128
#define MAX_USER_ID_LEN              64
#define MAX_USER_NAME_LEN            64

// ─── Structs ─────────────────────────────────────────────────────────────────

typedef struct {
    uint8_t id[CREDENTIAL_ID_LEN];
    uint8_t private_key[PRIVATE_KEY_LEN];
    uint8_t rp_id_hash[32];
    uint8_t user_id[MAX_USER_ID_LEN];
    uint8_t user_id_len;
    char    user_name[MAX_USER_NAME_LEN];
    uint32_t sign_count;
    bool    valid;
} fido2_credential_t;

typedef struct {
    uint8_t  nonce[8];
    uint32_t cid;         // assigned channel ID
    uint8_t  protocol;    // CTAPHID protocol version = 2
    uint8_t  major;
    uint8_t  minor;
    uint8_t  build;
    uint8_t  caps;        // capability flags
} ctaphid_init_response_t;

typedef struct {
    uint32_t cid;
    uint8_t  cmd;
    uint16_t bcnt;        // total payload length
    uint8_t  data[HID_PACKET_SIZE - 7];
} ctaphid_init_packet_t;

typedef struct {
    uint32_t cid;
    uint8_t  seq;
    uint8_t  data[HID_PACKET_SIZE - 5];
} ctaphid_cont_packet_t;
