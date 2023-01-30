
/* 
 * File:   FWSelection.h
 * Author: GianLuca
 *
 * Created on 22 dicembre 2015, 0.23
 */
#ifndef FWSELECTION_H
    #define	FWSELECTION_H
#endif

	#define K_SIMULATION_WORK               0		// Include/esclude parti di codice per permettere il collaudo del FW senza essere su un'impianto "VERO"		

    #define USA								1           // bit.0 (gestito a bit)
    #define EUR								2           // bit.1 (gestito a bit)
    #define JAP								4           // bit.2 (gestito a bit)
    #define K_Touch_Nation					EUR

    #define VentilMode                      1
    #define CoolMode                        2
    #define HotMode                         4
    #define AutoMode                        8

    #define K_MaxTouchFanSpeed              8    // Imposta il numero di step di velocità gestiti sul touch
                                                    // Se impostato a zero non viene inviato agli slave che si 
                                                    // settano il valore in automatico a seconda del device collegato

    #define K_AbilSplitMode                 VentilMode+HotMode+CoolMode //Sommare le modalità da visulaizzare sul touch.

#if(K_AbilSplitMode>=AutoMode)
    #define K_MasterAutoRoom                2       // Indirizzo della Room Master che comanda la Mode
    #define K_MasterAutoIst                 200     // Isteresi setpoint che switcha la Mode di funzionamento in modo Auto
#endif

    #define K_Baud57600                     0       // 1 = HiSpeed (57600Baud)
    #define K_Baud115200                    1       // 2 = HiSpeed (115200Baud)
    #define K_Baud250000                    2       // 3 = HiSpeed (250000Baud)

    #define K_EnableHiSpeedMyBus            K_Baud115200       // Vedi sopra

	#define K_AbilCompressor2				1		// 1=Abilita la gestione del compressore 2 (Inverter parallelo)
#if(K_AbilCompressor2==1)
	#define K_OnlyOneInverterComp2  		0		// 1=Il compressore è sullo stesso inverter del primo (2 compressori, 1 inverter)
#endif

	#define K_AUTOTUNING_OFF				1		// Flag per impedire il comando di Autotuning e inviare parametri di default.
    #define K_STEPOUT_DISABLE               0       // Flag per disabilitare SOUT Inverter.

                                                    // (risoluzione 0.01%) es. 1% -> 100

	#define K_MotorModel_Selection1			K_MotorType_DA422A3F26M       //	K_MotorType_DA130A1F25F3
	#define K_MotorModel_Selection2			K_MotorType_DA422A3F26M       //	K_MotorType_DA150A1F21N
                                                                            //	K_MotorType_DA270A2F20L
																			//	K_MotorType_DA330A2F20M
																			//	K_MotorType_DA422A3F26M
																			//	K_MotorType_DA422A3F27M
                                                                            //	K_MotorType_DA550A3F10M
                                                                            //  K_MotorType_DA550A3F11MD
                                                                            //  K_MotorType_DA640A3F20MAD
                                                                            //  K_MotorType_DA790A4F11UC1

    #define K_Power_InverterC1				5500	// Potenza Taglia Inverter compressore 1		Watt
	#define K_Power_InverterC2				5500	// Potenza Taglia Inverter compressore 2		Watt
	#define K_Power_InverterPump			1500	// Potenza Taglia Inverter Pompa				Watt
	#define K_PowerMax_Available			9000   //3500-100W pompa	4000	// Valore limitazione Potenza totale			Watt

    #define K_Inv_PercLimit_Out_Current     98 *100//100 *100// Perecentuale della corrente massima inverter erogabile in centesimi 

    #define K_Pec_Sum_Inverter_Comp         15      //Sommo una percentuale all'assorbimento inverter
    #define K_Pec_Sum_Inverter_Pump         0//15      //Sommo una percentuale all'assorbimento inverter

    #define K_Supply_Inverter               200         // Tensione di alimentazione Inverter in V (Serve per il cacolo della tensione In/Out di lavoro che viene restituita dall'inverter solo in percentuale!)
                                                        // Dal Manuale: Tensione in uscita:  
                                                        // La tensione visualizzata è la tensione di comando uscita. 
                                                        // Il valore di riferimento (valore 100%) è:
                                                        // 200 V per i modelli da 240 VAC 
                                                        // 400 V per i modelli da 500 VAC
                                                        // 566 V per i modelli da 700 VDC
                                          
    #define	Max_Speed_Cooling				(Max_Speed * 160L) / (k_LIM_MAX_FREQ_INV_COMP/100L)		// valore massimo per l'inverter in freddo in BIT (inserire valore in Hz)
	#define	Max_Speed_Heating				(Max_Speed * 160L) / (k_LIM_MAX_FREQ_INV_COMP/100L)		// valore massimo per l'inverter in caldo in BIT (inserire valore in Hz)

    #define	K_Perc_Ok_Compressor2			(Max_Speed * 90L) / (k_LIM_MAX_FREQ_INV_COMP/100L)     // Integra compressore2 dopo "K_Perc_Ok_Compressor2" di Out comp1 (inserire valore in Hz)
	#define	K_Perc_No_Compressor2			(Max_Speed * 40L) / (k_LIM_MAX_FREQ_INV_COMP/100L)     // Deintegra compressore2 dopo "K_Perc_No_Compressor2" di Out comp1 (inserire valore in Hz)

    //#define	K_TimeIntegrateCompressor2      20  //tempo di aggancio compressori dopo K_Perc_Ok_Compressor2 Hz in sec.
    //#define	K_TimeDeintegrateCompressor2    60  //tempo di sgancio compressori dopo K_Perc_No_Compressor2 Hz in sec.

    //Tempo di attesa ripartenza compressore dopo spegnimento

#if (K_SIMULATION_WORK==1)
	#define	WaitTimeRestart					1*60 // tempo in secondi prima del riavvio (minimo 120 secondi, un minuto prima si ferma il compressore)
