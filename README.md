##note# I new release has been uploaded in https://github.com/pbecchi/OpenESP8266_Sprinkler/tree/ForOS_Bee2_0%2COS_Solar%2COS_3_0
The new code is now compatible with OpenSprinkler 3.0, OS Bee 2.0 and OpenSprikler Solar!
You can find there a detail description of all new features and modifications to the code .


# OpenESP8266_Sprinkler
ESP8266 port of Opensprikler software.
NOTE:  This  code and the documentation is WORK IN PROGRES. All your comments and suggestions are welcome (please use GitHub Issue form).

Status
--------
15_5_2017

New brach upload in https://github.com/pbecchi/OpenESP8266_Sprinkler/tree/ForOS_Bee2_0%2COS_Solar%2COS_3_0
This code is now compatible with OpenSprinkler 3.0, OS Bee 2.0 and OpenSprikler Solar!
You can find there a detail description of all new features and modifications to the code .

15_6_2016

Last June release (brach MOD_AFTER_UPLOAD): I will do some other months of testing but new corrections or improvements are not forseen. But since I have worked with OpenSprinkler I have considered the UI to be the weekest aspect of this project and I intend  to create a new branch to revise and upgrade all User Interfaces, so that most of the operations can be controlled and monitored locally .

Release Notes
-----------------------
MAY 2017 New beta Release for Opensprinkler 3.0 OS Bee 2.0 and [Opensprinkler Solar](https://github.com/pbecchi/OpenSprinkler-Solar).
 Details :["see release notes"](https://github.com/pbecchi/OpenESP8266_Sprinkler/blob/ForOS_Bee2_0%2COS_Solar%2COS_3_0/OpenSprinkler_Arduino_V_2_1_6/OpenSprinkler%20SW%20new%20features.md)

JUNE 2016 V 2 2.1.6 Second ESP 8266 Beta release

Last relase include several improvements and additions:
    -WiFimanagger library to select right SSID and define password in a wireless manner.
    -Arduino OTA library library to upload firmware without using USB.  
    -correction of serveral bugs
    -testing of new hardware configurations:a Wiki with description of all Hw devices tested and relative pros/contras will be added to Git, you are invited to add your own projet and experience to it.
 
MARCH 2016 - V2.1.6 First ESP8266 BETA release
----------------------------------------------

OpenSprinkler_Arduino V2.1.6	- beta test / based on the unified firmware at https://github.com/OpenSprinkler/OpenSprinkler-Firmware version 2.1.6.

While this release works OK, PLEASE see the 'Notes' section below for some known issues

21-3-16 Branch "ModifAfterUpload"
--------------------------------

Hostname definition 	now you can access it with http://OSxx.local your Opensprinkler station xx is a unique n. from ESP hw n.
EEPROM on internalFlash now an external I2c cip is not required any more: up to 4kbytes can be store on flash
correction of  bugs  	ScanI2c()-scanning of i2c peripherals


10-4-2016 BRANCH "modifAfterUpload" (RELEASE 2_0)
-------------------------------------------------

Implemented Proto3 hardware configuration using both PCF8574 and 75SN575 IC .

BUGS COORECTION:

	-corrected PIN definition on shift register control code
	-corrected bug in log files I/O preventing historical log data extraction
	-corrected pin definitions (is now in term of GPIO n. )
	-enabled lcd brigthness control
	-source code: 
	
			all sources & libraries are in the same directory OPENSPRINKER_Arduino_V_2_1_6
			main renamed OpenSprinkler_ESP8266_v_2_1_6


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

- MCU
 
 	ESP8266 boards: better to use ESP8266 12E or NODEMCU v1.0 with more GPIO available.
			With NodeMCU you don’t have to worry about power supply and USB interface
            
- LCD
  1.  Parallel 16x2 and 20x4 LCD
  2.  Freetronics LCD Shield 16x2 + 5 buttons 
  3.  I2C 16x2 or 20x4 LCD on 20x4 2 additional lines and characters to be used for future developments
  4. Oled Graphic 128*64 I2c LCD 
- RTC
 
    DS1307 or DS3231(best choice for ESP8266) + AT24C32 I2C EEPROM as can be found on z-042 breakout boards.
- EEPROM
 
 	The presence of the I2C EEPROM is no more necessary for the actual (beta_ESP_01) code to work on ESP8266. ESP8266 flash memory can be used.
	
- ETHERNET
  1. 	WiFi on ESP8266  (code has been added to manage Wi-Fi connections to Hub)
  2.  	W5100 Ethernet shield (from Dave1001 code) 
  3.  	ENC28J60   (from original OS code)
- SD CARD
  1.  emulation of SD drive on FLASH memory (ESP8266 SPIFFS)
  2.  SD SPI drive should be compatible (having sufficient GPIO available on ESP8266) 
- PIN INPUT/OUTPUT 

   Since on ESP8266 only very few GPIO can be utilized, are bound to that number or
  1.  use PCF8574 (5) expansion IC with 8/16 additional IO pins (this is the preferred solution)
  2.  pin using interrupts should be routed directly to ESP8266
- PIN BUTTON
  1.  -analog input (5 inputs on Dave code Freetronic shield, 4 in my case using 4 switches).
  2.  -standard 3 BUTTON PIN input with or without use of PCF8574
- PIN for STATIONS output
  1.    shift register expander (like 75SN595) driven by 3 GPIO or thought an PCF8574
  2.    parallel connection to MCU (only for MEGA board) (from Dave1001 code)
  3.    HW for control of lactching solenoid (implemented in OS Bee 2.0 or OpenSprinkler Solar)
- SOLAR BATTERY power supply.
        Opensprinkler can now be operated on batteries recharged by a solar panel.
          
!!!!a serial connection (USB) is necessary to upload and debug the code 
		
!!!!!IMPORTANT: ESP8266 is only compatible with 3.3V logic levels!!!!!! (Use level shifters where necessary)
 
 Schematics of prototype boards I have built to test the code and some components data sheet are described in the WiKi
 You can use any combination of GPIO pin you would like as far as you use components supported by the code (libraries) and you change the  pins.h file accordingly (detail instructions  follows)

I intend to test as many different configurations as possible: participation to this effort is welcome! You could describe your HW configuration as well as functions of your prototype and give me input for code update.....thanks!
The file pins.h contain now 9 different hadware configurations that have been tested and run for a long period.


Compile and build
-----------------

To install and compile this code you need:
   
  1.  Arduino IDE 1.6.5+ -> http://arduino.cc/
	  Using ESP core support library from http://arduino.esp8266.com/versions/1.6.5-1160-gef26c5f/doc/reference.html
	  The code is large and may be difficult to manage with the limited resources of Arduino IDE
 
        My suggestion: use Visual Studio 2015 plus the Visual Micro add-on from here -> http://www.visualmicro.com/
          If you use Arduino Ide put all file in the same directory and get rid of "../" on includes!
       
  2.  Libraries:

Most of the libraries are included on the GitHub code and should be placed on the same directory on the code.

Following libraries should be on the sketchbook/libraries folder:
	
		-"Time.h"          	http://www.pjrc.com/teensy/td_libs_Time.html 
		-"TimeAlarms.h"    	http://www.pjrc.com/teensy/td_libs_TimeAlarms.html 
		-"DS1307RTC.h"     	http://www.pjrc.com/teensy/td_libs_DS1307RTC.html
		-"LiquidCrystalI2C.h 	https://bitbucket.org/fmalpartida/new-l ... V1.2.1.zip
		-"Time.h" 		https://github.com/PaulStoffregen/Time
		-"DS1307RTC.h" 		https://github.com/PaulStoffregen/DS1307RTC - 

Other libraries are standard (part of Arduino ESP core/Arduino IDE):

Additional libraries for different Hw configurations

		- "LiquidCrystal.h" Standard Arduino Library
		- "SPI.h"           Standard Arduino Library
		- "Ethernet.h"      Standard Arduino Library
		- "EthernetUdp.h"   Standard Arduino Library
		- "SDFat.h"         https://github.com/greiman/SdFat

ESP8266 not compatible libraries:

		- "ICMPPing.h"      https://github.com/BlakeFoster/Arduino-Ping have been replaced .

		- "MemoryFree.h"    http://playground.arduino.cc/Code/AvailableMemory
    
   ADDITIONAL LIBRARIIES for branch For OS Bee2.0 OS Solar, OS 3.0
        -see [installation.md](https://github.com/pbecchi/OpenESP8266_Sprinkler/blob/13606e0582eec0a5f2e4ec5140fc7da2a080cae6/Installation.md)
         		

   NOTE - unless you're careful, you may have some issues with the libraries above co-existing with OpenSprinklerGen library. The        easiest way to fix it is to remove OpenSprinkler from your Arduino library folder. 

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

- Software IDE

	- I'm using Visual Studio 2015 Community Edition plus Visual Micro to build and upload code.
	- this release was tested to compile and upload using Arduino IDE Version 1.6.5+  

- Discrete Outputs :

	- the number of discrete pins is pre-defined as 16 maximum 
         	- you'll need to edit OpenSprinklerGen2.cpp and defines.h if you want more - Hostname 
	- hostname is implemented for opensprinkler : (it is OSxx where xx are last to digit of MAC)
	
-Your HW configuration:

-editing Pins.h file it will be possible to select a different hardware configuration that suit your needs: 
		Actually I have made to different prototype and you will see how the code address both.
		
-other file that can be changed (carefully) are : config.h and defines.h ( for DEBUG and other code modifications).

-How to change Pins.h file

    The file details each pin used for your HW configuration. 
    Copy the example more similar to what you need and change it carefully to your HW definitions.
	You have first to define the configuration you are working on (ESP or other MCU) 
	and the type of peripheral you are using:see detailed comment in the file about it.
	Then you have to select the pins (for I2c, station output, shift registers, rain ....all other functions)
	and assign it to a pin number, using following rule:
		-pin <32 will be assigned to MCU (ESP8266) GPIO pin numbers( only few are available for ESP8266).
		-pin >32 and <48 will be assigned to an first I2c extender (the one with the lowest address)
		-pin >48 and <64 will be assigned to second I2c extender 
		-so on 16 pin at the time
	This way you can use expander for all functions (interrupt donot work right now) and free most of the pins on MCU

-First Code Startup

	When you start the code the program will check your connected i2c devices and define what to do with it: 
	check on Serial output to verify that your HW has been setup correctely.
	It will also check available WiFi networks and ask relative Password than will be stored in a file for future restart, now this is implemented using WiFi manager portal.
	All other restart dont need any intervention (if you dont need to discover new WiFi AP!).
	




TO DO

Progress with DEBUGGING and with documentation: your help and suggestion is highly appreciated!

	
