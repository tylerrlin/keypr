#include "usb_hid.h"
#include "ctap2_types.h"
#include "ctap2.h"
#include "USB.h"
#include "USBHID.h"
#include <Arduino.h>
#include <string.h>

USBHID HID;

// ─── FIDO2 HID Report Descriptor ─────────────────────────────────────────────
static const uint8_t fido2_hid_descriptor[] = {
    0x06, 0xD0, 0xF1,  // Usage Page (FIDO Alliance)
    0x09, 0x01,        // Usage (U2F Authenticator Device)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x20,        //   Usage (Input Report Data)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x40,        //   Report Count (64)
    0x81, 0x02,        //   Input (Data, Var, Abs)
    0x09, 0x21,        //   Usage (Output Report Data)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x40,        //   Report Count (64)
    0x91, 0x02,        //   Output (Data, Var, Abs)
    0xC0               // End Collection
};

// ─── Packet Reassembly ────────────────────────────────────────────────────────
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

// Forward declaration
static void handle_packet(const uint8_t *packet, size_t len);

// ─── Custom HID device using built-in ESP32 Arduino USBHID ───────────────────
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
        handle_packet(data, len);
    }

    bool sendPacket(const uint8_t *data) {
        // Wait until USB is ready
        uint32_t start = millis();
        while (!HID.ready()) {
            delay(1);
            if (millis() - start > 100) return false;
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
    uint32_t new_cid = (cid == CTAPHID_BROADCAST_CID) ? (uint32_t)esp_random() : cid;

    pkt[0] = (new_cid >> 24) & 0xFF;
    pkt[1] = (new_cid >> 16) & 0xFF;
    pkt[2] = (new_cid >>  8) & 0xFF;
    pkt[3] = (new_cid      ) & 0xFF;
    pkt[4] = CTAPHID_CMD_INIT;
    pkt[5] = 0x00;
    pkt[6] = 17;
    memcpy(&pkt[7], nonce, 8);
    pkt[15] = (new_cid >> 24) & 0xFF;
    pkt[16] = (new_cid >> 16) & 0xFF;
    pkt[17] = (new_cid >>  8) & 0xFF;
    pkt[18] = (new_cid      ) & 0xFF;
    pkt[19] = 2;     // protocol version
    pkt[20] = 1;     // major
    pkt[21] = 0;     // minor
    pkt[22] = 0;     // build
    pkt[23] = 0x04;  // CBOR capability
    send_packet(pkt, HID_PACKET_SIZE);
}

void ctaphid_send_cbor_response(uint32_t cid, const uint8_t *cbor_data, size_t cbor_len) {
    uint8_t pkt[HID_PACKET_SIZE] = {0};
    size_t offset = 0;
    uint8_t seq = 0;

    // Init packet
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

    while (offset < cbor_len) {
        memset(pkt, 0, HID_PACKET_SIZE);
        pkt[0] = (cid >> 24) & 0xFF;
        pkt[1] = (cid >> 16) & 0xFF;
        pkt[2] = (cid >>  8) & 0xFF;
        pkt[3] = (cid      ) & 0xFF;
        pkt[4] = seq++ & 0x7F;

        size_t chunk = (cbor_len - offset) < (HID_PACKET_SIZE - 5)
                     ? (cbor_len - offset) : (HID_PACKET_SIZE - 5);
        memcpy(&pkt[5], cbor_data + offset, chunk);
        send_packet(pkt, HID_PACKET_SIZE);
        offset += chunk;
    }
}

// ─── Incoming Packet Handler ──────────────────────────────────────────────────
static void handle_packet(const uint8_t *packet, size_t len) {
    if (len < 4) return;

    uint32_t cid = ((uint32_t)packet[0] << 24) |
                   ((uint32_t)packet[1] << 16) |
                   ((uint32_t)packet[2] <<  8) |
                   ((uint32_t)packet[3]);

    if (packet[4] & 0x80) {
        uint8_t  cmd  = packet[4];
        uint16_t bcnt = ((uint16_t)packet[5] << 8) | packet[6];

        if (cmd == CTAPHID_CMD_INIT) {
            ctaphid_send_init_response(cid, &packet[7]);
            return;
        }
        if (cmd == CTAPHID_CMD_PING) {
            ctaphid_send_cbor_response(cid, &packet[7], bcnt);
            return;
        }
        if (cmd == CTAPHID_CMD_WINK) {
            uint8_t pkt[HID_PACKET_SIZE] = {0};
            pkt[0]=(cid>>24)&0xFF; pkt[1]=(cid>>16)&0xFF;
            pkt[2]=(cid>> 8)&0xFF; pkt[3]=(cid    )&0xFF;
            pkt[4] = CTAPHID_CMD_WINK;
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
        ctaphid_send_error(cid, CTAP2_ERR_INVALID_COMMAND);
        return;
    }

    // Continuation packet
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
    ctap2_process(rx_state.payload, rx_state.total_len, out_buf, &out_len);
    ctaphid_send_cbor_response(rx_state.cid, out_buf, out_len);
}

// ─── Init ─────────────────────────────────────────────────────────────────────
void usb_hid_init(void) {
    USB.manufacturerName("HackAuth");
    USB.productName("ESP32S3 FIDO2 Key");

    fido2_hid.begin();
    HID.begin();
    USB.begin();

    Serial.println("[USB] FIDO2 HID initialized");
}

void usb_hid_task(void) {
    // Callbacks handle everything, nothing to poll
}