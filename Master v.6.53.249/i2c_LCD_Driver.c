//----------------------------------------------------------------------------------
//	File Name:		i2c_LCD_Driver.c
//	Device:			PIC18F4620 @ 10Mhz (PLLx4)
//	Autor:			G.L.
//	Date:			09/03/2016
//	Description:	I2C Routines for Driver Serial LCD (PCF8574)
//----------------------------------------------------------------------------------

#include "delay.h"
#include "i2c_LCD_Driver.h"
#include "soft_i2c.h"


void I2C_LCD_Cmd(char out_char) {

    char hi_n, lo_n;
    char rs = 0x00;

    hi_n = out_char & 0xF0;
    lo_n = (out_char << 4) & 0xF0;

    i2cStart();
    //I2C1_Is_Idle();
    i2cWriteByte(LCD_ADDR);
    //I2C1_Is_Idle();
    i2cWriteByte(hi_n | rs | 0x04 | 0x08);
    //I2C1_Is_Idle();
    DelayuSec(50);
    i2cWriteByte(hi_n | rs | 0x00 | 0x08);
    //I2C1_Is_Idle();
    DelayuSec(100);
    i2cWriteByte(lo_n | rs | 0x04 | 0x08);
    //I2C1_Is_Idle();
    DelayuSec(50);
    i2cWriteByte(lo_n | rs | 0x00 | 0x08);
    //I2C1_Is_Idle();
    i2cStop();

    if(out_char == _LCD_CLEAR)    DelaymSec(2);
}

void I2C_LCD_Chr(char row, char column, char out_char) {

    char hi_n, lo_n;
    char rs = 0x01;

    switch(row){

        case 1:
        I2C_LCD_Cmd(0x80 + (column - 1));
        break;
        case 2:
        I2C_LCD_Cmd(0xC0 + (column - 1));
        break;
        case 3:
        I2C_LCD_Cmd(0x94 + (column - 1));
        break;
        case 4:
        I2C_LCD_Cmd(0xD4 + (column - 1));
        break;
    };

    hi_n = out_char & 0xF0;
    lo_n = (out_char << 4) & 0xF0;

    i2cStart();
    //I2C1_Is_Idle();
    i2cWriteByte(LCD_ADDR);
    //I2C1_Is_Idle();
    i2cWriteByte(hi_n | rs | 0x04 | 0x08);
    //I2C1_Is_Idle();
    DelayuSec(50);
    i2cWriteByte(hi_n | rs | 0x00 | 0x08);
    //I2C1_Is_Idle();
    DelayuSec(100);
    i2cWriteByte(lo_n | rs | 0x04 | 0x08);
    //I2C1_Is_Idle();
    DelayuSec(50);
    i2cWriteByte(lo_n | rs | 0x00 | 0x08);
    //I2C1_Is_Idle();
    i2cStop();
}

void I2C_LCD_Chr_Cp(char out_char) {

    char hi_n, lo_n;
    char rs = 0x01;

    hi_n = out_char & 0xF0;
    lo_n = (out_char << 4) & 0xF0;

    i2cStart();
    //I2C1_Is_Idle();
    i2cWriteByte(LCD_ADDR);
    //I2C1_Is_Idle();
    i2cWriteByte(hi_n | rs | 0x04 | 0x08);
    //I2C1_Is_Idle();
    DelayuSec(50);
    i2cWriteByte(hi_n | rs | 0x00 | 0x08);
    //I2C1_Is_Idle();
    DelayuSec(100);
    i2cWriteByte(lo_n | rs | 0x04 | 0x08);
    //I2C1_Is_Idle();
    DelayuSec(50);
    i2cWriteByte(lo_n | rs | 0x00 | 0x08);
    //I2C1_Is_Idle();
    i2cStop();
}


void I2C_LCD_Init() {

    char rs = 0x00;

    i2cInit();
    
    i2cStart();
    //I2C1_Is_Idle();
    i2cWriteByte(LCD_ADDR);
    //I2C1_Is_Idle();
    DelaymSec(30); 

    i2cWriteByte(0x30 | rs | 0x04 | 0x08);
    //I2C1_Is_Idle();
    DelayuSec(50);
    i2cWriteByte(0x30 | rs | 0x00 | 0x08);
    //I2C1_Is_Idle();

    DelaymSec(10);

    i2cWriteByte(0x30 | rs | 0x04 | 0x08);
    //I2C1_Is_Idle();
    DelayuSec(50);
    i2cWriteByte(0x30 | rs | 0x00 | 0x08);
    //I2C1_Is_Idle();

    DelaymSec(10);

    i2cWriteByte(0x30 | rs | 0x04 | 0x08);
    //I2C1_Is_Idle();
    DelayuSec(50);
    i2cWriteByte(0x30 | rs | 0x00 | 0x08);
    //I2C1_Is_Idle();

    DelaymSec(10);

    i2cWriteByte(0x20 | rs | 0x04 | 0x08);
    //I2C1_Is_Idle();
    DelayuSec(50);
    i2cWriteByte(0x20 | rs | 0x00 | 0x08);
    //I2C1_Is_Idle();
    i2cStop();

    DelaymSec(10);

    I2C_LCD_Cmd(0x28);
    I2C_LCD_Cmd(0x06);
}

void I2C_LCD_Out(char row, char column, char *text) {
    while(*text)
         I2C_LCD_Chr(row, column++, *text++);
}

void I2C_LCD_Out_Cp(char *text) {
    while(*text)
         I2C_LCD_Chr_Cp(*text++);
}

