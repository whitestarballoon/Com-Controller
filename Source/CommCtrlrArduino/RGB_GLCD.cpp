/*
  RGB_GLCD.cpp - Arduino library support for LDS183 graphic LCDs
  Copyright (C)2011 Henning Karlsen. All right reserved
  
  The high level functions of this library are based on the demo-code provided by
  NKC Electronics. You can find the latest version of the library at
  http://www.henningkarlsen.com/electronics

  This library has been made especially for the excellent RGB LCD Shield for Arduino 
  65K color KIT by NKC Electronics. It might be possible to adapt the library for 
  other displays using the same controller, but I do not have any, so this is 
  unverified.

  If you make any modifications or improvements to the code, I would appreciate
  that you share the code with me so that I might include it in the next release.
  I can be contacted through http://www.henningkarlsen.com/electronics/contact.php

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Version:   1.0  - May  6 2010  - initial release
             1.1  - May  8 2010  - fixed a bug in drawLine
             1.2  - May 12 2010  - Added printNumI() and printNumF()
			 1.3  - May 13 2010  - Added support for Arduino Mega
  			 1.4  - Oct 14 2010  - Added drawBitmap()
 			 1.5  - Nov 24 2010  - Added Arduino Mega2560 compatibility
								   Added function to rotate bitmaps
 			 1.6  - Jan 16 2011  - Fixed a bug in the print() function
								   when using a background color different
								   from the screen background color.
  			 1.7  - Feb  1 2011  - Optimized drawBitmap() when not using 
								   rotation
			 1.8  - Mar  4 2011  - Fixed a bug in printNumF when the number to be
								   printed was (-)0.something
								   Fixed a bug in printNumIwhen the number to be 
								   printed was 0
   
*/

#define __WHITESTARBALLOON__

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "RGB_GLCD.h"
#include <avr/pgmspace.h>


#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