#else
	#define	WaitTimeRestart					5*60 // tempo in secondi prima del riavvio (minimo 120 secondi, un minuto prima si ferma il compressore)
#endif
    //Avvio compressori simultaneo in caldo
    #define k_ForceWorkAllCompressorHot     0   //Compressori in caldo con partenza gemellata

    #define	K_AbilPumpInverter				1		// Abilita gestione Pompa Inverter
#if(K_AbilPumpInverter==0)
    #define	K_AbilPumpDAC   				0		// Abilita gestione Pompa dolo con DAC
#endif
    #define K_Abil_Double_Pump              0       // Abilita gestione doppia pompa

#if(K_AbilPumpInverter==0 && K_AbilPumpDAC==0 && K_Abil_Double_Pump==0)
    #define K_External_SeaW_Supply          0
#endif

#if	(K_AbilPumpInverter==1)
    #define K_PumpModel_Selection			K_PumpType_DEFAULT                //K_PumpType_DEFAULT->            Identificativo per default inverter pompa
#endif                                                                        //K_PumpType_MPH_441_FGACE5->		Identificativo per pompa da 330 L/min
                                                                              //K_PumpType_MPH_453_FGACE5->		Identificativo per pompa da 500 L/min

#if(K_AbilPumpInverter==0)
	#define K_Absorption_Pump_OnOff			K_Power_InverterPump/10		// Assorbimento della pompa On-Off in decimi di Watt -> 40 = 400W
#endif

#if(K_AbilPumpInverter==1 || K_AbilPumpDAC==1)
	#define	kPump_Freq_Threshold_1			5000
	#define	kPump_Freq_Threshold_2			9000
	#define	kPump_Freq_Threshold_3			13000
	#define	kPump_Freq_Threshold_4			15000
	#define	kPump_Freq_Threshold_5			18000

	#define	K_LIM_MIN_1_FREQ_INV_PUMP		3000
	#define	K_LIM_MIN_2_FREQ_INV_PUMP		3500
	#define	K_LIM_MIN_3_FREQ_INV_PUMP		4000
	#define	K_LIM_MIN_4_FREQ_INV_PUMP		4500
	#define	K_LIM_MIN_5_FREQ_INV_PUMP		5000	

	#define K_LIM_MIN_FREQ_INV_PUMP			3000//2000 //1500	// MIN Freq Inverter pompa			
	#define K_LIM_MAX_FREQ_INV_PUMP			4800	//5500	// MAX Freq Inverter pompa

	#define K_LIM_FREQ_INV_PUMP_ECO_MODE	3500

	#define K_Force_Freq_Inv_Pump_Hot		1
    #define K_Force_Freq_Inv_Pump_Cold		0
#if(K_Force_Freq_Inv_Pump_Hot==1)
	#define K_LIM_FORCED_FREQ_INV_PUMP_HOT	4800
#endif
#if(K_Force_Freq_Inv_Pump_Cold==1)
	#define K_LIM_FORCED_FREQ_INV_PUMP_COLD	5000
#endif
#endif

    #define K_34Bar                         0           // 34Bar fondoscala
    #define K_45Bar                         1           // 45Bar fondoscala
    #define K_PressSensorSelect             K_34Bar     // Seleziona il modello di sonde di pressione collegate

	#define K_Condenser_Pressure_Abil		1   // Abilito la 3 sonda di pressione

#if(K_Condenser_Pressure_Abil==1)
    #define K_Condenser_Pressure_Simulated	0   // Decido se la 3 sonda di pressione deve essere simulata
#if (K_Condenser_Pressure_Simulated==1)   // Decido se la 3 sonda di pressione deve essere simulata
    #define K_Condenser_Pressure_Correct	0 // Dico a quanti Bar meno deve essere rispetto al liquido
#endif
#endif

    #define K_New_ReqPressModeCooling       0
#if (K_New_ReqPressModeCooling==1)
    #define K_MaxAirOut_PressWork           5   //Gradi sopra il quale inizio a contare il punteggio per pressione
    #define K_Min_Load                      20
    #define K_Min_Press                     7000
    #define K_Mid_Load                      50
    #define K_Mid_Press                     6300
    #define K_Max_Load                      100
    #define K_Max_Press                     5800

#endif

    #define	Kf_PoMax                            55.0 * 7680.0 / 100.0 
    #define K_SetPointHeatingSuperHeat          800    // Setpoint SuperHeat Riscaldamento
	// parametri cicli errore bassa pressione
	#define PressioneLo_Liq_P					3500 	// Pressione liq limite basso. Geneera l'allarme PressureLo.
	#define PressioneLo_Gas_G					3500 	// Pressione gas limite basso. Geneera l'allarme PressureLo.
	// parametri cicli errore alta pressione    

	#define PressioneHi_Liq_P					28000 	// Pressione liq limite alto. Geneera l'allarme PressureHi.

	#define	TempCompressorHI				    7500    // Temp limite della mandata del compressore. Genera l'allarme CompressorHi.

#if(K_Condenser_Pressure_Abil==1)
    #define K_Forced_PressioneHi_Liq_P_Liq_Pres 0       // Forza gli errori di alta pressione con la Press liquido e non condensatore.
#endif

#define PressioneHi_Gas_G                       28000 	// Pressione gas limite alto. Geneera l'allarme PressureHi.


	#define	K_AbilRechargeOil				0		// abilita ciclo recupero olio in riscaldamento

    #define K_AbilDefrostingCycle           0       // abilito il ciclo defrosting sul master in condizionamento.

#if (K_AbilDefrostingCycle==1)					   
    #define K_DefrostingPressTrig           6700    // Pressione GAS di intervento Errore Defrosting
    #define K_DefrostingPressOk             7000    // Pressione GAS di ripristino Errore Defrosting
    #define K_DefrostingCycleWork           20*60   // Tempo in Sec. di ciclo errore Defrosting primache scatta l'errore          
    #define K_DefrostingCycleCheck          5*60    // Tempo in Sec. di ripristino errore Defrosting dopo che la pressione è tornata sopra la soglia limite
    #define K_DefrostingCycleStop           5*60    // Tempo in Sec. di stop errore Defrosting prima che la macchina riparte
