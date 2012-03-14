

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

/* from CommCtrlArduino.ino */
void initIridiumNetworkInterrupt();

extern volatile int NetworkAvailableJustChanged;
extern volatile int SatelliteNetworkAvailable;

Iridium9602::Iridium9602(HardwareSerial& sPort) : _HardwareSerial(sPort), _rcvIdx(-1)
{
}



void Iridium9602::initModem()
{
        _rcvIdx = -1;
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

        // Wait until the Modem wants to talk nicely with OK's
        do {
                { //Delay without the delay command
                        unsigned long millistart = millis();
                        while ( millis() < millistart + 3000 ) {
                        }
                }
                _HardwareSerial.println("AT");
        } while ( ! expectPrefix("OK",2,1) );

        //Serial.println("SUCCESS");
        //return;

        //Set Serial Character Echo Off √
        _HardwareSerial.println("ATE0");
        expectPrefix("OK",2,1);
        flushIncomingMsg();

        //Set modem to not use flow control handshaking during SBD messaging √
        flushIncomingMsg();
        _HardwareSerial.println("AT&K0");
        expectPrefix("OK",2,1);
        flushIncomingMsg(); 	


        //Set response quiet mode - 0 = responses ARE sent to arduino from modem √
        _HardwareSerial.println("ATQ0");
        expectPrefix("OK",2,1);
        flushIncomingMsg();


        enableIncommingMsgAlert(false);


        // Write the defaults to the modem and use default and flush to eeprom √
        _HardwareSerial.println("AT&W0");
        expectPrefix("OK",2,1);
        flushIncomingMsg();

        //Designate Default Reset Profile √
        _HardwareSerial.println("AT&Y0");
        expectPrefix("OK",2,1);
        flushIncomingMsg();
       
}

/*
   boolean Iridium9602::checkIncomingMsg(void)
   {
   unsigned char inChar;
   boolean bMsgAvailable = false;
   Serial.println("CI:S");
   while (_HardwareSerial.available())
   {
   inChar = (unsigned char)_HardwareSerial.read();
//Serial.print("->"); Serial.println( (char) inChar );    
if ( _rcvIdx < 0 && ( inChar == 13 || inChar == 10)) continue;  // Ignore Leading CR's & LF's

_receivedCmd[++_rcvIdx] = inChar;
_receivedCmd[_rcvIdx+1] = '\0'; 
if (_rcvIdx >= 1 && _receivedCmd[_rcvIdx-2] == 13 && _receivedCmd[_rcvIdx-1] == 10 )
{
// We have a full message from the modem
_receivedCmd[_rcvIdx-1] = '\0';
//Serial.print("\n==>"); Serial.print( _receivedCmd); Serial.println("<==");     
bMsgAvailable = true;
break;
}
}
Serial.println("CI:R");
return bMsgAvailable;
}
*/



boolean Iridium9602::checkIncomingMsg(int timeout) // timeout in milliseconds
{
        unsigned long startMillis = millis();
        //Serial.print("CI w/ TO ");Serial.println(startMillis);
        unsigned char inChar;
        boolean bMsgAvailable = false;
        while ( (!bMsgAvailable )  && ( (millis() - startMillis) < timeout)  ) //&& (startMillis < millis() ) )
        {
                //Serial.println( _HardwareSerial.available() );
                while (_HardwareSerial.available())
                {
                        inChar = (unsigned char)_HardwareSerial.read();
                        Serial.print("->"); Serial.println( (char) inChar );    
                        if ( _rcvIdx < 0 && ( inChar == 13 || inChar == 10)) continue;  // Ignore Leading CR's & LF's

                        _receivedCmd[++_rcvIdx] = inChar;
                        _receivedCmd[_rcvIdx+1] = '\0'; 
                        if (_rcvIdx >= 1 && _receivedCmd[_rcvIdx-2] == 13 && _receivedCmd[_rcvIdx-1] == 10 )
                        {
                                // We have a full message from the modem
                                _receivedCmd[_rcvIdx-1] = '\0';
#ifdef printVerifiedModemSerialMessages
                                Serial.print("-->"); Serial.print( _receivedCmd); Serial.println("<--");    
#endif
                                bMsgAvailable = true;
                                break;
                        }
                }
        }
        //Serial.println("CITO:R");
        return bMsgAvailable;
}




boolean Iridium9602::checkFixedLenIncomingMsg(int iLen)
{
        unsigned char inChar;
        for ( int i = 0; i < (iLen * 3) ; i++ )
        {
                if (_HardwareSerial.available())
                {
                        inChar = (unsigned char)_HardwareSerial.read();
                        _receivedCmd[++_rcvIdx] = inChar;
                        _receivedCmd[_rcvIdx+1] = '\0';
                        if (_rcvIdx >= (iLen + 1)/*msg + 2 chksum digits*/   ) return true;
                } else {
                        delay(50);
                }
        }
        return false;
}

