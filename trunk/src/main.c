/*
 *  HeliClone  - Copyright (C) 2010
 *               Stefan Grufman <gruffypuffy <at> gmail.com>
 *
 *  Description: Replacement software for model radio:
 *               FS-TH9X / Turnigy 9X / Eurgle 9X / iMax 9X.
 *
 *  Based on various projects for this radion:
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
#include "gui.h"
#include "adc.h"
#include "menu.h"
#include "globals.h"
#include <avr\interrupt.h>
#include <string.h>


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
	for (i=0; i<8; i++)
	{
		g_RadioConfig.adc_c[i][0] = 0x0001;
		g_RadioConfig.adc_c[i][1] = 0x0200;
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

	// Load current model
	memset(&g_Model, 0, sizeof(g_Model));
	strncpy(&(g_Model.name[0]), "Model 1", 10);

}

/*--------------------------------------------------------------------------------
 * main - the main function
 *--------------------------------------------------------------------------------*/
int main(void)
{
	// Init runtime struct
	memset(&g_RadioRuntime, 0, sizeof(g_RadioRuntime));

    // Init stuff..
    hal_io_init();

    // LCD
    lcd_init();
    lcd_contrast(25);
    lcd_clear();
    lcd_refresh();

	// Load defalt settings...
	load_defaults();
	
	// ADC
	adc_init();

	// Start everything...
	init_main_tick();
	sei();

	// gui
	gui_init();
	menu_init();

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
