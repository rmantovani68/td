//----------------------------------------------------------------------------------
//	File Name:		soft_i2c.h
//	Device:			PIC18F4620 @ 10Mhz (PLLx4)
//	Autor:			G.L.
//	Date:			09/03/2016
//	Description:	Software I2C include
//----------------------------------------------------------------------------------
#ifndef SOFT_I2C_H
    #define SOFT_I2C_H

#include "HWSelection.h"
#if (DA210==1)
	#include <p24FJ256DA210.h>
#else
	#include <p24FJ256GB210.h>
#endif

#define SDA_IPIN PORTBbits.RB7 // PGD per collegamento PICKIT3 o ICD
#define SCL_IPIN PORTBbits.RB6 // PGC per collegamento PICKIT3 o ICD
#define SDA_OPIN LATBbits.LATB7// PGD per collegamento PICKIT3 o ICD
#define SCL_OPIN LATBbits.LATB6 // PGC per collegamento PICKIT3 o ICD
#define SDA_DIR TRISBbits.TRISB7  // PGD per collegamento PICKIT3 o ICD
#define SCL_DIR TRISBbits.TRISB6  // PGC per collegamento PICKIT3 o ICD

void i2cInit(void);
unsigned char i2cReadByte(void);
void i2cWriteByte(unsigned char outByte);
void i2cNack(void);
void i2cAck(void);
void i2cStart(void);
void i2cStop(void);
void i2cHighSda(void);
void i2cLowSda(void);
void i2cHighScl(void);
void i2cLowScl(void);

#endif