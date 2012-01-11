
// Process incoming 5-byte commands from sat uplink 
byte satProcessIncomingCommand (unsigned char* packetBufferLocal) {
  //Command goes from index byte packetPayloadStartIndex to byte packetPayloadStartIndex + 4 of packetBufferLocal
  // Byte packetPayloadStartIndex = Command counter
  // Byte packetPayloadStartIndex + 1 = I2C Module Address
  // Byte packetPayloadStartIndex + 2 = I2C command  
  // Byte packetPayloadStartIndex + 3 = Data Byte 0
  // Byte packetPayloadStartIndex + 4 = Data Byte 1

  lprintf("CC: ProcessINCMD");
  #ifdef satDataDebug
  printf_P(PSTR("SatInCMD: "));
  for (byte i=0;i<0x0D;i++) {
  printf_P(PSTR("%x "),packetBufferLocal[i]);
  }
  Serial.println();
  #endif
  if (true == satCheckForDupeCommand(packetBufferLocal[packetPayloadStartIndex])) {
  	return false;  // Return without processing if it's a duplicate
  }
  switch  (packetBufferLocal[packetPayloadStartIndex + 1]) {  //I2C Module Address byte
  case i2cCommCtrl:  //Comm Controller 
    //If it's for the comm Controller hand it off to be processed for local action
    satCommCommandProc(packetBufferLocal);
    return true;
  case i2cFlightComputer: //Flight Controller
    // Check for some special cases
    if (packetBufferLocal[packetPayloadStartIndex + 2] == 0x10) {  //Command: Cutdown at specific time
      packetBufferS[0]=0;  //Zero padded, which is why this special case is here.  FC wants 4 bytes.
      packetBufferS[1]=packetBufferLocal[packetPayloadStartIndex + 3];  //Data
      packetBufferS[2]=packetBufferLocal[packetPayloadStartIndex + 4];  //Data
      packetBufferS[3]=0;  //Zero padded, which is why this special case is here.  FC wants 4 bytes.
      I2CXmit(i2cFlightComputer, 0x10, packetBufferS, 4);  //Send to I2C
      break;
    }
  default:  //if it's not for the comm Controller, process for release onto the I2C bus
      packetBufferS[0]=packetBufferLocal[packetPayloadStartIndex + 3];  //Data
      packetBufferS[1]=packetBufferLocal[packetPayloadStartIndex + 4];  //Data
      I2CXmit(packetBufferLocal[packetPayloadStartIndex + 1], packetBufferLocal[packetPayloadStartIndex + 2], packetBufferS, 2);   //Send to I2C
      
  }
  return true;
}



byte satProcessIncomingMessage (unsigned char* packetBufferLocal,unsigned int packLen) {
  

     #ifdef satDataDebug
	 printf_P(PSTR("InboundMSG: "));
     lprintf("CC: ProcessINMSG");
	 satPrintPacket(packetBufferLocal);
	 #endif
	 //Check that message type is correct:
	 if (){
	 	printf_P(PSTR("Invalid inbound message type"));
	 	return false;
	 }
	 //OK, now we know we have the correct format message
	 //Message body should start at byte packetPayloadStartIndex
	 //Process this just like all other I2C commands
  #ifdef satDataDebug
  Serial.println("MSGOK");
  #endif
  if (true == satCheckForDupeCommand(packetBufferLocal[packetPayloadStartIndex])) {
  	return false;  // Return without processing if it's a duplicate
  	#ifdef satDataDebug
  	Serial.println("Dupe");
  	#endif
  }
  //Check to see if MSG is for us - the CommCtrlr
  if (packetBufferLocal[packetPayloadStartIndex+1] == i2cCommCtrl){
  		satCommCommandProc(packetBufferLocal);
  }
  //process for release onto the I2C bus
  //Copy message body to the short temp buffer, stopping before the two checksum bytes
      for (unsigned int i=packetPayloadStartIndex+3;i<packLen-2;i++) {
      	packetBufferA[i-packetPayloadStartIndex+3]=packetBufferLocal[i];  
      }
      I2CXmit(packetBufferLocal[packetPayloadStartIndex + 1], packetBufferLocal[ + 1], packetBufferA, packLen-satIncomingMessageHeaderLength);   //Send to I2C
      //?May want to check for I2C success status here!
//If all went well return true.
 return true;
	 
}


//Process commands destined for the comm controller from sat modem
//Pass in the actual sat command packet
void satCommCommandProc(unsigned char * packetBufferLocal) {
  //Command goes from index byte packetPayloadStartIndex to byte packetPayloadStartIndex + 4 of packetBufferLocal
  // Byte packetPayloadStartIndex = Command counter
  // Byte packetPayloadStartIndex + 1 = I2C Module Address
  // Byte packetPayloadStartIndex + 2 = I2C command  
  // Byte packetPayloadStartIndex + 3 = Data Byte 0
  // Byte packetPayloadStartIndex + 4 = Data Byte 1
  Serial.println("UplinkIsForMe");
  lprintf("CC: SatRXforCC");
  // Determine which COMM CONTROLLER COMMAND via Satellite is being given
  switch (packetBufferLocal[packetPayloadStartIndex + 2]){ //Check Command Byte 
  case 0x06:  //Reset heartbeat timer
    cdnCmdResetTimer();
    break;
  case 0x07:  //set cutdown timer to X minutes, and reset
    cdnCmdSet(packetBufferLocal[packetPayloadStartIndex + 3]);  // Send 1 byte to the cutdown command set function 
    satConfirmTimerCHG(packetBufferLocal[9]);
    break;
  case 0x99:  // CUTDOWN NOW
    cdnCmdCUTDOWNNOW();
    break;
  default: 
    ; 
  }
}


//Purpose:  Transmit a downlink message confirmation with recent TIME that the deadman timer has been reset.
void satConfirmTimerCHG(byte timerValue){
   Serial.println("SatConfirmTimerCHG");
   lprintf("CC: TXSatTimerChgConfirm");
    // Manual 6-byte report
   packetBufferS[0]= 0xFF;
   packetBufferS[1]= 0x01;  //Cutdown Timer Length Change command confirm indicator
   packetBufferS[2]= timerValue;
   packetBufferS[3]= lowByte(epochMinutes);
   packetBufferS[4]= highByte(epochMinutes);
   packetBufferS[5]= 0;
   //call function to send to sat modem here
}

