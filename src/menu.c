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
#include "mixer.h"
#include "template.h"
#include <avr/pgmspace.h>
#include <string.h>
#include <stdio.h>

/*--------------------------------------------------------------------------------
 * Private types
 *--------------------------------------------------------------------------------*/

typedef struct 
{
	char* name;
	GUI_SCREEN_FPTR screen;
	void* previous;
	void* next;
} SMenu;

typedef struct 
{
	uint8_t id;
	char* text;
	char* alternatives;
	uint8_t parameterId;
	GUI_SCREEN_FPTR screen;
} SSelection;

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
	MC_SET_SUBT_CH8,

	MC_SET_TYPE
} PARAMETER_ID;


/*--------------------------------------------------------------------------------
 * Prototypes
 *--------------------------------------------------------------------------------*/
uint8_t menu_adc_calibrate(GUI_EVENT event, uint8_t elapsedTime);
uint8_t menu_main_screen(GUI_EVENT event, uint8_t elapsedTime);
uint8_t menu_main_menu(GUI_EVENT event, uint8_t elapsedTime);

uint8_t menu_get_setting(uint8_t parameterId);
void menu_set_setting(uint8_t parameterId, uint8_t newValue);

// Menus
uint8_t menu_model_management(GUI_EVENT event, uint8_t elapsedTime);
uint8_t menu_model_config(GUI_EVENT event, uint8_t elapsedTime);
uint8_t menu_radio_config(GUI_EVENT event, uint8_t elapsedTime);
uint8_t menu_radio_install(GUI_EVENT event, uint8_t elapsedTime);

/* MAIN MENU CHAIN */
extern SMenu ModelSelection;
extern SMenu ModelConfiguration;
extern SMenu RadioConfiguration;
extern SMenu RadioInstall;

/*--------------------------------------------------------------------------------
 * LOCALS
 *--------------------------------------------------------------------------------*/

#define MNU_ID_PITCH_CURVE (0x12)
#define MNU_ID_THROTTLE_CURVE (0x13)

SSelection* currentSettings;
uint8_t numSettings = 0;
int8_t cursorSetting = -1;
uint8_t settingValue = -1;
uint8_t changedModel = 0;
int8_t cursor = 0;
int8_t cursor2 = 0;
int8_t servoSelected;

SMenu* currentMenu = (SMenu*)&ModelSelection;

// Indicates if "LEFT/RIGHT" are for navigating among
// menus...if 0, the menu-screen has ownership
uint8_t menuNavigation = 1;

uint8_t firstFreeSlot = 0xFF;
int8_t heliType = MDL_TYPE_HELI_SIM;

char MNU_MODEL_EMPTY[] 		PROGMEM = "                   ";
char MNU_MODEL_SERVO_CH[] 	PROGMEM = "CH";


char MNU_FN_AIL[] 	PROGMEM = "AIL";
char MNU_FN_ELE[] 	PROGMEM = "ELE";
char MNU_FN_RUD[] 	PROGMEM = "RUD";
char MNU_FN_PIT[] 	PROGMEM = "PIT";
char MNU_FN_THR[] 	PROGMEM = "THR";
char MNU_FN_GYR[] 	PROGMEM = "GYR";
char MNU_FN_AUX1[] 	PROGMEM = "AX1";
char MNU_FN_AUX2[] 	PROGMEM = "AX2";

char* fnStr[] PROGMEM = 
{
	MNU_FN_AIL,
	MNU_FN_THR,
	MNU_FN_ELE,
	MNU_FN_RUD,
	MNU_FN_GYR,
	MNU_FN_PIT,
	MNU_FN_AUX1,
	MNU_FN_AUX2
};


/*--------------------------------------------------------------------------------
 * menu_build_time_str
 *--------------------------------------------------------------------------------*/
void menu_build_time_str(int16_t time)
{
	int8_t minute, second;
	char* buf = &g_RadioRuntime.buffer[0];

	minute = time/60;
	second = time%60;

	if ((minute < 10) && (second < 10))
	{
		sprintf(buf, "0%d:0%d", minute, second);
	}
	else if ((minute < 10) && (second >= 10))
	{
		sprintf(buf, "0%d:%d", minute, second);
	}
	else if ((minute >= 10) && (second < 10))
	{
		sprintf(buf, "%d:%0d", minute, second);
	}
	else
	{
		sprintf(buf, "%d:%d", minute, second);
	}
}


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
 * menu_model_servo_mapping
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_SERVO_MAP_TITLE[]	PROGMEM = "Servo Map";
char MNU_MODEL_SERVO_MAP_SEL[]		PROGMEM = "Futaba   |Spektrum ";

uint8_t menu_get_current_mapping()
{
	// Do we have AILERON function mapped to CHANNEL 1 (i.e. value 0)
	// then we have FUTABA...else Spektrum...the only ones now...
	if (g_Model.functionToServoTable[0] == 0)
	{
		return 0;
	}
	else 
	{
		return 1;
	}
}

void  menu_set_current_mapping(uint8_t val)
{
	switch (val)
	{
		case 0:
			// FUTABA mapping...
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_AILERON)] = SERVO_CHANNEL(1);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_ELEVATOR)] = SERVO_CHANNEL(2);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_THROTTLE)] = SERVO_CHANNEL(3);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_RUDDER)] = SERVO_CHANNEL(4);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_GYRO_GAIN)] = SERVO_CHANNEL(5);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_PITCH)] = SERVO_CHANNEL(6);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_AUX1)] = SERVO_CHANNEL(7);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_AUX2)] = SERVO_CHANNEL(8);
			break;
		case 1:
			// SPEKTRUM mapping...
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_AILERON)] = SERVO_CHANNEL(2);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_ELEVATOR)] = SERVO_CHANNEL(3);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_THROTTLE)] = SERVO_CHANNEL(1);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_RUDDER)] = SERVO_CHANNEL(4);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_GYRO_GAIN)] = SERVO_CHANNEL(5);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_PITCH)] = SERVO_CHANNEL(6);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_AUX1)] = SERVO_CHANNEL(7);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_AUX2)] = SERVO_CHANNEL(8);
			break;					
		default:
			// FUTABA mapping...
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_AILERON)] = SERVO_CHANNEL(1);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_ELEVATOR)] = SERVO_CHANNEL(2);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_THROTTLE)] = SERVO_CHANNEL(3);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_RUDDER)] = SERVO_CHANNEL(4);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_GYRO_GAIN)] = SERVO_CHANNEL(5);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_PITCH)] = SERVO_CHANNEL(6);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_AUX1)] = SERVO_CHANNEL(7);
			g_Model.functionToServoTable[FUNCTION_INDEX(MIX_OUT_AUX2)] = SERVO_CHANNEL(8);
		break;
	}
}