#endif

    #define K_AbilValveOnDemand             0       // abilito la limitazione delle valvole per migliorare le prestazioni del condensatore
#if(K_AbilValveOnDemand==1)
    #define K_Max_Lim_ON_Demand_Perc_Valve  430 //370     // limite massimo di valvole limitato (%)
    #define K_Min_Lim_OFF_Demand_Perc_Valve K_Max_Lim_ON_Demand_Perc_Valve - 30 //20    //310     // limite minimo di valvole libere (%)
    #define K_TimeOnDemandValveSwitch       4*60 //5*60    // Tempo in Sec. di switch della priorità valvola
#endif

#if(K_AbilValveOnDemand==0)
    #define K_AbilScoreOnDemand              0       // abilito la limitazione delle valvole per migliorare le prestazioni del condensatore
#if(K_AbilScoreOnDemand==1)
    #define K_TimeOnDemandValveSwitch        3*60 //5*60    // Tempo in Sec. di switch della priorità valvola
    #define K_MaxAirOut                      1000   //Gradi sopra il quale inizio a contare il punteggio per riduzione
    //#define K_Max_Lim_ON_Demand_Total_Score  15                                 // limite massimo di Score limitato
    //#define K_Min_Lim_OFF_Demand_Total_Score K_Max_Lim_ON_Demand_Total_Score - 3  // limite minimo di Score libero
    //#define K_Max_Lim_ON_Demand_Perc_Valve   800 //370     // Limite massimo di valvole limitato (%) METTERE SUPERIORE A NUMERO SPLIT *100 SE NON SI VUOLE DARE LIMITAZIONE
#endif
#endif

    #define K_AbilCoolingSuperHeat          1        

	#define K_AbilErrorCompression			1		// 1=Abilita gestione Errore Compressione 0=Disabilita

	#define K_AbilInverterInit				1		// 1=Abilita Inizializzazione parametri inverter 0=Disabilita

	//#define K_AbilUTA						0		// 1=Abilita unità UTA 0=Disabilita

	#define K_EnableBrakeCurrent			0		// Abilita gestione Brake current 

	#define K_DisableSeaFlowStart			1		// Include/esclude parti di codice per permettere il collaudo del FW senza essere su un'impianto "VERO"		

	#define K_EnableTesting					0//1		// Include/esclude parti di codice per permettere il collaudo dell'hardware della scheda Master.
													// Il collaudo prevede: 
													//						- La chiusura dei rele sull'attivazione degli ingressi digitali:
													//						  REL1 = In1, REL2 = In2, REL3 = In3, REL4 = In4, REL5 = In5
													//					    - Le uscite 0-10V vengono portate a 10V su attivazione di In6
													//						- Vengono inviate tutte le temperature (Ch0..Ch5) e gli ingressi pressioni GAS e LIQ al Touch n.1 collegato sul BUS
													// N.B.: Durante la fase di collaudo controllare la corretta accensione di tutti i LED.
													// 1= Abilita la modalità "Collaudo"

#if (K_EnableBrakeCurrent==1)
	#define K_EnableBrakeStart				0		// Abilita gestione Brake current all'avvio
#endif

	#define K_AbilBridge1					0
	#define K_AbilBridge2					0
	#define K_AbilMODBCAN					1       // Abilita gestione dati scambio con convertitore MODBUS-CAN x Scheiber (AD67412-E4V)


#if (K_AbilBridge1==1 || K_AbilBridge2==1 || K_AbilMODBCAN==1)
	#define K_HiSpeedU1ModBUS               1       // 1 = Abilita l'alta velocità (125000Baud) sulle comunicazioni UART1 vs MODBUS (Per ora è usato se presente una o più schede Bridge)
                                                    // N.B. Ricordarsi di alzare la velocità anche sul Bridge (usart.h)
#endif

	#define K_DEBUG							0		
	#define K_DEBUG_LCD						0		// Abilita la gestione dell'eventuale display LCD connesso dul connettore ICD
	#define K_TEST							0		// Libero per usi di test
	#define K_DEVELOP						0		// Esegue il codice per il collaudo dell'HW

	#define	K_Ing1_ModeSelection			1		// 0=GenericInput  1=ServiceMode 2=EcoMode
	#define	K_Ing2_ModeSelection			0		// 0=iThermicComp1/iHighPressure  1=ShutDown Comp

	#define	K_MaxCommErrorAlarm             10		// Numero di errori di comunicazione (Mybus o ModBus) massimi prima di segnalare anomalia con accensione LED Rosso

    #define K_ABIL_UART_SERVICE_DEBUG       1       // Abilita le funzioni per l'output dei dati su UART3 e UART4 per scopi di debug



