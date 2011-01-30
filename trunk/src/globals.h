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


// Enable this to have SOFTSTART filter
//#define F_SOFTSTART


#ifdef USE_DEBUG_MODE
extern char debugLine1[20];
extern char debugLine2[20];
#endif

#define MDL_DEFAULT_THR_TIMER	(5)

#define MDL_MAX_CHANNELS		(8)
#define MDL_MAX_FUNCTIONS		(8)
#define MDL_MAX_MIXERS 			(16)
#define MDL_MAX_CURVE_POINTS 	(5)

#define SERVO_CHANNEL(N) (N-1)
#define FUNCTION_INDEX(N) (N-MIX_OUT_AILERON)

#define DO_GUI   (0x01)
#define DO_KEYS  (0x02)
#define DO_IO    (0x04)
#define DO_CLOCK (0x08)

typedef enum
{
	MDL_SWASH_AIL = 0,
	MDL_SWASH_ELE,
	MDL_SWASH_PIT
} MDL_SWASH;

typedef enum
{
	MDL_GYRO_AVCS = 0,
	MDL_GYRO_RATE
} MDL_GYRO;

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
	MDL_TYPE_HELI_SIM = 0,
	MDL_TYPE_HELI_FBL,
	MDL_TYPE_HELI_ECCPM_120,
	MDL_TYPE_HELI_ECCPM_140,
	MDL_MAX_TYPES,
	MDL_TYPE_GENERAL = 0xFF
} MDL_TYPE;

typedef enum
{
	MDL_EXPO_NORM = 0x0,
	MDL_EXPO_DUAL
} MDL_EXPO;


typedef enum
{
	MDL_TIMER_OFF         = 0,
	MDL_TIMER_SWITCH      = 200,
} MDL_TIMER;


typedef struct
{
	// LCD
	uint8_t contrast;
	uint8_t backlight;	// 0=ON, 1=OFF, 2=5s, 3=10s, 4=15s, 5=20s";

	// BEEP
	uint8_t keyBeep;
	uint8_t alarmBeep;
	uint8_t volumeBeep;

	// ADC
	uint16_t 	adc_c[8][3]; // mins, max, cent

	// MENUS
	uint8_t 	message_box_timeout;

	// VOLTAGE
	uint8_t		voltageWarning; 

	// Selected model number
	uint8_t 	selectedModel;

	// Softstart speed ramp
#ifdef F_SOFTSTART
	uint16_t	softStartMax;
	uint16_t    softStartIncEvery;
#endif

} SRadioConfig;

typedef struct
{
	// Clocks
	uint32_t systemTick;
	uint32_t secondTick;
	uint32_t modelTimer;
	uint8_t timerStarted;
	uint8_t alarmCleared;

	// Signals
	uint8_t	doSignal;

	// ADC
	uint16_t 	adc_r[8];    // raw reads
	int16_t		adc_s[8];    // reads - stable (-100,0,100)

	uint8_t  	adc_store_end; 	 // ADC auto calibration mode
	uint8_t  	adc_store_mid; 	 // ADC auto calibration mode

	// Servos output values (from mixing stage)
	int16_t srv_s[MDL_MAX_CHANNELS];  	// current calculated servos  <-100,100>

	// PPM active
	uint8_t 	ppmActive;

	// BEEP (next Beep request)
	uint8_t beep;
	uint8_t beepStyle;

	// Misc buffer...good to have ;-)
	char buffer[16];

#ifdef F_SOFTSTART
	// Soft-start 
	int16_t softStartLastTHR;
	uint8_t softStart;
	uint8_t softStartArmed;
	uint16_t softStartTick;
#endif

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
	uint8_t functionToServoTable[MDL_MAX_FUNCTIONS];			// 8/112

	// Curves
	int8_t curve[MDL_MAX_CURVES][MDL_MAX_CURVE_POINTS];			// 8*5=40/152

	// EXPO
	int8_t expo[4][2];											// 8/160

	// SWASH_MIX
	int8_t swash[3];											// 3/163

	// GYRO Gain
	int8_t gyro[2];												// 2/165

	// endpoints
	int8_t endPoint[2][MDL_MAX_CHANNELS];						// 16/181

	// timer
	uint16_t timer;												// 2/183
	uint8_t  timerCond;											// 1/184
	uint16_t timerAlarmLimit;									// 2/186

} SModel;

// Exported here...but created  in main.c
extern SRadioConfig g_RadioConfig;
extern SRadioRuntime g_RadioRuntime;
extern SModel g_Model;

#endif // _GLOBALS_H_
