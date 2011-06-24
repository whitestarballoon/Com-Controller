/* 
 I2CXmit - Program for communicating via I2C Bus
 Created by White Star Balloon, December 11, 2010.
 Last Revised December 13, 2010
 */

int I2CXmit(byte device, byte command, byte* data, int length)
{
#ifdef i2cDebug
  printf_P(PSTR("~I2CXmit~\nto Addr: %x\ncommand: %x"),device,command);

  for (byte i=0; i<length;i++) {
    printf_P(PSTR("index:%d data:%x  "),i,data[i]);
  }
  Serial.println();


#endif

  // Transmit over I2C
  Wire.beginTransmission(device);                      // Begin Transmission to (address)
  Wire.send(command);                                   // Put (command) on queue
  //  Wire.send(data);
  for (int i = 0;i < length; i++)                       // Loop to put all (data) on queue
  {
    Wire.send(data[i]);
  }
  int sentStatus = Wire.endTransmission();              // Send queue, will return status
  return sentStatus;                                   // Return Status of Transmission
}

/*
void I2Csend(byte length) {
 do                                                       // Sends data out on the I2C Bus
 { 
 i2csentStatus = I2CXmit(i2caddress, i2ccommand, i2cdata, length);     // Gets return value from endTransmission
 delay(random(1,200));                                   // Random delay in case of delivary failure
 } 
 while (i2csentStatus != 0);                              // Continue until data arrives
 }
 */


void i2cInitRoutine(){
  Wire.begin(i2cCommCtrl);                                   // Join I2C Bus as slave
  Wire.onReceive(i2cReceiveData);                            // Set On Receive Handler
  printf_P(PSTR("I2C Init Done.\n"));
}



//Wire library interrupt will pass number of bytes to receive
void i2cReceiveData(int wireDataLen) {
  int i=0,dataArraySize;
  if (1 == i2cCmdsStored) {  
    //There's already an i2c command stored! O noes!
    i2cSel == 1;   //Store all the stuff in the #1 slot for i2c
  } 
  else 
  {
    i2cSel == 0;  //Store all the stuff in the #0 slot for i2c
  }
#ifdef i2cDebug
  printf_P(PSTR("i2c 4 me!\n"));
#endif

  // Check to see if there's any I2C commands already 

  if ((wireDataLen -1) > i2cDataLenConst) {
    printf_P(PSTR("\ni2cRx: data too big!\n"));
    while(Wire.available() > 0)           // Loop to receive the Data from the I2C Bus
    {
      Wire.receive();   // Finish receiving data and do not store.
    }
  }
  else
  {
    i2cdataLen[i2cSel] = wireDataLen - 1;
    i2cRXCommand[i2cSel] = Wire.receive();          // Receive the Command as the first byte from the I2C Bus
    while(Wire.available() > 0)           // Loop to receive the Data from the I2C Bus
    {
      i2cdata[i2cSel][i] = Wire.receive();
      i++;
    }
  }
#ifdef i2cDebug
  // Printing code for debug usage                                                 
  printf_P(PSTR("i2c Packet Rx'd. Cmd: %x Data: "),i2cRXCommand[i2cSel]);
  for (int i = 0;i < i2cdataLen[i2cSel]; i++)
  {
    Serial.print(i2cdata[i2cSel][i], HEX));
    printf_P(PSTR(" "));
  }
  Serial.println();
#endif
  i2cCmdsStored++; //increase the tally of i2c stored commands
}