byte Font5x9Mono[535] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x08, 0x42, 0x10, 0x04, 
	0x00, 0x14, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x05, 0x7D, 0x4A, 
	0xFA, 0x80, 0x02, 0x3E, 0x8E, 0x2F, 0x88, 0x00, 0x02, 0x22, 
	0x22, 0x22, 0x00, 0x01, 0x14, 0xA2, 0x2B, 0x26, 0x80, 0x04, 
	0x22, 0x00, 0x00, 0x00, 0x00, 0x11, 0x08, 0x42, 0x10, 0x82, 
	0x02, 0x08, 0x42, 0x10, 0x84, 0x40, 0x00, 0x4A, 0xBA, 0xA4, 
	0x00, 0x00, 0x02, 0x13, 0xE4, 0x20, 0x00, 0x00, 0x00, 0x00, 
	0x01, 0x08, 0x80, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x02, 0x00, 0x00, 0x08, 0x88, 0x88, 0x00, 0x00, 
	0x74, 0x67, 0x5C, 0xC5, 0xC0, 0x01, 0x18, 0x42, 0x10, 0x8E, 
	0x00, 0x1D, 0x10, 0x99, 0x10, 0xF8, 0x00, 0xE8, 0x84, 0xC1, 
	0x8B, 0x80, 0x01, 0x19, 0x52, 0xF8, 0x84, 0x00, 0x7E, 0x1E, 
	0x08, 0x62, 0xE0, 0x01, 0xD0, 0xF4, 0x63, 0x17, 0x00, 0x1F, 
	0x08, 0x88, 0x84, 0x20, 0x00, 0x74, 0x62, 0xE8, 0xC5, 0xC0, 
	0x03, 0xA3, 0x18, 0xBC, 0x2E, 0x00, 0x00, 0x02, 0x00, 0x04, 
	0x00, 0x00, 0x00, 0x10, 0x00, 0x21, 0x10, 0x00, 0x08, 0x88, 
	0x20, 0x80, 0x00, 0x00, 0x1F, 0x07, 0xC0, 0x00, 0x00, 0x08, 
	0x20, 0x88, 0x80, 0x00, 0x0E, 0x88, 0x44, 0x40, 0x10, 0x00, 
	0x03, 0xA3, 0x5B, 0xC1, 0xC0, 0x01, 0x15, 0x18, 0xFE, 0x31, 
	0x00, 0x3D, 0x18, 0xFA, 0x31, 0xF0, 0x00, 0xE8, 0xC2, 0x10, 
	0x8B, 0x80, 0x0E, 0x4A, 0x31, 0x8C, 0xB8, 0x00, 0x7E, 0x10, 
	0xF4, 0x21, 0xF0, 0x03, 0xF0, 0x87, 0xA1, 0x08, 0x00, 0x0E, 
	0x8C, 0x21, 0x38, 0xB8, 0x00, 0x8C, 0x63, 0xF8, 0xC6, 0x20, 
	0x03, 0x88, 0x42, 0x10, 0x8E, 0x00, 0x1E, 0x21, 0x08, 0x52, 
	0x60, 0x01, 0x19, 0x53, 0x14, 0x94, 0x40, 0x08, 0x42, 0x10, 
	0x84, 0x3E, 0x00, 0x47, 0x75, 0xAC, 0x63, 0x10, 0x02, 0x39, 
	0xAD, 0x6B, 0x38, 0x80, 0x0E, 0x8C, 0x63, 0x18, 0xB8, 0x00, 
	0xF4, 0x63, 0x1F, 0x42, 0x00, 0x03, 0xA3, 0x18, 0xD6, 0x4D, 
	0x00, 0x3D, 0x18, 0xC7, 0xD1, 0x88, 0x00, 0xE8, 0xC1, 0xC1, 
	0x8B, 0x80, 0x0F, 0x90, 0x84, 0x21, 0x08, 0x00, 0x46, 0x31, 
	0x8C, 0x62, 0xE0, 0x02, 0x31, 0x8A, 0x94, 0x42, 0x00, 0x11, 
	0x8C, 0x6B, 0x55, 0x28, 0x00, 0x8C, 0x54, 0x45, 0x46, 0x20, 
	0x04, 0x63, 0x15, 0x10, 0x84, 0x00, 0x3E, 0x11, 0x11, 0x10, 
	0xF8, 0x00, 0x62, 0x10, 0x84, 0x21, 0x0C, 0x00, 0x41, 0x04, 
	0x10, 0x40, 0x00, 0x30, 0x84, 0x21, 0x08, 0x46, 0x00, 0x8A, 
	0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 
	0x21, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC1, 0x3A, 0x4E, 
	0x00, 0x21, 0x0E, 0x4A, 0x52, 0xE0, 0x00, 0x00, 0x3A, 0x10, 
	0x83, 0x80, 0x01, 0x09, 0xD2, 0x94, 0x9C, 0x00, 0x00, 0x0C, 
	0x97, 0x20, 0xE0, 0x00, 0xC8, 0x47, 0x10, 0x84, 0x00, 0x00, 
	0x03, 0x25, 0x29, 0x38, 0x4C, 0x84, 0x39, 0x29, 0x4A, 0x40, 
	0x01, 0x00, 0x42, 0x10, 0x84, 0x00, 0x08, 0x02, 0x10, 0x84, 
	0x21, 0x11, 0x08, 0x4A, 0x98, 0xA4, 0x80, 0x02, 0x10, 0x84, 
	0x21, 0x04, 0x00, 0x00, 0x1A, 0xAD, 0x6B, 0x50, 0x00, 0x00, 
	0xE4, 0xA5, 0x29, 0x00, 0x00, 0x03, 0x25, 0x29, 0x30, 0x00, 
	0x00, 0x39, 0x29, 0x4B, 0x90, 0x80, 0x00, 0xE9, 0x4A, 0x4E, 
	0x10, 0x80, 0x0B, 0x62, 0x10, 0x80, 0x00, 0x00, 0x32, 0x0C, 
	0x17, 0x00, 0x04, 0x23, 0x88, 0x42, 0x08, 0x00, 0x00, 0x12, 
	0x94, 0xA4, 0xE0, 0x00, 0x00, 0x94, 0xA5, 0x44, 0x00, 0x00, 
	0x04, 0x63, 0x5A, 0xA8, 0x00, 0x00, 0x22, 0xA2, 0x2A, 0x20, 
	0x00, 0x01, 0x29, 0x4A, 0x4E, 0x13, 0x00, 0x0F, 0x11, 0x10, 
	0xF0, 0x00, 0x22, 0x11, 0x04, 0x21, 0x04, 0x02, 0x10, 0x84, 
	0x21, 0x08, 0x00, 0x20, 0x84, 0x11, 0x08, 0x44, 0x01, 0x54, 
	0x00, 0x00, 0x00, 0x00, 0x00
}; /* 535 bytes */

