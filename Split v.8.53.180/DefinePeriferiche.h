#ifndef _PERIFERICHE_H_
	#define _PERIFERICHE_H_

	#include "FWSelection.h"
	//---------------------------------------
	// Define
	//---------------------------------------
	#define iA0				PORTAbits.RA0	//5432 1098 7654 3210
	#define iA1				PORTAbits.RA1	//0000 0111 1111 1111 => 0x07FF
	#define iA2				PORTAbits.RA2
	#define iA3				PORTAbits.RA3
	#define iA4				PORTAbits.RA4
	#define iA5				PORTAbits.RA5
	#define iA6				PORTAbits.RA6	
	#define iA7				PORTAbits.RA7
	#define NotImp_RA8		PORTAbits.RA8
	#define iRA0_NC			PORTAbits.RA9
	#define iVref_plus		PORTAbits.RA10		// reserved for ADC Vref + (2,5V)
	#define NotImp_RA11		PORTAbits.RA11
	#define NotImp_RA12		PORTAbits.RA12
	#define NotImp_RA13		PORTAbits.RA13
	#define iFree_RA14		PORTAbits.RA14
	#define iFree_RA15		PORTAbits.RA15

	#define iAn0_TSmall		LATBbits.LATB0		// ANA0		//5432 1098 7654 3210
	#define iAn1_TBig		LATBbits.LATB1		// ANA1		//0000 0001 1111 1111	=> 0x01FF
	#define iAn2_TOut		LATBbits.LATB2		// ANA2
	#define iAn3_TAmb		LATBbits.LATB3		// ANA3
	#define	iAn4_PBig		LATBbits.LATB4		// ANA4
	#define	iAn5_PSmall		LATBbits.LATB5		// ANA5
	#define	io_PGC			LATBbits.LATB6		// PGEC2
	#define	io_PGD			LATBbits.LATB7		// PGED2
	#define	iAn8_IFan		LATBbits.LATB8		// ANA8
	#define	oFree_RB9		LATBbits.LATB9
	#define	oFree_RB10		LATBbits.LATB10
	#define	oFree_RB11		LATBbits.LATB11
	#define	oFree_RB12		LATBbits.LATB12
	#define	oFree_RB13		LATBbits.LATB13
	#define	oFree_RB14		LATBbits.LATB14
	#define	oFree_RB15		LATBbits.LATB15

	#define NotImp_RC0		LATCbits.LATC0		//5432 1098 7654 3210
	#define oMode0			LATCbits.LATC1		//0000 0000 0001 0000	=> 0x0010
	#define oMode1			LATCbits.LATC2
	#define oMode2			LATCbits.LATC3
	#define iTacho			LATCbits.LATC4		// RPI41
	#define NotImp_RC5		LATCbits.LATC5
	#define NotImp_RC6		LATCbits.LATC6
	#define NotImp_RC7		LATCbits.LATC7
	#define NotImp_RC8		LATCbits.LATC8
	#define NotImp_RC9		LATCbits.LATC9
	#define NotImp_RC10		LATCbits.LATC10
	#define NotImp_RC11		LATCbits.LATC11
	#define iFree_RC12		LATCbits.LATC12
	#define iFree_RC13		LATCbits.LATC13
	#define iFree_RC14		LATCbits.LATC14
	#define iFree_RC15		LATCbits.LATC15

	#define iFault			PORTDbits.RD0		//5432 1098 7654 3210
	#define iHome			PORTDbits.RD1		//1000 1100 0000 0011	==> 0x8C03
	#define oSleep			LATDbits.LATD2		//
	#define oRst			LATDbits.LATD3		//
	#define oDecay			LATDbits.LATD4		//
	#define oEnable			LATDbits.LATD5		//
	#define oDir			LATDbits.LATD6		//
	#define oStep			LATDbits.LATD7		//
#if(K_Heater_Abil==1)
	#define oDigitOutHeater	LATDbits.LATD8		//
	#define iDigitOutHeater PORTDbits.RD8		//
#elif(K_Dampers_Selection>0)
    #define oDigitDampers1	LATDbits.LATD8		//
#elif(K_LoopPumpEnable==1)
    #define oDigitPumpLoop1	LATDbits.LATD8		//
	#define iDigitPumpLoop1	PORTDbits.RD8		//
#elif(SplitMode==K_I_Am_Atex_Fan) || (SplitMode==K_I_Am_On_Off_Fan)
    #define oDigitFan   	LATDbits.LATD8		//
	#define iDigitFan   	PORTDbits.RD8		//
#else
    #define oDigitEvapFan	LATDbits.LATD8		//
	#define iDigitEvapFan   PORTDbits.RD8		//
#endif
#if(K_Dampers_Selection>1)
    #define oDigitDampers2	LATDbits.LATD9		//
#elif(K_LoopDoublePumpEnable==1)
    #define oDigitPumpLoop2	LATDbits.LATD9		//
	#define iDigitPumpLoop2	PORTDbits.RD9		//
#elif(K_Double_Heater_Abil==1)
    #define oDigitOutHeater2 LATDbits.LATD9		//
	#define iDigitOutHeater2 PORTDbits.RD9		//
#else
    #define oDigitOutGas    LATDbits.LATD9		//
	#define iDigitOutGas    PORTDbits.RD9		//
#endif
	#define iDigiIn1		!PORTDbits.RD10		//
	#define iDigiIn2		!PORTDbits.RD11		//
	#define iDigiIn3		!PORTDbits.RD12		//
	#define oPWM_Fan1		LATDbits.LATD12		//
#if(K_Channel_Liquid_Light==1)
	#define oDigitOutLight	LATDbits.LATD13		//
