/* 
 * File:   FWSelection.h
 * Author: GianLuca
 *
 * Created on 22 dicembre 2015, 0.23
 */

#ifndef FWSELECTION_H
    #define	FWSELECTION_H
#endif
/*
	#define	K_AbilRechargeOilCaldo			0		// abilita ciclo recupero olio in riscaldamento
	#define	K_AbilRechargeOilFreddo			0		// abilita ciclo recupero olio in raffrescamento

	#define K_AbilErrorCompression			0		// 1=Abilita gestione Errore Compressione 0=Disabilita

	#define K_AbilInverterInit				0//1	// 1=Abilita Inizializzazione parametri inverter 0=Disabilita
*/

    #define K_EnableHiSpeedMyBus_Sec        1       // 1 = HiSpeed (115000Baud) 0 = LowSpeed (57600Baud)

    #define K_EnableHiSpeedModBusTouch_Sec  1       // 1 = HiSpeed (115000Baud) 0 = LowSpeed (57600Baud)

    #define K_EnableModBus_Sec              0       // 1 = Abilita il protocollo ModBUS sul canale "Sec" verso i SubSlave (al posto del MyBUS)

    #define K_EnableAutoSwithU1MyBUS        1       // 1 = Abilita lo switch forzato al protocollo MyBUS sul canale BUS primario (UART1) al primo messaggio MyBUS ricevuto
                                                    // ed inibisce la ricezione dei messagi con protocollo MODBUS fino al prossimo timeout di comunicazione (Comunication Lost)
                                                    // 0 = Lascia abilitato il doppio protocollo MyBUS e MODBUS sempre


//Implementazioni FW in base a tipologia modulo
	#define K_I_Am_StandardSplit            0	//0 = split normale
	#define K_I_Am_UTA_and_InvFAN           1	//1 = UTA Mode + Inverter Fan
	#define K_I_Am_StandardUTA              2	//2 = UTA Mode Fan normale
	#define K_I_Am_InverterFAN              3	//3 = SOLO Inverter Fan
	#define K_I_Am_ManualRegulation         4	//4 = Regolazioni manuali da commutatore collegato su DigIng1 e DigIng2 (tipo Cookpit)
	#define K_I_Am_StandardFAN              5	//5 = SOLO Fan Normale
	#define K_I_Am_Frigo                    6	//6 = Frigo
	#define K_I_Am_Freezer                  7	//7 = Freezer
    #define K_I_Am_Atex_Fan                 8	//8 = Atex Fan
    #define K_I_Am_LoopWater                9	//9 = Loop Water
    #define K_I_Am_On_Off_Fan               10	//10 = On Off Fan
	#define K_I_Am_OnlyTouchComm            254	//254 = Solo Comunicazione Touch

    #define SplitMode                       K_I_Am_StandardSplit

    #define K_Address_Reply                 NULL   //Inserire: NULL=Risponde a qualsiasi indirizzo;1..7,254= risponde al master con l'indirizzo specifico.	
                                                  //ATTENZIONE: Ricordarsi di programmare i sottoslave SEMPRE come NULL!!
                                                  //ATTENZIONE: Ricordarsi di programmare i sottoslave SEMPRE come NULL!!

    #define K_EBM_24V                       0               
    #define K_GR22_230V                     1               
    #define K_GR25_230V                     2               
    #define K_ZIEHL_ABEGG_ModBus            3
    #define K_EBM_3200RPM_ModBus            4

    #define K_VentilSelection               K_GR22_230V

    #define K_New_NetB                      1	// (Old=0; New=1) Definisce se c'è un pannellino NetBuilding nuovo o vecchio per eliminare alcuni registri che al vecchio davano fastidio.

    #define K_Force_Panel_Abil              0
    #define k_Abil_Panel_Shared             0	// (No Shared=0; Shared=1) Abilitala condivisione del pannellino sul Bus secondario in caso di schede nella stesa stanza ma con Master differenti.

    #define K_Panel_On_Uart3                0   // Low  -> P Panel
    #define K_ModBus_Generic_On_Uart3       0   // Mid  -> I Inverter
    #define K_MyBus_On_Uart3                0   // Hi   -> S Subslave

                                            //      SIP
                                            //      -----------------------------------------------------------------------------
                                            //      000     ALL COM2                        Test 2x30 min. = Error 0      *  OK
                                            //      001     Panel COM3                      Test 2x30 min. = Error 0      *  OK
                                            //      010     Inverter COM3                   Test 2x30 min. = Error 0      *  OK
                                            //      011     Panel + Inverter COM3           Test 2x30 min. = Error 0      *  OK
                                            //      100     Subslave COM3                   Test 2x30 min. = Error 0      ** OK
                                            //      101     Subslave + Panel COM3           Test 2x30 min. = Error 0      ** OK
                                            //      110     Subslave + Inverter COM3        Test 2x30 min. = Error 0      ** OK
                                            //      111     ALL COM3                        Test 2x30 min. = Error 0      ** OK
                                            //
                                            //      *       Test di riprogrammazione Subslave via BUS su COM2 eseguito con successo 
                                            //      **      Test di riprogrammazione Subslave via BUS su COM3 eseguito con successo 