GLCD::GLCD()
{
//Shield   Std      -> WhitestarBalloon
// _DC    Digital 4 -> A3
// _CS    Digital 2 -> A1
// _SDA   Digital 6 -> A5
// _RESET Digital 3 -> A2
// _CLK   Digital 5 -> A4
#if defined(__WHITESTARBALLOON__)
	_DCpin=3;	    //A3 PORT PF3
	_CSpin=1;		//A1 PORT PF1 
	_SDApin=5;		//A5 PORT PF5
	_RESETpin=2;	//A2 PORT PF2
	_CLKpin=4;		//A4 PORT PF4

	pinMode(A1, OUTPUT);
	pinMode(A2, OUTPUT);
	pinMode(A3, OUTPUT);
	pinMode(A4, OUTPUT);
	pinMode(A5, OUTPUT);

#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	_DCpin=5;		//PORTG  
	_CSpin=4;		//PORTE
	_SDApin=3;		//PORTH
	_RESETpin=5;	//PORTE
	_CLKpin=3;		//PORTE

	pinMode(2, OUTPUT);
	pinMode(3, OUTPUT);
	pinMode(4, OUTPUT);
	pinMode(5, OUTPUT);
	pinMode(6, OUTPUT);
#else
	_DCpin=4;
	_CSpin=2;
	_SDApin=6;
	_RESETpin=3;
	_CLKpin=5;

	pinMode(_DCpin, OUTPUT);
	pinMode(_CSpin, OUTPUT);
	pinMode(_SDApin, OUTPUT);
	pinMode(_RESETpin, OUTPUT);
	pinMode(_CLKpin, OUTPUT);
#endif
}

void GLCD::initLCD()
{
#if defined(__WHITESTARBALLOON__)
	cbi(PORTF, _CSpin);
	cbi(PORTF, _SDApin);
	sbi(PORTF, _CLKpin);

	sbi(PORTF, _RESETpin);
	cbi(PORTF, _RESETpin);
	sbi(PORTF, _RESETpin);

	sbi(PORTF, _CLKpin);
	sbi(PORTF, _SDApin);
	sbi(PORTF, _CLKpin);
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	cbi(PORTE, _CSpin);
	cbi(PORTH, _SDApin);
	sbi(PORTE, _CLKpin);

	sbi(PORTE, _RESETpin);
	cbi(PORTE, _RESETpin);
	sbi(PORTE, _RESETpin);

	sbi(PORTE, _CLKpin);
	sbi(PORTH, _SDApin);
	sbi(PORTE, _CLKpin);
#else
	cbi(PORTD, _CSpin);
	cbi(PORTD, _SDApin);
	sbi(PORTD, _CLKpin);

	sbi(PORTD, _RESETpin);
	cbi(PORTD, _RESETpin);
	sbi(PORTD, _RESETpin);

	sbi(PORTD, _CLKpin);
	sbi(PORTD, _SDApin);
	sbi(PORTD, _CLKpin);
#endif

	delay(10);

//Software Reset
	sendCMD(0x01);

// Write Contrast
	sendCMD(0x25);
	sendData(64);

//Sleep Out and booster on
	sendCMD(0x11);

	delay(10);

// Display Inversion off
	sendCMD(0x20);

// Idle Mode off
	sendCMD(0x38);

// Display on
	sendCMD(0x29);

// Normal Mode on
	sendCMD(0x13);

// Memory Data Access control
	sendCMD(0x36);
	sendData(0x60);

//16-Bit per Pixel
	sendCMD(0x3A);
	sendData(5);

// X_Address or Column Address Area
	sendCMD(0x2A);
	sendData(0);
	sendData(127);

// Frame Frequency Select
	sendCMD(0xB4);
	sendData(0x03);
	sendData(0x08);
	sendData(0x0b);
	sendData(0x0e);

// Display Control
	sendCMD(0xBA);
	sendData(0x07);
	sendData(0x0D);

//Page Adress Set
	sendCMD(0x2B);
	sendData(0);
	sendData(127);

	sendCMD(0x2C);
}

