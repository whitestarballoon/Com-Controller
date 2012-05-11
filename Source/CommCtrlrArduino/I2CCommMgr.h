#ifndef I2CCommMgr_h
#define I2CCommMgr_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <string.h>
#include "CommCtrlrConfig.h"

#include "I2CMsg.h"
#include "Iridium9602.h"
#include "SatCommMgr.h"


class I2CCommMgr
{

public:
        I2CCommMgr(SatCommMgr& satCommMgr);
        void i2cInit(void);

        /*
	static I2CCommMgr& getInstance() 
        {
          static I2CCommMgr instance;
          return instance;
        }
        */
        boolean CheckForI2CFreeze();
        void I2CAliveCheck();
        int I2CXmit(byte device, byte command, byte* data, int length);
        int I2CXmitMsg(byte device, byte* data, int length);
        void update();
        void I2CParse(I2CMsg i2cMsg);
        
private:
        static void i2cReceiveData(int wireDataLen);
		SatCommMgr& _satCommMgr;

};

#endif






