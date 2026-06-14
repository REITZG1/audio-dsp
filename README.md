# ESP32-P4 Digital Audio Mixer

Multiband DSP audio mixer berbasis ESP32-P4 (RISC-V) dengan equalizer, kompresor, echo, dan kontrol serial.

## Fitur

- Multiband DSP processor (1-10 band)
- Graphic equalizer per-band
- Stereo compressor dengan attack/release control
- Audio echo/delay
- Balance control
- Pre-emphasis dan Post-emphasis
- Output peak protection
- Konfigurasi via Serial terminal
- Penyimpanan konfigurasi ke SPIFFS

## Hardware Requirements

- ESP32-P4 DevKit board
- PCM5102A DAC (I2S output)
- PCM1802 ADC (I2S input) atau STM32F411 USB Audio
- Power supply via USB-C

### Pin Connections (ESP32-P4)

| Signal | GPIO Pin |
|--------|----------|
| I2S0_BCK  | GPIO5  |
| I2S0_LRCK | GPIO6  |
| I2S0_DOUT | GPIO7  |
| I2S0_DIN  | GPIO4  |
| CODEC_EN  | GPIO8  |
| MCLK      | GPIO21 |

*Sesuaikan pin dengan board Anda.*

## Cara Kompilasi

### Opsi 1: PlatformIO (Recommended)

```bash
# Install PlatformIO
pip install platformio

# Build
cd /mnt/sdcard/mixer
pio run -e esp32-p4

# Flash ke board
pio run -t upload -e esp32-p4

# Monitor serial
pio device monitor -b 115200
```

### Opsi 2: Arduino IDE

1. Install ESP32 Arduino Core v3.x dari Boards Manager
2. Tambahkan URL Board: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. Install library:
   - ArduinoJson (via Library Manager)
   - YummyDSP: `git clone https://github.com/junon10/yummyDSP`
   - AudioDriver: `git clone https://github.com/junon10/STM32F411_USB_AUDIO_DAC`
4. Board: ESP32-P4-DevKit atau Generic ESP32-P4
5. Partitions: Minimal SPIFFS (1.9MB APP / 1.5MB SPIFFS)
6. Flash Size: 16MB
7. Upload Speed: 921600

### Opsi 3: ESP-IDF

```bash
idf.py set-target esp32p4
idf.py menuconfig
idf.py build
idf.py -p /dev/ttyACM0 flash
```

## Output HEX/BIN

Setelah kompilasi, file firmware ada di:

- PlatformIO: `.pio/build/esp32-p4/firmware.bin`
- Arduino IDE: biasanya di folder build output
- Copy ke `firmware/release/` untuk production

### Flashing Manual

```bash
python esptool.py --chip esp32p4 --port /dev/ttyACM0 --baud 921600 \
  write_flash 0x0 firmware/release/audio_mixer_p4_merged.bin
```

## Kontrol via Serial

Konek via serial monitor (115200 baud), lalu ketik:

| Perintah | Fungsi |
|----------|--------|
| `menu`   | Tampilkan menu |
| `status` | Tampilkan status sistem |
| `1`      | Set Input Level |
| `2`      | Set Output Level |
| `3`      | Set Balance |
| `5`      | Toggle Compressor ON/OFF |
| `6`      | Toggle Mute ON/OFF |
| `7`      | Set Number of Bands |
| `17`     | Save config to SPIFFS |
| `18`     | Reset to default settings |
| `19+`    | Adjust individual EQ bands |

## Struktur Folder

```
/mnt/sdcard/mixer/
├── audio_mixer_p4.ino    # Main Arduino sketch
├── platformio.ini         # PlatformIO config
├── README.md              # File ini
├── src/
│   ├── settings.h         # Constants & pin definitions
│   ├── global_vars.h      # Global variables & audio task
│   ├── DualCompressor.h   # Compressor implementation
│   └── serial_cmd.h       # Serial command interface
├── libraries/             # Dependency libraries
│   ├── YummyDSP/
│   ├── AudioDriver/
│   └── ArduinoJson/
├── data/                  # SPIFFS data files
└── firmware/
    ├── debug/             # Debug firmware binaries
    ├── release/           # Release firmware binaries
    └── README.txt
```

## Catatan ESP32-P4

- ESP32-P4 adalah RISC-V dual-core @400MHz
- **TIDAK** memiliki WiFi/BT (berbeda dengan ESP32 biasa)
- Memiliki PSRAM hingga 32MB
- Menggunakan I2S0 untuk audio
- Arduino core support: experimental (v3.1+)
- Untuk production: gunakan ESP-IDF 5.3+

## Credits

- Base project: https://github.com/junon10/esp32-multiband-dsp
- DSP Library: YummyDSP by Junon M.
- Audio Driver by Junon M.
