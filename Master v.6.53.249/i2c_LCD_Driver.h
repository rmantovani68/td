//----------------------------------------------------------------------------------
//	File Name:		i2c_LCD_Driver.h
//	Device:			PIC18F4620 @ 10Mhz (PLLx4)
//	Autor:			G.L.
//	Date:			09/03/2016
//	Description:	I2C for Driver Serial LCD (PCF8574) include file
//----------------------------------------------------------------------------------

#ifndef I2C_LCD_H
    #define I2C_LCD_H

#define _LCD_FIRST_ROW          0x80     //Move cursor to the 1st row
#define _LCD_SECOND_ROW         0xC0     //Move cursor to the 2nd row
#define _LCD_THIRD_ROW          0x94     //Move cursor to the 3rd row
#define _LCD_FOURTH_ROW         0xD4     //Move cursor to the 4th row
#define _LCD_CLEAR              0x01     //Clear display
#define _LCD_RETURN_HOME        0x02     //Return cursor to home position, returns a shifted display to
                                         //its original position. Display data RAM is unaffected.
#define _LCD_CURSOR_OFF         0x0C     //Turn off cursor
#define _LCD_UNDERLINE_ON       0x0E     //Underline cursor on
#define _LCD_BLINK_CURSOR_ON    0x0F     //Blink cursor on
#define _LCD_MOVE_CURSOR_LEFT   0x10     //Move cursor left without changing display data RAM
#define _LCD_MOVE_CURSOR_RIGHT  0x14     //Move cursor right without changing display data RAM
#define _LCD_TURN_ON            0x0C     //Turn Lcd display on
#define _LCD_TURN_OFF           0x08     //Turn Lcd display off
#define _LCD_SHIFT_LEFT         0x18     //Shift display left without changing display data RAM
#define _LCD_SHIFT_RIGHT        0x1E     //Shift display right without changing display data RAM

// LCD Definitions
// Port PCF8574:  7  6  5  4  3  2  1  0
//                D7 D6 D5 D4 *  EN RW RS
//                                             			 bit    7 6 5 4 3  2  1  0  
#define LCD_ADDR 0x4E           		// Vedi datasheet ->    0 1 0 0 A2 A1 A0 R/W    Addr 0x27 -> 0x4E


// Prototipe functions
void I2C_LCD_Cmd(char out_char); 
void I2C_LCD_Chr(char row, char column, char out_char);
void I2C_LCD_Chr_Cp(char out_char);
void I2C_LCD_Init();
void I2C_LCD_Out(char row, char column, char *text);
void I2C_LCD_Out_Cp(char *text);

#endif
