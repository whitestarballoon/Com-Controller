
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
#include "SatQueue.h"

class I2CCommMgr;

class SatCommMgr
{

public:
        SatCommMgr(Iridium9602& satModem);
        
        void satCommInit(I2CCommMgr * i2cCommMgr);
        void update(void);
        void turnModemOn();
        void turnModemOff();
      
        
		
private:
        Iridium9602& _satModem;
        void sendShortMsg(ShortMsg sm);
        void sendLongMsg(unsigned char * mstr, int len);
        unsigned long _last_millis;
        //SatQueue& satQueue;
        I2CCommMgr * _i2cCommMgr;

		void parseIncommingMsg(unsigned char* packetBufferLocal,unsigned int packLen);
		void satCommCommandProc(unsigned char * packetBufferLocal);
		void satConfirmTimerCHG(byte timerValue);
		
};

#endif





