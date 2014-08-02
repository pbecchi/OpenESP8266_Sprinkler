OpenSprinkler-Arduino
=====================

This is a fork of Rays OpenSprinkler code thats amended to use alternative hardware:

(1) Arduino Mega 2560 
(2) Wiznet W5100 Ethernet with onboard SD Card 
(3) Freetronics LCD Keypad Shield
(4) Discrete IO outputs instead of using a shift register 

Version:     Opensprinkler 2.0.7 
Date:        July 5, 2014
Repository:  https://github.com/Dave1001/OpenSprinkler-Arduino
License:     Creative Commons Attribution-ShareAlike 3.0 license
   
==================================================================

The code implements a minimal set of functionality to replace the EtherCard class libraries and all underlying ENC28J60 code files used by Ray in the OpenSprinkler2 library.
   
All blocks of code that have been amended for the alternative hardware are marked with:
  "<MOD>"  at the start of the amended code block and 
  "</MOD>" at the end of the amended code block
     
If you need to reverse out changes to go back to the normal Opensprinkler config, just search for "MOD" in each file and undo the comments to go back to Rays code.

==================================================================

   Hardware:
   
   You'll need:
     - Arduino Mega 2560 (or any Arduino with enough progmem to handle binary sketch size of around 46Kb compiled)
       http://arduino.cc/en/Main/arduinoBoardMega2560 
       
     - Wiznet W5100 Ethernet Shield (i.e. Arduino 'standard')
       http://www.ebay.com/sch/i.html?_from=R40&_trksid=p2050601.m570.l1313.TR3.TRC0.A0.H0.Xw5100+shield&_nkw=w5100+shield&_sacat=0
     
     - Freetronics LCD Shield
       http://www.freetronics.com/collections/shields/products/lcd-keypad-shield
       
       This is a fairly common 16x2 LCD, however the main difference is the buttons are onboard the shield
       and are all sampled using one analog pin (each button has its own ADC voltage read from pin A0 
       indicate which one was pressed). 
       
     - Any old DS1307 compatible Real Time Clock ($2.45 inc postage off ebay)
     
     - Any 5V relay card that can be driven from Arduino digitial output pins. If you want to swap back to using
       a shift register, just search for comments labelled "shift register" and/or "digital outputs" in OpenSprinklerGen2.h, 
       OpenSprinklerGen2.cpp, and defines.h and swap the commented sections around.
       
     - Power supplies(s), ethernet switch, bits & pieces to hook it all up 
       
   ==========================================================================================================
  
   Issues and Limitations:

    - hostname isn't implemented for opensprinkler - you'll need to use IP to access
      (this seems to be a limitation of the Arduino Ethernet library)
      
    - port number is currently hard coded (default is 80) until I figure out how to
      change it dynamically when defining the EthernetServer object
      
    - the number of discrete pins is pre-defined as 16 maximum - you'll need to edit
      OpenSprinklerGen2.cpp and defines.h if you want more - make sure they're in multiples of 8 
   
   ==========================================================================================================
  
   To install and compile this code you need:
   
   1.  Arduino IDE    http://arduino.cc/en/Main/Software
  
   2.  Libraries:
          
          Get them all here -> 
          
          Or download them seperately from:
          <Wire.h>          Standard Arduino Library
          <LiquidCrystal.h> Standard Arduino Library
          <SPI.h>           Standard Arduino Library
          <Ethernet.h>      Standard Arduino Library
          <EthernetUdp.h>   Standard Arduino Library
          <ICMPPing.h>      https://github.com/BlakeFoster/Arduino-Ping (because I'm too lazy to write my own ping code - well done Blake!)
          <Time.h>          http://playground.arduino.cc/Code/time which links to http://www.pjrc.com/teensy/td_libs_Time.html 
          <TimeAlarms.h>    http://playground.arduino.cc/Code/time which links to http://www.pjrc.com/teensy/td_libs_TimeAlarms.html 
          <DS1307RTC.h>     http://playground.arduino.cc/Code/time which links to http://www.pjrc.com/teensy/td_libs_DS1307RTC.html
          <MemoryFree.h>    http://playground.arduino.cc/Code/AvailableMemory which links to https://dl.dropbox.com/u/56421988/MemoryFree.zip
          <tinyFAT.h>       http://www.henningkarlsen.com/electronics/library.php?id=37
                            
          NOTE - unless you're careful, you may have some issues with the libraries above co-existing 
          with OpenSprinklerGen2 library. The easiest way to fix it is to remove OpenSprinklerGen2 from your 
          Arduino library folder. 

    3. Assuming you have the libraries above installed, the code here *should* compile without too many issues (good luck!)
       

