// Glx_SWindows.h


#ifndef _GLX_SWINDOWS_h
#define _GLX_SWINDOWS_h
#define tft_t Adafruit_ILI9341
#define WIN_TEXT_SCROLL
#define WIN_GRAPH
#define WIN_BUTTONS
#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <Adafruit_ILI9341esp.h>
#include <Adafruit_GFX.h>
#include <XPT2046.h>
// For the esp shield, these are the default.
#define TFT_DC 2
#define TFT_CS 15
extern Adafruit_ILI9341 tft; //= Adafruit_ILI9341(TFT_CS, TFT_DC);
extern XPT2046 touch;//(/*cs=*/ 16, /*irq=*/ 0);

class Glx_GWindowsClass 
{
public:
	static  int winXmax, winXmin, winYmax, winYmin;

	void init(byte type,int x0,int y0,int x1,int y1);//screen pixels
	
//	void scroll(byte x,byte y);

//	boolean redraw( Graf* _g);
	//void clear(byte color);
	//void setBackground(byte color);
};
class Graf {
public:
	Graf(float * xp, float*yp, int nval);
	int nval;
	float x[100], y[100];
	float scax, scay;
	int x0, y0;
	
	byte color;
	boolean draw();
	boolean drawAxX(float y_pos,float deltx);
	boolean drawAxY(float x_pos);

};
struct Menu {
	
	Adafruit_GFX_Button button[10];
	void init();
	byte nbutton;
	String menuName[10];
	void draw();                     //draw current menu all buttons
	byte  menuIndex[10];


};
class Glx_keyborad {
public:
	Adafruit_GFX_Button button[47];
	void init(int x0,int y0, byte Uppercase);
	String line[4][2];
	char* LastLine[6];
	byte CaseStatus;
	char isPressed(uint16_t x,uint16_t y);
	void end();


};
class Glx_MWindowsClass {
public:

	
	void init( int x0, int y0, int x1, int y1);//screen pixels
	byte nmenu;
	Menu menu[20];
	int getPressed(uint16_t x,uint16_t y);
	
	

};
extern Glx_MWindowsClass Glx_MWindows;
static  int Xmax, Xmin, Ymax, Ymin;
static byte curr_m;
static int i_sel = 0;
#endif

