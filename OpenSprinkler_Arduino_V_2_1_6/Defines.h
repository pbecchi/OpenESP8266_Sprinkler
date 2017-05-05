/* OpenSprinkler Unified (AVR/RPI/BBB/LINUX) Firmware
 * Copyright (C) 2015 by Ray Wang (ray@opensprinkler.com)
 *
 * OpenSprinkler macro defines and hardware pin assignments
 * Feb 2015 @ OpenSprinkler.com
 *
 * This file is part of the OpenSprinkler library
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

#ifndef _DEFINES_H
#define _DEFINES_H
#define HOSTNAM

#include <Arduino.h>
#include "Config.h"
#include "libsel.h"
/** Firmware version, hardware version, and maximal values */
#define OS_FW_VERSION			216		// Firmware version: 216 means 2.1.6
										// if this number is different from the one stored in non-volatile memory
										// a device reset will be automatically triggered

#define OS_FW_MINOR				1		// Firmware minor version

/** Hardware version base numbers */
#define OS_HW_VERSION_BASE		0x00
#define OSPI_HW_VERSION_BASE	0x40
#define OSBO_HW_VERSION_BASE	0x80
#define SIM_HW_VERSION_BASE		0xC0

/** Hardware type macro defines */
#define HW_TYPE_AC				0xAC	// standard 24VAC for 24VAC solenoids only, with triacs
#define HW_TYPE_DC				0xDC	// DC powered, for both DC and 24VAC solenoids, with boost converter and MOSFETs
#define HW_TYPE_LATCH			0x1A	// DC powered, for DC latching solenoids only, with boost converter and H-bridges

/** File names */
#define WEATHER_OPTS_FILENAME "wtopts.txt"    // weather options file
#define STATION_ATTR_FILENAME "stns.dat"      // station attributes data file
#define STATION_SPECIAL_DATA_SIZE  23

#define FLOWCOUNT_RT_WINDOW		30    // flow count window (for computing real-time flow rate), 30 seconds

/** Station type macro defines */
#define STN_TYPE_STANDARD		0x00
#define STN_TYPE_RF				0x01
#define STN_TYPE_REMOTE			0x02
#define STN_TYPE_OTHER			0xFF

/** Sensor type macro defines */
#define SENSOR_TYPE_NONE		0x00
#define SENSOR_TYPE_RAIN		0x01
#define SENSOR_TYPE_FLOW		0x02
#define SENSOR_TYPE_OTHER		0xFF


#undef OS_HW_VERSION

/** Hardware defines */
#if defined(ARDUINO)

#if F_CPU==8000000L
#define OS_HW_VERSION (OS_HW_VERSION_BASE+20)
#elif F_CPU==12000000L
#define OS_HW_VERSION (OS_HW_VERSION_BASE+21)
#elif F_CPU==16000000L
#if defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284__)
#define OS_HW_VERSION (OS_HW_VERSION_BASE+23)
#else
#define OS_HW_VERSION (OS_HW_VERSION_BASE+22)
#endif
#endif
#include "Pins.h"


