# ESP32-S3 FIDO2 Hardware Security Key

Context-aware FIDO2 hardware key: only signs auth challenges when your phone
is actively BLE-connected. Walk away from your desk → auth is physically blocked.

## Hardware

- **ESP32-S3 DevKit** (any variant with native USB, e.g. ESP32-S3-DevKitC-1)
- Tactile button between **GPIO0** and **GND** (user presence / touch confirmation)
- LED on **GPIO2** (built-in on most devkits)

```
Button wiring:
  GPIO0 ──┤ BTN ├── GND

LED (usually built-in):
  GPIO2 ── LED ── 330Ω ── GND
```

## USB Wiring — CRITICAL

The ESP32-S3 has two USB ports:
- **USB-UART** (for flashing, serial monitor) → connect to your computer for development
- **USB-OTG** (native USB) → this is what presents as the FIDO2 security key

When demoing: plug the **USB-OTG port** into the target computer (Google/GitHub machine).
Keep the **USB-UART port** connected to your dev machine for serial logs.

On most DevKitC-1 boards:
- USB-UART = the port labeled "COM" or near the reset button
- USB-OTG = the other USB port (labeled "USB")

## Setup

### 1. Install PlatformIO

```bash
pip install platformio
# or use the VS Code extension
```

### 2. Build & Flash

```bash
cd esp32s3-fido2
pio run --target upload
pio device monitor  # watch serial output
```

### 3. Connect Your Phone via BLE

On Android: open any BLE scanner app (e.g. "nRF Connect") and connect to **FIDO2-AuthNode**.
Or build the companion web page (see below) for a cleaner demo.

LED will turn **solid ON** when phone is connected.

### 4. Register as Security Key

1. Phone BLE connected → LED on
2. Plug USB-OTG into computer
3. Go to github.com → Settings → Password and Authentication → Add a security key
4. Chrome will detect the device as a FIDO2 security key
5. Press the button when prompted for user presence
6. Done — credential stored in ESP32 NVS flash

### 5. Using for Login

1. Connect phone BLE first (LED on)
2. Go to GitHub login → use security key
3. Press button on ESP32 when prompted
4. Authenticated ✓

If phone is NOT connected → auth returns `CTAP2_ERR_OPERATION_DENIED` → login fails.

## Companion Phone Page (Web Bluetooth)

Save this HTML on your phone or host it locally. It maintains the BLE connection:

```html
<!DOCTYPE html>
<html>
<head><title>FIDO2 Auth Context</title></head>
<body>
  <h2>FIDO2 Auth Node</h2>
  <p id="status">Disconnected</p>
  <button onclick="connect()">Connect to Auth Node</button>
  <script>
    let device;
    async function connect() {
      device = await navigator.bluetooth.requestDevice({
        filters: [{ name: 'FIDO2-AuthNode' }],
        optionalServices: ['4fafc201-1fb5-459e-8fcc-c5c9c331914b']
      });
      device.addEventListener('gattserverdisconnected', () => {
        document.getElementById('status').textContent = '🔴 Disconnected';
        document.getElementById('status').style.color = 'red';
      });
      await device.gatt.connect();
      document.getElementById('status').textContent = '🟢 Connected — Auth Armed';
      document.getElementById('status').style.color = 'green';
    }
  </script>
</body>
</html>
```

## Architecture

```
[GitHub / Google login page]
         │
         │ WebAuthn / CTAP2 over USB HID
         ▼
[ESP32-S3 USB-OTG port]
         │
         ├─ Check: is phone BLE-connected?
         │         ↓ NO  → CTAP2_ERR_OPERATION_DENIED
         │         ↓ YES → continue
         │
         ├─ Wait for button press (user presence)
         │
         ├─ Sign challenge with stored P-256 key (mbedTLS)
         │
         └─ Return CTAP2 assertion → login succeeds

[Phone] ──BLE──► [ESP32-S3 BLE peripheral]
                  (NimBLE, advertising as FIDO2-AuthNode)
```

## Important Limitations for Hackathon

- Keys stored in **NVS flash** (not a secure element) — not suitable for production
- No PIN support (CTAP2 client PIN not implemented)
- No attestation (uses "none" attestation format) — Google/GitHub accept this for regular 2FA
- Works with **GitHub** and **Google regular 2FA** (not Advanced Protection Program)
- CBOR parsing is minimal — works for standard flows, edge cases may fail

## Testing Without a Real Site

Use the python-fido2 library to test your key:

```bash
pip install fido2
python3 -c "
from fido2.hid import CtapHidDevice
devs = list(CtapHidDevice.list_devices())
print('Found devices:', devs)
"
```

If your ESP32 shows up in that list, the USB HID layer is working correctly.