#if(SplitMode==K_I_Am_UTA_and_InvFAN || SplitMode==K_I_Am_InverterFAN)
	#define K_InverterMax35HZ				0	//0 = MAX 50Hz; 1 = MAX 35 Hz
#if(K_InverterMax35HZ==0)
	#define K_InverterMax65HZ				0	//0 = MAX 50Hz; 1 = MAX 65 Hz
#endif
#endif

//GL	#define K_SuperHeat_SourceSel			1	//0 = SuperHeat da Slave
												//1 = SuperHeat da Master

//GL	#define K_WorkPress_SourceSel			1	//0 = Working Pressure da Slave
												//1 = Working Pressure da Master

#define	K_PercMaxOpenValve					100	// Percentuale di apertura massima valvola

#define K_ValveNoPressCondenser             0   // 1 = Abilita la vecchia gestione valvola in caldo (senza press Condenser)     0 = Nuova gestione con Press Condenser

#define	FanOldStyleHeatCoreOff				0//1	// 1= Gestione ventola in Riscaldamento CoreOff solita    0= Gestione ventola in in Riscaldamento CoreOff in PWM: 3 min. OFF, 30 Sec. Night

#define	K_Lim_Custom_SP     				0
#if(K_Lim_Custom_SP==1)
	#define	K_Def_SPLIT_SP					3500
	#define	K_LimLo_SPLIT_SP				3000
	#define	K_LimHi_SPLIT_SP				4000
#endif

#if (SplitMode == K_I_Am_Frigo) || (SplitMode == K_I_Am_Freezer)
#define K_ADAuxChannelEnable        		3   //Abilita i canali aux per la media di temperatura ambiente (1=canale amb + aux1; 2=canale amb + aux1 + aux2;  3=aux1 + aux2;  4=aux1)
#else
#define K_ADAuxChannelEnable        		0   //Abilita i canali aux per la media di temperatura ambiente (1=canale amb + aux1; 2=canale amb + aux1 + aux2;  3=aux1 + aux2;  4=aux1)
#endif
#define K_FanBrake_DeltaTempInOut			0
#define K_FanBrake_DeltaTempReqOut			0
#define K_Dinamic_SuperHeat_Default         0

#if (SplitMode == K_I_Am_LoopWater)
#define K_Lim_CorrectSuperHeatAirOut        2500 
#else
#define K_Lim_CorrectSuperHeatAirOut        2000 
#endif

#if (SplitMode == K_I_Am_Frigo) || (SplitMode == K_I_Am_Freezer)
#define K_Default_Temp_CorrectSuperHeatAirOut   -1500       
#elif (SplitMode == K_I_Am_LoopWater)
#define K_Default_Temp_CorrectSuperHeatAirOut   1800   
#else
#define K_Default_Temp_CorrectSuperHeatAirOut   200   
#endif

#define K_Dinamic_SuperHeat_Default         0

#define K_Abil_Flood_Alarm                  0


#define K_Force_Off_No_Proprity_Zone_Eco_Mode 0

