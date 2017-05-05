
//
//    FILE: PCF8574.cpp
//  AUTHOR: Rob Tillaart
//    DATE: 02-febr-2013
// VERSION: 0.1.02
// PURPOSE: I2C PCF8574 library for Arduino
//     URL: 
//
// HISTORY:
// 0.1.02 replaced ints with uint8_t to reduce footprint;
//        added default value for shiftLeft() and shiftRight()
//        renamed status() to lastError();
// 0.1.01 added value(); returns last read 8 bit value (cached); 
//        value() does not always reflect the latest state of the pins!
// 0.1.00 initial version
// 
/* version of previous --------------

#define DEBU(p) //Serial.print('U');Serial.print(p);
#define DEBA(a) //Serial.print('a');Serial.print(a);
#define DEBD(p) //Serial.print('D');Serial.print(p);
#include "PCF8574Mio.h"

#include <Wire.h>
PCF8574::PCF8574(){}

void PCF8574::begin(int address)
{
	Serial.println(); Serial.print('@'); Serial.println(address);
	_address = address;
//	Wire.begin();
}*/
// HISTORY:
// 0.1.9  fix warning about return in readButton8()
// 0.1.08 2016-05-20 Merged work of Septillion 
//        Fix/refactor ButtonRead8() - see https://github.com/RobTillaart/Arduino/issues/38
//        missing begin() => mask parameter
// 0.1.07 2016-05-02 (manually merged) Septillion
//        added dataOut so a write() doesn't read first,
//        possibly corrupting a input pin;
//        fixed shift comment, should read 1..7;
//        added begin() to be sure it's in a known state,
//        states could be different if uC is reset and the PCF8574 isn't;
//        added buttonRead() and buttonRead8()
//        which only effect the output while reading
// 0.1.06 (intermediate) added defined errors + refactor rotate
// 0.1.05 2016-04-30 refactor, +toggleMask, +rotLeft, +rotRight
// 0.1.04 2015-05-09 removed ambiguity in read8()
// 0.1.03 2015-03-02 address int -> uint8_t
// 0.1.02 replaced ints with uint8_t to reduce footprint;
//        added default value for shiftLeft() and shiftRight()
//        renamed status() to lastError();
// 0.1.01 added value(); returns last read 8 bit value (cached);
//        value() does not always reflect the latest state of the pins!
// 0.1.00 initial version
//

#include "PCF8574Mio.h"

#include <Wire.h>

PCF8574::PCF8574()
{
//	_address = deviceAddress;
	_dataIn = 0;
	_dataOut = 0xFF;
	_buttonMask = 0xFF;
	_error = PCF8574_OK;
}

void PCF8574::begin(uint8_t deviceAddress)
{
	Serial.println(); Serial.print('@'); Serial.println(deviceAddress);
	_address = deviceAddress;
	Wire.begin();
	PCF8574::write8(0xFF);
}

// removed Wire.beginTransmission(addr);
// with  @100KHz -> 265 micros()
// without @100KHz -> 132 micros()
// without @400KHz -> 52 micros()
// TODO @800KHz -> ??
uint8_t PCF8574::read8()
{
	if (Wire.requestFrom(_address, (uint8_t)1) != 1)
	{
		_error = PCF8574_I2C_ERROR;
		return _dataIn; // last value
	}
#if (ARDUINO <  100)
	_dataIn = Wire.receive();
#else
	_dataIn = Wire.read();
#endif
	return _dataIn;
}

void PCF8574::write8(const uint8_t value)
{
	_dataOut = value;
	Wire.beginTransmission(_address);
	Wire.write(_dataOut);
	_error = Wire.endTransmission();
}

uint8_t PCF8574::read(const uint8_t pin)
{
	if (pin > 7)
	{
		_error = PCF8574_PIN_ERROR;
		return 0;
	}
	PCF8574::read8();
	return (_dataIn & (1 << pin)) > 0;
}

