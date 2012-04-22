#include <Wire.h>
#include <avr/io.h>
#include "I2CCommMgr.h"
#include "CommCtrlrConfig.h"
#include "DebugMsg.h"
#include "I2CQueue.h"
#include "CutDown.h"
#include "Iridium9602.h"
#include "SatCommMgr.h"

#define i2cDebug
/* 
 I2CXmit - Program for communicating via I2C Bus
 Created by White Star Balloon, December 11, 2010.
 Last Revised December 13, 2010
 */

I2CCommMgr::I2CCommMgr(SatCommMgr& satCommMgr):
_satCommMgr(satCommMgr)
{
}

//I2CMaxRetries should probably be no more than 10 as each retry delay will increase exponentially.
int I2CCommMgr::I2CXmit(byte device, byte command, byte* data, int length)
{
  
  int sentStatus;
  wdtrst();
  // Transmit over I2C
  for (unsigned int i = 1; i < i2cRetryLimit; i++) 
  {
    Wire.beginTransmission(device);                      // Begin Transmission to (address)
#if (ARDUINO >= 100)
    Wire.write(command);                                   // Put (command) on queue
#else
    Wire.send(command);                                   // Put (command) on queue
#endif
    for (int i = 0;i < length; i++)                       // Loop to put all (data) on queue
    {
#if (ARDUINO >= 100)
      Wire.write(data[i]);                                   // Put (command) on queue
#else
      Wire.send(data[i]);                                   // Put (command) on queue
#endif
    }
    sentStatus = Wire.endTransmission();              // Send queue, will return status
    if (sentStatus == 0) 
    {
      break;
    }
    Serial.println(F("I2CTXERR"));
    Serial.flush();
    //If it didn't' suceed for  any reason, try again
    delay(random(((i-1)*(i-1)*100),(i*i*100)));  //Delay for a random time between (i-1)^2*100 millis i^2*100 millis
  }
  return sentStatus;                                   // Return Status of Transmission	
}

