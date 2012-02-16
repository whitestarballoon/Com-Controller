#ifndef SatQueue_h
#define SatQueue_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <string.h>
#include "CommCtrlrConfig.h"

#include "LongMsg.h"
#include "ShortMsg.h"


class SatQueue
{

public:
	static SatQueue& getInstance() 
        {
          static SatQueue instance;
          return instance;
        }
        void empty(void);
        int count(void);
        boolean read(LongMsg& longMsg);
        boolean write(LongMsg& longMsg);
        
        boolean isShortMsgAvail(void);
        void writeShort(byte lat0, byte lat1, byte lat2, byte lon0, byte lon1, byte lon2, unsigned int alt, unsigned int epochMinutes);
        ShortMsg readShort(void);
        
private:
        SatQueue();
        LongMsg _lmQueue[LongMsgQueueLen];
        int _readPtr;
        int _writePtr;
        int _writeAvailable;
        boolean _bShortMsgPresent;
        ShortMsg _shortMsg;
        
};

#endif




