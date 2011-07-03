#include <EEPROM.h>
#include <Wire.h>
#include <NewSoftSerial.h> 
#include <avr/pgmspace.h>
#include <stdio.h>




/* 
 Communications Controller - Program for setting up all Digi m10 parameters
 Created by White Star Balloon, 2011073.

 */


/*************
 *
 *  Debuging Serial Output Verbosity Compiler options
 *
 ***************/
//***Uncomment this to get main loop serial statements
#define mainLoopDebug
//***Uncomment this to print out detailed satellite serial data packets
//#define satDataDebug
//#define satPrintEveryIncomingPacketDebug
//Uncomment this to print out the general action labels
#define actionLabels
//***uncomment this to have access to full parameter listing functions, not needed for flight
//#define satAccessAllParameters
//***Uncomment this to have I2C serial debugging output
//#define i2cDebug
//***Uncomment to Skip checking incoming checksums from sat modem
//#define skipSatInCHKs
//***Uncomment this for serial checksum debugging
//#define checkSumDebug

//Output file object for serial debug statements
static FILE mystdout = {0};

/*******************************
 *    Pin declarations         *
 *******************************/
const int sat_da = 2;                              // Satellite Modem Data Available
const int sat_sa = 3;                              // Satellite Modem Satellite Available
const int sat_tx = 4;                              // Satellite Modem Serial Transmit
const int sat_rx = 5;                              // Satellite Modem Serial Receive
const int PWR_EN = 6;                              // Satellite Modem Power Enable
const int sat_cts = 7;                             // Satellite Modem Clear to Send
const int sat_debug_rx = 8;                        // Satellite Modem Debug Receive
const int sat_rts = 9;                             // Satellite Modem Request to Send
const int sat_debug_tx = 10;                       // Sattelite Modem Debug Transmit
const int spare1 = 12;                             // Spare
const int extio1 = 12;                             // Satellite Modem
const int spare2 = 13;                             // Spare 
const int etx2_rx = 14;                            // External Transmitter 2 Receive
const int etx2_tx = 15;                            // External Transmitter 2 Transmit
const int etx1_rx = 16;                            // External Transmitter 1 Receive
const int etx1_tx = 17;                            // External Transmitter 1 Transmit
const int sda = 18;                                // I2C SDA
const int scl = 19;                                // I2C SCL

/*******************************
 *     Constants for Sat Modem *
 *******************************/
const byte packetLengthIndex = 2; 
const byte packetTypeIndex = 1;
const byte packetLLstatusCodeIndex = 4;
const byte packetRetStatusCodeIndex = 5;
const byte packetDTEheader = 0x86;  //0x85 or 0x86 sets the type of packet error tracking
//Configurable values for sat modem from here to break
const byte serialRetryLimit = 1;
const byte satIncomingPackLenLimit = 70;    //Used to define length of buffer arrays for packet data
const byte i2cRetryLimit = 10;
const unsigned int maxTelemLenConst = 1024;    //Maximum acceptable length of telemetry packet FROM EEPROM
const unsigned long satUplinkWaitDelayMillis = 60000;  //Time in mS for the CommCtrlr to wait for inbound msgs after sending one
const byte satParamQOBqty 0x16;
const byte satParamQIBqty 0x15;


/*******************************
 *   Internal EEPROM Locations         *
 *******************************/
const int EPLOCmessageNum = 0;  // EEPROM address for MHA message number byte
const int EPLOCcmdCounterArrayStart = 2; 
const int EPLENcmdCounterArray = 76;  //76 byte array to store used received command counter numbers.
const int EPLOCAtcReportPairArrayStart = 80;
const int EPLOCAtcReportPairArray = 12;
const int EPLOCI2CEEPLongMSGAddrArrayStart = 96;
const int EPLOCI2CEEPLongMSGAddrArrayLen = 405;  //fill the rest of eeprom with the circular buffer for this array 


/*******************************
 *     Settings for Sat Modem *
 *******************************/
PROGMEM prog_uchar satInitializeParam[]=  { 
  0x06, 0x28, 0x2D, 0x2F, 0x29, 0x01 };
PROGMEM prog_uchar satInitializeValues[]= { 
  0x01, 0x01, 0x00, 0x00, 0x05, 0x01 };
//  These will only be used for non-flight testing.
#ifdef satAccessAllParameters 
// Add 0x13 sco_msg_queue_size and 0x14 sct_msg_queue_size
PROGMEM prog_uchar satByteParams[]= { //Readable byte length sat parameters
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x0A, 0x0C, 0x0D, 0x0E, 0x0F, 0x15, 0x16, 0x1B, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x30, 0x32, 0x33, 0x34, 0x35, 0x36, 0x39, 0x3A };
PROGMEM prog_uchar satSetupWriteParams[]= {
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x13, 0x14, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x39, 0x3A, 0x3B };
PROGMEM prog_uchar satSetupWriteByteParams[]= {
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x13, 0x14, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x39, 0x3A, 0x3B };
PROGMEM prog_uchar satSetupWriteByteValues[]= {
  0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x40, 0x40, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };
