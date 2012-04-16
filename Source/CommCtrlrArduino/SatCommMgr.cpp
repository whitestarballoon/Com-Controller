
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

static unsigned long retry_timeouts[] = {1000L,5000L,3000L,30000L,30000L};
static int retry_timeouts_sz = sizeof(retry_timeouts) / sizeof(retry_timeouts[0]);
static unsigned char sbuf[satMessageCharBufferSize];

SatCommMgr::SatCommMgr(Iridium9602& sModem):_satModem(sModem), _last_millis(0)
{
        _retryTimeIdx = 0;
        _lastSessionTime = 0;
        _lastActivityTime = 0;
        _retryTimeIdx = 0;
        _randomizedRetryTime = 0;
}

void SatCommMgr::satCommInit(I2CCommMgr * i2cCommMgr)
{
        DebugMsg::msg_P("SAT",'D',PSTR("SatModem Init Start..."));
        _satModem.initModem();
        _i2cCommMgr = i2cCommMgr;
        DebugMsg::msg_P("SAT",'I',PSTR("SatModem Init Completed."));
#if 1
  {
    
    snprintf((char *)sbuf, 6, "hello");
    _satModem.loadMOMessage((unsigned char *)sbuf,5);
    Serial.print(F("Hello Sent!\n"));
  }
#endif

}



void SatCommMgr::update(void)
{
        bool initiate_session = false;
        unsigned char uplinkMsg[10];

#if 1
        static unsigned long _last_tick = millis();
        if (millis() - _last_tick > 10000UL) {
                DebugMsg::msg("TiCk", 't', 
                              (char *)(_satModem.isSatAvailable() ? "!" : "."));
                //Serial.print((_satModem.isSatAvailable() ? "!" : "."));
                _last_tick = millis();
        }
#endif

        //DebugMsg::msg_P("CC", 'D', PSTR("Before poll()"));
        _satModem.pollUnsolicitedResponse(200);
        //DebugMsg::msg_P("CC", 'D', PSTR("After poll()"));
        
        if (!_satModem.isMOMessageQueued()) {
                /* nothing in the MO queue, reset retryTimeIdx */
                _retryTimeIdx = 0;
        }

        if (_satModem.networkStateChanged() && _satModem.isSatAvailable()) {
                DebugMsg::msg_P("CC", 'D',  PSTR("Modem just became available"));
                //initiate_session = true;  // This shouldn't be enabled here, delete line and this comment if things work Ok.
                /* reset out retry array index */
                _retryTimeIdx = 0;
        }

        if (_satModem.isSatAvailable()) {
                //DebugMsg::msg_P("CC", 'D',  PSTR("Modem sat is available"));
                if (_satModem.isRinging()) {
                        DebugMsg::msg_P("CC", 'D', PSTR("Modem is ringing"));
                        initiate_session = true;
                }

                /* there is a message in the MO queue, but no session is active,
                 * figure out if it's time to initiate a new session
                 * No need to do all of this if initiate_session is already true
                 */
                if (!initiate_session && _satModem.isMOMessageQueued() 
                    && !_satModem.isSessionActive()) 
                {
                        //DebugMsg::msg_P("CC", 'D', PSTR("checking for time out"));
                        if (millis() - _lastSessionTime > _randomizedRetryTime) {
#if 0
                                DebugMsg::msg_P("CC", 'D', PSTR("[%d] = %lu --  %lu ms timeout hit"), 
                                                _retryTimeIdx,
                                                _randomizedRetryTime,
                                                millis() - _lastSessionTime);
#endif
                                /* don't let _retryTimeIdx go past the end of the array */
                                if (_retryTimeIdx + 1 < retry_timeouts_sz) {
                                        _retryTimeIdx++;
                                }
                                /*
                                 * Setup the randomized value that we will actually use for hte
                                 * time out, it would be from 10-ms to maximum of what's in
                                 * the array.
                                 */
                                _randomizedRetryTime = random(10, retry_timeouts[_retryTimeIdx]);
#if 0
                                DebugMsg::msg_P("CC", 'D', PSTR("New timeout [%d] = %lu"),
                                                _retryTimeIdx, _randomizedRetryTime);
#endif
                                /* reset the counter to start from now */
                                _lastSessionTime = millis();
                                initiate_session = 1;
                        }
                }
                
                /* check if maximum time between session has passed */
                if (!initiate_session && ((millis() - _lastSessionTime)  > satForceSBDSessionInterval)) {
                        DebugMsg::msg_P("CC", 'D', PSTR("force SBD session"));
                        initiate_session = 1;
                        _lastSessionTime = millis();
                }

#if 0
                DebugMsg::msg_P("CC", 'D', PSTR("is: %d sa: %d, lst: %lu to: %lu tl: %lu ms: %lu"),
                                initiate_session, 
                                _satModem.isSessionActive(),
                                _lastSessionTime,
                                _randomizedRetryTime,
                                (_randomizedRetryTime - (millis() - _lastSessionTime) > 0 ?
                                 _randomizedRetryTime - (millis() - _lastSessionTime) : 0),
                                millis());
#endif

                if (initiate_session && !_satModem.isSessionActive()) {
                       if (!_satModem.initiateSBDSession(satResponseTimeout)) {
                               /* force the time to now if no response was received before the timeout */
                               _lastSessionTime = millis();
                       }
                }
        }

        /* check for incoming data */
        if (_satModem.isMTMessageQueued()) {
                int sz; 
                sz = _satModem.loadMTMessage(uplinkMsg, sizeof(uplinkMsg));
                if (sz > 0) {
                        parseIncommingMsg(uplinkMsg, sz);
                } else {
                        DebugMsg::msg_P("CC", 'D', PSTR("loadMTMessage() returned = %d"), sz);
                }
        }

        /* now pull data from SatQueue into the MOQueue */
        if (SatQueue::getInstance().isMsgAvail()) // Got a  message that needs to be sent
        {
                SatQueue & q = SatQueue::getInstance();
                int i;
                char buf[20];
                memset(sbuf, 0, sizeof(sbuf));
                LongMsg msg;
                q.read(msg);

                //DebugMsg::msg("SC",'I'," sizeof(%d)", sizeof(sbuf));
                int msgLen = msg.getFormattedMsg((unsigned char *)sbuf, sizeof(sbuf));
                if (msgLen > 0) {
#if 1
                        Serial.print(F("Message to place in 9602 MOQ ==--> "));
                        for(i = 0; i < msgLen; i++) {
                                sprintf(buf, "%x ", sbuf[i]);
                                Serial.print(buf);
                        }
                        Serial.print(msgLen);
                        Serial.println("<--==========");
#endif
                        _satModem.loadMOMessage((unsigned char *)sbuf, (int)msgLen);
                        Serial.println(F("Message loaded in MOQ."));
                } else 
                {
                        DebugMsg::msg_P("SC",'E', PSTR("Message read failed with error#: %d "), (char)msgLen);
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
        int i = 0;

        Serial.print(F("Message from 9602 MTQ ==--> "));
        for(i = 0; i < packLen; i++) {
                char buf[5];
                sprintf(buf, "%x ", packetBufferLocal[i]);
                Serial.print(buf);
        }
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


