

// Give a buffer, with two extra bytes at the end
// Also give a LONG value representing the full length (including extra two)

void fletcher_encode( unsigned char* buffer, long counter ) {
  int i;
  unsigned char c0 = 0;
  unsigned char c1 = 0;
//Serial.print("Checksum Encode\n");
*( buffer + counter - 1 ) = 0;
  *( buffer + counter - 2 ) = 0;
  for (i=0; i<counter; i++) {
    c0 = c0 + *( buffer + i );
    //Serial.print(c0,HEX);
    //Serial.print(" ");
    c1 = c1 + c0;
    //Serial.println(c1,HEX);
  }
  *( buffer + counter - 2 ) = c0 - c1;
  *( buffer + counter - 1 ) = c1 - 2*c0;
}


long fletcher_decode( unsigned char* buffer, long counter) {
  long result = 0;
  int i;
  unsigned char c0 = 0;
  unsigned char c1 = 0;
//Serial.print("Checksum Decode\n");
  for ( i=0; i < counter; i++ ) {
    c0 = c0 + *( buffer + i );
    //Serial.print(c0,HEX);
    //Serial.print(" ");
    c1 = c1 + c0;
    //Serial.println(c1,HEX);
  }
  return ( (long) (c0 + c1) );  
}
