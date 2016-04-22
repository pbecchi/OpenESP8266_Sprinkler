// 
// 
// 

#include "Glx_SWindows.h"



Graf::Graf(float * xp, float * yp, int nv)
{
	nval = nv;
	for (byte i = 0; i < nval; i++)
	{
		x[i] = *(xp + i);
		y[i] = *(yp + i);
	}
}

boolean Graf::draw()
{

	tft.drawPixel(int(x[0] * scax) - x0, int(y[0] * scay) - y0, color);
	for (int i = 1; i < nval; i++)
		if (x[i] < Glx_GWindowsClass::winXmin && x[i] > Glx_GWindowsClass::winXmax )
			tft.drawLine(int(x[i - 1] * scax) - x0, 
					int(y[i - 1] * scay) - y0, 
					int(x[i] * scax) - x0,
					int(y[i] * scay) - y0, color);
	return true;
	
}

boolean Graf::drawAxX(float y,float deltX)
{
	tft.drawPixel(Glx_GWindowsClass::winXmin, y*scay-y0, color);
	tft.drawFastHLine(Glx_GWindowsClass::winXmin, y*scay - y0,
		Glx_GWindowsClass::winXmax- Glx_GWindowsClass::winXmin, color);
	byte ntics = (Glx_GWindowsClass::winXmax - Glx_GWindowsClass::winXmin); // deltX;
	for (byte i = 0; i < ntics; i++) {
		byte xp = Glx_GWindowsClass::winXmin;
		tft.drawFastVLine(xp + deltX, y*scay - y0 - 2, 4, color);
	}
	return true;
}

boolean Graf::drawAxY(float x)
{
	return boolean();
}

void Glx_GWindowsClass::init(byte type, int x0, int y0, int x1, int y1)
{
	{
		winXmax = x1;
		winXmin = x0;
		winYmax = y1;
		winYmin = y0; }
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


	 byte buttonW = 27;
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
