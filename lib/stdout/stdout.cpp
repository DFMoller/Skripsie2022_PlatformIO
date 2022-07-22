// #include <Arduino.h>
#include "stdout.h"

void StandardOutput(String message)
{
  Serial.print(message);
  SDState.stdoutFile = SD.open("stdout.txt", FILE_WRITE);
  if (SDState.stdoutFile) {
    SDState.stdoutFile.print(message);
    SDState.stdoutFile.close();
  } else Serial.println("error opening stdout.txt");
}

void initSDCard()
{
  Serial.print("\n######## INIT SD CARD ############################\n");
  // see if the card is present and can be initialized:
  if (!SD.begin(SDState.ChipSelect)) {
    StandardOutput("Card failed, or not present\n");
    while (1);
  }
  SDState.stdoutFile = SD.open("stdout.txt", FILE_WRITE);
  if (SDState.stdoutFile) {
    SDState.stdoutFile.print("\n######## INIT SD CARD ############################ - New Power Cycle\n");
    SDState.stdoutFile.close();
  } else Serial.println("error opening stdout.txt"); 
  StandardOutput("card initialized.\n");
  if(!SD.exists("log.txt")) // Add heading line
  {
    SDState.SDString = "dt,usage(kWh),peak(W)";
    SDState.sentDataFile = SD.open("log.txt", FILE_WRITE);
    if (SDState.sentDataFile) {
      SDState.sentDataFile.println(SDState.SDString);
      SDState.sentDataFile.close();
    } else StandardOutput("error opening log.txt"); 
  }
  StandardOutput("##################################################\n\n");
}