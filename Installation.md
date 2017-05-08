# Installation
Prior to compilation select the ESP board you are using.
The SW should run in any type of ESP board, anyhow dipending on the flash size:

    512  kBytes select 64k Spiffs , no Ota upload available
    1024  kBytes select 512k Spiffs, no Ota
    2048+  kByte select    1Mbcyte Spiffs Ota available

    


To install the code you have to use:

Arduino 1.6.8 IDE
ESP8266 Core Library ver 2.2 +

Following libraries are also necessary to compleate installation:
  - Adafruit-GFX-Library
  - Adafruit_SSD1306 library
  - Adafruit_INA219 library
  -[ ESP8266PING library] (github.com/dancol90/esp8266Ping )

### Firmware Upload


Firmware Upload can be done with Arduino OTA at the actual IP address of the unit.
Otherwise you can upload via the USB port of your ESP8266.
Make sure to set the ESP to firmware upload mode( for OSBee 2.0 and OS 3.0 keep Button 2 pressed while you restart your unit) 


