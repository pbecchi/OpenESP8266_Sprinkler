
// dimension of pd 3 bytes (flag,day1,day2) + 4 int (start times) +nvalves* 1 int (duration) = 7+2*nvalves +nchar  eg:30
#include "Glx_SWindows.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SSIDPASSWORD.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "libsel.h"
#include "Eeprom_ESP.h"
#include "Def.h"
#include <TimeLib.h> 
#include <WiFiUdp.h>
#include <FS.h>
#include <ThingSpeak.h>

//#define FTP
#define IOT
//////////////////////////////////////   OTA/////////////////////////////////////////
#define OTA
#ifdef OTA
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#endif
////////////////////////////////////////////////////////////////////////////////////////
//#include "Config.h"
//#include "../Defines.h"
//#include "../OpenSprinkler.h"
//#include "../utils.h"
//#include "../OpenSprinklerProgram.h"

// how many clients should be able to telnet to this ESP8266----------------------eelogger-------------------------
#include <Wire.h>

#include <RTClib.h>
#include <NPTtimeSync.h>
////////////////////////////////////////////////////////
#include <SPI.h>
#include <Adafruit_ILI9341esp.h>
#include <Adafruit_GFX.h>
#include <XPT2046.h>
#include "Glx_SWindows.h"

#include "Def.h"
//////////////////////////////////////////////////////////
#define NC 1
#define MAX_SRV_CLIENTS NC
#define N_OS_STA 4
/*
#ifdef FTP

#include <ESP8266WebServer.h>
#include <ESP8266FtpServer.h>

FtpServer ftpSrv;

#endif
*/
File logfile;
RTC_DS1307 RTC;
int p = 0, pold;							//EEprom read pointer
////////////////////////////////
#define LCD_TOUCH
#ifdef LCD_TOUCH
//-------------------------LCD definitions
#define TFT_DC 2
#define TFT_CS 15
#define N_Maxcurves 20

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
XPT2046 touch(/*cs=*/ 16, /*irq=*/ 0);

Adafruit_GFX_Button button;
Glx_MWindowsClass MWin;
Glx_keyborad MyKeyb;
Glx_TWindows TWin,TFWin;
Glx_GWindowsClass Gwin;
Graf myGraph[N_Maxcurves];

#define SERIAL TWin

#else
#define SERIAL Serial
#endif

//--------------------------------------------------------------------------eelogger-------------------------------
//#define APIWU

#ifdef APIWU
#define MAX_JSON_STRING 2600
#else
#define MAX_JSON_STRING 600
#endif
char json[ MAX_JSON_STRING];

#define SEQ_EE_START 0
#ifdef TELNET


static	bool noClient = true;
	WiFiServer server(23);
	WiFiClient Tclient;
	unsigned long TimeOUT,posFile=0,oldpos=0;
	
#define MAXBYTES 1000000
//#define POS posFile=logfile.position();if(posFile>MAXBYTES)reset_logfile()
//#define LOG oldpos=logfile.position()//logfile.seek(0,SeekEnd)

#define POS {}
#define LOG {}
#define LOGFILE

#ifdef LOGFILE
#define SP(x) SERIAL.print(x);Serial.print(x);if(Tclient)(Tclient.print(x));LOG;logfile.print(x);POS
#define SPL(x) SERIAL.println(x);Serial.println(x);if(Tclient)Tclient.println(x);TimeOUT = millis();LOG;logfile.println(x);POS
#define SPLF(x,y) SERIAL.println(x,y);Serial.println(x,y);if(Tclient)(Tclient.println(x,y));TimeOUT = millis();LOG;logfile.println(x,y);POS
#define SP_D(x) Serial.print(x);if(Tclient)(Tclient.print(x));LOG;logfile.print(x);POS
#define SPL_D(x) Serial.println(x);if(Tclient)(Tclient.println(x));TimeOUT = millis();LOG;logfile.println(x);POS
#define SPLF_D(x,y) Serial.println(x,y);if(Tclient)(Tclient.println(x,y));TimeOUT = millis();LOG;logfile.println(x,y);POS
#else  //LOGFILE
#define SP(x) SERIAL.print(x);Serial.print(x);if(Tclient)(Tclient.print(x))
#define SPL(x) SERIAL.println(x);Serial.println(x);if(Tclient)Tclient.println(x)
#define SPLF(x,y) SERIAL.println(x,y);Serial.println(x,y);if(Tclient)(Tclient.println(x,y))
#define SP_D(x) Serial.print(x);if(Tclient)(Tclient.print(x))
#define SPL_D(x) Serial.println(x);if(Tclient)(Tclient.println(x))
#define SPLF_D(x,y) Serial.println(x,y);if(Tclient)(Tclient.println(x,y))
#endif //LOGFILE
#else
#define SP(x) SERIAL.print(x);logf.print(x);Serial.print(x)
#define SPL(x) SERIAL.println(x);logf.println(x);Serial.println(x)
#define SPLF(x,y) SERIAL.println(x,y);logf.println(x);Serial.println(x,y)
#define SP_D(x) Serial.print(x);logf.print(x)
#define SPL_D(x) logf.println(x);Serial.println(x)
#define SPLF_D(x,y) logf.println(x);Serial.println(x,y)
#endif
#define SPS_D(x) SP_D(" ");SP_D(x)
#define SPT(x,y)  SP(x);SP(":");SP(y/10);SP(y%10)
#define SEQ_EE_START 0
#define MAXSEQ 100
#define NUM_OPTIONS 45
#define PD root["pd"]

///////////////////////////////////////////////////EEPROM MEMORY/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 0<sequn !1<-seq struct.45b*100 elements |1300 EEindex 1390<EEfill |1400<----- prog.structures  size* n.programs|2500 rain |2650 ET0 |3050 cD |
//  3200 programdata structures 148*N_OS_STA-|3800<-options[45,N_OS_STA<5]--|4080
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define CURRENT_DATA_POS 3050
#define EE_INDEX_POS 1300
#define	OPTION_STATION_DELAY_TIME 17
#define ADDR_EE_OPTIONS 3800
#define EEPROM_PROGSTART 1400
#define EE_PROG_POS 1390
#define NUM_OPTIONS 45
#define MAX_NUM_STATIONS  (8) //with no expensions boards
#define PROGRAM_NAME_SIZE 12
#define PROGRAMSTRUCT_SIZE sizeof(ProgramStruct)
#define MAX_NUM_STARTTIMES  4
	struct Lastrun {
		uint8 pid;
		int duration;
		int endtime;
	};
	struct Stat {
		byte pid;
		int startime;
		int remain;
	};
	struct Status {
		ulong curr_time;
		byte nboards;
		bool enabled;
		bool rain_delayed;
		bool rain_sensed;
		uint32 rd_stop_time;
		ulong checkwt_lasttime;
		ulong checkwt_success_lasttime;
		byte station_bits[6];
		Lastrun lastrun;
		Stat station[16];
	};
#define TOL_DUR_SEQ 40 //sec. toll. for duration flux check
#define TOL_FLUX_SEQ 50 //liters/hour toll for flux comp.
	struct sequence {           // EE size 13*100 bytes
		int start;
		ulong dur;
		byte valv;
		byte day0;
		byte day1;
		byte flux;
		byte flags;
		byte progIndex;
		bool Check_day_match(DateTime t) {

			byte weekday_t = t.dayOfTheWeek();        // weekday ranges from [0,6] within Sunday being 1
			byte day_t = t.day();
			byte month_t = t.month();
			byte days[] = { day0,day1 };
//			if (cD.rain_delay[valv / 10] >t.unixtime()) return false; //----- rain delay;
			byte wd = (weekday_t + 5) % 7;
			byte dt = day_t;
			byte type = (flags & 0B0110000) >> 4;
			byte oddeven = 0;
	//		SP_D("day "); SP_D(day_t); SP_D("day[0]"); SP_D(days[0]);SP_D("t."); SP_D(type);
			// check day match
			if (type == 0)
			{   // weekday match
				//SP_D("wd"); SP_D(1 << wd);
				if (!(days[0] & (1 << wd)) )
				{
				//	SPL_D("no");
					return false;
				}

			}
			else if (type == 3)
			{

		//		SPS_D(((t.unixtime() / SECS_PER_DAY) % days[1]));
				// this is an inverval program
				if (byte((t.unixtime() /  SECS_PER_DAY) % days[1]) != days[0])  return false;

			}
			//SPL_D("Y");
			/*
			// check odd/even day restriction
			if (!oddeven) {}
			else if (oddeven == 2) {
				// even day restriction
				if ((dt % 2) != 0)  return 0;
			}
			else if (oddeven == 1) {
				// odd day restriction
				// skip 31st and Feb 29
				if (dt == 31)  return 0;
				else if (dt == 29 && month_t == 2)  return 0;
				else if ((dt % 2) != 1)  return 0;
			}*/
		//	if (type == 3) {
		//		int  rest = int(t.unixtime() / 24 / 3600) % days[1] - days[0]; return rest;	}
		//	else
				return true;
		}
		int Check_Flux(uint16_t startime, int duration, int  fluxp) 
		{					//check flux reading ret 0 no match 1 full match 2 flux change
			SP_D(" daych start "); SP_D(int(startime - start * 30)); SP_D(" ");//comp. days
				if (abs(int((startime-15) -start*30) ) < 40)	//->15*2 sec delay for startup aquisition
				{
					SP_D(" dur1 "); SP_D(duration); SP_D(" dur2 ");SP_D (dur); SP_D(" ");                         //comp.minutes
					if (abs(int(duration) - int(dur)) < TOL_DUR_SEQ) {     //comp.seconds
						SP_D(" Fold "); SP_D(flux*20); SP_D(" Fnew ");  SPL_D(fluxp);
						if (abs(int(flux*20) - fluxp) > TOL_FLUX_SEQ)
						{
							

							return int(flux*20) - fluxp; //return flux diff
						}

						else				//  match!!
							return 0;
					}
					
				}
				return -10000;  //no match
			}
	};

	class ProgramStruct {
	public:

		byte 	enabled :1;  // HIGH means the program is enabled

						   // weather data
		byte use_weather :1;

		// odd/even restriction:
		// 0->none, 1->odd day (except 31st and Feb 29th)
		// 2->even day, 3->N/A
		byte oddeven :2;

		// schedule type:
		//0 weekly, 1->biweekly, 2->monthly, 3->interval
#define PROGRAM_TYPE_WEEKLY 0
#define PROGRAM_TYPE_BIWEEKLY 1
#define PROGRAM_TYPE_MONTHLY 2
#define PROGRAM_TYPE_INTERVAL 3


		byte type :2;

		// starttime type:
		// 0: repeating (give start time, repeat every, number of repeats)
		// 1: fixed start time (give arbitrary start times up to MAX_NUM_STARTTIMEs)
		byte starttime_type : 1;

		// misc. data
		byte dummy1 : 1;

		// weekly:   days[0][0..6] correspond to Monday till Sunday
		// bi-weekly:days[0][0..6] and [1][0..6] store two weeks
		// monthly:  days[0][0..5] stores the day of the month (32 means last day of month)
		// interval: days[0] stores the interval (0 to 255), days[1] stores the starting day remainder (0 to 254)
		byte days[2];

		// When the program is a fixed start time type:
		//   up to MAX_NUM_STARTTIMES fixed start times
		// When the program is a repeating type:
		//   starttimes[0]: start time
		//   starttimes[1]: repeat count
		//   starttimes[2]: repeat every
		// Start time structure:
		//   bit 15         : not used, reserved
		//   if bit 14 == 1 : sunrise time +/- offset (by lowest 12 bits)
		//   or bit 13 == 1 : sunset  time +/- offset (by lowest 12 bits)
		//      bit 12      : sign, 0 is positive, 1 is negative
		//      bit 11      : not used, reserved
		//   else: standard start time (value between 0 to 1440, by bits 0 to 10)
		int16_t starttimes[MAX_NUM_STARTTIMES];

		uint durations[MAX_NUM_STATIONS];  // duration / water time of each station

		char name[PROGRAM_NAME_SIZE];

		byte check_match(time_t t);
		int16_t starttime_decode(int16_t t);
		byte flags;
	protected:
		bool Check_day_match(DateTime t) {

			byte weekday_t = t.dayOfTheWeek();        // weekday ranges from [0,6] within Sunday being 1
			byte day_t = t.day();
			byte month_t = t.month();


			byte wd = (weekday_t + 5) % 7;
			byte dt = day_t;

			// check day match
			switch (type) {
			case PROGRAM_TYPE_WEEKLY:
				// weekday match
				if (!(days[0] & (1 << wd)))
					return false;
				break;

			case PROGRAM_TYPE_BIWEEKLY:
				// todo
				break;

			case PROGRAM_TYPE_MONTHLY:
				if (dt != (days[0] & 0b11111))
					return false;
				break;

			case PROGRAM_TYPE_INTERVAL:
				// this is an inverval program
				//SPL_D(t.unixtime());
				if (byte((t.unixtime() / SECS_PER_DAY) % days[1]) != days[0])  return false;
				break;
			}

			// check odd/even day restriction
			if (!oddeven) {}
			else if (oddeven == 2) {
				// even day restriction
				if ((dt % 2) != 0)  return false;
			}
			else if (oddeven == 1) {
				// odd day restriction
				// skip 31st and Feb 29
				if (dt == 31)  return false;
				else if (dt == 29 && month_t == 2)  return false;
				else if ((dt % 2) != 1)  return false;
			}
			return true;
		}

	};
#define SECS_PER_DAY 86400L
	// convert absolute remainder (reference time 1970 01-01) to relative remainder (reference time today)
	// absolute remainder is stored in nvm, relative remainder is presented to web
	void drem_to_relative(byte days[2]) {
		byte rem_abs = days[0];
		byte inv = days[1];
		// todo: use now_tz()?
		DateTime ora = adesso();
		SPL_D(ora.unixtime() / SECS_PER_DAY);
		days[0] = (byte)((rem_abs + inv - (ora.unixtime() / SECS_PER_DAY) % inv) % inv);
	}

	// relative remainder -> absolute remainder
	void drem_to_absolute(byte days[2]) {
		byte rem_rel = days[0];
		byte inv = days[1];
		// todo: use now_tz()?
		DateTime t = adesso();
		SPL_D(t.unixtime()); 
		byte res = (((t.unixtime() / SECS_PER_DAY) + rem_rel) % inv);
		days[0] = res;
		SPS_D(rem_rel); SPS_D(inv); SPS_D((t.unixtime() / SECS_PER_DAY)); SPS_D(res);
	}
#define PD_EEPROM_POS 3200
#define PD_SIZE 148 //sizeof(ProgramData)
//#define PDN_SIZE sizeof(ProgramDataN)

/*	class ProgramData {  //dimension 25 byte  *6->  EEPROM position 3600->3800
	public:
		byte nprogs;
		byte nboards ;
		byte mnp ;
		byte mnst;
		byte stations_delay_time;
		
		int  progChangTime, connTime;
		unsigned long  progChang, valveUsed;
		byte valveStatus[8];
		byte Status;
	};*/
	class ProgramData {  //dimension 148 byte  *4->  EEPROM position 3200->3800
	public:
		byte nprogs;
		byte nboards;
		byte mnp;
		byte mnst;
		byte stations_delay_time;

		int  progChangTime, connTime;
		unsigned long  progChang, valveUsed;
		byte valveStatus[8];
		byte Status;
		byte area[8];
		byte dummy[8];
		 char   names[8][15];
		 byte Kc[8];                   //  Crop coefficient

	};
	struct CurrentData {
		time_t rain_delay[4];
		float cumulRain = 0;
		int mm[30];
		byte dummy[30];
	};

	ProgramData pd[N_OS_STA];
	CurrentData cD;
	//ProgramDataN pdn[4];
	ulong last_minute = 0;
	ulong sunrise_time, sunset_time;
	byte sequn; //total n. of elements in the sequence structure
	int EEindex[8][5];
	Status status;
	sequence seq[MAXSEQ];

