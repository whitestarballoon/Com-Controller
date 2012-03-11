#include <Wire.h>
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

I2CCommMgr::I2CCommMgr(SatCommMgr& satCommMgr):_satCommMgr(satCommMgr)
{
}

int I2CCommMgr::I2CXmit(byte device, byte command, byte* data, int length)
{
  // Transmit over I2C
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
  int sentStatus = Wire.endTransmission();              // Send queue, will return status
  return sentStatus;                                   // Return Status of Transmission
}

int I2CCommMgr::I2CXmitMsg(byte device, byte* data, int length)
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


void I2CCommMgr::i2cInit()
{
  Wire.begin(i2cCommCtrlAddr);                                   // Join I2C Bus as slave
  Wire.onReceive(I2CCommMgr::i2cReceiveData);                            // Set On Receive Handler
  DebugMsg::msg_P("I2C",'I',PSTR("I2C Init Done. Addr %x"),i2cCommCtrlAddr);

}



//Wire library interrupt will pass number of bytes to receive
void I2CCommMgr::i2cReceiveData(int wireDataLen) 
{
  int i=0,dataArraySize;
  I2CMsg i2cMsg;


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
  DebugMsg::msg_P("I2C",'I',PSTR("i2c Packet Rx'd. Cmd: %x Data: "),i2cMsg.i2cRxCommand);
  for (int i = 0;i < i2cMsg.i2cDataLen; i++)
  {
    DebugMsg::msg_P("I2C",'I'," %0x",i2cMsg.i2cData[i]);
  }
#endif
  // Store the Command
  I2CQueue::getInstance().write(i2cMsg);
}


/*
  Look at the queue and see if there is anything that needs to be processed.
*/
void I2CCommMgr::update()
{
    
    if (  I2CQueue::getInstance().count() > 0)  // Got a message that needs processing
    {
      I2CParse( I2CQueue::getInstance().read());
      Serial.print("vvvvvvv Q:"); Serial.println(I2CQueue::getInstance().count());
    }
}



void I2CCommMgr::I2CParse(I2CMsg i2cMsg)
{

  DebugMsg::msg_P("I2C",'I',PSTR("I2C Parse"));
  
  switch(i2cMsg.i2cRxCommand){

    case i2cCmdSATTXATCRpt: 
    { 
      DebugMsg::msg_P("I2C",'I',PSTR("Store ATC Rpt"));
/*
      //Store Latest position and Epoch Time for later use
      latestPosition[0]=i2cdata[i2cSel][6];  //lat0
      latestPosition[1]=i2cdata[i2cSel][7];  //lat1
      latestPosition[2]=i2cdata[i2cSel][8];  //lat2
      latestPosition[3]=i2cdata[i2cSel][0];  //lon0
      latestPosition[4]=i2cdata[i2cSel][1];  //lon1
      latestPosition[5]=i2cdata[i2cSel][2];  //lon2
      epochMinutes = word((i2cdata[i2cSel][4] & B00001111),i2cdata[i2cSel][5]);  //Copy epochMinute value into ram
      satSaveATCPkt(); //save the ATC packet to be ready to send when sat is ready to accept.
*/
      break;
    } 
    
    case i2cCmdSATTxFrmEEPROM: 
    { 
      DebugMsg::msg_P("I2C",'I',PSTR("Store Long Rpt:"));
/*      
      //If Flight computer sends a pair of I2C EEPROM addresses that are the same, that's an error in the FC
      if((i2cdata[i2cSel][0] == i2cdata[i2cSel][2]) && (i2cdata[i2cSel][1] == i2cdata[i2cSel][3])){
        lprintf("CC:i2cEEPMsgAddrs same");
        printf_P(PSTR("i2cEEPMsgAddrs same\n"));
        break; // Do not continue, do not store requested pair.
      }
      satQueueI2CEEPROMLongMessage(i2cdata[i2cSel]);
      DebugMsg::msg_P("I2C",'I',PSTR("Store Long Rpt"));
*/
	  // how do you use this VVV	
	  //if (SatQueue::write()) 
	  if (0 == 1)
	  {
	  	DebugMsg::msg_P("I2C",'I',PSTR("Report Stored OK."));
	  } else {
	  	DebugMsg::msg_P("I2C",'W',PSTR("Report Store FAILED."));
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
      DebugMsg::msg_P("I2C",'E',PSTR("Unknown Command"), i2cMsg.i2cRxCommand);
    }
  }
}




