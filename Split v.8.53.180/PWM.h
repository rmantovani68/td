//----------------------------------------------------------------------------------
//	Progect name:	Triac.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			22/05/2011
//	Description:	Header per il modulo di gestione del triac
//----------------------------------------------------------------------------------
#ifndef _PWM_H_
	#define _PWM_H_

	#include "Timer.h"
	#include "FWSelection.h"

	//---------------------------------------
	// Define
	//---------------------------------------
	#define	PreScaler_1		0
	#define	PreScaler_2		1
	#define	PreScaler_4		2
	#define	PreScaler_8		3

/*
	#define	CoreOff					0
	#define	CoreRaffrescamento		0x0008
	#define	CoreRiscaldamento		0x0004
	#define CoreVentilazione		0x0010
*/
	#define TickRefreshVentil		3
/*
	#define K_FanSpeedOff           0           // 0
	#define K_FanSpeedNight			45          // 720..780
	#define K_FanSpeed1				55      	// 840
	#define K_FanSpeed2				70          // 1020..1080
	#define K_FanSpeed3				85          // 1140..1200
	#define K_FanSpeed4				110         // 1380..1440 
	#define K_FanSpeed5				140         // 1620..1680
	#define K_FanSpeed6				190         // 1860..1920
	#define K_FanSpeed7				255         // 2820..2880
*/
//
	#define K_FanSpeedOff           0           // 0

	#define K_EBM_FanSpeedNight		45          // 720..780
	#define K_EBM_FanSpeed1			55      	// 840
	#define K_EBM_FanSpeed2			70          // 1020..1080
	#define K_EBM_FanSpeed3			85          // 1140..1200
	#define K_EBM_FanSpeed4			110         // 1380..1440 
	#define K_EBM_FanSpeed5			140         // 1620..1680
	#define K_EBM_FanSpeed6			190         // 1860..1920
	#define K_EBM_FanSpeed7			255         // 2820..2880

	#define K_GR22_FanSpeedNight	43          // 360
	#define K_GR22_FanSpeed1		71      	// 720
	#define K_GR22_FanSpeed2		92          // 1020
	#define K_GR22_FanSpeed3		120         // 1260
	#define K_GR22_FanSpeed4		148         // 1620 
	#define K_GR22_FanSpeed5		179         // 1980
	#define K_GR22_FanSpeed6		200         // 2250
	#define K_GR22_FanSpeed7		255         // 2340

	#define K_GR25_FanSpeedNight	43          // 360
	#define K_GR25_FanSpeed1		71      	// 720
	#define K_GR25_FanSpeed2		92          // 1020
	#define K_GR25_FanSpeed3		120         // 1260
	#define K_GR25_FanSpeed4		148         // 1620 
	#define K_GR25_FanSpeed5		179         // 1980
	#define K_GR25_FanSpeed6		200         // 2250
	#define K_GR25_FanSpeed7		255         // 2340

    #define K_GRMod_FanSpeedNight	43          // 360
	#define K_GRMod_FanSpeed1		71      	// 720
	#define K_GRMod_FanSpeed2		92          // 1020
	#define K_GRMod_FanSpeed3		120         // 1260
	#define K_GRMod_FanSpeed4		148         // 1620 
	#define K_GRMod_FanSpeed5		179         // 1980
	#define K_GRMod_FanSpeed6		200         // 2250
	#define K_GRMod_FanSpeed7		255         // 2340

    #define K_FanSpeedAuto			256         // Valore per calcolo della velocità ventilatore in modo auto
#if(K_InverterMax35HZ==1)
	#define K_InverterFanSpeedOff           0           // 0
	#define K_InverterFanSpeedNight			1000//1500      // 720..780
	#define K_InverterFanSpeed1				1200//2000		// 840
	#define K_InverterFanSpeed2				1500//2500		// 1020..1080
	#define K_InverterFanSpeed3				2000//3000		// 1140..1200
	#define K_InverterFanSpeed4				2200//3500      // 1380..1440 
	#define K_InverterFanSpeed5				2500//4000		// 1620..1680
	#define K_InverterFanSpeed6				3000//4500		// 1860..1920
	#define K_InverterFanSpeed7				3500//5000      // 2220..2280
	#define K_InverterFanSpeedAuto			3500//5000		// Valore per calcolo della velocità ventilatore in modo auto
#elif(K_InverterMax65HZ==1)
	#define K_InverterFanSpeedOff           0           // 0
	#define K_InverterFanSpeedNight			3000      // 720..780
	#define K_InverterFanSpeed1				3500		// 840
	#define K_InverterFanSpeed2				4000		// 1020..1080
	#define K_InverterFanSpeed3				4500		// 1140..1200
	#define K_InverterFanSpeed4				5000      // 1380..1440 
	#define K_InverterFanSpeed5				5500		// 1620..1680
	#define K_InverterFanSpeed6				6000		// 1860..1920
	#define K_InverterFanSpeed7				6500      // 2220..2280
	#define K_InverterFanSpeedAuto			6500		// Valore per calcolo della velocità ventilatore in modo auto
