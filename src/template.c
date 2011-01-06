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

	// 1-1 mixers...for simulator etc...
	g_Model.mixers[0][0].type = MIX_DIRECT;
	g_Model.mixers[0][0].input = MIX_IN_AIL;
	g_Model.mixers[0][0].output = MIX_OUT_AILERON;
	g_Model.mixers[0][0].scale = 100;
	g_Model.mixers[0][0].condition = MIX_COND_TRUE;
	g_Model.mixers[0][0].multiplex = MIX_REPLACE;

	g_Model.mixers[0][1].type = MIX_DIRECT;
	g_Model.mixers[0][1].input = MIX_IN_THR;
	g_Model.mixers[0][1].output = MIX_OUT_THROTTLE;
	g_Model.mixers[0][1].scale = 100;
	g_Model.mixers[0][1].condition = MIX_COND_TRUE;
	g_Model.mixers[0][1].multiplex = MIX_REPLACE;

	g_Model.mixers[0][2].type = MIX_DIRECT;
	g_Model.mixers[0][2].input = MIX_IN_ELE;
	g_Model.mixers[0][2].output = MIX_OUT_ELEVATOR;
	g_Model.mixers[0][2].scale = 100;
	g_Model.mixers[0][2].condition = MIX_COND_TRUE;
	g_Model.mixers[0][2].multiplex = MIX_REPLACE;

	g_Model.mixers[0][3].type = MIX_DIRECT;
	g_Model.mixers[0][3].input = MIX_IN_RUD;
	g_Model.mixers[0][3].output = MIX_OUT_RUDDER;
	g_Model.mixers[0][3].scale = 100;
	g_Model.mixers[0][3].condition = MIX_COND_TRUE;
	g_Model.mixers[0][3].multiplex = MIX_REPLACE;

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


}