#define  STARTTIME_SIGN_BIT 12
#define  STARTTIME_SUNRISE_BIT 14
#define	STARTTIME_SUNSET_BIT 13
	/** Decode a sunrise/sunset start time to actual start time */
	int16_t starttime_decode(int16_t t) {
		if ((t >> 15) & 1) return -1;
		int16_t offset = t & 0x7ff;
		if ((t >> STARTTIME_SIGN_BIT) & 1) offset = -offset;
		if ((t >> STARTTIME_SUNRISE_BIT) & 1) { // sunrise time
			t = sunrise_time + offset;
			if (t<0) t = 0; // clamp it to 0 if less than 0
		}
		else if ((t >> STARTTIME_SUNSET_BIT) & 1) {
			t = sunset_time + offset;
			if (t >= 1440) t = 1439; // clamp it to 1440 if larger than 1440
		}
		return t;
	}
	void Json_Extract_jn(byte ic)
	{
		StaticJsonBuffer<1000> jsonBuffer;

		JsonObject& root = jsonBuffer.parseObject(json);
		// Test if parsing succeeds.
		if (!root.success()) {
			SPL("Sta names parseObject() failed");
			return;
		}
		else {

			SPL_D("Parsing...");

			for (int i = 0; i < MAX_NUM_STATIONS; i++)
			{
				const char * nome = root["snames"][i];
				//for(byte j=0;j<strlen(nome);j++)
				//String cs(nome);

				for (byte j=0;j<strlen(nome);j++)pd[ic].names[i][j] = nome[j];
				byte len = strlen(nome); if (len > 14)len = 14;
				pd[ic].names[i][len ] = 0;
				SPS_D(pd[ic].names[i]);

			}
		}
		
		eeprom_write_block(&pd[ic], (void*)(PD_EEPROM_POS + ic*PD_SIZE), PD_SIZE);					//----------------ADDR_EE_OPTIONS --- 3800--------4080 280 IC max 6
		SPL("Pd saved");
	}
	
	void Json_Extract_jo(byte ic) 
	{
		StaticJsonBuffer<1000> jsonBuffer;

		char op_json_names[][6] =
		{ "fwv\0\0",
			"tz\0\0\0",
			"ntp\0\0",
			"dhcp\0",
			"ip1\0\0",
			"ip2\0\0",
			"ip3\0\0",
			"ip4\0\0",
			"gw1\0\0",
			"gw2\0\0",
			"gw3\0\0",
			"gw4\0\0",
			"hp0\0\0",
			"hp1\0\0",
			"hwv\0\0",
			"ext\0\0",
			"seq\0\0",
			"sdt\0\0",
			"mas\0\0",
			"mton\0",
			"mtof\0",
			"urs\0\0",
			"rso\0\0",
			"wl\0\0\0",
			"den\0\0",
			"ipas\0",
			"devid",
			"con\0\0",
			"lit\0\0",
			"dim\0\0",
			"bst\0\0",
			"uwt\0\0",
			"ntp1\0",
			"ntp2\0",
			"ntp3\0",
			"ntp4\0",
			"lg\0\0\0",
			"mas2\0",
			"mton2",
			"mtof2",
			"fwm\0\0",
			"fpr0\0",
			"fpr1\0",
			"re\0\0\0",
			"reset" };
		JsonObject& root = jsonBuffer.parseObject(json);
		byte options[NUM_OPTIONS];
		// Test if parsing succeeds.
		if (!root.success()) {
			SPL("Options parseObject() failed");
			return;
		}
		else {

			SPL_D("Parsing...");
			for (int i = 0; i < NUM_OPTIONS; i++)
			{
				options[i] = root[op_json_names[i]];
				SP_D(op_json_names[ i ]); SPL_D(options[i]);
			}
			pd[ic].stations_delay_time = options[OPTION_STATION_DELAY_TIME];
			SP_D("stat.delay "); SPL_D(options[OPTION_STATION_DELAY_TIME]);
		}
	  eeprom_write_block(&options, (void*)(ADDR_EE_OPTIONS + NUM_OPTIONS*ic), NUM_OPTIONS);					//----------------ADDR_EE_OPTIONS --- 3800--------4080 280 IC max 6
	  SPL("Otpions saved");
	  
	  eeprom_write_block(&pd[ic], (void*)(PD_EEPROM_POS+ic*PD_SIZE),PD_SIZE);					//----------------ADDR_EE_OPTIONS --- 3800--------4080 280 IC max 6
	  SPL("Pd saved "); 
	}
	byte time_diff[4] = { 0,0,0,0 };
	void Json_Extract_jc(byte ic) {
		StaticJsonBuffer<1000> jsonBuffer;

		JsonObject& root = jsonBuffer.parseObject(json);
		
		// Test if parsing succeeds.
		if (!root.success()) {
			SPL("Jc parseObject() failed");
			return;
		}
		else {

			SPL_D("Jc Parsing...");
			    status.curr_time = root["devt"];
				SP("td_[s]"); SPL(now() - status.curr_time);
			    time_diff[ic] = now() - status.curr_time;
				status.nboards= root["nbrd"];            
				SP_D("nboards"); SPLF_D(status.nboards, DEC);
				status.enabled= root["en"];
				status.rain_delayed= root["rd"];          
				status.rain_sensed= root["rs"];          
				status.rd_stop_time= root["rdst"]; 
				cD.rain_delay[ic] = status.rd_stop_time;
				//location= root["loc"];
				//ADDR_NVM_WEATHER_KEY= root["wtkey"];
				sunrise_time= root["sunrise"];
				sunset_time= root["sunset"];
				//os.nvdata.external_ip= root["eip"];
				status.checkwt_lasttime= root["lwc"];
				status.checkwt_success_lasttime= root["lswc"];
				status.lastrun.pid = root["lrun"][0];
				status.lastrun.duration= root["lrun"][1];
				status.lastrun.endtime= root["lrun"][2];
				SP_D("lastrun.endtime"); SPLF_D(status.lastrun.endtime, DEC);
				for (int bid = 0; bid < status.nboards; bid++)
					status.station_bits[bid] = root["sbits"][bid];
				for (int sid = 0; sid<8*status.nboards; sid++)
				{
				   status.station[sid].pid= root["ps"][sid][0];
					status.station[sid].startime = root["ps"][sid][1];
					status.station[sid].remain = root["ps"][sid][2];
				}
				eeprom_write_block(&cD, (void *)CURRENT_DATA_POS, sizeof(CurrentData));
			}
		
		}

	bool Encode_jp(byte pid, byte icontr) 
	{

		StaticJsonBuffer<1300> jsonBuffer;
		JsonArray& jarray = jsonBuffer.createArray();
		eeprom_read_block(EEindex, (void*)EE_INDEX_POS, 80);
		ProgramStruct prog;
		eeprom_read_block(&prog, (void *)(EEindex[pid][icontr]), PROGRAMSTRUCT_SIZE);
		byte flags = 0;
		flags  += prog.enabled&1 ;
		flags += prog.use_weather&1 << 1;
		flags += prog.oddeven&11 << 2;
		flags += prog.type&11 << 4;
		flags += prog.starttime_type&1 << 6;
		jarray.add(flags);
		if (prog.type == 3) drem_to_relative(prog.days);
		jarray.add(prog.days[0]);
		jarray.add(prog.days[1]);
		JsonArray&  nestedArray = jarray.createNestedArray();
		for (int j = 0; j < pd[icontr].mnst; j++)
		{
			nestedArray.add(prog.starttimes[j]);
		}
		JsonArray&  nested1Array = jarray.createNestedArray();
		for (int j = 0; j < pd[icontr].nboards * 8; j++)
		{
			nested1Array.add(prog.durations[j]);
		}
		jarray.add(prog.name);
		jarray.printTo(SERIAL);
	}
	
	/*====================================================================================jp==============================================================
	/jp?pw=xxx
	Return Variables:
	 nprogs: Number of existing programs.
	 nboards: Number of boards (including main controller).
	 mnp: Maximum number of programs allowed.
	 mnst: Maximum number of program start times (fixed time type) allowed.
	 pnsize: Maximum number of characters allowed in program name.
	 pd: Array of program data. Each element corresponds to a program. See below for data structure.

	Program Data Structure: [flag, days0, days1, [start0, start1, start2, start3], [dur0, dur1, dur2…], name]

	 flag: a bit field storing program flags
	o bit 0: program enable (1: enabled; 0: disabled)
	o bit 1: use weather adjustment (1: yes; 0: no)
	o bit 2-3: odd/even restriction (0: none; 1: odd-day restriction; 2: even-day restriction; 3: undefined)
	o bit 4-5: program schedule type (0: weekday; 1: undefined; 2: undefined; 3: interval day)
	o bit 6: start time type (0: repeating type; 1: fixed time type)
	o bit 7: undefined

	 days0/days1:
	o If (flag.bits[4..5]==0), this is a weekday schedule:
	 days0.bits[0..6] store the binary selection bit from Monday to Sunday; days1 is unused.
	For example, days0=127 means the program runs every day of the week; days0=21 (0b0010101) means the program runs on Monday, Wednesday, Friday every week.
	o If (flag.bits[4..5]==3), this is an interval day schedule:
	 days1 stores the interval day, days0 stores the remainder (i.e. starting in day).
	For example, days1=3 and days0=0 means the program runs every 3 days, starting from today.
	 start0/start1/start2/start3:
	o Start times support using sunrise or sunset with a maximum offset value of +/- 4 hours in minute granularity:
	 If bits 13 and 14 are both cleared (i.e. 0), this defines the start time in terms of minutes since midnight.
	 If bit 13 is 1, this defines sunset time as start time. Similarly, if bit 14 is 1, this defines sunrise time.
	If either bit 13 or 14 is 1, the remaining 12 bits then define the offset. Specifically, bit 12 is the sign (if true, it is negative);
	the absolute value of the offset is the remaining 11 bits (i.e. start_time&0x7FF).
	o If (flag.bit6==1), this is a fixed start time type:
	 start0, start1, start2, start3 store up to 4 fixed start times (minutes from midnight). Acceptable range is -1 to 1440. If set to -1, the specific start time is disabled.
	o If (flag.bit6==0), this is a repeating start time type:
	 start0 stores the first start time (minutes from midnight), start1 stores the repeat count, start2 stores the interval time (in minutes); start3 is unused.
	For example, [480,5,120,0] means: start at 8:00 AM, repeat every 2 hours (120 minutes) for 5 times.

	 dur0, dur1…:
	The water time (in seconds) of each station. 0 means the station will not run. The number of elements here must match the number of stations.
	The duration value is compressed using the following algorithm:
	0 – 59 seconds, in steps of seconds
	1 to 179 minutes, in steps of minutes
	3 to 16 hours, in steps of hours
	65534 represents sunrise to sunset duration
	65535 represents sunset to sunrise duration  name: Program name

	Example Return:
	{"nprogs":7, "nboards":1, "mnp":14, "mnst":4, "pnsize":12, "pd":[[3,127,0,[480,2,240,0],[0,2700,0,2700,0,0,0,0],"Summer"], [2,9,0,[120,0,300,0],[0,3720,0,0,0,0,0,0],"Fall Prog"], [67,16,0,[1150,-1,-1,-1],[0,0,0,0,0,0,64800,0],"Pipe"]]}
	Change Program Data [Keyword /cp] 7
	*/
	bool Json_Extract_jp(byte ic, byte pmod[]) {
		StaticJsonBuffer<1600> jsonBuffer;
		ProgramStruct prog;

		int EEfill = eeprom_read_int(EE_PROG_POS);

		eeprom_read_block((void*)&EEindex, (void *)EE_INDEX_POS, 80);
		SP_D("EE_index_read "); SPLF_D(EEindex[0][ic], DEC);
		// Test if parsing succeeds.
		JsonObject& root = jsonBuffer.parseObject(json);
		delay(100);
		if (!root.success()) {
			SPL("Jp parseObject() failed");
			return false;
		}
		else {
			SPL_D("JP Parsing...");
			pd[ic].nprogs = root["nprogs"];
			pd[ic].nboards = root["nboards"];
			pd[ic].mnp = root["npt"];
			pd[ic].mnst = root["mnst"];
			SPLF_D(pd[ic].nprogs, DEC);
			for (int i0 = 0; i0 < pd[ic].nprogs; i0++) {
				pmod[i0] = 0;

				// iterate program struct.
				prog.flags = PD[i0][0];
				SP_D("flags"); SPL_D(prog.flags);
				prog.enabled = prog.flags & 0x01;
				prog.use_weather = (prog.flags & 0B010) >> 1;
				prog.oddeven = (prog.flags & 0B01100 )>> 2;
				prog.type = (prog.flags &           0B0110000 )>> 4;
				prog.starttime_type =( prog.flags & 0B1000000) >> 6;
				SP_D("type"); SPL_D(prog.type);
				ProgramStruct progold;
				if (EEindex[i0][ic] != 0)
					eeprom_read_block((void *)&progold, (void *)(EEindex[i0][ic]), PROGRAMSTRUCT_SIZE);


				prog.days[0] = PD[i0][1];
				prog.days[1] = PD[i0][2];
				if (prog.type == 3) {
					SP_D("Abs.days"); SP_D(prog.days[0]);
					drem_to_absolute(prog.days);
					SP_D(" to "); SPL_D(prog.days[0]);
				}
				for (byte j = 0; j < 2; j++)
					if (EEindex[i0][ic] != 0)
						if (prog.days[j] != progold.days[j]) {
							pmod[i0] = 2;
							SP("day["); SP(j); SP("] "); SP("from:"); SP(progold.days[j]); 
							SP(" to"); SPL(prog.days[j]);
						}
				SP_D("day1."); SPL_D(prog.days[1]);
				for (int j = 0; j < pd[ic].mnst; j++)
				{
					prog.starttimes[j] = PD[i0][3][j];
					SP_D("Start t."); SPL_D(prog.starttimes[j]);
					if (EEindex[i0][ic] != 0)
						if (prog.starttimes[j] != progold.starttimes[j]) {
							pmod[i0] = 2;
							SP("startime["); SP(j); SP("] "); SP("from:");
							SP(prog.starttimes[j]); SP(" to"); SPL(progold.starttimes[j]);
						}
				}
				for (int j = 0; j < pd[ic].nboards * 8; j++)
				{
					prog.durations[j] = PD[i0][4][j];
					SP_D("dur."); SPL_D(prog.durations[j]);
					if (EEindex[i0][ic] != 0)
						if (prog.durations[j] != progold.durations[j]) {
							pmod[i0] = 2;
							SP("duration["); SP(j); SP("] "); SP("from:");
							SP(prog.durations[j]); SP(" to"); SPL(progold.durations[j]);
						}
					if (prog.durations[j] > 0) {
						char buf[10];
						bool notF = true;
						for (byte icc = 0; icc < sprintf(buf, "%d", pd[ic].valveUsed); icc++)
							if (buf[icc] ==(char)( '0'+j)) { notF = false; break; }
						if (notF) { pd[ic].valveUsed  = pd[ic].valveUsed*10+ j;}
					}
				}

				const char* nam = PD[i0][5];
				for (byte i = 0; i < 12; i++)
					prog.name[i] = nam[i];
				
				
				if (pmod[i0] == 0)SPL_D(prog.name);
					if (EEindex[i0][ic] == 0) //---------------new program
					{
						pmod[i0] = 1;
						SPL("added prog.block");
						EEindex[i0][ic] = EEfill;
						eeprom_write_block((void *)&prog, (void *)(EEindex[i0][ic]), PROGRAMSTRUCT_SIZE);

						eeprom_write_block(&pd[ic], (void*)(PD_EEPROM_POS + ic*PD_SIZE), PD_SIZE);					//----------------ADDR_EE_OPTIONS --- 3800--------4080 280 IC max 6
						SPL("Pd saved");
						EEfill += PROGRAMSTRUCT_SIZE + 1;
						SP_D("Prog.block size b. "); SP_D(PROGRAMSTRUCT_SIZE); SP_D(" remaining"); SPL_D(ADDR_EE_OPTIONS - EEfill);
					}
				else      //---program present
				/*{
					ProgramStruct progold;
					eeprom_read_block( (void *)&progold, (void *)(EEindex[i0][ic]), PROGRAMSTRUCT_SIZE);

					if (!prog_comp((void *)&prog,(void *)&progold,PROGRAMSTRUCT_SIZE-10))  //------------program are modified
					{
						pmod[i0] = 2;
						SPL("Modified prog");
						eeprom_write_block((void *)&prog, (void *)(EEindex[i0][ic]), PROGRAMSTRUCT_SIZE);
					}

				}*/
				{
					SPL("prog.block overwritten");
					eeprom_write_block((void *)&prog, (void *)(EEindex[i0][ic]), PROGRAMSTRUCT_SIZE);
				}
				eeprom_write_int(EE_PROG_POS, EEfill);
				SP_D("EE_index_write "); SPLF_D(EEindex[pd[ic].nprogs - 1][ic], DEC);
				eeprom_write_block((void *)&EEindex, (void *)EE_INDEX_POS, 80);
				SP_D("BLOCK WRITTEN "); SPL_D(EEfill);

			}
		}
	}
	/*
	bool print_prog(byte ic);
	{	ProgramStruct prog;

	int EEfill = eeprom_read_int(EE_PROG_POS);

	eeprom_read_block((void*)&EEindex, (void *)EE_INDEX_POS, 80);
	SP_D("EE_index_read "); SPLF_D(EEindex[0][ic], DEC);
	ProgramStruct prog;

 		SPLF_D(pd[ic].nprogs, DEC);
		for (int i0 = 0; i0 < pd[ic].nprogs; i0++) {
			eeprom_read_block((void *)&prog, (void *)(EEindex[i0][ic]), PROGRAMSTRUCT_SIZE);

			SP_D("flags"); SPL_D(prog.flags);
			prog.enabled = prog.flags & 0x01;
			prog.use_weather = (prog.flags & 0B010) >> 1;
			prog.oddeven = (prog.flags & 0B01100) >> 2;
			prog.type = (prog.flags & 0B0110000) >> 4;
			prog.starttime_type = (prog.flags & 0B1000000) >> 6;
			SP_D("type"); SPL_D(prog.type);
					
		SP_D("day0."); SPL_D(prog.days[0]);
		SP_D("day1."); SPL_D(prog.days[1]);
		for (int j = 0; j < pd[ic].mnst; j++)
			{
				SP_D("Start t."); SPL_D(prog.starttimes[j]);
			}
			for (int j = 0; j < pd[ic].nboards * 8; j++)
			{
				SP_D("dur."); SPL_D(prog.durations[j]);

			SPL_D(prog.name);

*/

	bool prog_comp(void * p1, void * p2,byte size) { //----------------------Compare two structures----------------------
		for (byte i = 1; i < size; i++)
			if (*((byte *)p1 + i) != *((byte *)p2 + i)) { SP_D('%'); SPLF_D(i, DEC); return false; }  //program changed
		return true;                                                    //program no change
	}

	void eeprom_write_int(int pointer, int value) {
		eeprom_write_byte((byte *)pointer, value >>8);
		eeprom_write_byte((byte *)(pointer+1), value & 0xFF);

	}
	int eeprom_read_int(int pointer) {
	int res=	eeprom_read_byte((byte *)pointer)<<8;
    res=res+	eeprom_read_byte((byte *)(pointer + 1));
	return res;
	}
/*
#include <SPI.h>

#include <Adafruit_ILI9341esp.h>
#include <Adafruit_GFX.h>
#include <XPT2046.h>
#include "../../Arduino/OpenSprinkler/OpenSprinkler_Arduino_V_2_1_6/json_parse_program/Glx_SWindows.h"
*/
///////////////////////////////////////Global decl: Curves////////////////////////
#ifdef LCD_TOUCH

	byte	N_curves = 0;// total number of graphics created  
	byte  CurvesN[N_Maxcurves];//each curves defined by number CurvesN
	int   CurveColor[N_Maxcurves] = {//color of the curves
			ILI9341_DARKCYAN  ,
			ILI9341_BLUE,
			ILI9341_RED,
			ILI9341_GREEN,
			ILI9341_MAGENTA,
			ILI9341_ORANGE,
			ILI9341_YELLOW,
			ILI9341_MAROON,
			ILI9341_LIGHTGREY,
			ILI9341_PURPLE,
			ILI9341_CYAN,
			ILI9341_GREENYELLOW,
			ILI9341_PINK,
			ILI9341_OLIVE,
			ILI9341_DARKGREEN
	};
	bool loadGraph(byte ig,float x, float y)
	{                  // -------------------ig=valve & station number, x,y graph point coord.------
		byte igraph = 255;
		for (byte i = 0; i < N_curves; i++)
			if (ig == CurvesN[i])igraph = i;
		if (igraph == 255) {  // ------------------curve not found ...... new curve
			if (N_curves < N_Maxcurves) {
				igraph = N_curves;
				CurvesN[N_curves++] = ig;
			}
			else
				return false;   //-----------------curves exceed max.
		}          //-------------------------------load point data--------------------
		int ipoint = myGraph[igraph].nval;
#ifdef STL_VECTOR
		if (myGraph[igraph].x.size() <= ipoint)
		{
			myGraph[igraph].x.resize(ipoint + 4);
			myGraph[igraph].y.resize(ipoint + 4);
		}
#endif
		myGraph[igraph].x[ipoint] = x;
		myGraph[igraph].y[ipoint] = y;
		myGraph[igraph].nval++;
		
		
		
	}
