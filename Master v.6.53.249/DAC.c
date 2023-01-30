//----------------------------------------------------------------------------------
//	Progect name:	DAC.c
//	Device:			PIC18F4620 @ 10Mhz (PLLx4)
//	Autor:			Termodinamica Team
//	Date:			22/05/2011
//	Description:	Corpo delle funzioni per la gestione del modulo DAC
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//	Include
//----------------------------------------------------------------------------------
#include "HWSelection.h"
#if (DA210==1)
	#include <p24FJ256DA210.h>
#else
	#include <p24FJ256GB210.h>
#endif

#include <stdio.h>
#include "DefinePeriferiche.h"
#include "DAC.h"

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
void DA_Write(unsigned char Channel, unsigned char Value)
{	
	unsigned char i = 0;
	unsigned int App = 0;
	unsigned int ImmOut = 0;

	ImmOut = ((Channel << 8) & 0xFF00) | (Value & 0x00FF);
	
	oDAC_LD = 0;
	for(i=12; i>0; i--)
	{
		oDAC_SCK = 0;
		App = ImmOut >>(i-1);
		oDAC_SDO = ((App & 1) != 0);
		oDAC_SCK = 1;
	}

	oDAC_SCK = 0;
	oDAC_LD = 1;	// Strobe Load
	oDAC_SDO = 0;	// solo per perdere tempo
	oDAC_LD = 0;

/*
	ImmOut = ((Chanel) << 8)|Value;
	
	oDAC_LD = 0;
	for(i=11; i<12; i--)
	{
		oDAC_SCK = 0;
		App = ImmOut >>i;
		oDAC_SDO = ((App & 1) != 0);
		oDAC_SCK = 1;
	}
	oDAC_LD = 1;
*/
}

void Reset_All_DA(void)
{
	DA_Write(DA_CH_01, 0);
	DA_Write(DA_CH_02, 0);
	DA_Write(DA_CH_03, 0);
	DA_Write(DA_CH_04, 0);
}