void GLCD::lcdOff()
{
	sendCMD(0x28);
}

void GLCD::lcdOn()
{
	sendCMD(0x29);
}

void GLCD::setContrast(byte c)
{
	if (c>64)
		c=64;

	sendCMD(0x25);
	sendData(c);
}

void GLCD::clrScr()
{
	int i;
	setXY(0,0,128,128);
	for (i=0; i<16384; i++)
	{
		setPixel (0, 0, 0);
	}
}

void GLCD::fillScr(byte r, byte g, byte b)
{
	int i;
	setXY(0,0,128,128);
	for (i=0; i<16384; i++)
	{
		setPixel (r, g, b);
	}
}

void GLCD::setColor(byte r, byte g, byte b)
{
	colorr=r;
	colorg=g;
	colorb=b;
}

void GLCD::sendCMD(byte data)
{
  shiftBits(data, 0);
}

void GLCD::sendData(byte data)
{
  shiftBits(data, 1);
}

void GLCD::setPixel(byte r,byte g,byte b)
{
   sendData((r&248)|g>>5);
   sendData((g&7)<<5|b>>3);
}

void GLCD::shiftBits(byte b, int dc)
{
#if defined(__WHITESTARBALLOON__)
  cbi(PORTF, _CLKpin);
  if ((b&128)!=0)
    sbi(PORTF, _SDApin);
  else
    cbi(PORTF, _SDApin);
  sbi(PORTF, _CLKpin);
  
  cbi(PORTF, _CLKpin);
  if ((b&64)!=0)
    sbi(PORTF, _SDApin);
  else
    cbi(PORTF, _SDApin);
  sbi(PORTF, _CLKpin);
  
  cbi(PORTF, _CLKpin);
  if ((b&32)!=0)
    sbi(PORTF, _SDApin);
  else
    cbi(PORTF, _SDApin);
  sbi(PORTF, _CLKpin);
  
  cbi(PORTF, _CLKpin);
  if ((b&16)!=0)
    sbi(PORTF, _SDApin);
  else
    cbi(PORTF, _SDApin);
  sbi(PORTF, _CLKpin);
  
  cbi(PORTF, _CLKpin);
  if ((b&8)!=0)
    sbi(PORTF, _SDApin);
  else
    cbi(PORTF, _SDApin);
  sbi(PORTF, _CLKpin);
  
  cbi(PORTF, _CLKpin);
  if ((b&4)!=0)
    sbi(PORTF, _SDApin);
  else
    cbi(PORTF, _SDApin);
  sbi(PORTF, _CLKpin);
  
  cbi(PORTF, _CLKpin);
  if ((b&2)!=0)
    sbi(PORTF, _SDApin);
  else
    cbi(PORTF, _SDApin);
  sbi(PORTF, _CLKpin);
  
  cbi(PORTF, _CLKpin);
  if ((b&1)!=0)
    sbi(PORTF, _SDApin);
  else
    cbi(PORTF, _SDApin);
  if (dc == 1)
    sbi(PORTF, _DCpin);
  else        
    cbi(PORTF, _DCpin);
  sbi(PORTF, _CLKpin);

#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  cbi(PORTE, _CLKpin);
  if ((b&128)!=0)
    sbi(PORTH, _SDApin);
  else
    cbi(PORTH, _SDApin);
  sbi(PORTE, _CLKpin);
  
  cbi(PORTE, _CLKpin);
  if ((b&64)!=0)
    sbi(PORTH, _SDApin);
  else
    cbi(PORTH, _SDApin);
  sbi(PORTE, _CLKpin);
  
  cbi(PORTE, _CLKpin);
  if ((b&32)!=0)
    sbi(PORTH, _SDApin);
  else
    cbi(PORTH, _SDApin);
  sbi(PORTE, _CLKpin);
  
  cbi(PORTE, _CLKpin);
  if ((b&16)!=0)
    sbi(PORTH, _SDApin);
  else
    cbi(PORTH, _SDApin);
  sbi(PORTE, _CLKpin);
  
  cbi(PORTE, _CLKpin);
  if ((b&8)!=0)
    sbi(PORTH, _SDApin);
  else
    cbi(PORTH, _SDApin);
  sbi(PORTE, _CLKpin);
  
  cbi(PORTE, _CLKpin);
  if ((b&4)!=0)
    sbi(PORTH, _SDApin);
  else
    cbi(PORTH, _SDApin);
  sbi(PORTE, _CLKpin);
  
  cbi(PORTE, _CLKpin);
  if ((b&2)!=0)
    sbi(PORTH, _SDApin);
  else
    cbi(PORTH, _SDApin);
  sbi(PORTE, _CLKpin);
  
  cbi(PORTE, _CLKpin);
  if ((b&1)!=0)
    sbi(PORTH, _SDApin);
  else
    cbi(PORTH, _SDApin);
  if (dc == 1)
    sbi(PORTG, _DCpin);
  else        
    cbi(PORTG, _DCpin);
  sbi(PORTE, _CLKpin);

#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  cbi(PORTE, _CLKpin);
  if ((b&128)!=0)
    sbi(PORTH, _SDApin);
  else
    cbi(PORTH, _SDApin);
  sbi(PORTE, _CLKpin);
  
  cbi(PORTE, _CLKpin);
  if ((b&64)!=0)
    sbi(PORTH, _SDApin);
  else
    cbi(PORTH, _SDApin);
  sbi(PORTE, _CLKpin);
  
  cbi(PORTE, _CLKpin);
  if ((b&32)!=0)
    sbi(PORTH, _SDApin);
  else
    cbi(PORTH, _SDApin);
  sbi(PORTE, _CLKpin);
  
  cbi(PORTE, _CLKpin);
  if ((b&16)!=0)
    sbi(PORTH, _SDApin);
  else
    cbi(PORTH, _SDApin);
  sbi(PORTE, _CLKpin);
  
  cbi(PORTE, _CLKpin);
  if ((b&8)!=0)
    sbi(PORTH, _SDApin);
  else
    cbi(PORTH, _SDApin);
  sbi(PORTE, _CLKpin);
  
  cbi(PORTE, _CLKpin);
  if ((b&4)!=0)
    sbi(PORTH, _SDApin);
  else
    cbi(PORTH, _SDApin);
  sbi(PORTE, _CLKpin);
  
  cbi(PORTE, _CLKpin);
  if ((b&2)!=0)
    sbi(PORTH, _SDApin);
  else
    cbi(PORTH, _SDApin);
  sbi(PORTE, _CLKpin);
  
  cbi(PORTE, _CLKpin);
  if ((b&1)!=0)
    sbi(PORTH, _SDApin);
  else
    cbi(PORTH, _SDApin);
  if (dc == 1)
    sbi(PORTG, _DCpin);
  else        
    cbi(PORTG, _DCpin);
  sbi(PORTE, _CLKpin);
#else
  cbi(PORTD, _CLKpin);
  if ((b&128)!=0)
    sbi(PORTD, _SDApin);
  else
    cbi(PORTD, _SDApin);
  sbi(PORTD, _CLKpin);
  
  cbi(PORTD, _CLKpin);
  if ((b&64)!=0)
    sbi(PORTD, _SDApin);
  else
    cbi(PORTD, _SDApin);
  sbi(PORTD, _CLKpin);
  
  cbi(PORTD, _CLKpin);
  if ((b&32)!=0)
    sbi(PORTD, _SDApin);
  else
    cbi(PORTD, _SDApin);
  sbi(PORTD, _CLKpin);
  
  cbi(PORTD, _CLKpin);
  if ((b&16)!=0)
    sbi(PORTD, _SDApin);
  else
    cbi(PORTD, _SDApin);
  sbi(PORTD, _CLKpin);
  
  cbi(PORTD, _CLKpin);
  if ((b&8)!=0)
    sbi(PORTD, _SDApin);
  else
    cbi(PORTD, _SDApin);
  sbi(PORTD, _CLKpin);
  
  cbi(PORTD, _CLKpin);
  if ((b&4)!=0)
    sbi(PORTD, _SDApin);
  else
    cbi(PORTD, _SDApin);
  sbi(PORTD, _CLKpin);
  
  cbi(PORTD, _CLKpin);
  if ((b&2)!=0)
    sbi(PORTD, _SDApin);
  else
    cbi(PORTD, _SDApin);
  sbi(PORTD, _CLKpin);
  
  cbi(PORTD, _CLKpin);
  if ((b&1)!=0)
    sbi(PORTD, _SDApin);
  else
    cbi(PORTD, _SDApin);
  if (dc == 1)
    sbi(PORTD, _DCpin);
  else        
    cbi(PORTD, _DCpin);
  sbi(PORTD, _CLKpin);
#endif
}

