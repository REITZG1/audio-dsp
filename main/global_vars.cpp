#include "global_vars.h"

bool espRestart = false;
bool isGoodFileSystem = true;
uint32_t CounterLoopRestart_100ms = 0, RestartTimeX100ms = DEFAULT_RESTART_TIME;
uint16_t MuteTimeX100ms = DEFAULT_FORCED_MUTE_TIME;
int TimeZone = -3;

float VuArrValue[MAX_NUM_BANDS];
float InputLevel = DEFAULT_INPUT_LEVEL;
float OutputLevel = DEFAULT_OUTPUT_LEVEL;
float Balance = DEFAULT_BALANCE;
float Clipper = DEFAULT_CLIPPER;

bool Compressor = DEFAULT_COMPRESSOR;
bool BandSync = DEFAULT_BAND_SYNC;
bool Mute = DEFAULT_MUTE, ForcedMute = DEFAULT_FORCED_MUTE;
bool Reserved1 = DEFAULT_RESERVED1;
bool Reserved2 = DEFAULT_RESERVED2;

int NumBands = DEFAULT_NUM_BANDS;
float PreEmphasis = DEFAULT_PRE_EMPHASIS;
float PostEmphasis = DEFAULT_POST_EMPHASIS;
float StepBy = DEFAULT_STEP_BY;
float Echo = DEFAULT_ECHO;
float FiltersQFactor = DEFAULT_FILTERS_Q_FACTOR;

float Protection[ALL_NUM_BANDS];
float AttackTime[ALL_NUM_BANDS];
float ReleaseTime[ALL_NUM_BANDS];
float Gain[ALL_NUM_BANDS];
float Equalizer[MAX_NUM_BANDS];
float Equalizer_Linear[MAX_NUM_BANDS];
float Post_Emphasis_Linear[MAX_NUM_BANDS];
float Clipper_Linear_p = 0.0f;
float Clipper_Linear_n = 0.0f;

AudioDriver i2sCodec0;
AudioDriver i2sCodec1;
DualCompressor comp_st[MAX_NUM_BANDS];
DualCompressor limiter_st;
YummyDSP dspBAND[MAX_NUM_BANDS], dspDelay, dspFmFilter;
FilterNode LPF_BAND[MAX_NUM_BANDS], HPF_BAND[MAX_NUM_BANDS];
FilterNode LPF_FM_FILTER[NumFmFilters];
DelayNode ECHO;

float InputLevelLinear = 1.0f;
float OutputLevelLinearL = 0.0f;
float OutputLevelLinearR = 0.0f;
float BalanceLinearL = 0.0f;
float BalanceLinearR = 0.0f;
float VolCompensationLinear = 1.0f;

float Band_R[MAX_NUM_BANDS];
float Band_L[MAX_NUM_BANDS];
float inputSampleR = 0, inputSampleL = 0, outputSampleR = 0, outputSampleL = 0;

// 4-in / 4-out matrix mixer
float InTrim[NUM_IN_CH] = {0, 0, 0, 0};
bool InMute[NUM_IN_CH] = {false, false, false, false};
float InTrimLinear[NUM_IN_CH] = {1.0f, 1.0f, 1.0f, 1.0f};
float RouteGain[NUM_OUT_CH][NUM_IN_CH] = {
  {1.0f, 0.0f, 0.0f, 0.0f},
  {0.0f, 1.0f, 0.0f, 0.0f},
  {0.0f, 0.0f, 1.0f, 0.0f},
  {0.0f, 0.0f, 0.0f, 1.0f}
};
float OutLvl[NUM_OUT_CH] = {0, 0, 0, 0};
bool OutMute[NUM_OUT_CH] = {false, false, false, false};
float OutLvlLinear[NUM_OUT_CH] = {1.0f, 1.0f, 1.0f, 1.0f};

void changeEqualization() {
  float G = 0.0, lastG = 0.0;
  int center_freq = 0;
  for (int i = 0; i < NumBands; i++) {
    center_freq = getBandCenterFrequency(i);
    G = Equalizer[i] + getEqEmphasis(center_freq, PreEmphasis);
    if (G > lastG) lastG = G;
  }
  for (int i = 0; i < NumBands; i++) {
    center_freq = getBandCenterFrequency(i);
    Equalizer_Linear[i] = decibel_2_linear(Equalizer[i] + getEqEmphasis(center_freq, PreEmphasis) - lastG);
  }
  for (int i = 0; i < NumBands; i++) {
    center_freq = getBandCenterFrequency(i);
    Post_Emphasis_Linear[i] = decibel_2_linear(getEqEmphasis(center_freq, PostEmphasis));
  }
}

