/*
	This file manages the sensors of the OpenSprinkler system.

	The flow sensor operates as a finite state machine and communicates with the system via method calls placed in the main.cpp file.

	The states of the flow sensor are the following:
		FreeFlow
			- no station is activated but there is flow
			- when OPTION_FLOW_ALARM == 1, the freeflow is not enabled, this will generate an alert
			- when OPTION_FLOW_ALARM == 2, the freeflow enabled, but
				- we alert the system if the time length of the freeflow exceeds a specified time length OPTION_FREEFLOW_TIME
				- we also alert the system if the number of impulses reach a specified value OPTION_FREEFLOW_QUANTITY
		NoFlow
			- no station is activated and there is no flow
		StationFlow
			- station is activated and there is or is not flow
			- the system checks the PPM of the station and if it significantly differs from the nominal* value we make countermeasures

	Calibration:
		- every station has a nominal flow and current value
		- to calibrate flow and current, we set the nominal current value of the station to 0xFF
		- current_ref value: 0: factory reset has been done, 0xFF: calibration request 
		
	We might want a 2 level alert system:
		- Level 1: Alert (only notification)
		- Level 2: Fatal error: the flow is over 200% (FATAL_FLOW_LIMIT) of reference flow (notification and disabling the station)

	Time keeping:
		- for seconds I use the OpenSprinkler class' now_tz() method
		- for milliseconds I use the millis() method

	Storing data between reboots:
		- EEPROM: see in defines.h

	The entire file (if not stated otherwise) is written by CV.
	Modified by TCS.
*/

#include "SensorGroup.h"

extern OpenSprinkler os;
extern SdFat sd;
extern void write_log(byte type, ulong curr_time, ulong param);
extern void alarm_message(byte type);
extern void push_message(byte type, uint32_t lval, float fval, const char* sval);
extern byte push_message_cloud(byte type, ulong day=0);
extern void turn_off_station(byte sid, ulong curr_time);

uint16_t nvm_read_word(ulong addr);
void nvm_write_word(ulong addr, uint16_t f);

// FSM control variables
byte SensorGroup::current_state = NO_FLOW;
byte SensorGroup::old_state = NO_FLOW;
byte SensorGroup::next_state = NO_FLOW;
byte SensorGroup::event = NOTHING;
ulong SensorGroup::fsm_fail_counter;

//stations controls
byte SensorGroup::sid = 0;
bool SensorGroup::excluded;
bool SensorGroup::special;
bool SensorGroup::stat_started;
bool SensorGroup::stat_stopped;
bool SensorGroup::prog_finished;


//flow sensor handling
volatile ulong SensorGroup::flow_last_impulse_ms = 10000;	//hit of last impulse in msec
volatile ulong SensorGroup::sensor_impulses = 0;	//impulse counter during program
ulong SensorGroup::last_read_impulse_ms;			//last read of impulse hit time in msec
ulong SensorGroup::last_sensor_impulses;			//last read of the impulse counter
ulong SensorGroup::old_sensor_impulses = 0;			//previous read of the counter
float SensorGroup::flow_pulse_rate_IpG = 1.00;		//flow sensor conversion rate impulses per gallon
bool SensorGroup::hall_effect_sensor = 0;			//no debounce needed if hall effect sensor

//alarm handling
byte SensorGroup::curr_refval;				// The current Station CURRENT reference not calibrated if 0xFF
uint16_t SensorGroup::flow_refval;			// The current Station FLOW reference not calibrated if 0xFF
byte SensorGroup::flow_alarm_range = 20;
byte SensorGroup::current_alarm_range = 50;
char last_alarm_LCD[16];
byte SensorGroup::alarm_cnt=0;

//results
bool SensorGroup::display_GPM;				//if true display in Gallon and GPM, if false in Liter and liter per minute
float SensorGroup::realtime_GPM;			//measured in measure window
float SensorGroup::realtime_gallons;		//measured in the entire program run
ulong SensorGroup::realtime_current = 0;	//last read of solenoids current
ulong SensorGroup::station_impulses = 0;	//total no of impulses during station run
ulong SensorGroup::prog_impulses = 0;		//total number of impulses during program run
ulong SensorGroup::last_prog_impulses = 0;	//the previous program total
ulong SensorGroup::day_impulses = 0;		//total of impulses today
ulong SensorGroup::last_day_impulses = 0;	//total of impulses last day
ulong SensorGroup::window_impulses=0;		//number of impulses in measure window

// measure flow
ulong SensorGroup::prog_start_time = 0;		//the start of the first impulse in sec
ulong SensorGroup::station_start_time = 0;	//start of station run in sec
ulong SensorGroup::station_start_impulses;
ulong SensorGroup::measure_window_start = 0;//start of current flow measure window
ulong SensorGroup::measure_start_imp;		//the no of impulses at measure window start
ulong SensorGroup::sec_timer;				//timeout start in sec
bool SensorGroup::flow_valid_flag;			//if true, the flow measurement is valid
bool SensorGroup::prog_start_flag;

void clear_fatal_flags();

/** Alarm messages on LCD (stored in progmem) */
// Each string is strictly 16 characters
// with SPACE fillings if less
#define ALARM_MESSAGE_STEPSIZE 5
#if defined(ARDUINO)
const char alarm_message_text[] PROGMEM =
#else
const char alarm_message_text[] =
#endif
"Flow Stopped    "
"Frflow QuantAlrm"
"Frflow TimeAlarm"
"Leakage Alarm   "
"Leakage Stopped "
"High Flow Alarm "
"Low Flow Alarm "
"Fatal Flow Alarm"
"High Current Alm"
"Low Current Alrm";

// Flow sensor interrupt service routine with debounce
// this might run very often, so we do not want to put heavy stuff here
void flowsensor_ISR() {
	if(SensorGroup::hall_effect_sensor){
		SensorGroup::flow_last_impulse_ms = millis();
		SensorGroup::sensor_impulses++;
	
	} else {
		ulong curr = millis();
		// debouncing
		if (SensorGroup::flow_last_impulse_ms + FLOW_SENSOR_DEBOUNCE >= curr) {
			return;
			}

			SensorGroup::flow_last_impulse_ms = curr;
			SensorGroup::sensor_impulses++;
		}
}
	