void GLCD::setXY(byte x, byte y, byte dx, byte dy)
{
  sendCMD(0x2A);
  sendData(x);
  sendData(x+dx-1);

  sendCMD(0x2B);
  sendData(y);
  sendData(y+dy-1);

  sendCMD(0x2C);
}

void GLCD::print(char *st, byte x, byte y, byte r, byte g, byte b)
{
  int stl, i;
  
  stl = strlen(st);
  if (x==RIGHT)
	  x=128-(stl*6);
  if (x==CENTER)
	  x=(128-(stl*6))/2;
  
  for (i=0; i<stl; i++)
    printChar(*st++, x + (i*6), y, r, g, b);
}

void GLCD::printChar(byte c, byte x, byte y, byte r, byte g, byte b)
{
  byte bitmask, *pf, bitstart = 128;
  int cindex, iFont, count;
  
  setXY(x, y, 5, 9);
  
  iFont = (c - ' ') * 45 / 8;

  bitstart = 128 >> (((c - ' ') * 45) % 8);
  
  pf = &Font5x9Mono[iFont];
  
  count = 0;
  
  for (cindex=0; cindex<7; cindex++)
  {
    for (bitmask=bitstart; bitmask>0; bitmask=bitmask>>1)
	{
       if (count < 45)
	   {
		   if (*pf&bitmask)
			 setPixel(colorr, colorg, colorb);
		   else
			 setPixel(r, g, b); 
		   count++;
	   }
    }
    bitstart = 128;
    pf++;
  }
  setXY(x+5, y, 1, 9);
  for (cindex=0; cindex<9; cindex++)
	setPixel(r, g, b);
}

