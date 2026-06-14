#pragma once
#include <Arduino.h>
#include "global_vars.h"

const int SEP_COUNT = 60;

const char TEXT_MUTE[] = "Mute";
const char TEXT_COMPRESSOR[] = "Compressor";
const char TEXT_UNKNOWN_CMD[] = "Error: Unknown Command!";
const char TEXT_SAVED[] = "Settings saved successfully!";
const char TEXT_ENABLED[] = "Enabled";
const char TEXT_DISABLED[] = "Disabled";
const char TEXT_PLS_CHOOSE[] = "Please choose an option:";

enum {
  IDX_INPUT_LEVEL = 1,
  IDX_OUTPUT_LEVEL,
  IDX_BALANCE,
  IDX_CLIPPER,
  IDX_COMPRESSOR,
  IDX_MUTE,
  IDX_NUM_BANDS,
  IDX_PRE_EMPHASIS,
  IDX_POST_EMPHASIS,
  IDX_STEP_BY,
  IDX_ECHO,
  IDX_PROTECTION,
  IDX_GAIN,
  IDX_ATTACK_TIME,
  IDX_RELEASE_TIME,
  IDX_FILTERS_Q_FACTOR,
  IDX_SAVE,
  IDX_RESET_TO_DEFAULT,
  IDX_EQ_BAND,
  IDX_STATUS,
  IDX_HELP
};

String Separator(int len) {
  String s = "\n";
  for (int i = 0; i < len; i++) s += "-";
  s += "\n";
  return s;
}

void printFilterFrequencies() {
  Serial.println(Separator(SEP_COUNT));
  Serial.println("Filter Frequencies:\n");
  for (int i = NumBands - 1; i >= 0; i--)
    Serial.println("BAND " + String(i + 1) + " " + getStrFilterFreq(i));
  Serial.println(Separator(SEP_COUNT));
}

String getBalance(float from) {
  float P = abs(50.0 - from) * 2.0;
  if (from < 50.0) return ("L + " + String(P, 1) + "%");
  else if (from > 50.0) return ("R + " + String(P, 1) + "%");
  else return (String(P, 1) + "%");
}

String getCommands() {
  String msg = "";
  int m = MAX_NUM_BANDS;
  msg += Separator(SEP_COUNT);
  msg += "DIGITAL AUDIO MIXER - ESP32-P4\nVERSION: " + VERSION;
  msg += Separator(SEP_COUNT);
  msg += String(TEXT_PLS_CHOOSE) + "\n";
  msg += Separator(SEP_COUNT);
  msg += " 0. Status\n";
  msg += left(String(IDX_INPUT_LEVEL), 2) + ". Input Level:       " + String(InputLevel, 2) + " dB\n";
  msg += left(String(IDX_OUTPUT_LEVEL), 2) + ". Output Level:      " + String(OutputLevel, 2) + " dB\n";
  msg += left(String(IDX_BALANCE), 2) + ". Balance:           " + getBalance(Balance) + "\n";
  msg += left(String(IDX_CLIPPER), 2) + ". Clipper:           " + String(Clipper, 2) + " dB\n";
  msg += left(String(IDX_COMPRESSOR), 2) + ". Compressor:        " + String(Compressor ? TEXT_ENABLED : TEXT_DISABLED) + "\n";
  msg += left(String(IDX_MUTE), 2) + ". Mute:              " + String(Mute ? TEXT_ENABLED : TEXT_DISABLED) + "\n";
  msg += left(String(IDX_NUM_BANDS), 2) + ". Num Bands:         " + String(NumBands) + "\n";
  msg += left(String(IDX_PRE_EMPHASIS), 2) + ". Pre-Emphasis:      " + String(PreEmphasis, 2) + " dB\n";
  msg += left(String(IDX_POST_EMPHASIS), 2) + ". Post-Emphasis:     " + String(PostEmphasis, 2) + " dB\n";
  msg += left(String(IDX_STEP_BY), 2) + ". Step:              " + String(StepBy, 1) + " dB\n";
  msg += left(String(IDX_ECHO), 2) + ". Echo:              " + String(Echo, 2) + "\n";
  msg += left(String(IDX_PROTECTION), 2) + ". Protection:        " + String(Protection[m], 2) + " dB\n";
  msg += left(String(IDX_GAIN), 2) + ". Gain:              " + String(Gain[m], 2) + " dB\n";
  msg += left(String(IDX_ATTACK_TIME), 2) + ". Attack Time:       " + String(AttackTime[m], 0) + " ms\n";
  msg += left(String(IDX_RELEASE_TIME), 2) + ". Release Time:      " + String(ReleaseTime[m], 0) + " ms\n";
  msg += left(String(IDX_FILTERS_Q_FACTOR), 2) + ". Filters Q Factor:  " + String(FiltersQFactor, 3) + "\n";
  msg += left(String(IDX_SAVE), 2) + ". Save\n";
  msg += left(String(IDX_RESET_TO_DEFAULT), 2) + ". Reset to Default\n";
  for (int i = 0; i < NumBands; i++) {
    int j = NumBands - i - 1;
    msg += String(IDX_EQ_BAND + i) + ". EQ Band(" + String(j + 1) + ") " +
           getStrFilterFreq(j) + " " + String(Equalizer[j], 2) + " dB\n";
  }
  return msg;
}

