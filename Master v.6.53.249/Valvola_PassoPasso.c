//----------------------------------------------------------------------------------
//	Progect name:	ValvolaPassoPasso.c
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			Emanuele
//	Date:			22/05/2011
//	Description:	Corpo delle funzioni che gestiscono la valvola passo passo
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//	Include
//----------------------------------------------------------------------------------
#include "HWSelection.h"
#include <p24FJ256GB210.h>

#include <stdio.h>
#include <libpic30.h>
#include "Delay.h"
#include "DefinePeriferiche.h"
#include "Core.h"
#include "Valvola_PassoPasso.h"

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
void ResetStepDriver(void)
{
	oRst = 0;							// set reset to reset index
	DelayuSec(20);
	oRst = 1;							// normal run
	DelayuSec(20);	
}


void Refresh_MotSt()		// aggiorno lo stato del motore
{
	//DEBUG
	int	vFault;
	
	vFault = !iFault;
	if(vFault==1)
	{
		Me.CntValveFault++;
		ResetStepDriver();
	}
	//DEBUG
	
	oStep = 1;
	DelayuSec(5);
	oStep = 0;
	DelayuSec(5);

	if(oDir)	//OpenValve
		Me.ExpValve_Act +=1;	//CurPosition +=1;
	else
		Me.ExpValve_Act -=1;	//CurPosition -=1;
	

/*
	oStep = !oStep;
	
	if(!iHome)
	{	if(oDir)	//OpenValve
			Me.ExpValve_Act +=1;	//CurPosition +=1;
		else
			Me.ExpValve_Act -=1;	//CurPosition -=1;
	}	
*/	
	
}

void Free_Run(void)		// sblocco la valvola
{	
    oEnable = 1;		// Disabilito il controller della valvola
}

void Reduce_DriverCurrent(void)		// Riduco la corrente del driver valvola
{	
    oCurrDriver = 0;        // Valve Driver Current = 100mA
}

void Enable_Run(void)	// abilito la valvola al movimento
{	
#if (K_EnableReduceDriverCurrent==1)   
    oCurrDriver = 1;        // Valve Driver Current = 450mA
#endif    
    oEnable = 0;		// Abilito il controller della valvola
	oLedEEV = 0;		// Segnalo l'attivazione della valvola con il led
}

void Disable_Run(void)	// abilito la valvola al movimento
{	
    oLedEEV = 1;		// Segnalo la disattivazione della valvola con il led
}


void ResetValvola()						// inizializzo la valvola
{
	TimerValvola.Time = K_ValveResetStepTime;		// imposto il timer per il movimento della valvola ogni 10 ms
	Me.ExpValve_Req = 0;
	Me.ExpValve_Act = K_ValveResetStepNumber;
	Me.InitValvolaInCorso = 1;
}


void ReSyncValvola()                            // Risincronizzo la valvola in posizione Closed
{
	TimerValvola.Time = K_ValveReSyncStepTime;		
	Me.ExpValve_Req = 0;
	Me.ExpValve_Act = K_ValveReSyncStepNumber;
	Me.InitValvolaInCorso = 1;    
}


void WorkValvola()									// parametri di lavoro valvola
{
	if(Me.InitValvolaInCorso==0)
        TimerValvola.Time = K_ValveWorkStepTime;		// imposto il timer per il movimento della valvola ogni 10 ms
}


void InitValvola()						// inizializzo la valvola
{
	StepRefreshTime = K_ValveWorkStepTime;   	
	ValvolaMaxStepNum = K_ValveWorkStepNumber;		
	
	TimerDisableValve.Time = K_ValveDisableDriverTime;  	
	
	
	oSleep = 1;							// low power - sleep mode
//	DelayuSec(1);
//	Enable_Run();
//	DelayuSec(1);
    
#if (LowStep==1)
	oMode0 = 1;		// 1/2 step			// 000=full step2ph; 001=1/2step; 010=1/4step
	oMode1 = 0;							// 011=8microstep; 100=16microstep;
	oMode2 = 0;							// 101=110=111=32microstep    
    
	oDecay = 1;							// low=slow; high=fast; open=mixed    
#else
	oMode0 = 0;//1;		// 1/16 step         // 000=full step2ph; 001=1/2step; 010=1/4step
	oMode1 = 0;//1;							// 011=8microstep; 100=16microstep;
	oMode2 = 1;//0;							// 101=110=111=32microstep    
    
	oDecay = 0; 						// low=slow; high=fast; open=mixed    
#endif	
    
	DelayuSec(10);
	oRst = 0;							// set reset to reset index
	DelayuSec(20);
	oRst = 1;							// normal run
	DelayuSec(20);	
	
	ResetValvola();
}


