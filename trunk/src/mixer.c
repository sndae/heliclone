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
		case MIX_COND_NOT_THR:
		case MIX_COND_RUDDR:
		case MIX_COND_NOT_RUDDR:
		case MIX_COND_ELEDR:
		case MIX_COND_NOT_ELEDR:
		case MIX_COND_AILDR:
		case MIX_COND_NOT_AILDR:
		case MIX_COND_GEAR:
		case MIX_COND_NOT_GEAR:
		case MIX_COND_ID0:
		case MIX_COND_NOT_ID0:
		case MIX_COND_ID1:
		case MIX_COND_NOT_ID1:
		case MIX_COND_ID2:
		case MIX_COND_NOT_ID2:
			return 0;

		case MIX_COND_FALSE:
			return 0;

		default: 
			return 0;
	}
}


/*--------------------------------------------------------------------------------
 * mixer_mix
 *--------------------------------------------------------------------------------*/

void mixer_mix()
{
	uint8_t i,l;
	SMixer* mixer;
	int16_t output;
	int16_t input;
	int16_t tempV;

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

