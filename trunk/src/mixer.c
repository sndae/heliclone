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

#include "mixer.h"

#include "globals.h"
#include "hal_io.h"


#define CURVE(P, N) g_Model.curve[N][P]


/*--------------------------------------------------------------------------------
 * LOCALS
 *--------------------------------------------------------------------------------*/
int16_t mixerOutput[MDL_MAX_MIXERS];

int16_t servoSavedValue[MDL_MAX_CHANNELS];
uint8_t servoChanged[MDL_MAX_CHANNELS];


/*--------------------------------------------------------------------------------
 * mixer_init
 *--------------------------------------------------------------------------------*/
void mixer_init()
{
}

/*--------------------------------------------------------------------------------
 * mixer_get_input
 *--------------------------------------------------------------------------------*/
int16_t mixer_get_input(MIX_INPUT source)
{
	uint8_t index;
	switch (source)
	{
		// ADC Channels
		case MIX_IN_AIL:
		case MIX_IN_THR:
		case MIX_IN_ELE:
		case MIX_IN_RUD:
		case MIX_IN_POT1:
		case MIX_IN_POT2:
		case MIX_IN_POT3:
			index = source - MIX_IN_AIL;
			return g_RadioRuntime.adc_s[index];

		// Level 1 mixers...
		case MIX_IN_MIXER_1:
		case MIX_IN_MIXER_2:
		case MIX_IN_MIXER_3:
		case MIX_IN_MIXER_4:
		case MIX_IN_MIXER_5:
		case MIX_IN_MIXER_6:
		case MIX_IN_MIXER_7:
		case MIX_IN_MIXER_8:
			index = source - MIX_IN_MIXER_1;
			return mixerOutput[index];

		default:
			return 0;
	}
}

/*--------------------------------------------------------------------------------
 * mixer_set_output
 *--------------------------------------------------------------------------------*/
void mixer_set_output(MIX_OUTPUT destination, int16_t value)
{
	uint8_t index;
	switch (destination)
	{
		// Servo ouputs
		case MIX_OUT_SRV1:
		case MIX_OUT_SRV2:
		case MIX_OUT_SRV3:
		case MIX_OUT_SRV4:
		case MIX_OUT_SRV5:
		case MIX_OUT_SRV6:
		case MIX_OUT_SRV7:
		case MIX_OUT_SRV8:
			index = destination - MIX_OUT_SRV1;
			g_RadioRuntime.srv_s[index] = value;
			servoChanged[index] = 1;
			break;
		// Level two mixers...
		case MIX_OUT_MIXER_21:
		case MIX_OUT_MIXER_22:
		case MIX_OUT_MIXER_23:
		case MIX_OUT_MIXER_24:
		case MIX_OUT_MIXER_25:
		case MIX_OUT_MIXER_26:
		case MIX_OUT_MIXER_27:
		case MIX_OUT_MIXER_28:
			index = destination - MIX_OUT_MIXER_21;
			mixerOutput[index] = value;
			break;
		// Functional output
		case MIX_OUT_AILERON:
		case MIX_OUT_THROTTLE:
		case MIX_OUT_ELEVATOR:
		case MIX_OUT_RUDDER:
		case MIX_OUT_GYRO_GAIN:
		case MIX_OUT_PITCH:
		case MIX_OUT_AUX1:
		case MIX_OUT_AUX2:
			index = destination - MIX_OUT_AILERON;
			// Lookup where the function is mapped...
			index = g_Model.functionToServoTable[index];
			// Now set the wanted servo to the output.
			g_RadioRuntime.srv_s[index] = value;
			servoChanged[index] = 1;
			break;
		default:
			return;
	}
}

/*--------------------------------------------------------------------------------
 * mixer_get_output
 *--------------------------------------------------------------------------------*/
int16_t mixer_get_output(MIX_OUTPUT destination)
{
	uint8_t index;
	switch (destination)
	{
		// Servo ouputs
		case MIX_OUT_SRV1:
		case MIX_OUT_SRV2:
		case MIX_OUT_SRV3:
		case MIX_OUT_SRV4:
		case MIX_OUT_SRV5:
		case MIX_OUT_SRV6:
		case MIX_OUT_SRV7:
		case MIX_OUT_SRV8:
			index = destination - MIX_OUT_SRV1;
			return g_RadioRuntime.srv_s[index];

		// Functional output
		case MIX_OUT_AILERON:
		case MIX_OUT_THROTTLE:
		case MIX_OUT_ELEVATOR:
		case MIX_OUT_RUDDER:
		case MIX_OUT_GYRO_GAIN:
		case MIX_OUT_PITCH:
		case MIX_OUT_AUX1:
		case MIX_OUT_AUX2:
			index = destination - MIX_OUT_AILERON;
			// Lookup where the function is mapped...
			index = g_Model.functionToServoTable[index];
			return g_RadioRuntime.srv_s[index];
			break;

		default:
			return 0;
	}
}

/*--------------------------------------------------------------------------------
 * mixer_get_condition
 *--------------------------------------------------------------------------------*/
