# ESP32-P4 Digital Audio Mixer — Agent Guide

## Target hardware

- **ESP32-P4** (RISC-V, 400MHz, **no WiFi/BT**)
- Arduino support is experimental (v3.1+). For production use ESP-IDF 5.3+.
- PSRAM available (enable via `BOARD_HAS_PSRAM`).

## Build & flash

**PlatformIO** (recommended, use this):

```
pio run -e esp32-p4
pio run -t upload -e esp32-p4
pio device monitor -b 115200
```

Debug build: `pio run -e esp32-p4-debug` (sets `SERIAL_LOG=1`, verbose core logs).

Output `.bin` files: `.pio/build/esp32-p4/firmware.bin`. The `build.sh` script copies them to `firmware/release/` or `firmware/debug/`.

**Arduino IDE**: needs ESP32 board v3.1+ via `https://espressif.github.io/arduino-esp32/package_esp32_index.json`. Flash Size=16MB, Partition=Minimal SPIFFS, Upload Speed=921600.

## Architecture

Two FreeRTOS tasks pinned to cores:

| Task | Core | Stack | Prio | Role |
|------|------|-------|------|------|
| `audioTask` | 1 | 8192 | 10 | I2S read → DSP chain → I2S write |
| `serialCommTask` | 0 | 4096 | 6 | Serial command interpreter |

`loop()` is empty (just a delay). All work is in tasks.

Audio flow per sample: input → per-band HPF/LPF → EQ → compressor → post-emphasis → mix → echo → limiter → clipper → volume → output.

## Key code layout

- `audio_mixer_p4.ino` — minimal setup(), all logic in included headers
- `src/settings.h` — pins (I2S: BCK=5, LRCK=6, DOUT=7, DIN=4, EN=8), constants, 10-band filter frequency table
- `src/global_vars.h` — audio task, config read/write (JSON on SPIFFS), filter init
- `src/DualCompressor.h` — single-header linked-stereo compressor
- `src/serial_cmd.h` — stateful multi-step serial menu (type `menu` at 115200 baud)

## Control

Serial terminal at 115200 baud. Commands:

| Input | Action |
|-------|--------|
| `menu` | print full menu |
| `status` or `0` | show system state |
| `1`–`18` | set parameter (triggers multi-step prompt for value) |
| `5` | toggle compressor on/off |
| `6` | toggle mute |

Settings saved to SPIFFS as `/config.json`. Read on boot from `SPIFFS.begin()`.

## Library setup

Local libraries in `libraries/YummyDSP/src/`. `platformio.ini` uses `lib_extra_dirs = libraries`.

The `AudioDriver` was modified for ESP32-P4: uses `driver/i2s_std.h` (IDF 5.x new I2S API) with compile-time guard `CONFIG_IDF_TARGET_ESP32P4`. Falls back to legacy `driver/i2s.h` on other ESP32 targets.

External dependency: `bblanchon/ArduinoJson ^7.0`.

## Gotchas

- The `serial_cmd.h` state machine stores position in `static` variables inside each `changeParam`/`changeIntParam` function. Only one parameter change at a time.
- `outputSampleR/L` must be zeroed each sample loop (bug if not — accumulative). Already fixed in `global_vars.h`.
- `audioTask` runs at real-time priority (10) on core 1. Do not add blocking calls.
- `SPIFFS.begin(false)` (no auto-format) in first call; if it fails, `begin(true)` is used. This means boot can be slow on first-time or corrupted filesystem.
- Filter frequency table `FILTER_FREQ[N_BAND][N_FREQ]` in `settings.h` — rows = band count (1-10), columns = band edge frequencies. Changing this affects all EQ and crossover behaviour.
- The project was adapted from `junon10/esp32-multiband-dsp` — the original used ArduinoJson 5.x and WiFi. This version replaced both with ArduinoJson 7.x and serial-only control.
- Sample rate: 48000 Hz, 32-bit, I2S Philips format. `I2S_NUM_0`.
