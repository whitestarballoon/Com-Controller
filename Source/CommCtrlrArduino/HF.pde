/********
 *
 *  HF Transmitter Routines
 *
 *********/

/****************
 *
 *  Send Serial string to HF TX
 *
 *****************/



//Input: time to wait for HF in 100ths of second
boolean hfScanForReadySign(unsigned int timeToListen){
  hf.flush();  //Clear stale hf serial buffer
  for (byte i = 0; i<timeToListen; i++) {  //This loop keep checking for new serial data from HF 
  if(hf.available()) {
      return true;  // Exit and return true if there's bytes waiting from the HF radio
    }
    delay (10); 
  }
  return false;   //Exit here if no bytes hear from HF radio
}



// Do check to see that hf.available is true before calling this please.
//Wait for HF to be ready to receive command
byte hfSendSerialCommand(unsigned char * packetBufferLocal, byte packLen) {
   char tempChar = 0;
  if (hfScanForReadySign(1000)== false){
   return false;  //Exit this routine, HF transmitter never got ready to receive command. 
  }
  for (byte i = 0; i<100;i++) {
    while (hf.available()) {
      tempChar=hf.read();
      if (tempChar=='!') {    //HF TX sends a '!' when it's ready to listen
        hf.print("$$");     // Add the HF Command Preamble
        for (byte k =0; k<packLen; k++) {    //Loop for the number of bytes in the input array
          hf.print(packetBufferLocal[k]);    //Copy in data from input array
        }
        hf.println("*");                  //add on the trailing star.
      }

    }
    delay(10);
    }
  }
 
 
 //Update the internal variable for HF single telemetry channel
 void updateThreeNinersTelem(){
 }
