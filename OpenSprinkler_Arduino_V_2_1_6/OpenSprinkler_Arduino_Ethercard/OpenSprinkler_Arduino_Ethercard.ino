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

#include <Arduino.h>
#include "Config.h"
#include "../Defines.h"
#include "../OpenSprinkler.h"

#ifdef OPENSPRINKLER_ARDUINO

#include <Wire.h>
#include <SdFat.h>
#include <Time.h>
#include <DS1307RTC.h>
#ifdef LCDI2C
#include <LiquidCrystal_I2C.h>
#else
#include <LiquidCrystal.h>
#endif

#ifdef OPENSPRINKLER_ARDUINO_W5100
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <ICMPPing.h>
#include "../EtherCardW5100.h"
#else
#include <EtherCard.h>
#endif

#ifdef OPENSPRINKLER_ARDUINO_AUTOREBOOT
#include <TimeAlarms.h>
#endif // OPENSPRINKLER_ARDUINO_AUTOREBOOT

#ifdef OPENSPRINKLER_ARDUINO_FREEMEM
#include <MemoryFree.h>
#endif // OPENSPRINKLER_ARDUINO_FREEMEM

#endif

#ifndef OPENSPRINKLER_ARDUINO_WDT	// Only needed if not using WDT (to ensure the WDT is disbled)
#include <avr/wdt.h>
#endif

extern OpenSprinkler os;

void do_setup();
void do_loop();

void setup()
{
#ifdef OPENSPRINKLER_ARDUINO_AUTOREBOOT // Added for Auto Reboot   
    Alarm.alarmRepeat ( REBOOT_HR, REBOOT_MIN, REBOOT_SEC, reboot );
#endif // OPENSPRINKLER_ARDUINO_AUTOREBOOT

    do_setup();
}

void loop()
{
    do_loop();
}

#ifdef OPENSPRINKLER_ARDUINO_AUTOREBOOT // Added for Auto Reboot
void ( *resetPointer ) ( void ) = 0;			// AVR software reset function
void reboot()
{
    resetPointer();
}
#endif // OPENSPRINKLER_ARDUINO_AUTOREBOOT
