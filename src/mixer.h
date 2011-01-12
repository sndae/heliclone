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

#ifndef _MIXER_H_
#define _MIXER_H_

#include <stdint.h>

typedef enum
{
	// Analog inputs
	MIX_IN_AIL = 0x0,
	MIX_IN_THR,
	MIX_IN_ELE,
	MIX_IN_RUD,
	MIX_IN_POT1,
	MIX_IN_POT2,
	MIX_IN_POT3,

	// Other MIXERS (LEVEL 1)
	MIX_IN_MIXER_1 = 0x20,
	MIX_IN_MIXER_2,
	MIX_IN_MIXER_3,
	MIX_IN_MIXER_4,
	MIX_IN_MIXER_5,
	MIX_IN_MIXER_6,
	MIX_IN_MIXER_7,
	MIX_IN_MIXER_8,

	// Constants
	MIX_IN_MAX_VALUE = 0x30,
	MIX_IN_MIN_VALUE = 0x30

} MIX_INPUT;

typedef enum
{
	// Servo ouputs
	MIX_OUT_SRV1 = 0x0,
	MIX_OUT_SRV2,
	MIX_OUT_SRV3,
	MIX_OUT_SRV4,
	MIX_OUT_SRV5,
	MIX_OUT_SRV6,
	MIX_OUT_SRV7,
	MIX_OUT_SRV8,

	// Other MIXERS (LEVEL 2)
	MIX_OUT_MIXER_21 = 0x20,
	MIX_OUT_MIXER_22,
	MIX_OUT_MIXER_23,
	MIX_OUT_MIXER_24,
	MIX_OUT_MIXER_25,
	MIX_OUT_MIXER_26,
	MIX_OUT_MIXER_27,
	MIX_OUT_MIXER_28,

	// Functional ouputs
	MIX_OUT_AILERON = 0x30,
	MIX_OUT_THROTTLE,
	MIX_OUT_ELEVATOR,
	MIX_OUT_RUDDER,
	MIX_OUT_GYRO_GAIN,
	MIX_OUT_PITCH,
	MIX_OUT_AUX1,
	MIX_OUT_AUX2,


} MIX_OUTPUT;


typedef enum
{
	MIX_DIRECT = 0x00,	// output = scale*input
	MIX_CURVE,			// output = scale*curve[input]

} MIX_TYPE;

typedef enum
{
	MIX_ADD = 0x00,	
	MIX_MULTIPLY,
	MIX_REPLACE,

} MIX_MULTIPLEX;


typedef enum
{
	MIX_COND_TRUE = 0x00,

	// Switches
	MIX_COND_THR,
	MIX_COND_NOT_THR,
	MIX_COND_RUDDR,
	MIX_COND_NOT_RUDDR,
	MIX_COND_ELEDR,
	MIX_COND_NOT_ELEDR,
	MIX_COND_AILDR,
	MIX_COND_NOT_AILDR,
	MIX_COND_GEAR,
	MIX_COND_NOT_GEAR,
	MIX_COND_ID0,
	MIX_COND_NOT_ID0,
	MIX_COND_ID1,
	MIX_COND_NOT_ID1,
	MIX_COND_ID2,
	MIX_COND_NOT_ID2,

	MIX_COND_FALSE
} MIX_CONDITION;


typedef struct
{
	// Type
	MIX_TYPE type:3;

	// Input
	MIX_INPUT input:6;

	// Output
	MIX_OUTPUT output:6;

	uint8_t scale;		// -100...+100 %

	MIX_CONDITION	condition:5; // Switch position?

	MIX_MULTIPLEX multiplex:2;
	
	// What curve are we using?
	uint8_t curve:3;

	// Level (0 or 1)
	uint8_t level:1;

} SMixer;


extern void mixer_init(); 
extern void mixer_mix();

// For usage in menu to view the expo curve
extern int16_t mixer_expo(MIX_INPUT source, int16_t value);

#endif // _MIXER_H_
