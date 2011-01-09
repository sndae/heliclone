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

#ifndef _EEPROM_H_
#define _EEPROM_H_

#include <stdint.h>


#define EE_MAP_START		(0)
#define EE_MAGIC			(EE_MAP_START + 0)			// 2 bytes ('HC')
#define EE_VERSION			(EE_MAP_START + 2)			// 2 bytes (uint16_t)

#define EE_RADIO_CONFIG		(EE_MAP_START + 98)
#define EE_MODEL_CONGFIG(N)	(EE_MAP_START + (100*N))	// Each model gets 100 bytes..

#define EE_MAX_MODELS		(16)						// 16*100 bytes


extern void eeprom_init();
extern void eeprom_save_radio_config();
extern void eeprom_load_radio_config();
extern void eeprom_save_model_config(uint8_t modelNumber);
extern void eeprom_load_model_config(uint8_t modelNumber);

#endif // _EEPROM_H_