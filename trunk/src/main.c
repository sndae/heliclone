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

#include "lcd.h"
#include "hal_io.h"
#include "ppm.h"
#include "gui.h"
#include "adc.h"
#include "menu.h"
#include "mixer.h"
#include "globals.h"
#include "template.h"
#include "eeprom.h"
#include <avr\interrupt.h>
#include <string.h>
#include <stdio.h>

/*--------------------------------------------------------------------------------
 * GLOBALS
 *--------------------------------------------------------------------------------*/
SRadioConfig g_RadioConfig;
SRadioRuntime g_RadioRuntime;
SModel g_Model;

/*--------------------------------------------------------------------------------
 * LOCALS
 *--------------------------------------------------------------------------------*/
static uint8_t beep8 = 0;
static uint8_t tick_beep = 0;
static uint8_t tick_alarm = 0;
static uint8_t tick_volt_alarm = 5;

/*--------------------------------------------------------------------------------
 * Defines & Macros
 *--------------------------------------------------------------------------------*/
#define TMR0_TICK (156)

#define ALARM_BEEP_EVERY (3)

// Defined in ms
#define GUI_EVERY_TICK (100)
#define KEY_EVERY_TICK (100)
#define IO_EVERY_TICK (100)


char EEPROM_INFO_T[] 	PROGMEM = " EEPROM Needs Init ";
char EEPROM_INFO_1[] 	PROGMEM = "                   ";
char EEPROM_INFO_2[] 	PROGMEM = "  Please wait for  ";
char EEPROM_INFO_3[] 	PROGMEM = "  EEPROM to be     ";
char EEPROM_INFO_4[] 	PROGMEM = "  initialized!!!   ";

char SWITCH_INFO_T[] 	PROGMEM = "   Switch Error!   ";
char SWITCH_INFO_1[] 	PROGMEM = "                   ";
char SWITCH_INFO_2[] 	PROGMEM = " Move all switches ";
char SWITCH_INFO_3[] 	PROGMEM = " to back position! ";
char SWITCH_INFO_4[] 	PROGMEM = "                   ";
/*--------------------------------------------------------------------------------
 * init_main_tick
 *--------------------------------------------------------------------------------*/
void init_main_tick()
{

	// Timer 0 configuration (after Erazz / Thus) 
	TCCR0  = (7 << CS00);//  Norm mode, clk/1024
  	OCR0   = TMR0_TICK;
  	TIMSK |= (1<<OCIE0) ;
}

/*--------------------------------------------------------------------------------
 * load_model_defaults
 *--------------------------------------------------------------------------------*/
void load_model_defaults()
{

	// Load current model
	memset(&g_Model, 0, sizeof(g_Model));

	// Use a template to create defaults
	template_simulator();

	// The one and only model from start...
	g_RadioConfig.selectedModel = 0;
}

/*--------------------------------------------------------------------------------
 * load_defaults
 *--------------------------------------------------------------------------------*/
void load_defaults()
{
	uint8_t i=0;

	// Load defalt settings...
	memset(&g_RadioConfig, 0, sizeof(g_RadioConfig));

	// LCD
	g_RadioConfig.contrast = 25;

	// BEEP
	g_RadioConfig.keyBeep = 0; // OFF
	g_RadioConfig.alarmBeep = 1; // ONE BEEP
	g_RadioConfig.volumeBeep = 0xFF; 

	for (i=0; i<7; i++)
	{
		g_RadioConfig.adc_c[i][0] = 0xFFFF;
		g_RadioConfig.adc_c[i][1] = 0x0200;
		g_RadioConfig.adc_c[i][2] = 0x0000;
	}

	// DEBUG (Precalibrated)
	// Gryffys radio calib data... ;)
	g_RadioConfig.adc_c[0][0] = 0x0053;
	g_RadioConfig.adc_c[0][1] = 0x0209;
	g_RadioConfig.adc_c[0][2] = 0x03C1;

	g_RadioConfig.adc_c[1][0] = 0x003B;
	g_RadioConfig.adc_c[1][1] = 0x01E6;
	g_RadioConfig.adc_c[1][2] = 0x03BF;

	g_RadioConfig.adc_c[2][0] = 0x0011;
	g_RadioConfig.adc_c[2][1] = 0x01CE;
	g_RadioConfig.adc_c[2][2] = 0x036C;

	g_RadioConfig.adc_c[3][0] = 0x004C;
	g_RadioConfig.adc_c[3][1] = 0x0203;
	g_RadioConfig.adc_c[3][2] = 0x03C3;

	for (i=4; i<8; i++)
	{
		g_RadioConfig.adc_c[i][0] = 0x0013;
		g_RadioConfig.adc_c[i][1] = 0x0203;
		g_RadioConfig.adc_c[i][2] = 0x03FF;
	}


	// Default BATTERY readings.
	g_RadioConfig.adc_c[7][0]=385; // 5V
	g_RadioConfig.adc_c[7][1]=658;  // 9V
	g_RadioConfig.adc_c[7][2]=863; // 12V


	// MENUS (5 seconds, i.e. 50 GUI ticks)
	g_RadioConfig.message_box_timeout = 50;

	g_RadioConfig.voltageWarning = 40;
	g_RadioConfig.backlight = 2;

}