uint8_t menu_model_servo_mapping(GUI_EVENT event, uint8_t elapsedTime)
{
	uint8_t dirty = 1;
	uint8_t i;
	uint8_t x,y;
	uint8_t s;
	char* selString;
	uint8_t sstart;

	switch (event)
	{
		case GUI_EVT_SHOW:
			cursor = 0;
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
		case GUI_EVT_KEY_UP:
			break;
		case GUI_EVT_KEY_DOWN:
			break;
		case GUI_EVT_KEY_RIGHT:
			cursor++;
			if (cursor > 1)
			{
				cursor = 0;
			}
			menu_set_current_mapping(cursor);
			changedModel = 1;
			break;
		case GUI_EVT_KEY_LEFT:
			cursor--;
			if (cursor < 0)
			{
				cursor = 1;
			}
			menu_set_current_mapping(cursor);
			changedModel = 1;
			break;
		default:
			break;
	}



	lcd_clear();
	lcd_puts_P( 0, 0, MNU_MODEL_SERVO_MAP_TITLE);

	sstart = menu_get_current_mapping()*10;
	selString = (char*)(MNU_MODEL_SERVO_MAP_SEL + sstart);

	lcd_putsnAtt( 12*LCD_FONT_WIDTH, 0, selString, 9, LCD_INVERS);

	x = 0;
	y = 2;

	for (i=0; i<8; i++)
	{
		lcd_putsAtt((x)*LCD_FONT_WIDTH, (y)*LCD_FONT_HEIGHT, MNU_MODEL_SERVO_CH, LCD_NO_INV);
		lcd_outdezAtt((x + 3)*LCD_FONT_WIDTH, (y)*LCD_FONT_HEIGHT, i+1, LCD_NO_INV);

		lcd_putsAtt((3 + x)*LCD_FONT_WIDTH, (y)*LCD_FONT_HEIGHT, PSTR("->"), LCD_NO_INV);

		for (s=0; s<8; s++)
		{
			if (s == g_Model.functionToServoTable[i])
			{
				break;
			}
		}

		lcd_putsAtt((5+x)*LCD_FONT_WIDTH, (y)*LCD_FONT_HEIGHT, (char*)pgm_read_word(&fnStr[s]), LCD_NO_INV);

		y++;

		if (i == 3)
		{
			y = 2;
			x = 12;
		}
	}
	return dirty;
}


