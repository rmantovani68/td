//----------------------------------------------------------------------------------
//	Progect name:	Core.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			Emanuele
//	Date:			22/05/2011
//	Description:	Header delle funzioni della logica di funzionamento
//----------------------------------------------------------------------------------


#ifndef _CORE_ELABORAZIONE_H_
	#define _CORE_ELABORAZIONE_H_

#include "ProtocolloComunicazione.h"
#include "FWSelection.h"
#include "ProtocolloModBus.h"
#include "Timer.h"

	//----------------------------------------------------------------------------------
	//	Define
	//----------------------------------------------------------------------------------
	#define	k_Max_HoursOfLife				60*24		// 1440h = 60gg


	#define	RefreshTime						500

	// define modalità di funzionamento da touch
	#define	CoreOff							0
	#define	CoreAuto						2
	#define	CoreRiscaldamento				4//0x0004
	#define	CoreRaffrescamento				8//0x0008
	#define CoreVentilazione				16//0x0010
#if (K_AbilMODBCAN==1)  
	#define CoreDehumi      				32
#endif

	// define modalità di funzionamento engine box
	#define EngineBox_Off					0
	#define EngineBox_GoOff                 1
	#define EngineBox_Freddo				2
	#define EngineBox_GoFreddo				3
	#define EngineBox_Caldo                 4
	#define EngineBox_GoCaldo				5
	#define EngineBox_CriticalFault         6

	// define modalità di funzionamento pompa
	#define PumpSelection_Auto              1
	#define PumpSelection_1					2
	#define PumpSelection_2                 4


	// regolazione potenza compressore
	#define	Off_Speed						0		// valore di min dell'adc per il compressore (spento)
	#define Min_Speed						50 //40		// valore per la partenza del compressore (40Hz)
	#define	Max_Speed						255		// valore massimo per l'inverter (120Hz)
/*
#if(K_MotorModel_Selection1==K_MotorType_DA550A3F11MD)
    #define	Max_Speed_Cooling				(Max_Speed * 85) / 100		// valore massimo per l'inverter in freddo (100%)
	#define	Max_Speed_Heating				(Max_Speed * 65) / 100		// valore massimo per l'inverter in caldo (40%)
#else
    #define	Max_Speed_Cooling				(Max_Speed * 65) / 100		// valore massimo per l'inverter in freddo (100%)
	#define	Max_Speed_Heating				(Max_Speed * 45) / 100		// valore massimo per l'inverter in caldo (40%)
#endif
 */
	#define	Soglia_Inverter					250		// soglia di isteresi in mBar (1bit = 44mBar) per la regolazione dell'inverter
	#define	Soglia_Inverter_Caldo_HI		200		// soglia di isteresi in mBar (1bit = 44mBar) per la regolazione dell'inverter
	#define	Soglia_Inverter_Caldo_LO		200 //2500	// soglia di isteresi in mBar (1bit = 44mBar) per la regolazione dell'inverter
	#define	Soglia_Inverter_Freddo_HI		200 //2500	// soglia di isteresi in mBar (1bit = 44mBar) per la regolazione dell'inverter
	#define	Soglia_Inverter_Freddo_LO		200		// soglia di isteresi in mBar (1bit = 44mBar) per la regolazione dell'inverter
	#define	TempStartCompCaldo				2999	//2499	// temp da raggiungere all'avvio per riscaldare (16,5 Bar)
	#define TempStartCompFreddo				1000	// temp da raggiungere all'avio per raffreddare
/*
#if (K_SIMULATION_WORK==1)
	#define	WaitTimeRestart					30		// tempo in secondi prima del riavvio (minimo 120 secondi, un minuto prima si ferma il compressore)
#else	
	#define	WaitTimeRestart					120		// tempo in secondi prima del riavvio (minimo 120 secondi, un minuto prima si ferma il compressore)
#endif
*/
	#define	WaitErrorRestart				15		// tempo in secondi prima del riavvio (minimo 120 secondi, un minuto prima si ferma il compressore)
	#define PressStartLiqFreddo				17000

	#define K_Time_Restart_After_Blackout	180

#if (K_SIMULATION_WORK==1)
	#define	K_TimeSwitchCompressor			1000//3600//1000 //3600
	#define	K_TimeIntegrateCompressor2		120//10
	#define	K_TimeDeintegrateCompressor2	180//30
#else
	#define	K_TimeSwitchCompressor			3600
#ifndef K_TimeIntegrateCompressor2
	#define	K_TimeIntegrateCompressor2		120
#endif
#ifndef K_TimeDeintegrateCompressor2
	#define	K_TimeDeintegrateCompressor2	180
