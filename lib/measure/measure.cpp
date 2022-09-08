#include <Arduino.h>
#include "measure.h"
#include "stdout.h"

int AdcVal = 0;
float Current = 0;
float lastCurrent = 0;
float dp = 0;
float adjusted_dp = 0;
float adjusted_last_dp = 0;
float last_dp = 0;
float Csq = 0;
float SquareSum = 0;
uint16_t SquareSumLen = 0;
uint16_t midpasses = 0;
uint16_t frequency = 0;
float RMSCurrent = 0;
float RMSPower = 0;

void readCurrent(uint16_t sampleRate, bool debug)
{
  if (debug) digitalWrite(19, 1);
  if(SquareSumLen >= sampleRate)
  {
    if (debug) digitalWrite(17, 1);
    frequency = midpasses/2.0;
    if(abs(frequency-50.0)/50.0 > 0.5) RMSCurrent = 0; // High frequency caused by noise in very low signals
    else RMSCurrent = sqrt(SquareSum/SquareSumLen); // Calculate Irms from sum of squares
    RMSPower = RMSCurrent * 230.0;
    if(RMSPower > MeasurementState.PRMS_max) MeasurementState.PRMS_max = RMSPower;
    MeasurementState.PRMS = RMSPower;
    if(MeasurementState.PRMSBuff_Len < 2000)
    {
      MeasurementState.PRMSBuff[MeasurementState.PRMSBuff_Len] = MeasurementState.PRMS;
      MeasurementState.PRMSBuff_Len += 1;
    }
    SquareSum = 0;
    SquareSumLen = 0;
    midpasses = 0;
    if (debug) digitalWrite(17, 0);
  }
  AdcVal = analogRead(MeasurementState.AnalogPin); // ADC value (0-1023)
  Current = (AdcVal * 3300/1023 - 1650)/33.0 * 100/50; // Current in Amps (0-100)
  dp = (Current+lastCurrent)/2.0; // average of last two measurements to reduce noise
  if(dp*last_dp < 0) midpasses += 1; // used to calculate frequency
  // for RMS calculation
  Csq = dp * dp;
  SquareSum += Csq;
  SquareSumLen += 1;
  // update history
  lastCurrent = Current;
  last_dp = dp;
  if (debug) digitalWrite(19, 0);
}

void set_adc_prescaler()
{
  StandardOutput("####### Reducing default ADC prescaler ###########\n");
  ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV32 |    // Divide Clock by 32
                   ADC_CTRLB_RESSEL_10BIT;         // 10 bits resolution as default
  while( ADC->STATUS.bit.SYNCBUSY == 1 );
  StandardOutput("Prescaler set to 32\n");
  StandardOutput("##################################################\n\n");
}