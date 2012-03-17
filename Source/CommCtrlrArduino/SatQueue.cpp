
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <avr/pgmspace.h>
#include <string.h>

#include "SatQueue.h"



SatQueue::SatQueue()
{
        _readPtr = 0;
        _writePtr = 0;
        _writeAvailable=LongMsgQueueLen;
        _bShortMsgPresent = false;
}

SatQueue& SatQueue::getInstance(void)
{
        static SatQueue instance;
        return instance;
}       

void SatQueue::empty(void)
{
        _readPtr = 0;
        _writePtr = 0;
        _writeAvailable=LongMsgQueueLen;
}

int SatQueue::count(void)
{
        return (LongMsgQueueLen-_writeAvailable);
}

boolean SatQueue::read(LongMsg & msg)
{
        if (_writeAvailable == LongMsgQueueLen) return false; // Noting in the queue to read
        msg = _lmQueue[_readPtr];
        _readPtr = (_readPtr + 1) % LongMsgQueueLen;
        _writeAvailable++;
        return true;
}

boolean SatQueue::write(LongMsg & msg)
{
  byte localBuffer[10];
  
  memset(localBuffer, 0, sizeof(localBuffer));

#if 0
  Serial.println(F("write() called?"));
  Serial.print(F("SQ-I: Write Data V"));
  //msg.getFormattedMsg(localBuffer,4);
  Serial.print(F("SQ-I: Write Data VI"));
  for (int i = 0;i < 4; i++)
  {
    Serial.print(localBuffer[i],HEX); Serial.print(" ");
  }
  Serial.println();
  Serial.flush();
#endif
  
        if ( _writeAvailable == 0 ) return false;
        _lmQueue[_writePtr] = msg;
        _writePtr = (_writePtr + 1) % LongMsgQueueLen;
        _writeAvailable--;

        return true;
}


#if 0
boolean SatQueue::write(byte bStartAddrH, byte bStartAddrL, byte bEndAddrH, byte bEndAddrL)
{


        if ( _writeAvailable == 0 ) return false;
        _lmQueue[_writePtr].set(bStartAddrH, bStartAddrL, bEndAddrH, bEndAddrL);

        // Check that this is not a duplicate of the last message
        int iLastWritePtr = (_writePtr-1) % LongMsgQueueLen;
        if (_lmQueue[iLastWritePtr].equals(_lmQueue[_writePtr]) ) return true;  // Don't increment since this is a duplicate

        _writePtr = (_writePtr + 1) % LongMsgQueueLen;
        _writeAvailable--;

        return true;
}
#endif

#if 0
boolean SatQueue::isShortMsgAvail(void)
{
        return _bShortMsgPresent;
}
#endif


#if 0
void SatQueue::writeShort(byte lat0, byte lat1, byte lat2, byte lon0, byte lon1, byte lon2, unsigned int alt, unsigned int epochMinutes)
{
        _shortMsg.set(lat0, lat1, lat2, lon0, lon1, lon2, alt, epochMinutes);
        _bShortMsgPresent = true;
}

ShortMsg SatQueue::readShort(void)
{
        _bShortMsgPresent = false;
        return _shortMsg;
}
#endif


