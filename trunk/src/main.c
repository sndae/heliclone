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
 * Defines & Macros
 *--------------------------------------------------------------------------------*/
#define TMR0_TICK (156)


// Defined in ms
#define GUI_EVERY_TICK (100)
#define KEY_EVERY_TICK (100)
#define IO_EVERY_TICK (100)


char EEPROM_INFO_T[] 	PROGMEM = " EEPROM Needs Init ";
char EEPROM_INFO_1[] 	PROGMEM = "                   ";
char EEPROM_INFO_2[] 	PROGMEM = "  Please wait for  ";
char EEPROM_INFO_3[] 	PROGMEM = "  EEPROM to be     ";
char EEPROM_INFO_4[] 	PROGMEM = "  initialized!!!   ";

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

	g_RadioConfig.voltageWarning = 70;
	g_RadioConfig.backlight = 1;

	// No PPM out...
	g_RadioRuntime.ppmActive = 0;

}

/*--------------------------------------------------------------------------------
 * main - the main function
 *--------------------------------------------------------------------------------*/
int main(void)
{
	uint8_t i = 0;

	// Init runtime struct
	memset(&g_RadioRuntime, 0, sizeof(g_RadioRuntime));

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


	// Check EEPROM first, if not OK we must init it...
	if (eeprom_check() == 0)
	{
		// Make sure we post a message about this...
		menu_show_messagebox(EEPROM_INFO_T, EEPROM_INFO_1, EEPROM_INFO_2, EEPROM_INFO_3, EEPROM_INFO_4);
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

		eeprom_save_version();
	}
	else
	{
		// Load the user configs...
		eeprom_load_radio_config();
		eeprom_load_model_config(g_RadioConfig.selectedModel);
	}



#ifdef USE_DEBUG_MODE
	sprintf(debugLine1, "Model size: %d", sizeof(SModel));
	sprintf(debugLine2, "Mixer size: %d", sizeof(SMixer));
#endif	

	// Enable PPM
	g_RadioRuntime.ppmActive = 1;

	// Main loop.
    while (1 == 1)
	{
		// ADC
		adc_sample();

		// IO (Trims & Switches)
		if (g_RadioRuntime.doIO == 1)
		{
			g_RadioRuntime.doIO = 0;
			hal_io_handle(IO_EVERY_TICK);
		}

		// GUI
		if (g_RadioRuntime.doGui == 1)
		{
			g_RadioRuntime.doGui = 0;
			gui_execute(GUI_EVERY_TICK);
		}

		// KEYS
		if (g_RadioRuntime.doKeys == 1)
		{
			g_RadioRuntime.doKeys = 0;
			gui_handle_keys(KEY_EVERY_TICK);
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

	// Signal time for GUI
	if ((g_RadioRuntime.systemTick % GUI_EVERY_TICK) == 0)
	{
		g_RadioRuntime.doGui = 1;
	}

	// Signal time for KEYS
	if ((g_RadioRuntime.systemTick % KEY_EVERY_TICK) == 0)
	{
		g_RadioRuntime.doKeys = 1;
	}

	// Signal time for IO
	if ((g_RadioRuntime.systemTick % IO_EVERY_TICK) == 0)
	{
		g_RadioRuntime.doIO = 1;
	}

/*	// beep - playing sound
	if (beep8 & 0x01)
		devBeepPORT |=   (1<<devBeepPin); // <-1
	else
		devBeepPORT &=  ~(1<<devBeepPin); // <-0
	beep8 >>=1;
	tick_beep++;

	if (tick_beep>=8) {// new bit (every 1/10 sec)
		if (beep & 0x0001)
			beep8=volume;
		beep>>=1; // next bit 
		tick_beep=0;


		tick_tsec=0xFF;   // every 1/10 second
		tick10++;
		if (tick10>=10) {
			tick10=0;
			tick_sec=0xFF; // every second
		}
	}
*/

	/*
	cli();
	TIMSK |= (1<<OCIE0);
	sei();
	*/
}
