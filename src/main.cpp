#include <Arduino.h>
#include "secrets.h"
#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include "SAMDTimerInterrupt.h"
#include "SAMD_ISR_Timer.h"
#include <SD.h>
#include <BlynkSimpleWiFiNINA.h>

#include "stdout.h"
#include "comm.h"
#include "measure.h"

SAMDTimer ITimer(TIMER_TC3);
SAMD_ISR_Timer ISR_Timer;
#define HW_TIMER_INTERVAL_MS      1
#define TIMER_INTERVAL_1MS       1L
BlynkTimer timer;

// Wifi Credentials
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
char auth[] = BLYNK_AUTH_TOKEN;
char api_key[] = API_KEY;
int status = WL_IDLE_STATUS;     // the WiFi radio's status
uint8_t lastLoopMin = 0;
uint8_t thisLoopMin = 0;
bool postedFlag = false;

// API Credentialss
char server[] = "21593698.pythonanywhere.com";
char dt_server[] = "worldtimeapi.org";

WiFiClient client;
APIStateTemplate APIState;
SDStateTemplate SDState;
MeasurementStateTemplate MeasurementState;

void setupWiFi()
{
  StandardOutput("\n##### WiFi Setup #################################\n");
  if (WiFi.status() == WL_NO_MODULE) {
    StandardOutput("Communication with WiFi module failed!\n");
    while (true);
  }
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    StandardOutput("Please upgrade the firmware!\n");
  }
  
  while (WiFi.status() != WL_CONNECTED) {
    StandardOutput("Attempting to connect to WPA SSID: " + String(ssid) + "\n");
    WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network:
    delay(5000);
  }
  StandardOutput("Connected to: " + String(ssid) + "\n");
  Blynk.virtualWrite(V6, 1);
  StandardOutput("##################################################\n\n");
}

void maintainWiFi() {
  if(WiFi.status() != WL_CONNECTED) {
    Blynk.virtualWrite(V6, 0);
    StandardOutput("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
    StandardOutput("Trying to re-establish a WiFi connection on " + getCurrentDateTimeString() + "...\n");
    WiFi.end();
    WiFi.begin(ssid, pass);
    StandardOutput("Waiting 5 seconds...\n");
    delay(5000);
    if(WiFi.status() == WL_CONNECTED){
      StandardOutput("Connected to: " + String(ssid) + "\n");
      Blynk.virtualWrite(V6, 1);
    } else {
      StandardOutput("Still not connected.\n");
    }
    StandardOutput("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n\n");
  }
}

void TimerHandler(void)
{
  ISR_Timer.run();
}

void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V2, millis() / 1000);
  Blynk.virtualWrite(V5, MeasurementState.PrmsAverage);
}

void setupHardwareTimer()
{
    StandardOutput("\n######## START HARDWARE TIMER ####################\n");
  // Interval in millisecs
  if (ITimer.attachInterruptInterval_MS(HW_TIMER_INTERVAL_MS, TimerHandler))
  {
    StandardOutput("Starting ITimer OK\n");
  }
  else
  {
    StandardOutput("Can't set ITimer. Select another freq. or timer\n");
    while(1);
  }
  StandardOutput("##################################################\n\n");

  ISR_Timer.setInterval(TIMER_INTERVAL_1MS,  readCurrent);
}

void initBlynk()
{
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, myTimerEvent);
  Blynk.virtualWrite(V6, 0);
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  initBlynk();
  initSDCard();
  setupWiFi();
  updateSystemDateTime(); // Requires a Connection
  setupHardwareTimer();
  lastLoopMin = minute();
}

void loop() {
  thisLoopMin = minute();
  if(thisLoopMin % 30 == 0 && !postedFlag)
  {
    preparePostData();
    postData(true);
    postBacklog();
    updateSystemDateTime();
    postedFlag = true;
  } else if (thisLoopMin % 30 != 0 && postedFlag) postedFlag = false;
  maintainWiFi();
  Blynk.run();
  timer.run();
//  lastLoopMin = thisLoopMin;
}

