/*
-----------------------------------------------------------------------------
	Nome: New_I2C.h
	Autore: TDM Team
	Revisione\Data:	Rev.2 - 09/05/2010
	Descrizione: Header delle funzioni per la comunicazione su bus I2C
	Dipendenze: I2C.c - Corpo delle funzioni qui descritte
-----------------------------------------------------------------------------
*/
#include "usart.h"
//-----------------------------------------------------------------------------
//	Velocità di comunicazione
//-----------------------------------------------------------------------------
#define FOSC_I2C FOSC/1000 //16000//32000	// in Khz, frequenza di lavoro del Micro
#define	I2C_CK	50//100		// in Khz, Frequenza di lavoro del Bus I2C
#define DIVIDER_I2C (((FOSC_I2C/I2C_CK)-(FOSC_I2C/10000))-1)	//	Calcolo del divider da impostare
//#define DIVIDER_I2C	78	// 100Khz I2C Bus @ 8Mhz Clk

//---------------------------------------------------------------------------
//	Costanti per le funzioni
//---------------------------------------------------------------------------
#define	MACK	0
#define	NMACK	1

//---------------------------------------------------------------------------
//	Funzioni e procedure disponibili
//---------------------------------------------------------------------------
void I2C_Init(void);
unsigned char I2C_Is_Idle(void);
unsigned char I2C_Start(void);
unsigned char I2C_Repeated_Start(void);
void I2C_Stop(void);
void Reset_I2C_bus(void);
unsigned char I2C_Write(unsigned char Data);
unsigned char I2C_Read(unsigned char vAck);
unsigned char SetRegI2C(unsigned char Address, unsigned int Reg, unsigned char Data);
unsigned char ReadRegI2C(unsigned char Address, unsigned int Reg);
