/* 
 I2CeePROM - Program for reading the EEPROM via I2C Bus
 Created by White Star Ballon, December 27, 2010.
 Last Revised January 6, 2010
 */

byte I2CeePROMRead(byte device, unsigned int addr) 
{
  //Serial.println("iER");
  byte   eePROMbyte,sendStatus;
  for (byte i = 0; i < i2cRetryLimit; i++) {
  Wire.beginTransmission(device);                       
  Wire.send((int)(addr >> 8));                           // left-part of pointer address
  Wire.send((int)(addr & 0xFF));                         // and the right
  i2csentStatus = Wire.endTransmission();
  if (i2csentStatus != 0){
  	Serial.print("i2cErr: ");
  	Serial.print(i2csentStatus, DEC);
  	//Random delay routine:
	delayMicroseconds(random(1000));
	} else {
  Wire.requestFrom(device,(byte)1);                          

  eePROMbyte = Wire.receive();  
  return eePROMbyte; 
  }
  }
}


/*
byte I2CeePROMRead(int device, unsigned int addr) 
{
  byte eePROMbyte;                                      // returned value
  Serial.println("Wire.bT");
  Wire.beginTransmission(0x50);                       //  these three lines set the pointer position in the EEPROM
  Serial.println("Wire.s");
  Wire.send((int)(addr >> 8));                           // left-part of pointer address
  Serial.println("Wire.s2");
  Wire.send((int)(addr & 0xFF));                         // and the right
  Serial.println("Wire.eT");
  Wire.endTransmission();
  Serial.println("end Tx");
  Wire.requestFrom(0x50,1);                           // now get the byte of data...
  
  eePROMbyte = Wire.receive();  
  return eePROMbyte; // and return it as a result of the function readData
}

*/


