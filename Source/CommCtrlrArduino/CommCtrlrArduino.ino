#define __WHITESTARBALLOON__
#include "CommCtrlrConfig.h"

#include "Iridium9602.h"
#include <Wire.h>


#include "TimeKeeper.h"


#include "BaseMsg.h"
#include "DebugMsg.h"
#include "SatQueue.h"
#include "CutDown.h"
#include "I2CCommMgr.h"
#include "SatCommMgr.h"


//These things should be later integrated properly VVVVV
//Why won't these work when accessed from Iridium9602::init function?!?!?!?!  FIX VVV
volatile int NetworkAvailableJustChanged = 0;
volatile int SatelliteNetworkAvailable = 0;

char incomingByte = 0;

// It turns out the WhiteStar Bus to ArduinoMega board connects I2C to the
// old arduino pins as well as the new ArduinoMega i2c pins which means you
// you can not have anything on A4 and A5 or it will conflict. The LCD currently
// uses those pins as well so the LCD has been disabled. The LCD should be moved to the 
// ground support board later.

// Declare an instance of the class
Iridium9602 iridium = Iridium9602(IRIDIUM_SERIAL_PORT);
SatCommMgr satCommMgr = SatCommMgr( iridium ); 

TimeKeeper myTimeKeeper = TimeKeeper::getInstance();

I2CCommMgr i2cCommMgr( satCommMgr );


void setup()
{
  
   
    // Set all pins as input for bootup
   for(int i=2;i<13;i++){
    pinMode(i, INPUT);
   }
  
  // Disable all pullup resistors
   for(int i=2;i<13;i++){ 
    digitalWrite(i, LOW);
   } 

   pinMode(A2,INPUT);
   pinMode(A3,INPUT);
   pinMode(A4,INPUT);
   pinMode(A5,INPUT);
   digitalWrite(A2,LOW);
   digitalWrite(A3,LOW);
   digitalWrite(A4,LOW);
   digitalWrite(A5,LOW);


   //Set I2C White Star Shield alternate pins to inputs, then disable pullup resistor
   pinMode(A8,INPUT);
   pinMode(A9,INPUT);
   pinMode(A10,INPUT);
   pinMode(A11,INPUT);
   digitalWrite(A8,LOW);
   digitalWrite(A9,LOW);
   digitalWrite(A10,LOW);
   digitalWrite(A11,LOW);

  //Set up sat modem capacitor voltage measure pin as input
   pinMode(A0, INPUT);
   digitalWrite(A0, LOW);
  
   //Turn Iridium 9602 modem off
   pinMode(pinModemPowerSwitch, OUTPUT);
   digitalWrite(pinModemPowerSwitch, LOW);


   IRIDIUM_SERIAL_PORT.begin(19200);  //Sat Modem Port
   Serial.begin(115200);	//Debug-Programming Port
   //Serial.print("Booting...\n");
 

  DebugMsg::msg_P("CC",'I',PSTR("WSB Comm Controller Reporting for Duty!"));

  
//  DebugMsg::msg("CC",'I',"MSG %02d %02d %08d",30,40, 50);
//  DebugMsg::msg("CC",'I',"MSG %02d",10);

  CutDown::initCutdown(&CUTDOWN_SERIAL_PORT);
  
  //i2cCommMgr.i2cInit();
  DebugMsg::setI2CCommMgr( &i2cCommMgr );
  DebugMsg::msg_P("CC",'I',PSTR("Will Send Debug out I2C"));
  
  //satCommMgr.satCommInit(  &i2cCommMgr );

  DebugMsg::msg_P("CC",'I',PSTR("CommCtrlr Boot Finished."));

}

int i=0;
int a=0;
int b=0;
void loop()
{
  //serialIn(Serial);

  // Update the TimeKeeper Clock
  myTimeKeeper.update( );
  i2cCommMgr.update();
  satCommMgr.update();

}


//Why won't these work when accessed from Iridium9602::init function?!?!?!?!  FIX VVV
void IridiumUpdateNetworkAvailable()
{
	NetworkAvailableJustChanged = true;
	SatelliteNetworkAvailable = true;
}

void initIridiumNetworkInterrupt()
{
	 attachInterrupt(1, IridiumUpdateNetworkAvailable, CHANGE);
}

extern "C" {
int atexit(void (*func)(void))
{
        return 0;
}
};