#endif
#endif
/*
#if (K_SIMULATION_WORK==1)
	#define	K_Perc_Ok_Compressor2			(Max_Speed * 50) / 100	// Integra compressore2 dopo 50% di Out comp1
	#define	K_Perc_No_Compressor2			(Max_Speed * 30) / 100	// Deintegra compressore2 dopo 30% di Out comp1 NOTA: non andare sotto il 16% perchè corrisponde al minimo valore di out inverter ("Min_Speed"))
#else
#if(K_MotorModel_Selection1==K_MotorType_DA550A3F10M)
	#define	K_Perc_Ok_Compressor2			(Max_Speed * 40) / 100	// Integra compressore2 dopo 50% di Out comp1
	#define	K_Perc_No_Compressor2			(Max_Speed * 25) / 100	// Deintegra compressore2 dopo 30% di Out comp1
#else
	#define	K_Perc_Ok_Compressor2			(Max_Speed * 35) / 100	// Integra compressore2 dopo 50% di Out comp1
	#define	K_Perc_No_Compressor2			(Max_Speed * 25) / 100	// Deintegra compressore2 dopo 30% di Out comp1
//	#define	K_Perc_Ok_Compressor2			(Max_Speed * 50) / 100	// Integra compressore2 dopo 50% di Out comp1
//	#define	K_Perc_No_Compressor2			(Max_Speed * 30) / 100	// Deintegra compressore2 dopo 30% di Out comp1
#endif
#endif
*/
	#define	K_RecOil_GasG_Press				29000	// 
	#define K_RecOil_LiqP_Press				22000	// 

	#define	TimeSlowSlewRate				1000//2000	// normal mode
	#define	TimeFastSlewRate				2000	// fast mode
	#define	TimeStartColdSlewRate			4000	// start up
	#define	TimeStartHotSlewRate			6000	// start up
	#define	TimeErrLowPriorSlewRate			4000	// errore secondario
	#define TimeShutDown					2500    //700		// Durante lo spegnimento
	#define	TimeEmergencySlewRate			1200//1200	//500		// Emergency 
    #define	TimeEmergencyPressureSlewRate	1500//1000//500 //2500	//500		// ** Emergency solo per Pressure_Hi & Pressure_Lo   
	#define	TimeIncOutInverter_Cooling		4//2       // normal mode Secondi
	#define	TimeIncOutInverter_Heating		6       // normal mode  Secondi
	#define	TimeIncOutInverter_PowerReduce	2       // Power Reduce Mode mode  Secondi
	#define	TimeIncOutInverter_ValueInRange 2       // Value In Range mode  Secondi
	#define	IncOutInverter					2		// incremento dell'uscita dell'inverer
	#define EmergencyStepVariation			2//3		// variazione dell'inverter in caso di emergenza
    #define EmergencyPressureStepVariation	2//3 //2		// ** variazione dell'inverter in caso di emergenza solo per Pressure_Hi & Pressure_Lo
    #define IncOutInverter_Low_Critical     1       // variazione con errore secondario

	#define	TimeRecOilUpSlewRate			1000	// Velocità di salita compressore per ciclo recupero olio
	#define RecOilUpStepVariation			2		// step variazione passi compressore salita recupero olio
	#define	TimeRecOilDwSlewRate			1000	// Velocità di discesa compressore per ciclo recupero olio
	#define RecOilDwStepVariation			2		// step variazione passi compressore discesa recupero olio

	// parametri cicli errore alta pressione mandata compressore
#if (NEWHW==1)
	#define	TempCriticalCompressorHi		8000
#ifndef TempCompressorHI
	#define	TempCompressorHI				7200//7100    // Temp limite della mandata del compressore. Genera l'allarme CompressorHi.
#endif
	#define	TempRestarCompressorHI			TempCompressorHI-200 // Temp di riarmo per la mandata del comp.
#else
	#define	TempCriticalCompressorHi		6800
	#define	TempCompressorHI				6500    // Temp limite della mandata del compressore. Genera l'allarme CompressorHi. ATTENZIONE: NON ALZARE QUESTO DATO PERCHè IL VECCHIO HW ARRIVA A 70C MASSIMO COME TEMPERATURA
	#define	TempRestarCompressorHI			6000    // Temp di riarmo per la mandata del comp.
#endif
	#define	TimeCompressorHi				120//300  //30//180		// Time Out dell'errore, oltre il quale spengo.
	#define	TimeResCompressorHi				300	//1200	// Time Out del reset errore, oltre il quale azzero se non si presenta in questi minuti.
	#define	SpeedCompressorHI				Min_Speed	//85		// Velocità massima da mantenere durante l'errore.
	// parametri cicli errore bassa pressione ritorno compressore
#if(K_MotorModel_Selection1==K_MotorType_DA550A3F10M)
    #define	TempCompressorLoRaffr			-2500//-700	// Temp limite aspirazione compressore. Genera l'allarme CompressorLo in raffrescamento.
	#define	TempCompressorLoRisc			-2500//-1000	// Temp limite aspirazione compressore. Genera l'allarme CompressorLo in riscaldamento.
	#define	TempRestartCompressorLo			-2000//0		// Temp di riarmo per l'aspirazione del comp.
#else
    #define	TempCompressorLoRaffr			-1200//-700	// Temp limite aspirazione compressore. Genera l'allarme CompressorLo in raffrescamento.
	#define	TempCompressorLoRisc			-1200//-1000	// Temp limite aspirazione compressore. Genera l'allarme CompressorLo in riscaldamento.
	#define	TempRestartCompressorLo			-900//0		// Temp di riarmo per l'aspirazione del comp.
