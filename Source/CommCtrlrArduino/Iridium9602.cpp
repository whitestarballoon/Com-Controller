
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <avr/pgmspace.h>
#include <string.h>

#include "Iridium9602.h"



Iridium9602::Iridium9602(HardwareSerial& sPort) : _HardwareSerial(sPort), _rcvIdx(-1)
{
}

void Iridium9602::initModem()
{
    _rcvIdx = -1;
    //Send out some initialization to the modem
   //Set Serial Character Echo Off
   _HardwareSerial.print("ATE0\r");
   delay(200);
   checkIncomingMsg();
   Serial.println(_receivedCmd);
   clearIncomingMsg();
   
   //Set response quiet mode - 0 = responses ARE sent to arduino from modem
   _HardwareSerial.print("ATQ0\r");
   delay(200);
   checkIncomingMsg();
   Serial.println(_receivedCmd);
   clearIncomingMsg();

	
   //Ask for manufacturer ID number
Serial.print("Manufacturer ID: ")
   _HardwareSerial.print("AT+CGMI\r");
   delay(200);
   checkIncomingMsg();
   Serial.println(_receivedCmd);
   clearIncomingMsg();
   
   //Ask for modem Serial number
Serial.print("MODEM Serial#: ")
   _HardwareSerial.print("AT+CGSN\r");
   delay(200);
   checkIncomingMsg();
   Serial.println(_receivedCmd);
   clearIncomingMsg();


/*
   _HardwareSerial.print("+SBDAREG=1\r");
   delay(200);
   checkIncomingMsg();
   Serial.println(_receivedCmd);
   clearIncomingMsg();


   _HardwareSerial.print("AT+SBDWT=Test Message From Iridium\r");
   delay(200);
   checkIncomingMsg();
   Serial.println(_receivedCmd);
   clearIncomingMsg();
*/

   _HardwareSerial.flush();	
}


boolean Iridium9602::checkIncomingMsg(void)
{
  unsigned char inChar;
  boolean bMsgAvailable = false;
  while (_HardwareSerial.available())
  {
    inChar = (unsigned char)_HardwareSerial.read();
Serial.print((char)inChar);    
    _receivedCmd[++_rcvIdx] = inChar;
    _receivedCmd[_rcvIdx+1] = '\0'; 
    if (_rcvIdx >= 1 && _receivedCmd[_rcvIdx-2] == '\n' && _receivedCmd[_rcvIdx-1] == '\r')
    {
      // We have a full message from the modem
      _receivedCmd[_rcvIdx-1] = '\0';
Serial.print("\n-->"); Serial.print( _receivedCmd); Serial.println("<--");     
      bMsgAvailable = true;
      break;
    }
  }
  return bMsgAvailable;
}

boolean Iridium9602::checkIncomingMsg(int timeout) // timeout in milliseconds
{
  unsigned long startMillis = millis();
Serial.print("w/ Timeout");Serial.println(startMillis);
  unsigned char inChar;
  boolean bMsgAvailable = false;
  while ( (! bMsgAvailable )  && ( (millis() - startMillis) < timeout)  ) //&& (startMillis < millis() ) )
  {
	//Serial.println("*");
    while (_HardwareSerial.available())
    {

Serial.println("Available");

      inChar = (unsigned char)_HardwareSerial.read();
//if ( (char)inChar > ' ' )
//{
//  Serial.print((char)inChar);
//} else {
  Serial.println(inChar);
//}  
      _receivedCmd[++_rcvIdx] = inChar;
      _receivedCmd[_rcvIdx+1] = '\0'; 
      if (_rcvIdx >= 1 && _receivedCmd[_rcvIdx-2] == 10 && _receivedCmd[_rcvIdx-1] == 13 )
      {
        // We have a full message from the modem
Serial.print("\n-->"); Serial.print( _receivedCmd); Serial.println("<--");     
        _receivedCmd[_rcvIdx-1] = '\0';
        bMsgAvailable = true;
        break;
      }
    }
  }
Serial.println("Return");
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
  _HardwareSerial.flush();
}



char Iridium9602::checkSignal()
{
  char signal = '0';
Serial.println("Check Signal");
   clearIncomingMsg();
   _HardwareSerial.print("AT+CSQF\r"); // Use Fast Method
   delay(1000);
   if ( checkIncomingMsg(5000) )
   {
     Serial.println("Msg:");
     Serial.println(_receivedCmd);
     
     if (strncmp(_receivedCmd, "+CSQF:",6)==0 )
     {
       signal = _receivedCmd[6];
       Serial.print("Signal :"); Serial.println(signal);
     }
   }
   clearIncomingMsg();
   return signal;
}










boolean Iridium9602::sendMsg( unsigned char * msg, int length)
{
   return true;
}









int Iridium9602::getMsgWaitingCount()
{
	return 0;
}

int Iridium9602::retrieveMsg( unsigned char * msg)
{
	return 0;
}

boolean Iridium9602::isSatAvailable(void)
{
	return true;;
}

void Iridium9602::powerOff(void)
{
}

void Iridium9602::powerOn(void)
{
}

