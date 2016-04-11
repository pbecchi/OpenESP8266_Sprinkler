#pragma once
/*------------------------------------------------------------------------------------------------------
The file details each pin used for your HW configuration. 
You have first to define the configuration you are working on (ESP or other MCU) 
and the type of peripheral you are using:see detailed table about it
Then you have to select the pins (for I2c, station output, shift registers, rain ....all other functions)
and assign it to a pin number, using following rule:
    -pin <32 will be assigned to MCU (ESP8266) GPIO pin numbers( only few are available for ESP8266).
    -pin >32 and <48 will be assigned to an first I2c extender (the one with the lowest address)
    -pin >48 and <64 will be assigned to second I2c extender 
    -so on 16 pin at the time
This way you can use expander for all functions (interrupt donot work right now) and free most of the pins on MCU
------------------------------------------------------------------------------------------------------------------*/
// ===================================================================================================================
// HARDWARE CONFIGURATIONS								GPIO channels                     #define
//
// RTC               DS1307, DS33xx						I2C channels					allways
// LCD               Standard ,							GPIO 5 channels					undef
//					 Freetronic,						?								def OPENSPRINKLER_ARDUINO_FREETRONICS_LCD
//					 I2C								I2C ch							def LCDI2C
// Station Output	 Std.Shift registers				GPIO 3 channels					def SHIFT_REG
//                   Shift register+PCF8574				PCF8574 4 IO					def SHIFT_REG+ I2C_SHIFT_REG		
//					 Discrete							8/16 channels					def OPENSPRINKLER_ARDUINO_DISCRETE
//					 I2C								I2C ch							def I2C_SHIFT_REG + OPENSPRINKLER_ARDUINO_DISCRETE
// Buttons           Std. 3 Dig.Inputs					GPIO 3 Channels					undef BUTTON_ADC_PIN
//					 analog Input						1 anal. chan.					def BUTTON_ADC_PIN
// SD                Std. SPI MicroSD					SPI channels +1					def SD_FAT	
//					 SPIFFS (emulations in Flash mem)	none							def SPIFFSDFAT
// EEPROM            Std. 2kB on board(Mega)			onboard							AUTOMATIC ON MEGA
//					 I2C on RTC board	4kB				I2C channels					def esp8266 undef EEPROM_ESP(+++ Comment out #define EEPROM_ESP in libsel.h)
//					 on ESP8266 flash mem				internal						def EEPROM_ESP (+++#define EEPROM_ESP in libsel.h)
//ETHERNET			 std ENC							SPI channels					undef OPENSPRINKLER_ARDUINO_DISCRETE
//					 W5100 SHIELD						SPI channels					def OPENSPRINKLER_ARDUINO_W5100
//					 ESP8266 onboard					none							ESP8266 & def OPENSPRINKLER_ARDUINO_W5100
//ADDITIONAL DEV,	 RF_REMOTE							GPIO 2 Channels					def ADDITIONAL_SENSORS
//					 FLUX_GAGE							GPIO 1 + interrupt				=MEGA_W5100   (ass.to mega GIO)
//																						=PCF8574_C(ass.to PCF8574&ESP8266)
//																						=ESP8266_C(ass.only to ESP8266)
//===============================================additional options==============================================================
//Hostname			you can access with http://NAME.local where name is OSxx=n.	HW		def HOST_NAM
//

#include <Arduino.h>
#include "Config.h"
////////////////////////////////////////BASIC ESP DEF //////////////////////////////
#ifdef ESP8266
#define SDA_PIN          D2       //:this is standard....it may be changed
#define SCL_PIN          D1       //:this is standard  ---it may be chaNGED
#define OS_HW_VERSION    0.0      // it will define releases
#else // not compatible libraries ESP8266 
#define SDFAT                                   // SD card on 
#define MY_PING                                 // ping not available on ESP8266
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PIN_BACKLIGHT_MODE POSITIVE
#define MEGA_W5100 1
#define ESP8266_C 2
#define PCF8574_C 3
// BUT_ON 0                           // define if button pins are switched to ground 0 or to Vcc 1
#define BUT1_ON 0
#define BUT2_ON 0
#define BUT3_ON 0
#include "libsel.h"
//
//                       PIN    ASSIGNEMENT
//
///////////////////
#define PROTO 3
///////////////////
//////////////////////////////proto board 1//////////////////////////////////////////////
#if PROTO==1 // shift register 

#define SDA_PIN D5                         //:redefined 
#define SCL_PIN D2                         //:redefined
//#define OPENSPRINKLER_ARDUINO_W5100      //:required for ESP8266
#undef OPENSPRINKLER_ARDUINO_DISCRETE      //:shift registers
#define SHIFT_REG
#define BUTTON_ADC_PIN		 A0            // A0 is the button ADC input				//:analog buttons
#define LCDI2C								//:i2c LCD
#define SPIFFSDFAT							//:no sd ....EMULATED ON fLASH
#define ADDITIONAL_SENSORS ESP8266_C         //:additional sensors to ESP 
#undef EEPROM_ESP                            //modify in libsel.h