#endif
	#define PressCompressorLo				2000	// Pressione limite per l'aspirazione del comp.Genera il pers error CompressorLo.
	#define PressRestartCompressorLo		5000	// Pressione di riarmo del gas	DA CHIEDERE A MAURI
	#define	TimeCompressorLo				30//180		// Time Out dell'errore, oltre il quale spengo.
	#define	TimeResCompressorLo				300	//1200	// Time Out del reset errore, oltre il quale azzero se non si presenta in questi minuti.
	#define	SpeedCompressorLoRaff			Min_Speed	//127		// Velocità massima da mantenere durante l'errore in rafrescamento
	#define	SpeedCompressorLoRisc			Min_Speed	//75		// Velocità massima da mantenere durante l'errore ibn riscaldamento
	
	// parametri cicli errore surriscaldamento del condensatore
	#define TempCondensatoreHi					4500	// Temp limite superiore condensatore. Geneera l'allarme CondensatoreHi.
	#define TempRestartCondensatoreHi			3800 //4000	// Temp mandata al di sotto della quale riarmo l'allarme.		DA CHIEDERE A MAURI
//	#define PressCondensatoreHi					24000 //25000	// Pressione liq limite. Geneera l'allarme CondensatoreHi.
//	#define	PressRestartCondensatoreHi			23000 //** 22000	// Pressione liq al di sotto del quale riparte il funzionamento normale.
	#define	TimeCondensatoreHi					180		// Time Out dell'errore, oltre il quale spengo.
	#define	TimeResCondensatoreHi				1200	// Time Out del reset errore, oltre il quale azzero se non si presenta in questi minuti.
	#define	SpeedCondensatoreHi					Min_Speed	// Velocità massima da mantenere durante l'errore.

	// parametri cicli errore bassa temperatura del condensatore
	#define	TempCondensatoreLo				100 //300		// Temp limite oltre la quale viene generato l'allarme.
	#define	TempErrCondLo					500//300		// Errore limite accettato per errore bassa condensatore
	#define	TempRestartErrCondLo			300//300		// Errore limite accettato per errore bassa condensatore
	#define	TempRestartCondensatoreLo		400//200		// Temperatura di riarmo del blocco.
	#define	TimeCondensatoreLo				180		// Time Out dell'errore, oltre il quale spengo.
	#define	TimeResCondensatoreLo			2400//1200	// Time Out del reset errore, oltre il quale azzero se non si presenta in questi minuti.
	#define	SpeedCondensatoreLo				0		// Velocità massima da mantenere durante l'errore.

	//  parametri ciclo di recupero del gas
	#define Gas_Recovery_PressureLimit		20000	// pressione limite al di sotto della quale occorre eseguire il ciclo di recovery
	#define Liq_Recovery_PressureLimit		4000	// pressione limite al di sotto della quale occorre eseguire il ciclo di recovery
	#define Gas_Recovery_PressureRestore	5000	// pressione da controllare sul tubo piccolo per poter ripristinare il funzionamento normale
	#define	SpeedGasRecovery        		Min_Speed		// velocità alla quale va ilmotore in errore
    #define	Gas_Recovery_SpeedLimit			190     // velocità oltre la quale interviene se non è in pressione
	#define	Time_Gas_Recovery				180		// Time Out dell'errore, oltre il quale spengo.
	#define	TimeRes_Gas_Recovery			1200	// Time Out del reset errore, oltre il quale azzero se non si presenta in questi minuti.
	#define	Gas_Recovery_P_Ist				1000	// pressione in aggiunta di margine sulla pressione del liquido
	#define	Gas_Recovery_K_Iniet_Caldo		300		// K per il calcolo della temp di lavoro in caldo

	// paraetri ciclo recupero olio
    #define k_Recharge_Oil_Speed_Thersold   (Max_Speed * 10) / 100  //40%
	#define	TimeWorkRechargeOil				30//5		// tempo di durata del ciclo di recupero dell'olio (sec.)
	#define	TimePauseRechargeOil			15//10		// tempo di pausa del ciclo di recupero dell'olio (sec.)
#if (K_SIMULATION_WORK==0)
	#define	TimeAlarmRechargeOil			7200//1800	// tempo di allarme per ciclo recupero olio (sec.) 
    #define TimeLowCriticalAlarmRechargeOil 43200       // tempo di allarme secondario in priorità per ciclo recupero olio (sec.)
#else
	#define	TimeAlarmRechargeOil			60		// tempo di allarme per ciclo recupero olio (sec.) 
    #define TimeLowCriticalAlarmRechargeOil 120     // tempo di allarme secondario in priorità per ciclo recupero olio (sec.)
