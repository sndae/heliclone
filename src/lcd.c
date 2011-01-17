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
#include "util.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include <string.h>


/*--------------------------------------------------------------------------------
 * Local variables
 *--------------------------------------------------------------------------------*/
uint8_t displayBuf[LCD_DISPLAY_W * LCD_DISPLAY_H / 8];

#define LCD_DISPLAY_END           (displayBuf + sizeof(displayBuf))

#define LCD_BLINK_TIMER           (1)


/*--------------------------------------------------------------------------------
 * Fonts
 *--------------------------------------------------------------------------------*/
#include "font.lbm"
#define font_5x8_x20_x7f      (font + 3)
#include "font_dblsize.lbm"
#define font_10x16_x20_x7f    (font_dblsize + 3)


/*--------------------------------------------------------------------------------
 * Definitions & Macros
 *--------------------------------------------------------------------------------*/
#define LCD_BITMASK(bit)    (1 << (bit))


#define OUT_C_LCD_E     5
#define OUT_C_LCD_RnW   4
#define OUT_C_LCD_A0    3
#define OUT_C_LCD_RES   2
#define OUT_C_LCD_CS1   1
#define PORTA_LCD_DAT  PORTA
#define PORTC_LCD_CTRL PORTC

/*--------------------------------------------------------------------------------
 * lcd_clear
 *--------------------------------------------------------------------------------*/
void lcd_clear()
{
   memset(displayBuf, 0, sizeof(displayBuf));
}

/*--------------------------------------------------------------------------------
 * lcd_img
 *--------------------------------------------------------------------------------*/
void lcd_img(uint8_t i_x, uint8_t i_y, uint8_t *imgdat, uint8_t idx, uint8_t mode)
{
   uint8_t  *q   = imgdat;
   uint8_t  w    = pgm_read_byte(q++);
   uint8_t  hb   = (pgm_read_byte(q++) + 7) / 8;
   uint8_t  sze1 = pgm_read_byte(q++);

   q += idx * sze1;
   uint8_t inv = (mode & LCD_INVERS) ? 1 : (mode & LCD_BLINK ? LCD_BLINK_TIMER : 0);
   
   for (uint8_t yb = 0; yb < hb; yb++)
   {
      uint8_t *p = &displayBuf[(i_y / 8 + yb) * LCD_DISPLAY_W + i_x];
      for (uint8_t x = 0; x < w; x++)
      {
         uint8_t b = pgm_read_byte(q++);
         *p++ = (inv ? ~b : b);
      }
   }
}


/*--------------------------------------------------------------------------------
 * lcd_putcAtt
 *--------------------------------------------------------------------------------*/
void lcd_putcAtt(uint8_t x, uint8_t y, char c, uint8_t mode, uint8_t flag)
{
   uint8_t *p = &displayBuf[y / 8 * LCD_DISPLAY_W + x];

   uint8_t *q  = &font_5x8_x20_x7f[(c - 0x20) * 5];
   uint8_t inv = (mode & LCD_INVERS) ? 1 : (mode & LCD_BLINK ? LCD_BLINK_TIMER : 0);

   if (mode & LCD_DBLSIZE)
   {
      /* each letter consists of ten top bytes followed by
       * five bottom by ten bottom bytes (20 bytes per
       * char) */
      q = &font_10x16_x20_x7f[(c - 0x20) * 10 + ((c - 0x20) / 16) * 160];
      for (char i = 5; i != 0; i--)
      {
         /*top byte*/
         uint8_t b1 = i > 0 ? pgm_read_byte(q) : 0;
         /*bottom byte*/
         uint8_t b3 = i > 0 ? pgm_read_byte(160 + q) : 0;
         /*top byte*/
         uint8_t b2 = i > 0 ? pgm_read_byte(++q) : 0;
         /*bottom byte*/
         uint8_t b4 = i > 0 ? pgm_read_byte(160 + q) : 0;
         q++;
         if (inv)
         {
            b1 = ~b1;
            b2 = ~b2;
            b3 = ~b3;
            b4 = ~b4;
         }

         if (&p[LCD_DISPLAY_W + 1] < LCD_DISPLAY_END)
         {
            p[0]                 = b1;
            p[1]                 = b2;
            p[LCD_DISPLAY_W]     = b3;
            p[LCD_DISPLAY_W + 1] = b4;
            p += 2;
         }
      }
   }
   else
   {
      uint8_t condense = 0;

      if (flag & LCD_CONDENSE_LETTER)
      {
         *p++     = inv ? ~0 : 0;
         condense = 1;
      }

      for (char i = 5; i != 0; i--)
      {
         uint8_t b = pgm_read_byte(q++);
         if (condense && (i == 4))
         {
            /*condense the letter by skipping column 4 */
            continue;
         }
         if (p < LCD_DISPLAY_END)
         {
            *p++ = inv ? ~b : b;
         }
      }
      if (p < LCD_DISPLAY_END)
      {
         *p++ = inv ? ~0 : 0;
      }
   }
}

