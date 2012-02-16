
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <avr/pgmspace.h>
#include <string.h>

#include "TimeKeeper.h"

// This class uses the singleton pattern. Only one instance will ever exist
// It is brought in to existance by calling this getInstance()
// This method can be called as well but must be done within setup()

void TimeKeeper::setInitialSeconds(unsigned long ulSeconds)
{
  ulSecondsTick = ulSeconds;
}

void TimeKeeper::setLastMillis(unsigned long ulMillis)
{
  ulLastMillis = ulMillis;
}

// Updates the time and returns true if the time was incremented
// otherwise it return false
// This method should be called periodically to update the clock, it is not 
// dependant on when it is called just needs to be called before retrieving the time
// at the very least. Better to call in "loop"
boolean TimeKeeper::update()
{
  return update(millis());
}

boolean TimeKeeper::update(unsigned long ulMillis)
{
  //Serial.println("- - - - \n ST:");Serial.print(ulSecondsTick); Serial.print("   M:"); Serial.println(ulMillis);
  int iSecondsIncr = 0;
  if ( ulMillis < ulLastMillis) // Check for a roll over condition
  {
    iSecondsIncr =   ((4294967295ul - ulLastMillis) + ulMillis ) /1000;
  } else {
    iSecondsIncr = (ulMillis - ulLastMillis)/1000;
  }
  if (iSecondsIncr > 0)
  {
    ulSecondsTick += iSecondsIncr;
    ulLastMillis = ulMillis;
    return true;
  }
  return false;
}


String TimeKeeper::getFormattedTime()
{
  update();
  return getFormattedTime(ulSecondsTick);
}

// Static routing that format a seconds count in days::hrs:min:sec 0:00:00:00
String TimeKeeper::getFormattedTime(unsigned long ulSec)
{
    int iMin = (ulSec/60) % 60;
    int iHr = (ulSec / 3600 ) % 24;
    int iDay = (ulSec / 86400);

    char buffer[20];
    sprintf(buffer,"%d:%02d:%02d:%02ld",iDay,iHr, iMin, (ulSec%60));
    return buffer;
}


