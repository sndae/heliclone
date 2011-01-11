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

#include "menu.h"
#include "gui.h"
#include "lcd.h"
#include "adc.h"
#include "eeprom.h"
#include "globals.h"
#include "hal_io.h"
#include <avr/pgmspace.h>
#include <string.h>


/*--------------------------------------------------------------------------------
 * Prototypes
 *--------------------------------------------------------------------------------*/
uint8_t menu_adc_calibrate(GUI_EVENT event, uint8_t elapsedTime);
uint8_t menu_main_screen(GUI_EVENT event, uint8_t elapsedTime);
uint8_t menu_main_menu(GUI_EVENT event, uint8_t elapsedTime);

uint8_t menu_get_setting(uint8_t parameterId);
void menu_set_setting(uint8_t parameterId, uint8_t newValue);

// Menus
uint8_t menu_model_config(GUI_EVENT event, uint8_t elapsedTime);
uint8_t menu_radio_config(GUI_EVENT event, uint8_t elapsedTime);
uint8_t menu_radio_install(GUI_EVENT event, uint8_t elapsedTime);


/*--------------------------------------------------------------------------------
 * LOCALS
 *--------------------------------------------------------------------------------*/

#define MNU_ID_PITCH_CURVE (0x12)
#define MNU_ID_THROTTLE_CURVE (0x13)

typedef enum
{
	//
	// Radio params
	//
	RC_SET_VOLTAGE,
	RC_SET_BACKLIGHT,
	RC_SET_BEEP_KEYS,
	RC_SET_BEEP_ALARMS,

	//
	// Model params
	//
	MC_SET_DIR_CH1,
	MC_SET_DIR_CH2,
	MC_SET_DIR_CH3,
	MC_SET_DIR_CH4,
	MC_SET_DIR_CH5,
	MC_SET_DIR_CH6,
	MC_SET_DIR_CH7,
	MC_SET_DIR_CH8,

	MC_SET_SUBT_CH1,
	MC_SET_SUBT_CH2,
	MC_SET_SUBT_CH3,
	MC_SET_SUBT_CH4,
	MC_SET_SUBT_CH5,
	MC_SET_SUBT_CH6,
	MC_SET_SUBT_CH7,
	MC_SET_SUBT_CH8
} PARAMETER_ID;

typedef struct 
{
	uint8_t id;
	char* text;
	char* alternatives;
	uint8_t parameterId;
	GUI_SCREEN_FPTR screen;
} SSelection;


SSelection* currentSettings;
uint8_t numSettings = 0;
int8_t cursorSetting = -1;
uint8_t settingValue = -1;
uint8_t changedModel = 0;
int8_t cursor = 0;
int8_t cursor2 = 0;

/*--------------------------------------------------------------------------------
 * menu_init
 *--------------------------------------------------------------------------------*/
void menu_init()
{
	// Setup the first screen...
	gui_screen_push(&menu_main_screen);

	// Need calib?
	if (g_RadioRuntime.adc_store_end)
	{
		gui_screen_push(&menu_adc_calibrate);
	}
	
}


/*--------------------------------------------------------------------------------
 * menu_model_name_edit
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_NAME_EDIT_TITLE[] 		PROGMEM = "Model Name";
char MNU_MODEL_NAME_EDIT[] 				PROGMEM = "Name:";

uint8_t menu_model_name_edit(GUI_EVENT event, uint8_t elapsedTime)
{
	uint8_t i;
	uint8_t x,y;

	switch (event)
	{
		case GUI_EVT_SHOW:
			cursor = 0;
			cursor2 = g_Model.name[cursor];
			break;
		case GUI_EVT_HIDE:
			break;
		case GUI_EVT_KEY_EXIT:
			// We are done...restore?
			if (changedModel == 1)
			{
				eeprom_load_model_config(g_RadioConfig.selectedModel);
			}
			gui_screen_pop();
			break;
		case GUI_EVT_KEY_MENU:
			// We are done...Save?
			if (changedModel == 1)
			{
				eeprom_save_model_config(g_RadioConfig.selectedModel);
			}
			gui_screen_pop();
			break;
		case GUI_EVT_KEY_UP:
			cursor2++;
			if (cursor2 > 122)
			{
				cursor2 = 32;
			}
			g_Model.name[cursor] = (char)cursor2;
			changedModel = 1;
			break;
		case GUI_EVT_KEY_DOWN:
			cursor2--;
			if (cursor2 < 32)
			{
				cursor2 = 122;
			}
			g_Model.name[cursor] = (char)cursor2;
			changedModel = 1;
			break;
		case GUI_EVT_POT_MOVE:
			cursor2 = (int8_t)g_RadioRuntime.adc_s[GUI_POT]/2 + 80;
			if (cursor2 < 32)
			{
				cursor2 = 32;
			}
			if (cursor2 > 122)
			{
				cursor2 = 122;
			}
			g_Model.name[cursor] = (char)cursor2;
			changedModel = 1;
			break;
		case GUI_EVT_KEY_RIGHT:
			cursor++;
			if (cursor > 10)
			{
				cursor = 0;
			}
			break;
		case GUI_EVT_KEY_LEFT:
			cursor--;
			if (cursor < 0)
			{
				cursor = 9;
			}
			break;
		default:
			break;
	}

	lcd_clear();
	lcd_puts_P( 0, 0, MNU_MODEL_NAME_EDIT_TITLE);
	lcd_puts_P( 0, 2*LCD_FONT_HEIGHT, MNU_MODEL_NAME_EDIT);

	x = 6;
	y = 2;
	for (i=0; i<10; i++)
	{
		if (cursor == i)
		{
			lcd_putcAtt(x*LCD_FONT_WIDTH, y*LCD_FONT_HEIGHT, g_Model.name[i], LCD_INVERS, 0);
		}
		else
		{
			lcd_putcAtt(x*LCD_FONT_WIDTH, y*LCD_FONT_HEIGHT, g_Model.name[i], LCD_NO_INV, 0);
		}
		x++;
	}

	return 1;
}


/*--------------------------------------------------------------------------------
 * menu_message_box
 *--------------------------------------------------------------------------------*/
char* msgbox_title = 0;
char* msgbox_row1 = 0;
char* msgbox_row2 = 0;
char* msgbox_row3 = 0;
char* msgbox_row4 = 0;
uint8_t msgbox_countdown = 0;

char MNU_MSGBOX_CLEAR[] 	PROGMEM = "                   ";

