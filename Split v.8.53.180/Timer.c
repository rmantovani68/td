//----------------------------------------------------------------------------------
//	Progect name:	Timer.c
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			TDM Team
//	Date:			04/01/2014
//	Description:	Corpo delle funzioni Timer
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
#include "Timer.h"

//----------------------------------------------------------------------------------
//	Funzioni        
//----------------------------------------------------------------------------------
void Config_TMR1 (unsigned char PreScaler, int TickValue)
/*  
	PIC24FJ256 -> Fosc = 16Mhz 
	(Fosc)      => PreScal	=> PR1 TickValue	
	16Mhz		=> 64		=> 250			
	16Mhz		=> 250Khz	=> 1Khz (1mS)	
  
	PIC24FJ256 -> Fosc = 16Mhz 
	(Fosc)      => PreScal	=> PR1 TickValue	
	16Mhz		=> 64		=> 1000			
	16Mhz		=> 250Khz	=> 250Hz (4mS)	
 *  		
*/
{
	T1CON = 0;		// ripulisco la word di cfg
	switch (PreScaler)
	{
		case 1:				// Setto la configurazione del PostScaler
			T1CONbits.TCKPS0 = 0;
			T1CONbits.TCKPS1 = 0;
			break;

		case 8:
			T1CONbits.TCKPS0 = 1;
			T1CONbits.TCKPS1 = 0;
			break;

		case 64:
			T1CONbits.TCKPS0 = 0;
			T1CONbits.TCKPS1 = 1;
			break;

		case 255:
			T1CONbits.TCKPS0 = 1;
			T1CONbits.TCKPS1 = 1;
			break;
	}
	TMR1 = 0;				// resetto il contatore
	PR1 = TickValue;
	IFS0bits.T1IF = 0;		// resetto il flag di interrupt
	IPC0bits.T1IP = 5;		// imposto la priorità dell'interrupt bassa	
	IEC0bits.T1IE = 1;		// abilito l'interrupt di TMR1
}

void Start_TMR1(void)
{
	T1CONbits.TON = 1;		// attivo il timer
}

void Stop_TMR1(void)
{
	T1CONbits.TON = 0;		// disattivo il timer
	TMR1 = 0;		// resetto il conttore
}

void Pasue_TMR1(void)
{
	T1CONbits.TON = 0;		// disattivo il timer
}

void ChkTimer(volatile TypTimer * Timer)
{
	if(Timer->Enable == 1)
	{
		Timer->Value += 1;
		if ((Timer->Value) >= (Timer->Time))
		{
			Timer->TimeOut = 1;
			Timer->Value = 0;
		}
	}
}

void ChkTimerLong(volatile TypTimerLong * Timer)
{
	if(Timer->Enable == 1)
	{
		Timer->Value += 1;
		if ((Timer->Value) >= (Timer->Time))
		{
			Timer->TimeOut = 1;
			Timer->Value = 0;
		}
	}
}

void Init_Timer(void)
{
	Config_TMR1(64, 250);	//genero una base tempo di 1ms
	Start_TMR1();
}

/*
void RefreshRTC(TypRTC * RTC)
{
	RTC->Secondi +=1;
	if(RTC->Secondi >59)
	{	RTC->Secondi  = 0;
		RTC->Minuti +=1;
		if(RTC->Minuti >59)
		{	RTC->Minuti = 0;
			RTC->Ore +=1;
			if(RTC->Ore > 23)
			{	RTC->Ore = 0;
				RTC->GiornoSettimana +=1;
				if(RTC->GiornoSettimana >6)
				{	RTC->GiornoSettimana = 0;
					RTC->Settimana +=1;
				}
				RTC->Giorno +=1;
				if(RTC->Giorno >29)
				{	RTC->Giorno = 0;
					RTC->Mese +=1;
					if(RTC->Mese >12)
					{	RTC->Mese = 0;
						RTC->Anno +=1;
					}
				}
			}
		}
	}
}
*/


