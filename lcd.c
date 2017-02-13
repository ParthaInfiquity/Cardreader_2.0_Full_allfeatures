/*
 * lcd.c
 *
 *  Created on: Dec 18, 2016
 *      Author: Brijender
 */
#include "i2c_if.h"
#include "lcd.h"

#define RS 0  // Enable bit
#define RW 1  // Read/Write bit
#define EN 2  // Register select bit
#define BACKLIGHT 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7

#define LCD_NOBACKLIGHT 0x00
#define LCD_BACKLIGHT   0xFF
#define I2CBACKPACK_DEV_ADDR    0x3F
//#define I2CBACKPACK_DEV_ADDR    0x27
#define I2CBACKPCK_WRITE_LEN    1
#define I2CBACKPACK_STOP_BIT    1
#define LCD_WRITE_SUCCESS       1
#define LCD_WRITE_FAILED        0
#define RS_HIGH                 0x01
#define RW_HIGH                 0x02
#define EN_HIGH                 0x04

#define RS_HIGH                 0x01
#define SET_WRITE               0x00
#define ENABLE_HIGH             0x01

#define DISABLE_EN_RS           0x01
#define SUCCESS                 0   // added by brij

//global variales
   unsigned char _Addr;             // I2C Address of the IO expander
   unsigned char _backlightPinMask; // Backlight IO pin mask
   unsigned char _backlightStsMask; // Backlight status mask

   unsigned char _En;               // LCD expander word for enable pin
   unsigned char _Rw;               // LCD expander word for R/W pin
   unsigned char _Rs;               // LCD expander word for Register Select pin
   unsigned char _data_pins[4];     // LCD data lines

   t_backlighPol _polarity;   // Backlight polarity

void lcd_init(void)
{
	I2C_IF_Open(I2C_MASTER_MODE_STD);
   _Addr = (unsigned char)I2CBACKPACK_DEV_ADDR ;

   writeI2CLCD((unsigned char)0);

   _En = (unsigned char)EN;
   _Rw = (unsigned char)RW;
   _Rs = (unsigned char)RS;


   _data_pins[0] =  (unsigned char)D4;
   _data_pins[1] =	(unsigned char)D5;
   _data_pins[2] =  (unsigned char)D6;
   _data_pins[3] =  (unsigned char)D7;
    SetBacklightPin((unsigned char)BACKLIGHT);


   send_lcd((unsigned char)RESET_CMD  , COMMAND);  // command to reset LCD
   send_lcd((unsigned char)RESET_CMD , COMMAND);
   send_lcd((unsigned char)RESET_CMD , COMMAND);
   send_lcd((unsigned char)LCD_RETURNHOME , COMMAND); //return home
   __delay_cycles(160000);        // 2ms delay
   send_lcd((unsigned char)LCD_TWO_LINE_PIXEL_4BIT  , COMMAND);//4 bit 5*7 2 line lcd command

   send_lcd((unsigned char)LCD_CURSORON_DISPLAYON , COMMAND); // display on & cursor visible
   //send_lcd((unsigned char)0x0C , COMMAND);  //display on
   send_lcd((unsigned char)LCD_CLEARDISPLAY,COMMAND);  //clear display
   send_lcd((unsigned char)LCD_SETDDRAMADDR , COMMAND);  // Selecting DGRAM



   /*short int status;
   status=writeI2CLCD((unsigned char)BACKLIGHT_ON);
   if(status)
   writeI2CLCD((unsigned char)BACKLIGHT_OFF);
   */
}

//void send_lcd(unsigned char value , unsigned char mode)
void send_lcd(unsigned char value , unsigned char mode)
{
   	if(mode == (unsigned char)COMMAND)
	{
		//command( (value & 0xF0),COMMAND);
   	              command( (value >> 4)&0x0F,   COMMAND );
   			      command( (value & 0x0F), COMMAND );

	}
		else
     	{
	      command( (value >> 4)&0x0F,   DATA );
	      command( (value & 0x0F), DATA );
	   }
}