#else 
	#define K_InverterFanSpeedOff           0           // 0
	#define K_InverterFanSpeedNight			1500      // 720..780
	#define K_InverterFanSpeed1				2000		// 840
	#define K_InverterFanSpeed2				2500		// 1020..1080
	#define K_InverterFanSpeed3				3000		// 1140..1200
	#define K_InverterFanSpeed4				3500      // 1380..1440 
	#define K_InverterFanSpeed5				4000		// 1620..1680
	#define K_InverterFanSpeed6				4500		// 1860..1920
	#define K_InverterFanSpeed7				5000      // 2220..2280
	#define K_InverterFanSpeedAuto			5000		// Valore per calcolo della velocità ventilatore in modo auto
#endif

	#define K_Min_Current_Custom        	100     // Corrente assorbita MIN in milliampere INIZIALIZZATA PER GR22
	#define K_Max_Current_Custom        	900     // Corrente assorbita MAX in milliampere INIZIALIZZATA PER GR22
	#define K_Efficiency_Custom              47     // Efficienza della ventola in %         INIZIALIZZATA PER GR22
    #define K_Supply_Custom                 230

	#define K_Lim_Temp_Auto_FanSpeed_Min	-100
	#define K_Lim_Temp_Auto_FanSpeed_Max	150
	#define K_Lim_VelPwm_Auto_FanSpeed_Min	PWMValue[1]//K_FanSpeedNight //K_FanSpeedOff 
	#define K_Lim_VelPwm_Auto_FanSpeed_Max	PWMValue[8]//K_FanSpeed7

	// define modalità di ventilazione
	#define Ventil_Off						0
	#define Ventil_Night					1
	#define Ventil_1						2
	#define Ventil_2						3
	#define Ventil_3						4
	#define Ventil_4						5
	#define Ventil_5						6
	#define Ventil_6						7
	#define Ventil_7						8
	#define Ventil_Auto						9

   #define K_DefaultFanSpeed_NetB           4
   #define K_DefaultFanSpeed_SyxtS          8
   #define K_DefaultFanSpeed                9

    #define K_FanNewStyleHeatTimeON         30    // Tempo ON ventola in Riscaldamento CoreOff in PWM: 3 min. OFF, 30 Sec. Night -> vedi "FanOldStyleHeatCoreOff"
    #define K_FanNewStyleHeatTimeOFF        90    //180   // Tempo OFF ventola in Riscaldamento CoreOff in PWM: 1.5 min. OFF, 30 Sec. Night -> vedi "FanOldStyleHeatCoreOff"

	#define VariabiliGlobaliPWM()			\
		volatile int PWM_Counter = 0;		/* Contatore di tick */\
		volatile int SogliaCorrenteVentil;	\
		volatile int PWMValue[9]; 			/* 0->Off; 1->SpeedNight; 2->Speed1; ... 8->Speed7; */\
		volatile unsigned int TachoTMR;		\
		volatile unsigned int TimerTacho;	\
		volatile TypTimer RefreshVentil;		/* Timer per la varazione del pwm */\
		volatile TypTimer RefreshFineVentil;	/* Timer per la pausa durante l'aggiornamento del ventil */\
		unsigned int FlagTachoUpdateRequest;	\
		volatile TypTimer TimerCheckBatteryDefrost;
	
	#define IncludeVariabiliGlobaliPWM()			\
		extern volatile int PWM_Counter;			\
		extern volatile int SogliaCorrenteVentil;	\
		extern volatile int PWMValue[9];			\
		extern volatile unsigned int TachoTMR;		\
		extern volatile unsigned int TimerTacho;	\
		extern volatile TypTimer RefreshVentil;		\
		extern volatile TypTimer RefreshFineVentil;	\
		extern unsigned int FlagTachoUpdateRequest;\
		extern volatile TypTimer TimerCheckBatteryDefrost;

	//---------------------------------------
	// Variabili
	//---------------------------------------
	IncludeVariabiliGlobaliPWM();
	//IncludeVariabiliGlobaliCore();
	//IncludeVariabiliGlobaliProtocolloModBusSec();

	//---------------------------------------
	// Prototipi
	//---------------------------------------
    int DeltaTempInOut(void);
	int DeltaTempReqOut(void);
	void UpdateFanSpeed(void);
	int FanSpeedLimit(int SpeedReq);
    void Init_PWM_Value(void);
	void Init_PWM_Module(void);         // initializzazione del modulo
	void VariazionePWM(void);			// Gestisco la variazione contina del PWM
	//void AggiornaVentil(void);			// Gestisce la variabile TriacDelay in base alle richieste del master
	void FineRegVentil(int SpeedLimit);	// Gestisce la velocità della ventola affinche non generi un flusso d'aria superiore alla potenza erogata
	void ControlloVentil(void);			// Gestisco il controllo sulla corrente del ventilatore
	void Tacho_Init(void);				// Inizializzazione del controllo tachimetrico del ventil
	void TachoInterrupt(void);			// Gestione interrupt del tachimetro
	void RefreshTacho(void);			// Aggiorno il valore della letura tachimetrica in rpm/min
	unsigned char Incrementa(volatile int * Variabile, int Value, int Max);	// Gestisco l'incremento di una variabile
	unsigned char Decrementa(volatile int * Variabile, int Value, int Min);	// Gestisco il decremento di una variabile
    int TempAmbFanSel(void);
    int DeltaSP2Vel(int TempErr);    
    void FanPowerCalculation(void);
    void FAN_Rele_Evaporator(void);
    void FanM3hCalculation(void);
    void AtexFanControl(void);
    void AtexTachoControl(void);  
    void OnOffFanControl(void);
    void OnOffTachoControl(void);  
#endif