void SensorGroup::init() {

	DEBUG_PRINTLN("SensorGroup init starting...");

	ulong curr_time = os.now_tz();
	SensorGroup::init_prog_count(curr_time); // init FSM and flow counter
	
	// setting the impulse/gallon or impulse/liter value
	flow_pulse_rate_IpG = (float)((os.options[OPTION_PULSE_RATE_1] << 8) + (byte)os.options[OPTION_PULSE_RATE_0])/100;
	//if faster flow sensor applied, debouncing is off
	if(flow_pulse_rate_IpG > HALL_SENSOR_LIMIT) hall_effect_sensor = 1;

	// converting the impulse/liter to impulse/gallon if liter
	// the actual unit depends on the  OPTION_FLOWUNIT_GAL 1: gallon/minute, 0:liter/minute
	if (!os.options[OPTION_FLOWUNIT_GAL]) { 
	flow_pulse_rate_IpG = flow_pulse_rate_IpG * 3.78541;
	}

	// setting the flow and current alarm thresholds
	flow_alarm_range = int8_t(os.options[OPTION_FLOW_ALARM_RANGE]);
	current_alarm_range = int8_t(os.options[OPTION_CURR_RANGE]);
	
  #if defined(SERIAL_DEBUG)
	// setup data
	DEBUG_PRINT("Impulse/Gallon: ");
	DEBUG_PRINTLN(flow_pulse_rate_IpG);

	DEBUG_PRINT("flow diff/current diff:  ");
	DEBUG_PRINT(flow_alarm_range);
	DEBUG_PRINT(" / ");
	DEBUG_PRINTLN(current_alarm_range);

	DEBUG_PRINT("freeflow max quantity Gallon / time minute:  ");
	DEBUG_PRINT(os.options[OPTION_FREEFLOW_QUANTITY]);
	DEBUG_PRINT(" / ");
	DEBUG_PRINTLN(os.options[OPTION_FREEFLOW_TIME]);
  
	//os.options[OPTION_CAL_REQUEST]	= 1;
	DEBUG_PRINT(" CAL Request: / FLOW Alarm EN / FATAL EN / CURR Alarm EN:    ");
	DEBUG_PRINT(os.options[OPTION_CAL_REQUEST]);
	DEBUG_PRINT(" / ");
	DEBUG_PRINT(os.options[OPTION_FLOW_ALARM]);
	DEBUG_PRINT(" / ");
	DEBUG_PRINT(os.options[OPTION_FATAL_ALARM]);
	DEBUG_PRINT(" / ");
	DEBUG_PRINTLN(os.options[OPTION_CURR_ALARM]);

	for(int i=0; i< MAX_NUM_STATIONS; i++){	
	SensorGroup::curr_refval = os.station_attrib_bits_read(ADDR_NVM_CURR_REFS + (i));
	DEBUG_PRINT(curr_refval);
	DEBUG_PRINT(", ");
	}
	DEBUG_PRINTLN("");
	
	for(int i=0; i< MAX_NUM_STATIONS; i++){
		SensorGroup::flow_refval = (os.station_attrib_bits_read(ADDR_NVM_FLOW_REFS + 1 + (2*i)) << 8) +
		os.station_attrib_bits_read(ADDR_NVM_FLOW_REFS + (2*i) );
		DEBUG_PRINT(flow_refval);
		DEBUG_PRINT(", ");
	}
	DEBUG_PRINTLN("");
	
	for(int i=0; i<= MAX_EXT_BOARDS; i++){
		byte v = nvm_read_byte((byte*)ADDR_NVM_ALARM_FATAL + i);
		DEBUG_PRINT(v);
		DEBUG_PRINT(", ");
	}
	DEBUG_PRINTLN("");	
/**/
  #endif
  	
	//delete NVM calibration data
	
	//if the user needs calibration or the system has been factory reseted (NVM cleared to 0) sets to uncalibrated
	//current_ref byte: 0: factory reset; 0xff:calibrate station at next run; 
	if(os.options[OPTION_CAL_REQUEST] || !os.station_attrib_bits_read(ADDR_NVM_CURR_REFS)){
			for(int i=0; i< MAX_NUM_STATIONS; i++){
			SensorGroup::clear_station_calibration(i);
			}
		clear_fatal_flags();
		os.options[OPTION_CAL_REQUEST]	= 0;
		os.options_save();
	}
/**/	
  #if defined(SERIAL_DEBUG)
	DEBUG_PRINT(" CAL Request_after:  ");
	DEBUG_PRINTLN(os.options[OPTION_CAL_REQUEST]);
	for(int i=0; i< MAX_NUM_STATIONS; i++){	
	DEBUG_PRINT(", ");
	SensorGroup::curr_refval = os.station_attrib_bits_read(ADDR_NVM_CURR_REFS + (i));
	DEBUG_PRINT(curr_refval);
	}
	DEBUG_PRINTLN("");
	
	for(int i=0; i< MAX_NUM_STATIONS; i++){	
		DEBUG_PRINT(", ");
		SensorGroup::flow_refval = (os.station_attrib_bits_read(ADDR_NVM_FLOW_REFS + 1 + (2*i)) << 8) +
				os.station_attrib_bits_read(ADDR_NVM_FLOW_REFS + (2*i) );
		DEBUG_PRINT(flow_refval);
	}
		DEBUG_PRINTLN("");
	
	for(int i=0; i<= MAX_EXT_BOARDS; i++){
		byte v = nvm_read_byte((byte*)ADDR_NVM_ALARM_FATAL + i);
		DEBUG_PRINT(v);
		DEBUG_PRINT(", ");
	}
	DEBUG_PRINTLN("");	
	
	
  #endif
	


		
	
}

void SensorGroup::loop(ulong current_time) {
	
	check_sensors(current_time);
	
	if(os.options[OPTION_FSENSOR_TYPE == SENSOR_TYPE_FLOW]){
		SensorGroup::flow_loop(current_time);
		}
}

// TODO: Current ref deleted to 0xFF: at Fact Reset, at OPTION_CAL_REQUEST == 1,
//	TODO:	Reset calibrated values to 0xFF when Master station changed