int8_t mixer_get_condition(MIX_CONDITION cond)
{

	switch (cond)
	{
		case MIX_COND_TRUE:
			return 1;

		// Switches
		case MIX_COND_THR:
			return (hal_io_get_sw(SW_THR));
		case MIX_COND_NOT_THR:
			return (1 - hal_io_get_sw(SW_THR));
		case MIX_COND_RUDDR:
			return (hal_io_get_sw(SW_RUDDR));
		case MIX_COND_NOT_RUDDR:
			return (1 - hal_io_get_sw(SW_RUDDR));
		case MIX_COND_ELEDR:
			return (hal_io_get_sw(SW_ELEDR));
		case MIX_COND_NOT_ELEDR:
			return (1 - hal_io_get_sw(SW_ELEDR));
		case MIX_COND_AILDR:
			return (hal_io_get_sw(SW_AILDR));
		case MIX_COND_NOT_AILDR:
			return (1 - hal_io_get_sw(SW_AILDR));
		case MIX_COND_GEAR:
			return (hal_io_get_sw(SW_GEAR));
		case MIX_COND_NOT_GEAR:
			return (1 - hal_io_get_sw(SW_GEAR));
		case MIX_COND_ID0:
			return (hal_io_get_sw(SW_ID0));
		case MIX_COND_NOT_ID0:
			return (1 - hal_io_get_sw(SW_ID0));
		case MIX_COND_ID1:
			return (hal_io_get_sw(SW_ID1));
		case MIX_COND_NOT_ID1:
			return (1 - hal_io_get_sw(SW_ID1));
		case MIX_COND_ID2:
			return (hal_io_get_sw(SW_ID2));
		case MIX_COND_NOT_ID2:
			return (1 - hal_io_get_sw(SW_ID2));


		case MIX_COND_FALSE:
			return 0;

		default: 
			return 0;
	}
}


/*--------------------------------------------------------------------------------
 * mixer_get_curve_val
 *--------------------------------------------------------------------------------*/


int16_t mixer_get_curve_val(uint8_t curve, int16_t input)
{
	int16_t xa, xb;
	int16_t ya, yb;
	int32_t y;

	// Limit...
	if (input < -100)
	{
		input = -100;
	}
	if (input > 100)
	{
		input = 100;
	}

	// What segment	of the curve?
	if (input <= -50)
	{
		// [-100..-50]
		ya = CURVE(0, curve);
		yb = CURVE(1, curve);
		xa = -100;
		xb = -50;
	}
	else if ((input > -50) && (input <= 0))
	{
		// [-50..0]
		ya = CURVE(1, curve);
		yb = CURVE(2, curve);
		xa = -50;
		xb = 0;
	}
	else if ((input > 0) && (input <= 50))
	{
		// [0..50]
		ya = CURVE(2, curve);
		yb = CURVE(3, curve);
		xa = 0;
		xb = 50;
	}
	else if ((input > 50) && (input <= 100))
	{
		// [50..100]
		ya = CURVE(3, curve);
		yb = CURVE(4, curve);
		xa = 50;
		xb = 100;
	}
	else
	{
		// invalid...
		return 0;
	}

	// Interpolate between the points...
	y = ya + (yb - ya)*(input - xa)/(xb - xa);

	return (int16_t)y;
}

/*--------------------------------------------------------------------------------
 * mixer_mix
 *--------------------------------------------------------------------------------*/
void mixer_mix()
{
	uint8_t i,l;
	int16_t output;
	int16_t input;
	int16_t tempV;
	SMixer* mixer;

	for (i=0; i<MDL_MAX_CHANNELS; i++)
	{
		// Save values for servos
		servoSavedValue[i] = g_RadioRuntime.srv_s[i];
		servoChanged[i] = 0;
	
		// Clear values so mixers can do their job...
		g_RadioRuntime.srv_s[i] = 0;
	}

	for (l=0; l<2; l++)
	{
		for (i=0; i<MDL_MAX_MIXERS; i++)
		{
			mixer = &g_Model.mixers[l][i];

			// Only if the mixer is enabled by its condition...
			if (mixer_get_condition(mixer->condition))
			{
				// Get the output
				switch (mixer->type)
				{
					case MIX_DIRECT:
						// Output = Input*Scale;
						input = mixer_get_input(mixer->input);
						output = (input*mixer->scale)/100;
						break;
					case MIX_CURVE:
						// Output = curve[Input]*Scale;
						input = mixer_get_input(mixer->input);
						output = mixer_get_curve_val(mixer->curve, input);
						output = (output*mixer->scale)/100;
						break;
					default:
						output = 0;
						break;
				
				}


				// Multiplexing?
				switch (mixer->multiplex)
				{
					case MIX_ADD:
						tempV = mixer_get_output(mixer->output);
						output = tempV + output;
						break;
					case MIX_MULTIPLY:
						tempV = mixer_get_output(mixer->output);
						output = tempV * output;
						break;
					case MIX_REPLACE:
						// Nothing to do as we want: output = output;
						break;
					default:
						break;
				}
				// Now set the output from this mixer...
				mixer_set_output(mixer->output, output);
			}
		}
	}


	// If we did not change the values...restore em
	for (i=0; i<MDL_MAX_CHANNELS; i++)
	{
		if (servoChanged[i] == 0)
		{
			g_RadioRuntime.srv_s[i] = servoSavedValue[i];
		}

	}
}

