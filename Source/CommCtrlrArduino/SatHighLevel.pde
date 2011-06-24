//Input: pass integer 0 = no parameter setup, 1 = minimal parameter setup, 2 = most parameter setup
// Scratch space: PacketBufferB
void satInitRoutine(byte initParams) {
  unsigned int i;
  byte c=0;
  Serial.println("SatInit");
  digitalWrite(PWR_EN, LOW);   // Turn off the Satcom
  digitalWrite(sat_rts,LOW);  // Set RTS flow control to let data from SC to FIO
  delay(5000);  // Let things settle
  digitalWrite(PWR_EN, HIGH);    // Turn sat power on
  //Read next message number from EEPROM
  messageRefNum = EEPROM.read(EPLOCmessageNum);
  printf_P(PSTR("Msg #: "));
  Serial.println(messageRefNum, DEC);
  //delay(2000);
  //Initialize the Received Command Numbers Array to zeros
  for (i=EPLOCcmdCounterArrayStart;i<(EPLENcmdCounterArray + EPLOCcmdCounterArrayStart);i++) {
    EEPROM.write(i,0);
  }
  //Delay talking to modem till it's booted
  for (i=0;i<100;i++){    
    // DA pulse of the sat modem too hard to detect easily, just fixed time delay of about 10 seconds
    if (i%5 == 0){ //Modulo divide the loop counter to print a "." every 5 loops for UI
      printf_P(PSTR(".")); 
    }
    delay(100);
  }
  sat.flush();  //Clear the NewSoftSerial receive buffer


  if(initParams == 1) {
    Serial.println("Setting Params");
    //Iterate through stored parameters and values to initialize modem
    int arraysize = sizeof(satInitializeParam);
    for (int i=0;i<arraysize;i++){
      c=0;
      printf_P(PSTR("SatParam %x"),pgm_read_byte_near(satInitializeParam + i)); 
      for(i=0;i<serialRetryLimit;i++)   {
        satSetByteParameter(packetBufferB, pgm_read_byte_near(satInitializeParam + i),pgm_read_byte_near(satInitializeValues + i));
        Serial.println(". ");
        if (true == satPacketReceptionist(packetBufferB)){
          break;
        }
        Serial.println(i,DEC);
      } 
      Serial.println(" OK");  
    }
  }  
  else if (initParams == 2) {
#ifdef satAccessAllParameters
    satSetupParams();
#else
    Serial.println("SatInitError");
#endif
  }
  Serial.println("SatInit Done");
}



// Get single byte parameter 
byte satGetByteParameter(unsigned char* packetBufferLocal, unsigned char param){
  for (byte i=0; i<serialRetryLimit;i++){
    packetBufferLocal[0] = packetDTEheader;
    packetBufferLocal[1] = 0x12; //Packet type GetParameter
    packetBufferLocal[2] = 0x08; //Packet Length byte 0
    packetBufferLocal[3] = 0x00; //Packet Length byte 1
    packetBufferLocal[4] = packetSeqNum; //Sequence number/retries
    packetBufferLocal[5] = param; //parameter number
    packetBufferLocal[6] = 0x00;
    packetBufferLocal[7] = 0x00;
    fletcher_encode ( packetBufferLocal, 8 );    // Calculate checksum
    satDataSend(packetBufferLocal, 8);
    satPacketReceptionist(packetBufferLocal);
    //printf_P(PSTR("rxerr: %d\n"),rxerr);
    if (rxerr == 0 ){  //Check to see if we have "no error" status response. If not, send again.
      return packetBufferLocal[8];  //Return parameter value from packet, exiting this function
    }
  }
  return packetBufferLocal[8];
  //Increment packetSeqNum if it exceeds serialRetryLimit
  packetSeqNum++;
}


//Pass: pointer to an array that you want to put the returned data packet in.
//      Parameter byte of which parameter to return
//Output: Returns length of parameter list in bytes.  Returns 255 if there was an error in receiving.
//        List starts with first byte in output array position index 8.  

