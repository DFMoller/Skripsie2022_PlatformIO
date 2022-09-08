#pragma once

#include <Arduino.h>

bool tcIsSyncing();
void tcStartCounter();
void tcReset();
void tcDisable();
bool tcConfigure(uint16_t sampleRate, uint16_t prescaler);