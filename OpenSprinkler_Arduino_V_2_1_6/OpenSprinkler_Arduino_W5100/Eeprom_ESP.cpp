#include <Arduino.h>
#include <EEPROM.h>
#include "EEPROM_ESP.h"
#ifdef EEPROM_ESP
unsigned char eeprom_read_byte(
	unsigned char * pos)
{
	return EEPROM.read(int(pos));
}
int eeprom_read_word(
	const unsigned int * pointer) {
	return EEPROM.read(int(pointer++)) << 8 + EEPROM.read(int(pointer));
}

void eeprom_write_byte(
	unsigned char * pointer,
	unsigned char value)
{
	//Serial.println(int(pointer));

	EEPROM.write(int(pointer), value);
	EEPROM.commit();
}
void eeprom_read_block(
	void * __dst,
	const void * __src,
	unsigned int __n)//Read a block of __n bytes from EEPROM address __src to SRAM __dst.
{
	for (int i = 0; i < __n; i++) {

		*((char *)__dst + i) = eeprom_read_byte((uint8_t *)__src + i);

	}
}
void eeprom_write_block(
		const void * src,
		void * dst,
		unsigned int num)  //Write a block of __n bytes to EEPROM address __dst from __src.
{


	int pos = int(dst);
	for (int i = 0; i < num; i++) {
		byte data = *((unsigned  char*)src + i);
		//Serial.print(pos + i);
		EEPROM.write(pos + i, data);
	}

	EEPROM.commit();
	
}
#endif