#endif

	// parametri errore temperatura acqua di mare
	#define	Low_Temp_Sea_Water				300		//300
	#define	RestartLow_Temp_Sea_Water		400		//500
	#define	High_Temp_Sea_Water				3900	//4000
	#define	Restart_High_Temp_Sea_Water		3800	//3500

    // parametri errore temperatura carica refrigerante assente
	#define	K_RefrigCharge_Press_Min		3000

	// parametri cicli errore bassa pressione
	//#define PressioneLo_Liq_P					3000 	// Pressione liq limite basso. Geneera l'allarme PressureLo.
	//#define PressioneLo_Gas_G					2500 	// Pressione gas limite basso. Geneera l'allarme PressureLo.
	#define	PressRestartLo_Liq_P				PressioneLo_Liq_P+1000//4000	// Pressione liq al di sopra la quale riparte il funzionamento normale.
	#define	PressRestartLo_Gas_G				PressioneLo_Gas_G+1000//3500	// Pressione gas al di sopra la quale riparte il funzionamento normale.
	#define	SpeedPressioneLo					Min_Speed	// Velocità massima da mantenere durante l'errore.
	#define	K_TimeErrPressureLo					60		//					

	// parametri cicli errore alta pressione
	//#define PressioneHi_Liq_P					24000 	// Pressione liq limite alto. Geneera l'allarme PressureHi.
	//#define PressioneHi_Gas_G					33000 	// Pressione gas limite alto. Geneera l'allarme PressureHi.
    #define PressioneCriticalHi_Liq_P           32000   // Pressione liq critica per spegnimento IMMEDIATO motore
    #define PressioneCriticalHi_Gas_G           32000   // Pressione gas critica per spegnimento IMMEDIATO motore
	#define	PressRestartHi_Liq_P				PressioneHi_Liq_P-600//23400	//23000	// Pressione liq al di sopra la quale riparte il funzionamento normale.
	#define	PressRestartHi_Gas_G				PressioneHi_Gas_G-2000//31000	// Pressione gas al di sopra la quale riparte il funzionamento normale.
	#define	SpeedPressioneHi					Min_Speed	// Velocità massima da mantenere durante l'errore.
	#define	K_TimeErrPressureHi					60		//

	// parametri flussostato
	#define	TimeFlussostato_Work				5//3		// Tempo di ritardo sul Flussostato in lavoro
    #define	TimeFlussostato_StandBy				20  		// Tempo di ritardo sul Flussostato quando c'è la pompa solo che gira
    #define	TimeFlussostato_TestPump			180  		// Tempo di ritardo sul Flussostato quando sono in test pompa

	// regolazione pompa acqua condensatore
	#define	SP_Acqua_Freddo					3300
	#define IstCondensatoreFreddo			300
	#define	SP_Acqua_Caldo					2000
	#define IstCondensatoreCaldo			300
	#define	Pump_Min_Speed					0
	#define	Pump_Max_Speed					20000

	// Costanti
	#define	StagioneEstate					1
	#define	StagioneInverno					2

	// Limiti degli errori
	#define	ResetTimeErrorCounter			1200	// tempo in secondi senza errori per il reset dei timer
	#define CicliCompressorHi				1000//3//10//5	// numero di ripetizioni consecutive dell'errore
	#define CicliCompressorLo				1000//3//10//5	// numero di ripetizioni consecutive dell'errore
	#define CicliCondensatoreHi				10//1	// numero di ripetizioni consecutive dell'errore
	#define CicliCondensatoreLo				4//5	// numero di ripetizioni consecutive dell'errore
	#define	CicliGas_Recovery				50		// numero di ripetizioni consecutive dell'errore(molto alto perchè tendenzialmente non deve mai entare in permanente).
	#define CicliTestFlussoAcqua			5		// numero di ripetizioni del test
	#define	CicliTestValvoleEspansione		5		// numero di ripetizioni del test

	// paramtri test di flusso
	#define CompressorSpeedFluxTest			127		// velocità in bit del compressore
	#define	IsteresiPressFluxTest			3000	// isteresi di pressione da verificare
	#define IstStartPressureFluxTest		2000	// Pliq < P_Gas + isteresi, attendo che le due pressioni si equilibrino all'incirca di 2 bar prima di partire con il test

	//parametri per il test delle EEV
	#define CompressorSpeedEEV_Test			120		// velocità in bit delcompressore
	#define TimeEEV_Test					160		// tempo in secondi di durata del test
	#define	SogliaP_Small					5000	// soglia in mBar del tubo piccolo (>5Bar)
	#define	SogliaT_MandataComp				3500	// soglia da oltrepassare (>35°C)

	//parametri per Inverter compressore
	#define	k_LIM_MIN_FREQ_INV_COMP			0		// MIN Freq = 0Hz
	#define	k_LIM_MAX_FREQ_INV_COMP			18000	// MAX Freq = 180Hz (centesimi di Hz -> 1=0.01Hz)

	#define K_LIM_MIN_FREQ_INV_COMP2PUMP	5000	// MIN Freq Inverter compressore per pilotaggio vel. minima pompa			
	#define K_LIM_MAX_FREQ_INV_COMP2PUMP	36000	// MAX Freq Inverter compressore per pilotaggio vel. massima pompa

//	#define K_LIM_MIN_FREQ_INV_PUMP			3000//2000 //1500	// MIN Freq Inverter pompa			// da ver. 5.16.158 Spostati in FWSelection.h	
//	#define K_LIM_MAX_FREQ_INV_PUMP			5500//4500	//5500	// MAX Freq Inverter pompa

