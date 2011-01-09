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

#include "eeprom.h"
#include <avr/eeprom.h>

/*--------------------------------------------------------------------------------
 * LOCALS
 *--------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------
 * eeprom_init
 *--------------------------------------------------------------------------------*/
void eeprom_init()
{
}


/*--------------------------------------------------------------------------------
 * eeprom_check
 *--------------------------------------------------------------------------------*/
uint8_t eeprom_check()
{
	uint16_t buf;

	// Get the magic number (
	eeprom_read_block(&buf, (void*)EE_MAGIC, sizeof(buf));
	if (buf != 0x4843) // 'HC'	
	{
		return 0; 
	}

	eeprom_read_block(&buf, (void*)EE_VERSION, sizeof(buf));
	if (buf != EE_VERSION_NUMBER) 
	{ 
		return 0;
	}

	return 1;
}


/*--------------------------------------------------------------------------------
 * eeprom_save_version
 *--------------------------------------------------------------------------------*/
void eeprom_save_version()
{
}

/*--------------------------------------------------------------------------------
 * eeprom_save_radio_config
 *--------------------------------------------------------------------------------*/
void eeprom_save_radio_config()
{
}

/*--------------------------------------------------------------------------------
 * eeprom_load_radio_config
 *--------------------------------------------------------------------------------*/
void eeprom_load_radio_config()
{
}

/*--------------------------------------------------------------------------------
 * eeprom_save_model_config
 *--------------------------------------------------------------------------------*/
void eeprom_save_model_config(uint8_t modelNumber)
{
}

/*--------------------------------------------------------------------------------
 * eeprom_load_model_config
 *--------------------------------------------------------------------------------*/
void eeprom_load_model_config(uint8_t modelNumber)
{
}