void updateFilters() {
  static int num_bands = NumBands;
  if (num_bands != NumBands) {
    num_bands = NumBands;
    ForcedMute = true;
    MuteTimeX100ms = 10;
    for (int i = 0; i < MAX_NUM_BANDS; i++) dspBAND[i].clear();
    for (int i = NumBands - 1; i >= 0; i--) {
      int hpf = FILTER_FREQ[NumBands - 1][i];
      int lpf = FILTER_FREQ[NumBands - 1][i + 1];
      LPF_BAND[i].resetFilter(FilterNode::LPF, lpf, FiltersQFactor);
      HPF_BAND[i].resetFilter(FilterNode::HPF, hpf, FiltersQFactor);
    }
    for (int i = 0; i < NumBands; i++) {
      dspBAND[i].addNode(&LPF_BAND[i]);
      dspBAND[i].addNode(&HPF_BAND[i]);
    }
  }
}

void setAudioVolume() {
  OutputLevelLinearL = decibel_2_linear(OutputLevel + DEFAULT_VOL_CORRECTION);
  OutputLevelLinearR = decibel_2_linear(OutputLevel + DEFAULT_VOL_CORRECTION);
}

void commitConfig() {
  for (int i = 0; i < MAX_NUM_BANDS; i++) {
    int j = MAX_NUM_BANDS;
    comp_st[i].setParams(Protection[j], AttackTime[j], ReleaseTime[j], Gain[j], (float)SampleRateFreq, StepBy);
  }
  int k = MAX_NUM_BANDS + 1;
  limiter_st.setParams(Protection[k], AttackTime[k], ReleaseTime[k], Gain[k], (float)SampleRateFreq, StepBy);
  updateFilters();
  ECHO.setMix(Echo, true);
  changeEqualization();
  InputLevelLinear = decibel_2_linear(InputLevel);

  for (int ch = 0; ch < NUM_IN_CH; ch++) {
    InTrimLinear[ch] = decibel_2_linear(InTrim[ch]);
  }

  float P = Balance / 100.0f;
  if (Balance < 50.0f) {
    BalanceLinearL = Balance * 0.02f;
    BalanceLinearR = 1.0f;
  } else if (Balance > 50.0f) {
    BalanceLinearL = 1.0f;
    BalanceLinearR = abs(Balance - 100.0f) * 0.02f;
  } else {
    BalanceLinearL = 1.0f;
    BalanceLinearR = 1.0f;
  }
  setAudioVolume();
  OutputLevelLinearL *= BalanceLinearL;
  OutputLevelLinearR *= BalanceLinearR;

  for (int ch = 0; ch < NUM_OUT_CH; ch++) {
    OutLvlLinear[ch] = decibel_2_linear(OutLvl[ch]);
  }

  Clipper_Linear_p = decibel_2_linear(Clipper + 10.0f) * COMP_LIMIT;
  Clipper_Linear_n = Clipper_Linear_p * -1.0f;
  VolCompensationLinear = decibel_2_linear(DEFAULT_VOL_COMPENSATION) + 1.0f;
}

void init_filters() {
  for (int i = 0; i < MAX_NUM_BANDS; i++) {
    dspBAND[i].begin(SampleRateFreq);
    LPF_BAND[i].begin(SampleRateFreq, 2);
    HPF_BAND[i].begin(SampleRateFreq, 2);
  }
  for (int i = NumBands - 1; i >= 0; i--) {
    int hpf = FILTER_FREQ[NumBands - 1][i];
    int lpf = FILTER_FREQ[NumBands - 1][i + 1];
    LPF_BAND[i].setupFilter(FilterNode::LPF, lpf, FiltersQFactor);
    HPF_BAND[i].setupFilter(FilterNode::HPF, hpf, FiltersQFactor);
  }
  for (int i = 0; i < NumBands; i++) {
    dspBAND[i].addNode(&LPF_BAND[i]);
    dspBAND[i].addNode(&HPF_BAND[i]);
  }
  dspFmFilter.begin(SampleRateFreq);
  for (int i = 0; i < NumFmFilters; i++) {
    LPF_FM_FILTER[i].begin(SampleRateFreq, 2);
    LPF_FM_FILTER[i].setupFilter(FilterNode::LPF, 18000, 0.8f);
    dspFmFilter.addNode(&LPF_FM_FILTER[i]);
  }
  dspDelay.begin(SampleRateFreq);
  ECHO.begin(SampleRateFreq, 2);
  ECHO.setDelayMs(30.0f);
  ECHO.setMix(Echo, true);
  dspDelay.addNode(&ECHO);
}

