
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <avr/pgmspace.h>
#include <string.h>
#include <Wire.h>
#include "LongMsg.h"
#include "DebugMsg.h"


        void set(byte bStartAddrH, byte bStartAddrL, byte bEndAddrH, byte bEndAddrL);


    
#define LONG_MSG_LEN 10 
    int LongMsg::getFormattedLength()
    {
      return LONG_MSG_LEN;
    }
     

    unsigned char * LongMsg::getFormattedMsg()
    {
      unsigned char data[LONG_MSG_LEN];
      return data; 
    }


    
    
    
    byte LongMsg::I2CeePROMRead(byte device, unsigned int addr) 
    {
      byte   eePROMbyte,sendStatus;
      boolean i2csentStatus;  
      for (byte i = 0; i < i2cRetryLimit; i++) {
        Wire.beginTransmission(device);         
    
    #if (ARDUINO >= 100)
        Wire.write((int)(addr >> 8));                           // left-part of pointer address
        Wire.write((int)(addr & 0xFF));                         // and the right
    #else
        Wire.send((int)(addr >> 8));                           // left-part of pointer address
        Wire.send((int)(addr & 0xFF));                         // and the right
    #endif
        
    
        i2csentStatus = Wire.endTransmission();
        if (i2csentStatus != 0){
          DebugMsg::msg("LM",'E',PSTR(" i2cErr: %d "),i2csentStatus);
          //Random delay routine:
          delayMicroseconds(random(1000));
        } 
        else {
          Wire.requestFrom(device,(byte)1);                          
    
    #if (ARDUINO >= 100)
          eePROMbyte = Wire.read();  
    #else
          eePROMbyte = Wire.receive();  
    #endif
          return eePROMbyte; 
        }
      }
    }

