#ifndef I2CMsg_h
#define I2CMsg_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <string.h>
#include "CommCtrlrConfig.h"



class I2CMsg
{

public:
        byte i2cData[i2cMaxDataLen];
        byte i2cDataLen;
        byte i2cRxCommand;
private:
       
};

#endif

