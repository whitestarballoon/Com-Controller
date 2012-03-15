
#include "Iridium9602.h"
#include "I2CCommMgr.h"
#include "SatCommMgr.h"
#include "CommCtrlrConfig.h"
#include "DebugMsg.h"
#include "I2CQueue.h"
#include "CutDown.h"
#include "LongMsg.h"
#include "SatQueue.h"


#define SatDebug

SatCommMgr::SatCommMgr(Iridium9602& sModem):_satModem(sModem), _last_millis(0)
{
        //satQueue = SatQueue::getInstance();
}


void SatCommMgr::satCommInit(I2CCommMgr * i2cCommMgr)
{
        DebugMsg::msg_P("SAT",'D',PSTR("SatModem Init Start..."));
        _satModem.initModem();
		_i2cCommMgr = i2cCommMgr;
        DebugMsg::msg_P("SAT",'I',PSTR("SatModem Init Completed."));

}



void SatCommMgr::update(void)
{
        unsigned char lstr[100];
#if 1
        snprintf((char *)lstr, 6, "hello");
        _satModem.loadMOMessage((unsigned char *)lstr,5);
        _satModem.initiateSBDSession(2000);
        Serial.print(F("Sent!\n"));
        while(1) ;
#endif
        if(1 || _satModem.isSatAvailable())
        { 

                if ( (millis() - _last_millis) > 1000)
                {  
                        //Serial.println(F("SCM:update:Check Signal"));
                        Serial.print("!");
                        /*
                           char signal = _satModem.checkSignal();
                           int chars;
                           if ((signal == '0') || (signal == (char)255))   // 255 indicates no network available
                           {
                           chars = snprintf(lstr, 100, "No Signal");
                           } else {
                           chars = snprintf(lstr, 100, "Signal %c ", signal);
                           if ( chars >= 100 ) lstr[100]=0; 
                           }
                           DebugMsg::msg("CC",'I',lstr);
                           */
                        DebugMsg::msg("CC",'I',"SIG-OK");

                        /*          
                        // Check if anything is available from the sat modem
                        if ( _satModem.checkIncomingMsg() )  // Update will return true if there is a message to be processed (\r\n)
                        {
                        Serial.println(_satModem._receivedCmd);
                        }
                        */          



                        if ( SatQueue::getInstance().isMsgAvail() ) // Got a  message that needs to be sent
                        {
                                SatQueue & q = SatQueue::getInstance();
                                int i;
                                char buf[20];
                                memset(lstr, 0, sizeof(lstr));
                                LongMsg msg;
                                q.read(msg);

                                //DebugMsg::msg("SC",'I'," sizeof(%d)", sizeof(lstr));
                                int msgLen = msg.getFormattedMsg((unsigned char *)lstr, sizeof(lstr));
#if 1
                                Serial.print(F("==========--> "));
                                for(i = 0; i < msgLen; i++) {
                                        sprintf(buf, "%x ", lstr[i]);
                                        Serial.print(buf);
                                }
                                Serial.print(msgLen);
                                Serial.println("<--==========");
                                _satModem.loadMOMessage((unsigned char *)lstr,(int)msgLen);
                                _satModem.initiateSBDSession(2000);

#endif
                        }

                        _last_millis = millis();
                }
        }


}

void SatCommMgr::sendShortMsg(ShortMsg sm)
{
        //_satModem.sendMsg(sm.getFormattedMsg(), sm.getFormattedLength());
}

void SatCommMgr::sendLongMsg(unsigned char * mstr, int len)
{
        
        
}

void SatCommMgr::turnModemOn()
{
        _satModem.powerOn();
}

void SatCommMgr::turnModemOff()
{
        _satModem.powerOff();
}


void SatCommMgr::parseIncommingMsg(unsigned char* packetBufferLocal,unsigned int packLen)
{
#if 0
	/*
	//Check that message type is correct:
	if ()
	{
		printf_P(PSTR("Invalid inbound message type"));
		return false;
	}
	*/
	
	
	//OK, now we know we have the correct format message
	//Message body should start at byte packetPayloadStartIndex
	//Process this just like all other I2C commands

	#ifdef satDataDebug
		Serial.println(F("MSGOK"));
	#endif
	
	/*
	if ( satCheckForDupeCommand(packetBufferLocal[packetPayloadStartIndex]) )
	{
		#ifdef satDataDebug
			Serial.println("Dupe");
		#endif
		return false;  // Return without processing if it's a duplicate
	}
	*/
	
	//Check to see if MSG is for us - the CommCtrlr
	if (packetBufferLocal[packetPayloadStartIndex+1] == i2cCommCtrl){
  		satCommCommandProc(packetBufferLocal);
	} else {
		//process for release onto the I2C bus
		//Copy message body to the short temp buffer, stopping before the two checksum bytes
		for (unsigned int i=packetPayloadStartIndex+3;i<packLen-2;i++) {
			packetBufferA[i-packetPayloadStartIndex+3]=packetBufferLocal[i];  
		}
		I2CXmit(packetBufferLocal[packetPayloadStartIndex + 1], packetBufferLocal[ + 1], packetBufferA, packLen-satIncomingMessageHeaderLength);   //Send to I2C
		//?May want to check for I2C success status here!

		//If all went well return true.
		return true;
	}


	//Process commands destined for the comm controller from sat modem
	//Pass in the actual sat command packet
	void SatCommMgr::satCommCommandProc(unsigned char * packetBufferLocal) {
	  //Command goes from index byte packetPayloadStartIndex to byte packetPayloadStartIndex + 4 of packetBufferLocal
	  // Byte packetPayloadStartIndex = Command counter
	  // Byte packetPayloadStartIndex + 1 = I2C Module Address
	  // Byte packetPayloadStartIndex + 2 = I2C command  
	  // Byte packetPayloadStartIndex + 3 = Data Byte 0
	  // Byte packetPayloadStartIndex + 4 = Data Byte 1
	  Serial.println(F("UplinkIsForMe"));
	  // Determine which COMM CONTROLLER COMMAND via Satellite is being given
	  switch (packetBufferLocal[packetPayloadStartIndex + 2]){ //Check Command Byte 
	  case 0x06:  //Reset heartbeat timer
		CutDown::ResetTimer();
		break;
	  case 0x07:  //set cutdown timer to X minutes, and reset
		CutDown::CmdSet(packetBufferLocal[packetPayloadStartIndex + 3]);  // Send 1 byte to the cutdown command set function 
		satConfirmTimerCHG(packetBufferLocal[9]);
		break;
	  case 0x99:  // CUTDOWN NOW
		CutDown::CutdownNOW();
		break;
	  default: 
		; 
	  }
	}


	//Purpose:  Transmit a downlink message confirmation with recent TIME that the deadman timer has been reset.
	void SatCommMgr::satConfirmTimerCHG(byte timerValue)
	{

	   Serial.println(F("SatConfirmTimerCHG"));
	   // Manual 6-byte report
	   packetBufferS[0]= 0xFF;
	   packetBufferS[1]= 0x01;  //Cutdown Timer Length Change command confirm indicator
	   packetBufferS[2]= timerValue;
	   packetBufferS[3]= lowByte(epochMinutes);
	   packetBufferS[4]= highByte(epochMinutes);
	   packetBufferS[5]= 0;
	   //call function to send to sat modem here
	   
	}
#endif
}


