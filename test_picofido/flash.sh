#!/bin/zsh
#
# flash.sh — Build and flash pico-fido to Qualia ESP32-S3 RGB666
#

set -e

IDF_PATH="${IDF_PATH:-$HOME/esp/esp-idf}"
TARGET="esp32s3"
BAUD="460800"

# ── colours ────────────────────────────────────────────────────────────────
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; NC='\033[0m'
info()    { echo "${GREEN}[flash]${NC} $*"; }
warn()    { echo "${YELLOW}[flash]${NC} $*"; }
error()   { echo "${RED}[flash]${NC} $*" >&2; exit 1; }

# ── source ESP-IDF ──────────────────────────────────────────────────────────
if [ -z "$IDF_TOOLS_PATH" ]; then
    info "Sourcing ESP-IDF from $IDF_PATH …"
    source "$IDF_PATH/export.sh" > /dev/null 2>&1 || \
        error "Failed to source ESP-IDF. Set IDF_PATH to your esp-idf root."
fi

# ── detect port ─────────────────────────────────────────────────────────────
detect_port() {
    # Prefer a port that looks like a real USB device
    local port
    port=$(ls /dev/cu.usbmodem* /dev/cu.SLAB* /dev/cu.wchusbserial* \
               /dev/cu.usbserial* 2>/dev/null | head -1)
    echo "$port"
}

PORT="${PORT:-$(detect_port)}"

if [ -z "$PORT" ]; then
    error "No ESP32 serial port found. Plug in the device or set PORT=<path>."
fi
info "Using port: $PORT"

# ── cd to project root ───────────────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

# ── set target (idempotent) ──────────────────────────────────────────────────
CURRENT_TARGET=$(cat build/CMakeCache.txt 2>/dev/null | grep "^IDF_TARGET:" | cut -d= -f2 || true)
if [ "$CURRENT_TARGET" != "$TARGET" ]; then
    info "Setting IDF target to $TARGET …"
    idf.py set-target "$TARGET"
else
    info "Target already set to $TARGET, skipping."
fi

# ── build ────────────────────────────────────────────────────────────────────
info "Building …"
idf.py build

# ── flash ────────────────────────────────────────────────────────────────────
info "Flashing to $PORT at ${BAUD} baud …"
idf.py -p "$PORT" -b "$BAUD" flash

info "Done! Device has been reset and is running pico-fido."
info "Run 'idf.py -p $PORT monitor' to open the serial monitor."
