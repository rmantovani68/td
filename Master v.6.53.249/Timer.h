//----------------------------------------------------------------------------------
//	Progect name:	Timer.h
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			Emanuele
//	Date:			04/01/2014
//	Description:	Header delle funzioni Timer
//----------------------------------------------------------------------------------
#ifndef _TIMER_H_
	#define _TIMER_H_
#include "FWSelection.h"

#if (K_AbilMODBCAN==1)  
#define K_INTPRI_T1     7//6//5     // Timers mSec 
#define K_INTPRI_T2     5           // RX Interchar & Check validità messaggi ricevuti & Gestione lampeggio LEDs
#define K_INTPRI_T3     1//2//4        // Gestione invio messaggi di "Alive" vs convertitore CAN -> Scheiber
#define K_INTPRI_T4     4//         // AD Conversion
#define K_INTPRI_T5     5           // Timers Secondi

#define K_TIMEBASE_T1   250         // 250 = 1mS        1 = 4uS
#define K_TIMEBASE_T2   250         // 250 = 1mS
#define K_TIMEBASE_T3   62500       // 62500 = 250mS
#define K_TIMEBASE_T4   5000        // 5000 = 20mS
#define K_TIMEBASE_T5   62500       // 62500 = 250mS
#else
#define K_INTPRI_T1     5           // Timers mSec 
#define K_TIMEBASE_T1   250         // 250 = 1mS        1 = 4uS
#endif //#if (K_AbilMODBCAN==1)   


	//----------------------------------
	// Struttura timer
	//----------------------------------
	typedef struct
	{	unsigned int	Value;		// Contatore di ms
		unsigned int	Time;		// Valore di limite del timer
		unsigned 		Enable:1;	// Abilitazione del timer
		unsigned 		TimeOut:1;	// Flag di time out
	} TypTimer;

	typedef struct
	{	unsigned char	Secondi;
		unsigned char	Minuti;
		unsigned char	Ore;
		unsigned char	Giorno;
		unsigned char	GiornoSettimana;
		unsigned char	Settimana;
		unsigned char	Mese;
		unsigned char	Anno;
		unsigned int	HoursOfLife;        
	} TypRTC;

	//----------------------------------
	//	Prototipi delle funzioni
	//----------------------------------
	void Config_TMR1 (unsigned char PreScaler, int TickValue);
	void Start_TMR1(void);
	void Stop_TMR1(void);
	void Pasue_TMR1(void);
	void ChkTimer(volatile TypTimer * Timer);
#if (K_AbilMODBCAN==1)    
	void Init_Timer1(void);
#else
	void Init_Timer(void);
#endif //#if (K_AbilMODBCAN==1)       
	void RefreshRTC(TypRTC * RTC);
    
#if (K_AbilMODBCAN==1)        
    void Shot(int trigger, int *out, int *cnt, int time);
    
    void Config_TMR2 (unsigned char PreScaler, int TickValue);
	void Start_TMR2(void);
	void Stop_TMR2(void);
	void Pasue_TMR2(void);
	void Init_Timer2(void);
    
	void Config_TMR3 (unsigned char PreScaler, int TickValue);
	void Start_TMR3(void);
	void Stop_TMR3(void);
	void Pasue_TMR3(void);
	void Init_Timer3(void);

	void Config_TMR4 (unsigned char PreScaler, int TickValue);
	void Start_TMR4(void);
	void Stop_TMR4(void);
	void Pasue_TMR4(void);
	void Init_Timer4(void);

	void Config_TMR5 (unsigned char PreScaler, int TickValue);
	void Start_TMR5(void);
	void Stop_TMR5(void);
	void Pasue_TMR5(void);
	void Init_Timer5(void);    

    void LoadTimer(volatile TypTimer * Timer, unsigned int Time, unsigned int PresetValue,  unsigned int Enable);
#endif //#if (K_AbilMODBCAN==1) 
    
#endif