/*--------------------------------------------------------------------------------
 * lcd_putc
 *--------------------------------------------------------------------------------*/
void lcd_putc(uint8_t x, uint8_t y, char c)
{
   lcd_putcAtt(x, y, c, 0, 0);
}


/*--------------------------------------------------------------------------------
 * lcd_putsnAtt
 *--------------------------------------------------------------------------------*/
void lcd_putsnAtt(uint8_t x, uint8_t y, char *s, uint8_t len, uint8_t mode)
{
   while (len != 0)
   {
      char c = (mode & LCD_BSS_NO_INV) ? *s++ : pgm_read_byte(s++);
      lcd_putcAtt(x, y, c, mode, 0);
      x += LCD_FONT_WIDTH;
      len--;
   }
}

/*--------------------------------------------------------------------------------
 * lcd_putsn_P
 *--------------------------------------------------------------------------------*/
void lcd_putsn_P(uint8_t x, uint8_t y, char *s, uint8_t len)
{
   lcd_putsnAtt(x, y, s, len, 0);
}

/*--------------------------------------------------------------------------------
 * lcd_putsAtt
 *--------------------------------------------------------------------------------*/
uint8_t lcd_putsAtt(uint8_t x, uint8_t y, char *s, uint8_t mode)
{
   while (1)
   {
      char c = (mode & LCD_BSS_NO_INV) ? *s++ : pgm_read_byte(s++);
      if (!c)
      {
         break;
      }
      lcd_putcAtt(x, y, c, mode, 0);
      x += LCD_FONT_WIDTH;
      if (mode & LCD_DBLSIZE)
      {
         x += LCD_FONT_WIDTH;
      }
   }
   return(x);
}

/*--------------------------------------------------------------------------------
 * lcd_puts_P
 *--------------------------------------------------------------------------------*/
void lcd_puts_P(uint8_t x, uint8_t y, char *s)
{
   lcd_putsAtt(x, y, s, 0);
}


/*--------------------------------------------------------------------------------
 * lcd_outhex4
 *--------------------------------------------------------------------------------*/
void lcd_outhex4(uint8_t x, uint8_t y, uint16_t val)
{
   x += LCD_FONT_WIDTH_NUM * 4;
   for (int i = 0; i < 4; i++)
   {
      x -= LCD_FONT_WIDTH_NUM;
      char c = val & 0xf;
      c = c > 9 ? c + 'A' - 10 : c + '0';
      lcd_putcAtt(x, y, c, 0, c >= 'A' ? LCD_CONDENSE_LETTER : 0);
      val >>= 4;
   }
}


/*--------------------------------------------------------------------------------
 * lcd_outdez
 *--------------------------------------------------------------------------------*/
void lcd_outdez(uint8_t x, uint8_t y, int16_t val)
{
   lcd_outdezAtt(x, y, val, 0);
}


/*--------------------------------------------------------------------------------
 * lcd_outdezAtt
 *--------------------------------------------------------------------------------*/
void lcd_outdezAtt(uint8_t x, uint8_t y, int16_t val, uint8_t mode)
{
   lcd_outdezNAtt(x, y, val, mode, 5);
}

/*--------------------------------------------------------------------------------
 * lcd_outdezNAtt
 *--------------------------------------------------------------------------------*/
void lcd_outdezNAtt(uint8_t x, uint8_t y, int16_t val, uint8_t mode, uint8_t len)
{
   uint8_t fw = LCD_FONT_WIDTH_NUM;

   if (mode & LCD_DBLSIZE)
   {
      fw += fw;
   }

   uint8_t prec = LCD_PREC(mode);
   uint8_t neg  = val < 0;

   if (neg)
   {
      val = -val;
   }

   x -= LCD_FONT_WIDTH;

   for (uint8_t i = 0; i < len; i++)
   {
      if (prec && (prec == i))
      {
         x -= 1;
         lcd_putcAtt(x, y, (val % 10) + '0', mode, 0);
         lcd_plot(x + 5, y + 7); //komma
         lcd_plot(x + 6, y + 6); //komma
         prec = 0;
      }
      else
      {
         lcd_putcAtt(x, y, (val % 10) + '0', mode, 0);
      }

      val /= 10;

      if (!(mode & LCD_LEADING0) && !val && !prec)
      {
         break;
      }

      x -= fw;
   }

   if (neg)
   {
      lcd_putcAtt(x - fw, y, '-', mode, 0);
   }
   else if ((mode & LCD_SIGN))
   {
      lcd_putcAtt(x - fw, y, '+', mode, 0);
   }
}