uint8_t menu_message_box(GUI_EVENT event, uint8_t elapsedTime)
{
	uint8_t x,y;
	uint8_t dirty = 0;

	switch (event)
	{
		case GUI_EVT_SHOW:
			msgbox_countdown = g_RadioConfig.message_box_timeout;

			lcd_putsAtt( 1*LCD_FONT_WIDTH, 1*LCD_FONT_HEIGHT, MNU_MSGBOX_CLEAR, LCD_NO_INV);
			lcd_putsAtt( 1*LCD_FONT_WIDTH, 2*LCD_FONT_HEIGHT, MNU_MSGBOX_CLEAR, LCD_NO_INV);
			lcd_putsAtt( 1*LCD_FONT_WIDTH, 3*LCD_FONT_HEIGHT, MNU_MSGBOX_CLEAR, LCD_NO_INV);
			lcd_putsAtt( 1*LCD_FONT_WIDTH, 4*LCD_FONT_HEIGHT, MNU_MSGBOX_CLEAR, LCD_NO_INV);
			lcd_putsAtt( 1*LCD_FONT_WIDTH, 5*LCD_FONT_HEIGHT, MNU_MSGBOX_CLEAR, LCD_NO_INV);
			lcd_putsAtt( 1*LCD_FONT_WIDTH, 6*LCD_FONT_HEIGHT, MNU_MSGBOX_CLEAR, LCD_NO_INV);

			// Draw the messagebox
			if (msgbox_title)
			{
				lcd_putsAtt( 1*LCD_FONT_WIDTH, 1*LCD_FONT_HEIGHT, msgbox_title, LCD_INVERS);
			}

			if (msgbox_row1)
			{
				lcd_putsAtt( 1*LCD_FONT_WIDTH, 3*LCD_FONT_HEIGHT, msgbox_row1, LCD_NO_INV);
			}

			if (msgbox_row2)
			{
				lcd_putsAtt( 1*LCD_FONT_WIDTH, 4*LCD_FONT_HEIGHT, msgbox_row2, LCD_NO_INV);
			}

			if (msgbox_row3)
			{
				lcd_putsAtt( 1*LCD_FONT_WIDTH, 5*LCD_FONT_HEIGHT, msgbox_row3, LCD_NO_INV);
			}

			if (msgbox_row4)
			{
				lcd_putsAtt( 1*LCD_FONT_WIDTH, 6*LCD_FONT_HEIGHT, msgbox_row4, LCD_NO_INV);
			}

			// Messagebox border
			y = 1*LCD_FONT_HEIGHT-1;
			for (x=1*LCD_FONT_WIDTH-1; x<(1*LCD_FONT_WIDTH-1 + 19*LCD_FONT_WIDTH+1); x++)
			{
				lcd_plot(x,y);
			}
			y = 7*LCD_FONT_HEIGHT;
			for (x=1*LCD_FONT_WIDTH-1; x<(1*LCD_FONT_WIDTH-1 + 19*LCD_FONT_WIDTH+1); x++)
			{
				lcd_plot(x,y);
			}
			x = 1*LCD_FONT_WIDTH-1;
			for (y=1*LCD_FONT_HEIGHT-1; y<(1*LCD_FONT_HEIGHT-1 + 6*LCD_FONT_HEIGHT+1); y++)
			{
				lcd_plot(x,y);
			}
			x = 20*LCD_FONT_WIDTH;
			for (y=1*LCD_FONT_HEIGHT-1; y<(1*LCD_FONT_HEIGHT-1 + 6*LCD_FONT_HEIGHT+1); y++)
			{
				lcd_plot(x,y);
			}
			dirty = 1;

			break;
		case GUI_EVT_HIDE:
			break;
		case GUI_EVT_TICK:
			msgbox_countdown--;
			// Remove the messagebox?
			if (msgbox_countdown == 0)
			{
				// Remove us...
				gui_screen_pop();
			}
			break;
		case GUI_EVT_KEY_UP:
		case GUI_EVT_KEY_UP_LONG:
		case GUI_EVT_KEY_DOWN:
		case GUI_EVT_KEY_DOWN_LONG:
		case GUI_EVT_KEY_LEFT:
		case GUI_EVT_KEY_LEFT_LONG:
		case GUI_EVT_KEY_RIGHT:
		case GUI_EVT_KEY_RIGHT_LONG:
		case GUI_EVT_KEY_MENU:
		case GUI_EVT_KEY_MENU_LONG:
		case GUI_EVT_KEY_EXIT:
		case GUI_EVT_KEY_EXIT_LONG:
			// All other key events close the message box...
			gui_screen_pop();
			break;
		default:
			break;
	}

	return dirty;
}

void menu_show_messagebox(char *title, char *row1, char *row2, char* row3, char* row4)
{
	// Store pointers
	msgbox_title = title;
	msgbox_row1 = row1;
	msgbox_row2 = row2;
	msgbox_row3 = row3;
	msgbox_row4 = row4;

	// invoke the msgbox on the stack

	gui_screen_push(&menu_message_box);
}


