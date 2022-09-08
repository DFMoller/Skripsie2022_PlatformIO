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

void StandardOutput(String message);
void initSDCard();