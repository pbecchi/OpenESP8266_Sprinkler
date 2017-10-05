/*
 Name:		provaESPwiFi.ino
 Created:	10/08/2017 12:30:44
 Author:	pbecc
*/

#include <Arduino.h>
#include <Esp8266WiFi.h>
#include <FS.h>
#include <Wire.h>

#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define SSD1306
#ifdef SSD1306

#include <SPI.h>
#include "Adafruit_SSD1306.h"    // Modification of Adafruit_SSD1306 for ESP8266 compatibility
//#include "Adafruit_GFX.h"   // Needs a little change in original Adafruit library (See README.txt file)
#include <Wire.h>           // For I2C comm, but needed for not getting compile error

#define OLED_RESET  12 // Pin 15 -RESET digital signal
Adafruit_SSD1306 display(OLED_RESET); // FOR I2C

#define DEBUG_PRINT(x) Serial.print(x);display.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x);display.println(x);display.display();
#endif
char* SSID = "             ";
char* PASSWORD = "             ";
bool WiFiconnect() {
	//if (WiFi.status() != WL_CONNECTED)
	{
		// convert bytes to IPAddress
		/*	IPAddress ip = Byte2IP ( my_ip );
		IPAddress gw = Byte2IP ( gw_ip );
		IPAddress subnet = Byte2IP ( mask );
		*/
		DEBUG_PRINTLN("Wait WIFI...");
		delay(1000);
		WiFi.mode(WIFI_STA);
		delay(1000);
		WiFi.begin(SSID, PASSWORD);// , my_ip, dns_ip, gw_ip);
		byte netmask[4] = { 255,255,255,0 };
		WiFi.config(IPAddress(192,168,1,26), IPAddress(192, 168, 1, 1), netmask);
		DEBUG_PRINT("\nConnecting to "); DEBUG_PRINTLN(SSID);
		uint8_t i = 0;
		while (WiFi.status() != WL_CONNECTED && i++ <= 201) { yield(); delay(400); DEBUG_PRINT('.'); }
		if (i >= 201) {
			DEBUG_PRINT("Could not connect to"); DEBUG_PRINTLN(SSID);
			return false;
		}

		DEBUG_PRINT("Server started IP="); DEBUG_PRINTLN(WiFi.localIP());
		return true;
	}
}

//////////////////////////////////////////////////////////////////scanNetwork()///////////////////////////////////////
String found_SSID[5], found_psw[5];

char* CharFromString(String uno) {

	char *a = new char[uno.length() + 1];
	a[uno.length()] = 0;
	memcpy(a, uno.c_str(), uno.length());
	DEBUG_PRINT(a); DEBUG_PRINT(strlen(a));

	return a;
}

