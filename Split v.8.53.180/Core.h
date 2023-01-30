//----------------------------------------------------------------------------------
//	Progect name:	Core.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			22/05/2011
//	Description:	Header delle funzioni della logica di funzionamento
//----------------------------------------------------------------------------------
#ifndef _CORE_ELABORAZIONE_H_
	#define _CORE_ELABORAZIONE_H_

	#include "Timer.h"	
    //#include "Valvola_PassoPasso.h"
	#include "FWSelection.h"
#include "HWSelection.h"
#if (GB210==1) 
	#include <p24FJ256GB210.h>
#elif (DA210==1)
	#include <p24FJ256DA210.h>
#else
	#include <p24FJ256GB110.h>
#endif

#include "PWM.h"
	//----------------------------------------------------------------------------------
	//	Define
	//----------------------------------------------------------------------------------
	// Definizioni per operazioni logiche generiche sulle uscite LED
	#define	LED_ON							0
	#define	LED_OFF							1

	#define	RefreshTime					500
	#define	K_TimeRestartValve			120 //Tempo in secondi per la reinizializzazione della valvola alla riaccensione unit�

	#define	Max_Unit_Sec				16+1 // MAX + 1 perch� lo zero non � utilizzato (-> Room[0].)

	#define Max_Dimmer_Sec				4
	#define	MaxTouch					1//4
	#define	MaxNetB                     1//4
	#define	MaxSyxtS                    1//4

    #define K_ModDeviceNone             0
    #define K_ModDeviceExor             1
    #define K_ModDeviceSyxtS            2
    #define K_ModDeviceNetB             3

    #define K_SupplyVoltage             2400    // Tensione di alimentazione Slave (Teorica)) in centesimi di Volt

    //#define K_Power_Inverter            4000    // Potenza di taglia Inverter in W (Serve per il cacolo della corrente di lavoro che viene restituita dall'inverter solo in percentuale!)
//    #define K_Supply_Inverter           400//200     // Tensione di alimentazione Inverter in V (Serve per il cacolo della tensione In/Out di lavoro che viene restituita dall'inverter solo in percentuale!)
												// Dal Manuale: Tensione in uscita:  
												// La tensione visualizzata � la tensione di comando uscita. 
												// Il valore di riferimento (valore 100%) � 200 V per i modelli da 240 V e 
												// 400 V per i modelli da 500 V
    #define K_VoltOutMaxInverter        238     // Tensione MAX in uscita al motore
	
    // define core split e touch
	#define	CoreOff						0		// Spento
	#define CoreAuto					2//0x0002	// Automatico da touch
	#define	CoreRiscaldamento			4//0x0004	// Riscaldmaneto automatico, si calcola tutto e aziona in mase allo stato del box motore
	#define	CoreRaffrescamento			8//0x0008	// Raffreddamento automatico, si calcola tutto e aziona in mase allo stato del box motore
	#define CoreVentilazione			16//0x0010	// ventilazione automatica, gestisce autonomamente ventilatore e valvola
	#define CoreSlaveCaldo				17//0x0011	// segue la temperatura di evaporazione che gli viene impostata
	#define CoreSlaveFreddo				18//0x0012	// segue la temp di evporazione che gli viene impostat
	#define CoreManuale					19//0x0013	// viene gestito tutto in manuale
    #define CoreUtaAuto                 32


	// power limt
	#define InitPowerLimit				255

	// define modalit� engine box
	#define EngineBox_Off				0
	#define EngineBox_GoOff             1
	#define EngineBox_Freddo			2
	#define EngineBox_GoFreddo			3
	#define EngineBox_Caldo             4
	#define EngineBox_GoCaldo			5
	#define	EngineBox_CriticalFault     6

	// define modalit� di funzionamento pompa
	#define PumpSelection_Auto              1
	#define PumpSelection_1					2
	#define PumpSelection_2                 4

	// define codici errore engine box per valvola
	#define K_CodeErrBox_NoErr				0
	#define K_CodeErrBox_PressureHi			1
	#define K_CodeErrBox_PressureLo			2
	#define K_CodeErrBox_CompressorHi		3
	#define K_CodeErrBox_CompressorLo		4

//Valori per defrosting unit�

#if(SplitMode==K_I_Am_LoopWater)
    #define K_DefrostingTempLiqTrig         100    // Pressione GAS di intervento Errore Defrosting
    #define K_DefrostingTempLiqOk           200     // Pressione GAS di ripristino Errore Defrosting
    #define K_DefrostingTempGasTrig         100    // Pressione GAS di intervento Errore Defrosting
    #define K_DefrostingTempGasOk           200     // Pressione GAS di ripristino Errore Defrosting
    #define K_DefrostingCycleWork           60     // Tempo in Sec. di ciclo errore Defrosting        
#else
    #define K_DefrostingTempLiqTrig         -200//-500    // Pressione GAS di intervento Errore Defrosting
    #define K_DefrostingTempLiqOk           200     // Pressione GAS di ripristino Errore Defrosting
    #define K_DefrostingTempGasTrig         -200    // Pressione GAS di intervento Errore Defrosting
    #define K_DefrostingTempGasOk           200     // Pressione GAS di ripristino Errore Defrosting
    #define K_DefrostingCycleWork           5*60//10*60   // Tempo in Sec. di ciclo errore Defrosting  
#endif
    #define K_DefrostingCycleCheck          30      // Tempo in Sec. di ripristino errore Defrosting
    #define K_DefrostingCycleStop           2*60    //10      // Tempo in Sec. di stop errore Defrosting

//Tempo per massimo cambio Room in caso di dati non inizializzati
    #define K_TimeMaxCmdAddress             10

// regolazione Temperatura batteria Freddo (condizionamento)
	//#define	Freddo_Lim_Rit_Max			-500
	//#define	Freddo_Lim_Mand_Max			-1000
	#define	Freddo_Ist_SP				100
	#define	Freddo_TempBatt_MaxPwr                  0           //    0 -> 7 ////////////////////////////////////////////////
	#define	Freddo_TempBatt_MaxPwr_SlaveMas      0//-500           //  -500 -> 6 Bar
	#define	Freddo_OffSetBatt_MinPwr	1000		//500
	#define	Freddo_Ist_SP_Off           30
	#define	Freddo_Ist_SP_On            10
	#define	Freddo_Ist_Evap_SP_Max      30//0//100
	#define	Freddo_Ist_Evap_SP_Min      Freddo_Ist_SP_Off




	#define Freddo_DigiIn1_EvapTemp		0	//7 Bar				
	#define Freddo_DigiIn2_EvapTemp		0	//7 Bar				

    //#define K_Freddo_Valve_ViLimMin_Liq_PressHi     27000//28000//24000
    #define K_Freddo_Valve_ViLimMax_Liq_PressHi     30000
    #define K_Freddo_Valve_Perc_Liq_PressHi_Work    20
    #define K_Freddo_Valve_Perc_Liq_PressHi_StandBy 5

    //#define K_Freddo_Valve_ViLimMin_Gas_PressHi     28000
    #define K_Freddo_Valve_ViLimMax_Gas_PressHi     28000
    #define K_Freddo_Valve_Perc_Gas_PressHi_Work    20
    #define K_Freddo_Valve_Perc_Gas_PressHi_StandBy 5

    //#define K_Freddo_Valve_ViLimMin_Liq_PressLo     4000
    #define K_Freddo_Valve_ViLimMax_Liq_PressLo     3000
    #define K_Freddo_Valve_Perc_Liq_PressLo_Work    20
    #define K_Freddo_Valve_Perc_Liq_PressLo_StandBy 15//5

    //#define K_Freddo_Valve_ViLimMin_Gas_PressLo     4000
    #define K_Freddo_Valve_ViLimMax_Gas_PressLo     3000
    #define K_Freddo_Valve_Perc_Gas_PressLo_Work    20
    #define K_Freddo_Valve_Perc_Gas_PressLo_StandBy 5

    //#define K_Freddo_Valve_ViLimMin_TempHi          6800//6900
    #define K_Freddo_Valve_ViLimMax_TempHi          7300//7900
    #define K_Freddo_Valve_Perc_TempHi_Work         50//30
    #define K_Freddo_Valve_Perc_TempHi_StandBy      10

    #define K_Freddo_Valve_Perc_TempHi_Cold         1
    #define K_Freddo_Valve_Lim_Perc_TempHi_Cold     5
    #define K_Time_Wait_Correct_Valve_Err           30
    #define K_Time_Go_Correct_Valve_Err             5
    #define K_Correct_SuperHeat_Valve_Err           400

    #define K_CorrectSuperHeatAirOut                10//60
    //#define K_Lim_CorrectSuperHeatAirOut            1200        // 12K SuperHeat
    //#define K_Default_Temp_CorrectSuperHeatAirOut   200         // 2�C


    #define K_Freddo_Valve_ViLimMin_TempLo_Hot      -1500
    #define K_Freddo_Valve_ViLimMax_TempLo_Hot      -900
    #define K_Freddo_Valve_Perc_TempLo_Hot_Work     -20

    #define K_Freddo_Valve_ViLimMin_TempLo_Cold     -1000
    #define K_Freddo_Valve_ViLimMax_TempLo_Cold     -500
    #define K_Freddo_Valve_Perc_TempLo_Cold_Work    -20


//#define K_Freddo_Valve_VoLimMin_PressHi   0
    //#define K_Freddo_Valve_VoLimMax_PressHi   27000


	/*								^
						T_Evap (�C)	|
									|
	SP - Freddo_OffSetBatt_MinPwr	+===============+=========\\
									|			  	|		   \\
									|				|			\\
									|				|			 \\
									|				|			  \\
									|				|			   \\
									|				|				\\
									|				|				 \\
									|				|				 |\\
									|				|				 | \\
									|				|				 |	\\
									|				|				 |	 \\
									|				|				 |	  \\
									|				|				 |	   \\
									|				|				 |		\\
									|				|				 |		 \\
									|				|				 |		  \\
									|				|				 |		   \\
									|				|				 |			\\
									|				|				 |			 \\
									|				|				 |			  \\
									|				|				 |			   \\
									|				|				 |			    \\
									|				|				 |			     \\
			Freddo_TempBatt_MaxPwr	|---------------+----------------+----------------+=============================== = =
									|				|				 |				  |
									|				|				 |				  |
									+---------------+----------------+----------------+----------------------------------->
											SP-Freddo_Ist_SP		SP			SP+Freddo_Ist_SP					T_Amb (�C)
	*/

	// regolazione Temperatura batteria Caldo (riscaldamento)
	#define	Caldo_Lim_Rit_Min			4500
	#define	Caldo_Lim_Mand_Max			6700
	#define	Caldo_Ist_SP				30	//100	//200
#if(K_UTAEnable==0)
	#define	Caldo_TempBatt_MaxPwr		3800	//4450	//4200///////////////////////////////////////////////////////////////
	#define	Caldo_OffSetBatt_MinPwr		3000	//0
#else
	#define	Caldo_TempBatt_MaxPwr		4300	//4450	//4200///////////////////////////////////////////////////////////////
	#define	Caldo_OffSetBatt_MinPwr		3800	//0
