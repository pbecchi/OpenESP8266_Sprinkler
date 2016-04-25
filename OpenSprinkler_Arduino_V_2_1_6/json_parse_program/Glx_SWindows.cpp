// 
// 
// 

#include "Glx_SWindows.h"



void Graf::init( uint16_t col)
{
	//if (ymax == 0 && ymin == 0)
	{
		xmax = x[0]; xmin = x[0]; ymax = y[0]; ymin = y[0];
	}
	color = col;
	for (byte i = 0; i < nval; i++)
	{
		if( x [ i]>xmax)xmax= x[i];
		if (x[i]<xmin)xmin = x[i];
		if (y[i]>ymax)ymax = y[i];
		if (y[i]<ymin)ymin = y[i];
		
	}
	windowsW = winXmax - winXmin;
	
	x0 = xmin;
	y0 = ymin;
	windowsH = winYmax - winYmin;
	scay =  windowsH/(ymax-ymin);
	// serial.println(x0);
	// serial.println(y0);
	// serial.println(scay);

	scax =   windowsW/(xmax - xmin);
	// serial.println(scax);
	// serial.println(xmax);
	// serial.println(xmin);
}

boolean Graf::draw()
{
	

	//tft.drawPixel(int(x[0] * scax) - x0, int(y[0] * scay) - y0, color);
	for (int i = 1; i < nval; i++)
		//if (x[i] >winXmin && x[i] < winXmax )
			tft.drawLine(int((x[i - 1]  - x0)*scax+winXmin), 
						int((y[i - 1] - y0)*scay+winYmin), 
						int((x[i ] - x0)*scax+winXmin),
						int((y[i] - y0)*scay + winYmin),  color);
	return true;
	
}
#define TICKSIZE 2
boolean Graf::drawAxX(float y,float xstep)
{
//	tft.drawPixel(Glx_GWindowsClass::winXmin, y*scay-y0, color);
	tft.drawFastHLine(winXmin,(y- y0)*scay +winYmin,
		winXmax- winXmin, color);
	byte ntics = (xmax - xmin) / xstep;
	int i = 1;
	
	if (xmax - xmin > 1)
		while (ntics > 10) {
			xstep = xstep*i++;
			ntics = (xmax - xmin) / xstep;
			
		}
	   // serial.println(xstep);
	for (byte i = 0; i < ntics; i++) {
		int xp = (i*xstep+xmin-x0)*scax+winXmin;
		tft.drawFastVLine(xp , (y - y0)*scay+winYmin - TICKSIZE, 2*TICKSIZE, color);
		tft.setTextColor(ILI9341_BLACK);
		tft.drawFloat(i*xstep+xmin,1, xp-6,int(( y - y0)*scay+winYmin)+1, 1);
}
	return true;
}
void Graf::changeScaX(float dx) {
	scax = scax*dx;
	tft.fillRect(winXmin, winYmin, winXmax - winXmin, winYmax - winYmin, backgroundColor);
	
}
void Graf::scroll(float dx)
{
	x0 += (xmax-xmin)/dx;
	tft.fillRect(winXmin, winYmin, winXmax - winXmin, winYmax - winYmin, backgroundColor);
}
boolean Graf::drawAxY(int  x)  //yaxis at screen pos 0 for left 320 for right
{
	return boolean();
}

void Glx_GWindowsClass::init(byte type, int x0, int y0, int x1, int y1,uint16_t col_back)
{
	{ backgroundColor = col_back;
		winXmax = x1;
		winXmin = x0;
		winYmax = y1;
		winYmin = y0;
		tft.fillRect(x0, y0, x1 - x0, y1 - y0, col_back);
	}
}




void Glx_MWindowsClass::init(int x0, int y0, int x1, int y1)

	{
		Xmax = x1;
		Xmin = x0;
		Ymax = y1;
		Ymin = y0; 
}

