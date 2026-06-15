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
  IDX_HELP,
  // Matrix mixer
  IDX_IN_TRIM = 22,
  IDX_IN_MUTE,
  IDX_ROUTE_MATRIX,
  IDX_OUT_LVL,
  IDX_OUT_MUTE
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
  msg += "\n--- Matrix Mixer ---\n";
  msg += left(String(IDX_IN_TRIM), 2) + ". Input Trim (enter ch 0-3):\n";
  for (int ch = 0; ch < NUM_IN_CH; ch++)
    msg += "   Ch" + String(ch) + " = " + String(InTrim[ch], 1) + " dB" + (InMute[ch] ? " [MUTED]" : "") + "\n";
  msg += left(String(IDX_ROUTE_MATRIX), 2) + ". Route (enter: o i gain):\n";
  msg += "   OUT\\IN";
  for (int i = 0; i < NUM_IN_CH; i++) msg += "    IN" + String(i);
  msg += "\n";
  for (int o = 0; o < NUM_OUT_CH; o++) {
    msg += "     OUT" + String(o);
    for (int i = 0; i < NUM_IN_CH; i++)
      msg += "  " + String(RouteGain[o][i], 2);
    msg += "\n";
  }
  msg += left(String(IDX_OUT_LVL), 2) + ". Output Level (enter ch 0-3):\n";
  for (int ch = 0; ch < NUM_OUT_CH; ch++)
    msg += "   Ch" + String(ch) + " = " + String(OutLvl[ch], 1) + " dB" + (OutMute[ch] ? " [MUTED]" : "") + "\n";
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
  Serial.println("\nMatrix Mixer:");
  Serial.print("  Inputs: ");
  for (int ch = 0; ch < NUM_IN_CH; ch++) {
    Serial.printf("Ch%d:%.0fdB%s ", ch, InTrim[ch], InMute[ch]?"M":"");
  }
  Serial.println();
  Serial.print("  Outputs: ");
  for (int ch = 0; ch < NUM_OUT_CH; ch++) {
    Serial.printf("Ch%d:%.0fdB%s ", ch, OutLvl[ch], OutMute[ch]?"M":"");
  }
  Serial.println();
  Serial.println("  Routing (out\\in):");
  for (int o = 0; o < NUM_OUT_CH; o++) {
    Serial.printf("    Out%d:", o);
    for (int i = 0; i < NUM_IN_CH; i++) {
      Serial.printf(" %.2f", RouteGain[o][i]);
    }
    Serial.println();
  }
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
  static int matrixStep = 0;  // 0=idle, 1=inTrim ch, 2=inTrim val, 3=outLvl ch, 4=outLvl val, 5=route o, 6=route i, 7=route gain
  static int matrixCh = 0;
  static int matrixO = 0, matrixI = 0;
  int m = MAX_NUM_BANDS;

  if (Serial.available()) {
    String text = Serial.readStringUntil('\n');
    text.trim();
    if (text.length() == 0) return;

    // Handle matrix sub-steps
    if (matrixStep > 0) {
      if (matrixStep == 1) {  // waiting for input channel
        matrixCh = text.toInt();
        if (matrixCh < 0 || matrixCh >= NUM_IN_CH) {
          Serial.println("Invalid channel (0-" + String(NUM_IN_CH-1) + ")");
          matrixStep = 0;
          return;
        }
        Serial.print("Enter trim for Ch" + String(matrixCh) + " [" + String(MIN_INPUT_LEVEL) + ".." + String(MAX_INPUT_LEVEL) + "] dB:");
        matrixStep = 2;
        return;
      }
      if (matrixStep == 2) {  // waiting for input trim value
        float v = text.toFloat();
        if (v >= MIN_INPUT_LEVEL && v <= MAX_INPUT_LEVEL) {
          InTrim[matrixCh] = v;
          commitConfig();
          Serial.println("Ch" + String(matrixCh) + " trim = " + String(v, 1) + " dB");
        } else {
          Serial.println("Out of range");
        }
        matrixStep = 0;
        lastS = "";
        Serial.println(getCommands());
        return;
      }
      if (matrixStep == 3) {  // waiting for output channel
        matrixCh = text.toInt();
        if (matrixCh < 0 || matrixCh >= NUM_OUT_CH) {
          Serial.println("Invalid channel (0-" + String(NUM_OUT_CH-1) + ")");
          matrixStep = 0;
          return;
        }
        Serial.print("Enter level for Out" + String(matrixCh) + " [" + String(MIN_OUTPUT_LEVEL) + ".." + String(MAX_OUTPUT_LEVEL) + "] dB:");
        matrixStep = 4;
        return;
      }
      if (matrixStep == 4) {  // waiting for output level
        float v = text.toFloat();
        if (v >= MIN_OUTPUT_LEVEL && v <= MAX_OUTPUT_LEVEL) {
          OutLvl[matrixCh] = v;
          commitConfig();
          Serial.println("Out" + String(matrixCh) + " level = " + String(v, 1) + " dB");
        } else {
          Serial.println("Out of range");
        }
        matrixStep = 0;
        lastS = "";
        Serial.println(getCommands());
        return;
      }
      if (matrixStep == 5) {  // waiting for route output
        matrixO = text.toInt();
        if (matrixO < 0 || matrixO >= NUM_OUT_CH) {
          Serial.println("Invalid output (0-" + String(NUM_OUT_CH-1) + ")");
          matrixStep = 0;
          return;
        }
        Serial.print("Enter input (0-" + String(NUM_IN_CH-1) + "):");
        matrixStep = 6;
        return;
      }
      if (matrixStep == 6) {  // waiting for route input
        matrixI = text.toInt();
        if (matrixI < 0 || matrixI >= NUM_IN_CH) {
          Serial.println("Invalid input (0-" + String(NUM_IN_CH-1) + ")");
          matrixStep = 0;
          return;
        }
        Serial.print("Enter gain (0.0-1.0):");
        matrixStep = 7;
        return;
      }
      if (matrixStep == 7) {  // waiting for route gain
        float v = text.toFloat();
        if (v >= 0.0f && v <= 1.0f) {
          RouteGain[matrixO][matrixI] = v;
          commitConfig();
          Serial.println("Route Out" + String(matrixO) + " <- In" + String(matrixI) + " = " + String(v, 2));
        } else {
          Serial.println("Out of range (0.0-1.0)");
        }
        matrixStep = 0;
        lastS = "";
        Serial.println(getCommands());
        return;
      }
    }

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
    else if (idx == IDX_IN_TRIM) {
      if (text.length() > 3) {
        // One-liner: "22 ch val"
        int s1 = text.indexOf(' ', 3);
        if (s1 > 0) {
          int ch = text.substring(3, s1).toInt();
          float v = text.substring(s1+1).toFloat();
          if (ch >= 0 && ch < NUM_IN_CH && v >= MIN_INPUT_LEVEL && v <= MAX_INPUT_LEVEL) {
            InTrim[ch] = v;
            commitConfig();
            Serial.println("Ch" + String(ch) + " trim = " + String(v, 1) + " dB");
          } else {
            Serial.println("Invalid ch or value");
          }
          lastS = "";
        }
      } else {
        // Interactive
        matrixStep = 1;
        Serial.print("Enter input channel (0-" + String(NUM_IN_CH-1) + "):");
      }
    } else if (idx == IDX_IN_MUTE) {
      if (text.length() > 3) {
        int ch = text.substring(3).toInt();
        if (ch >= 0 && ch < NUM_IN_CH) {
          InMute[ch] = !InMute[ch];
          commitConfig();
          Serial.println("In" + String(ch) + " mute: " + String(InMute[ch] ? "ON" : "OFF"));
        }
      }
      lastS = "";
    } else if (idx == IDX_OUT_LVL) {
      if (text.length() > 3) {
        int s1 = text.indexOf(' ', 3);
        if (s1 > 0) {
          int ch = text.substring(3, s1).toInt();
          float v = text.substring(s1+1).toFloat();
          if (ch >= 0 && ch < NUM_OUT_CH && v >= MIN_OUTPUT_LEVEL && v <= MAX_OUTPUT_LEVEL) {
            OutLvl[ch] = v;
            commitConfig();
            Serial.println("Out" + String(ch) + " level = " + String(v, 1) + " dB");
          } else {
            Serial.println("Invalid ch or value");
          }
          lastS = "";
        }
      } else {
        matrixStep = 3;
        Serial.print("Enter output channel (0-" + String(NUM_OUT_CH-1) + "):");
      }
    } else if (idx == IDX_OUT_MUTE) {
      if (text.length() > 3) {
        int ch = text.substring(3).toInt();
        if (ch >= 0 && ch < NUM_OUT_CH) {
          OutMute[ch] = !OutMute[ch];
          commitConfig();
          Serial.println("Out" + String(ch) + " mute: " + String(OutMute[ch] ? "ON" : "OFF"));
        }
      }
      lastS = "";
    } else if (idx == IDX_ROUTE_MATRIX) {
      // One-liner: "24 o i gain"
      int s1 = text.indexOf(' ', 3);
      int s2 = text.indexOf(' ', s1 + 1);
      if (s1 > 0 && s2 > 0) {
        int o = text.substring(3, s1).toInt();
        int i = text.substring(s1 + 1, s2).toInt();
        float v = text.substring(s2 + 1).toFloat();
        if (o >= 0 && o < NUM_OUT_CH && i >= 0 && i < NUM_IN_CH && v >= 0.0f && v <= 1.0f) {
          RouteGain[o][i] = v;
          commitConfig();
          Serial.println("Route Out" + String(o) + " <- In" + String(i) + " = " + String(v, 2));
        } else {
          Serial.println("Invalid params (o i gain)");
        }
        lastS = "";
      } else {
        // Interactive
        matrixStep = 5;
        Serial.print("Enter output (0-" + String(NUM_OUT_CH-1) + "):");
      }
    } else if (idx == IDX_SAVE) {
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
