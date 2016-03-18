/*
 Name:		Sketch2.ino
 Created:	14/03/2016 12:23:55
 Author:	pbecc
*/
//#include "../Pins.h"
#include <ESP8266WiFi.h>
#include <FS.h>
// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	SPIFFS.begin();
}

// the loop function runs over and over again until power down or reset
void loop() {
	scanNetwork();
}

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
			Serial.print(i );
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
			psw [Npas]= Serial.readString();
			Serial.print(Ssid[Npas]);Serial.print(','); Serial.println(psw[Npas]);
			file.print(Ssid[Npas]); file.print(','); file.println(psw[Npas]);
			file.close();
		}
	}
}