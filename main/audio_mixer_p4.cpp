/*
  Project: Digital Audio Mixer - ESP32-P4
  Type: Multiband DSP Audio Mixer
  Microprocessor: ESP32-P4 (RISC-V)
  Author: Adapted from junon10/esp32-multiband-dsp
  License: GPLv3
  Board: ESP32-P4-DevKit or custom
  Framework: ESP-IDF with Arduino component
*/

#include <Arduino.h>
#include "driver/gpio.h"
#include "esp_task_wdt.h"
#include <Preferences.h>
#include <YummyDSP.h>
#include <AudioDriver.h>

#include "DualCompressor.h"
#include "settings.h"
#include "global_vars.h"
#include "serial_cmd.h"
#include "web_server.h"

Preferences preferences;

void setup() {
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = (uint32_t)WDT_TIMEOUT * 1000,
    .idle_core_mask = 0,
    .trigger_panic = true
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);

  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("===========================================");
  Serial.println("  DIGITAL AUDIO MIXER - ESP32-P4");
  Serial.println("  Multiband DSP Processor v" + VERSION);
  Serial.println("===========================================");
  Serial.println();

  for (int i = 0; i < MAX_NUM_BANDS; i++) {
    Equalizer[i] = 0.0f;
    Equalizer_Linear[i] = 0.0f;
    Post_Emphasis_Linear[i] = 0.0f;
  }

  init_config();

  init_filters();
  commitConfig();

  Serial.println("Mounting File System...");
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed - formatting...");
    SPIFFS.format();
    if (!SPIFFS.begin()) {
      Serial.println("SPIFFS Mount Failed after format!");
    }
  } else {
    Serial.println("SPIFFS mounted OK");
  }

  Serial.println();
  Serial.println("I2S Audio Config:");
  Serial.print("  Sample Rate: "); Serial.print(SampleRateFreq); Serial.println(" Hz");
  Serial.print("  Input Channels: "); Serial.println(NUM_IN_CH);
  Serial.print("  Output Channels: "); Serial.println(NUM_OUT_CH);

  Serial.println("\n--- I2S0 (PCM1808 #1 + PCM5102 #1) ---");
  Serial.printf("  BCK=%d LRCK=%d DOUT=%d DIN=%d\n", I2S_BCK_PIN, I2S_LRCLK_PIN, I2S_DOUT_PIN, I2S_DIN_PIN);

  int err = 0;
  i2sCodec0.setI2sPort(I2S_NUM_0);
  err += i2sCodec0.setFormat(SampleRateFreq, channelCount);
  err += i2sCodec0.setPins(I2S_BCK_PIN, I2S_LRCLK_PIN, I2S_DOUT_PIN, I2S_DIN_PIN, CODEC_ENABLE_PIN, I2S0_MCLK_PIN);
  err += i2sCodec0.start();

  Serial.println("\n--- I2S1 (PCM1808 #2 + PCM5102 #2) ---");
  Serial.printf("  BCK=%d LRCK=%d DOUT=%d DIN=%d\n", I2S1_BCK_PIN, I2S1_LRCLK_PIN, I2S1_DOUT_PIN, I2S1_DIN_PIN);

  i2sCodec1.setI2sPort(I2S_NUM_1);
  err += i2sCodec1.setFormat(SampleRateFreq, channelCount);
  err += i2sCodec1.setPins(I2S1_BCK_PIN, I2S1_LRCLK_PIN, I2S1_DOUT_PIN, I2S1_DIN_PIN, I2S1_ENABLE_PIN, I2S1_MCLK_PIN);
  err += i2sCodec1.start();

  if (err == 0) {
    Serial.println("\nBoth I2S ports started successfully!");
  } else {
    Serial.print("\nI2S error code: "); Serial.println(err);
  }

  Serial.println();
  Serial.println("System ready! Type 'menu' for commands.");
  Serial.println();

  xTaskCreatePinnedToCore(audioTask, "audioTask", 8192, NULL, 10, NULL, 1);
  delay(500);

  xTaskCreatePinnedToCore(serialCommTask, "serialCommTask", 4096, NULL, 6, NULL, 0);
  delay(500);

  // Initialize WiFi (ESP32-C6 via SDIO) and web server
  Serial.println("Initializing WiFi via ESP32-C6 co-processor...");
  if (initWiFi() == ESP_OK) {
    if (startWebServer() == ESP_OK) {
      Serial.println("Web server started! Open http://audiostreamer-p4.local or check router for IP.");
    }
  } else {
    Serial.println("WiFi init failed (C6 may not be ready). Continuing without network.");
  }
}

void loop() {
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}


