/*
void compareChecksums(){
  // Run same data through short report and long report, and compare outputs.
  unsigned int testchk;
  byte i = 8;

  printf_P(PSTR("Compare:")); 
  satPrintPacket(sampleLongMsg);

  printf_P(PSTR("\nfletcher_decode whole: "));
  testchk = fletcher_decode(sampleLongMsg,9);
  printf_P(PSTR("    %x\n"),testchk);

  sampleLongMsg[7] = 0; //reset checksum bytes
  sampleLongMsg[8] = 0;

  printf_P(PSTR("fletcher_encode whole: "));
  fletcher_encode(sampleLongMsg,(byte)9);
  printf_P(PSTR("%x%x\n"),sampleLongMsg[7],sampleLongMsg[8]);

  sampleLongMsg[7] = 0; //reset checksum bytes
  sampleLongMsg[8] = 0;


  printf_P(PSTR("Byte: %d Value:%x fe:\nOrig: \n"),i,sampleLongMsg[i]);
  satPrintPacket(sampleLongMsg);		
  //Fletcher Decode Method
  fletcher_encode(sampleLongMsg,i+1);
  Serial.println(sampleLongMsg[i-1],HEX);
  Serial.println(sampleLongMsg[i],HEX);

  //Check-Step Method
  checkSumA = 0;
  checkSumB = 0;
  sampleLongMsg[7] = 0; //reset checksum bytes
  sampleLongMsg[8] = 0;
  //loop to calculate step-checksum up to this point
  for (byte k=0; k<i+1;k++) {
    fletcherChk_step(sampleLongMsg[k]);
  }
  fletcherChk_stepOut();
  printf_P(PSTR(" cs:%x%x"),checkSumB,checkSumA);



}
*/

void satPrintPacket(unsigned char* packetBufferLocal){
  //printf_P(PSTR("satPacket: "));
  unsigned int packetLen=0;
  unsigned int a = packetBufferLocal[3];  // Put high byte in first
  packetLen = ((a << 8) + packetBufferLocal[2]);  // Shift high bits up, then Put low byte in 
  //printf_P(PSTR("packLen = %d "),packetLen);
  for(unsigned int i=0; i<packetLen;i++){
    printf_P(PSTR("%x "),packetBufferLocal[i]);
  }
}

/* 
//Print all parameters whatever the length, will generate errors as it just increments param number
void satPrintAllParameters(){
  //Serial.println(" All Value Parameters from Digi m10, will have errors. Hex, nonpadded \nParam,Value");
  for (byte i=0;i<0x3C;i++){
    if((i==0x1C)||(i==0x25)||(i==0x1A)||(i==0x24)||(i==0x26)||(i==0x2E)||(i==0x2F)){
      i++;
    }
    printf_P(PSTR("%x,")i);
    byte temp = satGetListParameter(packetBufferB,i); 

    if (temp == 255){
      //Error status detected in packet status code, do not use
      printf_P(PSTR(" Status Code Bad"));
    } 
    else {
      for (byte x=8;x<(temp+8);x++){  // 8 represents the start of the list in the full packet
        printf_P(PSTR("%x "),packetBufferB[x]);
    }
    Serial.println(); 
  }
}
*/
/************************
 *
 *    The following functions will not be used for flight code, only pre-flight testing
 *
 ************************/
#ifdef satAccessAllParameters 


// Very similar to Init routine, except sets many more parameters
void satSetupParams() {
  Serial.println("Setting Most Params");
  //Iterate through stored parameters and values to initialize modem
  int arraysize = sizeof(satSetupWriteByteParams);
  for (int i=0;i<arraysize;i++){
    printf_P(PSTR("SatParam %x"),pgm_read_byte_near(satSetupWriteByteParams + i); 
    do{
      satSetByteParameter(packetBufferB, pgm_read_byte_near(satInitializeParam + i),pgm_read_byte_near(satSetupWriteByteValues + i));
      printf_P(PSTR("."));
      satWaitReceiveOnePacket(packetBufferB);

    } 
    while (satCheckResponsePacket(packetBufferB) != true ) ;  //Check to see if we need to retry
    printf_P(PSTR(" OK\n"));  
  }
}


// Function to get all the 1 byte parameters from satellite modem and print them 
void satPrintAllByteParameters(){
  byte currParam;
  byte currValue; 
  printf_P(PSTR(" All Byte Value Parameters from Digi m10\nParam,Value"));
  for (int z=0;z<0x34;z++){
    currParam = pgm_read_byte_near(satByteParams + z);
    currValue = satGetByteParameter(packetBufferB,currParam);

    printf_P(PSTR("%x,%x\n"),currParam,currValue);
  }
}



#endif

/*

 //Sample Routines storage, don't call this function
 
 void sampleRoutines(){
 byte temp=0; 
 
 //Example of Getting a multibyte parameter:
 
 temp = satGetListParameter(packetBufferB,0x10); 
 printf_P(PSTR("Gateways, Minimum Priority for each: "));
 for (byte i=8;i<(temp+8);i++){  // 8 represents the start of the list in the full packet
 Serial.print(packetBufferB[i],HEX);
 printf_P(PSTR(" "));
 }
 printf_P(PSTR(" , "));
 
 temp = satGetListParameter(packetBufferB,0x11); 
 for (byte i=8;i<(temp+8);i++){  // 8 represents the start of the list in the full packet
 Serial.print(packetBufferB[i],HEX);
 printf_P(PSTR(" "));
 }
 //Serial.println();
 
 //Example of getting a byte parameter
 temp = satGetByteParameter(packetBufferB,0x16);  
 printf_P(PSTR("Msgs OB Queued: %d\n"),temp);
 
 
 }
 
 */



/*

//Check for incoming messages via sat modem
 void satLoopCheckIncomingMessages(){
 byte satDataAvailable;
 byte packLength=0;
 satDataAvailable = digitalRead(sat_da);
 if(satDataAvailable == 1) {
 Serial.println("DA, RTS high, sat holding data");
 for (int i=0;i<1000;i++){  // Loop through checking the incoming port for data, just to make sure it's obeying RTS
 if (sat.available()){
 satWaitReceiveOnePacket(packetBufferA);  //If there's serial data coming in, get it
 }
 printf_P(PSTR("."));
 delay(100);
 }
 Serial.println("\nClearing RTS.");
 digitalWrite(sat_rts,LOW);  // This should make the satmodem send data
 while (satDataAvailable = true) {
 if (sat.available()){
 packLength = satWaitReceiveOnePacket(packetBufferA);
 if (packLength>1){
 satPrintPacket(packetBufferA);
 }
 }
 satDataAvailable = digitalRead(sat_da);
 }
 }
 }
 
*/

