/* 
 I2CParse - Program for parsing data from the I2C Bus
 Created by White Star Ballon, December 13, 2010.
 Last Revised December 13, 2010
 */
/*
*******************************
 * Incoming I2C command list:
 *******************************
 i2cCmdSATTXATCRpt
 i2cCmdSATTxFrmEEPROM 
 i2cCmdHFUpdtTelem 
 i2cCmdHFTxShortRpt 
 i2cCmdHFSetTxRate 
 i2cCmdHFSnooze 
 i2cCmdCDNHeartBeat
 i2cCmdCDNSetTimerAndReset
 i2cCmdCDNCUTDOWNNOW
 i2cCmdSATPowerOn
 i2cCmdSATPowerOff
 
 */
#include <avr/pgmspace.h>

void I2CParse(byte command) {
  switch(command){
  case i2cCmdSATTXATCRpt: 
    { 
#ifdef actionLabels
      printf_P(PSTR("\ni2c StoreATC\n"));
#endif
      lprintf("CC:LatestOrbPosUpdtd\nCC:StoreATCRpt\n");
      //Store Latest position and Epoch Time for later use
      latestPosition[0]=i2cdata[i2cSel][6];  //lat0
      latestPosition[1]=i2cdata[i2cSel][7];  //lat1
      latestPosition[2]=i2cdata[i2cSel][8];  //lat2
      latestPosition[3]=i2cdata[i2cSel][0];  //lon0
      latestPosition[4]=i2cdata[i2cSel][1];  //lon1
      latestPosition[5]=i2cdata[i2cSel][2];  //lon2
      epochMinutes = word((i2cdata[i2cSel][4] & B00001111),i2cdata[i2cSel][5]);  //Copy epochMinute value into ram

#ifdef sendOrbPositionRpts
      checkIfTimeToUpdateOrbPosition();  //This will send an orb positiion rpt if enough time has elapsed since the last one
#endif
      satSaveATCPkt(); //save the ATC packet to be ready to send when sat is ready to accept.



      break;
    } 
  case i2cCmdSATTxFrmEEPROM: 
    { 
      
      printf_P(PSTR("i2c LngMsgIn\n"));
      //If Flight computer sends a pair of I2C EEPROM addresses that are the same, that's an error in the FC
      if((i2cdata[i2cSel][0] == i2cdata[i2cSel][2]) && (i2cdata[i2cSel][1] == i2cdata[i2cSel][3])){
        lprintf("CC:i2cEEPMsgAddrs same");
        printf_P(PSTR("i2cEEPMsgAddrs same\n"));
        break; // Do not continue, do not store requested pair.
      }
      satQueueI2CEEPROMLongMessage(i2cdata[i2cSel]);
      break;
    }
  case i2cCmdHFUpdtTelem: //Untested
    { 
      packetBufferS[0]=i2cdata[i2cSel][0];
      packetBufferS[1]=i2cdata[i2cSel][1];
      packetBufferS[2]=i2cdata[i2cSel][2];
      hfSendSerialCommand(packetBufferS,3);
      break;
    }
  case i2cCmdHFTxShortRpt: 
    { 
      packetBufferS[0]='P';
      hfSendSerialCommand(packetBufferS,1);
      break;
    }
  case i2cCmdHFSetTxRate: 
    { 
      packetBufferS[0]=i2cdata[i2cSel][0];
      hfSendSerialCommand(packetBufferS,1);
      break;
    }
  case i2cCmdHFSnooze: 
    { 
      packetBufferS[0]='X';
      hfSendSerialCommand(packetBufferS,1);	
      break;
    }
  case i2cCmdCDNHeartBeat: 
    { 
      cdnCmdResetTimer();
      lprintf("CC: HeartBeat RXed\n");
      printf_P(PSTR("CC: HeartBeat RXed\n"));
      break;
    }
  case i2cCmdCDNSetTimerAndReset: 
    { 
      cdnCmdSet(i2cdata[i2cSel][0]);  //Take 1 byte 0-255 for minutes
      lprintf("CC: CDNTimerSet\n");
      break;
    }
  case i2cCmdUpdateThreeNinersValue:
    { 
      updateThreeNinersTelem();
      break;
    }
  case i2cCmdSetGwy:
    {
      if (( 1 == i2cdata[i2cSel][0]) || ( 120 == i2cdata[i2cSel][0])) {
        gwy_id = i2cdata[i2cSel][0];  //Set gateway
        satSetByteParameter(packetBufferB, (byte)0x01, (byte)gwy_id);
        lprintf("CC: GWY CHGD TO %d\n", gwy_id);
        printf_P(PSTR("CC: GWY CHGD TO %d\n"), gwy_id);
      }
      break;
    }
  case i2cCmdSendOrbPositRpt:
    {
      satSendSCOrigPositionRpt(latestPosition,packetBufferS);
      break;
    }
    break;
  case i2cCmdCDNCUTDOWNNOW: 
    { 
      cdnCmdCUTDOWNNOW();
      break;
    }
  case i2cCmdSATPowerOn: 
    { 
      lprintf("CC: SATMODEM ON\n");
      printf_P(PSTR("SATMODEM ON\n"));
      satInitRoutine(0);
      break;
    }
  case i2cCmdSATPowerOff: 
    { 
      lprintf("CC: SATMODEM OFF\n");
      printf_P(PSTR("SATMODEM OFF\n"));
      digitalWrite(PWR_EN,LOW);
      break;
    }

  default:                                               // Ignore any command that is not in the list
    {
#ifdef i2cDebug
      printf_P(PSTR("i2c cmd unknown: %d"),command);
#endif
      lprintf("CC: i2c cmd unknown: %d\n",command);
    }
  }
}

