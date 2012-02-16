#ifndef Iridium9602_h
#define Iridium9602_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <string.h>

#define MAX_RECV_BUFFER 100

class Iridium9602
{

public:
	Iridium9602( HardwareSerial& sPort);
	void initModem();

        boolean checkIncomingMsg();
        boolean checkIncomingMsg(int);
        boolean checkFixedLenIncomingMsg(int iLen);
        
        void clearIncomingMsg();

        char checkSignal();

	boolean sendMsg( unsigned char * msg, int length);
	int getMsgWaitingCount();
	int retrieveMsg( unsigned char * msg);
	boolean isSatAvailable(void);
	void powerOff(void);
	void powerOn(void);	
        char _receivedCmd[MAX_RECV_BUFFER];

private:
	HardwareSerial& _HardwareSerial;
        int _rcvIdx;
	
	
};

#endif
