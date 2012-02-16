#include "ShortMsg.h"


     void ShortMsg::set( byte lat0, byte lat1, byte lat2, byte lon0, byte lon1, byte lon2, unsigned int alt, unsigned int epochMinutes )
     {
         _lat[0] = lat0; _lat[1] = lat1; _lat[2] = lat2;
         _lon[0] = lon0; _lon[1] = lon1; _lon[2] = lon2;
         _alt = alt; 
         _epochMinutes = epochMinutes;

     }
     
#define SHORT_MSG_LEN 10 
    int ShortMsg::getFormattedLength()
    {
      return SHORT_MSG_LEN;
    }
     

    unsigned char * ShortMsg::getFormattedMsg()
    {
      unsigned char data[SHORT_MSG_LEN];
      data[0] = _lat[0]; 
      data[1] = _lat[1];
      data[2] = _lat[2];
      data[3] = _lon[0];
      data[4] = _lon[1];
      data[5] = _lon[2];
      data[6] = lowByte(_alt);
      data[7] = highByte(_alt);
      data[8] = lowByte(_epochMinutes);
      data[9] = highByte(_epochMinutes);
      return data; 
    }
    
