#include "ShortMsg.h"


void ShortMsg::set( byte lat0, byte lat1, byte lat2, byte lon0, byte lon1, byte lon2, unsigned int alt, unsigned int epochMinutes )
{
        _lat[0] = lat0; _lat[1] = lat1; _lat[2] = lat2;
        _lon[0] = lon0; _lon[1] = lon1; _lon[2] = lon2;
        _alt = alt; 
        _epochMinutes = epochMinutes;

}

int ShortMsg::getFormattedMsg(unsigned char * data, int data_sz)
{
        if (data_sz > 0) 
                data[0] = _lat[0]; 
        if (data_sz > 1) 
                data[1] = _lat[1];
        if (data_sz > 2) 
                data[2] = _lat[2];
        if (data_sz > 3) 
                data[3] = _lon[0];
        if (data_sz > 4) 
                data[4] = _lon[1];
        if (data_sz > 5) 
                data[5] = _lon[2];
        if (data_sz > 6) 
                data[6] = lowByte(_alt);
        if (data_sz > 7) 
                data[7] = highByte(_alt);
        if (data_sz > 8) 
                data[8] = lowByte(_epochMinutes);
        if (data_sz > 9) 
                data[9] = highByte(_epochMinutes);

        return SHORT_MSG_LEN;
        //return data; 
}

