
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <avr/pgmspace.h>
#include <string.h>

#include "TimeKeeper.h"
#include "TextDisplay20x12.h"
#include "DebugMsg.h"

TextDisplay20x12 * DebugMsg::tdisp;

void DebugMsg::setDisplay(TextDisplay20x12 * t)
//void DebugMsg::setDisplay()
{
  tdisp = t;
}

 int DebugMsg::msg(String sSource, char cLevel, char *str, ...)
 {
   char lstr[100];
   int chars;
   Serial.print(sSource); Serial.print("-"); Serial.print(cLevel); Serial.print(" "); Serial.println( TimeKeeper::getInstance().getFormattedTime() );
   va_list args;
   va_start(args, str);
   chars = vsnprintf(lstr, 100, str, args);
   if ( chars >= 100 ) lstr[100]=0; 
   Serial.println(lstr);
   
   if (tdisp) {
     (*tdisp).displayStr((String)lstr);
     (*tdisp).newLine();
   }
   return 0;
 }

 int DebugMsg::msg_P(String sSource, char cLevel, char *str, ...)
 {
   char lstr[100];
   int chars;
   Serial.print(sSource); Serial.print("-"); Serial.print(cLevel); Serial.print(" "); Serial.println( TimeKeeper::getInstance().getFormattedTime() );
   va_list args;
   va_start(args, str);
   chars = vsnprintf_P(lstr, 100, str, args);
   if ( chars >= 100 ) lstr[100]=0; 
   Serial.println(lstr);
   
   if (tdisp) {
     (*tdisp).displayStr((String)lstr);
     (*tdisp).newLine();
   }
   return 0;
 }

