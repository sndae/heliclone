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
 * mixer_init
 *--------------------------------------------------------------------------------*/
void mixer_init()
{
}

/*--------------------------------------------------------------------------------
 * mixer_input
 *--------------------------------------------------------------------------------*/
int8_t mixer_input(MIX_INPUT source)
{
	uint8_t index;
	switch (source)
	{
		case MIX_IN_AIL:
		case MIX_IN_THR:
		case MIX_IN_ELE:
		case MIX_IN_RUD:
		case MIX_IN_POT1:
		case MIX_IN_POT2:
		case MIX_IN_POT3:
			index = source - MIX_IN_AIL;
			return g_RadioRuntime.adc_s[index];

		default:
			return 0;
	}
}

/*--------------------------------------------------------------------------------
 * mixer_output
 *--------------------------------------------------------------------------------*/
void mixer_output(MIX_OUTPUT destination, int8_t value)
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

		default:
			return;
	}
}


/*--------------------------------------------------------------------------------
 * mixer_mix
 *--------------------------------------------------------------------------------*/
void mixer_mix()
{
	uint8_t i,l;
	SMixer* mixer;

	for (l=0; l<2; l++)
	{
		for (i=0; i<MDL_MAX_MIXERS; i++)
		{
			mixer = &g_Model.mixers[l][i];


		}
	}
}

