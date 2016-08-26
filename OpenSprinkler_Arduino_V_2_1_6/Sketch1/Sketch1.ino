/*
 Name:		Sketch1.ino
 Created:	12/05/2016 10:29:06
 Author:	pbecc
*/

#include <ESP8266WiFi.h>
#include <SSIDPASSWORD.h>
#include <ArduinoOTA.h>

#include <ArduinoJson.h>
//const char* ssid     = "your-ssid";
//const char* password = "your-password";
#define TELNET
#define OTA

#ifdef TELNET
WiFiServer Tserver(23);
WiFiClient Tclient;

bool noClient = true;
long TimeOUT;
#define ONTIME 2000000L
int getClients()
{
	if (noClient) {
		Tclient = Tserver.available();
		if (Tclient) {
			noClient = false;
			Serial.print("New client! ");
			Tclient.flush();
			Tclient.print('>');

			TimeOUT = millis();
			return 1;
		}

		return 0;
	}
	else  //noclient false
		if (!Tclient.connected() || millis()>TimeOUT + ONTIME) {
			Tclient.stop();
			noClient = true;
			return 0;
		}
	return 1;
}
#define SP(x) {if(Tclient)Tclient.print(x);Serial.print(x);}
#define SPL(x) {if(Tclient)Tclient.println(x);Serial.println(x);}
#define SPS(x) {if(Tclient){Tclient.print(' ');Tclient.print(x);}Serial.print(x);}
#define SP_D(x) {if(Tclient)Tclient.print(x);Serial.print(x);}
#define SPL_D(x) {if(Tclient)Tclient.println(x);Serial.println(x);}
#define SPS_D(x) {if(Tclient){Tclient.print(' ');Tclient.print(x);}Serial.print(x);}
#else
#define SP(x) Serial.print(x)
#define SPL(x) Serial.println(x)
#define SPS(x) Serial.print(' ');Serial.print(x)
#define SP_D(x) Serial.print(x)
#define SPL_D(x) Serial.println(x)
#define SPS_D(x) Serial.print(' ');Serial.print(x)
#endif

//const char* host = "192.168.1.20";
//const char* streamId = "....................";
//const char* privateKey = "....................";

#define PROVA
WiFiClient client;
void setup() {
	Serial.begin(115200);
	delay(10);

	// We start by connecting to a WiFi network

	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);
	//WiFi.config({ 192,168,1,40 }, { 192,168,1,1 }, { 255,255,255,0 });
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
#ifdef TELNET
	Tserver.begin();
#endif
#ifdef OTA
	ArduinoOTA.onStart([]() {
		Serial.println("Start_Ota");
	});
	ArduinoOTA.onEnd([]() {
		Serial.println("\nEnd");
		ESP.restart();
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {//SP(".");
		{ Serial.print(" Progress:% "); Serial.println((progress / (total / 100))); }
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.print("Error : "); Serial.println(error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR)Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});
	ArduinoOTA.setHostname("EspSensors");

	ArduinoOTA.begin();

#endif
}

int value = 0;


float getvalue(char * buff, char * nome) {
	char* buffin;
	char nom[30];
	byte len = strlen(nome);
	for (byte i = len + 1; i > 0; i--)
		nom[i] = nome[i - 1];
	nom[0] = char(34);
	nom[len + 1] = char(34);
	nom[len + 2] = 0;

	buffin = strtok(buff, ",");
	if (strtok(buffin, ":") == nom)
		return atof(strtok(NULL, ","));
}
char * quoted(char * str) {
	char *point = strchr(str, '"')+1;
	return point;
}