#else
	#define oDigitOutDefros	LATDbits.LATD13		//
	#define iDigitOutDefros	PORTDbits.RD13		//
#endif
    #define iFree_RD14		LATDbits.LATD14		//
	#define iRx_Master		LATDbits.LATD15		// RP17

	#define iFree_RE0		LATEbits.LATE0		//5432 1098 7654 3210
	#define iFree_RE1		LATEbits.LATE1		//0000 0000 1100 0000	=> 0x00C0
	#define iFree_RE2		LATEbits.LATE2		//
	#define iFree_RE3		LATEbits.LATE3		//
	#define oCurrDriver		LATEbits.LATE4		//
    //#define iFree_RE4		LATEbits.LATE4		//
    #define iFree_RE5		LATEbits.LATE5		//
	#define oSCL			LATEbits.LATE6		// SCL3
	#define ioSDA			LATEbits.LATE7		// SDA3
	#define iFree_RE8		LATEbits.LATE8		//
	#define iFree_RE9		LATEbits.LATE9		//
	#define NotImp_RE10		LATEbits.LATE10		//
	#define NotImp_RE11		LATEbits.LATE11		//
	#define NotImp_RE12		LATEbits.LATE12		//
	#define NotImp_RE13		LATEbits.LATE13		//
	#define NotImp_RE14		LATEbits.LATE14		//
	#define NotImp_RE15		LATEbits.LATE15		//

	#define iFree_RF0		LATFbits.LATF0		//5432 1098 7654 3210
	#define oLedEEV			LATFbits.LATF1		//0000 0001 0001 0100	=> 0x0114
	#define oTx_Slave		LATFbits.LATF2		// RP30
	#define oRTS_Slave		LATFbits.LATF3		//
	#define oTx_Master		LATFbits.LATF4		// RP10
	#define oRTS_Master		LATFbits.LATF5		//
	#define NotImp_RF6		LATFbits.LATF6		//
	#define NotImp_RF7		LATFbits.LATF7		//
	#define iRx_Slave		LATFbits.LATF8		// RP15
	#define NotImp_RF9		LATFbits.LATF9		//
	#define NotImp_RF10		LATFbits.LATF10		//
	#define NotImp_RF11		LATFbits.LATF11		//
	#define iFree_RF12		LATFbits.LATF12		//
	#define iFree_RF13		LATFbits.LATF13		//
	#define NotImp_RF14		LATFbits.LATF14		//
	#define NotImp_RF15		LATFbits.LATF15		//

	#define oPwrLed			LATGbits.LATG0		//5432 1098 7654 3210
	#define oErrLed			LATGbits.LATG1		//0000 0000 0100 0000	=> 0x0040
	#define iFree_RG2		LATGbits.LATG2		//
	#define iFree_RG3		LATGbits.LATG3		//
	#define NotImp_RG4		LATFbits.LATG4		//
	#define NotImp_RG5		LATFbits.LATG5		//
	#define oPWM_Fan2		LATGbits.LATG6		// ---> RP21

#ifdef UART3_ON
	#define oRTS_UART3      LATGbits.LATG7		//
	#define oTx_UART3		LATGbits.LATG8		//  RP19
	#define iRx_UART3		LATGbits.LATG9		//  RP27
#else
	#define iFree_RG7		LATGbits.LATG7		//
	#define iFree_RG8		LATGbits.LATG8		//
	#define iFree_RG9		LATGbits.LATG9		//
#endif

	#define NotImp_RG10		LATFbits.LATG10		//
	#define NotImp_RG11		LATFbits.LATG11		//
	#define iFree_RG12		LATGbits.LATG12		//
	#define iFree_RG13		LATGbits.LATG13		//
	#define iFree_RG14		LATGbits.LATG14		//
	#define iFree_RG15		LATGbits.LATG15		//

	//---------------------------------------
	// Define
	//---------------------------------------
	// Ad ogni pin RPORn, può essere assegnato l'id di una uscita di una qualsiasi periferica
	//#define NULL		0		// Null
	#define C1OUT		1		// Comparator 1 Output
	#define C2OUT		2		// Comparator 2 Output
	#define U1TX		3		// UART 1 Transmit
	#define U1RTS		4		// UART 1 Request to Send
	#define U2TX		5		// UART 2 Transmit
	#define U2RTS		6		// UART 2 Request to Send
	#define SDO1		7		// SPI 1 Data Output
	#define SCK1OUT		8		// SPI 1 Clock Output
	#define SS1OUT		9		// SPI 1 Slave Select Output
	#define SDO2		10		// SPI 2 Data Output
	#define SCK2OUT		11		// SPI 2 Clock Output
	#define SS2OUT		12		// SPI 2 Slave Select Output
	#define OC_1			18		// Output Compare 1
	#define OC_2			19		// Output Compare 2
	#define OC_3			20		// Output Compare 3
	#define OC_4			21		// Output Compare 4
	#define OC_5			22		// Output Compare 5
	#define	OC_6			23		// Output Compare 6
	#define	OC_7			24		// Output Compare 7
	#define	OC_8			25		// Output Compare 8
	#define	U3TX		28		// UART 3 Transmit
	#define	U3RTS		29		// UART 3 Request to Send
	#define	U4TX		30		// UART 4 Transmit
	#define	U4RTS		31		// UART 4 Request to Send
	#define SDO3		32		// SPI 3 Data Output
	#define SCK3OUT		33		// SPI 3 Clock Output
	#define SS3OUT		34		// SPI 3 Slave Select Output
	#define OC9			35		// Output Compare 9
	#define	C3OUT		36		//	Comparator 3 Output

	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	void Remap_RP(void);

#endif