void GLCD::drawPixel(byte x, byte y)
{
	setXY(x, y, x, y);
	setPixel(colorr, colorg, colorb);
	setXY(0,0,128,128);
}

void GLCD::drawLine(byte x1, byte y1, byte x2, byte y2)
{
	int tmp;
	double delta, tx, ty;
	double m, b, dx, dy;

        if (((x2-x1)<0))
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
		tmp=y1;
		y1=y2;
		y2=tmp;
	}
        if (((y2-y1)<0))
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
		tmp=y1;
		y1=y2;
		y2=tmp;
	}

	if (y1==y2)
	{
		if (x1>x2)
		{
			tmp=x1;
			x1=x2;
			x2=tmp;
		}
		drawHLine(x1, y1, x2-x1);
	}
	else if (x1==x2)
	{
		if (y1>y2)
		{
			tmp=y1;
			y1=y2;
			y2=tmp;
		}
		drawVLine(x1, y1, y2-y1);
	}
	else if (abs(x2-x1)>abs(y2-y1))
	{
		delta=(double(y2-y1)/double(x2-x1));
		ty=double(y1);
		if (x1>x2)
                {
                        for (int i=x1; i>=x2; i--)
        	  	{
		        	drawPixel(i,int(ty+0.5));
        			ty=ty-delta;
	        	}
                }
                else
                {
                        for (int i=x1; i<=x2; i++)
        	  	{
		        	drawPixel(i,int(ty+0.5));
        			ty=ty+delta;
	        	}
                }
	}
	else
	{
		delta=(float(x2-x1)/float(y2-y1));
		tx=float(x1);
                if (y1>y2)
                {
        		for (int i=y2+1; i>y1; i--)
        		{
    	        		drawPixel(int(tx+0.5), i);
    		        	tx=tx+delta;
        		}
                }
                else
                {
        		for (int i=y1; i<y2+1; i++)
        		{
    	        		drawPixel(int(tx+0.5), i);
    		        	tx=tx+delta;
        		}
                }
	}

	setXY(0,0,128,128);
}

