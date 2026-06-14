#pragma once
#include <Arduino.h>

const String VERSION = "1.0.0.1 2025/06/14 P4";
#define CONFIG_TAG 124

//#define SERIAL_LOG 1

#define SAVE_AS_JSON 1
#define ENG_LANG 1

// Audio without floating point processing
//#define RAW_AUDIO true

const String APPLICATION_TITLE = "DIGITAL AUDIO MIXER";

// MAX
const float MAX_INPUT_LEVEL = 90.0f;
const float MAX_OUTPUT_LEVEL = 10.0f;
const float MAX_BALANCE = 100.0f;
const float MAX_CLIPPER = 40.0f;
const int MAX_NUM_BANDS = 10;
const float MAX_PRE_EMPHASIS = 30.0f;
const float MAX_POST_EMPHASIS = 30.0f;
const float MAX_STEP_BY = -3.0f;
const float MAX_ECHO = 1.0f;
const float MAX_PROTECTION = 30.0f;
const float MAX_GAIN = 80.0f;
const float MAX_ATTACK_TIME = 1000.0f;
const float MAX_RELEASE_TIME = 50000.0f;
const float MAX_EQ_BAND = 12.0f;
const float MAX_FILTERS_Q_FACTOR = 10.0f;

// MIN
const float MIN_INPUT_LEVEL = -50.0f;
const float MIN_OUTPUT_LEVEL = -70.0f;
const float MIN_BALANCE = 0.0f;
const float MIN_CLIPPER = -40.0f;
const int MIN_NUM_BANDS = 1;
const float MIN_PRE_EMPHASIS = -30.0f;
const float MIN_POST_EMPHASIS = -30.0f;
const float MIN_STEP_BY = -90.0f;
const float MIN_ECHO = 0.0f;
const float MIN_PROTECTION = 0.0f;
const float MIN_GAIN = 0.0f;
const float MIN_ATTACK_TIME = 0.0f;
const float MIN_RELEASE_TIME = 5.0f;
const float MIN_EQ_BAND = -12.0f;
const float MIN_FILTERS_Q_FACTOR = 0.1f;

// DEFAULT
const float DEFAULT_INPUT_LEVEL = 16.0f;
const float DEFAULT_OUTPUT_LEVEL = -20.0f;
const float DEFAULT_BALANCE = 50.0f;
const float DEFAULT_CLIPPER = 0.0f;
const bool DEFAULT_COMPRESSOR = true;
const bool DEFAULT_BAND_SYNC = true;
const bool DEFAULT_MUTE = false;
const bool DEFAULT_RESERVED1 = false;
const bool DEFAULT_RESERVED2 = true;
const int DEFAULT_NUM_BANDS = 8;
const float DEFAULT_PRE_EMPHASIS = 0.0f;
const float DEFAULT_POST_EMPHASIS = -6.0f;
const float DEFAULT_STEP_BY = -14.0f;
const float DEFAULT_ECHO = 0.0f;
const float DEFAULT_PROTECTION = 7.0f;
const float DEFAULT_GAIN = 50.0f;
const float DEFAULT_ATTACK_TIME = 30.0f;
const float DEFAULT_RELEASE_TIME = 30.0f;
const float DEFAULT_EQ_BAND = 0.0f;
const float DEFAULT_FILTERS_Q_FACTOR = 1.0f;
const bool DEFAULT_FORCED_MUTE = true;
const int DEFAULT_FORCED_MUTE_TIME = 100;
const int DEFAULT_RESTART_TIME = 100;
const float DEFAULT_VOL_CORRECTION = -10.0f;
const float DEFAULT_VOL_COMPENSATION = 20.0f;

#define NumFmFilters 0

const int ALL_NUM_BANDS = MAX_NUM_BANDS + 2;
const int N_BAND = 10, N_FREQ = 11;

// 04 HD 3 - Filter frequency table
const int FILTER_FREQ[N_BAND][N_FREQ] = {
  {20, 18000, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {20, 6000, 18000, 0, 0, 0, 0, 0, 0, 0, 0},
  {20, 200, 3000, 18000, 0, 0, 0, 0, 0, 0, 0},
  {20, 200, 500, 17000, 18000, 0, 0, 0, 0, 0, 0},
  {20, 200, 500, 3000, 14000, 18000, 0, 0, 0, 0, 0},
  {20, 200, 500, 2000, 6000, 15000, 18000, 0, 0, 0, 0},
  {20, 200, 500, 2000, 6000, 14000, 16000, 18000, 0, 0, 0},
  {20, 200, 500, 2000, 6000, 14000, 16000, 17000, 18000, 0, 0},
  {20, 200, 500, 2000, 6000, 14000, 15000, 16000, 17000, 18000, 0},
  {20, 200, 500, 2000, 5000, 13000, 14000, 15000, 16000, 17000, 18000}
};

#define DEFAULT_CONFIG_FILENAME "/config.json"

// I2S Basic Settings
const int SampleRateFreq = 48000;
const int channelCount = 2;

// ESP32-P4 I2S Pinout (typical)
// I2S0 Interface (Use these or adapt to your board)
#define I2S_BCK_PIN       5   // I2S0_BCK
#define I2S_LRCLK_PIN     6   // I2S0_LRCK
#define I2S_DOUT_PIN      7   // I2S0_DOUT (DAC data out)
#define I2S_DIN_PIN       4   // I2S0_DIN  (ADC data in)
#define CODEC_ENABLE_PIN  8   // Codec power / enable
#define CLOCK_OUT_PIN     21  // Optional MCLK output

// WDT
#define WDT_TIMEOUT 10

// Core affinity
static uint8_t CORE_ZERO = 0;
static uint8_t CORE_ONE = 1;
