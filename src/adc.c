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

#include "adc.h"
#include "globals.h"
#include <avr/io.h>


/*--------------------------------------------------------------------------------
 * Defines & Macros
 *--------------------------------------------------------------------------------*/
#define ADC_VREF_TYPE 0x40


/*--------------------------------------------------------------------------------
 * gui_init
 *--------------------------------------------------------------------------------*/
 void adc_init(void)
 {
 	uint8_t adc_input=0;

	//adc initialization
	ADMUX=ADC_VREF_TYPE;
	ADCSRA=0x85;

	// Clear all values.
	for (adc_input=0;adc_input<7;adc_input++)
	{
		g_RadioRuntime.adc_r[adc_input] = 0;
	}

 }

/*--------------------------------------------------------------------------------
 * adc_sample
 *--------------------------------------------------------------------------------*/
void adc_sample(void)
{ 
	int32_t tempV;
	uint8_t adc_input=0;
	int16_t trim_c; // calibration center after trim
	uint8_t store_mid = g_RadioRuntime.adc_store_mid;

	g_RadioRuntime.adc_store_mid = 0;

	for (adc_input=0;adc_input<7;adc_input++)
	{
		ADMUX=adc_input|ADC_VREF_TYPE;
		// Start the AD conversion
		ADCSRA|=0x40;
		// Wait for the AD conversion to complete
		while ((ADCSRA & 0x10)==0);
		ADCSRA|=0x10;

		g_RadioRuntime.adc_r[adc_input]= ADCW;

		
		if (g_RadioRuntime.adc_store_end) // auto calibrate (extend borders)
		{	
			if (g_RadioRuntime.adc_r[adc_input]<g_RadioConfig.adc_c[adc_input][0])
			{
				g_RadioConfig.adc_c[adc_input][0]= g_RadioRuntime.adc_r[adc_input];

				//temp = (g_RadioConfig.adc_c[adc_input][2] - g_RadioConfig.adc_c[adc_input][0])/2;
				//temp = temp + g_RadioConfig.adc_c[adc_input][0];
				//g_RadioConfig.adc_c[adc_input][1] = temp;
			}
			if (g_RadioRuntime.adc_r[adc_input]>g_RadioConfig.adc_c[adc_input][2])
			{
				g_RadioConfig.adc_c[adc_input][2]= g_RadioRuntime.adc_r[adc_input];

				//temp = (g_RadioConfig.adc_c[adc_input][2] - g_RadioConfig.adc_c[adc_input][0])/2;
				//temp = temp + g_RadioConfig.adc_c[adc_input][0];
				//g_RadioConfig.adc_c[adc_input][1] = temp;
			}

		}

		if (store_mid) // auto calibrate (mid point once)
		{	
			g_RadioConfig.adc_c[adc_input][1] = g_RadioRuntime.adc_r[adc_input];
		}

		trim_c = g_RadioConfig.adc_c[adc_input][1]; // no change

		if (g_RadioRuntime.adc_r[adc_input]<trim_c) // lower half
		{ 
			tempV = ((int32_t)trim_c - (int32_t)g_RadioRuntime.adc_r[adc_input]);
			tempV = tempV * 100;
			tempV = tempV / ((int32_t)trim_c - (int32_t)g_RadioConfig.adc_c[adc_input][0]);

			g_RadioRuntime.adc_s[adc_input] = -(int16_t)tempV;
		} 
		else // upper half
		{ 
			tempV = ((int32_t)g_RadioRuntime.adc_r[adc_input] - (int32_t)trim_c);
			tempV = tempV * 100;
			tempV = tempV / ((int32_t)g_RadioConfig.adc_c[adc_input][2] - (int32_t)trim_c);

			g_RadioRuntime.adc_s[adc_input] = (int16_t)tempV;
		}

	}

	// battery reading

	ADMUX=7|ADC_VREF_TYPE;
	// Start the AD conversion
	ADCSRA|=0x40;
	// Wait for the AD conversion to complete
	while ((ADCSRA & 0x10)==0);
	ADCSRA|=0x10;

	g_RadioRuntime.adc_r[7] *=3;
	g_RadioRuntime.adc_r[7] += ADCW;
	g_RadioRuntime.adc_r[7] /= 4;
}
