/* 
	Editor: http://www.visualmicro.com
	        visual micro and the arduino ide ignore this code during compilation. this code is automatically maintained by visualmicro, manual changes to this file will be overwritten
	        the contents of the Visual Micro sketch sub folder can be deleted prior to publishing a project
	        all non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
	        note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: Generic ESP8266 Module, Platform=esp8266, Package=esp8266
*/

#ifndef _VSARDUINO_H_
#define _VSARDUINO_H_
#define __ESP8266_ESp8266__
#define __ESP8266_ESP8266__
#define __ets__
#define ICACHE_FLASH
#define F_CPU 80000000L
#define DEBUG_ESP_PORT Serial
#define DEBUG_ESP_OTA
#define DEBUG_ESP_UPDATER
#define ARDUINO 10608
#define ARDUINO_ESP8266_ESP01
#define ARDUINO_ARCH_ESP8266
#define ESP8266
#define __cplusplus 201103L
#undef __cplusplus
#define __cplusplus 201103L
#define __STDC__
#define __ARM__
#define __arm__
#define __inline__
#define __asm__(x)
#define __extension__
#define __ATTR_PURE__
#define __ATTR_CONST__
#define __volatile__


#define __ASM
#define __INLINE
#define __attribute__(noinline)

//#define _STD_BEGIN
//#define EMIT
#define WARNING
#define _Lockit
#define __CLR_OR_THIS_CALL
#define C4005
//
//typedef int uint8_t;
//#define __ARMCC_VERSION 400678
//#define PROGMEM
//#define string_literal
//
//#define prog_void
//#define PGM_VOID_P int
//

typedef int _read;
typedef int _seek;
typedef int _write;
typedef int _close;
typedef int __cleanup;

//#define inline 

#define __builtin_clz
#define __builtin_clzl
#define __builtin_clzll
#define __builtin_labs
#define __builtin_va_list
typedef int __gnuc_va_list;

#define __ATOMIC_ACQ_REL

#define __CHAR_BIT__
#define _EXFUN()

typedef unsigned char byte;
extern "C" void __cxa_pure_virtual() {;}

#include "new"

#include <arduino.h>
#include <common.h> 
#include <pins_arduino.h> 
#undef F
#define F(string_literal) ((const PROGMEM char *)(string_literal))
#undef PSTR
#define PSTR(string_literal) ((const PROGMEM char *)(string_literal))
//current vc++ does not understand this syntax so use older arduino example for intellisense
//todo:move to the new clang/gcc project types.
#define interrupts() sei()
#define noInterrupts() cli()

#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\OpenSprinkler_Arduino_W5100\Config.h>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\Defines.h>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\OpenSprinkler_Arduino_W5100\Eeprom_ESP.cpp>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\OpenSprinkler_Arduino_W5100\Eeprom_ESP.h>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\EtherCardW5100.cpp>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\EtherCardW5100.h>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\Gpio.cpp>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\Gpio.h>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\OpenSprinkler.cpp>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\OpenSprinkler.h>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\OpenSprinklerMain.cpp>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\OpenSprinklerProgram.cpp>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\OpenSprinklerProgram.h>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\OpenSprinklerServer.cpp>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\OpenSprinklerServer.h>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\OpenSprinkler_Arduino_W5100\OpenSprinkler_Arduino_W5100.ino>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\PCF8574Mio.cpp>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\PCF8574Mio.h>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\Pins.h>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\Utils.cpp>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\Utils.h>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\Weather.cpp>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\Weather.h>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\eeprom_mio.cpp>
#include <..\OpenSprinkler\OpenSprinkler_Arduino_V_2_1_6\eeprom_mio.h>
#endif
