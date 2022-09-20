#include "stdout.h"

void StandardOutput(String message)
{
  if(Serial) Serial.print(message);
  SDState.stdoutFile = SD.open("stdout.txt", FILE_WRITE);
  if (SDState.stdoutFile) {
    SDState.stdoutFile.print(message);
    SDState.stdoutFile.close();
  } else if(Serial) Serial.println("error opening stdout.txt");
}

void initSDCard()
{
  Serial.print("\n######## INIT SD CARD ############################\n");
  // see if the card is present and can be initialized:
  if (!SD.begin(SDState.ChipSelect)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  SDState.stdoutFile = SD.open("stdout.txt", FILE_WRITE);
  if (SDState.stdoutFile) {
    SDState.stdoutFile.print("\n######## INIT SD CARD ############################ - New Power Cycle\n");
    SDState.stdoutFile.close();
  } else Serial.println("error opening stdout.txt"); 
  StandardOutput("Card initialized.\n");
  if(!SD.exists("log.txt")) // Add heading line
  {
    SDState.SDString = "dt,usage(Wh),peak(W)";
    SDState.logFile = SD.open("log.txt", FILE_WRITE);
    if (SDState.logFile) {
      SDState.logFile.println(SDState.SDString);
      SDState.logFile.close();
    } else StandardOutput("error opening log.txt"); 
  }
  StandardOutput("##################################################\n\n");
}