//void command(unsigned char value , unsigned char mode)
void command(unsigned char value , unsigned char mode)
{
   unsigned char pinMapValue =(unsigned char)0 ;


   /*for ( i = 0; i < 4; i++ )
      {
         if ( ( value & 0x1 ) == 1 )
         {
            pinMapValue |= _data_pins[i];
         }
         value = ( value >> 1 );
      }*/

    pinMapValue = value<<4;
   if ( mode == DATA )
      {
         mode = 0x01;
      }
      pinMapValue |= mode | _backlightStsMask;
      pulse_enable
	  (pinMapValue);
}

short int writeI2CLCD(unsigned char writeVal)
{
	       unsigned char ucStopBit, ucLen;
		   unsigned char aucDataBuf[1];

		    int iRetVal;

		    aucDataBuf[0] = writeVal;
		    ucLen = (unsigned char)I2CBACKPCK_WRITE_LEN ;
		    ucStopBit = (unsigned char)I2CBACKPACK_STOP_BIT;
		    iRetVal = I2C_IF_Write(_Addr, aucDataBuf, ucLen, ucStopBit);
		    if(iRetVal == SUCCESS)
		    {
		      return (short)LCD_WRITE_SUCCESS;
		    }
		    else
		     return  (short)LCD_WRITE_FAILED;
}


void SetBacklight(unsigned char value)
{
	if ( _backlightPinMask != 0x0 )
	   {
	      // Check for polarity to configure mask accordingly
	      // ----------------------------------------------------------
	      if  (((_polarity == POSITIVE) && (value > 0)) ||
	           ((_polarity == NEGATIVE ) && ( value == 0 )))
	      {
	         _backlightStsMask = _backlightPinMask & LCD_BACKLIGHT;
	      }
	      else
	      {
	         _backlightStsMask = _backlightPinMask & LCD_NOBACKLIGHT;
	      }
	      writeI2CLCD( _backlightStsMask );
	   }

}

void SetBacklightPin(unsigned char value )
{

	  _backlightPinMask = (unsigned char)(1<<BACKLIGHT);
      _polarity = POSITIVE;
	  SetBacklight(BACKLIGHT_ON);
}

void pulse_enable(unsigned char data)
{
	writeI2CLCD(data | EN_HIGH);   // set enable bit
    __delay_cycles(160000);        // 2ms delay
	writeI2CLCD(data & ~EN_HIGH);  // reset enable bit
}

void print(unsigned char *ptr)
{
  while(*ptr)
  {
	  send_lcd(*ptr , DATA);
      ++ptr;
  }
  }

void LCDSelectLine(unsigned char y)
{
	if(y == (unsigned char)0)
		   send_lcd((unsigned char)LCD_LINE_ONE,COMMAND);  //move cursor to first line
	   else if(y== (unsigned char)1)

	send_lcd((unsigned char)LCD_LINE_TWO,COMMAND);	  //move cursor to second line
}


void LCDClear()
{
	send_lcd((unsigned char)LCD_CLEARDISPLAY , COMMAND);
	__delay_cycles(160000);        // 2ms delay
}

void increment_cursor(unsigned char count)
{
        count = count| 0x80;
    	send_lcd((unsigned char)count,COMMAND);

}

void number(int n)
{
        unsigned char arr[16];
        int j=0;
        while(n>0)
        {
                arr[j++]=n%10;
                n=n/10;
        }
        arr[j--] = 0;
        for(;j>=0;j--)
        {
        	arr[j] += 48;
        }

        print(arr);
}
void print_num(int n)
{
	char arr[10];
	int j=0;
	         while(n>0)
	         {
	                 arr[j++]=n%10;
	                 n=n/10;
	         }
	         for(--j;j>=0;j--)
	        	 send_lcd((arr[j]+48) , DATA);
}
void PrintFloat(double n)
{
	int num = (int)n ;
	float diffValue;
	print_num(num);
	send_lcd('.' , DATA);
	 diffValue = n - (float)num;
	 print_num(diffValue*100);
}

void moveCursorLeft(int number)
{
	int i=0;
	while(i<number)
		send_lcd(LCD_CURSORSHIFT | LCD_CURSORMOVE | LCD_MOVELEFT,COMMAND);
}
