/* OpenSprinkler Unified (AVR/RPI/BBB/LINUX) Firmware
 * Copyright (C) 2015 by Ray Wang (ray@opensprinkler.com)
 *
 * OpenSprinkler library header file
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

#ifndef _OPENSPRINKLER_H
#define _OPENSPRINKLER_H

#include "Config.h"
#include "../Defines.h"

#if defined(ARDUINO) // heades for AVR

	// OPENSPRINKLER_ARDUINO
	#include <Arduino.h>
#ifndef ESP8266
	#include <avr/eeprom.h>
#else
	#include "EEprom_mio.h"
#endif
	#include <Wire.h>
	#include <Time.h>
#ifdef LCDI2C
#include <LiquidCrystal_I2C.h>
#else
#include <LiquidCrystal.h>
#endif
	#include <DS1307RTC.h>

	#ifdef OPENSPRINKLER_ARDUINO_W5100
	  #include "../EtherCardW5100.h"
	#else
	  #include <EtherCard.h>
	#endif // OPENSPRINKLER_ARDUINO_W5100

	#ifdef OPENSPRINKLER_ARDUINO_FREEMEM
		#include <MemoryFree.h>
	#endif // OPENSPRINKLER_ARDUINO_FREEMEM

#else // headers for RPI/BBB/LINUX
  #include <time.h>
  #include <string.h>
  #include <unistd.h>
  #include "etherport.h"
#endif // end of headers

#include "../Utils.h"

/** Non-volatile data */
struct NVConData {
  uint16_t sunrise_time;  // sunrise time (in minutes)
  uint16_t sunset_time;   // sunset time (in minutes)
  uint32_t rd_stop_time;  // rain delay stop time
  uint32_t external_ip;   // external ip
};

/** Station special attribute data */
struct StationSpecialData {
  byte type;
  byte data[STATION_SPECIAL_DATA_SIZE];
};

/** Remote station data structure */
// this must fit in STATION_SPECIAL_DATA_SIZE
struct RemoteStationData {
  byte ip[4];
  uint16_t port;
  byte sid;
};

/** Volatile controller status bits */
struct ConStatus {
  byte enabled:1;           // operation enable (when set, controller operation is enabled)
  byte rain_delayed:1;      // rain delay bit (when set, rain delay is applied)
  byte rain_sensed:1;       // rain sensor bit (when set, it indicates that rain is detected)
  byte program_busy:1;      // HIGH means a program is being executed currently
  byte has_curr_sense:1;    // HIGH means the controller has a current sensing pin
  byte has_sd:1;            // HIGH means a microSD card is detected
  byte safe_reboot:1;       // HIGH means a safe reboot has been marked
  byte has_hwmac:1;         // has hardware MAC chip
  byte req_ntpsync:1;       // request ntpsync
  byte req_network:1;       // request check network
  byte display_board:3;     // the board that is being displayed onto the lcd
  byte network_fails:3;     // number of network fails
  byte mas:8;               // master station index
  byte mas2:8;              // master2 station index
};

// ===== OPENSPRINKLER_ARDUINO =====
// Moved from opensprinkler.cpp to here
//CHANGED
#ifdef PUFFA //OPENSPRINKLER_ARDUINO

// Option json names (stored in progmem)
// IMPORTANT: each json name is strictly 5 characters with 0 fillings if less
prog_char op_json_names[] /*PROGMEM*/ =
"fwv\0\0"
"tz\0\0\0"
"ntp\0\0"
"dhcp\0"
"ip1\0\0"
"ip2\0\0"
"ip3\0\0"
"ip4\0\0"
"gw1\0\0"
"gw2\0\0"
"gw3\0\0"
"gw4\0\0"
"hp0\0\0"
"hp1\0\0"
"hwv\0\0"
"ext\0\0"
"seq\0\0"
"sdt\0\0"
"mas\0\0"
"mton\0"
"mtof\0"
"urs\0\0"
"rso\0\0"
"wl\0\0\0"
"den\0\0"
"ipas\0"
"devid"
"con\0\0"
"lit\0\0"
"dim\0\0"
"bst\0\0"
"uwt\0\0"
"ntp1\0"
"ntp2\0"
"ntp3\0"
"ntp4\0"
"lg\0\0\0"
"mas2\0"
"mton2"
"mtof2"
"fwm\0\0"
"fpr0\0"
"fpr1\0"
"re\0\0\0"
"reset";

const char wtopts_filename[] /*PROGMEM*/ = WEATHER_OPTS_FILENAME;
const char stns_filename[]  /*PROGMEM*/ = STATION_ATTR_FILENAME;

/** Option maximum values (stored in progmem) */
prog_char op_max[] /*PROGMEM*/ = {
	0,
	108,
	1,
	1,
	255,
	255,
	255,
	255,
	255,
	255,
	255,
	255,
	255,
	255,
	0,
	MAX_EXT_BOARDS,
	1,
	247,
	MAX_NUM_STATIONS,
	60,
	120,
	255,
	1,
	250,
	1,
	1,
	255,
	255,
	255,
	255,
	250,
	255,
	255,
	255,
	255,
	255,
	1,
	MAX_NUM_STATIONS,
	60,
	120,
	0,
	255,
	255,
	1,
	1
};

#endif // ===== OPENSPRINKLER_ARDUINO =====

class OpenSprinkler {
public:

