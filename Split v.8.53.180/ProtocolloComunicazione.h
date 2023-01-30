//----------------------------------------------------------------------------------
//	Progect name:	ProtocolloComunicazione.h
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			TDM Team
//	Date:			04/01/2014
//	Description:	Header delle funzioni che gestiscono la comunicazione con il sistema
//----------------------------------------------------------------------------------

#ifndef _PROTOCOLLO_COMUNICAZIONE_H_
	#define _PROTOCOLLO_COMUNICAZIONE_H_
	
	#include "ADC.h"
	#include "EEPROM.h"
	#include "PWM.h"
	#include "Valvola_PassoPasso.h"
	#include "Core.h"
	#include "Driver_Comunicazione.h"


	//----------------------------------------------------------------------------------
	//	Define
	//----------------------------------------------------------------------------------
	//#define Indirizzo_Modulo	My_Address
	#define	k_Split_Master_Add	1			// Indirizzo Slave master 
    #define k_Broadcast_Addr    0
    #define K_Address_MasterDefault    254//0xFE
	#define	K_Address_Consolle         255//0xFF



	#define	AbilRX_All			0           // 1 = Abilita la modalità risponditutto per il range di indirizzi specificato da "MinRX_All" e "MaxRX_All" (solo per DEBUG))
	#define	MinRX_All			1           
	#define	MaxRX_All			8


	#define Versione_Hw			8			// Compatibilita con HW (Slave v8.2 o sup.)
	#define Versione_Fw			53			// Compatibilita con Protocolli BUS/ModBUS
	#define Revisione_Fw		180			// Revisione FW (modifiche minori o bachi)
	// header
	#define	StartPack			0x55		// per l'auto BaudRate
	#define	Ack					0x8000		// dato di ack
	#define	N_Ack				0xFFFF		// dato di non ack

    #define K_MagicNumber       0x1234      // Magic Number x gestione funzione Autoreset da comando MyBUS
    
    #define K_Baud57600         0       // 1 = HiSpeed (57600Baud)
    #define K_Baud115200        1       // 2 = HiSpeed (115200Baud)
    #define K_Baud250000        2       // 3 = HiSpeed (250000Baud)

// Define per Change_Protocol
	#define	K_MyBus_Baud115200     0   // MyBUS 115200
	#define	K_MyBus_Baud57600      1   // MyBUS 57600
	#define	K_ModBus_Baud38400     2   // ModBUS 38400
	#define	K_ModBus_Baud57600     3   // ModBUS 57600
	#define	K_ModBus_Baud115200    4   // ModBUS 115200
	#define	K_MyBus_Baud250000     5   // MyBUS 250000
	#define	K_ModBus_Baud9600      6   // ModBUS 9600
	#define	K_ModBus_Baud19200     7   // ModBUS 19200


#if(K_MyBus_On_Uart3==0)
	#define	K_MyBus                0   // Comunico con un dispositivo my bus su Uart 2
	#define	K_ModBus_MyBus         0   // Comunico con un dispositivo my bus ma che parla in modbus su Uart 2
#else
	#define	K_MyBus                1   // Comunico con un dispositivo my bus su Uart 3
	#define	K_ModBus_MyBus         1   // Comunico con un dispositivo my bus ma che parla in modbus su Uart 3
#endif

#if(K_ModBus_Generic_On_Uart3==0)
	#define	K_ModBus_Generic       0   // Comunico con un dispositivo modbus generico Uart 2
#else
	#define	K_ModBus_Generic       1   // Comunico con un dispositivo modbus generico Uart 3
#endif

#if(K_Panel_On_Uart3==0)
	#define	K_ModBus_Panel         0   // Comunico con un pannello exor...syxthsense...netbuilding... Uart 2
#else
	#define	K_ModBus_Panel         1   // Comunico con un pannello exor...syxthsense...netbuilding... Uart 3