/*--------------------------------------------------------------------------------
 * lcd_plot
 *--------------------------------------------------------------------------------*/
void lcd_plot(uint8_t x, uint8_t y)
{
   uint8_t *p = &displayBuf[y / 8 * LCD_DISPLAY_W + x];

   if (p < LCD_DISPLAY_END)
   {
      *p |= LCD_BITMASK(y % 8);
   }
}

/*--------------------------------------------------------------------------------
 * lcd_plot_xor
 *--------------------------------------------------------------------------------*/
void lcd_plot_xor(uint8_t x, uint8_t y)
{
   uint8_t *p = &displayBuf[y / 8 * LCD_DISPLAY_W + x];

   if (p < LCD_DISPLAY_END)
   {
      *p ^= LCD_BITMASK(y % 8);
   }
}


/*--------------------------------------------------------------------------------
 * lcd_hlineStip_xor
 *--------------------------------------------------------------------------------*/
void lcd_hlineStip_xor(uint8_t x, uint8_t y, int8_t w, uint8_t pat)
{
   if (w < 0)
   {
      x += w;
      w  = -w;
   }

   uint8_t *p  = &displayBuf[y / 8 * LCD_DISPLAY_W + x];
   uint8_t msk = LCD_BITMASK(y % 8);
   while (w)
   {
      if (pat & 1)
      {
         *p ^= msk;
         pat = (pat >> 1) | 0x80;
      }
      else
      {
         pat = pat >> 1;
      }
      w--;
      p++;
   }
}

/*--------------------------------------------------------------------------------
 * lcd_hlineStip
 *--------------------------------------------------------------------------------*/
void lcd_hlineStip(uint8_t x, uint8_t y, int8_t w, uint8_t pat)
{
   if (w < 0)
   {
      x += w;
      w  = -w;
   }

   uint8_t *p  = &displayBuf[y / 8 * LCD_DISPLAY_W + x];
   uint8_t msk = LCD_BITMASK(y % 8);
   while (w)
   {
      if (pat & 1)
      {
         *p |= msk;
         pat = (pat >> 1) | 0x80;
      }
      else
      {
         pat = pat >> 1;
      }
      w--;
      p++;
   }
}

/*--------------------------------------------------------------------------------
 * lcd_hline_xor
 *--------------------------------------------------------------------------------*/
void lcd_hline_xor(uint8_t x, uint8_t y, int8_t w)
{
   lcd_hlineStip_xor(x, y, w, 0xff);
}

/*--------------------------------------------------------------------------------
 * lcd_hline
 *--------------------------------------------------------------------------------*/
void lcd_hline(uint8_t x, uint8_t y, int8_t w)
{
   lcd_hlineStip(x, y, w, 0xff);
}


/*--------------------------------------------------------------------------------
 * lcd_vline_xor
 *--------------------------------------------------------------------------------*/
void lcd_vline_xor(uint8_t x, uint8_t y, int8_t h)
{
   uint8_t *p = &displayBuf[y / 8 * LCD_DISPLAY_W + x];
   uint8_t *q = &displayBuf[(y + h) / 8 * LCD_DISPLAY_W + x];

   *p ^= ~(LCD_BITMASK(y % 8) - 1);
   while (p < q)
   {
      p  += LCD_DISPLAY_W;
      *p ^= 0xff;
   }
   *p ^= ~(LCD_BITMASK((y + h) % 8) - 1);
}

/*--------------------------------------------------------------------------------
 * lcd_vline
 *--------------------------------------------------------------------------------*/
void lcd_vline(uint8_t x, uint8_t y, int8_t h)
{
/*   uint8_t *p = &displayBuf[y / 8 * LCD_DISPLAY_W + x];
   uint8_t *q = &displayBuf[(y + h) / 8 * LCD_DISPLAY_W + x];

   *p |= ~(LCD_BITMASK(y % 8) - 1);
   while (p < q)
   {
      p  += LCD_DISPLAY_W;
      *p |= 0xff;
   }
   *p |= ~(LCD_BITMASK((y + h) % 8) - 1);
   */
	uint8_t i;
	for (i=0; i<h; i++)
	{
		lcd_plot(x,y+i);
	}
}

/*--------------------------------------------------------------------------------
 * lcd_line
 *--------------------------------------------------------------------------------*/
