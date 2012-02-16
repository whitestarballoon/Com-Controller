#define __WHITESTARBALLOON__
#include "CommCtrlrConfig.h"

#include "Iridium9602.h"
#include <Wire.h>
#include "RGB_GLCD.h"
#include "TimeKeeper.h"
#include "TextDisplay20x12.h"
#include "DebugMsg.h"
#include "SatQueue.h"
#include "CutDown.h"
#include "I2CCommMgr.h"
#include "SatCommMgr.h"

// Define Pins

#define pinRI 2
#define pinNA 3
#define pinDSR 6
#define pinModemPowerSwitch 4
#define pinBrownout 5
//define pinVoltageMeasure A0

char incomingByte = 0;

// Declare an instance of the class
TextDisplay20x12 myDisplay;

Iridium9602 iridium = Iridium9602(IRIDIUM_SERIAL_PORT);

SatCommMgr satCommMgr = SatCommMgr( iridium ); 

TimeKeeper myTimeKeeper = TimeKeeper::getInstance();

I2CCommMgr i2cCommMgr;


void setup()
{
  
    // Set all pins as input for bootup
   for(int i=2;i<13;i++){
    pinMode(i, INPUT);
   }
  
  // Disable all pullup resistors
   for(int i=0;i<13;i++){ 
    digitalWrite(i, LOW);
   } 

  //Set up sat modem capacitor voltage measure pin as input
   pinMode(A0, INPUT);
   digitalWrite(A0, LOW);
  
   //Shut modem off
   pinMode(pinModemPowerSwitch, OUTPUT);
   digitalWrite(pinModemPowerSwitch, LOW);
   //Wait for modem to fully power down - this is important delay!
   delay(2500);

   Serial1.begin(19200);
   Serial.begin(115200);
   Serial.print("Boot\n");
 
 
   //Turn modem on
   digitalWrite(pinModemPowerSwitch, HIGH);
   Serial.print("Modem Readiness:\n");
   delay(2000); 
 
  DebugMsg::setDisplay(&myDisplay);

  
  DebugMsg::msg_P("CC",'I',PSTR("Starting"));
  
  myDisplay.setTitle("WhiteStar Balloon");
  
  myDisplay.setStatusLeft("CommCtrl");
  myDisplay.setStatusRight("V0.05");
  myDisplay.initDisplay();
  delay(3000);
  
  myDisplay.setStatusLeft("Offline");
  //TextDisplay20x12::setStatusRight("");

  myDisplay.gotoXY(0,0);
  myDisplay.displayStr(" Waiting for data...");
  myDisplay.gotoXY(0,0);


//  DebugMsg::msg("CC",'I',"MSG %02d %02d %08d",30,40, 50);
//  DebugMsg::msg("CC",'I',"MSG %02d",10);

  CutDown::initCutdown(&CUTDOWN_SERIAL_PORT);

  i2cCommMgr.i2cInit();
  satCommMgr.satCommInit();
}

int i=0;
int a=0;
int b=0;
void loop()
{
  //serialIn(Serial);

  myDisplay.gotoXY(19,0); myDisplay.displayChar('|');
  // Update the TimeKeeper Clock
  if ( myTimeKeeper.update( ) ) 
  {
    myDisplay.setStatusRight(myTimeKeeper.getFormattedTime() );
  }

  myDisplay.gotoXY(19,0); myDisplay.displayChar('/');
  i2cCommMgr.update();

  myDisplay.gotoXY(19,0); myDisplay.displayChar('-');
  satCommMgr.update();

  
  myDisplay.gotoXY(19,0); myDisplay.displayChar('\\');
  
  if (Serial1.available()>0)
  {
    incomingByte = Serial1.read();
    Serial.print(incomingByte);
  }

/*
  i++;
  if (i > 32000) 
  {
    i = 0;
    a++;
    if ( a >= 10 )
    {
      a = 0;
      if (b == 0)
      {
        b=1;
        Serial1.print("AT+CCLK\r");
      } else {
        b=0;
        Serial1.print("AT+SBDGW\r");
      }
    } else {
      Serial1.print("AT+CSQ\r");
    }
    delay(1000);
  }
*/

}

/*
void serialIn (HardwareSerial s)
{
  if (s.available() )
  {
    myDisplay.displayChar( (char) s.read() );
  }
}
*/



