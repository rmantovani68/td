//----------------------------------------------------------------------------------
//	Progect name:	DAC.h
//	Device:			PIC18F4620 @ 10Mhz (PLLx4)
//	Autor:			Termodinamica Team
//	Date:			22/05/2011
//	Description:	Header per il modulo di gestione del DAC
//----------------------------------------------------------------------------------
#ifndef _DAC_H_
	#define _DAC_H_

	//---------------------------------------
	// Trama dei dati
	//---------------------------------------
	/*
		+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
		|  D11  |  D10  |  D09  |  D08  |  D07  |  D06  |  D05  |  D04  |  D03  |  D02  |  D01  |  D00  |
		+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
		|	  Indirizzo del canale		|			  Valore digitale da assegnare al canale			|
		+-------------------------------+---------------------------------------------------------------+

		LD	...--\_____________________________________________________________/--...
		CLK	...______/-\__/-\__/-\__/-\__/-\__/-\__/-\__/-\__/-\__/-\__/-\__/-\___...
		DI	..._____D11__D10__D09__D08__D07__D06__D05__D04__D03__D02__D01__D00____...
	*/
	//---------------------------------------
	// Define
	//---------------------------------------
	#define	DA_CH_01	0x8
	#define	DA_CH_02	0x4
	#define	DA_CH_03	0xA
	#define	DA_CH_04	0x6

	//---------------------------------------
	// Variabili
	//---------------------------------------
	#define VariabiliGlobaliDAC()       \
        unsigned char DAC_1 = 0;        \
        unsigned char DAC_2 = 0;        \
        unsigned char DAC_3 = 0;        \
        unsigned char DAC_4 = 0;


	#define IncludeVariabiliGlobaliDAC()    \
        extern unsigned char DAC_1;                \
        extern unsigned char DAC_2;                \
        extern unsigned char DAC_3;                \
        extern unsigned char DAC_4;

    IncludeVariabiliGlobaliDAC();


	//---------------------------------------
	// Prototipi
	//---------------------------------------
	void DA_Write(unsigned char Channel, unsigned char Value);
	void Reset_All_DA(void);
#endif
