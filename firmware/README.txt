ESP32-P4 DIGITAL AUDIO MIXER - FIRMWARE
=========================================

Compiled firmware files for ESP32-P4.

Directory structure:
  firmware/
    ├── README.txt        (this file)
    ├── debug/            (debug build with serial logs)
    └── release/          (optimized release build)

Build commands:
  PlatformIO: pio run -e esp32-p4
  Arduino IDE: Open audio_mixer_p4.ino, select board "ESP32-P4-DevKit"

Output files after compilation:
  .pio/build/esp32-p4/firmware.bin    - Main firmware
  .pio/build/esp32-p4/partitions.bin  - Partition table
  .pio/build/esp32-p4/bootloader.bin  - Bootloader
  .pio/build/esp32-p4/merged.bin      - All-in-one flash image

Flashing:
  pio run -t upload -e esp32-p4

  Or with esptool:
  esptool.py --chip esp32p4 --port /dev/ttyACM0 write_flash \
    0x0 .pio/build/esp32-p4/merged.bin