#endif
	#define Caldo_DigiIn1_EvapTemp		3500	//18 Bar				
	#define Caldo_DigiIn2_EvapTemp		4300	//22 Bar				

    #define K_SetPoint_SuperHeatRiscaldamento   100

	/*								^
						T_Evap (�C)	|
									|
			Caldo_TempBatt_MaxPwr	+===============+=========\\
									|			  	|		   \\
									|				|			\\
									|				|			 \\
									|				|			  \\
									|				|			   \\
									|				|				\\
									|				|				 \\
									|				|				 |\\
									|				|				 | \\
									|				|				 |	\\
									|				|				 |	 \\
									|				|				 |	  \\
									|				|				 |	   \\
									|				|				 |		\\
									|				|				 |		 \\
									|				|				 |		  \\
									|				|				 |		   \\
									|				|				 |			\\
									|				|				 |			 \\
									|				|				 |			  \\
									|				|				 |			   \\
									|				|				 |			    \\
									|				|				 |			     \\
	SP + Caldo_OffSetBatt_MinPwr	|---------------+----------------+----------------+=============================== = =
									|				|				 |				  |
									|				|				 |				  |
									+---------------+----------------+----------------+----------------------------------->
											SP-Caldo_Ist_SP			SP			SP+Caldo_Ist_SP						T_Amb (�C)
	*/

	// regolazione ciclo di recupero del gas
	#define Gas_Recovery_Temp				-700	// temperatura di intervento del ciclo di recupero del caldo
	#define	Gas_Recovery_Interval			30		// tempo in secondi, della durata del ciclo di recupero del gas
	#define Gas_Recovery_Pause				1200	// tempo in secondi tra un ciclo di recovery del gas ed il successivo
	#define	Gas_Recovery_FilterTime			120		// tempo in secondi di filtro della condizione per l'innesco del ciclo di gas recovery
	#define Gas_Recovery_PressureLimit		13000	// pressione limite al di sotto della quale occorre eseguire il ciclo di recovery

	// Costanti
	#define	StagioneEstate					1
	#define	StagioneInverno					2
	#define	IsteresiCambioModalita			400		// isteresi di temp in pi� e in meno oltre la quale parte il cambio di modalit�


	#define K_PerformanceLossBattery		500					//Perdita della resa batteria per movimento valvola.

//	#define	Setp_PressLiqP_Caldo			9000
	#define K_Iniet_Caldo					300			// Offset rispetto alla temp di acqua mare per il SP
	#define	T_Surriscaldamento_Caldo		400
	#define	WaitExpValvolaCaldo				300 //1000	quasi ok //2000	vecchio	// Tempo elaborazione movimento valvola espansione in caldo (msec.)
	#define	WaitExpValvolaFreddo			300 //2000//1000//500 		// Tempo elaborazione movimento valvola espansione in freddo (msec.)
	#define	K_Lim_Max_Press_Liq_Caldo		9000//10000		// Pressione limite del liquido calcolata da acqua di mare sopra la quale raso il valore.
	
	//#define	K_Lim_Max_Press_Correct_Valve	K_Freddo_Valve_ViLimMax_Liq_PressHi-2000//25500//23500 	// Valore dopo il quale le valvole iniziano ad aprire per tenere la pressione del liquido sotto i 24 bar in freddo.


	#define	Setp_PressLiqP_Freddo			9000
	#define T_Iniet_Freddo					200			// temperatura del tubo liquido da mantenere
	#define	T_Surriscaldamento_Freddo		400			// temperatura in piu del tubo gas da mantenere (tgas = T_Iniet_Freddo+T_Surriscaldamento_Freddo)
	#define T_Iniet_Liq_Freddo				200			// temperatura del tubo liquido da mantenere
//	#define	T_Surriscaldamento_Gas_Freddo	300			// temperatura in piu del tubo gas da mantenere (tgas = T_Iniet_Freddo+T_Surriscaldamento_Freddo)
//	#define	T_Surriscaldamento_Gas_SP_Ok	1300		// temperatura in piu del tubo gas da mantenere (tgas = T_Iniet_Freddo+T_Surriscaldamento_Freddo) quando SP ambiente � soddisfatto
	#define	K_Liq_Troppo_Caldo				100			// delta temperatura liquido sotto la quale inizio a considerare il gas
	#define	K_Liq_Troppo_Freddo				100			// delta temperatura liquido sotto la quale inizio a aprire la valvola
	#define	K_Gas_Troppo_Freddo				100			// delta temperatura gas sotto la quale inizio a chiudere la valvola

	// Parametri test EEV
	#define WaitOpenTime					20
	#define	WaitCloseTime					10
	#define	SogliaGusatoEEV					1000	// T_Big < T_Small
	#define	NumeroCicliTestEEV				5

	// Soglia errore batteria ghiacciata
	#define SogliVentNightBatteriaGhiacciata	-200
	#define SogliVent1_BatteriaGhiacciata		-300
	#define SogliVent2_BatteriaGhiacciata		-400
	#define SogliTempBatteriaGhiacciata			-500	// soglia sotto la quale si setta il flag batteria ghiacciata e viene bloccata la macchina

    #define K_TimerFloodFanCoil              15 //tempo in secondi per indicare errore rpesenza acqua fancoil.

    #define K_TimerGasLeaks                  2*60

    // Parametri per errori sonde di temperatura
    #define K_Gas_Probe_Lo                   -2950
    #define K_Gas_Probe_Hi                   6950
    #define K_Liquid_Probe_Lo                -2950
    #define K_Liquid_Probe_Hi                6950
    #define K_AirOut_Probe_Lo                -2950
    #define K_AirOut_Probe_Hi                6950
    #define K_Ambient_Probe_Lo               -950
    #define K_Ambient_Probe_Hi               3950

    #define K_TimeTemperatureProbe_Error     30

    #define K_DeltaTempElapsedTimeError      18      // Delta -> 0,18 �C
    #define K_TimerCheckProbe_Error          2
                                                                   // Limite conteggio timer = 182 Ore -> 65535 / 60 / 60 * 10
    #define K_MaxCntTempElapsedTimeError_Gas     1*60*60/K_TimerCheckProbe_Error //600    // Cnt = 600 x Cycletime  Example: 600 x 1000mS -> 600Sec -> 10min.
    #define K_MaxCntTempElapsedTimeError_Liquid  1*60*60/K_TimerCheckProbe_Error //600    // Cnt = 600 x Cycletime  Example: 600 x 1000mS -> 600Sec -> 10min.
    #define K_MaxCntTempElapsedTimeError_AirOut  2*60*60/K_TimerCheckProbe_Error //600    // Cnt = 600 x Cycletime  Example: 600 x 1000mS -> 600Sec -> 10min.
    #define K_MaxCntTempElapsedTimeError_Ambient_HI 2*60*60/K_TimerCheckProbe_Error //600    // Cnt = 600 x Cycletime  Example: 600 x 1000mS -> 600Sec -> 10min.
    #define K_MaxCntTempElapsedTimeError_Ambient_LO 8*60*60/K_TimerCheckProbe_Error //600    // Cnt = 600 x Cycletime  Example: 600 x 1000mS -> 600Sec -> 10min.

    #define K_TimerDampers_Alarm            60

    #define K_TimerFlow_Error               30

    #define K_Low_Temp_WaterOut             800

	// Valori di default funzionamento caricati in caso EEPROM non inizializzata
    #define K_Def_SPLIT_Nation              1               // bit.0=USA, bit.1=EUR
#if(K_UTAEnable==0 && SplitMode!=K_I_Am_InverterFAN && SplitMode!=K_I_Am_StandardFAN)
	#define	K_Def_SPLIT_FanMode				Ventil_Auto
#else
	#define	K_Def_SPLIT_FanMode				Ventil_7
#endif
#if(K_Lim_Custom_SP==0)
#if(K_UTAEnable==1)
	#define	K_Def_SPLIT_SP					2000
#elif(SplitMode==K_I_Am_Frigo)
	#define	K_Def_SPLIT_SP					100
#elif(SplitMode==K_I_Am_Freezer)
	#define	K_Def_SPLIT_SP					-1000
#elif(K_DiffPress_Fan_Control==1)
	#define	K_Def_SPLIT_SP					100
#elif(SplitMode==K_I_Am_LoopWater)
	#define	K_Def_SPLIT_SP					2200
#else
	#define	K_Def_SPLIT_SP					2000
#endif
#endif
	#define	K_Def_SPLIT_Mode				CoreVentilazione
	#define	K_Def_SPLIT_Pwr					0
	#define	K_Def_PWR_Limit					InitPowerLimit
	#define	K_Def_SPLIT_SP_F				6400
	#define K_Def_SPLIT_EcoModeEn			0
	//#define K_Def_SPLIT_PwrEcoMode			300
	#define K_Def_SPLIT_SystemDisable		0				// Flag SystemDisable di Default = 0 (disabilitato))
	#define K_Def_SPLIT_Uta_Power			0
	#define K_Def_SPLIT_Uta_Mode			CoreVentilazione
	#define K_Def_SPLIT_Uta_SP_Room			2000
	#define K_Def_SPLIT_Uta_SP_Humi			2
	#define K_Def_SPLIT_Uta_Fan_Speed		3
	#define K_Def_SPLIT_Uta_SP_Room_F		6400
	#define K_Def_SPLIT_PumpSwitchTime		12
	#define K_Def_SPLIT_PumpSelection		PumpSelection_Auto
	#define K_Def_SPLIT_HeaterPwr   		0

    #define K_Def_On_Priority_Mode              1
    #define K_Def_ValveOnDemand_Min_Percent_Val 18
    #define K_Def_Max_Lim_ON_Demand_Total_Score 40 
    #define K_Def_TimeOnDemandValveSwitch       3

    #define K_Def_TOUCH_TESTALL_ABIL                    3072//0
    #define K_Def_TOUCH_TESTALL_SUPERHEAT_SP_COOL       600
    #define K_Def_TOUCH_TESTALL_SUPERHEAT_SP_HEAT       800
    #define K_Def_TOUCH_TESTALL_MAX_OPEN_VALVE_COOL     100
    #define K_Def_TOUCH_TESTALL_MAX_OPEN_VALVE_HEAT     50
    #define K_Def_TOUCH_TESTALL_MINOPVALVE_COOL         0
    #define K_Def_TOUCH_TESTALL_MINOPVALVE_HEAT         5
    #define K_Def_TOUCH_TESTALL_WORK_TEMP_COOL          0//-500
    #define K_Def_TOUCH_TESTALL_WORK_TEMP_HEAT          4300

    #define K_Def_TOUCH_TESTALL_FROZEN_ABIL             0
    #define K_Def_TOUCH_TESTALL_SUPERHEAT_SP_FRIGO      1000
    #define K_Def_TOUCH_TESTALL_SUPERHEAT_SP_FREEZER    1000
    #define K_Def_TOUCH_TESTALL_MAXOPVALVE_FRIGO        20
    #define K_Def_TOUCH_TESTALL_MAXOPVALVE_FREEZER      20
    #define K_Def_TOUCH_TESTALL_MINOPVALVE_FRIGO        8
    #define K_Def_TOUCH_TESTALL_MINOPVALVE_FREEZER      8
    #define K_Def_TOUCH_TESTALL_TEMPBATT_SP_FRIGO       100
    #define K_Def_TOUCH_TESTALL_TEMPBATT_SP_FREEZER     -2500

    #define K_Def_TOUCH_ABIL_DEFROSTING                 0
    #define K_Def_TOUCH_DEFROSTING_STEP_TIME            8
    #define K_Def_TOUCH_DEFROSTING_TIME                 10
    #define K_Def_TOUCH_DEFROSTING_TEMP_SET             200
    #define K_Def_TOUCH_DRIPPING_TIME                   10
    #define K_Def_TOUCH_VALVOLA_FROZEN                  20

    #define K_Def_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN         350
    #define K_Def_TOUCH_TESTALL_SUPERHEAT_HEAT_IGAIN         0
    #define K_Def_TOUCH_TESTALL_SUPERHEAT_HEAT_DGAIN         0
    #define K_Def_TOUCH_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID 600

    #define K_Def_TOUCH_TESTALL_SUPERHEAT_COOL_PGAIN         227
    #define K_Def_TOUCH_TESTALL_SUPERHEAT_COOL_IGAIN         1
    #define K_Def_TOUCH_TESTALL_SUPERHEAT_COOL_DGAIN         0
    #define K_Def_TOUCH_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID 500

    #define K_Def_TOUCH_MAINTENANCE_FLAG                     0
    #define K_Def_TOUCH_MAINTENANCE_SET_TIME                 60

    #define K_Def_TOUCH_TRIGGER_INIT                         0

    #define K_Def_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT          600
    #define K_Def_TOUCH_SUPERHEAT_COOL_MAX_VALUE_CORRECT     400
    #define K_Def_TOUCH_SUPERHEAT_HEAT_MIN_SETPOINT          400
    #define K_Def_TOUCH_SUPERHEAT_HEAT_MAX_SETPOINT          700

    #define K_Def_TOUCH_THRESHOLD_COMPRESSOR_HI              7500

    #define K_Def_TOUCH_SET_HUMI                             50
    #define K_Def_TOUCH_SET_DELTA_TEMP_MIN_DEHUMI            1
    #define K_Def_TOUCH_SET_FAN_SPEED_DEHUMI                 3
    #define K_Def_TOUCH_SET_PRESS_DEHUMI                     7000
    #define K_Def_TOUCH_SET_TEMP_AIR_OUT                     500
    #define K_Def_TOUCH_SET_TEMP_AIR_OUT_DEHUMI              200


