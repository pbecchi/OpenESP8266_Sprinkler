 

 

# OpenSprinkler SW new features

 

| DESCRIPTION                                                      | Code modification define          | Modified routines                         |
|------------------------------------------------------------------|-----------------------------------|-------------------------------------------|
| 1.Compatibility with Oled 128x64 used as Display                 | \#define LCD\_SSD1306             | ,OpenSprinkler.cpp, OpenSprinklermain.cpp |
| 2.Operation on selflatching valves                               | \#define OSBEE ( 1 for OSBee 2.0) | OpenSprinkler.cpp                         |
| 3.Change to WiFi access selection method                         | \#define WIFIMANAGER 1            | opensprinklerServer.cpp                   |
| 4.Implement Battery Power control routines.                      | \#define BATTERY                  | OpenSprinkler.cpp OpenSprinkler.main      |
| 5.Modification of API for battery controls                       | \#define BATTERY                  | OpenSprinklerServer.cpp                   |
| 6.Modification of log files for battery monitor                  | \#define BATTERY                  | OpenSprinkler.cpp opensprinklerserver.cpp |
| 7.Recording and recalling on LCD of OS operation monitor message | \#define MESSAGE                  | utils.cpp opensprinklerserver.cpp         |
| 8.Unified RTC library for DS1307 MCP7940 PCF8523                 |                                   | New library: I2crtc.h                     |
| 9.Esp8266 ping library                                           |                                   |                                           |
| 10.Ina219 board.                                                 | \#define INA219                   | opensprikler.cpp                          |

    

    

    

    

   ##### 1 OLED 128x64

    

   Support of OLED 128x64 display replacing 20x2 or 20x4 character displays,
   has been added to the firmware.

   The display use I2C serial bus and has a minimal power consumption.

   Adafruit ADAFRUIT\_SSD1306 and ADAFRUIT\_GLX library has been utilized

   Existing output routines has been modified for the different number of lines
   and line length, maintaining as much similarity as possible.

   The additional lines available are used for battery info [ref 4] and for
   system messages. [ref.11]

    

    

   ###### 2 Self Latching Valves

    

   The valves require a pulse of 9v current to open and an inverted pulse to
   close:

   In the OpenSprinkler Solar unit this pulse is obtained from a capacitor
   charged putting the batteries in series

   in the OSBee unit the capacitor is charged by voltage step-up circuit.

   From SW standpoint operation is identical and consist in this sequence:

-   charge capacitor (open charge contat or , for batteries put batteries in
    series and close after .5 sec)

-   select valve and polarity (switch to positive common togever with all lines
    and to negative selcted valve line)

-   discharge capacitor (open and close contact relay)

   A couple of routines taken from OS Bee 2.0 firmware has been added to
   OpenSprinkler.cpp to perform above tasks.Selecting \#define OSBEE will
   redirect the valves control output to these routines (this is now only
   available for main board not for the extensions).

   Since valves are bistable,in case of system failure the valves will mantain
   its conditions (if it is open will stay open) but during restart startup all
   valves will be closed by the MCU.

    

  ##### 3 WiFi Manager Access point selection.

    

   Use of WiFiManager library allow selection of the WiFi AP. This feature was
   already available on previous release, but since the OS units operate most
   of the time unattended the automatic WiFiManager portal that open in case of
   WiFi unavailability, was frequently stopping OS operations.

   Now selecting \#define WIFIMANAGER 1 you can operate in a mixed mode,
   maintaining the advantage of the WiFimanager portal, but keeping a full
   autonomous operation.

   All selected Access Points SSID and password are store in a file . Each time
   the MCU try to connect ,it scan the WiFi networks available and select the
   best of the ones known (stored in the file).

   This way you can move the unit or access different AP without the need of
   user intervention to reenter the AP password.

   If you want to add one access point to your list , you have to restart the
   unit keeping pressed button 2. This will start WiFimanager portal to allow
   you to enter the new password.

   Button 2 is no more available to firmware Upload.

    

 #####  4 5 6 & 10 Solar Battery Power

    

   This feature, that i consider the most important, is available thank to the
   use of self latching valves and of Oled display. Using these components the
   average unit consumption is about 80 mAmps.

   For 5 day operation a 5000mAh battery is necessary ( counting only day hours
   with no light).In my proto this is accomplish with 2 18650 lipo 2600mAh that
   work most of the time in parallel, but are switched in series to charge the
   capacitor for operation of self latching valves.

   The batteries are charged with a TP4095 board by a 5V Solar Panel that can
   provide up to 5w. This way in full sunlight I can have a charging current of
   about 800mA and hopefully have the battery fully charged at the end of a
   sunny day.

   The software check battery voltage and , if an INA219 board is available,
   charging / discharging current.

   This way the actual battery charge can be computed . The lower line of the
   oled LCD show voltage, current and charge.

   In addition voltage charge and current are available through API
   ../jc?pw=...... and are also recorded in logs file : "bv" battery volts and
   "bc" battery charge.

   If battery is discharged (voltage reach a min. level) a power saving mode is
   activated:

   \-light sleep , the operation are halted for short (0-60 sec) every cycle,
   WiFi connectivity is maintained.

   \-deep sleep, that means full hibernation and after a period of hours the
   unit is restarted.

   In deep sleep the unit absorb less than 10mA and that way the battery
   duration can be extended for days, but no wifi connection or valves
   operation is possible during sleep interval.

    

   The two operational mode are controlled by options[45 to 50](to by read and
   modified by API /jo? , API /co? ):

| option[45] | DsleD | deep mode sleep duration       | hours     |
|------------|-------|--------------------------------|-----------|
| option[46] | DsleS | deep mode sleep start          | hour      |
| option[47] | DsleI | deep sleep interval duration   | minutes   |
| option[48] | DsleV | voltage to trigger depp sleep  | volts\*10 |
| option[49] | LsleV | voltage to trigger light sleep | volts\*10 |
| option[50] | LsleD | light sleep interval duration  | seconds   |
| option[51] | batC  | battery max capacity           | mAh/100   |

    

    

 #####  7 System messages

    

   The Unit monitor and records (with time stamp on file messages.txt) a
   certain number of system messages like:

   restart, Wifi connection, deep sleep, light sleep, battery fully charged,
   ....

   These messages are shown in the lower 3 lines of the oled LCD and recorded
   on message.txt file.

   Those lines can be recalled using Button n.3 , each push will go back 3
   lines and show it in the lower display.

    

   Use of Button 3 for extensions boards is no more available.

    

    

#####   8 Real Time Clock

    

   A new library for real time clock has been included in the firmware.

   This library allow the use of different type of RTC module: in addition to
   standard DS1307 , MCP7490 or PCF7323.

   The module is recognized at run time no flag or define is necessary!

    

##### ** 9 PING**

    

   A new ping library has been included in the compilation

    

    
