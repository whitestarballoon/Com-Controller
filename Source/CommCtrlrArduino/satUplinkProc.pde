
// Process incoming 5-byte commands from sat uplink 
byte satProcessIncomingCommand (unsigned char* packetBufferLocal) {
  //Command goes from index byte 6 to byte 10 of packetBufferLocal
  // Byte 6 = Command counter
  // Byte 7 = I2C Module Address
  // Byte 8 = I2C command  
  // Byte 9 = Data Byte 0
  // Byte 10 = Data Byte 1

  lprintf("CC: ProcessINCMD");
  #ifdef satDataDebug
  Serial.println("SatRXCMD");
  Serial.print("SatInCMD: ");
  for (byte i=0;i<0x0D;i++) {
  Serial.print(packetBufferLocal[i],HEX);
  Serial.print(" ");
  }
  Serial.println();
  #endif
  if (true == satCheckForDupeCommand(packetBufferLocal[6])) {
  	return false;  // Return without processing if it's a duplicate
  }
  switch  (packetBufferLocal[7]) {  //I2C Module Address byte
  case i2cCommCtrl:  //Comm Controller 
    //If it's for the comm Controller hand it off to be processed for local action
    satCommCommandProc(packetBufferLocal);
    return true;
  case i2cFlightComputer: //Flight Controller
    // Check for some special cases
    if (packetBufferLocal[8] == 0x10) {  //Command: Cutdown at specific time
      packetBufferS[0]=0;  //Zero padded, which is why this special case is here.  FC wants 4 bytes.
      packetBufferS[1]=packetBufferLocal[9];  //Data
      packetBufferS[2]=packetBufferLocal[10];  //Data
      packetBufferS[3]=0;  //Zero padded, which is why this special case is here.  FC wants 4 bytes.
      I2CXmit(i2cFlightComputer, 0x10, packetBufferS, 4);  //Send to I2C
      break;
    }
  default:  //if it's not for the comm Controller, process for release onto the I2C bus
      packetBufferS[0]=packetBufferLocal[9];  //Data
      packetBufferS[1]=packetBufferLocal[10];  //Data
      I2CXmit(packetBufferLocal[7], packetBufferLocal[8], packetBufferS, 2);   //Send to I2C
      
  }
  return true;
}



byte satProcessIncomingMessage (unsigned char* packetBufferLocal,unsigned int packLen) {
/*
Example message, body is 2 ascii letters 'the'
06 0C 0F 00 02 01 00 0E 01 01 74 68 65 00 8B 
0  1  2  3  4  5  6  7  8  9  10 11 12 13 14
*/   

     #ifdef satDataDebug
	 Serial.print("InboundMSG: ");
     lprintf("CC: ProcessINMSG");
	 satPrintPacket(packetBufferLocal);
	 #endif
	 //Check that message type is binary and that there is no subject:
	 if ((0x0E != packetBufferLocal[7]) && (0x00 != packetBufferLocal[6]) && (1 == packetBufferLocal[8])) {
	 	// If either value isn't what we want, then this is not a message we want to read.
	 	Serial.print("Invalid type or subj");
	 	return false;
	 }
	 //CHeck to see if incoming message I2C data section is too long
	// if ((packLen> > 70) && (packLen < 15)){
	  //  Serial.print("UplinkMSG toobig");
	  //  return false;
	// }
	 //OK, now we know we have the correct format message
	 //Message body should start at byte 10
	 //Process this just like all other I2C commands
	   //Command goes from index byte 6 to byte 10 of packetBufferLocal
  // Byte 10 = Command counter
  // Byte 11 = I2C Module Address
  // Byte 12 = I2C command  
  // Byte 13 = Data Byte 0 
  // Byte 14 = Data Byte 1 
  // Byte packlen-3 = Data Byte last
  #ifdef satDataDebug
  Serial.println("MSGOK");
  #endif
  if (true == satCheckForDupeCommand(packetBufferLocal[10])) {
  	return false;  // Return without processing if it's a duplicate
  	#ifdef satDataDebug
  	Serial.println("Dupe");
  	#endif
  }
  //Check to see if MSG is for CommCtrlr
  if (packetBufferLocal[11] == i2cCommCtrl){
  		for (int z=10; z<packLen; z++) {
  			//Shift msg down to match index positioning of an uplinked COMMAND
  			//A kludge to use the same func for both
  			packetBufferLocal[z-4] = packetBufferLocal[z];
  		}
  		satCommCommandProc(packetBufferLocal);
  }
  //process for release onto the I2C bus
  //Copy message body to the short temp buffer
      for (unsigned int i=13;i<packLen-2;i++) {
      	packetBufferA[i-13]=packetBufferLocal[i];  
      }
      I2CXmit(packetBufferLocal[11], packetBufferLocal[12], packetBufferA, packLen-15);   //Send to I2C
  
//If all went well return true.
 return true;
	 
}


