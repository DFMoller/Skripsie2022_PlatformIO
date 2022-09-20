#include <Arduino.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#include "comm.h"
#include "stdout.h"
#include "measure.h"

String getCurrentDateTimeString()
{
  uint16_t year_temp = year();
  uint8_t month_temp = month();
  uint8_t day_temp = day();
  uint8_t hour_temp = hour();
  uint8_t minute_temp = minute();
  uint8_t  buf[] = "xxxx-xx-xx xx:xx";
  buf[0] = ((year_temp/1000) % 10) + 48;
  buf[1] = ((year_temp/100) % 10) + 48;
  buf[2] = ((year_temp/10) % 10) + 48;
  buf[3] = (year_temp % 10) + 48;
  buf[5] = ((month_temp/10) % 10) + 48;
  buf[6] = (month_temp % 10) + 48;
  buf[8] = ((day_temp/10) % 10) + 48;
  buf[9] = (day_temp % 10) + 48;
  buf[11] = ((hour_temp/10) % 10) + 48;
  buf[12] = (hour_temp % 10) + 48;
  buf[14] = ((minute_temp/10) % 10) + 48;
  buf[15] = (minute_temp % 10) + 48;
  return (char*)buf;
}

void preparePostData()
{
  // Calculate Usage and Peak every 30 min. Print to Serial and to Server.
  APIState.peak = 0;
  APIState.usage = 0;
  APIState.datetime = getCurrentDateTimeString();
  float usage_accum = 0;
  APIState.peak = MeasurementState.PRMS_max; // W
  MeasurementState.PRMS_max = 0;
  for(int i = 0; i < MeasurementState.PRMSBuff_Len; i++)
  {
    usage_accum += MeasurementState.PRMSBuff[i]*(1.0/3600.0); // Wh
  }
  MeasurementState.PRMSBuff_Len = 0;
  APIState.usage = usage_accum; // Wh
}

int postToEndpoint(String endpoint, const JsonDocument& doc)
{
  if (client.connect(flask_server, 80)){
    client.print("POST ");
    client.print(endpoint);
    client.println(" HTTP/1.1");
    client.println("Host: 21593698.pythonanywhere.com");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(measureJson(doc));
    client.println("Content-Type: application/json");
    client.println(); //Terminate headers with a blank line
    serializeJson(doc, client); // Send JSON document in body
  } else{
      StandardOutput("Unable to connect to PythonAnywhere Flask server!\n");
      return 0;
  }
  StandardOutput("Response from Server:\n");
  String line = "";
  while (client.connected()) { 
   line = client.readStringUntil('\n'); 
   StandardOutput(line + "\n"); 
  }
  String event_str = "Device time at event: " + APIState.datetime;
  BlynkEventState.event = "data_posted";
  BlynkEventState.message = event_str;
  BlynkEventState.flag = true;
  return 1;
}

