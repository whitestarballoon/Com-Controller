#ifndef CommCtrlrConfig_h
#define CommCtrlrConfig_h

#define __WHITESTARBALLOON__
#define printExpectings
#define printVerifiedModemSerialMessages


#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

//************* Watchdog Setup ****************
extern unsigned long wdResetTime;
#define TIMEOUTPERIOD 120000UL             // You can make this time as long as you want,
                                       // it's not limited to 8 seconds like the normal
                                       // watchdog
#define wdtrst() wdResetTime = millis();  // This macro will reset the timer
/*
Global Configuration Information
*/
#define IRIDIUM_SERIAL_PORT Serial1
#define CUTDOWN_SERIAL_PORT Serial2

/*******************************
 *    Pin declarations         *
 *******************************/
#define pinRI 2
#define pinNA 3
#define pinDSR 6
#define pinModemPowerSwitch 4
#define pinBrownout 5
//define pinVoltageMeasure A0
const int sda = 18;                                // I2C SDA
const int scl = 19;                                // I2C SCL


/*******************************
 *    Address declarations     *
 * These are software variable  *
 *******************************/
const byte i2cFlightComputerAddr  = 0x05;           // Flight Computer I2C Address
const byte i2cCutDownAddr         = 0x06;           // Cut Down Module I2C Address
const byte i2cGroundSupportAddr   = 0x07;           // Ground Support Board I2C Address
const byte i2cCommCtrlAddr        = 0x08;           // Communication Computer I2C Address
const byte i2cBallastCtrlAddr     = 0x09;           // Ballast Computer I2C Address
const byte i2ceePROMAddr          = 0x50;           // EEPROM I2C Address

/*******************************
 *     Constants for Sat Modem *
 *******************************/
const byte satIncomingPackLenLimit = 70;             //Used to define length of buffer arrays for packet data
const unsigned int satPowerOffMinimumTime = 2000;    //Probably 2000 millis for iridium
const unsigned int maxTelemLenConst = 340;          //Maximum acceptable length of telemetry packet FROM EEPROM, set by Iridium MO max message size
#define satNetworkNotAvailable = 255;
const int LongMsgQueueLen = 20;             // Number of messages that can be in the queue to send out sat modem
const unsigned int satResponseTimeout = 2UL * 60UL * 1000UL;       // (ms) Timeout used when waiting for response timeouts
const unsigned int satSBDIXResponseLost = 30000;     // (ms) How much time to wait before assuming SBDIX command failed
/* (ms) Force initiate SBD session after period of no activity i
 * currently set to 120 minutes 
 */
const unsigned long satForceSBDSessionInterval = (120UL * 60UL * 1000UL); 

/*******************************
 *   I2C Incoming Queue
********************************/
#define I2C_MSG_QUEUE_LEN 4
const byte I2CMsgQueueLen = I2C_MSG_QUEUE_LEN;      // Number of messages that can be in the queue of received I2C msgs
const byte i2cMaxDataLen = 15;                      //I2C buffer length for cmd+data
const byte UseI2CForDebugMsgs = 0; 					// 0 == NO, !0 == Yes

/*******************************
 *   Constants for message payload processing (set these based on headers of sat modem provider)
********************************/
const byte packetPayloadStartIndex = 6;  // Message content starts here in a received packet from sat modem 6 for orbcomm, may be 0 for Iridium
const byte satIncomingMessageHeaderLength = 15;  //Length of inbound message headers, 15 for orbcomm, may be 0 for Iridium
const byte i2cRetryLimit = 10;
const unsigned int satMessageCharBufferSize = 340;  //Char array size for loading messages from eeprom into


/*******************************
 *     Constants for I2C Commands to Comm Controller  *
 *******************************/
const byte i2cCmdSATTXATCRpt = 0x00;
const byte i2cCmdSATTxFrmEEPROM = 0x01;
const byte i2cCmdHFUpdtTelem = 0x02;
const byte i2cCmdHFTxShortRpt = 0x03;
const byte i2cCmdHFSetTxRate = 0x04;
const byte i2cCmdHFSnooze = 0x05;
const byte i2cCmdCDNHeartBeat = 0x06;
const byte i2cCmdCDNSetTimerAndReset = 0x07;
const byte i2cCmdUpdateCurrentEpochTime = 0x08;
const byte i2cCmdUpdateThreeNinersValue = 0x09;
const byte i2cCmdCDNCUTDOWNNOW = 0x99;
const byte i2cCmdSATPowerOn = 0xBB;
const byte i2cCmdSATPowerOff = 0xAA;

/*******************************
 *   Internal EEPROM Locations         *
 *******************************/
const int EPLOCcmdCounterArrayStart = 2; 
const int EPLENcmdCounterArray = 76;  //76 byte array to store used received command counter numbers.
const int EPLOCAtcReportPairArrayStart = 80;  // Arduino EEPROM location to store the latest ATC report pair
const int EPLOCAtcReportPairArray = 12;






#endif