#define K_ValveOnDemand_Max_Escursion_100P  1   //Se abilitato l'escursione in limitazione va da 0 a 100% anche se la valvola è limitata
#define K_ValveOnDemand_Min_Percent_On      1   //Se abilitato l'escursione in limitazione quando la valvola dovrebbe esssere chiusa va da 0 a K_ValveOnDemand_Min_Percent_Val%
//#define K_ValveOnDemand_Min_Percent_Val     20   //Valore in % quando K_ValveOnDemand_Min_Percent_On è abilitato
#define K_Max_Escurs_Open_Valve_OnDemand    80

#define K_Force_Lim_Hi_SP_Split             0

#if(K_Force_Lim_Hi_SP_Split==1)
#define K_Forced_Lim_Hi_SP_Split_Hot        1800
//#define K_Forced_Lim_Hi_SP_Split_Cold       3000

#endif

#define K_Disable_Trig_Init                 0    //Se ==1 disabilita l'inizializzazione da Trig Init.

#define K_MinTimeSystemCycle                500 // Tempo minimo di ciclo programma 

#define K_Abil_DigIn_SystemDisable          0   // 1 = Abilita l'ingresso digitale n.3 come parallalo della funzione Touch SystemDisable

#define K_Abil_Off_FireAlarm                1   // 1 = Spegne l'unità se è presente un Fire Alarm.

#if(SplitMode==K_I_Am_StandardSplit)
	#define K_UTAEnable                 	0
    #define K_AbilFanInverter               0   // Flag per abilitare gestiuone FAN da Inverter
	#define K_FrigoEnable                   0	// Modalità Frigo
	#define K_FreezerEnable                 0	// Modalità Freezer
    
    #define K_AbilSplitDefrostingCycle      1

    #define K_Humidity_Read                 0
    #define K_Heater_Abil                   0

#if(K_Humidity_Read==1 && K_Heater_Abil==1)
    #define K_Humidity_Correct              0
#endif
#endif

#if(SplitMode==K_I_Am_UTA_and_InvFAN)
	#define K_UTAEnable                 	1
    #define K_AbilFanInverter               1   // Flag per abilitare gestiuone FAN da Inverter
#if(K_Abil_DigIn_SystemDisable==0)
    #define K_Abil_Fire_Alarm               1
#endif
	#define K_FrigoEnable                   0	// Modalità Frigo
	#define K_FreezerEnable                 0	// Modalità Freezer
    #define K_AbilSplitDefrostingCycle      1
    #define K_Force_SubSlave_Abil           1   // Abilita le comunicazioni verso il sottoslave per ventola secondaria (il sottoslave va solo in ventilazione).                                           
#endif

#if(SplitMode==K_I_Am_StandardUTA)
	#define K_UTAEnable                 	1
    #define K_AbilFanInverter               0   // Flag per abilitare gestiuone FAN da Inverter
#if(K_Abil_DigIn_SystemDisable==0)
    #define K_Abil_Fire_Alarm               0
#endif
	#define K_FrigoEnable                   0	// Modalità Frigo
	#define K_FreezerEnable                 0	// Modalità Freezer
    #define K_Force_SubSlave_Abil           0   // Abilita le comunicazioni verso il sottoslave per ventola secondaria (il sottoslave va solo in ventilazione).                                           
    #define K_SubSlave_Abil_Only_Fan        0   // Abilita le comunicazioni verso il sottoslave per ventola secondaria (il sottoslave va solo in ventilazione).                                           

#if(K_Force_SubSlave_Abil==1)
    #define K_UTA_SubSlave_Delta_Vel        0   // Inserire quante velocità in più il sottoslave deve avere rispetto allo slave (velocità uguale =0)
#endif
    #define K_AbilSplitDefrostingCycle      1

    #define K_DisAbil_TestAll_TempBatt      0
    #define K_DisAbil_TestAll_MaxValve      0
    #define K_DisAbil_TestAll_SuperHeat     0

    #define K_Heater_Abil                   0

#if(K_Heater_Abil==1)
    #define K_Double_Heater_Abil            0
#endif

#endif