int postData(bool first)
{
  if (first) StandardOutput("\n##### Posting Data to Flask ######################\n");
  StandardOutput("For " + APIState.datetime + "\n");
  StaticJsonDocument<256> data_doc;
  data_doc["datetime"] = APIState.datetime;
  data_doc["api_key"] = api_key;
  data_doc["usage"] = APIState.usage;
  data_doc["peak"] = APIState.peak;
//  Serial.println("JSON Objects to be sent via POST request:");
//  serializeJsonPretty(usage_doc, Serial);
//  Serial.println();
//  serializeJsonPretty(peak_doc, Serial);
//  Serial.println();
  StandardOutput("Sending Data to /postData\n");
  int res = postToEndpoint("/postData", data_doc);
  data_doc.clear();
  if(res == 0){
    if (first) {
      BlynkEventState.event = "connection_error";
      BlynkEventState.message = "Unable to send data to Flask Server";
      BlynkEventState.flag = true;
      if(!SD.exists("backlog.txt")) // Add heading line
      {
        SDState.SDString = "dt,usage(Wh),peak(W)";
        SDState.backlogFile = SD.open("backlog.txt", FILE_WRITE);
        if (SDState.backlogFile) {
          SDState.backlogFile.println(SDState.SDString);
          SDState.backlogFile.close();
        } else StandardOutput("error opening backlog.txt");
      }
//      SDString = APIState.datetime + ',' + String(APIState.usage) + ',' + String(APIState.peak);
      SDState.SDString = APIState.datetime + ',' + "xxxxx" + ',' + "xxxxx";
      SDState.SDString[17] = ((APIState.usage/10000) % 10) + 48;
      SDState.SDString[18] = ((APIState.usage/1000) % 10) + 48;
      SDState.SDString[19] = ((APIState.usage/100) % 10) + 48;
      SDState.SDString[20] = ((APIState.usage/10) % 10) + 48;
      SDState.SDString[21] = ((APIState.usage/1) % 10) + 48;
      SDState.SDString[23] = ((APIState.peak/10000) % 10) + 48;
      SDState.SDString[24] = ((APIState.peak/1000) % 10) + 48;
      SDState.SDString[25] = ((APIState.peak/100) % 10) + 48;
      SDState.SDString[26] = ((APIState.peak/10) % 10) + 48;
      SDState.SDString[27] = ((APIState.peak/1) % 10) + 48;
      SDState.backlogFile = SD.open("backlog.txt", FILE_WRITE); 
      // if the file is available, write to it:
      if (SDState.backlogFile) {
        SDState.backlogFile.println(SDState.SDString);
        SDState.backlogFile.close();
        String event_str = "Backlog added: " + SDState.SDString;
        BlynkEventState.event = "backlog";
        BlynkEventState.message = event_str;
        BlynkEventState.flag = true;
      } else StandardOutput("error opening backlog.txt\n");
    }
    StandardOutput("##################################################\n\n");
    return 0;
  }
  else{
    MeasurementState.last_successful_post = APIState.datetime;
    StandardOutput("Successful write to server.\n");
//    SDString = APIState.datetime + ',' + String(APIState.usage) + ',' + String(APIState.peak);
    SDState.SDString = APIState.datetime + ',' + "xxxxx" + ',' + "xxxxx";
    SDState.SDString[17] = ((APIState.usage/10000) % 10) + 48;
    SDState.SDString[18] = ((APIState.usage/1000) % 10) + 48;
    SDState.SDString[19] = ((APIState.usage/100) % 10) + 48;
    SDState.SDString[20] = ((APIState.usage/10) % 10) + 48;
    SDState.SDString[21] = ((APIState.usage/1) % 10) + 48;
    SDState.SDString[23] = ((APIState.peak/10000) % 10) + 48;
    SDState.SDString[24] = ((APIState.peak/1000) % 10) + 48;
    SDState.SDString[25] = ((APIState.peak/100) % 10) + 48;
    SDState.SDString[26] = ((APIState.peak/10) % 10) + 48;
    SDState.SDString[27] = ((APIState.peak/1) % 10) + 48;
    SDState.logFile = SD.open("log.txt", FILE_WRITE); 
    // if the file is available, write to it:
    if (SDState.logFile) {
      SDState.logFile.println(SDState.SDString);
      SDState.logFile.close();
    } else StandardOutput("error opening log.txt\n");
  }
  if (first) StandardOutput("##################################################\n\n");
  return 1;
}