byte satGetListParameter(unsigned char* packetBufferLocal, unsigned char param){
  for (byte i=0; i<serialRetryLimit;i++){
    packetBufferLocal[0] = packetDTEheader;
    packetBufferLocal[1] = 0x12; //Packet type GetParameter
    packetBufferLocal[2] = 0x08; //Packet Length byte 0
    packetBufferLocal[3] = 0x00; //Packet Length byte 1
    packetBufferLocal[4] = packetSeqNum; //Sequence number/retries
    packetBufferLocal[5] = param; //parameter number
    packetBufferLocal[6] = 0x00;
    packetBufferLocal[7] = 0x00;
    fletcher_encode ( packetBufferLocal, 8 );    // Calculate checksum
    satDataSend(packetBufferLocal, 8);
    satPacketReceptionist(packetBufferLocal);
    if (rxerr == true ){  //Check to see if we have "no error" status response. If not, send again.
      return (packetBufferLocal[7]);  //Return list length, exiting this function
    }
  }
  //Increment packetSeqNum if it exceeds serialRetryLimit
  packetSeqNum++;
  return 255;  // Unable to receive, or to parse, return error
}



// This function generically listens on the serial buffer, and takes in any waiting packets, and acts appropriately on them
// RETURNS:  #<0 for length of data received
// RETURNS:  0  for no data received.
unsigned int satPacketReceptionist(unsigned char* packetBufferLocal) {
  unsigned int tempPackLen = 0;
  tempPackLen = satWaitReceiveOnePacket(packetBufferLocal);
  if (0 < tempPackLen) {   // See if any data was returned.
    if (satCheckResponsePacket(packetBufferLocal,tempPackLen) == true ){  //Check to see if we have "no error" status response. If no error, leave function.
      //Return list length, exiting this function
      return (tempPackLen);
    }  
  }
  return false;  // If there's no data received, return false
}







void satSetByteParameter(unsigned char* packetBufferLocal, unsigned char param, unsigned char val) {
  // Set Parameter Packet
  // packetDTEheader, 13, L, L, re, parameter, pbytes, byte1, byteN, chk0, chk1
  // You must call the satCheckResponsePacket to increment the packet number after this
  packetBufferLocal[0] = packetDTEheader;
  packetBufferLocal[1] = 0x13;
  packetBufferLocal[2] = 0x0A;
  packetBufferLocal[3] = 0x00;
  packetBufferLocal[4] = packetSeqNum;
  packetBufferLocal[5] = param;
  packetBufferLocal[6] = 0x01;
  packetBufferLocal[7] = val;
  packetBufferLocal[8] = 0x00;
  packetBufferLocal[9] = 0x00;
  // Calculate checksum
  fletcher_encode ( packetBufferLocal, 10 );
  satDataSend(packetBufferLocal, 10);
}

// SC Originated GlobalGram
// Pass: inputDataBuffer of 12 bytes ATC report to send.
//       packetBufferLocal >14B long for scratch space.
void satSendATCGlobalGram(unsigned char* inputDataBuffer, unsigned char* packetBufferLocal){
  for (byte i=0; i<serialRetryLimit;i++){
    packetBufferLocal[0] = packetDTEheader;
    packetBufferLocal[1] = 0x0A;  //SC Originated GlobalGram
    packetBufferLocal[2] = 0x13;
    packetBufferLocal[3] = 0x00;
    packetBufferLocal[4] = packetSeqNum;
    packetBufferLocal[5] = messageRefNum;
    packetBufferLocal[6] = inputDataBuffer[0];
    packetBufferLocal[7] = inputDataBuffer[1];
    packetBufferLocal[8] = inputDataBuffer[2];
    packetBufferLocal[9] = inputDataBuffer[3];
    packetBufferLocal[10] = inputDataBuffer[4];
    packetBufferLocal[11] = inputDataBuffer[5];
    packetBufferLocal[12] = inputDataBuffer[6];
    packetBufferLocal[13] = inputDataBuffer[7];
    packetBufferLocal[14] = inputDataBuffer[8];
    packetBufferLocal[15] = inputDataBuffer[9];
    packetBufferLocal[16] = inputDataBuffer[10];
    packetBufferLocal[17] = inputDataBuffer[11];
    packetBufferLocal[18] = 0x00;
    packetBufferLocal[19] = 0x00;
    fletcher_encode ( packetBufferLocal, packetBufferLocal[2] );    // Calculate checksum
    satDataSend(packetBufferLocal, packetBufferLocal[2]);
    satPacketReceptionist(packetBufferLocal);
    if (rxerr == true ){  //Check to see if we have "no error" status response. If not, send again.
      break;
    }
  } 
  messageRefNum++;
  EEPROM.write(EPLOCmessageNum,messageRefNum);
}





