// Glx_SWindows.h


#ifndef _GLX_SWINDOWS_h
#define _GLX_SWINDOWS_h
#define tft_t Adafruit_ILI9341
#define WIN_TEXT_SCROLL
#define WIN_GRAPH
#define WIN_BUTTONS

#define STL_VECTOR
#define PLOT_VECTOR_SIZE 40
#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <Adafruit_ILI9341esp.h>
#include <Adafruit_GFX.h>
#include <XPT2046.h>
#include <vector>
// For the esp shield, these are the default.
#define TFT_DC 2
#define TFT_CS 15
#define ILI9341_VSCRDEF 0x33
#define ILI9341_VSCRSADD 0x37
#define TIME_SCA 1
#define TICKSIZEL 3
#define  xMargin 2
#define yMargin 2
#define YSign 1
#define LegYmargin 0
#define LegXmargin 4
extern Adafruit_ILI9341 tft; //= Adafruit_ILI9341(TFT_CS, TFT_DC);
extern XPT2046 touch;//(/*cs=*/ 16, /*irq=*/ 0);
static int ScreenH;
static  int winXmax, winXmin, winYmax, winYmin, backgroundColor;// xMargin, yMargin;
static float xmax, ymax, xmin, ymin;
static float scax, scay;

class Glx_GWindowsClass
{


public:
	void init(byte type, int x0, int y0, int x1, int y1, uint16_t backColor);//screen pixels
	int xpressed(int x,int y);
	int ypressed(int x,int y);

};

	class Graf {
	public:
		
		//static int Xmax = winXmax; static int Xmin = winXmin; static int Ymax = winYmax;
		void init( uint16_t col);
		int nval;
#ifdef STL_VECTOR
		std::vector<int> x;
		std::vector<int> y;
#else
		int  x[PLOT_VECTOR_SIZE], y[PLOT_VECTOR_SIZE];
#endif
//		float scax, scay;
		float x0, y0;
		int windowsH = 100, windowsW = 240, yGrafScr = 50;
		uint16_t color;
		boolean draw();
		boolean drawAxX(float y_pos, float deltx);
		void changeScaX(float dx);
		void scroll(float dx);
		boolean drawAxX(float y_pos, float deltx,byte ty);
		boolean drawAxy(int x_pos,float delty,byte ty);

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
	void charPos(byte nline, byte nchar,byte mode);
	void init(int x0, int y0, int x1, int y1,byte textH, byte mode);
	virtual size_t write(uint8_t);
	void throttle(int xpos, int ypos);
	void textColor(int color);

protected:
    int yS,yDraw, xPos, yStart, TOP_FIXED_AREA, BOT_FIXED_AREA, TEXT_HEIGHT;
	byte index;
	char blank[100];
	bool Scroll;
	void scrollAddress(uint16_t _vsp);
	void setupScrollArea(uint16_t tfa, uint16_t bfa);
	int scroll_line();
};
#endif