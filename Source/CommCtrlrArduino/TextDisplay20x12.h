#ifndef TextDisplay20x10_h
#define TextDisplay20x10_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "RGB_GLCD.h"
#include <string.h>

class TextDisplay20x12
{

public:
	TextDisplay20x12() : _tx(-1),_ty(0),_xOffset(4),_yOffset(3), _numLines(12), 
						 _width(128), _height(128),
						 _szTitle(""),_szStatusL(""), _szStatusR(""), _bDisplayed(false),
						 _cTitleR(0),_cTitleG(0),_cTitleB(0),_bgTitleR(0),_bgTitleG(0),_bgTitleB(255),
						 _cStatusR(255),_cStatusB(128),_cStatusG(128),_bgStatusR(64),_bgStatusG(64),_bgStatusB(64),
						 _cFrameR(0),_cFrameG(255),_cFrameB(0),
						 _cR(255),_cG(255),_cB(255), _bgR(0), _bgG(0),_bgB(0),
						 _contrast(64) {};
	void initDisplay();
	void lcdOff();
	void lcdOn();
	void clrScr();
	void setTitle(String s);
	void displayTitle();
	void setStatusLeft(String s);
	void setStatusRight(String s);
	void displayStatus();
	void displayFrame();
	void gotoXY(int x, int y);
	void newLine();
	void displayChar(char ch);
	void displayStr(String s);
	void clearLine(int y);
	
/*	
	void newline();
	void clearline(int y);
	void clearscreen();
	void setXY(int x, int y);
	void setTitle(String s);
	void setStatusLeft(String s);
	void setStatusRight(String s);
*/	
private:
	int _tx;
	int _ty;
	int _xOffset;
	int _yOffset;
	int _numLines;
	int _width;
	int _height;
	String _szTitle;
	String _szStatusL;
	String _szStatusR;
	bool _bDisplayed;
	int _cTitleR;	int _cTitleB;	int _cTitleG;
	int _bgTitleR;	int _bgTitleB;	int _bgTitleG;
	int _cStatusR;	int _cStatusB;	int _cStatusG;
	int _bgStatusR;	int _bgStatusB;	int _bgStatusG; 
	int _cFrameR; 	int _cFrameB; 	int _cFrameG;
	int _contrast;
	int _cR; int _cG; int _cB;
	int _bgR; int _bgG; int _bgB;
	
	GLCD myGLCD;
};

#endif
