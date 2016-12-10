#ifdef IOT
//#include <thingspeak-arduino-master\src\ThingSpeak.h>
struct MM {
public:
	unsigned int x;
	int y;
};
class  IOTstr{
public:

	   long Channel;
	   const char* key;
	   long values[8];
	   byte fieldn[8];
	   byte indval = 0;
	   byte nchar = 0;
	   IOTstr(char * mkey) { key = mkey; }
	   byte updateThingSpeak(long value, byte field_n, byte code)
	   {
		   if (code == 0) {
			   //void updateThingSpeak(long value, byte code) {
			   //Serial.println("update");
			   values[indval++] = value; nchar += 9; while (value >= 10) { value /= 10; nchar++; }
			   fieldn[indval - 1] = field_n;
				   return indval - 1;
		   }
		   else
		   {
#if ARDUINO <100 	
			   //Serial.println("connect....");
			   byte IOTip[4] = { 184,106,153,149 };  //ip of api.thingspeak.com
			   Client IOTclient(IOTip, 80);   //184,106,153,149
#else
#ifndef ESP8266 
			   EthernetClient IOTclient;
#else
			   WiFiClient IOTclient;
#endif
			   IOTclient.connect((uint8_t*)"api.thingspeak.com", 80);
#endif

#if ARDUINO >100
			   if (IOTclient.connected()) {

#else
			   if (IOTclient.connect()) {
#endif
				   delay(1000);
				   IOTclient.print("GET /update?api_key=FPIY8XPZBC2YGSS7&");

				   for (int ii = 0; ii < indval; ii++) {
					   IOTclient.print("field"); IOTclient.print(fieldn[ii]); IOTclient.print("="); IOTclient.print(values[ii]);
					   if (ii < indval - 1)IOTclient.print("&"); else IOTclient.println('\n');
				   }

				   //IOTclient.println(" HTTP/1.1\nHost: api.thingspeak.com\nConnection: close\n");//" HTTP/1.1\r\n" + "Host: " + "api.wunderground.com" + "\r\n" + "Connection: close \r\n\r\n"
				   indval = 0; nchar = 0;
				   delay(2000);
				   byte inc = 0;
				   //------------wait replay------------------------------------------------------------
				   while (!IOTclient.available() && inc < 250) { delay(20); inc++; }
				   //	Serial.println(inc, DEC);
				   //	while (IOTclient.available())Serial.print(IOTclient.read());

			   }
			   else Serial.println("coon.failed");
			   IOTclient.stop();
			   return 0;

			   }
		   return 100;
		   }
};
MM actualMM[30];
IOTstr IOTv[4] = { "15II2JPJ6PHZAJFC","KITD851JT8GQGQWL","AIN1ZOAMHDYQZO64","GW5VYMOOCGK62FDD" };


//, 194350, "15II2JPJ6PHZAJFC", 194351, "KITD851JT8GQGQWL", 194352, "AIN1ZOAMHDYQZO64", 194353, "GW5VYMOOCGK62FDD" };

byte load_mm(byte nvalve, int Dmm) {
	//load on vector actualMM and on thingspeak  mm of rain calculated now() for valve nvalve 
	byte j = 0;
	for (byte i=0;i<N_curves;i++)

		if (CurvesN[i] == nvalve) {
	//		actualMM[i].x = (now() - startime) / 60;
			actualMM[i].y += Dmm;
			j = i;
		}
	ThingSpeak.setField(nvalve % 10, actualMM[j].y);
	ThingSpeak.writeFields(IOTv[nvalve / 10].Channel, IOTv[nvalve / 10].key);
	return j;
}


#endif