byte scanNetwork(byte flag)
// scan network and return n.network with known password found stored on found_SSID[] and found_psw[] 
// if no one is known or netflag is set open configuration portal new network are append to file,
{

	byte netCount = 0;
	DEBUG_PRINTLN("scan start");

	// WiFi.scanNetworks will return the number of networks found
	int n = WiFi.scanNetworks();
	//message("scan done");
	if (n == 0) {
		DEBUG_PRINTLN("no networks found");
		return 0;
	} else {
#ifdef WIFIMULTI
#include <ESP8266WiFiMulti.h>
		struct WiFiAP {
			char	APname[20]; APpsw[20];
		};
		WiFiAP wifiAP[10];
		byte n = eepro_read_byte(WIFIAP_POS);
		for (byte i = 0; i < n; i++) {
			eeprom_read_block(&wifiAP, WIFIAP_POS + i * sizeof(wifiAP), sizeof(wifiAP));
			wifim.addAP(APname[i], APpsw[i]);
		}
#else
		String Ssid[10];
		String psw[10];
		File file;
		int PaswKnown = -1;
		int Npas = 0;
		if (!SPIFFS.exists("/SSID_PASSWORD"))
			file = SPIFFS.open("/SSID_PASSWORD", "w");               //new password file
		else {    //-----------------read SSID and PASSWORD from file.

			Serial.println("Reading password file....");
			file = SPIFFS.open("SSID_PASSWORD", "a+");               //read passwords from file
			file.seek(0, SeekSet);
			while (file.available()) {
				char buff[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
				file.readBytesUntil(',', buff, 20);

				Ssid[Npas] = buff;
				char buf[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

				int n = file.readBytesUntil('\n', buf, 20);
				buf[n - 1] = 0;
				psw[Npas++] = buf;
				Serial.print(Ssid[Npas - 1]);
				Serial.print('\t'); Serial.print("<>"); Serial.println(psw[Npas - 1]);
			}
		}
		Serial.print("read:");
		Serial.println(Npas);

		//message(" networks found");
		byte ind[10] = { 0,1,2,3,4,5,6,7,8,9 };
		byte ii = 1;
		while (ii != 0) {
			ii = 0;
			for (byte j = 0; j < n - 1; j++)

				if (WiFi.RSSI(ind[j]) <= WiFi.RSSI(ind[j + 1])) {
					ii = ind[j]; ind[j] = ind[j + 1]; ind[j + 1] = ii;
				}
		}
		for (int ii = 0; ii < n; ++ii) {
			int i = ind[ii];
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
			// ---need to order with RSSI???????????????????
			if (jpas >= 0) {
				Serial.println(" passw. available"); netCount++;
				found_SSID[netCount - 1] = Ssid[jpas];
				found_psw[netCount - 1] = psw[jpas];

			}

			else Serial.println();
			delay(10);
		}

		if (PaswKnown < 0 || flag == 1 ) {
#ifdef WIFIMANAGER
			WiFiManager wifiManager;
			wifiManager.setConfigPortalTimeout(120);
			/*
			IPAddress ip = IPAddress(my_ip[0], my_ip[1], my_ip[2], my_ip[3]);
			IPAddress gw = IPAddress(gw_ip[0], gw_ip[1], gw_ip[2], gw_ip[3]);
			IPAddress subnet = IPAddress(255, 255, 255, 0);
			DEBUG_PRINTLN(ip);

			wifiManager.setSTAStaticIPConfig(ip, gw, subnet);*/
			wifiManager.setAPCallback(configModeCallback);
			//
			//	if button 2 has been pressed will start configuration portal to select a different network
			//
			wifiManager.resetSettings();
			if (wifiManager.startConfigPortal("MyConnectAP")) {
				netCount++;
				//!!!!!!!!!!reconnect in station mode!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				DEBUG_PRINTLN(WiFi.getMode());
				found_SSID[netCount - 1] = WiFi.SSID();
				found_psw[netCount - 1] = WiFi.psk();
				file.print(WiFi.SSID()); file.print(','); file.println(WiFi.psk());
				WiFi.disconnect();
				file.close();
				WiFi.mode(WIFI_STA);
				EtherCardW5100::netflag = false;
			} else return 0;
#else //WIFIMANAGER
			//read from usb
			Serial.println("Select network n.");
			while (!Serial.available() && millis()<180000) delay(10);
			if (millis() > 180000)return 0;
			byte ch = Serial.read();
			Ssid[Npas] = WiFi.SSID(ch - '0');
			while (Serial.available())Serial.read();
			Serial.print("Enter password for "); Serial.println(Ssid[Npas]);
			while (!Serial.available()) delay(10);
			psw[Npas] = Serial.readStringUntil(10);
			Serial.print(Ssid[Npas]); Serial.print(','); Serial.println(psw[Npas]);


			////////// append to file///////////////////			
			file.print(Ssid[Npas]); file.print(','); file.println(psw[Npas]);
			file.close();
#endif

		}
#endif
	}


	return netCount;
}
WiFiServer incoming_server(80);
int ndisc = 0,tot_disctime=0; ulong disctime = 0;
void WiFiEvent(WiFiEvent_t event) {
	Serial.printf("[WiFi-event] event: %d\n", event);

	switch (event) {
	case WIFI_EVENT_STAMODE_GOT_IP:
		Serial.println("WiFi connected");
		Serial.println("IP: ");
		Serial.println(WiFi.localIP());
		break;
	case WIFI_EVENT_STAMODE_DISCONNECTED:
		Serial.print("WiFi lost connection ");
		ndisc++; Serial.println(ndisc);
		disctime = millis();
#ifdef SSD1306
		display.setCursor(0, 0);
		display.print("Disconnected");
		display.display();
#endif
		break;
	case WIFI_EVENT_STAMODE_CONNECTED:

		Serial.printf("after %d ms now  connected \r\n", millis() - disctime);
		tot_disctime += (millis() - disctime)/1000;
#ifdef SSD1306
		display.setCursor(0, 0);
		display.print("Connected     ");
		display.setCursor(0, 8);
		display.print("Disc Tot.t="); display.print(tot_disctime);
		display.print(" Last disc.t="); display.print((millis()-disctime)/1000.);
		display.display();
#endif
		break;
	case WIFI_EVENT_STAMODE_AUTHMODE_CHANGE:
		Serial.println("WiFi Authmode Changed");
		break;

	}
}




void setup() {
	Serial.begin(115200);
	SPIFFS.begin();
	Wire.begin();
#ifdef SSD1306
	delay(10);
	display.begin(SSD1306_SWITCHCAPVCC,0x3C);  // Switch OLED

	display.display();
	display.clearDisplay();
	delay(2000);

	display.setTextSize(1);
	display.setTextColor(WHITE);
	
#endif
	bool result = false;
	uint8_t n = 0;
	uint8_t nNetwork = scanNetwork(0);
	if (nNetwork == 0) {
		DEBUG_PRINTLN("no net"); ESP.restart();
	}
		while (n < nNetwork && !result) {
			SSID =CharFromString( found_SSID[n]);
			PASSWORD = CharFromString( found_psw[n]);
			n++;
			result = WiFiconnect();
		}
		if (result) {
			String outstr = "Conn. ";
			outstr += WiFi.SSID();
			DEBUG_PRINTLN(outstr);
				incoming_server.begin(); 
		}
		WiFi.onEvent(WiFiEvent);

	
}
// the setup function runs once when you press reset or power the board


// the loop function runs over and over again until power down or reset
void loop() {
	WiFiClient client = incoming_server.available();
	if (client) {                             // if you get a client,
		Serial.println("New Client.");           // print a message out the serial port
		String currentLine = "";                // make a String to hold incoming data from the client
		while (client.connected()) {            // loop while the client's connected
			if (client.available()) {             // if there's bytes to read from the client,
				char c = client.read();             // read a byte, then
				Serial.write(c);                    // print it out the serial monitor
				if (c == '\n') {                    // if the byte is a newline character

													// if the current line is blank, you got two newline characters in a row.
													// that's the end of the client HTTP request, so send a response:
					if (currentLine.length() == 0) {
						// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
						// and a content-type so the client knows what's coming, then a blank line:
						client.println("HTTP/1.1 200 OK");
						client.println("Content-type:text/html");
						client.println();

						// the content of the HTTP response follows the header:
						client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
						client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

						// The HTTP response ends with another blank line:
						client.println();
						// break out of the while loop:
						break;
					} else {    // if you got a newline, then clear currentLine:
						currentLine = "";
					}
				} else if (c != '\r') {  // if you got anything else but a carriage return character,
					currentLine += c;      // add it to the end of the currentLine
				}

				// Check to see if the client request was "GET /H" or "GET /L":
				if (currentLine.endsWith("GET /H")) {
					digitalWrite(16, HIGH);               // GET /H turns the LED on
				}
				if (currentLine.endsWith("GET /L")) {
					digitalWrite(16, LOW);                // GET /L turns the LED off
				}
			}
		}
		// close the connection:
		client.stop();
		Serial.println("Client Disconnected.");
	}
}