int Glx_MWindowsClass::getPressed(uint16_t x,uint16_t y)
{	//Serial.print(x); Serial.print('_'); Serial.println(y);
// ----------------------------check if is a button-----------------------
	for (byte i = 0; i < menu[curr_m].nbutton; i++)
		
	{
		menu[curr_m].button[i].press(menu[curr_m].button[i].contains(x, y));
		if (menu[curr_m].button[i].isPressed())
		{
			menu[curr_m].button[i].drawButton(true);
			
				while (touch.isTouching()){}
				{  menu[curr_m].button[i].press(false);
					menu[curr_m].button[i].drawButton();

					if (menu[curr_m].menuIndex[i] == 0)
						return (i + curr_m * 10 + 1);
					else
					{
						curr_m = menu[curr_m].menuIndex[i] - 1;
						menu[curr_m].init();
						menu[curr_m].draw();
						return -curr_m;
					}
				}
		}
		
	}
	//-------no button pressed-----------------------------------------------
	i_sel = -1;
	return 0;
}


void Menu::init() {
	int WindowsW = Xmax - Xmin;
	int WindowsH = Ymax - Ymin;
	byte buttonW = WindowsW / nbutton;
	byte buttonH =  WindowsH;

	char buf[10];
	//Serial.println(nbutton);
	for (byte i = 0; i < nbutton; i++) {
		 strcpy(buf , menuName[i].c_str());
		button[i].initButton(&tft,  i*buttonW+buttonW/2,buttonH/2, buttonW, buttonH, 
			ILI9341_DARKCYAN, ILI9341_BLUE, ILI9341_GREENYELLOW,
			buf, 1);

	}
}
void Menu::draw()
{
	
	for (byte i = 0; i < nbutton; i++) {
		button[i].drawButton();
	}
}

void Glx_keyborad::init(int x0, int y0,byte uppercase)
{
	line[0][0] = "1234567890";		line[0][1] = "!\"£$%&/()=";
	line[1][0] = "qwertyuiop";		line[1][1] = "QWERTYUIOP";
	line[2][0] = "asdfghjkl+";		line[2][1] = "ASDFGHJKL*";
	line[3][0] = "zxcvbnm,.-";		line[3][1] = "ZXCVBNM;:_";
	LastLine[0] = "<";
	LastLine[1] = "shift";
	LastLine[2] = "Space";
	LastLine[3] = "#";
	LastLine[4] = "'";
	LastLine[5] = "ret";


	 byte buttonW = tft.width()/11;
	 byte buttonH = 20;

	 for (byte j = 0; j < 4;j++)
		 for (byte i = 0; i < 10; i++)
		 {
			 char c[2];
			  c[0] = line[j][uppercase][i];
			  c[1] = 0;
			 button[j * 10 + i].initButton(&tft, 
				 i*(buttonW+4) + x0,
				 j*(buttonH+4)+y0, 
				 buttonW, buttonH,
				 ILI9341_DARKCYAN, ILI9341_BLUE, ILI9341_GREENYELLOW,
				 c, 1);
			 button[j * 10 + i].drawButton();
		 }
	 int px = x0;
	 for (byte i = 0; i < 6; i++)
	 {   
		 byte WW = buttonW;
		 if (i == 1 || i == 2 || i == 5) WW = buttonW + 20;
		 button[40 + i].initButton(&tft,
			 px,
			 4 * (buttonH + 4) + y0,
			 WW, buttonH,
			 ILI9341_DARKCYAN, ILI9341_BLUE, ILI9341_GREENYELLOW,
			 LastLine[i], 1);
		 px += WW +15;
		// Serial.println(px, DEC);
		 button[40 + i].drawButton();
	 }
}



char Glx_keyborad::isPressed(uint16_t x, uint16_t y)