#endif

	
	// comandi
	#define	Prg					0x01
	#define	ReadReg				0x02	
	#define	WriteRegBrdCst		0x03
	#define	WriteReg			0x04
	#define	WriteSplitRegBrdCst	0x05
	#define	WriteDimmRegBrdCst	0x06
	#define	WriteEngRegBrdCst	0x07
	#define	Check				0x08

	// registri locali
	#define	REG_SPLIT_TEMP_BULBO_GRANDE			1			// Temperatura bulbo grande
	#define	REG_SPLIT_TEMP_BULBO_PICCOLO		2			// Temperatura bulbo piccolo
	#define	REG_SPLIT_TEMP_ARIA_USCITA			3			// Temperatura arian in uscita
	#define	REG_SPLIT_TEMP_AMBIENTE				4			// Temperatura aria ambiente
	#define REG_SPLIT_PRES_BULBO_GRANDE			5			// Pressione bulbo grande
	#define REG_SPLIT_PRES_BULBO_PICCOLO		6			// Pressione bulbo piccolo
	#define REG_SPLIT_I_FAN						7			// Corrente ventilatore
	#define REG_SPLIT_YOU_ARE_SUBSLAVE			8			// Flag che dice ai sub split slave che sono split slave.
    #define REG_SPLIT_MY_SPLITMASTER_ADDRESS    9			// Indirizzo del mio Slave Master
    #define REG_SPLIT_TEMP_AMB_MIDDLE		    10			// Indirizzo della temperatura ambiente mediata con sottoslave
    #define REG_SPLIT_TEMP_AMB_REAL 		    11			// Indirizzo della temperatura ambiente reale utilizzata (mediata con sottoslave o no in mbas ea FW Selection)


	#define	REG_SPLIT_TEMP_BULBO_GRANDE_ADC		17 			// Valore ADC Temperatura bulbo grande
	#define	REG_SPLIT_TEMP_BULBO_PICCOLO_ADC	18			// Valore ADC Temperatura bulbo piccolo
	#define	REG_SPLIT_TEMP_ARIA_USCITA_ADC		19			// Valore ADC Temperatura arian in uscita
	#define	REG_SPLIT_TEMP_AMBIENTE_ADC			20			// Valore ADC Temperatura aria ambiente
	#define REG_SPLIT_PRES_BULBO_GRANDE_ADC		21			// Valore ADC Pressione bulbo grande
	#define REG_SPLIT_PRES_BULBO_PICCOLO_ADC	22			// Valore ADC Pressione bulbo piccolo
	#define REG_SPLIT_I_FAN_ADC					23			// Valore ADC Corrente ventilatore

	#define	REG_SPLIT_DIGIT_INPUT				33			// Stato degli ingressi digitali
	#define	REG_SPLIT_DIGIT_OUTPUT				34			// Stato delle uscite digitali
	#define	REG_SPLIT_PWM_REQ_FAN_VALUE			35			// Valore richiesto pwm ventil
	#define	REG_SPLIT_PWM_ACT_FAN_VALUE			36			// Valore istantaneo pwm ventil
	#define	REG_SPLIT_FAN_TACHO					37			// Valore in rivoluzioni per minuto del ventilatore
	#define	REG_SPLIT_VENTIL_MODE				38			// Velocità di ventilazione 0->Spento; 1->Night; 2->Speed1; ...; 8->Speed7; 9->Auto;
	#define	REG_SPLIT_FUNCTION_MODE				39			// Modalità corrente di lavoro per modalità auto (Off, ventil, raffrescamento, riscaldamento, Master-Auto)
	#define	REG_SPLIT_CURRENT_SP				40			// SP corrente per il calcolo della velocità in modalità auto
	#define REG_SPLIT_CURRENT_EVAP_TEMP			41			// Temperatura corrente di evaporazione
	#define REG_SPLIT_CURRENT_EVAP_PRESURE		42			// Pressione corrente di evaporazione

	#define	REG_SPLIT_VALVOLA_CUR_POS			43			// Registro con la posizione corrente della valvola
	#define	REG_SPLIT_VALVOLA_SET_POS			44			// Registro con la posizione desiderata della valvola
	#define	REG_SPLIT_VALV_STP_REF_TIME			45			// Registro con il tempo di aggiornamento tra un passo e l'altro
	#define	REG_SPLIT_VALV_MAX_STEP				46			// Registro con il massimo numero di passi
	#define	REG_SPLIT_CORRENTE_MAX_VALV			47			// Registro con il valore di corrente della valvola ADC
	#define	REG_SPLIT_CORRENTE_MIN_VALV			48			// Registro con il valore di corrente della valvola ADC
	#define	REG_SPLIT_THR_MIN_AMP_VALV			49			// Registro con il valore minimo di corrente della valvola
	#define	REG_SPLIT_THR_MAX_AMP_VALV			50			// Registro con il valore massimo di corrente della valvola
	#define	REG_SPLIT_DEFINE_MODE				51			// Modalità di lavoro selezionata sul touch anche se non attivo lil flag di acensione
	#define	REG_SPLIT_MASTER_STATUS             52			// Flags di stato dello Split Master
    #define REG_SPLIT_POWER                     53			// Flags di stato On/Off dello split (Touch[0].Script_Split_Pwr)
    #define REG_SPLIT_CUMULATIVE_ALARM          54			// Flags di stato Allarmi (Almeno un allarme presente sullo split)
	#define	REG_SPLIT_VALVOLA_ERR_POS			55			// Registro con la posizione in errore richiesta alla valvola
	#define	REG_SPLIT_VALVOLA_ERR_COD			56			// Registro con il codice di errore che richiede la posizione della valvola

	#define REG_SPLIT_ENGINE_BOX_WATER_PUMP_SPEED_PERC	64 	// Velocità della pompa dell'acqua in %

	#define REG_SPLIT_ENGINE_BOX_COMPRESSOR_SPEED	65		// Velocità del compressore
	#define REG_SPLIT_ENGINE_BOX_WATER_PUMP_SPEED	66		// Velocità della pompa dell'acqua
	#define	REG_SPLIT_ENGINE_BOX_FLUX				67		// Lettura Flussimetro circuito acqua mare
	#define REG_SPLIT_CURRENT_MODE_ENGINE_BOX		68		// Modalità corrente del box motore (fermo, clima, riscaldamento, avvio clima, avvio riscladamento)
	#define	REG_SPLIT_CURRENT_REQ_PRESSURE			69		// Pressione richiesta al box motore
	#define	REG_SPLIT_ENGINE_BOX_T_COMP_OUT			70		// Temperatura mandata compressore					
	#define	REG_SPLIT_ENGINE_BOX_T_COMP_SUCTION		71		// Temperatura ritorno compressore				
	#define	REG_SPLIT_ENGINE_BOX_T_SEA_WATER		72		// Temperatura acqua mare in ingresso alla pompa	
	#define	REG_SPLIT_ENGINE_BOX_T_CONDENSER		73		// Temperatura condensatore							
	#define	REG_SPLIT_ENGINE_BOX_T_GAS				74		// Temperatura tubo grande							
	#define	REG_SPLIT_ENGINE_BOX_T_LIQUID			75		// Temperatura tubo Piccolo							
	#define	REG_SPLIT_ENGINE_BOX_P_GAS				76		// Pressione tubo grande							
	#define	REG_SPLIT_ENGINE_BOX_P_LIQUID			77		// Pressione tubo piccolo							
	#define	REG_SPLIT_ENGINE_BOX_PRESSURE_VRV_BOX	78		// Pressione serbatoio VRV
	#define	REG_SPLIT_ENGINE_BOX_PRESSURE_AUX		79		// Pressione aux
	#define REG_SPLIT_ENGINE_BOX_RELE_SATUS			80		// Stato dei Rele
	#define REG_SPLIT_ENGINE_BOX_INPUT_STATUS		81		// Stato degli input
	#define REG_SPLIT_ENGINE_BOX_SYS_ID_LO			82		// System ID lower
	#define REG_SPLIT_ENGINE_BOX_SYS_ID_HI			83		// System ID higher
	//#define REG_SPLIT_ERRORE_1_BOX_MOTORE			84		// registro con gli errori del box motore
	//#define REG_SPLIT_ERRORE_PERS1_BOX_MOTORE		85		// registro con gli errori del box motore
	#define REG_SPLIT_STATUS_BOX_MOTORE				86		// stato della macchina a stati del box motore
	#define REG_SPLIT_ID_SPLIT_MASTER				87		// ID dello split master
	#define REG_SPLIT_POWER_LIMIT					88		// Limite di potenza impostata sul master
	#define REG_SPLIT_POWER_LIMIT_REQ				89		// Limite di potenza richiesta dallo split
	#define REG_SPLIT_STATUS						90		// Stato dello split, che si legge il box motore
	#define REG_SPLIT_ENGINE_BOX_CYCLE_TIME			91		// Tempo di esecuzione ciclo dell' unita Master (invio su Split per diagnostica)
	#define REG_SPLIT_ENGINE_BOX_HOURS_OF_LIFE		92		// Ore di vita dell' unita Master (invio su Split per diagnostica)
    #define REG_SPLIT_ENGINE_BOX_TOUCH_NATION       93		// Identificazione Nazione da inviare sul touch per modificare dinamicamente gli oggetti e unità di misura sul touch a seconda della Nazione (USA/EUR)	

    #define REG_DIAG_SUB_SPLIT_QUANTITY             94 		// r 
    #define REG_DIAG_SPLIT_RES_REQ_ADDRESS          95 		// r   DiagnosticSplit.Reset_Req_Address -> indirizzo dello slave su cui effettuare il reset allarmi proveniente dallo slave master
	#define	REG_DIAG_SPLIT_TEST_ADDRESS				96 		// r	#v16 // indirizzo dello split Master (dotato di touch x diagnostica) #v16
	#define	REG_DIAG_SPLIT_STATUS					97 		// w	
	#define	REG_DIAG_SPLIT_LIQ_TEMP					98 		// w
	#define	REG_DIAG_SPLIT_GAS_TEMP					99 		// w
	#define	REG_DIAG_SPLIT_AIR_IN					100		// w
	#define	REG_DIAG_SPLIT_AIR_OUT					101		// w
	#define	REG_DIAG_SPLIT_FAN_SPEED				102		// w
	#define	REG_DIAG_SPLIT_EXP_VALVE				103		// w
	#define	REG_DIAG_SPLIT_GAS_PRESSURE				104		// w
	#define	REG_DIAG_SPLIT_LIQ_PRESSURE				105		// w
	#define	REG_DIAG_SPLIT_REQUEST_AIR_TEMP			106		// w
	#define	REG_DIAG_SPLIT_REQUEST_AIR_PRES			107		// w
	#define	REG_DIAG_SPLIT_ENG_MODE					108		// w	
	#define	REG_DIAG_SPLIT_CYCLE_TIME				109		// w

	#define	REG_DIAG_INVERTER_ALARM					110		// w	 
	#define	REG_DIAG_INVERTER_OUT_FREQ				111		// w	 
	#define	REG_DIAG_INVERTER_STATUS				112		// w	 
	#define	REG_DIAG_INVERTER_OUT_CURRENT			113		// w 
	#define	REG_DIAG_INVERTER_IN_VOLTAGE			114		// w	 
	#define	REG_DIAG_INVERTER_OUT_VOLTAGE			115		// w	 
	#define	REG_DIAG_INVERTER_IN_POWER				116		// w
	#define	REG_DIAG_INVERTER_OUT_POWER				117		// w
	#define	REG_DIAG_INVERTER_REQ_FREQ				118		// w
	#define	REG_DIAG_INVERTER_MAX_REQ_FREQ			119		// w
    #define REG_DIAG_EVAP_TEMP_VALVE    			120		// w

	#define REG_SPLIT_ERRORE_1_BOX_MOTORE			121		// w	registro con gli errori del box motore
	#define REG_SPLIT_ERRORE_PERS1_BOX_MOTORE		122		// w	registro con gli errori persistenti 1 del box motore
	#define REG_SPLIT_ERRORE_PERS2_BOX_MOTORE		123		// w	registro con gli errori persistenti 2 del box motore
	#define	REG_DIAG_INVERTER_POWERINV				124		// w	
	#define REG_SPLIT_ERRORE_2_BOX_MOTORE			125		// w	registro con gli errori del box motore
	#define REG_SPLIT_CUMULATIVE_ERROR				126		// w	registro con i cumulativi errori normali e persistenti
	#define REG_DIAG_SPLIT_CMD_ADDRESS				127		// w	

	#define REG_SPLIT_PWM_VALUE_OFF					128		// Registro con il valore PWM per la velocità Spento
	#define REG_SPLIT_PWM_VALUE_NIGHT				129		// Registro con il valore PWM per la velocità Nigth
	#define REG_SPLIT_PWM_VALUE_SPEED1				130		// Registro con il valore PWM per la velocità 1
	#define REG_SPLIT_PWM_VALUE_SPEED2				131		// Registro con il valore PWM per la velocità 2
	#define REG_SPLIT_PWM_VALUE_SPEED3				132		// Registro con il valore PWM per la velocità 3
	#define REG_SPLIT_PWM_VALUE_SPEED4				133		// Registro con il valore PWM per la velocità 4
	#define REG_SPLIT_PWM_VALUE_SPEED5				134		// Registro con il valore PWM per la velocità 5
	#define REG_SPLIT_PWM_VALUE_SPEED6				135		// Registro con il valore PWM per la velocità 6
	#define REG_SPLIT_PWM_VALUE_SPEED7				136		// Registro con il valore PWM per la velocità 7