#if(SplitMode==K_I_Am_InverterFAN)
	#define K_UTAEnable                 	0
    #define K_AbilFanInverter               1   // Flag per abilitare gestiuone FAN da Inverter
	#define K_FrigoEnable                   0	// Modalità Frigo
	#define K_FreezerEnable                 0	// Modalità Freezer
    #define K_Force_SubSlave_Abil           1   // Abilita le comunicazioni verso il sottoslave per ventola secondaria (il sottoslave va solo in ventilazione).                                           
                                                // NOTA: il sottoslave deve essere programmato come slave normale e non UTA!!
#if(K_Force_SubSlave_Abil==1)
    #define K_UTA_SubSlave_Delta_Vel        0   // Inserire quante velocità in più il sottoslave deve avere rispetto allo slave (velocità uguale =0)
#endif

    #define K_DisAbil_TestAll_TempBatt      0
    #define K_DisAbil_TestAll_MaxValve      0
    #define K_DisAbil_TestAll_SuperHeat     0
#endif

#if(SplitMode==K_I_Am_ManualRegulation)
    #define K_ManualSpeedEnable             1   // Controllo ventilazione:  0 = Disabilitato (regolazione da Touch)    1 = Abilitato (regolazione da commutatore collegato a DigIng1 e DigIng2)
    #define K_ManualEvapTempEnable          1   // Calcolo Evap Temp Manuale:  0 = Disabilitato (regolazione da Touch)    1 = Abilitato (regolazione da commutatore collegato a DigIng1 e DigIng2)
	#define K_FrigoEnable                   0	// Modalità Frigo
	#define K_FreezerEnable                 0	// Modalità Freezer
#endif

#if(SplitMode==K_I_Am_StandardFAN)
	#define K_UTAEnable                 	0
    #define K_AbilFanInverter               0   // Flag per abilitare gestiuone FAN da Inverter
	#define K_FrigoEnable                   0	// Modalità Frigo
	#define K_FreezerEnable                 0	// Modalità Freezer
    #define K_Force_SubSlave_Abil           0   // Abilita le comunicazioni verso il sottoslave per ventola secondaria (il sottoslave va solo in ventilazione).                                           
                                                // NOTA: il sottoslave deve essere programmato come slave normale e non UTA!!
#if(K_Force_SubSlave_Abil==1)
    #define K_UTA_SubSlave_Delta_Vel        0   // Inserire quante velocità in più il sottoslave deve avere rispetto allo slave (velocità uguale =0)
#endif

    #define K_Force_DigiIn1_Ventil          0   // Forza l'accensione della ventola a velocità massima se c'è ingresso 1 attivo.

#if(K_Force_DigiIn1_Ventil==1)
    #define K_Fan_DigiIn1_Vel               7   // Inserire la velocità della ventola da seguire inmodalità estrattore bagni se c'è ingresso 1 attivo.
#endif

    #define K_Channel_Gas_Regulation        0   // Inserire la velocità della ventola da seguire inmodalità estrattore bagni se c'è ingresso 1 attivo.

#if (K_Channel_Gas_Regulation==1)
    #define K_Bypass_System_Disable         1   // Inserire la velocità della ventola da seguire inmodalità estrattore bagni se c'è ingresso 1 attivo.
#endif
    #define K_Channel_Liquid_Light          0   // Inserire la velocità della ventola da seguire inmodalità estrattore bagni se c'è ingresso 1 attivo.

    #define K_Dampers_Selection             0   // Abilita il controllo della serranda prima di attivare la ventilazione. 1=1 serranda (DigiIn1, DigiOut1); 2=2 serrande(DigiIn2, DigiOut2).

    #define K_AutoTempFanControl            0   // Abilita la sonda di temperatura per far estrattori aria (tipo REC).

#if(K_AutoTempFanControl==0)
    #define K_DiffPress_Fan_Control         0   //Modalità manuale o automatica della ventola con pressione differenziale (on-off da touch screen)
#if(K_DiffPress_Fan_Control==1)
    #define K_OnlyOne_Press_Sensor          0   //Indico che il sensore di pressione è SOLO sulo slave e non sul sottoslave. (0=sensore su slave e sottoslave; 1=sensore su slave e basta) ATTENZIONE: PROGRAMMARE SLAVE E SOTTOSLAVE CON STESSO FLAG!!
