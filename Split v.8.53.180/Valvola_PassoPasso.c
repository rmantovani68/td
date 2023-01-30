//----------------------------------------------------------------------------------
//	Progect name:	ValvolaPassoPasso.c
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			22/05/2011
//	Description:	Corpo delle funzioni che gestiscono la valvola passo passo
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
	DelayuSec(40);
	oRst = 1;							// normal run
	DelayuSec(40);	
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
	DelayuSec(10);
	oStep = 0;
	DelayuSec(10);

	if(oDir)	//OpenValve
		Me.ExpValve_Delta_Act +=1;	//CurPosition +=1;
	else
		Me.ExpValve_Delta_Act -=1;	//CurPosition -=1;
    
    if(Me.ExpValve_Delta_Act>K_Offset_Valve)    //Carico valore di Me.ExpValve_Act SOLO se la reale compensata con il delta è > del delta. 
        Me.ExpValve_Act = Me.ExpValve_Delta_Act-K_Offset_Valve;
    else                                        //Altrimenti l'Me.ExpValve_Act vale 0.
        Me.ExpValve_Act = 0;
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
	oLedEEV = LED_ON;		// Segnalo l'attivazione della valvola con il led
}

void Disable_Run(void)	// abilito la valvola al movimento
{	
    oLedEEV = LED_OFF;		// Segnalo la disattivazione della valvola con il led
}


void ResetValvola()						// inizializzo la valvola tutta chiusa
{
	Me.ExpValve_Req = 0;
	Me.ExpValve_Delta_Act = K_ValveResetStepNumber;
	Me.InitValvolaInCorso = 1;                
}

void OpenAllValvola()						// inizializzo la valvola tutta aperta
{
	Me.ExpValve_Req = Me.ValveLim.ValvolaAperta;
	Me.InitValvolaInCorso = 1;
}

void ReSyncValvola()                            // Risincronizzo la valvola in posizione Closed
{
	Me.ExpValve_Req = 0;
    Me.ExpValve_Delta_Act = K_ValveReSyncStepNumber;
	Me.InitValvolaInCorso = 1;    
}

void DischOffsValvola()                            // Scarico pian piano l'offset valvola fino a richiusura completa
{
	Me.ExpValve_Req = 0;
    //caso mai nel collaudo bisognerà scaricare tutto l'offset in un giro!! se addr.0 --> Me.ExpValve_Delta_Req = Me.ExpValve_Delta_Req-K_Offset_Valve. 
    //In questo caso, nella routine che la richiama, bisogna abbassare il tempo a zero!!
    if((EngineBox.RealFuncMode==EngineBox_GoOff)&&(EngineBox.Compressor_Speed==0) && (EngineBox.Compressor_Speed_C2==0))
        Me.ExpValve_Delta_Req = 0;
    else
        Me.ExpValve_Delta_Req = Me.ExpValve_Delta_Req-K_ValveDischOffStepNumber;
    if(Me.ExpValve_Delta_Req<0)
        Me.ExpValve_Delta_Req=0;
	Me.InitValvolaInCorso = 1;    
}
/*
void WorkValvola()									// parametri di lavoro valvola
{
	if(Me.InitValvolaInCorso==0)
    {
        // ARGOVALVOLATIMER
        //TimerValvola.Time = K_ValveWorkStepTime;		// imposto il timer per il movimento della valvola ogni 10 ms
        //Me.ExpValve_Speed_Mode = K_Valve_Low_Speed;        
    }
}
*/