/*
	#define	K_AbilSendData_RoomXX                    1		// Abilita l'invio dei setpoint specificati per la Room
#if K_AbilSendData_RoomXX==1                         
	#define	K_AbilDinamicSuperHeat_RoomXX            0		// Abilita sullo slave il SuperHeat Dinamico
#if(K_AbilDinamicSuperHeat_RoomXX==0)                
    #define	K_SuperHeat_SP_RoomXX                    600		// Setpoint di SuperHeat da inviare verso gli Slaves
#endif
	#define	K_Temp_Amb_Mid_Heating_RoomXX            1		// Abilita media temperatura funzionamento in Riscaldamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_Temp_Amb_Mid_Cooling_RoomXX            1		// Abilita media temperatura funzionamento in Condizionamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_WorkPressCool_SP_RoomXX                0		// SetPoint Pressione di lavoro in Freddo da inviare verso gli Slaves
    #define K_MinPressCool_SP_RoomXX                 500     // SetPoint Pressione minima di lavoro in freddo da inviare agli Slaves
	#define	K_WorkPressHeat_SP_RoomXX                4200	// SetPoint Pressione di lavoro in Caldo da inviare verso gli Slaves
	#define	K_MinPressHeat_SP_RoomXX                 3500    // SetPoint Pressione minima in Caldo da inviare verso gli Slaves
	#define	K_TempCorrectSuperHeat_AirOut_RoomXX     200	// Valore sotto il quale il superheat comincia ad alzarsi per evitare condensa tubi.
	#define	K_PerformanceLossBattery_SP_RoomXX       0       // SetPoint Perdita potenza batteria in Caldo da inviare verso gli Slaves
    #define	K_Other_Cabin_Number_RoomXX              0       // Numero di cabina alternativa da controllare dallo split
    
    #define	K_ValveLim_PercMaxOpenValve_RoomXX       100     // Percentuale di apertura massima valvola
    #define	K_ValveLim_ValvolaAperta_RoomXX          K_ValveLim_PercMaxOpenValve_RoomXX       // Escursione valvola aperta in %
    #define	K_ValveLim_ValvolaChiusa_RoomXX          0       // Escursione valvola aperta in %
    #define	K_ValveLim_ValvolaOff_RoomXX             0       // Percentuale valvola OFF    
    #define	K_ValveLim_Valve_Min_Go_Off_RoomXX       8       // Valore minimo della valvola con unità in spegnimento
    #define	K_ValveLim_ValveLostCommCool_RoomXX      0       // Valore in % della valvola in freddo con comunicazione persa
    #define	K_ValveLim_ValveLostCommHot_RoomXX       1       // Valore in % della valvola in caldo con comunicazione persa
    #define	K_ValveLim_AperturaMaxCaldo_RoomXX       100     // Apertura massima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMinCaldo_RoomXX       20      // Apertura minima % in lavoro x Caldo
    #define	K_ValveLim_InitValvolaFreddo_RoomXX      0       // Inizializzazione valvola % in Freddo
    #define	K_ValveLim_AperturaMaxFreddo_RoomXX      100     // Apertura massima % in lavoro x Freddo
    #define	K_ValveLim_AperturaMinFreddo_RoomXX      0       // Apertura minima % in lavoro x Freddo
    #define	K_ValveLim_ValveNoWorkMinFreddo_RoomXX   0       // Apertura minima % in NON lavoro x Caldo
    #define	K_ValveLim_ValveNoWorkMaxFreddo_RoomXX   20      // Apertura minima % in NON lavoro x Caldo
    #define	K_ValveON_Demand_Priority_RoomXX         1       // Priorità di lavoro della stanza quando Valve On Demand attivo.

 	#define K_FanVentil_Selection_RoomXX             0       //Seleziono modello ventola 0=EBM24V,1=GR22_230V,2=GR25_230V
 	#define K_FanM3h_Min_RoomXX                      100     //M3h al minimo della ventola
 	#define K_FanM3h_Max_RoomXX                      1000       //M3h al massimo della ventola

	#define K_Min_Current_Fan_RoomXX                 100     // Corrente assorbita MIN in milliampere INIZIALIZZATA PER GR22
	#define K_Max_Current_Fan_RoomXX                 900     // Corrente assorbita MAX in milliampere INIZIALIZZATA PER GR22
	#define K_Efficiency_Fan_RoomXX                  47      // Efficienza della ventola in %         INIZIALIZZATA PER GR22
    #define K_Supply_Fan_RoomXX                      230     // Tensione ventola per calcolo potenza  INIZIALIZZATA PER GR22

	#define K_FanSpeedOff_RoomXX                     0
	#define K_FanSpeedNight_RoomXX                   35
	#define K_FanSpeed1_RoomXX                  	 50
	#define K_FanSpeed2_RoomXX              		 70
	#define K_FanSpeed3_RoomXX              		 85
	#define K_FanSpeed4_RoomXX              		 110
	#define K_FanSpeed5_RoomXX              		 140
	#define K_FanSpeed6_RoomXX              		 190
	#define K_FanSpeed7_RoomXX              		 255

	#define K_Fan_New_Style_Hot_Time_On_RoomXX    	 30
    #define K_Fan_New_Style_Hot_Time_Off_RoomXX   	 180 
    #define	K_NetB_LCD_Intensity_Roomxx              12       // Intensità dell'LCD NetBuilding a riposo (0...255, default 12)
    #define K_Fan_Current_Limit_RoomXX               200      // Limitazione della corrente massima ventola in centesimi di A 200 = 2A

    #define	K_Extractor_Number_RoomXX                0       // Numero di cabina estrattore da controllare insieme al Fan Coil
    #define	K_Extractor_DeltaVel_RoomXX              0       // Fa girare a velocità maggiore l'estrattore se è maggiore di 0.


 */

//+++++++++ ROOM 1 +++++++++
	#define	K_AbilSendData_Room1                    1		// Abilita l'invio dei setpoint specificati per la Room
#if K_AbilSendData_Room1==1                         
	#define	K_AbilDinamicSuperHeat_Room1            0		// Abilita sullo slave il SuperHeat Dinamico
#if(K_AbilDinamicSuperHeat_Room1==0)                
    #define	K_SuperHeat_SP_Room1                    600		// Setpoint di SuperHeat da inviare verso gli Slaves
