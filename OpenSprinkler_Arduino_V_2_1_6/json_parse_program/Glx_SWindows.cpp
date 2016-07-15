// 
// a6d82bced638de3def1e9bbb4983225c
// 

#include "Glx_SWindows.h"



void Graf::init( uint16_t col)
{
	if (ymax == 0 && ymin == 0)
	{
		xmax = x[0]; xmin = x[0]; ymax = YSign*y[0]; ymin = YSign*y[0];
	}
	color = col;
	for (byte i = 0; i < nval; i++)
	{
		if( x [ i]>xmax)xmax= x[i];
		if (x[i]<xmin)xmin = x[i];
		if (y[i]*YSign>ymax)ymax = YSign*y[i];
		if (y[i] * YSign<ymin)ymin = YSign*y[i];
		
	}
	windowsW = winXmax - winXmin;
	
	x0 = xmin;
	y0 = ymin;
	windowsH = winYmax - winYmin;
	scay =  windowsH/(ymax-ymin);
	// serial.println(x0);
	Serial.println(y0);
	Serial.println(scay);
	Serial.println(winYmin);
	scax =   windowsW/(xmax - xmin);
	// serial.println(scax);
	Serial.println(ymax);
	//Serial.println(ymin);
}

boolean Graf::draw()
{
	

	//tft.drawPixel(int(x[0] * scax) - x0, int(y[0] * scay) - y0, color);
	for (int i = 1; i < nval; i++)
		//if (x[i] >winXmin && x[i] < winXmax )
			tft.drawLine(int((x[i - 1]  - x0)*scax+winXmin), 
						int((YSign*y[i - 1] - ymin)*scay+winYmin),
						int((x[i ] - x0)*scax+winXmin),
						int((YSign*y[i] - ymin)*scay + winYmin),  color);
	return true;
	
}
#define TICKSIZE 2
boolean Graf::drawAxX(float y,float xstep)
{
//	tft.drawPixel(Glx_GWindowsClass::winXmin, y*scay-y0, color);
	tft.drawFastHLine(winXmin,(y- ymin)*scay +winYmin,
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
		tft.drawFastVLine(xp , (y - ymin)*scay+winYmin - TICKSIZE, 2*TICKSIZE, color);
		tft.setTextColor(ILI9341_BLACK);
		tft.drawFloat(i*xstep+xmin,1, xp-6,int(( y - ymin)*scay+winYmin)+1, 1);
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
boolean Graf::drawAxX(float  y,float xstep,byte ty)  //yaxis at screen pos 0 for left 320 for right
{   
	byte ntics = (xmax - xmin) / xstep;
	tft.drawFastHLine(winXmin, (y - ymin)*scay + winYmin, winXmax - winXmin, color);

	float substep;
	int i, j = 0;
	if (ty == TIME_SCA) {
		int Xwind = winXmax - winXmin;
		if (Xwind/scax > 1440) { xstep = 120; substep = 30; }
		if (Xwind/scax > 720) { xstep = 60; substep = 10; }
		if (Xwind/scax > 240) { xstep = 30; substep = 5; }
		if (Xwind/scax > 120) { xstep = 10; substep = 2; }
		if (Xwind/scax > 60) { xstep = 5; substep = 1; }

	}
	else {
		float divisor[3] = { 1,2,5 };
		substep = 0;
		for (int j = -10; j < 10; j++)
		{
			for (byte k = 0; k < 3; k++)
				if ((winXmax - winXmin) / scax < pow(10., j)*divisor[k])
				{
					//if (k > 0)xstep = pow(10, j)*divisor[k - 1];
					//else xstep = pow(10, j - 1)*divisor[2];
					xstep = pow(10, j - 1)*divisor[k];
					substep = xstep*0.2; break;
				}
			if (substep != 0)break;
		}
	}
	ntics = (xmax - xmin) / substep;
//	Serial.print((winXmax-winXmin)/scax); Serial.print(' '); Serial.println(substep);
//	Serial.print(xstep); Serial.print(' '); Serial.println(substep);
// serial.println(xstep);
for (byte i = 0; i < ntics; i++) {
	int xp = (i*substep + xmin - x0)*scax + winXmin;
	tft.drawFastVLine(xp,(y - ymin)*scay + winYmin - TICKSIZE,  2 * TICKSIZE, color);
	tft.setTextColor(ILI9341_BLACK);
//	Serial.println(i*substep/xstep);
	if (i*substep/xstep-int(i*substep/xstep)== 0) {
		int ivalue(i*substep + xmin); if (ty == TIME_SCA)ivalue = ivalue / 60;
		tft.drawFastVLine(xp,(y - ymin)*scay + winYmin - TICKSIZEL,  2 * TICKSIZEL, color);
		tft.drawNumber(ivalue, xp - 6, int((y - ymin)*scay + winYmin), 1);
	}

	}
	return boolean();
}

boolean Graf::drawAxy(int x_pos, float step, byte ty)
{
	byte ntics = (xmax - xmin) / step;
#define TIME_SCA 1
#define TICKSIZEL 3
	float substep;
	int i, j = 0;

	//if (ty == TIME_SCA) {
	//	if ((xmax - xmin)*scay > 1440) { step = 60; substep = 30; }
	//	if ((xmax - xmin)*scay > 720) { step = 60; substep = 10; }
	//	if ((xmax - xmin)*scay > 240) { step = 60; substep = 5; }
	//	if ((xmax - xmin)*scay > 120) { step = 60; substep = 1; }
	//	if ((xmax - xmin)*scay > 60)xstep = 1;
	//}
	//else {
		for (int j = -10; j < 10; j++)
			if ((ymax - ymin)*scay <pow (10.,j) ){ step = pow(10.,j )/2; substep = step*.2; break; }
	//}
	ntics = (ymax - ymin)*scay / substep;
	

	// serial.println(xstep);
	for (byte i = 0; i < ntics; i++) {
		int yp = (i*substep + ymin - y0)*scay + winYmin;
		tft.drawFastHLine(LegXmargin-TICKSIZE,yp, TICKSIZE, color);
		tft.setTextColor(ILI9341_BLACK);
		if (int(i*substep) % int(step) == 0) {
			tft.drawFastHLine(LegXmargin-TICKSIZEL, yp,  TICKSIZEL, color);
			tft.drawFloat(i*step + xmin, 1,LegXmargin-3, yp -8, 1);
		}

	}
	return boolean();
}

void Glx_GWindowsClass::init(byte type, int x0, int y0, int x1, int y1,uint16_t col_back)
{
	{ backgroundColor = col_back;
		winXmax = x1-xMargin;
		winXmin = x0+xMargin+LegXmargin;
		winYmax = y1-yMargin-LegYmargin;
		winYmin = y0+yMargin;
		tft.fillRect(x0, y0, x1 - x0, y1 - y0, col_back);
	}
}

int Glx_GWindowsClass::xpressed(int x,int y)
{
	if (y>winYmin && y<winYmax) 
return		  (x - winXmin) / scax + xmin;
	else return -32000;
}


int Glx_GWindowsClass::ypressed(int x, int y)
{
	if (y>winYmin && y<winYmax)
		return		  (y - winYmin) / scay +ymin;
	else return -32000;
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
	line[0][0] = "1234567890";		line[0][1] = "!\"?$%&/()=";
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
void Glx_TWindows::charPos(byte nline, byte nchar,byte mode) //y pos, x pos , mode >0 clean mode length to right border
{
	tft.setCursor(nchar, nline+yS);
	
	if (mode>0)	tft.fillRect(0,nline+yS,mode, 10, ILI9341_BLACK); //char heigth 10 pix
}
void Glx_TWindows::init(int x0, int y0, int x1, int y1,byte textH, byte mode)
{
	yS = y0;
	if (mode == 0) {
		TEXT_HEIGHT = textH;// Height of text to be printed and scrolled
		ScreenH = tft.height();
		BOT_FIXED_AREA = ScreenH - y1; // Number of lines in bottom fixed area (lines counted from bottom of screen)
		TOP_FIXED_AREA = y0; // Number of lines in top fixed area (lines counted from top of screen)
		yStart = TOP_FIXED_AREA;
		yDraw = ScreenH - BOT_FIXED_AREA - TEXT_HEIGHT;

		// Keep track of the drawing x coordinate
		//	tft.fillRect(x0, y0, x1, y1, ILI9341_BLUE);
		tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
		for (byte i = 0; i < 100; i++) blank[i] = 0;
		setupScrollArea(TOP_FIXED_AREA, BOT_FIXED_AREA);
		Scroll = true;
	}
	else
		Scroll = false;
}

size_t Glx_TWindows::write(uint8_t data)
{
	if (Scroll)
	{
		if (data == '\r' || xPos > 231) {
			xPos = 0;
			yDraw = scroll_line(); // It takes about 13ms to scroll 16 pixel lines
			//long times = millis();
			delay(20);
		}
		if (data > 31 && data < 128) {
			//	if (millis()<times+20)

			xPos += tft.drawChar(data, xPos, yDraw, 2);
			blank[(18 + (yStart - TOP_FIXED_AREA) / TEXT_HEIGHT) % 19] = xPos; // Keep a record of line lengths
		}
		//change_colour = 1; // Line to indicate buffer is being emptied
	}
	else
		tft.write(data);
}



// ##############################################################################################
// Call this function to scroll the display one text line
// ##############################################################################################
int  Glx_TWindows::scroll_line() {
	int yTemp = yStart; // Store the old yStart, this is where we draw the next line
						// Use the record of line lengths to optimise the rectangle size we need to erase the top line
	tft.fillRect(0, yStart, 280/*blank[(yStart - TOP_FIXED_AREA) / TEXT_HEIGHT]*/, TEXT_HEIGHT, ILI9341_BLACK);

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
void Glx_TWindows::textColor(int color) {
	tft.setTextColor(color);
}
void Glx_TWindows::throttle(int xpos, int ypos) {

	char cc[] = { '|','/','-','\\' };
	tft.setCursor(xpos, ypos+yS);
	tft.fillRect(xpos, ypos + yS, 8,10 , ILI9341_BLACK);
//	tft.setTextColor(ILI9341_BLACK);
//	tft.write(cc[index]);
	if (index++ == 3)index = 0;
	tft.setTextColor(ILI9341_WHITE);
	tft.write(cc[index]);

}


