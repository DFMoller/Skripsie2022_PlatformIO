#pragma once

#include <Arduino.h>
#include <SD.h>

class SDStateTemplate{
    public:
        File logFile;
        File stdoutFile;
        File backlogFile;
        String SDString = "";
        const int ChipSelect = 10;
};

extern SDStateTemplate SDState;
extern uint8_t blynk_event_count;

void StandardOutput(String message);
void initSDCard();