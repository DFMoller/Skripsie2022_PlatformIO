#pragma once

#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include "stdout.h"
#include "BlynkEvent.h"
#include "System.h"

class APIStateTemplate{
  public:
    String datetime;
    uint16_t usage = 1000;
    uint16_t peak = 500;
    uint32_t last_post_time = 0;
    uint16_t posting_interval = 10*60; // Every 10 minutes
    uint32_t seconds_passed = 0;
    bool datetime_updated = false;
};

extern SystemStateTemplate SystemState;
extern APIStateTemplate APIState;
extern SDStateTemplate SDState;
extern WiFiClient client;
extern BlynkEventStateTemplate BlynkEventState;
extern char flask_server[];
extern char dt_server[];
extern char api_key[];

String getCurrentDateTimeString();
void preparePostData();
int postToEndpoint(String endpoint, const JsonDocument& doc);
int postData(bool first);
void postBacklog();
void updateSystemDateTime();