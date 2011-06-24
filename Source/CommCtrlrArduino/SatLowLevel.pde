





// Pass an array pointer to whole formed packet, with checksum.  Include length of packet including checksum
void satDataSend(unsigned char* packarray, unsigned char len){
  for (int i = 0;i < len; i++)
  {
    sat.print(packarray[i], BYTE); 
  }
#ifdef satDataDebug
  //Print out data that was sent to satellite modem
  printf_P(PSTR("SentToSat: "));
  for (int i = 0; i<len; i++) {

    printf_P(PSTR("%x "),packarray[i]);
  }
  Serial.println();
#endif
}


//Check the response packet to see what type it is, and deal accordingly
// Returns: True if response is status packet is OK.  False if not OK.
// Returns: rxerror has error code from LLACKs, 0 is good.
byte satCheckResponsePacket(unsigned char* packetBufferLocal,unsigned int packLen){
  //Packet response type
  rxerr = 0;
  //First check to see if it's the type we want, 0x06 from address, not 0x05.
  if ( 0x05 == packetBufferLocal[0]) {
    satSend5LLACK();  //Send an error ack to satmodem to try to force it to resend using correct packet seq number method.
    return false;  // Indicates no valid data was in packet.
  }

  //Choose action to take based on the type of packet indicated in the packet header
  switch(packetBufferLocal[packetTypeIndex]) {  

  case 0x01:  //Link Level ACK
    Serial.println("\nRXd LLACK");
    if (packetBufferLocal[packetLLstatusCodeIndex] != 0){
      rxerr = packetBufferLocal[packetLLstatusCodeIndex];
      //Do not increment packet number
      printf_P(PSTR("\nSat Ack Error: "));
      Serial.println(rxerr,DEC);
      return false;  // Just return false for all failing problems that need retry
    }
    else {   // Return Good, and increment packet sequence number for the next serial packet to use
      rxerr = 0;
      //Increment packetSeqNum 
      packetSeqNum++;
      return true;
      break;
    }
  case 0x04:  // System Announcement
    satSendLLACK();
    Serial.println("\nRXSysAnnPkt");
#ifdef satDataDebug
    satPrintPacket(packetBufferLocal);  
#endif
  case 0x05:  // Status Packet
    satSendLLACK();
    Serial.println("\nRXStatPkt");
#ifdef satDataDebug
    satPrintPacket(packetBufferLocal);
#endif
    break;
  case 0x0B:  // System Response, info about messages usually
    Serial.println("\nRXSysResp"); 
#ifdef satDataDebug
    satPrintPacket(packetBufferLocal);
#endif
	satSendLLACK();
    satParseSysResp(packetBufferLocal);
    break;
  case 0x0C: // Incoming SC Terminated Message
    //Reset heartbeat immediately for all incoming messages
    satSendLLACK();
    cdnCmdResetTimer();
    satProcessIncomingMessage(packetBufferLocal,packLen);
    break;
  case 0x0D: // Incoming SC Terminated 5-byte Command
    //Reset heartbeat immediately for all incoming commands
    satSendLLACK();
    cdnCmdResetTimer();
    satProcessIncomingCommand(packetBufferLocal);
    break;
  case 0x0E:  //Incoming SC Terminated GLOBALGRAM
    //satProcessIncomingGG(packetBufferLocal); 
    satSendLLACK();
    cdnCmdResetTimer();
    break;
  case 0x14: //Parameter Response, comes back after setting or getting any parameter val
    if (packetBufferLocal[packetRetStatusCodeIndex] != 0){
      rxerr = packetBufferLocal[packetRetStatusCodeIndex];
      return false;  // Just return false for all failing problems that need retry
    }
    else {  // Return Good, and increment packet sequence number for the next serial packet to use
      rxerr = 0;
      //Increment packetSeqNum 
      packetSeqNum++;
      satSendLLACK();
      return true;
    } 
    break;
  default:  // Default for unknown packets

    Serial.println("SatPktUnk:");
    packetBufferLocal[3] = 0; 
    packetBufferLocal[2] = 20; 
    satPrintPacket(packetBufferLocal);
    return false;
    //By not incrementing the packetSeqNum++ here, the previous command from the DTE will continue to retransmit 


  }
}



