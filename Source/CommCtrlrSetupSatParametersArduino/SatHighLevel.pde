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



