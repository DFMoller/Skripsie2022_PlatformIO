// BLYNK
#define BLYNK_TEMPLATE_ID "TMPLf5VKQGmB"
#define BLYNK_DEVICE_NAME "Skripsie2022"
#define BLYNK_FIRMWARE_VERSION        "0.1.21"
#define BLYNK_PRINT Serial
#define APP_DEBUG
#include "BlynkEdgent.h"

#include <Arduino.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <SD.h>

#include "stdout.h"
#include "comm.h"
#include "measure.h"
#include "timing.h"
#include "secrets.h"
#include "BlynkEvent.h"
#include "System.h"

BlynkTimer timer;
BlynkEventStateTemplate BlynkEventState;

char api_key[] = API_KEY;
int status = WL_IDLE_STATUS;     // the WiFi radio's status
uint8_t lastLoopMin = 0;
uint8_t thisLoopMin = 0;
bool postedFlag = false;
uint16_t sampleRate = 1000;
uint16_t prescaler = 64;
uint8_t ten_ms_counter = 0;
uint8_t blynk_event_count = 0;
uint8_t today = 0;

// API Credentialss
char flask_server[] = "21593698.pythonanywhere.com";
char dt_server[] = "worldtimeapi.org";

// Debug flag
bool debug = true ;  

SystemStateTemplate SystemState;
WiFiClient client;
APIStateTemplate APIState;
SDStateTemplate SDState;
MeasurementStateTemplate MeasurementState;

void BlynkEventTrigger()
{
  if(BlynkEventState.message != "") Blynk.logEvent(BlynkEventState.event, BlynkEventState.message);
  else Blynk.logEvent(BlynkEventState.event);
  blynk_event_count += 1;
  BlynkEventState.message = "";
  BlynkEventState.event = "";
  BlynkEventState.flag = false;
}

void Timer5_Handler() {
  readCurrent(sampleRate, debug);
  ten_ms_counter ++;
  // Call indicator_run() every 10ms
  if (ten_ms_counter >= sampleRate/100)
  {
    uint32_t time_before = micros();
    indicator_run();
    Serial.print(micros()-time_before);
    Serial.println("us");
    ten_ms_counter = 0;
  }
  TC5->COUNT32.INTFLAG.bit.MC0 = 1; //Writing a 1 to INTFLAG.bit.MC0 clears the interrupt so that it will run again
}

void TC5_Initialize()
{
  // if(tcConfigure(sampleRate, prescaler)) { // Configure TC5 to run at specified speed
  //   tcStartCounter(); //starts the timer
  // } else StandardOutput("Unadble to Start Timer TC5!\n");
  StandardOutput("############## TC5 Timer Setup ###################\n");
  MyTimer5.begin(sampleRate);
  StandardOutput("TC5 begin() called\n");
  MyTimer5.attachInterrupt(Timer5_Handler);
  StandardOutput("TC5 interrupt attached\n");
  MyTimer5.start();
  StandardOutput("TC5 Started\n");
  StandardOutput("##################################################\n\n");
}

void BlynkTimerHandler()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V0, millis() / 1000);
  Blynk.virtualWrite(V1, MeasurementState.PRMS);
  Blynk.virtualWrite(V2, getCurrentDateTimeString());
  Blynk.virtualWrite(V3, MeasurementState.frequency);
  Blynk.virtualWrite(V4, second());
  Blynk.virtualWrite(V5, blynk_event_count);
}

void initBlynk()
{
  StandardOutput("############### Blynk Init #######################\n");
  BlynkEdgent.begin();
  StandardOutput("BlynkEdgent.begin() Executed\n");
  timer.setInterval(1000L, BlynkTimerHandler);
  StandardOutput("Blynk timer interval set\n");
  BlynkEdgent.run();
  StandardOutput("BlynkEdgent.run() Called in Setup\n");
  StandardOutput("##################################################\n\n");

}

void debug_setup()
{
    pinMode(17, OUTPUT);
    pinMode(18, OUTPUT);
    digitalWrite(17, 0);
    digitalWrite(18, 0);

}

void maintainDatetime()
{
  if(!APIState.datetime_updated && (millis() - SystemState.ms_last_dt_update) > 300000) // 5 minutes
  {
    updateSystemDateTime();
  }
}

void logSerialStatus()
{
  if(Serial) Blynk.logEvent("field_setup", "Setup with Serial Communication");
  else Blynk.logEvent("field_setup", "Setup without Serial Communication");
  blynk_event_count += 1;
}

void setup() {
  if (debug)
  {
    debug_setup();
  }
  Serial.begin(115200);
  uint32_t serial_start = millis();
  while(!Serial && (millis()-serial_start) < 15000);
  initSDCard();
  set_adc_prescaler(); // Do this before starting interrupt
  TC5_Initialize(); // Setup Interupts
  initBlynk(); // Must be after TC5 is set above
  while(WiFi.status() != WL_CONNECTED || !Blynk.connected())
  {
    BlynkEdgent.run();
  }
  updateSystemDateTime(); // Requires a Connection
  logSerialStatus();
  postBacklog();
  today = day();
  StandardOutput("############# Main Loop Starting... ##############\n\n");
}

void loop() {
  thisLoopMin = minute();
  maintainDatetime();
  if(thisLoopMin % 30 == 0 && !postedFlag && APIState.datetime_updated)
  {
    preparePostData();
    postData(true);
    postBacklog();
    updateSystemDateTime();
    postedFlag = true;
  } else if (thisLoopMin % 30 != 0 && postedFlag) postedFlag = false;
  if(day() > today)
  {
    blynk_event_count = 0;
    today = day();
  }
  if (BlynkEventState.flag)
  {
    BlynkEventTrigger();
  }
  BlynkEdgent.run();
  timer.run();
}

