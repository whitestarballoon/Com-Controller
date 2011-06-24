#include <EEPROM.h>
#include <Wire.h>
#include <NewSoftSerial.h> 
#include <avr/pgmspace.h>

/***********
 * T-Do list
 * - see what dl 6/16 frames means regarding 0x3a parameter
 * - Implement 
 * - setting lat/lon sending to orbcomm
 * - Respond to System Announcement: announce_code 0 - Orbcomm requesting a position report to the gatway
 * - ?Status packet 
 * - System Response packet
 * - SC terminated user command
 * - SC Terminated message
 * - SC Originated Position Report
 * - 
 * 
 * 
 * 
 ************/

// uncomment this to print out detailed satellite serial data packets
//define satDataDebug
// uncomment this to have access to full parameter listing functions, not needed for flight
//#define satAccessAllParameters

/* 
 Communications Controller - Program for communicating with the Satellite Modem
 Created by White Star Balloon, December 8, 2010.
 Last Revised December 11, 2010
 */

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
const byte serialRetryLimit = 3;


/*******************************
 *    EEPROM Locations         *
 *******************************/
const int EPLOCmessageNum = 0;  // EEPROM address

/*******************************
 *     Settings for Sat Modem *
 *******************************/

PROGMEM prog_uchar satInitializeParam[]=  { 
  0x06, 0x28, 0x2D, 0x2F, 0x29, 0x01 };
PROGMEM prog_uchar satInitializeValues[]= { 
  0x01, 0x01, 0x00, 0x00, 0x05, 0x01 };

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
 *    Address declarations     *
 * These are software varible  *
 *******************************/

const byte FlightComputer = 0x05;
const byte CutDown = 0x06;
const byte GroundSupport = 0x07;
const byte CommCtrl = 0x08;
const byte BallastCtrl = 0x09;

/*******************************
 *    Variable Declarations    *
 *******************************/

byte test = B00001000;
byte y = 100;
byte address=0;
byte prefix=0;
byte command=0;
byte data[9]= { 
  0x00, 0x85, 0x12, 0x08, 0x00, 0x00, 0x31, 0x67, 0xC9 };
byte messageRefNum=0;
byte gwy_id = 1;


unsigned char packetBufferA[70];
unsigned char packetBufferB[70];
unsigned char packetBufferS[10];
unsigned char packetSeqNum=0;
unsigned char rxerr=0;
boolean satAvailable;
boolean tempBool;

/*******************************
 *    Serial declarations      *
 *******************************/
NewSoftSerial sat(sat_tx,sat_rx);

void setup() 
{

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


  /********************
   *
   *  Proceed with normal operation
   *
   ********************/

  digitalWrite(PWR_EN, LOW);   // Turn off the Satcom
  digitalWrite(sat_rts,LOW);  // Set RTS flow control to let data from SC to FIO
  //Wire.begin(CommCtrl);                                   // Join I2C Bus
  //Wire.onReceive(receiveData);                            // Set On Receive Handler
  delay(5000);

  sat.begin(9600);                                        // Start Satellite Serial Port
  Serial.begin(9600);                                     // Start Regular Serial Port
  Serial.println("bootup");


  satInitRoutine(0);  //0= skip setting init values, 1=Set a few params, 2= set most params.  Will power the modem on.  DO NOT COMMENT OUT
  //satPrintAllParameters();

/*
  byte temp = satGetByteParameter(packetBufferB,0x0D);  // Get sat modem diagnostic status
  Serial.print("Parameter Response: SatModem Diag Code: ");
  Serial.println(temp,DEC);
*/
  pinMode(sat_rx, INPUT);
  pinMode(sat_tx, INPUT);
  Serial.print("Idle.");
  delay(5000);
}

void loop()
{

}














