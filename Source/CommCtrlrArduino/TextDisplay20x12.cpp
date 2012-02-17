
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "RGB_GLCD.h"
#include <avr/pgmspace.h>
#include <string.h>

#include "TextDisplay20x12.h"

/*Display20x12::TextDisplay20x12()
{
}
*/
void TextDisplay20x12::initDisplay()
{
	myGLCD.lcdOn();
	myGLCD.initLCD();
	myGLCD.clrScr();
	_bDisplayed=true;
	myGLCD.setContrast(_contrast);
	
	_numLines = 12;
	_yOffset = 3; // Frame takes 2 lines
	if (strlen(&_szTitle[0]) > 0)
	{
		_numLines--;
		_yOffset += 10;
	}
	if (strlen(&_szStatusL[0]) > 0 || strlen(&_szStatusR[0]) > 0 )
	{
		_numLines--;
	}
	displayTitle();
	displayStatus();
	displayFrame();
}

void TextDisplay20x12::setTitle(String s)
{
	_szTitle = s;
	if (strlen(&s[0]) > 0 && _bDisplayed ) displayTitle();
}


void TextDisplay20x12::displayTitle()
{
	if (strlen(&_szTitle[0]) > 0)
	{
			myGLCD.setColor(_bgTitleR,_bgTitleG,_bgTitleB);
			myGLCD.fillRect(0,0,127,9);
			myGLCD.setColor(_cTitleR,_cTitleG,_cTitleB); //Title
			myGLCD.print(&_szTitle[0], CENTER, 1, _bgTitleR,_bgTitleG,_bgTitleB);
	}
}

void TextDisplay20x12::setStatusLeft(String s)
{
	_szStatusL = s;
	if (strlen(&s[0]) > 0 && _bDisplayed ) displayStatus();
}

void TextDisplay20x12::setStatusRight(String s)
{
	_szStatusR = s;
	if (strlen(&s[0]) > 0 && _bDisplayed ) displayStatus();
}


void TextDisplay20x12::displayStatus()
{
	if (strlen(&_szStatusL[0]) > 0 || strlen(&_szStatusR[0]) > 0 )
	{
			myGLCD.setColor(_bgStatusR,_bgStatusG,_bgStatusB);
			myGLCD.fillRect(0,117,127,127);
			myGLCD.setColor(_cStatusR,_cStatusG,_cStatusB);
			myGLCD.print(&_szStatusL[0], LEFT, 119,_bgStatusR,_bgStatusG,_bgStatusB);
			myGLCD.print(&_szStatusR[0], RIGHT, 119,_bgStatusR,_bgStatusG,_bgStatusB);
	}
}

void TextDisplay20x12::displayFrame()
{
	myGLCD.setColor(_cFrameR,_cFrameG,_cFrameB);
	myGLCD.drawRect(0,10,127,116);
}

void TextDisplay20x12::lcdOff()
{
	myGLCD.lcdOff();
}

void TextDisplay20x12::lcdOn()
{
	myGLCD.lcdOn();
}

void TextDisplay20x12::clrScr()
{
	myGLCD.clrScr();
}

void TextDisplay20x12::gotoXY(int x, int y)
{
	_tx = x;
	_ty = y;
	if (_tx > 20) _tx = 0;
	if (_ty >= _numLines) _ty = 0;
}

void TextDisplay20x12::newLine()
{
  _tx = 0;
  _ty++;
  if (_ty >= _numLines) _ty = 0;
}

void TextDisplay20x12::displayChar(char ch)
{
	String str = " ";
	str[0] = ch;
	if (_tx == 0) clearLine(_ty);
	int x = _tx * 6 + _xOffset;
	int y = _ty * 10 + _yOffset;
	myGLCD.setColor(_cR,_cG, _cB);
	myGLCD.print(&str[0],x, y, _bgR, _bgG, _bgB);
	_tx++;
	if (_tx >= 20) { newLine(); }
}

void TextDisplay20x12::displayStr(String s)
{
	int i = 0;
	while ( s[i] != 0 ) displayChar(s[i++]);
}

void TextDisplay20x12::clearLine(int y)
{
	y = y * 10 + _yOffset;
	myGLCD.setColor(_bgR,_bgG, _bgB);
	myGLCD.fillRect(_xOffset,y, _xOffset+(20*6)-1, y+10 );
}
