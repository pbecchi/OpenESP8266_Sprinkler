/*
 Name:		Sketch1.ino
 Created:	12/05/2016 10:29:06
 Author:	pbecc
*/

#include <ESP8266WiFi.h>
#include <SSIDPASSWORD.h>
//const char* ssid     = "your-ssid";
//const char* password = "your-password";
#define SP(x) Serial.print(x)
#define SPL(x) Serial.println(x)
const char* host = "192.168.1.20";
const char* streamId = "....................";
const char* privateKey = "....................";

#define PROVA

void setup() {
	Serial.begin(115200);
	delay(10);

	// We start by connecting to a WiFi network

	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

int value = 0;
WiFiClient client;
void loop() {
	delay(50000);
	++value;

	Serial.print("connecting to ");
	Serial.println(host);

	// Use WiFiClient class to create TCP connections
#ifndef PROVA
	const int httpPort = 80;
	IPAddress hostIP(192, 168, 1, 20);
	if (!client.connect(hostIP, httpPort)) {
		Serial.println("connection failed");
		return;
	}
	// We now create a URI for the request
	/* String url = "/input/";
	url += streamId;
	url += "?private_key=";
	url += privateKey;
	url += "&value=";
	url += value;
	*/
	String url = "/jo?pw=a6d82bced638de3def1e9bbb4983225c";
	Serial.print("Requesting URL: ");
	Serial.println(url);

	// This will send the request to the server
	client.print(String("GET ") + url + " HTTP/1.1\r\n" +
		"Host: " + host + "\r\n" +
		"Connection: close\r\n\r\n");
	delay(10);

	// Read all the lines of the reply from server and print them to Serial
	while (client.available()) {
		String line = client.readStringUntil('\r');
		Serial.print(line);
	}
#else
	for (byte i = 0; i < 3;i++)
	APIcall("/jo", i);
#endif
	Serial.println();
	Serial.println("closing connection");
}
#define MIL_JSON_ANS 10000
#ifdef PROVA
bool APIcall(String streamId, byte ic) {

	//String privateKey = "opendoor";
	String privateKey = "a6d82bced638de3def1e9bbb4983225c"; //MD5 hashed
	const int httpPort = 80;
	IPAddress jsonserver = IPAddress(192, 168, 1, ic + 20);
	char json[500];
	delay(2000);
	if (!client.connect(jsonserver, 80))
	{
		SP("connection failed "); SPL(jsonserver);
		client.stop();
		return false;
	}

	SP("Connected to:"); SP(jsonserver);
	String url = streamId;
	url += "?pw=";
	url += privateKey;

	//url += "&value=";
	//url += value;

	SP(" Requesting URL: ");
	SPL(url);
	//	client.print(url);
	// This will send the request to the server
	client.print(String("GET ") + url + " HTTP/1.1\r\n" +
		"Host: " + "192.168.1.20" + "\r\n" +
		"Connection: close\r\n\r\n");
	/*client.print(String(" GET ") + url +
	" HTTP/1.1\r\n"+
	//"Host: " + "192.168.1.20"  +
	"Connection: close\r\n\r\n");
	ulong stopmil = millis() + 30000;
	while (!client.available()) {
	if (millis() < stopmil) delay(100);
	else {
	client.stop();
	SPL("no answer");
	return false;
	}
	}*/

	int i = 0, json_try = 0; bool ISJSON = false;
	SP("Waiting Json");
	long time_step = millis() + MIL_JSON_ANS;
	while (millis()<time_step)
	{
		// Read all the lines of the reply from server and print them to Serialbol 
		while (client.available()) {
			char c = client.read();
			if (c == '{')ISJSON = true;
			if (ISJSON) json[i++] = c;// else SP(c);
		}
		if (ISJSON) {
			json[i] = 0;
			SPL(" read!");
			client.stop();
			return true;
		}
		//	else {
		//		SP("."); delay(100); //SPL_D(i);
		//	}
	}
	SPL(" no Json");
	client.stop();
	return false;

}
#endif