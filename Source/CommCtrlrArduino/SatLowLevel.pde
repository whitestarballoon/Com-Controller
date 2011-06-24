





// Pass an array pointer to whole formed packet, with checksum.  Include length of packet including checksum
void satDataSend(unsigned char* packarray, unsigned char len){
  for (int i = 0;i < len; i++)
  {
    sat.print(packarray[i], BYTE); 
  }
#ifdef satDataDebug
  //Print out data that was sent to satellite modem
  Serial.print("Sent to Sat: ");
  for (int i = 0; i<len; i++) {

    Serial.print(packarray[i], HEX);
    Serial.print(" ");
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
      Serial.print("\nSat Ack Error: ");
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
    Serial.println("\nRX Sys Ann Pkt, no pars.");
    satPrintPacket(packetBufferLocal);  
  case 0x05:  // Status Packet
    satSendLLACK();
    Serial.println("\nRX Stat Pkt, no pars");
    satPrintPacket(packetBufferLocal);
    break;
  case 0x0B:  // System Response
    Serial.println("\nRX Sys Resp, no pars");
    satPrintPacket(packetBufferLocal);
    satSendLLACK();
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
#ifdef satDataDebug
    Serial.println("SatPkt Type Unk:");
	packetBufferLocal[3] = 0; 
	packetBufferLocal[2] = 20; 
    satPrintPacket(packetBufferLocal);
#endif
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
#ifdef satDataDebug
  Serial.print("satPkts?");
#endif
  for (i=0;i<limit;i++){
    // Copy Serial Data From NewSoftSerial RX Buffer
    if (sat.available()){
	  dataReceivedFlag = true;
#ifdef satDataDebug
      Serial.print("Got:");
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
        
#ifdef satDataDebug
        //Print out the data received from sat modem
        //Serial.print(packetBufferLocal[r], HEX);   
        //Serial.print(" ");
#endif
        //Advance to the next byte in the rx buffer
        r++;
        //Serial.print("r:"); Serial.println(r,DEC);
        //Serial.print(" packLen: "); Serial.println(packLen,HEX);
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
	     Serial.println("BAD PKTADDR");
	     return 0;
	  }
	  
	  satPrintPacket(packetBufferLocal);
		//If the checksum for this packet is wrong, don't try to do anything more with it.
	  #ifndef skipSatInCHKs
	  if (false == satCheckIncomingPacketChecksum(packetBufferLocal, packLen)) {
		Serial.println("\nCHKSUM BAD");
		return 0;
	  }
	  Serial.println("\nCHKSUM OK");
	  #else
	  Serial.println("CHKSUM SKIP");
	  #endif
	  //If this is a duplicate packet, just skip doing anything further with it, and report it as 0 length.
	  if (true == satCheckForDupePacket(packetBufferLocal)) {
		return 0;
		Serial.println("\nDUPE PKT");
	  } 
	  else {
		return packLen;  //Returns the length of the captured, non-duplicate packet
	  }
  
  }
  else {
  Serial.println("None");
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
//Serial.print("Checksum Encode\n");
*( buffer + counter - 1 ) = 0;  //Set last two bytes to zero
  *( buffer + counter - 2 ) = 0;  //Set last two bytes to zero
  for (i=0; i<counter; i++) {
    c0 = c0 + *( buffer + i );

    c1 = c1 + c0;
    //Serial.print(c1,HEX);
    //Serial.print(c0,HEX);
    //Serial.print(" ");
  }
  *( buffer + counter - 2 ) = c0 - c1;
  *( buffer + counter - 1 ) = c1 - 2*c0;
}


long fletcher_decode( unsigned char* buffer, long counter) {
  long result = 0;
  int i;
  unsigned char c0 = 0;
  unsigned char c1 = 0;
//Serial.print("ChckD:");
  for ( i=0; i < counter; i++ ) {
    c0 = c0 + *( buffer + i );
    c1 = c1 + c0;
   // Serial.print(c1,HEX);
   // Serial.print(c0,HEX);
   // Serial.print(" ");
  }
  return ( (long) (c0 + c1) );  
}


// Increments fletcher checksum, call between every byte.  at end of message, call fletcherChk_stepOut
// Inputs:  checkSumA, CheckSumB, current byte
// Outputs: checkSumA, CheckSumB
void fletcherChk_step(byte currentByte) {
    //Serial.print("(");
	checkSumA = checkSumA + currentByte;
	checkSumB = checkSumB + checkSumA;
	//Serial.print(currentByte,HEX);
	//Serial.print(")chkstpBA:");
	//Serial.print(checkSumB,HEX);
	//Serial.print(checkSumA,HEX);
	//Serial.print(" ");
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
	/*
    Serial.print("postCSO checkSumA: ");
    Serial.println(checkSumA,HEX);
    Serial.print("postCSO checkSumB: ");
    Serial.println(checkSumB,HEX);
    */
}


	// Compare SERIAL packet counter in received packet to the satLatestPacNum global variable.  If it's the same,
	// that means it's a duplicate!  if it's not the same, copy it to the satLatestPacNum as the current,
	// and return false.   Calling functions deal with the fact that it's a dupe.
byte satCheckForDupePacket(unsigned char* packetBufferLocal) {	
	//packet number is always byte 4
	if (satLatestPacNum == packetBufferLocal[4]) {
	   //It's a duplicate!
	   return true;  //True
	} else {
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
