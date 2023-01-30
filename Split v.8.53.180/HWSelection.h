/* 
 * File:   HWSelection.h
 * Author: GianLuca
 *
 * Created on 30 luglio 2015, 0.23
 */

#ifndef HWSELECTION_H
    #define	HWSELECTION_H

#define	HW_VERSION		86	// 82 = v8.2 (GL) con uC GB110 + NEWH
							// 83 = v8.3 (GL)  con uC GB210 + NEWH scheda color BLU
                            // 85 = v8.5 (GL)  con uC GB210 + NEWH + UART3_ON (su palchetto) scheda color BLU
                            // 86 = v8.6 come v 8.5 ma con lettura canale +24V



#if HW_VERSION==82
#endif

#if HW_VERSION==83
	#define GB210	1           // Seleziona micro modello GB210. Per micro modello GB110 commentare la riga
#endif

#if HW_VERSION==85
	#define GB210       1       // Seleziona micro modello GB210. Per micro modello GB110 commentare la riga
    #define UART3_ON    1       // Abilita la gestione della UART3 
#endif

#if HW_VERSION==86
	#define DA210       0       // Seleziona micro modello DA210 
#if(DA210==1)
    #define GB210       0       
#else
    #define GB210       1       // Seleziona micro modello GB210.
#endif
    #define UART3_ON    1       // Abilita la gestione della UART3 
    #define SUPPLY24_ON 1       // Abilita la gestione del canale AD Supply 24V
#endif


#define LED_ON      0           // Inversione HW (Rev.5.0) -> LED Acceso
#define LED_OFF     1           // Inversione HW (Rev.5.0) -> LED Spento

#endif

