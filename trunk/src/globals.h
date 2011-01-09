/*
 *  HeliClone  - Copyright (C) 2010
 *               Stefan Grufman <gruffypuffy <at> gmail.com>
 *
 *  Description: Replacement software for model radio:
 *               FS-TH9X / Turnigy 9X / Eurgle 9X / iMax 9X.
 *
 *  Based on various projects for this radio:
 *   - RadioClone v 0.01  - Copyright (C) 2010 Rafal Tomczak <rafit <at> m2p.pl>
 *   - er9x -> http://code.google.com/p/er9x/
 *  
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdint.h>
#include "mixer.h"


// This eats a little RAM...remove me!
#define USE_DEBUG_MODE

#ifdef USE_DEBUG_MODE
extern char debugLine1[20];
extern char debugLine2[20];
#endif


#define MDL_MAX_CHANNELS		(8)
#define MDL_MAX_MIXERS 			(16)
#define MDL_MAX_CURVE_POINTS 	(5)

#define SERVO_CHANNEL(N) (N-1)
#define FUNCTION_INDEX(N) (N-MIX_OUT_AILERON)

typedef enum
{
	// Throttle curves
	MDL_CURVE_THR_ID0 = 0x00,
	MDL_CURVE_THR_ID1,
	MDL_CURVE_THR_ID2,
	MDL_CURVE_THR_HOLD,

	// Pitch curves
	MDL_CURVE_PIT_ID0,
	MDL_CURVE_PIT_ID1,
	MDL_CURVE_PIT_ID2,
	MDL_CURVE_PIT_HOLD,

	MDL_MAX_CURVES
} MDL_CURVES;


typedef enum
{
	MDL_TYPE_GENERAL = 0x0,
	MDL_TYPE_HELI_SIM = 0x10,
	MDL_TYPE_HELI_FBL,
	MDL_TYPE_HELI_ECCPM_120,
	MDL_TYPE_HELI_ECCPM_140,
	MDL_MAX_TYPES
} MDL_TYPE;



typedef struct
{
	// LCD
	uint8_t contrast;
	uint8_t backlight;	// 0=ON, 1=OFF, 2=5s, 3=10s, 4=15s, 5=20s";

	// BEEP
	uint8_t keyBeep;
	uint8_t alarmBeep;

	// ADC
	uint16_t 	adc_c[8][3]; // mins, max, cent

	// MENUS
	uint8_t 	message_box_timeout;

	// VOLTAGE
	uint8_t		voltageWarning; 

	// Selected model number
	uint8_t 	selectedModel;

} __attribute__((packed)) SRadioConfig;

typedef struct
{
	// Clocks
	uint32_t systemTick;

	// Signals
	uint8_t	doGui;
	uint8_t	doKeys;
	uint8_t	doIO;

	// ADC
	uint16_t 	adc_r[8];    // raw reads
	int16_t		adc_s[8];    // reads - stable (-100,0,100)

	uint8_t  	adc_store_end; 	 // ADC auto calibration mode
	uint8_t  	adc_store_mid; 	 // ADC auto calibration mode

	// Servos output values (from mixing stage)
	int16_t srv_s[MDL_MAX_CHANNELS];  	// current calculated servos  <-100,100>

} SRadioRuntime;


typedef struct
{
	// Model name
	char name[10];												// 10/10

	// Model type.
	MDL_TYPE type;												// 1/11

	// The trims...
	int8_t trim[4];												// 4/15

	// subtrims
	int8_t subTrim[MDL_MAX_CHANNELS];							// 8/23

	// Mixers (level 1 and level 2)
	SMixer mixers[MDL_MAX_MIXERS];								// 16*5=80/103

	// Servo reverse // 0- Normal, 1-Reverse, 
	// BIT-field. [BIT 0 => servo 0 etc]
	uint8_t servoDirection;										// 1/104

	// Servo vs channel table...
	// Used to select if CH1 is AIL etc...
	uint8_t functionToServoTable[MDL_MAX_CHANNELS];				// 8/112

	// Curves
	int8_t curve[MDL_MAX_CURVES][MDL_MAX_CURVE_POINTS];			// 8*5=40/152

	// EXPO
	int8_t expo[4][2];											// 8/160

} __attribute__((packed)) SModel;

// Exported here...but created  in main.c
extern SRadioConfig g_RadioConfig;
extern SRadioRuntime g_RadioRuntime;
extern SModel g_Model;

#endif // _GLOBALS_H_