#define HOR_AX 0.2
	void graphInit(){
		SP_D("mem.h."); SPL_D(ESP.getFreeHeap());
		if (plot_sequence()) {
			Gwin.init(0, 0, 21, 280, 150, ILI9341_WHITE);


			for (byte i = 0; i < N_curves; i++)
			{
				Serial.print("Curve "); Serial.print(CurvesN[i]);
				Serial.print(" n.points "); Serial.println(myGraph[i].nval);
				myGraph[i].init(CurveColor[i]);
			}
			for (byte i = 0; i < N_curves; i++)
				myGraph[i].draw();
			//		myGraph[i].drawAxX(0, 0.1);  // X axis with proposed min step to be adjusted by code

			myGraph[0].drawAxX(HOR_AX, 1., 0);
			SP_D("mem.h."); SPL_D(ESP.getFreeHeap());
		}
	}
	
	
	
	void setupLcdTouch() {
		SPI.setFrequency(ESP_SPI_FREQ);

		tft.begin();
		touch.begin(tft.width(), tft.height());  // Must be done before setting rotation
		Serial.print("tftx ="); Serial.print(tft.width()); Serial.print(" tfty ="); Serial.println(tft.height());
		tft.fillScreen(ILI9341_BLACK);
		// Replace these for your screen module calibrations
		touch.setCalibration(176, 256, 1739, 1791);//209, 1759, 1775, 273);
												   //tft.setRotation(1); //----land scape mode (text scrolling not available)
												   //-----------------------------init menus--------------------------------------
		MWin.init(0, 0, tft.width(), 20);
#define FILL_M(i,j,s,y) MWin.menu[i].menuName[j]=s;MWin.menu[i].menuIndex[j]=y;MWin.menu[i].nbutton++;  
		FILL_M(0, 0, "graf", 2)
			FILL_M(0, 1, "keyb.", 0)
			FILL_M(0, 2, "command", 3)
			FILL_M(2, 0, "json-jo", 0)
			FILL_M(2, 1, "json-jc", 0)
			FILL_M(2, 2, "json-jp", 0)
			FILL_M(2, 3, "ret", 1)
			FILL_M(1, 0, "<-", 0)
			FILL_M(1, 1, "->", 0)
			FILL_M(1, 2, "X+", 0)
			FILL_M(1, 3, "X-", 0)
			FILL_M(1, 4, "ret", 1)
			MWin.menu[0].init();
		MWin.menu[0].draw();
		//  Serial.println(MWin.menu[1].menuName[3]);
		//  Serial.println(MWin.menu[1].nbutton);
#ifdef GRAPH
		//graphInit();
	
#endif
		//  --------------------------------init KEYBOARD-------------------------------
		TWin.init(0, 240, tft.width(), tft.height(), 10, 0);
		TFWin.init(0, 180, tft.width(), 239, 10, 1);
	}
	unsigned int plot_start_time;
#define NPIXEL_TOUCH 6
	char touch_control(char cc)
	{
		char c=cc;
		static boolean keyb_on;
		uint16_t x, y;
		if (touch.isTouching())
		{
			touch.getPosition(x, y);
			int x_true = Gwin.xpressed(x, y);
			int y_true = Gwin.ypressed(x, y);
			if (x_true != -32000) {
		//			SP("x="); SP(x_true);		SP("y="); SPL(y_true);
					for (byte i = 0; i < N_curves; i++)
						for (byte j = 0; j < myGraph[i].nval - 1; j++)
							if (myGraph[i].x[j] < x_true&&x_true < myGraph[i].x[j + 1]) {
								int Yinterp = (myGraph[i].y[j + 1] - myGraph[i].y[j]) / float(myGraph[i].x[j + 1] - myGraph[i].x[j])*(x_true - myGraph[i].x[j]) + myGraph[i].y[j];
								if (abs(Yinterp - y_true) < 100) {
									time_t Xtime = (plot_start_time + x_true) * 60;
									SP(month(Xtime)); SP("/"); SP(day(Xtime)); SP(" "); SP(hour(Xtime)); SP(":"); SP(minute(Xtime)); SP("  ");
									SP(Yinterp); SP(" "); SP(CurvesN[i]); SPL(pd[CurvesN[i] / 10].names[CurvesN[i] % 10]); delay(1000); 
								}
							}
				}
			
			


			int ind = MWin.getPressed(x, y);
			if (ind > 0) {
			//	TWin.println(ind);//select menu function case of
				switch (ind) {

				case 2:
					if (!keyb_on) {
						MyKeyb.init(10, 50, 0); keyb_on = true;
						break;
					}
					else {
						MyKeyb.end(); keyb_on = false;
						break;
					}
				case 11: {
					for (byte i = 0; i < N_curves; i++)
						myGraph[i].scroll(10.);
					for (byte i = 0; i < N_curves; i++) 
						myGraph[i].draw();
						myGraph[0].drawAxX(HOR_AX, 1.,0);

					break;
				}
						 //<
				case 12: {                 //>
					for (byte i = 0; i < N_curves; i++)
						myGraph[i].scroll(-10.);
					for (byte i = 0; i < N_curves; i++)
					
						myGraph[i].draw();
						myGraph[0].drawAxX(HOR_AX, 1.,0);
					
					break; }
				case 13: {


					//+
					for (byte i = 0; i < N_curves; i++)
						myGraph[i].changeScaX(1.2);
					for (byte i = 0; i < N_curves; i++)
						myGraph[i].draw();
					myGraph[0].drawAxX(HOR_AX, 1., 0);

					break; }

				case 14: {
					for (byte i = 0; i < N_curves; i++)
						myGraph[i].changeScaX(0.8);
					for (byte i = 0; i < N_curves; i++)
						myGraph[i].draw();
					myGraph[0].drawAxX(HOR_AX, 1., 0);

					break; }//-

		////////////////////////////////////////////////////////////////
				case 21: {c = 'o'; TWin.textColor(ILI9341_WHITE); break; }
				case 22: {c = 'c'; TWin.textColor(ILI9341_WHITE); break; }
				case 23: {c = 'p'; TWin.textColor(ILI9341_WHITE); break; }
		 /////////////////////////////////////////////////////////////////

				
				}
			}
			else
			 {
				 c = MyKeyb.isPressed(x, y);
				if (c > 0)TWin.write(c);
			}
		}
		return c;
		
	}
#endif
//#define INIT_EEPROM /////////////////////////////only for reset run
#define INIT_TIME
#define PAGE_TELNET 1000
void	logView(char c) {
		if(c=='<'){
			logfile.close();
			logfile = SPIFFS.open("/logs.txt", "r+");
			Tclient.print("--------------------------");
			logfile.seek(0, SeekEnd);
			unsigned long filePos = logfile.position();
			int offset = filePos;
			if (filePos > PAGE_TELNET)
				 offset = PAGE_TELNET;
			delay(1000);
			while (Tclient.available()) {
				char cc = Tclient.read(); 
				if (cc > '0'&&cc <= '9')offset = offset + (cc - '0')*PAGE_TELNET;
			}
			Tclient.println(offset);
				
			
			logfile.seek (-offset,SeekEnd);
			SP_D(logfile.position()); SP_D(" "); SPL_D(filePos);
			logfile.read();
			while (logfile.available()&&logfile.position()<filePos) {

				Tclient.println(logfile.readStringUntil('\n'));
			}
			Tclient.println("-------------------------end");
			logfile.seek(filePos, SeekSet);
		}
		logfile.close();
		logfile = SPIFFS.open("/logs.txt", "a+");

		
			
	}
;
//IPAddress jsonserver(192, 168, 1, 10);
#define MIL_JSON_ANS 20000
WiFiClient client;
struct Geo
{
	float lat;//{"latitude"}
	float lon;//{"longitude"}
	int alt;//{"elevation"}

};
//File wfile;
struct Weather
{
	time_t time;        //"local_epoch"			local epoch time
	int temp;			//"temp_c"				temp ° celsius
	int humidity;		//"relative_humidity"	humidity %
	int rain1h;			//"precip_1h_metric"	rain last hour 0.1mm
	int rain;			//"precip_today_metric" rain today 0.1mm
	int wind;			//{"wind_kph"}			wind speed Km/h
						//		int atpres;			//"pressure_mb"			atm. pressure milliBar
	unsigned int sunrad;			//solar radiation factor;
	float penman;		//ETo penmam-monteith
	float water;		//cumulated ETo penman
	void write() {
		File wfile;
		if (SPIFFS.exists("/weather.log"))
			wfile = SPIFFS.open("/weather.log", "r+");
		else
			wfile = SPIFFS.open("/weather.log", "w+");

		if (!wfile) { Serial.println("Cannot open weather.log"); return; }
		wfile.seek(0, SeekEnd);
		wfile.seek(0, SeekEnd);
		wfile.print(time); wfile.print(',');
		wfile.print(temp); wfile.print(',');
		wfile.print(humidity); wfile.print(',');
		wfile.print(rain1h); wfile.print(',');
		wfile.print(wind); wfile.print(',');
		wfile.print(penman); wfile.print(',');
		wfile.print(water); wfile.print(',');
		wfile.print(sunrad); wfile.print(',');
		wfile.println(rain);
		wfile.close();
	}
	bool read(time_t timev) {
		char buf[20];
		byte n;
		time = 0;
		File wfile;
		if (SPIFFS.exists("/weather.log"))
			wfile = SPIFFS.open("/weather.log", "r+");
		else
			wfile = SPIFFS.open("/weather.log", "w+");

		if (!wfile) {
			Serial.println("Cannot open weather.log"); return 0;
		}
		wfile.seek(0, SeekEnd);
		long wfilepos = wfile.position();
		wfile.seek(0, SeekSet);
		while (time < timev&&wfile.available()) {
			if (wfile.find(10)) {
				n = wfile.readBytesUntil(',', buf, 20);
				buf[n] = 0;
				time = atol(buf);
		//		SPS_D(buf);
			}
			else {
				 if (wfile.read()<0  ) { wfile.close(); return 0; }
			}
		
		}
		if (!wfile.available()) { wfile.close(); return false; }
		n = wfile.readBytesUntil(',', buf, 20);
		
		buf[n] = 0; //SPS_D(buf); 
		temp = atoi(buf);
		n = wfile.readBytesUntil(',', buf, 20);
		buf[n] = 0; //SPS_D(buf); 
		humidity = atoi(buf);
		n = wfile.readBytesUntil(',', buf, 20);
		buf[n] = 0; //SPS_D(buf); 
		rain1h = atoi(buf);
		n = wfile.readBytesUntil(',', buf, 20);
		buf[n] = 0; //SPS_D(buf);
		wind = atoi(buf);
		n = wfile.readBytesUntil(',', buf, 20);
		buf[n] = 0;// SPS_D(buf);
		penman = atof(buf);
		n = wfile.readBytesUntil(',', buf, 20);
		buf[n] = 0;// SPS_D(buf);
		water = atof(buf);
		n = wfile.readBytesUntil(',', buf, 20);
		buf[n] = 0;// SPS_D(buf);
		sunrad = atoi(buf);
	
		n = wfile.readBytesUntil(',', buf, 20);
			buf[n] = 0;// SPS_D(buf);
			rain = atoi(buf);
		
		
		wfile.seek(wfilepos, SeekSet);
		wfile.close();
		return true;
	}

};
//////////////////////////////weather static---------------------------
#define NO_ET0
#ifndef NO_ET0
static bool getweather = true;
#include "ET_penmam.h"
#define ORA_WEATHER 0 //weather reading start time
#define FREQ_WEATHER 2// weather reading every ... hours
byte next_ora = ORA_WEATHER;
#define MAX_WEATHER_READ 24
Weather weather[MAX_WEATHER_READ]; byte iw = 0;
Weather w_max, w_min, w_mean;
//"/api/e409b2aeaa5e3ffe/conditions/q/Italy/Loano.json"
Geo sta = { 44.124748,8.25445,23 };
#endif
//#define CLEAR_WEATHER

void setup() {

		EEPROM.begin(4090);
		delay(1000);


		//SETUP RTC ___________________INTERRUPTS&---Starting receive pulses-----------------------------------------------------------------------------------
		Wire.begin();
		RTC.begin();
		SPIFFS.begin();
//		SPIFFS.format();
	
//#define INIT_EEPROM	

#ifndef INIT_EEPROM
		if (eeprom_read_byte(0) == 211)
#endif			//initialise EEprom
		{
			SPIFFS.format();

			for (byte i = 0; i < 40; i++)eeprom_write_byte((byte *)(EE_INDEX_POS + i), 0);
			for (byte i = 0; i < 250; i++)eeprom_write_byte((byte *)(PD_EEPROM_POS + i), 0);
			for (byte i = 0; i < 250; i++)eeprom_write_byte((byte *)(PD_EEPROM_POS +250+ i), 0);
			eeprom_write_int(EE_PROG_POS, EEPROM_PROGSTART);
			eeprom_write_byte(0, 0);
			//eeprom_write_block((void *)&EEindex, (void *)EE_INDEX_POS, 80);
			//EEwrite(0, 0);

		}

		Serial.begin(115200);
		Serial.println("Starting Json Parser/controller...");

//#define DIR


		if(SPIFFS.exists("/logs.txt"))
		 logfile = SPIFFS.open("/logs.txt", "a+");
		else
			logfile = SPIFFS.open("/logs.txt", "w+");

		 if (!logfile)Serial.println("Cannot open logFile");
	//	 while (logfile.available())Serial.print((char)logfile.read());
		logfile.seek(0,SeekEnd);
		posFile = logfile.position();
		if (posFile > MAXBYTES)reset_logfile();
//#define PROVAEEPROM

#ifdef PROVAEEPROM
		provaEEprom();
#endif
		eeprom_read_block(&cD, (void *)CURRENT_DATA_POS, sizeof(CurrentData));
		
		for (byte i = 0; i < 5; i++) { SPS_D(cD.rain_delay[i]); }
		SPL_D("=rain_delay");
#ifdef LCD_TOUCH
		setupLcdTouch();
#endif
		// We start by connecting to a WiFi network
		SP("Flash size "); SP(ESP.getFlashChipSizeByChipId());
		SP(" flash free "); SP(ESP.getFreeSketchSpace());
		SP("logf.");		SPL(posFile);
		SP("Connected to ");
		SPL(ssid);
		SP(WiFi.SSID());
		//	WiFi.mode(WIFI_STA);
		WiFi.begin(ssid, password);
		//	Wifi_station_set_auto_connect(true);
		byte count = 0,limitcount = 100;
		while (WiFi.status() != WL_CONNECTED) {
			delay(500);
			SP(".");
			
			if (count++ > limitcount) { WiFi.begin(ssid1, password1); count = 0; limitcount=+100; SP(ssid1); }
		}
		
		SPL("");
		SP("WiFi connected ");
		SP("IP address: ");
		SPL(WiFi.localIP());
		
		if (WiFi.localIP()[0] != 192)ESP.restart();//restart if network is not set up correct!

		delay(2000);

#ifdef INIT_TIME
		SP("NPT time Sync");
		setSyncInterval(28800);//every 8 h
		if(SyncNPT(1)) SPL("...OK!");
#endif
		DateTime ora = adesso();

#ifdef DIR
		Dir dir = SPIFFS.openDir("/");
		while (dir.next())
		{
			SP_D(">>"); SP_D(dir.fileName()); SP_D(" \t ");
			File f = dir.openFile("r"); SPL_D(f.size());
		}


#endif
	//	SPIFFS.remove("/waterlog.txt");
#ifdef WATERLOG
		if (!SPIFFS.exists("/waterlog.txt")) {
			
			File f = SPIFFS.open("/waterlog.txt", "w+");
			SPL("new waterlog");
			f.print(ora.day()); f.print('-'); f.print(ora.month());
			f.close();
		}
#endif
		// Telnet Client

#ifdef TELNET
		SP(ora.hour()); SP(":"); SP(ora.minute());
		server.begin();
		server.setNoDelay(true);
		SP("Ready! Use 'telnet ");
		SP(WiFi.localIP());
		SPL(" 23' to connect");
#endif
#ifdef GRAPH
		graphInit();
#endif
#ifdef OTA
		ArduinoOTA.onStart([]() {
			
			logfile.close();
			//SPIFFS.end();
			Serial.println("Start_Ota");
		});
		ArduinoOTA.onEnd([]() {
			Serial.println("\nEnd");
			ESP.restart();
		});
	/*	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {//SP(".");
			{
				Serial.print(" Progress:% "); Serial.println((progress / (total / 100)));
			}
		});
		ArduinoOTA.onError([](ota_error_t error) {
			Serial.print("Error : "); Serial.println( error);
			if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
			else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
			else if (error == OTA_CONNECT_ERROR)Serial.println("Connect Failed");
			else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
			else if (error == OTA_END_ERROR) Serial.println("End Failed");
		});*/
		ArduinoOTA.setHostname("OSmanSerra");

		ArduinoOTA.begin();

#endif
		 ora = adesso();
#ifdef CLEAR_WEATHER
		SPIFFS.remove("/weather.log");
#endif
#ifndef NO_ET0
		
		iw = 0;
		byte prevsunrad = 0;
		for (byte i = 0; i < 24 / FREQ_WEATHER; i++) {
			if (weather[iw].read(ora.unixtime() - SECS_PER_DAY + i * FREQ_WEATHER * 3600L)) {
				if ((weather[iw].sunrad > 1400 || weather[iw].sunrad == 0) 
					&& (hour(weather[iw].time) > 9 && hour(weather[iw].time) < 18))weather[iw].sunrad = prevsunrad;
				prevsunrad = weather[iw].sunrad; 
				SPS_D(day(weather[iw].time)); SPS_D(hour(weather[iw].time)); SP_D(":"); SP_D(minute(weather[iw].time));
				SPS_D(weather[iw].temp); SPS_D(weather[iw].humidity); SPS_D(weather[iw].rain1h); SP_D(" r="); SP_D(weather[iw].rain);
				SP_D(" ET="); SP_D(weather[iw].penman); SP_D("s_rad"); SPS_D(weather[iw].sunrad); SP_D(" water="); SPL_D(weather[iw].water);
				if (iw > 0) { if (weather[iw].time != weather[iw - 1].time)iw++; }
				else iw++;
			}
		}
		if(iw>0)iw--;
#endif
	/*	SPT(ora.hour(),ora.minute());
		if (ora.hour() > 24) {
			if(RTC.isrunning())
				SP("RTC problem....restart!");
			else
				SP("RTC not running....restart!");
			delay(120000L);
			ESP.restart();
			
		}*/
		/////////////////////////////////// START INTERRUPTS 
#ifdef FTP
		ftpSrv.begin("esp8266", "esp8266");    //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)
#endif

		startUpEdit();
		startUpFluxMonitor();
		// read Pd structure
		for (byte ic = 0; ic < N_OS_STA; ic++) {
			eeprom_read_block(&pd[ic], (void*)(PD_EEPROM_POS + ic*PD_SIZE), PD_SIZE);					//----------------ADDR_EE_OPTIONS --- 3800--------4080 280 IC max 6
			SP_D(pd[ic].valveUsed); SPS_D(pd[ic].Status);
	/*		pdn[ic].connTime = pd[ic].connTime;
			pdn[ic].mnp = pd[ic].mnp;
			pdn[ic].mnst = pd[ic].mnst;
			pdn[ic].nboards = pd[ic].nboards;
			pdn[ic].nprogs = pd[ic].nprogs;
			pdn[ic].progChang = pd[ic].progChang;
			pdn[ic].progChangTime = pd[ic].progChangTime;
			pdn[ic].valveUsed = pd[ic].valveUsed;
			pdn[ic].stations_delay_time = pd[ic].stations_delay_time;
			pdn[ic].Status = pd[ic].Status;*/
			for (byte j = 0; j < 8; j++) {
				SP_D(" "); SP_D(pd[ic].valveStatus[j]);
	//			pdn[ic].valveStatus[j] = pd[ic].valveStatus[j];
			
			}
			
			SPL_D();
		}
	//	SPL_D(PDN_SIZE);
	//	for (byte ic = 0; ic < N_OS_STA; ic++) 
	//		eeprom_write_block(&pdn[ic], (void*)(3200 + ic*PDN_SIZE), PDN_SIZE);
		
		print_status();
#ifdef IOTno
		static		WiFiClient IOTclient;

		ThingSpeak.begin(IOTclient);
#endif
	}