void init_config() {
  Serial.print("\nMounting File System: ");
  if (SPIFFS.begin(false)) {
    Serial.println("OK");
    isGoodFileSystem = true;
  } else {
    Serial.println("FAILED! Formatting...");
    if (SPIFFS.format()) {
      Serial.println("Format OK, remounting...");
      if (SPIFFS.begin(false)) {
        Serial.println("Mounted successfully!");
        isGoodFileSystem = true;
      } else {
        Serial.println("FAILED!");
        isGoodFileSystem = false;
      }
    } else {
      Serial.println("Format failed!");
      isGoodFileSystem = false;
    }
  }
}

String readFile(String path) {
  String S = "";
  if (SPIFFS.exists(path)) {
    File f = SPIFFS.open(path, "r");
    if (f && f.size()) {
      while (f.available()) S += char(f.read());
      f.close();
    }
  }
  return S;
}

bool writeFile(String path, String content) {
  File f = SPIFFS.open(path, "w");
  if (f) {
    f.print(content);
    f.close();
    return true;
  }
  return false;
}

void deleteFile(fs::FS &fs, const char *path) {
  if (fs.remove(path)) {
    Serial.printf("Deleted: %s\n", path);
  }
}

void loadDefaultConfig() {
  InputLevel = DEFAULT_INPUT_LEVEL;
  OutputLevel = DEFAULT_OUTPUT_LEVEL;
  Clipper = DEFAULT_CLIPPER;
  Compressor = DEFAULT_COMPRESSOR;
  BandSync = DEFAULT_BAND_SYNC;
  Mute = DEFAULT_MUTE;
  Reserved1 = DEFAULT_RESERVED1;
  Reserved2 = DEFAULT_RESERVED2;
  NumBands = DEFAULT_NUM_BANDS;
  PreEmphasis = DEFAULT_PRE_EMPHASIS;
  PostEmphasis = DEFAULT_POST_EMPHASIS;
  StepBy = DEFAULT_STEP_BY;
  Echo = DEFAULT_ECHO;
  FiltersQFactor = DEFAULT_FILTERS_Q_FACTOR;
  TimeZone = -3;
  for (int i = 0; i < ALL_NUM_BANDS; i++) {
    if (i < MAX_NUM_BANDS) Equalizer[i] = DEFAULT_EQ_BAND;
    Protection[i] = DEFAULT_PROTECTION;
    Gain[i] = DEFAULT_GAIN;
    AttackTime[i] = DEFAULT_ATTACK_TIME;
    ReleaseTime[i] = DEFAULT_RELEASE_TIME;
  }
  for (int ch = 0; ch < NUM_IN_CH; ch++) {
    InTrim[ch] = 0.0f;
    InMute[ch] = false;
  }
  for (int o = 0; o < NUM_OUT_CH; o++) {
    OutLvl[o] = 0.0f;
    OutMute[o] = false;
    for (int i = 0; i < NUM_IN_CH; i++) {
      RouteGain[o][i] = (i == o) ? 1.0f : 0.0f;
    }
  }
}

