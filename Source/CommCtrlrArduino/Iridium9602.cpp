
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
#define _WS_DEBUG 1

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
        _iMessagesWaiting = 0;
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

        enableIncommingMsgAlert(false);

        // Write the defaults to the modem and use default and flush to eeprom √
        sendCommandandExpectPrefix(F("AT&W0"), F("OK"), 1000);

        //Designate Default Reset Profile
        sendCommandandExpectPrefix(F("AT&Y0"), F("OK"), 1000);
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

void Iridium9602::parseMessage(char * cmd)
{
        Serial.print("Msg:");  Serial.println(_receivedCmd);
        if (strncmp(cmd, "+CIEV:", 6) == 0 ) {
                // Got Unsolisited Signal Response
                if (cmd[8] != '0') {
                        Serial.println(F("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"));
                        if (cmd[8] == '2' || cmd[8] == '3')
                        {
                                /*
                                */
                        }

                } else {
                        Serial.println(F("0000000000000000000000000000000000000000000000000000000000000"));
                }
        } else if (strncmp(cmd, "+SBDRING", 8) == 0 ) { // Got a ring up
                DebugMsg::msg_P("SAT",'D',PSTR("*** Modem is Ringing ***\n"));
                _bRing = true;
        } else if (strncmp(cmd, "+SBDIX:", 7) == 0 ) { // Got a connection status message
                // +SBDIX: 0, 98, 1, 25, 23, 1

                Serial.print(F("************************   ")); Serial.println(cmd);
                _bRing = true;
        }
        clearIncomingMsg();
}

/* implements the loop used to poll for data from the modem
 * and call chcker_func() when ever full response is received. If the 
 * function return true the loop will terminate, otherwise the response
 * will be handed over to parseMassage
 */
bool Iridium9602::expectLoop(const void * response,
                             unsigned int timeout,
                             bool clear_received,
                             bool (*checker_func)(const Iridium9602 &, const void *))
{
        unsigned int starttime = millis();

#if _WS_DEBUG
        if (_rcvIdx != 0) {
                DebugMsg::msg_P("SAT", 'D', PSTR("_rcvIdx is not 0 at start of %s"), __func__);
        }
#endif
        /* always run at least one loop iteration */
        do {
                /* time left */
                unsigned int to = timeout - (millis() - starttime);
                /* make sure that we don't pass in 0 or we'll block */
                if (to == 0) to = 1; /* use 1 ms */

                if (!checkIncomingCRLF(to)) {
#if _WS_DEBUG
                        DebugMsg::msg_P("SAT", 'D', PSTR("checkIncomingCRLF() timeout in %s"), __func__);
#endif
                        /* no more time left and no \r\n terminated string received */
                        return false;
                }


                if (checker_func(*this, response)) {
#if _WS_DEBUG
                        Serial.print(F("  <-- expected "));
                        Serial.println(_receivedCmd);
#endif
                        if (clear_received) {
                                clearIncomingMsg();
                        }
                        return true;
                }

#if _WS_DEBUG
                Serial.print(F("  <-- unexpected "));
                Serial.println(_receivedCmd);
#endif
                /* did not match the desired reponse, parse for asynchronous stuff */
                parseMessage(_receivedCmd);
        } while(millis() - starttime < timeout);
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
                               unsigned int timeout,
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
                               unsigned int timeout,
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
                                             unsigned int timeout,
                                             bool clear_received)
{
        sendCommand(command);
        return expectPrefix(response, timeout, clear_received);
}

bool Iridium9602::sendCommandandExpectPrefix(const __FlashStringHelper * command,
                                             const __FlashStringHelper * response,
                                             unsigned int timeout,
                                             bool clear_received)
{
        sendCommand(command);
        return expectPrefix(response, timeout, clear_received);
}

