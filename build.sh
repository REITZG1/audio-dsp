#!/bin/bash
# Build script for ESP32-P4 Digital Audio Mixer
# Usage: ./build.sh [release|debug]

set -e

BUILD_TYPE="${1:-release}"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
FIRMWARE_DIR="$PROJECT_DIR/firmware"

echo "========================================"
echo "  ESP32-P4 Digital Audio Mixer Build"
echo "  Build type: $BUILD_TYPE"
echo "========================================"
echo ""

# Check for PlatformIO
if command -v pio &> /dev/null; then
    echo "[1/3] Building with PlatformIO..."
    
    if [ "$BUILD_TYPE" = "debug" ]; then
        pio run -e esp32-p4-debug
    else
        pio run -e esp32-p4
    fi
    
    echo ""
    echo "[2/3] Copying firmware files..."
    
    BUILD_DIR="$PROJECT_DIR/.pio/build/esp32-p4"
    TARGET_DIR="$FIRMWARE_DIR/$BUILD_TYPE"
    
    if [ -f "$BUILD_DIR/firmware.bin" ]; then
        cp "$BUILD_DIR/firmware.bin" "$TARGET_DIR/"
        echo "  -> firmware.bin copied"
    fi
    
    if [ -f "$BUILD_DIR/partitions.bin" ]; then
        cp "$BUILD_DIR/partitions.bin" "$TARGET_DIR/"
        echo "  -> partitions.bin copied"
    fi
    
    if [ -f "$BUILD_DIR/bootloader.bin" ]; then
        cp "$BUILD_DIR/bootloader.bin" "$TARGET_DIR/"
        echo "  -> bootloader.bin copied"
    fi
    
    # Try to create merged flash image
    if command -v esptool.py &> /dev/null && [ -f "$BUILD_DIR/firmware.bin" ]; then
        echo ""
        echo "[3/3] Creating merged flash image..."
        python -m esptool merge_bin \
            --output "$TARGET_DIR/audio_mixer_p4_merged.bin" \
            --flash_mode qio \
            --flash_size 16MB \
            0x1000 "$BUILD_DIR/bootloader.bin" \
            0x8000 "$BUILD_DIR/partitions.bin" \
            0x10000 "$BUILD_DIR/firmware.bin"
        echo "  -> audio_mixer_p4_merged.bin created"
    fi
    
    echo ""
    echo "Build complete! Files in: $TARGET_DIR"
    
else
    echo "ERROR: PlatformIO not found!"
    echo ""
    echo "Install PlatformIO:"
    echo "  pip install platformio"
    echo ""
    echo "Or use Arduino IDE manually:"
    echo "  1. Open audio_mixer_p4.ino"
    echo "  2. Board: ESP32-P4-DevKit"
    echo "  3. Sketch -> Export Compiled Binary"
    echo "  4. Copy .bin files to firmware/$BUILD_TYPE/"
    exit 1
fi