/*--------------------------------------------------------------------------------
 * menu_model_timer_setup
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_TIMER_TITLE[]	PROGMEM = "Timer Setup";

char MNU_MODEL_TIMER_TIME[]			PROGMEM = "Time:";
char MNU_MODEL_TIMER_ALARM[]		PROGMEM = "Alarm:";
char MNU_MODEL_TIMER_MODE[]			PROGMEM = "Mode:";
char MNU_MODEL_TIMER_MODE_SEL[]		PROGMEM = "OFF |AUTO|MAN ";

uint8_t menu_model_timer_setup(GUI_EVENT event, uint8_t elapsedTime)
{
	uint8_t dirty = 1;
	int16_t v;
	uint8_t sstart;
	char* selString;

	switch (event)
	{
		case GUI_EVT_SHOW:
			cursor = 0;
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

				// Reset the timer also...
				g_RadioRuntime.modelTimer = g_Model.timer;
			}
			gui_screen_pop();
			break;
		case GUI_EVT_KEY_UP:
			cursor -= 1;
			break;
		case GUI_EVT_KEY_DOWN:
			cursor += 1;
			break;
		case GUI_EVT_KEY_RIGHT:
			switch (cursor)
			{
				case 0:
					if (g_Model.timerCond == 0)
					{
						g_Model.timerCond = MDL_DEFAULT_THR_TIMER;
					}
					else if (g_Model.timerCond == MDL_DEFAULT_THR_TIMER)
					{
						g_Model.timerCond = 200;
					}
					else 
					{
						g_Model.timerCond = 0;
					}
					break;
				case 1:
					g_Model.timer++;
					if (g_Model.timer > 3600)
					{
						g_Model.timer = 3600;
					}
					break;
				case 2:
					g_Model.timerAlarmLimit++;
					if (g_Model.timerAlarmLimit >= g_Model.timer)
					{
						g_Model.timerAlarmLimit = g_Model.timer - 1;
					}
					break;
				default:
					break;
			}
			changedModel = 1;
			break;
		case GUI_EVT_KEY_LEFT:
			switch (cursor)
			{
				case 0:
					if (g_Model.timerCond == 0)
					{
						g_Model.timerCond = 200;
					}
					else if (g_Model.timerCond == MDL_DEFAULT_THR_TIMER)
					{
						g_Model.timerCond = 0;
					}
					else 
					{
						g_Model.timerCond = MDL_DEFAULT_THR_TIMER;
					}
					break;
				case 1:
					g_Model.timer--;
					if (g_Model.timer < 10)
					{
						g_Model.timer = 10;
					}
					break;
				case 2:
					if (g_Model.timerAlarmLimit > 1)
					{
						g_Model.timerAlarmLimit--;
					}
					else
					{
						g_Model.timerAlarmLimit = 1;
					}
					break;
				default:
					break;
			}
			changedModel = 1;
			break;
		case GUI_EVT_POT_MOVE:
			v = (g_RadioRuntime.adc_s[GUI_POT] + 100)*5;
			if (v < 0)
			{
				v = 0;
			}
			switch (cursor)
			{
				case 0:
					break;
				case 1:
					g_Model.timer = v;
					if (g_Model.timer > 3600)
					{
						g_Model.timer = 3600;
					}

					break;
				case 2:
					g_Model.timerAlarmLimit = v;
					if (g_Model.timerAlarmLimit >= g_Model.timer)
					{
						g_Model.timerAlarmLimit = g_Model.timer - 1;
					}
					break;
				default:
					break;
			}
			
			changedModel = 1;
		default:
			break;
	}

	if (cursor > 2)
	{
		cursor = 0;
	}
	if (cursor < 0)
	{
		cursor = 2;
	}

	lcd_clear();
	lcd_puts_P( 0, 0, MNU_MODEL_TIMER_TITLE);

	// 0 - MODE
	lcd_putsAtt(0*LCD_FONT_WIDTH, 2*LCD_FONT_HEIGHT, MNU_MODEL_TIMER_MODE, LCD_NO_INV);
	if (g_Model.timerCond == 0)
	{
		sstart = 0;
	}
	else if (g_Model.timerCond == 200)
	{
		sstart = 2*5;
	}
	else
	{
		sstart = 1*5;
	}
	selString = (char*)(MNU_MODEL_TIMER_MODE_SEL + sstart);
	if (cursor == 0)
	{
		lcd_putsnAtt(8*LCD_FONT_WIDTH, 2*LCD_FONT_HEIGHT, selString, 4, LCD_INVERS);
	}
	else
	{
		lcd_putsnAtt(8*LCD_FONT_WIDTH, 2*LCD_FONT_HEIGHT, selString, 4, LCD_NO_INV);
	}


	// 1 - TIME
	lcd_putsAtt(0*LCD_FONT_WIDTH, 4*LCD_FONT_HEIGHT, MNU_MODEL_TIMER_TIME, LCD_NO_INV);
	menu_build_time_str(g_Model.timer);
	if (cursor == 1)
	{
		lcd_putsAtt(8*LCD_FONT_WIDTH, 4*LCD_FONT_HEIGHT, &g_RadioRuntime.buffer[0], LCD_BSS_INVERS);
	}
	else
	{
		lcd_putsAtt(8*LCD_FONT_WIDTH, 4*LCD_FONT_HEIGHT, &g_RadioRuntime.buffer[0], LCD_BSS_NO_INV);
	}


	// 2 - ALARM
	lcd_putsAtt(0*LCD_FONT_WIDTH, 6*LCD_FONT_HEIGHT, MNU_MODEL_TIMER_ALARM, LCD_NO_INV);
	menu_build_time_str(g_Model.timerAlarmLimit);
	if (cursor == 2)
	{
		lcd_putsAtt(8*LCD_FONT_WIDTH, 6*LCD_FONT_HEIGHT, &g_RadioRuntime.buffer[0], LCD_BSS_INVERS);
	}
	else
	{
		lcd_putsAtt(8*LCD_FONT_WIDTH, 6*LCD_FONT_HEIGHT, &g_RadioRuntime.buffer[0], LCD_BSS_NO_INV);
	}

	return dirty;
}


/*--------------------------------------------------------------------------------
 * menu_model_servo_limits
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_SERVO_LIMITS_TITLE[]	PROGMEM = "Servo Endpoints";

uint8_t menu_model_servo_limits(GUI_EVENT event, uint8_t elapsedTime)
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
			if (servoSelected % 2 == 0)
			{
				g_Model.endPoint[0][servoSelected/2] += 1;
			}
			else
			{
				g_Model.endPoint[1][servoSelected/2] += 1;
			}
			changedModel = 1;
			break;
		case GUI_EVT_KEY_DOWN:
			if (servoSelected % 2 == 0)
			{
				g_Model.endPoint[0][servoSelected/2] -= 1;
			}
			else
			{
				g_Model.endPoint[1][servoSelected/2] -= 1;
			}
			changedModel = 1;
			break;
		case GUI_EVT_POT_MOVE:
			v = 100 + g_RadioRuntime.adc_s[GUI_POT];
			if (servoSelected % 2 == 0)
			{
				g_Model.endPoint[0][servoSelected/2] = v;
			}
			else
			{
				g_Model.endPoint[1][servoSelected/2] = v;
			}
			changedModel = 1;
		default:
			break;
	}

	if (servoSelected > 15)
	{
		servoSelected = servoSelected - 16;
	}
	if (servoSelected < 0)
	{
		servoSelected = servoSelected + 16;
	}

	if (servoSelected % 2 == 0)
	{
		if (g_Model.endPoint[0][servoSelected/2] < 0)
		{
			g_Model.endPoint[0][servoSelected/2] = 0;
		}
		
		if (g_Model.endPoint[0][servoSelected/2] > 120)
		{
			g_Model.endPoint[0][servoSelected/2] = 120;
		}
	}
	else
	{
		if (g_Model.endPoint[1][servoSelected/2] < 0)
		{
			g_Model.endPoint[1][servoSelected/2] = 0;
		}
		
		if (g_Model.endPoint[1][servoSelected/2] > 120)
		{
			g_Model.endPoint[1][servoSelected/2] = 120;
		}
	}


	lcd_clear();
	lcd_puts_P( 0, 0, MNU_MODEL_SERVO_LIMITS_TITLE);
	s = 0;

	for(y=0; ((y<3) && (s<16)); y++)
	{
		for(x=0; ((x<3) && (s<16)); x++)
		{
			// "CHn"
			lcd_putsAtt((x*7)*LCD_FONT_WIDTH, (y*2 + 2)*LCD_FONT_HEIGHT, MNU_MODEL_SERVO_CH, LCD_NO_INV);
			lcd_outdezAtt((x*7 + 3)*LCD_FONT_WIDTH, (y*2 + 2)*LCD_FONT_HEIGHT, (s/2)+1, LCD_NO_INV);

			if (s == servoSelected)
			{
				drawingMode = LCD_INVERS;
			}
			else
			{
				drawingMode = LCD_NO_INV;
			}

			// The LOW value...
			v = g_Model.endPoint[0][s/2];
			lcd_outdezAtt((x*7+6)*LCD_FONT_WIDTH, (y*2 + 2)*LCD_FONT_HEIGHT, v, drawingMode);

			s++;

			if (s == servoSelected)
			{
				drawingMode = LCD_INVERS;
			}
			else
			{
				drawingMode = LCD_NO_INV;
			}

			// The HIGH value...
			v = g_Model.endPoint[1][s/2];
			lcd_outdezAtt((x*7+6)*LCD_FONT_WIDTH, (y*2 + 3)*LCD_FONT_HEIGHT, v, drawingMode);

			s++;
		}
	}

	return dirty;
}

/*--------------------------------------------------------------------------------
 * menu_model_gyro_gain
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_GYRO_TITLE[]	PROGMEM = "Gyro Setup";

char MNU_MODEL_GYRO_AVCS[]		PROGMEM = "AVCS:";
char MNU_MODEL_GYRO_RATE[]		PROGMEM = "RATE:";

char* gyroStr[] PROGMEM = 
{
	MNU_MODEL_GYRO_AVCS,
	MNU_MODEL_GYRO_RATE
};

uint8_t menu_model_gyro_gain(GUI_EVENT event, uint8_t elapsedTime)
{
	uint8_t dirty = 1;
	uint8_t i;
	uint8_t drawingMode;
	int8_t v;


	switch (event)
	{
		case GUI_EVT_SHOW:
			cursor = 0;
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
		case GUI_EVT_KEY_UP:
			cursor -= 1;
			break;
		case GUI_EVT_KEY_DOWN:
			cursor += 1;
			break;
		case GUI_EVT_KEY_RIGHT:
			g_Model.gyro[cursor] = g_Model.gyro[cursor] + 1;
			changedModel = 1;
			break;
		case GUI_EVT_KEY_LEFT:
			g_Model.gyro[cursor] = g_Model.gyro[cursor] - 1;
			changedModel = 1;
			break;
		case GUI_EVT_POT_MOVE:
			v = g_RadioRuntime.adc_s[GUI_POT];
			g_Model.gyro[cursor] = v;
			changedModel = 1;
		default:
			break;
	}

	if (cursor > 1)
	{
		cursor = 0;
	}
	if (cursor < 0)
	{
		cursor = 1;
	}

	if (g_Model.gyro[cursor] > 100)
	{
		g_Model.gyro[cursor] = 100;
	}	
	if (g_Model.gyro[cursor] < -100)
	{
		g_Model.gyro[cursor] = -100;
	}	


	lcd_clear();
	lcd_puts_P( 0, 0, MNU_MODEL_GYRO_TITLE);

	for (i=0; i<2; i++)
	{
		lcd_putsAtt((0)*LCD_FONT_WIDTH, (2 + i)*LCD_FONT_HEIGHT, (char*)pgm_read_word(&gyroStr[i]), LCD_NO_INV);

		if (i == cursor)
		{
			drawingMode = LCD_INVERS;
		}
		else
		{
			drawingMode = LCD_NO_INV;
		}

		// The value...
		v = g_Model.gyro[i];
		lcd_outdezAtt((8)*LCD_FONT_WIDTH, (2 + i)*LCD_FONT_HEIGHT, v, drawingMode);
	}

	return dirty;
}

/*--------------------------------------------------------------------------------
 * menu_model_swash_throw
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_SWASH_TITLE[]	PROGMEM = "Swash throw";

char MNU_MODEL_SWASH_AIL[]		PROGMEM = "AILERON:";
char MNU_MODEL_SWASH_ELE[]		PROGMEM = "ELEVATOR:";
char MNU_MODEL_SWASH_PIT[]		PROGMEM = "PITCH:";

char MNU_MODEL_NO_SWASH_MIX_T[] 	PROGMEM = "   No swash mix!   ";
char MNU_MODEL_NO_SWASH_MIX_2[] 	PROGMEM = "This model does not";
char MNU_MODEL_NO_SWASH_MIX_3[] 	PROGMEM = "use any swash mix. ";


char* swashStr[] PROGMEM = 
{
	MNU_MODEL_SWASH_AIL,
	MNU_MODEL_SWASH_ELE,
	MNU_MODEL_SWASH_PIT
};

uint8_t menu_model_swash_throw(GUI_EVENT event, uint8_t elapsedTime)
{
	uint8_t dirty = 1;
	uint8_t i;
	uint8_t drawingMode;
	int8_t v;


	switch (event)
	{
		case GUI_EVT_SHOW:
			cursor = 0;
			changedModel = 0;
			if ((g_Model.type != MDL_TYPE_HELI_ECCPM_120) && (g_Model.type != MDL_TYPE_HELI_ECCPM_140))
			{
				gui_screen_pop();
				menu_show_messagebox(0, MNU_MODEL_NO_SWASH_MIX_T, MNU_MODEL_EMPTY, MNU_MODEL_NO_SWASH_MIX_2, MNU_MODEL_NO_SWASH_MIX_3, MNU_MODEL_EMPTY);
				return 1;
			}
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
		case GUI_EVT_KEY_UP:
			cursor -= 1;
			break;
		case GUI_EVT_KEY_DOWN:
			cursor += 1;
			break;
		case GUI_EVT_KEY_RIGHT:
			g_Model.swash[cursor] = g_Model.swash[cursor] + 1;
			changedModel = 1;
			break;
		case GUI_EVT_KEY_LEFT:
			g_Model.swash[cursor] = g_Model.swash[cursor] - 1;
			changedModel = 1;
			break;
		case GUI_EVT_POT_MOVE:
			v = g_RadioRuntime.adc_s[GUI_POT];
			g_Model.swash[cursor] = v;
			changedModel = 1;
		default:
			break;
	}

	if (cursor > 2)
	{
		cursor = 0;
	}
	if (cursor < 0)
	{
		cursor = 2;
	}

	if (g_Model.swash[cursor] > 100)
	{
		g_Model.swash[cursor] = 100;
	}	
	if (g_Model.swash[cursor] < -100)
	{
		g_Model.swash[cursor] = -100;
	}	

	lcd_clear();
	lcd_puts_P( 0, 0, MNU_MODEL_SWASH_TITLE);

	for (i=0; i<3; i++)
	{
		lcd_putsAtt((0)*LCD_FONT_WIDTH, (2 + i)*LCD_FONT_HEIGHT, (char*)pgm_read_word(&swashStr[i]), LCD_NO_INV);

		if (i == cursor)
		{
			drawingMode = LCD_INVERS;
		}
		else
		{
			drawingMode = LCD_NO_INV;
		}

		// The value...
		v = g_Model.swash[i];
		lcd_outdezAtt((12)*LCD_FONT_WIDTH, (2 + i)*LCD_FONT_HEIGHT, v, drawingMode);
	}

	return dirty;
}

/*--------------------------------------------------------------------------------
 * menu_model_clone
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_CLONE_TITLE[] 		PROGMEM = "Clone Model";
char MNU_MODEL_FREE[] 				PROGMEM = "**free**";

char MNU_MODEL_CLONED[] 	PROGMEM = "   Model cloned!   ";

char MNU_MODEL_NO_SLOT_T[] 	PROGMEM = "   No free slot!   ";
char MNU_MODEL_NO_SLOT_1[] 	PROGMEM = "All model slots are";
char MNU_MODEL_NO_SLOT_2[] 	PROGMEM = "already used.      ";
char MNU_MODEL_NO_SLOT_3[] 	PROGMEM = "Delete one model   ";
char MNU_MODEL_NO_SLOT_4[] 	PROGMEM = "and try again!     ";


uint8_t menu_model_clone(GUI_EVENT event, uint8_t elapsedTime)
{
	char name[10];
	uint8_t i;
	uint8_t x,y;
	

	switch (event)
	{
		case GUI_EVT_SHOW:
			cursor = 0;
			cursor2 = 0;
			for (i=0; i<EE_MAX_MODELS; i++)
			{
				eeprom_load_model_name(i, name);
				if (name[0] != 0)
				{
					// Set a bit to indicate model exist!
					cursor2 |= (1<<i);
				}
			}

			firstFreeSlot = 0xFF;
			for (i=0; i<EE_MAX_MODELS; i++)
			{
				eeprom_load_model_name(i, name);
				if (name[0] == 0)
				{
					firstFreeSlot = i;
					break;
				}
			}

			if (firstFreeSlot == 0xFF)
			{
				gui_screen_pop();
				menu_show_messagebox(0, MNU_MODEL_NO_SLOT_T, MNU_MODEL_NO_SLOT_1, MNU_MODEL_NO_SLOT_2, MNU_MODEL_NO_SLOT_3, MNU_MODEL_NO_SLOT_4);
				return 1;
			}

			break;
		case GUI_EVT_HIDE:
			break;
		case GUI_EVT_KEY_EXIT:
			gui_screen_pop();
			break;
		case GUI_EVT_KEY_MENU:
			// Disable PPM
			g_RadioRuntime.ppmActive = 0;

			// Load the model to copy...
			eeprom_load_model_config(cursor);

			// Change the name...
			g_Model.name[6] = 'C';
			g_Model.name[7] = 'P';
			g_Model.name[8] = 'Y';
			g_Model.name[9] = 0x0;

			// Save at the free slot...
			eeprom_save_model_config(firstFreeSlot);

			// Load back the current model...
			eeprom_load_model_config(g_RadioConfig.selectedModel);

			// Enable PPM
			g_RadioRuntime.ppmActive = 1;

			gui_screen_pop();
			menu_show_messagebox(0, MNU_MODEL_EMPTY, MNU_MODEL_EMPTY, MNU_MODEL_CLONED, MNU_MODEL_EMPTY, MNU_MODEL_EMPTY);
			return 1;
		break;
		case GUI_EVT_KEY_UP:
			cursor--;
			if (cursor < 0)
			{
				cursor = 7;
			}
			while ((cursor2 & (1 << cursor)) == 0x00)
			{
				cursor--;
				if (cursor < 0)
				{
					cursor = 7;
				}
			}
			break;
		case GUI_EVT_KEY_DOWN:
			cursor++;
			if (cursor > 7)
			{
				cursor = 0;
			}
			while ((cursor2 & (1 << cursor)) == 0x00)
			{
				cursor++;
				if (cursor > 7)
				{
					cursor = 0;
				}
			}

		default:
			break;
	}

	lcd_clear();
	lcd_puts_P( 0, 0, MNU_MODEL_CLONE_TITLE);

	x = 0;
	y = 2;

	for (i=0; i<EE_MAX_MODELS; i++)
	{
		if (i == 4)
		{
			x = 12;
			y = 2;
		}
		eeprom_load_model_name(i, name);
		if (name[0] != 0)
		{
			if (cursor == i)
			{
				lcd_putsAtt(x*LCD_FONT_WIDTH, y*LCD_FONT_HEIGHT, name, LCD_BSS_INVERS);
			}
			else
			{
				lcd_putsAtt(x*LCD_FONT_WIDTH, y*LCD_FONT_HEIGHT, name, LCD_BSS_NO_INV);
			}
		}
		else
		{
			lcd_putsAtt(x*LCD_FONT_WIDTH, y*LCD_FONT_HEIGHT, MNU_MODEL_FREE, LCD_NO_INV);
		}
		y++;
	}

	return 1;
}

/*--------------------------------------------------------------------------------
 * menu_model_delete
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_DELETE_TITLE[] 		PROGMEM = "Delete Model";

char MNU_MODEL_NO_DEL_T[] 	PROGMEM = " Failed to delete! ";
char MNU_MODEL_NO_DEL_1[] 	PROGMEM = "Cannot delete the  ";
char MNU_MODEL_NO_DEL_2[] 	PROGMEM = "selected model.    ";
char MNU_MODEL_NO_DEL_3[] 	PROGMEM = "Select another one ";
char MNU_MODEL_NO_DEL_4[] 	PROGMEM = "and try again!     ";

char MNU_MODEL_DEL[] 	PROGMEM = "   Model deleted!  ";


uint8_t menu_model_delete(GUI_EVENT event, uint8_t elapsedTime)
{
	char name[10];
	uint8_t i;
	uint8_t x,y;
	

	switch (event)
	{
		case GUI_EVT_SHOW:
			cursor = 0;
			cursor2 = 0;
			for (i=0; i<EE_MAX_MODELS; i++)
			{
				eeprom_load_model_name(i, name);
				if (name[0] != 0)
				{
					// Set a bit to indicate model exist!
					cursor2 |= (1<<i);
				}
			}

			break;
		case GUI_EVT_HIDE:
			break;
		case GUI_EVT_KEY_EXIT:
			gui_screen_pop();
			break;
		case GUI_EVT_KEY_MENU:
			// Selected new model?
			if (cursor != g_RadioConfig.selectedModel)
			{
				// Do delete...
				eeprom_delete_model_config(cursor);

				gui_screen_pop();
				menu_show_messagebox(0, MNU_MODEL_EMPTY, MNU_MODEL_EMPTY, MNU_MODEL_DEL, MNU_MODEL_EMPTY, MNU_MODEL_EMPTY);
			}
			else
			{
				// Cannot delete selected model!
				menu_show_messagebox(0, MNU_MODEL_NO_DEL_T, MNU_MODEL_NO_DEL_1, MNU_MODEL_NO_DEL_2, MNU_MODEL_NO_DEL_3, MNU_MODEL_NO_DEL_4);
			}
			return 1;
			break;
		case GUI_EVT_KEY_UP:
			cursor--;
			if (cursor < 0)
			{
				cursor = 7;
			}
			while ((cursor2 & (1 << cursor)) == 0x00)
			{
				cursor--;
				if (cursor < 0)
				{
					cursor = 7;
				}
			}
			break;
		case GUI_EVT_KEY_DOWN:
			cursor++;
			if (cursor > 7)
			{
				cursor = 0;
			}
			while ((cursor2 & (1 << cursor)) == 0x00)
			{
				cursor++;
				if (cursor > 7)
				{
					cursor = 0;
				}
			}

		default:
			break;
	}

	lcd_clear();
	lcd_puts_P( 0, 0, MNU_MODEL_DELETE_TITLE);

	x = 0;
	y = 2;

	for (i=0; i<EE_MAX_MODELS; i++)
	{
		if (i == 4)
		{
			x = 12;
			y = 2;
		}
		eeprom_load_model_name(i, name);
		if (name[0] != 0)
		{
			if (cursor == i)
			{
				lcd_putsAtt(x*LCD_FONT_WIDTH, y*LCD_FONT_HEIGHT, name, LCD_BSS_INVERS);
			}
			else
			{
				lcd_putsAtt(x*LCD_FONT_WIDTH, y*LCD_FONT_HEIGHT, name, LCD_BSS_NO_INV);
			}
		}
		else
		{
			lcd_putsAtt(x*LCD_FONT_WIDTH, y*LCD_FONT_HEIGHT, MNU_MODEL_FREE, LCD_NO_INV);
		}
		y++;
	}

	return 1;
}


/*--------------------------------------------------------------------------------
 * menu_model_create
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_CREATE_TITLE[]	PROGMEM = "Create Model";


char MNU_MODEL_CREATE_TEMPLATE[]	PROGMEM = "Template:";
char MNU_MODEL_CREATE_TYPE0[]		PROGMEM = "Simulator";
char MNU_MODEL_CREATE_TYPE1[]		PROGMEM = "FBL";
char MNU_MODEL_CREATE_TYPE2[]		PROGMEM = "Swash-120";
char MNU_MODEL_CREATE_TYPE3[]		PROGMEM = "Swash-140";

char* heliTypeStr[] PROGMEM = 
{
	MNU_MODEL_CREATE_TYPE0,
	MNU_MODEL_CREATE_TYPE1,
	MNU_MODEL_CREATE_TYPE2,
	MNU_MODEL_CREATE_TYPE3,
};

char MNU_MODEL_MDL_INFO_T[] 	PROGMEM = "New model created! ";
char MNU_MODEL_MDL_INFO_1[] 	PROGMEM = "                   ";
char MNU_MODEL_MDL_INFO_2[] 	PROGMEM = "Do not forget to   ";
char MNU_MODEL_MDL_INFO_3[] 	PROGMEM = "config this model! ";
char MNU_MODEL_MDL_INFO_4[] 	PROGMEM = "                   ";

uint8_t menu_model_create(GUI_EVENT event, uint8_t elapsedTime)
{
	char name[10];
	uint8_t i;

	switch (event)
	{
		case GUI_EVT_SHOW:
						
			cursor = 0;

			firstFreeSlot = 0xFF;
			for (i=0; i<EE_MAX_MODELS; i++)
			{
				eeprom_load_model_name(i, name);
				if (name[0] == 0)
				{
					firstFreeSlot = i;
					break;
				}
			}

			if (firstFreeSlot == 0xFF)
			{
				gui_screen_pop();
				menu_show_messagebox(0, MNU_MODEL_NO_SLOT_T, MNU_MODEL_NO_SLOT_1, MNU_MODEL_NO_SLOT_2, MNU_MODEL_NO_SLOT_3, MNU_MODEL_NO_SLOT_4);
				return 1;
			}

			break;
		case GUI_EVT_HIDE:
			break;
		case GUI_EVT_KEY_MENU:
			// Let's create it...
			switch (heliType)
			{
				case MDL_TYPE_HELI_SIM:
					template_simulator();
					break;
				case MDL_TYPE_HELI_FBL:
					template_fbl();
					break;
				case MDL_TYPE_HELI_ECCPM_120:
					template_swash_120();
					break;
				case MDL_TYPE_HELI_ECCPM_140:
					template_swash_140();
					break;
			}

			// Select the new model and load it
			g_RadioConfig.selectedModel = firstFreeSlot;

			eeprom_save_radio_config();
			eeprom_save_model_config(g_RadioConfig.selectedModel);
		
			gui_screen_pop();
			menu_show_messagebox(0, MNU_MODEL_MDL_INFO_T, MNU_MODEL_MDL_INFO_1, MNU_MODEL_MDL_INFO_2, MNU_MODEL_MDL_INFO_3, MNU_MODEL_MDL_INFO_4);
			return 1;
			break;
		case GUI_EVT_KEY_EXIT:
			gui_screen_pop();
			break;
		case GUI_EVT_KEY_RIGHT:
			if (cursor == 0)
			{
				heliType++;
				if (heliType > 3)
				{
					heliType = 0;
				}
			}
			break;
		case GUI_EVT_KEY_DOWN:
			if (cursor == 0)
			{
				heliType--;
				if (heliType < 0)
				{
					heliType = 3;
				}
			}
			break;
		default:
			break;
	}

	lcd_clear();
	lcd_puts_P( 0, 0, MNU_MODEL_CREATE_TITLE);

	// Helitype
	lcd_puts_P(0*LCD_FONT_WIDTH, 2*LCD_FONT_HEIGHT, MNU_MODEL_CREATE_TEMPLATE);
	
	if (cursor == 0)
	{
		lcd_putsAtt(10*LCD_FONT_WIDTH, 2*LCD_FONT_HEIGHT,  (char*)pgm_read_word(&heliTypeStr[heliType]), LCD_INVERS);
	}
	else
	{
		lcd_putsAtt(10*LCD_FONT_WIDTH, 2*LCD_FONT_HEIGHT, (char*)pgm_read_word(&heliTypeStr[heliType]), LCD_NO_INV);
	}

	return 1;
}



/*--------------------------------------------------------------------------------
 * menu_model_select
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_SELECT_TITLE[] 		PROGMEM = "Select Model";

uint8_t menu_model_select(GUI_EVENT event, uint8_t elapsedTime)
{
	char name[10];
	uint8_t i;
	uint8_t x,y;
	

	switch (event)
	{
		case GUI_EVT_SHOW:
			cursor = g_RadioConfig.selectedModel;
			cursor2 = 0;
			for (i=0; i<EE_MAX_MODELS; i++)
			{
				eeprom_load_model_name(i, name);
				if (name[0] != 0)
				{
					// Set a bit to indicate model exist!
					cursor2 |= (1<<i);
				}
			}

			// DEBUG
			//cursor2 = 0x55;
	
			break;
		case GUI_EVT_HIDE:
			break;
		case GUI_EVT_KEY_EXIT:
			gui_screen_pop();
			break;
		case GUI_EVT_KEY_MENU:
			// Selected new model?
			if (cursor != g_RadioConfig.selectedModel)
			{
				// Select the new model and load it
				g_RadioConfig.selectedModel = cursor;
				eeprom_load_model_config(cursor);

				// Save the new selection in EEPROM
				eeprom_save_radio_config();
			}
			gui_screen_pop();
			break;
		case GUI_EVT_KEY_UP:
			cursor--;
			if (cursor < 0)
			{
				cursor = 7;
			}
			while ((cursor2 & (1 << cursor)) == 0x00)
			{
				cursor--;
				if (cursor < 0)
				{
					cursor = 7;
				}
			}
			break;
		case GUI_EVT_KEY_DOWN:
			cursor++;
			if (cursor > 7)
			{
				cursor = 0;
			}
			while ((cursor2 & (1 << cursor)) == 0x00)
			{
				cursor++;
				if (cursor > 7)
				{
					cursor = 0;
				}
			}

		default:
			break;
	}

	lcd_clear();
	lcd_puts_P( 0, 0, MNU_MODEL_SELECT_TITLE);

	x = 0;
	y = 2;

	for (i=0; i<EE_MAX_MODELS; i++)
	{
		if (i == 4)
		{
			x = 12;
			y = 2;
		}
		eeprom_load_model_name(i, name);
		if (name[0] != 0)
		{
			if (cursor == i)
			{
				lcd_putsAtt(x*LCD_FONT_WIDTH, y*LCD_FONT_HEIGHT, name, LCD_BSS_INVERS);
			}
			else
			{
				lcd_putsAtt(x*LCD_FONT_WIDTH, y*LCD_FONT_HEIGHT, name, LCD_BSS_NO_INV);
			}
		}
		else
		{
			lcd_putsAtt(x*LCD_FONT_WIDTH, y*LCD_FONT_HEIGHT, MNU_MODEL_FREE, LCD_NO_INV);		}
		y++;
	}

	return 1;
}

/*--------------------------------------------------------------------------------
 * menu_info_screen
 *--------------------------------------------------------------------------------*/
