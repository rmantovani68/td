//----------------------------------------------------------------------------------
//	Progect name:	Timer.h
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			TDM Team
//	Date:			04/01/2014
//	Description:	Header delle funzioni Timer
//----------------------------------------------------------------------------------
#ifndef _TIMER_H_
	#define _TIMER_H_

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
	{	
        unsigned long   Value;		// Contatore di ms
		unsigned long   Time;		// Valore di limite del timer
		unsigned 		Enable:1;	// Abilitazione del timer
		unsigned 		TimeOut:1;	// Flag di time out
	} TypTimerLong;
    
    
	typedef struct
	{	unsigned char	Secondi;
		unsigned char	Minuti;
		unsigned char	Ore;
		unsigned char	Giorno;
		unsigned char	GiornoSettimana;
		unsigned char	Settimana;
		unsigned char	Mese;
		unsigned char	Anno;
	} TypRTC;

	//----------------------------------
	//	Prototipi delle funzioni
	//----------------------------------
	void Config_TMR1 (unsigned char PreScaler, int TickValue);
	void Start_TMR1(void);
	void Stop_TMR1(void);
	void Pasue_TMR1(void);
	void ChkTimer(volatile TypTimer * Timer);
    void ChkTimerLong(volatile TypTimerLong * Timer);    
	void Init_Timer(void);
	//void RefreshRTC(TypRTC * RTC);

#endif