// Registri diagnostici SUB Split letti dal Master e inviati su Split Master per la visualizzazione su Touch
    #define REG_SUB_SPLIT_EVAP_TEMP_VALVE           379		// R   
    #define REG_SUB_SPLIT_SETP_SUPERHEAT            380		// R   
    #define REG_SUB_SPLIT_CUMULATIVE_ALARM			381		// R   
    #define REG_SUB_SPLIT_SUPERHEAT					382		// R   
    #define REG_SUB_SPLIT_SUPERHEAT_RISCALDAMENTO   383		// R   
    #define REG_SUB_SPLIT_RES_REQ_ADDRESS           384		// R   
	#define	REG_SUB_SPLIT_TEST_ADDRESS				385		// R	
	#define	REG_SUB_SPLIT_STATUS					386		// R	
	#define	REG_SUB_SPLIT_LIQ_TEMP					387		// R
	#define	REG_SUB_SPLIT_GAS_TEMP					388		// R
	#define	REG_SUB_SPLIT_AIR_IN					389		// R
	#define	REG_SUB_SPLIT_AIR_OUT					390		// R
	#define	REG_SUB_SPLIT_FAN_SPEED                 391		// R
	#define	REG_SUB_SPLIT_EXP_VALVE                 392		// R
	#define	REG_SUB_SPLIT_GAS_PRESSURE				393		// R
	#define	REG_SUB_SPLIT_LIQ_PRESSURE				394		// R
	#define	REG_SUB_SPLIT_REQUEST_AIR_TEMP			395		// R
	#define	REG_SUB_SPLIT_REQUEST_AIR_PRES			396		// R
	#define	REG_SUB_SPLIT_ENG_MODE					397		// R	
	#define	REG_SUB_SPLIT_CYCLE_TIME				398		// R
    #define REG_SUB_SPLIT_QUANTITY                  399		// R

    #define REG_SPLIT_EVAP_TEMP_VALVE               400		// R
															
    #define REG_SPLIT_ACC_INVERTER					401		// R
    #define REG_SPLIT_ACC_GASPRESS					402		// R
															
	#define	REG_MASTER_HW_VERSION					403		// R
	#define	REG_MASTER_FW_VERSION_HW				404		// R
	#define	REG_MASTER_FW_VERSION_FW				405		// R
	#define	REG_MASTER_FW_REVISION_FW				406		// R
															
	#define	REG_SPLIT_HW_VERSION					407		// R
	#define	REG_SPLIT_FW_VERSION_HW					408		// R
	#define	REG_SPLIT_FW_VERSION_FW					409		// R
	#define	REG_SPLIT_FW_REVISION_FW				410		// R
															
	#define	REG_SUB_SPLIT_HW_VERSION				411		// R
	#define	REG_SUB_SPLIT_FW_VERSION_HW				412		// R
	#define	REG_SUB_SPLIT_FW_VERSION_FW				413		// R
	#define	REG_SUB_SPLIT_FW_REVISION_FW			414		// R

	#define	REG_DIAG_SPLIT_HW_VERSION				415		// R
    #define REG_SUB_SPLIT_FLAGS_ENABLE_1_16         416		// R
    #define REG_SUB_SPLIT_FLAGS_OFFLINE_1_16        417		// R
    #define REG_SUB_SPLIT_FLAGS_ONLINE_1_16         418		// R
    #define REG_SUB_SPLIT_FLAGS_ALARM_1_16          419		// R
    #define REG_DIAG_SUB_SPLIT_FLAGS_ENABLE_1_16    420		// R
    #define REG_DIAG_SUB_SPLIT_FLAGS_OFFLINE_1_16   421		// R
    #define REG_DIAG_SUB_SPLIT_FLAGS_ONLINE_1_16    422		// R
    #define REG_DIAG_SUB_SPLIT_FLAGS_ALARM_1_16     423		// R
    #define REG_SUB_SPLIT_TEMP_AMB_MID              424		// R
    #define REG_DIAG_SPLIT_TEMP_AMB_MIDDLE			425		// R      // Indirizzo della temperatura ambiente mediata con sottoslave
	// Liberi....
    #define REG_DIAG_SPLIT_TEMP_AMB_REAL   			432		// R      // Indirizzo della temperatura ambiente Reale (mediata con sottoslave o meno da FWSelection)
    #define REG_DIAG_SPLIT_FUNCTION_MODE            433		// R
    #define REG_SUB_SPLIT_FUNCTION_MODE             434		// R
	#define	REG_SUB_SPLIT_EXP_VALVE_ERR_POS         435		// R
	#define	REG_SUB_SPLIT_EXP_VALVE_ERR_COD         436		// R
	#define	REG_DIAG_SPLIT_EXP_VALVE_ERR_POS        437		// R
	#define	REG_DIAG_SPLIT_EXP_VALVE_ERR_CODE       438		// R
	// Liberi....
	#define	REG_DIAG_SPLIT_FW_VERSION_HW			512		// R
	#define	REG_DIAG_SPLIT_FW_VERSION_FW			513		// R
	#define	REG_DIAG_SPLIT_FW_REVISION_FW			514		// R

	#define	REG_SPLIT_CURRENT_MODE_ENGINE_BOX2		515		// R
	#define	REG_SPLIT_ENGINE_BOX2_T_COMP_OUT		516		// R
	#define	REG_SPLIT_ERRORE_1_BOX2_MOTORE			517		// R
	#define	REG_SPLIT_ERRORE_PERS1_BOX2_MOTORE		518		// R
	#define	REG_SPLIT_ERRORE_PERS2_BOX2_MOTORE		519		// R
	#define	REG_SPLIT_CUMULATIVE_ERROR2				520		// R
	#define	REG_SPLIT_STATUS_BOX2_MOTORE			521		// R
	// Liberi....
	#define	REG_SPLIT_DOUBLE_COMPRESSSOR_OK			528		// R

	#define REG_DIAG_SPLIT_SUPERHEAT				529		// R
	#define REG_DIAG_SPLIT_SUPERHEAT_RISCALDAMENTO	530		// R
	#define REG_SPLIT_SUPERHEAT						531		// R
	#define REG_SPLIT_SUPERHEAT_RISCALDAMENTO		532		// R

	#define REG_DIAG_SPLIT_SETP_SUPERHEAT       	533		// R
	#define REG_SPLIT_SETP_SUPERHEAT        		534		// R

	#define	REG_SPLIT_POWER_MAX_AVAILABLE			536		// R
    #define REG_SPLIT_POWER_ECOMODE					537		// R    // Valore di limitazione potenza in Eco Mode
    #define REG_SPLIT_ECOMODE_ENABLE				538		// R    // Abilitazione Lim. Potenza ECO Mode		  
	#define REG_SPLIT_INVERTER_QUANTITY				539		// R
	#define REG_DIAG_INVERTER_ADDRESS				540		// R
	#define REG_SPLIT_BOX_ABSORPTION				541		// R
	#define REG_SPLIT_ENGINE_BOX2_COMPRESSOR_SPEED	542		// R
	#define REG_SPLIT_MAX_DIAG_BOX_QUANTITY			543		// R

	#define REG_SPLIT_TO_INIT_SP_ROOM				544    
	#define REG_SPLIT_TO_INIT_FAN_SPEED				545
	#define REG_SPLIT_TO_INIT_POWER					546
	#define REG_SPLIT_TO_INIT_SP_ROOM_F				547
	#define REG_SPLIT_TO_INIT_DEFMODE				548
	#define REG_SPLIT_TO_TRIGGER_INIT				549

	#define REG_SPLIT_FROM_INIT_SP_ROOM				550    
	#define REG_SPLIT_FROM_INIT_FAN_SPEED			551
	#define REG_SPLIT_FROM_INIT_POWER				552
	#define REG_SPLIT_FROM_INIT_SP_ROOM_F			553
	#define REG_SPLIT_FROM_INIT_DEFMODE				554
	#define REG_SPLIT_FROM_TRIGGER_INIT				555

	#define REG_SPLIT_SYNC_RD_SETP					556
	#define REG_SPLIT_SYNC_RD_FANMODE				557
	#define REG_SPLIT_SYNC_RD_SETPF					558
	#define REG_SPLIT_SYNC_RD_PWR					559
	#define REG_SPLIT_SYNC_RD_ECOMEN				560
	#define REG_SPLIT_SYNC_RD_PWRECOM				561

	#define REG_SPLIT_SYNC_WR_SETP					562
	#define REG_SPLIT_SYNC_WR_FANMODE				563
	#define REG_SPLIT_SYNC_WR_SETPF					564
	#define REG_SPLIT_SYNC_WR_PWR					565
	#define REG_SPLIT_SYNC_WR_ECOMEN				566
	#define REG_SPLIT_SYNC_WR_PWRECOM				567

	#define REG_SPLIT_WR_ECOMODE_STATUS				568

	#define REG_DIAG_SPLIT_RD_SET_POWER				569
	#define REG_DIAG_SPLIT_RD_SET_SETPOINT			570
	#define REG_DIAG_SPLIT_RD_SET_FANSPEED			571
	#define REG_DIAG_SPLIT_RD_SET_SETPOINT_F		572
	#define REG_DIAG_SPLIT_WR_SET_POWER				573
	#define REG_DIAG_SPLIT_WR_SET_SETPOINT			574
	#define REG_DIAG_SPLIT_WR_SET_FANSPEED			575
	#define REG_DIAG_SPLIT_WR_SET_SETPOINT_F		576

	#define REG_SPLIT_RD_MODE						577
	#define REG_SPLIT_RD_UTA_POWER					578
	#define REG_SPLIT_RD_UTA_MODE					579
	#define REG_SPLIT_RD_UTA_SP_ROOM				580
	#define REG_SPLIT_RD_UTA_SP_HUMI				581
	#define REG_SPLIT_RD_UTA_FAN_SPEED				582
	#define REG_SPLIT_RD_UTA_SP_ROOM_F				583

	#define REG_SPLIT_WR_MODE						584
	#define REG_SPLIT_WR_UTA_POWER					585
	#define REG_SPLIT_WR_UTA_MODE					586
	#define REG_SPLIT_WR_UTA_SP_ROOM				587
	#define REG_SPLIT_WR_UTA_SP_HUMI				588
	#define REG_SPLIT_WR_UTA_FAN_SPEED				589
	#define REG_SPLIT_WR_UTA_SP_ROOM_F				590

	#define REG_SPLIT_UTA_ABIL						591

    #define REG_SPLIT_UTA_CLEAR_ERROR_BOX           596

	#define REG_SPLIT_MASTER_RESTART_TIME			597

    #define REG_DIAG_SPLIT_UTA_ABIL                 598
    //#define REG_SPLIT_UTA_ABIL                      599

    #define REG_SPLIT_UTAH_ENGINECOMPSPEED			600  				// UTA_H.EngineCompSpeed	            
	#define REG_SPLIT_UTAH_ENGINEWATERPUMP			601  				// UTA_H.EngineWaterPump	              			
	#define REG_SPLIT_UTAH_ENGINEREQPRESSURE	    602  				// UTA_H.EngineReqPressure	                            
	#define REG_SPLIT_UTAH_ENGINETEMPOUT            603  				// UTA_H.EngineTempOut                     			
	#define REG_SPLIT_UTAH_ENGINETEMPIN			    604  				// UTA_H.EngineTempIn			                          
	#define REG_SPLIT_UTAH_ENGINETEMPAIR			605  				// UTA_H.EngineTempAir			                          
	#define REG_SPLIT_UTAH_ENGINETEMPBATT		    606  				// UTA_H.EngineTempBatt		                          
	#define REG_SPLIT_UTAH_ENGINETEMPGAS		    607  				// UTA_H.EngineTempGas		                          
	#define REG_SPLIT_UTAH_ENGINETEMPLIQ			608  				// UTA_H.EngineTempLiq		              			
	#define REG_SPLIT_UTAH_ENGINEPRESSGAS		    609  				// UTA_H.EnginePressGas		                          
	#define REG_SPLIT_UTAH_ENGINEPRESSLIQ           610  				// UTA_H.EnginePressLiq		              			
	#define REG_SPLIT_UTAH_ENGINEBOXERR1			611  				// UTA_H.EngineBoxErr1			                          
	#define REG_SPLIT_UTAH_ENGINEBOXPERSERR1	    612  				// UTA_H.EngineBoxPersErr1	                          
	#define REG_SPLIT_UTAH_ENGINEBOXSPLITERR1	    613  				// UTA_H.EngineBoxSplitErr1	                          
	#define REG_SPLIT_UTAH_ABSORPTION				614  				// UTA_H.Absorption				                          
	#define REG_SPLIT_UTAH_CUMULATIVEALARM		    615  				// UTA_H.CumulativeAlarm		                          
	#define REG_SPLIT_UTAH_SPLIT_LIQUIDTEMP		    616  				// UTA_H.Split.LiquidTemp		                          
	#define REG_SPLIT_UTAH_SPLIT_GASTEMP			617  				// UTA_H.Split.GasTemp			                          
											                               			                                                                              
	#define REG_SPLIT_UTAH_SPLIT_AIRINTEMP			618  				// UTA_H.Split.AirInTemp			                          
	#define REG_SPLIT_UTAH_SPLIT_AITOUTTEMP		    619  				// UTA_H.Split.AitOutTemp		                          
	#define REG_SPLIT_UTAH_SPLIT_EXP_VALVE          620  				// UTA_H.Split.Exp_Valve		              			
	#define REG_SPLIT_UTAH_SPLIT_HUMI				621  				// UTA_H.Split.Humi				              			
	#define REG_SPLIT_UTAH_SPLIT_REQAIRTEMP		    622  				// UTA_H.Split.ReqAirTemp		                          
	#define REG_SPLIT_UTAH_SPLIT_REQGASPRESS	    623  				// UTA_H.Split.ReqGasPress	                          
	#define REG_SPLIT_UTAH_SPLIT_SUPERHEAT			624  				// UTA_H.Split.Superheat		              			
	#define REG_SPLIT_UTAH_SPLIT_HW_VER			    625  				// UTA_H.Split.HW_Ver			                          
	#define REG_SPLIT_UTAH_SPLIT_FW_VER_HW			626  				// UTA_H.Split.FW_Ver_HW	              			
	#define REG_SPLIT_UTAH_SPLIT_FW_VER_FW          627  				// UTA_H.Split.FW_Ver_FW	              			
	#define REG_SPLIT_UTAH_SPLIT_FW_REV_FW			628  				// UTA_H.Split.FW_Rev_FW	              			
	#define REG_SPLIT_UTAH_HW_VER					629  				// UTA_H.HW_Ver					                          
	#define REG_SPLIT_UTAH_FW_VER_HW				630  				// UTA_H.FW_Ver_HW			              			
	#define REG_SPLIT_UTAH_FW_VER_FW				631  				// UTA_H.FW_Ver_FW			              			
	#define REG_SPLIT_UTAH_FW_REV_FW				632  				// UTA_H.FW_Rev_FW			              			
	#define REG_SPLIT_UTAH_UTASP_ON					633  				// UTA_H.UTASP_On				                          
 	#define REG_SPLIT_UTAH_COMP_STATUS				634  				// UTA_H.StatoCompressore			
	#define REG_SPLIT_UTAH_ENGINE_BOX_ID_MAS_SPLIT	635  				// UTA_H.IdMasterSplit			
	#define REG_SPLIT_UTAH_ENGINE_MODE				636  				// UTA_H.Mode			
   
	#define REG_SPLIT_READY_CMD_ADDRESS				637  				// 
 
	#define REG_DIAG_SPLIT_RD_UTA_MODE				638
	#define REG_DIAG_SPLIT_WR_UTA_MODE				639
	#define REG_SPLIT_SYNC_RD_UTA_MODE				640
	#define REG_SPLIT_SYNC_WR_UTA_MODE				641

    #define REG_SPLIT_FROM_INIT_UTAMODE 			642

	#define REG_DIAG_SPLIT_RD_HEATER_PWR			643
	#define REG_DIAG_SPLIT_WR_HEATER_PWR			644
    #define REG_SPLIT_SYNC_RD_HEATER_PWR			645
	#define REG_SPLIT_SYNC_WR_HEATER_PWR			646

	#define REG_SPLIT_WR_CHANNEL_GAS_REGULATION		647

	#define REG_SPLIT_HEATER_ABIL         			648
	#define REG_DIAG_SPLIT_HEATER_ABIL  			649

	#define REG_SPLIT_SYSTEM_DISABLE				650

	#define REG_SPLIT_RD_SYSTEM_DISABLE				651
	#define REG_SPLIT_WR_SYSTEM_DISABLE				652

	#define REG_SPLIT_WR_SUPERHEAT					653
	#define REG_SPLIT_WR_WORK_PRESS_COOL			654
	#define REG_SPLIT_WR_WORK_PRESS_HEAT			655
	#define REG_SPLIT_WR_PERFORMANCELOSSBATTERY		656

	#define REG_SPLIT_FLAGS_ENABLE_1_16				657
	#define REG_SPLIT_FLAGS_OFFLINE_1_16			658
	#define REG_SPLIT_FLAGS_ONLINE_1_16				659
	#define REG_SPLIT_FLAGS_ALARM_1_16				660
	#define REG_SPLIT_FLAGS_ENABLE_17_32			661
	#define REG_SPLIT_FLAGS_OFFLINE_17_32			662
	#define REG_SPLIT_FLAGS_ONLINE_17_32			663
	#define REG_SPLIT_FLAGS_ALARM_17_32				664

	#define REG_SPLIT_INV_INVOLT_C1                 665
	#define REG_SPLIT_INV_OUTPOWER_C1				666
	#define REG_SPLIT_INV_INVOLT_C2                 667
	#define REG_SPLIT_INV_OUTPOWER_C2				668

    #define REG_SPLIT_WR_MIN_PRESS_COOL             669
    #define REG_SPLIT_WR_MIN_PRESS_HEAT             670
    #define REG_SPLIT_WR_DINAMIC_SUPERHEAT          671

    #define REG_SPLIT_WR_OTHER_CABIN_NUMBER 		672
    #define REG_SPLIT_OTHER_CABIN_ST_TEMPERATURE    673
    #define REG_SPLIT_OTHER_CABIN_ST_ON_OFF         674
    #define REG_SPLIT_OTHER_CABIN_ST_SETP           675
    #define REG_SPLIT_OTHER_CABIN_ST_FAN            676
    #define REG_SPLIT_OTHER_CABIN_CMD_ON_OFF        677
    #define REG_SPLIT_OTHER_CABIN_CMD_SETP          678
    #define REG_SPLIT_OTHER_CABIN_CMD_SETP_F        679
    #define REG_SPLIT_OTHER_CABIN_CMD_FAN           680
    #define REG_SPLIT_OTHER_CABIN_CMD_SEND          681

    #define REG_SPLIT_DOUBLE_PUMP_SWITCH_TIME       682
    #define REG_SPLIT_DOUBLE_PUMP_SELECTION         683

    #define REG_SPLIT_INVERTER1_ERROR				684
    #define REG_SPLIT_INVERTER2_ERROR				685
    #define REG_SPLIT_INVERTER3_ERROR				686
    #define REG_SPLIT_INVERTER4_ERROR			    687

    #define REG_SPLIT_WR_INVERTER_SUPPLY            688
    #define REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING    689

    #define REG_SPLIT_SET_BAUDRATE                  690	// Vaule: 0 = HiSpeed (115000Baud) 1 = LowSpeed (57600Baud)

    #define REG_SPLIT_DIAG_ONLY_FAN                 691
    #define REG_SPLIT_ONLY_FAN                      692
    #define REG_SPLIT_OTHER_CABIN_ONLY_FAN          693

    #define REG_SPLIT_OTHER_CABIN_TRIGGER_INIT		694

    #define REG_SPLIT_SET_CMD_DISABLE               695
    #define REG_SPLIT_DIAG_SET_CMD_DISABLE  		696

    #define REG_SPLIT_VALVELIM_PERCMAXOPENVALVE		697 
    #define REG_SPLIT_VALVELIM_VALVOLAAPERTA		698	
    #define REG_SPLIT_VALVELIM_VALVOLACHIUSA		699
    #define REG_SPLIT_VALVELIM_VALVOLAOFF           700
    #define REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF     701
    #define REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL	702
    #define REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT     703               
    #define REG_SPLIT_VALVELIM_APERTURAMAXCALDO     704
    #define REG_SPLIT_VALVELIM_APERTURAMINCALDO     705
    #define REG_SPLIT_VALVELIM_INITVALVOLAFREDDO	706
    #define REG_SPLIT_VALVELIM_APERTURAMAXFREDDO	707
    #define REG_SPLIT_VALVELIM_APERTURAMINFREDDO	708
    #define REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO 709
    #define REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO 710

    #define REG_SPLIT_WR_DOUBLE_PUMP_ABIL           711
    #define REG_SPLIT_WR_MODE_ABIL                  712
    #define REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON      713
    #define REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF     714

    #define REG_SPLIT_UTA_MODE                      715

    #define REG_SPLIT_ENGBOX_SUPERH_HEATCALC        716
    #define REG_SPLIT_ENGBOX_PRESS_LIQUID_COND      717
    #define REG_SPLIT_ENGBOX_ALL_VALVEOPEN100P      718

	#define REG_SPLIT_MAX_FAN_SPEED                 719

    #define REG_SPLIT_MANUAL_PUMP_ABIL              720
    #define REG_SPLIT_MANUAL_PUMP_FREQ              721

    #define REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT   722

	#define	REG_SPLIT_VALVOLA_CUR_POS_PERC          723		// Registro con la posizione corrente della valvola in percentuale
	#define	REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY     724		// Registro con il numero della priorità della Room
	#define	REG_SPLIT_VALVOLA_CUR_NUMB_PRIORITY     725		// Registro con il numero della valvola in priorità
	#define	REG_SPLIT_VALVOLA_MAX_PERC_PRIORITY     726		// Registro con il valore in percentuale massimo in priorità
	#define	REG_SPLIT_VALVOLA_PID_COOL_PERC_POS     727		// Registro con il valore in percentuale richiesto dalle Room
    #define	REG_SPLIT_VALVOLA_SUM_TOT_PERCENT       728      // Registro con il valore in percentuale reale delle Room totali
	#define	REG_SPLIT_VALVOLA_SUM_REQ_PERCENT       729      // Registro con il valore in percentuale previsto delle Room totali
    #define	REG_SPLIT_VALVOLA_ROOM_NUMB_PRIORITY    730
    #define	REG_SPLIT_VALVOLA_ONDEMAND_LIM_MAX      731

    #define REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING    732

    #define REG_SPLIT_RD_ABIL_VALVE_ONDEMAND            733
    #define REG_SPLIT_RD_ABIL_SCORE_ONDEMAND            734
    #define REG_SPLIT_WR_ON_PRIORITY_MODE               735
    #define REG_SPLIT_WR_VALVE_ONDEMAND_MIN_PERC_VAL    736
    #define REG_SPLIT_WR_MAX_LIM_ONDEMAND_TOTAL_SCORE   737
    #define REG_SPLIT_WR_TIME_ONDEMAND_VALVE_SWITCH     738
    
    #define REG_SPLIT_RD_VALVE_ONDEMAND_MIN_PERC_VAL    739

    #define REG_SPLIT_SUPERHEAT_COOLINGCALC             740               // EngineBox[0].SuperHeat.CoolingCalc
    #define REG_SPLIT_K_SETPOINT_COOLING_SUPERHEAT      741               // K_SetPoint_Cooling_SuperHeat
    #define REG_SPLIT_SUPERHEAT_CORRECT_COOLING         742               // EngineBox[0].SuperHeat.Correct_Cooling

    #define REG_SPLIT_WR_NETB_LCD_INTENSITY             743 //733

    #define REG_SPLIT_WR_CONDENSER_PRESS_ABIL           744

    #define REG_SPLIT_TOTAL_FAN_ABSORPTION              745 
    #define REG_SPLIT_COMPRESSOR1_ABSORPTION            746 
    #define REG_SPLIT_COMPRESSOR2_ABSORPTION            747
    #define REG_SPLIT_SEA_WATER_PUMP_ABSORPTION         748 

    #define REG_DIAG_SPLIT_FAN_POWER                    749
    #define REG_DIAG_SPLIT_FAN_CURRENT                  750
    #define REG_DIAG_SPLIT_SUPPLY_VOLTAGE               751

    #define REG_SPLIT_FAN_POWER                         752
    #define REG_SPLIT_FAN_CURRENT                       753
    #define REG_SPLIT_SUPPLY_VOLTAGE                    754

    #define REG_SUB_SPLIT_FAN_POWER                     755
    #define REG_SUB_SPLIT_FAN_CURRENT                   756
    #define REG_SUB_SPLIT_SUPPLY_VOLTAGE                757

    #define REG_SPLIT_AND_SUB_FAN_POWER                 758

	#define REG_SPLIT_MODDEV_FLAGS_ENABLE_1_16			759
	#define REG_SPLIT_MODDEV_FLAGS_OFFLINE_1_16			760
	#define REG_SPLIT_MODDEV_FLAGS_ONLINE_1_16			761
	#define REG_SPLIT_MODDEV_FLAGS_ALARM_1_16			762
	#define REG_SPLIT_MODDEV_FLAGS_ENABLE_17_32			763
	#define REG_SPLIT_MODDEV_FLAGS_OFFLINE_17_32		764
	#define REG_SPLIT_MODDEV_FLAGS_ONLINE_17_32			765
	#define REG_SPLIT_MODDEV_FLAGS_ALARM_17_32			766

    #define REG_SPLIT_MODDEV_TYPE                       767
	#define REG_SPLIT_MODDEV_FLAGS_STATUS               768
    #define REG_SPLIT_MODDEV_AND_ADDR_TYPE              769

    #define REG_SPLIT_PRESSURE_LO_LIQ                   770
    #define REG_SPLIT_PRESSURE_LO_GAS                   771
    #define REG_SPLIT_PRESSURE_HI_LIQ                   772
    #define REG_SPLIT_PRESSURE_HI_GAS                   773

    #define REG_SPLIT_WR_FAN_CURRENT_LIMIT              774

    //Argo switch
    #define REG_SPLIT_RD_TESTALL_ABIL                   775
    #define REG_SPLIT_RD_TESTALL_SUPERHEAT_SP_COOL      776
    #define REG_SPLIT_RD_TESTALL_SUPERHEAT_SP_HEAT      777
    #define REG_SPLIT_RD_TESTALL_MAX_OPEN_VALVE_COOL    778
    #define REG_SPLIT_RD_TESTALL_MAX_OPEN_VALVE_HEAT    779
    #define REG_SPLIT_RD_TESTALL_WORK_TEMP_COOL         780
    #define REG_SPLIT_RD_TESTALL_WORK_TEMP_HEAT         781

    #define REG_SPLIT_WR_TESTALL_ABIL                   782
    #define REG_SPLIT_WR_TESTALL_WORK_PRESS_HEAT        783
    #define REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL         784
    #define REG_SPLIT_WR_TESTALL_WORK_PRESS_COOL        785
    #define REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMAXFREDDO 786
    //Argo switch

    #define REG_SPLIT_RD_FROZEN_MODE                    787
    #define REG_DIAG_SPLIT_FROZEN_MODE                  788

    #define REG_SPLIT_WR_FROZEN_ON                      789
    #define REG_SPLIT_WR_FROZEN_ABIL                    790


    //Argo switch FROZEN
    #define REG_SPLIT_RD_TESTALL_FROZEN_ABIL            791
    #define REG_SPLIT_RD_TESTALL_SUPERHEAT_SP_FRIGO     792
    #define REG_SPLIT_RD_TESTALL_SUPERHEAT_SP_FREEZER   793
    #define REG_SPLIT_RD_TESTALL_MAX_OPEN_VALVE_FRIGO   794
    #define REG_SPLIT_RD_TESTALL_MAX_OPEN_VALVE_FREEZER 795
    #define REG_SPLIT_RD_TESTALL_WORK_TEMP_FRIGO        796
    #define REG_SPLIT_RD_TESTALL_WORK_TEMP_FREEZER      797

    #define REG_SPLIT_WR_TESTALL_FROZEN_ABIL                        798
    #define REG_SPLIT_WR_TESTALL_SUPERHEAT_SP_FRIGO                 799
    #define REG_SPLIT_WR_TESTALL_SUPERHEAT_SP_FREEZER               800
    #define REG_SPLIT_WR_TESTALL_WORK_PRESS_FRIGO                   801
    #define REG_SPLIT_WR_TESTALL_WORK_PRESS_FREEZER                 802
    #define REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMAXFREDDO_FRIGO   803
    #define REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMAXFREDDO_FREEZER 804
    //Argo switch FROZEN

    #define REG_SPLIT_RD_ABIL_DEFROSTING                805
    #define REG_SPLIT_RD_DEFROSTING_STEP_TIME           806
    #define REG_SPLIT_RD_DEFROSTING_TIME                807
    #define REG_SPLIT_RD_DEFROSTING_TEMP_SET            808
    #define REG_SPLIT_RD_DRIPPING_TIME                  809

    #define REG_SPLIT_WR_ABIL_DEFROSTING                810
    #define REG_SPLIT_WR_DEFROSTING_STEP_TIME           811
    #define REG_SPLIT_WR_DEFROSTING_TIME                812
    #define REG_SPLIT_WR_DEFROSTING_TEMP_SET            813
    #define REG_SPLIT_WR_DRIPPING_TIME                  814
    
    #define REG_SPLIT_VALVELIM_VALVOLAFROZEN            815
    #define REG_SPLIT_RD_VALVOLAFROZEN                  816

    #define REG_SPLIT_RD_TESTALL_MIN_OPEN_VALVE_COOL        817
    #define REG_SPLIT_RD_TESTALL_MIN_OPEN_VALVE_HEAT        818
    #define REG_SPLIT_RD_TESTALL_MIN_OPEN_VALVE_FRIGO       819
    #define REG_SPLIT_RD_TESTALL_MIN_OPEN_VALVE_FREEZER     820
    #define REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMINFREDDO 821      
    #define REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMINCALDO  822
    #define REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMINFREDDO_FRIGO   823
    #define REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMINFREDDO_FREEZER 824

    #define REG_SPLIT_RD_MAX_VENTIL_MODE                825
    #define REG_DIAG_SPLIT_WR_MAX_VENTIL_MODE           826

    #define REG_SPLIT_READY_POWER                           827     
    #define REG_SPLIT_READY_CURRENT_SP                      828
    #define REG_SPLIT_READY_CURRENT_SP_F                    829     
    #define REG_SPLIT_READY_CURRENT_FANMODE                 830
    #define REG_SPLIT_READY_CURRENT_UTAMODE                 831

    #define REG_SPLIT_FAN_MODE                              832
    #define REG_SPLIT_READY_OFFLINE                         833

    #define REG_DIAG_SPLIT_MODDEV_TYPE                      834

    #define REG_SPLIT_RD_TESTALL_SUPERHEAT_HEAT_PGAIN            835
    #define REG_SPLIT_RD_TESTALL_SUPERHEAT_HEAT_IGAIN            836
    #define REG_SPLIT_RD_TESTALL_SUPERHEAT_HEAT_DGAIN            837
    #define REG_SPLIT_RD_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID    838
 
    #define REG_SPLIT_RD_TESTALL_SUPERHEAT_COOL_PGAIN            839
    #define REG_SPLIT_RD_TESTALL_SUPERHEAT_COOL_IGAIN            840
    #define REG_SPLIT_RD_TESTALL_SUPERHEAT_COOL_DGAIN            841
    #define REG_SPLIT_RD_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID    842

    #define REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL_PGAIN            843
    #define REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL_IGAIN            844
    #define REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL_DGAIN            845
    #define REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID    846

    #define REG_SPLIT_RD_MAINTENANCE_FLAG                        847
    #define REG_SPLIT_RD_TOUCH_MAINTENANCE_SET_TIME              848
    #define REG_SPLIT_WR_MAINTENANCE_REMANING_TIME               849

    #define REG_SPLIT_ENGBOX_SUPERH_PID_ERROR                    850

    #define REG_SPLIT_SUPERH_COOL_PID_ERROR                      851
    #define REG_SUB_SPLIT_SUPERH_COOL_PID_ERROR                  852
    #define REG_DIAG_SPLIT_SUPERH_COOL_PID_ERROR                 853

    #define REG_SPLIT_RD_SUPERHEAT_COOL_MIN_SETPOINT             854
    #define REG_SPLIT_RD_SUPERHEAT_COOL_MAX_VALUE_CORRECT        855
    #define REG_SPLIT_RD_SUPERHEAT_HEAT_MIN_SETPOINT             856
    #define REG_SPLIT_RD_SUPERHEAT_HEAT_MAX_SETPOINT             857

    #define REG_SPLIT_WR_SUPERHEAT_HEAT_MIN_SETPOINT             858
    #define REG_SPLIT_WR_SUPERHEAT_HEAT_MAX_SETPOINT             859

    #define REG_SPLIT_RD_TEMPERATURE_PROBE_ERROR                 860
    #define REG_SUB_SPLIT_RD_TEMPERATURE_PROBE_ERROR             861
    #define REG_DIAG_SPLIT_TEMPERATURE_PROBE_ERROR               862
    #define REG_SPLIT_WR_BOX_PROBE_ERROR                         863

	#define REG_MASTER_COMERROR_CNTCOMERR                        864
    #define REG_MASTER_COMERROR_TIMEOUTCOMERR                    865
    #define REG_MASTER_COMERROR_CRC_COMERR                       866
    #define REG_MASTER_COMERROR_MODCNTCOMERR                     867
    #define REG_MASTER_COMERROR_MODTIMEOUTCOMERR                 868
    #define REG_MASTER_COMERROR_MODCRC_COMERR                    869
    #define REG_MASTER_COMERROR_MODCNTCOMERR_U2                  870
    #define REG_MASTER_COMERROR_MODTIMEOUTCOMERR_U2              871
    #define REG_MASTER_COMERROR_MODCRC_COMERR_U2                 872
    #define REG_MASTER_COMERROR_CNTCOMERR_U3                     873
    #define REG_MASTER_COMERROR_TIMEOUTCOMERR_U3                 874
    #define REG_MASTER_COMERROR_CRC_COMERR_U3                    875

    #define REG_SPLIT_WR_EXTRACTOR_NUMBER                        876
    #define REG_SPLIT_WR_DELTAVEL_EXTRACTOR                      877
    #define REG_SPLIT_RD_EXTRACTOR_NUMBER                        878
    #define REG_SPLIT_RD_DELTAVEL_EXTRACTOR                      879
    #define REG_SPLIT_WR_EXTRACTOR_AUTOVEL                       880
    #define REG_SPLIT_WR_EXTRACTOR_ABIL                          881

    #define REG_DIAG_SPLIT_MODEL_VERSION                         882

    #define REG_SPLIT_FAN_M3H                                    883
    #define REG_SUB_SPLIT_FAN_M3H                                884
    #define REG_DIAG_SPLIT_FAN_M3H                               885


    #define REG_SPLIT_VENTIL_SELECTION                           886
    #define REG_SPLIT_M3H_MIN                                    887
    #define REG_SPLIT_M3H_MAX                                    888

    #define REG_SPLIT_AIR_POWER_OUTPUT                           889
    #define REG_SPLIT_AIR_POWER_BTU                              890
    #define REG_SUB_SPLIT_AIR_POWER_OUTPUT                       891
    #define REG_SUB_SPLIT_AIR_POWER_BTU                          892

    #define REG_DIAG_SPLIT_AIR_POWER_OUTPUT                      893
    #define REG_DIAG_SPLIT_AIR_POWER_BTU                         894

    #define REG_SPLIT_AND_SUB_AIR_POWER_OUTPUT                   895
    #define REG_SPLIT_AND_SUB_AIR_POWER_BTU                      896

    #define REG_SPLIT_TOTAL_AIR_POWER_OUTPUT                    897 
    #define REG_SPLIT_TOTAL_AIR_POWER_BTU                       898 

    #define REG_SYSTEM_EFFICENCY_EER                            899

    #define REG_SPLIT_OTHER_CABIN_ST_UTA_MODE                   900
    #define REG_SPLIT_OTHER_CABIN_CMD_UTA_MODE                  901

    #define REG_SPLIT_MIN_CURRENT_FAN                           902
    #define REG_SPLIT_MAX_CURRENT_FAN                           903
    #define REG_SPLIT_EFFICIENCY_FAN                            904
    #define REG_SPLIT_SUPPLY_FAN                                905

    #define REG_INVERTER_UPPER_OUT_FREQ                         906

    #define REG_SPLIT_RD_THRESHOLD_COMPRESSOR_HI                907
    #define REG_SPLIT_WR_THRESHOLD_COMPRESSOR_HI                908

    #define REG_SPLIT_TOUCH_PAGE                                909
    #define REG_SPLIT_CNTCOMERRSEC                              910
    #define REG_SPLIT_TIMEOUTCOMERRSEC                          911		
    #define REG_SPLIT_TIMEOUTINTERCHARCOMERRSEC                 912
    #define REG_SPLIT_CRC_COMERRSEC                             913
    #define REG_SPLIT_MODCNTCOMERRSEC                           914
    #define REG_SPLIT_MODTIMEOUTCOMERRSEC                       915
    #define REG_SPLIT_MODTIMEOUTINTERCHARCOMERRSEC              916
    #define REG_SPLIT_MODCRC_COMERRSEC                          917

    #define REG_DIAG_SPLIT_TOUCH_PAGE                           918
    #define REG_DIAG_SPLIT_CNTCOMERRSEC                         919
    #define REG_DIAG_SPLIT_TIMEOUTCOMERRSEC                     920		
    #define REG_DIAG_SPLIT_TIMEOUTINTERCHARCOMERRSEC            921
    #define REG_DIAG_SPLIT_CRC_COMERRSEC                        922
    #define REG_DIAG_SPLIT_MODCNTCOMERRSEC                      923
    #define REG_DIAG_SPLIT_MODTIMEOUTCOMERRSEC                  924
    #define REG_DIAG_SPLIT_MODTIMEOUTINTERCHARCOMERRSEC         925
    #define REG_DIAG_SPLIT_MODCRC_COMERRSEC                     926

    #define REG_SPLIT_READ_HUMIDITY                             927
    #define REG_DIAG_SPLIT_WR_HUMIDITY                          928

	#define REG_SPLIT_FROM_INIT_HEATER_PWR                      932
	#define REG_SPLIT_HEATER_PWR                                933

	#define REG_SUB_SPLIT_READ_HUMIDITY                         934
	#define REG_SPLIT_READ_HUMIDITY_MIDDLE                      935

	#define REG_SPLIT_RD_DEHUMI_ABIL                            936
	#define REG_SPLIT_RD_SET_TEMP_AIR_OUT                       937
	#define REG_SPLIT_RD_SET_HUMI                               938
	#define REG_SPLIT_RD_SET_DELTA_TEMP_MIN_DEHUMI              939
	#define REG_SPLIT_RD_SET_FAN_SPEED_DEHUMI                   940
	#define REG_SPLIT_RD_SET_PRESS_DEHUMI                       941
	#define REG_SPLIT_RD_SET_TEMP_AIR_OUT_DEHUMI                942

	#define REG_SPLIT_WR_DEHUMI_ABIL                            943
	#define REG_SPLIT_WR_SET_TEMP_AIR_OUT                       944
	#define REG_SPLIT_WR_SET_HUMI                               945
	#define REG_SPLIT_WR_SET_DELTA_TEMP_MIN_DEHUMI              946
	#define REG_SPLIT_WR_SET_FAN_SPEED_DEHUMI                   947
	#define REG_SPLIT_WR_SET_PRESS_DEHUMI                       948
	#define REG_SPLIT_WR_SET_TEMP_AIR_OUT_DEHUMI                949

    #define REG_DIAG_SPLIT_WR_MIN_SET_POINT                     950
    #define REG_DIAG_SPLIT_WR_MAX_SET_POINT                     951
    #define REG_DIAG_SPLIT_RD_MIN_SET_POINT                     952
    #define REG_DIAG_SPLIT_RD_MAX_SET_POINT                     953

    #define REG_DIAG_SPLIT_WR_GAS_LEAKS                         954

