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

                boolean expectPrefix(char *, int, int);

                void clearIncomingMsg();
                void flushIncomingMsg();
                void parseMessage(char *);

                char checkSignal();

                boolean isMessageWaiting();

                void enableIncommingMsgAlert(boolean);

                boolean sendMsgText( char * msg );
                boolean sendMsg( unsigned char * msg, int length);
                int getMsgWaitingCount();
                int retrieveMsg( unsigned char * msg);
                boolean isSatAvailable(void);
                void powerOff(void);
                void powerOn(void);	
                boolean isModemOn(void);
                boolean testForSatSimulatorPresence(void);
   				void loadMOMessage(unsigned char * , int ); 
   				void initiateSBDSessionHACK(void);
   				
                char _receivedCmd[MAX_RECV_BUFFER];
                
        private:
                void checkIncomingCRLF();

        private:
                HardwareSerial& _HardwareSerial;
                int _rcvIdx;
                char signal;
                boolean _bRing;
                int _iMessagesWaiting;




};

#endif

