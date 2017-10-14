/*
	This is the header file of SensorGroup.cpp

	The entire file (if not stated otherwise) is written by CV.
*/

#ifndef SENSORGROUP_H

	#define SENSORGROUP_H

	
	// ### Flow Sensor defines
	#define	FLOW_SENSOR_DEBOUNCE		10		// the debounce time of the flow sensor in milliseconds
	#define FLOW_TIMEOUT				5000	// the timeout of the flow sensor in milliseconds
												// if no pulse arrives in this time window, we say that we have no flow 
	#define SETTLING_UP					7		// the flow build up interval second
	#define SETTLING_DOWN				6		// the flow settle down to zero interval second
	#define FLOWSENSOR_TYPE_SEL			15		// if PULSE_RANGE > value, the flow is sensed by hall effect sensor, no debounce needed = 0 
	#define FATAL_FLOW_LIMIT			200		// the level of Fatal flow alarm in %
	#define HALL_SENSOR_LIMIT			3		// from this impulses per gallon hall sensor in the flow meter 
	//#define FLOWCOUNT_RT_WINDOW				//  (defines.h) specifies the interval on which we calculate the realtime PPM and GPM in seconds
												// depends on the flow meter there should be about 3 impulses in the window  at 10 l/min
	// ### END: FlowSensor defines

	// ### Soil Sensor defines
	#define SOIL_SAMPLE_INTERVAL		7		// the soil sensor sampling interval seconds
	
	#define SOIL_HIT_NUMBER				1		// the number of consecutive dry readings needed for action
	#define SOIL_ENABLED				1		// the value for the state "we have to water"
	#define SOIL_DISABLED				0		// the value for the state "we don't have to water"
	// ### END: Soil Sensor defines

	#include "defines.h"
	#include "OpenSprinkler.h"
	#include <SDFat.h>

// the possbile states of the flow handler FSM
typedef enum {
	NO_FLOW,		//0,FLOW_NOFLOW
	FREE_FLOW,		//1, FLOW_FREEFLOW
	LEAKAGE_FLOW,	//2
	STATION_START,	//3
	STATION_FLOW,		//4
	STATION_FLOW_CAL,   //5
	STFLOW_ALARM,		//6
	STCURRENT_ALARM,	//7
	PROGRAM_HOLD,		//8
	PROG_FINISH,		//9
	FAILED_STATE		//10

}FL_STATES;

// the possible events in flow handler FSM
typedef enum {
	NOTHING,				//0
	IMPULSES_STARTED,		//1
	IMPULSES_TIMEOUT,		//2
	STATION_STARTED,		//3
	SETTLE_TIMEOUT,			//4
	STATION_STOPPED,		//5
	FREEFLOW_TIME_ALARM,	//6
	FREEFLOW_QUANTITY_ALARM,//7
	OUT_OF_RANGE_HICURRENT,	//8
	OUT_OF_RANGE_LOWCURRENT,//9
	OUT_OF_RANGE_HIFLOW,	//10
	OUT_OF_RANGE_LOWFLOW,	//11
	FATAL_FLOW_ALARM,		//12
	PROG_QUE_EMPTY,			//13
	FAILED_STATE_EVENT		//14
}FL_EVENTS;


class SensorGroup {
public:

	// the FSM controls
	static byte current_state;
	static byte old_state;
	static byte next_state;
	static byte event;
	static ulong fsm_fail_counter;

	// the stations data
	// reference values copied from NVM at STATION_FLOW entry() function
	// Current ref deleted to 0xFF: at Fact Reset, at OPTION_CAL_REQUEST == 1, or Master station changed
	static bool excluded;	//set if station is excluded from alarm handling
	static bool special;	//set if station is special, no alarm handling is done
	static byte sid;		//running station id
	static bool stat_started;	//set when a station started
	static bool stat_stopped;	//set when a station stopped
	static bool prog_finished;	//set when a program stopped
	
