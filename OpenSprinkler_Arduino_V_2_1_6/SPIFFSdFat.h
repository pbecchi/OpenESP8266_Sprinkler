#define O_CREAT 0
#define O_WRITE 0
#define O_READ 1
#define O_TRUNC 0
#define SPI_HALF_SPEED 0

class SdFat   {
public:
	SdFat(){}
	void remove(char data[]){}
	bool exists(char* buf){}
	bool mkdir(char data[]){}
	bool chdir(char data[]){}
	char *  vwd(){}//da correggere--------------------------------------------------cerca chiamata ///////////
	bool begin(uint8_t csPin = SS, uint8_t divisor = 2){}
	
	bool cardBegin(uint8_t csPin = SS, uint8_t divisor = 2){}
	//	return card()->begin(&m_spi, csPin, divisor){}
	
};
class SdFile {

public:
	SdFile(){}
	bool open(char name[], int i){} 
	void close(){}
	void seekEnd(){}
	void seekSet(int pos){}
	bool fgets(char buf[], int bufflen){}
	void write(char *){}
	void write(char *, int l){}
	void write(const char *, int l){}
};