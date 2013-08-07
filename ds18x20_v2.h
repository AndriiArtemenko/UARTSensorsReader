/*
  CodeVisionAVR C Compiler
*/

#ifndef _DS18X20_INCLUDED_
#define _DS18X20_INCLUDED_

#include <1wire.h>
//-------------------------------------------------
#define DS18S20_FAMILY_CODE 0x10
#define DS18S20_SEARCH_ROM_CMD 0xf0
#define DS18S20_ALARM_SEARCH_CMD 0xec
//-------------------------------------------------
#define DS18B20_FAMILY_CODE 0x28
#define DS18B20_SEARCH_ROM_CMD 0xf0
#define DS18B20_ALARM_SEARCH_CMD 0xec
//-------------------------------------------------
#define DS18B20_9BIT_RES 0  // 9 bit thermometer resolution
#define DS18B20_10BIT_RES 1 // 10 bit thermometer resolution
#define DS18B20_11BIT_RES 2 // 11 bit thermometer resolution
#define DS18B20_12BIT_RES 3 // 12 bit thermometer resolution
//-------------------------------------------------


#pragma used+
//-------------------------------------------------
extern struct __ds18x20_scratch_pad_struct
       {
       unsigned char temp_lsb,temp_msb,
                temp_high,temp_low,
                conf_register,
                x1,
                x2,
                x3,
                crc;
       } __ds18x20_scratch_pad;
//-------------------------------------------------

//-------------------------------------------------
unsigned char ds18x20_select(unsigned char *addr);
unsigned char ds18x20_read_spd(unsigned char *addr);
//-------------------------------------------------

//-------------------------------------------------
int ds18s20_temperature(unsigned char *addr);

int ds18b20_temperature(unsigned char *addr);
//-------------------------------------------------
#pragma used-

#pragma library ds18x20_v2.lib

#endif