#if(K_AbilPumpInverter==1 || K_AbilPumpDAC==1)
	#define	K_LIM_ERR_INV_PMP				K_LIM_MAX_FREQ_INV_PUMP
#endif
	#define	K_SeaWTemp_Correction_Pump		800		// 8°C			
#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)                    
	#define	K_Lim_Press_Liq_Pump			21000	// Limite Pressione per movimento pompa
#else
	#define	K_Lim_Press_Liq_Pump			19500	// Limite Pressione per movimento pompa
#endif
    #define	K_Ist_Work_Pump					100		// Isteresi regolazione vel. pompa
	#define	K_LIM_SHUTDWN_INV_PMP			3000	// 30Hz velocità pompa in spegnimento
	#define K_TimeWorkPump                  1000	// Tempo di calcolo regolazione velocità pompa (mSec.)
	#define K_IncHzWorkPump                 50      // Hz di incremento regolazione velocità pompa (0.1Hz)

	#define K_TimeTrigPump					500		// Tempo di trig della pompa 

    #define K_TimeCoolingSuperHeatErr       30
    #define K_SetPoint_Cooling_SuperHeat    600     // SuperHeat = 6K
    #define K_Max_Correct_SuperHeat         400     // Massima correzione SuperHeat = 4K

/*
    #define K_DefrostingPressTrig           6700    // Pressione GAS di intervento Errore Defrosting
    #define K_DefrostingPressOk             7000    // Pressione GAS di ripristino Errore Defrosting
    #define K_DefrostingCycleWork           60*60   // Tempo in Sec. di ciclo errore Defrosting           
    #define K_DefrostingCycleStop           5*60    // Tempo in Sec. di stop errore Defrosting
    #define K_DefrostingCycleCheck          5*60    // Tempo in Sec. di ripristino errore Defrosting
*/

    //Parametri per l'errore del condenstore sporco
    #define K_CondenserFouled               300
    #define K_TempCondenserFouled           2000//1000
    #define K_TempRestartCondenserFouled    1000//30

    #define K_CoolWarm_Valve_Error          180//60
#if K_SIMULATION_WORK==1	
    #define K_TimeHeatingEngine             40 //40*60     // Pausa in sec. rima di arrestare corrente di preriscaldamento durante la fase OFF del compressore
    #define K_TimePostHeatingEngine         20 //15*60     // Pausa in sec. rima di arrestare corrente di preriscaldamento durante la fase OFF del compressore
#else
    #define K_TimeHeatingEngine             0 //120 //40*60     // Pausa in sec. rima di arrestare corrente di preriscaldamento durante la fase OFF del compressore
    #define K_TimePostHeatingEngine         0 //60 //15*60     // Pausa in sec. rima di arrestare corrente di preriscaldamento durante la fase OFF del compressore
#endif

	#define K_InverterAcceleration			100		// Valore Minimo di accelerazione per NON generare un allarme di Compressione
	#define K_TimeAccelerationErrorCheck	2*60    // Tempo in Sec per calcolo e check accelerazione GAS e Inverter per gestione Errore Compressione
    #define K_Delta_Error_Compressor        2000    // Delta in Bar sotto la quale scatta l'errore compressione
    #define K_Soglia_Error_Comp_Motore      (35 * Max_Speed) / 100    // Soglia di abilitazione controllo errore compressione

    //#define K_SetPointHeatingSuperHeat      200//350     // Setpoint SuperHeat Riscaldamento
    #define K_GainHeatingSuperHeat          1.0     // Guadagno SuperHeat Riscaldamento
    #define K_SetPressSeaWaterSP            500
    #define K_HeatingSuperHeatCompressorLo  -200
    
    #define K_TimeSuperHeat_Error           30

    // Parametri per errori sonde di temperatura
    #define K_Compressor_Output_Probe_Lo    50
    #define K_Compressor_Output_Probe_Hi    9950

    #define K_Compressor_Suction_Probe_Lo   -2950
    #define K_Compressor_Suction_Probe_Hi   6950

    #define K_Condenser_Probe_Lo            -2950
    #define K_Condenser_Probe_Hi            6950

    #define K_Sea_Water_Probe_Lo            -2950
    #define K_Sea_Water_Probe_Hi            6950

    #define K_Liquid_Probe_Lo               -2950
    #define K_Liquid_Probe_Hi               6950

    #define K_TimeTemperatureProbe_Error    30

    #define K_DeltaTempElapsedTimeError      18      // Delta -> 0,18 °C
    #define K_TimerCheckTemperatureProbe_Error          2
    
    //#define K_MaxCntTempElapsedTimeError    18 //ARGOVIRUSPUZZONE     30*60/K_TimerCheckTemperatureProbe_Error //600    // Cnt = 600 x Cycletime  Example: 600 x 1000mS -> 600Sec -> 10min.
                                                               // Limite conteggio timer = 182 Ore -> 65535 / 60 / 60 * 10
    #define K_MaxCntTempElapsedTimeError_Compressor_Output  1*60*60/K_TimerCheckTemperatureProbe_Error //600    // Cnt = 600 x Cycletime  Example: 600 x 1000mS -> 600Sec -> 10min.
    #define K_MaxCntTempElapsedTimeError_Compressor_Suction 1*60*60/K_TimerCheckTemperatureProbe_Error //600    // Cnt = 600 x Cycletime  Example: 600 x 1000mS -> 600Sec -> 10min.
    #define K_MaxCntTempElapsedTimeError_Condenser          5*60*60/K_TimerCheckTemperatureProbe_Error //600    // Cnt = 600 x Cycletime  Example: 600 x 1000mS -> 600Sec -> 10min.
    #define K_MaxCntTempElapsedTimeError_Sea_Water          12*60*60L/K_TimerCheckTemperatureProbe_Error //600    // Cnt = 600 x Cycletime  Example: 600 x 1000mS -> 600Sec -> 10min.
    #define K_MaxCntTempElapsedTimeError_Liquid             1*60*60/K_TimerCheckTemperatureProbe_Error //600    // Cnt = 600 x Cycletime  Example: 600 x 1000mS -> 600Sec -> 10min.
    #define K_MaxCntTempElapsedTimeError_Compressor2_Output 1*60*60/K_TimerCheckTemperatureProbe_Error //600    // Cnt = 600 x Cycletime  Example: 600 x 1000mS -> 600Sec -> 10min.

    // Parametri per errori sonde di pressione
    #define K_Pressure_Gas_Probe_Lo         100
    #define K_Pressure_Liquid_Probe_Lo      100
    #define K_Pressure_LiquidCond_Probe_Lo  100

