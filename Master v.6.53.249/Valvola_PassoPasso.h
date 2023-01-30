//----------------------------------------------------------------------------------
//	Progect name:	ValvolaPassoPasso.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			Emanuele
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
    
    #define K_WaitResyncValvola             10          // Tempo in Secondi per resync valvola quando in posizione chiusa
	#define K_ValveReSyncStepNumber			20 * K_VMV      // n. passi oltre il fine corsa in posizione di chiusura valvola

    #define K_EnableReduceDriverCurrent     0//1       // Abilita la riduzione della corrente del driver Valvola da 444mA a 100mA attraverso il pilotaggio 
                                                    // del pin del micro "RE4" settato come uscita Open-Drain.

 
	// Soglia intervento Valvola		
	// Il primo valore dei seguenti define rappresenta la percentuale riferita al valore massimo (ValvolaAperta)
	#define	MaxExcursionValve				480 * K_VMV

	#define	ValvolaAperta					PercMaxOpenValve * 480L * K_VMV / 100 //210
	#define	ValvolaChiusa					0 * ValvolaAperta / 100
	#define	IncOutValvole					1*K_VMV     
	#define	K_percErrValv					30          // Percentuale di incremento Valore "ValvolaAperta" con Errore CompressorHi o CompressorLo
	#define	K_percErrValvPress				40	//65
	//#define	K_ValveGoOff					240 *K_VMV 
    #define	K_ValvolaOff                    15 * ValvolaAperta / 100 //15%
	#define KValve_Min_Go_Off				8 * ValvolaAperta / 100  // Valore minimo della valvola con unità in spegnimento

	#define	K_ValveLostCommCool				0 * ValvolaAperta / 100  //0%
    #define	K_ValvoLostCommHot              1 * ValvolaAperta / 100  //1%
 
	#define K_ValveResetStepNumber			550 * K_VMV     // n. passi per reset valvola (Full Closed)
    #define K_ValveWorkStepNumber			100 * ValvolaAperta / 100	//480 *K_VMV    // n. passi per lavoro valvola (Full Closed)


	//#define InitValvolaCaldo				90  *K_VMV
	//#define ReInitValvolaCaldo				90  *K_VMV  // Chiedere a Mauri se va bene
	//#define	SogliaDeltaValvoleCaldo			50  *K_VMV
	//#define	ValvolaChiusaEngBoxCaldo		40  *K_VMV	//20	//Apertura se box acceso ma unità spenta
	#define	AperturaMaxCaldo				100 * ValvolaAperta / 100  //210			//Apertura massima in lavoro unità
	//#define	AperturaMidCaldo				168 *K_VMV	//100		//Apertura media in lavoro unità
	#define	AperturaMinCaldo				20 * ValvolaAperta / 100	//50	//Apertura minima in lavoro unità
	//#define	K_ValveNoWorkMinCaldo           24  *K_VMV	//
	//#define	K_ValveNoWorkMaxCaldo           96  *K_VMV	//

	#define InitValvolaFreddo				0 * ValvolaAperta / 100	//24  *K_VMV //80 (5%)
	//#define ReInitValvolaFreddo				50  *K_VMV
	//#define	SogliaDeltaValvoleFreddo		50   *K_VMV
	#define	AperturaMaxFreddo				100 * ValvolaAperta / 100 //210			//Apertura massima in lavoro unità
	//#define	AperturaMidFreddo				90  *K_VMV			//Apertura media in lavoro unità
	#define	AperturaMinFreddo				0 * ValvolaAperta / 100 //50 	//0		//Apertura minima in lavoro unità
	#define	K_ValveNoWorkMinFreddo          0 * ValvolaAperta / 100	//
	#define	K_ValveNoWorkMaxFreddo          20 * ValvolaAperta / 100	//


	//iFault		// attivo basso, overcurrent, overtemp
	//iHome			// attivo basso; at home of step table

	#define VariabiliGlobaliValvolaExp()		\
		volatile TypTimer TimerValvola;			\
		volatile TypTimer TimerDisableValve;	\
		volatile unsigned char StepRefreshTime;	\
		volatile unsigned int ValvolaMaxStepNum

	#define IncludeVariabiliGlobaliValvolaExp()			\
		extern volatile TypTimer TimerValvola;			\
		extern volatile TypTimer TimerDisableValve;		\
		extern volatile unsigned char StepRefreshTime;	\
		extern volatile unsigned int ValvolaMaxStepNum

	//---------------------------------------
	// Variabili
	//---------------------------------------
	IncludeVariabiliGlobaliValvolaExp();
	IncludeVariabiliGlobaliCore();

	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	void Refresh_MotSt(void);
	void Free_Run(void);
	void Enable_Run(void);
	void Disable_Run(void);	
	void InitValvola(void);
	void RefreshValue(void);
	void WorkValvola(void);
	void ResetValvola(void);	
    void ReSyncValvola(void);
#endif
