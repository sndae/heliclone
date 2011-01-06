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


#include "ppm.h"
#include "globals.h"
#include "hal_io.h"
#include <avr\interrupt.h>

/*--------------------------------------------------------------------------------
 * Defines & Macros
 *--------------------------------------------------------------------------------*/

#define PPM_PRESCALER devPrescale_8 // CS11  // prescaler 8 from ATMega128 0> 2 pulses /us
#define PPM_TO_TICK   	2     		// for conversion

#define PPM_SYNC_TIME   (500*10*2)
#define	PPM_SPACE_TIME  (450*2)

#define	PPM_SIGNAL_MIN  (500*2)
#define	PPM_SIGNAL_MED  (1000*2)
#define	PPM_SIGNAL_MAX  (1500*2)

/*--------------------------------------------------------------------------------
 * LOCALS
 *--------------------------------------------------------------------------------*/
volatile uint8_t	ppm_current_ch; 		// last bit is signal
uint16_t	ppm_table[MDL_MAX_CHANNELS+1];	// signals to send + sync
uint8_t		ppm_max_chanel; 				// when to restart
uint8_t		ppm_critical=0; 				// critical section

// ppm frames per seconds
uint16_t ppm_tick=0;

uint8_t		ppm_mode	   = 0; // PPM - variable packet length
uint8_t		ppm_chanels    = 8;

/*--------------------------------------------------------------------------------
 * ppm_init
 *--------------------------------------------------------------------------------*/
void ppm_init() 
{
	uint8_t i = 0;

	ppm_current_ch = 0;
	ppm_max_chanel = (MDL_MAX_CHANNELS<<1)+1;

	// Fill up the ppm table with defaults
	for (i=0; i<MDL_MAX_CHANNELS; i++)
	{
		// Servos in the middle...
		ppm_table[i] = PPM_SIGNAL_MED;
	}
	ppm_table[MDL_MAX_CHANNELS] = PPM_SYNC_TIME; // 5 ms - sync

	// ppm out
	devPPMoutDRR |= (1<<devPPMoutPin);
	devPPMoutPORT &= ~(1<<devPPMoutPin); // PPM to 0 - singal start from pause

	// sim control
	DDRG  |=  0x10;
	PORTG &= ~0x10; // PG4 - SIM CONTROL, 0

	// TIMER1 (the PPM generator)
	// Output compare toggles OC1A pin
	// Start timer without prescaler
	TCCR1B = PPM_PRESCALER; //;
    TCCR1B |= (1 << WGM12); // Configure timer 1 for CTC mode Clear Timer on Compare
    TIMSK |= (1 << OCIE1A); // Enable CTC interrupt

	// Start TIMER1
	// first chanel pulse
    OCR1A = ppm_table[ppm_current_ch>>1];
}

/*--------------------------------------------------------------------------------
 * ppm_init
 *--------------------------------------------------------------------------------*/
void ppm_reinit() 
{
	ppm_max_chanel = (MDL_MAX_CHANNELS<<1)+1;
	ppm_table[MDL_MAX_CHANNELS] = PPM_SYNC_TIME; // 5 ms - sync
}



/*--------------------------------------------------------------------------------
 * TIMER 1 - Interrupt Service Routine
 *--------------------------------------------------------------------------------*/
ISR(TIMER1_COMPA_vect) 
{
	// Change to next channel
	ppm_current_ch++;

	// Make a PPM on the PIN by going up/down...
	// Always set "next time" in OCR1A reg 
	// to get timing of the PPM

    if (ppm_current_ch & 0x01) 
	{ 
		// Start of this channel...

		// PIN UP
		devPPMoutPORT |= (1<<devPPMoutPin);
		
		// set data to signal length
	    OCR1A = ppm_table[ppm_current_ch>>1]; 
	} 
	else 
	{ 
		// End of this channel...

		// PIN DOWN
		devPPMoutPORT &= ~(1<<devPPMoutPin);
		
		// Now load with "separation time" between channels
	    OCR1A = PPM_SPACE_TIME; 
	}

	// Start all over again...
	if (ppm_current_ch > ppm_max_chanel) 
	{
		ppm_current_ch = 0;
		ppm_tick++;
	}
}

/*--------------------------------------------------------------------------------
 * ppm_export
 *--------------------------------------------------------------------------------*/
void ppm_export() 
{
	uint8_t i;
	int16_t currentServo;
	uint16_t nextValue;
	int32_t tempV;

	for (i=0; i<MDL_MAX_CHANNELS; i++) 
	{
		// NOTE: The srv_s[] table holds the wanted output for each servo channel.
		// This value is [-100...+100]
		// The ppm_table[] will (after this function is called) hold the pulse-times
		// for the pulse position modulation. It will be scaled according to:
		// PPM_SIGNAL_MIN...PPM_SIGNAL_MED...PPM_SIGNAL_MAX
		// [500...1000...1500] us
		// So... srv_s[]=0     =>  ppm_table[]=1000*2
		//       srv_s[]=-100  =>  ppm_table[]=500*2
		//       srv_s[]=+100  =>  ppm_table[]=1500*2
		// This is what this function will try to calculate... :)

		currentServo = g_RadioRuntime.srv_s[i];

		// Apply wanted Subtrims
		currentServo += g_Model.subTrim[i];

		// Direction INVERSED?
		if ((g_Model.servoDirection & (1 << i)) == (1 << i))
		{
			currentServo = -currentServo;	
		}

		if (currentServo < 0)
		{
			// Range: [-100...0]

			// No overdrive...clip to min value
			if (currentServo < -100) 
			{
				currentServo = -100;
				nextValue = PPM_SIGNAL_MIN;
			} 
			else
			{
				tempV = currentServo*((int32_t)PPM_SIGNAL_MED - (int32_t)PPM_SIGNAL_MIN);
				tempV = tempV / 100;
				nextValue = (uint16_t)PPM_SIGNAL_MED + (uint16_t)tempV;
			}
		}
		else
		{
			// Range: [0...+100]

			// No overdrive...clip to max value
			if (currentServo > 100) 
			{
				currentServo = 100;
				nextValue = PPM_SIGNAL_MAX;
			} 
			else
			{
				tempV = currentServo*((int32_t)PPM_SIGNAL_MAX - (int32_t)PPM_SIGNAL_MED);
				tempV = tempV / 100;
				nextValue = (uint16_t)PPM_SIGNAL_MED + (uint16_t)tempV;
			}
		}

		// Semaphorish...
		// make sure that value will be no accessed
		while (((ppm_current_ch & 0x01)==0) && (ppm_current_ch >> 1) == i) 
		{
		}; // wait when channel will be not accessed by interrupt

		ppm_table[i] = nextValue;
	}
	
}