void postBacklog()
{
  if(SD.exists("backlog.txt"))
  {
    SDState.backlogFile = SD.open("backlog.txt", FILE_READ);
    if (SDState.backlogFile)
    {
      String line = "";
      uint8_t backlogCount = 0;
      int res;
      int successfulWrites = 0;
      char c;
      StandardOutput("\n################## BACKLOG #######################\n");
      uint8_t ln_num = 1;
      while(SDState.backlogFile.available())
      {
        line = SDState.backlogFile.readStringUntil('\n');
        if (ln_num > 1)
        {
          APIState.usage = line.substring(17, 22).toInt();
          APIState.peak = line.substring(23).toInt();
          APIState.datetime = line.substring(0, 16);
          StandardOutput("Line " + String(ln_num) + ": ");
          StandardOutput("DT: " + String(APIState.datetime) + " || ");
          StandardOutput("Usage: " + String(APIState.usage) + " || ");
          StandardOutput("Peak: " + String(APIState.peak) + "\n");
          res = postData(false);
          if (res == 1) successfulWrites ++;
          backlogCount ++;
        }
        ln_num ++;
      }
      SDState.backlogFile.close();
      if (backlogCount == successfulWrites && SD.exists("backlog.txt")) {
        SD.remove("backlog.txt");
        StandardOutput("backlog.txt deleted\n");
        BlynkEventState.event = "backlog";
        BlynkEventState.message = "backlog.txt deleted after " + String(successfulWrites) + " successful writes";
        BlynkEventState.flag = true;
      }
    }
    StandardOutput("##################################################\n\n");
  }
}

void updateSystemDateTime()
{
  SystemState.ms_last_dt_update = millis();
  StandardOutput("\n############### Update Sys Time ##################\n");
  StandardOutput("Starting connection to worldtimeapi server...\n"); 
  if (client.connect(dt_server, 80)) { 
    StandardOutput("Connected to worldtimeapi server.\n"); 
    // Make a HTTP request: 
    client.println("GET /api/timezone/Africa/Johannesburg HTTP/1.1");
    client.println("Host: worldtimeapi.org");
    client.println("Connection: close");
    client.println();
    int secondsWaited = 0;
    while (!client.available()) {
      delay(2000);
      secondsWaited += 2;
      StandardOutput("Waiting for data from worldtimeapi server...\n");
      if (secondsWaited > 20) {
        StandardOutput("Time Out! Took too long to respond\n");
        StandardOutput("##################################################\n\n");
        return;
      }
    }
    //  Check HTTP Status
    char httpStatus[32] = {0};
    client.readBytesUntil('\r', httpStatus, sizeof(httpStatus));
    if(strcmp(httpStatus + 9, "200 OK") != 0){
      StandardOutput("Unexpected Response: " + String(httpStatus + 9) + "\n");
      StandardOutput("##################################################\n\n");
      return;
    }
    //  Skip HTTP Headers
    char endOfHeaders[] = "\r\n\r\n";
    if(!client.find(endOfHeaders)){
      StandardOutput("Invalid Response\n");
      StandardOutput("##################################################\n\n");
      return;
    }
    //  Create filter for parsing Json
    StaticJsonDocument<16> filter;
    filter["unixtime"] = true;
    StaticJsonDocument<256> doc; // Document to store incoming json after being filtered (experimentally found that a size of at least 3 is required)
    DeserializationError error;
    error = deserializeJson(doc, client, DeserializationOption::Filter(filter));
    if (error) {
      StandardOutput("deserializeJson() failed: " + String(error.f_str()) + "\n");
      StandardOutput("##################################################\n\n");
      return;
    }
    StandardOutput("WorldTimeApi UnixTime Response:\n");
    StandardOutput("**JSON only printed in serial monitor, but api\n  call has been successful.\n");
    // Serial.println();
    // serializeJsonPretty(doc, Serial);
    // Serial.println();
    uint32_t unixtime = 0;
    unixtime = doc["unixtime"];
    unixtime += 120*60; // Plus two hours
    setTime(unixtime);
    APIState.last_post_time = unixtime;
    client.stop(); //  Disconnect
    filter.clear();
    doc.clear();
    StandardOutput("DT Set: " + getCurrentDateTimeString() + "\n");
    APIState.datetime_updated = true;
  } else { 
    StandardOutput("Unable to connect to WorldTimeApi server\n");
    BlynkEventState.event = "connection_error";
    BlynkEventState.message = "Unable to connect to WorldTimeApi server";
    BlynkEventState.flag = true;
  } 
  StandardOutput("##################################################\n\n");
}