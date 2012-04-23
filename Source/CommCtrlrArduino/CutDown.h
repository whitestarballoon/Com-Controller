#ifndef CutDown_h
#define CutDown_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <string.h>
#include "CommCtrlrConfig.h"


class CutDown
{

public:
        
        static void initCutdown(HardwareSerial * sPort);
        static void CutdownNOW();
        static void ResetTimer();
        static void CmdSet(unsigned char deadManTime);
        
private:
        static HardwareSerial * cdn;
        
};

#endif




