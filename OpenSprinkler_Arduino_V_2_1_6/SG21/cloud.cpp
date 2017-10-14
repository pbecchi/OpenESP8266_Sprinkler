
// ==========================================
// ====== PUSH NOTIFICATION FUNCTIONS =======
// ==========================================
void ip2string(char* str, byte ip[4]) {
	for (byte i = 0; i<4; i++) {
		itoa(ip[i], str + strlen(str), 10);
		if (i != 3) strcat(str, ".");
	}
}

void push_message(byte type, uint32_t lval, float fval, const char* sval) {

#if !defined(ARDUINO) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284__)

	static const char* server = DEFAULT_IFTTT_URL;
	static char key[IFTTT_KEY_MAXSIZE];
	static char postval[TMP_BUFFER_SIZE];

	//@tcs: return when no internet connection
	if (os.status.network_fails != 0) return; //do not post if network failed

											  // check if this type of event is enabled for push notification
	if ((os.options[OPTION_IFTTT_ENABLE] & type) == 0) return;
	key[0] = 0;
	read_from_file(ifkey_filename, key);
	key[IFTTT_KEY_MAXSIZE - 1] = 0;

	if (strlen(key) == 0) return;

#if defined(ARDUINO)
	uint16_t _port = ether.hisport; // make a copy of the original port
	ether.hisport = 80;
#endif

	strcpy_P(postval, PSTR("{\"value1\":\""));

	switch (type) {

	case IFTTT_STATION_RUN:

		strcat_P(postval, PSTR("Station "));
		os.get_station_name(lval, postval + strlen(postval));
		strcat_P(postval, PSTR(" closed. It ran for "));
		itoa((int)fval / 60, postval + strlen(postval), 10);
		strcat_P(postval, PSTR(" minutes "));
		itoa((int)fval % 60, postval + strlen(postval), 10);
		strcat_P(postval, PSTR(" seconds."));
		if (os.options[OPTION_FSENSOR_TYPE] == SENSOR_TYPE_FLOW) {
			strcat_P(postval, PSTR(" Flow rate: "));
#if defined(ARDUINO)
			dtostrf(sensors.realtime_GPM, 5, 2, postval + strlen(postval));
#else
			sprintf(tmp_buffer + strlen(tmp_buffer), "%5.2f", flow_last_gpm);
#endif
		}
		break;

	case IFTTT_PROGRAM_SCHED:

		if (sval) strcat_P(postval, PSTR("Manually scheduled "));
		else strcat_P(postval, PSTR("Automatically scheduled "));
		strcat_P(postval, PSTR("Program "));
		{
			ProgramStruct prog;
			pd.read(lval, &prog);
			if (lval<pd.nprograms) strcat(postval, prog.name);
		}
		strcat_P(postval, PSTR(" with "));
		itoa((int)fval, postval + strlen(postval), 10);
		strcat_P(postval, PSTR("% water level."));
		break;

	case IFTTT_RAINSENSOR:

		strcat_P(postval, (lval == LOGDATA_RAINDELAY) ? PSTR("Rain delay ") : PSTR("Rain sensor "));
		strcat_P(postval, ((int)fval) ? PSTR("activated.") : PSTR("de-activated"));

		break;

	case IFTTT_FLOWSENSOR:
		strcat_P(postval, PSTR("Flow count: "));
		itoa(lval, postval + strlen(postval), 10);
		strcat_P(postval, PSTR(", volume: "));
		{
			uint32_t volume = os.options[OPTION_PULSE_RATE_1];
			volume = (volume << 8) + os.options[OPTION_PULSE_RATE_0];
			volume = lval*volume;
			itoa(volume / 100, postval + strlen(postval), 10);
			strcat(postval, ".");
			itoa(volume % 100, postval + strlen(postval), 10);
		}
		break;

	case IFTTT_WEATHER_UPDATE:
		if (lval>0) {
			strcat_P(postval, PSTR("External IP updated: "));
			byte ip[4] = { (lval >> 24) & 0xFF,(lval >> 16) & 0xFF,(lval >> 8) & 0xFF,lval & 0xFF };
			ip2string(postval, ip);
		}
		if (fval >= 0) {
			strcat_P(postval, PSTR("Water level updated: "));
			itoa((int)fval, postval + strlen(postval), 10);
			strcat_P(postval, PSTR("%."));
		}

		break;

	case IFTTT_REBOOT:
#if defined(ARDUINO)
		strcat_P(postval, PSTR("Rebooted. Device IP: "));
		ip2string(postval, ether.myip);
		strcat(postval, ":");
		itoa(_port, postval + strlen(postval), 10);
#else
		strcat_P(postval, PSTR("Process restarted."));
#endif
		break;
	}

	strcat_P(postval, PSTR("\"}"));

	DEBUG_PRINT("THE IFTTT MESSAGE:  ");
	DEBUG_PRINTLN(postval);

#if defined(ARDUINO)

	if (!ether.dnsLookup(server, true)) {
		// if DNS lookup fails, use default IP
		ether.hisip[0] = 54;
		ether.hisip[1] = 172;
		ether.hisip[2] = 244;
		ether.hisip[3] = 116;
	}

	ether.httpPostVar(PSTR("/trigger/sprinkler/with/key/"), PSTR(DEFAULT_IFTTT_URL), key, postval, httpget_callback);
	for (int l = 0; l<100; l++)  ether.packetLoop(ether.packetReceive());
	ether.hisport = _port;

#else

	EthernetClient client;
	struct hostent *host;

	host = gethostbyname(server);
	if (!host) {
		DEBUG_PRINT("can't resolve http station - ");
		DEBUG_PRINTLN(server);
		return;
	}

	if (!client.connect((uint8_t*)host->h_addr, 80)) {
		client.stop();
		return;
	}

	char postBuffer[1500];
	sprintf(postBuffer, "POST /trigger/sprinkler/with/key/%s HTTP/1.0\r\n"
		"Host: %s\r\n"
		"Accept: */*\r\n"
		"Content-Length: %d\r\n"
		"Content-Type: application/json\r\n"
		"\r\n%s", key, host->h_name, strlen(postval), postval);
	client.write((uint8_t *)postBuffer, strlen(postBuffer));

	bzero(ether_buffer, ETHER_BUFFER_SIZE);

	time_t timeout = now() + 5; // 5 seconds timeout
	while (now() < timeout) {
		int len = client.read((uint8_t *)ether_buffer, ETHER_BUFFER_SIZE);
		if (len <= 0) {
			if (!client.connected())
				break;
			else
				continue;
		}
		httpget_callback(0, 0, ETHER_BUFFER_SIZE);
	}

	client.stop();

#endif

#endif
}