/*--------------------------------------------------------------------------------
 * menu_model_curve_edit
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_PITCH_CURVE_TITLE[] 		PROGMEM = "Pitch Curves";
char MNU_MODEL_THROTTLE_CURVE_TITLE[] 	PROGMEM = "Throttle Curves";

char MNU_MODEL_PITCH_CURVE[] 			PROGMEM = "PIT";
char MNU_MODEL_THROTTLE_CURVE[] 		PROGMEM = "THR";

char MNU_MODEL_CURVE_ID0[] 	PROGMEM = "ID0";
char MNU_MODEL_CURVE_ID1[] 	PROGMEM = "ID1";
char MNU_MODEL_CURVE_ID2[] 	PROGMEM = "ID2";
char MNU_MODEL_CURVE_HLD[] 	PROGMEM = "HOLD";

uint8_t menu_model_curve_edit(GUI_EVENT event, uint8_t elapsedTime)
{
	char* menuTitle;
	uint8_t menuId;
	uint8_t i, c = 0;
	char* modeStr = MNU_MODEL_CURVE_ID0;
	int16_t x1,y1,x2,y2;
	int16_t px1,py1,px2,py2;
	SSelection* selection;

	// Keep track of PITCH or THROTTLE curves???
	selection = (SSelection*)&currentSettings[cursorSetting];
	menuId = (uint8_t)pgm_read_byte(&selection->id);
	if (menuId == MNU_ID_PITCH_CURVE)
	{
		menuTitle = MNU_MODEL_PITCH_CURVE;

		if (hal_io_get_sw(SW_ID0))
		{
			c = MDL_CURVE_PIT_ID0;
			modeStr = MNU_MODEL_CURVE_ID0;
		}
		else if (hal_io_get_sw(SW_ID1))
		{
			c = MDL_CURVE_PIT_ID1;
			modeStr = MNU_MODEL_CURVE_ID1;
		}
		else if (hal_io_get_sw(SW_ID2))
		{
			c = MDL_CURVE_PIT_ID2;
			modeStr = MNU_MODEL_CURVE_ID2;
		}
		
		// Override with HOLD
		if (hal_io_get_sw(SW_THR))
		{
			c = MDL_CURVE_PIT_HOLD;
			modeStr = MNU_MODEL_CURVE_HLD;
		}

	}
	else
	{
		menuTitle = MNU_MODEL_THROTTLE_CURVE;

		if (hal_io_get_sw(SW_ID0))
		{
			c = MDL_CURVE_THR_ID0;
			modeStr = MNU_MODEL_CURVE_ID0;
		}
		else if (hal_io_get_sw(SW_ID1))
		{
			c = MDL_CURVE_THR_ID1;
			modeStr = MNU_MODEL_CURVE_ID1;
		}
		else if (hal_io_get_sw(SW_ID2))
		{
			c = MDL_CURVE_THR_ID2;
			modeStr = MNU_MODEL_CURVE_ID2;
		}
		
		// Override with HOLD
		if (hal_io_get_sw(SW_THR))
		{
			c = MDL_CURVE_THR_HOLD;
			modeStr = MNU_MODEL_CURVE_HLD;
		}
	}

	switch (event)
	{
		case GUI_EVT_SHOW:
			cursor = 0;
			changedModel = 0;
			break;
		case GUI_EVT_HIDE:
			break;
		case GUI_EVT_KEY_EXIT:
			// We are done...restore?
			if (changedModel == 1)
			{
				eeprom_load_model_config(g_RadioConfig.selectedModel);
			}
			gui_screen_pop();
			break;
		case GUI_EVT_KEY_MENU:
			// We are done...Save?
			if (changedModel == 1)
			{
				eeprom_save_model_config(g_RadioConfig.selectedModel);
			}
			gui_screen_pop();
			break;
		case GUI_EVT_KEY_UP:
			cursor--;
			if (cursor < 0)
			{
				cursor = 4;
			}
			break;
		case GUI_EVT_KEY_DOWN:
			cursor++;
			if (cursor > 4)
			{
				cursor = 0;
			}
			break;
		case GUI_EVT_KEY_RIGHT:
			g_Model.curve[c][cursor] += 1;
			if (g_Model.curve[c][cursor] > 100)
			{
				g_Model.curve[c][cursor] = 100;
			}
			changedModel = 1;
			break;
		case GUI_EVT_KEY_LEFT:
			g_Model.curve[c][cursor] -= 1;
			if (g_Model.curve[c][cursor] < -100)
			{
				g_Model.curve[c][cursor] = -100;
			}
			changedModel = 1;
			break;
		case GUI_EVT_POT_MOVE:
			g_Model.curve[c][cursor] = g_RadioRuntime.adc_s[GUI_POT];
			if (g_Model.curve[c][cursor] < -100)
			{
				g_Model.curve[c][cursor] = -100;
			}
			if (g_Model.curve[c][cursor] > 100)
			{
				g_Model.curve[c][cursor] = 100;
			}
			changedModel = 1;
			break;

		default:
			break;
	}

	lcd_clear();
	lcd_puts_P( 0, 0, menuTitle);
	lcd_puts_P( 5*LCD_FONT_WIDTH, 0, modeStr);

	// Draw the X-Y-axis... (64x64 pixels)
	lcd_hline(LCD_DISPLAY_W-64, LCD_DISPLAY_H-32,    64);
	lcd_vline(LCD_DISPLAY_W-32, LCD_DISPLAY_H-64,    64);

	// Scale on X-axis
	lcd_vline(LCD_DISPLAY_W-64, LCD_DISPLAY_H-33,    3);
	lcd_vline(LCD_DISPLAY_W-48, LCD_DISPLAY_H-33,    3);
	lcd_vline(LCD_DISPLAY_W-16, LCD_DISPLAY_H-33,    3);
	lcd_vline(LCD_DISPLAY_W-1, LCD_DISPLAY_H-33,    3);

	// Scale on Y-axis
	lcd_hline(LCD_DISPLAY_W-33, LCD_DISPLAY_H-64,    3);
	lcd_hline(LCD_DISPLAY_W-33, LCD_DISPLAY_H-48,    3);
	lcd_hline(LCD_DISPLAY_W-33, LCD_DISPLAY_H-16,    3);
	lcd_hline(LCD_DISPLAY_W-33, LCD_DISPLAY_H-1,    3);

	for (i=0; i<MDL_MAX_CURVE_POINTS; i++)
	{
		lcd_outdezAtt(1*LCD_FONT_WIDTH, (i + 2)*LCD_FONT_HEIGHT, i+1, LCD_NO_INV);
		if (cursor == i)
		{
			lcd_outdezAtt(7*LCD_FONT_WIDTH, (i + 2)*LCD_FONT_HEIGHT, g_Model.curve[c][i], LCD_INVERS);
		}
		else
		{
			lcd_outdezAtt(7*LCD_FONT_WIDTH, (i + 2)*LCD_FONT_HEIGHT, g_Model.curve[c][i], LCD_NO_INV);
		}
	}

	// Draw the curve...
	x1 = -100;
	y1 = g_Model.curve[c][0];

	for (i=1; i<MDL_MAX_CURVE_POINTS; i++)
	{
		x2 = x1 + 50;
		y2 = g_Model.curve[c][i];

		px1 = 95 + (x1*32/100);
		py1 = 31 - (y1*32/100);
		px2 = 95 + (x2*32/100);
		py2 = 31 - (y2*32/100);

		if (px1 < 0)
		{
			px1 = 0;
		}
		if (px1 >= LCD_DISPLAY_W)
		{
			px1 = LCD_DISPLAY_W-1;
		}

		if (px2 < 0)
		{
			px2 = 0;
		}
		if (px2 >= LCD_DISPLAY_W)
		{
			px2 = LCD_DISPLAY_W-1;
		}

		if (py1 < 0)
		{
			py1 = 0;
		}
		if (py1 >= LCD_DISPLAY_H)
		{
			py1 = LCD_DISPLAY_H-1;
		}

		if (py2 < 0)
		{
			py2 = 0;
		}
		if (py2 >= LCD_DISPLAY_H)
		{
			py2 = LCD_DISPLAY_H-1;
		}

		lcd_line(px1, py1, px2, py2);

		x1 = x2;
		y1 = y2;

	}

	return 1;
}

/*--------------------------------------------------------------------------------
 * menu_model_servo_direction
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_SERVO_TITLE[] 		PROGMEM = "Servo Direction";
char MNU_MODEL_SERVO_CH[] 			PROGMEM = "CH";
char MNU_MODEL_SERVO_CH_NORM[] 		PROGMEM = "NOR";
char MNU_MODEL_SERVO_CH_INV[] 		PROGMEM = "INV";

int8_t servoSelected;

uint8_t menu_model_servo_direction(GUI_EVENT event, uint8_t elapsedTime)
{
	uint8_t dirty = 1;
	uint8_t x,y,s,v;
	uint8_t drawingMode;


	switch (event)
	{
		case GUI_EVT_SHOW:
			servoSelected = 0;
			changedModel = 0;
			break;
		case GUI_EVT_HIDE:
			break;
		case GUI_EVT_TICK:
			break;
		case GUI_EVT_KEY_EXIT:
			// We are done...restore?
			if (changedModel == 1)
			{
				eeprom_load_model_config(g_RadioConfig.selectedModel);
			}
			gui_screen_pop();
			break;
		case GUI_EVT_KEY_MENU:
			// We are done...save?
			if (changedModel == 1)
			{
				eeprom_save_model_config(g_RadioConfig.selectedModel);
			}
			gui_screen_pop();
			break;
		case GUI_EVT_KEY_RIGHT:
			servoSelected++;
			break;
		case GUI_EVT_KEY_LEFT:
			servoSelected--;
			break;
		case GUI_EVT_KEY_DOWN:
		case GUI_EVT_KEY_UP:
			v = menu_get_setting(servoSelected + MC_SET_DIR_CH1);
			if (v == 0)
			{
				v = 1;
			}
			else
			{
				v = 0;
			}
			menu_set_setting(servoSelected + MC_SET_DIR_CH1, v);
			changedModel = 1;
			break;
		default:
			break;
	}

	if (servoSelected > 7)
	{
		servoSelected = servoSelected - 8;
	}
	if (servoSelected < 0)
	{
		servoSelected = servoSelected + 8;
	}

	lcd_clear();
	lcd_puts_P( 0, 0, MNU_MODEL_SERVO_TITLE);
	s = 0;

	for(y=0; ((y<3) && (s<MDL_MAX_CHANNELS)); y++)
	{
		for(x=0; ((x<3) && (s<MDL_MAX_CHANNELS)); x++)
		{
			// "CHn"
			lcd_putsAtt((x*7)*LCD_FONT_WIDTH, (y*2 + 2)*LCD_FONT_HEIGHT, MNU_MODEL_SERVO_CH, LCD_NO_INV);
			lcd_outdezAtt((x*7 + 3)*LCD_FONT_WIDTH, (y*2 + 2)*LCD_FONT_HEIGHT, s+1, LCD_NO_INV);

			if (s == servoSelected)
			{
				drawingMode = LCD_INVERS;
			}
			else
			{
				drawingMode = LCD_NO_INV;
			}

			// The value...
			v = menu_get_setting(s + MC_SET_DIR_CH1);
			if (v)
			{
				lcd_putsAtt((x*7+3)*LCD_FONT_WIDTH+2, (y*2 + 2)*LCD_FONT_HEIGHT, MNU_MODEL_SERVO_CH_INV, drawingMode);
			}
			else
			{
				lcd_putsAtt((x*7+3)*LCD_FONT_WIDTH+2, (y*2 + 2)*LCD_FONT_HEIGHT, MNU_MODEL_SERVO_CH_NORM, drawingMode);
			}

			s++;
		}
	}

	return dirty;
}

/*--------------------------------------------------------------------------------
 * menu_model_servo_subtrim
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_SERVO_SUBTRIM_TITLE[]	PROGMEM = "Subtrims";

uint8_t menu_model_servo_subtrim(GUI_EVENT event, uint8_t elapsedTime)
{
	uint8_t dirty = 1;
	uint8_t x,y,s;
	uint8_t drawingMode;
	int8_t v;


	switch (event)
	{
		case GUI_EVT_SHOW:
			servoSelected = 0;
			changedModel = 0;
			break;
		case GUI_EVT_HIDE:
			break;
		case GUI_EVT_TICK:
			break;
		case GUI_EVT_KEY_EXIT:
			// We are done...restore?
			if (changedModel == 1)
			{
				eeprom_load_model_config(g_RadioConfig.selectedModel);
			}
			gui_screen_pop();
			break;
		case GUI_EVT_KEY_MENU:
			// We are done...save?
			if (changedModel == 1)
			{
				eeprom_save_model_config(g_RadioConfig.selectedModel);
			}
			gui_screen_pop();
			break;
		case GUI_EVT_KEY_RIGHT:
			servoSelected += 1;
			break;
		case GUI_EVT_KEY_LEFT:
			servoSelected -= 1;
			break;
		case GUI_EVT_KEY_UP:
			v = menu_get_setting(servoSelected + MC_SET_SUBT_CH1);
			v += 1;
			menu_set_setting(servoSelected + MC_SET_SUBT_CH1, v);
			changedModel = 1;
			break;
		case GUI_EVT_KEY_DOWN:
			v = menu_get_setting(servoSelected + MC_SET_SUBT_CH1);
			v -= 1;
			menu_set_setting(servoSelected + MC_SET_SUBT_CH1, v);
			changedModel = 1;
			break;
		case GUI_EVT_POT_MOVE:
			v = g_RadioRuntime.adc_s[GUI_POT];
			menu_set_setting(servoSelected + MC_SET_SUBT_CH1, v);
			changedModel = 1;
		default:
			break;
	}

	if (servoSelected > 7)
	{
		servoSelected = servoSelected - 8;
	}
	if (servoSelected < 0)
	{
		servoSelected = servoSelected + 8;
	}


	lcd_clear();
	lcd_puts_P( 0, 0, MNU_MODEL_SERVO_SUBTRIM_TITLE);
	s = 0;

	for(y=0; ((y<3) && (s<MDL_MAX_CHANNELS)); y++)
	{
		for(x=0; ((x<3) && (s<MDL_MAX_CHANNELS)); x++)
		{
			// "CHn"
			lcd_putsAtt((x*7)*LCD_FONT_WIDTH, (y*2 + 2)*LCD_FONT_HEIGHT, MNU_MODEL_SERVO_CH, LCD_NO_INV);
			lcd_outdezAtt((x*7 + 3)*LCD_FONT_WIDTH, (y*2 + 2)*LCD_FONT_HEIGHT, s+1, LCD_NO_INV);

			if (s == servoSelected)
			{
				drawingMode = LCD_INVERS;
			}
			else
			{
				drawingMode = LCD_NO_INV;
			}

			// The value...
			v = menu_get_setting(s + MC_SET_SUBT_CH1);
			lcd_outdezAtt((x*7+6)*LCD_FONT_WIDTH, (y*2 + 2)*LCD_FONT_HEIGHT, v, drawingMode);

			s++;
		}
	}

	return dirty;
}


/*--------------------------------------------------------------------------------
 * menu_adc_calibrate
 *--------------------------------------------------------------------------------*/
