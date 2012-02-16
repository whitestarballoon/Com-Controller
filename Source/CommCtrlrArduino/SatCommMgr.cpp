
#include "Iridium9602.h"
#include "SatCommMgr.h"
#include "CommCtrlrConfig.h"
#include "DebugMsg.h"
#include "I2CQueue.h"
#include "CutDown.h"
#include "LongMsg.h"
#include "SatQueue.h";


#define SatDebug

        SatCommMgr::SatCommMgr(Iridium9602& sModem):_satModem(sModem), _last_millis(0)
        {
          //satQueue = SatQueue::getInstance();
        }

        void SatCommMgr::satCommInit()
        {
          _satModem.initModem();
          DebugMsg::msg_P("SAT",'I',PSTR("SatModem Init Done.\n"));

        }


        void SatCommMgr::update(void)
        {
          
          if ( millis() - _last_millis > 10000)
          {
            Serial1.println("Check Signal");
            _satModem.checkSignal();  
            _last_millis = millis();
          }
          
          // Check if anything is available from the sat modem
          if ( _satModem.checkIncomingMsg() )  // Update will return true if there is a message to be processed (\r\n)
          {
            Serial.println(_satModem._receivedCmd);
          }

          if ( SatQueue::getInstance().isShortMsgAvail() ) // Got a short message that needs sent
          {
            
          }
          
          if (  SatQueue::getInstance().count() > 0)  // Got a long message that needs sent
          {

          }

        }

        void SatCommMgr::sendShortMsg(ShortMsg sm)
        {
          _satModem.sendMsg(sm.getFormattedMsg(), sm.getFormattedLength());
        }