// Registri d'informazione

    #define	REG_SUB_SPLIT_MODEL_VERSION             65439   //0xFFA0		// Identificativo del modulo (tipo di modulo) del subslave
    #define	REG_SPLIT_FW_VERSION                    65440   //0xFFA0		// Versione del Fw, coincide con l'hardware
	#define	REG_SPLIT_FW_REVISION                   65441   //0xFFA1		// Revisione del Fw
	#define	REG_SPLIT_MODEL_VERSION                 65442   //0xFFA2		// Identificativo del modulo (tipo di modulo)
	#define	REG_SPLIT_ADD_MODULO                    65443   //0xFFA3		// Indirizzo del modulo
	#define	REG_SPLIT_SYSTEM_NUMBER_LO              65444   //0xFFA4		// System number (Low word)
	#define	REG_SPLIT_SYSTEM_NUMBER_HI              65445   //0xFFA5		// System number (high word)
	#define	REG_SPLIT_FABRICATION_DAY               65446   //0xFFA6		// Giorno della data di fabbricazione
	#define	REG_SPLIT_FABRICATION_MONTH             65447   //0xFFA7		// Mese della data di fabbricazione
	#define	REG_SPLIT_FABRICATION_YEAR              65448   //0xFFA8		// Anno della data di fabbricazione
	#define	REG_SPLIT_CRC_16                        65449   //0xFFA9		// Valore del CRC della flash del micro
	#define	REG_SPLIT_SAVE_2_E2                     65450   //0xFFAA		// Salva i dati in E2

