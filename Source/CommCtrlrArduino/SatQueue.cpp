
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
        
        boolean SatQueue::read(LongMsg& longMsg)
        {
          if (_writeAvailable == LongMsgQueueLen) return false; // Noting in the queue to read
          longMsg = _lmQueue[_readPtr];
          _readPtr = (_readPtr + 1) % LongMsgQueueLen;
          _writeAvailable++;
          return true;
        }
        
        boolean SatQueue::write(LongMsg& longMsg)
        {
          if ( _writeAvailable == 0 ) return false;
          _lmQueue[_writePtr] = longMsg;
          
          _writePtr = (_writePtr + 1) % LongMsgQueueLen;
          _writeAvailable--;
          
          return true;
        }
        
        
        boolean SatQueue::isShortMsgAvail(void)
        {
          return _bShortMsgPresent;
        }
        
        
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