	//flow sensor
	static volatile ulong flow_last_impulse_ms;	// the time of the last impulse from the flow sensor in millisecond
	static volatile ulong sensor_impulses; //counts in flow in ISR routine
	// last_read_impulse_ms		the last read of "flow_last_impulse_ms"
	// last_sensor_impulses		the last read of "sensor_impulses" value
	// old_sensor_impulses		the previous value of "last_sensor_impulses" value
	static ulong last_read_impulse_ms, last_sensor_impulses, old_sensor_impulses;
	static float flow_pulse_rate_IpG;	// the impulse/gallon of the flow sensor
	static bool hall_effect_sensor;		//flow sensor has hall(magnetic) detector built in

	//alarm
	static byte curr_refval;	// the current Station (sid) CURRENT reference not calibrated if 0xFF
	static uint16_t flow_refval;	// The current Station FLOW reference not calibrated if 0xFFFF
	static byte flow_alarm_range; // the maximal difference in station flow in percent (%)
	static byte current_alarm_range; // the maximal difference in station current usage before alert in percent(%)
	static byte alarm_cnt;


	//results
	// realtime_GPM				the realtime GPM of the state, calculated continuously (interval specified in CURRENT_PPM_GPM_TIME_LENGTH)
	// station_impulses			the number of GPM impulses in a stat
	// prog_impulses			the number of impulses at the end of program
	// day_impulses				the number of impulses in the day
	// last_day_impulses
	// realtime_current			the actual current of the station in mA
	static bool display_GPM;
	static float realtime_GPM;
	static float realtime_gallons;
	static ulong realtime_current;
	static ulong station_impulses;
	static ulong prog_impulses, last_prog_impulses, day_impulses, last_day_impulses;
	static ulong window_impulses;	


	//measure
	// measure_window_start			
	// measure_start_imp		the measure window entering impulse data
	// prog_start_time				when the actual flow has been initiated, in a program or freeflow
	// station_start_time			the time the state started (e. g.: station or freeflow)
	// sec_timer, msec_timer		timers
	// station_start_impulses		copy of sensor_impulses at station, freeflow, leakage start
	static ulong station_start_impulses;
	static ulong measure_start_imp;
	static ulong measure_window_start, prog_start_time, station_start_time;
	static ulong sec_timer;
	static bool flow_valid_flag;
	static bool prog_start_flag;
	
	static ulong last_soil_read;
	

	// The interrupt method of the flow sensor, runs every time a pulse is sent by the flow sensor
	static void flowsensor_ISR();

	// initialization
	static void init();

	// runs every time a staion is activated
	static void station_started(byte sid);
	
	// runs every time a staion is deactivated
	static void station_stopped(byte sid);
	
	static void program_stopped();
	static void day_flow_calc(ulong curr_time);

	// enable or disable calibration on a station
	static void set_calibration_for_station(byte sid, bool flow_enable_calibration, bool current_enable_calibration);

	// set the soil sensors for a station
	static void set_soil_sensors_for_station(byte sid, bool use_sensor_1, bool use_sensor_2);

	// reset the stored station flow/current values
	static void reset_nominal_station_data(byte sid, bool reset_flow, bool reset_current);
	static void clear_station_calibration(byte sid);
	static void clear_calibration();
	
	// runs in every loop and calculates stuff
	static void loop(ulong current_time);

	// check the rain and soil sensors' status and update the os
	static void check_sensors(ulong curr_time);

	// runs rarer than the main loop, handles the flow sensor realtime calculations
	static void calculate_logdata();
	static void alert(byte type, ulong current_time);
	static void change_state(byte new_state, ulong curr_time_s);
	static void flow_loop(ulong current_time_s);
	static void init_prog_count(ulong current_time);
	static void init_station_count(ulong current_time);
	static int check_flow_alarm();
	static int check_current_alarm();
	static void read_flow_sensor();
private:
	
	// write a station's data to the SD card
	static void station_data_to_EEPROM(byte sid);
	
	// write all station data to the SD card
	static void all_station_data_to_EEPROM();

	// check and create the nessesary data structure on the SD card
	static bool init_ext_EEPROM();
};
#endif // !FLOWSENSOR_H