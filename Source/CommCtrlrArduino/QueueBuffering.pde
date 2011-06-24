// In here manage the onboard EEPROM queues of long messages


//Move the I2C EEPROM pointers back one long message
void satQueueBackupOne(){
	//Remove the long message
    satClearOutgoingMHA(satLongMsgInOBQMHA);
	EPLOCI2CEEPLongMSGAddrArrayDataStart = EPLOCI2CEEPLongMSGAddrArrayDataStartPREVIOUS; 
	satLongMsgInOBQMHA = 0xFF;  //Set to the value that indicates there's not one stored in the sat q;
	satLongMsgStored = true;
	satLongMsgInOBQ = false;
}


void satQueueI2CEEPROMLongMessage(unsigned char * addressArray) {
  //Check to see if queue is full already
  if ((EPLOCI2CEEPLongMSGAddrArrayDataEnd + 4) % EPLOCI2CEEPLongMSGAddrArrayLen != EPLOCI2CEEPLongMSGAddrArrayDataStart) {
    //Queue is not full
    printf_P(PSTR("EPQ\n"));
    //Add pair of addresses to end address
    #ifdef eepQDebug 
    printf_P(PSTR("%x%x %x%x\n"),addressArray[0],addressArray[1],addressArray[2],addressArray[3]);
    #endif
    EEPROM.write(EPLOCI2CEEPLongMSGAddrArrayStart+EPLOCI2CEEPLongMSGAddrArrayDataEnd+0,addressArray[0]);
    EEPROM.write(EPLOCI2CEEPLongMSGAddrArrayStart+EPLOCI2CEEPLongMSGAddrArrayDataEnd+1,addressArray[1]);
    EEPROM.write(EPLOCI2CEEPLongMSGAddrArrayStart+EPLOCI2CEEPLongMSGAddrArrayDataEnd+2,addressArray[2]);
    EEPROM.write(EPLOCI2CEEPLongMSGAddrArrayStart+EPLOCI2CEEPLongMSGAddrArrayDataEnd+3,addressArray[3]);
    //Increment data end pointer
    printf_P(PSTR("EPEndCurr: %d"),EPLOCI2CEEPLongMSGAddrArrayStart+EPLOCI2CEEPLongMSGAddrArrayDataEnd);
    EPLOCI2CEEPLongMSGAddrArrayDataEnd = (EPLOCI2CEEPLongMSGAddrArrayDataEnd + 4) % EPLOCI2CEEPLongMSGAddrArrayLen;
    printf_P(PSTR(" EPEndNext: %d"),EPLOCI2CEEPLongMSGAddrArrayStart+EPLOCI2CEEPLongMSGAddrArrayDataEnd);
    satLongMsgStored = true;
  } 
  else {
    //Queue is full!
    printf_P(PSTR("EEPROM Address Queue Full!\n"));
    lprintf("CC:LngMsgQFul\n"); 
    satLongMsgStored = true;
  }
}


//INPUT:  Scratch storage array SMALL
void  satSendNextQueuedLongMSG(unsigned char* packetBufferLocal) {
  //Test to see if the queue is empty
  if (EPLOCI2CEEPLongMSGAddrArrayDataEnd != EPLOCI2CEEPLongMSGAddrArrayDataStart) {
  
    //queue is NOT empty
    printf_P(PSTR("SendQdMsg"));
    //Read out values at current address
    packetBufferLocal[0] = EEPROM.read(EPLOCI2CEEPLongMSGAddrArrayStart+EPLOCI2CEEPLongMSGAddrArrayDataStart+0);
    packetBufferLocal[1] = EEPROM.read(EPLOCI2CEEPLongMSGAddrArrayStart+EPLOCI2CEEPLongMSGAddrArrayDataStart+1);
    packetBufferLocal[2] = EEPROM.read(EPLOCI2CEEPLongMSGAddrArrayStart+EPLOCI2CEEPLongMSGAddrArrayDataStart+2);
    packetBufferLocal[3] = EEPROM.read(EPLOCI2CEEPLongMSGAddrArrayStart+EPLOCI2CEEPLongMSGAddrArrayDataStart+3);
    //Push the last data array location forward past the data we just sent
    printf_P(PSTR("EPStrCurr: %d"),EPLOCI2CEEPLongMSGAddrArrayStart+EPLOCI2CEEPLongMSGAddrArrayDataStart);
    //Save the data end pointer for possible later rollback
    EPLOCI2CEEPLongMSGAddrArrayDataStartPREVIOUS = EPLOCI2CEEPLongMSGAddrArrayDataStart;
    EPLOCI2CEEPLongMSGAddrArrayDataStart = (EPLOCI2CEEPLongMSGAddrArrayDataStart + 4) % EPLOCI2CEEPLongMSGAddrArrayLen;
    //Normal increment number before sending serial data
    printf_P(PSTR(" EPStrNext: %d DATA:"),EPLOCI2CEEPLongMSGAddrArrayStart+EPLOCI2CEEPLongMSGAddrArrayDataStart);
    printf_P(PSTR("%x%x%x%x\n"),packetBufferLocal[0],packetBufferLocal[1],packetBufferLocal[2],packetBufferLocal[3]);
    packetSeqNum++;
    satLongMsgInOBQ = true;
    satLongMsgInOBQMHA = messageNumber;
    satSendDirectlyFromI2CEEPROM(packetBufferLocal);
    
  } 
  else {
    satLongMsgStored = false;
  }
}



