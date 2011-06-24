

// Get single byte parameter 
byte satGetByteParameter(unsigned char* packetBufferLocal, unsigned char param){
  for (byte i=0; i<serialRetryLimit;i++){
    // You must call the satCheckResponsePacket after this to increment the packet number
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
    satWaitReceiveOnePacket(packetBufferLocal);
    if (satCheckResponsePacket(packetBufferLocal) == true ){  //Check to see if we have "no error" status response. If not, send again.
      return packetBufferLocal[8];  //Return parameter value from packet, exiting this function
    }
  }
  //Increment packetSeqNum if it exceeds serialRetryLimit
  packetSeqNum++;

}


//Pass: pointer to an array that you want to put the returned data packet in.
//      Parameter byte of which parameter to return
//Output: Returns length of parameter list in bytes.  Returns 255 if there was an error in receiving.
//        List starts with first byte in output array position index 8.  

byte satGetListParameter(unsigned char* packetBufferLocal, unsigned char param){
  for (byte i=0; i<serialRetryLimit;i++){
    // You must call the satCheckResponsePacket after this to increment the packet number
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
    satWaitReceiveOnePacket(packetBufferLocal);   // This will overwrite the sent command in the buffer with data in from the sat modem
    if (satCheckResponsePacket(packetBufferLocal) == true ){  //Check to see if we have "no error" status response. If not, send again
      //Return list length, exiting this function
      return (packetBufferLocal[7]);
    }
  }
  //Increment packetSeqNum if it exceeds serialRetryLimit
  packetSeqNum++;
  return 255;  // Unable to receive, or to parse, return error
}




//Pass the buffer to be written to, and the true/false of whether you want to print the result to debug serial
// Returns: Length in bytes of packet received
byte satWaitReceiveOnePacket(unsigned char* packetBufferLocal) {
  unsigned int packLen = 0;   //Start high, and read the actual length that comes in
  unsigned int i=0,r=0,limit = 5000;
#ifdef satDataDebug
  Serial.print("Waiting for sat response: ");
#endif
  for (i=0;i<limit;i++){
    // Copy Serial Data From NewSoftSerial RX Buffer
    if (sat.available()){

#ifdef satDataDebug
      Serial.print("Serial In: ");
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

        //Print out the data received from sat modem
#ifdef satDataDebug
        Serial.print(packetBufferLocal[r], HEX);   
        Serial.print(" ");
#endif
        //Advance to the next byte in the rx buffer
        r++;
        //Serial.print("r:"); Serial.println(r,DEC);
        //Serial.print(" packLen: "); Serial.println(packLen,HEX);
        if((r >= packLen)&&(r > ( packetLengthIndex + 1))) {
          i=limit; //terminate for loop too with this
          //Serial.println("break for r packlen");
          break;
        }
      }
#ifdef satDataDebug
      Serial.println();
#endif
    }
    delay(1);
  }

  return packLen;  //Returns the length of the captured packet
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
    satWaitReceiveOnePacket(packetBufferLocal);
    if (satCheckResponsePacket(packetBufferLocal) == true ){  //Check to see if we have "no error" status response. If not, send again.
      break;
    }

  }
  messageRefNum++;
  EEPROM.write(EPLOCmessageNum,messageRefNum);
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
    satWaitReceiveOnePacket(packetBufferLocal);
    if (satCheckResponsePacket(packetBufferLocal) == true ){  //Check to see if we have "no error" status response. If not, send again.
      break;
    }

  }
  messageRefNum++;
  EEPROM.write(EPLOCmessageNum,messageRefNum);

}

//Pass: inputdatabuffer of: type code, Value bytes 0,1,2,3, gwy_id
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
    satWaitReceiveOnePacket(packetBufferLocal);
    if (satCheckResponsePacket(packetBufferLocal) == true ){  //Check to see if we have "no error" status response. If not, send again.
      break;
    }

  }
  messageRefNum++;
  EEPROM.write(EPLOCmessageNum,messageRefNum);
}