/** Non-volatile memory (NVM) defines */
#if defined(ARDUINO)

	/** 2KB NVM (ATmega644) data structure:
	  * |         |     |  ---STRING PARAMETERS---      |           |   ----STATION ATTRIBUTES-----      |          |
	  * | PROGRAM | CON | PWD | LOC | JURL | WURL | KEY | STN_NAMES | MAS | IGR | MAS2 | DIS | SEQ | SPE | OPTIONS  |
	  * |  (996)  |(12) |(36) |(48) | (40) | (40) |(24) |   (768)   | (6) | (6) |  (6) | (6) | (6) | (6) |  (48)    |
	  * |         |     |     |     |      |      |     |           |     |     |      |     |     |     |          |
	  * 0        996  1008   1044  1092  1132   1172   1196        1964  1970  1976   1982  1988  1994  2000      2048
	  */

	/** 4KB NVM (ATmega1284) data structure:
	  * |         |     |  ---STRING PARAMETERS---      |           |   ----STATION ATTRIBUTES-----      |          |
	  * | PROGRAM | CON | PWD | LOC | JURL | WURL | KEY | STN_NAMES | MAS | IGR | MAS2 | DIS | SEQ | SPE | OPTIONS  |
	  * |  (2438) |(12) |(36) |(48) | (48) | (48) |(24) |   (1344)  | (7) | (7) |  (7) | (7) | (7) | (7) |   (56)   |
	  * |         |     |     |     |      |      |     |           |     |     |      |     |     |     |          |
	  * 0       2438  2450   2486  2534  2582   2630   2654        3998  4005  4012   4019  4026  4033  4040      4096
	  */

	#if defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284__)//|| defined(BATTERY) // for 4KB NVM

		#define MAX_EXT_BOARDS    6  // maximum number of exp. boards (each expands 8 stations)
		#define MAX_NUM_STATIONS  ((1+MAX_EXT_BOARDS)*8)  // maximum number of stations

		#define NVM_SIZE            4096  // For AVR, nvm data is stored in EEPROM, ATmega1284 has 4K EEPROM
		#define STATION_NAME_SIZE   24    // maximum number of characters in each station name

		#define MAX_PROGRAMDATA     2438  // program data
		#define MAX_NVCONDATA       12    // non-volatile controller data
		#define MAX_USER_PASSWORD   36    // user password
		#define MAX_LOCATION        48    // location string
		#define MAX_JAVASCRIPTURL   48    // javascript url
		#define MAX_WEATHERURL      48    // weather script url
		#define MAX_WEATHER_KEY     24    // weather api key

	#else

		#define MAX_EXT_BOARDS		5  // maximum number of exp. boards (each expands 8 stations)
		#define MAX_NUM_STATIONS	((1+MAX_EXT_BOARDS)*8)  // maximum number of stations
#ifdef BATTERY							//more room to store battery options
		#define NVM_SIZE 3072
#else
		#define NVM_SIZE            2048  // For AVR, nvm data is stored in EEPROM, ATmega644 has 2K EEPROM
#endif
        #define STATION_NAME_SIZE   16    // maximum number of characters in each station name

		#define MAX_PROGRAMDATA     996   // program data
		#define MAX_NVCONDATA       12     // non-volatile controller data
		#define MAX_USER_PASSWORD   36    // user password
		#define MAX_LOCATION        48    // location string
		#define MAX_JAVASCRIPTURL   40    // javascript url
		#define MAX_WEATHERURL      40    // weather script url
		#define MAX_WEATHER_KEY     24    // weather api key,

	#endif

#else // NVM defines for RPI/BBB/LINUX

	// These are kept the same as AVR for compatibility reasons
	// But they can be increased if needed
	#define NVM_FILENAME        "nvm.dat" // for RPI/BBB, nvm data is stored in a file

	#define MAX_EXT_BOARDS    6  // maximum number of exp. boards (each expands 8 stations)
	#define MAX_NUM_STATIONS  ((1+MAX_EXT_BOARDS)*8)  // maximum number of stations

	#define NVM_SIZE            4096
	#define STATION_NAME_SIZE   24    // maximum number of characters in each station name

	#define MAX_PROGRAMDATA     2438  // program data
	#define MAX_NVCONDATA       12     // non-volatile controller data
	#define MAX_USER_PASSWORD   36    // user password
	#define MAX_LOCATION        48    // location string
	#define MAX_JAVASCRIPTURL   48    // javascript url
	#define MAX_WEATHERURL      48    // weather script url
	#define MAX_WEATHER_KEY     24    // weather api key

#endif  // end of NVM defines