// Pass: inputDataBuffer of 6 bytes to send.
//       packetBufferLocal >14B long for scratch space.
void satSendDefaultReport(unsigned char* inputDataBuffer, unsigned char* packetBufferLocal){
  for (byte i=0; i<serialRetryLimit;i++){
    packetBufferLocal[0] = packetDTEheader;
    packetBufferLocal[1] = 0x09;
    packetBufferLocal[2] = 0x0E;
    packetBufferLocal[3] = 0x00;
    packetBufferLocal[4] = packetSeqNum;
    packetBufferLocal[5] = messageRefNum;
    packetBufferLocal[6] = inputDataBuffer[0];
    packetBufferLocal[7] = inputDataBuffer[1];
    packetBufferLocal[8] = inputDataBuffer[2];
    packetBufferLocal[9] = inputDataBuffer[3];
    packetBufferLocal[10] = inputDataBuffer[4];
    packetBufferLocal[11] = inputDataBuffer[5];
    packetBufferLocal[12] = 0x00;
    packetBufferLocal[13] = 0x00;
    fletcher_encode ( packetBufferLocal, packetBufferLocal[2] );    // Calculate checksum
    satDataSend(packetBufferLocal, packetBufferLocal[2]);
    satPacketReceptionist(packetBufferLocal);
    if (rxerr == true ){  //Check to see if we have "no error" status response. If not, send again.
      break;
    }
  } 
  messageRefNum++;
  EEPROM.write(EPLOCmessageNum,messageRefNum);
}




//Pass: inputdatabuffer of message body
//.     PaxketBufferLocal scratch interim storage of whole packet - msg length plus 7.
//.     messageLen length of message body

void satSendDefaultMsg(unsigned char* inputDataBuffer, unsigned char* packetBufferLocal, unsigned int messageLen){
  unsigned int a,q=0;
  unsigned int packetLen = messageLen+8;
  unsigned char packetLenLowByte = (unsigned char) packetLen;
  a = ((unsigned int) packetLen >> 8);
  unsigned char  packetLenHighByte = (unsigned char) a;
  for (byte i=0; i<serialRetryLimit;i++){
    packetBufferLocal[0] = packetDTEheader;
    packetBufferLocal[1] = 0x07;
    packetBufferLocal[2] = packetLenLowByte;
    packetBufferLocal[3] = packetLenHighByte;
    packetBufferLocal[4] = packetSeqNum;
    packetBufferLocal[5] = messageRefNum;
    for(q=0;q<messageLen;q++){
      packetBufferLocal[q+6] = inputDataBuffer[q];
    }
    packetBufferLocal[q+6] = 0x00;
    packetBufferLocal[q+7] = 0x00;

    fletcher_encode ( packetBufferLocal, packetBufferLocal[2] );    // Calculate checksum
    satDataSend(packetBufferLocal, packetBufferLocal[2]);
    satPacketReceptionist(packetBufferLocal);
    if (rxerr == true ){  //Check to see if we have "no error" status response. If not, send again.
      break;
    }
  }
  messageRefNum++;
  EEPROM.write(EPLOCmessageNum,messageRefNum);
}

//Pass: inputdatabuffer 6 byte array of: type code, Value bytes 0,1,2,3, gwy_id
//      packetBufferLocal - scratch byte array of 14 bytes
void satSendCommCommand(unsigned char* inputDataBuffer, unsigned char* packetBufferLocal){
  for (byte i=0; i<serialRetryLimit;i++){

    packetBufferLocal[0] = packetDTEheader;
    packetBufferLocal[1] = 0x03;  // Communications Command
    packetBufferLocal[2] = 0x0D;
    packetBufferLocal[3] = 0x00;
    packetBufferLocal[4] = packetSeqNum;
    packetBufferLocal[5] = inputDataBuffer[0]; //type code of comm command
    packetBufferLocal[6] = inputDataBuffer[1]; //value byte 0
    packetBufferLocal[7] = inputDataBuffer[2]; //value byte 1
    packetBufferLocal[8] = inputDataBuffer[3]; //value byte 2
    packetBufferLocal[9] = inputDataBuffer[4]; //value byte 3
    packetBufferLocal[10] = inputDataBuffer[5];  //gateway id, for type codes 0,1,3,5-15,23
    packetBufferLocal[11] = 0x00;
    packetBufferLocal[12] = 0x00;

    fletcher_encode ( packetBufferLocal, packetBufferLocal[2] );    // Calculate checksum
    satDataSend(packetBufferLocal, packetBufferLocal[2]);
    satPacketReceptionist(packetBufferLocal);
    if (rxerr == true ){  //Check to see if we have "no error" status response. If not, send again.
      break;
    }

  }
}


