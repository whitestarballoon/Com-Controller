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

                void clearIncomingMsg();
                void flushIncomingMsg();

                int checkSignal();

                int getMessageWaitingCount();
                int getRecentMTStatus();
                /* 
                 * return the result of the last SBDI* command.
                 * 1 if last command was successfull
                 * <1 if last SBD session init command failed
                 * 0 = SBDIX response was never received
                 * - errors are the response from MTST negated.
                 */
                inline int lastSessionResult(void) const { return _lastSessionResult; }
                inline unsigned long lastSessionTime(void) const { return _lastSessionTime; }

                bool setIncommingMsgAlert(bool);
                bool setIndicatorReporting(bool);
                void powerOff(void);
                void powerOn(void);	


                bool isSatAvailable(void);
                bool isModemOn(void);
                bool isSimulatorPresent(void);
                inline bool isMOMessageQueued(void) const { return _MOQueued; }
                inline bool isMTMessageQueued(void) const { return _MTQueued > 0; }
                inline int whatIsMTMessageLength(void) const { return _MTMsgLen; }
                inline bool isSessionActive(void) const { return _sessionInitiated; }
                inline bool isRinging(void) const { return _bRing; }

                bool loadMOMessage(unsigned char * , int ); 
                bool initiateSBDSession(unsigned long timeout);
   				
                /* parse an unsolicited response from the modem */
                void parseUnsolicitedResponse(char *);

                /*
                 * This function should be called at frequent intervals
                 * in order to handle various delayed and unsolicited
                 *
                 * If timeout is 0, then function will block until a 
                 * response is received from the modem. Otherwise
                 * the timeout is specified in milliseconds.
                 */
                bool pollUnsolicitedResponse(unsigned long timeout);

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
                bool checkIncomingCRLF(unsigned long timeout);

                /*
                 * Waits for a prefix reposen from the modem, timeout can be
                 * specified in milliseconds.
                 *
                 * If clear_received is true then the _recievedCmd buffer
                 * will be reset before this function returns.
                 */
                bool expectPrefix(const char * response,
                                  unsigned long timeout,
                                  bool clear_received = true);
                bool expectPrefix(const __FlashStringHelper * response,
                                  unsigned long timeout,
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
                                                unsigned long timeout,
                                                bool clear_received = true);
                bool sendCommandandExpectPrefix(const __FlashStringHelper * command,
                                                const __FlashStringHelper * response,
                                                unsigned long timeout,
                                                bool clear_received = true);

                /* 
                 * Copy data from MT into into msg, msg_sz should
                 * specify the size of the buffer that msg points to.
                 *
                 * Function will return number of bytes actually copied and
                 * will clear the MT queue on the modem.
                 *
                 * If msg is not big enough function will return -1 and will
                 * not do anything else.
                 */
                int loadMTMessage(unsigned char * msg, int msg_sz);

                inline const char * get_receivedCmd(void) const { return _receivedCmd; }

                /* will return true if the state of the network has changed 
                 * since the last time this function was called
                 */
                inline bool networkStateChanged(void) {
                  bool tmp = _networkStateChanged;
                  _networkStateChanged = false;
                  return tmp;
                }
                

        private:
                unsigned char wait_read(void);

                bool expectLoop(const void * response,
                                unsigned long timeout,
                                bool clear_received,
                                bool (*checker_func)(const Iridium9602 &, const void *));
        private:
                char _receivedCmd[MAX_RECV_BUFFER];
                HardwareSerial& _HardwareSerial;
                int _rcvIdx;
                int _signal;
                bool _networkAvailable;
                bool _networkStateChanged;
                bool _bRing;
                bool _sessionInitiated;
                bool _MOQueued;
                int _MTQueued;
    			int  _MTMsgLen;
                int  _GSSQueued;
                int _MTStatus;
                unsigned long _lastSessionTime;
                bool _lastSessionResult;
};

#endif

