# Flashing the Keypr FIDO2 Passkey Firmware

This guide covers how to build and flash the modified pico-fido firmware onto an **ESP32-S3** board from a fresh machine on **macOS, Linux, or Windows**. The firmware has been patched to work as a PIN-less FIDO2 passkey (discoverable credential / resident key) compatible with Chrome and webauthn.io.

---

## What Was Changed

All changes are in `test_picofido/src/fido/`. The upstream source is [polhenarejos/pico-fido](https://github.com/polhenarejos/pico-fido).

### `cbor_get_info.c` — `cbor_get_info()`

| What                                                                    | Why                                                                                                                                                                                                               |
| ----------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `lfields` reduced from 15 → 14 (vendor cmds field `0x15` removed)       | `libfido2` and Chrome don't understand `0x15`; its presence caused `FIDO_ERR_RX` on every `getInfo` call, making the device invisible to all FIDO2 tooling                                                        |
| Added field `0x09` (`transports: ["usb"]`)                              | Without this, webauthn.io and Chrome do not show the "Use your security key" dialog                                                                                                                               |
| Options map reduced from 9 entries → 4 (`rk, up, uv, makeCredUvNotRqd`) | Removed `clientPin`, `alwaysUv`, `credMgmt`, `authnrCfg`, `largeBlobs`, `pinUvAuthToken`, `setMinPINLength`, `ep` — advertising `clientPin` caused Chrome to attempt PIN setup and block passkey registration     |
| Extensions array reduced from 7 → 2 (`hmac-secret, credProtect`)        | Removed `credBlob`, `largeBlobKey`, `minPinLength`, `hmac-secret-mc`, `thirdPartyPayment` — `credBlob` in particular caused Chrome's `ReadCTAPGetInfoResponse` to return `nullopt`, falling back to U2F-only mode |
| Versions array reduced from 4 → 3 (removed `FIDO_2_2`)                  | `FIDO_2_2` is not fully implemented; advertising it triggered unimplemented code paths                                                                                                                            |
| `makeCredUvNotRqd=true` added to options                                | Required by FIDO 2.1 §6.1.3 to allow credential creation without a PIN token                                                                                                                                      |

### `cbor_make_credential.c` — `cbor_make_credential()`

| What                                             | Why                                                                                                                                                                                 |
| ------------------------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Removed `CTAP2_ERR_PUAT_REQUIRED` block          | Original code returned `CTAP2_ERR_PUAT_REQUIRED` (0x36) when `pinUvAuthParam` was absent and a PIN had been set. Since we don't require a PIN, this blocked all credential creation |
| Removed `CTAP2_ERR_INVALID_OPTION` for `uv=true` | Chrome sends `uv=true` in `makeCredential` when the device advertises `uv=true` in getInfo. The original code rejected this                                                         |

### `cbor_get_assertion.c` — `cbor_get_assertion()`

| What                                             | Why                                                                                                                                        |
| ------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------ |
| Removed `CTAP2_ERR_INVALID_OPTION` for `uv=true` | Same as above — Chrome sends `uv=true` in assertions; original code rejected it. Now sets `FIDO2_AUT_FLAG_UV` in authenticatorData instead |

### `fido.c` — `scan_files_fido()`

| What                                                                                     | Why                                                                                                                                                                                                                                                                                                                                                                           |
| ---------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Added `load_keydev` probe on boot; wipes and regenerates `ef_keydev` if decryption fails | Flash contained stale PIN-encrypted `ef_keydev` from a previous firmware session. The original code only regenerated `ef_keydev` when completely empty — if it had data (even unreadable data), it skipped regeneration. Every subsequent crypto operation (`derive_key` → `load_keydev`) then failed with `PICOKEY_EXEC_ERROR`, returning `CTAP1_ERR_OTHER` (0x7f) to Chrome |

---

## Prerequisites

### macOS

```bash
# ESP-IDF
mkdir -p ~/esp && cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf && git checkout v5.5
./install.sh esp32s3

# libfido2 (optional, for testing)
brew install libfido2
```

### Linux (Ubuntu/Debian)

```bash
# ESP-IDF dependencies
sudo apt update
sudo apt install git wget flex bison gperf python3 python3-pip python3-venv \
    cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0

# ESP-IDF
mkdir -p ~/esp && cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf && git checkout v5.5
./install.sh esp32s3

# USB permissions (so you can flash without sudo)
sudo usermod -aG dialout $USER   # log out and back in after this

# libfido2 (optional, for testing)
sudo apt install libfido2-dev fido2-tools
```

### Windows

```powershell
# 1. Install Python 3.x from https://www.python.org (check "Add to PATH")
# 2. Install Git from https://git-scm.com
# 3. Install CMake from https://cmake.org
# 4. Install Ninja: https://github.com/ninja-build/ninja/releases
#    (add ninja.exe to PATH)

# 5. Clone ESP-IDF in PowerShell
mkdir C:\esp
cd C:\esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout v5.5
.\install.ps1 esp32s3
```

> **Windows alternative**: Use the [ESP-IDF Windows Installer](https://dl.espressif.com/dl/esp-idf/) — it handles all dependencies automatically and provides a pre-configured ESP-IDF PowerShell/CMD environment.

For `libfido2` on Windows (optional testing):

```powershell
# Using winget
winget install OpenSC.libfido2
# Or download the prebuilt binary from https://developers.yubico.com/libfido2/Releases/
```

---

## Building & Flashing

### Step 1 — Activate ESP-IDF environment

**macOS / Linux:**

```bash
source ~/esp/esp-idf/export.sh
```

**Windows (PowerShell):**

```powershell
C:\esp\esp-idf\export.ps1
```

**Windows (CMD):**

```cmd
C:\esp\esp-idf\export.bat
```

> Run this in every new terminal session before using `idf.py`.

### Step 2 — Enter the project directory

```bash
cd path/to/keypr/test_picofido
```

### Step 3 — Build

```bash
idf.py build
```

On first build, ESP-IDF will automatically download `managed_components/` (TinyUSB, NeoPixel, etc.). This requires internet access.

### Step 4 — Find the flash port

**macOS** — plug in the device, then:

```bash
ls /dev/cu.usbmodem*
# typically: /dev/cu.usbmodem1101
```

**Linux:**

```bash
ls /dev/ttyACM*
# typically: /dev/ttyACM0
```

**Windows** — open Device Manager → Ports (COM & LPT) → look for **USB Serial Device** or **CP210x**. Note the COM port number (e.g. `COM3`).

### Step 5 — Flash

**macOS:**

```bash
idf.py -p /dev/cu.usbmodem1101 -b 460800 flash
```

**Linux:**

```bash
idf.py -p /dev/ttyACM0 -b 460800 flash
```

**Windows:**

```powershell
idf.py -p COM3 -b 460800 flash
```

> **⚠️ Important — replug after flash**: The post-flash RTS reset leaves this board in download mode. **Unplug and replug the USB cable** after flashing to boot into HID/passkey mode. The device will show as `0x303a` (Espressif JTAG) until replugged.

### Step 6 — Verify

**macOS / Linux:**

```bash
fido2-token -L
# ioreg://...: vendor=0x2e8a, product=0x10fe (Pol Henarejos Pico Key)

DEV=$(fido2-token -L | grep -o 'ioreg://[0-9]*')   # macOS
DEV=$(fido2-token -L | awk '{print $1}' | tr -d ':') # Linux
fido2-token -I "$DEV"
```

**Windows (PowerShell):**

```powershell
fido2-token -L
# \.\HID#VID_2E8A&PID_10FE#...

fido2-token -I (fido2-token -L | Select-String "2e8a" | ForEach-Object { $_.ToString().Split(":")[0] })
```

Expected output:

```
version strings: U2F_V2, FIDO_2_0, FIDO_2_1
transport strings: usb
options: rk, up, uv, makeCredUvNotRqd     ← no clientPin
pin retries: undefined                     ← no PIN set
```

---

## Resetting the Key Store

Wipes all credentials and regenerates device keys. **Must be done within 10 seconds of plugging in.**

**macOS:**

```bash
# Unplug and replug the device, then immediately run:
until fido2-token -L 2>/dev/null | grep -q "2e8a"; do sleep 0.05; done \
  && fido2-token -R $(fido2-token -L | grep -o 'ioreg://[0-9]*') \
  && echo "Done"
```

**Linux:**

```bash
until fido2-token -L 2>/dev/null | grep -q "2e8a"; do sleep 0.05; done \
  && fido2-token -R $(fido2-token -L | awk '{print $1}' | tr -d ':') \
  && echo "Done"
```

**Windows (PowerShell):**

```powershell
# Replug device, then:
$dev = (fido2-token -L | Select-String "2e8a").ToString().Split(" ")[0].TrimEnd(":")
fido2-token -R $dev
```

If you get `FIDO_ERR_NOT_ALLOWED` — the 10-second window passed. Unplug, replug, and try again immediately.

---

## Testing Registration (webauthn.io)

1. Open **Chrome** (not Firefox or Safari — they use different CTAP flows)
2. Go to [https://webauthn.io](https://webauthn.io)
3. Enter a **new username** you haven't used before and click **Register**
4. Chrome shows _"Use your security key"_ — click it
5. **Press the button** on the device when the LED blinks
6. Registration completes — you should see a success message

To authenticate: click **Authenticate** with the same username and press the button again.

> If webauthn.io doesn't show the security key option, open `chrome://device-log` and check for FIDO errors. The `transport strings: usb` in getInfo is what triggers the security key UI.

---

## Troubleshooting

| Symptom                                                     | Cause                                                    | Fix                                                                              |
| ----------------------------------------------------------- | -------------------------------------------------------- | -------------------------------------------------------------------------------- |
| Device shows as `0x303a` (Espressif) after flash            | Post-flash RTS reset put device in download mode         | **Unplug and replug USB**                                                        |
| Port not found (`/dev/cu.usbmodem*` missing)                | Device already running as HID; JTAG only appears at boot | Replug to expose JTAG, flash quickly, then replug again to boot                  |
| Linux: `Permission denied` on serial port                   | User not in `dialout` group                              | `sudo usermod -aG dialout $USER`, log out and back in                            |
| `fido2-token -I` returns `FIDO_ERR_RX`                      | Vendor cmds field `0x15` in getInfo broke libfido2       | Already fixed in this firmware                                                   |
| `makeCredential` returns `0x7f` (`CTAP1_ERR_OTHER`)         | Stale PIN-encrypted `ef_keydev` from previous firmware   | Already fixed — auto-recovered on boot; or run `fido2-token -R`                  |
| `makeCredential` returns `0x36` (`CTAP2_ERR_PUAT_REQUIRED`) | Old firmware requiring PIN token                         | Already fixed in `cbor_make_credential.c`                                        |
| Chrome asks for PIN                                         | `clientPin` in getInfo options                           | Already fixed — `clientPin` absent from options                                  |
| Chrome doesn't show security key option                     | `transports: usb` not advertised                         | Already fixed in `cbor_get_info.c`                                               |
| `fido2-token -R` returns `FIDO_ERR_NOT_ALLOWED`             | Missed 10-second boot window                             | Unplug, replug, and run reset command immediately                                |
| Windows: device not recognized                              | Missing USB driver                                       | Install [Zadig](https://zadig.akeo.ie/) and install WinUSB driver for the device |