#endif
    #define K_DiffPress_Auto_Fan_Control    0   //Modalità completamente automatica di funzionamento della ventola in base alla pressione differenziale (on-off automatico)
#if(K_DiffPress_Auto_Fan_Control==1)
    #define K_DiffPress_Auto_Min            5
    #define K_DiffPress_Auto_Max            100
#endif
    #define K_DiffPress_SensorSelection     0        // 0=Premasgard 2121SD,  1=E+E ELEKTRONIK Mod. EE610-HV51A7
#if(K_DiffPress_SensorSelection==0)
    #define K_Pressure_Max_Value            100          //Fondo scala pressione differenziale
#endif
#endif

    #define K_Humidity_Read                 0

    #define K_Abil_Fire_Alarm               0
#endif

#if(SplitMode==K_I_Am_LoopWater)
	#define K_UTAEnable                 	0
    #define K_AbilFanInverter               0   // Flag per abilitare gestiuone FAN da Inverter
	#define K_FrigoEnable                   0	// Modalità Frigo
	#define K_FreezerEnable                 0	// Modalità Freezer

    #define K_LoopPumpEnable                1
    #define K_LoopDoublePumpEnable          1

	#define K_ErrorSPEnable                 1	// Abilito errore se non va in temperatura entro un certo tempo
#if(K_ErrorSPEnable==1)
    #define K_DeltaTemperatureFault         500 // delta di temperatura per errore freezer
    #define K_DeltaRestartTemperatureFault  100 // delta di temperatura per rientro errore freezer
    #define K_TimeTemperatureFault          15*60
#endif

    #define K_AbilSplitDefrostingCycle      1

    #define K_Low_Temp_WaterOut             800

    #define K_Disable_Trig_Init             1    //Se ==1 disabilita l'inizializzazione da Trig Init.
#endif

#if(SplitMode==K_I_Am_InverterFAN)
    #define K_M3h_Min                        500
    #define K_M3h_Max                        2000
#elif(K_VentilSelection==K_GR22_230V)
    #define K_M3h_Min                        100
    #define K_M3h_Max                        1000
#elif(K_VentilSelection==K_GR25_230V)
    #define K_M3h_Min                        100
    #define K_M3h_Max                        1200
#elif(K_VentilSelection==K_ZIEHL_ABEGG_ModBus)
    #define K_M3h_Min                        100
    #define K_M3h_Max                        3000
#elif(K_VentilSelection==K_EBM_3200RPM_ModBus)
    #define K_M3h_Min                        100
    #define K_M3h_Max                        3000
#else
    #define K_M3h_Min                        80
    #define K_M3h_Max                        600
#endif

#if (SplitMode==K_I_Am_Frigo)
	#define K_FrigoEnable                   1	// Modalità Frigo
	#define K_FreezerEnable                 0	// Modalità Freezer
    #define K_Gas_Valve_Control             0	// Controllo con valvola gas
    #define K_TimeTemperatureFault          60*60
    #define K_DeltaTemperatureFault         500 // delta di temperatura per errore freezer
    #define K_DeltaRestartTemperatureFault  100 // delta di temperatura per rientro errore freezer
#if(K_Gas_Valve_Control==1)
    #define K_Press_Close_Valve_Control     10000	// Massima pressione di lavoro consentita
#endif
    #define K_AbilSplitDefrostingCycle          0
    #define K_Defr_Res_Power                    1000    //Potenza in watt della resistenza di defrosting
    #define K_Defr_Res_Voltage                  230     //Alimentazione della resistenza di defrosting
    #define K_Evaporator_FAN                    0  // 1= Abilita uscita digitale n.xx con unità accesa, 0=uscita disabilitata
    #define K_SuperHeatEngBox                   1
#endif

#if (SplitMode==K_I_Am_Freezer)
	#define K_FreezerEnable                 1	// Modalità Freezer
	#define K_FrigoEnable                   0	// Modalità Frigo
    #define K_Gas_Valve_Control             0	// Controllo con valvola gas
    #define K_TimeTemperatureFault          60*60
    #define K_DeltaTemperatureFault         500 // delta di temperatura per errore freezer
    #define K_DeltaRestartTemperatureFault  100 // delta di temperatura per rientro errore freezer