//***********************************************
//****** CLOUD communication 
//***********************************************

void cloud_json_stations_attrib(const char* name, int addr, char* postval_pt)
{
  DEBUG_PRINTLN(String("NAME:  ")+ String(name));
	
  strcat_P(postval_pt, PSTR("\""));
  strcpy(tmp_buffer, name);
  strcat(postval_pt, tmp_buffer);
  strcat_P(postval_pt, PSTR("\":["));
  
  byte *attrib = (byte*)tmp_buffer;
  os.station_attrib_bits_load(addr, attrib);
  for(byte i=0;i<os.nboards;i++) {
	  itoa(attrib[i], tmp_buffer, 10);
	  strcat(postval_pt, tmp_buffer);
      if(i!=os.nboards-1)
 	   strcat_P(postval_pt, PSTR(","));
    }
	strcat_P(postval_pt, PSTR("],"));
}


byte push_message_cloud(byte type, ulong day) {

/*
******** PARAMETERS
type: kind of packet sending
day:	SEND_CLOUD_LOG : if day==0:today, if 367>day>0 last days history if day > 366 the filename (unix day)


*****************  EXAMPLES

cloud server messages
push_message_cloud(SEND_CLOUD_OPTIONS, 0); //Send options to cloud server

************** Cloud Message Type *
#define SEND_CLOUD_OPTIONS		0
#define SEND_CLOUD_SETTINGS		1
#define SEND_CLOUD_PROGRAMS		2
#define SEND_CLOUD_STATIONS		3
#define SEND_CLOUD_STATUS_SPEC	4
#define SEND_CLOUD_LOG			5

Received cloud server response and commands:
{"result":x} x=1 means "success", see API 217 1st page and Tables_SG20 page API_SG20.
change password
change options, variables(settings), etc.
process by void handle_web_request(char *p)

*/

const char cloud_message_id[] PROGMEM =
"rjo\0"	//options	/jo
"rjc\0"	//settings	/jc
"rjp\0"	//programs	/jp
"rjn\0"	//station	/jn
"rjs\0"	//station status and special station data	/js, /je
"rjl\0";	//log records	/jl


#if !defined(ARDUINO) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284__)

DEBUG_PRINT("freeRam1: ");
DEBUG_PRINTLN(freeRam());

	if(os.status.network_fails != 0) return 4; //do not post if network failed

	static const char* server = DEFAULT_CLOUD_URL;
	static char postval[POST_BUFFER_SIZE] = {'\0'}; //TMP_BUFFER_SIZE is too small!
	char buf[] = {0,0,0,0,0,0,0,0,0,0,0,0,'\0'};
	byte b, ret_val=0;
		
DEBUG_PRINT("freeRam2: ");
DEBUG_PRINTLN(freeRam());

	millis_cnt = millis();


// preparing http POST sendings to cloud server
// sending one by one jc, jo jp, jn, js packets
// plus the logrecords in a packet

// message identifier based on package contenttype
	strcpy_P(postval, PSTR("{\"mid\":\""));

	switch(type){
		case SEND_CLOUD_OPTIONS :
			strcpy(tmp_buffer, "rjo\0");
			break;
		case SEND_CLOUD_SETTINGS:
			strcpy(tmp_buffer, "rjc\0");
			break;
		case SEND_CLOUD_PROGRAMS:
			strcpy(tmp_buffer, "rjp\0");
			break;
		case SEND_CLOUD_STATIONS:
			strcpy(tmp_buffer, "rjn\0");
			break;
		case SEND_CLOUD_STATUS_SPEC:
			strcpy(tmp_buffer, "rjs\0");
			break;
		case SEND_CLOUD_LOG:
			strcpy(tmp_buffer, "rjl\0");
			break;
			
	}	
/*			DEBUG_PRINT("type / tmp:   ");
			DEBUG_PRINT(type);
			DEBUG_PRINT(" / ");
			DEBUG_PRINT(tmp_buffer);
			DEBUG_PRINTLN();
*/
			
//puts the password and the MAC adress
//the password have to be scrambled (ToDo)		
			strcat(postval,tmp_buffer);
			sprintf(tmp_buffer, "\",\"lpw\":%s,\"macad\":\"",client_pw );
			strcat(postval, tmp_buffer);

			for (byte i = 0; i<6; i++) {
				b = ether.mymac[i] >> 4;
				buf[2*i]= (b<10) ? (b + 48) : (b + 55);
				b = ether.mymac[i] & 0x0F;
				buf[2*i+1] = (b<10) ? (b + 48) : (b + 55);
			}
			strcat(postval, buf);
			strcat_P(postval, PSTR("\","));

// end of message header

	switch (type){
		case SEND_CLOUD_LOG: {
			if(!day)	day = os.now_tz() / 86400L;

			strcat_P(postval, PSTR("["));
			itoa(day, tmp_buffer, 10);	   
			make_logfile_name(tmp_buffer);
			SdFile file;
			DEBUG_PRINTLN(tmp_buffer);

			  #if defined(ARDUINO)  // prepare to open log file for Arduino
				   if (!sd.exists(tmp_buffer)){
					   last_sent_log = 0;
					   ret_val = 3; //filename error don't send postval
					   break;
				   }
				   file.open(tmp_buffer, O_READ);
			  #endif // prepare to open log file

				bool comma = 0;
				ulong rec_cnt=0, temp_cnt=0;
			   int res;
			   while(true) {
				  #if defined(ARDUINO)
				   res = file.fgets(tmp_buffer, TMP_BUFFER_SIZE);
				   if (res <= 0) {
					   file.close();
					   ret_val = 2; //end of file, send postval
					   last_sent_log = 0;					   
					   break;
				   }
				  #endif
					tmp_buffer[TMP_BUFFER_SIZE-1]=0; // make sure the search will end
		   
				   // if this is the first record, do not print comma					   
				  rec_cnt++;
					if(rec_cnt > last_sent_log){  //this is the first record to send
						if (comma)
					   		strcat_P(postval,PSTR(","));
						else {
							comma=1;
							temp_cnt = rec_cnt;
							}
							
						if(strlen(postval) + strlen(tmp_buffer) + 3  > POST_BUFFER_SIZE){  //will fit to postval?
					   			file.close();
								ret_val = 1; //postval is full, send it, the file not finished
								break;  
						}
				   		strcat(postval,tmp_buffer);
						   
						last_sent_log++;
					}
			    }
			
			strcat_P(postval, PSTR("]}"));
		}
 			break;

		case SEND_CLOUD_OPTIONS:	// based on server_json_options_main() in server.cpp
		{
			
			byte oid;
			  for(oid=0;oid<NUM_OPTIONS;oid++) {
			#if !defined(ARDUINO) // do not send the following parameters for non-Arduino platforms
				if (oid==OPTION_USE_NTP     || oid==OPTION_USE_DHCP    ||
					oid==OPTION_STATIC_IP1  || oid==OPTION_STATIC_IP2  || oid==OPTION_STATIC_IP3  || oid==OPTION_STATIC_IP4  ||
					oid==OPTION_GATEWAY_IP1 || oid==OPTION_GATEWAY_IP2 || oid==OPTION_GATEWAY_IP3 || oid==OPTION_GATEWAY_IP4)
					continue;
			#endif
				int32_t v=os.options[oid];
				if (oid==OPTION_MASTER_OFF_ADJ || oid==OPTION_MASTER_OFF_ADJ_2 ||
					oid==OPTION_MASTER_ON_ADJ  || oid==OPTION_MASTER_ON_ADJ_2 ||
					oid==OPTION_STATION_DELAY_TIME) {
				  v=water_time_decode_signed(v);
				}
			#if defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284__)
				if (oid==OPTION_BOOST_TIME) {
				  if (os.hw_type==HW_TYPE_AC) continue;
				  else v<<=2;
				}
			#else
				if (oid==OPTION_BOOST_TIME) continue;
			#endif

				if (oid==OPTION_SEQUENTIAL_RETIRED) continue;
				if (oid==OPTION_DEVICE_ID && os.status.has_hwmac) continue; // do not send DEVICE ID if hardware MAC exists
   
			#if defined(ARDUINO) 
				if (os.lcd.type() == LCD_I2C) {
				  // for I2C type LCD, we can't adjust contrast or backlight
				  if(oid==OPTION_LCD_CONTRAST || oid==OPTION_LCD_BACKLIGHT) continue;
				}
			#endif

				// each json name takes 5 characters
				strncpy_P0(tmp_buffer, op_json_names+oid*5, 5);
				strcat_P(postval,PSTR("\""));
				strcat(postval,tmp_buffer);
				strcat_P(postval,PSTR("\":"));
	
				itoa( v, tmp_buffer, 10);
				strcat(postval, tmp_buffer);
		
				if(oid!=NUM_OPTIONS-1){
				  strcat_P(postval,PSTR(","));
				}
			  }
				strcat_P(postval,PSTR(",\"dexp\":"));
				v=os.detect_exp();			  
				itoa( v, tmp_buffer, 10);
				strcat(postval, tmp_buffer);
				
				strcat_P(postval,PSTR(",\"mexp\":"));
				itoa( MAX_EXT_BOARDS, tmp_buffer, 10);
				strcat(postval, tmp_buffer);
				
				strcat_P(postval,PSTR(",\"hwt\":"));
				itoa( os.hw_type, tmp_buffer, 10);
				strcat(postval, tmp_buffer);
				
				//TODO: append alm_cnt alarm counter
				
				strcat_P(postval,PSTR("}"));
		}
			break;
			
		case SEND_CLOUD_PROGRAMS:  //based on server_json_programs_main() in server.cpp
		{
			// bfill.emit_p(PSTR("\"nprogs\":$D,\"nboards\":$D,\"mnp\":$D,\"mnst\":$D,\"pnsize\":$D,\"pd\":["),
			//      pd.nprograms, os.nboards, MAX_NUMBER_PROGRAMS, MAX_NUM_STARTTIMES, PROGRAM_NAME_SIZE);

				strcat_P(postval,PSTR("\"nprog\":"));
				itoa( pd.nprograms, tmp_buffer, 10);
				strcat(postval, tmp_buffer);
				
				strcat_P(postval,PSTR(",\"nboards\":"));
				itoa(  os.nboards, tmp_buffer, 10);
				strcat(postval, tmp_buffer);
				
				strcat_P(postval,PSTR(",\"mnp\":"));
				itoa( MAX_NUMBER_PROGRAMS, tmp_buffer, 10);
				strcat(postval, tmp_buffer);
				
				strcat_P(postval,PSTR(",\"mnst\":"));
				itoa( MAX_NUM_STARTTIMES, tmp_buffer, 10);
				strcat(postval, tmp_buffer);
				
				strcat_P(postval,PSTR(",\"pnsize\":"));
				itoa( PROGRAM_NAME_SIZE, tmp_buffer, 10);
				strcat(postval, tmp_buffer);
				
				strcat_P(postval,PSTR(",\"pd\":["));

				byte pid, i;
				ProgramStruct prog;
				for(pid=0;pid<pd.nprograms;pid++) {
					pd.read(pid, &prog);
					if (prog.type == PROGRAM_TYPE_INTERVAL && prog.days[1] > 1) {
						pd.drem_to_relative(prog.days);
					}
					byte flag_0 = *(char*)(&prog);		//read program structure binary attributes (called flag) value
					byte soil_flags = *((char*)(&prog)+1); //read soil sensor program attach flag value
				
					//bfill.emit_p(PSTR("[$D,$D,$D,$D,["), flag_0, soil_flags, prog.days[0], prog.days[1]
					//sprintf(tmp_buffer,"[%d,%d,%d,%d,[", flag_0, soil_flags, prog.days[0], prog.days[1]);
					//strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR("["));
					itoa( flag_0, tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(","));
					itoa( soil_flags, tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(","));
					itoa( prog.days[0], tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(","));
					itoa(prog.days[1], tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(",["));
					
					// start times data
					for (i=0;i<(MAX_NUM_STARTTIMES-1);i++) {
						//bfill.emit_p(PSTR("$D,"), prog.starttimes[i]);
						//sprintf(tmp_buffer,"%d,", prog.starttimes[i]);
						//strcat(postval, tmp_buffer);						
						itoa(prog.starttimes[i], tmp_buffer, 10);
						strcat(postval, tmp_buffer);
						strcat_P(postval,PSTR(","));					  
					
					}
					//bfill.emit_p(PSTR("$D],["), prog.starttimes[i]);  // this is the last element
					//sprintf(tmp_buffer,"%d],[", prog.starttimes[i]);  // this is the last element
					//strcat(postval, tmp_buffer);
					itoa(prog.starttimes[i], tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR("],["));					  
					
					// station water time
					for (i=0; i<os.nstations-1; i++) {
						//bfill.emit_p(PSTR("$L,"),(unsigned long)prog.durations[i]);
						//sprintf(tmp_buffer,"$lu,",(unsigned long)prog.durations[i]);
						//strcat(postval, tmp_buffer);
						itoa((unsigned long)prog.durations[i], tmp_buffer, 10);
						strcat(postval, tmp_buffer);
						strcat_P(postval,PSTR(","));					  
					
					}
					//bfill.emit_p(PSTR("$L],\""),(unsigned long)prog.durations[i]); // this is the last element
					//sprintf(tmp_buffer,"%lu],\"",(unsigned long)prog.durations[i]); // this is the last element
					//strcat(postval, tmp_buffer);
					itoa((unsigned long)prog.durations[i], tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR("],\""));					  

					// program name
					strncpy(tmp_buffer, prog.name, PROGRAM_NAME_SIZE);
					tmp_buffer[PROGRAM_NAME_SIZE] = 0;  // make sure the string ends
					//bfill.emit_p(PSTR("$S"), tmp_buffer);
					strcat(postval, tmp_buffer);
					if(pid!=pd.nprograms-1) {
						//bfill.emit_p(PSTR("\"],"));
						strcat_P(postval,PSTR("\"],"));					  
					} else {
						//bfill.emit_p(PSTR("\"]"));
						strcat_P(postval,PSTR("\"]"));
					}
			  }
			strcat_P(postval,PSTR("]}"));
		}
			break;
			
		case SEND_CLOUD_STATIONS:	//see server_json_stations_main() in server.cpp
		{	cloud_json_stations_attrib(PSTR("masop"), ADDR_NVM_MAS_OP, postval);
			cloud_json_stations_attrib(PSTR("ignore_rain"), ADDR_NVM_IGNRAIN, postval);
			cloud_json_stations_attrib(PSTR("masop2"), ADDR_NVM_MAS_OP_2, postval);
			cloud_json_stations_attrib(PSTR("stn_dis"), ADDR_NVM_STNDISABLE, postval);
			cloud_json_stations_attrib(PSTR("stn_seq"), ADDR_NVM_STNSEQ, postval);
			cloud_json_stations_attrib(PSTR("stn_as1"), ADDR_NVM_SSENSOR_1, postval);  // attach soil sensor1 status on station
			cloud_json_stations_attrib(PSTR("stn_as2"), ADDR_NVM_SSENSOR_2, postval);  // attach soil sensor2 status on station
			cloud_json_stations_attrib(PSTR("stn_spe"), ADDR_NVM_STNSPE, postval);
/**/			
			//bfill.emit_p(PSTR("\"snames\":["));
			strcat_P(postval,PSTR("\"snames\":["));
			
			byte sid;
			for(sid=0;sid<os.nstations;sid++) {
			os.get_station_name(sid, tmp_buffer);
			
			//bfill.emit_p(PSTR("\"$S\""), tmp_buffer);
			strcat_P(postval, PSTR("\""));
			strcat(postval, tmp_buffer);
			strcat_P(postval, PSTR("\""));
			
			if(sid!=os.nstations-1)
				//bfill.emit_p(PSTR(","));
				strcat_P(postval, PSTR(","));
				
			}
			//bfill.emit_p(PSTR("],\"maxlen\":$D}"), STATION_NAME_SIZE);
			sprintf(tmp_buffer,"],\"maxlen\":%d}", STATION_NAME_SIZE);
			strcat(postval, tmp_buffer);
		}
			break;
			
		case SEND_CLOUD_STATUS_SPEC:
      // TODO
			break;
			
		case SEND_CLOUD_SETTINGS:   //based on server_json_controller_main() in server.cpp
		{
			  byte bid, sid;
			  ulong curr_time = os.now_tz();
			  //os.nvm_string_get(ADDR_NVM_LOCATION, tmp_buffer);
			/*	  bfill.emit_p(PSTR("\"devt\":$L,\"nbrd\":$D,\"en\":$D,\"rd\":$D,\"rs\":$D,\"rdst\":$L,"
				  "\"loc\":\"$E\",\"wtkey\":\"$E\",\"sunrise\":$D,\"sunset\":$D,\"eip\":$L,\"lwc\":$L,\"lswc\":$L,"
				  "\"lrun\":[$D,$D,$D,$L],"),
				  curr_time,
				  os.nboards,
				  os.status.enabled,
				  os.status.rain_delayed,
				  os.status.rain_sensed,
				  os.nvdata.rd_stop_time,
				  ADDR_NVM_LOCATION,
				  ADDR_NVM_WEATHER_KEY,
				  os.nvdata.sunrise_time,
				  os.nvdata.sunset_time,
				  os.nvdata.external_ip,
				  os.checkwt_lasttime,
				  os.checkwt_success_lasttime,
				  pd.lastrun.station,
				  pd.lastrun.program,
				  pd.lastrun.duration,
				  pd.lastrun.endtime);
			*/	  
					strcat_P(postval,PSTR("\"devt\":"));
					ultoa( (ulong)curr_time , tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(",\"nbrd\":"));
					itoa( os.nboards , tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(",\"en\":"));
					itoa( os.status.enabled , tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(",\"rd\":"));
					itoa( os.status.rain_delayed , tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(",\"rs\":"));
					itoa( os.status.rain_sensed , tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(",\"rdst\":"));
					itoa( os.nvdata.rd_stop_time , tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(",\"loc\":\""));
					nvm_read_block(tmp_buffer, (void*)ADDR_NVM_LOCATION, MAX_LOCATION);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR("\",\"wtkey\":\""));
					nvm_read_block(tmp_buffer, (void*)ADDR_NVM_WEATHER_KEY, MAX_WEATHER_KEY);			
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR("\",\"sunrise\":"));
					itoa( os.nvdata.sunrise_time , tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(",\"sunset\":"));
					itoa( os.nvdata.sunset_time , tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(",\"eip\":"));
					ultoa( (ulong)os.nvdata.external_ip , tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(",\"lwc\":"));
					ultoa( (ulong)os.checkwt_lasttime , tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(",\"lswc\":"));
					ultoa( os.checkwt_success_lasttime , tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(",\"lrun\":["));
					itoa( pd.lastrun.station , tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(","));
					itoa( pd.lastrun.program , tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(","));
					itoa( pd.lastrun.duration , tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(","));
					ultoa( pd.lastrun.endtime , tmp_buffer, 10);
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR("],"));

			  #if defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284__)
			  if(os.status.has_curr_sense) {
				  //bfill.emit_p(PSTR("\"curr\":$D,"), os.read_current());
					strcat_P(postval,PSTR("\"curr\":"));
					itoa( sensors.realtime_current , tmp_buffer, 10);  
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(","));
			  }
			  #endif
			  if(os.options[OPTION_FSENSOR_TYPE]==SENSOR_TYPE_FLOW) {
				  //bfill.emit_p(PSTR("\"flcrt\":$L,\"flwrt\":$D,"), sensors.station_impulses, FLOWCOUNT_RT_WINDOW);
					strcat_P(postval,PSTR("\"flcrt\":"));
					itoa( sensors.station_impulses , tmp_buffer, 10);  
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(","));
					strcat_P(postval,PSTR(",\"flwrt\":"));
					itoa( FLOWCOUNT_RT_WINDOW , tmp_buffer, 10);  
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(","));
			  }
			  //bfill.emit_p(PSTR("\"sbits\":["));
			  strcat_P(postval,PSTR("\"sbits\":["));
			  // print sbits
			  for(bid=0;bid<os.nboards;bid++){
					//bfill.emit_p(PSTR("$D,"), os.station_bits[bid]);
					itoa( os.station_bits[bid], tmp_buffer, 10);  
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(","));
			  }
			  //bfill.emit_p(PSTR("0],\"ps\":["));
			  strcat_P(postval,PSTR("0],\"ps\":["));
			  
			  // print ps
			  for(sid=0;sid<os.nstations;sid++) {
				  unsigned long rem = 0;
				  byte qid = pd.station_qid[sid];
				  RuntimeQueueStruct *q = pd.queue + qid;
				  if (qid<255) {
					  rem = (curr_time >= q->st) ? (q->st+q->dur-curr_time) : q->dur;
					  if(rem>65535) rem = 0;
				  }
				  //bfill.emit_p(PSTR("[$D,$L,$L]"), (qid<255)?q->pid:0, rem, (qid<255)?q->st:0);
					strcat_P(postval,PSTR("["));
					itoa( ((qid<255)?q->pid:0), tmp_buffer, 10);  
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(","));
					itoa( rem, tmp_buffer, 10);  
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR(","));
					itoa( ((qid<255)?q->st:0), tmp_buffer, 10);  
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR("]"));
				  
				  //bfill.emit_p((sid<os.nstations-1)?PSTR(","):PSTR("]"));
					strcat_P(postval,(sid<os.nstations-1)?PSTR(","):PSTR("]"));
			  }

			  if(read_from_file(wtopts_filename, tmp_buffer)) {
				  //bfill.emit_p(PSTR(",\"wto\":{$S}"), tmp_buffer);
					strcat_P(postval,PSTR(",\"wto\":{"));
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR("}"));	  
			  }
  
			  #if !defined(ARDUINO) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284__)
			  if(read_from_file(ifkey_filename, tmp_buffer)) {
				  //bfill.emit_p(PSTR(",\"ifkey\":\"$S\""), tmp_buffer);
					strcat_P(postval,PSTR(",\"ifkey\":\""));
					strcat(postval, tmp_buffer);
					strcat_P(postval,PSTR("\""));	  
			  }
			  #endif
			  //bfill.emit_p(PSTR("}"));
		  		strcat_P(postval,PSTR("}"));
		}
			break;	
	}
	
	DEBUG_PRINT(type);
	DEBUG_PRINTLN("   Cloud DATA: ");
	//DEBUG_PRINTLN(postval);
	DEBUG_PRINT("Lenght of json DATA: ");
	DEBUG_PRINT(strlen(postval));
	DEBUG_PRINT("      ret_val:  ");
	DEBUG_PRINT(ret_val);
	DEBUG_PRINT("    last_sent_log:  ");
	DEBUG_PRINTLN(last_sent_log);

	delay(1);
	  
 #if defined(ARDUINO)
    uint16_t _port = ether.hisport; // make a copy of the original port
    ether.hisport = 80;

  if(!ether.dnsLookup(server, true)) {
	  ret_val=4;  //not valid server url
    // if DNS lookup fails, use default IP
    ether.hisip[0] = 54;
    ether.hisip[1] = 172;
    ether.hisip[2] = 244;
    ether.hisip[3] = 116;
  }
	DEBUG_PRINT("\n  #ProcessCloud: ");
	DEBUG_PRINTLN(millis() - millis_cnt);

	millis_cnt = millis();
	if(ret_val < 3)
		ether.httpPost( PSTR("/sr\0"), PSTR(DEFAULT_CLOUD_URL), PSTR("Expect: 100-continue"), postval, httpget_callback);
		//Content-type: application/json\n\nConnection: Keep-Alive\n

  for(int l=0;l<250;l++)  ether.packetLoop(ether.packetReceive());
  ether.hisport = _port;
  
	DEBUG_PRINT("  #SendCloud: ");
	DEBUG_PRINTLN(millis() - millis_cnt);

 #else

  EthernetClient client;
  struct hostent *host;

  host = gethostbyname(server);
  if (!host) {
    DEBUG_PRINT("can't resolve http station - ");
    DEBUG_PRINTLN(server);
    return;
  }

  if (!client.connect((uint8_t*)host->h_addr, 80)) {
    client.stop();
    return;
  }

  char postBuffer[1500];
  sprintf(postBuffer, "POST /trigger/sprinkler/with/key/%s HTTP/1.0\r\n"
                      "Host: %s\r\n"
                      "Accept: */*\r\n"
                      "Content-Length: %d\r\n"
                      "Content-Type: application/json\r\n"
                      "\r\n%s", key, host->h_name, strlen(postval), postval);
  client.write((uint8_t *)postBuffer, strlen(postBuffer));

  bzero(ether_buffer, ETHER_BUFFER_SIZE);

  time_t timeout = now() + 5; // 5 seconds timeout
  while(now() < timeout) {
    int len=client.read((uint8_t *)ether_buffer, ETHER_BUFFER_SIZE);
    if (len<=0) {
      if(!client.connected())
        break;
      else
        continue;
    }
    httpget_callback(0, 0, ETHER_BUFFER_SIZE);
  }

  client.stop();

 #endif
  
#endif

return ret_val;
}

