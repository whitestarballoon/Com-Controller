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
        LongMsg() { }

        LongMsg(byte bStartAddrH, byte bStartAddrL, byte bEndAddrH, byte bEndAddrL)
        {
          set(bStartAddrH, bStartAddrL, bEndAddrH, bEndAddrL);
        }

        void set(byte bStartAddrH, byte bStartAddrL, byte bEndAddrH, byte bEndAddrL);
        boolean equals( LongMsg m );

        inline int getFormattedLength(void) { return _EndAddr - _StartAddr + 1; }
        int getFormattedMsg(unsigned char * data, int data_sz);

private:
        unsigned int _StartAddr;
        unsigned int _EndAddr;
		
        byte I2CeePROMRead(byte device, unsigned int addr);
};

#endif

