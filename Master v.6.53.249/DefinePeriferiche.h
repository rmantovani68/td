#ifndef _PERIFERICHE_H_
	#define _PERIFERICHE_H_

#include "FWSelection.h"
#include "HWSelection.h"


	//---------------------------------------
	// Define I/O PIC24FJ256GB210
	// Definire come "PORTXbits.yyy" se ingresso oppure come "LATXbits.LATBy" se uscita
	// la direzione si imposta relativamente alla posizione del bit con "0" se uscita, oppure con "1" se ingresso
	//---------------------------------------
	#define iA0				PORTAbits.RA0		// DipSwitch Address					//5432 1098 7654 3210
	#define iA1				PORTAbits.RA1		// DipSwitch Address					//0000 0111 1111 1111 => 0x07FF
	#define iA2				PORTAbits.RA2		// DipSwitch Address
	#define iA3				PORTAbits.RA3		// DipSwitch Address
	#define iA4				PORTAbits.RA4		// DipSwitch Address
	#define iA5				PORTAbits.RA5		// DipSwitch Address
	#define iA6				PORTAbits.RA6		// DipSwitch Address
	#define iA7				PORTAbits.RA7		// DipSwitch Address
	#define NotImp_RA8		PORTAbits.RA8		// NOT EXIST
	#define iRA0_NC			PORTAbits.RA9		// reserved for ADC Vref - (connected to GND)
	#define iVref_plus		PORTAbits.RA10		// reserved for ADC Vref + (connected to 2,5Vref)
	#define NotImp_RA11		PORTAbits.RA11		// NOT EXIST
	#define NotImp_RA12		PORTAbits.RA12		// NOT EXIST
	#define NotImp_RA13		PORTAbits.RA13		// NOT EXIST
	#define iFree_RA14		PORTAbits.RA14		// >>> N.C.
	#define iFree_RA15		PORTAbits.RA15		// >>> N.C.

	#define iAn0_TOutComp	LATBbits.LATB0		// AN0  T Out Comp.							//5432 1098 7654 3210												***
	#define iAn1_TInComp	LATBbits.LATB1		// AN1  T In Comp.							//0000 0001 1111 1111	=> 0x01FF									***
	#define iAn2_TCond		LATBbits.LATB2		// AN2  T Condenser																								***
	#define iAn3_TSeaW		LATBbits.LATB3		// AN3  T Sea Water																								***
	#define	iAn4_TLiq		LATBbits.LATB4		// AN4  T Liquid																								***
	#define	iAn5_TGas		LATBbits.LATB5		// AN5  T Gas																									***
	#define	io_PGC			LATBbits.LATB6		// PGEC2
	#define	io_PGD			LATBbits.LATB7		// PGED2
	#define	iAn8_PGas		LATBbits.LATB8		// AN8  P Gas																									***
	#define	iAn9_PLiq		LATBbits.LATB9		// AN9  P Liquid																								***
	#define	iAn10_PCond		LATBbits.LATB10		// AN10 Press Condenser																							***
	#define	iAn11_24VCC		LATBbits.LATB11		// AN11 +24V alim.																								***
	#define	oFree_RB12		LATBbits.LATB12		// >>> N.C.
	#define	oFree_RB13		LATBbits.LATB13		// >>> N.C.
	#define	oFree_RB14		LATBbits.LATB14		// >>> N.C.
	#define	oFree_RB15		LATBbits.LATB15		// >>> N.C.

	#define NotImp_RC0		LATCbits.LATC0		// NOT EXIST								//5432 1098 7654 3210
	#define oFree_RC1		LATCbits.LATC1		// >>> N.C.									//0000 0000 0001 0000	=> 0x0010									***
	#define oFree_RC2		LATCbits.LATC2		// >>> N.C.																										***
	#define oRTS_UART4		LATCbits.LATC3		// RTS UART n.4		pin.8																							***
	#define oRTS_UART3		LATCbits.LATC4		// RTS UART n.3		pin.9						// RPI41															***
	#define NotImp_RC5		LATCbits.LATC5		// NOT EXIST
	#define NotImp_RC6		LATCbits.LATC6		// NOT EXIST
	#define NotImp_RC7		LATCbits.LATC7		// NOT EXIST
	#define NotImp_RC8		LATCbits.LATC8		// NOT EXIST
	#define NotImp_RC9		LATCbits.LATC9		// NOT EXIST
	#define NotImp_RC10		LATCbits.LATC10		// NOT EXIST
	#define NotImp_RC11		LATCbits.LATC11		// NOT EXIST
	#define iFree_RC12		LATCbits.LATC12		// >>> N.C.
	#define iFree_RC13		LATCbits.LATC13		// >>> N.C.
	#define iKeySwitch		PORTCbits.RC14 		// Key Switch for User Interface Display 7 Segment																***
	#define iFree_RC15		LATCbits.LATC15		// >>> N.C.

	#define oSegmentA		LATDbits.LATD0		// Segment A - 7SegDisplay					//5432 1098 7654 3210												***
	#define oSegmentB		LATDbits.LATD1		// Segment B - 7SegDisplay					//1000 1100 0000 0011	==> 0x8C03									***
	#define oSegmentC		LATDbits.LATD2		// Segment C - 7SegDisplay																						***
	#define oSegmentD		LATDbits.LATD3		// Segment D - 7SegDisplay																						***
	#define oSegmentE		LATDbits.LATD4		// Segment E - 7SegDisplay																						***
	#define oSegmentF		LATDbits.LATD5		// Segment F - 7SegDisplay																						***
	#define oSegmentG		LATDbits.LATD6		// Segment G - 7SegDisplay																						***
	#define oSegmentDP		LATDbits.LATD7		// Segment DP - 7SegDisplay																						***
