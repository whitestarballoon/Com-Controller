
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <avr/pgmspace.h>
#include <string.h>

#include "I2CQueue.h"


        I2CQueue::I2CQueue()
        {
          _readPtr = 0;
          _writePtr = 0;
          _writeAvailable=I2CMsgQueueLen;
        }

        void I2CQueue::empty(void)
        {
          _readPtr = 0;
          _writePtr = 0;
          _writeAvailable=I2CMsgQueueLen;
        }

        int I2CQueue::count(void)
        {
          return (I2CMsgQueueLen-_writeAvailable);
        }
        
        I2CMsg I2CQueue::read()
        {
          //if (_writeAvailable == I2CMsgQueueLen) return false; // Noting in the queue to read
          I2CMsg i2cMsg = _i2cQueue[_readPtr];
          _readPtr = (_readPtr + 1) % I2CMsgQueueLen;
          _writeAvailable++;
          return i2cMsg;
        }
        
        boolean I2CQueue::write(I2CMsg& i2cMsg)
        {
          if ( _writeAvailable == 0 ) return false;
          _i2cQueue[_writePtr] = i2cMsg;
          
          _writePtr = (_writePtr + 1) % I2CMsgQueueLen;
          _writeAvailable--;
          
          return true;
        }

