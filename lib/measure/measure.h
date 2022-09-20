#pragma once

#include <Arduino.h>

class MeasurementStateTemplate{
  public:
    int AnalogPin = A7;
    float PRMSBuff[2000];
    uint16_t PRMSBuff_Len = 0;
    float PRMS = 0;
    float PRMS_max = 0;
    uint16_t frequency = 0;
    String last_successful_post = "";
};

extern MeasurementStateTemplate MeasurementState;

void readCurrent(uint16_t sampleRate, bool debug);
void set_adc_prescaler();