void showStatus() {
  Serial.println(Separator(SEP_COUNT));
  Serial.println("SYSTEM STATUS:");
  Serial.println(Separator(SEP_COUNT));
  Serial.print("Active Bands: "); Serial.println(NumBands);
  Serial.print("Sample Rate:  "); Serial.print(SampleRateFreq); Serial.println(" Hz");
  Serial.print("Compressor:   "); Serial.println(Compressor ? "ON" : "OFF");
  Serial.print("Mute:         "); Serial.println(Mute ? "YES" : "NO");
  Serial.print("Input Level:  "); Serial.print(InputLevel); Serial.println(" dB");
  Serial.print("Output Level: "); Serial.print(OutputLevel); Serial.println(" dB");
  Serial.print("Balance:      "); Serial.println(getBalance(Balance));
  Serial.print("Echo:         "); Serial.println(Echo);
  Serial.print("Pre-Emphasis: "); Serial.print(PreEmphasis); Serial.println(" dB");
  Serial.print("Post-Emphasis:"); Serial.print(PostEmphasis); Serial.println(" dB");
  Serial.print("Filters Q:    "); Serial.println(FiltersQFactor);
  Serial.print("SPIFFS:       "); Serial.println(isGoodFileSystem ? "OK" : "ERROR");
  Serial.println(Separator(SEP_COUNT));
}

void changeParam(String &returned_text, const String menu_label, const int menu_index,
                 String text, float &value, const float min_value,
                 const float max_value, const String unit) {
  static int pos = 0;
  if (pos == 0) {
    Serial.println("\nEnter value for " + menu_label + " [" +
                   String(min_value, 2) + " .. " + String(max_value, 2) + "] " + unit + ":");
    returned_text = String(menu_index);
    pos++;
  } else {
    float number = text.toFloat();
    if ((number >= min_value) && (number <= max_value)) {
      value = number;
      commitConfig();
      pos = 0;
      returned_text = "";
      Serial.println("Set to " + String(number, 2) + " " + unit + "\n");
      Serial.println(getCommands());
    } else {
      Serial.println("Error: " + String(number, 2) + " out of range [" +
                     String(min_value, 2) + " .. " + String(max_value, 2) + "]");
    }
  }
}

void changeIntParam(String &returned_text, const String menu_label, const int menu_index,
                    String text, int &value, const int min_value,
                    const int max_value, const String unit) {
  static int pos = 0;
  if (pos == 0) {
    Serial.println("\nEnter value for " + menu_label + " [" +
                   String(min_value) + " .. " + String(max_value) + "] " + unit + ":");
    returned_text = String(menu_index);
    pos++;
  } else {
    int number = text.toInt();
    if ((number >= min_value) && (number <= max_value)) {
      value = number;
      commitConfig();
      pos = 0;
      returned_text = "";
      Serial.println("Set to " + String(number) + " " + unit + "\n");
      Serial.println(getCommands());
    } else {
      Serial.println("Error: " + String(number) + " out of range [" +
                     String(min_value) + " .. " + String(max_value) + "]");
    }
  }
}

