
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <avr/pgmspace.h>
#include <string.h>

#include "CommCtrlrConfig.h"
#include "CutDown.h"
#include "DebugMsg.h"
#include "I2CCommMgr.h"



/********
 *
 * Note, many of these repeat with for loops, for the purpose of not having to wait
 *   for the cutdown controller to respond.
 *
 ********/

HardwareSerial * CutDown::cdn;

//Cutdown Initialization
void CutDown::initCutdown(HardwareSerial * sPort)
{
  cdn = sPort;
  boolean respondFlag = false;	
  char tempin;
  DebugMsg::msg_P("CD",'I',PSTR("cdnInit."));
  for (byte m = 0; m<10; m++) {  // Look for response

    Serial2.println("!R");  //Reset deadman timer
    delay(100);
    if(Serial2.available()) {   // NewSoftSerial read will return -1 when nothing is received
      //A char has been returned
      tempin = Serial2.read();
      DebugMsg::msg_P("CD",'I',PSTR("Data RX: %s ( %0x )"), tempin, tempin);  
      if ('R' == tempin) {
        respondFlag = true;
      }
    }
  }
  if (respondFlag == false) {
    DebugMsg::msg_P("CD",'E',PSTR("CUTDOWN DEAD."));
  }
  delay(500);
  CmdSet(250);  // Immediately set the cutdown to maximum time to give time to work
}



//Cut down immediately
void CutDown::CutdownNOW()
{
  for (byte m = 0; m<10; m++) {
    Serial2.println("!CUTDOWNNOW");
    delay(100);
  }
  String packetBufferS;
  packetBufferS[0]=0;  //empty I2c Data

//FIX vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv  Need to figure out how to call the damn i2c comm manager functions here.  #$(*@#$@)(*&
// (*i2cCommMgr).I2CXmit(i2cFlightComputerAddr, 0x0F, packetBufferS, 0);//Send cutdown notice to flight computer
//FIX ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  DebugMsg::msg_P("CD",'W',PSTR("Cutdown NOW"));
}

//Heartbeat reset
void CutDown::ResetTimer() {
  for (byte i = 0; i<10; i++) {
    Serial2.println("!R");
    delay(100);
  }
  //Serial.println("cdn!R");
  DebugMsg::msg_P("CD",'I',PSTR("Deadman Timer Reset! Die another day."));
}

// Set deadman timer time in minutes, which also resets the timer
void CutDown::CmdSet(unsigned char deadManTime) {
  DebugMsg::msg_P("CD",'I',PSTR("Timer Set to %d Minutes."), deadManTime);
  
  // DO NOT TRY TO PRINT TO I2C IN THIS FUNCTION IF I2C HAS NOT YET INITIALIZED!  IT WILL FREEZE 
  //COMMCONTROLLER DURING BOOT SEQUENCE WHEN IT INITIALIZES CUTDOWN MODULE!
  for (byte i = 0; i<10; i++) {
    Serial2.print("!T");
    // Zero pad the value for ASCII numbers to cutdown controller
    if (deadManTime>99){
      Serial2.print(deadManTime,DEC);
    } 
    else if (deadManTime>9) {
      Serial2.print("0");
      Serial2.print(deadManTime,DEC);
    } 
    else {
      Serial2.print("00");
      Serial2.print(deadManTime,DEC);    
    }
    Serial2.println();
    delay(100);

    if(Serial2.available()) {   // NewSoftSerial read will return -1 when nothing is received
      //A char has been returned
      DebugMsg::msg_P("CD",'I',PSTR("Set confirmation: %02x"),Serial2.read());
    }

  }

}







