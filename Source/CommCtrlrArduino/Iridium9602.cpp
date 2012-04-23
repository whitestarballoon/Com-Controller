
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <avr/pgmspace.h>
#include <string.h>

#include "CommCtrlrConfig.h"
#include "Iridium9602.h"
#include "DebugMsg.h"

/* define to enable debug messages */
//#define _WS_DEBUG 1

/* from CommCtrlArduino.ino */
void initIridiumNetworkInterrupt();

extern volatile int NetworkAvailableJustChanged;
extern volatile int SatelliteNetworkAvailable;

Iridium9602::Iridium9602(HardwareSerial& sPort) : _HardwareSerial(sPort), _rcvIdx(0)
{
}



void Iridium9602::initModem()
{
        _rcvIdx = 0;
        _bRing = false;
        _sessionInitiated = false;
        _MOQueued = false;
        _MTQueued = 0;
        _MTMsgLen = 0;
        _lastSessionTime = 0;
        _lastSessionResult = 1;

        Serial.println(F("Iridium9602 settling while off..."));
        //Wait for modem to be fully powered down - this is important delay!
        { //Delay without the delay command
                unsigned long millistart = millis();
                while ( millis() < millistart + 5000 ) {
                }
        }

        //delay(2500);
        if (HIGH == digitalRead(pinDSR)) {
                DebugMsg::msg_P("SAT",'D',PSTR("DSR HIGH."));
        } else {
                DebugMsg::msg_P("SAT",'D',PSTR("DSR LOW"));
        }
        //Turn modem on
        Serial.println(F("Turning Sat Modem On... "));
        Serial.flush();
        digitalWrite(pinModemPowerSwitch, HIGH);
        //Check for DSR line going high to indicate boot has finished
        { 
                unsigned long millistart = millis();
                while ( (millis() < (millistart + 60000)) ) //Delay without the delay command
                {
                        if (HIGH == digitalRead(pinDSR)) 
                        {	
                                break; 
                        }
                }
        }
        if (HIGH == digitalRead(pinDSR)) 
        {
                DebugMsg::msg_P("SAT",'D',PSTR("Modem alive."));
                //Setup NetworkAvailable Pinchange interrupt 
                initIridiumNetworkInterrupt();

        } else {
                DebugMsg::msg_P("SAT",'E',PSTR("!! Modem did not complete boot sequence. !!"));
        }

        while(!sendCommandandExpectPrefix(F("AT"), F("OK"), 500)) {
#if _WS_DEBUG
                DebugMsg::msg_P("SAT", 'I', PSTR(" Modem is not talking to me yet"));
#endif
        }
        DebugMsg::msg_P("SAT", 'I', PSTR(" Modem talked to me"));

        //Set Serial Character Echo Off 
        sendCommandandExpectPrefix(F("ATE0"), F("OK"), 1000);

        //Set modem to not use flow control handshaking during SBD messaging √
        sendCommandandExpectPrefix(F("AT&K0"), F("OK"), 1000);

        //Set response quiet mode - 0 = responses ARE sent to arduino from modem √
        sendCommandandExpectPrefix(F("ATQ0"), F("OK"), 1000);

        setIncommingMsgAlert(false);

        // Write the defaults to the modem and use default and flush to eeprom √
        sendCommandandExpectPrefix(F("AT&W0"), F("OK"), 1000);

        //Designate Default Reset Profile
        sendCommandandExpectPrefix(F("AT&Y0"), F("OK"), 1000);

        setIndicatorReporting(true);
        setIncommingMsgAlert(true);
}

void Iridium9602::clearIncomingMsg(void)
{
        _rcvIdx = 0;
        _receivedCmd[0] = '\0';
}

void Iridium9602::flushIncomingMsg(void)
{
        clearIncomingMsg();
        while (_HardwareSerial.available()) _HardwareSerial.read();
}

#define NEXT_VAL(v) \
do { \
        int __tmp;                                              \
        /* skip white spaces */                                 \
        while(*p && *p == ' ') p++;                             \
        /* convert MO status to int, n point to first char */   \
        /* after the number */                                  \
        __tmp = strtol(p, &n, 10);                              \
        if ((v))                                                \
                *(v) = __tmp;                                   \
        /* if p == n then no number was found */                \
        if (p == n) /* no number */                             \
                goto err_out;                                   \
        p = n;                                                  \
        /* have to be at EOL or at a ',' */                     \
        if (*p != ',' && *p != '\0') goto err_out;              \
        /* p should poimnt at start of next number or space */  \
        p++;                                                    \
} while (0) 