void InitValveRangeValue(void)
{
    Me.ValveLim.MasterRx.PercMaxOpenValve    = K_PercMaxOpenValve;    
    Me.ValveLim.MasterRx.ValvolaAperta		= K_ValvolaAperta;			
    Me.ValveLim.MasterRx.ValvolaChiusa		= K_ValvolaChiusa;			
    Me.ValveLim.MasterRx.ValvolaOff          = K_ValvolaOff;            
    Me.ValveLim.MasterRx.Valve_Min_Go_Off	= K_Valve_Min_Go_Off;		
    Me.ValveLim.MasterRx.ValveLostCommCool	= K_ValveLostCommCool;		
    Me.ValveLim.MasterRx.ValvoLostCommHot    = K_ValvoLostCommHot;                      
    Me.ValveLim.MasterRx.AperturaMaxCaldo_Local	= K_AperturaMaxCaldo;	
    Me.ValveLim.MasterRx.AperturaMaxCaldo = K_AperturaMaxCaldo;
    Me.ValveLim.MasterRx.AperturaMinCaldo_Local	= K_AperturaMinCaldo;		
    Me.ValveLim.MasterRx.InitValvolaFreddo	= K_InitValvolaFreddo;		
    Me.ValveLim.MasterRx.AperturaMaxFreddo_Local	= K_AperturaMaxFreddo;		
    Me.ValveLim.MasterRx.AperturaMinFreddo_Local	= K_AperturaMinFreddo;		
    Me.ValveLim.MasterRx.ValveNoWorkMinFreddo= K_ValveNoWorkMinFreddo;  
    Me.ValveLim.MasterRx.ValveNoWorkMaxFreddo= K_ValveNoWorkMaxFreddo;
    Me.ValveLim.EEV_FluxTest        = K_EEV_FluxTest;
    Me.ValveLim.MasterRx.ValvolaFrozen = K_AperturaValvolaFrozen;
}


