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

#if 0
                boolean checkIncomingMsg();
                boolean checkIncomingMsg(int);
                boolean checkFixedLenIncomingMsg(int iLen);

                boolean expectPrefix(char *, int, int);
#endif

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
                bool loadMOMessage(unsigned char * , int ); 
                bool initiateSBDSession(unsigned int timeout);
   				
                
                /* 
                 * Will look for a single full response from the modem.
                 * Full respone is ASCII coded string terminated by \r\n
                 * The data from the modem will be placed in _receivedCmd
                 *
                 * if timeout is not 0, function will wait for at most milliseconds.
                 *
                 * Returns true if \r\n terminate response was found, false if
                 * response wasn't found before the specified timeout.
                 */
                bool checkIncomingCRLF(unsigned int timeout);

                /*
                 * Waits for a prefix reposen from the modem, timeout can be
                 * specified in milliseconds.
                 *
                 * If clear_received is true then the _recievedCmd buffer
                 * will be reset before this function returns.
                 */
                bool expectPrefix(const char * response,
                                  unsigned int timeout,
                                  bool clear_received = true);
                bool expectPrefix(const __FlashStringHelper * response,
                                  unsigned int timeout,
                                  bool clear_received = true);

                /*
                 * Send a command to the modem.
                 */
                void sendCommand(const char * command);
                void sendCommand(const __FlashStringHelper * command);

                /*
                 * Send command to modem and wait for response prefix, a timeout can
                 * be specified.
                 *
                 * "OK" will match "OK" and "OKAY"
                 *
                 * if timeout is not 0, function will wait for at most milliseconds.
                 * otherwise it will block waiting for the response to come in from
                 * the modem.
                 *
                 * If clear_received is true, clear the recieved buffer if response prefix
                 * matched.
                 *
                 * Returns true if response was foudn, false if timeout was reached
                 * before response was found.
                 * response wasn't found before the specified timeout.
                 */
                bool sendCommandandExpectPrefix(const char * command,
                                                const char * response,
                                                unsigned int timeout,
                                                bool clear_received = true);
                bool sendCommandandExpectPrefix(const __FlashStringHelper * command,
                                                const __FlashStringHelper * response,
                                                unsigned int timeout,
                                                bool clear_received = true);

                inline const char * get_receivedCmd(void) const { return _receivedCmd; }
        private:
                bool expectLoop(const void * response,
                                unsigned int timeout,
                                bool clear_received,
                                bool (*checker_func)(const Iridium9602 &, const void *));
        private:
                char _receivedCmd[MAX_RECV_BUFFER];
                HardwareSerial& _HardwareSerial;
                int _rcvIdx;
                char signal;
                boolean _bRing;
                int _iMessagesWaiting;
};

#endif

