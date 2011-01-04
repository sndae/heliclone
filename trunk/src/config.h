//
// Title        : MMC2IEC - Configuration
// Author       : Lars Pontoppidan
// Date         : Jan. 2007
// Version      : 0.7
// Target MCU   : AtMega32(L) at 8 MHz
//
//
// DISCLAIMER:
// The author is in no way responsible for any problems or damage caused by
// using this code. Use at your own risk.
//
// LICENSE:
// This code is distributed under the GNU Public License
// which can be found at http://www.gnu.org/licenses/gpl.txt
//

#ifndef CONFIG_H
#define CONFIG_H

// Enable UART debugging here by uncommenting UART_DEBUG
// #define UART_DEBUG

/*
 * // SDCARD_DETECT return non-zero when card is inserted
 * // If no card detect signal is available, comment the defines
 * #define SDCARD_DETECT         (!(PIND&(1<<2)))
 * #define SDCARD_DETECT_SETUP() DDRD&=~(1<<2);PORTD|=(1<<2)
 */

/*
 * // SDCARD_WP must return non-zero when card is write protected
 * // If no card detect signal is available, comment the defines
 * #define SDCARD_WP         (PIND&(1<<6))
 * #define SDCARD_WP_SETUP() DDRD&=~(1<<6);PORTD|=(1<<6)
 */

/*
 * // If DEV9_JUMPER is non-zero, IEC device number is 9, otherwise 8
 * // If no DEV9 jumper is available, comment the defines
 * #define DEV9_JUMPER         (!(PINA&(1<<2)))
 * #define DEV9_JUMPER_SETUP() DDRA&=~(1<<2);PORTA|=(1<<2)
 */

/*
 * // If DEV10_JUMPER is non-zero, IEC device number is 10, otherwise 8
 * // If no DEV10 jumper is available, comment the defines
 * #define DEV10_JUMPER         (!(PINA&(1<<3)))
 * #define DEV10_JUMPER_SETUP() DDRA&=~(1<<3);PORTA|=(1<<3)
 */

/*
 * // BUSY led, recommended color: green
 * #define BUSY_LED_ON()     PORTA &= ~(1<<0)
 * #define BUSY_LED_OFF()    PORTA |= (1<<0)
 * #define BUSY_LED_SETDDR() DDRA |= (1<<0)
 *
 * // DIRTY led, recommended color: red
 * #define DIRTY_LED_ON()     PORTA &= ~(1<<1)
 * #define DIRTY_LED_OFF()    PORTA |= (1<<1)
 * #define DIRTY_LED_SETDDR() DDRA |= (1<<1)
 */


// IEC signals
#define IEC_PIN          PINC
#define IEC_DDR          DDRC
#define IEC_PORT         PORTC

#define IEC_BIT_ATN      (1 << 0)
#define IEC_BIT_DATA     (1 << 1)
#define IEC_BIT_CLOCK    (1 << 2)



#endif
