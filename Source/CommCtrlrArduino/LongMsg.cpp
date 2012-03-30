
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
#define __WS_DEBUG

void LongMsg::set(byte bStartAddrH, byte bStartAddrL, byte bEndAddrH, byte bEndAddrL)
{

#ifdef __WS_DEBUG
        DebugMsg::msg_P("LM",'E', PSTR(" LongMsg::set(%d, %d, %d, %d)"), bStartAddrH, bStartAddrL,
                                    bEndAddrH, bEndAddrL);
#endif


        _StartAddr = word(bStartAddrH, bStartAddrL);
        _EndAddr = word(bEndAddrH, bEndAddrL);
}

int LongMsg::getFormattedMsg(unsigned char * data, int data_sz)
{
        byte gottenLength = getFormattedLength();
        int i = 0;

#ifdef __WS_DEBUG
        DebugMsg::msg_P("LM", 'I', PSTR(" s: %x e: %x gottenLength: %d data_sz: %d"),
                        _StartAddr, _EndAddr, gottenLength, data_sz);

#endif
        if ( gottenLength <= 0) {
                return -1;
        }

        if (gottenLength > data_sz) {
        
                DebugMsg::msg_P("LM",'E',PSTR("buf not big enough for FormattedMsg(%d) need %d"),
                                            data_sz, gottenLength);
                return -1;
        }

        for (unsigned int iAddr = _StartAddr ; iAddr <= _EndAddr ; iAddr++, i++)
        {
        	int FromEEPROM = I2CeePROMRead( i2ceePROMAddr, iAddr);
        	    if (-1 < FromEEPROM){ 
                	data[i] = FromEEPROM;
                } else {
#ifdef __WS_DEBUG
        DebugMsg::msg_P("LM", 'E', PSTR("FAILED TO READ ALL EEPROM BYTES, ABORTING."));
#endif                
                return -1; //eeprom read failed to get every byte, so do not continue processing this message.
                }
        }
        
#ifdef __WS_DEBUG
        DebugMsg::msg_P("LM", 'I', PSTR("Done reading message from eeprom."));
#endif
		return getFormattedLength();
}

boolean LongMsg::equals( LongMsg m )
{
        return ( m._StartAddr == _StartAddr && m._EndAddr == _EndAddr);
}

int LongMsg::I2CeePROMRead(byte device, unsigned int addr) 
{
        byte  EEPROMByte,sendStatus;
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
                delay(random(30));  //Needed to make it stop crashing? seen at http://www.arduino.cc/playground/Code/I2CEEPROM24LC512
                
                if (i2csentStatus != 0){
                        DebugMsg::msg_P("LM",'E', PSTR(" i2cTXErr: %d "), i2csentStatus);
                        //Random delay routine:
                        delay(random(3000));
                } 
                else {
                        Wire.requestFrom(device,(byte)1);                          

#if (ARDUINO >= 100)
                        EEPROMByte = Wire.read();  
#else
                        EEPROMByte = Wire.receive();  
#endif
                        return EEPROMByte; // Byte read OK
                }
        }
        DebugMsg::msg_P("LM",'E', PSTR("I2C Retries exceeded!"));
        return -1; //byte NOT read ok
}

