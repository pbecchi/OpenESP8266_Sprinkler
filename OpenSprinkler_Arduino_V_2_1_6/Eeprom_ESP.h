#pragma once
#include "libsel.h"
#ifdef EEPROM_ESP

#ifndef _EEPROM_ESP_h
#define _EEPROM_ESP_h
/*
#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
*/
#include <stdio.h>
#endif

unsigned char eeprom_read_byte(
	unsigned char * __p);
int eeprom_read_word(
	const unsigned int * __p);
void eeprom_write_byte(
	unsigned char * __p,
	unsigned char __value);
void eeprom_read_block(
	void * __dst,
	const void * __src,
	unsigned int __n);//Read a block of __n bytes from EEPROM address __src to SRAM __dst.
void eeprom_write_block(
	const void * __src,
	void * __dst,
	unsigned int __n); //Write a block of __n bytes to EEPROM address __dst from __src.

#endif