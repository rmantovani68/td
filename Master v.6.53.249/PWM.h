//----------------------------------------------------------------------------------
//	Progect name:	Triac.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			Emanuele
//	Date:			22/05/2011
//	Description:	Header per il modulo di gestione del triac
//----------------------------------------------------------------------------------
#ifndef _PWM_H_
	#define _PWM_H_

	#include "Timer.h"

	//---------------------------------------
	// Define
	//---------------------------------------
	#define	PreScaler_1		0
	#define	PreScaler_2		1
	#define	PreScaler_4		2
	#define	PreScaler_8		3

	#define	CoreOff					0
	#define	CoreRaffrescamento		0x0008
	#define	CoreRiscaldamento		0x0004
	#define CoreVentilazione		0x0010

	#define TickRefreshVentil		3

	#define K_FanSpeedOff           0		// 0
	#define K_FanSpeedNight			45      // 720..780
	#define K_FanSpeed1				55		// 840
	#define K_FanSpeed2				70		// 1020..1080
	#define K_FanSpeed3				85		// 1140..1200
	#define K_FanSpeed4				110     // 1380..1440 
	#define K_FanSpeed5				140		// 1620..1680
	#define K_FanSpeed6				190		// 1860..1920
	#define K_FanSpeed7				255     // 2220..2280
	#define K_FanSpeedAuto			256		// Valore per calcolo della velocità ventilatore in modo auto

	#define K_InverterFanSpeedOff           0           // 0
	#define K_InverterFanSpeedNight			1500        // 720..780
	#define K_InverterFanSpeed1				2000		// 840
	#define K_InverterFanSpeed2				2500		// 1020..1080
	#define K_InverterFanSpeed3				3000		// 1140..1200
	#define K_InverterFanSpeed4				3500        // 1380..1440 
	#define K_InverterFanSpeed5				4000		// 1620..1680
	#define K_InverterFanSpeed6				4500		// 1860..1920
	#define K_InverterFanSpeed7				5000        // 2220..2280
	#define K_InverterFanSpeedAuto			5000		// Valore per calcolo della velocità ventilatore in modo auto




	#define K_Lim_Temp_Auto_FanSpeed_Min	-100
	#define K_Lim_Temp_Auto_FanSpeed_Max	150
	#define K_Lim_VelPwm_Auto_FanSpeed_Min	K_FanSpeedNight //K_FanSpeedOff 
	#define K_Lim_VelPwm_Auto_FanSpeed_Max	K_FanSpeed7

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

    #define K_FanCurrentThreshold           6000    // Corrente in mA - NON UTILIZZATO!


	#define VariabiliGlobaliPWM()			\
		volatile int PWM_Counter = 0;		/* Contatore di tick */\
		volatile int SogliaCorrenteVentil;	\
		volatile int PWMValue[9]; 			/* 0->Off; 1->SpeedNight; 2->Speed1; ... 8->Speed7; */\
		volatile int TempLimitSpeed[8];		/* 0->SpeedNight; 1->Speed1; ... 7->Speed7; */\
		volatile unsigned int TachoTMR;		\
		volatile unsigned int TimerTacho;	\
		volatile TypTimer RefreshVentil;		/* Timer per la varazione del pwm */\
		volatile TypTimer RefreshFineVentil;	/* Timer per la pausa durante l'aggiornamento del ventil */\
		unsigned FlagTachoUpdateRequest
	
	#define IncludeVariabiliGlobaliPWM()			\
		extern volatile int PWM_Counter;			\
		extern volatile int SogliaCorrenteVentil;	\
		extern volatile int PWMValue[9];			\
		extern volatile int TempLimitSpeed[8];		\
		extern volatile unsigned int TachoTMR;		\
		extern volatile unsigned int TimerTacho;	\
		extern volatile TypTimer RefreshVentil;		\
		extern volatile TypTimer RefreshFineVentil;	\
		extern unsigned FlagTachoUpdateRequest;		\
		extern volatile int CntVentilRefresh;

	//---------------------------------------
	// Variabili
	//---------------------------------------
	IncludeVariabiliGlobaliPWM();
	IncludeVariabiliGlobaliCore();
	IncludeVariabiliGlobaliProtocolloModBus();

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
	int	 AutoSpeed(void);				// Gestisce la selezione della velocità quando impostata in auto
	void FineRegVentil(int SpeedLimit);	// Gestisce la velocità della ventola affinche non generi un flusso d'aria superiore alla potenza erogata
	void ControlloVentil(void);			// Gestisco il controllo sulla corrente del ventilatore
	void Tacho_Init(void);				// Inizializzazione del controllo tachimetrico del ventil
	void TachoInterrupt(void);			// Gestione interrupt del tachimetro
	void RefreshTacho(void);			// Aggiorno il valore della letura tachimetrica in rpm/min
	unsigned char Incrementa(int * Variabile, int Value, int Max);	// Gestisco l'incremento di una variabile
	unsigned char Decrementa(int * Variabile, int Value, int Min);	// Gestisco il decremento di una variabile
    int TempAmbFanSel(void);
#endif
