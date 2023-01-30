//----------------------------------------------------------------------------------
//	Progect name:	Timer.c
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			Emanuele
//	Date:			04/01/2014
//	Description:	Corpo delle funzioni Timer
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
#include "ProtocolloComunicazione.h"
#include "Timer.h"
#include "EEPROM.h"

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
void Config_TMR1 (unsigned char PreScaler, int TickValue)
/*
	(FOsc/4)	=> PreScal	=> TickValue
	96Mhz		=> 16		=> 150
	24Mhz		=> 1.5Mhz	=> 10Khz
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
	IPC0bits.T1IP = K_INTPRI_T1;		// imposto la priorità dell'interrupt bassa	
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

#if (K_AbilMODBCAN==1)
void Init_Timer1(void)
#else
void Init_Timer(void)
#endif //#if (K_AbilMODBCAN==1)   
{
	Config_TMR1(64, K_TIMEBASE_T1);	//genero una base tempo di 1ms
	Start_TMR1();
}

#if (K_AbilMODBCAN==1)
void Config_TMR2 (unsigned char PreScaler, int TickValue)
/*
	(FOsc/4)	=> PreScal	=> TickValue
	96Mhz		=> 16		=> 150
	24Mhz		=> 1.5Mhz	=> 10Khz
*/
{
	T2CON = 0;		// ripulisco la word di cfg
	switch (PreScaler)
	{
		case 1:				// Setto la configurazione del PostScaler
			T2CONbits.TCKPS0 = 0;
			T2CONbits.TCKPS1 = 0;
			break;

		case 8:
			T2CONbits.TCKPS0 = 1;
			T2CONbits.TCKPS1 = 0;
			break;

		case 64:
			T2CONbits.TCKPS0 = 0;
			T2CONbits.TCKPS1 = 1;
			break;

		case 255:
			T2CONbits.TCKPS0 = 1;
			T2CONbits.TCKPS1 = 1;
			break;
	}
	TMR2 = 0;				// resetto il contatore
	PR2 = TickValue;
	_T2IF = 0;		// resetto il flag di interrupt
	_T2IP = K_INTPRI_T2;		// imposto la priorità dell'interrupt bassa	
	_T2IE = 1;		// abilito l'interrupt di TMR1
}

void Start_TMR2(void)
{
	T2CONbits.TON = 1;		// attivo il timer
}

void Stop_TMR2(void)
{
	T2CONbits.TON = 0;		// disattivo il timer
	TMR2 = 0;               // resetto il conttore
}

void Pasue_TMR2(void)
{
	T2CONbits.TON = 0;		// disattivo il timer
}

void Init_Timer2(void)
{
	Config_TMR2(64, K_TIMEBASE_T2);	//genero una base tempo di 10ms
	Start_TMR2();
}



void Config_TMR3 (unsigned char PreScaler, int TickValue)
/*
	(FOsc/4)	=> PreScal	=> TickValue
	96Mhz		=> 16		=> 150
	24Mhz		=> 1.5Mhz	=> 10Khz
*/
{
	T3CON = 0;		// ripulisco la word di cfg
	switch (PreScaler)
	{
		case 1:				// Setto la configurazione del PostScaler
			T3CONbits.TCKPS0 = 0;
			T3CONbits.TCKPS1 = 0;
			break;

		case 8:
			T3CONbits.TCKPS0 = 1;
			T3CONbits.TCKPS1 = 0;
			break;

		case 64:
			T3CONbits.TCKPS0 = 0;
			T3CONbits.TCKPS1 = 1;
			break;

		case 255:
			T3CONbits.TCKPS0 = 1;
			T3CONbits.TCKPS1 = 1;
			break;
	}
	TMR3 = 0;				// resetto il contatore
	PR3 = TickValue;
	_T3IF = 0;		// resetto il flag di interrupt
	_T3IP = K_INTPRI_T3;		// imposto la priorità dell'interrupt bassa	
	_T3IE = 1;		// abilito l'interrupt di TMR1
}

void Start_TMR3(void)
{
	T3CONbits.TON = 1;		// attivo il timer
}

void Stop_TMR3(void)
{
	T3CONbits.TON = 0;		// disattivo il timer
	TMR3 = 0;               // resetto il conttore
}

void Pasue_TMR3(void)
{
	T3CONbits.TON = 0;		// disattivo il timer
}

void Init_Timer3(void)
{
	Config_TMR3(255/*64*/, K_TIMEBASE_T3);	//genero una base tempo di xxx ms
	//Start_TMR3();
}