char MNU_INFO_TITLE[] 				PROGMEM = "Heliclone Info";
char MNU_INFO_VERSION_TXT[]			PROGMEM = "Version:";
char MNU_INFO_TIME[]	 			PROGMEM = __TIME__;
char MNU_INFO_DATE[]	 			PROGMEM = __DATE__;
char MNU_INFO_COPYR[] 				PROGMEM = "By Stefan Grufman";

uint8_t menu_info_screen(GUI_EVENT event, uint8_t elapsedTime)
{
	switch (event)
	{
		case GUI_EVT_SHOW:
			break;
		case GUI_EVT_HIDE:
			break;
		case GUI_EVT_KEY_EXIT:
			gui_screen_pop();
			break;
		default:
			break;
	}

	lcd_clear();
	lcd_puts_P( 0, 0, MNU_INFO_TITLE);
	lcd_puts_P( 0, 2*LCD_FONT_HEIGHT, MNU_INFO_VERSION_TXT);
	lcd_puts_P( 0, 3*LCD_FONT_HEIGHT, MNU_INFO_DATE);
	lcd_puts_P( 0, 4*LCD_FONT_HEIGHT, MNU_INFO_TIME);
	lcd_puts_P( 0, 6*LCD_FONT_HEIGHT, MNU_INFO_COPYR);


	return 1;
}

