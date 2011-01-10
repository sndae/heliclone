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

#include "template.h"
#include "globals.h"
#include <string.h>

void template_simulator() 
{
	strncpy(&(g_Model.name[0]), "Simulator", 10);

	// All servos NORMAL direction
	g_Model.servoDirection = 0x00;

	// FlySky original mapping...
	g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_AILERON)] = SERVO_CHANNEL(1);
	g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_ELEVATOR)] = SERVO_CHANNEL(2);
	g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_THROTTLE)] = SERVO_CHANNEL(3);
	g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_RUDDER)] = SERVO_CHANNEL(4);
	g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_GYRO_GAIN)] = SERVO_CHANNEL(5);
	g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_PITCH)] = SERVO_CHANNEL(6);
	g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_AUX1)] = SERVO_CHANNEL(7);
	g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_AUX2)] = SERVO_CHANNEL(8);

	// Throttle Curve ID0
	g_Model.curve[MDL_CURVE_THR_ID0][0] = -100;
	g_Model.curve[MDL_CURVE_THR_ID0][1] = -50;
	g_Model.curve[MDL_CURVE_THR_ID0][2] = 0;
	g_Model.curve[MDL_CURVE_THR_ID0][3] = 50;
	g_Model.curve[MDL_CURVE_THR_ID0][4] = 100;

	// Throttle Curve ID1
	g_Model.curve[MDL_CURVE_THR_ID1][0] = 100;
	g_Model.curve[MDL_CURVE_THR_ID1][1] = 75;
	g_Model.curve[MDL_CURVE_THR_ID1][2] = 50;
	g_Model.curve[MDL_CURVE_THR_ID1][3] = 75;
	g_Model.curve[MDL_CURVE_THR_ID1][4] = 100;

	// Throttle Curve ID2
	g_Model.curve[MDL_CURVE_THR_ID2][0] = -100;
	g_Model.curve[MDL_CURVE_THR_ID2][1] = -50;
	g_Model.curve[MDL_CURVE_THR_ID2][2] = 0;
	g_Model.curve[MDL_CURVE_THR_ID2][3] = 50;
	g_Model.curve[MDL_CURVE_THR_ID2][4] = 100;

	// Throttle Curve HOLD
	g_Model.curve[MDL_CURVE_THR_HOLD][0] = -100;
	g_Model.curve[MDL_CURVE_THR_HOLD][1] = -100;
	g_Model.curve[MDL_CURVE_THR_HOLD][2] = -100;
	g_Model.curve[MDL_CURVE_THR_HOLD][3] = -100;
	g_Model.curve[MDL_CURVE_THR_HOLD][4] = -100;

	// Pitch Curve ID0
	g_Model.curve[MDL_CURVE_PIT_ID0][0] = -15;
	g_Model.curve[MDL_CURVE_PIT_ID0][1] = -7;
	g_Model.curve[MDL_CURVE_PIT_ID0][2] = 0;
	g_Model.curve[MDL_CURVE_PIT_ID0][3] = 50;
	g_Model.curve[MDL_CURVE_PIT_ID0][4] = 100;

	// Pitch Curve ID1
	g_Model.curve[MDL_CURVE_PIT_ID1][0] = -100;
	g_Model.curve[MDL_CURVE_PIT_ID1][1] = -50;
	g_Model.curve[MDL_CURVE_PIT_ID1][2] = 0;
	g_Model.curve[MDL_CURVE_PIT_ID1][3] = 50;
	g_Model.curve[MDL_CURVE_PIT_ID1][4] = 100;

	// Pitch Curve ID2
	g_Model.curve[MDL_CURVE_PIT_ID2][0] = -100;
	g_Model.curve[MDL_CURVE_PIT_ID2][1] = -50;
	g_Model.curve[MDL_CURVE_PIT_ID2][2] = 0;
	g_Model.curve[MDL_CURVE_PIT_ID2][3] = 50;
	g_Model.curve[MDL_CURVE_PIT_ID2][4] = 100;

	// Pitch Curve HOLD
	g_Model.curve[MDL_CURVE_PIT_HOLD][0] = -100;
	g_Model.curve[MDL_CURVE_PIT_HOLD][1] = -50;
	g_Model.curve[MDL_CURVE_PIT_HOLD][2] = 0;
	g_Model.curve[MDL_CURVE_PIT_HOLD][3] = 50;
	g_Model.curve[MDL_CURVE_PIT_HOLD][4] = 100;


	// Simulator mixers...
	g_Model.mixers[0].type = MIX_DIRECT;
	g_Model.mixers[0].input = MIX_IN_AIL;
	g_Model.mixers[0].output = MIX_OUT_AILERON;
	g_Model.mixers[0].scale = 100;
	g_Model.mixers[0].condition = MIX_COND_TRUE;
	g_Model.mixers[0].multiplex = MIX_REPLACE;
	g_Model.mixers[0].level = 0;

	g_Model.mixers[1].type = MIX_DIRECT;
	g_Model.mixers[1].input = MIX_IN_ELE;
	g_Model.mixers[1].output = MIX_OUT_ELEVATOR;
	g_Model.mixers[1].scale = 100;
	g_Model.mixers[1].condition = MIX_COND_TRUE;
	g_Model.mixers[1].multiplex = MIX_REPLACE;
	g_Model.mixers[1].level = 0;

	g_Model.mixers[2].type = MIX_DIRECT;
	g_Model.mixers[2].input = MIX_IN_RUD;
	g_Model.mixers[2].output = MIX_OUT_RUDDER;
	g_Model.mixers[2].scale = 100;
	g_Model.mixers[2].condition = MIX_COND_TRUE;
	g_Model.mixers[2].multiplex = MIX_REPLACE;
	g_Model.mixers[2].level = 0;

	// THROTTLE ID0
	g_Model.mixers[3].type = MIX_CURVE;
	g_Model.mixers[3].input = MIX_IN_THR;
	g_Model.mixers[3].output = MIX_OUT_THROTTLE;
	g_Model.mixers[3].scale = 100;
	g_Model.mixers[3].condition = MIX_COND_ID0;
	g_Model.mixers[3].multiplex = MIX_ADD;
	g_Model.mixers[3].curve = MDL_CURVE_THR_ID0;
	g_Model.mixers[3].level = 0;

	// THROTTLE ID1
	g_Model.mixers[4].type = MIX_CURVE;
	g_Model.mixers[4].input = MIX_IN_THR;
	g_Model.mixers[4].output = MIX_OUT_THROTTLE;
	g_Model.mixers[4].scale = 100;
	g_Model.mixers[4].condition = MIX_COND_ID1;
	g_Model.mixers[4].multiplex = MIX_ADD;
	g_Model.mixers[4].curve = MDL_CURVE_THR_ID1;
	g_Model.mixers[4].level = 0;

	// THROTTLE ID2
	g_Model.mixers[5].type = MIX_CURVE;
	g_Model.mixers[5].input = MIX_IN_THR;
	g_Model.mixers[5].output = MIX_OUT_THROTTLE;
	g_Model.mixers[5].scale = 100;
	g_Model.mixers[5].condition = MIX_COND_ID2;
	g_Model.mixers[5].multiplex = MIX_ADD;
	g_Model.mixers[5].curve = MDL_CURVE_THR_ID2;
	g_Model.mixers[5].level = 0;

	// THROTTLE HOLD
	g_Model.mixers[6].type = MIX_CURVE;
	g_Model.mixers[6].input = MIX_IN_THR;
	g_Model.mixers[6].output = MIX_OUT_THROTTLE;
	g_Model.mixers[6].scale = 100;
	g_Model.mixers[6].condition = MIX_COND_THR;
	g_Model.mixers[6].multiplex = MIX_REPLACE;
	g_Model.mixers[6].curve = MDL_CURVE_THR_HOLD;
	g_Model.mixers[6].level = 0;

	// PITCH ID0
	g_Model.mixers[7].type = MIX_CURVE;
	g_Model.mixers[7].input = MIX_IN_THR;
	g_Model.mixers[7].output = MIX_OUT_PITCH;
	g_Model.mixers[7].scale = 100;
	g_Model.mixers[7].condition = MIX_COND_ID0;
	g_Model.mixers[7].multiplex = MIX_ADD;
	g_Model.mixers[7].curve = MDL_CURVE_PIT_ID0;
	g_Model.mixers[7].level = 0;

	// PITCH ID1
	g_Model.mixers[8].type = MIX_CURVE;
	g_Model.mixers[8].input = MIX_IN_THR;
	g_Model.mixers[8].output = MIX_OUT_PITCH;
	g_Model.mixers[8].scale = 100;
	g_Model.mixers[8].condition = MIX_COND_ID1;
	g_Model.mixers[8].multiplex = MIX_ADD;
	g_Model.mixers[8].curve = MDL_CURVE_PIT_ID1;
	g_Model.mixers[8].level = 0;

	// PITCH ID2
	g_Model.mixers[9].type = MIX_CURVE;
	g_Model.mixers[9].input = MIX_IN_THR;
	g_Model.mixers[9].output = MIX_OUT_PITCH;
	g_Model.mixers[9].scale = 100;
	g_Model.mixers[9].condition = MIX_COND_ID2;
	g_Model.mixers[9].multiplex = MIX_ADD;
	g_Model.mixers[9].curve = MDL_CURVE_PIT_ID2;
	g_Model.mixers[9].level = 0;

	// PITCH HOLD
	g_Model.mixers[10].type = MIX_CURVE;
	g_Model.mixers[10].input = MIX_IN_THR;
	g_Model.mixers[10].output = MIX_OUT_PITCH;
	g_Model.mixers[10].scale = 100;
	g_Model.mixers[10].condition = MIX_COND_THR;
	g_Model.mixers[10].multiplex = MIX_REPLACE;
	g_Model.mixers[10].curve = MDL_CURVE_PIT_HOLD;
	g_Model.mixers[10].level = 0;

}