/*
void InitValvola()						// inizializzo la valvola
{
	StepRefreshTime = K_ValveWorkStepTime;   	
	ValvolaMaxStepNum = K_ValveWorkStepNumber;	
	
	oSleep = 1;							// low power - sleep mode
	DelayuSec(1);
	Enable_Run();
	DelayuSec(1);
    
#ifdef LowStep
	oMode0 = 1;		// 1/2 step			// 000=full step2ph; 001=1/2step; 010=1/4step
	oMode1 = 0;							// 011=8microstep; 100=16microstep;
	oMode2 = 0;							// 101=110=111=32microstep    
    
	oDecay = 1;							// low=slow; high=fast; open=mixed    
#else
	oMode0 = 1;		// 1/8 step         // 000=full step2ph; 001=1/2step; 010=1/4step
	oMode1 = 1;							// 011=8microstep; 100=16microstep;
	oMode2 = 0;							// 101=110=111=32microstep    
    
	oDecay = 0; 						// low=slow; high=fast; open=mixed    
#endif	
    

    
	DelayuSec(10);
	oRst = 0;							// set reset to reset index
	DelayuSec(20);
	oRst = 1;							// normal run
	DelayuSec(20);	

	oDir = CloseValve;					// 1 apre; 0 chiude
	TimerValvola.TimeOut = 0;
	TimerValvola.Time = K_ValveResetStepTime;	//2; // inc/dec STEP valvola ogni 2mS
	TimerValvola.Enable = 1;

    TimerDisableValve.Time = K_ValveDisableDriverTime;    
    
	Me.ExpValve_Act = K_ValveResetStepNumber; //DEBUG 240; // passi da effettuare in chiusura
	while(Me.ExpValve_Act > 0)			// loop fino al termine dei cicli
	{
		if(TimerValvola.TimeOut)		// se è scaduto il timer
		{
			TimerValvola.TimeOut = 0;	// resetto il time out
			Refresh_MotSt();
		}
	}
	TimerValvola.Enable = 0;			// disattivo il timer
	oStep = 0;
	Me.ExpValve_Act = 0;				//CurPosition	= 0;
	Me.ExpValve_Req = 0;				//SetValue = 0;
	//Free_Run();
    Disable_Run();                              // Spengo LED Giallo Valvola    
}
*/

/*
void RefreshValue()						// aggiorna la valvola
{	int Delta = 0;						// variabile con la differenza tra la posizione attuale e quella desiderata

	if(Me.ExpValve_Req > ValvolaMaxStepNum)		// verifico che il numero di passo richiesto sia inferiore al massimo
		Me.ExpValve_Req = ValvolaMaxStepNum;	// nel caso imposto il valore al massimo consentito

	if(Me.ExpValve_Req > Me.ExpValve_Act)			// se devo aprire la valvola
	{	Delta = Me.ExpValve_Req - Me.ExpValve_Act;	// mi calcolo il delta
		oDir = OpenValve;							// setto la direzione della valvola in apertura
	}
	else											// altrimenti se devo chiudere
	{	Delta = Me.ExpValve_Act - Me.ExpValve_Req;	// mi calcolo il delta invertendo i valori
		oDir = CloseValve;							// setto la direzione della valvola in chiusura
	}
	if(Delta > 0)
	{
		TimerValvola.Time = StepRefreshTime;		// imposto il timer per il movimento della valvola ogni 10 ms
		TimerValvola.Enable = 1;					// attivo il timer per il movimento della valvola se serve
		Enable_Run();                               // Abilito valvola e LED Giallo
		while(Me.ExpValve_Req != Me.ExpValve_Act)	// luppo fino al raggiungimento della posizione corretta
		{
			if(TimerValvola.TimeOut)				// attendo il time out del timer per aggirnare la posizione
			{	TimerValvola.TimeOut = 0;			// resetto lo stato di time out
				Refresh_MotSt();					// genero l'impulso per i passi e aggiorno il ctrl
			}
		}
		//Free_Run();
        Disable_Run();                              // Spengo LED Giallo Valvola
        TimerDisableValve.Enable = 1;					// attivo il timer per la disattivazione del driver valvola
        TimerDisableValve.TimeOut = 0;                  // resetto lo stato di time out
        TimerDisableValve.Value = 0;					// e resetto conteggio
	}
	TimerValvola.Enable = 0;						// fermo il timer al termine dell'operazione
    
    if(TimerDisableValve.TimeOut)
    {
        TimerDisableValve.TimeOut = 0;              // resetto lo stato di time out
        Free_Run();                                 // Disabilito driver valvola
        TimerDisableValve.Enable = 0;				// disattivo il timer 
    }

}
 */

