
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <avr/pgmspace.h>
#include <string.h>

#include "TimeKeeper.h"
#include "DebugMsg.h"
#include "CommCtrlrConfig.h"


 I2CCommMgr * DebugMsg::_i2cCommMgr;


 void DebugMsg::setI2CCommMgr(I2CCommMgr * i2cCommMgr)
 {
   _i2cCommMgr = i2cCommMgr;
 }

 int DebugMsg::msg(String sSource, char cLevel, char *str, ...)
 {
   char lstr[100];
   char lstr2[100];
   int chars;
   Serial.print(sSource); Serial.print("-"); Serial.print(cLevel); Serial.print(" "); Serial.print( TimeKeeper::getInstance().getFormattedTime() ); Serial.print("  ");
   va_list args;
   va_start(args, str);
   chars = vsnprintf(lstr, 100, str, args);
   if ( chars >= 100 ) lstr[100]=0; 
   Serial.println(lstr);
   
   if (_i2cCommMgr) //Function has be set
   {
     chars = snprintf(lstr2, 100,  "C%s", lstr);
     //chars = snprintf(lstr2, 100,  "C:%s->%s", TimeKeeper::getInstance().getFormattedTime(),lstr);
     (*_i2cCommMgr).I2CXmitMsg(i2cGroundSupportAddr, (byte*)lstr2, chars);
   } 
   
   return 0;
 }

 int DebugMsg::msg_P(String sSource, char cLevel, char *str, ...)
 {
   char lstr[100];
   char lstr2[100];
   int chars;
   Serial.print(sSource); Serial.print("-"); Serial.print(cLevel); Serial.print(" "); Serial.print( TimeKeeper::getInstance().getFormattedTime() ); Serial.print("  ");
   va_list args;
   va_start(args, str);
   chars = vsnprintf_P(lstr, 100, str, args);
   if ( chars >= 100 ) lstr[100]=0; 
   Serial.println(lstr);

   if (_i2cCommMgr) //Function has be set
   {
     //chars = snprintf(lstr2, 100,  "C:%s->%s", TimeKeeper::getInstance().getFormattedTime(),lstr);
     chars = snprintf(lstr2, 100,  "C%s", lstr);
     (*_i2cCommMgr).I2CXmitMsg(i2cGroundSupportAddr, (byte*)lstr2, chars);
   } 
   
   return 0;
 }



