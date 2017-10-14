/* OpenSprinkler Unified (AVR/RPI/BBB/LINUX) Firmware
 * Copyright (C) 2015 by Ray Wang (ray@opensprinkler.com)
 *
 * Main loop
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

#ifndef OPENSPRINKLER_ARDUINO_WDT	// Only needed if not using WDT (to ensure the WDT is disbled)
#ifndef ESP8266
#include <avr/wdt.h>
#endif
#endif

#include <limits.h>
#include "Defines.h"
#include "OpenSprinkler.h"
#include "OpenSprinklerProgram.h"
#include "Weather.h"
#ifdef SG21
 #include "SensorGroup.h"
#endif
#if defined(ARDUINO)
#ifdef ESP8266
#ifndef SDFAT
 #include <FS.h>
//#include "SPIFFSdFat.h"
#else
#include <SD.h>
#include <SdFat.h>
SdFat sd;                                 // SD card object
#endif
#endif

#include <Wire.h>

#ifdef OPENSPRINKLER_ARDUINO_W5100
byte EtherCardW5100::buffer[ETHER_BUFFER_SIZE]; // Ethernet packet buffer
#else
byte Ethernet::buffer[ETHER_BUFFER_SIZE];		// Ethernet packet buffer
#endif
#ifdef OS217
void reset_all_stations();
void reset_all_stations_immediate();
void push_message(byte type, uint32_t lval = 0, float fval = 0.f, const char* sval = NULL);
void manual_start_program(byte, byte);
void httpget_callback(byte, uint16_t, uint16_t);
//_______________________________________________________________________________
#ifdef SG21
extern void flowsensor_ISR();
// @tcsaba: new functions
void check_sensors(ulong curr_time);
void check_network();
byte push_message_cloud(byte type, ulong day = 0);
int freeRam();
void make_logfile_name(char *name);
#endif
//____________________________________________________________________________________

#else
void reset_all_stations();

void manual_start_program ( byte pid );
#endif
#else // header and defs for RPI/BBB
#include <sys/stat.h>
#include "etherport.h"
#include "server.h"
#include "gpio.h"
char ether_buffer[ETHER_BUFFER_SIZE];
EthernetServer *m_server = 0;
EthernetClient *m_client = 0;
#endif
time_t getNtpTime();
// Small variations have been added to the timing values below
// to minimize conflicting events
#define NTP_SYNC_INTERVAL       86403L  // NYP sync interval, 24 hrs
#define RTC_SYNC_INTERVAL       60      // RTC sync interval, 60 secs
#define CHECK_NETWORK_INTERVAL  601     // Network checking timeout, 10 minutes
#define CHECK_WEATHER_TIMEOUT   3601    // Weather check interval: 1 hour
#define CHECK_WEATHER_SUCCESS_TIMEOUT 86433L // Weather check success interval: 24 hrs
#define LCD_BACKLIGHT_TIMEOUT   15      // LCD backlight timeout: 15 secs
#define PING_TIMEOUT            200     // Ping test timeout: 200 ms
//_______________________________________________________________________________
#ifdef SG21

// @tcsaba: cloud refresh intervals
#define CLOUD_SYNC_INTERVAL     60		// Cloud refresh
#define CLOUD_SYNC_FAST         10      // Cloud refresh fast
#endif
//________________________________________________________________________________

extern char tmp_buffer[];       // scratch buffer
BufferFiller bfill;             // buffer filler
#ifdef ESP8266
extern DS1307RTC RTC;
#endif
// ====== Object defines ======
OpenSprinkler os; // OpenSprinkler object
ProgramData pd;   // ProgramdData object
// _______________________________________________________________________________
#ifdef SG21

SensorGroup sensors;	//SensorGroup object

						// @tcsaba: variables 
uint16_t v;	// current measurement variables
byte today;
bool new_day = 0;

//LCD display multiplexing
ulong disp_cnt;
bool sensor_display = true;
#define SHOW_TIME 2000	  	//show time msec
#define SHOW_SENSOR 4000		//show sensors on LCD
#define SHOW_PULSES 1       //show flow pulses in 2nd LCD line, comment it out if not wanted

//Cloud sync variables
char client_pw[] = "OsClientTCS\0";
ulong  last_cloud_refresh = 0, cloud_refresh_period = 30, packet_delay = 2;
bool send_post = false, send_history = 0;
byte log_status = 0;

ulong last_sent_log = 0;
unsigned int log_end, log_to_send;
ulong log_rec_counter;
ulong millis_cnt, millis_cnt_2;


//#if defined(ARDUINO)
// @tcs: if DEBUG enabled, print to the serial monitor today or n days log records at startup
#if defined(SERIAL_DEBUG)
/*void print_today_log(ulong days) {
	ulong start, end;
	ulong file_size;
	end = os.now_tz() / 86400L;
	 		start = end - days;
	for (int i = start; i <= end; i++) {
		itoa(i, tmp_buffer, 10);

		DEBUG_PRINTLN(tmp_buffer);
		make_logfile_name(tmp_buffer);

		DEBUG_PRINT("\nFilename:   ");
		DEBUG_PRINTLN(tmp_buffer);

		if (!sd.exists(tmp_buffer)) continue;
		SdFile file;
		file.open(tmp_buffer, O_READ);
		file_size = file.fileSize();

		int res;
		while (true) {
			res = file.fgets(tmp_buffer, TMP_BUFFER_SIZE);
			DEBUG_PRINT(tmp_buffer);
			DEBUG_PRINT("\r");
			last_sent_log++;
			if (res <= 0) {
				file.close();
				break;
			}
		}
		DEBUG_PRINT("No of logs:  /  File_size: ");
		DEBUG_PRINT(last_sent_log);
		DEBUG_PRINT(" / ");
		DEBUG_PRINTLN(file_size);

		last_sent_log = 0;
	}
}*/
#endif

// ====== UI defines ======
#else
#ifdef OS217
/* ====== Robert Hillman (RAH)'s implementation of flow sensor ======
* flow_begin - time when valve turns on
* flow_start - time when flow starts being measured (i.e. 2 mins after flow_begin approx
* flow_stop - time when valve turns off (last rising edge pulse detected before off)
* flow_gallons - total # of gallons+1 from flow_start to flow_stop
* flow_last_gpm - last flow rate measured (averaged over flow_gallons) from last valve stopped (used to write to log file). */
ulong flow_begin, flow_start, flow_stop, flow_gallons;
float flow_last_gpm = 0;

volatile ulong flow_count = 0;
/** Flow sensor interrupt service routine */
void flow_isr() {
	if (os.options[OPTION_SENSOR_TYPE] != SENSOR_TYPE_FLOW) return;
	ulong curr = millis();

	if (curr - os.flowcount_time_ms < 50) return;  // debounce threshold: 50ms
	flow_count++;
	os.flowcount_time_ms = curr;

	/* RAH implementation of flow sensor */
	if (flow_start == 0) { flow_gallons = 0; flow_start = curr; }  // if first pulse, record time
	if ((curr - flow_start)<90000) { flow_gallons = 0; } // wait 90 seconds before recording flow_begin
	else { if (flow_gallons == 1) { flow_begin = curr; } }
	flow_stop = curr; // get time in ms for stop
	flow_gallons++;  // increment gallon count for each interrupt
					 /* End of RAH implementation of flow sensor */
}

#else

//_____________________________________________________________________________________________________

volatile ulong flow_count = 0;
/** Flow sensor interrupt service routine */
void flow_isr()
{
    if ( os.options[OPTION_SENSOR_TYPE]!=SENSOR_TYPE_FLOW ) return;
    ulong curr = millis();
    if ( curr-os.flowcount_time_ms < 50 ) return; // debounce threshold: 50ms
    flow_count++;
    os.flowcount_time_ms = curr;
}
#endif //OS217
#endif //SG21
#undef DB_MASK
#define DB_MASK 2
#if defined(ARDUINO)
// ====== UI defines ======
static char ui_anim_chars[3] = {'.', 'o', 'O'};

#define UI_STATE_DEFAULT   0
#define UI_STATE_DISP_IP   1
#define UI_STATE_DISP_GW   2
#define UI_STATE_RUNPROG   3