static bool parseSBDIXResponse(char * buf, int * mo_st, int * mt_st,
                               int * mt_len, int * mt_queued)
{
        /* <MO status>,<MOMSN>,<MT status>,<MTMSN>,<MT length>,<MT queued> */
        char * p, * n = NULL;

        p = buf + 7;

        /* get MO status */
        NEXT_VAL(mo_st);

        /* skip MOMSN */
        p = strchr(p, ',');
        if (*p == '\0') goto err_out;
        p++;

        NEXT_VAL(mt_st);
        /* skip MTMSN */

        p = strchr(p, ',');
        if (!*p) goto err_out;
        p++;

        NEXT_VAL(mt_len);
        NEXT_VAL(mt_queued);

        /* if we get here parse was good */
        return true;

err_out:
        return false;
}

#undef NEXT_VAL


void Iridium9602::parseUnsolicitedResponse(char * cmd)
{
        /* check time session lost timeout */

        if (strncmp_P(_receivedCmd, PSTR("+SBDIX:"), 7) == 0) {
                //DebugMsg::msg_P("SAT", 'D', PSTR("Match SBDIX"));
                int mo_st = -1, mt_st, mt_len, mt_q;
                if (parseSBDIXResponse(_receivedCmd, &mo_st, &mt_st, &mt_len, &mt_q)) {
                    DebugMsg::msg_P("Sat", 'D', PSTR("Got good +SBDIX respponse"));
                    DebugMsg::msg_P("Sat", 'D', PSTR("  mo_st: %d mt_st: %d mt_len: %d mt_queue: %d"),
                                    mo_st, mt_st, mt_len, mt_q);
                    
                    if (mt_st == 1) {  // Received message if 1
                    	_MTQueued = mt_q + 1;
                    	_MTMsgLen = mt_len;	
                    } else if ((mt_st == 2)) {
                    	//MT message didn't transfer properly, need to start retry now
                    
                    }
                    
                    /* update count stored at GSS */
                    _GSSQueued = mt_q;
                    _MTStatus = mt_st;
                }


                clearIncomingMsg();
                expectPrefix(F("OK"), satResponseTimeout);
                _sessionInitiated = false;
    

                if (_MOQueued && 
                    (mo_st >= 0) && (mo_st <= 4)) {   //4 or less indicates sent success
                        _lastSessionResult = 1;
                        /* clear out the MO queue since message was sent */
                        sendCommandandExpectPrefix("AT+SBDD0", "OK", satResponseTimeout);
                        _MOQueued = false;
                } else  {
                        _lastSessionResult = -mo_st;
                }
        } else if (strncmp_P(_receivedCmd, PSTR("SBDRING"), 8) == 0) {
                //DebugMsg::msg_P("SAT", 'D', PSTR("Match RING"));
                _bRing = true;
        } else if (strncmp_P(_receivedCmd, PSTR("+CIEV:"), 6) == 0) {
                //DebugMsg::msg("SAT", 'D', "Match CIEV");
                if (_receivedCmd[6] == '0') {
                        /* signal level 0 - 5 */
                        _signal = _receivedCmd[8] - '0';
                } else {
                        /* network available */
                        if (_receivedCmd[0] == '1')  {
                                _networkAvailable = true;
                                _networkStateChanged = true;
                        } else {
                                _networkAvailable = false;
                                _signal = 0;
                                _networkStateChanged = true;
                        }
                }
        }
}

/* implements the loop used to poll for data from the modem
 * and call chcker_func() when ever full response is received. If the 
 * function return true the loop will terminate, otherwise the response
 * will be handed over to parseMassage
 */