char MNU_ADC_WRAP[] 	PROGMEM = "<    >";
char MNU_ADC_CALIB[] 	PROGMEM = "Calibrate Sticks: ";
char MNU_ADC_INFO_T[] 	PROGMEM = "   HLP:Calibrate!  ";
char MNU_ADC_INFO_1[] 	PROGMEM = "Move sticks to all ";
char MNU_ADC_INFO_2[] 	PROGMEM = "ends. Center the   ";
char MNU_ADC_INFO_3[] 	PROGMEM = "sticks. Press MENU ";
char MNU_ADC_INFO_4[] 	PROGMEM = "key to SAVE.       ";

uint8_t menu_adc_calibrate(GUI_EVENT event, uint8_t elapsedTime)
{
	uint8_t i;
	switch (event)
	{
		case GUI_EVT_SHOW:
			// Clear old calibration
			g_RadioRuntime.adc_store_end = 1;
			for (i=0; i<7; i++)
			{
				g_RadioConfig.adc_c[i][0] = 0xFFFF;
				g_RadioConfig.adc_c[i][1] = 0x0200;
				g_RadioConfig.adc_c[i][2] = 0x0000;
			}
			break;
		case GUI_EVT_HIDE:
			// Calibration not needed anymore...
			g_RadioRuntime.adc_store_end = 0;
			// But make sure we store the mid positions also.
			g_RadioRuntime.adc_store_mid = 1;
			break;
		case GUI_EVT_TICK:
			break;
		default:
			break;
	}

	// Draw the calibration screen.
	lcd_clear();
	lcd_puts_P( 0, 0, MNU_ADC_CALIB);
	for(uint8_t i=0; i<7; i++)
	{
		uint8_t y = i*LCD_FONT_HEIGHT + 0;

		lcd_outhex4( 1*LCD_FONT_WIDTH,  y+1*LCD_FONT_HEIGHT, g_RadioRuntime.adc_r[i]);
		lcd_puts_P( 11*LCD_FONT_WIDTH,  y+1*LCD_FONT_HEIGHT, MNU_ADC_WRAP);
		lcd_outhex4( 8*LCD_FONT_WIDTH-3,y+1*LCD_FONT_HEIGHT, g_RadioConfig.adc_c[i][0]);
		lcd_outhex4(12*LCD_FONT_WIDTH,  y+1*LCD_FONT_HEIGHT, g_RadioConfig.adc_c[i][1]);
		lcd_outhex4(17*LCD_FONT_WIDTH,  y+1*LCD_FONT_HEIGHT, g_RadioConfig.adc_c[i][2]);
	}
	switch (event)
	{
		case GUI_EVT_SHOW:
			// Show help...
			menu_show_messagebox(MNU_ADC_INFO_T, MNU_ADC_INFO_1, MNU_ADC_INFO_2, MNU_ADC_INFO_3, MNU_ADC_INFO_4);
			break;
		case GUI_EVT_TICK:
		case GUI_EVT_HIDE:
			break;
		case GUI_EVT_KEY_MENU:
			// Save EEPROM if pressed MENU
			eeprom_save_radio_config();
			gui_screen_pop();
			break;
		case GUI_EVT_KEY_UP:
		case GUI_EVT_KEY_UP_LONG:
		case GUI_EVT_KEY_DOWN:
		case GUI_EVT_KEY_DOWN_LONG:
		case GUI_EVT_KEY_LEFT:
		case GUI_EVT_KEY_LEFT_LONG:
		case GUI_EVT_KEY_RIGHT:
		case GUI_EVT_KEY_RIGHT_LONG:
		case GUI_EVT_KEY_MENU_LONG:
		case GUI_EVT_KEY_EXIT:
		case GUI_EVT_KEY_EXIT_LONG:
			// We are done...remove us on any key-event
			// make sure to load back old settings to
			// undo calibration...
			eeprom_load_radio_config();
			gui_screen_pop();
			break;
		default:
			break;
	}

	// Always redraw this screen...
	return 1;
	
}