#include <stdarg.h>
	//--------------------------------------------------------------------------
#ifndef NO_ET0
	float previous_factor = 1.; byte count_trial = 0;
#ifdef APIWU
	byte APIweatherV(String streamId, String nomi[], byte Nval, float val[]) {

		
		SP_D("connecting to WU");
		//char json1[2500];
		const int httpPort = 80;
		if (!client.connect("api.wunderground.com", 80))
		{
			client.stop();

			SP("connection failed ");// SPL(jsonserver);
			return 0;
		}
		SP_D("mem.h."); SPL_D(ESP.getFreeHeap());
		//	SP("Connect: "); SP(jsonserver);
		String url = streamId;
		//url += "?pw=";
		//url += privateKey;

		//url += "&value=";
		//url += value;
	//	client.flush();
		Serial.print(" Requesting URL: ");
		Serial.print(url);
		//	client.print(url);
		// This will send the request to the server
		client.print(String("GET ") + url + " HTTP/1.1\r\n" +
			"Host: " + "api.wunderground.com" + "\r\n" +
			"Connection: close\r\n\r\n");

		int i = 0, json_try = 0; bool ISJSON = false; byte ii = 0;
		Serial.println("Waiting Json");
		long time_step = millis() + MIL_JSON_ANS;
		char c, cp, cpp; bool obs=false;
		
		while (millis() < time_step)
		{
			// Read all the lines of the reply from server and print them to Serialbol 
			while (millis() < time_step - MIL_JSON_ANS / 2&&!obs)
				if (client.available()) {
#ifdef VERIFY_WU_ANSWER
					Serial.print(client.read());
#else
				//	obs = client.findUntil(nomi[0].c_str(), "}}}");
					obs = client.findUntil("current_observation", "}}}");
#endif
	//				if (obs)break;
	//				else { SPL_D("error"); client.stop(); return 0; }
				} else
				{
					Serial.print('.'); delay(100);
				}
			if (!obs) {
				SPL_D("error");
				while (client.available()) Serial.print(client.read());
				client.stop(); return 0;
			}
			while (millis() < time_step&&c != '}'&&cp != '}'&&cpp != '}')

				while (obs&&client.available() && i < MAX_JSON_STRING) {
					cpp = cp;
					cp = c;
					c = client.read();
					Serial.print(c);
					//	buff[ii++] = c;
					//if (cpp == 'r'&&cp=='e'&&cpp=='s'){
					if (c == '{')
						ISJSON = true;

					//	json[i++] = cpp; json[i++] = cp;}
					if (ISJSON) json[i++] = c;
				}
			//Serial.println("endwhile");
			if (ISJSON) {
				json[i - 1] = 0;
				client.stop();

				SP_D("Connected ! "); //SPL(jsonserver);
									  //SP_D(json);
				SPL_D(" Json read!"); SPL_D(i);
				SP_D("m.b.h."); SPL_D(ESP.getFreeHeap());
                DynamicJsonBuffer jsonBuffer;
				JsonObject& root = jsonBuffer.parseObject(json);
				SP_D("m.a.h."); SPL_D(ESP.getFreeHeap());
				// Test if parsing succeeds.
				if (!root.success()) {
					SPL("Weather parseObject() failed");
					return 0;
				}
				else {

					SPL_D("Weather Parsing...");
					for (byte i = 1; i < Nval + 1; i++) {
						if (nomi[i] == "local_epoch")
						{
							time_t time = root["local_epoch"];
							SPL_D(time);
							val[i - 1] = (float)time;
						}
						else
						{
							float valore = root[nomi[i]];
							val[i - 1] = valore;
							//	SP_D(valore);
						}
					}
					return 1;
				}
				//va_end(args);
			}

			else SPL_D("no json");

		}
		client.stop();
		SP_D("mem.h."); SPL_D(ESP.getFreeHeap());
		return 0;
	}
	
	byte APIweather(String streamId) {

		float val[8];
		count_trial++;

		String Str = "/api/48dfa951428393ba/conditions/q/Italy/pws:ISAVONAL1.json";
		String nomi[] = { "current_observation","local_epoch","temp_c","relative_humidity","precip_1h_metric","precip_today_metric","wind" };
		if (!APIweatherV(Str, nomi, 6, val)) return 0;
		SP_D("iw"); SPS_D(iw);
		if (day((time_t)val[0]) < day(adesso()) - 1 || day((time_t)val[0]) > day(adesso()) + 1)return 0;
		count_trial = 0;
		iw++; if (iw > MAX_WEATHER_READ)iw = 0;
		weather[iw].time = (time_t)val[0];//root["local_epoch"];
		weather[iw].temp = val[1]; //root["temp_c"];
		weather[iw].humidity = val[2];//root["humidity"]; 
		float rain1h = val[3]; //root["precip_1h_metric"];
		weather[iw].rain1h = val[4];// rain1h * 10;
		float rain = val[5];// root["precip_today_metric"];
		weather[iw].rain = rain * 10;
		weather[iw].wind = val[6];// root["wind_kph"];
		//------------call local sensors---------------------
		char buff[20]; char * ii; int ival[4]; byte i = 1;
		if (ServerCall(buff, "//", 77) > 1) {
			ival[0]= atoi(strtok(buff, " "));
			ii = strtok(NULL, " ");
			while (ii != NULL) { ival[i++] = atoi(ii); ii = strtok(NULL, " "); }
			SP_D("KWh="); SPL_D(ival[2]);
			weather[iw].water = ival[2];
		}

//-----------------end json decode-------------------------------------------

		Str = "/api/48dfa951428393ba/conditions/q/Italy/Savona.json";
		String nomiS[] = { "current_observation","local_epoch","solarradiation" };

		bool savona_result = !APIweatherV(Str, nomiS, 2, val);
		weather[iw].sunrad = int(val[1]);

		byte prevrain = weather[MAX_WEATHER_READ].rain;
		if (day(MAX_WEATHER_READ) != day(iw))prevrain = 0;
		if (iw > 0) {
			prevrain = weather[iw - 1].rain;
			if (day(iw - 1) != day(iw))prevrain = 0;
		}
		if (weather[iw].rain > prevrain)
		{
			for (byte ic = 0; ic < N_OS_STA; ic++)
				for (byte i = 0; i < 8; i++)pd[ic].dummy[i] += weather[iw].rain - prevrain;
		}


		//	weather[iw].atpres = root["pressure_mb"];
		w_max.time = weather[iw].time;
		w_mean.temp = 0;
		w_max.temp = -10;
		w_min.temp = 50;
		w_mean.humidity = 0;
		w_max.humidity = 0;
		w_min.humidity = 100;
		w_mean.sunrad = 0;
		//			w_mean.atpres = 0;
		//			w_max.atpres = 0;
		//			w_min.atpres = 10000;

		w_max.rain = weather[iw].rain;  // today rain
		w_min.rain = 0;
		byte iel = 0;
		for (byte j = 0; j < 24 / FREQ_WEATHER; j++) {
			int i = iw - j; if (i < 0)i = MAX_WEATHER_READ + 1 - i;
			if (weather[i].humidity > 0 && weather[i].humidity < 100) {
				iel++;
				SPS_D(weather[i].temp); SPS_D(weather[i].humidity); SP_D(" "); SPL_D(weather[i].wind);
				w_min.time = weather[i].time;					//------------starting time for averages
				w_mean.temp += weather[i].temp;
				if (w_max.temp < weather[i].temp)w_max.temp = weather[i].temp;
				if (w_min.temp > weather[i].temp)w_min.temp = weather[i].temp;
				w_mean.humidity += weather[i].humidity;
				if (w_max.humidity < weather[i].humidity)w_max.humidity = weather[i].humidity;
				if (w_min.humidity > weather[i].humidity)w_min.humidity = weather[i].humidity;
				w_mean.wind += weather[i].wind;
				if (w_max.wind < weather[i].wind)w_max.wind = weather[i].wind;
				if (w_min.wind > weather[i].wind)w_min.wind = weather[i].wind;
				//			w_mean.atpres += weather[i].atpres;
				//			if (w_max.atpres < weather[i].atpres)w_max.atpres = weather[i].atpres;
				//			if (w_min.atpres > weather[i].atpres)w_max.atpres = weather[i].atpres;
#define SUN_START 300
				if (day(weather[i].time) != day(w_max.time) && w_min.rain == 0)
					w_min.rain = weather[i].rain;					//yesterday--------rain------

				//sunrad in kJoule/day

		//		if(weather[iwo].time+30000>(uint32_t)val[0])
		//		w_mean.sunrad += (weather[iwo].sunrad + weather[iw].sunrad) / 2 * ((weather[iw].time - weather[iwo].time)/1000);
		//		else
		//		w_mean.sunrad += ( val[1]) / 2 *
		//			(((hour((time_t)val[0]))*60+ minute((time_t)val[0])-SUN_START)*6 / 100);

			}
		}
		if (iel == 0)return 0;
		w_mean.temp /= iel;
		w_mean.wind /= iel;
		//		w_mean.atpres *= FREQ_WEATHER;
		//		w_mean.atpres /= 24;
		w_mean.humidity /= iel;
		//compute expented sunrad
		int iwo;
		int doy = (month(w_max.time) - 1) * 30 + day(w_max.time);
#define MID_DAY (sunset_time!=0?(sunset_time-sunrise_time)/120:13.5) //mid day sun hour
		float time_angle = (hour(weather[iw].time) + 2. + minute(weather[iw].time) / 60. - MID_DAY + sta.lon / 360.);
		float visibility = 20;       //               _______________best visibility km
		float sun_elevation = asin(sin(radians(sta.lat))*sin(sol_dec(doy)) + cos(radians(sta.lat))*cos(sol_dec(doy))*cos(time_angle*3.1415 / 12));
		float expected_sunrad = 0;
		if (sun_elevation > 0)
			expected_sunrad = 1352.*exp(-(39 / visibility + 0.85)*atmos_pres(sta.alt) / atmos_pres(0) / (0.9 + 9.4 * sin(sun_elevation)));
		SPS_D(sol_dec(doy)); SPS_D(MID_DAY); SPS_D(time_angle); SPS_D(sun_elevation); SPS_D("es"); SPL_D(expected_sunrad);
		static int nfactor;
		if (expected_sunrad > 10) 
			if (nfactor == 0) 
				if (time_angle > 0)  // if is a restart and afternoon recompute previous_factor from records 
					for (byte ii = 1; ii < 6; ii++) {
						iwo = iw - ii; if (iwo < 0)iwo = MAX_WEATHER_READ - iwo;
						time_angle = (hour(weather[iwo].time) + 2. + minute(weather[iwo].time) / 60. - MID_DAY + sta.lon / 360.);
						sun_elevation = asin(sin(radians(sta.lat))*sin(sol_dec(doy)) + cos(radians(sta.lat))*cos(sol_dec(doy))*cos(time_angle*3.1415 / 12));
						float expecte_sunrad = 0;
						if (sun_elevation > 0)
							expecte_sunrad = 1352.*exp(-(39 / visibility + 0.85)*atmos_pres(sta.alt) / atmos_pres(0) / (0.9 + 9.4 * sin(sun_elevation)));

						previous_factor = (previous_factor*nfactor + weather[iwo].sunrad) / (nfactor + expecte_sunrad);
						nfactor = nfactor + expecte_sunrad;
					}

		if (savona_result ||
			(weather[iw].sunrad > expected_sunrad || weather[iw].sunrad < expected_sunrad*0.05)) //error reading sunrad assume =previous reading
		{
			SPS_D("apply cal. sunrad");
			if (previous_factor < 1.) {
			weather[iw].sunrad = expected_sunrad*previous_factor;
			}
			else weather[iw].sunrad = expected_sunrad;
		}
		else			//----------------------compute previous_factor as average of sunrad/expected_sunrad
			if (expected_sunrad > 10) {
				 previous_factor = (previous_factor*nfactor + weather[iw].sunrad) /( nfactor+expected_sunrad);
				nfactor=nfactor+expected_sunrad;
			}
			else nfactor = 0;
		SPS_D("p_f"); SPS_D(previous_factor);
		//compute day cumulative sun radiation
		float day_sunrad = 0;
		for (byte j = 1; j < 24 / FREQ_WEATHER; j++) {
			int i = iw - j; if (i < 0)i = MAX_WEATHER_READ + 1 + i;
			 iwo = i + 1; if (i > MAX_WEATHER_READ)iwo = 0;
	//		if (j == 0)iwo = iw - 24 / FREQ_WEATHER + 1; if (iwo < 0)iwo = MAX_WEATHER_READ + iwo + 1;
			if (weather[i].sunrad >= 0 && weather[i].sunrad < 1100 && weather[iwo].sunrad >= 0 && weather[iwo].sunrad < 1100) {
				long dtime;
				if (j == 0)
					//		 dtime = -(weather[i].time - weather[iwo].time-SECS_PER_DAY);
					dtime = (-hour(weather[iwo].time) * 3600 + minute(weather[iwo].time) * 60) + (hour(weather[i].time) * 3600 + minute(weather[iwo].time) * 60);
				else
					dtime = -(weather[i].time - weather[iwo].time);

				if (dtime > 40000)dtime = 12000;                              // missing data---max step 3.5 hours
				day_sunrad += (weather[iwo].sunrad + weather[i].sunrad) / 2 * (dtime / 1000);
				SPS_D(i); SPS_D(iwo); SPS_D(dtime); SPS_D(weather[i].time); SPS_D(weather[i].sunrad); SP_D(" "); SPL_D(day_sunrad);
			}
		}
		if (day_sunrad > 28000.) {
			byte	ix = iw-1;
			if (ix < 0)ix = MAX_WEATHER_READ;
			while (weather[ix].penman > 6.) {
				ix--; if (ix < 0)ix = MAX_WEATHER_READ;
			}
			weather[iw].penman = weather[ix].penman;
		}
		else
		{
			w_mean.sunrad = int(day_sunrad);
			SP_D("t"); SPS_D(w_max.temp); SPS_D(w_mean.temp); SPS_D(w_min.temp);
			SP_D("h"); SPS_D(w_max.humidity); SPS_D(w_min.humidity);
			SP_D("w"); SPS_D(w_mean.wind);
			SP_D("s"); SPS_D(w_mean.sunrad);
			SP_D("r"); SPS_D(w_max.rain); SPS_D(w_min.rain);
			//__________________penman____________________________________________
			int sunHours = 10;
			float rad_clearsky = clear_sky_rad(
				sta.alt,
				et_rad(sta.lat, sta.lon,
					sunset_hour_angle(sta.lat, sol_dec(doy)),
					inv_rel_dist_earth_sun(doy)));

			float Sun = sol_rad_from_sun_hours(
				daylight_hours(doy),
				sunHours,
				et_rad(sta.lat, sta.lon,
					sunset_hour_angle(sta.lat, sol_dec(doy)),
					inv_rel_dist_earth_sun(doy)));
			if (iel > 8)Sun = w_mean.sunrad / 1000;
			SPS_D(rad_clearsky); SPS_D(Sun);

			float net_radi = net_rad(net_in_sol_rad(Sun),
				net_out_lw_rad(
					w_min.temp, w_max.temp,
					Sun,
					rad_clearsky,
					ea_from_tmin(w_min.temp)));
			SPS_D(net_radi);
			weather[iw].penman = penman_monteith_ETo(net_radi, w_mean.temp, w_mean.wind / 3.6,
				mean_es(w_min.temp, w_max.temp),
				ea_from_tmin(w_min.temp),
				delta_sat_vap_pres(w_mean.temp),
				psy_const(atmos_pres(sta.alt)),
				0.10);
		}
		//--------------------------zone water balance------------------ precision of dummy(byte) ?????+penman/12!!!!
		//					for (byte ic = 0; ic < N_OS_STA; ic++)
		//						for (byte i = 0; i < 8; i++)pd[ic].dummy[i] += -weather[iw].penman  * FREQ_WEATHER/24;
		 iwo = MAX_WEATHER_READ; if (iw > 0)iwo = iw - 1;
//		weather[iw].water = weather[iwo].water - weather[iw].penman  * FREQ_WEATHER / 24;
		weather[iw].write();
		SP(hour(weather[iw].time)); SP(":"); SP(minute(weather[iw].time)); SP(" t=");
		SP(weather[iw].temp); SP(" S=");
		SP(weather[iw].sunrad); SP("R=");
		SP(weather[iw].rain); SP(" ETo=");
		SP(weather[iw].penman); SP(" W=");
		SPL(weather[iw].water);
		return 2;
	}
#endif
				/*return 2;

			}
			else {
				SPL(" no Json!");
				SPL_D(i);
			}
		}
		client.stop();

		SPL(" no Json!");
		//buff[ii] = 0;
		//SPL_D(buff);
		return 1;

	}*/
