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


void I2CParse(byte command) {
  switch(command){
  case i2cCmdSATTXATCRpt: 
    { 
      #ifdef i2cDebug
      Serial.println("\ni2c TXSATC");
      #endif
      lprintf("CC:LatestOrbPosUpdtd\nCC:TXATCRpt\n");
      //Store Latest position and Epoch Time for later use
      latestPosition[0]=i2cdata[i2cSel][6];  //lat0
      latestPosition[1]=i2cdata[i2cSel][7];  //lat1
      latestPosition[2]=i2cdata[i2cSel][8];  //lat2
      latestPosition[3]=i2cdata[i2cSel][0];  //lon0
      latestPosition[4]=i2cdata[i2cSel][1];  //lon1
      latestPosition[5]=i2cdata[i2cSel][2];  //lon2
      epochMinutes = word((i2cdata[i2cSel][4] & B00001111),i2cdata[i2cSel][5]);  //Copy epochMinute value into ram
      
      Serial.println("TXrptA>");
      packetSeqNum++;
      satSendDefaultReport(i2cdata[i2cSel],packetBufferS); //Send FIRST 6 bytes of 12 bytes of ATC Report i2c data
      //Shift upper 6 bytes down by 6 bytes
      packetSeqNum++;
      satPacketReceptionist(packetBufferB); //See if any packet response is heard from satmodem
      for(byte i;i<6;i++) {
      	i2cdata[i2cSel][i] = i2cdata[i2cSel][i+6];  
      }
      Serial.println("TXrptB>");
      satSendDefaultReport(i2cdata[i2cSel],packetBufferS); //Send LAST 6 bytes of 12 bytes of ATC Report i2c data
      satPacketReceptionist(packetBufferB); //See if any packet response is heard from satmodem
      #ifdef sendOrbPositionRpts
      	checkIfTimeToUpdateOrbPosition();  //This will send an orb positiion rpt if enough time has elapsed since the last one
      #endif
      
      
      break;
    } 
  case i2cCmdSATTxFrmEEPROM: 
    { 
      Serial.print("i2c TXSLong");
      satSendDirectlyFromI2CEEPROM(i2cdata[i2cSel]);
      break;
    }
  case i2cCmdHFUpdtTelem: 
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
      satInitRoutine(0);
      break;
    }
  case i2cCmdSATPowerOff: 
    { 
	lprintf("CC: SATMODEM OFF\n");
      digitalWrite(PWR_EN,LOW);
      break;
    }

  default:                                               // Ignore any command that is not in the list
    {
      #ifdef i2cDebug
      Serial.print("i2c cmd unknown: ");
      Serial.println(command,DEC);
      #endif
      lprintf("CC: i2c cmd unknown\n");
    }
  }
}