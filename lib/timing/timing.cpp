#include <Arduino.h>
#include "timing.h"
#include "stdout.h"

// Function that is used to check if TC5 is done syncing
// returns true when it is done syncing
bool tcIsSyncing()
{
  return TC5->COUNT32.STATUS.reg & TC_STATUS_SYNCBUSY;
}

// This function enables TC5 and waits for it to be ready
void tcStartCounter()
{
  TC5->COUNT32.CTRLA.reg |= TC_CTRLA_ENABLE; // set the CTRLA register
  while (tcIsSyncing()); // wait until snyc'd
}

// Reset TC5
void tcReset()
{
  TC5->COUNT32.CTRLA.reg = TC_CTRLA_SWRST;
  while (tcIsSyncing());
  while (TC5->COUNT32.CTRLA.bit.SWRST);
}

// disable TC5
void tcDisable()
{
  TC5->COUNT32.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (tcIsSyncing());
}

// Output Copare Mode configured to requested parameters
bool tcConfigure(uint16_t sampleRate, uint16_t prescaler)
{
  StandardOutput("####### Configuring Timer Output Capture #########\n");
  // select the generic clock generator used as source to the generic clock multiplexer
  GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5));
  while (GCLK->STATUS.bit.SYNCBUSY);

  tcReset(); // reset TC5

  // Set Timer counter 5 Mode to 32 bits, it will become a 32bit counter
  TC5->COUNT32.CTRLA.reg |= TC_CTRLA_MODE_COUNT32;
  // Set TC5 waveform generation mode to 'match frequency'
  TC5->COUNT32.CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;

  // set prescaler to requested value
  if (prescaler == 1024)
  {
    TC5->COUNT32.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024 | TC_CTRLA_ENABLE; // it will divide GCLK_TC frequency by 1024
    // With psc=1024, timer frequency is reduced to 48 000 000 / 1024 = 46875 Hz for eg.
  }
  else if (prescaler == 256)
  {
    TC5->COUNT32.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV256 | TC_CTRLA_ENABLE; // it will divide GCLK_TC frequency by 1024
  }
  else if (prescaler == 64)
  {
    TC5->COUNT32.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV64 | TC_CTRLA_ENABLE; // it will divide GCLK_TC frequency by 1024
  }
  else if (prescaler == 16)
  {
    TC5->COUNT32.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV16 | TC_CTRLA_ENABLE; // it will divide GCLK_TC frequency by 1024
  }
  else if (prescaler == 8)
  {
    TC5->COUNT32.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV8 | TC_CTRLA_ENABLE; // it will divide GCLK_TC frequency by 1024
  }
  else if (prescaler == 4)
  {
    TC5->COUNT32.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV4 | TC_CTRLA_ENABLE; // it will divide GCLK_TC frequency by 1024
  }
  else if (prescaler == 2)
  {
    TC5->COUNT32.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV2 | TC_CTRLA_ENABLE; // it will divide GCLK_TC frequency by 1024
  }
  else if (prescaler == 1)
  {
    TC5->COUNT32.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1 | TC_CTRLA_ENABLE; // it will divide GCLK_TC frequency by 1024
  }
  else
  {
    StandardOutput("Incompatible prescaler value of " + String(prescaler) + "\n");
    StandardOutput("##################################################\n\n");
    return false;
  }
  StandardOutput("Prescaler set to " + String(prescaler) + "\n");

  // Setting ARR-1 Value
  TC5->COUNT32.CC[0].reg = (uint32_t)(SystemCoreClock / (prescaler * sampleRate) - 1);
  while (tcIsSyncing());
  // Serial.print("SystemCoreClock: ");
  // Serial.print(SystemCoreClock);
  // Serial.print(" || Sample Rate: ");
  // Serial.print(sampleRate);
  // Serial.print(" || prescaler: ");
  // Serial.println(prescaler);
  // Serial.print("(SystemCoreClock / (prescaler * sampleRate) - 1) = ");
  // Serial.println(SystemCoreClock / (prescaler * sampleRate) - 1);
  StandardOutput("ARR-1 Value Set\n");

  // Configure interrupt request
  NVIC_DisableIRQ(TC5_IRQn);
  NVIC_ClearPendingIRQ(TC5_IRQn);
  NVIC_SetPriority(TC5_IRQn, 0);
  NVIC_EnableIRQ(TC5_IRQn);
  StandardOutput("Interrupt Request Configured\n");

  // Enable the TC5 interrupt request
  StandardOutput("About to Enable TC5 Interrupt Requests\n");
  TC5->COUNT16.INTENSET.bit.MC0 = 1;
  StandardOutput("TC5 Interrupt Requests Enabled\n");
  while (tcIsSyncing()); // wait until TC5 is done syncing
  StandardOutput("TC5 Done Syncing\n");
  StandardOutput("##################################################\n\n");
  return true;
}