#endif
	#define	K_Temp_Amb_Mid_Heating_Room1            1		// Abilita media temperatura funzionamento in Riscaldamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_Temp_Amb_Mid_Cooling_Room1            1		// Abilita media temperatura funzionamento in Condizionamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_WorkPressCool_SP_Room1                0		// SetPoint Pressione di lavoro in Freddo da inviare verso gli Slaves
    #define K_MinPressCool_SP_Room1                 500     // SetPoint Pressione minima di lavoro in freddo da inviare agli Slaves
	#define	K_WorkPressHeat_SP_Room1                4100	// SetPoint Pressione di lavoro in Caldo da inviare verso gli Slaves
	#define	K_MinPressHeat_SP_Room1                 3100    // SetPoint Pressione minima in Caldo da inviare verso gli Slaves
	#define	K_PerformanceLossBattery_SP_Room1       400     // SetPoint Perdita potenza batteria in Caldo da inviare verso gli Slaves
    #define	K_ValveLim_PercMaxOpenValve_Room1       100     // Percentuale di apertura massima valvola
    #define	K_ValveLim_AperturaMinCaldo_Room1       5      // Apertura minima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMaxCaldo_Room1       100     // Apertura massima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMinFreddo_Room1      0       // Apertura minima % in lavoro x Freddo
    #define	K_ValveLim_AperturaMaxFreddo_Room1      100           // Apertura massima % in lavoro x Freddo
	#define	K_TempCorrectSuperHeat_AirOut_Room1     200         // Valore sotto il quale il superheat comincia ad alzarsi per evitare condensa tubi.      

	#define K_FanSpeedNight_Room1                   43 
	#define K_FanSpeed1_Room1                       59 
	#define K_FanSpeed2_Room1                       75 
	#define K_FanSpeed3_Room1                       91 
	#define K_FanSpeed4_Room1                       107
	#define K_FanSpeed5_Room1                       132
	#define K_FanSpeed6_Room1                       156
	#define K_FanSpeed7_Room1                       180
#endif

//+++++++++ ROOM 2 +++++++++
	#define	K_AbilSendData_Room2                    1		// Abilita l'invio dei setpoint specificati per la Room
#if K_AbilSendData_Room2==1                         
	#define	K_AbilDinamicSuperHeat_Room2            0		// Abilita sullo slave il SuperHeat Dinamico
#if(K_AbilDinamicSuperHeat_Room2==0)                
    #define	K_SuperHeat_SP_Room2                    600		// Setpoint di SuperHeat da inviare verso gli Slaves
#endif
	#define	K_Temp_Amb_Mid_Heating_Room2            3		// Abilita media temperatura funzionamento in Riscaldamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_Temp_Amb_Mid_Cooling_Room2            2		// Abilita media temperatura funzionamento in Condizionamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_WorkPressCool_SP_Room2                0		// SetPoint Pressione di lavoro in Freddo da inviare verso gli Slaves
    #define K_MinPressCool_SP_Room2                 500     // SetPoint Pressione minima di lavoro in freddo da inviare agli Slaves
	#define	K_WorkPressHeat_SP_Room2                4100	// SetPoint Pressione di lavoro in Caldo da inviare verso gli Slaves
	#define	K_MinPressHeat_SP_Room2                 3100    // SetPoint Pressione minima in Caldo da inviare verso gli Slaves
	#define	K_PerformanceLossBattery_SP_Room2       400     // SetPoint Perdita potenza batteria in Caldo da inviare verso gli Slaves
    #define	K_ValveLim_PercMaxOpenValve_Room2       100     // Percentuale di apertura massima valvola
    #define	K_ValveLim_AperturaMinCaldo_Room2       5      // Apertura minima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMaxCaldo_Room2       100     // Apertura massima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMinFreddo_Room2      0       // Apertura minima % in lavoro x Freddo
    #define	K_ValveLim_AperturaMaxFreddo_Room2      100           // Apertura massima % in lavoro x Freddo
	#define	K_TempCorrectSuperHeat_AirOut_Room2     200         // Valore sotto il quale il superheat comincia ad alzarsi per evitare condensa tubi.      

	#define K_FanSpeedNight_Room2                   43 
	#define K_FanSpeed1_Room2                       59 
	#define K_FanSpeed2_Room2                       75 
	#define K_FanSpeed3_Room2                       91 
	#define K_FanSpeed4_Room2                       107
	#define K_FanSpeed5_Room2                       132
	#define K_FanSpeed6_Room2                       156
	#define K_FanSpeed7_Room2                       180
#endif

//+++++++++ ROOM 3 +++++++++
	#define	K_AbilSendData_Room3                    1		// Abilita l'invio dei setpoint specificati per la Room
#if K_AbilSendData_Room3==1                         
	#define	K_AbilDinamicSuperHeat_Room3            0		// Abilita sullo slave il SuperHeat Dinamico
#if(K_AbilDinamicSuperHeat_Room3==0)                
    #define	K_SuperHeat_SP_Room3                    700		// Setpoint di SuperHeat da inviare verso gli Slaves
#endif
	#define	K_Temp_Amb_Mid_Heating_Room3            3		// Abilita media temperatura funzionamento in Riscaldamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_Temp_Amb_Mid_Cooling_Room3            2		// Abilita media temperatura funzionamento in Condizionamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_WorkPressCool_SP_Room3                0		// SetPoint Pressione di lavoro in Freddo da inviare verso gli Slaves
    #define K_MinPressCool_SP_Room3                 500     // SetPoint Pressione minima di lavoro in freddo da inviare agli Slaves
	#define	K_WorkPressHeat_SP_Room3                4100	// SetPoint Pressione di lavoro in Caldo da inviare verso gli Slaves
	#define	K_MinPressHeat_SP_Room3                 3100    // SetPoint Pressione minima in Caldo da inviare verso gli Slaves
	#define	K_PerformanceLossBattery_SP_Room3       400     // SetPoint Perdita potenza batteria in Caldo da inviare verso gli Slaves
    #define	K_ValveLim_PercMaxOpenValve_Room3       100     // Percentuale di apertura massima valvola
    #define	K_ValveLim_AperturaMinCaldo_Room3       5      // Apertura minima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMaxCaldo_Room3       100     // Apertura massima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMinFreddo_Room3      0       // Apertura minima % in lavoro x Freddo
    #define	K_ValveLim_AperturaMaxFreddo_Room3      80           // Apertura massima % in lavoro x Freddo
	#define	K_TempCorrectSuperHeat_AirOut_Room3     200         // Valore sotto il quale il superheat comincia ad alzarsi per evitare condensa tubi.      

	#define K_FanSpeedNight_Room3                   43
	#define K_FanSpeed1_Room3                       71
	#define K_FanSpeed2_Room3                       100
	#define K_FanSpeed3_Room3                       105
	#define K_FanSpeed4_Room3                       110
	#define K_FanSpeed5_Room3                       115
	#define K_FanSpeed6_Room3                       120
	#define K_FanSpeed7_Room3                       125