/*--------------------------------------------------------------------------------
 * menu_main_screen
 *--------------------------------------------------------------------------------*/
void menu_main_draw_v_trim(uint8_t xpos, uint8_t ypos, ADC_INPUTS a)
{
	uint8_t pos = 0;
	int8_t trimPos = 0;
	int16_t adcValue = 0;

	// Draw the "outline" of the trims
	lcd_vline(xpos+2, ypos+1,  51);
	lcd_vline(xpos+5, ypos+1,  51);
	lcd_hline(xpos+3, ypos,    2);
	lcd_hline(xpos+3, ypos+52, 2);
	
	// Draw the trim 0 pos...
	lcd_hline(xpos+1, ypos + 25,  6);

	// Draw the trim indicator...
	trimPos = (int8_t)(ypos + 25) + (g_Model.trim[a] / 4);
	lcd_hline(xpos, trimPos-1,  8);
	lcd_hline(xpos, trimPos+1,  8);
	lcd_plot(xpos, trimPos);
	lcd_plot(xpos+7, trimPos);
	

	// Value is <-100.100> => converted to 0-50
	adcValue = (g_RadioRuntime.adc_s[a] + 100)/4;

	// Vertical ADC
	for (pos=0; pos<(uint8_t)adcValue; pos++)
	{
		lcd_plot(xpos+3, ypos+51-pos);
		lcd_plot(xpos+4, ypos+51-pos);
	}
	
}

void menu_main_draw_h_trim(uint8_t xpos, uint8_t ypos, ADC_INPUTS a)
{
	uint8_t pos = 0;
	int8_t trimPos = 0;
	int16_t adcValue = 0;

	// Draw the "outline" of the trims
	lcd_hline(xpos+1, ypos+2,  51);
	lcd_hline(xpos+1, ypos+5,  51);
	lcd_vline(xpos,   ypos+3,  2);
	lcd_vline(xpos+52,ypos+3,  2);

	// Draw the trim 0 pos...
	lcd_vline(xpos + 25, ypos+1, 6);
	
	// Draw the trim indicator...
	trimPos = (int8_t)(xpos + 25) + (g_Model.trim[a] / 4);
	lcd_vline(trimPos-1, ypos, 8);
	lcd_vline(trimPos+1, ypos, 8);
	lcd_plot(trimPos, ypos);
	lcd_plot(trimPos, ypos + 7);

	// Value is <-100.100> => converted to 0-50
	adcValue = (g_RadioRuntime.adc_s[a] + 100)/4;

	// Horizontal ADC
	for (pos=0; pos<(uint8_t)adcValue; pos++)
	{
		lcd_plot(xpos+1+pos, ypos+3);
		lcd_plot(xpos+1+pos, ypos+4);
	}
	
}


void menu_main_draw_battey(uint8_t x, uint8_t y) 
{
 	uint16_t v;
	uint8_t drawingMode = LCD_NO_INV;

	v = 50; // 5V
	v += ((g_RadioRuntime.adc_r[7]-g_RadioConfig.adc_c[7][0]) * 10) / 
	     ((g_RadioConfig.adc_c[7][2]-g_RadioConfig.adc_c[7][0])/7);

	if (g_RadioRuntime.adc_r[7] < g_RadioConfig.adc_c[7][1])
	{
	 	// We are under alarm...
		drawingMode = LCD_INVERS;
	}


	// Integer part of Volt (2 digits max...)
	lcd_outdezAtt(x - LCD_FONT_WIDTH*3 + 3, y, (v/10), drawingMode);

	// The "."
	lcd_putc(x - LCD_FONT_WIDTH*3 + 3, y, '.');

	// Tens of Volt
	lcd_outdezAtt(x - LCD_FONT_WIDTH*1, y, (v/100), drawingMode);

	// The "V"
	lcd_putc(x - LCD_FONT_WIDTH*1, y, 'V');

}

void main_draw_servo(uint8_t x, uint8_t y, uint8_t servo)
{
	uint8_t pos;
	int16_t servoValue = 0;

	// Draw the "outline" of the servo
	lcd_hline(x, y,  20);
	lcd_hline(x, y+2,20);
	lcd_vline(x, y, 3);
	lcd_vline(x+20, y, 3);

	// Value is <-100.100> => converted to 0-20
	servoValue = (g_RadioRuntime.srv_s[servo] + 100)/10;

	// Horizontal ADC
	for (pos=0; pos<(uint8_t)servoValue; pos++)
	{
		lcd_plot(x+1+pos, y+1);
	}

}


// Local stuff for main screen
// 0 - Servo output (analog)
// 1 - Servo output (deciaml)
// 2 - Analog input (decimal)
// 3 - Analog input (raw)
// 4 - Timer mode 
// 10 - Debug mode...
int8_t mainDisplayMode = 0;

char MNU_MAIN_MODE_1[] 	PROGMEM = "Servo output:";
char MNU_MAIN_MODE_2[] 	PROGMEM = "Stick input:";
char MNU_MAIN_MODE_3[] 	PROGMEM = "ADC Raw input:";

#ifdef USE_DEBUG_MODE
char MNU_MAIN_MODE_10[] PROGMEM = "-- DEBUG MODE --";

char debugLine1[20] = {0};
char debugLine2[20] = {0};
#endif

char MNU_MAIN_CH1[] 	PROGMEM = "C1";
char MNU_MAIN_CH2[] 	PROGMEM = "C2";
char MNU_MAIN_CH3[] 	PROGMEM = "C3";
char MNU_MAIN_CH4[] 	PROGMEM = "C4";
char MNU_MAIN_CH5[] 	PROGMEM = "C5";
char MNU_MAIN_CH6[] 	PROGMEM = "C6";
char MNU_MAIN_CH7[] 	PROGMEM = "C7";
char MNU_MAIN_CH8[] 	PROGMEM = "C8";
char* channelTable[] PROGMEM = 
{
	MNU_MAIN_CH1,
	MNU_MAIN_CH2,
	MNU_MAIN_CH3,
	MNU_MAIN_CH4,
	MNU_MAIN_CH5,
	MNU_MAIN_CH6,
	MNU_MAIN_CH7,
	MNU_MAIN_CH8
};

