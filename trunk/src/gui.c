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

#include "gui.h"
#include "lcd.h"
#include "globals.h"
#include "hal_io.h"
#include <avr/pgmspace.h>
#include <string.h>
#include <stdlib.h>

/*--------------------------------------------------------------------------------
 * Defines & Macros
 *--------------------------------------------------------------------------------*/
#define GUI_STACK_SIZE (8)

// keys masks
#define KEY_MASK_MENU  0x02
#define KEY_MASK_EXIT  0x04
#define KEY_MASK_DOWN  0x08
#define KEY_MASK_UP    0x10
#define KEY_MASK_RIGHT 0x20
#define KEY_MASK_LEFT  0x40

// 100 = 1 second
// 10 = 0.1 second
#define KEY_LONG_PRESSED_TIME (100)
#define KEY_REPEAT_TIME (40)
#define KEY_PRESSED_TIME (10)
#define KEY_REPEAT_PRESS_TIME (30)

#define EVENT_QUEUE_LEN (6)

/*--------------------------------------------------------------------------------
 * LOCALS
 *--------------------------------------------------------------------------------*/
GUI_SCREEN_FPTR gui_stack[GUI_STACK_SIZE];
uint8_t  gui_stackPtr = 0;

// Ten keys
GUI_KEY_STATE keyState[GUI_NUM_KEYS];
uint16_t keyTicks[GUI_NUM_KEYS];
uint8_t longPressEnabled = 1;

// Event queue
GUI_EVENT eventQueue[EVENT_QUEUE_LEN];
uint8_t eventQueueWritePtr;
uint8_t eventQueueReadPtr;

int16_t lastNextPrevious = 0;

/*--------------------------------------------------------------------------------
 * Constants
 *--------------------------------------------------------------------------------*/
const uint8_t keyMasks[6] PROGMEM = 
	{	
		KEY_MASK_UP, 
		KEY_MASK_DOWN, 
		KEY_MASK_LEFT, 
		KEY_MASK_RIGHT, 
		KEY_MASK_MENU, 
		KEY_MASK_EXIT 
	};

const uint8_t keyEventShort[6] PROGMEM = 
	{
		GUI_EVT_KEY_UP,
		GUI_EVT_KEY_DOWN,
		GUI_EVT_KEY_LEFT,
		GUI_EVT_KEY_RIGHT,
		GUI_EVT_KEY_MENU,
		GUI_EVT_KEY_EXIT
	};

const uint8_t keyEventLong[6] PROGMEM = 
	{
		GUI_EVT_KEY_UP_LONG,
		GUI_EVT_KEY_DOWN_LONG,
		GUI_EVT_KEY_LEFT_LONG,
		GUI_EVT_KEY_RIGHT_LONG,
		GUI_EVT_KEY_MENU_LONG,
		GUI_EVT_KEY_EXIT_LONG
	};


/*--------------------------------------------------------------------------------
 * gui_init
 *--------------------------------------------------------------------------------*/
void gui_init()
{
	memset(gui_stack, 0, sizeof(gui_stack));
	gui_stackPtr = 0;

	memset(keyState, KEY_RELEASED, sizeof(keyState));
	memset(keyTicks, 0, sizeof(keyTicks));

	memset(eventQueue, GUI_EVT_NONE, sizeof(eventQueue));
	eventQueueWritePtr = 0;
	eventQueueReadPtr = 0;

	lastNextPrevious = 0;
}


/*--------------------------------------------------------------------------------
 * gui_event_put
 *--------------------------------------------------------------------------------*/
void gui_event_put(GUI_EVENT newEvent)
{
	eventQueue[eventQueueWritePtr] = newEvent;
	eventQueueWritePtr++;	
	if (eventQueueWritePtr >= EVENT_QUEUE_LEN)
	{
		eventQueueWritePtr = 0;
	}
}

/*--------------------------------------------------------------------------------
 * gui_event_get
 *--------------------------------------------------------------------------------*/
GUI_EVENT gui_event_get()
{
	GUI_EVENT nextEvent = GUI_EVT_NONE; 
	if (eventQueueReadPtr == eventQueueWritePtr)
	{
		return nextEvent;
	}

	nextEvent = eventQueue[eventQueueReadPtr];
	eventQueue[eventQueueReadPtr] = GUI_EVT_NONE;

	eventQueueReadPtr++;	
	if (eventQueueReadPtr >= EVENT_QUEUE_LEN)
	{
		eventQueueReadPtr = 0;
	}

	return nextEvent;
}
/*--------------------------------------------------------------------------------
 * gui_screen_push
 *--------------------------------------------------------------------------------*/
void gui_screen_push(GUI_SCREEN_FPTR newScreen)
{
	uint8_t dirty = 0;

	gui_stackPtr++;
	if(gui_stackPtr >= GUI_STACK_SIZE)
	{
		//TODO: How to handle this error?
		gui_stackPtr--;
		return;
	}

	gui_stack[gui_stackPtr] = newScreen;
	dirty = (*newScreen)(GUI_EVT_SHOW, 0);

	if (dirty)
	{
		lcd_refresh();
	}
}