static byte ui_state = UI_STATE_DEFAULT;
static byte ui_state_runprog = 0;
static int iline = 2;
void ui_state_machine()
{

    if ( !os.button_timeout )
    {

		iline = 2;
        os.lcd_set_brightness ( 0 );
        ui_state = UI_STATE_DEFAULT;  // also recover to default state
    }

    // read button, if something is pressed, wait till release
    byte button = os.button_read ( BUTTON_WAIT_HOLD );
	//DEBUG_PRINTLN(button);
  if (button & BUTTON_FLAG_DOWN) {   // repond only to button down events
        os.button_timeout = LCD_BACKLIGHT_TIMEOUT;
        os.lcd_set_brightness ( 1 );
  } else {
        return;
    }

  switch(ui_state) {
    case UI_STATE_DEFAULT:
    switch (button & BUTTON_MASK) {
        case BUTTON_1:
            if ( button & BUTTON_FLAG_HOLD ){  // holding B1: stop all stations
            

#ifndef BUTTON_ADC_PIN
                
                if ( digitalRead ( PIN_BUTTON_3 ) ==0 ){ // if B3 is pressed while holding B1, run a short test (internal test)
                
#ifdef SG21
					manual_start_program(255, 0);
#else
					manual_start_program(255);
#endif       
				} else if (digitalRead(PIN_BUTTON_2)==0) { // if B2 is pressed while holding B1, display gateway IP
                    os.lcd_print_ip ( ether.gwip, 0 );
                    os.lcd.setCursor ( 0, 1 );
                    os.lcd_print_pgm ( PSTR ( "(gwip)" ) );
                    ui_state = UI_STATE_DISP_IP;
        } else {
#endif
                    reset_all_stations();
                }

      } else {  // clicking B1: display device IP and port

                os.lcd_print_ip ( ether.myip, 0 );
                os.lcd.setCursor ( 0, 1*YFACTOR );
                os.lcd_print_pgm ( PSTR ( ":" ) );
                os.lcd.print ( ether.hisport );
                os.lcd_print_pgm ( PSTR ( " (osip)" ) );
                ui_state = UI_STATE_DISP_IP;
            }
            break;
        case BUTTON_2:
            if ( button & BUTTON_FLAG_HOLD )  // holding B2: reboot
            {

#ifndef BUTTON_ADC_PIN
    
                if ( digitalRead ( PIN_BUTTON_1 ) ==0 ){ // if B1 is pressed while holding B2, display external IP
                
					os.lcd_print_ip((byte*)(&os.nvdata.external_ip), 1);
                    os.lcd.setCursor ( 0, 1 );
                    os.lcd_print_pgm ( PSTR ( "(eip)" ) );
                    ui_state = UI_STATE_DISP_IP;
        } else if (digitalRead(PIN_BUTTON_3)==0) {  // if B3 is pressed while holding B2, display last successful weather call
          os.lcd.clear();
                    os.lcd_print_time ( os.checkwt_success_lasttime );
                    os.lcd.setCursor ( 0, 1 );
                    os.lcd_print_pgm ( PSTR ( "(lswc)" ) );
                    ui_state = UI_STATE_DISP_IP;
                } else {
#endif
                    os.reboot_dev();
                }

      } else {  // clicking B2: display MAC and gate way IP
        os.lcd_clear();
                os.lcd_print_mac ( ether.mymac );
                ui_state = UI_STATE_DISP_GW;
            }
            break;

        case BUTTON_3:
      if (button & BUTTON_FLAG_HOLD) {  // holding B3: go to main menu
                os.lcd_print_line_clear_pgm ( PSTR ( "Run a Program:" ), 0 );
                os.lcd_print_line_clear_pgm ( PSTR ( "Click B3 to list" ), 1 );
                ui_state = UI_STATE_RUNPROG;
            } else {// clicking B3: switch board display (cycle through master and all extension boards)
            
#ifdef MESSAGE 
				print_message(-iline++);
				delay(2000);
		
			//	while (button_read(BUTTON_WAIT_NONE)&BUTTON_MASK == 2)
			//	{
			//		print_message(-ii++ );
			//		delay(2000);
			//	}
#else
                os.status.display_board = ( os.status.display_board + 1 ) % ( os.nboards );
#endif
			}
            break;
        }
        break;
    case UI_STATE_DISP_IP:
    case UI_STATE_DISP_GW:
        ui_state = UI_STATE_DEFAULT;
        break;
    case UI_STATE_RUNPROG:
    if ((button & BUTTON_MASK)==BUTTON_3) {
      if (button & BUTTON_FLAG_HOLD) {
                // start
#ifdef SG21
        manual_start_program(ui_state_runprog, 0);
#else
		  manual_start_program(ui_state_runprog);
#endif
		  ui_state = UI_STATE_DEFAULT;
      } else {
                ui_state_runprog = ( ui_state_runprog+1 ) % ( pd.nprograms+1 );
                os.lcd_print_line_clear_pgm ( PSTR ( "Hold B3 to start" ), 0 );
        if(ui_state_runprog > 0) {
                    ProgramStruct prog;
                    pd.read ( ui_state_runprog-1, &prog );
                    os.lcd_print_line_clear_pgm ( PSTR ( " " ), 1 );
                    os.lcd.setCursor ( 0, 1 );
                    os.lcd.print ( ( int ) ui_state_runprog );
                    os.lcd_print_pgm ( PSTR ( ". " ) );
                    os.lcd.print ( prog.name );
        } else {
                    os.lcd_print_line_clear_pgm ( PSTR ( "0. Test (1 min)" ), 1 );
                }
            }
        }
        break;
    }
}

// ======================
// Setup Function
// ======================
void do_setup()
{

#ifdef OPENSPRINKLER_ARDUINO_WDT // Turn WDT on or off
    /* Clear WDT reset flag. */
    MCUSR &= ~ ( 1<<WDRF );
#else
#ifndef ESP8266
	wdt_disable();
#endif
#endif // OPENSPRINKLER_ARDUINO_WDT

#ifdef OPENSPRINKLER_ARDUINO_W5100
    // This code is needed to handle multiple SPI devices - all the CS pins for each SPI device
    // need to be pulled high to avoid interefering with each other (ethernet, SD, RF24 etc)
    for ( int i = 0; i < SPI_DEVICES; i++ )
    {
        pinMode ( spi_ss_pin[i], OUTPUT );
        digitalWrite ( spi_ss_pin[i], HIGH );
    }
#endif

#ifdef OPENSPRINKLER_ARDUINO
    DEBUG_BEGIN ( 115200 );
#else
	DEBUG_BEGIN ( 9600 );
#endif

    DEBUG_PRINTLN ( "Started..." );
    os.begin();          // OpenSprinkler init
	DEBUG_PRINTLN("OptSetup...");
    os.options_setup();  // Setup options
	//______________________________________________________________________________________
#ifdef SG21
		 // @tcs:Set up sensors moved from OpenSprinkler.cpp by CV, see in SensorGroup.cpp
	sensors.init();

	// @tcs: we set the ISR only if we have a flow sensor
	if (os.options[OPTION_FSENSOR_TYPE] != SENSOR_TYPE_NONE) {
		attachInterrupt(PIN_FLOWSENSOR_INT, flowsensor_ISR, FALLING);
	}
#endif
//	_______________________________________________________________________________________
	DEBUG_PRINTLN("init...");
    pd.init();            // ProgramData init
	DEBUG_PRINTLN("clock...");
    setSyncInterval ( RTC_SYNC_INTERVAL ); // RTC sync interval
    // if rtc exists, sets it as time sync source
 

	setSyncProvider(RTC.get);
	DEBUG_PRINT(hour()); DEBUG_PRINT(":"); DEBUG_PRINTLN(minute());

    os.lcd_print_time ( os.now_tz() ); // display time to LCD
	DEBUG_PRINTLN("Time printed...");
#ifdef OPENSPRINKLER_ARDUINO_HEARTBEAT
    pinMode ( PIN_HEARTBEAT, OUTPUT );
#endif // OPENSPRINKLER_ARDUINO_HEARTBEAT

#ifdef OPENSPRINKLER_ARDUINO_WDT
    // enable WDT
    /* In order to change WDE or the prescaler, we need to
     * set WDCE (This will allow updates for 4 clock cycles).
     */
    WDTCSR |= ( 1<<WDCE ) | ( 1<<WDE );
    /* set new watchdog timeout prescaler value */
    WDTCSR = 1<<WDP3 | 1<<WDP0;  // 8.0 seconds
    /* Enable the WD interrupt (note no reset). */
    WDTCSR |= _BV ( WDIE );
#endif // OPENSPRINKLER_ARDUINO_WDT 

    if ( os.start_network() )  // initialize network
    {
        os.status.network_fails = 0;
    }
    else
    {
        os.status.network_fails = 1;
    }
    os.status.req_network = 0;
    os.status.req_ntpsync = 1;

    os.apply_all_station_bits(); // reset station bits
	os.button_timeout = LCD_BACKLIGHT_TIMEOUT;
	DEBUG_PRINTLN("Setup complete");
#ifdef SG21
	today = os.weekday_today(); //@tcs: set today weekday
	
	#if defined(SERIAL_DEBUG)  
   ////t  print_today_log(0);  //@tcs: give how many days' log records want to be listed at reset
  #endif
#endif
	delay(4000);
}