uint8_t menu_main_screen(GUI_EVENT event, uint8_t elapsedTime)
{
	int16_t adcValue = 0;
	uint8_t a;
	uint8_t x,y;
	uint8_t txtOffset;
	uint8_t numOffset;

	switch (event)
	{
		case GUI_EVT_SHOW:
			break;
		case GUI_EVT_HIDE:
			break;
		case GUI_EVT_TICK:
			break;
		case GUI_EVT_KEY_UP:
			mainDisplayMode++;
			if (mainDisplayMode > 3)
			{
				mainDisplayMode = 0;
			}
			break;
		case GUI_EVT_KEY_DOWN:
			mainDisplayMode--;
			if (mainDisplayMode < 0)
			{
				mainDisplayMode = 3;
			}
			break;
		case GUI_EVT_KEY_MENU_LONG:
			gui_screen_push(&menu_main_menu);
			return 0;
#ifdef USE_DEBUG_MODE
		// Hidden debug mode...
		case GUI_EVT_KEY_LEFT_LONG:
			mainDisplayMode = 10;
			break;
#endif
		default:
			break;
	}

	// Draw the calibration screen.
	lcd_clear();

	// MODEL NAME
	lcd_putsnAtt(1*LCD_FONT_WIDTH,  0, g_Model.name, strlen(g_Model.name), LCD_BSS_NO_INV);

	// BATTERY
	menu_main_draw_battey(LCD_DISPLAY_W, 0);

	switch (mainDisplayMode)
	{
		case 0:
			main_draw_servo(20, 20, 0);
			main_draw_servo(54, 20, 1);
			main_draw_servo(88, 20, 2);
			main_draw_servo(20, 30, 3);
			main_draw_servo(54, 30, 4);
			main_draw_servo(88, 30, 5);
			main_draw_servo(20, 40, 6);
			main_draw_servo(54, 40, 7);

			break;

		case 1:
			//lcd_putsAtt(4*LCD_FONT_WIDTH,  1*LCD_FONT_HEIGHT+5, MNU_MAIN_MODE_1, LCD_NO_INV);
			x = 6;
			y = 1;
			for (a=0; a<MDL_MAX_CHANNELS; a++)
			{
				if ((a == 0) || (a == 3) || (a == 6))
				{
					txtOffset = 2;
					numOffset = 0;
				}
				else if ((a == 2) || (a == 5))
				{
					txtOffset = -3;
					numOffset = -5;
				}
				else
				{
					txtOffset = -2;
					numOffset = -4;
				}

				adcValue = g_RadioRuntime.srv_s[a];
				lcd_putsAtt((x-5)*LCD_FONT_WIDTH+txtOffset,  y*LCD_FONT_HEIGHT,  (char*)pgm_read_word(&channelTable[a]), LCD_NO_INV);
				lcd_outdezAtt((x+1)*LCD_FONT_WIDTH+numOffset, y*LCD_FONT_HEIGHT, adcValue, LCD_NO_INV);

				x = x + 7;
				if ((a == 2) || (a == 5))
				{
					// New line...
					y = y + 2;
					x = 6;
				}
			}	
			break;
		case 2:
			lcd_putsAtt(4*LCD_FONT_WIDTH,  1*LCD_FONT_HEIGHT+5, MNU_MAIN_MODE_2, LCD_NO_INV);
			x = 5;
			y = 3;
			for (a=0; a<ADC_MAX_NUM_ADC; a++)
			{
				// Get the ADC value
				adcValue = g_RadioRuntime.adc_s[a];
				lcd_outdezAtt(x*LCD_FONT_WIDTH, y*LCD_FONT_HEIGHT, adcValue, LCD_NO_INV);

				x = x + 4;
				if (a == 3)
				{
					// New line...
					y = y + 2;
					x = 5;
				}
			}	
			break;
		case 3:
			lcd_putsAtt(4*LCD_FONT_WIDTH,  1*LCD_FONT_HEIGHT+5, MNU_MAIN_MODE_3, LCD_NO_INV);
			x = 3;
			y = 3;
			for (a=0; a<ADC_MAX_NUM_ADC; a++)
			{
				// Get the ADC value
				lcd_outhex4( x*LCD_FONT_WIDTH,  y*LCD_FONT_HEIGHT, g_RadioRuntime.adc_r[a]);

				x = x + 4;
				if (a == 3)
				{
					// New line...
					y = y + 2;
					x = 3;
				}
			}	
			break;
#ifdef USE_DEBUG_MODE
		case 10:
			lcd_putsAtt(4*LCD_FONT_WIDTH,  1*LCD_FONT_HEIGHT+5, MNU_MAIN_MODE_10, LCD_NO_INV);
			lcd_putsAtt(4*LCD_FONT_WIDTH,  3*LCD_FONT_HEIGHT+5, debugLine1, LCD_BSS_NO_INV);
			lcd_putsAtt(4*LCD_FONT_WIDTH,  4*LCD_FONT_HEIGHT+5, debugLine2, LCD_BSS_NO_INV);
			break;
#endif		
		default:
			break;
	}

	// TRIMS
	menu_main_draw_v_trim(0, LCD_DISPLAY_H-53, ADC_THR);
	menu_main_draw_v_trim(LCD_DISPLAY_W - 8, LCD_DISPLAY_H-53, ADC_ELE);
	menu_main_draw_h_trim(10, LCD_DISPLAY_H-8, ADC_RUD);
	menu_main_draw_h_trim(LCD_DISPLAY_W - 53 - 10, LCD_DISPLAY_H-8, ADC_AIL);


	// Always redraw this screen...
	return 1;
	
}


/*--------------------------------------------------------------------------------
 * menu_main_menu
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_SELECTION_NAME[] PROGMEM = "Model Selection   1/5";
char MNU_MODEL_CONFIG_NAME[] 	PROGMEM = "Model Config      2/5";
char MNU_MODEL_MGMT_NAME[] 		PROGMEM = "Model Management  3/5";
char MNU_RADIO_CONFIG_NAME[] 	PROGMEM = "Radio Config      4/5";
char MNU_RADIO_INSTALL_NAME[] 	PROGMEM = "Radio Install     5/5";

typedef struct 
{
	char* name;
	GUI_SCREEN_FPTR screen;
	void* previous;
	void* next;
} SMenu;


/* MAIN MENU CHAIN */
extern SMenu ModelSelection;
extern SMenu ModelConfiguration;
extern SMenu ModelManagement;
extern SMenu RadioConfiguration;
extern SMenu RadioInstall;


SMenu ModelSelection PROGMEM = 
{
	MNU_MODEL_SELECTION_NAME,
	0,
	(void*)&RadioInstall,
	(void*)&ModelConfiguration
};

SMenu ModelConfiguration PROGMEM = 
{
	MNU_MODEL_CONFIG_NAME,
	&menu_model_config,
	(void*)&ModelSelection,
	(void*)&ModelManagement,
};

SMenu ModelManagement PROGMEM = 
{
	MNU_MODEL_MGMT_NAME,
	0,
	(void*)&ModelConfiguration,
	(void*)&RadioConfiguration,
};

SMenu RadioConfiguration PROGMEM = 
{
	MNU_RADIO_CONFIG_NAME,
	&menu_radio_config,
	(void*)&ModelManagement,
	(void*)&RadioInstall,
};

SMenu RadioInstall PROGMEM = 
{
	MNU_RADIO_INSTALL_NAME,
	&menu_radio_install,
	(void*)&RadioConfiguration,
	(void*)&ModelSelection,
};


SMenu* currentMenu = (SMenu*)&ModelSelection;

// Indicates if "LEFT/RIGHT" are for navigating among
// menus...if 0, the menu-screen has ownership
uint8_t menuNavigation = 1;