void readConfig() {
  String strJson = readFile(DEFAULT_CONFIG_FILENAME);
  if (strJson.isEmpty()) {
    loadDefaultConfig();
    return;
  }

  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, strJson);
  if (error) {
    Serial.println("JSON parse error, using defaults");
    loadDefaultConfig();
    return;
  }

  InputLevel = doc["Il"] | DEFAULT_INPUT_LEVEL;
  OutputLevel = doc["Ol"] | DEFAULT_OUTPUT_LEVEL;
  Clipper = doc["Clip"] | DEFAULT_CLIPPER;
  Compressor = doc["Comp"] | DEFAULT_COMPRESSOR;
  BandSync = doc["Sync"] | DEFAULT_BAND_SYNC;
  Mute = doc["Mute"] | DEFAULT_MUTE;
  Reserved1 = doc["Res1"] | DEFAULT_RESERVED1;
  Reserved2 = doc["Res2"] | DEFAULT_RESERVED2;
  NumBands = doc["Nb"] | DEFAULT_NUM_BANDS;
  PreEmphasis = doc["PreE"] | DEFAULT_PRE_EMPHASIS;
  PostEmphasis = doc["PostE"] | DEFAULT_POST_EMPHASIS;
  StepBy = doc["Step"] | DEFAULT_STEP_BY;
  Echo = doc["Echo"] | DEFAULT_ECHO;
  FiltersQFactor = doc["Fqf"] | DEFAULT_FILTERS_Q_FACTOR;
  TimeZone = doc["Tz"] | -3;

  for (int i = 0; i < ALL_NUM_BANDS; i++) {
    if (i < MAX_NUM_BANDS) Equalizer[i] = doc["Eq"][i] | DEFAULT_EQ_BAND;
    Protection[i] = doc["Prot"][i] | DEFAULT_PROTECTION;
    Gain[i] = doc["Gn"][i] | DEFAULT_GAIN;
    AttackTime[i] = doc["Atk"][i] | DEFAULT_ATTACK_TIME;
    ReleaseTime[i] = doc["Rls"][i] | DEFAULT_RELEASE_TIME;
  }

  // Matrix mixer params
  for (int ch = 0; ch < NUM_IN_CH; ch++) {
    InTrim[ch] = doc["InTrim"][ch] | 0.0f;
    InMute[ch] = doc["InMute"][ch] | false;
  }
  for (int o = 0; o < NUM_OUT_CH; o++) {
    OutLvl[o] = doc["OutLvl"][o] | 0.0f;
    OutMute[o] = doc["OutMute"][o] | false;
    for (int i = 0; i < NUM_IN_CH; i++) {
      RouteGain[o][i] = doc["Route"][o][i] | ((i == o) ? 1.0f : 0.0f);
    }
  }
}

bool saveConfig() {
  DynamicJsonDocument doc(4096);

  doc["Il"] = InputLevel;
  doc["Ol"] = OutputLevel;
  doc["Clip"] = Clipper;
  doc["Comp"] = Compressor;
  doc["Sync"] = BandSync;
  doc["Mute"] = Mute;
  doc["Res1"] = Reserved1;
  doc["Res2"] = Reserved2;
  doc["Nb"] = NumBands;
  doc["PreE"] = PreEmphasis;
  doc["PostE"] = PostEmphasis;
  doc["Step"] = StepBy;
  doc["Echo"] = Echo;
  doc["Fqf"] = FiltersQFactor;
  doc["Tz"] = TimeZone;

  JsonArray Eq = doc.createNestedArray("Eq");
  JsonArray Prot = doc.createNestedArray("Prot");
  JsonArray Gn = doc.createNestedArray("Gn");
  JsonArray Atk = doc.createNestedArray("Atk");
  JsonArray Rls = doc.createNestedArray("Rls");

  for (int i = 0; i < ALL_NUM_BANDS; i++) {
    if (i < MAX_NUM_BANDS) Eq.add(Equalizer[i]);
    Prot.add(Protection[i]);
    Gn.add(Gain[i]);
    Atk.add(AttackTime[i]);
    Rls.add(ReleaseTime[i]);
  }

  // Matrix mixer params
  JsonArray inTrimArr = doc.createNestedArray("InTrim");
  JsonArray inMuteArr = doc.createNestedArray("InMute");
  for (int ch = 0; ch < NUM_IN_CH; ch++) {
    inTrimArr.add(InTrim[ch]);
    inMuteArr.add(InMute[ch]);
  }

  JsonArray outLvlArr = doc.createNestedArray("OutLvl");
  JsonArray outMuteArr = doc.createNestedArray("OutMute");
  for (int o = 0; o < NUM_OUT_CH; o++) {
    outLvlArr.add(OutLvl[o]);
    outMuteArr.add(OutMute[o]);
  }

  JsonArray routeArr = doc.createNestedArray("Route");
  for (int o = 0; o < NUM_OUT_CH; o++) {
    JsonArray row = routeArr.createNestedArray();
    for (int i = 0; i < NUM_IN_CH; i++) {
      row.add(RouteGain[o][i]);
    }
  }

  String output;
  serializeJson(doc, output);

  if (writeFile(DEFAULT_CONFIG_FILENAME, output)) {
    Serial.println("Config saved!");
    return true;
  } else {
    Serial.println("Config save FAILED!");
    return false;
  }
}