// Arduino software reset function
void ( * sysReset ) ( void ) = 0;

#ifdef OPENSPRINKLER_ARDUINO_WDT
volatile byte wdt_timeout = 0;
/** WDT interrupt service routine */
ISR ( WDT_vect )
{
    wdt_timeout += 1;
    // this isr is called every 8 seconds
  if (wdt_timeout > 15) {
        // reset after 120 seconds of timeout
        sysReset();
    }
}
#endif // OPENSPRINKLER_ARDUINO_WDT 

#else

void do_setup() { //setup OsPi and OSBO
    initialiseEpoch();   // initialize time reference for millis() and micros()
    os.begin();          // OpenSprinkler init
    os.options_setup();  // Setup options

    pd.init();            // ProgramData init

    if ( os.start_network() )  // initialize network
    {
        DEBUG_PRINTLN ( "network established." );
        os.status.network_fails = 0;
    }
    else
    {
        DEBUG_PRINTLN ( "network failed." );
        os.status.network_fails = 1;
    }
    os.status.req_network = 0;
}
#endif
#ifdef SG21
 void write_log(byte type, ulong curr_time, ulong param);
#else
 void write_log ( byte type, ulong curr_time );
#endif
 void schedule_all_stations ( ulong curr_time );
void turn_off_station ( byte sid, ulong curr_time );
void process_dynamic_events ( ulong curr_time );
void check_network();
void check_weather();
void perform_ntp_sync();
void delete_log ( char *name );
void handle_web_request ( char *p );

