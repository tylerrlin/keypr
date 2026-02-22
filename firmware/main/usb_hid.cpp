// #include "usb_hid.h"
// #include "ctap2_types.h"
// #include "ctap2.h"
// #include "USB.h"
// #include "USBHID.h"
// #include "tusb.h"
// #include <Arduino.h>
// #include <string.h>

// USBHID HID;

// // ─── FIDO2 HID Report Descriptor ─────────────────────────────────────────────
// static const uint8_t fido2_hid_descriptor[] = {
//     0x06, 0xD0, 0xF1,  // Usage Page (FIDO Alliance)
//     0x09, 0x01,        // Usage (U2F Authenticator Device)
//     0xA1, 0x01,        // Collection (Application)
//     0x09, 0x20,        //   Usage (Input Report Data)
//     0x15, 0x00,        //   Logical Minimum (0)
//     0x26, 0xFF, 0x00,  //   Logical Maximum (255)
//     0x75, 0x08,        //   Report Size (8)
//     0x95, 0x40,        //   Report Count (64)
//     0x81, 0x02,        //   Input (Data, Var, Abs)
//     0x09, 0x21,        //   Usage (Output Report Data)
//     0x15, 0x00,        //   Logical Minimum (0)
//     0x26, 0xFF, 0x00,  //   Logical Maximum (255)
//     0x75, 0x08,        //   Report Size (8)
//     0x95, 0x40,        //   Report Count (64)
//     0x91, 0x02,        //   Output (Data, Var, Abs)
//     0xC0               // End Collection
// };

// // ─── Packet Reassembly ────────────────────────────────────────────────────────
// #define MAX_PAYLOAD_LEN 1200

// static struct {
//     uint32_t cid;
//     uint8_t  cmd;
//     uint16_t total_len;
//     uint16_t received_len;
//     uint8_t  payload[MAX_PAYLOAD_LEN];
//     uint8_t  expected_seq;
//     bool     in_progress;
// } rx_state = {0};

// // Forward declaration
// static void handle_packet(const uint8_t *packet, size_t len);

// // ─── Custom HID device using built-in ESP32 Arduino USBHID ───────────────────
// class FIDO2HIDDevice : public USBHIDDevice {
// public:
//     FIDO2HIDDevice() {}

//     void begin() {
//         HID.addDevice(this, sizeof(fido2_hid_descriptor));
//     }

//     uint16_t _onGetDescriptor(uint8_t *dst) override {
//         memcpy(dst, fido2_hid_descriptor, sizeof(fido2_hid_descriptor));
//         return sizeof(fido2_hid_descriptor);
//     }

//     void _onOutput(uint8_t report_id, const uint8_t *data, uint16_t len) override {
//         (void)report_id;
//         handle_packet(data, len);
//     }


//     bool sendPacket(const uint8_t *data) {
//         // // Wait until USB is ready
//         uint32_t start = millis();
//         // while (!HID.ready()) {
//         //     delay(1);
//         //     if (millis() - start > 100) return false;
//         // }
//        // delay(10);

//         return HID.SendReport(0, data, HID_PACKET_SIZE);

//         //return HID.SendReport(0, data, HID_PACKET_SIZE);
//     }
// };

// static FIDO2HIDDevice fido2_hid;

// // ─── Send Helpers ─────────────────────────────────────────────────────────────
// static void send_packet(const uint8_t *data, size_t len) {
//     uint8_t packet[HID_PACKET_SIZE] = {0};
//     memcpy(packet, data, len < HID_PACKET_SIZE ? len : HID_PACKET_SIZE);
//     fido2_hid.sendPacket(packet);
// }

// void ctaphid_send_error(uint32_t cid, uint8_t err) {
//     uint8_t pkt[HID_PACKET_SIZE] = {0};
//     pkt[0] = (cid >> 24) & 0xFF;
//     pkt[1] = (cid >> 16) & 0xFF;
//     pkt[2] = (cid >>  8) & 0xFF;
//     pkt[3] = (cid      ) & 0xFF;
//     pkt[4] = CTAPHID_CMD_ERROR;
//     pkt[5] = 0x00;
//     pkt[6] = 0x01;
//     pkt[7] = err;
//     send_packet(pkt, HID_PACKET_SIZE);
// }

// void ctaphid_send_init_response(uint32_t cid, const uint8_t *nonce) {