bool Iridium9602::checkIncomingCRLF(unsigned int timeout)
{
        unsigned int endtime = 0;
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

char Iridium9602::checkSignal()
{
        if (sendCommandandExpectPrefix("AT+CSQF", "+CSQF:", 3000, false)) {
                signal = _receivedCmd[6];
        } else {
                signal = '\0'; // Return a null if unable to get a response
        }

        return signal;
}


boolean Iridium9602::isMessageWaiting() // This is based on the fact that we received a +SBDRING
{
        return _bRing;
}


boolean Iridium9602::sendMsgText( char * msg )
{
        flushIncomingMsg();
        // Check that we have a signal. If not don't send the message
        char cResult = checkSignal();
        if (cResult != '\0' && cResult != '0') // Then we must have a signal
        {
                //enableIncommingMsgAlert(false); // turn off alerts
                flushIncomingMsg();
                _HardwareSerial.print("AT+SBDWT="); _HardwareSerial.println(msg);
                delay(1000);
                if (expectPrefix("OK",2, 5) ) // Got a ready so send the data
                {
                        // Initiate the Send
                        _HardwareSerial.println("AT+SBDIX");
                        delay(1000);
                        if (expectPrefix("OK",2, 5) )
                        {
                                // Clear the outgoing message buffer in the modem
                                _HardwareSerial.println("AT+SBDD0");
                                //enableIncommingMsgAlert(true); // turn on alerts

                                flushIncomingMsg();
                                return true;
                        } 
                }
                flushIncomingMsg();
        } 
        //enableIncommingMsgAlert(true); // turn on alerts
        return false;
}


boolean Iridium9602::sendMsg( unsigned char * msg, int length)
{
        // Check that we have a signal. If not don't send the message
        char cResult = checkSignal();
        //if (cResult != '\0' && cResult != '0') // Then we must have a signal
        {
                flushIncomingMsg();
                //enableIncommingMsgAlert(false); // turn off alerts
                _HardwareSerial.print("AT+SBDWB="); _HardwareSerial.println(length);
                delay(1000);
                if (expectPrefix("READY",5, 5) ) // Got a ready so send the data
                {
                        // Pump out the Binary Data
                        for (int i = 0; i < length ; i++)
                        {
                                _HardwareSerial.write(msg[i]);
                        }
                        if (expectPrefix("0",1,5))
                        {
                                // Initiate the Send
                                _HardwareSerial.println("AT+SBDIX");
                                delay(1000);
                                if (expectPrefix("0",2, 5) )
                                {
                                        // Clear the outgoing message buffer in the modem
                                        _HardwareSerial.println("AT+SBDD0");

                                        //enableIncommingMsgAlert(true); // turn on alerts

                                        flushIncomingMsg();
                                        return true;
                                } 
                        } else {
                                // Modem did not accept
                                return false;
                        }
                }
                flushIncomingMsg();
        } 
        //enableIncommingMsgAlert(true); // turn on alerts
        return false;
}

void Iridium9602::enableIncommingMsgAlert(boolean bEnable)
{

        if (bEnable)
        {
                sendCommandandExpectPrefix(F("AT+CIER=1,1,0,0"), F("OK"), 1000);
                sendCommandandExpectPrefix(F("AT+SBDMTA=1"), F("OK"), 1000);
        } else {
                sendCommandandExpectPrefix(F("AT+CIER=0,0,0,0"), F("OK"), 1000);
                sendCommandandExpectPrefix(F("AT+SBDMTA=0"), F("OK"), 1000);
        }
}



int Iridium9602::getMsgWaitingCount()
{
        //SBDIX 	
        return 0;
}

int Iridium9602::retrieveMsg( unsigned char * msg)
{
        return 0;
}

boolean Iridium9602::isSatAvailable(void)
{
        if (digitalRead(pinNA) == HIGH) 
        {
                return true;
        } 

        return false;
}

void Iridium9602::powerOff(void)
{
        sendCommandandExpectPrefix(F("AT+*F"), F("OK"), 10000);      //Make sat modem prepare for poweroff
        //Wait until OK for up to 10 seconds
        digitalWrite(pinModemPowerSwitch,LOW);  //Power modem off.
}

void Iridium9602::powerOn(void)
{
        Iridium9602::initModem();
}

boolean Iridium9602::isModemOn(void)
{
        if (digitalRead(pinDSR) == LOW)  //Low == 9602 is powered on
        {
                return true;
        }
        return false;
}

boolean Iridium9602::testForSatSimulatorPresence(void)
{
        for (int i = 0; i < 10; i++) {
                if (sendCommandandExpectPrefix(F("RUASIM?"), F("YES"), 500))      //Ask if modem is really a simulator
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
        unsigned int checksum = 0;
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

        if (!expectPrefix(F("READY"), 1000)) return false;

#ifdef _WS_DEBUG
        Serial.print(F("Starting ... ck:"));
        sprintf(buf, "%x\n", checksum);
        Serial.print(buf);

        for (int i = 0; i < messageLength; i++){
                sprintf(buf, "%02x ", messageArray[i]);
                Serial.print(buf);
        }
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
        if (!expectPrefix(F("0"), 1000)) return false;
        Serial.println(F("Message Loaded in 9602 MOQueue"));

        return true;
}

bool Iridium9602::initiateSBDSession(unsigned int timeout)
{
#if 0
        _HardwareSerial.println("AT+SBDD0");
        checkIncomingCRLF(2000);
#endif
        return sendCommandandExpectPrefix(F("AT+SBDIX"), F("OK"), timeout);
}

#if 0
bool Iridium9602::handleUnsolisitedResponse(unsigned int timeout)
{
        unsigned int starttime = millis();

        do {

        } while(millis() - starttime < timeout);
}
#endif