bool Iridium9602::expectLoop(const void * response,
                             unsigned long timeout,
                             bool clear_received,
                             bool (*checker_func)(const Iridium9602 &, const void *))
{
        unsigned long starttime = millis();

#if _WS_DEBUG
        if (_rcvIdx != 0) {
                DebugMsg::msg_P("SAT", 'D', PSTR("_rcvIdx is not 0 at start of %s"), __func__);
        }
#endif
        /* always run at least one loop iteration */
        do {
                /* time left */
                unsigned long to = timeout - (millis() - starttime);
                /* make sure that we don't pass in 0 or we'll block */
                if (to == 0) to = 1; /* use 1 ms */

#if 0
                DebugMsg::msg_P("SAT", 'I', PSTR("%s: timeout: %d to: %d st: %d ml: %d"), 
                                __func__, timeout, to, starttime, millis());
#endif
                if (!checkIncomingCRLF(to)) {
                        continue;
                }


                if (checker_func(*this, response)) {
#if _WS_DEBUG
                        Serial.print(F("  <-- [solicited] <"));
                        Serial.print(_receivedCmd);
                        Serial.println(">");
#endif
                        if (clear_received) {
                                clearIncomingMsg();
                        }
                        return true;
                }

#if _WS_DEBUG
                Serial.print(F("  <-- [unsolicited] <"));
                Serial.print(_receivedCmd);
                Serial.println(">");
#endif
                /* did not match the desired reponse, parse for asynchronous stuff */
                parseUnsolicitedResponse(_receivedCmd);
                clearIncomingMsg();
        } while(timeout == 0 || millis() - starttime < timeout);

        return false;
}

static bool __prefixCheck(const Iridium9602 & sat, const void * data)
{

        /* If we are here then _receivedCmd should have the response */
        if (strncmp(sat.get_receivedCmd(), (const char *)data, strlen((const char *)data)) == 0) {
                return true;
        }

        return false;
}

bool Iridium9602::expectPrefix(const char * response,
                               unsigned long timeout,
                               bool clear_received)
{

        return expectLoop(response, timeout, clear_received, __prefixCheck);
}

static bool __prefixCheck_P(const Iridium9602 & sat, const void * data)
{
        /* If we are here then _receivedCmd should have the response */
        if (strcmp_P(sat.get_receivedCmd(), (PGM_P)data) >= 0) {
                return true;
        }

        return false;
}

bool Iridium9602::expectPrefix(const __FlashStringHelper * response,
                               unsigned long timeout,
                               bool clear_received)
{
        return expectLoop(response, timeout, clear_received, __prefixCheck_P);
}

void Iridium9602::sendCommand(const char * command)
{
        _HardwareSerial.print(command);
        _HardwareSerial.print("\r\n");

#if _WS_DEBUG
        Serial.print(F(" --> >"));
        Serial.println(command);

#endif
}

void Iridium9602::sendCommand(const __FlashStringHelper * command)
{

        _HardwareSerial.print(command);
        _HardwareSerial.print("\r\n");

#if _WS_DEBUG
        Serial.print(F(" --> >"));
        Serial.println(command);
#endif
}

bool Iridium9602::sendCommandandExpectPrefix(const char * command,
                                             const char * response,
                                             unsigned long timeout,
                                             bool clear_received)
{
        sendCommand(command);
        return expectPrefix(response, timeout, clear_received);
}

bool Iridium9602::sendCommandandExpectPrefix(const __FlashStringHelper * command,
                                             const __FlashStringHelper * response,
                                             unsigned long timeout,
                                             bool clear_received)
{
        sendCommand(command);
        return expectPrefix(response, timeout, clear_received);
}

bool Iridium9602::checkIncomingCRLF(unsigned long timeout)
{
        unsigned long endtime = 0;
        char inChar;

        /* calculate end time for our loop */
        if (timeout > 0) {
                endtime = millis() + timeout;
        }

        /* execute at leat once */
        do 
        {
                /* this is here to keep on reading as long as modem says that 
                 * data is available with out caring for the timeout
                 */
quick_restart:
                /* want to make sure that we have enough space in the buffer to
                 * null terminate it
                */
                if (_rcvIdx >= MAX_RECV_BUFFER - 1) {
                        /* at this point the things are screwed up enough that a reboot is best */
#if _WS_DEBUG
                        _receivedCmd[_rcvIdx] = '\0';
                        Serial.write((uint8_t *)_receivedCmd, sizeof(_receivedCmd));
#endif
                        DebugMsg::msg_P("SAT", 'E', PSTR("Ran out of space in _receivedCmd buffer, current buffer:\n%s\n"), _receivedCmd);
                        clearIncomingMsg();
                }

                if (_HardwareSerial.available())
                {
                        inChar = _HardwareSerial.read();
                        /* ignore white space chars at start of line */
                        if (_rcvIdx == 0 && (inChar == '\r' || inChar == '\n')) continue;
                        _receivedCmd[_rcvIdx++] = (unsigned char )inChar;
                        _receivedCmd[_rcvIdx] = 0;
                        if (_rcvIdx >= 2 && _receivedCmd[_rcvIdx - 2] == '\r' && _receivedCmd[_rcvIdx - 1] == '\n') {
                                /* get rid of trailing white spaces */
                                while (_receivedCmd[_rcvIdx-1] == '\r' || _receivedCmd[_rcvIdx-1] == '\n') {
                                        _receivedCmd[--_rcvIdx] = '\0';
                                }
                                return true;
                        }
                        goto quick_restart;
                }
        /*
         * second compound statement will only execute if
         * timeout != 0.
         *
         * Will loop as long as timeout is 0 or endtime
         * is less than current time (millis())
         */
        } while(timeout == 0 || millis() < endtime);

        return false;
}