void lcd_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
	int16_t dx, dy, stepx, stepy, fraction;

	dy = y2 - y1;
	dx = x2 - x1;

	if (dy<0) 
	{ 
		dy= -dy; 
		stepy= -1; 
	}
	else
	{
		stepy = 1;
	}
	
	if (dx<0)
	{ 
		dx= -dx; 
		stepx= -1; 
	}
	else 
	{
		stepx = 1;
	}

	dx <<= 1;
	dy <<= 1;
	lcd_plot(x1, y1);

	if (dx>dy)
	{
		fraction = dy - (dx >> 1);

		while (x1!=x2)
    	{
    		if (fraction>=0)
			{
				y1 += stepy; 
				fraction -= dx; 
			}
    		
			x1 += stepx;
    		fraction += dy;
    		lcd_plot(x1, y1);
    	}
  	}
	else
  	{
		fraction = dx - (dy >> 1);
  		while (y1!=y2)
    	{
    		if (fraction>=0)
			{
				x1 += stepx; 
				fraction -= dy;
			}
    
			y1 += stepy;
    		fraction += dx;
    		lcd_plot(x1, y1);
    	}
  	}
}


/*--------------------------------------------------------------------------------
 * lcdSendCtl
 *--------------------------------------------------------------------------------*/
void lcdSendCtl(uint8_t val)
{
   PORTC_LCD_CTRL &= ~(1 << OUT_C_LCD_CS1);
   PORTC_LCD_CTRL &= ~(1 << OUT_C_LCD_A0);
   PORTC_LCD_CTRL &= ~(1 << OUT_C_LCD_RnW);
   PORTA_LCD_DAT   = val;
   PORTC_LCD_CTRL |= (1 << OUT_C_LCD_E);
   PORTC_LCD_CTRL &= ~(1 << OUT_C_LCD_E);
   PORTC_LCD_CTRL |= (1 << OUT_C_LCD_A0);
   PORTC_LCD_CTRL |= (1 << OUT_C_LCD_CS1);
}

/*--------------------------------------------------------------------------------
 * lcdSendCtl
 *--------------------------------------------------------------------------------*/
void lcdSendDat(uint8_t val)
{
   PORTC_LCD_CTRL &= ~(1 << OUT_C_LCD_CS1);
   PORTC_LCD_CTRL |= (1 << OUT_C_LCD_A0);
   PORTC_LCD_CTRL &= ~(1 << OUT_C_LCD_RnW);
   PORTA_LCD_DAT   = val;
   PORTC_LCD_CTRL |= (1 << OUT_C_LCD_E);
   PORTC_LCD_CTRL &= ~(1 << OUT_C_LCD_E);
   PORTC_LCD_CTRL |= (1 << OUT_C_LCD_A0);
   PORTC_LCD_CTRL |= (1 << OUT_C_LCD_CS1);
}

/*--------------------------------------------------------------------------------
 * lcdSendCtl
 *--------------------------------------------------------------------------------*/
void lcd_init()
{

	DDRA = 0xff;  PORTA = 0x00;
	DDRC = 0x3e;  PORTC = 0xc1; //pullups nc

	PORTC_LCD_CTRL &= ~(1 << OUT_C_LCD_RES); //LCD_RES
	_delay_us(1);
	_delay_us(1);                             //    f520  call  0xf4ce  delay_1us() ; 0x0xf4ce
	PORTC_LCD_CTRL |= (1 << OUT_C_LCD_RES);  //  f524  sbi 0x15, 2 IOADR-PORTC_LCD_CTRL; 21           1
	_delay_us(1500);

	lcdSendCtl(0xe2);                        //Initialize the internal functions
	lcdSendCtl(0xae);                        //DON = 0: display OFF
	lcdSendCtl(0xa1);                        //ADC = 1: reverse direction(SEG132->SEG1)
	lcdSendCtl(0xA6);                        //REV = 0: non-reverse display
	lcdSendCtl(0xA4);                        //EON = 0: normal display. non-entire
	lcdSendCtl(0xA2);                        // Select LCD bias=0
	lcdSendCtl(0xC0);                        //SHL = 0: normal direction (COM1->COM64)
	lcdSendCtl(0x2F);                        //Control power circuit operation VC=VR=VF=1
	lcdSendCtl(0x25);                        //Select int resistance ratio R2 R1 R0 =5
	lcdSendCtl(0x81);                        //Set reference voltage Mode
	lcdSendCtl(0x22);                        // 24 SV5 SV4 SV3 SV2 SV1 SV0 = 0x18
	lcdSendCtl(0xAF);                        //DON = 1: display ON
  
}


void lcd_contrast(uint8_t val)
{
   lcdSendCtl(0x81);
   lcdSendCtl(val);
}


void lcd_refresh()
{
   uint8_t *p = displayBuf;

   for (uint8_t y = 0; y < 8; y++)
   {
      lcdSendCtl(0x04);
      lcdSendCtl(0x10);     //column addr 0
      lcdSendCtl(y | 0xB0); //page addr y
      for (uint8_t x = 0; x < 128; x++)
      {
         lcdSendDat(*p);
         p++;
      }
   }
}
