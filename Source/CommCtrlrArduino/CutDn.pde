//Stuff for serial flash strings
extern "C" int lprintf(char*, ...);
extern "C" int lprintf_P(const char*, ...);	


/********
 *
 * Note, many of these repeat with for loops, for the purpose of not having to wait
 *   for the cutdown controller to respond.
 *
 ********/

//Cutdown Initialization
void cdnInit(){
  boolean respondFlag = false;	
  char tempin;
  printf_P(PSTR("cdnInit. Data RX: "));
  for (byte i = 0; i<10; i++) {  // Look for response

    cdn.println("!R");  //Reset deadman timer
    delay(100);
    if(cdn.available()) {   // NewSoftSerial read will return -1 when nothing is received
      //A char has been returned
      tempin = cdn.read();
      Serial.print(tempin,BYTE);  
      if ('R' == tempin) {
        respondFlag = true;
      }
    }
  }
  if (respondFlag == false) {
    Serial.println("\nCUTDOWN DEAD?");
    //lprintf("CC: CUTDOWN DEAD!!!\n");
  }
  delay(500);
  cdnCmdSet(250);  // Immediately set the cutdown to maximum time to give time to work
}



//Cut down immediately
void cdnCmdCUTDOWNNOW(){
  for (byte i = 0; i<10; i++) {
    cdn.println("!CUTDOWNNOW");
    delay(100);
  }
  packetBufferS[0]=0;  //No real Data
  I2CXmit(i2cFlightComputer, 0x0F, packetBufferS, 0);//Send cutdown comm
  lprintf("CC:CUTDOWNNOW!");
  Serial.println("cdn!CUTDOWNNOW");
}

//Heartbeat reset
void cdnCmdResetTimer() {
  for (byte i = 0; i<10; i++) {
    cdn.println("!R");
    delay(100);
  }
  Serial.println("cdn!R");
}

// Set deadman timer time in minutes, which also resets the timer
void cdnCmdSet(unsigned char deadManTime) {
  printf_P(PSTR("\ncdn!TmrChg "));
  Serial.println(deadManTime,DEC);
  // DO NOT TRY TO PRINT TO I2C IN CUTDOWN!  IT WILL FREEZE COMMCONTROLLER ON BOOT!
  for (byte i = 0; i<10; i++) {
    cdn.print("!T");
    // Zero pad the value for ASCII numbers to cutdown controller
    if (deadManTime>99){
      cdn.print(deadManTime,DEC);
    } 
    else if (deadManTime>9) {
      cdn.print("0");
      cdn.print(deadManTime,DEC);
    } 
    else {
      cdn.print("00");
      cdn.print(deadManTime,DEC);    
    }
    cdn.println();
    delay(100);
    if(cdn.available()) {   // NewSoftSerial read will return -1 when nothing is received
      //A char has been returned
      Serial.print(cdn.read(),BYTE);
    }
  }

}






