#pragma once

#include <Arduino.h>

class BlynkEventStateTemplate{
  public:
    bool flag = false;
    String event = "";
    String message = "";
};