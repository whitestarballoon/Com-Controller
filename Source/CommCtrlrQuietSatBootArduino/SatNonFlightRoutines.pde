



void satPrintPacket(unsigned char* packetBufferLocal){
  Serial.print("PrintPacket: ");
  unsigned int packetLen=0;
  unsigned int a = packetBufferLocal[3];  // Put high byte in first
  packetLen = ((a << 8) + packetBufferLocal[2]);  // Shift high bits up, then Put low byte in 

  for(unsigned int i=0; i<packetLen;i++){
    Serial.print(packetBufferLocal[i],HEX);
    Serial.print(" ");
  }

}


//Print all parameters whatever the length, will generate errors as it just increments param number
void satPrintAllParameters(){
  Serial.println(" All Value Parameters from Digi m10, will have errors. Hex, nonpadded \nParam,Value");
  for (byte i=0;i<0x3C;i++){
    if((i==0x1C)||(i==0x25)||(i==0x1A)||(i==0x24)||(i==0x26)||(i==0x2E)||(i==0x2F)){
      i++;
    }
    Serial.print(i,HEX);
    Serial.print(",");
    byte temp = satGetListParameter(packetBufferB,i); 

    if (temp == 255){
      //Error status detected in packet status code, do not use
      Serial.print(" Status Code Bad - skipping");
    } 
    else {
      for (byte x=8;x<(temp+8);x++){  // 8 represents the start of the list in the full packet
        Serial.print(packetBufferB[x],HEX);
        Serial.print(" ");
      }
    }
    Serial.println(); 
  }
}

/************************
 *
 *    The following functions will not be used for flight code, only pre-flight testing
 *
 ************************/
#ifdef satAccessAllParameters 


// Very similar to Init routine, except sets many more parameters
void satSetupParams() {
  Serial.println("Setting Most Parameters");
  //Iterate through stored parameters and values to initialize modem
  int arraysize = sizeof(satSetupWriteByteParams);
  for (int i=0;i<arraysize;i++){
    Serial.print("SatParam ");
    Serial.print(pgm_read_byte_near(satSetupWriteByteParams + i),HEX); 
    do{
      satSetByteParameter(packetBufferB, pgm_read_byte_near(satInitializeParam + i),pgm_read_byte_near(satSetupWriteByteValues + i));
      Serial.print(".");
      satWaitReceiveOnePacket(packetBufferB);

    } 
    while (satCheckResponsePacket(packetBufferB) != true ) ;  //Check to see if we need to retry
    Serial.println(" OK");  
  }
}



// Function to get all the 1 byte parameters from satellite modem and print them 
void satPrintAllByteParameters(){
  byte currParam;
  byte currValue; 
  Serial.println(" All Byte Value Parameters from Digi m10\nParam,Value");
  for (int z=0;z<0x34;z++){
    currParam = pgm_read_byte_near(satByteParams + z);
    currValue = satGetByteParameter(packetBufferB,currParam);

    Serial.print(currParam,HEX);
    Serial.print(",");
    Serial.print(currValue,HEX);
    Serial.println();
  }
}



#endif



//Sample Routines storage, don't call this function

void sampleRoutines(){
  byte temp=0; 

  //Example of Getting a multibyte parameter:

  temp = satGetListParameter(packetBufferB,0x10); 
  Serial.print("Gateways, Minimum Priority for each: ");
  for (byte i=8;i<(temp+8);i++){  // 8 represents the start of the list in the full packet
    Serial.print(packetBufferB[i],HEX);
    Serial.print(" ");
  }
  Serial.print(" , ");

  temp = satGetListParameter(packetBufferB,0x11); 
  for (byte i=8;i<(temp+8);i++){  // 8 represents the start of the list in the full packet
    Serial.print(packetBufferB[i],HEX);
    Serial.print(" ");
  }
  //Serial.println();

  //Example of getting a byte parameter
  temp = satGetByteParameter(packetBufferB,0x16);  
  Serial.print("Msgs OB Queued: ");
  Serial.println(temp,DEC);


}