void SensorGroup::clear_station_calibration(byte sid){
	//sets the appropriate current_ref and flow_ref bytes to 0xFF in NVM
	byte tmp = 0xFF;
	nvm_write_byte((byte*)(ADDR_NVM_CURR_REFS + (sid)), tmp );
	nvm_write_byte((byte*)(ADDR_NVM_FLOW_REFS + 1 + (2*sid)), tmp );
	nvm_write_byte((byte*)(ADDR_NVM_FLOW_REFS + (2*sid)), tmp );
	
	//TODO: change to nvm_write_block()
}

void SensorGroup::clear_calibration(){
	//clear the entire flow&current_refs in NVM to 0xFF, 0xFFFF
	for(int i=0; i > MAX_NUM_STATIONS;i++){
		SensorGroup::clear_station_calibration(i);
	}
}

void clear_fatal_flags(){
	for(int i=0; i<= MAX_EXT_BOARDS; i++){
		byte v = 0;
		nvm_write_byte((byte*)ADDR_NVM_ALARM_FATAL + i, v );
	}
}

void store_alarm_ref(byte sid){  // store the realtime_GPM and realtime_current values in NVM
	uint16_t f = (int)SensorGroup::realtime_GPM * 8.0;
	byte v = f & 0xFF;
	ulong addr = (ADDR_NVM_FLOW_REFS + (2*sid));	//low byte
	nvm_write_byte((byte*) addr, v );
	v = (f >> 8) & 0xFF;  //  shift the high byte
	addr +=addr;
	nvm_write_byte( (byte*)(addr), v );
	
	v = (SensorGroup::realtime_current >> 2) & 0xFF; //divide by 4
	if( v < 8) v = 8;
	if(v == 0xFF) v = 0xFE;
	addr = (ADDR_NVM_CURR_REFS + (sid));	
	nvm_write_byte((byte*)addr, v );
}

void read_station_data(byte sid){ //reads exclude, remote flags, station refs, 
	SensorGroup::excluded = os.station_attrib_bits_read(ADDR_NVM_ST_EXCLUDE_AL + (sid >> 3)) & (sid % 8);
	SensorGroup::special = os.station_attrib_bits_read(ADDR_NVM_STNSPE + (sid >> 3)) & (sid % 8);
	SensorGroup::curr_refval = os.station_attrib_bits_read(ADDR_NVM_CURR_REFS + (sid));
	//SensorGroup::curr_refval = SensorGroup::curr_refval << 2;
	SensorGroup::flow_refval = (os.station_attrib_bits_read(ADDR_NVM_FLOW_REFS + 1 + (2*sid)) << 8) +
				os.station_attrib_bits_read(ADDR_NVM_FLOW_REFS + (2*sid) );
	//SensorGroup::flow_refval = SensorGroup::flow_refval >> 3;
}

void SensorGroup::init_prog_count(ulong curr_time){ //at each program start, and freeflow start
	//initialize flow sensor reading
	SensorGroup::sensor_impulses = 0;
	SensorGroup::last_sensor_impulses = 0;
	SensorGroup::old_sensor_impulses = 0;
	//initialize program related counting
	SensorGroup::prog_start_time = curr_time;
	SensorGroup::prog_impulses = 0;
	SensorGroup::realtime_GPM = 0.0;
	//SensorGroup::window_impulses=0;
	//SensorGroup::realtime_gallons = 0;
	//SensorGroup::station_impulses = 0;
}

void SensorGroup::init_station_count(ulong curr_time){ // at the beginning of each station and freeflow cycle
	SensorGroup::read_flow_sensor();
	//initialize station counting
	SensorGroup::station_start_time = curr_time;
	SensorGroup::realtime_GPM = 0.0;
	//SensorGroup::window_impulses=0;
	SensorGroup::station_start_impulses = SensorGroup::last_sensor_impulses;
	SensorGroup::station_impulses = 0;
	SensorGroup::realtime_gallons = 0;
	//initialize measure window
	SensorGroup::measure_window_start = curr_time;
	SensorGroup::measure_start_imp = SensorGroup::station_start_impulses;
	SensorGroup::flow_valid_flag = 0;
}

int SensorGroup::check_current_alarm(){
	if(os.options[OPTION_CURR_ALARM] == 0  ) return(0);	//if alarm enabled check current alarm
	uint32_t c1 = ((uint32_t)SensorGroup::curr_refval << 2) * (100 - SensorGroup::current_alarm_range)/100;
	uint32_t c2 = ((uint32_t)SensorGroup::curr_refval << 2) * (100 + SensorGroup::current_alarm_range)/100;
/*		DEBUG_PRINT(" realtime_curr , c1, c2:  ");
		DEBUG_PRINT(realtime_current);
		DEBUG_PRINT(" , ");
		DEBUG_PRINT(c1);
		DEBUG_PRINT(" , ");
		DEBUG_PRINTLN(c2);
*/
		if(SensorGroup::realtime_current < c1 ){
			return(1);
		}
		else if(SensorGroup::realtime_current > c2 ) {
			return(2);
		}
		return(0);
}

int SensorGroup::check_flow_alarm(){
		if(os.options[OPTION_FLOW_ALARM] == 0 ) return(0);  	//if alarm enabled check flow alarm
		float v1 = (SensorGroup::realtime_GPM * 8.0);
		float v2 = ((float)SensorGroup::flow_refval * (100 - SensorGroup::flow_alarm_range))/100;
		float v3 = ((float)SensorGroup::flow_refval * (100 + SensorGroup::flow_alarm_range))/100;
/*		DEBUG_PRINT(" realtime_GPM , v2, v3:  ");
		DEBUG_PRINT(realtime_GPM);
		DEBUG_PRINT(" , ");
		DEBUG_PRINT(v2/8.0);
		DEBUG_PRINT(" , ");
		DEBUG_PRINTLN(v3/8.0);
*/		
		if(v1 < v2){ 
			return(1);
			}
		else if(v1 > v3) {
			return(2);			
		}
}

void SensorGroup::station_started(byte current_sid) {
	SensorGroup::sid = current_sid;
	SensorGroup::stat_started = true;
}

void SensorGroup::station_stopped(byte current_sid) {
	SensorGroup::sid = current_sid;
	SensorGroup::stat_stopped = true;
	//SensorGroup::stat_started = false;
}

void SensorGroup::program_stopped(){	
	SensorGroup::prog_finished = true;
	//SensorGroup::stat_started = false;
	//SensorGroup::stat_stopped = false;

}

