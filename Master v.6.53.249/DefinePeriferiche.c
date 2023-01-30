//----------------------------------------------------------------------------------
//	Progect name:	ProtocolloComunicazione.c
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			Emanuele
//	Date:			22/05/2011
//	Description:	Corpo delle funzioni che gestiscono la comunicazione con il sistema
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

#include "DefinePeriferiche.h"

#include "Driver_Comunicazione.h"
#include "Driver_ModBus.h"
#include "ProtocolloComunicazione.h"
#include "ProtocolloModBus.h"
#include "EEPROM.h"
#include "PID.h"
#include "ADC.h"
#include "DAC.h"
#include "Core.h"
#include "ServiceComunication.h"
/* */




#define	NULL	0

/*
VariabiliGlobaliADC();
VariabiliGlobaliDAC();
VariabiliGlobaliDriverComunicazione();
VariabiliGlobaliDriverModBus();
VariabiliGlobaliProtocolloComunicazione();
VariabiliGlobaliProtocolloModBus();
VariabiliGlobaliEEPROM();
VariabiliGlobaliPID();
VariabiliGlobaliCore();
GlobalVarServiceComunication();
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
	_IOLOCK = 0; //OSCCONbits.IOLOCK = 0;
    

	//-------------------------------------------------------------------
	// Configurazione Input (*** = NEW HW Master 5.0)
	//-------------------------------------------------------------------
/*	---- INPUT ----
	Ogni input ha una periferica, una volta abilitata, con i 5 bit del registro, si imposta il numero dell'RPIN(X)
	Ex:
	RPINR18bits.U1RX = 4;		// il segnale di input U1Rx (reciver UART1), viene assegnato all'ingresso RPIN4
*/
	_INT1R = 0x1F; //RPINR0bits.INT1R = 0x1F;		// Ext. interrupt 1
	_INT2R0 = 0x1F; //RPINR1bits.INT2R = 0x1F;		// Ext. interrupt 2
	_INT3R0 = 0x1F; //RPINR1bits.INT3R = 0x1F;		// Ext. interrupt 3
	_INT4R = 0x1F; //RPINR2bits.INT4R = 0x1F;		// Ext. interrupt 4
	_T2CKR = 0x1F; //RPINR3bits.T2CKR = 0x1F;		// Timer 2 ext CK
	_T3CKR = 41; //RPINR3bits.T3CKR = 41;			// Timer 3 ext CK			==> on RPI41
	_T4CKR = 0x1F; //RPINR4bits.T4CKR = 0x1F;		// Timer 4 ext CK
	_T5CKR = 0x1F; //RPINR4bits.T5CKR = 0x1F;		// Timer 5 ext CK
	_IC1R = 0x1F; //RPINR7bits.IC1R = 0x1F;			// Input capture 1
	_IC2R = 0x1F; //RPINR7bits.IC2R = 0x1F;			// Input capture 2
	_IC3R = 0x1F; //RPINR8bits.IC3R = 0x1F;			// Input capture 3
	_IC4R = 0x1F; //RPINR8bits.IC4R = 0x1F;			// Input capture 4
	_IC5R = 0x1F; //RPINR9bits.IC5R = 0x1F;			// Input capture 5
	_IC6R = 0x1F; //RPINR9bits.IC6R = 0x1F;			// Input capture 6
	_IC7R = 0x1F; //RPINR10bits.IC7R = 0x1F;		// Input capture 7
	_IC8R = 0x1F; //RPINR10bits.IC8R = 0x1F;		// Input capture 8
	_OCFAR = 0x1F; //RPINR11bits.OCFAR = 0x1F;		// Output compare Fault A
	_OCFBR = 0x1F; //RPINR11bits.OCFBR = 0x1F;		// Output compare Fault B
	_IC9R = 0x1F; //RPINR15bits.IC9R = 0x1F;		// Input capture 9
	_U3RXR = 26; //0x1F; //RPINR17bits.U3RXR = 0x1F;// UART 3 Recive			==> on RP26 *** pin.11
	_U1RXR = 5; //RPINR18bits.U1RXR = 5;			// UART 1 Recive			==> on RP5
	_U1CTSR = 0x1F; //RPINR18bits.U1CTSR = 0x1F;	// UART 1 Clear to send
	_U2RXR = 15; //RPINR19bits.U2RXR = 15;			// UART 2 Recive			==> ON RP15
	_U2CTSR = 0x1F; //RPINR19bits.U2CTSR = 0x1F;	// UART 2 Clear to send
	_SDI1R = 0x1F; //RPINR20bits.SDI1R = 0x1F;		// SPI 1 Data Input
	_SCK1R = 0x1F; //RPINR20bits.SCK1R = 0x1F;		// SPI 1 Clock Input
	_SS1R = 0x1F; //RPINR21bits.SS1R = 0x1F;		// SPI 1 Slave Select Input
	_U3CTSR = 0x1F; //RPINR21bits.U3CTSR = 0x1F;	// UART 3 Clear to send
	_SDI2R = 0x1F; //RPINR22bits.SDI2R = 0x1F;		// SPI 2 Data Input
	_SCK2R = 0x1F; //RPINR22bits.SCK2R = 0x1F;		// SPI 2 Clock Input
	_SS2R = 0x1F; //RPINR23bits.SS2R = 0x1F;		// SPI 2 Slave Select Input
	_U4RXR = 27; //0x1F; //RPINR27bits.U4RXR = 0x1F;// UART 4 Recive			==> on RP27 *** pin.14
	_U4CTSR = 0x1F; //RPINR27bits.U4CTSR = 0x1F;	// UART 4 Clear to send 
	_SDI3R = 0x1F; //RPINR28bits.SDI3R = 0x1F;		// SPI 3 Data Input
	_SCK3R = 0x1F; //RPINR28bits.SCK3R = 0x1F;		// SPI 3 Clock Input
	_SS3R = 0x1F; //RPINR29bits.SS3R = 0x1F;		// SPI 3 Slave Select Input

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
	_RP0R = NULL; //RPOR0bits.RP0R = NULL;		// RP0
	_RP1R = NULL; //RPOR0bits.RP1R = NULL;		// RP1
	_RP2R = NULL; //RPOR1bits.RP2R = NULL;		// RP2
	_RP3R = NULL; //RPOR1bits.RP3R = NULL;		// RP3
	_RP4R = NULL; //RPOR2bits.RP4R = NULL;		// RP4
	_RP5R = NULL; //RPOR2bits.RP5R = NULL;		// RP5
	_RP6R = NULL; //RPOR3bits.RP6R = NULL;		// RP6
	_RP7R = NULL; //RPOR3bits.RP7R = NULL;		// RP7
	_RP8R = NULL; //RPOR4bits.RP8R = NULL;		// RP8
	_RP9R = NULL; //RPOR4bits.RP9R = NULL;		// RP9
	_RP10R = U1TX; //RPOR5bits.RP10R = U1TX;		// RP10	=> U1Tx 485
	_RP11R = NULL; //RPOR5bits.RP11R = NULL;		// RP11
	_RP12R = NULL; //RPOR6bits.RP12R = NULL;		// RP12
	_RP13R = NULL; //RPOR6bits.RP13R = NULL;		// RP13
	_RP14R = NULL; //RPOR7bits.RP14R = NULL;		// RP14
	_RP15R = NULL; //RPOR7bits.RP15R = NULL;		// RP15
	_RP16R = NULL; //RPOR8bits.RP16R = NULL;		// RP16
	_RP17R = NULL; //RPOR8bits.RP17R = NULL;		// RP17
	_RP18R = NULL; //RPOR9bits.RP18R = NULL;		// RP18
	_RP19R = U4TX; //NULL; //RPOR9bits.RP19R = NULL;		// RP19 => U4Tx 485   ***  pin.12  
	_RP20R = NULL; //RPOR10bits.RP20R = NULL;	// RP20
	_RP21R = U3TX; //OC_1; //RPOR10bits.RP21R = OC_1;		// RP21 => U3Tx 485   ***  pin.10                   // RP21 => OC1 Fan Output compare PWM
	_RP22R = NULL; //RPOR11bits.RP22R = NULL;	// RP22
	_RP23R = NULL; //RPOR11bits.RP23R = NULL;	// RP23
	_RP24R = NULL; //RPOR12bits.RP24R = NULL;	// RP24
	_RP25R = NULL; //RPOR12bits.RP25R = NULL;	// RP25
	_RP26R = NULL; //RPOR13bits.RP26R = NULL;	// RP26
	_RP27R = NULL; //RPOR13bits.RP27R = NULL;	// RP27
	_RP28R = NULL; //RPOR14bits.RP28R = NULL;	// RP28
	_RP29R = NULL; //RPOR14bits.RP29R = NULL;	// RP29
	_RP30R = U2TX; //RPOR15bits.RP30R = U2TX;	// RP30	=> U2Tx
	_RP31R = NULL; //RPOR15bits.RP31R = NULL;	// RP31

	// Look sequence
	OSCCON &= 0xFF00;
	OSCCON |= 0x46;
	OSCCON |= 0x57;
	_IOLOCK = 1; //OSCCONbits.IOLOCK = 1;
}