uint8_t menu_get_setting(uint8_t parameterId)
{
	switch (parameterId)
	{
		case RC_SET_VOLTAGE:
			return (g_RadioConfig.voltageWarning - 50)/5;
			break;
		case RC_SET_BACKLIGHT:
			return g_RadioConfig.backlight;
			break;
		case RC_SET_BEEP_KEYS:
			return g_RadioConfig.keyBeep;
			break;
		case RC_SET_BEEP_ALARMS:
			return g_RadioConfig.alarmBeep;
			break;

		case MC_SET_DIR_CH1:
		case MC_SET_DIR_CH2:
		case MC_SET_DIR_CH3:
		case MC_SET_DIR_CH4:
		case MC_SET_DIR_CH5:
		case MC_SET_DIR_CH6:
		case MC_SET_DIR_CH7:
		case MC_SET_DIR_CH8:
			parameterId = parameterId - MC_SET_DIR_CH1;
			return ((g_Model.servoDirection & (1 << parameterId)) & (1 << parameterId));
			break;

		case MC_SET_SUBT_CH1:
		case MC_SET_SUBT_CH2:
		case MC_SET_SUBT_CH3:
		case MC_SET_SUBT_CH4:
		case MC_SET_SUBT_CH5:
		case MC_SET_SUBT_CH6:
		case MC_SET_SUBT_CH7:
		case MC_SET_SUBT_CH8:
			parameterId = parameterId - MC_SET_SUBT_CH1;
			return g_Model.subTrim[parameterId];
			break;
		default:
			break;
	}
	return 0;
}

void menu_set_setting(uint8_t parameterId, uint8_t newValue)
{
	switch (parameterId)
	{
		case RC_SET_VOLTAGE:
			g_RadioConfig.voltageWarning = 50 + (newValue*5);
			if (g_RadioConfig.voltageWarning > 100)
			{
				g_RadioConfig.voltageWarning = 100;
			}
			if (g_RadioConfig.voltageWarning < 50)
			{
				g_RadioConfig.voltageWarning = 50;
			}
			break;
		case RC_SET_BACKLIGHT:
			g_RadioConfig.backlight = newValue;
			if (g_RadioConfig.backlight == 0xFF)
			{
				g_RadioConfig.backlight = 0;
			}
			if (g_RadioConfig.backlight > 5)
			{
				g_RadioConfig.backlight = 5;
			}
			break;
		case RC_SET_BEEP_KEYS:
			g_RadioConfig.keyBeep = newValue;
			if (g_RadioConfig.keyBeep == 0xFF)
			{
				g_RadioConfig.keyBeep = 0;
			}
			if (g_RadioConfig.keyBeep > 1)
			{
				g_RadioConfig.keyBeep = 1;
			}
		
			break;
		case RC_SET_BEEP_ALARMS:
			g_RadioConfig.alarmBeep = newValue;
			if (g_RadioConfig.alarmBeep == 0xFF)
			{
				g_RadioConfig.alarmBeep = 0;
			}
			if (g_RadioConfig.alarmBeep > 3)
			{
				g_RadioConfig.alarmBeep = 3;
			}
			break;


		case MC_SET_DIR_CH1:
		case MC_SET_DIR_CH2:
		case MC_SET_DIR_CH3:
		case MC_SET_DIR_CH4:
		case MC_SET_DIR_CH5:
		case MC_SET_DIR_CH6:
		case MC_SET_DIR_CH7:
		case MC_SET_DIR_CH8:
			parameterId = parameterId - MC_SET_DIR_CH1;
			if (newValue == 1)
			{
				g_Model.servoDirection |= (1 << parameterId);
			}
			else
			{
				g_Model.servoDirection &= ~(1 << parameterId);
			}
			break;

		case MC_SET_SUBT_CH1:
		case MC_SET_SUBT_CH2:
		case MC_SET_SUBT_CH3:
		case MC_SET_SUBT_CH4:
		case MC_SET_SUBT_CH5:
		case MC_SET_SUBT_CH6:
		case MC_SET_SUBT_CH7:
		case MC_SET_SUBT_CH8:
			parameterId = parameterId - MC_SET_SUBT_CH1;
			g_Model.subTrim[parameterId] = newValue;
			break;
		default:
			break;
	}
}

uint8_t menu_settings(GUI_EVENT event, uint8_t elapsedTime)
{
	uint8_t dirty = 0;
	uint8_t s;
	uint8_t start, end, line;
	SSelection* selection;
	uint8_t sstart;
	uint8_t value;
	char* selString;
	char* selectionAlternatives;
	GUI_SCREEN_FPTR screenFnPtr;

	switch (event)
	{
		case GUI_EVT_KEY_EXIT:
			// Go back to menu navigation if needed
			if (menuNavigation == 0)
			{
				menuNavigation = 1;
				cursorSetting = -1;
			}
			break;
		case GUI_EVT_KEY_UP:
			cursorSetting--;
			if (cursorSetting < 0)
			{
				// Return navigation to menu
				menuNavigation = 1;
				cursorSetting = -1;
			}
			break;
		case GUI_EVT_KEY_DOWN:
			cursorSetting++;
			if (cursorSetting >= numSettings)
			{
				// No wrap...stay here!
				cursorSetting = numSettings-1;
			}
			break;
		case GUI_EVT_KEY_RIGHT:
			if ((menuNavigation == 0) && (cursorSetting > -1))
			{
				selection = (SSelection*)&currentSettings[cursorSetting];
				value = menu_get_setting((uint8_t)pgm_read_byte(&selection->parameterId));
				value++;
				menu_set_setting((uint8_t)pgm_read_byte(&selection->parameterId), value);
		
			}
			break;
		case GUI_EVT_KEY_LEFT:
			if ((menuNavigation == 0) && (cursorSetting > -1))
			{
				selection = (SSelection*)&currentSettings[cursorSetting];
				value = menu_get_setting((uint8_t)pgm_read_byte(&selection->parameterId));
				value--;
				menu_set_setting((uint8_t)pgm_read_byte(&selection->parameterId), value);
			}
			break;
		case GUI_EVT_KEY_MENU:
			selection = (SSelection*)&currentSettings[cursorSetting];
			screenFnPtr = (GUI_SCREEN_FPTR)pgm_read_word(&selection->screen);
			if (screenFnPtr != 0)
			{
				gui_screen_push(screenFnPtr);
				return 0;
			}
			break;
		default:
			break;
	}

	
	if (cursorSetting > 4)
	{
		start = cursorSetting - 4;
		if (cursorSetting + 1 < numSettings)
		{
			end = cursorSetting + 1;
		}
		else
		{
			end = numSettings;
		}
	}
	else
	{
		start = 0;
		
		if (numSettings > 5)
		{
			end = 5;
		}
		else
		{
			end = numSettings;
		}
	}

	for (s=start, line = 0; s<end; s++, line++)
	{
		selection = (SSelection*)&currentSettings[s];
		screenFnPtr = (GUI_SCREEN_FPTR)pgm_read_word(&selection->screen);

		// Menu or settings?
		if (screenFnPtr == 0)
		{
			// Settings text
			lcd_putsAtt(0,  (line+2)*LCD_FONT_HEIGHT , (char*)pgm_read_word(&selection->text), LCD_NO_INV);

			selectionAlternatives = (char*)pgm_read_word(&selection->alternatives);
			value = menu_get_setting((uint8_t)pgm_read_byte(&selection->parameterId));

			sstart = value*5;
			selString = (char*)pgm_read_word(&selection->alternatives) + sstart;

			// Selection text
			if (cursorSetting == s)
			{
				lcd_putsnAtt(17*LCD_FONT_WIDTH, (line+2)*LCD_FONT_HEIGHT , selString, 4, LCD_INVERS);
			}
			else
			{
				lcd_putsnAtt(17*LCD_FONT_WIDTH, (line+2)*LCD_FONT_HEIGHT , selString, 4, LCD_NO_INV);
			}
		}
		else
		{
			// Submenu...just draw text
			if (cursorSetting == s)
			{
				lcd_putsAtt(0,  (line+2)*LCD_FONT_HEIGHT , (char*)pgm_read_word(&selection->text), LCD_INVERS);
			}
			else
			{
				lcd_putsAtt(0,  (line+2)*LCD_FONT_HEIGHT , (char*)pgm_read_word(&selection->text), LCD_NO_INV);
			}
		}

	}

	return dirty;
}