void GLCD::drawRect(byte x1, byte y1, byte x2, byte y2)
{
	int tmp;

	if (x1>x2)
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
	}
	if (y1>y2)
	{
		tmp=y1;
		y1=y2;
		y2=tmp;
	}

	drawHLine(x1, y1, x2-x1);
	drawHLine(x1, y2, x2-x1);
	drawVLine(x1, y1, y2-y1);
	drawVLine(x2, y1, y2-y1);
}

void GLCD::drawRoundRect(byte x1, byte y1, byte x2, byte y2)
{
	int tmp;

	if (x1>x2)
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
	}
	if (y1>y2)
	{
		tmp=y1;
		y1=y2;
		y2=tmp;
	}
	if ((x2-x1)>4 && (y2-y1)>4)
	{
		drawPixel(x1+1,y1+1);
		drawPixel(x2-1,y1+1);
		drawPixel(x1+1,y2-1);
		drawPixel(x2-1,y2-1);
		drawHLine(x1+2, y1, x2-x1-4);
		drawHLine(x1+2, y2, x2-x1-4);
		drawVLine(x1, y1+2, y2-y1-4);
		drawVLine(x2, y1+2, y2-y1-4);
	}
}

void GLCD::fillRect(byte x1, byte y1, byte x2, byte y2)
{
	int tmp;

	if (x1>x2)
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
	}
	if (y1>y2)
	{
		tmp=y1;
		y1=y2;
		y2=tmp;
	}

	for (int i=0; i<((y2-y1)/2)+1; i++)
	{
		drawHLine(x1, y1+i, x2-x1);
		drawHLine(x1, y2-i, x2-x1);
	}
}

void GLCD::fillRoundRect(byte x1, byte y1, byte x2, byte y2)
{
	int tmp;

	if (x1>x2)
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
	}
	if (y1>y2)
	{
		tmp=y1;
		y1=y2;
		y2=tmp;
	}

	if ((x2-x1)>4 && (y2-y1)>4)
	{
		for (int i=0; i<((y2-y1)/2)+1; i++)
		{
			switch(i)
			{
			case 0:
				drawHLine(x1+2, y1+i, x2-x1-4);
				drawHLine(x1+2, y2-i, x2-x1-4);
				break;
			case 1:
				drawHLine(x1+1, y1+i, x2-x1-2);
				drawHLine(x1+1, y2-i, x2-x1-2);
				break;
			default:
				drawHLine(x1, y1+i, x2-x1);
				drawHLine(x1, y2-i, x2-x1);
			}
		}
	}
}

void GLCD::drawCircle(int x, int y, int radius)
{
  int f = 1 - radius;
  int ddF_x = 1;
  int ddF_y = -2 * radius;
  int x1 = 0;
  int y1 = radius;
 
  drawPixel(x, y + radius);
  drawPixel(x, y - radius);
  drawPixel(x + radius, y);
  drawPixel(x - radius, y);
 
  while(x1 < y1)
  {
    if(f >= 0) 
    {
      y1--;
      ddF_y += 2;
      f += ddF_y;
    }
    x1++;
    ddF_x += 2;
    f += ddF_x;    
    drawPixel(x + x1, y + y1);
    drawPixel(x - x1, y + y1);
    drawPixel(x + x1, y - y1);
    drawPixel(x - x1, y - y1);
    drawPixel(x + y1, y + x1);
    drawPixel(x - y1, y + x1);
    drawPixel(x + y1, y - x1);
    drawPixel(x - y1, y - x1);
  }
}

void GLCD::fillCircle(int x, int y, int radius)
{
	for(int y1=-radius; y1<=radius; y1++) 
		for(int x1=-radius; x1<=radius; x1++) 
			if(x1*x1+y1*y1 <= radius*radius) 
				drawPixel(x+x1, y+y1); 
}

void GLCD::drawHLine(byte x, byte y, byte l)
{
	setXY(x, y, x+l+1, y);
	for (int i=0; i<l+1; i++)
		setPixel(colorr, colorg, colorb);
}