bool flow_timeout(ulong curr_time_ms){
		if( curr_time_ms > (SensorGroup::last_read_impulse_ms + FLOW_TIMEOUT)){ return(true);}  // Flow stopped
			else return(false);	
}

void SensorGroup::read_flow_sensor(){
	noInterrupts();
	SensorGroup::last_sensor_impulses = SensorGroup::sensor_impulses;
	SensorGroup::last_read_impulse_ms = SensorGroup::flow_last_impulse_ms;
	interrupts();
}

//******* FSM state change and state_entry() handling
//*******
void SensorGroup::change_state(byte chg_state, ulong curr_time_s){

  switch(chg_state){ 
	case NO_FLOW:			// Initial state, no flow detected
		SensorGroup::window_impulses = 0;
		break;
	case LEAKAGE_FLOW:		//Flow detected when no Station runs and Freeflow disabled
		SensorGroup::init_station_count(curr_time_s);
		SensorGroup::alert(LOGDATA_ALARM_LEAKAGE_START, curr_time_s);
		break;
	case FREE_FLOW:			//Flow detected when no Station runs and Freeflow enabled
		SensorGroup::init_station_count(curr_time_s);
		break;
	case STATION_START:		// Let flow build up at Station start
		read_station_data(sid);
		SensorGroup::init_station_count(curr_time_s);
		SensorGroup::sec_timer = curr_time_s;
/*		
			DEBUG_PRINT("prog  / last_prog / day_imp:  ");
			DEBUG_PRINT(SensorGroup::prog_impulses);
			DEBUG_PRINT(" / ");
			DEBUG_PRINT(SensorGroup::last_prog_impulses);
			DEBUG_PRINT(" / ");
			DEBUG_PRINTLN(SensorGroup::day_impulses);
*/			
		break;
	case STATION_FLOW:		// Flow during normal Station run with Alarm handling
		break;
	case STATION_FLOW_CAL:	//Flow during normal Station run and Calibration
		break;
	case PROGRAM_HOLD:		// Waits for next Station run if Program not finished
		//write_log(LOGDATA_STATION, curr_time_s);
		SensorGroup::prog_impulses += SensorGroup::station_impulses;
		break;
	case STFLOW_ALARM:		// During Station run Flow Alarm detected
		break;
	case STCURRENT_ALARM:	// During Station run Current Alarm detected
		break;
	case PROG_FINISH:		//  Let flow settle to zero after the last Station off
		SensorGroup::sec_timer = curr_time_s;
		break;
	case FAILED_STATE:		// In case of failed operation
	default:					// In case of failed operation
		SensorGroup::fsm_fail_counter++;
/*
		DEBUG_PRINT("FSM failed state/event: ");
		DEBUG_PRINT(FAILED_STATE);
		DEBUG_PRINT(" / ");
		DEBUG_PRINT(event);
		DEBUG_PRINT("   Fail Counter: ");
		DEBUG_PRINTLN(SensorGroup::fsm_fail_counter);
*/
		write_log(LOGDATA_FAILED_STATE, curr_time_s, 0);
		chg_state = NO_FLOW;
		break;
	}
	
	SensorGroup::old_state = SensorGroup::current_state;
	SensorGroup::current_state = chg_state;

	DEBUG_PRINT("Exiting change state EVENT / old_state / STATE :  ");
	DEBUG_PRINT(event);
	DEBUG_PRINT(" / ");
	DEBUG_PRINT(SensorGroup::old_state);
	DEBUG_PRINT(" / ");
	DEBUG_PRINTLN(SensorGroup::current_state);
/**/	
}// End of change_state()


