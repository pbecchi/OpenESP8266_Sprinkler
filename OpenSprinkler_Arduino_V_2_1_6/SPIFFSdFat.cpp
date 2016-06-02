#include "SPIFFSdFat.h"
#include "eeprom_mio.h"
#include <FS.h>
#define SPIFFS_F_BY 2048
char emode[6];// = { "r","w","a","r+","w+","a+" };
File MyFile;
char FULLName[20]={'\0'};
//Class SdFile
    SdFile::SdFile() {  }
	bool SdFile::open(char nome[], int i) {
		int i = 0, j = 0;
		while (FULLName[i] != '\0')i++;
		while (nome[j] != '\0')FULLName[i++] = nome[j++];
		FULLName[i] = '\0';
		return  SPIFFS.open(FULLName, emode[i]);
	}
	void SdFile::close() { MyFile.close(); }
	void SdFile::seekEnd() { MyFile.seek(0,SeekEnd); }
	void SdFile::seekSet(int pos) { MyFile.seek(pos,SeekSet); }
	bool SdFile::fgets(char buf[], int bufflen) { return MyFile.readBytesUntil('\n',buf, bufflen)); }
	void SdFile::write(char * c) { MyFile.print(c); }
	void SdFile::write(char * c, int l) { for (int i = 0; i < l;i++) MyFile.print(c+i); }
	void SdFile::write(const char * c, int l) { for (int i = 0; i < l; i++) MyFile.print(c + i); }

//Class SdFat
	SdFat::SdFat() { SPIFFS_formatted = eeprom_read_byte((void *)SPIFFS_F_BY); }
	void SdFat::remove(char data[]) { SPIFFS.remove(data); }
	bool SdFat::exists(char* buf) { SPIFFS.exists(buf); }
	bool SdFat::mkdir(char data[]) {}
	bool SdFat::chdir(char data[]) { int i; while (data[i] != 0) FULLName[i++] = data[i]; FULLName[i] = '\0'; }
	char *  SdFat::vwd() {}//da correggere--------------------------------------------------cerca chiamata ///////////
	bool SdFat::begin(uint8_t csPin = SS, uint8_t divisor = 2) {
		
		SPIFFS.begin(); if (SPIFFS_formatted=='F')SPIFFS_formatted=SPIFFS.format();

	}

	bool SdFat::cardBegin(uint8_t csPin = SS, uint8_t divisor = 2) {
		//	return card()->begin(&m_spi, csPin, divisor);
	}