// Purpose:  To wait a few seconds for satellite modem to say something, then read it in.
//Pass the buffer to be written to, and the true/false of whether you want to print the result to debug serial
// Returns: Length in bytes of packet received
unsigned int satWaitReceiveOnePacket(unsigned char* packetBufferLocal) {
  unsigned int packLen = 0;   //Start high, and read the actual length that comes in
  unsigned int i=0,r=0,limit = 50;
  boolean dataReceivedFlag = false;
#if  defined(satDataDebug) || defined(satPrintEveryIncomingPacketDebug)
  printf_P(PSTR("satIn?"));
#endif
  for (i=0;i<limit;i++){
    // Copy Serial Data From NewSoftSerial RX Buffer
    if (sat.available()){
    #if  defined(satDataDebug) || defined(satPrintEveryIncomingPacketDebug)
      printf_P(PSTR("yep"));
    #endif
      dataReceivedFlag = true;
#ifdef satPrintEveryIncomingPacketDebug
      printf_P(PSTR("In:"));
#endif 
      //Copy the data in the 64 byte serial RX buffer into the packetBuffer
      // sat.available returns # of bytes avail to be read, when it's zero, while loop exits
      while (sat.available()){
        packetBufferLocal[r] = sat.read();
        //grab packet length index when both bytes have arrived
        if (r == (packetLengthIndex + 1)) {
          unsigned int a = packetBufferLocal[(packetLengthIndex + 1)];  // Put high byte in first
          packLen = ((a << 8) + packetBufferLocal[packetLengthIndex]);  // Shift high bits up, then Put low byte in 
          // Now packLen has the real packet length in it.  This will be how many times the for loop copies data from the buffer.
        }

#ifdef satPrintEveryIncomingPacketDebug
        //Print out the data received from sat modem
        printf_P(PSTR("%x "),packetBufferLocal[r]); 
#endif
        //Advance to the next byte in the rx buffer
        r++;
        //printf_P(PSTR("r:%d\n"),r);
        //printf_P(PSTR(" packLen: %d\n"),packLen); 
        //  Exit the loop if we've reached the end of one packet.  Compare R to packetlength index 
        //   just to make sure we don't read packlen till it has actual value in it.
        if((r >= packLen)&&(r > ( packetLengthIndex + 1))) {  
          i=limit; //terminate for loop too with this
          //Serial.println("break for r packlen");
          break;
        }
      }

    }
    delay(100);
  }

  if(true == dataReceivedFlag) {
    //Check for valid packet address to me
    if ((0x06 != packetBufferLocal[0]) && (0x05 != packetBufferLocal[0])) {
      printf_P(PSTR("BAD PKTADR: %x\n"),packetBufferLocal[0]);
      return 0;
    }
	#ifdef satDataDebug
    satPrintPacket(packetBufferLocal);
    #endif
    //If the checksum for this packet is wrong, don't try to do anything more with it.
#ifndef skipSatInCHKs
    if (false == satCheckIncomingPacketChecksum(packetBufferLocal, packLen)) {
#ifdef satDataDebug
      Serial.println("\nCHKSUM BAD");
#endif
      return 0;
    }
#ifdef satDataDebug
    Serial.println("\nCHKSUM OK");
#endif
#else
    Serial.println("CHKSUM SKIP");
#endif
    //If this is a duplicate packet, just skip doing anything further with it, and report it as 0 length.
    if (true == satCheckForDupePacket(packetBufferLocal)) {
      return 0;
#ifdef actionLabels
      Serial.println("\nDUPE PKT");
#endif
    } 
    else {
      return packLen;  //Returns the length of the captured, non-duplicate packet
    }

  }
  else {
#ifdef satDataDebug
    Serial.println("None");
#endif
    return false;
  }

}



