#include <EEPROM.h>
#include <Wire.h>
#include <NewSoftSerial.h> 
#include <avr/pgmspace.h>
#include <stdio.h>




/* 
 Communications Controller - Program for communicating with the World from the Balloon
 Created by White Star Balloon, 2010,2011,2012.
 */


/***********
 * Satellite To-Do list
 * To Implement 
  	-add cutdown timer change response packet
  	-Iridium

 * Known Bugs To Solve
 ************/

/*************
 *
 *  Debuging Serial Output Verbosity Compiler options
 *
 ***************/
//***Uncomment this to get main loop serial statements
#define mainLoopDebug
//***Uncomment this to print out detailed satellite serial data packets
//#define satDataDebug
//Uncomment this to print out the general action labels
#define actionLabels
//***Uncomment this to have I2C serial debugging output
//#define i2cDebug
//***Uncomment this to turn Sat Modem OFF after boot
//#define simulateSat

//Output file object for serial debug statements
static FILE mystdout = {0};

/*******************************
 *    Pin declarations         *
 *******************************/
const int sat_sa = 3;                              // Satellite Modem Satellite Available
const int sat_tx = 4;                              // Satellite Modem Serial Transmit
const int sat_rx = 5;                              // Satellite Modem Serial Receive
const int PWR_EN = 6;                              // Satellite Modem Power Enable
const int sat_cts = 7;                             // Satellite Modem Clear to Send
const int sat_rts = 9;                             // Satellite Modem Request to Send
const int etx2_rx = 14;                            // External Transmitter 2 Receive
const int etx2_tx = 15;                            // External Transmitter 2 Transmit
const int etx1_rx = 16;                            // External Transmitter 1 Receive
const int etx1_tx = 17;                            // External Transmitter 1 Transmit
const int sda = 18;                                // I2C SDA
const int scl = 19;                                // I2C SCL

/*******************************
 *    Address declarations     *
 * These are software variable  *
 *******************************/
const byte i2cFlightComputer = 0x05;                  // Flight Computer I2C Address
const byte i2cCutDown = 0x06;                         // Cut Down Module I2C Address
const byte i2cGroundSupport = 0x07;                   // Ground Support Board I2C Address
const byte i2cCommCtrl = 0x08;                        // Communication Computer I2C Address
const byte i2cBallastCtrl = 0x09;                     // Ballast Computer I2C Address
const byte i2ceePROM = 0x50;                          // EEPROM I2C Address

/*******************************
 *     Constants for Sat Modem *
 *******************************/
const byte satIncomingPackLenLimit = 70;    //Used to define length of buffer arrays for packet data
const unsigned int satPowerOffMinimumTime = 2000; //Probably 2000 millis for iridium
const unsigned int maxTelemLenConst = 1024;    //Maximum acceptable length of telemetry packet FROM EEPROM

/*******************************
 *   Constants for message payload processing (set these based on headers of sat modem provider)
********************************/
const byte packetPayloadStartIndex = 6;  // Message content starts here in a received packet from sat modem 6 for orbcomm, may be 0 for Iridium
const byte satIncomingMessageHeaderLength = 15;  //Length of inbound message headers, 15 for orbcomm, may be 0 for Iridium
const byte i2cRetryLimit = 10;


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
const byte i2cDataLenConst = 15;  //I2C buffer length for cmd+data

/*******************************
 *   Internal EEPROM Locations         *
 *******************************/
const int EPLOCcmdCounterArrayStart = 2; 
const int EPLENcmdCounterArray = 76;  //76 byte array to store used received command counter numbers.
const int EPLOCAtcReportPairArrayStart = 80;  // Arduino EEPROM location to store the latest ATC report pair
const int EPLOCAtcReportPairArray = 12;



/*******************************
 *    Variable Declarations    *
 *******************************/
unsigned char packetBufferA[satIncomingPackLenLimit];		//General long reusable scratch array
unsigned char packetBufferB[satIncomingPackLenLimit];		//General long reusable scratch array
unsigned char packetBufferS[15];							//General long reusable scratch array
boolean tempBool;											//Scratch
byte i2ccommand;											
byte i2cRXCommand[2]={
  	0xFF,0xFF};                                      // Global variable for I2C Command Transmission
byte i2cdata[2][i2cDataLenConst]; // Global variable for I2C Data
int i2cdataLen[2]={
  0,0};
