#pragma once
#include <Arduino.h>
#include <esp_task_wdt.h>

void commandInterpreter();
#include <YummyDSP.h>
#include <AudioDriver.h>
#include <ArduinoJson.h>
#include "DualCompressor.h"
#include "settings.h"
#include "SPIFFS.h"

extern bool espRestart;
extern bool isGoodFileSystem;
extern uint32_t CounterLoopRestart_100ms;
extern uint32_t RestartTimeX100ms;
extern uint16_t MuteTimeX100ms;
extern int TimeZone;

extern float VuArrValue[MAX_NUM_BANDS];
extern float InputLevel;
extern float OutputLevel;
extern float Balance;
extern float Clipper;

extern bool Compressor;
extern bool BandSync;
extern bool Mute;
extern bool ForcedMute;
extern bool Reserved1;
extern bool Reserved2;

extern int NumBands;
extern float PreEmphasis;
extern float PostEmphasis;
extern float StepBy;
extern float Echo;
extern float FiltersQFactor;

extern float Protection[ALL_NUM_BANDS];
extern float AttackTime[ALL_NUM_BANDS];
extern float ReleaseTime[ALL_NUM_BANDS];
extern float Gain[ALL_NUM_BANDS];
extern float Equalizer[MAX_NUM_BANDS];
extern float Equalizer_Linear[MAX_NUM_BANDS];
extern float Post_Emphasis_Linear[MAX_NUM_BANDS];
extern float Clipper_Linear_p;
extern float Clipper_Linear_n;

extern AudioDriver i2sCodec0;
extern AudioDriver i2sCodec1;
extern DualCompressor comp_st[MAX_NUM_BANDS];
extern DualCompressor limiter_st;
extern YummyDSP dspBAND[MAX_NUM_BANDS];
extern YummyDSP dspDelay;
extern YummyDSP dspFmFilter;
extern FilterNode LPF_BAND[MAX_NUM_BANDS];
extern FilterNode HPF_BAND[MAX_NUM_BANDS];
extern FilterNode LPF_FM_FILTER[NumFmFilters];
extern DelayNode ECHO;

extern float InputLevelLinear;
extern float OutputLevelLinearL;
extern float OutputLevelLinearR;
extern float BalanceLinearL;
extern float BalanceLinearR;
extern float VolCompensationLinear;

extern float Band_R[MAX_NUM_BANDS];
extern float Band_L[MAX_NUM_BANDS];
extern float inputSampleR;
extern float inputSampleL;
extern float outputSampleR;
extern float outputSampleL;

// 4-in / 4-out matrix mixer
extern float InTrim[NUM_IN_CH];
extern bool InMute[NUM_IN_CH];
extern float InTrimLinear[NUM_IN_CH];
extern float RouteGain[NUM_OUT_CH][NUM_IN_CH];
extern float OutLvl[NUM_OUT_CH];
extern bool OutMute[NUM_OUT_CH];
extern float OutLvlLinear[NUM_OUT_CH];

void init_filters();
void commitConfig();
void changeEqualization();
void updateFilters();
void setAudioVolume();
void audioTask(void *pvParameters);
void serialCommTask(void *pvParameters);
void timeCounterTask(void *pvParameters);
String readFile(String path);
bool writeFile(String path, String content);
void loadDefaultConfig();
void readConfig();
bool saveConfig();
void deleteFile(fs::FS &fs, const char *path);
void init_config();

inline float decibel_2_linear(float from) {
  return pow(10.0f, from / 10.0f);
}

inline float linear_2_decibel(float from) {
  return 10.0f * log10(from);
}

inline String left(String s, int len) {
  String S = s;
  while (S.length() < len) S = " " + S;
  return S;
}

inline String right(String s, int len) {
  String S = s;
  while (S.length() < len) S += ".";
  return S;
}

inline float getEqEmphasis(float freq, float att_final) {
  float max_freq = FILTER_FREQ[NumBands - 1][NumBands];
  float factor = att_final / max_freq;
  return freq * factor;
}

inline String getStrFilterFreq(int band) {
  String s = "";
  if ((band >= 0) && (band < NumBands)) {
    int hpf = FILTER_FREQ[NumBands - 1][band];
    int lpf = FILTER_FREQ[NumBands - 1][band + 1];
    s = left(String(hpf), 6) + "Hz to " + left(String(lpf), 6) + "Hz";
  } else {
    s = "Error: Invalid index!";
  }
  return s;
}

inline int getFilterFreq(int index) {
  int f = 0;
  if ((index >= 0) && (index <= NumBands)) {
    f = FILTER_FREQ[NumBands - 1][index];
  }
  return f;
}

inline int getBandCenterFrequency(int band_index) {
  int hpf = FILTER_FREQ[NumBands - 1][band_index];
  int lpf = FILTER_FREQ[NumBands - 1][band_index + 1];
  return hpf + ((lpf - hpf) / 2);
}