/** NVM data addresses */
#define ADDR_NVM_PROGRAMS      (0)   // program starting address
#define ADDR_NVM_NVCONDATA     (ADDR_NVM_PROGRAMS+MAX_PROGRAMDATA)
#define ADDR_NVM_PASSWORD      (ADDR_NVM_NVCONDATA+MAX_NVCONDATA)
#define ADDR_NVM_LOCATION      (ADDR_NVM_PASSWORD+MAX_USER_PASSWORD)
#define ADDR_NVM_JAVASCRIPTURL (ADDR_NVM_LOCATION+MAX_LOCATION)
#define ADDR_NVM_WEATHERURL    (ADDR_NVM_JAVASCRIPTURL+MAX_JAVASCRIPTURL)
#define ADDR_NVM_WEATHER_KEY   (ADDR_NVM_WEATHERURL+MAX_WEATHERURL)
#define ADDR_NVM_STN_NAMES     (ADDR_NVM_WEATHER_KEY+MAX_WEATHER_KEY)
#define ADDR_NVM_MAS_OP        (ADDR_NVM_STN_NAMES+MAX_NUM_STATIONS*STATION_NAME_SIZE) // master op bits
#define ADDR_NVM_IGNRAIN       (ADDR_NVM_MAS_OP+(MAX_EXT_BOARDS+1))  // ignore rain bits
#define ADDR_NVM_MAS_OP_2      (ADDR_NVM_IGNRAIN+(MAX_EXT_BOARDS+1)) // master2 op bits
#define ADDR_NVM_STNDISABLE    (ADDR_NVM_MAS_OP_2+(MAX_EXT_BOARDS+1))// station disable bits
#define ADDR_NVM_STNSEQ        (ADDR_NVM_STNDISABLE+(MAX_EXT_BOARDS+1))// station sequential bits
#define ADDR_NVM_STNSPE        (ADDR_NVM_STNSEQ+(MAX_EXT_BOARDS+1)) // station special bits (i.e. non-standard stations)
#define ADDR_NVM_OPTIONS       (ADDR_NVM_STNSPE+(MAX_EXT_BOARDS+1))  // options

/** Default password, location string, weather key, script urls */
#define DEFAULT_PASSWORD          "opendoor"
#define DEFAULT_LOCATION          "Boston,MA"
#define DEFAULT_WEATHER_KEY       "e409b2aeaa5e3ffe"	// replace this key with your own
#define DEFAULT_JAVASCRIPT_URL    "https://ui.opensprinkler.com/js"
#define DEFAULT_WEATHER_URL       "weather.opensprinkler.com"


/** Macro define of each option
  * Refer to OpenSprinkler.cpp for details on each option
  */

typedef enum
{
    OPTION_FW_VERSION = 0,
    OPTION_TIMEZONE,
    OPTION_USE_NTP,
    OPTION_USE_DHCP,
    OPTION_STATIC_IP1,
    OPTION_STATIC_IP2,
    OPTION_STATIC_IP3,
    OPTION_STATIC_IP4,
    OPTION_GATEWAY_IP1,
    OPTION_GATEWAY_IP2,
    OPTION_GATEWAY_IP3,
    OPTION_GATEWAY_IP4,
    OPTION_HTTPPORT_0,
    OPTION_HTTPPORT_1,
    OPTION_HW_VERSION,
    OPTION_EXT_BOARDS,
    OPTION_SEQUENTIAL_RETIRED,
    OPTION_STATION_DELAY_TIME,
    OPTION_MASTER_STATION,
    OPTION_MASTER_ON_ADJ,
    OPTION_MASTER_OFF_ADJ,
    OPTION_SENSOR_TYPE,
    OPTION_RAINSENSOR_TYPE,
    OPTION_WATER_PERCENTAGE,
    OPTION_DEVICE_ENABLE,
    OPTION_IGNORE_PASSWORD,
    OPTION_DEVICE_ID,
    OPTION_LCD_CONTRAST,
    OPTION_LCD_BACKLIGHT,
    OPTION_LCD_DIMMING,
    OPTION_BOOST_TIME,
    OPTION_USE_WEATHER,
    OPTION_NTP_IP1,
    OPTION_NTP_IP2,
    OPTION_NTP_IP3,
    OPTION_NTP_IP4,
    OPTION_ENABLE_LOGGING,
    OPTION_MASTER_STATION_2,
    OPTION_MASTER_ON_ADJ_2,
    OPTION_MASTER_OFF_ADJ_2,
    OPTION_FW_MINOR,
    OPTION_PULSE_RATE_0,
    OPTION_PULSE_RATE_1,
    OPTION_REMOTE_EXT_MODE,
    OPTION_RESET,
#ifdef BATTERY
	D_SLEEP_DURATION,
	D_SLEEP_START_TIME,
	D_SLEEP_INTERVAL,
	MIN_VOLTS_D_SLEEP,
	MIN_VOLTS_L_SLEEP,
	L_SLEEP_DURATION,
	BATTERY_mAH,
#endif
    NUM_OPTIONS	// total number of options
} OS_OPTION_t;

