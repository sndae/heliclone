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

#ifndef _LCD_H_
#define _LCD_H_


#include <stdint.h>

#define LCD_DISPLAY_W          128
#define LCD_DISPLAY_H          64


#define LCD_NO_INV             0x00
#define LCD_INVERS             0x01
#define LCD_BLINK              0x02
#define LCD_LEADING0           0x04
#define LCD_SIGN               0x08

#define LCD_BSS_NO_INV         0x10
#define LCD_BSS_INVERS         0x11
#define LCD_BSS_BLINK          0x12
#define LCD_PREC1              0x20
#define LCD_PREC2              0x40
#define LCD_PREC3              0x60
#define LCD_PREC(n)    (((n) >> 5) & 3)
#define LCD_DBLSIZE            0x80

/* flag for lcd_putcAtt() */
#define LCD_CONDENSE_LETTER    0x01

#define LCD_FONT_WIDTH           6
#define LCD_FONT_WIDTH_NUM       5
#define LCD_FONT_HEIGHT          8


/*--------------------------------------------------------------------------------
 * External interface
 *--------------------------------------------------------------------------------*/
extern void lcd_putcAtt(uint8_t x, uint8_t y, char c, uint8_t mode, uint8_t flag);
extern uint8_t lcd_putsAtt(uint8_t x, uint8_t y, char *s, uint8_t mode);
extern void lcd_putsnAtt(uint8_t x, uint8_t y, char *s, uint8_t len, uint8_t mode);

extern void lcd_putc(uint8_t x, uint8_t y, char c);
extern void lcd_puts_P(uint8_t x, uint8_t y, char *s);
extern void lcd_putsn_P(uint8_t x, uint8_t y, char *s, uint8_t len);

extern void lcd_outhex4(uint8_t x, uint8_t y, uint16_t val);
extern void lcd_outdezAtt(uint8_t x, uint8_t y, int16_t val, uint8_t mode);
extern void lcd_outdezNAtt(uint8_t x, uint8_t y, int16_t val, uint8_t mode, uint8_t len);

extern void lcd_outdez(uint8_t x, uint8_t y, int16_t val);

extern void lcd_plot(uint8_t x, uint8_t y);
extern void lcd_hline(uint8_t x, uint8_t y, int8_t w);
extern void lcd_hlineStip(uint8_t x, uint8_t y, int8_t w, uint8_t pat);
extern void lcd_vline(uint8_t x, uint8_t y, int8_t h);

extern void lcd_plot_xor(uint8_t x, uint8_t y);
extern void lcd_hline_xor(uint8_t x, uint8_t y, int8_t w);
extern void lcd_hlineStip_xor(uint8_t x, uint8_t y, int8_t w, uint8_t pat);
extern void lcd_vline_xor(uint8_t x, uint8_t y, int8_t h);

extern void lcd_img_f(uint8_t x, uint8_t y);
extern void lcd_img(uint8_t i_x, uint8_t i_y, uint8_t *imgdat, uint8_t idx, uint8_t mode);

extern void lcd_init();
extern void lcd_clear();
extern void lcd_refresh();
extern void lcd_contrast(uint8_t val);

#endif // _LCD_H_