/****************************
 *
 *   Fletcher Checksum Routines
 *
 ****************************/

// Give a buffer, with two extra bytes at the end
// Also give a LONG value representing the full length (including extra two)

void fletcher_encode( unsigned char* buffer, long counter ) {
  int i;
  unsigned char c0 = 0;
  unsigned char c1 = 0;
  //printf_P(PSTR("Checksum Encode\n"));
  *( buffer + counter - 1 ) = 0;  //Set last two bytes to zero
  *( buffer + counter - 2 ) = 0;  //Set last two bytes to zero
  for (i=0; i<counter; i++) {
    c0 = c0 + *( buffer + i );

    c1 = c1 + c0;
    //printf_P(PSTR("%x%x "),c1,c0);
  }
  *( buffer + counter - 2 ) = c0 - c1;
  *( buffer + counter - 1 ) = c1 - 2*c0;
}


long fletcher_decode( unsigned char* buffer, long counter) {
  long result = 0;
  int i;
  unsigned char c0 = 0;
  unsigned char c1 = 0;
  #ifdef checkSumDebug
  printf_P(PSTR("ChckD:"));
  #endif
  for ( i=0; i < counter; i++ ) {
    c0 = c0 + *( buffer + i );
    c1 = c1 + c0;
    #ifdef checkSumDebug
    printf_P(PSTR("%x%x "),c1,c0);
    #endif
  }
  return ( (long) (c0 + c1) );  
}


// Increments fletcher checksum, call between every byte.  at end of message, call fletcherChk_stepOut
// Inputs:  checkSumA, CheckSumB, current byte
// Outputs: checkSumA, CheckSumB
void fletcherChk_step(byte currentByte) {
  #ifdef checkSumDebug
  printf_P(PSTR("currByte:"));
  #endif
  checkSumA = checkSumA + currentByte;
  checkSumB = checkSumB + checkSumA;
  #ifdef checkSumDebug
  printf_P(PSTR("%x chkstpBA:%x%x\n"), currentByte,checkSumB,CheckSumA);
  #endif
}


// Inputs:  checkSumA, CheckSumB
// Outputs: checkSumA, CheckSumB as final bytes to go into message
void fletcherChk_stepOut() {
  long checksum = 0;
  byte c0,c1;


  c0 = checkSumA;
  c1 = checkSumB;
  checkSumA = (c0 - c1);
  checkSumB = (c1 - 2 * c0);
  #ifdef checkSumDebug
    printf_P(PSTR("postCSO checkSumA: %x\npostCSO checkSumB: %x\n"),checkSumA, checkSumB);
  #endif
}


// Compare SERIAL packet counter in received packet to the satLatestPacNum global variable.  If it's the same,
// that means it's a duplicate!  if it's not the same, copy it to the satLatestPacNum as the current,
// and return false.   Calling functions deal with the fact that it's a dupe.
byte satCheckForDupePacket(unsigned char* packetBufferLocal) {	
  //packet number is always byte 4
  if (satLatestPacNum == packetBufferLocal[4]) {
    //It's a duplicate!
    return true;  //True
  } 
  else {
    // Set new current packet number
    satLatestPacNum = packetBufferLocal[4];
  }
  return false;  // false
}

//verify checksum in packet
// Returns:  TRUE if checksums match.
byte satCheckIncomingPacketChecksum(unsigned char* packetBufferLocal, unsigned int packLen) {
  byte supposedchecksumLowByte, supposedchecksumHighByte;
  //Store existing,supposed checksum
  supposedchecksumLowByte = packetBufferLocal[packLen-2];
  supposedchecksumHighByte = packetBufferLocal[packLen-1];

  //Recalculate checksum
  fletcher_encode(packetBufferLocal, (long) packLen);

  //Compare to stored original checksum
  if ((packetBufferLocal[packLen-2] == supposedchecksumLowByte) && (packetBufferLocal[packLen-1] == supposedchecksumHighByte)){
    return true;   
  }
  return false;
}

