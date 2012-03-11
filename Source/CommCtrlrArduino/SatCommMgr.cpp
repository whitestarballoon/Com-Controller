
#include "Iridium9602.h"
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

        void SatCommMgr::satCommInit()
        {
          DebugMsg::msg_P("SAT",'D',PSTR("SatModem Init Start..."));
          _satModem.initModem();
          DebugMsg::msg_P("SAT",'I',PSTR("SatModem Init Completed."));

        }



        void SatCommMgr::update(void)
        {
		//FIX vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv  - use some sort of global variable for satSimulatorConnected
		//if(_satModem.isSatAvailable() || satSimulatorConnected == true)
		if(_satModem.isSatAvailable())
		{ 
          if ( millis() - _last_millis > 1000)
          {  
             //Serial.println(F("SCM:update:Check Signal"));
             char signal = _satModem.checkSignal();
             char lstr[100];
             int chars;
             if ((signal == '0') || (signal == (char)255))   // 255 indicates no network available
             {
               chars = snprintf(lstr, 100, "No Signal");
             } else {
               chars = snprintf(lstr, 100, "Signal %c ", signal);
               if ( chars >= 100 ) lstr[100]=0; 
             }
             DebugMsg::msg("CC",'I',lstr);

             _last_millis = millis();
          }
        }
          
/*          
          // Check if anything is available from the sat modem
          if ( _satModem.checkIncomingMsg() )  // Update will return true if there is a message to be processed (\r\n)
          {
            Serial.println(_satModem._receivedCmd);
          }
*/
          if ( SatQueue::getInstance().isShortMsgAvail() ) // Got a short message that needs to be sent
          {
          	Serial.println(F("ATCReport Would send to sat here"));
          }
          
          if (  SatQueue::getInstance().count() > 0)  // Got a long message that needs to be sent
          {
			Serial.println(F("LongMessage Would send to sat here"));
          }

        }

        void SatCommMgr::sendShortMsg(ShortMsg sm)
        {
          _satModem.sendMsg(sm.getFormattedMsg(), sm.getFormattedLength());
        }
        
        void SatCommMgr::turnModemOn()
        {
          _satModem.powerOn();
        }
        
        void SatCommMgr::turnModemOff()
        {
          _satModem.powerOff();
        }