void RefreshValue()						// aggiorna la valvola
{	
	int Delta = 0;						// variabile con la differenza tra la posizione attuale e quella desiderata

	if(Me.ExpValve_Req > ValvolaMaxStepNum)		// verifico che il numero di passo richiesto sia inferiore al massimo
		Me.ExpValve_Req = ValvolaMaxStepNum;	// nel caso imposto il valore al massimo consentito

	if(Me.ExpValve_Req > Me.ExpValve_Act)			// se devo aprire la valvola
	{	
		Delta = Me.ExpValve_Req - Me.ExpValve_Act;	// mi calcolo il delta
		oDir = OpenValve;							// setto la direzione della valvola in apertura
	}
	else											// altrimenti se devo chiudere
	{	
		Delta = Me.ExpValve_Act - Me.ExpValve_Req;	// mi calcolo il delta invertendo i valori
		oDir = CloseValve;							// setto la direzione della valvola in chiusura
	}
	if(Delta > 0)
	{
		//TimerValvola.Time = StepRefreshTime;		// imposto il timer per il movimento della valvola ogni 10 ms
		TimerValvola.Enable = 1;					// attivo il timer per il movimento della valvola se serve
		Enable_Run();                               // Abilito valvola e LED Giallo
		
		//if(Me.ExpValve_Req != Me.ExpValve_Act)	// luppo fino al raggiungimento della posizione corretta
		//{
		if(TimerValvola.TimeOut)				// attendo il time out del timer per aggirnare la posizione
		{	
			TimerValvola.TimeOut = 0;			// resetto lo stato di time out
			Refresh_MotSt();					// genero l'impulso per i passi e aggiorno il ctrl
		}
		//}
		//Free_Run();
        TimerDisableValve.Enable = 1;					// attivo il timer per la disattivazione del driver valvola
        TimerDisableValve.TimeOut = 0;                  // resetto lo stato di time out
        TimerDisableValve.Value = 0;					// e resetto conteggio		
 
	}
	else
	{
		Disable_Run();                              // Spengo LED Giallo Valvola
//        TimerDisableValve.Enable = 1;					// attivo il timer per la disattivazione del driver valvola
//        TimerDisableValve.TimeOut = 0;                  // resetto lo stato di time out
//        TimerDisableValve.Value = 0;					// e resetto conteggio		
		TimerValvola.Enable = 0;						// fermo il timer al termine dell'operazione
    }
	
	
    if(TimerDisableValve.TimeOut)
    {
        TimerDisableValve.TimeOut = 0;              // resetto lo stato di time out
#if (K_EnableReduceDriverCurrent==1)        
        Reduce_DriverCurrent();        
#else
        Free_Run();                                 // Disabilito driver valvola
#endif
        TimerDisableValve.Enable = 0;				// disattivo il timer 
    }
	
	if(Me.InitValvolaInCorso == 1 && Delta==0)
		Me.InitValvolaInCorso = 0;

}
