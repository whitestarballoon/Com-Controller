
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


void LongMsg::set(byte bStartAddrH, byte bStartAddrL, byte bEndAddrH, byte bEndAddrL)
{

        DebugMsg::msg("LM",'E'," LongMsg::set(%d, %d, %d, %d)\n", bStartAddrH, bStartAddrL,
                                    bEndAddrH, bEndAddrL);


        _StartAddr = word(bStartAddrH, bStartAddrL);
        _EndAddr = word(bEndAddrH, bEndAddrL);
}

int LongMsg::getFormattedMsg(unsigned char * data, int data_sz)
{
        byte gottenLength = getFormattedLength();
        int i = 0;

        DebugMsg::msg("LM", 'I', " s: %x e: %x gottenLength: %d data_sz: %d", _StartAddr, _EndAddr, gottenLength, data_sz);
        if ( gottenLength <= 0) {
                return -1;
        }

        if (gottenLength > data_sz) {
        
                DebugMsg::msg("LM",'E'," buf not big enough for FormattedMsg(%d) need %d"),
                                            data_sz, gottenLength;
                return -1;
        }

        for (unsigned int iAddr = _StartAddr ; iAddr <= _EndAddr ; iAddr++, i++)
        {
                data[i] = I2CeePROMRead( i2ceePROMAddr, iAddr);
        }
        return getFormattedLength();
}

boolean LongMsg::equals( LongMsg m )
{
        return ( m._StartAddr == _StartAddr && m._EndAddr == _EndAddr);
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
                        DebugMsg::msg("LM",'E'," i2cTXErr: %d ",i2csentStatus);
                        //Random delay routine:
                        delayMicroseconds(random(3000));
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