//     uint8_t pkt[HID_PACKET_SIZE] = {0};

//     uint32_t new_cid = esp_random();

//     // RESPONSE HEADER CID MUST MATCH REQUEST CID
//     pkt[0] = (cid >> 24) & 0xFF;
//     pkt[1] = (cid >> 16) & 0xFF;
//     pkt[2] = (cid >> 8) & 0xFF;
//     pkt[3] = cid & 0xFF;

//     pkt[4] = CTAPHID_CMD_INIT;
//     pkt[5] = 0x00;
//     pkt[6] = 17;

//     memcpy(&pkt[7], nonce, 8);

//     pkt[15] = (new_cid >> 24) & 0xFF;
//     pkt[16] = (new_cid >> 16) & 0xFF;
//     pkt[17] = (new_cid >> 8) & 0xFF;
//     pkt[18] = new_cid & 0xFF;

//     pkt[19] = 2;
//     pkt[20] = 1;
//     pkt[21] = 0;
//     pkt[22] = 0;

//     pkt[23] = 0x04;

//     send_packet(pkt, HID_PACKET_SIZE);
// }

// // void ctaphid_send_init_response(uint32_t cid, const uint8_t *nonce) {
// //     uint8_t pkt[HID_PACKET_SIZE] = {0};
// //     uint32_t new_cid = (cid == CTAPHID_BROADCAST_CID) ? (uint32_t)esp_random() : cid;

// //     // Header ALWAYS uses broadcast CID for INIT response
// //     pkt[0] = 0xFF;
// //     pkt[1] = 0xFF;
// //     pkt[2] = 0xFF;
// //     pkt[3] = 0xFF;
// //     pkt[4] = CTAPHID_CMD_INIT;
// //     pkt[5] = 0x00;
// //     pkt[6] = 17;  // payload length
// //     // Payload:
// //     memcpy(&pkt[7], nonce, 8);           // echo nonce
// //     pkt[15] = (new_cid >> 24) & 0xFF;   // new CID
// //     pkt[16] = (new_cid >> 16) & 0xFF;
// //     pkt[17] = (new_cid >>  8) & 0xFF;
// //     pkt[18] = (new_cid      ) & 0xFF;
// //     pkt[19] = 2;     // CTAPHID protocol version
// //     pkt[20] = 1;     // major
// //     pkt[21] = 0;     // minor
// //     pkt[22] = 0;     // build
// //     pkt[23] = 0x04;  // capabilities: CBOR
// //     send_packet(pkt, HID_PACKET_SIZE);
// // }

// // void ctaphid_send_init_response(uint32_t cid, const uint8_t *nonce) {
// //     uint8_t pkt[HID_PACKET_SIZE] = {0};
// //     uint32_t new_cid = (cid == CTAPHID_BROADCAST_CID) ? (uint32_t)esp_random() : cid;

// //     pkt[0] = (new_cid >> 24) & 0xFF;
// //     pkt[1] = (new_cid >> 16) & 0xFF;
// //     pkt[2] = (new_cid >>  8) & 0xFF;
// //     pkt[3] = (new_cid      ) & 0xFF;
// //     pkt[4] = CTAPHID_CMD_INIT;
// //     pkt[5] = 0x00;
// //     pkt[6] = 17;
// //     memcpy(&pkt[7], nonce, 8);
// //     pkt[15] = (new_cid >> 24) & 0xFF;
// //     pkt[16] = (new_cid >> 16) & 0xFF;
// //     pkt[17] = (new_cid >>  8) & 0xFF;
// //     pkt[18] = (new_cid      ) & 0xFF;
// //     pkt[19] = 2;     // protocol version
// //     pkt[20] = 1;     // major
// //     pkt[21] = 0;     // minor
// //     pkt[22] = 0;     // build
// //     pkt[23] = 0x04;  // CBOR capability
// //     send_packet(pkt, HID_PACKET_SIZE);
// // }

// void ctaphid_send_cbor_response(uint32_t cid, const uint8_t *cbor_data, size_t cbor_len) {
//     uint8_t pkt[HID_PACKET_SIZE] = {0};
//     size_t offset = 0;
//     uint8_t seq = 0;

//     // Init packet
//     pkt[0] = (cid >> 24) & 0xFF;
//     pkt[1] = (cid >> 16) & 0xFF;
//     pkt[2] = (cid >>  8) & 0xFF;
//     pkt[3] = (cid      ) & 0xFF;
//     pkt[4] = CTAPHID_CMD_CBOR;
//     pkt[5] = (cbor_len >> 8) & 0xFF;
//     pkt[6] = cbor_len & 0xFF;