//Simple ack for anything the satmodem sends us.  Speeds up round trip comms.
void satSendLLACK() {
#ifdef satDataDebug
  printf_P(PSTR("SndLLACK"));
  #endif
  packetBufferS[0] = packetDTEheader;
  packetBufferS[1] = 0x01;
  packetBufferS[2] = 0x07;
  packetBufferS[3] = 0x00;
  packetBufferS[4] = 0x00;
  packetBufferS[5] = 0x00;
  packetBufferS[6] = 0x00;
  fletcher_encode(packetBufferS, 7);    // Calculate checksum
  satDataSend(packetBufferS, 7);
  packetSeqNum++;
}

//Send this when the sat modem sends us something beginning with a 5.  Hope to force it to respond with a 6.
// See p 11 of Orbcomm Serial Spec on Invalid packets
void satSend5LLACK() {
  packetBufferS[0] = 0x86;
  packetBufferS[1] = 0x01;
  packetBufferS[2] = 0x07;
  packetBufferS[3] = 0x00;
  packetBufferS[4] = 0x06;  // Packet rejected, unknown type
  packetBufferS[5] = 0x00;
  packetBufferS[6] = 0x00;
  fletcher_encode(packetBufferS, 7);    // Calculate checksum
  satDataSend(packetBufferS, 7);
  packetSeqNum++;
}


// keep track of uplink command counter numbers that have been used to prevent dupes.  
// issue here is that satcom network can deliver Command multiple times.
// Uses eeprom to save ram
boolean satCheckForDupeCommand (unsigned char receivedCommandNum) {
  byte tempCommandNum,j;
  int e;
  //Search through list of received commands
  for (e=EPLOCcmdCounterArrayStart;e<(EPLENcmdCounterArray + EPLOCcmdCounterArrayStart);e++) {
    tempCommandNum = EEPROM.read(e);
    if (tempCommandNum == receivedCommandNum) {
      return true;  //Exit function and return True if it's a duplicate command.
    }  // If not duplicate, continue looping to check the rest of the received commands
  }
  //It's not a duplicate, so move the list up by one.
  //START AT THE HIGH END OF THE ARRAY AND GO DOWN!
  for (e=(EPLENcmdCounterArray + EPLOCcmdCounterArrayStart);e>EPLOCcmdCounterArrayStart;e--){
    j=EEPROM.read(e-1);  //Copy next lower value 
    EEPROM.write(e,j);  // Paste it in the current spot.
  }
  //Insert received Command Number into the bottom spot.
  EEPROM.write(EPLOCcmdCounterArrayStart,receivedCommandNum);
  return false;  // If it makes it here, it's not a duplicate, so return false.
}


void msgCounterHouseKeeping(){
  if ( 0xFF == messageRefNum ) {
    messageRefNum = 0;  // Skip over 0xFF, seems to cause problems with sat modem.
  }	
}

//Save ATCpacket into internal EEPROM over the top of the old one.
void satSaveATCPkt() {
#ifdef actionLabels
  printf_P(PSTR("OverwriteATC\n"));
#endif
  lprintf("CC:SaveATCPkt\n");
  for(byte i=0;i<(EPLOCAtcReportPairArray);i++){
    EEPROM.write(i+EPLOCAtcReportPairArrayStart,i2cdata[i2cSel][i]);  //Copy the ATC report data from the i2c command.
  }
  ATCRptReady = true;  //Indicate globally that a report is stored and ready.
  //Clear out any previous unsent ATC messages that are queued the sat modem
  satClearATCPacketsFromSatQ();
}


