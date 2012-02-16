#ifndef DebugMsg_h
#define DebugMsg_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <string.h>

#include "CommCtrlrConfig.h"
#include "TextDisplay20x12.h"

class DebugMsg
{

public:
        static int msg(String sSource, char cLevel, char *str, ...);
        static int msg_P(String sSource, char cLevel, char *str, ...);
        //static void setDisplay();
        static void setDisplay(TextDisplay20x12 * t);
private:
        static TextDisplay20x12 * tdisp;
};

#endif


