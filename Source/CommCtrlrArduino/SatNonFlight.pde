
//This function will just debug print the contents of the packet buffer anytime you call it.  
//Useful for debugging.
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