// Valori per check limiti 
    //#define K_LimLo_SPLIT_Nation            0               // bit.0=USA, bit.1=EUR
	//#define K_LimHi_SPLIT_Nation            2               // bit.0=USA, bit.1=EUR
	#define	K_LimLo_SPLIT_FanMode			Ventil_Night
#if(K_UTAEnable==0&&SplitMode!=K_I_Am_InverterFAN && SplitMode!=K_I_Am_StandardFAN)
	#define	K_LimHi_SPLIT_FanMode			Ventil_Auto
#else
	#define	K_LimHi_SPLIT_FanMode			Ventil_7
#endif
#if(K_Lim_Custom_SP==0)
#if(K_UTAEnable==1)
	#define	K_LimLo_SPLIT_SP				2000
	#define	K_LimHi_SPLIT_SP				2500
#elif(SplitMode==K_I_Am_Frigo)
	#define	K_LimLo_SPLIT_SP				100
	#define	K_LimHi_SPLIT_SP				1000
#elif(SplitMode==K_I_Am_Freezer)
	#define	K_LimLo_SPLIT_SP				-2000
	#define	K_LimHi_SPLIT_SP				0
#elif(K_DiffPress_Fan_Control==1)
	#define	K_LimLo_SPLIT_SP				100
	#define	K_LimHi_SPLIT_SP				2000
#elif(SplitMode==K_I_Am_LoopWater)
	#define	K_LimLo_SPLIT_SP				2200
	#define	K_LimHi_SPLIT_SP				3000
#else
	#define	K_LimLo_SPLIT_SP				1775
	#define	K_LimHi_SPLIT_SP				3000
#endif
#endif
	//#define	K_LimLo_SPLIT_Mode				CoreVentilazione
	//#define	K_LimHi_SPLIT_Mode				CoreVentilazione
	#define	K_LimLo_SPLIT_Pwr				0
	#define	K_LimHi_SPLIT_Pwr				1
	#define	K_LimLo_PWR_Limit				102		// 40% su scala 0..255
	#define	K_LimHi_PWR_Limit				255		// 100% su scala 0..255
#if(K_UTAEnable==0)
	#define	K_LimLo_SPLIT_SP_F				6400
	#define	K_LimHi_SPLIT_SP_F				8600
#else
	#define	K_LimLo_SPLIT_SP_F				6800
	#define	K_LimHi_SPLIT_SP_F				7700
#endif
	#define K_LimLo_SPLIT_EcoModeEn			0
	#define K_LimHi_SPLIT_EcoModeEn			1

#if(K_Fixed_EcoMode==0)
	#define K_LimLo_SPLIT_PwrEcoMode		50
#else
	#define K_LimLo_SPLIT_PwrEcoMode		K_Def_SPLIT_PwrEcoMode//300
