// lcd_mio.h

#ifndef _LCD_MIO_h
#define _LCD_MIO_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


class Lcd_mioClass
{



public:

	Lcd_mioClass();
	void init();
	void createChar(uint8_t, uint8_t[]);
	void clear();
	void print(uint8_t);

	void print(uint16_t n);
	
	void setCursor(uint8_t, uint8_t);
	void blink();
	void noBlink();
	virtual size_t write(uint8_t);


};

//	extern Lcd_mioClass Lcd_mio1;

#endif