/*--------------------------------------------------------------------------------
 * gui_screen_pop
 *--------------------------------------------------------------------------------*/
void gui_screen_pop()
{
	uint8_t dirty = 0;

	if(gui_stackPtr > 0)
	{
		dirty = (*gui_stack[gui_stackPtr])(GUI_EVT_HIDE, 0);

		gui_stackPtr--;
	}
	else
	{
		//TODO: How to handle?
	}

	if (dirty)
	{
		lcd_refresh();
	}

}

/*--------------------------------------------------------------------------------
 * gui_handle_keys
 *--------------------------------------------------------------------------------*/

void gui_handle_keys(uint8_t elapsedTime)
{
	uint8_t k = 0;
	uint8_t kstatus = 0;
	uint8_t* maskPtr = 0;
	uint8_t mask = 0;

	k = 0;
	kstatus = 0;
	maskPtr = (uint8_t*)&keyMasks[0];
	mask = 0;

	// Read key status
	kstatus = (~devKeysPIN) & devKeysMask;


	for (k = 0; k<GUI_NUM_KEYS; k++)
	{
		mask = pgm_read_byte(maskPtr);

		// Tick time in the current state...
		keyTicks[k] += (elapsedTime/10);

		switch (keyState[k])
		{
			case KEY_RELEASED:
				if (kstatus & mask)
				{
					// Must have been released for a while before we can
					// press it...avoid "double clicks"
					if ((keyTicks[k] >= KEY_PRESSED_TIME))
					{
						keyState[k] = KEY_PRESSED;
						keyTicks[k] = 0;
					}
				}

				break;
			case KEY_PRESSED:
				if (kstatus & mask)
				{
					if ((longPressEnabled == 1) && (keyTicks[k] >= KEY_LONG_PRESSED_TIME))
					{
						// long press
						gui_event_put(pgm_read_byte(&keyEventLong[k]));
						keyState[k] = KEY_WAIT_FOR_RELEASE;
						keyTicks[k] = 0;
					}
					else if ((longPressEnabled == 0) && (keyTicks[k] >= KEY_REPEAT_TIME))
					{
						// move to "Repeat state"?
						gui_event_put(pgm_read_byte(&keyEventShort[k]));
						keyState[k] = KEY_REPEAT;
					}
				}
				else
				{
					if (keyTicks[k] >= KEY_PRESSED_TIME)
					{
						// We have pressed less than REPEAT, but more than KEY
						// i.e. let's send out a key event!
						gui_event_put(pgm_read_byte(&keyEventShort[k]));
						keyState[k] = KEY_RELEASED;
						keyTicks[k] = 0;
					}
					else
					{
						// Nada...too short...return to released
						keyState[k] = KEY_RELEASED;
						keyTicks[k] = 0;
					}
				}
				break;
			case KEY_REPEAT:
				if (kstatus & mask)
				{
					// Send repeat key_press...
					if (keyTicks[k] % KEY_REPEAT_PRESS_TIME == 0)
					{
						gui_event_put(pgm_read_byte(&keyEventShort[k]));
					}
				}
				else
				{
					// Repeat mode released, back to released state
					keyState[k] = KEY_RELEASED;
					keyTicks[k] = 0;
				}
				break;
			case KEY_WAIT_FOR_RELEASE:
				if (!(kstatus & mask))
				{
					keyState[k] = KEY_RELEASED;
					keyTicks[k] = 0;
				}
				break;
			default:
				break;
		}

		// Next keymask
		maskPtr++;
	}


	// Use the ADC from POT7 to generate POT_MOVE events...for easier GUI.
	if (g_RadioRuntime.adc_s[GUI_POT] != lastNextPrevious)
	{
		gui_event_put(GUI_EVT_POT_MOVE);
	}
	lastNextPrevious = g_RadioRuntime.adc_s[GUI_POT];

}

/*--------------------------------------------------------------------------------
 * gui_execute
 *--------------------------------------------------------------------------------*/
void gui_execute(uint8_t elapsedTime)
{
	uint8_t dirty = 0;
	GUI_EVENT event = GUI_EVT_NONE;

	// LCD Drawing of selected screen
	if (gui_stack[gui_stackPtr] != 0)
	{
		dirty |= (*gui_stack[gui_stackPtr])(GUI_EVT_TICK, elapsedTime);
	}

	// Also...check if any event, then send this to the screen also...
	event = gui_event_get();
	if (event != GUI_EVT_NONE)
	{
		if (gui_stack[gui_stackPtr] != 0)
		{
			dirty |= (*gui_stack[gui_stackPtr])(event, elapsedTime);
		}
	}

	if (dirty)
	{
		lcd_refresh();
	}
}

/*--------------------------------------------------------------------------------
 * gui_set_long_press
 *--------------------------------------------------------------------------------*/
void gui_set_long_press(uint8_t longPress)
{
	longPressEnabled = longPress;
}
