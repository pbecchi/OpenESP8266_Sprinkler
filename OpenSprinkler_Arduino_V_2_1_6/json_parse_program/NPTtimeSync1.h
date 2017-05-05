#pragma once

#include <TimeLib.h> 
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <RTCLIB.h>
/////////////////////////to syncronise= setSyncProvider(getNtpTime);RTC.set(now()); 

extern RTC_DS1307 RTC;


time_t getNtpTime();
void sendNTPpacket(IPAddress &address);
bool SyncNPT(byte DST);