//
// Model Config
//

SSelection modelConfig[5] PROGMEM = 
{
	{
		0x10,
		MNU_MODEL_SERVO_TITLE,
		0,
		0,
		&menu_model_servo_direction
	},
	{
		0x11,
		MNU_MODEL_SERVO_SUBTRIM_TITLE,
		0,
		0,
		&menu_model_servo_subtrim
	},
	{
		MNU_ID_PITCH_CURVE,
		MNU_MODEL_PITCH_CURVE_TITLE,
		0,
		0,
		&menu_model_curve_edit
	},
	{
		MNU_ID_THROTTLE_CURVE,
		MNU_MODEL_THROTTLE_CURVE_TITLE,
		0,
		0,
		&menu_model_curve_edit
	},
	{
		0x14,
		MNU_MODEL_NAME_EDIT_TITLE,
		0,
		0,
		&menu_model_name_edit
	}
	
	
};


uint8_t menu_model_config(GUI_EVENT event, uint8_t elapsedTime)
{
	switch (event)
	{
		case GUI_EVT_SHOW:
			numSettings = 5;
			currentSettings = (SSelection*)&modelConfig[0];
			break;
		default:
			break;
	}

	return menu_settings(event, elapsedTime);
}

//
// Radio Config Settings
//
char MNU_RADIO_CONFIG_VOLTAGE[] 		PROGMEM = "Voltage Warning";
char MNU_RADIO_CONFIG_VOLTAGE_SEL[] 	PROGMEM = "5.0 |5.5 |6.0 |6.5 |7.0 |7.5 |8.0 |8.5 |9.0 |9.5 |10.0";
char MNU_RADIO_CONFIG_BACKLIGHT[] 		PROGMEM = "Backlight";
char MNU_RADIO_CONFIG_BACKLIGHT_SEL[] 	PROGMEM = "ON  |OFF |5s  |10s |15s |20s ";
char MNU_RADIO_CONFIG_BEEP_KEYS[] 		PROGMEM = "BEEP Keys";
char MNU_RADIO_CONFIG_BEEP_KEYS_SEL[]	PROGMEM = "ON  |OFF ";
char MNU_RADIO_CONFIG_BEEP_ALARMS[] 	PROGMEM = "BEEP Alarms";
char MNU_RADIO_CONFIG_BEEP_ALARMS_SEL[] PROGMEM = " 1  | 2  | 3  |OFF ";


SSelection radioSettings[4] PROGMEM = 
{
	{
		0x20,
		MNU_RADIO_CONFIG_VOLTAGE,
		MNU_RADIO_CONFIG_VOLTAGE_SEL,
		RC_SET_VOLTAGE,
		0
	},
	{
		0x21,
		MNU_RADIO_CONFIG_BACKLIGHT,
		MNU_RADIO_CONFIG_BACKLIGHT_SEL,
		RC_SET_BACKLIGHT,
		0
	},
	{
		0x22,
		MNU_RADIO_CONFIG_BEEP_KEYS,
		MNU_RADIO_CONFIG_BEEP_KEYS_SEL,
		RC_SET_BEEP_KEYS,
		0
	},
	{
		0x23,
		MNU_RADIO_CONFIG_BEEP_ALARMS,
		MNU_RADIO_CONFIG_BEEP_ALARMS_SEL,
		RC_SET_BEEP_ALARMS,
		0
	}
};

uint8_t menu_radio_config(GUI_EVENT event, uint8_t elapsedTime)
{
	switch (event)
	{
		case GUI_EVT_SHOW:
			// Load the "correct" settings...
			numSettings = 4;
			currentSettings = (SSelection*)&radioSettings[0];
			break;
		default:
			break;
	}

	return menu_settings(event, elapsedTime);
}


//
// Radio Installing
//
char MNU_RADIO_INSTALL_CALIBRATE[] 		PROGMEM = "Calibrate Sticks";


SSelection radioInstalling[1] PROGMEM = 
{
	{
		0x30,
		MNU_RADIO_INSTALL_CALIBRATE,
		0,
		0,
		&menu_adc_calibrate
	}
};


uint8_t menu_radio_install(GUI_EVENT event, uint8_t elapsedTime)
{
	switch (event)
	{
		case GUI_EVT_SHOW:
			numSettings = 1;
			currentSettings = (SSelection*)&radioInstalling[0];
			break;
		default:
			break;
	}

	return menu_settings(event, elapsedTime);
}



uint8_t menu_main_menu(GUI_EVENT event, uint8_t elapsedTime)
{
	uint8_t dirty = 0;
	GUI_SCREEN_FPTR menuScreenPtr;

	switch (event)
	{
		case GUI_EVT_SHOW:
			gui_set_long_press(0);
			break;
		case GUI_EVT_HIDE:
			gui_set_long_press(1);
			break;
		case GUI_EVT_TICK:
			break;
		case GUI_EVT_KEY_EXIT:
			if (menuNavigation == 1)
			{
				gui_screen_pop();
				return 0;
			}
			break;
		case GUI_EVT_KEY_RIGHT:
			if (menuNavigation == 1)
			{
				currentMenu = (SMenu*)pgm_read_word(&currentMenu->next);
				event = GUI_EVT_SHOW;
			}
			break;
		case GUI_EVT_KEY_LEFT:
			if (menuNavigation == 1)
			{
				currentMenu = (SMenu*)pgm_read_word(&currentMenu->previous);
				event = GUI_EVT_SHOW;
			}
			break;
		case GUI_EVT_KEY_DOWN:
			if (menuNavigation == 1)
			{
				menuScreenPtr = (GUI_SCREEN_FPTR)pgm_read_word(&currentMenu->screen);

				// Trun control over (if there is a screen)
				if (menuScreenPtr != 0)
				{
					menuNavigation = 0;
				}
			}
		default:
			break;
	}


	menuScreenPtr = (GUI_SCREEN_FPTR)pgm_read_word(&currentMenu->screen);

	lcd_clear();

	dirty = 1;
	
	lcd_putsnAtt(0*LCD_FONT_WIDTH,  0, (char*)pgm_read_word(&currentMenu->name), 17, LCD_NO_INV);
	if (menuNavigation == 1)
	{
		lcd_putsnAtt(18*LCD_FONT_WIDTH,  0, (char*)pgm_read_word(&currentMenu->name)+18, 3, LCD_INVERS);
	}
	else
	{
		lcd_putsnAtt(18*LCD_FONT_WIDTH,  0, (char*)pgm_read_word(&currentMenu->name)+18, 3, LCD_NO_INV);
	}
	lcd_hline(0, 1, LCD_DISPLAY_W);

	if (menuScreenPtr)
	{
		dirty |= (*menuScreenPtr)(event, elapsedTime);
	}
	
	return dirty;
}