/*--------------------------------------------------------------------------------
 * menu_model_expo_edit
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_EXPO_EDIT_TITLE[] 		PROGMEM = "Expo";
char MNU_MODEL_EXPO_AIL[] 				PROGMEM = "AIL";
char MNU_MODEL_EXPO_ELE[] 				PROGMEM = "ELE";
char MNU_MODEL_EXPO_RUD[] 				PROGMEM = "RUD";
char MNU_MODEL_EXPO_DUAL[] 				PROGMEM = "DUAL";
char MNU_MODEL_EXPO_NORM[] 				PROGMEM = "NORM";

uint8_t menu_model_expo_edit(GUI_EVENT event, uint8_t elapsedTime)
{
	int16_t x,y, px, py;
	int8_t i;
	MIX_INPUT input = MIX_IN_AIL;
	char* expoStr;
	char* expoModeStr;
	uint8_t expoMode;
	int16_t adcValue;

	switch (event)
	{
		case GUI_EVT_SHOW:
			cursor = 0;
			cursor2 = 0;
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
				cursor = 1;
			}
			break;
		case GUI_EVT_KEY_DOWN:
			cursor++;
			if (cursor > 1)
			{
				cursor = 0;
			}
			break;
		case GUI_EVT_KEY_RIGHT:
			if (cursor == 0)
			{
				cursor2++;
				if (cursor2 > 2)
				{
					cursor2 = 0;
				}
			}
			break;
		case GUI_EVT_KEY_LEFT:
			if (cursor == 0)
			{
				cursor2--;
				if (cursor2 < 0)
				{
					cursor2 = 2;
				}
			}
			break;
		default:
			break;
	}

	lcd_clear();
	lcd_puts_P( 0, 0, MNU_MODEL_EXPO_EDIT_TITLE);

	// What expo is used...
	expoStr = MNU_MODEL_EXPO_AIL;
	expoMode = MDL_EXPO_NORM;
	expoModeStr = MNU_MODEL_EXPO_NORM;
	adcValue = g_RadioRuntime.adc_s[ADC_AIL];
	switch (cursor2)
	{
		case 0:
			expoStr = MNU_MODEL_EXPO_AIL;
			input = MIX_IN_AIL;
			if (hal_io_get_sw(SW_AILDR))
			{
				expoMode = MDL_EXPO_DUAL;
				expoModeStr = MNU_MODEL_EXPO_DUAL;
			}
			adcValue = g_RadioRuntime.adc_s[ADC_AIL];
			break;
		case 1:
			expoStr = MNU_MODEL_EXPO_ELE;
			input = MIX_IN_ELE;
			if (hal_io_get_sw(SW_ELEDR))
			{
				expoMode = MDL_EXPO_DUAL;
				expoModeStr = MNU_MODEL_EXPO_DUAL;
			}
			adcValue = g_RadioRuntime.adc_s[ADC_ELE];
			break;
		case 2:
			expoStr = MNU_MODEL_EXPO_RUD;
			input = MIX_IN_RUD;
			if (hal_io_get_sw(SW_RUDDR))
			{
				expoMode = MDL_EXPO_DUAL;
				expoModeStr = MNU_MODEL_EXPO_DUAL;
			}
			adcValue = g_RadioRuntime.adc_s[ADC_RUD];
			break;
		default:
			break;
	}


	// Are we in position for some value changes?
	if (cursor == 1)
	{
		switch (event)
		{
			case GUI_EVT_KEY_RIGHT:
				g_Model.expo[input][expoMode] += 1;
				changedModel = 1;
				break;
			case GUI_EVT_KEY_LEFT:
				g_Model.expo[input][expoMode] -= 1;
				changedModel = 1;
				break;
			case GUI_EVT_POT_MOVE:
				g_Model.expo[input][expoMode] = g_RadioRuntime.adc_s[GUI_POT];
				changedModel = 1;
				break;

			default:
				break;
		}
	}

	// Limit
	if (g_Model.expo[input][expoMode] > 100)
	{
		g_Model.expo[input][expoMode] = 100;
	}
	if (g_Model.expo[input][expoMode] < -100)
	{
		g_Model.expo[input][expoMode] = -100;
	}

	if (cursor == 0)
	{
		lcd_putsAtt(0, 2*LCD_FONT_HEIGHT, expoStr, LCD_INVERS);
	}
	else
	{
		lcd_putsAtt(0, 2*LCD_FONT_HEIGHT, expoStr, LCD_NO_INV);
	}
	lcd_puts_P(5*LCD_FONT_WIDTH, 2*LCD_FONT_HEIGHT, expoModeStr);

	// VALUE CHANGE on 1
	if (cursor == 1)
	{
		lcd_outdezAtt(4*LCD_FONT_WIDTH, 4*LCD_FONT_HEIGHT, g_Model.expo[input][expoMode], LCD_INVERS);
	}
	else
	{
		lcd_outdezAtt(4*LCD_FONT_WIDTH, 4*LCD_FONT_HEIGHT, g_Model.expo[input][expoMode], LCD_NO_INV);
	}

	// Draw the X-Y-axis... (64x64 pixels)
	lcd_hline(LCD_DISPLAY_W-63, LCD_DISPLAY_H-32,    63);
	lcd_vline(LCD_DISPLAY_W-32, LCD_DISPLAY_H-64,    64);

	// Scale on X-axis
	lcd_vline(LCD_DISPLAY_W-63, LCD_DISPLAY_H-33,    3);
	lcd_vline(LCD_DISPLAY_W-48, LCD_DISPLAY_H-33,    3);
	lcd_vline(LCD_DISPLAY_W-16, LCD_DISPLAY_H-33,    3);
	lcd_vline(LCD_DISPLAY_W-1, LCD_DISPLAY_H-33,    3);

	// Scale on Y-axis
	lcd_hline(LCD_DISPLAY_W-33, LCD_DISPLAY_H-64,    3);
	lcd_hline(LCD_DISPLAY_W-33, LCD_DISPLAY_H-48,    3);
	lcd_hline(LCD_DISPLAY_W-33, LCD_DISPLAY_H-16,    3);
	lcd_hline(LCD_DISPLAY_W-33, LCD_DISPLAY_H-1,    3);

	// Draw the curve...
	for (i=-100; i<=100; i++)
	{
		x = i;
		y = mixer_expo(input, x);

		px = 97 + (x*32/100);
		py = 31 - (y*32/100);

		if (px < 0)
		{
			px = 0;
		}
		if (px >= LCD_DISPLAY_W)
		{
			px = LCD_DISPLAY_W-1;
		}

		if (py < 0)
		{
			py = 0;
		}
		if (py >= LCD_DISPLAY_H)
		{
			py = LCD_DISPLAY_H-1;
		}

		lcd_plot(px, py);
	}

	// Draw the actual position of the input
	px = (LCD_DISPLAY_W-32) + (adcValue*32/100);
	lcd_vline(px, LCD_DISPLAY_H-64,    64);

	return 1;
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
			if (cursor > 8)
			{
				cursor = 0;
			}
			break;
		case GUI_EVT_KEY_LEFT:
			cursor--;
			if (cursor < 0)
			{
				cursor = 8;
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
	for (i=0; i<9; i++)
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
			if (msgbox_countdown != 0xFF)
			{
				msgbox_countdown--;
				// Remove the messagebox?
				if (msgbox_countdown == 0)
				{
					// Remove us...
					gui_screen_pop();
				}
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

void menu_show_messagebox(uint8_t time, char *title, char *row1, char *row2, char* row3, char* row4)
{
	// Store pointers
	msgbox_title = title;
	msgbox_row1 = row1;
	msgbox_row2 = row2;
	msgbox_row3 = row3;
	msgbox_row4 = row4;
	if (time == 0)
	{
		msgbox_countdown = g_RadioConfig.message_box_timeout;
	}
	else
	{
		msgbox_countdown = time;
	}

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
	int16_t adcValue;

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
	lcd_hline(LCD_DISPLAY_W-63, LCD_DISPLAY_H-32,    63);
	lcd_vline(LCD_DISPLAY_W-32, LCD_DISPLAY_H-64,    64);

	// Scale on X-axis
	lcd_vline(LCD_DISPLAY_W-63, LCD_DISPLAY_H-33,    3);
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

		px1 = 97 + (x1*32/100);
		py1 = 31 - (y1*32/100);
		px2 = 97 + (x2*32/100);
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

	// Draw the actual position of the input
	adcValue = g_RadioRuntime.adc_s[ADC_THR];
	px1 = (LCD_DISPLAY_W-32) + (adcValue*32/100);
	lcd_vline(px1, LCD_DISPLAY_H-64,    64);
	
	return 1;
}

/*--------------------------------------------------------------------------------
 * menu_model_servo_direction
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_SERVO_TITLE[] 		PROGMEM = "Servo Direction";
char MNU_MODEL_SERVO_CH_NORM[] 		PROGMEM = "NOR";
char MNU_MODEL_SERVO_CH_INV[] 		PROGMEM = "INV";

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
char MNU_MODEL_SERVO_SUBTRIM_TITLE[]	PROGMEM = "Servo Subtrim";

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
			menu_show_messagebox(0, MNU_ADC_INFO_T, MNU_ADC_INFO_1, MNU_ADC_INFO_2, MNU_ADC_INFO_3, MNU_ADC_INFO_4);
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
 	int32_t v;
	uint8_t drawingMode = LCD_BSS_NO_INV;
	char* buf = &g_RadioRuntime.buffer[0];

	v = ((int32_t)g_RadioRuntime.adc_r[7] - (int32_t)g_RadioConfig.adc_c[7][0]);
	v = v * 700;
	v = v / ((int32_t)g_RadioConfig.adc_c[7][2] - (int32_t)g_RadioConfig.adc_c[7][0]);

	v = v + 500;

	if (v < g_RadioConfig.voltageWarning*10)
	{
	 	// We are under alarm...
		drawingMode = LCD_BSS_INVERS;
	}

	if ((v%100) < 10)
	{
		sprintf(buf, "%d.0%dV", (uint16_t)(v/100), (uint16_t)(v%100));
	}
	else
	{
		sprintf(buf, "%d.%dV", (uint16_t)(v/100), (uint16_t)(v%100));
	}
	lcd_putsAtt(x - LCD_FONT_WIDTH*strlen(buf) - 2, y, buf, drawingMode);
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
int8_t mainDisplayMode = 4;

char MNU_MAIN_TIMER_OFF[] 	PROGMEM = "XX:XX";
char MNU_MAIN_MODE_1[] 	PROGMEM = "Servo output:";
char MNU_MAIN_MODE_2[] 	PROGMEM = "Stick input:";
char MNU_MAIN_MODE_3[] 	PROGMEM = "ADC Raw input:";
char MNU_MAIN_MODE_4_SS[] 	PROGMEM = "SOFTSTART";

#ifdef USE_DEBUG_MODE
char MNU_MAIN_MODE_10[] PROGMEM = "-- DEBUG MODE --";

char debugLine1[20] = {0};
char debugLine2[20] = {0};
#endif

char MNU_MAIN_ZERO_SEC[] 	PROGMEM = "00";

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
			if (mainDisplayMode > 4)
			{
				mainDisplayMode = 0;
			}
			break;
		case GUI_EVT_KEY_DOWN:
			mainDisplayMode--;
			if (mainDisplayMode < 0)
			{
				mainDisplayMode = 4;
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
	lcd_putsnAtt(0*LCD_FONT_WIDTH + 2,  0, g_Model.name, strlen(g_Model.name), LCD_BSS_NO_INV);

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
		case 4:
			adcValue = g_RadioRuntime.modelTimer;
			if (g_Model.timerCond != 0)
			{
				menu_build_time_str(adcValue);
				lcd_putsAtt(6*LCD_FONT_WIDTH,  3*LCD_FONT_HEIGHT+5, &g_RadioRuntime.buffer[0], LCD_BSS_NO_INV|LCD_DBLSIZE);
			}
			else
			{
				lcd_putsAtt(6*LCD_FONT_WIDTH,  3*LCD_FONT_HEIGHT+5, MNU_MAIN_TIMER_OFF, LCD_NO_INV|LCD_DBLSIZE);
			}

#ifdef F_SOFTSTART
			if (g_RadioRuntime.softStart == 1)
			{
				lcd_putsAtt(3*LCD_FONT_WIDTH,  5*LCD_FONT_HEIGHT+5, MNU_MAIN_MODE_4_SS, LCD_NO_INV|LCD_DBLSIZE);
			}
#endif

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
char MNU_MODEL_SELECTION_NAME[] PROGMEM = "Model Management  1/4";
char MNU_MODEL_CONFIG_NAME[] 	PROGMEM = "Model Config      2/4";
char MNU_RADIO_CONFIG_NAME[] 	PROGMEM = "Radio Config      3/4";
char MNU_RADIO_INSTALL_NAME[] 	PROGMEM = "Radio Setup&Info  4/4";

SMenu ModelSelection PROGMEM = 
{
	MNU_MODEL_SELECTION_NAME,
	&menu_model_management,
	(void*)&RadioInstall,
	(void*)&ModelConfiguration
};

SMenu ModelConfiguration PROGMEM = 
{
	MNU_MODEL_CONFIG_NAME,
	&menu_model_config,
	(void*)&ModelSelection,
	(void*)&RadioConfiguration,
};

SMenu RadioConfiguration PROGMEM = 
{
	MNU_RADIO_CONFIG_NAME,
	&menu_radio_config,
	(void*)&ModelConfiguration,
	(void*)&RadioInstall,
};

SMenu RadioInstall PROGMEM = 
{
	MNU_RADIO_INSTALL_NAME,
	&menu_radio_install,
	(void*)&RadioConfiguration,
	(void*)&ModelSelection,
};

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

/*--------------------------------------------------------------------------------
 * menu_settings
 *--------------------------------------------------------------------------------*/

