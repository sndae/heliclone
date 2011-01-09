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

#include "hal_io.h"
#include "globals.h"

//#define DEBUG_SWITCHES

#ifdef DEBUG_SWITCHES
#ifdef USE_DEBUG_MODE
#include <string.h>
#endif
#endif

/*--------------------------------------------------------------------------------
 * Defines & Macros
 *--------------------------------------------------------------------------------*/

#define TRIM_NUM_KEYS (8)

typedef enum
{
	TRIM_KEY_RELEASED,
	TRIM_KEY_PRESSED,
	TRIM_KEY_REPEAT
} TRIM_KEY_STATE;

// 100 = 1 second
// 10 = 0.1 second
#define TRIM_KEY_REPEAT_TIME (50)
#define TRIM_KEY_PRESSED_TIME (10)
#define TRIM_KEY_REPEAT_PRESS_TIME (30)

#define TRIM_STEP (4)


/*--------------------------------------------------------------------------------
 * LOCALS
 *--------------------------------------------------------------------------------*/
// 8 trim keys
TRIM_KEY_STATE trimKeyState[TRIM_NUM_KEYS];
uint16_t trimKeyTicks[TRIM_NUM_KEYS];

// Holds a BIT-field of all switches
uint8_t switchState;


/*--------------------------------------------------------------------------------
 * io_init
 *--------------------------------------------------------------------------------*/
void hal_io_init(void)
{

    // Init of the PORTS (direction etc)
	devTrimsDRR     |= devTrimsMask;
	devKeysDRR      |= devKeysMask;
	devSwitches1DRR |= devSwitches1Mask;
	devSwitches2DRR |= devSwitches2Mask;

	devTrimsPORT    |= devTrimsMask;
	devKeysPORT     |= devKeysMask;
	devSwitches1PORT|= devSwitches1Mask;
	devSwitches2PORT|= devSwitches2Mask;

	devTrimsDRR     &= ~devTrimsMask;
	devKeysDRR      &= ~devKeysMask;
	devSwitches1DRR &= ~devSwitches1Mask;
	devSwitches2DRR &= ~devSwitches2Mask;

	// beep
	devBeepDRR  |=    1<<devBeepPin;
	devBeepPORT &=  ~(1<<devBeepPin); // <-0

	// lcd backlight
	devBacklightDRR  |=   1<<devBacklightPin;
	devBacklightPORT |=   1<<devBacklightPin; // <-1


    devKeysDRR  = 0x81;
    devKeysPORT = 0x7e;                             //pullups keys+nc

    devSwitches1DRR  = 0x08;
    devSwitches1PORT = 0xff - (1 << devBeepPin);    //pullups + buzzer 0

    devSwitches2DRR  = 0x10;
    devSwitches2PORT = 0xff;                        //pullups + SIM_CTL=1 = phonejack = ppm_in

}

/*--------------------------------------------------------------------------------
 * hal_io_get_sw()
 *--------------------------------------------------------------------------------*/
uint8_t hal_io_get_sw(uint8_t swId)
{
	uint8_t temp;
	switch (swId)
	{
		case SW_ID0:
			temp = switchState & 0xC0;
			if (temp == 0x80)
			{
				return 1;
			}
			else
			{
				return 0;
			}
			break;
		case SW_ID1:
			temp = switchState & 0xC0;
			if (temp == 0x00)
			{
				return 1;
			}
			else
			{
				return 0;
			}
			break;
		case SW_ID2:
			temp = switchState & 0xC0;
			if (temp == 0x40)
			{
				return 1;
			}
			else
			{
				return 0;
			}
			break;
		case SW_AILDR:
			temp = switchState & 0x02;
			return (temp == 0x02);
			break;
		case SW_ELEDR:
			temp = switchState & 0x04;
			return (temp == 0x04);
			break;
		case SW_RUDDR:
			temp = switchState & 0x08;
			return (temp == 0x08);
			break;
		case SW_TRN:
			temp = switchState & 0x20;
			return (temp == 0x20);
			break;
		case SW_GEAR:
			temp = switchState & 0x10;
			return (temp == 0x10);
			break;
		case SW_THR:
			temp = switchState & 0x01;
			// THR sw is reversed...
			return (temp == 0x00);
			break;
		default:
			return 0;
			break;
	}
}