//******FSM state_do() & state_exit() functions, executed in every second in the main loop
//******
void SensorGroup::flow_loop(ulong current_time_s){  
	ulong curr_time_ms = millis();  //read msec timer
	read_flow_sensor();
	SensorGroup::event = NOTHING;

	// calculating the flow, the quantity, and current values
/*	DEBUG_PRINT("stat_stopped / prog_finished:  ");
	DEBUG_PRINT(stat_stopped);
	DEBUG_PRINT(" / ");
	DEBUG_PRINTLN(prog_finished);
*/	
	if((SensorGroup::current_state != NO_FLOW)  && !stat_stopped && !prog_finished)	{  //&& (SensorGroup::current_state != PROGRAM_HOLD)
		if(current_time_s > (SensorGroup::measure_window_start + FLOWCOUNT_RT_WINDOW)){  //realtime measurement
			SensorGroup::window_impulses = SensorGroup::last_sensor_impulses - SensorGroup::measure_start_imp;
			SensorGroup::realtime_GPM = (SensorGroup::last_sensor_impulses - SensorGroup::measure_start_imp)
			 / SensorGroup::flow_pulse_rate_IpG / (FLOWCOUNT_RT_WINDOW / 60.0);
			SensorGroup::measure_window_start = current_time_s;
			SensorGroup::measure_start_imp = SensorGroup::last_sensor_impulses;
			SensorGroup::flow_valid_flag = 1;
			}
		else 
			if(!flow_valid_flag && ((SensorGroup::last_sensor_impulses - SensorGroup::measure_start_imp) > 4) && 
			realtime_GPM < 0.1 ){
				SensorGroup::realtime_GPM = ((SensorGroup::last_sensor_impulses - SensorGroup::measure_start_imp) / 
				SensorGroup::flow_pulse_rate_IpG) / ((current_time_s - measure_window_start) / 60.0);
			}
	SensorGroup::prog_impulses = SensorGroup::last_sensor_impulses;
	SensorGroup::station_impulses = SensorGroup::last_sensor_impulses - SensorGroup::station_start_impulses;  //since station started
	SensorGroup::realtime_gallons = SensorGroup::station_impulses / SensorGroup::flow_pulse_rate_IpG;
	SensorGroup::realtime_current = os.read_current();
	}
/*		DEBUG_PRINT("last_sensor: ");
		DEBUG_PRINT(last_sensor_impulses);
		DEBUG_PRINT("   MeasStartImp: ");
		DEBUG_PRINT(measure_start_imp);
		DEBUG_PRINT("   MeasStartWindow: ");
		DEBUG_PRINTLN(measure_window_start);
		DEBUG_PRINT("Impulses: ");
		DEBUG_PRINTLN(station_impulses);
		DEBUG_PRINT("Realtime Gallons: ");
		DEBUG_PRINTLN(realtime_gallons);
		DEBUG_PRINT("Electric current: ");
		DEBUG_PRINTLN(realtime_current);
		DEBUG_PRINT("Realtime GPM: ");
		DEBUG_PRINTLN(realtime_GPM);
*/

  switch (SensorGroup::current_state){
	case STATION_FLOW:
		if(SensorGroup::stat_stopped){
			SensorGroup::stat_stopped = false;
			SensorGroup::event = STATION_STOPPED;
			next_state = PROGRAM_HOLD;
			break;					
		}
		if(flow_timeout(curr_time_ms)){
			SensorGroup::event = IMPULSES_TIMEOUT;
			SensorGroup::alert(LOGDATA_ALARM_FLOW_STOPPED, current_time_s);
			next_state = PROGRAM_HOLD;
			break;					
			}
		//if the station is not excluded of alarm / not special, flow settled and data is calibrated
		if( !SensorGroup::excluded && !SensorGroup::special && SensorGroup::flow_valid_flag 
			&& SensorGroup::curr_refval < 0xFF && SensorGroup::curr_refval > 0) {
/*				DEBUG_PRINT("Electric current: ");
				DEBUG_PRINT(realtime_current);
				DEBUG_PRINT("     Realtime GPM: ");
				DEBUG_PRINTLN(realtime_GPM);
				DEBUG_PRINT("OptionsAlarm curr / flov/ curr refval / flow_refval:  ");
				DEBUG_PRINT(os.options[OPTION_CURR_ALARM]);
				DEBUG_PRINT(" / ");
				DEBUG_PRINT(os.options[OPTION_FLOW_ALARM]);
				DEBUG_PRINT(" / ");
				DEBUG_PRINT(curr_refval);
				DEBUG_PRINT(" / ");
				DEBUG_PRINTLN(flow_refval);
				Serial.flush();
*/
				switch(SensorGroup::check_current_alarm()){
					case 0:
						switch(SensorGroup::check_flow_alarm()){
							case 0:
							break;
							case 1:
								SensorGroup::event = OUT_OF_RANGE_LOWFLOW;
								SensorGroup::alert(LOGDATA_ALARM_FLOW_LOW, current_time_s);
								SensorGroup::next_state = STFLOW_ALARM;
								break;
							case 2:
								SensorGroup::event = OUT_OF_RANGE_HIFLOW;
								SensorGroup::alert(LOGDATA_ALARM_FLOW_HIGH, current_time_s);
								SensorGroup::next_state = STFLOW_ALARM;
						}
						break;	
					case 2:
						SensorGroup::event = OUT_OF_RANGE_HICURRENT;
						SensorGroup::alert(LOGDATA_ALARM_CURRENT_HIGH, current_time_s);
						SensorGroup::next_state = STCURRENT_ALARM;
						break;
					case 1:
						SensorGroup::event = OUT_OF_RANGE_LOWCURRENT;
						SensorGroup::alert(LOGDATA_ALARM_CURRENT_LOW, current_time_s);
						SensorGroup::next_state = STCURRENT_ALARM;
				}
			}
		break;
	case STATION_FLOW_CAL:
		if(SensorGroup::stat_stopped){
			SensorGroup::stat_stopped = false;
			SensorGroup::event = STATION_STOPPED;
/*	DEBUG_PRINT("End of loop EVENT / STATE / realtime_GPM / real_current: ");
	DEBUG_PRINT(event);
	DEBUG_PRINT(" / ");
	DEBUG_PRINT(SensorGroup::current_state);
	DEBUG_PRINT(" / ");
	DEBUG_PRINT(SensorGroup::realtime_GPM);
	DEBUG_PRINT(" / ");
	DEBUG_PRINTLN(SensorGroup::realtime_current);
*/			
			if((SensorGroup::realtime_GPM > 1.0) && (SensorGroup::realtime_current > 40) &&
				(current_time_s - station_start_time) >= 31){  
				// min value to save calibration: 1, 40, min station run 30 sec

				// save the flow reference to the NVM, LSB = 0,125 gallon, minimum = 0x08, maximum = 0xFFFE
				uint16_t f = (uint16_t)(SensorGroup::realtime_GPM * 8.0);
				byte v = f & 0xFF;
				nvm_write_byte((byte*)(ADDR_NVM_FLOW_REFS + (2*sid)), v ); //save low byte
				v = (f >> 8) & 0xFF;  //  shift the high byte
				nvm_write_byte( (byte*)(ADDR_NVM_FLOW_REFS + 1 + (2*sid)), v ); //save high byte
				
				// save the current reference, LSB = 4mA, minimum = 0x0A, maximum = 0xFE
				v = (SensorGroup::realtime_current >> 2) & 0xFF; //divide by 4
				if(v == 0xFF) v = 0xFE;
				nvm_write_byte((byte*)(ADDR_NVM_CURR_REFS + (sid)), v );

				write_log(LOGDATA_CALIBRATED, current_time_s, 0);
				
/*				#if defined(SERIAL_DEBUG)
					for(int i=0; i< MAX_NUM_STATIONS; i++){	
						SensorGroup::curr_refval = os.station_attrib_bits_read(ADDR_NVM_CURR_REFS + (i));
						DEBUG_PRINT(curr_refval);
						DEBUG_PRINT(", ");
					}
					DEBUG_PRINTLN(",");

					for(int i=0; i< MAX_NUM_STATIONS; i++){	
						SensorGroup::flow_refval = (os.station_attrib_bits_read(ADDR_NVM_FLOW_REFS + 1 + (2*i)) << 8) +  // high byte
								os.station_attrib_bits_read(ADDR_NVM_FLOW_REFS + (2*i) );  //low byte
						DEBUG_PRINT(flow_refval);
						DEBUG_PRINT(", ");
					}
					DEBUG_PRINTLN(",");
				#endif
*/			}	
		next_state = PROGRAM_HOLD;			
		break;
		}
		if(flow_timeout(curr_time_ms)){
			SensorGroup::event = IMPULSES_TIMEOUT;
			SensorGroup::alert(LOGDATA_ALARM_FLOW_STOPPED, current_time_s);
			next_state = PROGRAM_HOLD;			
			}		
		break;
			
	case PROGRAM_HOLD:
		if(SensorGroup::stat_stopped){
			SensorGroup::stat_stopped = false;
			SensorGroup::event = STATION_STOPPED;
			break;						
			}
		if(prog_finished || !os.status.program_busy){
			prog_finished = false;
			SensorGroup::event = PROG_QUE_EMPTY;
			next_state = PROG_FINISH;						
			break;
			}
		if(stat_started){
			stat_started = false;
			SensorGroup::event = STATION_STARTED;
			next_state = STATION_START;
			}
		break;
		
	case STATION_START:	
		if(SensorGroup::stat_stopped){
			SensorGroup::stat_stopped = false;
			SensorGroup::event = STATION_STOPPED;
			next_state = PROGRAM_HOLD;			
			}
		if(SensorGroup::sec_timer + SETTLING_UP < current_time_s){		//Event: settling timeout ended
			SensorGroup::event = SETTLE_TIMEOUT;
			if((os.options[OPTION_CURR_ALARM] != 0 || os.options[OPTION_FLOW_ALARM] != 0 ) && SensorGroup::curr_refval == 0xFF){
				next_state = STATION_FLOW_CAL;
				}
				else next_state = STATION_FLOW;
			}
		break;
		
	case PROG_FINISH:	
		if(SensorGroup::sec_timer + SETTLING_DOWN < current_time_s){		//Event: settling timeout ended
			SensorGroup::event = SETTLE_TIMEOUT;
			write_log(LOGDATA_PROGFLOW, current_time_s, 0);
			write_log(LOGDATA_PROGFLOW2, current_time_s, 0);
			
			day_impulses = (os.station_attrib_bits_read(ADDR_NVM_IMPULSES + 3 ) << 8) +
						os.station_attrib_bits_read(ADDR_NVM_IMPULSES + 2 );
			SensorGroup::day_impulses += SensorGroup::prog_impulses;
			uint16_t f = (uint16_t)(day_impulses);
			byte v = f & 0xFF;
			nvm_write_byte((byte*)(ADDR_NVM_IMPULSES + 2), v ); //save low byte
			v = (f >> 8) & 0xFF;  //  shift the high byte
			nvm_write_byte( (byte*)(ADDR_NVM_IMPULSES + 3), v ); //save high byte
			
			SensorGroup::last_prog_impulses = SensorGroup::prog_impulses;
			f = (uint16_t)(last_prog_impulses);
			v = f & 0xFF;
			nvm_write_byte((byte*)(ADDR_NVM_IMPULSES ), v ); //save low byte
			v = (f >> 8) & 0xFF;  //  shift the high byte
			nvm_write_byte( (byte*)(ADDR_NVM_IMPULSES + 1), v ); //save high byte
						
/*			DEBUG_PRINT("prog  / last_prog / day_imp:  ");
			DEBUG_PRINT(SensorGroup::prog_impulses);
			DEBUG_PRINT(" / ");
			DEBUG_PRINT(SensorGroup::last_prog_impulses);
			DEBUG_PRINT(" / ");
			DEBUG_PRINTLN(SensorGroup::day_impulses);
*/			
			next_state = NO_FLOW;
			init_prog_count(current_time_s);
						
			}
		break;
		
	case NO_FLOW:
		if(stat_started){
			SensorGroup::stat_started = false;
			SensorGroup::event = STATION_STARTED;
			init_prog_count(current_time_s);
			next_state = STATION_START;
		break;
		}
		if(stat_stopped){
			SensorGroup::stat_stopped = false;
			SensorGroup::event = STATION_STOPPED;
			next_state = PROGRAM_HOLD;			
		break;
		}
		if((SensorGroup::old_sensor_impulses) < SensorGroup::last_sensor_impulses){  //any pulses arrived?

			DEBUG_PRINT("old_sensor  / last_sensor:  ");
			DEBUG_PRINT(SensorGroup::old_sensor_impulses);
			DEBUG_PRINT(" / ");
			DEBUG_PRINTLN(SensorGroup::last_sensor_impulses);

			SensorGroup::event = IMPULSES_STARTED;
//			init_prog_count(current_time_s);
			if (os.status.program_busy) {
				next_state = STATION_START;
				}
			else {
				if(os.options[OPTION_FLOW_ALARM] == 2) next_state = FREE_FLOW;
				else next_state = LEAKAGE_FLOW;
				}
		}
		break;
		
	case FREE_FLOW:
		if(stat_stopped){
			SensorGroup::stat_stopped = false;
			SensorGroup::event = STATION_STOPPED;
			next_state = PROGRAM_HOLD;			
			break;
			}
		if(stat_started){
			stat_started = false;
			SensorGroup::event = STATION_STARTED;
			write_log(LOGDATA_FREEFLOW_END, current_time_s, 0);				
			init_prog_count(current_time_s);
			next_state = STATION_START;
			break;
			}
		if(flow_timeout(curr_time_ms)){
			SensorGroup::event = IMPULSES_TIMEOUT;
			write_log(LOGDATA_FREEFLOW_END, current_time_s, 0);
			next_state = NO_FLOW;
			break;
			}
		if(os.options[OPTION_FLOW_ALARM] == 2 && SensorGroup::flow_valid_flag ){		
			if(((current_time_s - SensorGroup::prog_start_time) / 60) > os.options[OPTION_FREEFLOW_TIME]){
			//Freeflow timeout alarm
				SensorGroup::event = FREEFLOW_TIME_ALARM;		
				SensorGroup::alert(LOGDATA_ALARM_FF_TIME, current_time_s);
				next_state = NO_FLOW;
			} 
			else if((SensorGroup::prog_impulses / SensorGroup::flow_pulse_rate_IpG) > os.options[OPTION_FREEFLOW_QUANTITY] ){  //Freeflow max flow alarm
					SensorGroup::event = FREEFLOW_QUANTITY_ALARM;
									
/*						DEBUG_PRINT(" prog_start_time , current_time_s:  ");
						DEBUG_PRINT(prog_start_time);
						DEBUG_PRINT(" , ");
						DEBUG_PRINTLN(current_time_s);
*/										
					SensorGroup::alert(LOGDATA_ALARM_FF_QUANTITY, current_time_s);
					next_state = NO_FLOW;
					}
		}
		break;
		
	case LEAKAGE_FLOW:
		if(stat_stopped){
			SensorGroup::stat_stopped = false;
			SensorGroup::event = STATION_STOPPED;
			next_state = PROGRAM_HOLD;			
			break;
			}
		if(stat_started){
			stat_started = false;
			SensorGroup::event = STATION_STARTED;
			write_log(LOGDATA_ALARM_LEAKAGE_END, current_time_s, 0);
			init_prog_count(current_time_s);
			next_state = STATION_START;
			break;
			}
		if(flow_timeout(curr_time_ms)){
			SensorGroup::event = IMPULSES_TIMEOUT;
			SensorGroup::alert(LOGDATA_ALARM_LEAKAGE_END, current_time_s);
			next_state = NO_FLOW;
			}
		break;
		
	case STFLOW_ALARM:
		if(SensorGroup::stat_stopped){
			SensorGroup::stat_stopped = false;
			SensorGroup::event = STATION_STOPPED;
			next_state = PROGRAM_HOLD;
			break;			
		}
		if(flow_timeout(curr_time_ms)){
			SensorGroup::event = IMPULSES_TIMEOUT;
			SensorGroup::alert(LOGDATA_ALARM_FLOW_STOPPED, current_time_s);
			next_state = PROGRAM_HOLD;			
			break;
			}
		if(os.options[OPTION_FATAL_ALARM] == 1){
			float v1 = (SensorGroup::realtime_GPM * 8.0);
			uint32_t v2 = ((uint32_t)SensorGroup::flow_refval * FATAL_FLOW_LIMIT)/100;
/*				DEBUG_PRINT("Fatal real_GPM*8 , v2:  ");
				DEBUG_PRINT(v1);
				DEBUG_PRINT(" , ");
				DEBUG_PRINTLN(v2); */
			if( v1 > v2 ){		
				SensorGroup::event = FATAL_FLOW_ALARM;
				//in case of fatal error, the disable flag is set to disable from future operation
				SensorGroup::alert(LOGDATA_ALARM_FATAL_FLOW, current_time_s);
				byte d = 1 << (sid);
/*				DEBUG_PRINT("sid / d:  ");
				DEBUG_PRINT(sid);
				DEBUG_PRINT(" / ");
				DEBUG_PRINTLN(d);*/
				d = d | nvm_read_byte((byte*)ADDR_NVM_ALARM_FATAL + (sid >> 3));
				nvm_write_byte((byte*)ADDR_NVM_ALARM_FATAL + (sid >> 3), d );
/*				DEBUG_PRINT("sid / Fatal flag:  ");
				DEBUG_PRINT(sid);
				DEBUG_PRINT(" / ");
				DEBUG_PRINTLN(d);*/
				 //the process_dinamic_events() will handle the disable flag and stops the station
				turn_off_station(sid, current_time_s);
				next_state = PROGRAM_HOLD;			
				}
			}
		break;
		
	case STCURRENT_ALARM:
		if(SensorGroup::stat_stopped){
			SensorGroup::stat_stopped = false;
			SensorGroup::event = STATION_STOPPED;
			next_state = PROGRAM_HOLD;
			break;			
			}
		if(flow_timeout(curr_time_ms)){
			SensorGroup::event = IMPULSES_TIMEOUT;
			SensorGroup::alert(LOGDATA_ALARM_FLOW_STOPPED, current_time_s);
			next_state = PROGRAM_HOLD;
			break;
			}
		if(!SensorGroup::excluded && !SensorGroup::special && SensorGroup::flow_valid_flag ){	// if the station is not excluded and flow data is valid
			switch(SensorGroup::check_flow_alarm()){
				case 0:
				break;
				case 1:
				SensorGroup::event = OUT_OF_RANGE_LOWFLOW;
				SensorGroup::alert(LOGDATA_ALARM_FLOW_LOW, current_time_s);
				SensorGroup::next_state = STFLOW_ALARM;
				break;
				case 2:
				SensorGroup::event = OUT_OF_RANGE_HIFLOW;
				SensorGroup::alert(LOGDATA_ALARM_FLOW_HIGH, current_time_s);
				SensorGroup::next_state = STFLOW_ALARM;
				}
			}
		break;
	
	default: 
		SensorGroup::event = FAILED_STATE_EVENT;
		next_state = FAILED_STATE;
	}
	
	if(!(SensorGroup::event == NOTHING))
		change_state(next_state, current_time_s);
	
	DEBUG_PRINT("EndOfLoop EVENT / STATE / sid / impulses / realtime_GPM / real_current: ");
	DEBUG_PRINT(event);
	DEBUG_PRINT(" / ");
	DEBUG_PRINT(SensorGroup::current_state);
	DEBUG_PRINT(" / ");
	DEBUG_PRINT(sid);
	DEBUG_PRINT(" / ");
	DEBUG_PRINT(SensorGroup::last_sensor_impulses);
	DEBUG_PRINT(" / ");
	DEBUG_PRINT(SensorGroup::realtime_GPM);
	DEBUG_PRINT(" / ");
	DEBUG_PRINTLN(SensorGroup::realtime_current);
/**/	
	SensorGroup::old_sensor_impulses = SensorGroup::last_sensor_impulses;

}//End of flow_loop()