void satClearATCPacketsFromSatQ(){
  //Clear out any previous unsent ATC messages that are queued the sat modem
  printf_P(PSTR("Clearing ATC if flags indicate there are ATC packets stored.\n"),satMHANumReportA);
  if (true == satATCRptPairSittingInSatModemQueue) { //Tests true if any ATC short reports still in modem queue
    
    if(0xFF != satMHANumReportA) {  //0xFF is the value set when the message has already been sent
      //Clear MHA number A
      lprintf("CC:ClrMHA%d\n",satMHANumReportA);
      printf_P(PSTR("ClrMHA%d\n"),satMHANumReportA);
      satClearOutgoingMHA(satMHANumReportA);
    }
    if(0xFF != satMHANumReportB) {  //0xFF is the value set when the message has already been sent
      //Clear MHA number B
      lprintf("CC:ClrMHA%d\n",satMHANumReportB);
      printf_P(PSTR("ClrMHA%d\n"),satMHANumReportB);
      satClearOutgoingMHA(satMHANumReportB);
    }
  } else {
  printf_P(PSTR("According to flag satATCRptPairSittingInSatModemQueue == false\n"),satMHANumReportA);
  }
}

void satClearLongMessagefromSatQ() {
	printf_P(PSTR("Check satLongMsgInOBQMHA flag:%d\n"),satLongMsgInOBQMHA);
	if(true == satLongMsgInOBQ) {  
		printf_P(PSTR("Clr Long Msg MHA:%d\n"),satLongMsgInOBQMHA);
		satQueueBackupOne();  //Clear out the message in the queue
		}
}



//Send ATC report that's been stored in EEPROM
void satSendLatestStoredATCPkt(){
  for(byte i=0;i<(EPLOCAtcReportPairArray);i++){
    packetBufferB[i] = EEPROM.read(i+EPLOCAtcReportPairArrayStart);  //Pull ATC report back into ram
  }
  //actually transmit the two packets.  Requires no local data passing, all globals are passed.
  #ifdef actionLabels
  printf_P(PSTR("PutStoredATCintoModemA>%d\n"),messageRefNum);
  #endif
  packetSeqNum++;
  satMHANumReportA = messageRefNum;  //Store the sat modem message ref num of ReportA
  satSendDefaultReport(packetBufferB,packetBufferS); //Send FIRST 6 bytes of 12 bytes of ATC Report i2c data
  //Shift upper 6 bytes down by 6 bytes
  packetSeqNum++;
  satPacketReceptionist(packetBufferB); //See if any packet response is heard from satmodem
  for(byte i;i<6;i++) {
    packetBufferB[i] = packetBufferB[i+6];  
  }
  satMHANumReportB = messageRefNum;  //Store the sat modem message ref num of ReportB
  #ifdef actionLabels
  printf_P(PSTR("PutStoredATCintoModemB>%d"),messageRefNum);
  #endif
  satSendDefaultReport(packetBufferB,packetBufferS); //Send LAST 6 bytes of 12 bytes of ATC Report i2c data
  packetSeqNum++;
  satPacketReceptionist(packetBufferB); //See if any packet response is heard from satmodem
  satATCRptPairSittingInSatModemQueue = true;
  ATCRptReady = false;  //ATC Report has been sent.
}

//Often the response to a CommCommand
void satParseSysResp(unsigned char* packetBufferLocal) {
  /*
  5 - Origin type  3 = the Sat modem itself
   7 - Status byte
   9 - mha number
   */

  switch (packetBufferLocal[7]) {  //Status byte
  case 0x0A:  // Message transfer failed, will requeue
  	#ifdef actionLabels
  	printf_P(PSTR("=msgTxFail#%d"),packetBufferLocal[9]);
  	#endif
    lprintf("CC:msgTxFail\n");
    break;
  case 0x0B:  //Response to a COMM command that we sent to satmodem
    printf_P(PSTR("=commCmdRspDiag:%d re#%d\n"),packetBufferLocal[8],packetBufferLocal[9]);
  	break;
  case 0x0F: // Message was received by indicated ack originator
  	#ifdef actionLabels
    printf_P(PSTR("=msgDlvd#%d"),packetBufferLocal[9]);
    #endif
    lprintf("CC:msgDlvd\n");
    satMsgSentCleanup(packetBufferLocal);
    break;
  default:
  	#ifdef actionLabels
    printf_P(PSTR("=statusUnk#%d  Msg#%d \n"),packetBufferLocal[7],packetBufferLocal[9]);
    #endif
    lprintf("CC:SysRes:%d\n",packetBufferLocal[7]);
  }
}


