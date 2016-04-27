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
#define ILI9341_VSCRDEF 0x33
#define ILI9341_VSCRSADD 0x37

extern Adafruit_ILI9341 tft; //= Adafruit_ILI9341(TFT_CS, TFT_DC);
extern XPT2046 touch;//(/*cs=*/ 16, /*irq=*/ 0);
static int ScreenH;
static  int winXmax, winXmin, winYmax, winYmin, backgroundColor;
static float xmax, ymax, xmin, ymin;
class Glx_GWindowsClass
{


public:
	void init(byte type, int x0, int y0, int x1, int y1, uint16_t backColor);//screen pixels

};
	class Graf {
	public:
		
		//static int Xmax = winXmax; static int Xmin = winXmin; static int Ymax = winYmax;
		void init( uint16_t col);
		int nval;
		float x[100], y[100];
		float scax, scay;
		float x0, y0;
		int windowsH = 100, windowsW = 240, yGrafScr = 50;
		uint16_t color;
		boolean draw();
		boolean drawAxX(float y_pos, float deltx);
		void changeScaX(float dx);
		void scroll(float dx);
		boolean drawAxY(int x_scr_pos);

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


class Glx_TWindows :public Print {
public:
	void init(int x0, int y0, int x1, int y1,byte textH, byte mode);
	virtual size_t write(uint8_t);
protected:
	int yDraw, xPos, yStart, TOP_FIXED_AREA, BOT_FIXED_AREA, TEXT_HEIGHT;
	char blank[50];
	void scrollAddress(uint16_t _vsp);
	void setupScrollArea(uint16_t tfa, uint16_t bfa);
	int scroll_line();
};
#endif