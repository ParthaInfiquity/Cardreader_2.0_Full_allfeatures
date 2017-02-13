/*
 * lcd.h
 *
 *  Created on: Dec 18, 2016
 *      Author: Brijender
 */



#ifndef LCD_H_
#define LCD_H_
// LCD COMMANDS

#define LCD_CLEARDISPLAY        0x01
#define LCD_RETURNHOME          0x02
#define LCD_ENTRYMODESET        0x04
#define LCD_DISPLAYCONTROL      0x08
#define LCD_CURSORSHIFT         0x10
#define LCD_FUNCTIONSET         0x20
#define LCD_SETCGRAMADDR        0x40
#define LCD_SETDDRAMADDR        0x80
#define LCD_LINE_ONE            0x80
#define LCD_LINE_TWO            0xC0
#define CURSOR_LEFT             0x10
#define LCD_TWO_LINE_PIXEL_4BIT   0x28
// flags for display entry mode
// ---------------------------------------------------------------------------
#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

#define LCD_INC_CURSOR          0x06
#define LCD_DEC_CURSOR          0x04
// flags for display on/off and cursor control
// ---------------------------------------------------------------------------
#define LCD_DISPLAYON           0x04
#define LCD_DISPLAYOFF          0x00
#define LCD_CURSORON            0x02
#define LCD_CURSOROFF           0x00
#define LCD_BLINKON             0x01
#define LCD_BLINKOFF            0x00

#define LCD_CURSORON_DISPLAYON  0x0E
// flags for display/cursor shift
// ---------------------------------------------------------------------------
#define LCD_DISPLAYMOVE         0x08
#define LCD_CURSORMOVE          0x00
#define LCD_MOVERIGHT           0x04
#define LCD_MOVELEFT            0x00

// flags for function set
// ---------------------------------------------------------------------------
#define LCD_8BITMODE            0x10
#define LCD_4BITMODE            0x00
#define LCD_2LINE               0x08
#define LCD_1LINE               0x00
#define LCD_5x10DOTS            0x04
#define LCD_5x8DOTS             0x00

#define RESET_CMD               0x32
// Define COMMAND and DATA LCD Rs (used by send method).
// ---------------------------------------------------------------------------
#define COMMAND                 0
#define DATA                    1
#define FOUR_BITS               2


/*!
 @defined
 @abstract   Defines the duration of the home and clear commands
 @discussion This constant defines the time it takes for the home and clear
 commands in the LCD - Time in microseconds.
 */
#define HOME_CLEAR_EXEC      2000

/*!
    @defined
    @abstract   Backlight off constant declaration
    @discussion Used in combination with the setBacklight to swith off the
 LCD backlight. @set setBacklight
*/
#define BACKLIGHT_OFF           0

/*!
 @defined
 @abstract   Backlight on constant declaration
 @discussion Used in combination with the setBacklight to swith on the
 LCD backlight. @set setBacklight
 */
#define BACKLIGHT_ON          255


/*!
 @typedef
 @abstract   Define backlight control polarity
 @discussion Backlight control polarity. @see setBacklightPin.
 */
typedef enum { POSITIVE, NEGATIVE } t_backlighPol;

void lcd_init(void);
short int writeI2CLCD(unsigned char);

//void send(unsigned char);
void send_lcd(unsigned char , unsigned char);
void command(unsigned char  , unsigned char);

void SetBacklightPin(unsigned char);
void SetBacklight(unsigned char);
void pulse_enable(unsigned char);
void print(unsigned char *);

void LCDSelectLine(unsigned char);
void increment_cursor(unsigned char);
void LCDClear();
void number(int n);
void PrintFloat(double);
void print_num(int );
void moveCursorLeft(int);
#endif /* LCD_H_ */




