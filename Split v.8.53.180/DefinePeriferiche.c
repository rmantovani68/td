//----------------------------------------------------------------------------------
//	Progect name:	ProtocolloComunicazione.c
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			22/05/2011
//	Description:	Corpo delle funzioni che gestiscono la comunicazione con il sistema
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//	Include
//----------------------------------------------------------------------------------
#include "HWSelection.h"
#if (GB210==1) 
	#include <p24FJ256GB210.h>
#elif (DA210==1)
	#include <p24FJ256DA210.h>
#else
	#include <p24FJ256GB110.h>
#endif
#include "DefinePeriferiche.h"

#define	NULL	0
/*
VariabiliGlobaliADC();
VariabiliGlobaliDriverComunicazione();
VariabiliGlobaliDriverComunicazioneSec();
VariabiliGlobaliDriverModBusSec();
VariabiliGlobaliProtocolloComunicazione();
VariabiliGlobaliProtocolloComunicazioneSec();
VariabiliGlobaliProtocolloModBusSec();
VariabiliGlobaliEEPROM();
VariabiliGlobaliPWM();
VariabiliGlobaliValvolaExp();
#ifdef UART3_ON
GlobalVarServiceComunication();
#endif
*/

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
void Remap_RP(void)
{
	// Unlook sequence
	OSCCON &= 0xFF00;
	OSCCON |= 0x46;
	OSCCON |= 0x57;
	OSCCONbits.IOLOCK = 0;

	//-------------------------------------------------------------------
	// Configurazione Input
	//-------------------------------------------------------------------
/*	---- INPUT ----
	Ogni input ha una periferica, una volta abilitata, con i 5 bit del registro, si imposta il numero dell'RPIN(X)
	Ex:
	RPINR18bits.U1RX = 4;		// il segnale di input U1Rx (reciver UART1), viene assegnato all'ingresso RPIN4
*/
	RPINR0bits.INT1R = 0x1F;		// Ext. interrupt 1
	RPINR1bits.INT2R = 0x1F;		// Ext. interrupt 2
	RPINR1bits.INT3R = 0x1F;		// Ext. interrupt 3
	RPINR2bits.INT4R = 0x1F;		// Ext. interrupt 4
#if (GB210==1 || DA210==1)
#else	
	RPINR2bits.T1CKR = 0x1F; 
#endif	
	RPINR3bits.T2CKR = 0x1F;		// Timer 2 ext CK
	RPINR3bits.T3CKR = 41;			// Timer 3 ext CK			==> on RPI41
	RPINR4bits.T4CKR = 0x1F;		// Timer 4 ext CK
	RPINR4bits.T5CKR = 0x1F;		// Timer 5 ext CK
	RPINR7bits.IC1R = 0x1F;			// Input capture 1
	RPINR7bits.IC2R = 0x1F;			// Input capture 2
	RPINR8bits.IC3R = 0x1F;			// Input capture 3
	RPINR8bits.IC4R = 0x1F;			// Input capture 4
	RPINR9bits.IC5R = 0x1F;			// Input capture 5
	RPINR9bits.IC6R = 0x1F;			// Input capture 6
	RPINR10bits.IC7R = 0x1F;		// Input capture 7
	RPINR10bits.IC8R = 0x1F;		// Input capture 8
	RPINR11bits.OCFAR = 0x1F;		// Output compare Fault A
	RPINR11bits.OCFBR = 0x1F;		// Output compare Fault B
	RPINR15bits.IC9R = 0x1F;		// Input capture 9
#ifdef UART3_ON    
	RPINR17bits.U3RXR = 27;         // UART 3 Recive            -> RP27 (pin.14)
#else
	RPINR17bits.U3RXR = 0x1F;		// UART 3 Recive           
#endif
	RPINR18bits.U1RXR = 5;			// UART 1 Recive			==> on RP5
	RPINR18bits.U1CTSR = 0x1F;		// UART 1 Clear to send
	RPINR19bits.U2RXR = 15;			// UART 2 Recive			==> ON RP15
	RPINR19bits.U2CTSR = 0x1F;		// UART 2 Clear to send
	RPINR20bits.SDI1R = 0x1F;		// SPI 1 Data Input
	RPINR20bits.SCK1R = 0x1F;		// SPI 1 Clock Input
	RPINR21bits.SS1R = 0x1F;		// SPI 1 Slave Select Input
	RPINR21bits.U3CTSR = 0x1F;		// UART 3 Clear to send
	RPINR22bits.SDI2R = 0x1F;		// SPI 2 Data Input
	RPINR22bits.SCK2R = 0x1F;		// SPI 2 Clock Input
	RPINR23bits.SS2R = 0x1F;		// SPI 2 Slave Select Input
	RPINR27bits.U4RXR = 0x1F;		// UART 4 Recive
	RPINR27bits.U4CTSR = 0x1F;		// UART 4 Clear to send
	RPINR28bits.SDI3R = 0x1F;		// SPI 3 Data Input
	RPINR28bits.SCK3R = 0x1F;		// SPI 3 Clock Input
	RPINR29bits.SS3R = 0x1F;		// SPI 3 Slave Select Input

	//-------------------------------------------------------------------
	// Configurazione Output
	//-------------------------------------------------------------------
/*	---- OUTPUT ----
	Ad ogni pin RPORn, può essere assegnato l'id di una uscita di una qualsiasi periferica
	NULL = 0;		// Null
	C1OUT = 1;		// Comparator 1 out
	C2OUT = 2;		// Comparator 2 out
	U1TX = 3;		// UART1 Tx
	U1RTS = 4;		// UART1 RTS
	U2TX = 5;		// UART2 Tx
	U2RTS = 6;		// UART2 RTS
	SDO1 = 7;		// SPI1 Data out
	SCK1OUT = 8;	// SPI1 Clock out
	SS1OUT = 9;		// SPI1 Slave select out
	SDO2 = 10;		// SPI2 Data out
	SCK2OUT = 11;	// SPI2 Clock out
	SS2OUT = 12;	// SPI2 Slave select out
	OC1 = 18;		// Output compare 1
	OC2 = 19;		// Output compare 2
	OC3 = 20;		// Output compare 3
	OC4 = 21;		// Output compare 4
	OC5 = 22;		// Output compare 5
	Ex:
	RP1OR1bits.RP20R = OC1;	// setto sul pin RP20 l'uscita 18, Output compare 1
*/
	RPOR0bits.RP0R = NULL;		// RP0
	RPOR0bits.RP1R = NULL;		// RP1
	RPOR1bits.RP2R = NULL;		// RP2
	RPOR1bits.RP3R = NULL;		// RP3
	RPOR2bits.RP4R = NULL;		// RP4
	RPOR2bits.RP5R = NULL;		// RP5
	RPOR3bits.RP6R = NULL;		// RP6
	RPOR3bits.RP7R = NULL;		// RP7
	RPOR4bits.RP8R = NULL;		// RP8
	RPOR4bits.RP9R = NULL;		// RP9
	RPOR5bits.RP10R = U1TX;		// RP10	=> U1Tx 485
	RPOR5bits.RP11R = NULL;		// RP11
	RPOR6bits.RP12R = NULL;		// RP12
	RPOR6bits.RP13R = NULL;		// RP13
	RPOR7bits.RP14R = NULL;		// RP14
	RPOR7bits.RP15R = NULL;		// RP15
	RPOR8bits.RP16R = NULL;		// RP16
	RPOR8bits.RP17R = NULL;		// RP17
	RPOR9bits.RP18R = NULL;		// RP18
#ifdef UART3_ON    
	RPOR9bits.RP19R = U3TX;		// RP19 -> U3TX (pin.12)
#else    
	RPOR9bits.RP19R = NULL;		// RP19 
#endif    
	RPOR10bits.RP20R = NULL;	// RP20
	RPOR10bits.RP21R = OC_1;		// RP21 => OC1 Fan Output compare PWM
	RPOR11bits.RP22R = NULL;	// RP22
	RPOR11bits.RP23R = NULL;	// RP23
	RPOR12bits.RP24R = NULL;	// RP24
	RPOR12bits.RP25R = NULL;	// RP25
	RPOR13bits.RP26R = NULL;	// RP26
	RPOR13bits.RP27R = NULL;	// RP27
	RPOR14bits.RP28R = NULL;	// RP28
	RPOR14bits.RP29R = NULL;	// RP29
	RPOR15bits.RP30R = U2TX;	// RP30	=> U2Tx
	RPOR15bits.RP31R = NULL;	// RP31

	// Look sequence
	OSCCON &= 0xFF00;
	OSCCON |= 0x46;
	OSCCON |= 0x57;
	OSCCONbits.IOLOCK = 1;
}