/** Log Data Type */
#define LOGDATA_STATION    0x00
#define LOGDATA_RAINSENSE  0x01
#define LOGDATA_RAINDELAY  0x02
#define LOGDATA_WATERLEVEL 0x03
#define LOGDATA_FLOWSENSE  0x04

#ifdef GETOUT //da eliminare
#ifdef OPENSPRINKLER_ARDUINO_DISCRETE
	/* READ ME - PIN_EXT_BOARDS defines the total number of discrete digital IO pins
	used to control watering stations. There are 8 stations per extender board, that
	MUST have PIN_EXT_BOARDS x 8  pins defined below (otherwise you'll get out of range
	issues with the array of pins defined in OpenSprinklerGen2.cpp) */
	#define PIN_EXT_BOARDS	2

	/* Use these pins when the control signal to switch watering solenoids on and off
	is driven directly from the arduino digital output pins (i.e. not using a shift register
	like the regular opensprinkler hardware) */
	#define PIN_STN_S01		46
	#define PIN_STN_S02		44
	#define PIN_STN_S03		42
	#define PIN_STN_S04		40
	#define PIN_STN_S05		47
	#define PIN_STN_S06		45
	#define PIN_STN_S07		43
	#define PIN_STN_S08		41

	#define PIN_STN_S09		32
	#define PIN_STN_S10		34
	#define PIN_STN_S11     36
	#define PIN_STN_S12		38
	#define PIN_STN_S13		33
	#define PIN_STN_S14		35
	#define PIN_STN_S15		37
	#define PIN_STN_S16		39

	#define PIN_RF_DATA		30    // RF data pin - efault is 38
	#define PORT_RF			PORTA
	#define PINX_RF			PINA3

#else
#ifndef ESP8266
	// hardware pins
	#define PIN_BUTTON_1      31    // button 1
	#define PIN_BUTTON_2      30    // button 2
	#define PIN_BUTTON_3      29    // button 3
	#define PIN_RF_DATA       28    // RF data pin
	#define PORT_RF        PORTA
	#define PINX_RF        PINA3

	#define PIN_SR_LATCH       3    // shift register latch pin
	#define PIN_SR_DATA       21    // shift register data pin
	#define PIN_SR_CLOCK      22    // shift register clock pin
	#define PIN_SR_OE          1    // shift register output enable pin

#else //-----------------------ESP8266------------------------------------------
/*#define PIN_BUTTON_1      31    // button 1
#define PIN_BUTTON_2      30    // button 2
#define PIN_BUTTON_3      29    // button 3
#define PIN_RF_DATA       28    // RF data pin
#define PORT_RF        PORTA
#define PINX_RF        PINA3
*/
#define BUTTON_ADC_PIN    A0    // A0 is the button ADC input
#define SDA_PIN          D5
#define SCL_PIN          D2
#define PIN_SR_LATCH       D3    // shift register latch pin
#define PIN_SR_DATA        D0    // shift register data pin
#define PIN_SR_CLOCK       10    // shift register clock pin
//#define PIN_SR_OE              // shift register output enable pin
#define OS_HW_VERSION    0.0
#endif//--------------------------------------------------------------------------
#endif // OPENSPRINKLER_ARDUINO_DISCRETE
#ifdef OPENSPRINKLER_ARDUINO_FREETRONICS_LCD
	/* Note - D4 on the Freetronics LCD shield clashes with the
	chipselect pin for the SD card that is also D4 on some W5100 shields.
	You may need to jumper it to D2 as described at:
	http://forum.freetronics.com/viewtopic.php?t=770 */
	
	// Freetronics LCD with pin D4 bridged to pin D2
	#define PIN_LCD_RS         8    // LCD rs pin
	#define PIN_LCD_EN         9    // LCD enable pin
	#define PIN_LCD_D4         2    // LCD D4 pin
	#define PIN_LCD_D5         5    // LCD D5 pin
	#define PIN_LCD_D6         6    // LCD D6 pin
	#define PIN_LCD_D7         7    // LCD D7 pin
	#define PIN_LCD_BACKLIGHT  3    // LCD backlight pin
	#define PIN_LCD_CONTRAST  A1    // LCD contrast pin - NOT USED FOR FREETRONICS LCD (just set this to an unused pin) 