/////////////////////////////proto board 2////////////////////////////////////////////////
#elif PROTO==2
#define PCF8574_M
//#define OPENSPRINKLER_ARDUINO_W5100         //:required for ESP8266 
#define OPENSPRINKLER_ARDUINO_DISCRETE      //:no shift registers
#define I2C_SHIFT_REG						//: stations on PCF8574 n.1...7  
//#define BUTTON_ADC_PIN        A0                   //:digital buttons ---> IO n.on PCF8574 n.0 pins: Ox00 <>0x02
#define LCDI2C								//: assign LCD address
#define SPIFFSDFAT							//:no SD
#define ADDITIONAL_SENSORS PCF8574_C        //:additional sensors on PCF8574 n.0 
////////////////////////////////////////////prtotype n.3 Ian Board///////////////////////////////////////////////////////
#elif PROTO==3
#define PCF8574_M
#define SDA_PIN 4
#define SCL_PIN 5
#define SHIFT_REG
#undef OPENSPRINKLER_ARDUINO_DISCRETE
#define LCDI2C
//#define SDFAT
//#define PIN_SD_CS 16
#define SPIFFSDFAT
#define EEPROM_ESP
#define PIN_BUTTON_1 0
#define PIN_BUTTON_2 2
#define PIN_BUTTON_3 15
#define BUT3_ON 1
#define I2C_SHIFT_REG
#define DUMMY_PIN 0x25  //dummy pin for unused functions
#define LCD_ADDR 0x20    // following are pin of the I2c lcd expander
#define PIN_LCD_RS        0    // LCD rs pin
#define PIN_LCD_RW        7    // LCD rw pin dummy decl.
#define PIN_LCD_EN        1    // LCD enable pin
#define PIN_LCD_D4        2    // LCD d4 pin
#define PIN_LCD_D5        3    // LCD d5 pin
#define PIN_LCD_D6        6    // LCD d6 pin
#define PIN_LCD_D7        5    // LCD d7 pin
#define PIN_LCD_BACKLIGHT 4    // LCD backlight pin
#define PIN_BACKLIGHT_MODE NEGATIVE //POSITIVE

#endif
//////check if libsel.h is correct
#ifdef EEPROM_ESP
//#undef EEPROM_ESP

#include "libsel.h"

#ifndef EEPROM_ESP
#error "file LIBSEL.H need to be corrected:uncomment #define"
#endif
#else
#include "libsel.h"
#ifdef EEPROM_ESP
#error "file LIBSEL.H need to be corrected:comment out #define"
#endif
#endif

#ifdef OPENSPRINKLER_ARDUINO_DISCRETE //no shift register used

#ifndef I2C_SHIFT_REG
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

//#define PIN_RF_DATA		30    // RF data pin - efault is 38
//#define PORT_RF			
//#define PINX_RF			
#else    //digital station pinout on PCF8574
#define PIN_EXT_BOARDS	1

#define PIN_STN_S01		0x20
#define PIN_STN_S02		0x21
#define PIN_STN_S03		0x22
#define PIN_STN_S04		0x23
#define PIN_STN_S05		0x24
#define PIN_STN_S06		0x25
#define PIN_STN_S07		0x26
#define PIN_STN_S08		0x27
#endif

#endif

// hardware pins
#if defined(SHIFT_REG)
#ifdef I2C_SHIFT_REG
// standard PCF8574 Pinout
#define PIN_SR_LATCH       0x22    // shift register latch pin
#define PIN_SR_DATA        0x24    // shift register data pin
#define PIN_SR_CLOCK       0x21    // shift register clock pin
#define PIN_SR_OE          0x23    // shift register output enable pin
#else
 //-----------------------ESP8266------------------------------------------
#define PIN_SR_LATCH       D3    // shift register latch pin
#define PIN_SR_DATA        D0    // shift register data pin
#define PIN_SR_CLOCK       10    // shift register clock pin
//#define PIN_SR_OE              // shift register output enable pin
#endif// END ESP8266--------------------------------------------------------------------------
#endif