/*--------------------------------------------------------------------------------
 * handle_timers
 *--------------------------------------------------------------------------------*/
void handle_timers()
{
	int16_t adcV;
	uint8_t timerTicking = 0;

	if (g_RadioRuntime.modelTimer != 0)
	{
		if ((g_Model.timerCond > 0) && (g_Model.timerCond <= 100))
		{
			// Throttle % timer mode
			adcV = g_RadioRuntime.adc_s[ADC_THR];
			
			// Convert to 0...200
			adcV = adcV + 100;

			// Convert to %
			adcV = adcV / 2;

			if (adcV >= g_Model.timerCond)
			{
				g_RadioRuntime.modelTimer--;
				timerTicking = 1;
			}
		}
		else if (g_Model.timerCond == 200)
		{
			if (g_RadioRuntime.timerStarted)
			{
				g_RadioRuntime.modelTimer--;
				timerTicking = 1;
			}
		}
	}


	// Only sound Alarm if timer is active
	if (timerTicking == 1)
	{
		if (g_RadioRuntime.modelTimer <= g_Model.timerAlarmLimit)
		{
			// Alarm!!! 
			tick_alarm++;

			if (((tick_alarm % ALARM_BEEP_EVERY) == 0) || (tick_alarm == 1))
			{
				gui_beep(g_RadioConfig.alarmBeep);
				lcd_backlight(LCD_BACKLIGHT_ON);
			}
		}
		else
		{
			// Reset this one
			tick_alarm = 0;
		}
	}

	// Backlight timer
	lcd_backlight_timer();

	// Voltage alarm?
	tick_volt_alarm--;
	if (tick_volt_alarm == 0)
	{
		adcV = ((int32_t)g_RadioRuntime.adc_r[7] - (int32_t)g_RadioConfig.adc_c[7][0]);
		adcV = adcV * 700;
		adcV = adcV / ((int32_t)g_RadioConfig.adc_c[7][2] - (int32_t)g_RadioConfig.adc_c[7][0]);
		adcV = adcV + 500;

		if (adcV < g_RadioConfig.voltageWarning*10)
		{
			gui_beep(g_RadioConfig.alarmBeep);
			lcd_backlight(LCD_BACKLIGHT_ON);
		}
		tick_volt_alarm = 5;
	}

}

/*--------------------------------------------------------------------------------
 * main - the main function
 *--------------------------------------------------------------------------------*/
