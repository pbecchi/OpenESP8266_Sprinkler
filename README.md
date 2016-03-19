# OpenESP8266_Sprinkler
ESP8266 port of Opensprikler software.
NOTE:  This  code and the documentation is WORK IN PROGRES. All your comments and suggestions are welcome (please use GitHub Issue form).


MARCH 2016 - V2.1.6 First ESP8266 BETA release
----------------------------------------------

OpenSprinkler_Arduino V2.1.6	- beta test / based on the unified firmware at https://github.com/OpenSprinkler/OpenSprinkler-Firmware

While this release works OK, PLEASE see the 'Notes' section below for some known issues


Overview
--------

OpenSprinkler-Arduino is a branch  from Ray's OpenSprinkler code found in:

 https://github.com/OpenSprinkler/OpenSprinkler-Firmware

Scope of these modification is to increase the HW components compatible with OpenSprinkler: 
	starting with  a beta version  ported to ESP8266.

This code is a modified  starting from Opensprinkler-Arduino code : 
the OPENSPRINKLER code modified by Dave for Arduino MEGA porting , found in :

https://github.com/Dave1001/OpenSprinkler-Arduino

In general the approach is to make the minimum changes necessary to use standard Arduino libraries, 
and to get alternative hardware to run, and to debug.

Changes from Rays original code are marked with OPENSPRINKLER_ARDUINO (or variations thereof)and 
changes for ESP8266 porting are  bounded by "#ifdef ESP8266" , "#endif" commands.

Refer to the start of 'Config.h' and 'Pins.h' for options to substitute different hardware and turn functions on or off.


 FULL CREDIT to Ray  for all the hard work to build and maintain the Open Sprinkler project and to Dave for his porting to Mega that have simplified a lot my conversion to ESP8266.         
--------------------------------------------------------------------------------

Current Release
---------------

Version:     Opensprinkler V2.1.6 / beta_ESP_01

Date:        March 2016

Repository:  https://github.com/pbecchi/OpenESP8266_Sprinkler

License:     Creative Commons Attribution-ShareAlike 3.0 license

Hardware
--------
This Code is made to be compatible with several MCU and several peripheral: 

-the code has been ported to ESP8266 but compatibility with previous MCU & peripheral  has been maintained
-the list of peripheral that should be compatible is the following:
	-MCU
		ESP8266 boards: better to use ESP8266 v1.2 or NODEMCU v1.0 with more GPIO available.
		With NodeMCU you don’t have to worry about power supply and USB interface.
	-LCD
		Parallel 16x2 and 20x4 LCD(x) 
		Freetronics LCD Shield 16x2 + 5 buttons 
		I2C 16x2 or 20*4 LCD(x)
           (x) additional lines and characters to be used for future developments
	
      -RTC DS1307 or DS3231(best choice for ESP8266) + AT24C32 I2C EEPROM as can be found on z-042 breakout boards.
		The presence of the I2C EEPROM is necessary for the actual (beta_ESP_01) code to work on ESP8266.
	
      -ETHERNET
		-use of ESP8266 internal capability (code has been added to manage Wi-Fi connections to Hubs in a transparent way)
		-W5100 Ethernet shield (from Dave1001 code) 
		-ENC28J60   (from original OS code)
	-SD CARD
		-at the moment (current release), on ESP8266, I have added a tested emulation of SD drive on FLASH memory (ESP8266 SPIFFS)
		-SD SPI drive should be compatible (having sufficient GPIO available on ESP8266) 
	-PIN INPUT/OUTPUT 
		Since on ESP8266 only very few GPIO can be utilized, are bound to that number or
		-use PCF8574 (5) expansion IC with 8/16 additional IO pins (this is the preferred solution)
		-pin using interrupts should be routed directly to ESP8266
	-PIN BUTTON
		-analog input (5 inputs on Dave code Freetronic shield, 4 in my case using 4 switches).
		-standard 3 BUTTON PIN input with or without use of PCF8574
	-PIN for STATIONS output
		-shift register expander (like 75SN595) driven by 3 GPIO or thought an PCF8574
		-parallel connection to MCU (only for MEGA board) (from Dave1001 code)

!!!!a serial connection (USB) is necessary to upload and debug the code and for FIRST TIME CODE STARTUP (selection of WiFi network enter password etc…..)
		
