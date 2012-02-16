#ifndef ShortMsg_h
#define ShortMsg_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <string.h>
#include "CommCtrlrConfig.h"



class ShortMsg
{
  public:
          void set( byte lat0, byte lat1, byte lat2, byte lon0, byte lon1, byte lon2, unsigned int alt, unsigned int epochMinutes );
          int getFormattedLength();
          unsigned char * getFormattedMsg();
          
  private:
          unsigned int _alt, _epochMinutes;
          byte _lat[3];
          byte _lon[3];

};

#endif

