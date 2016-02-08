// 
// 
// 

#include "lcd_mio.h"
/*
Lcd_mioClass::Lcd_mioClass(uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
}

Lcd_mioClass::Lcd_mioClass(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
}
*/
Lcd_mioClass::Lcd_mioClass() { Serial.begin(115200); }
void Lcd_mioClass::init(){}
void Lcd_mioClass::createChar(uint8_t n, uint8_t t[]){}
void Lcd_mioClass::clear(){}
void Lcd_mioClass::print(uint8_t c) { Serial.print((char)c); }
void Lcd_mioClass::print(uint16_t n) { Serial.print(n); }

void Lcd_mioClass::setCursor(uint8_t x, uint8_t y){}
size_t Lcd_mioClass::write(uint8_t(c)) { Serial.print(c); }
void Lcd_mioClass::blink() {}
void Lcd_mioClass::noBlink() {}