//Send SC-Originated Position Report
// Setup: You should update the position array from GPS or something before this func
// Pass: position - array that has the 6 bytes
//       which is the Lat and Lon using ORBCOMM's proper compression format.
//       packetBufferLocal - >14B long for scratch space.
void satSendSCOrigPositionRpt(unsigned char* position, unsigned char* packetBufferLocal){
#ifdef actionLabels
  printf_P(PSTR("Sending SC ORB Pos Rpt\n"));
  #endif
  lprintf("CC:OrbPosRpt\n");
  for (byte i=0; i<serialRetryLimit;i++){
    packetBufferLocal[0] = packetDTEheader;
    packetBufferLocal[1] = 0x11;
    packetBufferLocal[2] = 0x0E;
    packetBufferLocal[3] = 0x00;
    packetBufferLocal[4] = packetSeqNum;
    packetBufferLocal[5] = messageRefNum;
    packetBufferLocal[6] = position[0];
    packetBufferLocal[7] = position[1];
    packetBufferLocal[8] = position[2];
    packetBufferLocal[9] = position[3];
    packetBufferLocal[10] = position[4];
    packetBufferLocal[11] = position[5];
    packetBufferLocal[12] = 0x00;
    packetBufferLocal[13] = 0x00;
    fletcher_encode ( packetBufferLocal, packetBufferLocal[2] );    // Calculate checksum
    satDataSend(packetBufferLocal, packetBufferLocal[2]);
    satPacketReceptionist(packetBufferLocal);
    if (rxerr == true ){  //Check to see if we have "no error" status response. If not, send again.
      break;
    }
  } 
  messageRefNum++;
  EEPROM.write(EPLOCmessageNum,messageRefNum);
}




void checkIfTimeToUpdateOrbPosition() {
  //See if position has changed be lots
  if ((lastTimeOrbPositionRptSent + minutesBetweenOrbPositionRpts) <= epochMinutes) {
  #ifdef actionLabels
    printf_P(PSTR("EPM:%d  LastPosTm:%d\n"),epochMinutes,lastTimeOrbPositionRptSent);
    #endif
    //Time to send position report!
    packetSeqNum++;
    satSendSCOrigPositionRpt(latestPosition,packetBufferS);
    lastTimeOrbPositionRptSent = epochMinutes;  //Update the last time sent to now
  }
}