!!!!!IMPORTANT: ESP8266 is only compatible with 3.3V logic levels!!!!!! (Use level shifters where necessary)
 
 Schematics of prototype boards I have built to test the code and some components data sheet are in the HARDWARE directory.

I intend to test as many different configurations as possible: participation to this effort is welcome!



In addition you'll need:

- Any 5V relay card that can be driven from Arduino digital output pins
  or a simple Triac gate circuit (see Hw schematics) 

- Power supplies(s), bits & pieces to hook it all up 

Compile and build
-----------------

To install and compile this code you need:
   
  1.  Arduino IDE 1.6.5+ -> http://arduino.cc/
		Using ESP core support library from http://arduino.esp8266.com/versions/1.6.5-1160-gef26c5f/doc/reference.html

 	  The code is large and may be difficult to manage with the limited resources of Arduino IDE
 
       My suggestion: use Visual Studio 2015 plus the Visual Micro add-on from here -> http://www.visualmicro.com/
       
  2.  Libraries:

	Most of the libraries are included on the GitHub code and should be placed on the same directory on the code.

	Following libraries should be on the sketchbook/libraries folder:
	
	-"Time.h"          	http://www.pjrc.com/teensy/td_libs_Time.html 
	-"TimeAlarms.h"    	http://www.pjrc.com/teensy/td_libs_TimeAlarms.html 
	-"DS1307RTC.h"     	http://www.pjrc.com/teensy/td_libs_DS1307RTC.html
	-"LiquidCrystal_I2C.h"	https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library

	Other libraries are standard (part of Arduino ESP core/Arduino IDE):

	Additional libraries for different Hw configurations

	- "LiquidCrystal.h" Standard Arduino Library
	- "SPI.h"           Standard Arduino Library
	- "Ethernet.h"      Standard Arduino Library
	- "EthernetUdp.h"   Standard Arduino Library
	- "SDFat.h"         https://github.com/greiman/SdFat

	ESP8266 not compatible libraries:

	- "ICMPPing.h"      https://github.com/BlakeFoster/Arduino-Ping
	- "MemoryFree.h"    http://playground.arduino.cc/Code/AvailableMemory 		

  NOTE - unless you're careful, you may have some issues with the libraries above co-existing with OpenSprinklerGen library. The easiest way to fix it is to remove OpenSprinkler from your Arduino library folder. 

  Assuming you have the libraries above installed, the code here *should* compile without too many issues (good luck!)
  
Notes
----------------------

If you're trying to make it work:

- Library conflicts:
	- if you're not careful you'll get a whole bunch (kind of like .dll hell from Windows in the old days before .net)
	- I STRONGLY recommend:
		- get rid of any Opensprinkler code you may have in your Arduino 'library' folder
		- update your Arduino libraries to the latest from  link above

- Multiple Simultaneous Button Presses:
 	-Work only with digital buttons

If you're a developer:

- Software IDE
	- I'm using Visual Studio 2015 Community Edition plus Visual Micro to build and upload code.
	- this release was tested to compile and upload using Arduino IDE Version 1.6.5  

- Discrete Outputs :
	- the number of discrete pins is pre-defined as 16 maximum 
      - you'll need to edit OpenSprinklerGen2.cpp and defines.h if you want more - Hostname 
	- hostname isn't implemented for opensprinkler : (I am working on it and soon will be available)
	
-Your HW configuration:

	-editing Pins.h file it will be possible to select a different hardware configuration that suit your needs: 
		Actually I have made to different prototype and you will see how the code address both.
		I will improve documentation on the file for explanation on how to change it.
	-other file that can be changed (carefully) are : config.h and defines.h ( for DEBUG and other code modifications).


TO DO

Progress with DEBUGGING and with documentation: your help and suggestion is highly appreciated!

Complete following modification that will be included in next releases:

Already under development (will be included in next release):

-Hostname definition in term of a given or automatic NAME for the unit that will be found in the net as NAME.local

-EEPROM on ESP8266 flash memory as an alternative to I2C EEPROM

Modifications Planned for future releases :

-SPI interface (e.g. for external SD)

-coding of weather algorithms internally

-other outcoming of your suggestions and recommendations!



