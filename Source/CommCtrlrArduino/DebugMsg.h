#ifndef DebugMsg_h
#define DebugMsg_h

// Workaround for http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
#undef PROGMEM
#define PROGMEM __attribute__((section(".progmem.data")))

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <string.h>

#include "I2CCommMgr.h"
#include "CommCtrlrConfig.h"


class DebugMsg
{

public:
        static int msg(String sSource, char cLevel, char *str, ...);
        static int msg_P(String sSource, char cLevel, char *str, ...);
        static void setI2CCommMgr(I2CCommMgr * i2cCommMgr);
        
private:
        static I2CCommMgr * _i2cCommMgr;
        
};

#endif