#endif

//+++++++++ ROOM 4 +++++++++
	#define	K_AbilSendData_Room4                    1		// Abilita l'invio dei setpoint specificati per la Room
#if K_AbilSendData_Room4==1                         
	#define	K_AbilDinamicSuperHeat_Room4            0		// Abilita sullo slave il SuperHeat Dinamico
#if(K_AbilDinamicSuperHeat_Room4==0)                
    #define	K_SuperHeat_SP_Room4                    600		// Setpoint di SuperHeat da inviare verso gli Slaves
#endif
	#define	K_Temp_Amb_Mid_Heating_Room4            3		// Abilita media temperatura funzionamento in Riscaldamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_Temp_Amb_Mid_Cooling_Room4            2		// Abilita media temperatura funzionamento in Condizionamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_WorkPressCool_SP_Room4                0		// SetPoint Pressione di lavoro in Freddo da inviare verso gli Slaves
    #define K_MinPressCool_SP_Room4                 500     // SetPoint Pressione minima di lavoro in freddo da inviare agli Slaves
	#define	K_WorkPressHeat_SP_Room4                4100	// SetPoint Pressione di lavoro in Caldo da inviare verso gli Slaves
	#define	K_MinPressHeat_SP_Room4                 3100    // SetPoint Pressione minima in Caldo da inviare verso gli Slaves
	#define	K_PerformanceLossBattery_SP_Room4       400     // SetPoint Perdita potenza batteria in Caldo da inviare verso gli Slaves
    #define	K_ValveLim_PercMaxOpenValve_Room4       100     // Percentuale di apertura massima valvola
    #define	K_ValveLim_AperturaMinCaldo_Room4       5      // Apertura minima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMaxCaldo_Room4       100     // Apertura massima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMinFreddo_Room4      0       // Apertura minima % in lavoro x Freddo
    #define	K_ValveLim_AperturaMaxFreddo_Room4      100           // Apertura massima % in lavoro x Freddo
	#define	K_TempCorrectSuperHeat_AirOut_Room4     200         // Valore sotto il quale il superheat comincia ad alzarsi per evitare condensa tubi.      

	#define K_FanSpeedNight_Room4                   43
	#define K_FanSpeed1_Room4                       58
	#define K_FanSpeed2_Room4                       73
	#define K_FanSpeed3_Room4                       88
	#define K_FanSpeed4_Room4                       103
	#define K_FanSpeed5_Room4                       118
	#define K_FanSpeed6_Room4                       133
	#define K_FanSpeed7_Room4                       148
#endif

//+++++++++ ROOM 5 +++++++++
	#define	K_AbilSendData_Room5                    1		// Abilita l'invio dei setpoint specificati per la Room
#if K_AbilSendData_Room5==1                         
	#define	K_AbilDinamicSuperHeat_Room5            0		// Abilita sullo slave il SuperHeat Dinamico
#if(K_AbilDinamicSuperHeat_Room5==0)                
    #define	K_SuperHeat_SP_Room5                    700		// Setpoint di SuperHeat da inviare verso gli Slaves
#endif
	#define	K_Temp_Amb_Mid_Heating_Room5            3		// Abilita media temperatura funzionamento in Riscaldamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_Temp_Amb_Mid_Cooling_Room5            2		// Abilita media temperatura funzionamento in Condizionamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_WorkPressCool_SP_Room5                0		// SetPoint Pressione di lavoro in Freddo da inviare verso gli Slaves
    #define K_MinPressCool_SP_Room5                 500     // SetPoint Pressione minima di lavoro in freddo da inviare agli Slaves
	#define	K_WorkPressHeat_SP_Room5                4100	// SetPoint Pressione di lavoro in Caldo da inviare verso gli Slaves
	#define	K_MinPressHeat_SP_Room5                 3100    // SetPoint Pressione minima in Caldo da inviare verso gli Slaves
	#define	K_PerformanceLossBattery_SP_Room5       400     // SetPoint Perdita potenza batteria in Caldo da inviare verso gli Slaves
    #define	K_ValveLim_PercMaxOpenValve_Room5       100     // Percentuale di apertura massima valvola
    #define	K_ValveLim_AperturaMinCaldo_Room5       5      // Apertura minima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMaxCaldo_Room5       100     // Apertura massima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMinFreddo_Room5      0       // Apertura minima % in lavoro x Freddo
    #define	K_ValveLim_AperturaMaxFreddo_Room5      80           // Apertura massima % in lavoro x Freddo
	#define	K_TempCorrectSuperHeat_AirOut_Room5     200         // Valore sotto il quale il superheat comincia ad alzarsi per evitare condensa tubi.      

	#define K_FanSpeedNight_Room5                   43
	#define K_FanSpeed1_Room5                       58
	#define K_FanSpeed2_Room5                       73
	#define K_FanSpeed3_Room5                       88
	#define K_FanSpeed4_Room5                       103
	#define K_FanSpeed5_Room5                       118
	#define K_FanSpeed6_Room5                       133
	#define K_FanSpeed7_Room5                       148
#endif

//+++++++++ ROOM 6 +++++++++
	#define	K_AbilSendData_Room6                    1		// Abilita l'invio dei setpoint specificati per la Room
#if K_AbilSendData_Room6==1                         
	#define	K_AbilDinamicSuperHeat_Room6            0		// Abilita sullo slave il SuperHeat Dinamico
#if(K_AbilDinamicSuperHeat_Room6==0)                
    #define	K_SuperHeat_SP_Room6                    700		// Setpoint di SuperHeat da inviare verso gli Slaves