void satMsgSentCleanup(unsigned char* packetBufferLocal){
	byte tempOBQ;
	
  //if(true == satATCRptPairSittingInSatModemQueue
  // byte 9 is MHA Message number
  if (packetBufferLocal[9] == satMHANumReportA) {  //Satreport Part A was sent
    lprintf("CC:ConfSntMHAa%d\n",satMHANumReportA);
    printf_P(PSTR("Confirmed Sent MHA A:%d,Setting MHAA to 0xFF\n"),satMHANumReportA);
    satMHANumReportA = 0xFF;  //Set to a flag value that it will never naturally be set to
  }
  if (packetBufferLocal[9] == satMHANumReportB){  //Satreport Part B was sent
    lprintf("CC:ConfSntMHAb%d\n",satMHANumReportB);
    printf_P(PSTR("Confirmed Sent MHA B:%d, Setting MHAB to 0xFF\n"),satMHANumReportB);
    satMHANumReportB = 0xFF;  //Set to a flag value that it will never naturally be set to

  }

  //When both messages get out the sat modem this should test true for the first check
  if ((0xFF == satMHANumReportA) && ( 0xFF == satMHANumReportA) && (true == satATCRptPairSittingInSatModemQueue)) {
    satATCRptPairSittingInSatModemQueue = false;
    //Once all messages are sent, pause for uplink command before loading other messages
    satWaitingForUplinkCmdsFlag = true;
    //Set the appointment time when it will be OK to send more messages
    satWaitIsOverTime = millis() + satUplinkWaitDelayMillis;
  }  	

  //0x15 is queued outbound messages
  //check to see how many messages are in the outbound queue
  tempOBQ = satGetByteParameter(packetBufferS, 0x16);
  #ifdef actionLabels
  printf_P(PSTR("GetOBQ# %d"),tempOBQ);
  #endif
  if(0 == tempOBQ){
    printf_P(PSTR("Outbound Queue Empty!"));
    satOBQueueEmpty = true;
  } else {
    satOBQueueEmpty = false;
  }
}

void satClearOutgoingMHA(byte mhaNumber){
  printf_P(PSTR("Message is being Clrd Frm Sat Q: %d\n"),mhaNumber);
  lprintf("CC:ClrdFrmSatQ:%d\n",mhaNumber);
  packetBufferS[0]=18; //Clear SC Originated message by MHA Number 
  packetBufferS[1]=mhaNumber;  //MHA Number to clear
  packetBufferS[2]=0;
  packetBufferS[3]=0;
  packetBufferS[4]=0;
  packetBufferS[5]=0;
  satSendCommCommand(packetBufferS,packetBufferB);  
  packetSeqNum++;
  satPacketReceptionist(packetBufferB);
}

boolean satHasATCPairBeenTXed() {
	satGetMessageTransmissionStatus(satMHANumReportA);
	satPacketReceptionist(packetBufferB);
  	satGetMessageTransmissionStatus(satMHANumReportB);
  	satPacketReceptionist(packetBufferB);
}



//Input:  MHA Message number to query sat modem for status of
//Return: 
byte satGetMessageTransmissionStatus(byte MHANumber) {
	//Get MEssage Status Comm Command type code: 4 
	packetBufferS[0]=4; //Request status of the given MHA # 
    packetBufferS[1]=MHANumber; 
    packetBufferS[2]=0;
    packetBufferS[3]=0;
    packetBufferS[4]=0;
    packetBufferS[5]=0;
	satSendCommCommand(packetBufferS, packetBufferA);
	packetSeqNum++;
}