byte i2cCmdsStored=0;
byte i2cSel=0;
int i2csentStatus;                                 // Status of I2C Tranismitted Message
unsigned int epochMinutes=0;  //Last time received from Flight Computer, in Minutes (+/- 1 minute accuracy) 
//^^^^^^^^^^^^^^^^^^^^^^^^^     this is NOT updated regularly, only during an ATC short report TX
boolean ATCRptReady = false; //Set to true if there's an ATC report waiting to be sent.
boolean LongMsgReady = false; //Set to true if there's a long Message waiting to be sent.
boolean satSyncFlag = false;									//Flag that should be set when sat network is available
boolean prevSatSyncStateFlag = false;



/*******************************
 *    Serial declarations      *
 *******************************/
NewSoftSerial sat(sat_tx,sat_rx);
NewSoftSerial hf(etx1_tx,etx1_rx);
NewSoftSerial cdn(etx2_tx,etx2_rx);

//More stuff to allow flash-based debugging serial statements
extern "C" int lprintf(char*, ...);
extern "C" int lprintf_P(const char*, ...);	


void setup() {
  /*******************************
   *
   *   Pin I/O direction setting
   *
   ********************************/
  pinMode(sat_sa, INPUT);
  pinMode(sat_tx, INPUT);
  pinMode(sat_rx, OUTPUT);
  pinMode(PWR_EN, OUTPUT);
  pinMode(sat_cts, INPUT);
  pinMode(sat_debug_rx, OUTPUT);
  pinMode(sat_rts, OUTPUT);
  pinMode(sat_debug_tx, INPUT);
  pinMode(spare1, INPUT);
  pinMode(extio1, INPUT);
  pinMode(spare2, OUTPUT);                                // LED Pin
  pinMode(etx2_rx, OUTPUT);
  pinMode(etx2_tx, INPUT);
  pinMode(etx1_rx, OUTPUT);
  pinMode(etx2_tx, INPUT);
  pinMode(sda, INPUT);
  pinMode(scl, INPUT);

  /*************************
   *
   *  Set Pullups
   *
   **************************/

  /*********************
   *
   * Serial ports initialization
   *
   **********************/
   
//More stuff for flash string serial statements
    fdev_setup_stream(&mystdout,output_putchar,NULL,_FDEV_SETUP_WRITE);
	stdout = &mystdout; //Required for printf init
	

  hf.begin(4800);                                        // start the HF transmitter Serial Port
  sat.begin(9600);                                        // Start Satellite Serial Port
  cdn.begin(1200);    									//Start cutdown Serial Port 
  Serial.begin(9600);                                     // Start hardware UART Serial Port for debugging
  delay(2500);
  Serial.println("Booting");  
  delay(500);
  /********************
   *
   *  Proceed with normal operation
   *
   ********************/
  cdnInit();
  satInitRoutine();  
  i2cInitRoutine();   //Initialize i2c as a slave device

#ifdef simulateSat
    digitalWrite(PWR_EN,LOW);
  lprintf("CC:SATMODEM OFF");
#endif

  printf_P(PSTR("Booted."));

  lprintf_P("CC: Booted\n");

   #ifndef mainLoopDebug
   	Serial.println(" NO MAIN DBUG");
   #endif
}



void loop() {
  unsigned long time, timestart, qchecktime=0;
  unsigned char cycle,r;
  byte tempOBQ=0;

  //Wake up the sat port
  sat.available();

  //Set start of loop time for later delay of at least 1s
  time = millis();
  timestart = time;

  // Check for I2C inbound things to process that came in during interrupts
  if (i2cCmdsStored > 0) {
    //If there's 1 command stored, then select #0
    //If there's 2 commnds stored, then select #1
    i2cSel = i2cCmdsStored-1;  
    //Parse selected command
    I2CParse(i2cRXCommand[i2cSel]);                                      // Parse the command that was received
    i2cCmdsStored--;  // After done processing, decrement the stored tally
    msgCounterHouseKeeping();  //Checks to see if variable messageRefNum is OK
  }
  
  satAOSLOSAlert();  //alert upon Aquisition of signal and Loss of signal
  satOutgoingMsgOperations();


#ifdef mainLoopDebug
	
  //Generates serial debugging output visual pattern, not for flight!
  if (digitalRead(sat_sa)){
    printf_P(PSTR("|"));  // Generate vertical bars if there's sat signal
  } 
  else {
    printf_P(PSTR("."));
  }
  cycle = random(100);
  if ( 95 < cycle ) {
    Serial.println();
  }
#endif
  

  time = millis();
  while (timestart+1000>time){
    time = millis();
  }
}


//Stuff for flash serial debug
static int output_putchar(char c, FILE *stream)
{
    if (c == '\n') output_putchar('\r', stream);
    Serial.print(c);
    return 0;
}




