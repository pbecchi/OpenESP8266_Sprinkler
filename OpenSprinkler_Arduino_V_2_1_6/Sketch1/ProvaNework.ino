/*
 Name:		Sketch1.ino
 Created:	14/03/2016 12:11:11
 Author:	pbecc
*/
#include <Arduino.h>
#include "../Pins.h"

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);


}

// the loop function runs over and over again until power down or reset
void loop() {
	delay(2000);
	scanNetwork();
  
}