/** Main Loop */
#define LOGDATA_BATTERY 5
#define LOGDATA_BATTERYV 6
static unsigned long milli_logBattery = 0;
void do_loop()
{
#ifdef LCD_SSD1306
	//os.ShowNet();
#ifdef BATTERY
	int volts=os.BatteryVoltage();
	os.Sleep(volts);
	//delay(100);
#endif
#endif

    static ulong last_time = 0;
    static ulong last_minute = 0;

    byte bid, sid, s, pid, qid, bitvalue;
    ProgramStruct prog;

    os.status.mas = os.options[OPTION_MASTER_STATION];
    os.status.mas2= os.options[OPTION_MASTER_STATION_2];
    time_t curr_time = os.now_tz();
#ifdef BATTERY
	if (millis() > milli_logBattery) {
#ifdef INA219
		write_log(LOGDATA_BATTERY, curr_time); 
#endif
		unsigned long millisdelay = 900000UL;								// during day 15 min interval
		if (hour() > 20) { if (hour() - 20 < 10) millisdelay= 3600000UL; }
		else if (20 + 10> 24) if (hour() < 20 + 10 - 24)millisdelay = 3600000UL; //during nigth 60 min interval
		milli_logBattery = millis() + millisdelay;
		write_log(LOGDATA_BATTERYV, curr_time);
	}
#endif
																			 // ====== Process Ethernet packets ======
#if defined(ARDUINO)  // Process Ethernet packets for Arduino
    uint16_t pos=ether.packetLoop ( ether.packetReceive() );
    if ( pos>0 ) { // packet received
#ifdef OPENSPRINKLER_ARDUINO_W5100
        handle_web_request ( ( char* ) EtherCardW5100::buffer + pos );
#else
        handle_web_request ( ( char* ) Ethernet::buffer+pos );
#endif
    }
//	DEBUG_PRINT('-'); DEBUG_PRINT(__LINE__);
#ifdef OPENSPRINKLER_ARDUINO_WDT
    wdt_reset();  // reset watchdog timer
    wdt_timeout = 0;
#endif // OPENSPRINKLER_ARDUINO_WDT 

    ui_state_machine();

#else // Process Ethernet packets for RPI/BBB
    EthernetClient client = m_server->available();
  if (client) {
    while(true) {
            int len = client.read ( ( uint8_t* ) ether_buffer, ETHER_BUFFER_SIZE );
      if (len <=0) {
        if(!client.connected()) {
                    break;
        } else {
                    continue;
                }
      } else {
                m_client = &client;
                ether_buffer[len] = 0;  // put a zero at the end of the packet
                handle_web_request ( ether_buffer );
                m_client = 0;
                break;
            }
        }
    }
#endif  // Process Ethernet packets

    // if 1 second has passed
    if ( last_time != curr_time ) {
        last_time = curr_time;
        if ( os.button_timeout ) os.button_timeout--;
		//DEBUG_PRINT("+&");
#if defined(ARDUINO)
        if ( !ui_state )
            os.lcd_print_time ( os.now_tz() );    // print time
#endif

        // ====== Check raindelay status ======
        if ( os.status.rain_delayed ){
            if ( curr_time >= os.nvdata.rd_stop_time ) { // rain delay is over
                os.raindelay_stop();
            }
    } else {
      if (os.nvdata.rd_stop_time > curr_time) {   // rain delay starts now
                os.raindelay_start();
            }
        }
        // ====== Check controller status changes and write log ======
    if (os.old_status.rain_delayed != os.status.rain_delayed) {
      if (os.status.rain_delayed) {
                // rain delay started, record time
                os.raindelay_start_time = curr_time;
//_________________________________________________________________________________________________
#ifdef SG21
		 write_log(LOGDATA_RAINDELAY2, curr_time, 0);
////t	  push_message(IFTTT_RAINSENSOR, LOGDATA_RAINDELAY, 1);
	  } else {
		  // rain delay stopped, write log
		  write_log(LOGDATA_RAINDELAY, curr_time, 0);
		  write_log(LOGDATA_RAINDELAY2, curr_time, 0);
////t	 push_message(IFTTT_RAINSENSOR, LOGDATA_RAINDELAY, 0);
#else
//	 push_message(IFTTT_RAINSENSOR, LOGDATA_RAINDELAY, 1);
	  }	  else {
		  write_log(LOGDATA_RAINDELAY, curr_time);
#endif
//___________________________________________________________________________________________

            }
            os.old_status.rain_delayed = os.status.rain_delayed;
        }
#ifndef SG21
        // ====== Check rain sensor status ======
	if (os.options[OPTION_SENSOR_TYPE] == SENSOR_TYPE_RAIN) // if a rain sensor is connected
	{
		os.rainsensor_status();
		if (os.old_status.rain_sensed != os.status.rain_sensed)
		{
			if (os.status.rain_sensed)
			{
				// rain sensor on, record time
				os.sensor_lasttime = curr_time;
			}
			else
			{
				// rain sensor off, write log
				if (curr_time > os.sensor_lasttime + 10)  // add a 10 second threshold
				{
					// to avoid faulty rain sensors generating
					// too many log records
					write_log(LOGDATA_RAINSENSE, curr_time);
				}
			}
			os.old_status.rain_sensed = os.status.rain_sensed;
		}
#ifdef OS217
		// ===== Check program switch status =====
	if (  os.programswitch_status(curr_time)) {
      reset_all_stations_immediate(); // immediately stop all stations
      if(pd.nprograms > 0)  manual_start_program(1, 0);
#endif	
		}
#endif
		// ====== Schedule program data ======
        ulong curr_minute = curr_time / 60;
        boolean match_found = false;
        RuntimeQueueStruct *q;
        // since the granularity of start time is minute
        // we only need to check once every minute
		//DEBUG_PRINT(__LINE__); DEBUG_PRINT("_");
    if (curr_minute != last_minute) {
            last_minute = curr_minute;
#ifdef SG21
		    // ====== if a day has been changed send end of the day log if no program running =====
		  // ====== If a program is running the final statistics and day flow will be logged on the new date.=====
				
				if (today != os.weekday_today()) {  //new day?
				if (!new_day) {					//do regular new day functions here
					os.options[OPTION_SEND_LOGFILES] = 1;  //send the previous day log again
					log_rec_counter = 0;
					sensors.alarm_cnt = 0;
					new_day = true;
				}
				
					if (!os.status.program_busy) {    //if program running we wait till finish to save the program and daily flow
					sensors.day_flow_calc(curr_time);	// logs all day flow log
					today = os.weekday_today();			//set today identifier
					new_day = false;
					
				}
			}
#endif
//_____________________________________________________________________________

            // check through all programs
            for ( pid=0; pid<pd.nprograms; pid++ ){
                pd.read ( pid, &prog );
       if(prog.check_match(curr_time)) {
//___________________________________________________________________________________
#ifdef SG21
			      // if soil sensor attached to the current program, and the soil is wet, skip schedule watering.
					if (prog.attach_soil_sensor_1 && !os.status.dry_soil_1)
						 write_log(LOGDATA_SOIL1_PROG_CANCEL, curr_time, pid);
					else if (prog.attach_soil_sensor_2 && !os.status.dry_soil_2)
						 write_log(LOGDATA_SOIL2_PROG_CANCEL, curr_time, pid);
					else {
#endif
						//_____________________________________________________________________________________

											// program match found
											// process all selected stations
          for(sid=0;sid<os.nstations;sid++) {
							bid = sid >> 3;
							s = sid & 0x07;
							// skip if the station is a master station (because master cannot be scheduled independently
							if ((os.status.mas == sid + 1) || (os.status.mas2 == sid + 1))
								continue;

							// if station has non-zero water time and the station is not disabled
            if (prog.durations[sid] && !(os.station_attrib_bits_read(ADDR_NVM_STNDISABLE+bid)&(1<<s))) {
								// water time is scaled by watering percentage
								ulong water_time = water_time_resolve(water_time_decode(prog.durations[sid]));
								// if the program is set to use weather scaling
              if (prog.use_weather) {
									byte wl = os.options[OPTION_WATER_PERCENTAGE];
									water_time = water_time * wl / 100;
									if (wl < 20 && water_time < 10) // if water_percentage is less than 20% and water_time is less than 10 seconds
										// do not water
										water_time = 0;
								}

              if (water_time) {
									// check if water time is still valid
									// because it may end up being zero after scaling
									q = pd.enqueue();
                if (q) {
										q->st = 0;
										q->dur = water_time;
										q->sid = sid;
										q->pid = pid + 1;
										match_found = true;
                } else {
										// queue is full
									}
								}// if water_time
							}// if prog.durations[sid]
						}// for  sid
#ifdef SG21
					}
#endif
                }// if check_match
            }// for pid
			//DEBUG_PRINT(__LINE__); DEBUG_PRINT("_");

            // calculate start and end time
      if (match_found) {
                schedule_all_stations ( curr_time );

                // For debugging: print out queued elements
                DEBUG_PRINT ( "en:" );
                for ( q=pd.queue; q<pd.queue+pd.nqueue; q++ )
                {
                    DEBUG_PRINT ( "[" );
                    DEBUG_PRINT ( q->sid );
                    DEBUG_PRINT ( "," );
                    DEBUG_PRINT ( q->dur );
                    DEBUG_PRINT ( "," );
                    DEBUG_PRINT ( q->st );
                    DEBUG_PRINT ( "]" );
                }
                DEBUG_PRINTLN ( "" );
            }
        }//if_check_current_minute

        // ====== Run program data ======
        // Check if a program is running currently
        // If so, do station run-time keeping
    if (os.status.program_busy){
            // first, go through run time queue to assign queue elements to stations
            q = pd.queue;
            qid=0;
      for(;q<pd.queue+pd.nqueue;q++,qid++) {
                sid=q->sid;
                byte sqi=pd.station_qid[sid];
                // skip if station is already assigned a queue element
                // and that queue element has an earlier start time
                if ( sqi<255 && pd.queue[sqi].st<q->st ) continue;
                // otherwise assign the queue element to station
                pd.station_qid[sid]=qid;
            }
            // next, go through the stations and perform time keeping
      for(bid=0;bid<os.nboards; bid++) {
                bitvalue = os.station_bits[bid];
        for(s=0;s<8;s++) {
                    byte sid = bid*8+s;

                    // skip master station
                    if ( os.status.mas == sid+1 ) continue;
                    if ( os.status.mas2== sid+1 ) continue;
                    if ( pd.station_qid[sid]==255 ) continue;

                    q = pd.queue + pd.station_qid[sid];
                    // check if this station is scheduled, either running or waiting to run
          if (q->st > 0) {
                        // if so, check if we should turn it off
            if (curr_time >= q->st+q->dur) {
                            turn_off_station ( sid, curr_time );
#ifdef SG21                      
      			  sensors.station_stopped(sid);
#endif
						}
                    }
                    // if current station is not running, check if we should turn it on
          if(!((bitvalue>>s)&1)) {
            if (curr_time >= q->st && curr_time < q->st+q->dur) {

                            //turn_on_station(sid);
                            os.set_station_bit ( sid, 1 );
#ifdef SG21
      			  sensors.station_started(sid);
#endif
                        } //if curr_time > scheduled_start_time
                    } // if current station is not running
                }//end_s
            }//end_bid

            // finally, go through the queue again and clear up elements marked for removal
            int qi;
      for(qi=pd.nqueue-1;qi>=0;qi--) {
                q=pd.queue+qi;
        if(!q->dur || curr_time>=q->st+q->dur)  {
                    pd.dequeue ( qi );
                }
            }

            // process dynamic events
            process_dynamic_events ( curr_time );

            // activate / deactivate valves
            os.apply_all_station_bits();

            // check through runtime queue, calculate the last stop time of sequential stations
            pd.last_seq_stop_time = 0;
            ulong sst;
            byte re=os.options[OPTION_REMOTE_EXT_MODE];
            q = pd.queue;
            for ( ; q<pd.queue+pd.nqueue; q++ )
            {
                sid = q->sid;
                bid = sid>>3;
                s = sid&0x07;
                // check if any sequential station has a valid stop time
                // and the stop time must be larger than curr_time
                sst = q->st + q->dur;
        if (sst>curr_time) {
                    // only need to update last_seq_stop_time for sequential stations
          if (os.station_attrib_bits_read(ADDR_NVM_STNSEQ+bid)&(1<<s) && !re) {
                        pd.last_seq_stop_time = ( sst>pd.last_seq_stop_time ) ? sst : pd.last_seq_stop_time;
                    }
                }
            }

            // if the runtime queue is empty
            // reset all stations
      if (!pd.nqueue) {
                // turn off all stations
                os.clear_all_station_bits();
                os.apply_all_station_bits();
#ifdef SG21
        sensors.program_stopped();
#endif
		// reset runtime
                pd.reset_runtime();
                // reset program busy bit
                os.status.program_busy = 0;
                // log flow sensor reading if flow sensor is used
                if ( os.options[OPTION_SENSOR_TYPE]==SENSOR_TYPE_FLOW ) {
          ////t          write_log ( LOGDATA_FLOWSENSE, curr_time );
                }

                // in case some options have changed while executing the program
                os.status.mas = os.options[OPTION_MASTER_STATION]; // update master station
                os.status.mas2= os.options[OPTION_MASTER_STATION_2]; // update master2 station
            }
        }//if_some_program_is_running

        // handle master
        if ( os.status.mas>0 ){
            byte mas_on_adj = os.options[OPTION_MASTER_ON_ADJ]; ////t
            byte mas_off_adj= os.options[OPTION_MASTER_OFF_ADJ];
            byte masbit = 0;
            os.station_attrib_bits_load ( ADDR_NVM_MAS_OP, ( byte* ) tmp_buffer ); // tmp_buffer now stores masop_bits
      for(sid=0;sid<os.nstations;sid++) {
                // skip if this is the master station
                if ( os.status.mas == sid+1 ) continue;
                bid = sid>>3;
                s = sid&0x07;
                // if this station is running and is set to activate master
        if ((os.station_bits[bid]&(1<<s)) && (tmp_buffer[bid]&(1<<s))) {
                    q=pd.queue+pd.station_qid[sid];
                    // check if timing is within the acceptable range
                    if ( curr_time >= q->st + mas_on_adj &&
              curr_time <= q->st + q->dur + mas_off_adj) {
                        masbit = 1;
                        break;
                    }
                }
            }
            os.set_station_bit ( os.status.mas-1, masbit );
        }
        // handle master2
        if ( os.status.mas2>0 ) {
            byte mas_on_adj_2 = os.options[OPTION_MASTER_ON_ADJ_2];////t
            byte mas_off_adj_2= os.options[OPTION_MASTER_OFF_ADJ_2];
            byte masbit2 = 0;
            os.station_attrib_bits_load ( ADDR_NVM_MAS_OP_2, ( byte* ) tmp_buffer ); // tmp_buffer now stores masop2_bits
      for(sid=0;sid<os.nstations;sid++) {
                // skip if this is the master station
                if ( os.status.mas2 == sid+1 ) continue;
                bid = sid>>3;
                s = sid&0x07;
                // if this station is running and is set to activate master
        if ((os.station_bits[bid]&(1<<s)) && (tmp_buffer[bid]&(1<<s))) {
                    q=pd.queue+pd.station_qid[sid];
                    // check if timing is within the acceptable range
                    if ( curr_time >= q->st + mas_on_adj_2 &&
                            curr_time <= q->st + q->dur + mas_off_adj_2 - 60 )
                    {
                        masbit2 = 1;
                        break;
                    }
                }
            }
            os.set_station_bit ( os.status.mas2-1, masbit2 );
        }

        // process dynamic events
#ifdef SG21
	v=0;
	for (sid = 0; sid <= MAX_EXT_BOARDS; sid++) {
		v += os.station_bits[sid];
	}
	if(v==0) {
		os.current_offset = analogRead(PIN_CURR_SENSE);
	}

	// call the loop of the sensor group
	sensors.loop(curr_time);
#endif
        process_dynamic_events ( curr_time );
		//DEBUG_PRINT(__LINE__); DEBUG_PRINT("_");

        // activate/deactivate valves
        os.apply_all_station_bits();

#if defined(ARDUINO)
        // process LCD display
        if ( !ui_state )
#ifdef OPENSPRINKLER_ARDUINO_FREEMEM
            os.lcd_print_memory ( 1 );
#else
            os.lcd_print_station ( 1, ui_anim_chars[curr_time % 3] );
#endif // OPENSPRINKLER_ARDUINO_FREEMEM
		

        // check safe_reboot condition
        if ( os.status.safe_reboot )
        {
            // if no program is running at the moment
            if ( !os.status.program_busy )
            {
                // and if no program is scheduled to run in the next minute
                bool willrun = false;
                for ( pid=0; pid<pd.nprograms; pid++ )
                {
                    pd.read ( pid, &prog );
                    if ( prog.check_match ( curr_time+60 ) )
                    {
                        willrun = true;
                        break;
                    }
                }
                if ( !willrun )
                {
                    os.reboot_dev();
                }
            }
        }
#endif
		//DEBUG_PRINT(__LINE__); DEBUG_PRINT("_");
#ifndef SG21
        // real-time flow count
        static ulong flowcount_rt_start = 0;
        if ( os.options[OPTION_SENSOR_TYPE]==SENSOR_TYPE_FLOW )
        {
            if ( curr_time % FLOWCOUNT_RT_WINDOW == 0 )
            {
                os.flowcount_rt = ( flow_count > flowcount_rt_start ) ? flow_count - flowcount_rt_start: 0;
                flowcount_rt_start = flow_count;
            }
        }
        // perform ntp sync
        if ( curr_time % NTP_SYNC_INTERVAL == 0 ) os.status.req_ntpsync = 1;
        perform_ntp_sync();
        // check network connection
        if ( curr_time && ( curr_time % CHECK_NETWORK_INTERVAL==0 ) )  os.status.req_network = 1;
        check_network();
#endif
        // check weather
        check_weather();
		//DEBUG_PRINT(__LINE__); DEBUG_PRINT("_");
/*
#ifdef OPENSPRINKLER_ARDUINO_HEARTBEAT
		if (curr_time % 2 == 0)
		{
			digitalWrite(PIN_HEARTBEAT, HIGH);
			DEBUG_PRINT(F("."));
		}
		else
			digitalWrite(PIN_HEARTBEAT, LOW);
#endif // OPENSPRINKLER_ARDUINO_HEARTBEAT
*/
    }

#if !defined(ARDUINO)
    delay ( 1 ); // For OSPI/OSBO/LINUX, sleep 1 ms to minimize CPU usage
#endif
}

/** Make weather query */
void check_weather() {
    // do not check weather if
    // - network check has failed, or
    // - a program is currently running
    // - the controller is in remote extension mode
    if ( os.status.network_fails>0 || os.status.program_busy || os.options[OPTION_REMOTE_EXT_MODE] ) return;

    ulong ntz = os.now_tz();
  if (os.checkwt_success_lasttime && (ntz > os.checkwt_success_lasttime + CHECK_WEATHER_SUCCESS_TIMEOUT)) {
        // if weather check has failed to return for too long, restart network
        os.checkwt_success_lasttime = 0;
        // mark for safe restart
        os.status.safe_reboot = 1;
		write_message("restart for missing weather data!");
        return;
    }
  if (!os.checkwt_lasttime || (ntz > os.checkwt_lasttime + CHECK_WEATHER_TIMEOUT)) {
        os.checkwt_lasttime = ntz;
        GetWeather();
    }
}

/** Turn off a station
 * This function turns off a scheduled station
 * and writes log record
 */
void turn_off_station(byte sid, ulong curr_time) {
    os.set_station_bit ( sid, 0 );

    byte qid = pd.station_qid[sid];
    // ignore if we are turning off a station that's not running or scheduled to run
    if ( qid>=pd.nqueue )  return;

    RuntimeQueueStruct *q = pd.queue+qid;

    // check if the current time is past the scheduled start time,
    // because we may be turning off a station that hasn't started yet
  if (curr_time > q->st) {
        // record lastrun log (only for non-master stations)
    if(os.status.mas!=(sid+1) && os.status.mas2!=(sid+1)) {
            pd.lastrun.station = sid;
            pd.lastrun.program = q->pid;
            pd.lastrun.duration = curr_time - q->st;
            pd.lastrun.endtime = curr_time;

            // log station run
#ifdef SG21
			write_log(LOGDATA_STATION, curr_time, 0);
	////t		push_message(IFTTT_STATION_RUN, sid, pd.lastrun.duration);
#else
            write_log ( LOGDATA_STATION, curr_time );
#endif 
	}
    }

    // dequeue the element
    pd.dequeue ( qid );
    pd.station_qid[sid] = 0xFF;
}

/** Process dynamic events
 * such as rain delay, rain sensing
 * and turn off stations accordingly
 */
void process_dynamic_events ( ulong curr_time ){
//	__________________________________________________________________________________
#ifdef SG21
	 bool rain = false, soil1_wet = false, soil2_wet = false;
   bool en = os.status.enabled ? true : false;
bool fatal_closeout = false;
	if (os.status.rain_delayed || (os.status.rain_sensed && (os.options[OPTION_RSENSOR_TYPE] == SENSOR_TYPE_RAIN))) rain = true;
	  //check soil sensors disable irrigation?
		if ((os.status.dry_soil_1 == 0) && (os.old_status.dry_soil_1 == 0) && (os.options[OPTION_SSENSOR_1] != SENSOR_TYPE_NONE)) {
		soil1_wet = true;
		
	}
	if ((os.status.dry_soil_2 == 0) && (os.old_status.dry_soil_2 == 0) && (os.options[OPTION_SSENSOR_2] != SENSOR_TYPE_NONE)) {
		soil2_wet = true;
		
	}
	
		  //is fatal flow disable function enabled?
		if (os.options[OPTION_FATAL_ALARM] && os.options[OPTION_FSENSOR_TYPE]) {
		fatal_closeout = true;
		
	}
	byte sid, s, bid, qid, rbits, sbits1, sbits2, fatal_bits;
#else
	bool rain = false;
	bool en = os.status.enabled ? true : false;
	 if (os.status.rain_delayed || (os.status.rain_sensed && os.options[OPTION_SENSOR_TYPE] == SENSOR_TYPE_RAIN)) rain = true;
	byte sid, s, bid, qid, rbits;
#endif
	//__________________________________________________________________________________

    // check if rain is detected
    
    for ( bid=0; bid<os.nboards; bid++ ) {
        rbits = os.station_attrib_bits_read ( ADDR_NVM_IGNRAIN+bid );
#ifdef SG21
	  sbits1 = os.station_attrib_bits_read(ADDR_NVM_SSENSOR_1+bid);	//attached soil1 byte of the current board
	  sbits2 = os.station_attrib_bits_read(ADDR_NVM_SSENSOR_2+bid);	//attached soil2 byte of the current board
	  fatal_bits = os.station_attrib_bits_read(ADDR_NVM_ALARM_FATAL+bid);
#endif
        for ( s=0; s<8; s++ ) {
            sid=bid*8+s;

            // ignore master stations because they are handled separately
            if ( os.status.mas == sid+1 ) continue;
            if ( os.status.mas2== sid+1 ) continue;
            // If this is a normal program (not a run-once or test program)
            // and either the controller is disabled, or
            // if raining and ignore rain bit is cleared
            // FIX ME
            qid = pd.station_qid[sid];
            if ( qid==255 ) continue;
            RuntimeQueueStruct *q = pd.queue + qid;
#ifdef SG21
      if ((q->pid<99) && (!en || 
        (fatal_closeout && fatal_bits&(1<<s))	|| 
			  (soil1_wet && sbits1&(1<<s))			|| 
			  (soil2_wet && sbits2&(1<<s))			||  
			  (rain && !(rbits&(1<<s))) )	) { 
		  turn_off_station(sid, curr_time);
		  if (fatal_closeout && fatal_bits&(1 << s)) write_log(LOGDATA_FATAL_STATION_CANCEL, curr_time, sid);
		  if (soil1_wet && sbits1&(1 << s)) write_log(LOGDATA_SOIL1_STATION_CANCEL, curr_time, sid);
		  if (soil2_wet && sbits2&(1 << s)) write_log(LOGDATA_SOIL2_STATION_CANCEL, curr_time, sid);
		  if (rain && !(rbits&(1 << s))) write_log(LOGDATA_RAIN_STATION_CANCEL, curr_time, sid);

#else
            if ( ( q->pid<99 ) && ( !en || ( rain && ! ( rbits& ( 1<<s ) ) ) ) )
            {
                turn_off_station ( sid, curr_time );
#endif
            }
        }
    }
}

/** Scheduler
 * This function loops through the queue
 * and schedules the start time of each station
 */
void schedule_all_stations(ulong curr_time) {

    ulong con_start_time = curr_time + 1;   // concurrent start time
    ulong seq_start_time = con_start_time;  // sequential start time

    int16_t station_delay = water_time_decode_signed ( os.options[OPTION_STATION_DELAY_TIME] );
    // if the sequential queue has stations running
  if (pd.last_seq_stop_time > curr_time) {
        seq_start_time = pd.last_seq_stop_time + station_delay;
    }

    RuntimeQueueStruct *q = pd.queue;
    byte re = os.options[OPTION_REMOTE_EXT_MODE];
    // go through runtime queue and calculate start time of each station
  for(;q<pd.queue+pd.nqueue;q++) {
        if ( q->st ) continue; // if this queue element has already been scheduled, skip
        if ( !q->dur ) continue; // if the element has been marked to reset, skip
        byte sid=q->sid;
        byte bid=sid>>3;
        byte s=sid&0x07;

        // if this is a sequential station and the controller is not in remote extension mode
        // use sequential scheduling. station delay time apples
    if (os.station_attrib_bits_read(ADDR_NVM_STNSEQ+bid)&(1<<s) && !re) {
            // sequential scheduling
            q->st = seq_start_time;
            seq_start_time += q->dur;
            seq_start_time += station_delay; // add station delay time
    } else {
            // otherwise, concurrent scheduling
            q->st = con_start_time;
            // stagger concurrent stations by 1 second
            con_start_time++;
        }
        DEBUG_PRINT ( "[" );
        DEBUG_PRINT ( sid );
        DEBUG_PRINT ( ":" );
        DEBUG_PRINT ( q->st );
        DEBUG_PRINT ( "," );
        DEBUG_PRINT ( q->dur );
        DEBUG_PRINT ( "]" );
        DEBUG_PRINTLN ( pd.nqueue );
        if ( !os.status.program_busy ){
            os.status.program_busy = 1;  // set program busy bit
            // start flow count
#ifdef SG21
      if(os.options[OPTION_FSENSOR_TYPE] == SENSOR_TYPE_FLOW) {  // if flow sensor is connected
#else
            if ( os.options[OPTION_SENSOR_TYPE] == SENSOR_TYPE_FLOW ) {// if flow sensor is connected 
				os.flowcount_log_start = flow_count;
#endif
				
                os.sensor_lasttime = curr_time;
            }
        }
    }
}

/** Immediately reset all stations
 * No log records will be written
 */
void reset_all_stations_immediate() {
    os.clear_all_station_bits();
    os.apply_all_station_bits();
    pd.reset_runtime();
}

/** Reset all stations
 * This function sets the duration of
 * every station to 0, which causes
 * all stations to turn off in the next processing cycle.
 * Stations will be logged
 */
void reset_all_stations() {
    RuntimeQueueStruct *q = pd.queue;
    // go through runtime queue and assign water time to 0
  for(;q<pd.queue+pd.nqueue;q++) {
        q->dur = 0;
    }
}


/** Manually start a program
 * If pid==0, this is a test program (1 minute per station)
 * If pid==255, this is a short test program (2 second per station)
 * If pid > 0. run program pid-1
 */
#ifdef SG21
void manual_start_program(byte pid, byte uwt) {
#else
void manual_start_program(byte pid) {
#endif

	boolean match_found = false;
    reset_all_stations_immediate();
    ProgramStruct prog;
    ulong dur;
    byte sid, bid, s;
  if ((pid>0)&&(pid<255)) {
        pd.read ( pid-1, &prog );
    }
  for(sid=0;sid<os.nstations;sid++) {
        bid=sid>>3;
        s=sid&0x07;
        dur = 60;
        if ( pid==255 )  dur=2;
        else if ( pid>0 )
            dur = water_time_resolve ( water_time_decode ( prog.durations[sid] ) );
        RuntimeQueueStruct *q = pd.enqueue();
        if ( q && dur>0 && ! ( os.station_attrib_bits_read ( ADDR_NVM_STNDISABLE+bid ) & ( 1<<s ) ) )
        {
            q->st = 0;
            q->dur = dur;
            q->sid = sid;
            q->pid = 254;
            match_found = true;
        }
    }
    if ( match_found ) {
        schedule_all_stations ( os.now_tz() );
    }
}

// ================================
// ====== LOGGING FUNCTIONS =======
// ================================
#ifdef ESP8266
char LOG_PREFIX[] = "logs/";
#elif defined(ARDUINO)
char LOG_PREFIX[] = "/logs/";
#else
char LOG_PREFIX[] = "./logs/";
#endif

/** Generate log file name
 * Log files will be named /logs/xxxxx.txt
 */
void make_logfile_name ( char *name )
{
#if defined(ARDUINO) && !defined(ESP8266)
    sd.chdir ( "/" );
#endif
    strcpy ( tmp_buffer+TMP_BUFFER_SIZE-10, name );
    strcpy ( tmp_buffer, LOG_PREFIX );
    strcat ( tmp_buffer, tmp_buffer+TMP_BUFFER_SIZE-10 );
    strcat_P ( tmp_buffer, PSTR ( ".txt" ) );
}

/* To save RAM space, we store log type names
 * in program memory, and each name
 * must be strictly two characters with an ending 0
 * so each name is 3 characters total
 */
static prog_char log_type_names[] /*PROGMEM*/ =
    "  \0"
    "rs\0"
    "rd\0"
    "wl\0"
    "fl\0"
	"bc\0"
	"bv\0";

/** write run record to log on SD card */
#ifdef SG21
	void write_log(byte type, ulong curr_time, ulong param=0) {

	char tmp2[10];
	char tmp3[10];
#else
void write_log(byte type, ulong curr_time) {

#endif

    if ( !os.options[OPTION_ENABLE_LOGGING] ) return;

    // file name will be logs/xxxxx.tx where xxxxx is the day in epoch time
    ultoa ( curr_time / 86400, tmp_buffer, 10 );
    make_logfile_name ( tmp_buffer );

#if defined(ARDUINO) // prepare log folder for Arduino
    if ( !os.status.has_sd )  return;
	
#ifndef ESP8266
    sd.chdir ( "/" );
  if (sd.chdir(LOG_PREFIX) == false) {
        // create dir if it doesn't exist yet
    if (sd.mkdir(LOG_PREFIX) == false) {
            return;
        }
    }
    SdFile file;
	int ret = file.open ( tmp_buffer, O_CREAT | O_WRITE );
    file.seekEnd();
  if(!ret) {
        return;
    }
#else //ESP8266
	DEBUG_PRINT("logFile W>"); DEBUG_PRINTLN(tmp_buffer);
	memmove(tmp_buffer + 1, tmp_buffer, strlen(tmp_buffer)+1); tmp_buffer[0] = '/';
	File file;
	if (SPIFFS.exists(tmp_buffer))
	{
		file = SPIFFS.open(tmp_buffer, "r+"); DEBUG_PRINTLN("file exist open R/W");
	}
	else
	{
		file = SPIFFS.open(tmp_buffer, "w"); DEBUG_PRINTLN("file dont exist open R/W");
	}
	
	int ret = (bool) file;
	file.seek(0, SeekEnd);
	if (!ret)
	{
		DEBUG_PRINTLN("NoGood");
		return;
	}
#endif //ESP8266

#else // prepare log folder for RPI/BBB
    struct stat st;
    if ( stat ( get_filename_fullpath ( LOG_PREFIX ), &st ) ){
    if(mkdir(get_filename_fullpath(LOG_PREFIX), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH)) {
            return;
}
    FILE *file;
    file = fopen ( get_filename_fullpath ( tmp_buffer ), "rb+" );
  if(!file) {
        file = fopen ( get_filename_fullpath ( tmp_buffer ), "wb" );
        if ( !file )  return;
    }
    fseek ( file, 0, SEEK_END );
#endif  // prepare log folder
#ifdef SG21
	// ### CLASSIC FUNCTIONAL EVENT LOGS

	if (type == LOGDATA_STATION) {

		// station ended: [program type,station id, duration, time, impulses, current]
		sprintf(tmp_buffer, "[%u,%u,%u,%lu,%lu,%lu]\n",
			pd.lastrun.program, pd.lastrun.station,
			pd.lastrun.duration, curr_time,
			sensors.station_impulses, sensors.realtime_current);
	}

	if (type == LOGDATA_PROGFLOW) {  //was LOGDATA_FLOWSENSE

									 // program ended: [flow_impulses, "fl", flow_duration, time]
		sprintf(tmp_buffer, "[%lu,\"fl\",%lu,%lu]\n", sensors.prog_impulses,
			curr_time - sensors.prog_start_time, curr_time);
	}

	if (type == LOGDATA_RAINDELAY) {
		// rain delay: [0,"rd",length of rain delay,curr_time]
		sprintf(tmp_buffer, "[0,\"rd\",%lu,%lu]\n",
			(curr_time > os.raindelay_start_time) ? (curr_time - os.raindelay_start_time) : 0,
			curr_time);
	}

	if (type == LOGDATA_RAINSENSE) {
		// rain sensor: [0,"rs",length of ? ,curr_time] it is the orinal log: only logs when rain_sensed ==> 0, and logs the length of 0 state
		sprintf(tmp_buffer, "[0,\"rs\",%lu,%lu]\n",    //%d, os.status.rain_sensed,
			(curr_time > os.sensor_lasttime) ? (curr_time - os.sensor_lasttime) : 0,
			curr_time);
	}

	if (type == LOGDATA_WATERLEVEL) {
		// value of waterlevel: [0,"wl", percent, curr_time]
		sprintf(tmp_buffer, "[0,\"wl\",%u,%lu]\n",
			os.options[OPTION_WATER_PERCENTAGE],
			curr_time);
	}

	// ### NEW FUNCTIONAL EVENT LOGS

	if (type == LOGDATA_PROGFLOW2) {

		// program ended: [0, "fp",type, time,flow_impulses, flow_duration]
		sprintf(tmp_buffer, "[0,\"fp\",%d,%lu,%lu,%lu,%lu,%lu]\n", type, curr_time,
			sensors.prog_impulses, curr_time - sensors.prog_start_time, sensors.last_prog_impulses, sensors.day_impulses);
	}

	if (type == LOGDATA_DAYFLOW) {

		// daily water use: [0, "fd", type, time, day_impulses]
		sprintf(tmp_buffer, "[0,\"fd\",%d,%lu,%lu]\n", type, curr_time, sensors.day_impulses);
	}

	if (type == LOGDATA_RAINSENSE2) {
		// rain sensor: [0,"rn",type,curr_time,status:0/1,curr_time] 
		sprintf(tmp_buffer, "[0,\"rn\",%d,%lu,%d]\n", type, curr_time, os.status.rain_sensed);
	}

	if (type == LOGDATA_RAINDELAY2) {
		// rain delay: [0,"re",type,curr_time,state of rain delay]
		sprintf(tmp_buffer, "[0,\"re\",%d,%lu,%d]\n", type, curr_time, os.status.rain_delayed);
	}

	if (type == LOGDATA_SOIL1) {
		// soil1 activated: [0,"s1", type,curr_time, status]
		sprintf(tmp_buffer, "[0,\"s1\",%d,%lu,%d]\n", type, curr_time, os.status.dry_soil_1);
		//curr_time > os.s1sensor_lasttime ? curr_time - os.s1sensor_lasttime : 0
	}

	if (type == LOGDATA_SOIL2) {
		// soil2 activated: [0,"s2",type,curr_time,soil_sensor2 status]
		sprintf(tmp_buffer, "[0,\"s2\",%d,%lu,%d]\n", type, curr_time, os.status.dry_soil_2);
		//, time since last change: curr_time > os.s2sensor_lasttime ? curr_time - os.s2sensor_lasttime : 0
	}

	if (type == LOGDATA_SOIL1_PROG_CANCEL) {
		// soil1 activated: [0,"s1pc",type,curr_time,program id, soil_sensor1 status]
		sprintf(tmp_buffer, "[0,\"s1pc\",%d,%lu,%lu,%d]\n", type, curr_time, param, os.status.dry_soil_1);
	}

	if (type == LOGDATA_SOIL2_PROG_CANCEL) {
		// soil1 activated: [0,"s2pc",type,curr_time,program id, soil_sensor2 status]
		sprintf(tmp_buffer, "[0,\"s2pc\",%d,%lu,%lu,%d]\n", type, curr_time, param, os.status.dry_soil_2);
	}

	if (type == LOGDATA_SOIL1_STATION_CANCEL) {
		// soil1 activated: [0,"s1sc",type,  curr_time, station id, soil_sensor1 status]
		sprintf(tmp_buffer, "[0,\"s1sc\",%d,%lu,%d,%d]\n", type, curr_time, param, os.status.dry_soil_1);
	}

	if (type == LOGDATA_SOIL2_STATION_CANCEL) {
		// soil1 activated: [0,"s2sc",type, curr_time, station id, soil_sensor2 status]
		sprintf(tmp_buffer, "[0,\"s2sc\",%d,%lu,%lu,%d]\n", type, curr_time, param, os.status.dry_soil_2);
	}

	if (type == LOGDATA_FATAL_STATION_CANCEL) {
		// soil1 activated: [0,"fatsc",type,curr_time, station id]
		sprintf(tmp_buffer, "[0,\"fatsc\",%d,%lu,%lu]\n", type, curr_time, param);
	}

	if (type == LOGDATA_RAIN_STATION_CANCEL) {
		// soil1 activated: [0,"rnsc",type, time, station id, rain sensor status, rain delay status]
		sprintf(tmp_buffer, "[0,\"rnsc\",%d,%lu,%lu,%d,%d]\n", type, curr_time, param, os.status.rain_sensed,
			os.status.rain_delayed);
	}

	if (type == LOGDATA_CALIBRATED) {

		// calibration saved: [0,"cal",type, time, station id, current, flow]
		dtostrf(sensors.realtime_GPM, 4, 2, tmp2);
		sprintf(tmp_buffer, "[0,\"cal\",%d,%lu,%d,%lu,%s]\n", type, curr_time,
			sensors.sid, sensors.realtime_current, tmp2);
	}

	//####  ALARM LOGS

	if (type == LOGDATA_ALARM_FLOW_STOPPED) {

		// alarm:flow stopped: [0, "alfs",type, time, flow impulses since start, running time]
		sprintf(tmp_buffer, "[0,\"alfs\",%d,%lu,%d,%lu,%lu]\n", type, curr_time, sensors.sid,
			sensors.prog_impulses, curr_time - sensors.prog_start_time);
	}

	if (type == LOGDATA_ALARM_FLOW_LOW || type == LOGDATA_ALARM_FLOW_HIGH) {
		// station flow error: [0,"alf",type,time, station id, ref flow, realtime GPM]
		dtostrf(sensors.realtime_GPM, 4, 2, tmp2);
		sprintf(tmp_buffer, "[0,\"alf\",%d,%lu,%d,%d,%s]\n",
			type, curr_time, sensors.sid, sensors.flow_refval >> 3, tmp2);
	}

	if (type == LOGDATA_ALARM_CURRENT_LOW || type == LOGDATA_ALARM_CURRENT_HIGH) {
		// station current error: [0,"alc",type,time, station id, ref current, realtime current]
		sprintf(tmp_buffer, "[0,\"alc\",%d,%lu,%d,%d,%lu]\n",
			type, curr_time, sensors.sid, sensors.curr_refval << 2, sensors.realtime_current);
	}

	if (type == LOGDATA_ALARM_FF_QUANTITY || type == LOGDATA_ALARM_FF_TIME || type == LOGDATA_FREEFLOW_END) {
		// freeflow overrun: [0,"alff",type, time, impulses, runtime]
		//dtostrf(sensors.realtime_GPM,4,2,tmp2);
		//dtostrf(sensors.realtime_gallons,4,2,tmp3);
		sprintf(tmp_buffer, "[0,\"alff\",%d,%lu,%lu,%lu]\n",
			type, curr_time, sensors.prog_impulses, curr_time - sensors.prog_start_time);
	}

	if (type == LOGDATA_ALARM_LEAKAGE_START) {
		// leakage error: [0,"alls",type,time]
		sprintf(tmp_buffer, "[0,\"alls\",%d,%lu]\n",
			type, curr_time);
	}

	if (type == LOGDATA_ALARM_LEAKAGE_END) {
		// station flow error: [0,"alle",type,time,FF quantity,FF duration]
		//dtostrf(sensors.realtime_gallons,4,2,tmp3);
		sprintf(tmp_buffer, "[0,\"alle\",%d,%lu,%lu,%lu]\n",
			type, curr_time, sensors.prog_impulses, curr_time - sensors.prog_start_time);
	}

	if (type == LOGDATA_ALARM_FATAL_FLOW) {
		// station fatal flow error: [0,"alfat",type,time,station id, ref flow, realtime GPM]
		dtostrf(sensors.realtime_GPM, 4, 2, tmp2);
		sprintf(tmp_buffer, "[0,\"alfat\",%d,%lu,%d,%d,%s]\n",
			type, curr_time, sensors.sid, sensors.flow_refval >> 3, tmp2);
	}

	//  #### ADMIN LOGS

	if (type == LOGDATA_FAILED_STATE) {
		// FSM failed routing: [0,"fstat",type,time,station id,event, old_state, curr state]
		dtostrf(sensors.realtime_GPM, 4, 2, tmp2);
		sprintf(tmp_buffer, "[0,\"fstat\",%d,%lu,%d,%d,%d,%d]\n",
			type, curr_time, sensors.sid, sensors.event, sensors.old_state,
			sensors.current_state);
	}
	log_rec_counter++;
	// ### END OF RECORD DATA
#else

    strcpy_P ( tmp_buffer, PSTR ( "[" ) );

    if ( type == LOGDATA_STATION )
    {
        itoa ( pd.lastrun.program, tmp_buffer+strlen ( tmp_buffer ), 10 );
        strcat_P ( tmp_buffer, PSTR ( "," ) );
        itoa ( pd.lastrun.station, tmp_buffer+strlen ( tmp_buffer ), 10 );
        strcat_P ( tmp_buffer, PSTR ( "," ) );
        // duration is unsigned integer
        ultoa ( ( ulong ) pd.lastrun.duration, tmp_buffer+strlen ( tmp_buffer ), 10 );
    }
 
		else
    {
        ulong lvalue;
        if ( type==LOGDATA_FLOWSENSE )
        {
            lvalue = ( flow_count>os.flowcount_log_start ) ? ( flow_count-os.flowcount_log_start ) :0;
        }
        else
        {
            lvalue = 0;
        }
        ultoa ( lvalue, tmp_buffer+strlen ( tmp_buffer ), 10 );
        strcat_P ( tmp_buffer, PSTR ( ",\"" ) );
        strcat_P ( tmp_buffer, log_type_names+type*3 );
        strcat_P ( tmp_buffer, PSTR ( "\"," ) );

        switch ( type )
        {
        case LOGDATA_RAINSENSE:
        case LOGDATA_FLOWSENSE:
            lvalue = ( curr_time>os.sensor_lasttime ) ? ( curr_time-os.sensor_lasttime ) :0;
            break;
        case LOGDATA_RAINDELAY:
            lvalue = ( curr_time>os.raindelay_start_time ) ? ( curr_time-os.raindelay_start_time ) :0;
            break;
        case LOGDATA_WATERLEVEL:
            lvalue = os.options[OPTION_WATER_PERCENTAGE];
            break;
#ifdef BATTERY
		case LOGDATA_BATTERY:
			lvalue = os.BatteryCharge();
			break;
		case LOGDATA_BATTERYV:
			lvalue = os.BatteryVoltage();
			break;
#endif
		}
        ultoa ( lvalue, tmp_buffer+strlen ( tmp_buffer ), 10 );
    }
    strcat_P ( tmp_buffer, PSTR ( "," ) );
    ultoa ( curr_time, tmp_buffer+strlen ( tmp_buffer ), 10 );
    strcat_P ( tmp_buffer, PSTR ( "]\r\n" ) );
#endif
#if defined (ARDUINO)

	DEBUG_PRINTLN(tmp_buffer);
	//DEBUG_PRINTLN(freeRam());

#ifndef ESP8266
    file.write ( tmp_buffer );
#else 
	strcat(tmp_buffer, "\0");
	file.print(tmp_buffer);
#endif

    file.close();
#else
    fwrite ( tmp_buffer, 1, strlen ( tmp_buffer ), file );
    fclose ( file );
#endif
}


/** Delete log file
 * If name is 'all', delete all logs
 */
void delete_log(char *name) {
    if ( !os.options[OPTION_ENABLE_LOGGING] ) return;
#if defined(ARDUINO)
    if ( !os.status.has_sd ) return;

    if ( strncmp ( name, "all", 3 ) == 0 ) {
#ifndef ESP8266
		// delete the log folder
        SdFile file;

    if (sd.chdir(LOG_PREFIX)) {
            // delete the whole log folder
            sd.vwd()->rmRfStar();
        }
#else
		/////t delete all log files
#endif
		return;

    }    else {
        make_logfile_name ( name );
#ifndef ESP8266
		if ( !sd.exists ( tmp_buffer ) )  return;
        sd.remove ( tmp_buffer );
#else  //ESP8266
		if (!SPIFFS.exists(tmp_buffer))  return;
		SPIFFS.remove(tmp_buffer);
#endif //ESP8266
	}
#else // delete_log implementation for RPI/BBB
  if (strncmp(name, "all", 3) == 0) {
        // delete the log folder
        rmdir ( get_filename_fullpath ( LOG_PREFIX ) );
        return;
  } else {
        make_logfile_name ( name );
        remove ( get_filename_fullpath ( tmp_buffer ) );
    }
#endif
}


/** Perform network check
 * This function pings the router
 * to check if it's still online.
 * If not, it re-initializes Ethernet controller.
 */
void check_network() {
#if defined(ARDUINO)
    // do not perform network checking if the controller has just started, or if a program is running
  if (os.status.program_busy) {return;}

    // check network condition periodically
  if (os.status.req_network) {
        os.status.req_network = 0;
        // change LCD icon to indicate it's checking network
        if ( !ui_state ) {
#ifndef LCD_SSD1306
			os.lcd.setCursor(15, 1);
			os.lcd.write(4);
#else
			os.lcd.drawBitmap(18*XFACTOR, 0, blank24x18_bmp, 5, 9, BLACK);
			os.lcd.display();
#endif
        }
		boolean failed = true;
#ifdef ESP8266
		byte ntry = 0;
		DEBUG_PRINT("ping ");
		while (!ether.packetLoopIcmpCheckReply(ether.gwip)) {
	//		pulseLed(100, 100, 1);
			
			if (!ui_state)
			{
#ifndef LCD_SSD1306
				os.lcd.setCursor(15, 1);
				os.lcd.write(4);
#else
				os.lcd.drawBitmap(18 * XFACTOR, 0, blank24x18_bmp, 5, 9, BLACK);
				os.lcd.display();
				delay(500);
				os.lcd.drawBitmap(18 * XFACTOR, 0, antenna5x9_bmp, 5, 9, WHITE);
				os.lcd.display();

#endif
			}

			ntry++;
			if (ntry > 10)break;
		}
		if (ntry <= 10) {
			failed = false;
			DEBUG_PRINTLN(" OK");
		}
		else {
			DEBUG_PRINTLN(" failed"); write_message("ping failed");
		}

#else

		// ping gateway ip
		ether.clientIcmpRequest(ether.gwip);

		ulong start = millis();
	
		// wait at most PING_TIMEOUT milliseconds for ping result
		do
        {
            ether.packetLoop ( ether.packetReceive() );
            if ( ether.packetLoopIcmpCheckReply ( ether.gwip ) )
            {
                failed = false;
                break;
            }
        }
        while ( millis() - start < PING_TIMEOUT );
#endif
		if ( failed )
        {
            os.status.network_fails++;
            // clamp it to 6
            if ( os.status.network_fails > 6 ) os.status.network_fails = 6;
        }
        else os.status.network_fails=0;
        // if failed more than 6 times, restart
        if ( os.status.network_fails>=6 )
		{
			write_message("netw. reconn. impossible");
            // mark for safe restart
            os.status.safe_reboot = 1;
        }
        else if ( os.status.network_fails>2 )
        {
			write_message("try to reconnect!");
            // if failed more than twice, try to reconnect
			if (os.start_network())
				os.status.network_fails = 0;
			else
				os.status.network_fails = 1;
        }
    }
#else
    // nothing to do here
    // Linux will do this for you
#endif
}

/** Perform NTP sync */
void perform_ntp_sync() {
#if defined(ARDUINO)
    // do not perform sync if this option is disabled, or if network is not available, or if a program is running
    if ( !os.options[OPTION_USE_NTP] || os.status.network_fails>0 || os.status.program_busy ) return;

    if ( os.status.req_ntpsync )  {
        os.status.req_ntpsync = 0;
    if (!ui_state) {
            os.lcd_print_line_clear_pgm ( PSTR ( "NTP Syncing..." ),1 );
        }
	//	setSyncProvider(getNtpTime);
        ulong t = getNtpTime();
    if (t>0) {
            setTime ( t );
            RTC.set ( t );
        }
    }
#else
    // nothing to do here
    // Linux will do this for you
#endif
}

#if !defined(ARDUINO) // main function for RPI/BBB
int main(int argc, char *argv[]) {
    do_setup();

  while(true) {
        do_loop();
    }
    return 0;
}
#endif