#if(K_PressSensorSelect==K_34Bar)
    #define K_Pressure_Gas_Probe_Hi         33800
    #define K_Pressure_Liquid_Probe_Hi      33800
    #define K_Pressure_LiquidCond_Probe_Hi  33800
#else
    #define K_Pressure_Gas_Probe_Hi         44500
    #define K_Pressure_Liquid_Probe_Hi      44500
    #define K_Pressure_LiquidCond_Probe_Hi  44500
#endif
    #define K_DeltaPressElapsedTimeError     180      // Delta -> 180 mBar
    #define K_TimerCheckPressureProbe_Error  2        //30*60   // 

    #define K_TimePressureProbe_Error        30

    #define K_MaxCntTempElapsedTimeError_Pressure_Gas           1*60*60/K_TimerCheckPressureProbe_Error //600    // Cnt = 600 x Cycletime  Example: 600 x 1000mS -> 600Sec -> 10min.
    #define K_MaxCntTempElapsedTimeError_Pressure_Liquid        1*60*60/K_TimerCheckPressureProbe_Error //600    // Cnt = 600 x Cycletime  Example: 600 x 1000mS -> 600Sec -> 10min.
    #define K_MaxCntTempElapsedTimeError_Pressure_LiquidCond    1*60*60/K_TimerCheckPressureProbe_Error //600    // Cnt = 600 x Cycletime  Example: 600 x 1000mS -> 600Sec -> 10min.
    

    //Parametri per errore floating voltage
#if(K_Supply_Inverter==283)
    #define K_Time_Resume_Ok_Voltage          20//120
    #define k_Voltage_Inverter_Min_Ok         280  //90*100//Maggiore di 510 Volt      90% @ 508 Vdc              100% @ 564 Vdc
    #define K_Time_Error_Ok_Voltage           30
    #define k_Voltage_Inverter_Max_Ok         330  //129*100//Minore di 730 Volt       129% @ 727 Vdc
    #define k_Voltage_Inverter_Nominal        283  //100% corrispondente a 400Vac * 1.41 (sulla Vdc)
#else
    #define K_Time_Resume_Ok_Voltage          20//120
    #define k_Voltage_Inverter_Min_Ok         510  //90*100//Maggiore di 510 Volt      90% @ 508 Vdc              100% @ 564 Vdc
    #define K_Time_Error_Ok_Voltage           30
    #define k_Voltage_Inverter_Max_Ok         730  //129*100//Minore di 730 Volt       129% @ 727 Vdc
    #define k_Voltage_Inverter_Nominal        564  //100% corrispondente a 400Vac * 1.41 (sulla Vdc)
#endif