#endif
	#define	K_Temp_Amb_Mid_Heating_Room6            3		// Abilita media temperatura funzionamento in Riscaldamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_Temp_Amb_Mid_Cooling_Room6            2		// Abilita media temperatura funzionamento in Condizionamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_WorkPressCool_SP_Room6                0		// SetPoint Pressione di lavoro in Freddo da inviare verso gli Slaves
    #define K_MinPressCool_SP_Room6                 500     // SetPoint Pressione minima di lavoro in freddo da inviare agli Slaves
	#define	K_WorkPressHeat_SP_Room6                4100	// SetPoint Pressione di lavoro in Caldo da inviare verso gli Slaves
	#define	K_MinPressHeat_SP_Room6                 3100    // SetPoint Pressione minima in Caldo da inviare verso gli Slaves
	#define	K_PerformanceLossBattery_SP_Room6       400     // SetPoint Perdita potenza batteria in Caldo da inviare verso gli Slaves
    #define	K_ValveLim_PercMaxOpenValve_Room6       100     // Percentuale di apertura massima valvola
    #define	K_ValveLim_AperturaMinCaldo_Room6       5      // Apertura minima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMaxCaldo_Room6       100     // Apertura massima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMinFreddo_Room6      0       // Apertura minima % in lavoro x Freddo
    #define	K_ValveLim_AperturaMaxFreddo_Room6      80           // Apertura massima % in lavoro x Freddo
	#define	K_TempCorrectSuperHeat_AirOut_Room6     200         // Valore sotto il quale il superheat comincia ad alzarsi per evitare condensa tubi.      

	#define K_FanSpeedNight_Room6                   43
	#define K_FanSpeed1_Room6                       58
	#define K_FanSpeed2_Room6                       73
	#define K_FanSpeed3_Room6                       88
	#define K_FanSpeed4_Room6                       103
	#define K_FanSpeed5_Room6                       118
	#define K_FanSpeed6_Room6                       133
	#define K_FanSpeed7_Room6                       148
#endif

//+++++++++ ROOM 7 +++++++++
	#define	K_AbilSendData_Room7                    1		// Abilita l'invio dei setpoint specificati per la Room
#if K_AbilSendData_Room7==1                         
	#define	K_AbilDinamicSuperHeat_Room7            0		// Abilita sullo slave il SuperHeat Dinamico
#if(K_AbilDinamicSuperHeat_Room7==0)                
    #define	K_SuperHeat_SP_Room7                    700		// Setpoint di SuperHeat da inviare verso gli Slaves
#endif
	#define	K_Temp_Amb_Mid_Heating_Room7            3		// Abilita media temperatura funzionamento in Riscaldamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_Temp_Amb_Mid_Cooling_Room7            2		// Abilita media temperatura funzionamento in Condizionamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_WorkPressCool_SP_Room7                0		// SetPoint Pressione di lavoro in Freddo da inviare verso gli Slaves
    #define K_MinPressCool_SP_Room7                 500     // SetPoint Pressione minima di lavoro in freddo da inviare agli Slaves
	#define	K_WorkPressHeat_SP_Room7                4100	// SetPoint Pressione di lavoro in Caldo da inviare verso gli Slaves
	#define	K_MinPressHeat_SP_Room7                 3100    // SetPoint Pressione minima in Caldo da inviare verso gli Slaves
	#define	K_PerformanceLossBattery_SP_Room7       400     // SetPoint Perdita potenza batteria in Caldo da inviare verso gli Slaves
    #define	K_ValveLim_PercMaxOpenValve_Room7       100     // Percentuale di apertura massima valvola
    #define	K_ValveLim_AperturaMinCaldo_Room7       5      // Apertura minima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMaxCaldo_Room7       100     // Apertura massima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMinFreddo_Room7      0       // Apertura minima % in lavoro x Freddo
    #define	K_ValveLim_AperturaMaxFreddo_Room7      80           // Apertura massima % in lavoro x Freddo
	#define	K_TempCorrectSuperHeat_AirOut_Room7     200         // Valore sotto il quale il superheat comincia ad alzarsi per evitare condensa tubi.      

	#define K_FanSpeedNight_Room7                   43
	#define K_FanSpeed1_Room7                       58
	#define K_FanSpeed2_Room7                       73
	#define K_FanSpeed3_Room7                       88
	#define K_FanSpeed4_Room7                       103
	#define K_FanSpeed5_Room7                       118
	#define K_FanSpeed6_Room7                       133
	#define K_FanSpeed7_Room7                       148
#endif

//+++++++++ ROOM 8 +++++++++
	#define	K_AbilSendData_Room8                    1		// Abilita l'invio dei setpoint specificati per la Room
#if K_AbilSendData_Room8==1                         
	#define	K_AbilDinamicSuperHeat_Room8            0		// Abilita sullo slave il SuperHeat Dinamico
#if(K_AbilDinamicSuperHeat_Room8==0)                
    #define	K_SuperHeat_SP_Room8                    700		// Setpoint di SuperHeat da inviare verso gli Slaves
#endif
	#define	K_Temp_Amb_Mid_Heating_Room8            3		// Abilita media temperatura funzionamento in Riscaldamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_Temp_Amb_Mid_Cooling_Room8            2		// Abilita media temperatura funzionamento in Condizionamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_WorkPressCool_SP_Room8                0		// SetPoint Pressione di lavoro in Freddo da inviare verso gli Slaves
    #define K_MinPressCool_SP_Room8                 500     // SetPoint Pressione minima di lavoro in freddo da inviare agli Slaves
	#define	K_WorkPressHeat_SP_Room8                4100	// SetPoint Pressione di lavoro in Caldo da inviare verso gli Slaves
	#define	K_MinPressHeat_SP_Room8                 3100    // SetPoint Pressione minima in Caldo da inviare verso gli Slaves
	#define	K_PerformanceLossBattery_SP_Room8       400     // SetPoint Perdita potenza batteria in Caldo da inviare verso gli Slaves
    #define	K_ValveLim_PercMaxOpenValve_Room8       100     // Percentuale di apertura massima valvola
    #define	K_ValveLim_AperturaMinCaldo_Room8       5      // Apertura minima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMaxCaldo_Room8       100     // Apertura massima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMinFreddo_Room8      0       // Apertura minima % in lavoro x Freddo
    #define	K_ValveLim_AperturaMaxFreddo_Room8      80           // Apertura massima % in lavoro x Freddo
	#define	K_TempCorrectSuperHeat_AirOut_Room8     200         // Valore sotto il quale il superheat comincia ad alzarsi per evitare condensa tubi.      

	#define K_FanSpeedNight_Room8                   43
	#define K_FanSpeed1_Room8                       58
	#define K_FanSpeed2_Room8                       73
	#define K_FanSpeed3_Room8                       88
	#define K_FanSpeed4_Room8                       103
	#define K_FanSpeed5_Room8                       118
	#define K_FanSpeed6_Room8                       133
	#define K_FanSpeed7_Room8                       148
