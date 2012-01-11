





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







//verify checksum in packet
// Returns:  TRUE if checksums match.
byte satCheckIncomingPacketChecksum(unsigned char* packetBufferLocal, unsigned int packLen) {
  byte supposedchecksumLowByte, supposedchecksumHighByte;
  //Store existing,supposed checksum
  supposedchecksumLowByte = packetBufferLocal[packLen-2];
  supposedchecksumHighByte = packetBufferLocal[packLen-1];

  //Recalculate checksum of message here
  

  //Compare to received checksum
  if () {  //Matches
  	 	return true;   
  }
  return false;
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



//Save ATCpacket  over the top of the old one.
void satSaveATCPkt() {
#ifdef actionLabels
  printf_P(PSTR("OverwriteATC\n"));
#endif
  lprintf("CC:SaveATCPkt\n");
  for(byte i=0;i<(EPLOCAtcReportPairArray);i++){
    EEPROM.write(i+EPLOCAtcReportPairArrayStart,i2cdata[i2cSel][i]);  //Copy the ATC report data from the i2c command.
  }
  ATCRptReady = true;  //Indicate globally that a report is stored and ready.
}