void CalculateValveRangeValue(void)
{
#if(SplitMode == K_I_Am_Frigo)    
    if(EngineBox.TestAllAbil.MaxOpValve_Frigo)
        Me.ValveLim.MasterRx.AperturaMaxFreddo = EngineBox.TestAll_MaxOpValve_Frigo;
    else
        Me.ValveLim.MasterRx.AperturaMaxFreddo = Me.ValveLim.MasterRx.AperturaMaxFreddo_Local;
    if(EngineBox.TestAllAbil.MinOpValve_Frigo)
        Me.ValveLim.MasterRx.AperturaMinFreddo = EngineBox.TestAll_MinOpValve_Frigo;
    else
        Me.ValveLim.MasterRx.AperturaMinFreddo = Me.ValveLim.MasterRx.AperturaMinFreddo_Local;
#elif(SplitMode == K_I_Am_Freezer)    
    if(EngineBox.TestAllAbil.MaxOpValve_Freezer)
        Me.ValveLim.MasterRx.AperturaMaxFreddo = EngineBox.TestAll_MaxOpValve_Freezer;
    else
        Me.ValveLim.MasterRx.AperturaMaxFreddo = Me.ValveLim.MasterRx.AperturaMaxFreddo_Local;
    if(EngineBox.TestAllAbil.MinOpValve_Freezer)
        Me.ValveLim.MasterRx.AperturaMinFreddo = EngineBox.TestAll_MinOpValve_Freezer;
    else
        Me.ValveLim.MasterRx.AperturaMinFreddo = Me.ValveLim.MasterRx.AperturaMinFreddo_Local; 
#elif(SplitMode == K_I_Am_LoopWater)    
    if(EngineBox.TestAllAbil.MaxOpValve_Freezer)
        Me.ValveLim.MasterRx.AperturaMaxFreddo = EngineBox.TestAll_MaxOpValve_Freezer;
    else
        Me.ValveLim.MasterRx.AperturaMaxFreddo = Me.ValveLim.MasterRx.AperturaMaxFreddo_Local;
    if(EngineBox.TestAllAbil.MinOpValve_Freezer)
        Me.ValveLim.MasterRx.AperturaMinFreddo = EngineBox.TestAll_MinOpValve_Freezer;
    else
        Me.ValveLim.MasterRx.AperturaMinFreddo = Me.ValveLim.MasterRx.AperturaMinFreddo_Local;     
#else
    
#if (K_DisAbil_TestAll_MaxValve==1)
        Me.ValveLim.MasterRx.AperturaMaxFreddo = Me.ValveLim.MasterRx.AperturaMaxFreddo_Local;  
        Me.ValveLim.MasterRx.AperturaMinFreddo = Me.ValveLim.MasterRx.AperturaMinFreddo_Local;
#else    
    if(EngineBox.TestAllAbil.MaxOpValve_Cool)
        Me.ValveLim.MasterRx.AperturaMaxFreddo = EngineBox.TestAll_MaxOpValve_Cool;
    else
        Me.ValveLim.MasterRx.AperturaMaxFreddo = Me.ValveLim.MasterRx.AperturaMaxFreddo_Local;        
    if(EngineBox.TestAllAbil.MinOpValve_Cool)
        Me.ValveLim.MasterRx.AperturaMinFreddo = EngineBox.TestAll_MinOpValve_Cool;
    else
        Me.ValveLim.MasterRx.AperturaMinFreddo = Me.ValveLim.MasterRx.AperturaMinFreddo_Local;
#endif        
    if(DiagnosticSplit.SuperheatRiscaldamento>=EngineBox.SuperHeat_Heat_Max_SetPoint || !EngineBox.TestAllAbil.SuperHeat_Heat_Correct)
    {
        if(EngineBox.TestAllAbil.MinOpValve_Heat)
            Me.ValveLim.MasterRx.AperturaMinCaldo = EngineBox.TestAll_MinOpValve_Heat;
        else
            Me.ValveLim.MasterRx.AperturaMinCaldo = Me.ValveLim.MasterRx.AperturaMinCaldo_Local;     
        
        Incrementa(&Me.ValveLim.MasterRx.AperturaMaxCaldo, 2, Me.ValveLim.MasterRx.AperturaMaxCaldo_Local);
    }
    else
    {
        if(DiagnosticSplit.SuperheatRiscaldamento<EngineBox.SuperHeat_Heat_Min_SetPoint)    
        {
            Decrementa(&Me.ValveLim.MasterRx.AperturaMinCaldo, 2, K_SuperHeat_Heat_Min_Error_Open);    
            Decrementa(&Me.ValveLim.MasterRx.AperturaMaxCaldo, 1, Me.ValveLim.MasterRx.AperturaMinCaldo*2);
        }
        else if((DiagnosticSplit.SuperheatRiscaldamento<EngineBox.SuperHeat_Heat_Max_SetPoint)/* && (DiagnosticSplit.SuperheatRiscaldamento>EngineBox.SuperHeat_Heat_Min_SetPoint)*/)
        {
            if(EngineBox.TestAllAbil.MinOpValve_Heat)
                Me.ValveLim.MasterRx.AperturaMinCaldo = EngineBox.TestAll_MinOpValve_Heat;
            else
                Me.ValveLim.MasterRx.AperturaMinCaldo = Me.ValveLim.MasterRx.AperturaMinCaldo_Local;     
            Decrementa(&Me.ValveLim.MasterRx.AperturaMaxCaldo, 1, Me.ValveLim.MasterRx.AperturaMinCaldo*2);
        }
//        else
//            Incrementa(&Me.ValveLim.MasterRx.AperturaMaxCaldo, 2, Me.ValveLim.MasterRx.AperturaMaxCaldo_Local);
    }
#endif
        
        
    Me.ValveLim.PercMaxOpenValve    = Me.ValveLim.MasterRx.PercMaxOpenValve;                                        // K_PercMaxOpenValve
    Me.ValveLim.ValvolaAperta		= (unsigned int)((float)Me.ValveLim.PercMaxOpenValve  * 480L * K_VMV / 100.0);                           // K_ValvolaAperta		K_PercMaxOpenValve * 480L * K_VMV / 100 //210	
    Me.ValveLim.ValvolaChiusa		= (unsigned int)((float)Me.ValveLim.MasterRx.ValvolaChiusa * (float)Me.ValveLim.ValvolaAperta / 100.0);         // K_ValvolaChiusa		0 * K_ValvolaAperta / 100
    Me.ValveLim.ValvolaOff          = (unsigned int)((float)Me.ValveLim.MasterRx.ValvolaOff * (float)Me.ValveLim.ValvolaAperta / 100.0);            // K_ValvolaOff         0 * K_ValvolaAperta / 100 
    Me.ValveLim.Valve_Min_Go_Off	= (unsigned int)((float)Me.ValveLim.MasterRx.Valve_Min_Go_Off * (float)Me.ValveLim.ValvolaAperta / 100.0);      // K_Valve_Min_Go_Off	8 * K_ValvolaAperta / 100  // Valore minimo della valvola con unità in spegnimento
    Me.ValveLim.ValveLostCommCool	= (unsigned int)((float)Me.ValveLim.MasterRx.ValveLostCommCool * (float)Me.ValveLim.ValvolaAperta / 100.0);     // K_ValveLostCommCool	0 * K_ValvolaAperta / 100  //0%		
    Me.ValveLim.ValvoLostCommHot    = (unsigned int)((float)Me.ValveLim.MasterRx.ValvoLostCommHot * (float)Me.ValveLim.ValvolaAperta / 100.0);      // K_ValvoLostCommHot   1 * K_ValvolaAperta / 100  //1%
    Me.ValveLim.AperturaMaxCaldo	= (unsigned int)((float)Me.ValveLim.MasterRx.AperturaMaxCaldo * (float)Me.ValveLim.ValvolaAperta / 100.0);      // K_AperturaMaxCaldo	100 * K_ValvolaAperta / 100  //210			//Apertura massima in lavoro unità
    Me.ValveLim.AperturaMinCaldo	= (unsigned int)((float)Me.ValveLim.MasterRx.AperturaMinCaldo * (float)Me.ValveLim.ValvolaAperta / 100.0);      // K_AperturaMinCaldo	20 * K_ValvolaAperta / 100	//50	//Apertura minima in lavoro unità
    Me.ValveLim.InitValvolaFreddo	= (unsigned int)((float)Me.ValveLim.MasterRx.InitValvolaFreddo * (float)Me.ValveLim.ValvolaAperta / 100.0);     // K_InitValvolaFreddo	0 * K_ValvolaAperta / 100	//24  *K_VMV //80 (5%)
    Me.ValveLim.AperturaMaxFreddo	= (unsigned int)((float)Me.ValveLim.MasterRx.AperturaMaxFreddo * (float)Me.ValveLim.ValvolaAperta / 100.0);     // K_AperturaMaxFreddo	100 * K_ValvolaAperta / 100 //210			//Apertura massima in lavoro unità	
    Me.ValveLim.AperturaMinFreddo	= (unsigned int)((float)Me.ValveLim.MasterRx.AperturaMinFreddo * (float)Me.ValveLim.ValvolaAperta / 100.0);     // K_AperturaMinFreddo	0 * K_ValvolaAperta / 100 //50 	//0		//Apertura minima in lavoro unità
    Me.ValveLim.ValveNoWorkMinFreddo= (unsigned int)((float)Me.ValveLim.MasterRx.ValveNoWorkMinFreddo * (float)Me.ValveLim.ValvolaAperta / 100.0);  // K_ValveNoWorkMinFreddo   0 * K_ValvolaAperta / 100	//
    Me.ValveLim.ValveNoWorkMaxFreddo= (unsigned int)((float)Me.ValveLim.MasterRx.ValveNoWorkMaxFreddo * (float)Me.ValveLim.ValvolaAperta / 100.0);  // K_ValveNoWorkMaxFreddo   20 * K_ValvolaAperta / 100
    Me.ValveLim.EEV_FluxTest        = (unsigned int)((0.0 * (float)Me.ValveLim.ValvolaAperta / 100.0));                                        // K_EEV_FluxTest           100 * K_ValvolaAperta / 100  
    Me.ValveLim.ValvolaFrozen       = (unsigned int)((float)Me.ValveLim.MasterRx.ValvolaFrozen * (float)Me.ValveLim.ValvolaAperta / 100.0);  // K_ValveNoWorkMaxFreddo   20 * K_ValvolaAperta / 100
    
    ValvolaMaxStepNum = Me.ValveLim.ValvolaAperta;
}



