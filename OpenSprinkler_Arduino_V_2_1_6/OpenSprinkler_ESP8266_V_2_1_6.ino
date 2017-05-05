/* ====================================================

Version:	OpenSprinkler_Arduino 2.1.6

Date:		January 2016

Repository: https://github.com/Dave1001/OpenSprinkler-Arduino

License:	Creative Commons Attribution-ShareAlike 3.0 license

About:		OpenSprinkler-Arduino is a fork of Ray's OpenSprinkler code thats amended to use alternative hardware:
				- Arduino Mega 2560 (Arduino MCU that can handle compiled code size of around 60K)
				- Your choice of ethernet:
					- Wiznet W5100 Ethernet with onboard SD Card or
					- Enc28j60 ethernet with external SD card
				- Freetronics LCD Keypad Shield
				- Discrete IO outputs instead of using a shift register 

			PLUS this version adds a couple of additional functions:
				- ability to reboot daily to ensure stable operation
				- ability to display free memory on the LCD for debugging
				- heartbeat function to say 'alls well' - flashes an LED and the ':' on the LCD time at 1Hz
				- ability to turns the WDT on or off (refer to your reference documentationas to whether WDT is supported by the bootloader on your arduino)

			In general the approach is to make only the absolute minimum changes necessary to use standard Arduino libraries, 
			and to get alternative hardware to run. Otherwise the code is 'as is' from https://github.com/OpenSprinkler/OpenSprinkler-Firmware

			Changes from Rays original code are marked with OPENSPRINKLER_ARDUINO (or variations thereof)

			Refer to the start of 'Config.h' for options to substitute different hardware and turn functions on or off.

			As always - FULL CREDIT to Ray for all his hard work to build and maintain the Open Sprinkler project!

/* ====================================================
Original Opensprinkler code commences below here
==================================================== */

/* OpenSprinkler Unified (AVR/RPI/BBB/LINUX) Firmware
* Copyright (C) 2015 by Ray Wang (ray@opensprinkler.com)
*
* Main loop wrapper for Arduino
* Feb 2015 @ OpenSprinkler.com
*
* This file is part of the OpenSprinkler Firmware
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see
* <http://www.gnu.org/licenses/>.
*/

#include <EEPROM.h>
#include <Arduino.h>
#include "Config.h"
#include "Defines.h"

#include "OpenSprinkler.h"

#ifdef OPENSPRINKLER_ARDUINO

	#include <Wire.h>
#ifdef ESP8266

//#include "SPIFFSdFat.h"
//#include "PCF8574\PCF8574.h"
#include "PCF8574Mio.h"
#include <ESP8266mDNS.h>
#ifndef SDFAT
#include <FS.h>
#else 
#include <SD.h>
#include <SdFat.h>
#endif
#endif
	#include <Time.h>
#ifndef DS1307RTC
	#include <DS1307RTC.h>
#else
	#include "RTClib-master\RTClib.h"
#endif

#ifdef LCDI2C
    #include <LiquidCrystal_I2C.h>
#else
	#include <LiquidCrystal.h>
#endif

	#ifdef OPENSPRINKLER_ARDUINO_W5100
#ifndef ESP8266

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#else
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#ifdef OTA_UPLOAD
#include <ArduinoOTA.h>
#endif

#endif
//		#include <ICMPPing.h>
		#include "EtherCardW5100.h"
	#else
		//#include <EtherCard.h>
	#endif

	#ifdef OPENSPRINKLER_ARDUINO_AUTOREBOOT
		#include <TimeAlarms.h>
	#endif // OPENSPRINKLER_ARDUINO_AUTOREBOOT

	#ifdef OPENSPRINKLER_ARDUINO_FREEMEM
		#include <MemoryFree.h>
	#endif // OPENSPRINKLER_ARDUINO_FREEMEM
#endif

#ifndef OPENSPRINKLER_ARDUINO_WDT	// Only needed if not using WDT (to ensure the WDT is disbled)
#ifndef ESP8266
#include <avr/wdt.h>
#endif
#endif
byte DB = 0xFF;
#ifdef ESP8266
extern DS1307RTC RTC;
#endif

extern PCF8574 PCF[10];

extern OpenSprinkler os;

void do_setup();
void do_loop();

void setup()
{
	delay(2000);
#ifdef EEPROM_ESP
	EEPROM.begin(NVM_SIZE);
	
#endif

#ifdef OPENSPRINKLER_ARDUINO_AUTOREBOOT // Added for Auto Reboot   
   Alarm.alarmRepeat ( REBOOT_HR, REBOOT_MIN, REBOOT_SEC, reboot );
#endif // OPENSPRINKLER_ARDUINO_AUTOREBOOT
   do_setup();
#ifdef OTA_UPLOAD
   // Port defaults to 8266
   // ArduinoOTA.setPort(8266);

   // Hostname defaults to esp8266-[ChipID]
   // ArduinoOTA.setHostname("myesp8266");

   // No authentication by default
   // ArduinoOTA.setPassword((const char *)"123");

   ArduinoOTA.onStart([]() {
	   Serial.println("Start");
   });
   ArduinoOTA.onEnd([]() {
	   write_message("firmware reloaded!");
	   Serial.println("\nEnd");
   });
   ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
	   Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
   });
   ArduinoOTA.onError([](ota_error_t error) {
	   Serial.printf("Error[%u]: ", error);
	   write_message("firmware upload error!");
	   if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
	   else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
	   else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
	   else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
	   else if (error == OTA_END_ERROR) Serial.println("End Failed");
   });
   ArduinoOTA.begin();
#endif
}

void loop()
{
	DEBUG_COMMAND
#ifdef OTA_UPLOAD
		ArduinoOTA.handle();
#endif
   do_loop();
}

#ifdef OPENSPRINKLER_ARDUINO_AUTOREBOOT // Added for Auto Reboot
void ( *resetPointer ) ( void ) = 0;			// AVR software reset function
void reboot()
{
    resetPointer();
}
#endif // OPENSPRINKLER_ARDUINO_AUTOREBOOT

