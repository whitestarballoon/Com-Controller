#ifndef ShortMsg_h
#define ShortMsg_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <string.h>
#include "CommCtrlrConfig.h"

#include "BaseMsg.h"

#define SHORT_MSG_LEN 10 

class ShortMsg : public BaseMsg
{
  public:
          ShortMsg( byte lat0, byte lat1, byte lat2, byte lon0, byte lon1, byte lon2, unsigned int alt, unsigned int epochMinutes )
          {
                  set(lat0, lat1, lat2, lon0, lon1, lon2, alt, epochMinutes);
          }

          void set( byte lat0, byte lat1, byte lat2, byte lon0, byte lon1, byte lon2, unsigned int alt, unsigned int epochMinutes );
		  
#if 0
          boolean getFormattedMsg(unsigned char * data );
          unsigned char * getFormattedMsg();
          int getFormattedLength();
#endif
          
          virtual int getFormattedLength(void) { return SHORT_MSG_LEN; }
          virtual int getFormattedMsg(unsigned char * data, int data_sz);
          
  private:
          unsigned int _alt, _epochMinutes;
          byte _lat[3];
          byte _lon[3];

};

#endif

