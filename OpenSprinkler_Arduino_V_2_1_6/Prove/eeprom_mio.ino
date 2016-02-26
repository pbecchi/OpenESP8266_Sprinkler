#ifdef ESP8266
#define EEPROMaddress 0x57 // RTC board ZS042
#else 
#define EEPROMaddress 0x50// RTC board TinyRTC
#endif

unsigned char eeprom_read_byte(unsigned char * pos)

	// Read one byte from the EEPROM
//	uint8_t readEEPROM(uint8_t EEPROMaddress, uint8_t page, uint8_t entry)
	{  
//		uint16_t pageEntryAddress = (uint16_t)((uint16_t)page << 5) | entry;
//		uint8_t  highAddressByte = (uint8_t)(pageEntryAddress >> 8);  // byte with the four MSBits of the address
//		uint8_t  lowAddressByte = (uint8_t)(pageEntryAddress - ((uint16_t)highAddressByte << 8)); // byte with the eight LSbits of the address
		uint16_t highAddressByte = (int)pos>>8;
		uint16_t  lowAddressByte = (int)pos & 0xFF;
		uint8_t data=0xFF;                                    // `data` will store the register data	 
		Wire.beginTransmission(EEPROMaddress);           // Initialize the Tx buffer
		Wire.write(highAddressByte);                     // Put slave register address 1 in Tx buffer
		Wire.write(lowAddressByte);                      // Put slave register address 2 in Tx buffer
		Wire.endTransmission(false);                     // Send the Tx buffer, but send a restart to keep connection alive
		Wire.requestFrom(EEPROMaddress, 1);              // Read one byte from slave register address 
		delay(10);                                       // maximum write cycle time per data sheet
		if(Wire.available()) data = Wire.read();                              // Fill Rx buffer with result
		delay(10);
		return data;                                     // Return data read from slave register
	}


int eeprom_read_word(		const unsigned int * pos) // Read one word from the EEPROM uint8_t readEEPROM(uint8_t EEPROMaddress, uint8_t page, uint8_t entry)
{
	//		uint16_t pageEntryAddress = (uint16_t)((uint16_t)page << 5) | entry;
	//		uint8_t  highAddressByte = (uint8_t)(pageEntryAddress >> 8);  // byte with the four MSBits of the address
	//		uint8_t  lowAddressByte = (uint8_t)(pageEntryAddress - ((uint16_t)highAddressByte << 8)); // byte with the eight LSbits of the address
	uint16_t highAddressByte = (int)*pos >> 8;
	uint16_t  lowAddressByte = (int)*pos & 0xFF;
	uint8_t data=0xFF;                                    // `data` will store the register data	 
	Wire.beginTransmission(EEPROMaddress);           // Initialize the Tx buffer
	Wire.write(highAddressByte);                     // Put slave register address 1 in Tx buffer
	Wire.write(lowAddressByte);                      // Put slave register address 2 in Tx buffer
	Wire.endTransmission(false);                     // Send the Tx buffer, but send a restart to keep connection alive
	Wire.requestFrom(EEPROMaddress, 1);             // Read one byte from slave register address 
	delay(10);                                       // maximum write cycle time per data sheet
	if(Wire.available()) data = Wire.read();                              // Fill Rx buffer with result
	delay(10);
	return data;                                     // Return data read from slave register
}

	void eeprom_write_byte(unsigned char * pos,unsigned char data)	{ // Write one byte to the EEPROM
	//	void writeEEPROM(uint8_t EEPROMaddress, uint8_t page, uint8_t entry, uint8_t data){
			// Construct EEPROM address from page and entry input
			// There are 128 pages and 32 entries (bytes) per page
			// EEPROM address are 16-bit (2 byte) address where the MS four bits are zero (or don't care)
			// the next seven MS bits are the page and the last five LS bits are the entry location on the page
			//uint16_t pageEntryAddress = (uint16_t)((uint16_t)page << 5) | entry;
			//uint8_t  highAddressByte = (uint8_t)(pageEntryAddress >> 8);  // byte with the four MSBits of the address
			//uint8_t  lowAddressByte = (uint8_t)(pageEntryAddress - ((uint16_t)highAddressByte << 8)); // byte with the eight LSbits of the address
            
		uint16_t highAddressByte = (int)pos >> 8;
		uint16_t  lowAddressByte = (int)pos & 0xFF;
		
			Wire.beginTransmission(EEPROMaddress);    // Initialize the Tx buffer
			Wire.write(highAddressByte);              // Put slave register address 1 in Tx buffer
			Wire.write(lowAddressByte);               // Put slave register address 2 in Tx buffer
			Wire.write(data);                         // Put data in Tx buffer
			delay(5);                                // maximum write cycle time per data sheet
			Wire.endTransmission();                   // Send the Tx buffer
			delay(5);
		
	}
	void eeprom_read_block(
		void * __dst,
		const void * __src,
		size_t __n) //Read a block of __n bytes from EEPROM address __src to SRAM __dst.
	{   
		char * bufp = (char *)__dst;

		for (int i = 0; i < __n; i++) {

			char c = eeprom_read_byte((unsigned char *)__src+i);
			*bufp++ = c;
		}
	}
	void eeprom_write_block(
		const void * src,
		void * dst,
		size_t num) //Write a block of __n bytes to EEPROM address __dst from __src.
	{
		char * bufp = (char *)src;
		uint8_t * EEpos =(uint8_t *) dst;
		for (int i = 0; i < (int) num; i++) {
		
			char ch = *bufp;


			eeprom_write_byte(EEpos++, ch);
			bufp++;
		}
	}