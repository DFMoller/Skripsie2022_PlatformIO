#include <Arduino.h>
#include "secrets.h"
#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <SD.h>
#include <BlynkSimpleWiFiNINA.h>
// #include <Blynk.h>

#include "stdout.h"
#include "comm.h"
#include "measure.h"
#include "timing.h"

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
uint16_t sampleRate = 2000;
uint16_t prescaler = 64;

// API Credentialss
char server[] = "21593698.pythonanywhere.com";
char dt_server[] = "worldtimeapi.org";

// Debug flag
bool debug = false;

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

void TC5_Handler (void) {
  readCurrent(sampleRate, debug);
  TC5->COUNT32.INTFLAG.bit.MC0 = 1; //Writing a 1 to INTFLAG.bit.MC0 clears the interrupt so that it will run again
}

void BlynkTimerHandler()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V2, millis() / 1000);
  Blynk.virtualWrite(V5, MeasurementState.PRMS);
}

void initBlynk()
{
  StandardOutput("xxxxxxxxxxxxxxx Blynk Init xxxxxxxxxxxxxxxxxxxxxxx\n");
  Blynk.begin(auth, ssid, pass);
  StandardOutput("Connected\n");
  // Blynk.config();
  timer.setInterval(1000L, BlynkTimerHandler);
  StandardOutput("Blynk timer interval set\n");
  Blynk.virtualWrite(V6, 0);
  StandardOutput("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n\n");
}

void debug_setup()
{
    pinMode(17, OUTPUT);
    pinMode(19, OUTPUT);
    digitalWrite(17, 0);
    digitalWrite(19, 0);
}

void setup() {
  double start_time = millis();
  if (debug)
  {
    debug_setup();
  }
  Serial.begin(115200);
  while(!Serial && (millis()-start_time) < 15000);
  initSDCard();
  setupWiFi();
  initBlynk();
  updateSystemDateTime(); // Requires a Connection
  set_adc_prescaler(); // Do this before starting interrupt
  if(tcConfigure(sampleRate, prescaler)) { //configure the timer to run at <sampleRate>Hertz
    tcStartCounter(); //starts the timer
  } else StandardOutput("Unadble to Start Timer TC5!\n");
  postBacklog();
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
}