void Iridium9602::clearIncomingMsg(void)
{
        _rcvIdx = -1;
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


boolean Iridium9602::expectPrefix(char * sPrefix, int iPrefixLen, int iTimeoutSeconds)
{
        unsigned int iInitialMillis;
        iInitialMillis = millis();
        while ( iInitialMillis + (iTimeoutSeconds * 1000) > millis() )
        {
                if ( checkIncomingMsg(1000) )
                {
                        Serial.print("Cmd:");
                        Serial.println(_receivedCmd);
                        if ( strncmp(_receivedCmd, sPrefix, iPrefixLen ) == 0 ) 
                        {
#ifdef printExpectings
                                Serial.print(F("Expecting: ")); Serial.println(sPrefix);
                                Serial.println(F(" Found"));
#endif
                                return true;
                        }
#ifdef printExpectings
                        Serial.print(F("Expecting: ")); Serial.println(sPrefix);
                        Serial.print(F(", but found: "));
                        Serial.println(_receivedCmd);
#endif
                        // If it is not the expected Message go ahead and parse to see if it is something unsolisited.
                        parseMessage(_receivedCmd);
                }
        }
        DebugMsg::msg_P("SAT",'W',PSTR("EXPECTED RESPONSE FROM MODEM NOT FOUND!\n"));
        return false;
}


void Iridium9602::checkIncomingCRLF()
{
  char inChar;
  Serial.print(F("Modem response: "));
  do 
  {
    if (_rcvIdx >= MAX_RECV_BUFFER) {
      Serial.write((uint8_t *)_receivedCmd, sizeof(_receivedCmd));
      _rcvIdx = 0;
    }
    if (_HardwareSerial.available())
    {
      inChar = _HardwareSerial.read();
      _receivedCmd[++_rcvIdx] = (unsigned char )inChar;
      if (_rcvIdx > 2 && _receivedCmd[_rcvIdx - 1] == '\r' && _receivedCmd[_rcvIdx] == '\n') {
        Serial.write((uint8_t *)_receivedCmd, _rcvIdx);
        _rcvIdx = 0;
        break;
      }
    } else {
      delay(50);
    }
  } while(true);
  Serial.println("");
}

char Iridium9602::checkSignal()
{

        //Serial.println(F("IRID:Check Signal:Start"));
        clearIncomingMsg();
        _HardwareSerial.print("AT+CSQF\r"); // Use Fast Method

        if (expectPrefix("+CSQF:",6, 5) )
        {
                signal = _receivedCmd[6];
        } else {
                signal = '\0'; // Return a null if unable to get a response
        }
        //Serial.println(F("IRID:Check Signal:Return"));

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
                _HardwareSerial.println("AT+CIER=1,1,0,0");
                expectPrefix("OK",2,4);
                _HardwareSerial.println("AT+SBDMTA=1");
                expectPrefix("OK",2,4);
        } else {
                _HardwareSerial.println("AT+CIER=0,0,0,0");
                expectPrefix("OK",2,4);
                _HardwareSerial.println("AT+SBDMTA=0");
                expectPrefix("OK",2,4);
        }
        flushIncomingMsg();
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
        _HardwareSerial.println("AT+*F");      //Make sat modem prepare for poweroff
        expectPrefix("OK",2,10);  //Wait until OK for up to 10 seconds
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
                _HardwareSerial.println("RUASIM?");      //Ask if modem is really a simulator
                if(expectPrefix("YES",3,1))
                {
                        return true;
                }
        }
}



//FUNCTION: Write binary data to sat modem MO Queue 
   void Iridium9602::loadMOMessage(unsigned char* messageArray, int messageLength) 
{
   

  //Compute Checksum
    unsigned int checksum = 0;
    char buf[10];
    byte checksumHighByte;
    byte checksumLowByte;
    String localstring;
	//Checksum is 2-byte summation of entire SBD message, high order byte first.  
	for (int i = 0; i < messageLength; i++){
		checksum += messageArray[i]; 
	}
	checksumHighByte = highByte(checksum);
	checksumLowByte = lowByte(checksum);
#if 0
	messageArray[messageLength] = checksumHighByte;
	messageArray[messageLength + 1] = checksumLowByte;
	delay(100);
#endif
	_HardwareSerial.print("AT+SBDWB=");
	_HardwareSerial.println(messageLength);
	checkIncomingCRLF();
//	delay(1000);
	
#if 0
	for (int i = 0; i < (messageLength + 2); i++){
		_HardwareSerial.print(messageArray[i]); 
	}
#else
        Serial.print(F("Starting ... ck:"));
        sprintf(buf, "%x\n", checksum);
        Serial.print(buf);

	for (int i = 0; i < messageLength; i++){
          sprintf(buf, "%02x ", messageArray[i]);
          Serial.print(buf);
	}

        _HardwareSerial.write(messageArray, messageLength);
        _HardwareSerial.write(&checksumHighByte, 1);
        _HardwareSerial.write(&checksumLowByte, 1);
        //_HardwareSerial.write((uint8_t *)checksum, 2);

        sprintf(buf, "* %02x ", checksumHighByte);
        Serial.print(buf);
        sprintf(buf, "%02x\n", checksumLowByte);
        Serial.print(buf);
        
        Serial.println(F("DONE"));
#endif
	_HardwareSerial.println();
	checkIncomingCRLF();
	checkIncomingCRLF();
	Serial.println(F("Message Loaded in 9602 MOQueue"));
}

  void Iridium9602::initiateSBDSessionHACK(void)
  {
  	 delay(500);
  	 _HardwareSerial.println("AT+SBDIX");
	checkIncomingCRLF();
  	 Serial.println("SBD session initiated");
  }




