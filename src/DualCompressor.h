#pragma once
#include <Arduino.h>

const float COMP_LIMIT = 5.0f;

class DualCompressor {
  private:
    const float MAX_COMP_LEVEL = 70.0f;
    const float DEFAULT_COMP_LEVEL = 10.0f;
    const float MIN_COMP_LEVEL = -70.0f;
    const float DEFAULT_PROTECTION = 7.0f;
    const int32_t DEFAULT_ATTACK = 100;
    const int32_t DEFAULT_RELEASE = 1500;
    const float DEFAULT_STEP_BY = -10.0f;

    float _sample_rate;
    float _limit_p, _limit_n;
    float _protection_p, _protection_n;
    float _min_comp_level;
    int32_t _attack, _release;
    float _gain, _step;
    int32_t _attack_cnt = 0, _release_cnt = 0;
    float _level;

    float decibel_2_linear(float from) { return pow(10.0f, from / 10.0f); }
    float timeCalc(float uS, float Step) { return (uS * 10.0 * Step); }

  public:
    DualCompressor() {
      _protection_p = decibel_2_linear(DEFAULT_PROTECTION) * COMP_LIMIT;
      _protection_n = _protection_p * -1.0;
      _limit_p = COMP_LIMIT;
      _limit_n = _limit_p * -1.0;
      _attack = DEFAULT_ATTACK;
      _release = DEFAULT_RELEASE;
      _gain = decibel_2_linear(MAX_COMP_LEVEL);
      _level = decibel_2_linear(DEFAULT_COMP_LEVEL);
      _step = decibel_2_linear(DEFAULT_STEP_BY);
    }

    ~DualCompressor() {}

    void setParams(float Protection, float Attack_Time, float Release_Time,
                   float Gain, float Sample_Rate = 44100, float Step_By = 0.1f) {
      _sample_rate = Sample_Rate;
      float _cycle_time_seg = 1 / _sample_rate;
      _step = decibel_2_linear(Step_By);
      _attack = timeCalc(Attack_Time, _step) / _cycle_time_seg / 1000.0;
      _release = timeCalc(Release_Time, _step) / _cycle_time_seg / 1000.0;
      _protection_p = decibel_2_linear(Protection) * COMP_LIMIT;
      _protection_n = _protection_p * -1.0;
      _gain = decibel_2_linear(Gain);
      _min_comp_level = decibel_2_linear(MIN_COMP_LEVEL);
    }

    inline void process(float &ch_L_input_output, float &ch_R_input_output) {
      float valueL = ch_L_input_output * _level;
      float valueR = ch_R_input_output * _level;

      if ((valueL > _limit_p) && (valueR > _limit_p)) {
        if (_attack_cnt > _attack) {
          _attack_cnt = 0;
          _level -= _step;
          if (_level < _min_comp_level) _level = _min_comp_level;
        }
        if ((valueL > _protection_p) && (valueR > _protection_p)) {
          _level = _protection_p / fabs(ch_L_input_output);
        }
      } else {
        if (_release_cnt > _release) {
          _release_cnt = 0;
          _level += _step;
          if (_level > _gain) _level = _gain;
        }
      }

      _attack_cnt++;
      _release_cnt++;

      ch_L_input_output = ch_L_input_output * _level;
      ch_R_input_output = ch_R_input_output * _level;
    }
};