//float getvalue(char * buff, String nome[],byte k) {
byte JsonDecode(byte k, char buff[], String nome[], float val[]) {

char* buffin;
	char * p2;
	char * p1;
	byte i = 0;
	for ( i=0;i<k;i++)
	nome[i] =  char(34)+nome[i] + char(34);
	//SPL(nome);
	buffin = strtok_r(buff, ",", &p1);

	SPL(buffin);
	char * title = " ";
	 int comp = -1;
	while (buffin != NULL )
	{
		 buffin = strtok_r(NULL, ",}", &p1);
		 if (buffin != NULL) {
			// SPL(buffin);
			 title = strchr(strtok_r(buffin, ":", &p2), '"');

			 if (title != NULL) {
				// SP(strlen(title)); SPL(title);
				 i = 0;
				 while (comp != 0&&i<k) {
					 comp = strcmp(title, nome[i].c_str()); i++;
				 }
				 if (comp == 0) {
					 SP("T "); SPL(title);
					 // float val;
					 char valore[20];
					 char * pointer = strtok_r(NULL, ",}", &p2);
					 char *point1 = strchr(pointer, '"');			//if value in between " " extract it
					 if (point1 != NULL) {
						 byte kk = 1;
						 while (pointer[kk] != '"') { valore[kk - 1] = pointer[kk++]; }
						 valore[kk - 1] = 0;
					 }
					 else strcpy(valore, pointer);
					 //SP(valore);
					 if (strchr(valore, '.') == NULL) {
						 long v = atol(valore); SPL(v);
						 val[i - 1] = (float)v;
					 }
					 else  val[i-1] = atof(valore);
					 SPL(val[i-1]);
					 comp = -1;
				 }
			 }
			 else comp = -1;
		 }
	}
	return 0 ;
	
}
//byte JsonDecode(byte Nval, char json[], String nomi[], float val[]) {
//	char* js0 = json;
//	for (byte i = 0; i < Nval; i++)
//	{
//		json = js0;
//		val[0] = getvalue(json, nomi,2);
//		SPL(val[0]);
//	}
//}

byte prova() {
	WiFiClient client;
	const int httpPort = 80;
	SPL("WU");
	if (client.connected())client.stopAll();
	
	//IPAddress(95, 101, 244, 53)
	if (!client.connect("api.wunderground.com", 80))
	{
		client.stop();
		SP("connection failed ");// SPL(jsonserver);
	//	pulseLed(2000, 0, 1);
		return 0;
	}
	/*/api/48dfa951428393ba/conditions/q/Italy/pws:ISAVONAL1.json HTTP/1.1
	Host: api.wunderground.com
	Connection: close */
	String url = "GET ";
   String streamId = "/api/48dfa951428393ba/conditions/q/Italy/pws:ISAVONAL1.json";

	//String streamId = "/api/e409b2aeaa5e3ffe/conditions/q/IVONtaly/Savona.json";
	//url += "?pw=";
	//url += privateKey;
	url = url + streamId
		+ " HTTP/1.1\r\n" + "Host: " + "api.wunderground.com" + "\r\n" 
		+ "Connection: close \r\n\r\n";
	//url += "&value=";
	//url += value;
	//	client.flush();
	//	Serial.print(" Requesting URL: ");
	//	Serial.print(url);
	client.print(url);
	//	client.print("GET /api/48dfa951428393ba/conditions/q/Italy/pws:ISAVONAL1.json  HTTP/1.1 \r\n Connection: close \r\n \r\n");
	ulong millismax = millis() + 10000;
	while (!client.available() && millis() < millismax)delay(10);
	if (!client.available())SPL("no rep");
	char c[3000]; int i = 0;
	byte cont = 0;
	while (i<3000&&cont<200){
		if(client.available())c[i++] = client.read();
		else { delay(50); cont++; }
	}
	c[i] = 0;
	SPL(c);
	float valori[2];
	String Nomi[3] = { "local_epoch","temp_c","relative_humidity" };
	JsonDecode(3, c, Nomi, valori);
	SPL(valori[0]);
	SPL(valori[1]);
	SPL((long)valori[2]);
	client.stop();

}
void loop() {
#ifdef TELNET
	getClients();
#endif
#ifdef OTA
	ArduinoOTA.handle();
#endif
	static ulong mymillis = 60000;
	if (millis() > mymillis) { prova(); mymillis = millis() + 120000; }

#ifdef PROVAOLD
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
	float val[4]; 
	time_t time;

	String Str="/api/e409b2aeaa5e3ffe/conditions/q/Italy/Savona.json";
	String nomi[] =  { "current_observation","local_epoch","solarradiation" }; 

	APIweatherV(Str, nomi, 2,  val);
	//for (byte i = 0; i < 3;i++)
	//APIcall("/jo", i);
#endif



	Serial.println(val[1]);
	Serial.println((time_t)val[0]);

	Serial.println("closing connection");
#endif
	
}