#endif

//+++++++++ ROOM 9 +++++++++
	#define	K_AbilSendData_Room9                    1		// Abilita l'invio dei setpoint specificati per la Room
#if K_AbilSendData_Room9==1                         
	#define	K_AbilDinamicSuperHeat_Room9            0		// Abilita sullo slave il SuperHeat Dinamico
#if(K_AbilDinamicSuperHeat_Room9==0)                
    #define	K_SuperHeat_SP_Room9                    700		// Setpoint di SuperHeat da inviare verso gli Slaves
#endif
	#define	K_Temp_Amb_Mid_Heating_Room9            3		// Abilita media temperatura funzionamento in Riscaldamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_Temp_Amb_Mid_Cooling_Room9            2		// Abilita media temperatura funzionamento in Condizionamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_WorkPressCool_SP_Room9                0		// SetPoint Pressione di lavoro in Freddo da inviare verso gli Slaves
    #define K_MinPressCool_SP_Room9                 500     // SetPoint Pressione minima di lavoro in freddo da inviare agli Slaves
	#define	K_WorkPressHeat_SP_Room9                4100	// SetPoint Pressione di lavoro in Caldo da inviare verso gli Slaves
	#define	K_MinPressHeat_SP_Room9                 3100    // SetPoint Pressione minima in Caldo da inviare verso gli Slaves
	#define	K_PerformanceLossBattery_SP_Room9       400     // SetPoint Perdita potenza batteria in Caldo da inviare verso gli Slaves
    #define	K_ValveLim_PercMaxOpenValve_Room9       100     // Percentuale di apertura massima valvola
    #define	K_ValveLim_AperturaMinCaldo_Room9       5      // Apertura minima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMaxCaldo_Room9       100     // Apertura massima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMinFreddo_Room9      0       // Apertura minima % in lavoro x Freddo
    #define	K_ValveLim_AperturaMaxFreddo_Room9      80           // Apertura massima % in lavoro x Freddo
	#define	K_TempCorrectSuperHeat_AirOut_Room9     200         // Valore sotto il quale il superheat comincia ad alzarsi per evitare condensa tubi.      

	#define K_FanSpeedNight_Room9                   43
	#define K_FanSpeed1_Room9                       71
	#define K_FanSpeed2_Room9                       100
	#define K_FanSpeed3_Room9                       105
	#define K_FanSpeed4_Room9                       110
	#define K_FanSpeed5_Room9                       115
	#define K_FanSpeed6_Room9                       120
	#define K_FanSpeed7_Room9                       125
#endif

//+++++++++ ROOM 10 +++++++++
	#define	K_AbilSendData_Room10                    1		// Abilita l'invio dei setpoint specificati per la Room
#if K_AbilSendData_Room10==1                         
	#define	K_AbilDinamicSuperHeat_Room10            0		// Abilita sullo slave il SuperHeat Dinamico
#if(K_AbilDinamicSuperHeat_Room10==0)                
    #define	K_SuperHeat_SP_Room10                    600		// Setpoint di SuperHeat da inviare verso gli Slaves
#endif
	#define	K_Temp_Amb_Mid_Heating_Room10            3		// Abilita media temperatura funzionamento in Riscaldamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_Temp_Amb_Mid_Cooling_Room10            2		// Abilita media temperatura funzionamento in Condizionamento 0=Temperatura da Slave indipendente; 1=Temperatura da Slave+Sottoslave; 2=Temperatura da Slave+Sottoslave+Pannellino; 3=Temperatura da Pannellino
	#define	K_WorkPressCool_SP_Room10                0		// SetPoint Pressione di lavoro in Freddo da inviare verso gli Slaves
    #define K_MinPressCool_SP_Room10                 500     // SetPoint Pressione minima di lavoro in freddo da inviare agli Slaves
	#define	K_WorkPressHeat_SP_Room10                4100	// SetPoint Pressione di lavoro in Caldo da inviare verso gli Slaves
	#define	K_MinPressHeat_SP_Room10                 3100    // SetPoint Pressione minima in Caldo da inviare verso gli Slaves
	#define	K_PerformanceLossBattery_SP_Room10       400     // SetPoint Perdita potenza batteria in Caldo da inviare verso gli Slaves
    #define	K_ValveLim_PercMaxOpenValve_Room10       100     // Percentuale di apertura massima valvola
    #define	K_ValveLim_AperturaMinCaldo_Room10       5      // Apertura minima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMaxCaldo_Room10       100     // Apertura massima % in lavoro x Caldo
    #define	K_ValveLim_AperturaMinFreddo_Room10      0       // Apertura minima % in lavoro x Freddo
    #define	K_ValveLim_AperturaMaxFreddo_Room10      100           // Apertura massima % in lavoro x Freddo
	#define	K_TempCorrectSuperHeat_AirOut_Room10     200         // Valore sotto il quale il superheat comincia ad alzarsi per evitare condensa tubi.      

	#define K_FanSpeedNight_Room10                   43 
	#define K_FanSpeed1_Room10                       59 
	#define K_FanSpeed2_Room10                       75 
	#define K_FanSpeed3_Room10                       91 
	#define K_FanSpeed4_Room10                       107
	#define K_FanSpeed5_Room10                       132
	#define K_FanSpeed6_Room10                       156
	#define K_FanSpeed7_Room10                       180
#endif