void audioTask(void *pvParameters) {
  esp_task_wdt_add(NULL);

  while (true) {
    // Read both I2S ports
    i2sCodec0.readBlock();
    i2sCodec1.readBlock();

    for (int i = 0; i < AudioDriver::BufferSize; i++) {
      // --- Read 4 inputs ---
      float in[4];
      in[0] = i2sCodec0.readSample(i, 0);  // PCM1808 #1 L
      in[1] = i2sCodec0.readSample(i, 1);  // PCM1808 #1 R
      in[2] = i2sCodec1.readSample(i, 0);  // PCM1808 #2 L
      in[3] = i2sCodec1.readSample(i, 1);  // PCM1808 #2 R

      // --- Per-input processing ---
      for (int ch = 0; ch < NUM_IN_CH; ch++) {
        in[ch] *= InTrimLinear[ch];
        if (InMute[ch]) in[ch] = 0.0f;
        in[ch] *= InputLevelLinear;
      }

      // --- Matrix mixer: sum into 4 output busses ---
      float out[4] = {0, 0, 0, 0};
      for (int o = 0; o < NUM_OUT_CH; o++) {
        for (int ch = 0; ch < NUM_IN_CH; ch++) {
          out[o] += in[ch] * RouteGain[o][ch];
        }
      }

      // --- Outputs 0,1: full multiband DSP (existing chain) ---
      outputSampleR = 0.0f;
      outputSampleL = 0.0f;

      inputSampleR = out[0];
      inputSampleL = out[1];

      if (Compressor) {
        for (int j = 0; j < NumBands; j++) {
          Band_R[j] = dspBAND[j].process(inputSampleR, 0);
          Band_L[j] = dspBAND[j].process(inputSampleL, 1);

          if (Band_R[j] > VuArrValue[j]) VuArrValue[j] = Band_R[j];

          Band_R[j] *= Equalizer_Linear[j];
          Band_L[j] *= Equalizer_Linear[j];

          comp_st[j].process(Band_L[j], Band_R[j]);

          Band_R[j] *= Post_Emphasis_Linear[j];
          Band_L[j] *= Post_Emphasis_Linear[j];

          outputSampleR += Band_R[j];
          outputSampleL += Band_L[j];
        }

        if (NumFmFilters > 0) {
          outputSampleR = dspFmFilter.process(outputSampleR, 0);
          outputSampleL = dspFmFilter.process(outputSampleL, 1);
        }

        if (Echo > 0.0f) {
          outputSampleR = dspDelay.process(outputSampleR, 0);
          outputSampleL = dspDelay.process(outputSampleL, 1);
        }
      } else {
        outputSampleR = inputSampleR;
        outputSampleL = inputSampleL;
        outputSampleR *= VolCompensationLinear;
        outputSampleL *= VolCompensationLinear;
      }

      if (Reserved1) limiter_st.process(outputSampleL, outputSampleR);

      outputSampleR = constrain(outputSampleR, Clipper_Linear_n, Clipper_Linear_p);
      outputSampleL = constrain(outputSampleL, Clipper_Linear_n, Clipper_Linear_p);

      if (Mute || ForcedMute) {
        outputSampleR = 0.f;
        outputSampleL = 0.f;
      } else {
        outputSampleR *= OutputLevelLinearR;
        outputSampleL *= OutputLevelLinearL;
      }

      // --- Outputs 2,3: simple level ---
      out[2] = constrain(out[2], Clipper_Linear_n, Clipper_Linear_p);
      out[3] = constrain(out[3], Clipper_Linear_n, Clipper_Linear_p);

      if (OutMute[2]) out[2] = 0.0f;
      else out[2] *= OutLvlLinear[2];

      if (OutMute[3]) out[3] = 0.0f;
      else out[3] *= OutLvlLinear[3];

      // --- Write to both DACs ---
      i2sCodec0.writeStereoSample(outputSampleR, outputSampleL, i);
      i2sCodec1.writeStereoSample(out[3], out[2], i);
    }

    i2sCodec0.writeBlock();
    i2sCodec1.writeBlock();
    esp_task_wdt_reset();
  }
}

void serialCommTask(void *pvParameters) {
  esp_task_wdt_add(NULL);
  const TickType_t xDelay = 10 / portTICK_PERIOD_MS;

  while (true) {
    commandInterpreter();
    esp_task_wdt_reset();
    vTaskDelay(xDelay);
  }
}
