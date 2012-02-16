/*
  RGB_GLCD.h - Arduino library support for LDS183 graphic LCDs
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

#ifndef RGB_GLCD_h
#define RGB_GLCD_h

#define LEFT 0
#define RIGHT 255
#define CENTER 254

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif


class GLCD
{
public:
	GLCD();
	void initLCD();
	void lcdOff();
	void lcdOn();
	void setContrast(byte c);
	void clrScr();
	void fillScr(byte r, byte g, byte b);
	void setColor(byte r, byte g, byte b);
	void drawPixel(byte x, byte y);
	void drawLine(byte x1, byte y1, byte x2, byte y2);
	void drawRect(byte x1, byte y1, byte x2, byte y2);
	void drawRoundRect(byte x1, byte y1, byte x2, byte y2);
	void fillRect(byte x1, byte y1, byte x2, byte y2);
	void fillRoundRect(byte x1, byte y1, byte x2, byte y2);
	void drawCircle(int x, int y, int radius);
	void fillCircle(int x, int y, int radius);
	void print(char *st, byte x, byte y, byte r=0, byte g=0, byte b=0);
	void printNumI(long num, byte x, byte y, byte r=0, byte g=0, byte b=0);
	void printNumF(double num, byte dec, byte x, byte y, byte r=0, byte g=0, byte b=0);
    void drawBitmap(int x, int y, int sx, int sy, unsigned int* data, int scale=1);
    void drawBitmap(int x, int y, int sx, int sy, unsigned int* data, int deg, int rox, int roy);

private:
	byte colorr;
	byte colorg;
	byte colorb;
	int _DCpin;
	int _CSpin;
	int _SDApin;
	int _RESETpin;
	int _CLKpin;
	void setPixel(byte r,byte g,byte b);
	void sendData(byte data);
	void sendCMD(byte data);
	void shiftBits(byte b, int dc);
	void printChar(byte c, byte x, byte y, byte r, byte g, byte b);
	void drawHLine(byte x, byte y, byte l);
	void drawVLine(byte x, byte y, byte l);
	void setXY(byte x, byte y, byte dx, byte dy);
};

#endif
