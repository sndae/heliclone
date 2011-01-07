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

#ifndef _HAL_IO_H_
#define _HAL_IO_H_

#include <avr/io.h>
#include <avr/pgmspace.h>


#define ADC_VREF_TYPE 0x40


/*--------------------------------------------------------------------------------
 * Port configurations
 *--------------------------------------------------------------------------------*/
#define  devTrimsDRR  DDRD
#define  devTrimsPIN  PIND
#define  devTrimsPORT PORTD
#define  devTrimsMask 0xff

#define  devKeysDRR  DDRB
#define  devKeysPIN  PINB
#define  devKeysPORT PORTB
#define  devKeysMask 0x7E


#define  devSwitches1DRR  DDRE
#define  devSwitches1PIN  PINE
#define  devSwitches1PORT PORTE
#define  devSwitches1Mask 0x77

#define  devSwitches2DRR  DDRG
#define  devSwitches2PIN  PING
#define  devSwitches2PORT PORTG
#define  devSwitches2Mask 0x0B

// ppm out , PB0
#define  devPPMoutDRR  DDRB
#define  devPPMoutPORT PORTB
#define  devPPMoutPin  0 // PB0


// ppm in PE7
#define  devPPMinDRR  DDRE
#define  devPPMinPORT PORTE
#define  devPPMinPin  7 // PE7


#define OUT_G_SIM_CTL  4 //1 : phone-jack=ppm_in


// beep
#define  devBeepDRR   DDRE
#define  devBeepPIN   PINE
#define  devBeepPORT  PORTE
#define  devBeepPin   3 // PE3

// LCD backlight
#define  devBacklightDRR   DDRB
#define  devBacklightPORT  PORTB
#define  devBacklightPin   7 

// PRESCALER
#define devPrescale_1     0x01
#define devPrescale_8     0x02
#define devPrescale_32    0x03
#define devPrescale_64    0x04
#define devPrescale_128   0x05
#define devPrescale_256   0x06
#define devPrescale_1024  0x07


typedef enum
{
	SW_ID0,
	SW_ID1,
	SW_ID2,
	SW_AILDR,
	SW_ELEDR,
	SW_RUDDR,
	SW_TRN,
	SW_GEAR,
	SW_THR,
} SW_IDS;


/*--------------------------------------------------------------------------------
 * External interface
 *--------------------------------------------------------------------------------*/
extern void hal_io_init(void);
extern void hal_io_handle(uint8_t elapsedTime);


#endif //_HAL_IO_H_