void GLCD::drawVLine(byte x, byte y, byte l)
{
	for (int i=0; i<l+1; i++)
	{
		setXY(x, y+i, x, y+i);
		setPixel(colorr, colorg, colorb);
	}
}

void GLCD::printNumI(long num, byte x, byte y, byte r, byte g, byte b)
{
  char buf[25];
  char st[27];
  boolean neg=false;
  int c=0;
  
  if (num==0)
  {
	  st[0]=48;
	  st[1]=0;
  }
  else
  {
	  if (num<0)
	  {
		neg=true;
		num=-num;
	  }
	  
	  while (num>0)
	  {
		buf[c]=48+(num % 10);
		c++;
		num=(num-(num % 10))/10;
	  }
	  buf[c]=0;
	  
	  if (neg)
	  {
		st[0]=45;
	  }
	  
	  for (int i=0; i<c; i++)
	  {
		st[i+neg]=buf[c-i-1];
	  }
	  st[c+neg]=0;
  }
  
  print(st,x,y,r,g,b);
}

void GLCD::printNumF(double num, byte dec, byte x, byte y, byte r, byte g, byte b)
{
  char buf[25];
  char st[27];
  boolean neg=false;
  int c=0;
  int c2;
  unsigned long inum;
  
  if (num==0)
  {
	  st[0]=48;
	  st[1]=46;
	  for (int i=0; i<dec; i++)
		  st[2+i]=48;
	  st[2+dec]=0;
  }
  else
  {
	  if (num<0)
	  {
		neg=true;
		num=-num;
	  }
	  
	  if (dec<1)
		dec=1;
	  if (dec>5)
		dec=5;
	  
	  inum=long(num*pow(10,dec));
	  
	  while (inum>0)
	  {
		buf[c]=48+(inum % 10);
		c++;
		inum=(inum-(inum % 10))/10;
	  }
	  if ((num<1) and (num>0))
	  {
		  buf[c]=48;
		  c++;
	  }
	  buf[c]=0;
	  
	  if (neg)
	  {
		st[0]=45;
	  }
	  
	  c2=neg;
	  for (int i=0; i<c; i++)
	  {
		st[c2]=buf[c-i-1];
		c2++;
		if ((c-(c2-neg))==dec)
		{
		  st[c2]=46;
		  c2++;
		}
	  }
	  st[c2]=0;
  }
  
  print(st,x,y,r,g,b);
}

void GLCD::drawBitmap(int x, int y, int sx, int sy, unsigned int* data, int scale)
{
	unsigned int col;
	int tx, ty, tc, tsx, tsy;
	byte r, g, b;

	if (scale==1)
	{
		setXY(x, y, sx, sy);
		for (tc=0; tc<(sx*sy); tc++)
		{
			col=pgm_read_word(&data[tc]);
			sendData(col>>8);
			sendData(col & 0xff);
		}
	}
	else
	{
		for (ty=0; ty<sy; ty++)
		{
			setXY(x, y+(ty*scale), ((sx*scale)), (ty*scale)+scale);
			for (tsy=0; tsy<scale; tsy++)
				for (tx=0; tx<sx; tx++)
				{
					col=pgm_read_word(&data[(ty*sx)+tx]);
					for (tsx=0; tsx<scale; tsx++)
					{
						sendData(col>>8);
						sendData(col & 0xff);
					}
				}
		}
	}
}

void GLCD::drawBitmap(int x, int y, int sx, int sy, unsigned int* data, int deg, int rox, int roy)
{
	unsigned int col;
	int tx, ty, newx, newy;
	byte r, g, b;
	double radian;
	radian=deg*0.0175;  

	if (deg==0)
		drawBitmap(x, y, sx, sy, data);
	else
	{
		for (ty=0; ty<sy; ty++)
			for (tx=0; tx<sx; tx++)
			{
				col=pgm_read_word(&data[(ty*sx)+tx]);
				r=(col & 0xF800)>>8;
				g=(((col & 0x7e0)>>5)<<2);
				b=(col & 0x1F)<<3;

				setColor(r,g,b);

				newx=x+rox+(((tx-rox)*cos(radian))-((ty-roy)*sin(radian)));
				newy=y+roy+(((ty-roy)*cos(radian))+((tx-rox)*sin(radian)));

				drawPixel(newx, newy);
			}
	}
}

