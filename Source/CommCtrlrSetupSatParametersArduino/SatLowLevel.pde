





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
    satSendLLACK();
    printf_P(PSTR("\nSC Terminated Message received\n"));
    break;
  case 0x0D: // Incoming SC Terminated 5-byte Command
    //Reset heartbeat immediately for all incoming commands
    satSendLLACK();
    printf_P(PSTR("\nSC Terminated Command received\n"));
    break;
  case 0x0E:  //Incoming SC Terminated GLOBALGRAM  - highly unlikely in real life
    //satProcessIncomingGG(packetBufferLocal); 
    satSendLLACK();
    printf_P(PSTR("\nSC Terminated Command received\n"));
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





void msgCounterHouseKeeping(){
  if ( 0xFF == messageRefNum ) {
    messageRefNum = 0;  // Skip over 0xFF, seems to cause problems with sat modem.
  }	
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