int I2CCommMgr::I2CXmitMsg(byte device, byte* data, int length)
{

  int sentStatus;
  wdtrst();
  // Transmit over I2C
  for (unsigned int i = 1; i < i2cRetryLimit; i++) 
  {
    // Transmit over I2C
    Wire.beginTransmission(device);                      // Begin Transmission to (address)
    for (int i = 0;i < length; i++)                       // Loop to put all (data) on queue
    {
#if (ARDUINO >= 100)
      Wire.write(data[i]);                                   // Put (command) on queue
#else
      Wire.send(data[i]);                                   // Put (command) on queue
#endif
    }
    sentStatus = Wire.endTransmission();              // Send queue, will return status
    if (sentStatus == 0) 
    {
      break;
    }
    Serial.println(F("I2CTXERR"));
    Serial.flush();
    //If it didn't' suceed for  any reason, try again
    delay(random(((i-1)*(i-1)*100),(i*i*100)));  //Delay for a random time between (i-1)^2*100 millis i^2*100 millis
  }
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


void I2CCommMgr::i2cInit()
{
  wdtrst();
  Wire.begin(i2cCommCtrlAddr);                                   // Join I2C Bus as slave
#define TWI_FREQ_WSBFAST 400000UL
#ifndef CPU_FREQ
#define CPU_FREQ = 16000000UL
#endif
  //TWBR = ((CPU_FREQ / TWI_FREQ_WSBFAST) - 16) / 2;  // Make I2C FASTER! to 400KHz

  Wire.onReceive(I2CCommMgr::i2cReceiveData);                            // Set On Receive Handler
  DebugMsg::msg_P("I2C",'I',PSTR("I2C Init Done. Addr %x"),i2cCommCtrlAddr);

}



//Wire library interrupt will pass number of bytes to receive
void I2CCommMgr::i2cReceiveData(int wireDataLen) 
{
  int i=0,dataArraySize;
  I2CMsg i2cMsg;
	wdtrst();

  // Check to see if the I2C messsage is too big to handle if so throw it away
  if ((wireDataLen -1) > i2cMaxDataLen) {
    Serial.println("2B");
    DebugMsg::msg_P("I2C",'I',PSTR("\ni2cRx: data too big!"));
    while(Wire.available() > 0)           // Loop to receive the Data from the I2C Bus
    {
#if (ARDUINO >= 100)
      Wire.read();   // Finish receiving data and do not store.
#else
      Wire.receive();   // Finish receiving data and do not store.
#endif
    }
  }
  else
  {
    i2cMsg.i2cDataLen = wireDataLen - 1;
#if (ARDUINO >= 100)
    i2cMsg.i2cRxCommand = Wire.read();          // Receive the Command as the first byte from the I2C Bus
#else
    i2cMsg.i2cRxCommand = Wire.receive();          // Receive the Command as the first byte from the I2C Bus
#endif

    while(Wire.available() > 0)           // Loop to receive the Data from the I2C Bus
    {
#if (ARDUINO >= 100)
      i2cMsg.i2cData[i] = Wire.read();
#else
      i2sMsg.i2cData[i] = Wire.receive();
#endif
      i++;
    }
  }
  //  Serial.print("Cmd:");Serial.print(i2cMsg.i2cRxCommand);
#ifdef i2cDebug_DONT_ENABLE  // This makes the serial out too long and causes lockup
  // Printing code for debug usage                                                 
  DebugMsg::msg("I2C",'I',"i2c Packet Rx'd. Cmd: %x Data: ",i2cMsg.i2cRxCommand);
  for (int i = 0;i < i2cMsg.i2cDataLen-1; i++)
  {
    Serial.print(i2cMsg.i2cData[i],HEX); 
    Serial.print(" ")
      //DebugMsg::msg("I2C",'I'," %0x",i2cMsg.i2cData[i]);
    }
    Serial.println();
#endif
  // Store the Command
  I2CQueue::getInstance().write(i2cMsg);
}


/*
  Look at the queue and see if there is anything that needs to be processed.
 */
void I2CCommMgr::update()
{
  wdtrst();
  if (  I2CQueue::getInstance().count() > 0)  // Got a message that needs processing
  {
    I2CParse( I2CQueue::getInstance().read());
    //Serial.print("vvvvvvv Q:"); Serial.println(I2CQueue::getInstance().count());
  }
}



void I2CCommMgr::I2CParse(I2CMsg i2cMsg)
{
 wdtrst();
#if 0
  //DebugMsg::msg_P("I2C",'I',PSTR("I2C Parse"));
  DebugMsg::msg("I2C",'I',("i2c Packet Rx'd. Cmd: %0x Data V "),i2cMsg.i2cRxCommand);
  Serial.flush();
  for (int i = 0;i < i2cMsg.i2cDataLen; i++)
  {
    Serial.print(i2cMsg.i2cData[i],HEX); 
    Serial.print(" ");
  }
  Serial.println();
  Serial.flush();
#endif
  switch(i2cMsg.i2cRxCommand){

  case i2cCmdSATTXATCRpt: 
    { 
      DebugMsg::msg_P("I2C",'I',PSTR("Store ATC Rpt no function"));
   
      break;
    } 

  case i2cCmdSATTxFrmEEPROM: 
    { 
      //Check for duplicate start and end addresses
      if( (i2cMsg.i2cData[0] == i2cMsg.i2cData[2]) && (i2cMsg.i2cData[1] == i2cMsg.i2cData[3])) {
        DebugMsg::msg_P("I2C",'W',PSTR("LongMsg Start and End Addrs The Same.  No Send."));
      } 
      else {
        DebugMsg::msg_P("I2C",'I',PSTR("Store Long Message"));
        LongMsg msg(i2cMsg.i2cData[0],i2cMsg.i2cData[1],i2cMsg.i2cData[2],i2cMsg.i2cData[3]);

        if (SatQueue::getInstance().write(msg))
        {
          //DebugMsg::msg_P("I2C",'I',PSTR("Report Stored OK."));
        } 
        else {
          DebugMsg::msg_P("I2C",'W',PSTR("Report Store FAILED."));
        }
      }
      break;
    }


  case i2cCmdCDNHeartBeat: 
    { 
      DebugMsg::msg_P("I2C",'I',PSTR("CutDn Heart Beat"));
      CutDown::ResetTimer();
      break;
    }

  case i2cCmdCDNSetTimerAndReset: 
    { 
      DebugMsg::msg_P("I2C",'I',PSTR("CutDn Set Timer and Reset"));
      CutDown::CmdSet(i2cMsg.i2cData[0]);  //Take 1 byte 0-255 for minutes
      break;
    }


  case i2cCmdCDNCUTDOWNNOW: 
    { 
      DebugMsg::msg_P("I2C",'I',PSTR("CutDn Now"));
      CutDown::CutdownNOW();
      break;
    }

  case i2cCmdSATPowerOn: 
    { 
      DebugMsg::msg_P("I2C",'I',PSTR("SatModem ON"));
      _satCommMgr.turnModemOn();
      break;
    }

  case i2cCmdSATPowerOff: 
    {
      DebugMsg::msg_P("I2C",'I',PSTR("SatModem OFF"));
      _satCommMgr.turnModemOff();
      break;
    }

  default:                                               // Ignore any command that is not in the list
    {
      DebugMsg::msg("I2C",'E',"Unknown Command: %0x"), i2cMsg.i2cRxCommand;
    }
  }
}





