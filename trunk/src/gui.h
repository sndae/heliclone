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

#ifndef _GUI_H_
#define _GUI_H_

#include <stdint.h>

typedef enum
{
	GUI_SCREEN_MODE_FOREVER = 0x0,
	GUI_SCREEN_MODE_1S,
	GUI_SCREEN_MODE_UNTIL_KEY
} GUI_SCREEN_MODE;

typedef enum
{
	GUI_EVT_NONE = 0x0,
	GUI_EVT_SHOW,
	GUI_EVT_HIDE,
	GUI_EVT_TICK,
	GUI_EVT_KEY_UP,
	GUI_EVT_KEY_UP_LONG,
	GUI_EVT_KEY_DOWN,
	GUI_EVT_KEY_DOWN_LONG,
	GUI_EVT_KEY_LEFT,
	GUI_EVT_KEY_LEFT_LONG,
	GUI_EVT_KEY_RIGHT,
	GUI_EVT_KEY_RIGHT_LONG,
	GUI_EVT_KEY_MENU,
	GUI_EVT_KEY_MENU_LONG,
	GUI_EVT_KEY_EXIT,
	GUI_EVT_KEY_EXIT_LONG,
	GUI_EVT_POT_MOVE,
} GUI_EVENT;


typedef enum
{
	KEY_UP = 0x0,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_MENU,
	KEY_EXIT,
	GUI_NUM_KEYS
} GUI_KEY;

typedef enum
{
	KEY_RELEASED,
	KEY_PRESSED,
	KEY_REPEAT,
	KEY_WAIT_FOR_RELEASE
} GUI_KEY_STATE;

#define GUI_POT (6)

typedef uint8_t (*GUI_SCREEN_FPTR)(GUI_EVENT event, uint8_t elapsedTime);

extern void gui_init();
extern void gui_screen_push(GUI_SCREEN_FPTR newScreen);
extern void gui_screen_pop();
extern void gui_execute(uint8_t elapsedTime);
extern void gui_handle_keys(uint8_t elapsedTime);
extern void gui_set_long_press(uint8_t longPress);

extern void gui_beep(uint8_t beepSeqence);


#endif // _GUI_H_
