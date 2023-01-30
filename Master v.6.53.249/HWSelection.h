/* 
 * File:   HWSelection.h
 * Author: GianLuca
 *
 * Created on 30 luglio 2015, 0.23
 */

#ifndef HWSELECTION_H
    #define	HWSELECTION_H


#define	HW_VERSION		50	// 42 = v4.2 (Termodinamica Team), 
							// 43 = v4.3 (con micro 4620)
							// 44 = v4.4 (con micro 4620 e pin swap su E2) -> scheda color BLU
                            // 50 = v5.0 (con micro PIC24FJ256GB210) + UART3 e 4 (su palchetto) -> scheda color BLU

/*
#if HW_VERSION==42
	#define HW4550		1		// 1 = OLD HW v4.2
#endif
*/

#if HW_VERSION==43
	#define NEWHW		0		// 1 = Predispone la configurazione e l'utilizzo delle risorse del nuovo HW v4.4, 0 = OLD HW
	#define XTAL10		0		// 1 = Quarzo 10Mhz, 0 = Quarzo da 20Mhz 
#endif

#if HW_VERSION==44
	#define NEWHW		1		// 1 = Predispone la configurazione e l'utilizzo delle risorse del nuovo HW v4.4, 0 = OLD HW
	#define XTAL10		1		// 1 = Quarzo 10Mhz, 0 = Quarzo da 20Mhz 
#endif

#if HW_VERSION==50
	#define DA210       0       // Seleziona micro modello DA210 (Se DA210==0 si utilizza micro GB210)

	#define NEWHW		1		// 1 = Predispone la configurazione e l'utilizzo delle risorse del nuovo HW v4.4, 0 = OLD HW
    #define UART3_ON    1       // Abilita la gestione della UART3 e 4
#endif

#define LED_ON      0           // Inversione HW (Rev.5.0) -> LED Acceso
#define LED_OFF     1           // Inversione HW (Rev.5.0) -> LED Spento

#endif