//     size_t first_chunk = cbor_len < (HID_PACKET_SIZE - 7)
//                        ? cbor_len : (HID_PACKET_SIZE - 7);
//     memcpy(&pkt[7], cbor_data, first_chunk);
//     send_packet(pkt, HID_PACKET_SIZE);
//     offset += first_chunk;

//     while (offset < cbor_len) {
//         memset(pkt, 0, HID_PACKET_SIZE);
//         pkt[0] = (cid >> 24) & 0xFF;
//         pkt[1] = (cid >> 16) & 0xFF;
//         pkt[2] = (cid >>  8) & 0xFF;
//         pkt[3] = (cid      ) & 0xFF;
//         pkt[4] = seq++ & 0x7F;

//         size_t chunk = (cbor_len - offset) < (HID_PACKET_SIZE - 5)
//                      ? (cbor_len - offset) : (HID_PACKET_SIZE - 5);
//         memcpy(&pkt[5], cbor_data + offset, chunk);
//         send_packet(pkt, HID_PACKET_SIZE);
//         offset += chunk;
//     }
// }

// // ─── Incoming Packet Handler ──────────────────────────────────────────────────
// static void handle_packet(const uint8_t *packet, size_t len) {
//     if (len < 4) return;

//     uint32_t cid = ((uint32_t)packet[0] << 24) |
//                    ((uint32_t)packet[1] << 16) |
//                    ((uint32_t)packet[2] <<  8) |
//                    ((uint32_t)packet[3]);

//     if (packet[4] & 0x80) {
//         uint8_t  cmd  = packet[4];
//         uint16_t bcnt = ((uint16_t)packet[5] << 8) | packet[6];

//         if (cmd == CTAPHID_CMD_INIT) {
//             ctaphid_send_init_response(cid, &packet[7]);
//             return;
//         }
//         if (cmd == CTAPHID_CMD_PING) {
//             ctaphid_send_cbor_response(cid, &packet[7], bcnt);
//             return;
//         }
//         if (cmd == CTAPHID_CMD_WINK) {
//             uint8_t pkt[HID_PACKET_SIZE] = {0};
//             pkt[0]=(cid>>24)&0xFF; pkt[1]=(cid>>16)&0xFF;
//             pkt[2]=(cid>> 8)&0xFF; pkt[3]=(cid    )&0xFF;
//             pkt[4] = CTAPHID_CMD_WINK;
//             send_packet(pkt, HID_PACKET_SIZE);
//             return;
//         }
//         if (cmd == CTAPHID_CMD_CBOR) {
//             rx_state.cid = cid;
//             rx_state.cmd = cmd;
//             rx_state.total_len = bcnt;
//             rx_state.received_len = 0;
//             rx_state.expected_seq = 0;
//             rx_state.in_progress = true;

//             size_t copy = bcnt < (HID_PACKET_SIZE - 7)
//                         ? bcnt : (HID_PACKET_SIZE - 7);
//             memcpy(rx_state.payload, &packet[7], copy);
//             rx_state.received_len = copy;

//             if (rx_state.received_len >= rx_state.total_len) {
//                 rx_state.in_progress = false;
//                 goto process_ctap;
//             }
//             return;
//         }
//         ctaphid_send_error(cid, CTAP2_ERR_INVALID_COMMAND);
//         return;
//     }

//     // Continuation packet
//     if (rx_state.in_progress && cid == rx_state.cid) {
//         uint8_t seq = packet[4] & 0x7F;
//         if (seq != rx_state.expected_seq) {
//             ctaphid_send_error(cid, CTAP2_ERR_INVALID_SEQ);
//             rx_state.in_progress = false;
//             return;
//         }
//         rx_state.expected_seq++;

//         size_t remaining = rx_state.total_len - rx_state.received_len;
//         size_t copy = remaining < (HID_PACKET_SIZE - 5)
//                     ? remaining : (HID_PACKET_SIZE - 5);
//         memcpy(rx_state.payload + rx_state.received_len, &packet[5], copy);
//         rx_state.received_len += copy;

//         if (rx_state.received_len >= rx_state.total_len) {
//             rx_state.in_progress = false;
//             goto process_ctap;
//         }
//         return;
//     }
//     return;