#if (K_AbilCompressor2==1)		// K_AbilCompressor2 = 1 -> Compressore doppio
	#define iDigiIn1     	!PORTDbits.RD8		// Digital Input n.1																							***
	#define iDigiIn2    	!PORTDbits.RD9		// Digital Input n.2																							***
	#define iThermicComp2	!PORTDbits.RD10		// Digital Input n.3																							***
	#define iLowOilLevel1	!PORTDbits.RD11		// Digital Input n.4																							***
	#define iLowOilLevel2	!PORTDbits.RD12		// Digital Input n.5																							***
	#define iFlowSwitch		!PORTDbits.RD13		// Digital Input n.6																							***
#else
	#define iDigiIn1    	!PORTDbits.RD8		// Digital Input n.1																							***
	#define iDigiIn2    	!PORTDbits.RD9		// Digital Input n.2																							***
	#define iLowPressure	!PORTDbits.RD10		// Digital Input n.3																							***
	#define iThermicComp1	!PORTDbits.RD11		// Digital Input n.4																							***
#if (K_AbilRechargeOil==1)
	#define	iLowOilLevel1	!PORTDbits.RD12 	//Digital Input n.5
#else
	#define	iFloodSensor	!PORTDbits.RD12 	//Digital Input n.5
#endif	
	#define iFlowSwitch		!PORTDbits.RD13		// Digital Input n.6																							***
#endif
	#define iFree_RD14		LATDbits.LATD14		// >>> N.C.
	#define iRx_Master		LATDbits.LATD15		// RP?? - RX UART n.1 Master	RP17	


/*
Cablaggio uscite rele con  1 compressore:
--------------------------------------------------
1 = Inverter			-
2 = Valvola Caldo/Freddo	-
3 = Pompa Acqua			-
4 = Pompa Acqua 2
5 = PersErr			-
*/
#if (K_AbilCompressor2==0)		// K_AbilCompressor2 = 1 -> Compressore doppio
	#define oPersErr    	LATEbits.LATE0		//	Rele Inverter Compressore				//5432 1098 7654 3210												***
	#define oCaldoFreddo	LATEbits.LATE1		//	Rele Valvola Caldo/Freddo				//0000 0000 1100 0000	=> 0x00C0									***
	#define oPompaAcqua		LATEbits.LATE2		//	Rele Pompa Acqua																							***
	#define oPompaAcqua2	LATEbits.LATE3		//	Rele Pompa Acqua 2																							***	
	#define	oOil1			LATEbits.LATE4		//	Rele carica olio compressore 1
#else
/*
Cablaggio uscite rele con doppio compressore:
--------------------------------------------------
1 = Pompa Acqua2			
2 = Valvola Caldo/Freddo	
3 = Pompa Acqua				
4 = Olio 2					
5 = Olio 1					
*/
#if(K_OnlyOneInverterComp2==1)
	#define oCompressor2	LATEbits.LATE0		//	Rele Pompa Acqua					RL1 = AUX3					***
#elif(K_Abil_Double_Pump==1)
	#define oPompaAcqua2	LATEbits.LATE0		//	Rele Pompa Acqua					RL1 = AUX3					***