#ifdef PROVAOLD

#define MIL_JSON_ANS 10000
#define MAX_JSON_STRING 2500
#define SP_D(x) Serial.print(x)
#define SPL_D(x) Serial.println(x)
char json[2800];
byte APIweatherV(String streamId, String nomi[], byte Nval, float val[]) {

	va_list args;
	//va_start(args, Nval);
	SP_D("connecting to WU");

	const int httpPort = 80;
	if (!client.connect("api.wunderground.com", 80))
	{
		client.stop();

		SP("connection failed ");// SPL(jsonserver);
		return 0;
	}
	//	SP("Connect: "); SP(jsonserver);
	String url = streamId;
	//url += "?pw=";
	//url += privateKey;

	//url += "&value=";
	//url += value;

	Serial.print(" Requesting URL: ");
	Serial.print(url);
	//	client.print(url);
	// This will send the request to the server
	client.print(String("GET ") + url + " HTTP/1.1\r\n" +
		"Host: " + "api.wunderground.com" + "\r\n" +
		"Connection: close\r\n\r\n");

	int i = 0, json_try = 0; bool ISJSON = false; byte ii = 0;
	Serial.println("Waiting Json");
	long time_step = millis() + MIL_JSON_ANS;
	char c, cp, cpp; bool obs;
	while (millis() < time_step)
	{
		// Read all the lines of the reply from server and print them to Serialbol 
		while (millis() < time_step - MIL_JSON_ANS / 2)
			if (client.available()) {

				obs = client.findUntil(nomi[0].c_str(), "}}}");
				if (obs)break;
				else { SPL_D("error"); return 1; }
			}
		while (millis() < time_step&&c != '}'&&cp != '}'&&cpp != '}')

			while (obs&&client.available() && i < MAX_JSON_STRING) {
				cpp = cp;
				cp = c;
				c = client.read();
				Serial.print(c);
				//	buff[ii++] = c;
				//if (cpp == 'r'&&cp=='e'&&cpp=='s'){
				if (c == '{')
					ISJSON = true;

				//	json[i++] = cpp; json[i++] = cp;}
				if (ISJSON) json[i++] = c;
			}
		//Serial.println("endwhile");
		if (ISJSON) {
			json[i - 1] = 0;
			client.stop();

			SP_D("Connected ! "); //SPL(jsonserver);
								  //SP_D(json);
			SPL_D(" Json read!"); SPL_D(i);

			StaticJsonBuffer<2000> jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(json);

			// Test if parsing succeeds.
			if (!root.success()) {
				SPL("Weather parseObject() failed");
				return 0;
			}
			else {

				SPL_D("Weather Parsing...");
				for (byte i = 1; i < Nval + 1; i++) {
					if (nomi[i] == "local_epoch")
					{
						time_t time = root["local_epoch"];
						SPL_D(time);
						val[i - 1] = (float)time;
					}
					else
					{
						float valore = root[nomi[i]];
						val[i - 1] = valore;
						//	SP_D(valore);
					}
				}
				return 1;
			}
			//va_end(args);
		}

		else SPL_D("no json");

	}
	return 0;
}
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