#endif
	#define K_LimHi_SPLIT_PwrEcoMode		K_Def_SPLIT_PwrEcoMode//300
	#define K_LimLo_SPLIT_SystemDisable		0				// Flag SystemDisable di Default = 0 (disabilitato))
	#define K_LimHi_SPLIT_SystemDisable		1				// Flag SystemDisable di Default = 0 (disabilitato))
	#define K_LimLo_SPLIT_Uta_Power			0
	#define K_LimHi_SPLIT_Uta_Power			1
	//#define K_LimLo_SPLIT_Uta_Mode			1				//DA SISTEMARE
	//#define K_LimHi_SPLIT_Uta_Mode			1				//DA SISTEMARE
	#define K_LimLo_SPLIT_Uta_SP_Room		500
	#define K_LimHi_SPLIT_Uta_SP_Room		3500
	#define K_LimLo_SPLIT_Uta_SP_Humi		1
	#define K_LimHi_SPLIT_Uta_SP_Humi		3
	#define K_LimLo_SPLIT_Uta_Fan_Speed		1
	#define K_LimHi_SPLIT_Uta_Fan_Speed		3
	#define K_LimLo_SPLIT_Uta_SP_Room_F		4100
	#define K_LimHi_SPLIT_Uta_SP_Room_F		9500
	#define K_LimLo_SPLIT_PumpSwitchTime	1
	#define K_LimHi_SPLIT_PumpSwitchTime	12
	#define K_LimLo_SPLIT_HeaterPwr   		0
	#define K_LimHi_SPLIT_HeaterPwr   		1


    #define K_LimLo_SPLIT_On_Priority_Mode  0
    #define K_LimHi_SPLIT_On_Priority_Mode  1
        
    #define K_LimLo_SPLIT_ValveOnDemand_Min_Percent_Val 0
    #define K_LimHi_SPLIT_ValveOnDemand_Min_Percent_Val 40
    
    #define K_LimLo_SPLIT_Max_Lim_ON_Demand_Total_Score 10
    #define K_LimHi_SPLIT_Max_Lim_ON_Demand_Total_Score 1000
    
    #define K_LimLo_SPLIT_TimeOnDemandValveSwitch   1
    #define K_LimHi_SPLIT_TimeOnDemandValveSwitch   10

    #define K_LimLo_TOUCH_TESTALL_ABIL                    0
    #define K_LimHi_TOUCH_TESTALL_ABIL                    65535
    #define K_LimLo_TOUCH_TESTALL_SUPERHEAT_SP_COOL       100
    #define K_LimHi_TOUCH_TESTALL_SUPERHEAT_SP_COOL       2500
    #define K_LimLo_TOUCH_TESTALL_SUPERHEAT_SP_HEAT       100
    #define K_LimHi_TOUCH_TESTALL_SUPERHEAT_SP_HEAT       2500
    #define K_LimLo_TOUCH_TESTALL_MAX_OPEN_VALVE_COOL     5
    #define K_LimHi_TOUCH_TESTALL_MAX_OPEN_VALVE_COOL     100
    #define K_LimLo_TOUCH_TESTALL_MIN_OPEN_VALVE_COOL     0
    #define K_LimHi_TOUCH_TESTALL_MIN_OPEN_VALVE_COOL     30
    #define K_LimLo_TOUCH_TESTALL_MAX_OPEN_VALVE_HEAT     5
    #define K_LimHi_TOUCH_TESTALL_MAX_OPEN_VALVE_HEAT     100
    #define K_LimLo_TOUCH_TESTALL_MIN_OPEN_VALVE_HEAT     1
    #define K_LimHi_TOUCH_TESTALL_MIN_OPEN_VALVE_HEAT     30
    #define K_LimLo_TOUCH_TESTALL_WORK_TEMP_COOL          -2500
    #define K_LimHi_TOUCH_TESTALL_WORK_TEMP_COOL          500
    #define K_LimLo_TOUCH_TESTALL_WORK_TEMP_HEAT          3600
    #define K_LimHi_TOUCH_TESTALL_WORK_TEMP_HEAT          4600

    #define K_LimLo_TOUCH_TESTALL_FROZEN_ABIL              0
    #define K_LimHi_TOUCH_TESTALL_FROZEN_ABIL              255
    #define K_LimLo_TOUCH_TESTALL_SUPERHEAT_SP_FRIGO       100
    #define K_LimHi_TOUCH_TESTALL_SUPERHEAT_SP_FRIGO       2500
    #define K_LimLo_TOUCH_TESTALL_SUPERHEAT_SP_FREEZER     100
    #define K_LimHi_TOUCH_TESTALL_SUPERHEAT_SP_FREEZER     2500
    #define K_LimLo_TOUCH_TESTALL_MAX_OPEN_VALVE_FRIGO     5
    #define K_LimHi_TOUCH_TESTALL_MAX_OPEN_VALVE_FRIGO     100
    #define K_LimLo_TOUCH_TESTALL_MIN_OPEN_VALVE_FRIGO     0
    #define K_LimHi_TOUCH_TESTALL_MIN_OPEN_VALVE_FRIGO     30
    #define K_LimLo_TOUCH_TESTALL_MAX_OPEN_VALVE_FREEZER   5
    #define K_LimHi_TOUCH_TESTALL_MAX_OPEN_VALVE_FREEZER   100
    #define K_LimLo_TOUCH_TESTALL_MIN_OPEN_VALVE_FREEZER   0
    #define K_LimHi_TOUCH_TESTALL_MIN_OPEN_VALVE_FREEZER   30
    #define K_LimLo_TOUCH_TESTALL_TEMPBATT_SP_FRIGO        -2500
    #define K_LimHi_TOUCH_TESTALL_TEMPBATT_SP_FRIGO        500
    #define K_LimLo_TOUCH_TESTALL_TEMPBATT_SP_FREEZER      -2500
    #define K_LimHi_TOUCH_TESTALL_TEMPBATT_SP_FREEZER      0

    #define K_LimLo_TOUCH_ABIL_DEFROSTING                  0
    #define K_LimHi_TOUCH_ABIL_DEFROSTING                  15
    #define K_LimLo_TOUCH_DEFROSTING_STEP_TIME             2
    #define K_LimHi_TOUCH_DEFROSTING_STEP_TIME             24
    #define K_LimLo_TOUCH_DEFROSTING_TIME                  10
    #define K_LimHi_TOUCH_DEFROSTING_TIME                  30
    #define K_LimLo_TOUCH_DEFROSTING_TEMP_SET              200//500
    #define K_LimHi_TOUCH_DEFROSTING_TEMP_SET              1000
    #define K_LimLo_TOUCH_DRIPPING_TIME                    1
    #define K_LimHi_TOUCH_DRIPPING_TIME                    10
    #define K_LimLo_TOUCH_VALVOLA_FROZEN                   0
    #define K_LimHi_TOUCH_VALVOLA_FROZEN                   100

    #define K_LimLo_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN          0
    #define K_LimHi_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN          32000
    #define K_LimLo_TOUCH_TESTALL_SUPERHEAT_HEAT_IGAIN          0
    #define K_LimHi_TOUCH_TESTALL_SUPERHEAT_HEAT_IGAIN          32000
    #define K_LimLo_TOUCH_TESTALL_SUPERHEAT_HEAT_DGAIN          0
    #define K_LimHi_TOUCH_TESTALL_SUPERHEAT_HEAT_DGAIN          32000
    #define K_LimLo_TOUCH_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID  1
    #define K_LimHi_TOUCH_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID  10000

    #define K_LimLo_TOUCH_TESTALL_SUPERHEAT_COOL_PGAIN          0
    #define K_LimHi_TOUCH_TESTALL_SUPERHEAT_COOL_PGAIN          32000
    #define K_LimLo_TOUCH_TESTALL_SUPERHEAT_COOL_IGAIN          0
    #define K_LimHi_TOUCH_TESTALL_SUPERHEAT_COOL_IGAIN          32000
    #define K_LimLo_TOUCH_TESTALL_SUPERHEAT_COOL_DGAIN          0
    #define K_LimHi_TOUCH_TESTALL_SUPERHEAT_COOL_DGAIN          32000
    #define K_LimLo_TOUCH_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID  1
    #define K_LimHi_TOUCH_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID  10000

    #define K_LimLo_TOUCH_MAINTENANCE_FLAG                      0
    #define K_LimHi_TOUCH_MAINTENANCE_FLAG                      3
    #define K_LimLo_TOUCH_MAINTENANCE_SET_TIME                  1
    #define K_LimHi_TOUCH_MAINTENANCE_SET_TIME                  365

    #define K_LimLo_TOUCH_TRIGGER_INIT                          0
    #define K_LimHi_TOUCH_TRIGGER_INIT                          15

    #define K_LimLo_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT           400
    #define K_LimHi_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT           1000
    #define K_LimLo_TOUCH_SUPERHEAT_COOL_MAX_VALUE_CORRECT      0
    #define K_LimHi_TOUCH_SUPERHEAT_COOL_MAX_VALUE_CORRECT      2000
    #define K_LimLo_TOUCH_SUPERHEAT_HEAT_MIN_SETPOINT           100
    #define K_LimHi_TOUCH_SUPERHEAT_HEAT_MIN_SETPOINT           1000
    #define K_LimLo_TOUCH_SUPERHEAT_HEAT_MAX_SETPOINT           600
    #define K_LimHi_TOUCH_SUPERHEAT_HEAT_MAX_SETPOINT           1200

    #define K_LimLo_TOUCH_THRESHOLD_COMPRESSOR_HI               6900
    #define K_LimHi_TOUCH_THRESHOLD_COMPRESSOR_HI               7900

    #define K_LimLo_TOUCH_SET_HUMI                             45
    #define K_LimHi_TOUCH_SET_HUMI                             65
    #define K_LimLo_TOUCH_SET_DELTA_TEMP_MIN_DEHUMI            0
    #define K_LimHi_TOUCH_SET_DELTA_TEMP_MIN_DEHUMI            200
    #define K_LimLo_TOUCH_SET_FAN_SPEED_DEHUMI                 1
    #define K_LimHi_TOUCH_SET_FAN_SPEED_DEHUMI                 8
    #define K_LimLo_TOUCH_SET_PRESS_DEHUMI                     5000
    #define K_LimHi_TOUCH_SET_PRESS_DEHUMI                     10000
    #define K_LimLo_TOUCH_SET_TEMP_AIR_OUT                     0
    #define K_LimHi_TOUCH_SET_TEMP_AIR_OUT                     2000
    #define K_LimLo_TOUCH_SET_TEMP_AIR_OUT_DEHUMI              0
    #define K_LimHi_TOUCH_SET_TEMP_AIR_OUT_DEHUMI              2000

    #define VariabiliGlobaliCore()							\
		volatile TypTimer LCD_ScreeScheduler;				\
		volatile TypTimer PausaCompressore;					\
		volatile TypTimer TimerCompressore;					\
		volatile TypTimer TimerDefostCycle;					\
		volatile TypTimer TimerRecoveryGasCycle;			\
		volatile TypTimer TimerRecoveryGasInterval;			\
		volatile TypTimer TimerRecoveryGasFilter;			\
		volatile TypTimer TimerTestEEV;						\
        volatile TypTimer TimerExecPID;                     \
		volatile TypTimer TimerRestartValve;				\
		volatile char CyclTestEEV;							\
		volatile TypEngineBox EngineBox;					\
		volatile TypInitSplit Reinit;						\
		volatile TypSplit Me;								\
		volatile TypTouch Touch[MaxTouch];					\
		volatile TypNetB NetB[MaxNetB];                     \
		volatile TypSyxtS SyxtS[MaxSyxtS];					\
		volatile TypSplitSlave Room[Max_Unit_Sec];	/* definisco le variabili per le stanze */	\
		unsigned int EnableMySecBus;						\
		volatile char DividerValvola;						\
		volatile TypTimer TimerExpValvola;					\
        volatile TypTimer TimerResyncValvola;				\
        volatile TypTimer TimerExpValv_Err;                 \
		volatile TypDiagUnit DiagnosticSplit;				\
		volatile TypDiagInverter DiagnosticInverter;        \
        volatile TypTimer TimerSpeedChangeCoreOff;          /* Timer per Boost ventola in SP raggiunto*/\
        volatile TypTimer TimerFanVelSwitch;               /* Timer per switch velocit� ventola in CoreOFF (Riscaldamento) -> FanOldStyleHeatCoreOff */\
        volatile TypTimer TimerCheckFanTacho;               \
        volatile TypTimerLong TimerDefrostingCycleWork;     \
        volatile TypTimer TimerDefrostingCycleStop;         \
        volatile TypTimer TimerDefrostingCycleCheck;        \
        volatile TypTimer TimerOnValveFrozen;               \
        volatile TypTimer TimerTemperatureFault;            \
        volatile TypTimer TimerCorrectSuperHeatAirOut;      \
        volatile TypTimer TimerMaxTimeCmdAddress;           \
        volatile TypTimer TimerFloodFanCoil;                \
        volatile TypTimer TimerGasLeaks;                    \
        volatile TypTimer TimerTemperatureProbeError;       \
        volatile TypTimer TimerErrFlow;                     \
        volatile TypTimerLong TimerSelectPumpWork;          \
        volatile TypTimer TimerOutWaterStop;                \
        volatile TypTimer TimerHeaterFresh;                 \
        volatile TypTimer TimerDampersAlarm;

	#define IncludeVariabiliGlobaliCore()					\
		extern volatile TypTimer LCD_ScreeScheduler;		\
		extern volatile TypTimer PausaCompressore;			\
		extern volatile TypTimer TimerCompressore;			\
		extern volatile TypTimer TimerDefostCycle;			\
		extern volatile TypTimer TimerRecoveryGasCycle;		\
		extern volatile TypTimer TimerRecoveryGasInterval;	\
		extern volatile TypTimer TimerRecoveryGasFilter;	\
		extern volatile TypTimer TimerTestEEV;				\
        extern volatile TypTimer TimerExecPID;              \
		extern volatile TypTimer TimerRestartValve;			\
		extern volatile char CyclTestEEV;					\
		extern volatile TypEngineBox EngineBox;         	\
		extern volatile TypInitSplit Reinit;				\
		extern volatile TypSplit Me;						\
		extern volatile TypTouch Touch[MaxTouch];			\
		extern volatile TypNetB NetB[MaxNetB];              \
		extern volatile TypSyxtS SyxtS[MaxSyxtS];           \
		extern volatile TypSplitSlave Room[Max_Unit_Sec];	\
		extern unsigned int EnableMySecBus;                 \
		extern volatile char DividerValvola;				\
		extern volatile TypTimer TimerExpValvola;			\
        extern volatile TypTimer TimerResyncValvola;        \
        extern volatile TypTimer TimerExpValv_Err;          \
		extern volatile TypDiagUnit DiagnosticSplit;		\
		extern volatile TypDiagInverter DiagnosticInverter; \
        extern volatile TypTimer TimerSpeedChangeCoreOff;   /* Timer per Boost ventola in SP raggiunto*/\
        extern volatile TypTimer TimerFanVelSwitch;         /* Timer per switch velocit� ventola in CoreOFF (Riscaldamento) -> FanOldStyleHeatCoreOff */\
        extern volatile TypTimer TimerCheckFanTacho;        \
        extern volatile TypTimerLong TimerDefrostingCycleWork;  \
        extern volatile TypTimer TimerDefrostingCycleStop;  \
        extern volatile TypTimer TimerDefrostingCycleCheck; \
        extern volatile TypTimer TimerOnValveFrozen;        \
        extern volatile TypTimer TimerTemperatureFault;     \
        extern volatile TypTimer TimerCorrectSuperHeatAirOut; \
        extern volatile TypTimer TimerMaxTimeCmdAddress;    \
        extern volatile TypTimer TimerFloodFanCoil;         \
        extern volatile TypTimer TimerGasLeaks;             \
        extern volatile TypTimer TimerTemperatureProbeError;\
        extern volatile TypTimer TimerErrFlow;              \
        extern volatile TypTimerLong TimerSelectPumpWork;   \
        extern volatile TypTimer TimerOutWaterStop;         \
        extern volatile TypTimer TimerHeaterFresh;          \
        extern volatile TypTimer TimerDampersAlarm;

	//----------------------------------
	// Strutture
	//----------------------------------
	typedef struct
	{			 int	SetPoint;
		unsigned int	FunctionMode;			// 0=Raffrescamento / 1=Riscaldamento
				 int	FanMode;				// 0..4 (0=OFF, 1,2,3=VEL 4=Auto)
				 int	Temperature;
		unsigned int	Pressure;				// Pressione ambiente in decimi (hPa)
                 int	FW_Version;
                 int	FW_Release;           
			unsigned 	Enable:1;				// se � abilitato
			unsigned	OffLine:1;				// se non ha risposto
			unsigned	OnLine:1;				// se � OnLine            
            unsigned    DeviceAlarm:1;

            unsigned    PermanentOffline:1;
            unsigned int CntErrTouch;
	} TypNetB;
    
	typedef struct
	{			 int	SetPoint;
		unsigned int	FunctionMode;			// 0=Raffrescamento / 1=Riscaldamento
				 int	FanMode;				// 0..4 (0=OFF, 1,2,3=VEL 4=Auto)
				 int	Temperature;
                 int    HeaterPwr;
		unsigned int	Pressure;				// Pressione ambiente in decimi (hPa)
                 int	FW_Version;
                 int	FW_Release;           
			unsigned 	Enable:1;				// se � abilitato
			unsigned	OffLine:1;				// se non ha risposto
			unsigned	OnLine:1;				// se � OnLine           
            unsigned    DeviceAlarm:1;                        
            unsigned    FwVer_Upper_423:1;
            unsigned    PermanentOffline:1;
            unsigned int CntErrTouch;

	} TypSyxtS;    

	typedef struct
	{			 
			int	SetPoint;
			int	FunctionMode;         
			int	FanMode;
			int	PowerLimit;
			int	OldPowerLimit;
			int	InfraRedMode;
			int	Current_Page;			// pagina corrente del touch
			int	Split_Test_Add;			// Indirizzo Slave che si vuole monitorare #v16
			//int	DimCh1;		
			//int	DimCh2;
			//int	DimCh3;
			//int	DimCh4;
			//int	DimCh5;
			//int	DimCh6;
			int	DefMode;
			int	Script_Split_Pwr;
			int	Script_SetPoint_F;
	   unsigned Enable:1;				// se � abilitato
	   unsigned	OffLine:1;				// se non ha risposto
	   unsigned	OnLine:1;				// se � abilitato e online       
	   unsigned PermanentOffline:1;
       unsigned DeviceAlarm:1;       

			int Uta_Power;
			int Uta_Mode;
			int Uta_SP_Room;
			int Uta_SP_Humi;
			int Uta_Fan_Speed;
			int Uta_SP_Room_F;	
			
			int SystemDisable;
            
            int HeaterPwr;
			
			int	Uta_Abil;
            unsigned int CntErrTouch;
            
        unsigned int On_Priority_Mode;
        unsigned int ValveOnDemand_Min_Percent_Val;
        unsigned int Max_Lim_ON_Demand_Total_Score;
        unsigned int TimeOnDemandValveSwitch;
        
            int TestAll_Abil;
            int TestAll_SuperHeat_SP_Cool;
            int TestAll_SuperHeat_SP_Heat;
            int TestAll_MaxOpValve_Cool;
            int TestAll_MaxOpValve_Heat;
            int TestAll_TempBatt_SP_Cool;
            int TestAll_TempBatt_SP_Heat;
                        
            int Frozen_Mode;
            
            int TestALL_Frozen_Abil;
            int TestAll_SuperHeat_SP_Frigo;
            int TestAll_SuperHeat_SP_Freezer;
            int TestAll_MaxOpValve_Frigo;
            int TestAll_MaxOpValve_Freezer;
            int TestAll_TempBatt_SP_Frigo;
            int TestAll_TempBatt_SP_Freezer;
            
            int TestAll_MinOpValve_Cool;
            int TestAll_MinOpValve_Heat;
            int TestAll_MinOpValve_Frigo;
            int TestAll_MinOpValve_Freezer;
            
            int Abil_Defrosting;
            int Defrosting_Step_Time;
            int Defrosting_Time;      
            int Defrosting_Temp_Set;        
            int Dripping_Time;
            unsigned int Valvola_Frozen;
            
            unsigned int TestAll_SuperHeat_Heat_pGain;
            unsigned int TestAll_SuperHeat_Heat_iGain;
            unsigned int TestAll_SuperHeat_Heat_dGain;
            unsigned int TestAll_SuperHeat_Heat_Time_Exec_PID;

            unsigned int TestAll_SuperHeat_Cool_pGain;
            unsigned int TestAll_SuperHeat_Cool_iGain;
            unsigned int TestAll_SuperHeat_Cool_dGain;
            unsigned int TestAll_SuperHeat_Cool_Time_Exec_PID;
            
            unsigned int Maintenance_Flag;
            unsigned int Maintenance_Set_Time;
            unsigned int Maintenance_Remaning_Time;
            
            int SuperHeat_Cool_Min_SetPoint;
            int SuperHeat_Cool_Max_Value_Correct;
            int SuperHeat_Heat_Min_SetPoint;
            int SuperHeat_Heat_Max_SetPoint;

            int Threshold_Compressor_Hi;            

            int Set_Humi;
            int Set_Delta_Temp_Min_Dehumi;
            int Set_Fan_Speed_Dehumi;
            int Set_Press_Dehumi;
            int Set_Temp_Air_Out;
            int Set_Temp_Air_Out_Dehumi;

		struct 
		{	
			int	SetPoint;
			int	FanMode;            
			int	Script_Split_Pwr;
			int	Script_SetPoint_F;            
            int Uta_Mode;
            int DefMode;
            int SystemDisable;
            
            struct 
            {	
                int		Pwr;
                int		SetPoint;
                int		FanMode;
                int		SetPoint_F;
                int     Uta_Mode;
                int     HeaterPwr;
            } DiagWrite;         
            
		} Previous;

		struct 
		{	
			int	SetPoint;
			int	FanMode;            
			int	Script_Split_Pwr;
			int	Script_SetPoint_F;            
            int Uta_Mode;
            int DefMode;
            int SystemDisable;            
                
            struct 
            {	
                int		Pwr;
                int		SetPoint;
                int		FanMode;
                int		SetPoint_F;
                int     Uta_Mode;
                int     HeaterPwr;
            } DiagWrite;                    
            
		} CheckCounter;
        
        
		struct 
		{	
			int		Pwr;
			int		SetPoint;
			int		FanMode;
			int		SetPoint_F;
            int     Uta_Mode;
            int     HeaterPwr;

		} DiagWrite;
			
	} TypTouch;


	typedef struct
	{
				 int 	Address;
                 int    SubAddress;
				 int	CmdAddress;
                 int    OldValueCmdAddress;
				 int	SplitStatus;
				 int	Temp_Gas;
				 int	Temp_Liq;
				 int	Temp_AirOut;
				 int	Temp_AirIn;
				 int	FanSpeed;
				 int	Exp_Valve;
                 int	Exp_Valve_Err;
                 int	Exp_Valve_Err_Code;
				 int	Gas_Pressure;
				 int	Liq_Pressure;
				 int	Req_AirTemp;
				 int	Req_Pressure;
				 int	Split_Eng_Mode;
                 int    Reset_Req_Address;
                 int    Sub_Slave_Quantity;
				 int    EvapTempValve;
		unsigned int	Ready_CMD_Address;
                 int    Ready_PowerOn;
                 int    Ready_SetPoint;
                 int    Ready_SetPoint_F;
                 int    Ready_FanMode;
                 int    Ready_UtaMode;
                 int    ModDevType;
         
		unsigned int	DataProcess;
		unsigned int	DataReady;                 
		unsigned int	CycleTime;
		unsigned int	HW_Version;				// REG_TOUCH_DIAG_SPLIT_HW_VERSION
		unsigned int	FW_Version_HW;			// REG_TOUCH_DIAG_SPLIT_FW_VERSION_HW                
        unsigned int	FW_Version_FW;			// REG_TOUCH_DIAG_SPLIT_FW_VERSION_FW
        unsigned int	FW_Revision_FW;			// REG_TOUCH_DIAG_SPLIT_FW_REVISION_FW
        unsigned int	Model_FW_Version;
				 int	Superheat;			
				 int	SuperheatRiscaldamento;
                 int    SetP_Superheat;
                 int    UTA_Abil;
        unsigned int    Sub_Slave_Flags_Enable_1_16;
        unsigned int    Sub_Slave_Flags_Online_1_16;
        unsigned int    Sub_Slave_Flags_Offline_1_16;
        unsigned int    Sub_Slave_Flags_Alarm_1_16;  
        unsigned int    Only_Fan;
				 int	TempAmbMiddle;
                 int    TempAmbReal;
                 int    Set_CMD_Disable;  
                 int    FunctionMode;
				 int	HumidityMiddle;
                 
        unsigned int    Fan_Power;
        unsigned int    Fan_Current;
        unsigned int    Supply_Voltage;     
        unsigned int    FrozenMode;
        unsigned int    MaxVentilMode;
        unsigned int    Humidity;
        
        unsigned int    MinSetPoint;
        unsigned int    MaxSetPoint;       
        
        unsigned int    Fan_M3h;
        
        unsigned int    AirPowerOutput;
        unsigned int    AirPowerBTU;
		
        unsigned int    Touch_Page;

        unsigned int    Heater_Abil;
        
                 int    Superheat_Cool_PID_Error;
                 
        unsigned int    Temperature_Probe_Error;
        
		unsigned 		Enable:1;				// se � abilitato
		unsigned		OffLine:1;				// se non ha risposto
        
		struct 
		{	
            unsigned int	CntComErrSec;			
            unsigned int    TimeOutComErrSec;		
            unsigned int	TimeOutInterCharComErrSec;
            unsigned int	CRC_ComErrSec;
            unsigned int	ModCntComErrSec;			
            unsigned int	ModTimeOutComErrSec;		
            unsigned int	ModTimeOutInterCharComErrSec;
            unsigned int	ModCRC_ComErrSec;
		} ComError;             
        
	} TypDiagUnit;


	typedef struct
	{
		unsigned int 	Alarm;
		unsigned int	OutFreq;
		unsigned int	Status;
		unsigned int	OutCurr;
		unsigned int	InVolt;
		unsigned int	OutVolt;
		unsigned int	InPower;
		unsigned int	OutPower;
		unsigned int	ReqFreq;
		unsigned int	MaxReqFreq;
		unsigned int	Power_Inverter;
		unsigned int	PowerMax_Available;
		unsigned int	Max_Inverter;
		unsigned int	DiagAddress;			
		unsigned 		Enable:1;				// se � abilitato
		unsigned		OffLine:1;				// se non ha risposto
	} TypDiagInverter;


	typedef struct
	{        
	   struct 
	   {	
                int	Gas;        // temperatura tubo del Gas
                int	Liquid;     // temperatura tubo del Liquido
                int	AirOut;     // temperatura aria in uscita
                int	Ambient;    // temperatura ambiente

            struct 
            {	
                    int	Gas;
                    int	Liquid;
                    int	AirOut;
                    int	Ambient;
            } PreviusVal;
            union ByteBit
            {
                 unsigned int Compact;        
                 struct 
                 {
                     unsigned	Gas:1;
                     unsigned	Liquid:1;
                     unsigned	AirOut:1;
                     unsigned	Ambient:1;
                 } Bit;        
            }; 

            union ByteBit Error;

                struct 
                {	
                          int	Gas;
                          int	Liquid;
                          int	AirOut;
                          int	Ambient;
                } CntError;                

                struct 
                {	
                          int	Gas;
                          int	Liquid;
                          int	AirOut;
                          int	Ambient;
                } Freeze;                
        } Temperature;

				 int	Temp_Amb;		// temperatura ambiente
				 int	Temp_AirOut;	// temperatura aria in uscita
				 int	Temp_Big;		// temperatura tubo grande
				 int	Temp_Small;		// temperatura tubo piccolo
		unsigned int	Pres_Big;		// pressione tubo grande
		unsigned int	Pres_Small;		// pressione tubo piccolo

				 int	SP;				// SP richiesto
				 int	PowerLimit;		// limite di potenza da richiedere
				 int	OldPowerLimit;	// vecchio valore del limite di potenza da richiedere
				 int	Evap_Temp;		// temperatura di evaporazione richiesta
				 int	Evap_Press;		// pressione di evaporazione richiesta
				 int	Setp_PressLiqP_Caldo;	//SetPressione lavoro valvola in caldo

				 int	Function_Mode;	// modalit� di lavoro richiesta. Sovrascritta da touch se presente
				 int	DefMode;
				 int	Old_Function_Mode;	// modalit� di lavoro precedente
				 int	Ventil_Mode;	// modalit� di ventilazione richiesta. Sovrascritta dal touch se presente
				 int	Fan_MaxSpeed;	// Massima velocit� ventilatore calcolata
				 int	Pwm_Req;		// velocit� attualmente richiesta alla ventola (PWM request)
				 int	Pwm_Value;		// velocita attualmente richiesta alla ventola (PWM Value)
		unsigned int	Fan_Tacho;		// velocit� attuale del ventilatore
		unsigned int	Fan_Power;		// potenza attuale del ventilatore
		unsigned int	Fan_Current;	// corrente attuale del ventilatore 
		unsigned int	OutDefrosPower;	// potenza attuale della resistenza didefrosting
		unsigned int	OutDefrosCurrent;// corrente attuale della resistenza didefrosting
        unsigned int    Supply_Voltage;    
        unsigned int    Fan_Current_Limit;
        unsigned int    Fan_Current_Fast;
        unsigned int    Ventil_Selection;
				 int	ExpValve_Req;	// apertura richiesta alla valvola
				 int	ExpValve_Act;	// stato attuale della valvola
                 int    ExpValve_Ctrl;   // apertura richiesta dalla valvola in regolazione
                 int    ExpValve_Err;   // apertura richiesta dalla valvola in errore
                 int    ExpValve_Err_Code;  // codice di errore che richiede apertura valvola
                 int    ExpValve_Delta_Req; //Delta della valvola sommato al funzionamento per garantre l'apertura minima appena la valvola chiede apertura.
                 int    ExpValve_Delta_Act; //Delta della valvola sommato al funzionamento per garantre l'apertura minima appena la valvola chiede apertura.
                 int	CntValveFault;  // Contatore Fault della valvola  
				 int	InitValvolaInCorso;
                 int    ExpValve_Pid_Cooling;
                 
        unsigned int    RoomMaster_Status; // Flags di stato provenienti da Split Master (Room1)         
		unsigned int	CumulativeAlarm;

		unsigned int	SplitStatus;	// int con lo stato dello split
										// #bit0 => Clear Err Req
										// #bit1 => EEV Open
										// #bit2 => EEV Close
										// #bit3 => Batteria ghiacciata
										// #bit4 => Fan Fault
										// #bit5 => Fan OverLoad
										// #bit6 => EEV_Test_End
										// #bit7 => Battery defrost
										// #bit8 => 
										// #bit9 => 
										// #bit10 => 
										// #bit11 => 
										// #bit12 => 
										// #bit13 => 
										// #bit14 => 
										// #bit15 =>                                                
        unsigned int    MyOrSubSplitMode;
                 int    MyOrSubSplitEvap_Temp;
		unsigned int	CycleTime;
	
		unsigned char	Digi_In;		// stato ingressi digitali ausiliari
		unsigned char	Digi_Out;		// stato uscite digitali ausiliarie	
		unsigned char	My_Address;		// indirizzo dellp split
        unsigned char   My_SplitMaster_Address; //Indirizzo dello split Master verso i SubSlave        
		unsigned char	NetBSelect;	// Flag attivazione gestione Pannelli NetBuilding (Dip Switch n.8 = ON)
        unsigned char	SyxtSSelect;// Flag attivazione gestione Pannelli SyxtSense (Dip Switch n.7 = ON)
        unsigned char	ModBusDeviceDetected;// Flag x gestione rilevamento automatico device modbus collegato: 0= NoDevice connected, 1=Touch Exor/Carel connected, 2=Touch Syxthsense connected, 3=NetBuilding Panel connected
        unsigned char	EnableHiSpeedModBusTouch_Sec;
        unsigned char	EnableHiSpeedMyBus_Sec;        
				 int	Subcooling;				
				 int	Superheat;	
				 int	EvapLiq;
				 int    EvapTempValve;		
				 int	SuperheatRiscaldamento;
                 int    T_Big_Request;
                 int    Sub_Slave_Quantity; // Numero dei sotto Slave presenti sul BUS secondario
				 int	I_Am_SubSlave;		// Flag che avvisa allo SubSlave che � uno SubSlave (1=SUB)		
                 int    InputSystemDisable;
		unsigned int	Power_EcoMode;
		unsigned int	EcoModeEnable;
				 int	SuperHeat_SP;
				 //GL int	SuperHeat_FromMaster;
				 int	Freddo_TempBatt_SP;
				 int	Freddo_TempBatt_Min;
				 int	Caldo_TempBatt_SP;
                 int	Caldo_TempBatt_Min;
				 //int	Freddo_TempBatt_FromMaster;
				 //int	Caldo_TempBatt_FromMaster;
				 int	PerformanceLossBattery_SP;
                 int    Dinamic_SuperHeat;
                 int    TempAmbMiddle;
				 int	HumidityMiddle;
				 int	TempAmbRealSelection;
				 int	TempAmbTouchVisual;
                 int    Inverter_Supply;
                 int    TempAmb_Middle_Heating;
                 int    TempAmb_Middle_Cooling;
                 int    DoublePump_Abil;
                 int    SplitMode_Abil;
                 int    PanelTempAmb;
        unsigned int    FanNewStyleTimeOff;
        unsigned int    FanNewStyleTimeOn;
                char    SubSlaveAddressAutoreset;
        unsigned int    GoBusMergeMode;
        unsigned int    SetSubSlaveReset;
        unsigned int    AutoresetMagicNumber;
        unsigned int    AutoresetEnabled;
        unsigned int    MaxFanVentil_Mode;
        unsigned int    Humidity;
        unsigned int    Humidity_Correct;
                 int    CorrectSuperHeatAirOut_Local;
                 int    CorrectSuperHeatAirOut;
        unsigned int    SubSlaveSumPercValve;
        unsigned int    SubSlaveSumValvePidCooling;
        unsigned int    SubSlaveTotalFanPower;
        unsigned int    SubSlaveTotalAirPowerOutput;
        unsigned int    SubSlaveTotalAirPowerBTU;
        unsigned int    ExpValve_On_Demand_Priority;
        unsigned int    ExpValve_Cur_Number_Priority;
        unsigned int    ExpValve_On_Demand_Request;
        unsigned int    ExpValve_Sum_Tot_Percent;
        unsigned int    ExpValve_Sum_Req_Percent;
        unsigned int    ExpValve_Room_Number_Priority;
        unsigned int    ExpValve_On_Demand_Lim_Max;        
        unsigned int    ExpValve_Speed_Mode;                
        unsigned int    NetB_LCD_Intens;
        unsigned char   On_Priority_Mode;
        unsigned int    ValveOnDemand_Min_Percent_Val;
        unsigned int    ModDeviceType;
        unsigned int    EnableCommunication_Sec;        
        unsigned int    ModDeviceStatus;        
        unsigned int    ModDeviceEnableAndOffLine;
        unsigned int    ModDeviceAlarm;        
        
        unsigned int    Channel_Gas_Regulation;
        unsigned int    Channel_Liquid_Light;
        
        unsigned int    Fan_M3h;
        unsigned int    M3h_Min;
        unsigned int    M3h_Max;
        unsigned int    AirPowerOutput;
        unsigned int    AirPowerBTU;
        
        unsigned int    Min_Current_Fan;
        unsigned int    Max_Current_Fan;
        unsigned int    Efficiency_Fan;
        unsigned int    Supply_Fan;
        
        unsigned int    Pump_Work;
        unsigned int    SelectWorkingPump;

        unsigned int    EEPROM_SectorPage;

		unsigned int	HW_Version;				// REG_TOUCH_DIAG_SPLIT_HW_VERSION
		unsigned int	FW_Version_HW;			// REG_TOUCH_DIAG_SPLIT_FW_VERSION_HW                
        unsigned int	FW_Version_FW;			// REG_TOUCH_DIAG_SPLIT_FW_VERSION_FW
        unsigned int	FW_Revision_FW;			// REG_TOUCH_DIAG_SPLIT_FW_REVISION_FW		
		//DEBUGGONE
		//unsigned int	BroadcastRxCounter;
		//DEBUGGONE
                 int    SuperHeat_SP_Local;
                 int    Freddo_TempBatt_SP_Local;
                 int    Caldo_TempBatt_SP_Local;    
                 
        unsigned int	ComToUart3;
        
        unsigned int    HeaterFresh;
		
		struct 
		{	
            unsigned	ClearErrReq:1;			// Flag di richiesta di cancellazione degli errori EngineBox
            unsigned    ClearComErr:1;
			unsigned	Battery_Defrost:1;		// Flag che indica che il ventilatore della batteria sta girando per non farla ghiacciare
            unsigned    Battery_Drip:1;
			unsigned	EEV_Open:1;				// Flag di errore valvola aperta
			unsigned	EEV_Close:1;			// Flag di errore valvola chiusa
			unsigned	BatteriaGhiacciata:1;	// Flag di errore batteria ghiacciata
			unsigned	Fan_Fault:1;			// Flad di errore della velocit� del ventilatore
			unsigned	Fan_OverLoad:1;			// Flad di errore di sovraccarico del ventilatore
			unsigned	EEV_Test_End:1;			// Flag che indica la fine del test EEV
            unsigned    Flood_Alarm:1;          // Flag che indica l'errore di presenza acqua nella vaschetta FanCoil
            unsigned    GasLeaks:1;             // Flag che indica l'errore di presenza perdita gas
            unsigned	ClearSplitErrReq:1;     // Flag di richiesta di cancellazione degli errori Split
            unsigned    ClearDiagnosticSplitErrReq:1;   // Flag di richiesta di cancellazione degli errori Split dalla pagina di diagnostica split master
            unsigned    ReadMasReset:1;
            unsigned    FireAlarm:1;
            unsigned	EEPROM_ReadWrite:1;
            unsigned	EEPROM_Retention:1;          
            unsigned    EEPROM_Cell:1;
            unsigned    SPTemperatureFault:1;
            unsigned    TemperatureProbe:1;
            unsigned    Dampers:1;   
            unsigned    Flow:1;
            unsigned    Flow2:1;
        } Error;
		
        struct 
		{            
            unsigned    U1_MyBUS:1;
            unsigned    U2_MyBUS:1;
            unsigned    U1_ModBUS:1;
            unsigned    U2_ModBUS:1;
            unsigned    CumulativeAlarm:1;
        } ErrorLedBlink;
        
		struct 
		{	unsigned int	Split_Flags_Enable_1_16;		// Montaggio degli Enable dei sottoslave per diagnostica su touch
			unsigned int	Split_Flags_Offline_1_16;		// Montaggio degli Offline dei sottoslave per diagnostica su touch
			unsigned int	Split_Flags_Online_1_16;		// Montaggio degli Online dei sottoslave per diagnostica su touch
			unsigned int	Split_Flags_Alarm_1_16;		// Montaggio degli allarmi dei sottoslave per diagnostica su touch
            unsigned int    Split_Flags_EnableAndOffline_1_16; //Monitoraggio dei sottoslave enable ma offline
		} Room;        
        
		struct 
		{	unsigned int	Number;		
                     int	Temperature;	
			unsigned int	ST_On_Off;	
                     int	ST_SetP;	
                     int	ST_Fan;	
                     int    ST_UTAMode;
			unsigned int	CMD_On_Off;	
                     int	CMD_SetP;	
                     int	CMD_SetP_F;	
                     int	CMD_Fan;
                     int    CMD_UTAMode;
                     int	CMD_Send;
                     int    ReadOk;
            unsigned int    Only_Fan;
            unsigned int    Trigger_Init;
		} OtherCabin;        

		struct 
		{	
            unsigned int    Abil;
            unsigned int	Number;		
                     int	DeltaVel;
                     int    AutoVel;
		} Extractor;
        
		struct 
		{	
            unsigned int PercMaxOpenValve;
            unsigned int ValvolaAperta;		
            unsigned int ValvolaChiusa;		
            unsigned int ValvolaOff;          
            unsigned int Valve_Min_Go_Off;	
            unsigned int ValveLostCommCool;	
            unsigned int ValvoLostCommHot;                 
            unsigned int AperturaMaxCaldo;	
            unsigned int AperturaMinCaldo;	
            unsigned int InitValvolaFreddo;	
            unsigned int AperturaMaxFreddo;	
            unsigned int AperturaMinFreddo;	
            unsigned int ValveNoWorkMinFreddo;
            unsigned int ValveNoWorkMaxFreddo;
            unsigned int EEV_FluxTest;
            unsigned int ValvolaFrozen;
            
            struct 
            {	
                unsigned int PercMaxOpenValve;                
                unsigned int ValvolaAperta;		
                unsigned int ValvolaChiusa;		
                unsigned int ValvolaOff;          
                unsigned int Valve_Min_Go_Off;	
                unsigned int ValveLostCommCool;	
                unsigned int ValvoLostCommHot;                 
                unsigned int AperturaMaxCaldo;	
                unsigned int AperturaMinCaldo;	
                unsigned int InitValvolaFreddo;	
                unsigned int AperturaMaxFreddo;	
                unsigned int AperturaMinFreddo;	
                unsigned int ValveNoWorkMinFreddo;
                unsigned int ValveNoWorkMaxFreddo;
                unsigned int AperturaMaxFreddo_Local;
                unsigned int ValvolaFrozen;
                unsigned int AperturaMinFreddo_Local;
                unsigned int AperturaMinCaldo_Local;
                unsigned int AperturaMaxCaldo_Local;
            } MasterRx;
		} ValveLim;      
        
        
		struct 
		{	
            unsigned int	 Mode;           // Modalit�: bit0=1 -> Frigo,   bit1=1 -> Freezer
                     int	 Temperature;	
                unsigned int PercMaxOpenValve;                
                unsigned int ValvolaAperta;		
                unsigned int ValvolaChiusa;	              
                unsigned int InitValvola;	
                unsigned int AperturaMax;	
                unsigned int AperturaMin;	                
		} Frozen;                
        
		struct 
		{	
            unsigned int	CntComErrSec;			
            unsigned int    TimeOutComErrSec;		
            unsigned int	TimeOutInterCharComErrSec;
            unsigned int	CRC_ComErrSec;
            unsigned int	ModCntComErrSec;			
            unsigned int	ModTimeOutComErrSec;		
            unsigned int	ModTimeOutInterCharComErrSec;
            unsigned int	ModCRC_ComErrSec;
		} ComError;             
        
	} TypSplit;

	typedef struct
	{
				 int	Temp_Amb;		// temperatura ambiente
				 int	Temp_AirOut;	// temperatura aria in uscita
				 int	Temp_Big;		// temperatura tubo grande
				 int	Temp_Small;		// temperatura tubo piccolo
				 int	Pres_Big;		// pressione tubo grande
				 int	Pres_Small;		// pressione tubo piccolo

				 int	SP;				// SP richiesto
				 int	Evap_Temp;		// temperatura di evaporazione richiesta
                 int	Evap_Press;		// presione di evaporazione richiesta
                 
                 int	EvapTempValve;	// temperatura di evaporazione richiesta valvola
                        
				 int	Function_Mode;	// modalit� di lavoro richiesta. Sovrascritta da touch se presente
				 int	Ventil_Mode;	// modalit� di ventilazione richiesta. Sovrascritta dal touch se presente

				 int	ExpValve_Req;	// apertura richiesta alla valvola
				 int	ExpValve_Act;	// stato attuale della valvola
                 int    ExpValve_Err;   // apertura richiesta dalla valvola in errore
                 int    ExpValve_Err_Code;  // codice di errore che richiede apertura valvola
                 int    ExpValve_Pid_Cooling;                 
                 int	CntValveFault;  // Contatore Fault della valvola  
                 int    FanSpeed;
				 int	Superheat;			
				 int	SuperheatRiscaldamento;
                 int    SetP_Superheat;
        unsigned int    CycleTime;
                 
        unsigned int    CumulativeAlarm;
				 
		unsigned int	SplitStatus;	
        
		unsigned int	Fan_Power;	
        unsigned int	Fan_Current;	
        unsigned int	Supply_Voltage;	
        unsigned int	FrozenMode;	
                 int    SuperHeat_Pid_Cool_Error;
        unsigned int    Temperature_Probe_error;

        unsigned int    Fan_M3h;
        unsigned int    AirPowerOutput;
        unsigned int    AirPowerBTU;

        unsigned int    Humidity;
       
		unsigned int	HW_Version;				
		unsigned int	FW_Version_HW;			              
        unsigned int	FW_Version_FW;			
        unsigned int	FW_Revision_FW;			  
        unsigned int    Model_FW_Version;

		unsigned		 Enable:1;
		unsigned		 OffLine:1;
		unsigned		 OnLine:1;
        
	} TypSplitSlave;