// process_ctap:
//     uint8_t out_buf[1024] = {0};
//     size_t  out_len = sizeof(out_buf);
//     ctap2_process(rx_state.payload, rx_state.total_len, out_buf, &out_len);
//     ctaphid_send_cbor_response(rx_state.cid, out_buf, out_len);
// }

// void fido2_hid_early_init(void) {
//     USB.manufacturerName("HackAuth");
//     USB.productName("ESP32S3 FIDO2 Key");
//     fido2_hid.begin();  // registers descriptor with HID stack
//     HID.begin();
//     USB.begin();
// }

// // ─── Init ─────────────────────────────────────────────────────────────────────
// void usb_hid_init(void) {
//     USB.manufacturerName("HackAuth");
//     USB.productName("ESP32S3 FIDO2 Key");

//     fido2_hid.begin();
//     HID.begin();
//     USB.begin();

//     Serial.println("[USB] FIDO2 HID initialized");
// }

// void usb_hid_task(void) {
//     if (tud_hid_ready()) {
//         // nothing to do on send side
//     }
    
//     // Read raw from the HID OUT endpoint directly
//     uint8_t buf[64];
//     uint32_t len = tud_vendor_read(buf, sizeof(buf));
//     if (len > 0) {
//         handle_packet(buf, len);
//     }
//     //static uint8_t buf[64];
//     // tud_task() ensures TinyUSB processes pending events
//     //tud_task();
//     // Callbacks handle everything, nothing to poll
// }

#include "usb_hid.h"
#include "ctap2_types.h"
#include "ctap2.h"
#include "USB.h"
#include "USBHID.h"
#include <Arduino.h>
#include <string.h>

USBHID HID;

static uint8_t pending_packet[HID_PACKET_SIZE];
static volatile bool packet_pending = false;

// ─── NeoPixel for debug feedback ──────────────────────────────────────────────

// ─── FIDO2 HID Report Descriptor ─────────────────────────────────────────────
// Per FIDO HID Protocol Specification §4
static const uint8_t fido2_hid_descriptor[] = {
    0x06, 0xD0, 0xF1,  // Usage Page (FIDO Alliance, 0xF1D0)
    0x09, 0x01,        // Usage (FIDO Authenticator)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x20,        //   Usage (Input Report Data)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8 bits)
    0x95, 0x40,        //   Report Count (64 bytes)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    0x09, 0x21,        //   Usage (Output Report Data)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8 bits)
    0x95, 0x40,        //   Report Count (64 bytes)
    0x91, 0x02,        //   Output (Data, Variable, Absolute)
    0xC0               // End Collection
};

// ─── Packet Reassembly State ──────────────────────────────────────────────────
#define MAX_PAYLOAD_LEN 1200

static struct {
    uint32_t cid;
    uint8_t  cmd;
    uint16_t total_len;
    uint16_t received_len;
    uint8_t  payload[MAX_PAYLOAD_LEN];
    uint8_t  expected_seq;
    bool     in_progress;
} rx_state = {0};

static void handle_packet(const uint8_t *packet, size_t len);

// ─── FIDO2 HID Device Class ───────────────────────────────────────────────────
class FIDO2HIDDevice : public USBHIDDevice {
public:
    FIDO2HIDDevice() {}

    void begin() {
        HID.addDevice(this, sizeof(fido2_hid_descriptor));
    }

    uint16_t _onGetDescriptor(uint8_t *dst) override {
        memcpy(dst, fido2_hid_descriptor, sizeof(fido2_hid_descriptor));
        return sizeof(fido2_hid_descriptor);
    }

    void _onOutput(uint8_t report_id, const uint8_t *data, uint16_t len) override {
    (void)report_id;
    if (!packet_pending) {
        memcpy(pending_packet, data, len < HID_PACKET_SIZE ? len : HID_PACKET_SIZE);
        packet_pending = true;
    }
    }

    bool sendPacket(const uint8_t *data) {
        uint32_t start = millis();
        while (!HID.ready()) {   // <-- HID.ready(), not tud_hid_ready()
            if (millis() - start > 200) return false;
            delay(1);
        }
        return HID.SendReport(0, data, HID_PACKET_SIZE);
    }
};

static FIDO2HIDDevice fido2_hid;