void SensorGroup::day_flow_calc(ulong curr_time){
	day_impulses = (os.station_attrib_bits_read(ADDR_NVM_IMPULSES + 3 ) << 8) +
				os.station_attrib_bits_read(ADDR_NVM_IMPULSES + 2 );
	write_log(LOGDATA_DAYFLOW, curr_time, 0);
	
	last_day_impulses = day_impulses;
	uint16_t f = (uint16_t)(last_day_impulses);
	byte v = f & 0xFF;
	nvm_write_byte((byte*)(ADDR_NVM_IMPULSES + 4), v ); //save low byte
	v = (f >> 8) & 0xFF;  //  shift the high byte
	nvm_write_byte( (byte*)(ADDR_NVM_IMPULSES + 5), v ); //save high byte
	
	day_impulses =0;
	
	nvm_write_byte((byte*)(ADDR_NVM_IMPULSES + 2), 0 ); //save low byte
	v = (f >> 8) & 0xFF;  //  shift the high byte
	nvm_write_byte( (byte*)(ADDR_NVM_IMPULSES + 3), 0 ); //save high byte
	
	
}

uint16_t nvm_read_word(ulong addr){	//low byte address
	return((os.station_attrib_bits_read(addr + 1 ) << 8) +  //read high byte
			os.station_attrib_bits_read(addr));				//read low byte
}