int Iridium9602::checkSignal()
{
#if 0
        if (sendCommandandExpectPrefix("AT+CSQF", "+CSQF:", 3000, false)) {
                signal = _receivedCmd[6];
        } else {
                signal = '\0'; // Return a null if unable to get a response
        }
#endif

        return _signal;
}

bool Iridium9602::setIncommingMsgAlert(bool bEnable)
{

        if (bEnable)
        {
                return sendCommandandExpectPrefix(F("AT+SBDMTA=1"), F("OK"), satResponseTimeout);
        } else {
                return sendCommandandExpectPrefix(F("AT+SBDMTA=0"), F("OK"), satResponseTimeout);
        }
}

bool Iridium9602::setIndicatorReporting(bool bEnable)
{
        if (bEnable) {
                return sendCommandandExpectPrefix(F("AT+CIER=1,1,1"), F("OK"), satResponseTimeout);
        } else {
                return sendCommandandExpectPrefix(F("AT+CIER=0,0,0"), F("OK"), satResponseTimeout);
        }
}

int Iridium9602::getMessageWaitingCount(void)
{
        return _GSSQueued;
}

int Iridium9602::getRecentMTStatus(void)
{
        return _MTStatus;
}


unsigned char Iridium9602::wait_read(void)
{

        while (!_HardwareSerial.available()) 
                ;
                
       return _HardwareSerial.read();
}

int Iridium9602::loadMTMessage(unsigned char * msg, int msg_sz)
{
        unsigned char inChar = 0;
        int br = 0; /* bytes read */
        unsigned long checksum = 0;
        unsigned long checksum_from_modem = 0;
        unsigned long msgLen = 0;

        sendCommand("AT+SBDRB");

        msgLen = (inChar = wait_read()) << 8;
        DebugMsg::msg_P("SAT", 'D', PSTR("1 = %x"), inChar);
        msgLen = (inChar |= _HardwareSerial.read());
        DebugMsg::msg_P("SAT", 'D', PSTR("2 = %x"), inChar);

        DebugMsg::msg_P("SAT",'D',PSTR("msgLen = %d."), msgLen);

        while (br < msgLen) {
                inChar = _HardwareSerial.read();
                /* ignore white space chars at start of line */
                DebugMsg::msg_P("SAT", 'D', PSTR("3 = %x"), inChar);
                msg[br++] = inChar;
                checksum += inChar;
        }

        DebugMsg::msg_P("SAT",'D',PSTR("ck = %X."), checksum);

        checksum_from_modem = (inChar = _HardwareSerial.read()) << 8;
        DebugMsg::msg_P("SAT", 'D', PSTR("4 = %x"), inChar);
        DebugMsg::msg_P("SAT",'D',PSTR("cksm = %X."), checksum_from_modem);

        checksum_from_modem |= (inChar = _HardwareSerial.read());
        DebugMsg::msg_P("SAT", 'D', PSTR("5 = %x"), inChar);
        DebugMsg::msg_P("SAT",'D',PSTR("cksm = %X."), checksum_from_modem);

        sendCommandandExpectPrefix("AT+SBDD1", "OK", satResponseTimeout);
        
        if (checksum_from_modem != checksum) {
                DebugMsg::msg_P("SAT",'E',PSTR("calculated check sum (%X) != modem check sum (%X)"),
                                checksum, checksum_from_modem);
                br = 0;
        }

        _MTMsgLen = 0;
        _MTQueued--; 
        return br;
}

bool Iridium9602::isSatAvailable(void)
{
        if (digitalRead(pinNA) == HIGH) 
        {
                return true;
        } 

        return false;
}

