#ifndef LongMsg_h
#define LongMsg_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <string.h>
#include "CommCtrlrConfig.h"



class LongMsg
{

public:
        void set(byte bStartAddrH, byte bStartAddrL, byte bEndAddrH, byte bEndAddrL);
        int getFormattedLength();
        unsigned char * getFormattedMsg();        
private:
        unsigned int _StartAddr;
        unsigned int _EndAddr;

        byte I2CeePROMRead(byte device, unsigned int addr);
       
};

#endif