/*
	typedef struct
	{			 int	Ch1;
			 	 int	Ch2;
			 	 int	Ch3;
			 	 int	Ch4;
			 	 int	Ch5;
			 	 int	Ch6;
			 	 int	HoldCh1;
			 	 int	HoldCh2;
			 	 int	HoldCh3;
			 	 int	HoldCh4;
			 	 int	HoldCh5;
			 	 int	HoldCh6;
		unsigned		RefreshT:1;					// richiesta refresh touch
		unsigned 		Enable:1;					// se � abilitato
		unsigned		OffLine:1;					// se non ha risposto
		unsigned		RequestRefresh:1;			// ho appena aggiornato un dato ed � necessario aggiornare l'usicta dell'inverter
	} TypDimmer;
*/
	typedef struct
	{			 
                 int	Temp_Mandata;
				 int	Temp_Ritorno;
				 int	Temp_Condensatore;
				 int	Temp_Acqua_Mare;
				 int	Temp_Collettore_Grande;
				 int	Temp_Collettore_Piccolo;
		unsigned int	Pressione_Gas_G;			// Pressione in mBar tubo grande (Gas)
		unsigned int	Pressione_Liq_P;			// Pressione in mBar tubo piccolo (Liquido)
		unsigned int	Pressione_VRV_Box;			// Pressione in mBar del VRV
		unsigned int	Pressione_Aux;				// Pressione in mBar AUX
				 int	Ric_Temp;					// Temperatura richiesta al compressore
		unsigned int	Ric_Pressione;				// pressione richiesta al compressore
				 int	Compressor_Speed;			// richiesta da parte del programma di gestione dell'inverter
				 int	Pump_Speed;					// velocit� della pompa dell'acqua
				 int	Water_Flux;					// Flusso dell'acqua nel condensatore
				 int	PowerLimit;					// massima potenza disponibile per l'inverter
				 int	OldPowerLimit;				// memoria della massima potenza disponibile per l'inverter
				 int	FunctionMode;				// modalit� del box motore
													// Mode 0 => Off
													// Mode 1 => Shutting down
													// Mode 2 => Freddo
													// Mode 3 => Go Freddo
													// Mode 4 => Caldo
													// Mode 5 => Go Caldo
				 char	StatoCompressore;			// stato della macchina a stati di avvio della macchina
				 char	IdMasterSplit;				// id dello split pi� gravoso
		unsigned		InverterRaffrescamento:1;	// Modalit� di funzionamento inverter in raffrescamento
		unsigned		InverterRiscaldamento:1;	// Modalit� di funzionamento inverter in riscaldamento
		unsigned		GoRiscaldamento:1;			// Flag di richiesta avvio riscaldamento
		unsigned		GoRaffrescamento:1;			// Flag di richiesta avvio raffrescamento
		unsigned		GoShutDwComp:1;				// Flag di richiesta spegnimento
		unsigned		CumErr:1;					// Flag cumulativo di errore
		
				 int	FunctionMode_C2;			// Gestione secondo compressore
				 int	Temp_Compressor_Output_C2;	// Gestione secondo compressore
	    unsigned int	CumulativeError_C2;			// Cumulativo errori (bit0=Normali + bit1=Persistenti) proveniente dal Master
				 char	StatoCompressore_C2;		// stato della macchina a stati di avvio della macchina
		unsigned int	DoubleCompressorOn;			
				 int	Compressor_Speed_C2;
				 int	Max_Comp_Quantity;
				 int	SystemDisable;				//Disabilita il sistema di condizionamento quando il valore==1
				 int	Pump_Speed_Perc;
		unsigned int	MasterRestartTime;		 
        unsigned int    RealFuncMode;               // Modalit� di funzionamento unificata tra i due compressori
        
                 int    SuperHeat_CoolingCalc;
                 int    K_SetPoint_Cooling_SuperHeat;
                 int    SuperHeat_Correct_Cooling;
                 int    Condenser_Press_Abil;
                 
        unsigned int    Total_Fan_Absorption;
        unsigned int    Compressor1_Absorption;
        unsigned int    Compressor2_Absorption;
        unsigned int    Sea_Water_Pump_Absorption;
        
        unsigned int    Total_AirPowerOutput;
        unsigned int    Total_AirPower_BTU;
        unsigned int    System_Efficency_EER;

        unsigned int    Inverter_Upper_Out_Freq;
        
        unsigned int    TestAll_Abil;
                 int    TestAll_TempBatt_SP_Heat;
                 int    TestAll_SuperHeat_SP_Cool;
                 int    TestAll_TempBatt_SP_Cool;
                 int    TestAll_MaxOpValve_Cool;
                 
                 int    TestAll_SuperHeat_Cool_pGain;
                 int    TestAll_SuperHeat_Cool_iGain;
                 int    TestAll_SuperHeat_Cool_dGain;
                 int    TestAll_SuperHeat_Cool_Time_Exec_PID;

                         
                 int    Frozen_Abil;
                 int    Frozen_On;                 
				 
        unsigned int    TestAll_Frozen_Abil;
                 int    TestAll_TempBatt_SP_Frigo;
                 int    TestAll_TempBatt_SP_Freezer;
                 int    TestAll_SuperHeat_SP_Frigo;
                 int    TestAll_SuperHeat_SP_Freezer;
                 int    TestAll_MaxOpValve_Frigo;                   
                 int    TestAll_MaxOpValve_Freezer;
                 
                 int    TestAll_MinOpValve_Cool;
                 int    TestAll_MinOpValve_Heat;
                 int    TestAll_MinOpValve_Frigo;
                 int    TestAll_MinOpValve_Freezer;                 
                 
                int     Abil_Defrosting;
                int     Defrosting_Step_Time;
                int     Defrosting_Time;      
                int     Defrosting_Temp_Set;        
                int     Dripping_Time;                 
                                 
		struct 
		{	
            unsigned	CompressorHi:1;				// Flag di richiesta avvio procedura di recovery per alta temp
			unsigned	CompressorLo:1;				// Flag di richiesta avvio procedura di recovery per bassa temp
			unsigned	CondensatoreHi:1;			// Flag del ciclo di sbrinamento, quado ghiaccia il condensatore
			unsigned	CondensatoreLo:1;			// Flag di richiesta avvio procedura di recovery per alta temperatura condensatore
			unsigned	Gas_Recovery:1;				// Flad del ciclo di recupero del gas
			unsigned	Pressure_Lo:1;				// Flag di avvio procedura recovery per bassa pressione
			unsigned	Pressure_Hi:1;				// Flag di avvio procedura recovery per alta pressione
			unsigned	Recharge_Oil:1;				// Flag di avvio procedura di recupero dell'olio motore
			unsigned	CompressorHi_C2:1;
            unsigned    Defrosting:1;
            unsigned    MasterRestart:1;
            unsigned    Condenser_Fouled:1;
            unsigned    SuperHeat:1;
            unsigned    TemperatureProbe:1;
		} Error;

		struct 
		{	
            unsigned int	ErrorMode;			
            unsigned int	CntComErr;			
            unsigned int    TimeOutComErr;		
            unsigned int	CRC_ComErr;
            unsigned int	ModCntComErr;			
            unsigned int	ModTimeOutComErr;		
            unsigned int	ModCRC_ComErr;
            unsigned int	ModCntComErr_U2;			
            unsigned int	ModTimeOutComErr_U2;		
            unsigned int	ModCRC_ComErr_U2;
            unsigned int	CntComErr_U3;
            unsigned int	TimeOutComErr_U3;
            unsigned int	CRC_ComErr_U3;        
		} ComError;               
        
		struct 
		{	
            unsigned	CompressorHi:1;				// Flag di richiesta avvio procedura di recovery per alta temp
			unsigned	CompressorLo:1;				// Flag di richiesta avvio procedura di recovery per bassa temp
			unsigned	CondensatoreHi:1;			// Flag del ciclo di sbrinamento, quado ghiaccia il condensatore
			unsigned	CondensatoreLo:1;			// Flag di richiesta avvio procedura di recovery per alta temperatura condensatore
			unsigned	Gas_Recovery:1;				// Flad del ciclo di recupero del gas
			unsigned	Allagamento:1;				// stato dell'ingressio della sonda di allagamento
			unsigned	TermicaCompressore:1;		// stato della termica sul compressore
			unsigned	SeaWaterLo:1;				// Temp acqua di mare troppo fredda
			unsigned	SeaWaterHi:1;				// Temp acqua di mare troppo calda
			unsigned	CaricaRefrig:1;				// Flag che indica la mancanza di gas
			unsigned	WaterSeaFlux:1;				// Flag dell'allarme di flusso del condensatore
			unsigned	EEV_Open:1;					// Flag cumulativo di una EEV bloccata aperta nell'impianto
			unsigned	EEV_Close:1;				// Flag cumulativo di una EEV bloccata chiusa nell'impianto
			unsigned	BatteriaGhiacciata:1;		// Flag cumulativo di una batteria ghiacciata nell'impianto
			unsigned	Fan_Speed:1;				// Flag cumulativo di un ventilatore che non gira
			unsigned	Fan_OverLoad:1;				// Flag cumulativo di un ventilatore in sovraccarico
			unsigned	Acceleration_Error:1;
			unsigned	Acceleration_Error_C2:1;
			unsigned	CompressorHi_C2:1;			
			unsigned	TermicaCompressore_C2:1;
            unsigned    SuperHeat:1;
            unsigned    FireAlarm:1;
		} PersErr;

		struct
		{	
            unsigned	FluxReq:1;					// Flag che indica lo svolgimento del test o la richiesta
			unsigned	EEV_Req:1;					// Flag che indica lo svolgimento del test o la richiesta
			unsigned	EEV_Go:1;					// Flag che indica agli split di procedere con il test delle valvole
		} Test;

		struct
		{	
            unsigned	CompressorHi:1;
			unsigned	CompressorLo:1;
			unsigned	TermicaComp:1;
			unsigned	Allagamento:1;
			unsigned	ClearErrorReq:1;
		} In;
		unsigned int	InputSt;					// cumulativo degli ingressi

		struct
		{	
            unsigned	ValvolaAspirazioneVRV:1;	// Rele marcia arresto, utilizzato per il controllo della valvola sull'aspirazione del motore per il vrv
			unsigned	Valvola_Caldo:1;			// Valvola caldo/freddo
			unsigned	Pompa_Acqua:1;				// Pompa di ricircolo acqua
			unsigned	ReleAux1:1;					// Aux 1
			unsigned	ReleAux2:1;					// Aux 2
		} Out;
		unsigned int	ReleSt;						// cumulativo dei rele

		unsigned int	Errori1;					// Errori NON Persistenti 1
        unsigned int    Errori2;                    // Errori NON Persistenti 2
		unsigned int	ErroriPers1;				// Errori Persistenti 1
		unsigned int	ErroriPers2;				// Errori Persistenti 2
		unsigned int	Errori3;					// Errori NON Persistenti Split
		unsigned int	CumulativeError;			// Cumulativo errori (bit0=Normali + bit1=Persistenti) proveniente dal Master

		unsigned int	SysNumLo;					// integer con gli errori:
		unsigned int	SysNumHi;					// integer con gli errori:
		unsigned int	SystemCycleTime;			// Tempo di ciclo unita master x diagnostica
		unsigned int	HoursOfLife;				// Ore di vita del Master x diagnostica
        unsigned int    Touch_Nation;               // Identificativo Nazione da inviare su Touch per localizzazione pagine/oggetti 
                                                    // bit.0=USA, bit.1=EUR
		int				Acceleration_Inverter;
        int				Acceleration_GasPressure;	
		unsigned int	PowerBoxAbsorption;

		unsigned int	Threshold_Compressor_Hi;
 
        unsigned int    HW_Version;					// REG_TOUCH_MASTER_HW_VERSION
		unsigned int	FW_Version_HW;				// REG_TOUCH_MASTER_FW_VERSION_HW                
		unsigned int	FW_Version_FW;				// REG_TOUCH_MASTER_FW_VERSION_FW
		unsigned int	FW_Revision_FW;				// REG_TOUCH_MASTER_FW_REVISION_FW		
		unsigned int	DiagAddress;
		unsigned int	Eco_Mode_Status;
        unsigned int    InVolt_C1;
        unsigned int    OutPower_C1;
        unsigned int    InVolt_C2;
        unsigned int    OutPower_C2;    
        unsigned int    Inverter1CodeFault;    
        unsigned int    Inverter2CodeFault;    
        unsigned int    Inverter3CodeFault;    
        unsigned int    Inverter4CodeFault;    
		
        
        int             DoublePumpSwitchTime;
        unsigned int    DoublePumpSelection;

        unsigned int    Manual_Pump_Abil;
		unsigned int    Manual_Pump_Freq;
        
        int             SuperH_HeatCalc;
        int             Pressure_LiquidCond; //PressW_HeatCalc;
        int             SuperH_Pid_Error;
        int             AllValveOpen100p;
        
        unsigned int    AbilValveOnDemand;
        unsigned int    AbilScoreOnDemand;

        unsigned int    Maintenance_ReadMasReset:1;
        
        unsigned int    PressureLoLiqValue;				                          
        unsigned int    PressureLoGasValue;
		unsigned int    PressureHiLiqValue;  
		unsigned int    PressureHiGasValue;         
        unsigned int    Lim_Max_Press_Correct_Valve;

                 int    SuperHeat_Heat_Min_SetPoint;
                 int    SuperHeat_Heat_Max_SetPoint;
        
        unsigned int    Temperature_Probe_Error;
 
                 int    Dehumi_Abil;
                 int    Set_Humi;
                 int    Set_Delta_Temp_Min_Dehumi;
                 int    Set_Fan_Speed_Dehumi;
                 int    Set_Press_Dehumi;
                 int    Set_Temp_Air_Out;
                 int    Set_Temp_Air_Out_Dehumi;
        
        unsigned int AddressGasLeaks;
            
		struct 
		{	 
			int EngineCompSpeed;	            		                        	
			int EngineFanSpeed;				                           
			int EngineReqPressure;	                            			
			int EngineTempOut;                     					                        				
			int EngineTempIn;			                          		                        				
			int EngineTempAir;			                          		                        				
			int EngineTempBatt;		                          		                        			
			int EngineTempGas;		                          		                        				
			int EngineTempLiq;		              					                        	
			int EnginePressGas;		                          		                        		
			int EnginePressLiq;		              			
			int EngineBoxErr1;			                          		                        	    
			int EngineBoxPersErr1;	                          		                        	    
			int EngineBoxSplitErr1;	                          		                        					
			int Absorption;				                          		                        	    
			int CumulativeAlarm;		                          				
			int Split_LiquidTemp;		                          				
			int Split_GasTemp;			                          		                        		
			int Split_AirInTemp;			                          		                        			
			int Split_AitOutTemp;		                          		                        			
			int Split_Exp_Valve;		              					                        					
			int Split_Humi;				              				    
			int Split_ReqAirTemp;		                          		                         
			int Split_ReqGasPress;	                          		                        			
			int Split_Superheat;		              					                        				
			int Split_HW_Ver;			                          		                        
			int Split_FW_Ver_HW;	              					                        		
			int Split_FW_Ver_FW;	              					                        		
			int Split_FW_Rev_FW;	              					                        							
			int HW_Ver;					                          		                        					
			int FW_Ver_HW;			              					                        					
			int FW_Ver_FW;			              					                       					
			int FW_Rev_FW;			              					                       							
			int UTASP_On;    
			int StatoCompressore; 		                          
			int IdMasterSplit;
			int Mode; 		
			int ClearError;
		} UTA_H;        
		
		struct 
		{	 		
			unsigned int FlagsEnable_1_16;
			unsigned int FlagsOffLine_1_16;
			unsigned int FlagsOnLine_1_16;
			unsigned int FlagsAlarm_1_16;
			unsigned int FlagsEnable_17_32;
			unsigned int FlagsOffLine_17_32;
			unsigned int FlagsOnLine_17_32;
			unsigned int FlagsAlarm_17_32;
            
            struct
            {
                unsigned int FlagsEnable_1_16;
                unsigned int FlagsOffLine_1_16;
                unsigned int FlagsOnLine_1_16;
                unsigned int FlagsAlarm_1_16;
                unsigned int FlagsEnable_17_32;
                unsigned int FlagsOffLine_17_32;
                unsigned int FlagsOnLine_17_32;
                unsigned int FlagsAlarm_17_32;  
                unsigned int AddrType;
            } ModDev;            
            
		} Split;        
		
        struct
		{
            int         SuperHeat_SP_Cool;
			int         SuperHeat_SP_Heat;
			int         MaxOpValve_Cool;
			int         MaxOpValve_Heat;
			int         TempBatt_SP_Cool;
			int         TempBatt_SP_Heat;

            int         SuperHeat_SP_Frigo;
			int         SuperHeat_SP_Freezer;
			int         MaxOpValve_Frigo;
			int         MaxOpValve_Freezer;
			int         TempBatt_SP_Frigo;
			int         TempBatt_SP_Freezer;
            
            int         MinOpValve_Cool;
            int         MinOpValve_Heat;
            int         MinOpValve_Frigo;
            int         MinOpValve_Freezer;
            int         SuperHeat_Custom_Par_Cool_PID;
            int         SuperHeat_Cool_Correct;
            int         SuperHeat_Heat_Correct;
            int         Split_Probe_Error;
            int         Master_Probe_Error;
		} TestAllAbil;	        
        
		
	} TypEngineBox;
	
	typedef struct
	{			
		struct 
		{	 
			int SetPoint;
			int FanSpeed;
			int Power;
			int SetPoint_F;
			int TrigInit;
			int DefMode;
		} ToEngBox;

		struct 
		{	 
			int SetPoint;
			int FanSpeed;
			int Power;
			int SetPoint_F;
			int TrigInit;
			int DefMode;
            int UTA_Mode;			
            int HeaterPwr;
		} FromEngBox;

	} TypInitSplit;
		

	//---------------------------------------
	// Variabili 
	//---------------------------------------
	IncludeVariabiliGlobaliCore();

	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	void NetB2Touch(void);
	//void SyxtS2Touch(void);
	void FunctionMode(void);
	unsigned int PressureGas(int ReqTemp);
    int PressureGasToTempR410A(unsigned int ReqPress);  
    unsigned int TempToPressureGasR410A(int ReqTemp); 
	void TempEvaporazione(void);
	void RegolaValvola(void);
    void RealFunctionMode(void);
	int TempAmbMidSel(void);
    void HeaterControl(void);
    float ValueIn2ValueOut(float Vin, float VinMin, float VinMax, float VoutMin, float VoutMax);
    void DefrostingCycle(void);      
    int ValueToPercent(int Value, int MaxValue);
    void GasValveControl(void);
    void FloodFanCoilAlarmCycle(void);
    void CheckErrorTemperatureProbe(void);
    int CheckProbeFreeze(int Value, int * PrevVal, int * CntErr, unsigned int MaxCnt);
    void DampersAlarm(void);
    void GasLeaksAlarmCycle(void);
    //int ValueIn2ValueOut(int Vin, int VinMin, int VinMax, int VoutMin, int VoutMax);
    //unsigned int ValueIn2ValueOut(unsigned int Vin, unsigned int VinMin, unsigned int VinMax, unsigned int VoutMin, unsigned int VoutMax);
#endif
