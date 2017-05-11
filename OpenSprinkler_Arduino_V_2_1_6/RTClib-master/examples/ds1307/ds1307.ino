// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "../OpenSprinkler/OpenSprinkler_Arduino_V_2_1_6/RTClib-master/RTClib.h"
#define TIMELIB
#include <Time/TimeLib.h>
//RTC_DS1307 rtc;
RTC_MCP7940 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup () {
 // while (!Serial); // for Leonardo/Micro/Zero
	Wire.begin();//12,14);
  Serial.begin(115200);
  Serial.println("start...");
  byte ifound = 0;
  for (byte i = 0; i < 127; i++)
  {
	  Wire.beginTransmission(i);
	  if (Wire.endTransmission() == 0)Serial.println(i);
  }
  
  if ( !rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  Serial.println("test rtc");
  if (!rtc.isrunning()) {
	  Serial.println("RTC is NOT running!");
  }
    // following line sets the RTC to the date & time this sketch was compiled
   // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  
#ifdef TIMELIB
  setSyncProvider(rtc.get);
#endif
}

void loop () {
#ifdef TIMELIB
	Serial.print(year(), DEC);
	Serial.print('/');
	Serial.print(month(), DEC);
	Serial.print('/');
	Serial.print(day(), DEC);
	Serial.print(" (");
	Serial.print(daysOfTheWeek[(dayOfWeek(now())+6)%7]);
	Serial.print(") ");
	Serial.print(hour(), DEC);
	Serial.print(':');
	Serial.print(minute(), DEC);
	Serial.print(':');
	Serial.print(second(), DEC);
	Serial.println();
#else
   DateTime now = rtc.now();
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    
    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
  
    // calculate a date which is 7 days and 30 seconds into the future
    DateTime future (now + TimeSpan(7,12,30,6));
    
    Serial.print(" now + 7d + 30s: ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
    Serial.println();
#endif    
    Serial.println();
    delay(3000);
}