/*
	// Freetronics LCD 
	#define PIN_LCD_RS         8    // LCD rs pin - default = 19
	#define PIN_LCD_EN         9    // LCD enable pin - default = 18
	#define PIN_LCD_D4         4    // LCD d4 pin - default = 20
	#define PIN_LCD_D5         5    // LCD d5 pin - default = 21
	#define PIN_LCD_D6         6    // LCD d6 pin - default = 22
	#define PIN_LCD_D7         7    // LCD d7 pin - default = 23
	#define PIN_LCD_BACKLIGHT  3    // LCD backlight pin - default = 12
	#define PIN_LCD_CONTRAST  A1    // LCD contrast pin - NOT USED FOR FREETRONICS LCD (just set this to an unused pin) 
*/
	// some example macros with friendly labels for LCD backlight / pin control
	#define LCD_BACKLIGHT_OFF()     digitalWrite( PIN_LCD_BACKLIGHT, LOW )
	#define LCD_BACKLIGHT_ON()      digitalWrite( PIN_LCD_BACKLIGHT, HIGH )
	#define LCD_BACKLIGHT(state)    {if( state ){digitalWrite( PIN_LCD_BACKLIGHT, HIGH );}else{digitalWrite( PIN_LCD_BACKLIGHT, LOW );} }
    #define BUTTON_ADC_PIN    A0    // A0 is the button ADC input

#else
	// regular 16x2 LCD pin defines
	#define PIN_LCD_RS        19    // LCD rs pin
	#define PIN_LCD_EN        18    // LCD enable pin
	#define PIN_LCD_D4        20    // LCD d4 pin
	#define PIN_LCD_D5        21    // LCD d5 pin
	#define PIN_LCD_D6        22    // LCD d6 pin
	#define PIN_LCD_D7        23    // LCD d7 pin
	#define PIN_LCD_BACKLIGHT 12    // LCD backlight pin
	#define PIN_LCD_CONTRAST  13    // LCD contrast pin
#endif // OPENSPRINKLER_ARDUINO_FREETRONICS_LCD
#ifdef BUTTON_ADC_PIN

	// ADC readings expected for the 5 buttons on the ADC input

#define RIGHT_10BIT_ADC    80   // right
#define UP_10BIT_ADC     150   // up
#define DOWN_10BIT_ADC   240    // down
#define LEFT_10BIT_ADC   400   // left
#define SELECT_10BIT_ADC  280    // select
#define BUTTONHYSTERESIS  16    // hysteresis for valid button sensing window

#define BUTTON_RIGHT       1    // values used for detecting analog buttons
#define BUTTON_UP          2    // 
#define BUTTON_DOWN        3    // 
#define BUTTON_LEFT        4    // 
#define BUTTON_SELECT      5    //   
#endif
#ifndef OPENSPRINKLER_ARDUINO_DISCRETE
	// DC controller pin defines
	#define PIN_BOOST         20    // booster pin
	#define PIN_BOOST_EN      23    // boost voltage enable pin
#endif // OPENSPRINKLER_ARDUINO_DISCRETE

#ifdef OPENSPRINKLER_ARDUINO_HEARTBEAT
	#define PIN_HEARTBEAT     13    // Pin to show a heartbeat LED
#endif // OPENSPRINKLER_ARDUINO_HEARTBEAT

#ifdef OPENSPRINKLER_ARDUINO_W5100 // Wiznet W5100
	#define PIN_ETHER_CS     10    // Ethernet controller chip select pin - default is 10
	#define PIN_SD_CS        4     // SD card chip select pin - default is 4

	// Define the chipselect pins for all SPI devices attached to the arduino
	// Unused pins needs to be pulled high otherwise SPI doesn't work properly
	#define SPI_DEVICES   0      // number of SPI devices
	const uint8_t spi_ss_pin[] =   // SS pin for each device
	{
		53,						   // SD card chipselect on standard Arduino W5100 Ethernet board
		34						   // Ethernet chipselect on standard Arduino W5100 Ethernet board
	};
#else
	#define PIN_ETHER_CS      53    // Ethernet controller chip select pin
	#define PIN_SD_CS         34    // SD card chip select pin
#endif // OPENSPRINKLER_ARDUINO_W5100