void PCF8574::write(const uint8_t pin, const uint8_t value)
{
	if (pin > 7)
	{
		_error = PCF8574_PIN_ERROR;
		return;
	}
	if (value == LOW)
	{
		_dataOut &= ~(1 << pin);
	}
	else
	{
		_dataOut |= (1 << pin);
	}
	write8(_dataOut);
}

void PCF8574::toggle(const uint8_t pin)
{
	if (pin > 7)
	{
		_error = PCF8574_PIN_ERROR;
		return;
	}
	toggleMask(1 << pin);
}

void PCF8574::toggleMask(const uint8_t mask)
{
	_dataOut ^= mask;
	PCF8574::write8(_dataOut);
}

void PCF8574::shiftRight(const uint8_t n)
{
	if (n == 0 || n > 7) return;
	_dataOut >>= n;
	PCF8574::write8(_dataOut);
}

void PCF8574::shiftLeft(const uint8_t n)
{
	if (n == 0 || n > 7) return;
	_dataOut <<= n;
	PCF8574::write8(_dataOut);
}

int PCF8574::lastError()
{
	int e = _error;
	_error = PCF8574_OK;
	return e;
}

void PCF8574::rotateRight(const uint8_t n)
{
	uint8_t r = n & 7;
	_dataOut = (_dataOut >> r) | (_dataOut << (8 - r));
	PCF8574::write8(_dataOut);
}

void PCF8574::rotateLeft(const uint8_t n)
{
	rotateRight(8 - (n & 7));
}

//added 0.1.07/08 Septillion
uint8_t PCF8574::readButton8(const uint8_t mask)
{
	uint8_t temp = _dataOut;
	PCF8574::write8(mask | _dataOut);
	PCF8574::read8();
	PCF8574::write8(temp);
	return _dataIn;
}

//added 0.1.07 Septillion
uint8_t PCF8574::readButton(const uint8_t pin)
{
	if (pin > 7)
	{
		_error = PCF8574_PIN_ERROR;
		return 0;
	}
	uint8_t temp = _dataOut;
	PCF8574::write(pin, HIGH);
	uint8_t rtn = PCF8574::read(pin);
	PCF8574::write8(temp);
	return rtn;
}

//added 0.1.08 Septillion
void PCF8574::setButtonMask(uint8_t mask) {
	_buttonMask = mask;
}

//
// END OF FILE
//
/*
uint8_t PCF8574::read8()
{
	Wire.beginTransmission(_address);
	Wire.requestFrom(_address, 1);
#if (ARDUINO <  100)
	_data = Wire.receive();
#else
	_data = Wire.read();
#endif
	_error = Wire.endTransmission();
	return _data;
}

uint8_t PCF8574::value()
{
	return _data;
}

void PCF8574::write8(uint8_t value)
{   DEBA(_address)
	Wire.beginTransmission(_address);
	_data = value;
	Wire.write(_data);
	_error = Wire.endTransmission();
}

uint8_t PCF8574::read(uint8_t pin)
{
	PCF8574::read8();
	return (_data & (1 << pin)) > 0;
}

void PCF8574::write(uint8_t pin, uint8_t value)
{
	PCF8574::read8();
	if (value == LOW)
	{DEBD(pin)
		_data &= ~(1 << pin);
	}
	else
	{DEBU(pin)
		_data |= (1 << pin);
	}

	PCF8574::write8(_data);
}

void PCF8574::toggle(uint8_t pin)
{
	PCF8574::read8();
	_data ^= (1 << pin);
	PCF8574::write8(_data);
}

void PCF8574::shiftRight(uint8_t n)
{
	if (n == 0 || n > 7) return;
	PCF8574::read8();
	_data >>= n;
	PCF8574::write8(_data);
}

void PCF8574::shiftLeft(uint8_t n)
{
	if (n == 0 || n > 7) return;
	PCF8574::read8();
	_data <<= n;
	PCF8574::write8(_data);
}

int PCF8574::lastError()
{
	int e = _error;
	_error = 0;
	return e;
}*/
//
// END OF FILE
//