/*--------------------------------------------------------------------------------
 * hal_io_trim_pressed
 *--------------------------------------------------------------------------------*/
void hal_io_trim_pressed(uint8_t key)
{
	uint8_t trimPos = key >> 1;

	// Up or down trim...
	
	if (key >= 2)
	{
		// TRIMS 1,2,3 standard
		if (key % 2 == 0)
		{
			g_Model.trim[trimPos] -= TRIM_STEP;
		}
		else
		{
			g_Model.trim[trimPos] += TRIM_STEP;
		}
	}
	else
	{
		// TRIM 0 reversed
		if (key % 2 == 0)
		{
			g_Model.trim[trimPos] += TRIM_STEP;
		}
		else
		{
			g_Model.trim[trimPos] -= TRIM_STEP;
		}
	}

	// Limit
	if (g_Model.trim[trimPos] > 100)
	{
		g_Model.trim[trimPos] = 100;
	}
	if (g_Model.trim[trimPos] < -100)
	{
		g_Model.trim[trimPos] = -100;
	}

}

/*--------------------------------------------------------------------------------
 * hal_io_handle
 *--------------------------------------------------------------------------------*/
void hal_io_handle(uint8_t elapsedTime)
{
	uint8_t t = 0;
	uint8_t tstatus = 0;
	uint8_t mask = 0;
	uint8_t button;

	//
	// TRIMS
	//

	// Read trim (button) status
	tstatus = (~devTrimsPIN) & devTrimsMask;

	// Four trims...with two buttons each...
	for (t = 0; t<TRIM_NUM_KEYS; t++)
	{
		mask = (1 << t);

		// Tick time in the current state...
		trimKeyTicks[t] += (elapsedTime/10);

		switch (trimKeyState[t])
		{
			case TRIM_KEY_RELEASED:
				if (tstatus & mask)
				{
					// Must have been released for a while before we can
					// press it...avoid "double clicks"
					if ((trimKeyTicks[t] >= TRIM_KEY_PRESSED_TIME))
					{
						trimKeyState[t] = TRIM_KEY_PRESSED;
						trimKeyTicks[t] = 0;
					}
				}
				break;
			case TRIM_KEY_PRESSED:
				if (tstatus & mask)
				{
					// move to "Repeat state"?
					if (trimKeyTicks[t] >= TRIM_KEY_REPEAT_TIME)
					{
						hal_io_trim_pressed(t);
						trimKeyState[t] = TRIM_KEY_REPEAT;
					}
				}
				else
				{
					if (trimKeyTicks[t] >= TRIM_KEY_PRESSED_TIME)
					{
						// We have pressed less than REPEAT, but more than KEY
						// i.e. let's send out a key event!
						hal_io_trim_pressed(t);
						trimKeyState[t] = TRIM_KEY_RELEASED;
						trimKeyTicks[t] = 0;
					}
					else
					{
						// Nada...too short...
						trimKeyState[t] = TRIM_KEY_RELEASED;
						trimKeyTicks[t] = 0;
					}
				}
				break;
			case TRIM_KEY_REPEAT:
				if (tstatus & mask)
				{
					// Send repeat key_press...
					if (trimKeyTicks[t] % TRIM_KEY_REPEAT_PRESS_TIME == 0)
					{
						hal_io_trim_pressed(t);
					}
				}
				else
				{
					// Repeat mode released, back to released state
					trimKeyState[t] = TRIM_KEY_RELEASED;
					trimKeyTicks[t] = 0;
				}
				break;
			default:
				break;
		}
	}

	//
	// SWITCHES
	//

	// Sample the ports and merge the results into sw
	switchState = (~devSwitches1PIN) & devSwitches1Mask;
	button = (~devSwitches2PIN);
	switchState |= (button & 0x08)<<4;
	switchState |= (button & 0x01)<<3;

#ifdef DEBUG_SWITCHES
#ifdef USE_DEBUG_MODE
	strcpy(debugLine1, "SW: ");
	for (uint8_t i=0; i<8; i++)
	{
		if ((switchState & (1 << i)) == (1 << i))
		{
			strcat(debugLine1, "1");
		}
		else
		{
			strcat(debugLine1, "0");
		}
	}
#endif	
#endif


}