void InitValvola()						// inizializzo la valvola
{
    InitValveRangeValue();
        
	StepRefreshTime = K_ValveWorkStepTime;   	
	ValvolaMaxStepNum = K_ValveWorkStepNumber;		
	
	TimerDisableValve.Time = K_ValveDisableDriverTime;  	
	
	
	oSleep = 1;							// low power - sleep mode
//	DelayuSec(2);
//	Enable_Run();
//	DelayuSec(2);
    
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
    
	DelayuSec(20);
	oRst = 0;							// set reset to reset index
	DelayuSec(40);
	oRst = 1;							// normal run
	DelayuSec(40);	
	
	ResetValvola();
}


/*
void InitValvola()						// inizializzo la valvola
{
	StepRefreshTime = K_ValveWorkStepTime;   	
	ValvolaMaxStepNum = K_ValveWorkStepNumber;	
	
	oSleep = 1;							// low power - sleep mode
	DelayuSec(2);
	Enable_Run();
	DelayuSec(2);
    
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
    

    
	DelayuSec(20);
	oRst = 0;							// set reset to reset index
	DelayuSec(40);
	oRst = 1;							// normal run
	DelayuSec(40);	

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

void RefreshValue(void)						// aggiorna la valvola
{	
    //ArgoValvola
    static int DutyCnt = 0;
#define K_MaxDutyCnt     2 //0//1//2//3       // (es. 3-> 1 passo SI e 3 NO)    3 = 25%     2 = 33%      1 = 50%     0 = 100%
                                              //  25% Work Duty -> 15Hz step    Tempo Open/Close = 31,2 Sec -> 480 x 16 step (7680)
                                              //  33% Work Duty -> 20Hz step    Tempo Open/Close = 23,4 Sec -> 480 x 16 step (7680)
                                              //  50% Work Duty -> 30Hz step    Tempo Open/Close = 15,6 Sec -> 480 x 16 step (7680)
                                              // 100% Work Duty -> 60Hz step    Tempo Open/Close = 7,8 Sec -> 480 x 16 step (7680)
    //ArgoValvola
    
	int Delta = 0;						// variabile con la differenza tra la posizione attuale e quella desiderata

	if(Me.ExpValve_Req > ValvolaMaxStepNum)		// verifico che il numero di passo richiesto sia inferiore al massimo
		Me.ExpValve_Req = ValvolaMaxStepNum;	// nel caso imposto il valore al massimo consentito

	if(Me.ExpValve_Req < Me.ValveLim.ValvolaChiusa)		// verifico che il numero di passo richiesto sia superiore al minimo
		Me.ExpValve_Req = Me.ValveLim.ValvolaChiusa;	// nel caso imposto il valore al minimo consentito

    if(Me.ExpValve_Req>0)   //Se request > 0 carico nel delta request, altrimenti non faccio nulla perchè il ReSyncValvola() si occuperà di tirar giù la Me.ExpValve_Delta_Req con il tempo giusto
        Me.ExpValve_Delta_Req = Me.ExpValve_Req+K_Offset_Valve;
    else if(Me.ExpValve_Req==0 && Me.ExpValve_Delta_Req>K_Offset_Valve)   //Se request > 0 carico nel delta request, altrimenti non faccio nulla perchè il ReSyncValvola() si occuperà di tirar giù la Me.ExpValve_Delta_Req con il tempo giusto
        Me.ExpValve_Delta_Req=K_Offset_Valve;

	if(Me.ExpValve_Delta_Req > Me.ExpValve_Delta_Act)			// se devo aprire la valvola
	{	
		Delta = Me.ExpValve_Delta_Req - Me.ExpValve_Delta_Act;	// mi calcolo il delta
		oDir = OpenValve;							// setto la direzione della valvola in apertura
	}
	else											// altrimenti se devo chiudere
	{	
		Delta = Me.ExpValve_Delta_Act - Me.ExpValve_Delta_Req;	// mi calcolo il delta invertendo i valori
		oDir = CloseValve;							// setto la direzione della valvola in chiusura
	}    
    

#if (K_EnableReduceDriverCurrent==1)   
    if(Me.ExpValve_Speed_Mode==K_Valve_Low_Speed)
    {
        if(DutyCnt > 0)
        {
            Reduce_DriverCurrent();         // Valve Driver Current = 100mA              
            if(DutyCnt++>K_MaxDutyCnt)
                DutyCnt=0;
            else
                return;
        }
    }
#endif                      

    
	if(Delta > 0)
	{
                
        Enable_Run();                               // Abilito valvola e LED Giallo            
        Refresh_MotSt();					// genero l'impulso per i passi e aggiorno il ctrl
        DutyCnt=1;

        TimerDisableValve.Enable = 1;					// attivo il timer per la disattivazione del driver valvola
        TimerDisableValve.TimeOut = 0;                  // resetto lo stato di time out
        TimerDisableValve.Value = 0;					// e resetto conteggio		
	}
	else
	{
		Disable_Run();                              // Spengo LED Giallo Valvola
        
#if (K_EnableReduceDriverCurrent==1)   
    if(Me.ExpValve_Speed_Mode==K_Valve_Hi_Speed)
        Reduce_DriverCurrent();         // Valve Driver Current = 100mA              
#endif            
    }
	
	
    //if(TimerDisableValve.TimeOut)
    //ArgoValvola
#if (K_EnableReduceDriverCurrent==0)        
    if(TimerDisableValve.TimeOut==1)
    //ArgoValvola        
    {
        Free_Run();                                 // Disabilito driver valvola

        TimerDisableValve.TimeOut = 0;              // resetto lo stato di time out
        TimerDisableValve.Enable = 0;				// disattivo il timer 
    }
#endif
	
	if(Me.InitValvolaInCorso == 1 && Delta==0)
    {
		Me.InitValvolaInCorso = 0;
    }

}