/*
    #define k_Voltage_Inverter_Min_Ok        90*100//Maggiore di 508 Volt      90% @ 508 Vdc              100% @ 564 Vdc
    #define K_Time_Error_Ok_Voltage          30
    #define k_Voltage_Inverter_Max_Ok        129*100//Minore di 727 Volt       129% @ 727 Vdc
*/
/*
Devi moltiplicare la tensione di alimentazione x 1,41

Per l?inverter S15 la tensione di alimentazione in AC è min. 380V max 500V, quindi su bus Dc la devi alimentare min. 535V max. 705V.

Naturalmente in uscita sull?inverter avrai la tensione d?ingresso sul Bus Dc diviso 1,41

Purtroppo in nessun manuale Toshiba viene descritto questo tipo di alimentazione sul bus Dc

  
 */
	//---------------------------------------
	// Definizione fasi x Macchina a Stati per gestione compressore
	//---------------------------------------
	#define START_SM						0		
	#define CHECK_COMP_SPEED				1
	#define SET_COMP_SPEED_OFF				2	
	#define CHECK_COMP_TIMEOFF				3		
	#define CHECK_COMP_WORK					4
	#define WAIT_COMP_START					5       
	#define WAIT_CHECK_FLUX					6	
	#define SET_COMP_SPEED_START			7
	#define START_COOL_MODE					8	
	#define START_HOT_MODE					9

	//------------------------------------------------------------
	//	Definizione fasi x gestione test flusso acqua condensatore
	//------------------------------------------------------------
	#define START_FLUX_TEST					20
	#define CHECK_TIME_NOFLUX				21
	#define CHECK_TIME_YESFLUX				22
	#define EXIT_FLUX_TEST					23
	//------------------------------------------------------------
	//	Definizione fasi x gestione test EEV
	//------------------------------------------------------------
	#define START_EEV_TEST					30
	#define MAKE_EEV_TEST					31
	#define CHECK_SPEED_READY_EEV_TEST		32
	#define CHECK_END_EEV_TEST				33
	#define MAKE_END_EEV_TEST				34
	#define CHECK_TIMEOUT_EEV_TEST			35
	#define EXIT_EEV_TEST					36


	//--------------------------------------
	// Strutture
	//--------------------------------------

	//---------------------------------------
	// Variabili 
	//---------------------------------------

    #define VariabiliGlobaliCore()              \
        volatile TypTimer LCD_ScreeScheduler;       \
        volatile TypTimer PausaCompressore;         \
        volatile TypTimer TimerCompressoreC1;       \
        volatile TypTimer TimerCompressoreC2;       \
        volatile TypTimer TimerSmBox1;              \
        volatile TypTimer TimerSmBox2;              \
        volatile TypTimer TimerErrCompressorHiC1;   \
        volatile TypTimer TimerErrCompressorHiC2;   \
        volatile TypTimer TimerErrCompressorLo;     \
        volatile TypTimer TimerErrCondensatoreLo;   \
        volatile TypTimer TimerErrCondensatoreHi;   \
        volatile TypTimer TimerErrFlussostato;      \
        volatile TypTimer TimerErrGasRecovery;      \
        volatile TypTimer TimerResErrCompressorHiC1;\
        volatile TypTimer TimerResErrCompressorHiC2;\
        volatile TypTimer TimerResErrCompressorLo;  \
        volatile TypTimer TimerResErrCondensatoreHi;\
        volatile TypTimer TimerResErrCondensatoreLo;\
        volatile TypTimer TimerErrPressureHi;\
        volatile TypTimer TimerErrPressureLo;\
        volatile TypTimer TimerResTime_Gas_Rec; \
        volatile TypTimer TimerHeatingEngine1;			/* Timer per gestione tempo di riscaldamento motore con DC Braking*/                \
        volatile TypTimer TimerHeatingEngine2;			/* Timer per gestione tempo di riscaldamento motore con DC Braking*/                \
        volatile TypTimer TimerPostHeatingEngine;		/* Timer per gestione tempo di riscaldamento motore con DC Braking in fase OFF*/    \
        volatile TypTimer TimerAcceleration;			/* Timer per gestione Errore di compressione*/                                      \
        volatile TypTimer TimerWorkPump;    \
        volatile TypTimer TimerWorkRechargeOil1;    \
        volatile TypTimer TimerPauseRechargeOil1;   \
        volatile TypTimer TimerAlarmRechargeOil1;   \
        volatile TypTimer TimerLowCriticalAlarmRechargeOil1;\
        volatile TypTimer TimerWorkRechargeOil2;             \
        volatile TypTimer TimerPauseRechargeOil2;           \
        volatile TypTimer TimerAlarmRechargeOil2;           \
        volatile TypTimer TimerLowCriticalAlarmRechargeOil2;    \
        volatile TypTimer TimerDefrostingCycleWork;          \
        volatile TypTimer TimerDefrostingCycleStop;         \
       	volatile TypTimer TimerCondenserFouled;             \
        volatile TypTimer TimerDefrostingCycleCheck;
                
      

	
        
    #define IncludeVariabiliGlobaliCore()              \
        extern volatile TypTimer LCD_ScreeScheduler;       \
        extern volatile TypTimer PausaCompressore;         \
        extern volatile TypTimer TimerCompressoreC1;       \
        extern volatile TypTimer TimerCompressoreC2;       \
        extern volatile TypTimer TimerSmBox1;              \
        extern volatile TypTimer TimerSmBox2;              \
        extern volatile TypTimer TimerErrCompressorHiC1;   \
        extern volatile TypTimer TimerErrCompressorHiC2;   \
        extern volatile TypTimer TimerErrCompressorLo;     \
        extern volatile TypTimer TimerErrCondensatoreLo;   \
        extern volatile TypTimer TimerErrCondensatoreHi;   \
        extern volatile TypTimer TimerErrFlussostato;      \
        extern volatile TypTimer TimerErrGasRecovery;      \
        extern volatile TypTimer TimerResErrCompressorHiC1;\
        extern volatile TypTimer TimerResErrCompressorHiC2;\
        extern volatile TypTimer TimerResErrCompressorLo;  \
        extern volatile TypTimer TimerResErrCondensatoreHi;\
        extern volatile TypTimer TimerResErrCondensatoreLo;\
        extern volatile TypTimer TimerErrPressureHi;\
        extern volatile TypTimer TimerErrPressureLo;\
        extern volatile TypTimer TimerResTime_Gas_Rec;\
        extern volatile TypTimer TimerHeatingEngine1;			/* Timer per gestione tempo di riscaldamento motore con DC Braking*/                \
        extern volatile TypTimer TimerHeatingEngine2;			/* Timer per gestione tempo di riscaldamento motore con DC Braking*/                \
        extern volatile TypTimer TimerPostHeatingEngine;		/* Timer per gestione tempo di riscaldamento motore con DC Braking in fase OFF*/    \
        extern volatile TypTimer TimerAcceleration;			/* Timer per gestione Errore di compressione*/                                      \
        extern volatile TypTimer TimerWorkPump;        \
        extern volatile TypTimer TimerWorkRechargeOil1;    \
        extern volatile TypTimer TimerPauseRechargeOil1;   \
        extern volatile TypTimer TimerAlarmRechargeOil1;   \
        extern volatile TypTimer TimerLowCriticalAlarmRechargeOil1;\
        extern volatile TypTimer TimerWorkRechargeOil2;             \
        extern volatile TypTimer TimerPauseRechargeOil2;           \
        extern volatile TypTimer TimerAlarmRechargeOil2;           \
        extern volatile TypTimer TimerLowCriticalAlarmRechargeOil2; \
        extern volatile TypTimer TimerDefrostingCycleWork;          \
        extern volatile TypTimer TimerDefrostingCycleStop;          \
        extern volatile TypTimer TimerCondenserFouled;              \
        extern volatile TypTimer TimerDefrostingCycleCheck;
        

        
        
        IncludeVariabiliGlobaliCore();
        
        
        
	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	//void CompressorCheck(volatile TypTimer * TimerElaborazione);
	void CompressorCheck(void);
	void CompressorAdjustment(TypEngineBox * CompNumber, TypTimer * TimerCompressore);
	int	 CompressorTemp(void);
	unsigned int PressureGas(int ReqTemp);
    int PressureGasToTempR410A(unsigned int ReqPress);
	unsigned int TempToPressureGasR410A(int ReqTemp);
	void PumpManagement(void);
	unsigned char Increment(int * Variabile, int Value, int Max);
	unsigned char Decrement(int * Variabile, int Value, int Min);
	void ErrorCheck(void);
    void Error_Thermic_Comp(void);
    void Error_Fire_Alarm(void);
    void Error_Flood(void);
    void Error_Sea_Water_Lo(void);
    void Error_Sea_Water_Hi(void);
    void Error_Refrigerant_Charge(void);
    void Error_Compressor_Hi(void);
    void Error_Compressor_Lo(void);
    void Error_Condenser_Hi(void);
    void Error_Condenser_Lo(void);
    void Error_Gas_Recovery(void);
    void Error_Pressure_Lo(void);
    void Error_Pressure_Hi(void);
    void Error_Condenser_Fouled(void);
    void Error_Flow_Test(void);
    void Error_Flow_Req(void);
    void Error_EEV_Test(void);
    void Error_Reset(void);
    void Error_Compact(void);
	void RechargeOilCycle(TypEngineBox * NumComp, TypTimer * TimerWork, TypTimer * TimerPause, TypTimer * TimerAlarm, TypTimer * TimerLowCriticalAlarm);
	int DAC2FreqInverterComp(int outDAC);
	unsigned int FreqInverterPump2DAC(unsigned int freq);
    ///void CompressorHeatingEngine(TypTimer * Timer, int index, int InvAddr);
	void CompressionError(void);
	void WarmColdValveManagement(void);
    void AbsorptionManagement(void);    
	void EngineBoxAutoModeSelection(TypEngineBox * CompNumber);
	int WorkEngineBoxStateMachine(TypEngineBox * CompNumber, TypTimer * TimerWork);
	void EngineBox_ModeSelection(TypEngineBox * CompNumber);
	void SpeedPower_CompressorRegulation(int index, TypEngineBox * NumComp, volatile TypTimer * TimerElaborazione);
	void SelectWorkCompressor(void);	
	int FreqInv2FreqPump(long FcIn, int FcMin, long FcMax, int FpMin, long FpMax);
	int ValueToPercent(int Value, int MaxValue);
	int ValueToPercentDecimal(int Value, int MaxValue);
	void CheckSwitchPump(void);
    void PID_EngineBoxSuperHeat(void);    
    void PID_EngineBoxPressSeaWater(void);
    void DefrostingCycle(void);
    void CoolingEngineBoxSuperHeat(void);
    float round(float fvalue);
    int Delta2ReqPress(int LoadPerc);
    float ValueIn2ValueOut(float Vin, float VinMin, float VinMax, float VoutMin, float VoutMax);
    int CheckTempProbeFreeze(int Value, int * PrevVal, int * CntErr, unsigned int MaxCnt);
    int CheckPressProbeFreeze(int Value, int * PrevVal, int * CntErr, unsigned int MaxCnt);
#endif