void nvm_write_word(ulong addr, uint16_t f){	//low byte address
	byte v = f & 0xFF;
	nvm_write_byte((byte*)(addr), v );		//save low byte
	v = (f >> 8) & 0xFF;					//shift the high byte
	nvm_write_byte( (byte*)(addr + 1), v ); //save high byte
}

void SensorGroup::check_sensors(ulong curr_time){

	// ====== Check rain sensor status ======
    if (os.options[OPTION_RSENSOR_TYPE] == SENSOR_TYPE_RAIN) { // if a rain sensor is connected
      os.rainsensor_status();
      if (os.old_status.rain_sensed != os.status.rain_sensed) {
        if (os.status.rain_sensed) {
          write_log(LOGDATA_RAINSENSE2, curr_time, 0);	//in SG logs when the rain_sensed ==> 1    
          os.sensor_lasttime = curr_time;  // when rain_sensed == 1, record time, there is no logging in the original version
//          push_message(IFTTT_RAINSENSOR, LOGDATA_RAINSENSE, 1, "");
        } else {
          // rain sensor off, write log
          if (curr_time>os.sensor_lasttime+10) {  // add a 10 second threshold
                                                  // to avoid faulty rain sensors generating
                                                  // too many log records
            write_log(LOGDATA_RAINSENSE2, curr_time, 0);	//in SG logs when the rain_sensed ==> 0     
            write_log(LOGDATA_RAINSENSE, curr_time, 0);	//original OS log: only when rain_sensed ==> 0 
           push_message(IFTTT_RAINSENSOR, LOGDATA_RAINSENSE, 0, "");
          }
        }
        os.old_status.rain_sensed = os.status.rain_sensed;
      }
    }
 
    // ====== Check soil sensor1 status ======
    if (os.options[OPTION_SSENSOR_1] != SENSOR_TYPE_NONE) { // if digital soil sensor1 is connected
      os.soilsensor_status();
      if (os.old_status.dry_soil_1 != os.status.dry_soil_1) {	//1: enable irrigation, 0:disable irrigation
		    if (curr_time > os.s1sensor_lasttime + 3) {			// add a 10 second threshold to avoid faulty sensing, generating too many log records
				os.s1sensor_lasttime = curr_time;				// soil sensor, record time of change
				os.old_status.dry_soil_1 = os.status.dry_soil_1;
			   	write_log(LOGDATA_SOIL1, curr_time, 0);
/*				if (os.status.dry_soil_1) {								
//		            push_message(IFTTT_SOILSENSOR, LOGDATA_SOIL1, 1);		
				} else {												
//					push_message(IFTTT_SOILSENSOR, LOGDATA_SOIL1, 0);
					}		  
*/				}
		}
    }

    // ====== Check soil sensor2 status ======
    if (os.options[OPTION_SSENSOR_2] != SENSOR_TYPE_NONE) { // if digital soil sensor2 is connected
      os.soilsensor_status();
      if (os.old_status.dry_soil_2 != os.status.dry_soil_2) {	//1: enable irrigation, 0:disable irrigation
			    if (curr_time > os.s2sensor_lasttime + 3) {			// add a few second threshold to avoid faulty sensing, generating too many log records
				os.s2sensor_lasttime = curr_time;				// soil sensor, record time of change
				os.old_status.dry_soil_2 = os.status.dry_soil_2;
			   	write_log(LOGDATA_SOIL2, curr_time, 0);
/*				if (os.status.dry_soil_2) {								
//		            push_message(IFTTT_SOILSENSOR, LOGDATA_SOIL2, 1);		
				} else {												
//					push_message(IFTTT_SOILSENSOR, LOGDATA_SOIL2, 0);
					}		  
*/				}
		}
    }
}