uint8_t menu_get_setting(uint8_t parameterId)
{
	switch (parameterId)
	{
		case RC_SET_VOLTAGE:
			return (g_RadioConfig.voltageWarning - 40)/5;
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
		case MC_SET_TYPE:
			return g_Model.type;
			break;
		default:
			break;
	}
	return 0;
}

void menu_set_setting(uint8_t parameterId, uint8_t newValue)
{
	uint8_t saveEeprom = 0;

	switch (parameterId)
	{
		case RC_SET_VOLTAGE:
			g_RadioConfig.voltageWarning = 40 + (newValue*5);
			if (g_RadioConfig.voltageWarning > 100)
			{
				g_RadioConfig.voltageWarning = 100;
			}
			if (g_RadioConfig.voltageWarning < 40)
			{
				g_RadioConfig.voltageWarning = 40;
			}
			saveEeprom = 1;
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
			if (g_RadioConfig.backlight == 1)
			{
				lcd_backlight(LCD_BACKLIGHT_OFF);
			}
			else
			{
				lcd_backlight(LCD_BACKLIGHT_ON);
			}
			saveEeprom = 1;
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
			saveEeprom = 1;
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
			saveEeprom = 1;
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
		case MC_SET_TYPE:
			g_Model.type = newValue;
			if (g_Model.type == 0xFF)
			{
				g_Model.type = 0;
			}
			if (g_Model.type > 3)
			{
				g_Model.type = 3;
			}
			saveEeprom = 2;
			break;
		default:
			break;
	}

	if (saveEeprom == 1)
	{
		eeprom_save_radio_config();
	}
	if (saveEeprom == 2)
	{
		eeprom_save_model_config(g_RadioConfig.selectedModel);
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

/*--------------------------------------------------------------------------------
 * menu_model_config
 *--------------------------------------------------------------------------------*/
char MNU_MODEL_CONFIG_TYPE[] 			PROGMEM = "Model Type";
char MNU_MODEL_CONFIG_TYPE_SEL[] 		PROGMEM = "SIM |FBL |S120|S140";

SSelection modelConfig[12] PROGMEM = 
{
	{
		0x1b,
		MNU_MODEL_TIMER_TITLE,
		0,
		0,
		menu_model_timer_setup
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
		0x17,
		MNU_MODEL_GYRO_TITLE,
		0,
		0,
		&menu_model_gyro_gain
	},
	{
		0x18,
		MNU_MODEL_SWASH_TITLE,
		0,
		0,
		&menu_model_swash_throw
	},
	{
		0x1b,
		MNU_MODEL_SERVO_MAP_TITLE,
		0,
		0,
		&menu_model_servo_mapping
	},
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
		0x15,
		MNU_MODEL_SERVO_LIMITS_TITLE,
		0,
		0,
		&menu_model_servo_limits
	},
	{
		0x16,
		MNU_MODEL_EXPO_EDIT_TITLE,
		0,
		0,
		&menu_model_expo_edit
	},
	{
		0x19,
		MNU_MODEL_NAME_EDIT_TITLE,
		0,
		0,
		&menu_model_name_edit
	},
	{
		0x1a,
		MNU_MODEL_CONFIG_TYPE,
		MNU_MODEL_CONFIG_TYPE_SEL,
		MC_SET_TYPE,
		0
	}

};


uint8_t menu_model_config(GUI_EVENT event, uint8_t elapsedTime)
{
	switch (event)
	{
		case GUI_EVT_SHOW:
			numSettings = 12;
			currentSettings = (SSelection*)&modelConfig[0];
			break;
		default:
			break;
	}

	return menu_settings(event, elapsedTime);
}

/*--------------------------------------------------------------------------------
 * menu_radio_config
 *--------------------------------------------------------------------------------*/
char MNU_RADIO_CONFIG_VOLTAGE[] 		PROGMEM = "Voltage Warning";
char MNU_RADIO_CONFIG_VOLTAGE_SEL[] 	PROGMEM = "4.0 |4.5 |5.0 |5.5 |6.0 |6.5 |7.0 |7.5 |8.0 |8.5 |9.0 |9.5 |10.0";
char MNU_RADIO_CONFIG_BACKLIGHT[] 		PROGMEM = "Backlight";
char MNU_RADIO_CONFIG_BACKLIGHT_SEL[] 	PROGMEM = "ON  |OFF |5s  |10s |15s |20s ";
char MNU_RADIO_CONFIG_BEEP_KEYS[] 		PROGMEM = "BEEP Keys";
char MNU_RADIO_CONFIG_BEEP_KEYS_SEL[]	PROGMEM = "OFF |ON  ";
char MNU_RADIO_CONFIG_BEEP_ALARMS[] 	PROGMEM = "BEEP Alarms";
char MNU_RADIO_CONFIG_BEEP_ALARMS_SEL[] PROGMEM = "OFF | 1  | 2  | 3  ";


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


/*--------------------------------------------------------------------------------
 * menu_radio_install
 *--------------------------------------------------------------------------------*/
char MNU_RADIO_INSTALL_CALIBRATE[] 		PROGMEM = "Calibrate Sticks";
char MNU_RADIO_INSTALL_INFO[] 			PROGMEM = "About";


SSelection radioInstalling[2] PROGMEM = 
{
	{
		0x30,
		MNU_RADIO_INSTALL_CALIBRATE,
		0,
		0,
		&menu_adc_calibrate
	},
	{
		0x31,
		MNU_RADIO_INSTALL_INFO,
		0,
		0,
		&menu_info_screen
	}

	
};


uint8_t menu_radio_install(GUI_EVENT event, uint8_t elapsedTime)
{
	switch (event)
	{
		case GUI_EVT_SHOW:
			numSettings = 2;
			currentSettings = (SSelection*)&radioInstalling[0];
			break;
		default:
			break;
	}

	return menu_settings(event, elapsedTime);
}


/*--------------------------------------------------------------------------------
 * menu_model_management
 *--------------------------------------------------------------------------------*/

SSelection modelManagement[4] PROGMEM = 
{
	{
		0x40,
		MNU_MODEL_SELECT_TITLE,
		0,
		0,
		&menu_model_select
	},
	{
		0x41,
		MNU_MODEL_CREATE_TITLE,
		0,
		0,
		&menu_model_create
	},
	{
		0x42,
		MNU_MODEL_DELETE_TITLE,
		0,
		0,
		&menu_model_delete
	},
	{
		0x43,
		MNU_MODEL_CLONE_TITLE,
		0,
		0,
		&menu_model_clone
	}
};


uint8_t menu_model_management(GUI_EVENT event, uint8_t elapsedTime)
{
	switch (event)
	{
		case GUI_EVT_SHOW:
			numSettings = 4;
			currentSettings = (SSelection*)&modelManagement[0];
			break;
		default:
			break;
	}

	return menu_settings(event, elapsedTime);
}

