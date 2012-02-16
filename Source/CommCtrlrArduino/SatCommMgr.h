
#ifndef SatCommMgr_h
#define SatCommMgr_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <string.h>
#include "CommCtrlrConfig.h"

#include "LongMsg.h"
#include "SatQueue.h";

class SatCommMgr
{

public:
        SatCommMgr(Iridium9602& satModem);
        
        void satCommInit();

        void update(void);
        
private:
        Iridium9602& _satModem;
        void sendShortMsg(ShortMsg sm);
        unsigned long _last_millis;
        //SatQueue& satQueue;
};

#endif




