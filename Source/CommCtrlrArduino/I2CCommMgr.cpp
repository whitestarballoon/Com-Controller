#include <Wire.h>
#include "I2CCommMgr.h"
#include "CommCtrlrConfig.h"
#include "DebugMsg.h"
#include "I2CQueue.h"
#include "CutDown.h"

#define i2cDebug
/* 
 I2CXmit - Program for communicating via I2C Bus
 Created by White Star Balloon, December 11, 2010.
 Last Revised December 13, 2010
 */

I2CCommMgr::I2CCommMgr()
{
}

int I2CCommMgr::I2CXmit(byte device, byte command, byte* data, int length)
{

#ifdef i2cDebug
  DebugMsg::msg_P("I2",'I',PSTR("~I2CXmit~\nto Addr: %x\ncommand: %x"),device,command);

  for (byte i=0; i<length;i++) {
    DebugMsg::msg_P("I2",'I',PSTR("index:%d data:%x  "),i,data[i]);
  }
#endif

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
  DebugMsg::msg_P("I2C",'I',PSTR("I2C Init Done.\n"));
}



//Wire library interrupt will pass number of bytes to receive
void I2CCommMgr::i2cReceiveData(int wireDataLen) 
{
  int i=0,dataArraySize;
  I2CMsg i2cMsg;
#ifdef i2cDebug
  DebugMsg::msg_P("I2C",'I',PSTR("i2c 4 me!\n"));
#endif

  // Check to see if there's any I2C commands already 

  if ((wireDataLen -1) > i2cMaxDataLen) {
    DebugMsg::msg_P("I2C",'I',PSTR("\ni2cRx: data too big!\n"));
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
#ifdef i2cDebug
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
      DebugMsg::msg_P("I2C",'I',PSTR("Store Long Rpt"));
/*      
      //If Flight computer sends a pair of I2C EEPROM addresses that are the same, that's an error in the FC
      if((i2cdata[i2cSel][0] == i2cdata[i2cSel][2]) && (i2cdata[i2cSel][1] == i2cdata[i2cSel][3])){
        lprintf("CC:i2cEEPMsgAddrs same");
        printf_P(PSTR("i2cEEPMsgAddrs same\n"));
        break; // Do not continue, do not store requested pair.
      }
      satQueueI2CEEPROMLongMessage(i2cdata[i2cSel]);
      DebugMsg::msg_P("I2C",'I',PSTR("Store ATC Rpt"));
*/
      break;
    }
    
    case i2cCmdHFUpdtTelem: //Untested
    { 
      DebugMsg::msg_P("I2C",'I',PSTR("Store HF Update Telem"));
/*
      packetBufferS[0]=i2cdata[i2cSel][0];
      packetBufferS[1]=i2cdata[i2cSel][1];
      packetBufferS[2]=i2cdata[i2cSel][2];
      hfSendSerialCommand(packetBufferS,3);
*/
      break;
    }
  case i2cCmdHFTxShortRpt: 
    { 
      DebugMsg::msg_P("I2C",'I',PSTR("Store HF Tx Short Rpt"));
/*
      packetBufferS[0]='P';
      hfSendSerialCommand(packetBufferS,1);
*/
      break;
    }
    
    case i2cCmdHFSetTxRate: 
    { 
      DebugMsg::msg_P("I2C",'I',PSTR("Store HF Set Tx Rate"));
/*      
      packetBufferS[0]=i2cdata[i2cSel][0];
      hfSendSerialCommand(packetBufferS,1);
*/
      break;
    }
    
    case i2cCmdHFSnooze: 
    { 
      DebugMsg::msg_P("I2C",'I',PSTR("Store HF Cmd Snooze"));
/*
      packetBufferS[0]='X';
      hfSendSerialCommand(packetBufferS,1);	
*/
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

    case i2cCmdUpdateThreeNinersValue:
    { 
      DebugMsg::msg_P("I2C",'I',PSTR("Update 3 Niners"));
/*
      updateThreeNinersTelem();
*/
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
//      satInitRoutine(0);
      break;
    }
    
    case i2cCmdSATPowerOff: 
    { 
      DebugMsg::msg_P("I2C",'I',PSTR("SatModem OFF"));
//      satShutdownRoutine();
      break;
    }

  default:                                               // Ignore any command that is not in the list
    {
      DebugMsg::msg_P("I2C",'E',PSTR("Unknown Command"), i2cMsg.i2cRxCommand);
    }
  }
}