#endif


/*******************************
 *    Variable Declarations    *
 *******************************/
byte messageRefNum=0;      									//MHA Message Num in RAM
byte gwy_id = 1;											//Current destination message gateway 1=North Am.  120 = Europe/Afr 
byte satLatestPacNum=0;										//Orbcomm Serial Protocol packet counter variable for incoming duplicate comparison
unsigned char packetBufferA[satIncomingPackLenLimit];		//General long reusable scratch array
unsigned char packetBufferB[satIncomingPackLenLimit];		//General long reusable scratch array
unsigned char packetBufferS[15];							//General long reusable scratch array
unsigned char packetSeqNum=0;								//Orbcomm Serial Protocol packet counter variable outbound
unsigned char rxerr=0;										//Error types from serial packet receive functions will get put here
boolean tempBool;											//Scratch
byte i2ccommand;											
int i2csentStatus;                                 // Status of I2C Tranismitted Message
byte checkSumA,checkSumB;							//Global Fletcher Checksum tracking for byte-by-byte I2C to Sat message sending


/*******************************
 *    Serial declarations      *
 *******************************/
NewSoftSerial sat(sat_tx,sat_rx);

//More stuff to allow flash-based debugging serial statements
extern "C" int lprintf(char*, ...);
extern "C" int lprintf_P(const char*, ...);	


void setup() {
  /*******************************
   *
   *   Pin I/O direction setting
   *
   ********************************/
  pinMode(sat_da, INPUT);
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
  digitalWrite(sat_da, LOW);

  /*********************
   *
   * Serial ports initialization
   *
   **********************/
   
//More stuff for flash string serial statements
    fdev_setup_stream(&mystdout,output_putchar,NULL,_FDEV_SETUP_WRITE);
	stdout = &mystdout; //Required for printf init
	

  sat.begin(9600);                                        // Start Satellite Serial Port
  Serial.begin(9600);                                     // Start hardware UART Serial Port for debugging
  delay(2500);
  Serial.println("Booting");  
  delay(500);
  /********************
   *
   *  Proceed with normal operation
   *
   ********************/
  satInitRoutine(0);  //SETTINGS OTHER THAN 0 ARE DEPRECATED. 0= skip setting init values, 1=Set a few params, 2= set most params.  Will power the modem on.  DO NOT COMMENT OUT
  //satPrintAllParameters();
//  i2cInitRoutine();   //Initialize i2c as a slave device



    //  byte temp = satGetByteParameter(packetBufferB,0x0D);  // Get sat modem diagnostic status
  //printf_P(PSTR("m10 Diag Code: "));
  //Serial.println(temp,DEC);
  printf_P(PSTR("Booted."));

  lprintf_P("CC: Booted\n");

  /*
  // Manual 6-byte command
   packetBufferS[0]= 'T';
   packetBufferS[1]= 'M';
   packetBufferS[2]= 'R';
   packetBufferS[3]= 'x';
   packetBufferS[4]= 'x';
   packetBufferS[5]= 'x';
   packetBufferS[6]= 'e';
   packetBufferS[7]= 'e';
   packetBufferS[8]= 'e';
   packetBufferS[9]= 'e';
   satSendDefaultReport(packetBufferS,packetBufferB);
   */

  //satSendSCOrigPositionRpt(latestPosition,packetBufferS);  //Defaults to Lousiville, KY
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

 

  // This just makes sure any random info packets from sat modem get dealt with soon after they arrive 
  //  in the NewSoftSerial 64 byte buffer.  This should wait for new data for only 6 to 10 seconds. 
  //  Satcom should send anything that's waiting every 5 sec
  while (sat.available()){  //available means there's NewSoftSerial data buffered in from the sat modem
    satPacketReceptionist(packetBufferB);
  }  


  satAOSLOSAlert();  //alert upon Aquisition of signal and Loss of signal
  msgCounterHouseKeeping();	

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
  //Check Queue values every so often
  //if (qchecktime+15000 < time) {  
	  printf_P(PSTR(" q16:"));
	  tempOBQ = satGetByteParameter(packetBufferB, 0x16);
	  printf_P(PSTR("%x  q15:"),tempOBQ);
	  tempOBQ = satGetByteParameter(packetBufferB, 0x15);
	  printf_P(PSTR("%x "),tempOBQ);
	  //qchecktime = time;
  //}
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