#endif
#define STD_PENMAN 2
	void AreaFromET() {
		for (byte i = 1; i < sequn+1; i++)
		{
			byte freq = seq[i].day1;
			if (freq == 0)freq = 1;
			pd[seq[i].valv / 10].area[seq[i].valv % 10] = seq[i].dur*seq[i].flux / freq / 180 / STD_PENMAN;
			SPL_D(seq[i].dur*seq[i].flux / freq / 180 / STD_PENMAN);
		}
		for (byte ic=0;ic<N_OS_STA;ic++)
		eeprom_write_block((void *)&pd[ic], (void*)(PD_EEPROM_POS + int(ic) *PD_SIZE), PD_SIZE);

	}
	byte JsonDecode(byte k, char buff[], String nome[], float val[],byte code) {

		char* buffin;
		char * p2;
		char * p1;
		byte i = 0;
		if (code == 0) {
			for (i = 0; i < k; i++)
				nome[i] = char(34) + nome[i] + char(34);
			//SPL(nome);
		}
		buffin = strtok_r(buff, "},", &p1);

		SPL(buffin);
		char * title = " ";
		int comp = -1;
		while (buffin != NULL)
		{
			buffin = strtok_r(NULL, "},", &p1);
			if (buffin != NULL) {
				 SPL(buffin);
				if(code==0)title = strchr(strtok_r(buffin, ":", &p2), '"');
				else title = strtok_r(buffin, ":", &p2);
				if (title != NULL) {
					 SP(strlen(title)); SPL(title);
					i = 0;
					while (comp != 0 && i<k) {
						comp = strcmp(title, nome[i].c_str()); i++;
					}
					if (comp == 0) {
						SP("T "); SPL(title);
						// float val;
						char valore[20];
						char * pointer = strtok_r(NULL, "},", &p2);
						char *point1 = strchr(pointer, '"');			//if value in between " " extract it
						if (point1 != NULL) {
							byte kk = 1;
							while (pointer[kk] != '"') { valore[kk - 1] = pointer[kk++]; }
							valore[kk - 1] = 0;
						}
						else strcpy(valore, pointer);
						//SP(valore);
						if (strchr(valore, '.') == NULL) {
							long v = atol(valore); SPL(v);
							val[i - 1] = (float)v;
						}
						else  val[i - 1] = atof(valore);
						SPL(val[i - 1]);
						comp = -1;
					}
				}
				else comp = -1;
			}
		}
		return 0;

	}
	float readFromSensor(char * nome, byte ip) {
		char buff[500];
		char command[20];
		SP("reading ET0 from 192.168.1."); SPL(ip);
		sprintf(command, "/sensor?t=%d", 400);
		SPL(command);
		ServerCall(buff, command, ip);
		float val[2];
		String nomi[1] = nome;
		JsonDecode(1, buff, nomi, val, 1);
		SPL(val[0]);
		return val[0];
	}
	float readET0(byte days, byte ip,float *rain) {
		char  buff[500];
		char command[20];
		long time_d = 500;
		if (days < 20) {
			if (hour(now()) < 20 && days == 0) return -1;
			time_d = SECS_PER_DAY*days;
			if (time_d == 0)time_d = 450;
			time_t tim = now() - time_d;
			if (hour(tim) < 20)time_d -= (21 - hour(tim)) * 3600;
			SP(hour(tim)); SP(":"); SP(minute(tim));
		}
		else
			time_d = days * 1000;
		float ET0;
		
		
			SP("reading ET0 from 192.168.1."); SPL(ip);
			sprintf(command, "/sensor?t=%d", time_d);
			SPL(command);
			ServerCall(buff, command, ip);
			float val[2];
			String nomi[2] = { "avg_var","precip_today_metric" };
			JsonDecode(2, buff, nomi, val,1);
			SPL(val[0]);
			SPL(val[1]);
			*rain = val[1];
			return val[0];
	//	time_d = time_d + 60000L;
		
	}
	bool SendMessage(char * message)
	{
		File messfile;
		DateTime ora = adesso();
		if (SPIFFS.exists("/messages.txt"))
			messfile = SPIFFS.open("/messages.txt", "a+");
		else
			messfile = SPIFFS.open("/messages.txt", "w+");

		if (!messfile) { Serial.println("Cannot open message fie"); return 0; }
		messfile.seek(0, SeekEnd);
		messfile.print(ora.day()); messfile.print('-'); messfile.print(ora.month()); messfile.print(' ');
		messfile.print(ora.hour()); messfile.print(':'); messfile.print(ora.minute()); messfile.print(' ');
		messfile.println(message);
		SPL_D(message);
		messfile.close();
		return 1;

	}
	bool ReadMessage(int n)
	{
		File messfile;
		DateTime ora = RTC.now();
		if (SPIFFS.exists("/messages.txt"))
			messfile = SPIFFS.open("/messages.txt", "r+");
		else
			{ Serial.println("Cannot open message file"); return 0; }
		messfile.seek(0, SeekSet);

//		messfile.seek(n*100, SeekEnd);
/*
		long pos=messfile.position();
		if (pos == 0)return 0;
		pos--;
		messfile.seek(--pos, SeekSet);
		char c = messfile.read();
		byte  giorno = 32, mese = 12; char buff[10];
		while (ora.day()+ora.month()*31-n_day <= giorno+mese*31&&pos>0) {
			while (c != 10) { messfile.seek(pos--, SeekSet); c = messfile.read(); }
			buff[messfile.readBytesUntil('-', buff,10)]=0;
			giorno = atoi(buff);
			buff[messfile.readBytesUntil(' ', buff, 10) ] = 0;
			mese = atoi(buff);
			
		}
		
		if (pos == 0)return 0;
		*/
		while (messfile.available())Tclient.print((char)messfile.read());
		messfile.close();
		return 1;

	}
	//--------------------------------------------command queue-----------------------------
	static unsigned long timeTry[5];
	static String storedCommand[5];
	static String storedStream[5];
	static byte ipStored[5];
	static byte ntrial[5];
	static byte nTry=0;

	void API_repeat(String streamId, String command, byte ic, long timeDel) {
		if (timeDel > 0) {

			timeTry[nTry] = millis() + timeDel;
			storedCommand[nTry] = command;
			storedStream[nTry] = streamId;
			ipStored[nTry] = ic + 20;
			ntrial[nTry] = 0;
			nTry++;
			SPS_D(ic); SPS_D(command); SPL_D(nTry);
			return;
		}
	}
	void API_repeat(long timeDel){
		
		for (byte k = 0; k < nTry; k++)
			if (millis() >timeTry[k]&&ntrial[k]<10) {
				String streamId = storedStream[k];
				String command = storedCommand[k];
				byte ic = ipStored[k] - 20;
				timeTry[k] = millis() + timeDel;
				ntrial[k]++;
				if (API_command(streamId, command, ic)) {
					SP_D(command); SPL_D(ic);
					for (byte j = k + 1; j < nTry; j++) {
						storedStream[j - 1] = storedStream[j];
						storedCommand[j - 1] = storedCommand[j];
						timeTry[j - 1] = timeTry[j];
						ipStored[j - 1] = ipStored[j];
						ntrial[j - 1] = ntrial[j];
					}
					k--;
					nTry--;
				}
			}
			return;
	}
	byte API_command(String streamId, String command, byte ic) {

		String privateKey = "a6d82bced638de3def1e9bbb4983225c"; //MD5 hashed
		const int httpPort = 80;
		
		IPAddress jsonserver = IPAddress(192, 168, 1, ic + 20);
		SP(jsonserver);
		if (!client.connect(jsonserver, 80))
		{
			client.stop();

			SPL(" connection failed ");
			return 0;
		}
		//	SP("Connect: "); SP(jsonserver);
		String url = streamId;
		url += "?pw=";
		url += privateKey;
		url += command;
		SP_D(" Requesting URL: ");
		SPL_D(url);
		// This will send the request to the server
		client.print(String("GET ") + url + " HTTP/1.1\r\n" +
			"Host: " + "192.168.1.20" + "\r\n" +
			"Connection: close\r\n\r\n");
		byte count = 0; bool res = false;
		while (!client.available()&&count<240) { delay(100); count++; }
		if (count == 240)return 0;
		while (client.available()) {
			char c = client.read();
			SP_D(c);
			if (c == ':'&&client.available()) {
				c = client.read();
				SP_D(c);
				if (c == '1')res = true;
			}
		}
		//if (res)if (client.read() != '1'&&client.read()!='1')res = false;
		client.stop();
		return res;
	}

	int ServerCall( char buff[],char command[], byte ic) {
		WiFiClient client;
		//String privateKey = "a6d82bced638de3def1e9bbb4983225c"; //MD5 hashed
		const int httpPort = 80;
		IPAddress jsonserver = IPAddress(192, 168, 1, ic);

		if (!client.connect(jsonserver, 80))
		{
			client.stop();

			SP("connection failed "); SPL(jsonserver);
			return 0;
		}
		//	SP("Connect: "); SP(jsonserver);
		String Out = "GET ";
		Out += command;
		Out += " HTTP/1.1\r\n Connection: close\r\n\r\n";
		client.print("GET ");
		client.print(command);
		client.println(" HTTP/1.1\r\n Connection: close\r\n\r\n");
		byte i = 0;
		while (!client.available()&&i<255) {
			i++; delay(50); Serial.print('.');
		}
		if (i == 255) {
			client.stop(); SPL("No reply"); return 0;
		}
		SPS_D("resp=");

		int j = 0;
		byte cont = 0;
		while (j<500 && cont<200) {
			if (client.available())buff[j++] = client.read();
			else { delay(50); cont++; }
		}
		/*while (client.available()) {
			buff[j++] = client.read(); Serial.print(buff[j - 1]); 
				
		}*/
		buff[j++] = 0;
		client.stop();
		SPL_D(j);
		return j;
	}
#define RAIN_LOST 0.5
#define WEATHER_DAYS 3
	byte month_penman[12] = { 10,11,20,25,33,39,46,39,26,18,11,10 };//monthly average Savona *10
	int weather_control(float day_rain ,float ET0) {	//set water delay reading 
		float prevRain = cD.cumulRain;												//calculate total rain over weathre days
		if (cD.cumulRain == 0 && day_rain <= 1.)return 0;
		cD.cumulRain -= ET0;
		SP_D("CuRain"); SPL_D(cD.cumulRain);
		if (cD.cumulRain < 0)cD.cumulRain = 0;

		if (day_rain > 20)day_rain = (day_rain - 20)*RAIN_LOST;
		if(cD.cumulRain<20)
		cD.cumulRain += day_rain;

#define MAX_RAIN 30 //this is maximum rain soil can absorb____________________________________

		if (cD.cumulRain > 20)cD.cumulRain = MAX_RAIN - 20 * (MAX_RAIN - 20) / cD.cumulRain;
		long max_rain_delay = 0;
		for (byte i = 0; i < 5; i++) if (cD.rain_delay[i] > max_rain_delay)max_rain_delay = cD.rain_delay[i];
		SPL_D( (max_rain_delay - now()) / SECS_PER_DAY*month_penman[month()]);
		// change raindelay if today rain>1 and total cumulated>5 or if cumulated rain is less than day delay interval * penman
		if ((cD.cumulRain >=5&&day_rain>1)||cD.cumulRain+2<(max_rain_delay-now())/SECS_PER_DAY*month_penman[month()]) {
			SP("total-day rain "); SP(cD.cumulRain); SP(" "); SP(day_rain); SP("to");
			float rain_delay = cD.cumulRain / month_penman[month()]*10;
			char rainCommand[10];
			time_t rain_del = now() + rain_delay*SECS_PER_DAY;
			rain_del = rain_del - hour(rain_del) * 3600;						 // __________________________stop at midmnight
			sprintf(rainCommand, "&rd=%d", int(rain_delay * 24) - hour(rain_del));
			SP_D(rain_del); SP_D(" "); SPL_D (rainCommand);

			for (byte ic = 0; ic < N_OS_STA; ic++) {
		// correct rain delay if greater or smaller	
		//		if (rain_del > cD.rain_delay[ic])
				{
					cD.rain_delay[ic] = rain_del;
					if (API_command("/cv", rainCommand, ic)) { SP(ic); SP("_"); }
					else API_repeat("/cv", rainCommand, ic, 100000);
				}
			}
			eeprom_write_block(&cD, (void *)CURRENT_DATA_POS, sizeof(CurrentData));
		}
		return int((cD.cumulRain-prevRain)*10);
	}


	byte APIcall(String streamId, byte ic) {

		//String privateKey = "opendoor";
		String privateKey = "a6d82bced638de3def1e9bbb4983225c"; //MD5 hashed
		const int httpPort = 80;
		IPAddress jsonserver = IPAddress(192, 168, 1, ic + 20);
		SP(jsonserver);
		//	delay(2000);
		if (!client.connect(jsonserver, 80))
		{
			client.stop();

			SPL("connection failed "); 
			return 0;
		}
//	SP("Connect: "); SP(jsonserver);
		String url = streamId;
		url += "?pw=";
		url += privateKey;
// This will send the request to the server
		client.print(String("GET ") + url + " HTTP/1.1\r\n" +
			"Host: " + "192.168.1.20" + "\r\n" +
			"Connection: close\r\n\r\n");
// read json------------------------------------------------
		int i = 0, json_try = 0; bool ISJSON = false;
		long time_step = millis() + MIL_JSON_ANS;
		while (millis() < time_step)
		{
// Read all the lines of the reply from server and print them to Serialbol 
			while (client.available()) {
				char c = client.read();
				if (c == '{')ISJSON = true;
				if (ISJSON) json[i++] = c;
			}
			if (ISJSON) {
				json[i] = 0;
				client.stop();

				SP("Connected to:"); SPL(jsonserver);
				SPL(" Json read!");
				SPL_D(json);
				return 2;
			}
			//	else {
			//		SP("."); delay(100); //SPL_D(i);
			//	}
		}
		client.stop();

		SPL(" no Json!");
		return 1;

	}
#define NUM_FLUX_COLOR 5
	int color[] = { ILI9341_GREEN,ILI9341_BLUE,ILI9341_YELLOW,ILI9341_ORANGE,ILI9341_RED };

	//////////// PRINT STATUS/////
#define TABW 40		
	/*void printLine(byte x,byte y,byte nargs, ...) {
		va_list args;
		byte tabn = 1;
		TFWin.charPos(x, y);
		va_start(args, nargs);
		for (i = 0; i < nargs; i++)
		{
			TFWin.print(args);
			TFWin.charPos(x + TABW*tabn++, y);
		}
		va_end(args);
	}
	*/
	time_t adesso() { if (RTC.isrunning()) RTC.now(); else now(); }
#define DLINES 10
#define STARTLINE 20
	void print_status() {
		DateTime ora = adesso();
		print_Inter();
// first line for interrupts status/////////////////////////////////////////////////////////
		TFWin.charPos(10, 0, 280);

		TFWin.print("sta IP lastco lastv. progch timech"); 
		TFWin.print(ora.hour()); TFWin.print(':'); TFWin.print(ora.minute());
		for (byte i = 0; i < N_OS_STA; i++) {
			byte tab = 1;
			TFWin.charPos(i*DLINES + STARTLINE, 0, 280);

			TFWin.print("..1."); TFWin.print(20 + i); TFWin.charPos(i*DLINES + STARTLINE, TABW*tab++, 0);
			TFWin.print(pd[i].connTime / 60); TFWin.print(':'); TFWin.print(pd[i].connTime % 60);
			TFWin.charPos(i*DLINES + STARTLINE, TABW*tab++, 0);
			TFWin.print(pd[i].progChang); TFWin.charPos(i*DLINES + STARTLINE, TABW*tab++, 0);
			TFWin.print(pd[i].progChangTime / 60); TFWin.print(':'); TFWin.print(pd[i].progChangTime % 60);
			TFWin.charPos(i*DLINES + STARTLINE, TABW*tab++, 0);
			char buf[8];
			byte nchar = sprintf(buf, "%d", pd[i].valveUsed);
			SP_D("valves "); SP_D(buf); SP_D(" status "); for (byte ix = 0; ix < nchar; ix++) {SP_D(pd[i].valveStatus[buf[ix]-'0']); SP_D(" ");}
			SPL_D();
			if(pd[i].valveUsed!=0)
				for (byte ic = 0; ic < nchar-1; ic++) {
					if (pd[i].valveStatus[buf[ic] - '0'] != 0)
						TFWin.textColor(color[pd[i].valveStatus[buf[ic] - '0']]);
					TFWin.print(buf[ic]);
					TFWin.textColor(ILI9341_WHITE);
				}
			TFWin.textColor(ILI9341_WHITE);
			TFWin.println();
		}
	}
	void reset_logfile(){
		SPIFFS.remove("/log1.txt");

		SPIFFS.remove("/messages.txt");

		SPIFFS.rename("/logs.txt", "/log1.txt");
		SPIFFS.remove("/logs.txt");
		logfile = SPIFFS.open("/logs.txt", "w+");
		posFile = 0;
	}
#define TIME_INT_JSON   3600000  // 1h
#define TIME1_INT_JSON 43400000  //12 h
#define TIME2_INT_JSON 14400000  //4h
	byte ichange = 0;
	byte change_prog_n[20];
	unsigned long time_json = 0, time_json1 = 0, time_json2 = 0;
	static bool json_flag = false;
	static byte count;
	byte countp = 0;
	static 	bool noCon[] = { true,true,true,true,true },rest=false;
	DateTime last_JP[5]; long timerest,m10000=60000;
	boolean input_seq = false, input_pd = false,input_kc=false;
	static byte ET = 0;
////////rain//////
#define RAIN_POS 2500
	class rain {
		
	public:
		byte rainD = 0;
		 rain() {
			rainD = EEPROM.read(RAIN_POS - 1);
			if (rainD >= 60)rainD = 0;
		//rainD = 1;

		}
		void store(float rain, int day) {
			if (rain > 128)rain = 128;
			rainD = EEPROM.read(RAIN_POS - 1);
			byte rainB = int(rain) *2+ int(day / 256);
			byte dayB = day % 256;
			SP_D("Store Rain D"); SPS_D(rainD);
			SPS_D(rainB); SPS_D(dayB); SPL_D();
			if (rainD > 60) {
				SPL("toomany rainy days!-reset"); 
				rainD = 0;
		//		EEPROM.write(RAIN_POS - 1, 0);
		
			}   
			EEPROM.write(RAIN_POS + rainD * 2, rainB);
			EEPROM.write(RAIN_POS + rainD * 2+1, dayB);
			rainD++;
			EEPROM.write(RAIN_POS - 1, rainD);

			EEPROM.commit();
		}
		float get(int day) {
			rainD = EEPROM.read(RAIN_POS - 1);
		//	SP_D(day);			 SP_D("Rd"); SPL_D(rainD);
			for (byte i = 0; i < rainD; i++) {
				byte rainB = EEPROM.read(RAIN_POS + i * 2);
				byte dayB = EEPROM.read(RAIN_POS + i * 2 + 1);
		//		SPS_D(rainB); SPS_D(dayB); SPS_D(rainB & 1); SPL_D();
				if ((rainB & 1) * 256 + dayB == day)return rainB / 2.;
			}
		return 0;

		}
		void print(){
			rainD = EEPROM.read(RAIN_POS - 1);
			SP_D("Rd"); SPL_D(rainD);
			//	SP_D(day); SP_D("Rd"); SPL_D(rainD);
			for (byte i = 0; i < rainD; i++) {
				byte rainB = EEPROM.read(RAIN_POS + i * 2);
				byte dayB = EEPROM.read(RAIN_POS + i * 2 + 1);
				//		SPS_D(rainB); SPS_D(dayB); SPS_D(rainB & 1); SPL_D();
				SP_D((rainB & 1) * 256 + dayB); SP_D("\t"); SPL_D(rainB / 2.);
				
			}
			
		}

	};
	rain r;
