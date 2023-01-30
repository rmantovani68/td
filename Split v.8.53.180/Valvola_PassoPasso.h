//----------------------------------------------------------------------------------
//	Progect name:	ValvolaPassoPasso.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			22/05/2011
//	Description:	Header delle funzioni che gestiscono la valvola passo passo
//----------------------------------------------------------------------------------
#ifndef _VALVOLA_PASSO_PASSO_H_
	#define _VALVOLA_PASSO_PASSO_H_
	#include "Timer.h"
	#include "Core.h"


	//---------------------------------------
	// Define
	//---------------------------------------
	#define	OpenValve                       1       // 1 apre; 0 chiude
	#define	CloseValve                      0

    #define LowStep                         0//1    // 1=Seleziona il modo 1/4 step   0 = Seleziona 16 microstep

    #define K_Valve_Low_Speed               0
    #define K_Valve_Hi_Speed                1

    #define K_ExecValve_Cnt_LoSpeed         1
    #define K_ExecValve_Cnt_HiSpeed         0



    // K x Selezione Valori limite Valvola (LowStep o FullStep)
#if (LowStep==1)
    #define K_VMV                           2       // K Valve Multiplication Value for 1/2 step
#else
    #define K_VMV                           16//8   // K Valve Multiplication Value for 1/8 step
#endif



#if (LowStep==1)                            // 1/2 step
    #define K_ValveResetStepTime            10      // Tempo in ms tra un passo e l'altro della valvola durante la fase di reset
    #define K_ValveWorkStepTime             20      // Tempo in ms tra un passo e l'altro della valvola durante la fase di lavoro
    #define K_ValveReSyncStepTime           20      // Tempo in ms tra un passo e l'altro della valvola durante la fase di resync
#else                                       // 1/16 step
    #define K_ValveResetStepTime            1       // Tempo in ms tra un passo e l'altro della valvola durante la fase di reset        
    #define K_ValveWorkStepTime             2       // Tempo in ms tra un passo e l'altro della valvola durante la fase di lavoro
    #define K_ValveReSyncStepTime           2       // Tempo in ms tra un passo e l'altro della valvola durante la fase di resync
#endif	

	#define K_ValveDisableDriverTime        2//60		// Tempo in Secondi per disabiltazione driver valvola dall'ultimo pilotaggio
    
    #define K_WaitResyncValvola             300//10          // Tempo in Secondi per resync valvola quando in posizione chiusa
	#define K_ValveReSyncStepNumber			5 * K_VMV //20 * K_VMV      // n. passi oltre il fine corsa in posizione di chiusura valvola

    #define K_WaitDischOffsValvola          20          // Tempo in Secondi per scarico offset valvola quando in è già chiusa
	#define K_ValveDischOffStepNumber		3 * K_VMV   // n. passi che carico in fase di scarico offset valvola


    #define K_EnableReduceDriverCurrent     1//0//1       // Abilita la riduzione della corrente del driver Valvola da 444mA a 100mA attraverso il pilotaggio 
                                                    // del pin del micro "RE4" settato come uscita Open-Drain.

 
	// Soglia intervento Valvola		
	// Il primo valore dei seguenti define rappresenta la percentuale riferita al valore massimo (ValvolaAperta)
	#define	K_MaxExcursionValve				480 * K_VMV

    #define K_Offset_Valve                  20  * K_VMV//48  * K_VMV        //Offset di movimento della valvola per iniziare l'apertura
                                                       //48 dovrebbe essere valore ottimale, ma va testato bene!!
	#define	K_ValvolaAperta					K_PercMaxOpenValve //* 480L * K_VMV / 100 //210
	#define	K_ValvolaChiusa					0 //* K_ValvolaAperta / 100
	#define	K_IncOutValvole					1*K_VMV     
	#define	K_percErrValv					20//30          // Percentuale di incremento Valore "ValvolaAperta" con Errore CompressorHi o CompressorLo
	#define	K_percErrValvPress				20//40	//65
    #define	K_percErrValvLoTemp				20
	#define K_percErrUnitStandby			5	//Percentuale apertura valvola se l'unità è in stand-by pronta per ripartire
    #define	K_ValvolaOff                    0 ///*15*/ * K_ValvolaAperta / 100 //15%
	#define K_Valve_Min_Go_Off				8 //* K_ValvolaAperta / 100  // Valore minimo della valvola con unità in spegnimento

	#define K_SuperHeat_Heat_Min_Error_Open 1 //Bypasso limite valvola e lo metto a 1% per correggere errore SuperHeat in caldo.

	#define	K_ValveLostCommCool				0 //* K_ValvolaAperta / 100  //0%
    #define	K_ValvoLostCommHot              1 //* K_ValvolaAperta / 100  //1%
 
	#define K_ValveResetStepNumber			550 * K_VMV     // n. passi per reset valvola (Full Closed)
    #define K_ValveWorkStepNumber			100 //* K_ValvolaAperta / 100	//480 *K_VMV    // n. passi per lavoro valvola (Full Closed)


	#define	K_AperturaMaxCaldo				100 //* K_ValvolaAperta / 100  //210			//Apertura massima in lavoro unità
	#define	K_AperturaMinCaldo				0/*20*/ //* K_ValvolaAperta / 100	//50	//Apertura minima in lavoro unità
#if (SplitMode == K_I_Am_LoopWater)
	#define K_InitValvolaFreddo				40 //* K_ValvolaAperta / 100	//24  *K_VMV //80 (5%)
#else
    #define K_InitValvolaFreddo				0 //* K_ValvolaAperta / 100	//24  *K_VMV //80 (5%)
#endif
	#define	K_AperturaMaxFreddo				100 //* K_ValvolaAperta / 100 //210			//Apertura massima in lavoro unità
	#define	K_AperturaMinFreddo				0 //* K_ValvolaAperta / 100 //50 	//0		//Apertura minima in lavoro unità
	#define	K_ValveNoWorkMinFreddo          0 //* K_ValvolaAperta / 100	//
	#define	K_ValveNoWorkMaxFreddo          20// * K_ValvolaAperta / 100	//
    #define K_AperturaValvolaFrozen         20

	// parametri test flusso condensatore
	#define K_EEV_FluxTest					0 * K_ValvolaAperta / 100

	//iFault		// attivo basso, overcurrent, overtemp
	//iHome			// attivo basso; at home of step table

	#define VariabiliGlobaliValvolaExp()		\
		volatile TypTimer TimerValvola;			\
		volatile TypTimer TimerDisableValve;	\
		volatile unsigned char StepRefreshTime;	\
		volatile unsigned int ValvolaMaxStepNum;

	#define IncludeVariabiliGlobaliValvolaExp()			\
		extern volatile TypTimer TimerValvola;			\
		extern volatile TypTimer TimerDisableValve;		\
		extern volatile unsigned char StepRefreshTime;	\
		extern volatile unsigned int ValvolaMaxStepNum;

	//---------------------------------------
	// Variabili
	//---------------------------------------
	IncludeVariabiliGlobaliValvolaExp();
	//IncludeVariabiliGlobaliCore();

	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	void Refresh_MotSt(void);
	void Free_Run(void);
	void Enable_Run(void);
	void Disable_Run(void);	
	void InitValvola(void);
	void InitValveRangeValue(void);
    void RefreshValue(void);
	void WorkValvola(void);
	void ResetValvola(void);	
    void ReSyncValvola(void);
    void DischOffsValvola(void);
    void CalculateValveRangeValue(void);
    void OpenAllValvola(void);	
#endif
