// 
// 
// 
//
//#define I2CLCD

#ifdef I2CLCD
#include <LiquidCrystal\LiquidCrystal_I2C.h>
#include <LCD.h>
#endif
#include "lcd_mio.h"

/*
Lcd_mioClass::Lcd_mioClass(uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
}

Lcd_mioClass::Lcd_mioClass(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
}
*/

Lcd_mioClass::Lcd_mioClass() 
{
#ifdef I2CLCD
  LiquidCrystal_I2c lcdi2c(0x27);
#else
     Serial.begin(115200);
#endif
}

void Lcd_mioClass::init() 
{
#ifdef I2CLCD
	lcdi2c.begin(40,4);
#endif
}

void Lcd_mioClass::createChar(uint8_t n, uint8_t t[])
{
#ifdef I2CLCD
	lcdi2c.createChar(n, t);
#endif
}
void Lcd_mioClass::clear() {
#ifdef I2CLCD
	lcdi2c.clear();
#else
	Serial.println();
#endif
}

//void Lcd_mioClass::print(uint8_t c) { Serial.print(c); }
void Lcd_mioClass::setCursor(uint8_t x, uint8_t y)
{
#ifdef I2CLCD
	lcdi2c.setCursor(x, y);
#endif
}

size_t Lcd_mioClass::write(uint8_t(c)) 
{
#ifdef I2CLCD
	lcdi2c.print(char(c));
#else
	Serial.write(c);
	
#endif
}
void Lcd_mioClass::blink() {
#ifdef I2cLCD
	lcdi2c.blink();
#endif

}
void Lcd_mioClass::noBlink() {
#ifdef I2CLCD
	lcdi2c.noblink();
#endif
}