void commandInterpreter() {
  static String lastS = "";
  int m = MAX_NUM_BANDS;

  if (Serial.available()) {
    String text = Serial.readStringUntil('\n');
    text.trim();
    if (text.length() == 0) return;

    String S = text;
    if (!lastS.equals("")) S = lastS;

    if (text.equalsIgnoreCase("menu") || text.equalsIgnoreCase("help")) {
      lastS = "";
      Serial.println(getCommands());
      return;
    }

    if (text.equalsIgnoreCase("status") || text.equals("0")) {
      lastS = "";
      showStatus();
      return;
    }

    int idx = S.toInt();

    if (idx == IDX_INPUT_LEVEL)
      changeParam(lastS, "Input Level", IDX_INPUT_LEVEL, text, InputLevel, MIN_INPUT_LEVEL, MAX_INPUT_LEVEL, "dB");
    else if (idx == IDX_OUTPUT_LEVEL)
      changeParam(lastS, "Output Level", IDX_OUTPUT_LEVEL, text, OutputLevel, MIN_OUTPUT_LEVEL, MAX_OUTPUT_LEVEL, "dB");
    else if (idx == IDX_BALANCE)
      changeParam(lastS, "Balance", IDX_BALANCE, text, Balance, MIN_BALANCE, MAX_BALANCE, "%");
    else if (idx == IDX_CLIPPER)
      changeParam(lastS, "Clipper", IDX_CLIPPER, text, Clipper, MIN_CLIPPER, MAX_CLIPPER, "dB");
    else if (idx == IDX_COMPRESSOR) {
      Compressor = !Compressor;
      lastS = "";
      Serial.println(getCommands());
      Serial.println(String(TEXT_COMPRESSOR) + ": " + String(Compressor ? TEXT_ENABLED : TEXT_DISABLED));
    } else if (idx == IDX_MUTE) {
      Mute = !Mute;
      lastS = "";
      Serial.println(String(TEXT_MUTE) + ": " + String(Mute ? TEXT_ENABLED : TEXT_DISABLED));
    } else if (idx == IDX_NUM_BANDS)
      changeIntParam(lastS, "Num Bands", IDX_NUM_BANDS, text, NumBands, MIN_NUM_BANDS, MAX_NUM_BANDS, "");
    else if (idx == IDX_PRE_EMPHASIS)
      changeParam(lastS, "Pre-Emphasis", IDX_PRE_EMPHASIS, text, PreEmphasis, MIN_PRE_EMPHASIS, MAX_PRE_EMPHASIS, "dB");
    else if (idx == IDX_POST_EMPHASIS)
      changeParam(lastS, "Post-Emphasis", IDX_POST_EMPHASIS, text, PostEmphasis, MIN_POST_EMPHASIS, MAX_POST_EMPHASIS, "dB");
    else if (idx == IDX_STEP_BY)
      changeParam(lastS, "Step By", IDX_STEP_BY, text, StepBy, MIN_STEP_BY, MAX_STEP_BY, "dB");
    else if (idx == IDX_ECHO)
      changeParam(lastS, "Echo", IDX_ECHO, text, Echo, MIN_ECHO, MAX_ECHO, "");
    else if (idx == IDX_PROTECTION)
      changeParam(lastS, "Protection", IDX_PROTECTION, text, Protection[m], MIN_PROTECTION, MAX_PROTECTION, "dB");
    else if (idx == IDX_GAIN)
      changeParam(lastS, "Gain", IDX_GAIN, text, Gain[m], MIN_GAIN, MAX_GAIN, "dB");
    else if (idx == IDX_ATTACK_TIME)
      changeParam(lastS, "Attack Time", IDX_ATTACK_TIME, text, AttackTime[m], MIN_ATTACK_TIME, MAX_ATTACK_TIME, "ms");
    else if (idx == IDX_RELEASE_TIME)
      changeParam(lastS, "Release Time", IDX_RELEASE_TIME, text, ReleaseTime[m], MIN_RELEASE_TIME, MAX_RELEASE_TIME, "ms");
    else if (idx == IDX_FILTERS_Q_FACTOR)
      changeParam(lastS, "Filters Q Factor", IDX_FILTERS_Q_FACTOR, text, FiltersQFactor, MIN_FILTERS_Q_FACTOR, MAX_FILTERS_Q_FACTOR, "");
    else if (idx == IDX_SAVE) {
      lastS = "";
      if (saveConfig()) Serial.println(TEXT_SAVED);
      else Serial.println("Error saving file!");
    } else if (idx == IDX_RESET_TO_DEFAULT) {
      lastS = "";
      loadDefaultConfig();
      commitConfig();
      Serial.println(getCommands());
      Serial.println("Default settings loaded!");
    } else if ((idx >= IDX_EQ_BAND) && (idx < IDX_EQ_BAND + NumBands)) {
      int i = idx - IDX_EQ_BAND;
      i = NumBands - i - 1;
      String lbl = "EQ Band[" + String(i + 1) + "]";
      changeParam(lastS, lbl, idx, text, Equalizer[i], MIN_EQ_BAND, MAX_EQ_BAND, "dB");
    } else {
      lastS = "";
      Serial.println(TEXT_UNKNOWN_CMD);
      Serial.println("Type 'menu' for available commands.");
    }
  }
}
