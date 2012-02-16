#ifndef I2CQueue_h
#define I2CQueue_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <string.h>
#include "CommCtrlrConfig.h"

#include "I2CMsg.h"


class I2CQueue
{

public:
	static I2CQueue& getInstance() 
        {
          static I2CQueue instance;
          return instance;
        }
        void empty(void);
        int count(void);
        I2CMsg read();
        boolean write(I2CMsg& i2cMsg);
private:
        I2CQueue();
        I2CMsg _i2cQueue[I2CMsgQueueLen];
        int _readPtr;
        int _writePtr;
        int _writeAvailable;       
};

#endif




