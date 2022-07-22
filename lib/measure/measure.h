#pragma once


class MeasurementStateTemplate{
  public:
    uint16_t BuffC[2000];
    uint8_t LenA = 0;
    uint8_t LenB = 0;
    uint16_t LenC = 0;
    int Millivolts = 0;
    uint16_t ReadMax = 0;
    uint16_t Prms = 0;
    uint32_t PrmsTotal = 0;
    uint16_t PrmsAverage = 0;
    int AnalogPin = A7;
    uint16_t SensorValue = 0;
};

extern MeasurementStateTemplate MeasurementState;

void readCurrent();