#ifdef IOT
	//#include <thingspeak-arduino-master\src\ThingSpeak.h>
	struct MM {
	public:
		unsigned int x;
		int y;
	};
	class  IOTstr {
	public:

		long Channel;
		const char* key;
		long values[8];
		byte fieldn[8];
		byte indval = 0;
		byte nchar = 0;
		IOTstr(char * mkey) { key = mkey; }
		byte updateThingSpeak(long value, byte field_n, byte code)
		{
			if (code == 0) {
				//void updateThingSpeak(long value, byte code) {
				//Serial.println("update");
				values[indval++] = value; nchar += 9; while (value >= 10) { value /= 10; nchar++; }
				fieldn[indval - 1] = field_n;
				return indval - 1;
			}
			else
			{
#if ARDUINO <100 	
				//Serial.println("connect....");
				byte IOTip[4] = { 184,106,153,149 };  //ip of api.thingspeak.com
				Client IOTclient(IOTip, 80);   //184,106,153,149
				if (IOTclient.connect()) {
#else
#ifndef ESP8266 
				EthernetClient IOTclient;
#else
				WiFiClient IOTclient;
#endif
			    if(	IOTclient.connect("api.thingspeak.com", 80)){
#endif

					delay(1000);
					String url = "GET /update?api_key=";
					url += key;
					url += "&";
					SPL_D(url);
					IOTclient.print(url);// "GET /update?api_key=FPIY8XPZBC2YGSS7&");

					for (int ii = 0; ii < indval; ii++) {
						SP_D(fieldn[ii]); SP_D('>'); SPL_D(values[ii]);
						IOTclient.print("field"); IOTclient.print(fieldn[ii]); IOTclient.print("="); IOTclient.print(values[ii]);
						if (ii < indval - 1)IOTclient.print("&"); else IOTclient.println('\n');
					}

					//IOTclient.println(" HTTP/1.1\nHost: api.thingspeak.com\nConnection: close\n");//" HTTP/1.1\r\n" + "Host: " + "api.wunderground.com" + "\r\n" + "Connection: close \r\n\r\n"
					indval = 0; nchar = 0;
					delay(2000);
					byte inc = 0;
					//------------wait replay------------------------------------------------------------
					while (!IOTclient.available() && inc < 250) { delay(20); inc++; }
					SPL_D(inc);
					while (IOTclient.available()) { SP_D(IOTclient.read());}

				}
				else { SPL_D("conn.failed"); }
				IOTclient.stop();
				return 0;

				}
			return 100;
			}
		};
//	MM actualMM[30];
	IOTstr IOTv[5] = {"YVDDAJM6VBW72YH8", "15II2JPJ6PHZAJFC","KITD851JT8GQGQWL","AIN1ZOAMHDYQZO64","GW5VYMOOCGK62FDD" };
//179379,"YVDDAJM6VBW72YH8", 194350, "15II2JPJ6PHZAJFC", 194351, "KITD851JT8GQGQWL", 194352, "AIN1ZOAMHDYQZO64", 194353, "GW5VYMOOCGK62FDD" };

	byte load_mm(byte nvalve, int Dmm) {
		//load on vector actualMM and on thingspeak  mm of rain calculated now() for valve nvalve 
		byte j = 0;
		for (byte i = 0; i<N_curves; i++)

			if (CurvesN[i] == nvalve) {
				cD.mm[i] += Dmm;
				j = i;
			}
		IOTv[nvalve / 10].updateThingSpeak(cD.mm[j], nvalve % 10, 0);
//		ThingSpeak.setField(nvalve % 10, actualMM[j].y);
//		ThingSpeak.writeFields(IOTv[nvalve / 10].Channel, IOTv[nvalve / 10].key);

		return j;
	}



#endif
	bool FTP_mode = false;
	byte ETcalc[40], prevET;
/////////////////////////////////////////////////////////////// loop /////////////////////////////////////////////////////
	void loop() {
		API_repeat(120000);
#ifdef FTP

		if(FTP_mode)
		ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!  
#endif
#ifdef OTA
		ArduinoOTA.handle();
#endif
		/*	if (millis() > m10000) {
				m10000 = millis() + 60000;	Serial.print('P'); Serial.println(posFile);
				logfile.close(); logfile = SPIFFS.open("/logs.txt", "r+");

			}*/
		char c, cc;

		/////////////////////////////////////////monitor edit intervals///////////////////////////////////////////////////////////////
		cc = edit_intervals();		//////////////////////////////////////////////////////pulse monitor routine///////////////////////////////////////////////////////////////
		blinker();
		/////////////////////////////////////////////////////Opensprikler control/////////////
		//password HASHED=a6d82bced638de3def1e9bbb4983225c
		DateTime ora = adesso();

		//  if(cc!=' ')SPL_D(cc);
		  //s---------------------------------print sequence---------------------------
		if (cc == 's') { print_seq(0);			//          if (Tclient.available())print_seq(Tclient.read() - '0');
		}
		if (cc == ' ')cc = 0;
		//#ifdef LCD_TOUCH
		c = touch_control(cc);
		if (c == 'F') {
			if (!FTP_mode)FTP_mode = true; else FTP_mode = false;
			SPL_D(FTP_mode);
		}
		if (c == 'P') {
			int arr[N_Maxcurves];
			int val = inputI("time?");
			plot_interp(arr, val);
			SP_D(hour(plot_start_time + val * 60)); SPS_D(minute(plot_start_time + val * 60)); SPL_D();
			for (byte i = 0; i < N_curves; i++) {
				SPS_D(CurvesN[i]); SPS_D(arr[i]); SPL_D();
			}
		}
		//r---------------------------------restart/reset--------------------
		if (c == 'r') {
			rest = true;
			timerest = millis();
			SPL("Restart 0-no rest,1-no reset,2-reset,3-logFileReset");
		}
		if (rest&&c == '1')
		{
			SPL("Restarting..");
			ESP.restart();
		}
		else if (rest&&c == '2')
		{
			eeprom_write_byte(0, 211);
			SPL("Resetting");
			ESP.restart();
		}
		else if (rest&&millis() > timerest + 5000)rest = false;
		else if (rest&&c == '3') {
			SPL_D("resetting logFile");
			reset_logfile();
		}
#ifndef NO_ET0
		if (c == 'W') { weather[iw].water = 0; SP(" reset  wather "); SendMessage("reset water"); }
#else
#define MYSECS_PER_YEAR 31557600UL
#define ET_POS 2650	
//#define RAIN_POS 3020
#ifdef WATERLOG
		if (c == 'w') {
			int day = inputI("day"); byte month = inputI("month/valve");
			if (day > 0)
				print_waterlog(day, month);
			else
				valve_print_waterlog(-day, month);
		}
#endif
		if (c == 'W') {
			SPL_D();
			r.print();
			int dayBack = inputI("dayList");
			
			SPL_D("ET0\tRain");
			for (byte i = dayBack; i > 0; i--) {
				//SPL(ET_POS + (now() % MYSECS_PER_YEAR+43600L) / SECS_PER_DAY - i);
				SP_D(float(eeprom_read_byte((byte *)(ET_POS + (now() % MYSECS_PER_YEAR + 43600L) / SECS_PER_DAY - i))) / 40.);
				SP_D("\t"); SPL_D(r.get((now() % MYSECS_PER_YEAR + 43600L) / SECS_PER_DAY - i));
			}
			SPL_D("ET0 rain correction");
			dayBack = inputI("day back?-0_exit");
			SPL_D();
			if (dayBack > 0) {
				float rain;
				ET = readET0(dayBack, 30,&rain) * 40;
				//____________________________________save ET0 of dayback  to EEPROM_________________________

				int day = (now() % MYSECS_PER_YEAR+43600L) / SECS_PER_DAY;
				SPL(ET_POS + day - dayBack);
				eeprom_write_byte((byte *)(ET_POS + day - dayBack), ET);
				//_________________________________store rain of dayback on EEPROM_______________________________________
				int rainBack = inputI("store rain_b?-0_exit<<<<_neg. reset all rain values");
				SPL();
				if (rainBack > 0) {
					r.store(rain, (now() % MYSECS_PER_YEAR + 43600L) / SECS_PER_DAY-dayBack);
					weather_control(rain, ET / 40.);

				}
				if (rainBack < 0)EEPROM.write(RAIN_POS-1, 0);
				EEPROM.commit();
			}

		}
#define HOUR_ET0 24

		//__________________________________rain delay end_________________________________________________
		//if (now() >= cD.rain_delay[0] - 3600) {
		//	float rain = readFromSensor("precip_today_metric", 30);
		//	if (rain > 2)weather_control(rain, 0);
		//}
		//____________________________________before MidNight ____________________________________________________
		if (hour() == HOUR_ET0-1 && minute() > 55)
		{
			if (ET == 0) {								//first time ET0 is 0
				float day_rain;
				ET = readET0(0, 30,&day_rain) * 40;
				if (ET > 0) {
					SP("storedET*40="); SPL(ET);
					//____________________________________save ET0  to EEPROM_________________________
					int 	 day = (now() % MYSECS_PER_YEAR) / SECS_PER_DAY;

					if (day_rain > 1) {
						r.store(day_rain, day);
					}
					eeprom_write_byte((byte *)(ET_POS + day), ET);
					//					rain.k++;rain.day[rain.k]=day;rain.mm[rain.k]=rain;eeprom_write_block(&rain,RAIN_POS,sizeof(rain));
					int cumulRainVar=weather_control(day_rain, ET / 40.);
#ifdef IOT
//----------------------add rain mm increments to each zone--------------------- 
					for (byte iseq = 0; iseq <N_curves; iseq++) {
						load_mm(CurvesN[iseq], cumulRainVar-ETcalc[iseq]);   //----------------load values to be written on thingspeak
		//ET water reduction already accounted ETcalc[iseq] must be deducted
					}
					eeprom_write_block(&cD, (void *)CURRENT_DATA_POS, sizeof(CurrentData));
					for (byte icc = 0; icc < 4; icc++)IOTv[icc].updateThingSpeak(0, 0, 1);//---write on ThingSpeak for each OS station
#endif					
					EEPROM.commit();
				}
			}
		}
		else ET = 0;

#endif
#ifdef FREQ_WEATHER		
		static bool status_printed;
		if (ora.minute() == 0)
		{
			if (!status_printed) { status_printed = true; print_status(); }
		}
		else status_printed = false;
#ifndef NO_ET0
		if (next_ora >= 24 && ora.hour() == 0)next_ora -= 24;
		if (ora.hour() >= next_ora)
		{
			if (count_trial < 10)
			{
#ifdef APIWU
				if (APIweather("/api/48dfa951428393ba/conditions/q/Italy/pws:ISAVONAL1.json") == 2)
#else
				if(1)
#endif
				{
					if (weather[iw].rain > 0) {
						for (byte i = 0; i < N_OS_STA; i++)if (weather_control(i)) {
							SP("Delayed"); SPL(i);
						}
						SendMessage("rain delay");
					}
					next_ora = ora.hour() + FREQ_WEATHER;
					SP_D("next w.h."); SPL_D(next_ora);
				}
			}
			else
			{
				count_trial = 0;
				SP_D("no Wu reached ");
				next_ora = ora.hour() + FREQ_WEATHER;
				SP_D("next w.h."); SPL_D(next_ora);
			}
		}
#endif
#endif
		//k-----------------------change crop cooef.
		if (input_kc || c == 'k') {
			SP("Pd modify Kc: "); int valv = inputI("valv?");
			if (valv < 0) {
				SPL_D("Save Pd!");
				input_pd = false;
				for (byte i = 1; i < N_OS_STA; i++)
					eeprom_write_block((void *)&pd[i], (void*)(PD_EEPROM_POS + i *PD_SIZE), PD_SIZE);
			}
			else {
				pd[valv / 10].Kc[valv % 10] = inputI("Kc?(0.1 to 2.56 int*100)");
				input_pd = true;
				SP_D("pd["); SP_D(valv / 10); SP_D("].Kc["); SP_D(valv % 10); SP_D("]="); SPL_D(pd[valv / 10].Kc[valv % 10]);
			}
		}
		//a-----------------------change area
		if (input_pd || c == 'a') {
			SP("Pd modify Area "); int valv = inputI("valv");
			if (valv < 0) {
				SPL_D("Save Pd!");
				input_pd = false;
				for (byte i = 0; i < N_OS_STA; i++)
					eeprom_write_block((void *)&pd[i], (void*)(PD_EEPROM_POS + i *PD_SIZE), PD_SIZE);
			}
			else {
				pd[valv / 10].area[valv % 10] = inputI("area");
				input_pd = true;
				SP_D("pd["); SP_D(valv / 10); SP_D("].area["); SP_D(valv % 10); SP_D("]="); SPL_D(pd[valv / 10].area[valv % 10]);
			}
		}
		//f-----------------------change flux
		if (input_seq || c == 'f') {
			SP("Seq modify Flux "); int iseq = inputI("seq");
			if (iseq < 0) {
				input_seq = false;
				eeprom_write_block((void *)&seq, (void *)(SEQ_EE_START + 1), MAXSEQ * sizeof(sequence));
			}
			else {
				seq[iseq].flux = inputI("flux");
				input_seq = true;
				SP_D("seq["); SP_D(iseq); SP_D("].flux="); SPL_D(seq[iseq].flux);
			}
		}
		// print stations names---------------------------------------------------
		if (c == 'n') {
			byte j = 0;
			for (byte k = 0; k < N_OS_STA; k++)
				for (byte i = 0; i < 8; i++){
					byte plot_n = 0;
					// ---------print only if plot of valve exist--------------------------
					while (CurvesN[plot_n++] != k * 10 + i&&plot_n<N_curves);
					if (plot_n < N_curves) {
						SP(k); SP(" "); SP(i); SP(" ");
						TFWin.textColor(CurveColor[plot_n]);
						//const char* nome= pd[seq[i].valv / 10].names[seq[i].valv % 10].c_str();
						//nome[strlen(nome) + 1] = 0;
						for (byte jj = 0; jj < strlen(pd[seq[i].valv / 10].names[seq[i].valv % 10]); jj++)
						{
							SP(pd[k].names[i][jj]);
						}

						SPL();

						TFWin.textColor(ILI9341_WHITE);
					}
				}

		}

		//n----------------------/jn API load names
	//	if (c != 0||time_json>0||time_json1>0)
	//	{   
		if (c == 'o') {
			if (time_json1 == 0) { time_json1 = TIME1_INT_JSON; SP("json /jo /jn on"); }
			else {
				time_json1 = 0; SP("json /jo /jn off");
			}
		}
		if (c == 'p') {
			if (time_json2 == 0) { time_json2 = TIME2_INT_JSON; SP("json /jp on"); }
			else { time_json2 = 0; SP("json /jp off"); }
		}
		if (c == 'c') {
			if (time_json == 0) { time_json = TIME_INT_JSON; SP("json /jc /jn on"); }
			else { time_json = 0; SP("json /jc /jn off"); }
		}

		//		if (c == 'p'&&time_json > 0)time_json = 0;
		//		else
		//		if (c == 'c'&&time_json1 > 0)time_json1 = 0;
		//		else
		//		{
			//p-----------------------/jp API programs		
		//			if(c=='p'&&time_json2==0||time_json2>0)   //4h
		

		if (c=='p'||(millis() >= time_json2&&time_json2!=0))
		{
			time_json2 = millis() + TIME2_INT_JSON;

			bool noConFlag = false;
			for (byte ic = 0; ic < N_OS_STA; ic++)
				if (noCon[ic])
				{

					SP_D(ora.hour()); SP_D(":"); SP_D(ora.minute()); SP_D("  ");
					if (APIcall("/jp", ic) < 2 && countp++ < 10) {
						time_json2 = millis() + 60000;
						noCon[ic] = true;
						noConFlag = true;
						//delay(3000); 
					}
					else
						//int connTime[5],progChangTime[10], progChang[10];
						if (countp <= 10)
						{
							pd[ic].connTime = ora.hour() * 60 + ora.minute();
							noCon[ic] = false;
							byte pmod[10];
							Json_Extract_jp(ic, pmod);

							for (int i = 0; i < pd[ic].nprogs; i++)
							{
								if (pmod[i] != 0)
								{
									Encode_jp(i, ic); SPL();
									SP("Sta."); SP(ic); SP("prog."); SP(i);

								}
								if (pmod[i] != 0)
								{
									if (pmod[i] == 1)
									{
										SPL(" new!");
										program_put_sequence(ic, i);
										pd[ic].progChang = pd[ic].progChang * 10 + i + 1;
										pd[ic].progChangTime = ora.hour() * 60 + ora.minute();

									}
									else
									{
										SPL("Modif.");
										program_rem_sequence(ic, i);
										program_put_sequence(ic, i);
										pd[ic].progChang = pd[ic].progChang * 10 + i + 1;
										pd[ic].progChangTime = ora.hour() * 60 + ora.minute();


									}
								}
							}
						}



					//  if (!json_flag)time_json = millis() + TIME_INT_JSON;
					//	SP("try again in s."); SPL((time_json - millis()) / 1000);
					else { SP("192.168.1."); SP(20 + ic); SPL(" not reachable!"); 
					char buff[20]; sprintf(buff, "/jp Cannot reach 192.168.1.%d", 20 + ic);
					SendMessage(buff);
					}
				}
			if (!noConFlag)
				for (byte jj = 0; jj < 5; jj++)noCon[jj] = true;
			print_status();
		}
		//c-----------------------jc API control-----------------------		

		//		if (c == 'c'&&time_json1 == 0 || time_json1>0)

		if (c=='c'||(millis() >= time_json&&time_json != 0))
		{
			time_json = millis() + TIME_INT_JSON;
			for (byte ic = 0; ic < N_OS_STA; ic++)
			{
				byte count = 0;
				//wait 6 sec for 10 time to get connection
				while (!APIcall("/jc", ic) && count++ <=5) { delay(2000); }
				if (count <= 5)
				{
					Json_Extract_jc(ic);
				}
				else { SP(20 + ic); SPL(" not reachable!"); 
				char buff[20]; sprintf(buff, "/jc Cannot reach 192.168.1.%d", 20 + ic);
				SendMessage(buff);
				}

			}
		}
		//o----------------------------------jo jn options--------------------------
		if (c=='o'||(millis() > time_json1&&time_json1 != 0))
		{
			time_json1 = millis() + TIME1_INT_JSON;


			for (byte ic = 0; ic < N_OS_STA; ic++)
			{
				byte count2 = 0;
				//wait 6 sec for 10 time to get connection
				while (!APIcall("/jo", ic) && count2++ <= 5) { delay(1000); }
				if (count2 <= 5)
				{
					Json_Extract_jo(ic);
					if (APIcall("/jn", ic) == 2)Json_Extract_jn(ic);

				}
				else { SP(20 + ic); SPL(" not reachable!");
				char buff[20]; sprintf(buff, "/jn Cannot reach 192.168.1.%d", 20 + ic);
				SendMessage(buff);
				}
			}


		}
		//		}
		//	}
	}
	
//	void blPr(void * pointer,byte size) {
//		for (byte i = 0; i < size; i++) { Serial.print(' '); Serial.print((byte) (*pointer+i))); }
//	}

#define PROGRAM_TYPE_WEEKLY   0
#define PROGRAM_TYPE_BIWEEKLY 1
#define PROGRAM_TYPE_MONTHLY  2
#define PROGRAM_TYPE_INTERVAL 3

#define STARTTIME_SUNRISE_BIT 14
#define STARTTIME_SUNSET_BIT  13
#define STARTTIME_SIGN_BIT    12

#if !defined(SECS_PER_DAY)
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#endif
	bool print_seq(int day)
	{
		sequn = eeprom_read_byte(SEQ_EE_START);
		SP_D("seq.n"); SPLF_D(sequn, DEC);
		if (sequn == 0)return false;
		eeprom_read_block(&seq, (void *)(SEQ_EE_START + 1), MAXSEQ * sizeof(sequence));
		//byte day = 0;
		DateTime oggi = adesso();
		SPL_D("day+\tstart\tdur.\tnprog\tvalve\tday[0]\tday[1]\tflags\tflux.\tarea\tmm_eq");
		bool confirmed[4][8];
		byte MMeq[4][8];
		for (byte i = 0; i < 4; i++)for (byte j = 0; j < 8; j++) { confirmed[i][j] = true; MMeq[i][j] = 0; }
		for (int i = 1; i < sequn + 1; i++)
		{
		//	bool prin = false;
			if (day >= 0)
			{
			//	SP_D(day);
			//	if (seq[i].Check_day_match(oggi.operator+(day * 24 * 3600))) { SP_D("A "); }
							}

		//	else
		//		prin = true;
		//	if (prin)
			{
			//	SP_D("\t");
				SP_D(seq[i].start / 60); SP_D(":"); SP_D(seq[i].start % 60); SP_D("\t");
				SP_D(seq[i].dur); SP_D("\t");
				SP_D(seq[i].progIndex); SP_D("\t");
				SP_D(seq[i].valv); SP_D("\t");
				long valvole = pd[seq[i].valv / 10].valveUsed;
				while (seq[i].valv%10  !=valvole % 10 && valvole >= 10) { valvole /= 10; }
				if(valvole<10&& seq[i].valv != valvole)
					pd[seq[i].valv / 10].valveUsed= pd[seq[i].valv / 10].valveUsed*10+seq[i].valv%10;
				confirmed[seq[i].valv / 10][seq[i].valv % 10] = true;
				SP_D(seq[i].day0); SP_D("\t");
				SP_D(seq[i].day1); SP_D("\t");
				SP_D(seq[i].flags); SP_D("\t");
				SP_D(seq[i].flux); SP_D("\t");
				SP_D(pd[seq[i].valv / 10].area[seq[i].valv % 10]); SP_D("\t");
				SPL_D(pd[seq[i].valv / 10].dummy[seq[i].valv % 10]);// -weather[iw].water*pd[seq[i].valv / 10].area[seq[i].valv % 10]);
			//	if (pd[seq[i].valv / 10].area[seq[i].valv % 10] > 0) {
			//		MMeq[seq[i].valv / 10][seq[i].valv % 10] += seq[i].flux*(seq[i].dur / 18.) / pd[seq[i].valv / 10].area[seq[i].valv % 10];
			//		SPL_D(MMeq[seq[i].valv / 10][seq[i].valv % 10]);
			//	}
			//	else SPL_D();
			}
		}
		for (byte ic = 0; ic < N_OS_STA; ic++) {
			SP_D(pd[ic].valveUsed); SPS_D(pd[ic].Status);
			for (byte j = 0; j < 8; j++) {

				if (!confirmed[ic][j])
				{
					SP_D(" ");
				}
				else
				{
					SP_D("_");
				}
				SP_D(pd[ic].valveStatus[j]);

		/*		if (!confirmed[ic][j])
				{ // not present in seq. remove valv n. from valveUsed
					long valvole = pd[ic].valveUsed;
					long corrValv = 0;
					while (valvole >= 10) {
						if (j != valvole % 10)
							corrValv = corrValv * 10 + valvole % 10;
						valvole /= 10;
					}
					pd[ic].valveUsed = corrValv;
					pd[ic].valveStatus[j] = 0;
				}*/
			}
			SPL_D();
		}
	}
#define MAX_PLOT_DAYS 7
#define EEPOS_PLOT 4021
#define EEPOS_COMMAND 4000
#define EESIZE_W 80
#define PLOT_START_DAY -3
	struct Working_Set {
		byte ncommand;
		char commands[20];
		byte N_curves;
		unsigned plotStartTime; 
		int plotStartValue[20];
		void savePlot() {
			int Epos = EEPOS_PLOT;
			eeprom_write_byte((unsigned char *)Epos++, N_curves);
			eeprom_write_int(Epos++, plotStartTime>>16);
			Epos++;
			eeprom_write_int(Epos++, plotStartTime & 0xFFFF);
			Epos++;
			for (byte i = 0; i < N_curves; i++){
				eeprom_write_int(Epos++, plotStartValue[i]); 
				Epos++;
		}
		}
		void saveCommand() {
			int Epos = EEPOS_COMMAND;
			for (byte i = 0; i < ncommand; i++)eeprom_write_byte((unsigned char *)Epos++, commands[i]);
		}
	};
	Working_Set w;
	void plot_interp(int Yinterp[],int x_true)
	{
		for (byte i = 0; i < N_curves; i++)
			for (byte j = 0; j < myGraph[i].nval - 1; j++)
				if (myGraph[i].x[j] < x_true&&x_true < myGraph[i].x[j + 1]) {
					 Yinterp[i]=(myGraph[i].y[j + 1] - myGraph[i].y[j]) / float(myGraph[i].x[j + 1] - myGraph[i].x[j])*(x_true - myGraph[i].x[j]) + myGraph[i].y[j];
				}
	}
/*
#define SPS_DD SPS_D
#define SPL_DD SPL_D
#define SP_DD SP_D
*/
#define SPS_DD(x) Serial.print(x);Serial.print(' ')
#define SPL_DD(x) Serial.println(x)
#define SP_DD(x) Serial.print(x)
#define SHIFT_PLOT_MM 1
	
	bool plot_sequence()
	{
		int y[40];// = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
		int prec_time[40];// = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
		for (byte i = 0; i < 40; i++) { y[i] = 0; prec_time[i] = 0; }
		for (byte ic = 0; ic < N_OS_STA; ic++) {
			eeprom_read_block(&pd[ic], (void*)(PD_EEPROM_POS + ic*PD_SIZE), PD_SIZE);
		}
		sequn = eeprom_read_byte(SEQ_EE_START);
		Serial.println(sequn,DEC);
		if (sequn == 0)return false;
		eeprom_read_block(&seq, (void *)(SEQ_EE_START + 1), MAXSEQ*sizeof(sequence));
		int day = PLOT_START_DAY;
		DateTime oggi = adesso();

		eeprom_read_block(&w, (void*)(EEPOS_COMMAND), EESIZE_W);
		SPS_DD(w.commands); SPS_DD("NC"); SPS_DD(w.N_curves);
		for (byte i = 0; i < w.N_curves; i++) {
			SPS_DD(w.plotStartValue[i]);// SPS_DD(CurvesN[i]);
		}

#ifdef newCode  //TO BE TESTED plot continue previous plot???
		//__________________________________shift days previous plot_________________________________
		byte shift = -PLOT_START_DAY - 1;
		day = w.plotStartTime - oggi.unixtime() / 60.;
		if (oggi.unixtime()/60>plot_start_time+shift*1440) {
			//if (w.plotStartTime < oggi.unixtime() + day*SECS_PER_DAY) {
			plot_start_time +=shift*1440;
			for (byte i = 0; i < w.N_curves; i++) {
				y[CurvesN[i]] = w.plotStartValue[i]; SPS_DD(CurvesN[i]); SPS_DD(y[CurvesN[i]]);
		}
			loadGraph(w.plotStarTime-oggi.unixtime()/60.,y[i],)
		else
#endif
		plot_start_time = oggi.unixtime() / 60;
		//if prog.check_day_match(oggi)
		//int prev_time = 0; 
		byte y00 = 0;
		
		while (day++ < MAX_PLOT_DAYS) {
			SPS_DD(day);
			// compute using recorded ETo values
			float day_penman = month_penman[oggi.month()-1] / 10.;
#ifndef NO_ET0
			if (day < 0) {
				Weather Wu;
				SPS_DD("+");
				Wu.read(oggi.unixtime() + (day * 24 * 3600L));
				SPS_DD("/");
				if (Wu.penman < 5 && Wu.penman >= 0)
					day_penman = Wu.penman; // = read_penman(oggi.operator+(day * 24 * 3600));

			}
			SP_DD("ETo"); SPL_DD(day_penman);
#else			
			float dayrain = 0;
			if (day < 0) {
				SP_D("EEPOS"); SPL_D((ET_POS + (now() % MYSECS_PER_YEAR+43600L) / SECS_PER_DAY + day));
				day_penman = (float(eeprom_read_byte((byte *)(ET_POS + (now() % MYSECS_PER_YEAR+43600L) / SECS_PER_DAY + day))) / 40.);
				dayrain = r.get((now() % MYSECS_PER_YEAR+43600L) / SECS_PER_DAY + day);
			}
			SP_D(dayrain); SP_D(" ");		SPL_D(day_penman);
			if (day_penman > 6.||day_penman==0) day_penman = month_penman[oggi.month()] / 10.;
#endif
			float kappaC = 1;
			SP_D("T");	SPL_D(oggi.unixtime() + day * 24 * 3600);
			for (int i = 1; i < sequn + 1; i++)
			{
				if (y[seq[i].valv] == 0)y[seq[i].valv] = SHIFT_PLOT_MM * y00++;
				int y0 = 0;
		       
				if( oggi.unixtime()+(seq[i].start + day * 24 * 3600)>cD.rain_delay[seq[i].valv/10]&& //interval is after rain _delay period
				seq[i].Check_day_match(oggi.operator+(day * 24 * 3600))){

					if (prec_time[seq[i].valv] < day * 1440)  //________last interval_____previous day
					{//
						//SP_DD("Match d."); SP_DD(day); SP_DD(" s."); SPL_DD(i);
						int dayStart = seq[i].start + day * 1440;
						//if (pd[seq[i].valv / 10].Kc[seq[i].valv % 10] > 10)kappaC = pd[seq[i].valv / 10].Kc[seq[i].valv % 10]/100;
					//_________________________dcompute beginning of the day MM variation_____draw segment up to irrigation start___________					
						y[seq[i].valv] = y[seq[i].valv] - (dayStart - day * 1440)*(kappaC*day_penman - dayrain) / 14.40;
						loadGraph(seq[i].valv, dayStart, y[seq[i].valv] + y0);
						//_____________________draw irrigation interval______________________________________________________________________
						SP_DD(prec_time[seq[i].valv]); SPS_DD(seq[i].valv); SPS_DD(dayStart); SP_DD(" "); SP_DD(y[seq[i].valv] + y0);
						if (pd[seq[i].valv / 10].area[seq[i].valv % 10] > 0)
							y[seq[i].valv] = y[seq[i].valv] + seq[i].flux *20.*seq[i].dur / 36.00 / pd[seq[i].valv / 10].area[seq[i].valv % 10];
						else
							y[seq[i].valv] = y[seq[i].valv] + 100;
						loadGraph(seq[i].valv, dayStart + int(seq[i].dur / 60), y[seq[i].valv] + y0);
						//____________________________compute rest of the day mm variation_____________________________________________________________
						y[seq[i].valv] = y[seq[i].valv] - (1440 - dayStart + day * 1440)*(kappaC*day_penman - dayrain) / 14.40;
						SPS_DD(dayStart + int(seq[i].dur / 60)); SP_DD(" "); SPL_DD(y[seq[i].valv] + y0);
						prec_time[seq[i].valv] = dayStart + int(seq[i].dur / 60);
					}
					else //__________________________PREVIOUS INTERVAL SAME DAY_________________________________________
					{//_________________________dcompute  MM variation_____draw segment up to irrigation start___________
						int dayStart = seq[i].start + day * 1440;
						y[seq[i].valv] = y[seq[i].valv] - (-(1440  + day * 1440) + dayStart )*(kappaC*day_penman - dayrain) / 14.40;
						loadGraph(seq[i].valv, dayStart, y[seq[i].valv] + y0);
						//_____________________draw irrigation interval______________________________________________________________________
						SP_DD(prec_time[seq[i].valv]); SPS_DD(seq[i].valv); SPS_DD(dayStart); SP_DD(" "); SP_DD(y[seq[i].valv] + y0);
						if (pd[seq[i].valv / 10].area[seq[i].valv % 10] > 0)
							y[seq[i].valv] = y[seq[i].valv] + seq[i].flux *20.*seq[i].dur / 36.00 / pd[seq[i].valv / 10].area[seq[i].valv % 10];
						else
							y[seq[i].valv] = y[seq[i].valv] + 100;
						loadGraph(seq[i].valv, dayStart + int(seq[i].dur / 60), y[seq[i].valv] + y0);
						//____________________________compute rest of the day mm variation_____________________________________________________________
						y[seq[i].valv] = y[seq[i].valv] - (1440 - dayStart + day * 1440)*(kappaC*day_penman - dayrain) / 14.40;
						SPS_DD(dayStart + int(seq[i].dur / 60)); SP_DD(" "); SPL_DD(y[seq[i].valv] + y0);
						prec_time[seq[i].valv] = dayStart + int(seq[i].dur / 60);
					}
			}
				else  //______________________compute day mm variation____________________________________________________
				{
					y[seq[i].valv] = y[seq[i].valv] - (1440)*(kappaC*day_penman - dayrain) / 14.40;
					//if no interval plot end of the day mm
					loadGraph(seq[i].valv, (day + 1) * 1440, y[seq[i].valv] + y0);
				}
#ifdef newCode
			if(day==-1)w.plotStartValue[CurvesN[seq[i].valv]]=y[seq[i].valv];-------------------------------------------store values at midnigth day=-1
#endif
			}
		}
		{
			// --------------- save computed water balance at 12:00 AM-------------------------------
			w.plotStartTime = oggi.unixtime() - oggi.hour() * 3600 - oggi.minute() * 60 - oggi.second();
			SPS_DD("Pl st"); SPS_DD(1440-oggi.hour() * 60 - oggi.minute());
			plot_interp(w.plotStartValue, (1440-oggi.hour() * 60 - oggi.minute()));
			w.N_curves = N_curves;
			for (byte i = 0; i < w.N_curves; i++) {
				SPS_DD(w.plotStartValue[i]);// SPS_DD(CurvesN[i]);
			}
			w.savePlot();
			SPL_DD();
		}
			for (byte j = 0; j < N_curves;j++)
			{
#ifdef STL_VECTOR
				myGraph[j].x.shrink_to_fit();
				myGraph[j].y.shrink_to_fit();
#endif
				for (byte i = 0; i < myGraph[j].nval; i++)

				{
					SPS_DD(myGraph[j].x[i]);
				}
				SPL_DD();
				for (byte i = 0; i < myGraph[j].nval; i++)
				{
					SPS_DD(myGraph[j].y[i]);
				}
				SPL_DD();
			
		}
		return true;
	}
#define Flux_Correction false
#define Valve_Correction 0
	byte iprec = 0,nextValv=0,nextIndex=0;

	int precTime = 0, nextSeqStart = 0, nextSeqEnd = 0;
	boolean startFlag = true;
	int check_sequence(uint16_t startf[], int durf[], int  fluxf[],int ff) //startime in 2s steps,duration in sec,flux in Lt/h
																	 // check sequences loaded from OS units versus water usage and 
																	 // check missing watering cycles and flux under or over target flux
	{
		//	sequence seq[MAXSEQ];
		sequn = eeprom_read_byte(SEQ_EE_START);
		eeprom_read_block(&seq, (void *)(SEQ_EE_START + 1), MAXSEQ * sizeof(sequence));
		long timeSpan = 0;
		int FluxDiff;
		byte fluxSteps[] = { 2,7,15,30,60 };
		DateTime ora = adesso();
		byte newFlux = 0;
		int valv = -1;
		byte	i = iprec;			// -------start from previous checked cycle
		SP_D("PrecT"); SPL_D(precTime);
		while (seq[i].start+seq[i].dur/60-1 <= ora.hour() * 60 + ora.minute() ||		//seq[i] e	before   actual time
			(ora.hour() * 60 + ora.minute() < precTime))			//seq[i] start     on previous day
		{
			FluxDiff = -10000;

			SP_D(seq[i].start / 60); SP_D(":"); SP_D(seq[i].start % 60); SP_D(" ");
			if (cD.rain_delay[seq[i].valv / 10] > ora.unixtime()) { SP_D("R_D"); }     //rain delay condition
			else
				if (!seq[i].Check_day_match(ora)) { SP_D("noDayMach"); }
	
				else
			{																		//cycle match day restrictions
				for (byte jf = 0; jf < ff; jf++) {
					FluxDiff = seq[i].Check_Flux(startf[jf], durf[jf], fluxf[jf]);	//cycle match in startime and duration

					if (FluxDiff!= -10000)break;
				}
				if (FluxDiff != -10000)											// it means interval match
				{
					 valv = seq[i].valv;
					 newFlux = seq[i].flux - FluxDiff / 20;
					if (seq[i].flux == 0) seq[i].flux = abs(-FluxDiff / 20);        //fisrt time : add flux info to seq
					else
					{
						byte fluxInd = 5;
						for (byte j = 0; j < 5; j++)
							if (seq[i].flux * fluxSteps[j] > abs(FluxDiff)) {
								fluxInd = j + 1; break;
							}
						if (fluxInd>4){
							char buff[30]; sprintf(buff, "Valve %d has flux difference of %d",seq[i].valv, FluxDiff);
							SendMessage(buff);
						}
						pd[seq[i].valv / 10].valveStatus[(seq[i].valv % 10)] = fluxInd;  //valveStatus contain fluxInd
			//		else
			//			pd[seq[i].valv / 10].valveStatus[(seq[i].valv % 10)] = NUM_FLUX_COLOR; //
						SP_D("fluxI"); SP_D(fluxInd); SPS_D((seq[i].valv % 10)); 
						SPS_D(seq[i].valv / 10); SPS_D(pd[seq[i].valv / 10].valveStatus[(seq[i].valv % 10)]);
					}
//---------------------recalculate zone water balance if is still 0 ----[mm]--------------------
//	if(pd[seq[i].valv / 10].area[seq[i].valv % 10]>0)	pd[seq[i].valv / 10].dummy[seq[i].valv % 10] += seq[i].flux * seq[i].dur / pd[seq[i].valv / 10].area[seq[i].valv % 10] / 180;
				pd[seq[i].valv / 10].dummy[seq[i].valv % 10] = newFlux;
//---------------------------------------------------------------------------
#ifdef IOT
// calculate water added computing mm before and after valve interval 
//			prevET =previuos ET or day ET? compute ET water reduction up to start time;
				byte in=load_mm(seq[i].valv, -prevET*(hour() * 60 + minute()) / 1440);   //   tobe mooved at begin of interval in INTERRUPT.ino
				ETcalc[in] = prevET*(hour() * 60 + minute()) / 1440;

//			newFlux or actual flux /area are mm added by irrigation				

				load_mm(seq[i].valv, newFlux / pd[seq[i].valv / 10].area[seq[i].valv % 10]);
				IOTv[seq[i].valv/10].updateThingSpeak(0, 0, 1);
				eeprom_write_block(&cD, (void *)CURRENT_DATA_POS, sizeof(CurrentData));
				
#endif
				
					SP("Seq.match p."); SP(seq[i].progIndex); SP(" v."); SP(seq[i].valv); SP(" fdif."); SP(FluxDiff - 1); SP("w"); SPL(pd[seq[i].valv / 10].dummy[seq[i].valv % 10]);
					//if(pd[seq[i].valv / 10].Status >0)pd[seq[i].valv / 10].Status =0;) //unit works! 
					eeprom_write_block((void *)&pd[seq[i].valv / 10],(void*)(PD_EEPROM_POS + int(seq[i].valv / 10) *PD_SIZE), PD_SIZE);
					//eeprom_read_block((void *)&pd[seq[i].valv / 10], (void*)(PD_EEPROM_POS + int(seq[i].valv / 10) *PD_SIZE), PD_SIZE);
					//SPS_D(pd[seq[i].valv / 10].valveStatus[(seq[i].valv % 10)]); SP_D(" read ");

					if (seq[i].valv==Valve_Correction&&Flux_Correction)seq[i].flux += FluxDiff / 2;
		//			iprec = i;
		//			break;
					precTime = ora.hour() * 60 + ora.minute();
				}
				else																	//FluxDiff==0 no interval match
					if (!startFlag) {											//---not fisrt time
	//---?				if (seq[i].start == nextSeqStart) 							//is expected start
						{
							pd[seq[i].valv / 10].dummy[seq[i].valv % 10] = 0;
							pd[seq[i].valv / 10].valveStatus[seq[i].valv % 10] = 5;    //5 red means expected watering not executed
							eeprom_write_block((void *)&pd[seq[i].valv / 10], (void*)(PD_EEPROM_POS + int(seq[i].valv / 10) *PD_SIZE), PD_SIZE);

	//---?					pd[seq[i].valv / 10].Status = pd[seq[i].valv / 10].Status * 10 + seq[i].valv % 10;
						}							//OS unit status=0 OK
					//		if (pd[seq[i].valv / 10].Status>=Alarm_Times)				//if more times valves are not working
			     	//			if(pd[seq[i].valv / 10].Status%10!=(pd[seq[i].valv / 10].Status/10)%10) //on different valves 			     				)				//if more times valves are not working
			     	//			
					//			if(!check_unit_ok(seq[i].valv / 10)) send_message(sprintf("UNIT %d Stopped",seq[i].valv / 10));
					//		else  // SAME VALVE
					//			send_message(sprintf("UNIT %d valve %d not working",seq[i].valv/10,seq[i].valv % 10unit));
					//   	on line? unit not working? send message
						
					}
				
			}
			
			i++;
			iprec = i;
			if (i >= sequn + 1){
				i = 1;
				precTime = 0;
				timeSpan += 24 * 3600;
		}
			
		}
	
		//	i++; 

		while ( !seq[i].Check_day_match(ora.operator+(timeSpan))|| cD.rain_delay[seq[i].valv / 10] >ora.unixtime()+timeSpan)
			{ i++; if (i >= sequn+1 ) { i = 1; timeSpan += 24 * 3600; } }
		nextSeqStart = seq[i].start;
		nextValv = seq[i].valv;
		nextIndex = i;
		nextSeqEnd = nextSeqStart + seq[i].dur / 60;
		SP_D("next S "); SP_D(nextSeqStart); SP_D(" next E "); SPL_D(nextSeqEnd);
		precTime = ora.hour() * 60 + ora.minute();
		startFlag = false;

		if (FluxDiff == -10000) {
			SPL(" no Seq match ");
			
			return -1;
		}
		else {
			//write sequence to eeprom
			eeprom_write_byte(SEQ_EE_START, sequn);
			eeprom_write_block((void *)&seq, (void *)(SEQ_EE_START + 1), MAXSEQ * sizeof(sequence));
			
			return valv;
		}
	}
bool program_rem_sequence(byte icontr, byte pid)
{
//	sequence seq[MAXSEQ];
	sequn = eeprom_read_byte(SEQ_EE_START);
	byte ij = 1;
	eeprom_read_block(&seq, (void *)(SEQ_EE_START + 1), MAXSEQ*sizeof(sequence));
	for (int i = 1; i < sequn+1; i++)
	{
		if (seq[i].progIndex != icontr * 16 + pid) {
			seq[ij++] = seq[i];
		}
	}
	SPL();
	SP("removed "); SPL(sequn-ij+1);

	sequn = ij-1;

	//write sequence to eeprom
	eeprom_write_byte(SEQ_EE_START, sequn);
	eeprom_write_block((void *)&seq, (void *)(SEQ_EE_START + 1),  MAXSEQ*sizeof(sequence));

}
#define SPSEQ for (byte i=1; i < sequn+1; i++) {SP_D(seq[i].start); SP_D(' ');}SPL_D();
bool program_put_sequence(byte icontr, byte pid) // put in sequence stack OS station icontr program n. Pid
{
	//read sequence from eeprom
//	sequence seq[MAXSEQ];
	sequn = eeprom_read_byte(SEQ_EE_START);
	eeprom_read_block(&seq, (void *)(SEQ_EE_START + 1), MAXSEQ*sizeof(sequence));
	SPSEQ
	SPL_D(' ');
	eeprom_read_block(EEindex, (void*)EE_INDEX_POS, 80);
	if (sequn == 0) seq[0].start = 0;
	SP_D("EE_index_read "); SPLF_D(EEindex[pid][icontr], DEC); 

	//	blPr((void *)&EEindex);
		//	byte pidd = 0;
		//	for (byte icontr = 0; icontr < Max_OS_number; icontr++)
		//		for (int pid = 0; pid < pd.nprograms; pid++)
	
		ProgramStruct prog;
 		eeprom_read_block(&prog, (void *)( EEindex[pid][icontr]), PROGRAMSTRUCT_SIZE);
		SP_D(prog.enabled); SP_D(prog.use_weather); SPL_D(prog.starttime_type);
		if (!prog.enabled) return false;
		// calculate progra start times
		int16_t start[8];
		int tspan = 0; for (int sid = 0; sid < pd[icontr].nboards * 8 ; sid++) tspan = tspan + prog.durations[sid]/60;
		uint start0 = starttime_decode(prog.starttimes[0]);   //decode startime using bit 13 14
		int16_t repeat = prog.starttimes[1];
		int16_t interval = prog.starttimes[2];
		//unsigned int current_minute = (t % 86400L) / 60;
		byte totalstart = 0;
		if (prog.starttime_type==1)
		{
			// given fixed start time type
		
			for (byte i = 0; i < MAX_NUM_STARTTIMES; i++)
			if(prog.starttimes[i]>0)
			{
				start0 = starttime_decode(prog.starttimes[i]);  // if curren_minute matches any of the given start time, return 1
				int interv = 0;
				int dstart = check_collision(prog.flags,prog.days, start0, icontr * 16 + pid, tspan);
				
				for (int sid = 0; sid < pd[icontr].nboards * 8 ; sid++)
					if (prog.durations[sid]>0)
					{
						SP_D('N'); SP_D(start0 + interv); SP_D('%');

						//------------------------------------find sequential location <k>

						for (byte k = sequn; k >= 0; k--)
							if ((start0 + interv) >= seq[k].start)
							{
							//	SP_D("k"); SP_D(k);
								//--------------------------insert new elemnent
								//////////////////////////////////////////////////////check overlapping////////////////////
							
//if ((start0+interv<1440&&(start0 + interv) >= seq[k].start)||
//(start0 + interv>1440 && (start0 + interv-1440) >= seq[k].start))
//{	//--------------------------insert new elemnent
/////////////////////////////////////////////////////////////////////////////////////////////
//if(start0+interv<1440)seq[k + 1].start = (start0 + interv);
//else seq[k + 1].start = (start0 + interv) - 1440; //going to next day ------------------check prog days??

							 	k++;
								//if (sequn > MAXSEQ) return false;
								seq[k].start = (start0 + interv);
								interv = prog.durations[sid]/60 + interv + pd[icontr].stations_delay_time/60;		//interval between valves act.
								seq[k].dur = prog.durations[sid];
								seq[k].valv = sid + icontr * 10;									//number of OS controller
								seq[k].day0 = prog.days[0];
								seq[k].day1 = prog.days[1];
								seq[k].flags = prog.flags;
								seq[k].progIndex = icontr * 16 + pid;
								seq[k].flux = 0;
								SPLF_D(k, DEC);
								SP_D("startTime "); SPL_D(seq[k].start);
								SP_D("dur "); SP_D(seq[k].dur); SP_D(" valv "); SPLF_D(seq[k].valv, DEC);
								SP_D("day0 "); SPLF_D(seq[k].day0, DEC);
								SP_D("day1 "); SPLF_D(seq[k].day1, DEC);
								SP_D(" index "); SPLF_D(seq[k].progIndex, DEC);
								//				seq.verif_overlap(k);
							
								break;
							}
							else
							{	//-----------------------------move elements up
								seq[k + 1] = seq[k];
								SP_D('>');	SP_D(seq[k + 1].start); SP_D(' ');
							}
						
						sequn++;
							}
			}
		}
		else {
			// repeating type start-------------------------------------------------------------------
			for (int i = 0; i < repeat+1; i++) {
				totalstart = i - 1;
				int dstart = check_collision(prog.flags,prog.days, start0, icontr * 16 + pid, tspan);
					if (dstart != 0&&repeat==0)
					{
						prog.starttimes[0] += dstart;
						Encode_jp(pid, icontr);
						prog.starttimes[0] -= dstart;
					}
					int interv = 0;
				 for (int sid = 0; sid < pd[icontr].nboards*8 ; sid++)
				 {
					 
					if (prog.durations[sid] > 0)
					{
						SP_D('N'); SP_D(start0 + interv); SP_D('|');
						//------------------------------------find sequential location <k>
						
						for (byte k = sequn; k >= 0; k--)

							if ((start0+interv<1440&&(start0 + interv) >= seq[k].start)||
								(start0 + interv>1440 && (start0 + interv-1440) >= seq[k].start))
							{	//--------------------------insert new elemnent

								
								/////////////////////////////////////////////////////////////////////////////////////////////
								if(start0+interv<1440)seq[k + 1].start = (start0 + interv);
								else seq[k + 1].start = (start0 + interv) - 1440; //going to next day ------------------check prog days??
								k++;
								interv = prog.durations[sid]/60 + interv + pd[icontr].stations_delay_time/60;		//interval between valves act.
								SPL_D(); SP_D("startTime "); SPL_D(seq[k].start);
								seq[k].dur = prog.durations[sid];
								seq[k].valv = sid + icontr * 10;									//number of OS controller
								seq[k].day0 = prog.days[0];
								seq[k].day1 = prog.days[1];
								seq[k].flags = prog.flags;
								seq[k].progIndex = icontr * 16 + pid;
								seq[k].flux = 0;
								SPLF_D(k, DEC);
								SP_D("dur "); SP_D(seq[k].dur);
								SP_D(" valv "); SPLF_D(seq[k].valv, DEC);
								SP_D("day0 "); SPLF_D(seq[k].day0, BIN);
								SP_D("day1 "); SPLF_D(seq[k].day1, DEC);
								SP_D(" index "); SPLF_D(seq[k].progIndex, DEC);
								break;

							}
							else
							{	//-----------------------------move elements up
								seq[k + 1] = seq[k];
								SP_D('<');	SP_D(seq[k + 1].start); SP_D(' ');
							}
						sequn++;
						SPL_D("sequn+");

						//				seq.verif_overlap(k);
					}
				
				}
				 start0 = start0 + interval;
				 if (start0 > 1440)start0 = start0 - 1440;
			}
		}
		SPSEQ
		//write sequence to eeprom
		eeprom_write_byte(SEQ_EE_START,sequn);
		eeprom_write_block((void *)&seq, (void *)(SEQ_EE_START + 1), MAXSEQ*sizeof(sequence));
		return true;
}
byte k_macth(byte flags,byte days[], int i)
{
	int sign = i / abs(i);
	i = abs(i);
	while (!day_match(flags,days, i)&&i>=1&&i<=sequn+1) i =i+ sign;
	return i;

}

bool day_match(byte flags,byte days[], byte k)
{
//	SP('?'); SP(days[0]); SP('?'); SP(seq[k].day0); SP(' '); SP(days[0] & seq[k].day0);
	if (((flags & 0B0110000) >> 4 == 0) && ((seq[k].flags & 0B0110000) >> 4 == 0)     )            //weekday
	{
		if ((days[0] & seq[k].day0 )!= 0)
		{
			SP_D(" m "); SP_D(int(k));
			return true;
		}                    //week days match
		else
			return false;
	}
	else
	
	if (((flags & 0B0110000) >> 4 == 3) && ((seq[k].flags & 0B0110000) >> 4 == 3))
		//         both days interval--------------------
		
		{
			if (days[1] != seq[k].day1)  //different intervals
			{
			//	SP_D("m-"); SP_D(int(k));
				return true;   //different interval will overlap

			}
			else                          //same interval  
				if (days[0] == seq[k].day0)      //same next day
				{
					SP_D(" M "); SP_D(int(k));
					return true;                    //run same day
				}
				return false;
		}//all other cases----------------
	return true;
}
int check_collision(byte flags,byte days[], int start0, byte thisprog, int tspan)
{/////////////////////////////////////////////////////check overlapping////////////////////
/*	byte days[2] = { seq[k].day0,seq[k].day1 };
	int start0;
	int thisprog = seq[k].progIndex;
	int tspan = 0; for (byte jj = 1; jj < sequn+1; jj++)if (seq[jj].progIndex == thisprog)tspan = tspan+seq[jj].dur/60;
	SP(" tspan"); SP(tspan);*/
	//find location and overlap to start check

	byte k1=0,k2=0;
	
	for (byte i = 1; i < sequn+1; i++)
	{
		if (day_match(flags,days, i) && seq[i].start + seq[i].dur / 60>=start0&&seq[i].start <= start0)                  //|++++++|---|----|
		{
			if (k1 == 0)k1 = i; if (k2<i)k2 = i;
		}
		if (day_match(flags,days, i) && seq[i].start + seq[i].dur / 60 >= start0 + tspan&&seq[i].start <= start0 + tspan) //|------|+++|++++|
		{
			if (k1 == 0)k1 = i; if (k2 < i)k2 = i;
		}
		if (day_match(flags,days, i) && seq[i].start + seq[i].dur / 60 <= start0 + tspan&&seq[i].start >= start0)    // |------|+++++|------|
		{
			if (k1 == 0)k1 = i; if (k2 < i)k2 = i;
		}

		if (day_match(flags,days, i) && seq[i].start + seq[i].dur / 60 <= start0 + tspan&&seq[i].start >= start0)    // |++++|-----|++++++|
		{
			if (k1 == 0)k1 = i; if (k2 < i)k2 = i;
		}

	}
	if (k1 == 0)
	{			//if no everlap return
		SP_D(" OK ");
		return 0;
	}
	SP("Check prog."); SPL(thisprog);
	// sum of durations of all zones
	int starttime_correction = 0;
	byte av_span = 0;
	byte lower_bound = k_macth(flags,days, -k1);
	byte upper_bound = k_macth(flags,days, k2);
	av_span = seq[upper_bound].start - (seq[lower_bound].start + seq[lower_bound].dur / 60);
	SPL_D(); SP_D(lower_bound); SP_D(' '); SP_D(upper_bound); SP_D(" avspan"); SP_D(av_span);
	if (av_span > tspan)    //---------------
	{
		if (start0 < seq[lower_bound].start + seq[lower_bound].dur / 60)starttime_correction = -start0 + (seq[lower_bound].start + seq[lower_bound].dur / 60);
		if (start0 + tspan > seq[upper_bound].start)starttime_correction = -start0 - tspan + (seq[upper_bound].start);
		SP(" adj. collision interval moving");  SP(" min "); SPL(starttime_correction);
		char buff[30]; sprintf(buff, "Move program. %d, %d minute to avoid collision ",thisprog,starttime_correction);
		SendMessage(buff);
		return starttime_correction;
	}
	else          //-------------------------check other intervals
	{  //above-----
		while (av_span < tspan&&upper_bound < sequn)
		{
			lower_bound = upper_bound;
			upper_bound = k_macth(flags,days, upper_bound);
			if (upper_bound == lower_bound)break;
			av_span = seq[upper_bound].start - (seq[lower_bound].start + seq[lower_bound].dur / 60);
		}
		if (av_span>tspan)
		{
			starttime_correction = (seq[lower_bound].start + seq[lower_bound].dur / 60) - start0;
			SP(" adj. collision interval moving");  SPL(starttime_correction);

			return starttime_correction;
		}
		else
		{
			while (av_span < tspan&&lower_bound>1)
			{
				upper_bound = lower_bound;
				lower_bound = k_macth(flags,days, -lower_bound);
				if (upper_bound == lower_bound)break;
				av_span = seq[upper_bound].start - (seq[lower_bound].start + seq[lower_bound].dur / 60);
			}
			if (av_span>tspan)
			{
				starttime_correction = (seq[lower_bound].start + seq[lower_bound].dur / 60) - start0;
				SP(" adj. collision interval moving");  SPL(starttime_correction);

				return starttime_correction;
			}
			else
			{
				SP("no solution found req. interv."); SPL(tspan);
				return  -10000;
			}
		}

	}
}