//
//
////////////////////////////////////////////////////LCD/////////////////////////////////////////
//
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
#elif defined(LCDI2C)       //PINOUT of LCD BOARD not related to MCU pins
#ifndef LCD_ADDR   //already define in protoboard declarations
#define LCD_ADDR 0x27
#define PIN_LCD_RS        0    // LCD rs pin
#define PIN_LCD_RW        1    // LCD rw pin
#define PIN_LCD_EN        2    // LCD enable pin
#define PIN_LCD_D4        4    // LCD d4 pin
#define PIN_LCD_D5        5    // LCD d5 pin
#define PIN_LCD_D6        6    // LCD d6 pin
#define PIN_LCD_D7        7    // LCD d7 pin
#define PIN_LCD_BACKLIGHT 3    // LCD backlight pin
#endif
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
//
//////////////////////////////////////////////BUTTON PINS//////////////////////////
//
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
#else
#ifndef PIN_BUTTON_1
#define PIN_BUTTON_1   10 //  0x20// 31    // button 1
#define PIN_BUTTON_2   14 //  0x21// 30    // button 2
#define PIN_BUTTON_3   16//  0x22// 29    // button 3
#endif
#endif
//
//------------------------------------------------ NOT ENABLED----------------------------------
//
/*
#ifndef OPENSPRINKLER_ARDUINO_DISCRETE
// DC controller pin defines
#define PIN_BOOST         20    // booster pin
#define PIN_BOOST_EN      23    // boost voltage enable pin
#endif // OPENSPRINKLER_ARDUINO_DISCRETE
*/
//
//-----------------------------------------------------SPI PINS------------
//
#ifdef OPENSPRINKLER_ARDUINO_W5100 // Wiznet W5100
#define PIN_ETHER_CS     10     // Ethernet controller chip select pin - default is 10
#define PIN__CS        4     // SD card chip select pin - default is 4

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
//
///////////////////////////////////////////////////ADDITIONAL SENSORS PINS////////////////
//

#if ADDITIONAL_SENSORS==MEGA_W5100
// You'll need to use different pins for these items if you have them connected
#define PIN_RAINSENSOR     18    // rain sensor is connected to pin D3
#define PIN_FLOWSENSOR     19    // flow sensor (currently shared with rain sensor, change if using a different pin)
#define PIN_FLOWSENSOR_INT 20    // flow sensor interrupt pin (INT1)
#define PIN_EXP_SENSE      21    // expansion board sensing pin (A4)
#define PIN_CURR_SENSE     A3    // current sensing pin (A7)
#define PIN_CURR_DIGITAL   A3    // digital pin index for A7
#elif ADDITIONAL_SENSORS==ESP8266_C //OPENSPRINKLER ARDUINO DISCRETE
#define PIN_RAINSENSOR    D6    // rain sensor is connected to pin D3
#define PIN_FLOWSENSOR    D3   // flow sensor (currently shared with rain sensor, change if using a different pin)
#define PIN_FLOWSENSOR_INT 0    // flow sensor interrupt pin (INT1)
#define PIN_EXP_SENSE      D6    // expansion board sensing pin (A4)
#elif ADDITIONAL_SENSORS==PCF8574_C
#define PIN_RAINSENSOR    D6    // rain sensor is connected to pin D3
#define PIN_FLOWSENSOR    D5      // flow sensor (currently shared with rain sensor, change if using a different pin)
#define PIN_FLOWSENSOR_INT 5      // flow sensor interrupt pin (INT1)
#define PIN_EXP_SENSE      D6    // expansion board sensing pin (A4)
#define PIN_CURR_SENSE     A0    // current sensing pin (A7)
#define PIN_CURR_DIGITAL   A0    // digital pin index for A7

#else //additional sensors undefined ESP8266
#ifdef ESP8266
#define PIN_RAINSENSOR    DUMMY_PIN    // rain sensor is connected to pin D3
#define PIN_FLOWSENSOR    DUMMY_PIN    // flow sensor (currently shared with rain sensor, change if using a different pin)
#define PIN_FLOWSENSOR_INT DUMMY_PIN    // flow sensor interrupt pin (INT1)
#define PIN_EXP_SENSE      DUMMY_PIN    // expansion board sensing pin (A4)
#define PIN_CURR_SENSE     DUMMY_PIN    // current sensing pin (A7)
#define PIN_CURR_DIGITAL  DUMMY_PIN    // digital pin index for A7

#else //This is setting for Ray O.S. 

#define PIN_RAINSENSOR    11    // rain sensor is connected to pin D3
#define PIN_FLOWSENSOR    11    // flow sensor (currently shared with rain sensor, change if using a different pin)
#define PIN_FLOWSENSOR_INT 1    // flow sensor interrupt pin (INT1)
#define PIN_EXP_SENSE      4    // expansion board sensing pin (A4)
#define PIN_CURR_SENSE     7    // current sensing pin (A7)
#define PIN_CURR_DIGITAL  24    // digital pin index for A7


#define PIN_RF_DATA       28    // RF data pin
#define PORT_RF        PORTA
#define PINX_RF        PINA3
#endif 
#endif      //ESP8266