void Iridium9602::powerOff(void)
{
        sendCommandandExpectPrefix(F("AT+*F"), F("OK"), satResponseTimeout);      //Make sat modem prepare for poweroff
        //Wait until OK for up to 10 seconds
        digitalWrite(pinModemPowerSwitch,LOW);  //Power modem off.
}

void Iridium9602::powerOn(void)
{
        Iridium9602::initModem();
}

bool Iridium9602::isModemOn(void)
{
        if (digitalRead(pinDSR) == LOW)  //Low == 9602 is powered on
        {
                return true;
        }
        return false;
}

bool Iridium9602::isSimulatorPresent(void)
{
        for (int i = 0; i < 10; i++) {
                if (sendCommandandExpectPrefix(F("RUASIM?"), F("YES"), satResponseTimeout))      //Ask if modem is really a simulator
                {
                        return true;
                }
        }

        return false;
}



//FUNCTION: Write binary data to sat modem MO Queue 
bool Iridium9602::loadMOMessage(unsigned char* messageArray, int messageLength) 
{
        //Compute Checksum
        unsigned long checksum = 0;
        char buf[15];
        byte checksumHighByte;
        byte checksumLowByte;
        //Checksum is 2-byte summation of entire SBD message, high order byte first.  
        for (int i = 0; i < messageLength; i++){
                checksum += messageArray[i]; 
        }
        checksumHighByte = highByte(checksum);
        checksumLowByte = lowByte(checksum);
        snprintf(buf, sizeof(buf), "AT+SBDWB=%d", messageLength);
        sendCommand(buf);

        if (!expectPrefix(F("READY"), satResponseTimeout)) return false;

#ifdef _WS_DEBUG
#if 0
        Serial.print(F("Starting ... ck:"));
        sprintf(buf, "%x\n", checksum);
        Serial.print(buf);

        for (int i = 0; i < messageLength; i++){
                sprintf(buf, "%02x ", messageArray[i]);
                Serial.print(buf);
        }
#endif
#endif

        /* write message length */
        checksum = _HardwareSerial.write(messageArray, messageLength);
        /* write checksum bytes */
        checksum += _HardwareSerial.write(&checksumHighByte, 1);
        checksum += _HardwareSerial.write(&checksumLowByte, 1);

        /* check that we wrote expected number of bytes write calls above failed */
        if (checksum != messageLength + 2) return false;

#ifdef _WS_DEBUG
        sprintf(buf, "* %02x ", checksumHighByte);
        Serial.print(buf);
        sprintf(buf, "%02x\n", checksumLowByte);
        Serial.print(buf);
        Serial.println(F("DONE"));
#endif

        /* XXX Don't know if new line is needed */
        _HardwareSerial.println();
        if (!expectPrefix(F("0"), satResponseTimeout)) return false;
        /* we hope that we get it, 
         * but I've seen sometime no OK.
         * */
        if (!expectPrefix(F("OK"), satResponseTimeout)) {
                /* XXX */
                Serial.println(F("Couldn't get OK, if this message gets delivered it might be zero byte, verify and fix me to resend!"));
        }

        //Serial.println(F("Message Loaded in 9602 MOQueue"));

        _MOQueued = true;
        return true;
}

bool Iridium9602::initiateSBDSession(unsigned long timeout)
{
#if 0
        _HardwareSerial.println("AT+SBDD0");
        checkIncomingCRLF(2000);
#endif
        bool ret = false;
        

        if (_sessionInitiated) goto out;

        if (_bRing) {
        	ret = sendCommandandExpectPrefix(F("AT+SBDIXA"), F("OK"), timeout);
        } else {
        	ret = sendCommandandExpectPrefix(F("AT+SBDIX"), F("OK"), timeout);
        }
        _sessionInitiated = true;
        _bRing = false;

out:
        return ret;
}

bool Iridium9602::pollUnsolicitedResponse(unsigned long timeout)
{
        unsigned long starttime = millis();

        do {
                unsigned long to = timeout - (millis() - starttime);
                if (to == 0) to = 1;
                if (checkIncomingCRLF(to)) {
                        DebugMsg::msg_P("SAT", 'D',PSTR("Unsolicited response: %s"), _receivedCmd);
                        parseUnsolicitedResponse(_receivedCmd);
                        clearIncomingMsg();
                }
        } while(timeout == 0 || millis() - starttime < timeout);

        if (millis() - _lastSessionTime > satSBDIXResponseLost) {
                _sessionInitiated = false;
                _lastSessionResult = 0;
        }

        return false;
}

