Status - Under Test
-------------------
Please note - V2.0.7 has been updated to the latest OpenSprinkler code. It seems to work OK but has not been extensively tested.

As an alternative - the 2.0.0 code is rock solid and works fine (albeit with a few less features).

OpenSprinkler-Arduino
---------------------

This is a fork of Rays OpenSprinkler code thats amended to use alternative hardware:

- Arduino Mega 2560
- Wiznet W5100 Ethernet with onboard SD Card
- Freetronics LCD Keypad Shield
- Discrete IO outputs instead of using a shift register 

Version Info
------------

Version:     Opensprinkler 2.0.7

Date:        July 5, 2014

Repository:  https://github.com/Dave1001/OpenSprinkler-Arduino

License:     Creative Commons Attribution-ShareAlike 3.0 license
   

Code Mods
---------

This code implements a minimal set of functionality to replace the EtherCard class libraries and all underlying ENC28J60 code files used by Ray in the OpenSprinkler2 library.
   
All blocks of code that have been amended for the alternative hardware are marked with "MOD"
     
If you need to reverse out changes to go back to the normal Opensprinkler config, just remove the compiler switches at the start of 'defines.h' to go back to Rays code.

Hardware
--------
   
You'll need:

- Arduino Mega 2560 (or any Arduino with enough progmem to handle binary sketch size of around 65Kb compiled) from [Arduino Mega]

- Wiznet W5100 Ethernet Shield (i.e. Arduino 'standard') - $7 from [Ebay W5100]

- Freetronics LCD Shield from [Freetronics LCD]

    This is a fairly common 16x2 LCD, however the main difference is the buttons are onboard the shield and are all sampled using one analog pin (each button has its own ADC voltage read from pin A0 to indicate which one was pressed). 

- Any DS1307 compatible Real Time Clock - $1 from [Ebay DS1307]

- Any 5V relay card that can be driven from Arduino digitial output pins. 

- Power supplies(s), ethernet switch, bits & pieces to hook it all up 
  
Issues and Limitations
----------------------

- hostname isn't implemented for opensprinkler - you'll need to use IP to access (this seems to be a limitation of the Arduino Ethernet library

- port number is currently hard coded (default is 80) until I figure out how to change it dynamically when defining the EthernetServer object

- the number of discrete pins is pre-defined as 16 maximum - you'll need to edit OpenSprinklerGen2.cpp and defines.h if you want more - make sure they're in multiples of 8 

Installation
------------

To install and compile this code you need:
   
   1.  Arduino IDE    http://arduino.cc/en/Main/Software
  
   2.  Libraries:

   Get them all here -> https://github.com/Dave1001/OpenSprinkler-Arduino/tree/master/Controller_W5100_V_2_0_7/libraries

   Or download them from:

- "Wire.h"          Standard Arduino Library
- "LiquidCrystal.h" Standard Arduino Library
- "SPI.h"           Standard Arduino Library
- "Ethernet.h"      Standard Arduino Library
- "EthernetUdp.h"   Standard Arduino Library
- "ICMPPing.h"      https://github.com/BlakeFoster/Arduino-Ping
- "Time.h"          http://www.pjrc.com/teensy/td_libs_Time.html 
- "TimeAlarms.h"    http://www.pjrc.com/teensy/td_libs_TimeAlarms.html 
- "DS1307RTC.h"     http://www.pjrc.com/teensy/td_libs_DS1307RTC.html
- "SDFat.h"         http://arconlab.com/lab/Arduino/Library/SD%20Reader%20-%20Fat32/html/
- "MemoryFree.h"    http://playground.arduino.cc/Code/AvailableMemory which links to https://dl.dropbox.com/u/56421988/MemoryFree.zip

  NOTE - unless you're careful, you may have some issues with the libraries above co-existing with OpenSprinklerGen2 library. The easiest way to fix it is to remove OpenSprinklerGen2 from your Arduino library folder. 

  Assuming you have the libraries above installed, the code here *should* compile without too many issues (good luck!)

[Freetronics LCD]:http://www.freetronics.com/collections/shields/products/lcd-keypad-shield
[Ebay W5100]:http://www.ebay.com/sch/i.html?_from=R40&_trksid=p2050601.m570.l1313.TR3.TRC0.A0.H0.Xw5100+shield&_nkw=w5100+shield&_sacat=0
[Ebay DS1307]:http://www.ebay.com/sch/i.html?_from=R40&_trksid=p2050601.m570.l1313.TR10.TRC2.A0.H0.Xds1307&_nkw=ds1307&_sacat=0
[Arduino Mega]:http://arduino.cc/en/Main/arduinoBoardMega2560