int main(void)
{
	uint8_t i = 0;

	// Init runtime struct
	memset(&g_RadioRuntime, 0, sizeof(g_RadioRuntime));

	// No PPM out...
	g_RadioRuntime.ppmActive = 0;

    // Init stuff..
    hal_io_init();

	// Eeprom
	eeprom_init();

    // LCD
    lcd_init();
    lcd_contrast(25);
    lcd_clear();
    lcd_refresh();

	// ADC
	adc_init();

	// MIXER
	mixer_init();

	// Start everything...that needs IRQs...
	init_main_tick();
	ppm_init();
	sei();

	// gui
	gui_init();
	menu_init();

	// Check for default SWITCHES settings.
	hal_io_handle(IO_EVERY_TICK);
	if (hal_io_sw_is_default() == 0)
	{
		menu_show_messagebox(0xFF, SWITCH_INFO_T, SWITCH_INFO_1, SWITCH_INFO_2, SWITCH_INFO_3, SWITCH_INFO_4);
		gui_execute(GUI_EVERY_TICK);

		// Stay here if we have switches in wrong pos
		while (hal_io_sw_is_default() == 0)
		{
			hal_io_handle(IO_EVERY_TICK);
		}

		gui_screen_pop();
	}

	// Check EEPROM first, if not OK we must init it...
	if (eeprom_check() == 0)
	{
		// Make sure we post a message about this...
		menu_show_messagebox(0xFF, EEPROM_INFO_T, EEPROM_INFO_1, EEPROM_INFO_2, EEPROM_INFO_3, EEPROM_INFO_4);
		gui_execute(GUI_EVERY_TICK);

		// Load defalt settings...
		load_defaults();

		// Save defaults...
		eeprom_save_radio_config();


		// Clear models...
		memset(&g_Model, 0, sizeof(g_Model));
		for (i=1; i<EE_MAX_MODELS; i++)
		{
			eeprom_save_model_config(i);
		}

		// Create and save a default model...
		load_model_defaults();
		eeprom_save_model_config(g_RadioConfig.selectedModel);

		// Set the new timer data
		g_RadioRuntime.modelTimer = g_Model.timer;

		eeprom_save_version();

		gui_screen_pop();
	}
	else
	{
		// Load the user configs...
		eeprom_load_radio_config();
		eeprom_load_model_config(g_RadioConfig.selectedModel);
	}

	// Turn on backlight (if config allows)
	lcd_backlight(LCD_BACKLIGHT_ON);

	// Enable PPM
	g_RadioRuntime.ppmActive = 1;

	// Main loop.
    while (1 == 1)
	{
		// ADC
		adc_sample();

		// IO (Trims & Switches)
		if (g_RadioRuntime.doSignal & DO_IO)
		{
			g_RadioRuntime.doSignal &= ~DO_IO;
			hal_io_handle(IO_EVERY_TICK);
		}

		// GUI
		if (g_RadioRuntime.doSignal & DO_GUI)
		{
			g_RadioRuntime.doSignal &= ~DO_GUI;
			gui_execute(GUI_EVERY_TICK);
		}

		// KEYS
		if (g_RadioRuntime.doSignal & DO_KEYS)
		{
			g_RadioRuntime.doSignal &= ~DO_KEYS;
			gui_handle_keys(KEY_EVERY_TICK);
		}

		// CLOCK
		if (g_RadioRuntime.doSignal & DO_CLOCK)
		{
			g_RadioRuntime.doSignal &= ~DO_CLOCK;
			handle_timers();
		}

		// MIXER
		mixer_mix();

		// Export PPM data to PPM ISR
		if (g_RadioRuntime.ppmActive)
		{
			ppm_export();
		}
		
	}

    return(0);
}



/*--------------------------------------------------------------------------------
 * Main tick
 *--------------------------------------------------------------------------------*/
// tick 1/80 sec 
ISR(TIMER0_COMP_vect, ISR_NOBLOCK) //10ms timer
{
	/*
	cli();
	TIMSK &= ~(1<<OCIE0); //stop reentrance
	sei();
	*/

    OCR0 = OCR0 + TMR0_TICK;

	// Save in MS
	g_RadioRuntime.systemTick += 10;

	if ((g_RadioRuntime.systemTick % 1000) == 0)
	{
		g_RadioRuntime.secondTick++;
		g_RadioRuntime.doSignal |= DO_CLOCK;
	}

	// Signal time for GUI
	if ((g_RadioRuntime.systemTick % GUI_EVERY_TICK) == 0)
	{
		g_RadioRuntime.doSignal |= DO_GUI;
	}

	// Signal time for KEYS
	if ((g_RadioRuntime.systemTick % KEY_EVERY_TICK) == 0)
	{
		g_RadioRuntime.doSignal |= DO_KEYS;
	}

	// Signal time for IO
	if ((g_RadioRuntime.systemTick % IO_EVERY_TICK) == 0)
	{
		g_RadioRuntime.doSignal |= DO_IO;
	}

	// beep - playing sound
	if (beep8 & 0x01)
	{
		devBeepPORT |= (1<<devBeepPin); // <-1
	}
	else
	{
		devBeepPORT &= ~(1<<devBeepPin); // <-0
	}

	beep8 >>=1;
	tick_beep++;

	if (tick_beep >= 8) 
	{
		// new bit (every 1/10*8 = 80ms)
		if (g_RadioRuntime.beep & 0x0001)
		{
			beep8 = g_RadioRuntime.beepStyle;
		}

		g_RadioRuntime.beep >>= 1; // next bit 

		tick_beep=0;
	}

	/*
	cli();
	TIMSK |= (1<<OCIE0);
	sei();
	*/
}
