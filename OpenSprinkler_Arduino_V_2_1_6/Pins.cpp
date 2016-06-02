#include <Arduino.h>

#include "Pins.h"
#include <Wire.h>
#include "PCF8574Mio.h"
#include <FS.h>

enum I2Cdev_t
{
	RTC_TYPE,
	EEPROM_TYPE,
	PCF8574_TYPE,
	PCF8574A_TYPE
};
byte ADR[4] = {
	0x20,
	0x38,
	0x64,
	0x50
};
byte ADR_R[4] = {
	8,
	8,
	16,
	8
};
bool ADR_TYP[4] = {
	true,
	true,
	false,
	false
};
String ADR_DEV_NAME[4] = {

	"PCF8574",
	"PCF8574A",
	"DS3231",
	"AT24Cxx"
};
void ScanI2c()
{
	byte address[20];
	byte error;
	int nDevices;

	Serial.println("Scanning...");

	nDevices = 0;
	for (byte addres = 1; addres < 127; addres++)
	{
		// The i2c_scanner uses the return value of
		// the Write.endTransmisstion to see if
		// a device did acknowledge to the address.
		Wire.beginTransmission(addres);
		error = Wire.endTransmission();
		bool trov = false;
		byte trova = 255;
		if (error == 0)
		{
			Serial.print("I2C device found at address 0x");
			if (addres < 16)
				Serial.print("0");
			Serial.print(addres, HEX);
			Serial.print("\t");
			byte id = 0;
			while (id++ < 4) {
				if (addres >= ADR[id] && addres < ADR[id] + ADR_R[id]) {
					Serial.println(ADR_DEV_NAME[id]); trov = true;
					trova= id;

				}
			}
			if (!trov) Serial.println(" unknown");
		}
		if (trova!=255)
		if (ADR_TYP[trova]) {
			address[nDevices] = addres;
			nDevices++;
#ifdef PCF8574_
			PCF[nDevices].begin((int)address[nDevices]);
#endif
		}
		else if (error == 4)
		{
			Serial.print("Unknow error at address 0x");
			if (address[nDevices]<16)
				Serial.print("0");
			Serial.println(address[nDevices], HEX);
		}
	}
	if (nDevices == 0)
		Serial.println("No I2C devices found\n");
	else
		Serial.println("done\n");

//	return nDevices;
}
#include <ESP8266WiFi.h>
#include <FS.h>
void scanNetwork()
{
	Serial.println("scan start");

	// WiFi.scanNetworks will return the number of networks found
	int n = WiFi.scanNetworks();
	Serial.println("scan done");
	if (n == 0)
		Serial.println("no networks found");
	else
	{
		String Ssid[10];
		String psw[10];
		File file;
		int PaswKnown = -1;
		int Npas = 0;
		if (!SPIFFS.exists("SSID_PASSWORD"))
			file = SPIFFS.open("SSID_PASSWORD", "w");               //new password file
		else
		{    //-----------------read SSID and PASSWORD from file.

			Serial.println("Reading password file....");
			file = SPIFFS.open("SSID_PASSWORD", "r+");               //read passwords from file
			while (file.available())
			{
				char buff[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
				file.readBytesUntil(',', buff, 20);

				Ssid[Npas] = buff;
				char buf[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

				file.readBytesUntil('\n', buf, 20);

				psw[Npas++] = buf;
				Serial.print(Ssid[Npas - 1]);
				Serial.print('\t'); Serial.print("<>"); Serial.println(psw[Npas - 1]);
			}
		}
		Serial.print(n);
		Serial.println(" networks found");
		for (int i = 0; i < n; ++i)
		{
			// Print SSID and RSSI for each network found
			Serial.print(i);
			Serial.print(": ");
			int jpas = Npas - 1;
			while (WiFi.SSID(i) != Ssid[jpas] && jpas >= 0)jpas--;
			if (jpas >= 0)PaswKnown = i;
			Serial.print(WiFi.SSID(i));
			Serial.print(" (");
			Serial.print(WiFi.RSSI(i));
			Serial.print(")");
			Serial.print((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");

			if (jpas >= 0)Serial.println(" passw. available"); else Serial.println();
			delay(10);
		}

		if (PaswKnown < 0)
		{
			Serial.println("Select network n.");
			while (!Serial.available()) delay(10);
			byte ch = Serial.read();
			Ssid[Npas] = WiFi.SSID(ch - '0');
			Serial.print("Enter password for "); Serial.println(Ssid[Npas]);
			while (!Serial.available()) delay(10);
			psw[Npas] = Serial.readString();
			Serial.print(Ssid[Npas]); Serial.print(','); Serial.println(psw[Npas]);
			file.print(Ssid[Npas]); file.print(','); file.println(psw[Npas]);
			file.close();
		}
	}
}