#if(K_Gas_Valve_Control==1)
    #define K_Press_Close_Valve_Control     10000	// Massima pressione di lavoro consentita
#endif
    #define K_AbilSplitDefrostingCycle          1
    #define K_Defr_Res_Power                    1000    //Potenza in watt della resistenza di defrosting
    #define K_Defr_Res_Voltage                  230     //Alimentazione della resistenza di defrosting
    #define K_Evaporator_FAN                    0   // 1= Abilita uscita digitale n.xx con unità accesa, 0=uscita disabilitata
    #define K_SuperHeatEngBox                   1
#endif

#if(SplitMode==K_I_Am_OnlyTouchComm)
	#define K_UTAEnable                 	0
    #define K_AbilFanInverter               0   // Flag per abilitare gestiuone FAN da Inverter
	#define K_FrigoEnable                   0	// Modalità Frigo
	#define K_FreezerEnable                 0	// Modalità Freezer

    #define K_Abil_Fire_Alarm               1
#endif

#if (SplitMode == K_I_Am_Frigo) || (SplitMode == K_I_Am_Freezer) || (SplitMode == K_I_Am_LoopWater)
	#define	K_TachoSpeedAlarmDisable		1	// Disabilità l'allarme di errore velocità Tacho
#else
	#define	K_TachoSpeedAlarmDisable        0	// Disabilità l'allarme di errore velocità Tacho
#endif

    #define K_Temp_Only_Slave                    0  //Temperatura da Slave indipendente
    #define K_Temp_Middle_SubSlave               1  //Temperatura da Slave + Sottoslave
    #define K_Temp_Middle_SubSlave_And_Panel     2  //Temperatura da Slave + Sottoslave + Pannellino
    #define K_Temp_Only_Panel                    3  //Temperatura da Pannellino

#if((SplitMode == K_I_Am_UTA_and_InvFAN) || (SplitMode == K_I_Am_StandardUTA))
	#define K_Mid_SlaveSec_Temp_Amb_Function_Heating     K_Temp_Only_Slave 
	#define K_Mid_SlaveSec_Temp_Amb_Function_Cooling     K_Temp_Only_Slave 
#else
	#define K_Mid_SlaveSec_Temp_Amb_Function_Heating     K_Temp_Middle_SubSlave_And_Panel 
	#define K_Mid_SlaveSec_Temp_Amb_Function_Cooling     K_Temp_Middle_SubSlave 
#endif

    #define K_DoublePump_Abil                0

    #define VentilMode                       1
    #define CoolMode                         2
    #define HotMode                          4
    #define AutoMode                         8

    #define K_Mode_Abil                      VentilMode+CoolMode+HotMode //Sommare le modalità da visulaizzare sul touch.

//    #define K_AbilFanInverter               1//0   // Flag per abilitare gestiuone FAN da Inverter

    #define K_Supply_Inverter               200         // Tensione di alimentazione Inverter in V (Serve per il cacolo della tensione In/Out di lavoro che viene restituita dall'inverter solo in percentuale!)
                                                        // Dal Manuale: Tensione in uscita:  
                                                        // La tensione visualizzata è la tensione di comando uscita. 
                                                        // Il valore di riferimento (valore 100%) è 200 V per i modelli da 240 V e 
                                                        // 400 V per i modelli da 500 V

    #define K_Fixed_EcoMode                 0           // 0 = valore ecomode variabile; 1 = Valore ecomode fisso.
    #define K_Def_SPLIT_PwrEcoMode			400         //Inizializzo potenza in EcoMode


	#define K_DEBUG							0
	#define K_DEBUG_LCD						0
	#define K_TEST							0
	#define K_DEVELOP						1		// Esegue il codice per il test dell'Hardware (HW > v8.2)  

	#define	K_MaxCommErrorAlarm             10		// Numero di errori di comunicazione (Mybus o ModBus) massimi prima di segnalare anomalia con accensione LED Rosso

	#define K_SIMULATION_WORK				0		// Include/esclude parti di codice per permettere il collaudo del FW senza essere su un'impianto "VERO"		