// Registri per funzioni speciali
	#define	REG_BOARD_AUTORESET                     65520   //0xFFF0		// 65520d: Autoreset scheda (vedi Bootloader)
                                                            // per poter eseguire l'autoreset inviare in sequenza i seguenti valori: 0x00, 0x55 (85d), 0xAA (170), 0x55AA (21930d)
	#define	REG_BOARD_SUBSLAVE_AUTORESET_ADDR       65521   //0xFFF1		// 65521d: Reset scheda sottoslave (vedi Bootloader)
                                                            // per poter inviare il comando di reset al sottoslave inviare in sequenza i seguenti valori: 0x00, 0x55 (85d), 0xAA (170), 0x55AA (21930d)
	#define	REG_BOARD_SPECIALFUNC2                  65522   //0xFFF2		// 65522d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC3                  65523   //0xFFF3		// 65523d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC4                  65524   //0xFFF4		// 65524d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC5                  65525   //0xFFF5		// 65525d: Libero per funzioni speciali
//.....
	#define	REG_BOARD_SPECIALFUNC15                 65535   //0xFFFF		// 65535d: Libero per funzioni speciali

	#define VariabiliGlobaliProtocolloComunicazione()	\
		volatile unsigned char	 My_Address = 0

	#define IncludeVariabiliGlobaliProtocolloComunicazione()\
		extern volatile unsigned char	 My_Address

	//---------------------------------------
	// Variabili 
	//---------------------------------------
	IncludeVariabiliGlobaliProtocolloComunicazione();
	//IncludeVariabiliGlobaliADC();
	//IncludeVariabiliGlobaliEEPROM();
	//IncludeVariabiliGlobaliPWM();
	//IncludeVariabiliGlobaliValvolaExp();
	//IncludeVariabiliGlobaliCore();

	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	void	SetMyAddress(char My_Add);
	char	GetMyAddress(void);
	void	Process_Cmd(volatile TypBuffer * LastCmd);
	int		GetData (volatile int Reg);
	char	PutData (volatile int Reg, volatile int Value);
	void	RefreshDiagnosticSplit(void);			// #v16
    void    Change_ProtocolBusSlave(int ProtocolIndex, unsigned int N_Uart);
    void    ResetComFlags(void);
	
#endif