// Send telemetry data via DEFAULT MESSAGE from I2C EEPROM
// Interleave reading I2C and sending sat serial
void satSendDirectlyFromI2CEEPROM(unsigned char* packetBufferLocal) {
  unsigned int telemetryLength=0, startAddress=0,endAddress=0,packetLen;
  byte tempDataByte,packetLenLowByte,packetLenHighByte;
  //initialize global checksum accumulation variables
  checkSumA = 0;
  checkSumB = 0;

  //Serial.println("SatSendI2CEP");
  //Copy start and end address from i2cdata array
  //Start address
  unsigned int a = packetBufferLocal[0];  // Put high byte in first
  //printf_P(PSTR("1a:%x\n"),a);
  startAddress = ((a << 8) + packetBufferLocal[1]);  // Shift high bits up, then Put low byte in 
  //printf_P(PSTR("1(a<<8):%x"),(a<<8));
  //End address
  a = packetBufferLocal[2];  // Put high byte in first
  endAddress = ((a << 8) + packetBufferLocal[3]);  // Shift high bits up, then Put low byte in 
  printf_P(PSTR("startAddress: %x endAddress: %x"),startAddress,endAddress);
  //Calculate length of message
  telemetryLength = endAddress-startAddress;
  printf_P(PSTR(" telemetry string Len: %d\n"),telemetryLength);
  if (maxTelemLenConst < telemetryLength) {
    //Send only up to sane length for really long messages.
    lprintf("CC: MSG2long!, sending %d of %d\n", maxTelemLenConst, telemetryLength);
    telemetryLength = maxTelemLenConst;  //Set length to the max allowable length
    return;

  }
  printf_P(PSTR("MHA:%d"),messageRefNum);
  packetLen = 6 + telemetryLength + 2; // Total packet length including header and checksum 
  // Split length int into two bytes
  packetLenLowByte = (unsigned char) packetLen;
  a = ((unsigned int) packetLen >> 8);
  packetLenHighByte = (unsigned char) a;
  //Loop that will construct message, checksum it between every byte, and send every byte
  for (byte i=0; i<serialRetryLimit;i++){
    //Send packet header to sat modem
    packetBufferLocal[0] = packetDTEheader;
    fletcherChk_step(packetBufferLocal[0]);			//Calculate rolling fletcher checksum
    packetBufferLocal[1] = 0x07;
    fletcherChk_step(packetBufferLocal[1]);			//Calculate rolling fletcher checksum
    packetBufferLocal[2] = packetLenLowByte;
    fletcherChk_step(packetBufferLocal[2]);			//Calculate rolling fletcher checksum
    packetBufferLocal[3] = packetLenHighByte;
    fletcherChk_step(packetBufferLocal[3]);			//Calculate rolling fletcher checksum
    packetBufferLocal[4] = packetSeqNum;
    fletcherChk_step(packetBufferLocal[4]);			//Calculate rolling fletcher checksum
    packetBufferLocal[5] = messageRefNum;
    fletcherChk_step(packetBufferLocal[5]);			//Calculate rolling fletcher checksum
    //Send header bytes to sat modem
    satDataSend(packetBufferLocal,6);
    #ifdef satDataDebug
    Serial.println("MsgToSat:");
    #endif
    //Send data byte by byte to sat modem
    for(unsigned int g = 0; g<telemetryLength; g++ ) {
      unsigned int tempAddr = startAddress + g;
      //  printf_P(PSTR(" tempAddr: "));
      //  Serial.println(tempAddr,HEX);
      tempDataByte = I2CeePROMRead(i2ceePROM,tempAddr);
      fletcherChk_step(tempDataByte);				//Calculate rolling fletcher checksum
      //Send byte to sat modem
      sat.print(tempDataByte);
      #ifdef satDataDebug
      Serial.print(tempDataByte,HEX);
      printf_P(PSTR(" "));
      #endif
    }
    //Add two ending zeroes for Fletcher Checksum
    fletcherChk_step((byte)0);
    fletcherChk_step((byte)0);    
    //send two checksum bytes
    fletcherChk_stepOut();						//Calculates final checksum, leaves in A and B
    sat.print(checkSumA);						//Send high byte of checksum
    sat.print(checkSumB);						//Send low byte of checksum
    //wait for LL response ack
    satPacketReceptionist(packetBufferLocal);
    if (rxerr == true ){  //Check to see if we have "no error" status response. If not, send again.
      break;
    }
  }
  messageRefNum++;
  EEPROM.write(EPLOCmessageNum,messageRefNum);
}



//Check to see that we haven't heard a real sat in a while, and then clear out the msgs in the inbound queue
void satClearInboundMsgsIfNeeded(){
  if(true == satSyncFlag) {  //if there's sat sync available, skip clearing msgs and reset counter
    satMsgTermClearoutCounter=0;
    satInboundMsgsCleared=false;
    return; //exit this whole function
  } 
  else {
    satMsgTermClearoutCounter++;  //If not in sat sync, increment the counter by one 
  }
  if((satMsgTermClearoutCounter > 240) && ( false == satInboundMsgsCleared) ){
#ifdef satDataDebug
    printf_P(PSTR("ClrInboundMsgs\n"));
#endif
    lprintf("CC:ClearMsgs\n");
    packetBufferS[0]=20; //Clear all SC-Terminated Messages
    packetBufferS[1]=0;
    packetBufferS[2]=0;
    packetBufferS[3]=0;
    packetBufferS[4]=0;
    packetBufferS[5]=0;
    satSendCommCommand(packetBufferS,packetBufferB);
    satMsgTermClearoutCounter=0;
    satInboundMsgsCleared = true;
    packetSeqNum++;
  } 
}


void satAOSLOSAlert(){  //Signal Aquisition indicator
  satSyncFlag = digitalRead(sat_sa);   //read sat available pin
  if (satSyncFlag != prevSatSyncStateFlag) {
    if(satSyncFlag == true){
      printf_P(PSTR("AOS\n"));
      lprintf("CC:AOS\n");
    } 
    else {
      printf_P(PSTR("LOS\n"));
      lprintf("CC:LOS\n");
    }
    prevSatSyncStateFlag = satSyncFlag;
  }
}

