/* 
 I2CXmit - Program for communicating via I2C Bus
 Created by White Star Balloon, December 11, 2010.
 Last Revised December 13, 2010
 */

int I2CXmit(byte device, byte command, byte* data, int length)
{
#ifdef i2cDebug
  Serial.print("~I2CXmit~\nto Addr: ");
  Serial.println(device,HEX);
  Serial.print("command: ");
  Serial.println(command,HEX);

  for (byte i=0; i<length;i++) {
    Serial.print("data: ");
    Serial.print(i,DEC);
    Serial.print(" ");
    Serial.print(data[i],HEX);
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
  Serial.println("I2C Init Done.");
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
  Serial.println("i2c 4 me!");
#endif

// Check to see if there's any I2C commands already 
  
  if ((wireDataLen -1) > i2cDataLenConst) {
    Serial.println("\ni2cReceiveData: data too big!");
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
  Serial.print("i2c Packet Rx'd. ");
  Serial.print("Cmd: ");
  Serial.print(i2cRXCommand[i2cSel], HEX);
  Serial.print(" ");
  Serial.print("Data: ");
  for (int i = 0;i < i2cdataLen[i2cSel]; i++)
  {
    Serial.print(i2cdata[i2cSel][i], HEX);
    Serial.print(" ");
  }
  Serial.println();
#endif
  i2cCmdsStored++; //increase the tally of i2c stored commands
}





// Thist prints strings out the Ground Support Board by using i2c! Yay
int lprintf(char *str, ...)
{
	long time;
	long timestart;
	char lstr[30];
	uint8_t i2cSend[31];
	int chars;
	va_list args;

	va_start(args, str);

	chars = vsnprintf(lstr, 30, str, args);

	if(chars > 30)
	{
		va_end(args);
		return 1;
	} else {
	  I2CXmit(i2cGroundSupport, 0x05, (byte*)lstr, chars);
		va_end(args);
		return 0;
	}
  //Delay a bit for the GSP to catch up
  time = millis();
  timestart = time;
  while (timestart+100>time){
    time = millis();
  }
}