#ifdef OPENSPRINKLER_ARDUINO_DISCRETE
// You'll need to use different pins for these items if you have them connected
#define PIN_RAINSENSOR     18    // rain sensor is connected to pin D3
#define PIN_FLOWSENSOR     19    // flow sensor (currently shared with rain sensor, change if using a different pin)
#define PIN_FLOWSENSOR_INT 20    // flow sensor interrupt pin (INT1)
#define PIN_EXP_SENSE      21    // expansion board sensing pin (A4)
#define PIN_CURR_SENSE     A3    // current sensing pin (A7)
#define PIN_CURR_DIGITAL   A3    // digital pin index for A7
#else
#ifdef ESP8266
#define PIN_RAINSENSOR    D6    // rain sensor is connected to pin D3
#define PIN_FLOWSENSOR    D1    // flow sensor (currently shared with rain sensor, change if using a different pin)
#define PIN_FLOWSENSOR_INT 5    // flow sensor interrupt pin (INT1)
#define PIN_EXP_SENSE      D6    // expansion board sensing pin (A4)
#else
#define PIN_RAINSENSOR    11    // rain sensor is connected to pin D3
#define PIN_FLOWSENSOR    11    // flow sensor (currently shared with rain sensor, change if using a different pin)
#define PIN_FLOWSENSOR_INT 1    // flow sensor interrupt pin (INT1)
#define PIN_EXP_SENSE      4    // expansion board sensing pin (A4)
#define PIN_CURR_SENSE     7    // current sensing pin (A7)
#define PIN_CURR_DIGITAL  24    // digital pin index for A7
#endif
#endif
#endif //GETOUT
// Ethernet buffer size
#if defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284__)
#define ETHER_BUFFER_SIZE   1500 // ATmega1284 has 16K RAM, so use a bigger buffer
#else
#define ETHER_BUFFER_SIZE   960  // ATmega644 has 4K RAM, so use a smaller buffer
#endif

#ifdef OPENSPRINKLER_ARDUINO_AUTOREBOOT
#define REBOOT_HR            12     // hour to perform daily reboot
#define REBOOT_MIN           00     // min  to perform daily reboot
#define REBOOT_SEC           00     // sec  to perform daily reboot
#endif	// OPENSPRINKLER_ARDUINO_AUTOREBOOT

#ifdef OPENSPRINKLER_ARDUINO_WDT
#ifndef ESP8266
#define 	wdt_reset()   __asm__ __volatile__ ("wdr")  // watchdog timer reset
#endif
#endif	// OPENSPRINKLER_ARDUINO_WDT
#define DEBUG_BEGIN(x)   Serial.begin(x)
#define SERIAL_DEBUG
#if defined(SERIAL_DEBUG) /** Serial debug functions */
extern byte DB;
#define DB_MASK 0xFF
#define DEBUG_COMMAND  if(Serial.available()){DB=Serial.read()-'0';DB=DB*10+Serial.read()-'0';Serial.print("->");Serial.println(DB);}

#define DEBUG_PRINTF(x,y)   if(DB&DB_MASK) Serial.print(x,y)
#define DEBUG_PRINT(x)  if(DB&DB_MASK) Serial.print(x)
#define DEBUG_PRINTLN(x) if(DB&DB_MASK) Serial.println(x)  //;Serial.print(DB,DEC);Serial.println(DB_MASK,DEC);
#else
#define DEBUG_PRINTF {}
#define DEBUG_COMMAND {}
//#define DEBUG_BEGIN(x)   {}
#define DEBUG_PRINT(x)   {}
#define DEBUG_PRINTLN(x) {}
#endif
#ifndef ESP8266
	typedef unsigned char   uint8_t;
	typedef unsigned int    uint16_t;
	typedef int int16_t;
#endif
#ifdef OPENSPRINKLER_ARDUINO
#ifndef ESP8266
	typedef const char prog_char;
	typedef const char prog_uchar;
#endif
#endif

#endif // Hardware defines for RPI/BBB
#ifdef PIPPO
/** OSPi pin defines */
#if defined(OSPI)

#define OS_HW_VERSION    OSPI_HW_VERSION_BASE
#define PIN_SR_LATCH      22    // shift register latch pin
#define PIN_SR_DATA       27    // shift register data pin
#define PIN_SR_DATA_ALT   21    // shift register data pin (alternative, for RPi 1 rev. 1 boards)
#define PIN_SR_CLOCK       4    // shift register clock pin
#define PIN_SR_OE         17    // shift register output enable pin
#define PIN_RAINSENSOR    14    // rain sensor
#define PIN_FLOWSENSOR    14    // flow sensor (currently shared with rain sensor, change if using a different pin)
#define PIN_RF_DATA       15    // RF transmitter pin
#define PIN_BUTTON_1      23    // button 1
#define PIN_BUTTON_2      24    // button 2
#define PIN_BUTTON_3      25    // button 3

