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
	static SatQueue& getInstance();
        void empty(void);
        int count(void);
        boolean read(LongMsg & msg);
        boolean write(LongMsg & msg);
        boolean isMsgAvail(void)
        {
          return _writeAvailable != LongMsgQueueLen;
        }

#if 0
        boolean read(LongMsg& longMsg);
        boolean write(LongMsg& longMsg);
		boolean write(byte bStartAddrH, byte bStartAddrL, byte bEndAddrH, byte bEndAddrL);
        
        boolean isShortMsgAvail(void);
        void writeShort(byte lat0, byte lat1, byte lat2, byte lon0, byte lon1, byte lon2, unsigned int alt, unsigned int epochMinutes);
        ShortMsg readShort(void);
#endif
        
private:
        SatQueue();
        LongMsg _lmQueue[LongMsgQueueLen];
        int _readPtr;
        int _writePtr;
        int _writeAvailable;
        boolean _bShortMsgPresent;
        //ShortMsg _shortMsg;
        
};

#endif