void SensorGroup::alert(byte type, ulong current_time) {

	// copy the message on LCD
	strncpy_P0(last_alarm_LCD, alarm_message_text + 16 * (type-0x10), 16);
	
/*	TODO: display on LCD the last alarm message in multiplexed form
	os.lcd.setCursor(0, 0);
	os.lcd.print(last_alarm_LCD);
*/
	// Serial Monitor print of alerts
	switch (type) {
	case LOGDATA_ALARM_FLOW_STOPPED:
		DEBUG_PRINTLN("Alert! Flow stopped!");
		alarm_cnt++;
		break;
	case LOGDATA_ALARM_FF_QUANTITY:
		DEBUG_PRINTLN("Alert! Freeflow max impulses reached!");
		alarm_cnt++;
		break;
	case LOGDATA_ALARM_FF_TIME:
		DEBUG_PRINTLN("Alert! Freeflow max time reached!");
		alarm_cnt++;
		break;
	case LOGDATA_ALARM_LEAKAGE_START:
		DEBUG_PRINTLN("Alert! LEAKAGE detected!");
		alarm_cnt++;
		break;
	case LOGDATA_ALARM_LEAKAGE_END:
		DEBUG_PRINTLN("Alert! Leakage finished!");
		alarm_cnt++;
		break;
	case LOGDATA_ALARM_FLOW_HIGH:
		DEBUG_PRINTLN("Alert! HIGH FLOW detected!");
		alarm_cnt++;
		break;
	case LOGDATA_ALARM_FLOW_LOW:
		DEBUG_PRINTLN("Alert! LOW FLOW detected!");
		alarm_cnt++;
		break;
	case LOGDATA_ALARM_FATAL_FLOW:
		DEBUG_PRINTLN("Alert! FATAL FLOW detected! Station disabled!");
		alarm_cnt++;
		break;
	case LOGDATA_ALARM_CURRENT_HIGH:
		DEBUG_PRINTLN("Alert! HIGH STATION CURRENT detected!");
		alarm_cnt++;
		break;
	case LOGDATA_ALARM_CURRENT_LOW:
		DEBUG_PRINTLN("Alert! LOW STATION CURRENT detected!");
		alarm_cnt++;
		break;
	default:
		alarm_cnt++;
		DEBUG_PRINTLN("Unknown alert!");
		break;
	}

	//write alerts to log
	write_log(type, current_time, 0);
}