/** BBB pin defines */
#elif defined(OSBO)

#define OS_HW_VERSION    OSBO_HW_VERSION_BASE
// these are gpio pin numbers, refer to
// https://github.com/mkaczanowski/BeagleBoneBlack-GPIO/blob/master/GPIO/GPIOConst.cpp
#define PIN_SR_LATCH      60    // P9_12, shift register latch pin
#define PIN_SR_DATA       30    // P9_11, shift register data pin
#define PIN_SR_CLOCK      31    // P9_13, shift register clock pin
#define PIN_SR_OE         50    // P9_14, shift register output enable pin
#define PIN_RAINSENSOR    48    // P9_15, rain sensor is connected to pin D3
#define PIN_FLOWSENSOR    48    // flow sensor (currently shared with rain sensor, change if using a different pin)
#define PIN_RF_DATA       51    // RF transmitter pin

#else
// For Linux or other software simulators
// use fake hardware pins
#if defined(DEMO)
#define OS_HW_VERSION 255   // assign hardware number 255 to DEMO firmware
#else
#define OS_HW_VERSION SIM_HW_VERSION_BASE
#endif
#define PIN_SR_LATCH    0
#define PIN_SR_DATA     0
#define PIN_SR_CLOCK    0
#define PIN_SR_OE       0
#define PIN_RAINSENSOR  0
#define PIN_FLOWSENSOR  0
#define PIN_RF_DATA     0

#endif

#define ETHER_BUFFER_SIZE   16384

#define DEBUG_BEGIN(x)          {}  /** Serial debug functions */
#define ENABLE_DEBUG
#if defined(ENABLE_DEBUG)
inline  void DEBUG_PRINT ( int x )
{
    printf ( "%d", x );
}
inline  void DEBUG_PRINT ( const char*s )
{
    printf ( "%s", s );
}
#define DEBUG_PRINTLN(x)        {DEBUG_PRINT(x);printf("\n");}
#else
#define DEBUG_PRINT(x) {}
#define DEBUG_PRINTLN(x) {}
#endif

inline void itoa ( int v,char *s,int b )
{
    sprintf ( s,"%d",v );
}
inline void ultoa ( unsigned long v,char *s,int b )
{
    sprintf ( s,"%lu",v );
}
#define now()       time(0)

/** Re-define avr-specific (e.g. PGM) types to use standard types */
#define pgm_read_byte(x) *(x)
#define PSTR(x)      x
#define strcat_P     strcat
#define strcpy_P     strcpy
#define PROGMEM
typedef const char prog_char;
typedef const char prog_uchar;
typedef const char* PGM_P;
typedef unsigned char   uint8_t;
typedef short           int16_t;
typedef unsigned short  uint16_t;
typedef bool boolean;

#endif  // end of Hardawre defines

#define TMP_BUFFER_SIZE     128  // scratch buffer size

/** Other defines */
// button values
#define BUTTON_1            0x01
#define BUTTON_2            0x02
#define BUTTON_3            0x04

// button status values
#define BUTTON_NONE         0x00  // no button pressed
#define BUTTON_MASK         0x0F  // button status mask
#define BUTTON_FLAG_HOLD    0x80  // long hold flag
#define BUTTON_FLAG_DOWN    0x40  // down flag
#define BUTTON_FLAG_UP      0x20  // up flag

// button timing values
#define BUTTON_DELAY_MS        1  // short delay (milliseconds)
#define BUTTON_HOLD_MS      1000  // long hold expiration time (milliseconds)

// button mode values
#define BUTTON_WAIT_NONE       0  // do not wait, return value immediately
#define BUTTON_WAIT_RELEASE    1  // wait until button is release
#define BUTTON_WAIT_HOLD       2  // wait until button hold time expires

#define DISPLAY_MSG_MS      2000  // message display time (milliseconds)

typedef unsigned char byte;
typedef unsigned long ulong;

#endif  // _DEFINES_H


