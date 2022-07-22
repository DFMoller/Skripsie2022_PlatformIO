#include <Arduino.h>
#include "measure.h"


void readCurrent()
{
  if(MeasurementState.LenA >= 200)
  {
    // Calculate Prms for the last 200ms and store in buffB
    MeasurementState.Millivolts = MeasurementState.ReadMax*(3300/1023.0) - 1660 - 100;
    if(MeasurementState.Millivolts < 0) MeasurementState.Millivolts = 0;
    MeasurementState.Prms = (MeasurementState.Millivolts/33.0) * 2/sqrt(2) * 230; // Prms
    MeasurementState.PrmsTotal += MeasurementState.Prms;
    MeasurementState.LenB ++;
    MeasurementState.ReadMax = 0;
    MeasurementState.LenA = 0;
  }
  else if(MeasurementState.LenB >= 5)
  {
    // Calculate average Prms from buffB for the last second and store in buffC
    MeasurementState.PrmsAverage = MeasurementState.PrmsTotal / 5.0;
    MeasurementState.BuffC[MeasurementState.LenC] = MeasurementState.PrmsAverage;
    MeasurementState.PrmsTotal = 0;
    MeasurementState.LenC ++;
    MeasurementState.LenB = 0;
  }
  MeasurementState.SensorValue = analogRead(MeasurementState.AnalogPin);
  if(MeasurementState.SensorValue > MeasurementState.ReadMax) MeasurementState.ReadMax = MeasurementState.SensorValue;
  MeasurementState.LenA ++;
}