  // data members
#if defined(ARDUINO)

#ifdef LCDI2C
static	LiquidCrystal_I2C lcd;
#else
static	LiquidCristal lcd;
#endif

#else
  // todo: LCD define for RPI
#endif

#if defined(OSPI)
  static byte pin_sr_data;    // RPi shift register data pin
                              // to handle RPi rev. 1
#endif

  static NVConData nvdata;
  static ConStatus status;
  static ConStatus old_status;
  static byte nboards, nstations;
  static byte hw_type;            // hardware type

  static byte options[];		  // option values, max, name, and flag

  static byte station_bits[];     // station activation bits. each byte corresponds to a board (8 stations)
                                  // first byte-> master controller, second byte-> ext. board 1, and so on

#ifdef OPENSPRINKLER_ARDUINO_DISCRETE 
  static int station_pins[];
#endif // OPENSPRINKLER_ARDUINO_DISCRETE

  // variables for time keeping
  static ulong sensor_lasttime;  // time when the last sensor reading is recorded
  static ulong flowcount_time_ms;// time stamp when new flow sensor click is received (in milliseconds)
  static ulong flowcount_rt;     // flow count (for computing real-time flow rate)
  static ulong flowcount_log_start; // starting flow count (for logging)
  static ulong raindelay_start_time;  // time when the most recent rain delay started
  static byte  button_timeout;        // button timeout
  static ulong checkwt_lasttime;      // time when weather was checked
  static ulong checkwt_success_lasttime; // time when weather check was successful
  // member functions
  // -- 

  static void update_dev();   // update software for Linux instances
  static void reboot_dev();   // reboot the microcontroller
  static void begin();        // initialization, must call this function before calling other functions
  static byte start_network();  // initialize network with the given mac and port
#if defined(ARDUINO)
  static bool read_hardware_mac();  // read hardware mac address
#endif
  static time_t now_tz();
  // -- station names and attributes
  static void get_station_name(byte sid, char buf[]); // get station name
  static void set_station_name(byte sid, char buf[]); // set station name
  static uint16_t parse_rfstation_code(byte *code, ulong *on, ulong *off); // parse rf code into on/off/time sections
  static void switch_rfstation(byte *code, bool turnon);  // switch rf station
  static void switch_remotestation(byte *code, bool turnon); // switch remote station
  static void station_attrib_bits_save(int addr, byte bits[]); // save station attribute bits to nvm
  static void station_attrib_bits_load(int addr, byte bits[]); // load station attribute bits from nvm
  static byte station_attrib_bits_read(int addr); // read one station attribte byte from nvm

  // -- options and data storeage
  static void nvdata_load();
  static void nvdata_save();

  static void options_setup();
  static void options_load();
  static void options_save();

  static byte password_verify(char *pw);  // verify password

  // -- controller operation
  static void enable();           // enable controller operation
  static void disable();          // disable controller operation, all stations will be closed immediately
  static void raindelay_start();  // start raindelay
  static void raindelay_stop();   // stop rain delay
  static void rainsensor_status();// update rainsensor status
#if defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284__)
  static uint16_t read_current(); // read current sensing value
#endif
  static int detect_exp();        // detect the number of expansion boards
  static byte weekday_today();    // returns index of today's weekday (Monday is 0)

  static byte set_station_bit(byte sid, byte value); // set station bit of one station (sid->station index, value->0/1)
  static void switch_special_station(byte sid, byte value); // swtich special station
  static void clear_all_station_bits(); // clear all station bits
  static void apply_all_station_bits(); // apply all station bits (activate/deactive values)

  // -- LCD functions
#if defined(ARDUINO) // LCD functions for Arduino
  static void lcd_print_pgm(PGM_P /* PROGMEM*/ str);           // print a program memory string
  static void lcd_print_line_clear_pgm(PGM_P /*PROGMEM*/ str, byte line);
  static void lcd_print_time(time_t t);                  // print current time
  static void lcd_print_ip(const byte *ip, byte endian);    // print ip
  static void lcd_print_mac(const byte *mac);             // print mac
  static void lcd_print_station(byte line, char c);       // print station bits of the board selected by display_board
  static void lcd_print_version(byte v);                   // print version number

  // -- UI and buttons
  static byte button_read(byte waitmode); // Read button value. options for 'waitmodes' are:
                                          // BUTTON_WAIT_NONE, BUTTON_WAIT_RELEASE, BUTTON_WAIT_HOLD
                                          // return values are 'OR'ed with flags
                                          // check defines.h for details
#ifdef OPENSPRINKLER_ARDUINO_FREETRONICS_LCD 
  static byte button_sample();			  // new function to sample analog button input
#endif // OPENSPRINKLER_ARDUINO_FREETRONICS_LCD
#ifdef ESP8266 
  static byte button_sample();			  // new function to sample analog button input
#endif 

  // -- UI functions --
  static void ui_set_options(int oid);    // ui for setting options (oid-> starting option index)
  static void lcd_set_brightness(byte value=1);
  static void lcd_set_contrast();

#ifdef OPENSPRINKLER_ARDUINO_FREEMEM       // Added for debugging
  static void lcd_print_memory(byte line); // print current free memory and an animated character to show activity
#endif

private:
  static void lcd_print_option(int i);  // print an option to the lcd
  static void lcd_print_2digit(int v);  // print a integer in 2 digits
  static void lcd_start();
  static byte button_read_busy(byte pin_butt, byte waitmode, byte butt, byte is_holding);
#if defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284__)
  static byte engage_booster;
#endif
#endif // LCD functions
};

#endif  // _OPENSPRINKLER_H