//Master Outgoing message operations management
//Includes queueing, clearing the queue, monitoring for sent status and prioritization
void satOutgoingMsgOperations(){
  //	Is there sat sync right now?
  //	Are we waiting for uplink commands right now? (i.e. shouldn't be transmitting)
  //Check for Satellite Availability to trigger sending telemetry
  if (LOW == digitalRead(sat_sa)){   //read sat available pin 
	  //Is this a transition to LOW from previous HIGH?
	  if (satSyncFlag) {  //If satSyncFlag is high and the pin is low, we've just transitioned
	  	//Clear long message from sat modem here
	  	satClearOutgoingMHA(satLongMsgInOBQMHA);
	  	satSyncFlag = false);  //set Sync flag to actual state of pin, which is low
	  } 
	  return;  //Leave if there's no signal, nothing left to do.
  } else {
  satSyncFlag = true;
  }
  //Check for being in the delay timer
  if (satWaitingForUplinkCmdsFlag) return;
  //Read in the first gateway from sat
  gwy_id = satGetByteParameter(packetBufferB,0x10);  
  //Is Gateway 0?  if 0 then we have no gateway to talk to
  if ((0 == gwy_id) || (rxerr != 0)) return;  // IF Gwy = 0 or there was an error, then just leave.
  //Check to see if the sat queue is empty or not
  if (0 == satGetByteParameter(satParamQOBqty)){  //Empty if zero
  	// SatQ is EMPTY -------------------
  	if (ATCRptReady) {  //Is a new ATC report Ready to send?
  		//Yes there are ATC reports ready to send
  		//LOAD ATC Report pair
  		satSendLatestStoredATCPkt();
  		return;
  	} else {
  		//No ATC report ready to send
  		if(LongMsgReady) {  //Is there a new long Message waiting in the eeprom queue?
  			//YES there is a new long message waiting in the eeprom queue
  			//LOAD next Long message
  			satSendNextQueuedLongMSG(packetBufferS);
  		} else { 
  			//NO there is no long message waiting in the eeprom queue
  			return;
  		}
  	}
  } else {  //SatQ NOT EMPTY -------------------
  	// There are message(s) in the outbound queue
  	if (ATCRptReady) {  //Is a new ATC report Ready to send?
  		//Yes there are ATC reports ready to send
  		//Clear SatQ of Old ATC Messages
  		satClearATCPacketsFromSatQ();
  		//Clear SatQ of Old Long Messages
  		satClearLongMessagefromSatQ();
  		//LOAD ATC Report pair
  		satSendLatestStoredATCPkt();
  		return;
  	} else { 
  		//NO there are no ATC reports ready to send
  		//Is existing ATC pair sent from SatQ?
  		satHasATCPairBeenTXed()  // <---  THIS FUNC NOT DONE
  		
  }
  
  
  
  
  
  // ------------- REMOVE STUFF BELOW HERE ---------- 
  
  
      //	Is there an ATC report ready to transmit?
      if (true == ATCRptReady) {   
        //Read in the first gateway from sat
        gwy_id = satGetByteParameter(packetBufferB,0x10);  
        printf_P(PSTR("GetGwy:"),gwy_id);
    	if ((0 != gwy_id) && (rxerr == 0)) {  //If there's a gateway listed that's not 0, there's a gateway to talk to
        	if (satLongMsgInOBQ == true) {  //Test to see if there's a long message sitting in the q
        		printf_P(PSTR("REMOVING LONG MSG MHA %d FROM SAT Q, WILL SEND LATER\n"),satLongMsgInOBQMHA); 
        		//Remove long message from sat modem q and roll back pointers so it gets sent next time
        		satQueueBackupOne();
        	}
        	//Send report here
       		 satSendLatestStoredATCPkt();
        	ATCRptReady = false;  //ATC Report has been sent.
        	return;  //Do not continue, as there's no point in sending a long message when the ATC report just got sent.
        }
      }
      satSyncFlag = digitalRead(sat_sa);   //read sat available pin update
      //  LONG MESSAGE
      //check to see if we're ok to send a Long message
      if ((true == satLongMsgStored) && (true == satOBQueueEmpty) && (true == satSyncFlag)){
      	//Read in the first gateway from sat
      	
      	gwy_id = satGetByteParameter(packetBufferB,0x10);  //Read in the first gateway from sat
      	printf_P(PSTR("GetGwy: %d"),gwy_id);
    	if ((0 != gwy_id) && (rxerr == 0)) {  //If there's a gateway listed that's not 0, there's a gateway to talk to
        	satSendNextQueuedLongMSG(packetBufferS);
        	satOBQueueEmpty = false;
        	
        }
      }
    }
  // =============  REMOVE STUFF ABOVE HERE ==================
}