// ─── Send Helpers ─────────────────────────────────────────────────────────────
static void send_packet(const uint8_t *data, size_t len) {
    uint8_t packet[HID_PACKET_SIZE] = {0};
    memcpy(packet, data, len < HID_PACKET_SIZE ? len : HID_PACKET_SIZE);
    fido2_hid.sendPacket(packet);
}

void ctaphid_send_error(uint32_t cid, uint8_t err) {
    uint8_t pkt[HID_PACKET_SIZE] = {0};
    pkt[0] = (cid >> 24) & 0xFF;
    pkt[1] = (cid >> 16) & 0xFF;
    pkt[2] = (cid >>  8) & 0xFF;
    pkt[3] = (cid      ) & 0xFF;
    pkt[4] = CTAPHID_CMD_ERROR;
    pkt[5] = 0x00;
    pkt[6] = 0x01;
    pkt[7] = err;
    send_packet(pkt, HID_PACKET_SIZE);
}

void ctaphid_send_init_response(uint32_t cid, const uint8_t *nonce) {
    uint8_t pkt[HID_PACKET_SIZE] = {0};
    // Always generate a new CID for the channel
    uint32_t new_cid = esp_random();
    // Ensure new_cid is never 0 or broadcast
    while (new_cid == 0 || new_cid == 0xFFFFFFFF) new_cid = esp_random();

    // Response header: echo back the SAME cid that was in the request
    // (per CTAPHID spec section 11.2.9.1.3)
    pkt[0] = (cid >> 24) & 0xFF;
    pkt[1] = (cid >> 16) & 0xFF;
    pkt[2] = (cid >>  8) & 0xFF;
    pkt[3] = (cid      ) & 0xFF;
    pkt[4] = CTAPHID_CMD_INIT;   // 0x86
    pkt[5] = 0x00;               // length high byte
    pkt[6] = 17;                 // length low byte (17 byte payload)
    // Payload bytes 7-23:
    memcpy(&pkt[7], nonce, 8);   // [7-14]  echo nonce
    pkt[15] = (new_cid >> 24) & 0xFF;  // [15-18] allocated channel ID
    pkt[16] = (new_cid >> 16) & 0xFF;
    pkt[17] = (new_cid >>  8) & 0xFF;
    pkt[18] = (new_cid      ) & 0xFF;
    pkt[19] = 2;     // [19] CTAPHID protocol version
    pkt[20] = 1;     // [20] major device version
    pkt[21] = 0;     // [21] minor device version
    pkt[22] = 0;     // [22] build device version
    pkt[23] = 0x04;  // [23] capabilities: CBOR (0x04)

    send_packet(pkt, HID_PACKET_SIZE);
}

void ctaphid_send_cbor_response(uint32_t cid, const uint8_t *cbor_data, size_t cbor_len) {
    uint8_t pkt[HID_PACKET_SIZE] = {0};
    size_t offset = 0;
    uint8_t seq = 0;

    // First (init) packet
    pkt[0] = (cid >> 24) & 0xFF;
    pkt[1] = (cid >> 16) & 0xFF;
    pkt[2] = (cid >>  8) & 0xFF;
    pkt[3] = (cid      ) & 0xFF;
    pkt[4] = CTAPHID_CMD_CBOR;
    pkt[5] = (cbor_len >> 8) & 0xFF;
    pkt[6] = cbor_len & 0xFF;

    size_t first_chunk = cbor_len < (HID_PACKET_SIZE - 7)
                       ? cbor_len : (HID_PACKET_SIZE - 7);
    memcpy(&pkt[7], cbor_data, first_chunk);
    send_packet(pkt, HID_PACKET_SIZE);
    offset += first_chunk;

    // Continuation packets
    while (offset < cbor_len) {
        memset(pkt, 0, HID_PACKET_SIZE);
        pkt[0] = (cid >> 24) & 0xFF;
        pkt[1] = (cid >> 16) & 0xFF;
        pkt[2] = (cid >>  8) & 0xFF;
        pkt[3] = (cid      ) & 0xFF;
        pkt[4] = seq++ & 0x7F;  // sequence number, MSB=0

        size_t chunk = (cbor_len - offset) < (HID_PACKET_SIZE - 5)
                     ? (cbor_len - offset) : (HID_PACKET_SIZE - 5);
        memcpy(&pkt[5], cbor_data + offset, chunk);
        send_packet(pkt, HID_PACKET_SIZE);
        offset += chunk;
    }
}

