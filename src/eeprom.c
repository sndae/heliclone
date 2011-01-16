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
#include "globals.h"
#include <avr/eeprom.h>
#include <string.h>

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

	// Get the magic number
	eeprom_read_block(&buf, (void*)EE_MAGIC, sizeof(buf));
	if (buf != EE_MAGIC_NUMBER)	
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
	
	// Get the magic number
	uint16_t buf = EE_MAGIC_NUMBER;
	eeprom_write_block(&buf, (void*)EE_MAGIC, sizeof(buf));

	while (eeprom_is_ready()!=0);

	buf = EE_VERSION_NUMBER;
	eeprom_write_block(&buf, (void*)EE_VERSION, sizeof(buf));

	while (eeprom_is_ready()!=0);
}

/*--------------------------------------------------------------------------------
 * eeprom_save_radio_config
 *--------------------------------------------------------------------------------*/
void eeprom_save_radio_config()
{
	eeprom_write_block(&g_RadioConfig, (void*)EE_RADIO_CONFIG, sizeof(SRadioConfig));
	
	while (eeprom_is_ready()!=0);
}

/*--------------------------------------------------------------------------------
 * eeprom_load_radio_config
 *--------------------------------------------------------------------------------*/
void eeprom_load_radio_config()
{
	eeprom_read_block(&g_RadioConfig, (void*)EE_RADIO_CONFIG, sizeof(SRadioConfig));
}

/*--------------------------------------------------------------------------------
 * eeprom_save_model_config
 *--------------------------------------------------------------------------------*/
void eeprom_save_model_config(uint8_t modelNumber)
{
	eeprom_write_block(&g_Model, (void*)EE_MODEL_CONGFIG(modelNumber), sizeof(SModel));
	
	while (eeprom_is_ready()!=0);
}

/*--------------------------------------------------------------------------------
 * eeprom_delete_model_config
 *--------------------------------------------------------------------------------*/
void eeprom_delete_model_config(uint8_t modelNumber)
{
	if (g_RadioConfig.selectedModel == modelNumber)
	{
		// Safety...should not happen!
		return;
	}

	// Disable PPM
	g_RadioRuntime.ppmActive = 0;

    // Clear the "current RAM model" memory...
	memset(&g_Model, 0, sizeof(g_Model));

	// Use the empty data to save a "zeroed" model...
	eeprom_save_model_config(modelNumber);

	// Load back the original model
	eeprom_load_model_config(g_RadioConfig.selectedModel);
	
	// Enable PPM
	g_RadioRuntime.ppmActive = 1;

}

/*--------------------------------------------------------------------------------
 * eeprom_load_model_config
 *--------------------------------------------------------------------------------*/
void eeprom_load_model_config(uint8_t modelNumber)
{
	eeprom_read_block(&g_Model, (void*)EE_MODEL_CONGFIG(modelNumber), sizeof(SModel));
}

/*--------------------------------------------------------------------------------
 * eeprom_save_model_trim
 *--------------------------------------------------------------------------------*/
void eeprom_save_model_trim(uint8_t modelNumber)
{
    uint16_t position;
	position = ((uint16_t)&(g_Model.trim)) - ((uint16_t)&g_Model);
	position = position + EE_MODEL_CONGFIG(modelNumber);

	eeprom_write_block(&g_Model.trim,(void*)position, sizeof(g_Model.trim));

	while (eeprom_is_ready()!=0);
}

/*--------------------------------------------------------------------------------
 * eeprom_load_model_name
 *--------------------------------------------------------------------------------*/
void eeprom_load_model_name(uint8_t modelNumber, char* nameBuf)
{
    uint16_t position;
	position = ((uint16_t)&(g_Model.name)) - ((uint16_t)&g_Model);
	position = position + EE_MODEL_CONGFIG(modelNumber);

	eeprom_read_block(nameBuf, (void*)position, sizeof(g_Model.name));
}