void Config_TMR4 (unsigned char PreScaler, int TickValue)
/*
	(FOsc/4)	=> PreScal	=> TickValue
	96Mhz		=> 16		=> 150
	24Mhz		=> 1.5Mhz	=> 10Khz
*/
{
	T4CON = 0;		// ripulisco la word di cfg
	switch (PreScaler)
	{
		case 1:				// Setto la configurazione del PostScaler
			T4CONbits.TCKPS0 = 0;
			T4CONbits.TCKPS1 = 0;
			break;

		case 8:
			T4CONbits.TCKPS0 = 1;
			T4CONbits.TCKPS1 = 0;
			break;

		case 64:
			T4CONbits.TCKPS0 = 0;
			T4CONbits.TCKPS1 = 1;
			break;

		case 255:
			T4CONbits.TCKPS0 = 1;
			T4CONbits.TCKPS1 = 1;
			break;
	}
	TMR4 = 0;				// resetto il contatore
	PR4 = TickValue;
	_T4IF = 0;		// resetto il flag di interrupt
	_T4IP = K_INTPRI_T4;		// imposto la priorità dell'interrupt bassa	
	_T4IE = 1;		// abilito l'interrupt di TMR1
}

void Start_TMR4(void)
{
	T4CONbits.TON = 1;		// attivo il timer
}

void Stop_TMR4(void)
{
	T4CONbits.TON = 0;		// disattivo il timer
	TMR4 = 0;               // resetto il conttore
}

void Pasue_TMR4(void)
{
	T4CONbits.TON = 0;		// disattivo il timer
}

void Init_Timer4(void)
{
	Config_TMR4(64, K_TIMEBASE_T4);	//genero una base tempo di 1ms
	Start_TMR4();
}



void Config_TMR5 (unsigned char PreScaler, int TickValue)
/*
	(FOsc/4)	=> PreScal	=> TickValue
	96Mhz		=> 16		=> 150
	24Mhz		=> 1.5Mhz	=> 10Khz
*/
{
	T5CON = 0;		// ripulisco la word di cfg
	switch (PreScaler)
	{
		case 1:				// Setto la configurazione del PostScaler
			T5CONbits.TCKPS0 = 0;
			T5CONbits.TCKPS1 = 0;
			break;

		case 8:
			T5CONbits.TCKPS0 = 1;
			T5CONbits.TCKPS1 = 0;
			break;

		case 64:
			T5CONbits.TCKPS0 = 0;
			T5CONbits.TCKPS1 = 1;
			break;

		case 255:
			T5CONbits.TCKPS0 = 1;
			T5CONbits.TCKPS1 = 1;
			break;
	}
	TMR5 = 0;				// resetto il contatore
	PR5 = TickValue;
	_T5IF = 0;		// resetto il flag di interrupt
	_T5IP = K_INTPRI_T5;		// imposto la priorità dell'interrupt bassa	
	_T5IE = 1;		// abilito l'interrupt di TMR1
}

void Start_TMR5(void)
{
	T5CONbits.TON = 1;		// attivo il timer
}

void Stop_TMR5(void)
{
	T5CONbits.TON = 0;		// disattivo il timer
	TMR5 = 0;               // resetto il conttore
}

void Pasue_TMR5(void)
{
	T5CONbits.TON = 0;		// disattivo il timer
}

void Init_Timer5(void)
{
	Config_TMR5(64, K_TIMEBASE_T5);	//genero una base tempo di 1ms
	Start_TMR5();
}


// Genera uno "shot" di tempo time sulla var. "out" con a partire dalla var "trigger"
void Shot(int trigger, int *out, int *cnt, int time)
{
    //if(trigger==0) 
    //    *cnt = 0;
    if(trigger>0 && *out==0)
    {
        *out = 1;
    }
    else if((*cnt)++>=time)
    {
        //if(time>0)
        *out=0;
        *cnt=0;
    }

}        


void LoadTimer(volatile TypTimer * Timer, unsigned int Time, unsigned int PresetValue,  unsigned int Enable)
{
    Timer->TimeOut=0;
    Timer->Enable=Enable;
    Timer->Value=PresetValue;
    Timer->Time=Time;
}
#endif //#if (K_AbilMODBCAN==1) 


void RefreshRTC(TypRTC * RTC)
{
	RTC->Secondi +=1;
	if(RTC->Secondi >59)
	{	RTC->Secondi  = 0;
		RTC->Minuti +=1;
		if(RTC->Minuti >59)
		{	
			RTC->Minuti = 0;			
			if(EngineBox[0].NoPayCastigationActivation==1)
			{
				if(++RTC->HoursOfLife > Room[k_Split_Master_Add].Maintenance_Set_Time) 		// Mod. v4.4.117
					RTC->HoursOfLife = Room[k_Split_Master_Add].Maintenance_Set_Time;			// Limito conteggio...
				StoreInt(RTC->HoursOfLife, E2_Addr_Hours);	// Salvo ore conteggiate in E2	
			}			
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