// ─── Incoming Packet Handler ──────────────────────────────────────────────────
static void handle_packet(const uint8_t *packet, size_t len) {
    if (len < 5) return;

    uint32_t cid = ((uint32_t)packet[0] << 24) |
                   ((uint32_t)packet[1] << 16) |
                   ((uint32_t)packet[2] <<  8) |
                   ((uint32_t)packet[3]);

    // Initialization packet (MSB of cmd byte is 1)
    if (packet[4] & 0x80) {
        uint8_t  cmd  = packet[4];
        uint16_t bcnt = ((uint16_t)packet[5] << 8) | packet[6];

        if (cmd == CTAPHID_CMD_INIT) {
            ctaphid_send_init_response(cid, &packet[7]);
            return;
        }
        if (cmd == CTAPHID_CMD_PING) {
            // Echo back
            ctaphid_send_cbor_response(cid, &packet[7], bcnt);
            return;
        }
        if (cmd == CTAPHID_CMD_WINK) {
            uint8_t pkt[HID_PACKET_SIZE] = {0};
            pkt[0]=(cid>>24)&0xFF; pkt[1]=(cid>>16)&0xFF;
            pkt[2]=(cid>> 8)&0xFF; pkt[3]=(cid    )&0xFF;
            pkt[4] = CTAPHID_CMD_WINK;
            pkt[5] = 0; pkt[6] = 0;
            send_packet(pkt, HID_PACKET_SIZE);
            return;
        }
        if (cmd == CTAPHID_CMD_CBOR) {
            rx_state.cid = cid;
            rx_state.cmd = cmd;
            rx_state.total_len = bcnt;
            rx_state.received_len = 0;
            rx_state.expected_seq = 0;
            rx_state.in_progress = true;

            size_t copy = bcnt < (HID_PACKET_SIZE - 7)
                        ? bcnt : (HID_PACKET_SIZE - 7);
            memcpy(rx_state.payload, &packet[7], copy);
            rx_state.received_len = copy;

            if (rx_state.received_len >= rx_state.total_len) {
                rx_state.in_progress = false;
                goto process_ctap;
            }
            return;
        }
        // Unknown command
        ctaphid_send_error(cid, CTAP2_ERR_INVALID_COMMAND);
        return;
    }

    // Continuation packet (MSB of seq byte is 0)
    if (rx_state.in_progress && cid == rx_state.cid) {
        uint8_t seq = packet[4] & 0x7F;
        if (seq != rx_state.expected_seq) {
            ctaphid_send_error(cid, CTAP2_ERR_INVALID_SEQ);
            rx_state.in_progress = false;
            return;
        }
        rx_state.expected_seq++;

        size_t remaining = rx_state.total_len - rx_state.received_len;
        size_t copy = remaining < (HID_PACKET_SIZE - 5)
                    ? remaining : (HID_PACKET_SIZE - 5);
        memcpy(rx_state.payload + rx_state.received_len, &packet[5], copy);
        rx_state.received_len += copy;

        if (rx_state.received_len >= rx_state.total_len) {
            rx_state.in_progress = false;
            goto process_ctap;
        }
        return;
    }
    return;

process_ctap:
    uint8_t out_buf[1024] = {0};
    size_t  out_len = sizeof(out_buf);
    uint8_t result = ctap2_process(rx_state.payload, rx_state.total_len, out_buf, &out_len);
    if (result != CTAP2_OK || out_len == 0) {
        ctaphid_send_error(rx_state.cid, result ? result : CTAP2_ERR_OTHER);
        return;
    }
    ctaphid_send_cbor_response(rx_state.cid, out_buf, out_len);
}

// ─── Init ─────────────────────────────────────────────────────────────────────
void fido2_hid_early_init(void) {
    // NeoPixel must init early too
    USB.manufacturerName("HackAuth");
    USB.productName("ESP32S3 FIDO2 Key");
    fido2_hid.begin();
    HID.begin();
    USB.begin();
}

void usb_hid_init(void) {
    Serial.println("[USB] FIDO2 HID initialized");
}

void usb_hid_task(void) {
    // Arduino ESP32 core 3.x handles USB events in background RTOS task
    // _onOutput callback fires automatically when host sends data
    // Nothing to poll here
    if (packet_pending) {
        packet_pending = false;
        handle_packet(pending_packet, HID_PACKET_SIZE);
    }
}