{	char key[6] = { '<', ' ', ' ', '\'', '#', 13 };

	for (byte j = 0; j < 5;j++)
	for (byte i = 0; i < (j==4?6:10); i++)
	{     
		button[i + j * 10].press(button[i+j*10].contains(x, y));
		
		if (button[i + j * 10].isPressed())
		{
			//Serial.print('p');
			button[i + j * 10].drawButton(true);
	
			while (touch.isTouching()) {}
			{
				if (j < 4) {
					button[i + j * 10].press(false);
					button[i + j * 10].drawButton();
					return (char)line[j][CaseStatus][i];
				}
				else {
					if (i == 1)
					{
						if (CaseStatus == 0) {
							CaseStatus = 1;
							init(15, 50, 1);
							return 0;
						}
						else {
							CaseStatus = 0;
							init(15, 50, 0);
							return 0;
						}
					}
					else
					{
						button[i +  40].press(false);
						button[i + 40].drawButton();

						return key[i];
					}
					}

			}
		}

	}
	//-------no button pressed-----------------------------------------------
	
	return 0;

}

void Glx_keyborad::end()
{
}

void Glx_TWindows::init(int x0, int y0, int x1, int y1,byte textH, byte mode)
{
	TEXT_HEIGHT = textH;// Height of text to be printed and scrolled
	ScreenH = tft.height();
		BOT_FIXED_AREA =ScreenH- y1; // Number of lines in bottom fixed area (lines counted from bottom of screen)
		TOP_FIXED_AREA = y0; // Number of lines in top fixed area (lines counted from top of screen)
		yStart = TOP_FIXED_AREA;
		yDraw = ScreenH- BOT_FIXED_AREA - TEXT_HEIGHT;

	// Keep track of the drawing x coordinate
	//	tft.fillRect(x0, y0, x1, y1, ILI9341_BLUE);
	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
	for (byte i = 0; i<40; i++) blank[i] = 0;
	setupScrollArea(TOP_FIXED_AREA, BOT_FIXED_AREA);
}

  size_t Glx_TWindows::write(uint8_t data)
{
if (data == '\r' || xPos>231) {
	xPos = 0;
	yDraw = scroll_line(); // It takes about 13ms to scroll 16 pixel lines
}
if (data > 31 && data < 128) {
	xPos += tft.drawChar(data, xPos, yDraw, 2);
	blank[(18 + (yStart - TOP_FIXED_AREA) / TEXT_HEIGHT) % 19] = xPos; // Keep a record of line lengths
}
//change_colour = 1; // Line to indicate buffer is being emptied
  }



// ##############################################################################################
// Call this function to scroll the display one text line
// ##############################################################################################
int  Glx_TWindows::scroll_line() {
	int yTemp = yStart; // Store the old yStart, this is where we draw the next line
						// Use the record of line lengths to optimise the rectangle size we need to erase the top line
	tft.fillRect(0, yStart, blank[(yStart - TOP_FIXED_AREA) / TEXT_HEIGHT], TEXT_HEIGHT, ILI9341_BLACK);

	// Change the top of the scroll area
	yStart += TEXT_HEIGHT;
	// The value must wrap around as the screen memory is a circular buffer
	if (yStart >= ScreenH - BOT_FIXED_AREA) yStart = TOP_FIXED_AREA + (yStart - ScreenH + BOT_FIXED_AREA);
	// Now we can scroll the display
	scrollAddress(yStart);
	return  yTemp;
}

// ##############################################################################################
// Setup a portion of the screen for vertical scrolling
// ##############################################################################################
// We are using a hardware feature of the display, so we can only scroll in portrait orientation
void  Glx_TWindows::setupScrollArea(uint16_t TFA, uint16_t BFA) {
	tft.writecommand(ILI9341_VSCRDEF); // Vertical scroll definition
	tft.writedata(TFA >> 8);
	tft.writedata(TFA);

	tft.writedata((ScreenH - TFA - BFA) >> 8);
	tft.writedata(ScreenH - TFA - BFA);
	tft.writedata(BFA >> 8);
	tft.writedata(BFA);
}

// ##############################################################################################
// Setup the vertical scrolling start address
// ##############################################################################################
void  Glx_TWindows::scrollAddress(uint16_t VSP) {
	tft.writecommand(ILI9341_VSCRSADD); // Vertical scrolling start address
	tft.writedata(VSP >> 8);
	tft.writedata(VSP);
}