#else
	#define	oPersErr		LATEbits.LATE0		//	Rele Segnalazione Errori persistenti
#endif											//						***
	#define oCaldoFreddo	LATEbits.LATE1		//	Rele Valvola Caldo/Freddo			RL2 = VALVE WARM/COLD																		***								
	#define oPompaAcqua		LATEbits.LATE2		//	Rele Pompa Acqua					RL3 = WATER PUMP																		***
	#define oOil2			LATEbits.LATE3		//	Rele Olio 2							RL4 = AUX2																		***
	#define oOil1			LATEbits.LATE4		//	Rele Olio 1							RL5 = AUX1																		***
#endif

    #define iFree_RE5		LATEbits.LATE5		// >>> N.C.
	#define oSCL			LATEbits.LATE6		// SCL3	EEPROM I2C Esterna
	#define ioSDA			LATEbits.LATE7		// SDA3 EEPROM I2C Esterna
	#define iFree_RE8		LATEbits.LATE8		// >>> N.C.
	#define iFree_RE9		LATEbits.LATE9		// >>> N.C.
	#define NotImp_RE10		LATEbits.LATE10		// NOT EXIST
	#define NotImp_RE11		LATEbits.LATE11		// NOT EXIST
	#define NotImp_RE12		LATEbits.LATE12		// NOT EXIST
	#define NotImp_RE13		LATEbits.LATE13		// NOT EXIST
	#define NotImp_RE14		LATEbits.LATE14		// NOT EXIST
	#define NotImp_RE15		LATEbits.LATE15		// NOT EXIST

	#define iFree_RF0		LATFbits.LATF0		// >>> N.C.									//5432 1098 7654 3210
	#define iFree_RF1		LATFbits.LATF1		// >>> N.C.									//0000 0001 0001 0100	=> 0x0114									***
	#define oTx_Slave		LATFbits.LATF2		// RP30 - TX BUS Slave
	#define oRTS_Slave		LATFbits.LATF3		// Out RTS BUS Slave
	#define oTx_Master		LATFbits.LATF4		// RP10 - TX BUS Master
	#define oRTS_Master		LATFbits.LATF5		// Out RTS BUS Master
	#define NotImp_RF6		LATFbits.LATF6		// NOT EXIST
	#define NotImp_RF7		LATFbits.LATF7		// >>> N.C.
	#define iRx_Slave		LATFbits.LATF8		// RP15 - RX BUS Slave
	#define NotImp_RF9		LATFbits.LATF9		// NOT EXIST
	#define NotImp_RF10		LATFbits.LATF10		// NOT EXIST
	#define NotImp_RF11		LATFbits.LATF11		// NOT EXIST
	#define iFree_RF12		LATFbits.LATF12		// >>> N.C.
	#define iFree_RF13		LATFbits.LATF13		// >>> N.C.
	#define NotImp_RF14		LATFbits.LATF14		// NOT EXIST
	#define NotImp_RF15		LATFbits.LATF15		// NOT EXIST

	#define oPwrLed			LATGbits.LATG0		// LED BLU Power							//5432 1098 7654 3210
	#define oErrLed			LATGbits.LATG1		// LED ROSSO Errori							//0000 0000 0100 0000	=> 0x0040
	#define iFree_RG2		LATGbits.LATG2		// >>> N.C.
	#define iFree_RG3		LATGbits.LATG3		// >>> N.C.
	#define NotImp_RG4		LATFbits.LATG4		// NOT EXIST
	#define NotImp_RG5		LATFbits.LATG5		// NOT EXIST
	#define oTx_UART3		LATGbits.LATG6		// RP21 - TX UART n.3																							***
	#define iRx_UART3		LATGbits.LATG7		// RP26 - RX UART n.3																							***
	#define oTx_UART4		LATGbits.LATG8		// RP19 - TX UART n.4																							***
	#define iRx_UART4		LATGbits.LATG9		// RP27 - RX UART n.4																							***

	#define NotImp_RG10		LATFbits.LATG10		// NOT EXIST	
	#define NotImp_RG11		LATFbits.LATG11		// NOT EXIST
	#define oDAC_SCK		LATGbits.LATG12		// Out segnale SCK DAC 0..10V																					***
	#define oDAC_SDO		LATGbits.LATG13		// Out segnale SDO DAC 0..10V																					***
	#define oDAC_LD			LATGbits.LATG14		// Out segnale LOAD DAC 0..10V																					***
	#define iFree_RG15		LATGbits.LATG15		// >>> N.C.



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