//Process commands destined for the comm controller from sat modem
//Pass in the actual sat command packet
void satCommCommandProc(unsigned char * packetBufferLocal) {
  //Command goes from index byte 6 to byte 10 of packetBufferLocal
  // Byte 6 = Command counter
  // Byte 7 = I2C Module Address
  // Byte 8 = I2C command
  // Byte 9 = Data Byte 0
  // Byte 10 = Data Byte 1
  Serial.println("UplinkIsForMe");
  lprintf("CC: SatRXforCC");
  // Determine which COMM CONTROLLER COMMAND via Satellite is being given
  switch (packetBufferLocal[8]){ //Check Command Byte 
  case 0x06:  //Reset heartbeat timer
    cdnCmdResetTimer();
    break;
  case 0x07:  //set cutdown timer to X minutes, and reset
    cdnCmdSet(packetBufferLocal[9]);  // Send 1 byte to the cutdown command set function 
    satConfirmTimerCHG(packetBufferLocal[9]);
    break;
  case i2cCmdSetGwy:   //change ORBCOMM default GWY
    if (( 1 == packetBufferLocal[9]) || ( 120 == packetBufferLocal[9])) {
       gwy_id = packetBufferLocal[9];  //Set gateway
       satSetByteParameter(packetBufferB, (byte)0x01, (byte)gwy_id); 
    break;
    }
  case i2cCmdSendOrbPositRpt:
    satSendSCOrigPositionRpt(latestPosition,packetBufferS);
    break;
  case 0x99:  // CUTDOWN NOW
    cdnCmdCUTDOWNNOW();
    break;
  default: 
    ; 
  }
}

/*
//Process incoming GlobalGram NOT DONE!!!!  EXAMINE WITH FINE TOOTHED COMB BEFORE USE
byte satProcessIncomingGG(unsigned char* packetBufferLocal,unsigned int packLen) {
	 //Data body should start at byte 8
	 //Process this just like all other I2C commands
  // Byte 8 = Command counter
  // Byte 9 = I2C Module Address
  // Byte 10 = I2C command  
  // Byte 11 = Data Byte 0 
  // Byte 12 = Data Byte 1 
  // Byte packlen-3 = Data Byte last
  if (true == satCheckForDupeCommand(packetBufferLocal[8])) {
  	return false;  // Return without processing if it's a duplicate
  	#ifdef satDataDebug
  	Serial.println("Dupe");
  	#endif
  }
  //Check to see if GG is for CommCtrlr
  if (packetBufferLocal[9] == i2cCommCtrl){
  		for (int z=8; z<packLen; z++) {
  			//Shift msg down to match index positioning of an uplinked COMMAND
  			//A kludge to use the same func for both
  			packetBufferLocal[z-2] = packetBufferLocal[z];
  		}
  		satCommCommandProc(packetBufferLocal);
  }
  //process for release onto the I2C bus
  //Copy message body to the short temp buffer
      for (unsigned int i=13;i<packLen-2;i++) {
      	packetBufferA[i-13]=packetBufferLocal[i];  
      }
     I2CXmit(packetBufferLocal[9], packetBufferLocal[10], packetBufferA, packLen-15);   //Send to I2C
  
//If all went well return true.
 return true;

}
*/
//Purpose:  Transmit a confirmation that the deadman timer has been reset.
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
   satSendDefaultReport(packetBufferS,packetBufferB);
}

