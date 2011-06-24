





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
// Returns: True if response is status packet and is OK.  False if not OK.
byte satCheckResponsePacket(unsigned char* packetBufferLocal){
  //Packet response type
  rxerr = 0;
  switch(packetBufferLocal[packetTypeIndex]) {

  case 0x01:  //Link Level ACK
    if (packetBufferLocal[packetLLstatusCodeIndex] != 0){
      rxerr = packetBufferLocal[packetLLstatusCodeIndex];
      //Do not increment packet number
      return false;  // Just return false for all failing problems that need retry
    }
    else {   // Return Good, and incremenet packet sequence number for the next serial packet to use
      rxerr = 0;
      //Increment packetSeqNum 
      packetSeqNum++;
      return true;
      break;
    }
  case 0x04:  // System Announcement
    Serial.println("\nRX System Announcement Packet, no parsing");
    satPrintPacket(packetBufferLocal);  
  case 0x05:  // Status Packet
    Serial.println("\nRX Status Packet, no parsing");
    satPrintPacket(packetBufferLocal);
    break;
  case 0x0B:  // System Response
    Serial.println("\nRX System Response, no parsing");
    satPrintPacket(packetBufferLocal);
    break;
  case 0x0C: // Incoming SC Terminated Message
    break;
  case 0x14: //Parameter Response, comes back after setting or getting any parameter val
    if (packetBufferLocal[packetRetStatusCodeIndex] != 0){
      rxerr = packetBufferLocal[packetRetStatusCodeIndex];
      return false;  // Just return false for all failing problems that need retry
    }
    else {  // Return Good, and incremenet packet sequence number for the next serial packet to use
      rxerr = 0;
      //Increment packetSeqNum 
      packetSeqNum++;
      satSendLLACK();
      return true;
    }
    break;
  default:  // Default for unknown packets
#ifdef satDataDebug
    Serial.println("Failed to match RX response packet type.");
    satPrintPacket(packetBufferLocal);
#endif
    return false;
    //By not incrementing the packetSeqNum++ here, the previous command from the DTE will continue to retransmit 


  }
}



//Input: pass integer 0 = no parameter setup, 1 = minimal parameter setup, 2 = most parameter setup
// Scratch space: PacketBufferB
void satInitRoutine(byte initParams) {
  unsigned int q;
  byte c=0;
  Serial.println("SatInitRoutine");
  digitalWrite(PWR_EN, HIGH);    // Turn sat power on
  //Read next message number from EEPROM
  messageRefNum = EEPROM.read(EPLOCmessageNum);
  Serial.print("Current Message Number: "); 
  Serial.println(messageRefNum,DEC);
  for (q=0;q<100;q++){    //Delay talking to modem 
    // Just delay for the DA pulse of the sat modem, too hard to detect.
    if (q%5 == 0){ //Modulo divide the loop counter to print a "." every 5 loops for UI
      Serial.print("."); 
    }
    delay(100);
  }
  sat.flush();
  Serial.println("\nSatReadyForSerialCommand");

  if(initParams == 1) {
    Serial.println("Setting Parameters");
    //Iterate through stored parameters and values to initialize modem
    int arraysize = sizeof(satInitializeParam);
    for (int i=0;i<arraysize;i++){
      c=0;
      Serial.print("SatParam ");
      Serial.print(pgm_read_byte_near(satInitializeParam + i),HEX); 
      do{

        satSetByteParameter(packetBufferB, pgm_read_byte_near(satInitializeParam + i),pgm_read_byte_near(satInitializeValues + i));
        Serial.println(". ");
        satWaitReceiveOnePacket(packetBufferB);
        c++;
        Serial.println(c,DEC);
      } 
      while ((satCheckResponsePacket(packetBufferB) != true )||(c>serialRetryLimit)) ;  //Check to see if we need to retry
      Serial.println(" OK");  
    }
  }  
  else if (initParams == 2) {
#ifdef satAccessAllParameters
    satSetupParams();
#else
    Serial.println("Can't setup all params, satAccessAllParameters disabled in source code");
#endif

  }
  Serial.println("SatInitRoutine Done");
}









