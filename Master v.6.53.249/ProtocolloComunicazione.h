//----------------------------------------------------------------------------------
//	Progect name:	ProtocolloComunicazione.h
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			Emanuele
//	Date:			04/01/2014
//	Description:	Header delle funzioni che gestiscono la comunicazione con il sistema
//----------------------------------------------------------------------------------

#ifndef _PROTOCOLLO_COMUNICAZIONE_H_
	#define _PROTOCOLLO_COMUNICAZIONE_H_

#include "Timer.h"

	//----------------------------------------------------------------------------------
	//	Define
	//----------------------------------------------------------------------------------
	#define	Master_Default_Address			254//0xFE
    #define k_Broadcast_Addr    0

	#define Versione_Hw			6			// Compatibilita con HW (Master v4.3 o sup.)
	#define Versione_Fw			53			// Compatibilita con Protocolli BUS/ModBUS
	#define Revisione_Fw		249 		// Revisione FW (modifiche minori o bachi)  
	#define Id_Modulo			0x06		// Master Inverter

	#define Indirizzo_Modulo	Master_Default_Address
/*
    #define USA                 1           // bit.0 (gestito a bit)
    #define EUR                 2           // bit.1 (gestito a bit)
    #define JAP                 4           // bit.2 (gestito a bit)
    #define K_Touch_Nation      EUR
*/
	#define	Real_Max_Room		30+1//35+1//30+1		// Room meno indirizzi riservati a Bridge o schede speciali
	#define	Max_Room			30+1//35+1		// MAX + 1 perchè lo zero non è utilizzato (-> Room[0].)
											// da 1..30 -> unità Split
											// da 31..35 riservate per gestioni speciali 
    #define k_Bridge1_Addr		31			// Indirizzo scheda Bridge n.1 (BUS UTA-H)
    #define k_Bridge2_Addr		32			// Indirizzo scheda Bridge n.2 (BUS Bridge)
    #define k_Bridge3_Addr		33			// Indirizzo scheda Bridge n.3 (eventuale)
    #define k_Bridge4_Addr		34			// Indirizzo scheda Bridge n.4 (eventuale)

#if (K_AbilMODBCAN==1)
    #define k_MODBCAN_Addr      35			// Indirizzo modulo MODBUS-CAN converter HD67412
#else
    #define k_Spare1_Addr		35			// Indirizzo scheda speciale !?  
#endif //#if (K_AbilMODBCAN==1) 
 /*   #define k_Spare2_Addr		36			// Indirizzo scheda speciale !?
    #define k_Spare3_Addr		37			// Indirizzo scheda speciale !?
    #define k_Spare4_Addr		38			// Indirizzo scheda speciale !?
    #define k_Spare5_Addr		39			// Indirizzo scheda speciale !?
    #define k_Spare6_Addr		40			// Indirizzo scheda speciale !?
*/

	#define Max_Engine_Box		2
	#define Max_Dimmer			2
	#define MaxChkRetries		5		// check
	#define MaxChkWait			5//25		// check
	#define MaxReadRetries		3		// Read
	#define	MaxReadWait			50//200//25//75		// Read
	#define MaxWriteRetries		3		// Write
	#define	MaxWriteWait		50//200//25//200		// Write
	#define	WriteBrcWait		150// microsecondi in Broadcast
	#define	WriteBrcRetries		1		// Broadcast

#if (K_EnableHiSpeedMyBus==1)
	#define	k_WaitSendMessage   500     // Pausa in uS per garantire una minima distanza tra un Tx e il successivo 		                        
#else
	#define	k_WaitSendMessage   1000    // Pausa in uS per garantire una minima distanza tra un Tx e il successivo 		                        
#endif	

	#define	k_Split_Master_Add	1		// Indirizzo Slave master (con touch x diagnostica) #v16



	// header
	#define	StartPack	0x55	// per l'auto BaudRate
	#define	Ack			0x8000	// dato di ack
	#define	N_Ack		0xFFFF	// dato di non ack
	
	// comandi
	#define	Prg					0x01
	#define	ReadReg				0x02	
	#define	WriteRegBrdCst		0x03
	#define	WriteReg			0x04
	#define	WriteSplitRegBrdCst	0x05
	#define	WriteDimmRegBrdCst	0x06
	#define	WriteEngRegBrdCst	0x07
	#define	Check				0x08


// Define per Change_Protocol
	#define	K_MyBus_Baud115200     0   // MyBUS 115200
	#define	K_MyBus_Baud57600      1   // MyBUS 57600
	#define	K_ModBus_Baud38400     2   // ModBUS 38400
	#define	K_ModBus_Baud57600     3   // ModBUS 57600
	#define	K_ModBus_Baud115200    4   // ModBUS 115200
	#define	K_MyBus_Baud250000     5   // MyBUS 250000



	// registri modulo split
	// registri locali
	#define	REG_SPLIT_TEMP_BULBO_GRANDE			0x0001		// Temperatura bulbo grande
	#define	REG_SPLIT_TEMP_BULBO_PICCOLO		0x0002		// Temperatura bulbo piccolo
	#define	REG_SPLIT_TEMP_ARIA_USCITA			0x0003		// Temperatura arian in uscita
	#define	REG_SPLIT_TEMP_AMBIENTE				0x0004		// Temperatura aria ambiente
	#define REG_SPLIT_PRES_BULBO_GRANDE			0x0005		// Pressione bulbo grande
	#define REG_SPLIT_PRES_BULBO_PICCOLO		0x0006		// Pressione bulbo piccolo
	#define REG_SPLIT_I_FAN						0x0007		// Corrente ventilatore

    #define REG_SPLIT_TEMP_AMB_MIDDLE		    0x000A      // Indirizzo della temperatura ambiente mediata con sottoslave
    #define REG_SPLIT_TEMP_AMB_REAL 		    0x000B      // Indirizzo della temperatura ambiente reale utilizzata (mediata con sottoslave o no in mbas ea FW Selection)

	#define	REG_SPLIT_TEMP_BULBO_GRANDE_ADC		0x0011		// Valore ADC Temperatura bulbo grande
	#define	REG_SPLIT_TEMP_BULBO_PICCOLO_ADC	0x0012		// Valore ADC Temperatura bulbo piccolo
	#define	REG_SPLIT_TEMP_ARIA_USCITA_ADC		0x0013		// Valore ADC Temperatura arian in uscita
	#define	REG_SPLIT_TEMP_AMBIENTE_ADC			0x0014		// Valore ADC Temperatura aria ambiente
	#define REG_SPLIT_PRES_BULBO_GRANDE_ADC		0x0015		// Valore ADC Pressione bulbo grande
	#define REG_SPLIT_PRES_BULBO_PICCOLO_ADC	0x0016		// Valore ADC Pressione bulbo piccolo
	#define REG_SPLIT_I_FAN_ADC					0x0017		// Valore ADC Corrente ventilatore

	#define	REG_SPLIT_DIGIT_INPUT				0x0021		// Stato degli ingressi digitali
	#define	REG_SPLIT_DIGIT_OUTPUT				0x0022		// Stato delle uscite digitali
	#define	REG_SPLIT_PWM_REQ_FAN_VALUE			0x0023		// Valore richiesto pwm ventil
	#define	REG_SPLIT_PWM_ACT_FAN_VALUE			0x0024		// Valore istantaneo pwm ventil
	#define	REG_SPLIT_FAN_TACHO					0x0025		// Valore in rivoluzioni per minuto del ventilatore
	#define	REG_SPLIT_VENTIL_MODE				0x0026		// Velocità di ventilazione 0->Spento; 1->Night; 2->Speed1; ...; 8->Speed7; 9->Auto;
	#define	REG_SPLIT_FUNCTION_MODE				0x0027		// Modalità corrente di lavoro per modalità auto (Off, ventil, raffrescamento, riscaldamento, Master-Auto)
	#define	REG_SPLIT_CURRENT_SP				0x0028		// SP corrente per il calcolo della velocità in modalità auto
	#define REG_SPLIT_CURRENT_EVAP_TEMP			0x0029		// Temperatura corrente di evaporazione
	#define REG_SPLIT_CURRENT_EVAP_PRESURE		0x002A		// Pressione corrente di evaporazione

	#define	REG_SPLIT_VALVOLA_CUR_POS			0x002B		// Registro con la posizione corrente della valvola
	#define	REG_SPLIT_VALVOLA_SET_POS			0x002C		// Registro con la posizione desiderata della valvola
	#define	REG_SPLIT_VALV_STP_REF_TIME			0x002D		// Registro con il tempo di aggiornamento tra un passo e l'altro
	#define	REG_SPLIT_VALV_MAX_STEP				0x002E		// Registro con il massimo numero di passi
	#define	REG_SPLIT_CORRENTE_MAX_VALV			0x002F		// Registro con il valore di corrente della valvola ADC
	#define	REG_SPLIT_CORRENTE_MIN_VALV			0x0030		// Registro con il valore di corrente della valvola ADC
	#define	REG_SPLIT_THR_MIN_AMP_VALV			0x0031		// Registro con il valore minimo di corrente della valvola
	#define	REG_SPLIT_THR_MAX_AMP_VALV			0x0032		// Registro con il valore massimo di corrente della valvola

	#define	REG_SPLIT_DEFINE_MODE				0x0033		// Modalità di lavoro selezionata sul touch anche se non attivo il flag di accensione
	#define	REG_SPLIT_MASTER_STATUS             0x0034		// Flags di stato dello Split Master
    #define REG_SPLIT_POWER                     0x0035		// Flags di stato On/Off dello split (Touch[0].Script_Split_Pwr)
    #define REG_SPLIT_CUMULATIVE_ALARM          0x0036		// Flags di stato Allarmi (Almeno un allarme presente sullo split)
	#define	REG_SPLIT_VALVOLA_ERR_POS			0x0037		// Registro con la posizione in errore richiesta alla valvola
	#define	REG_SPLIT_VALVOLA_ERR_COD			0x0038		// Registro con il codice di errore che richiede la posizione della valvola

	#define REG_SPLIT_ENGINE_BOX_WATER_PUMP_SPEED_PERC	0x0040		// Velocità della pompa dell'acqua in %

	#define REG_SPLIT_ENGINE_BOX_COMPRESSOR_SPEED	0x0041		// Velocità del compressore
	#define REG_SPLIT_ENGINE_BOX_WATER_PUMP_SPEED	0X0042		// Velocità della pompa dell'acqua
	#define	REG_SPLIT_ENGINE_BOX_FLUX				0x0043		// Lettura Flussimetro circuito acqua mare
	#define REG_SPLIT_CURRENT_MODE_ENGINE_BOX		0x0044		// Modalità corrente del box motore (fermo, clima, riscaldamento, avvio clima, avvio riscladamento)
	#define	REG_SPLIT_CURRENT_REQ_PRESSURE			0x0045		// Pressione richiesta al box motore
	#define	REG_SPLIT_ENGINE_BOX_T_COMP_OUT			0x0046		// Temperatura mandata compressore
	#define	REG_SPLIT_ENGINE_BOX_T_COMP_SUCTION			0x0047		// Temperatura ritorno compressore
	#define	REG_SPLIT_ENGINE_BOX_T_SEA_WATER		0x0048		// Temperatura acqua mare in ingresso alla pompa
	#define	REG_SPLIT_ENGINE_BOX_T_CONDENSER		0x0049		// Temperatura condensatore
	#define	REG_SPLIT_ENGINE_BOX_T_GAS				0x004A		// Temperatura tubo grande
	#define	REG_SPLIT_ENGINE_BOX_T_LIQUID			0x004B		// Temperatura tubo Piccolo
	#define	REG_SPLIT_ENGINE_BOX_P_GAS				0x004C		// Pressione tubo grande
	#define	REG_SPLIT_ENGINE_BOX_P_LIQUID			0x004D		// Pressione tubo piccolo
	#define	REG_SPLIT_ENGINE_BOX_P_VRV_BOX			0x004E		// Pressione serbatoio VRV
	#define	REG_SPLIT_ENGINE_BOX_P_AUX				0x004F		// Pressione aux
	#define REG_SPLIT_ENGINE_BOX_RELE_SATUS			0x0050		// Stato dei Rele
	#define REG_SPLIT_ENGINE_BOX_INPUT_STATUS		0x0051		// Stato degli input
	#define REG_SPLIT_ENGINE_BOX_SYS_ID_LO			0x0052		// System ID lower
	#define REG_SPLIT_ENGINE_BOX_SYS_ID_HI			0x0053		// System ID higher
	//#define REG_SPLIT_ERRORE_1_BOX_MOTORE			0x0054		// registro con gli errori del box motore
	//#define REG_SPLIT_ERRORE_2_BOX_MOTORE			0x0055		// registro con gli errori del box motore
	#define REG_SPLIT_STATUS_BOX_MOTORE				0x0056		// stato della macchina a stati del box motore
	#define REG_SPLIT_ID_SPLIT_MASTER				0x0057		// ID dello split master
	#define REG_SPLIT_POWER_LIMIT					0x0058		// Limite di potenza impostata sul master
	#define REG_SPLIT_POWER_LIMIT_REQ				0x0059		// Limite di potenza richiesta dallo split
	#define REG_SPLIT_STATUS						0x005A		// Stato dello split, che si legge il box motore
	#define REG_SPLIT_ENGINE_BOX_CYCLE_TIME			0x005B		// Tempo di esecuzione ciclo dell' unita Master (invio su Split per diagnostica)
	#define REG_SPLIT_ENGINE_BOX_HOURS_OF_LIFE		0x005C		// Ore di vita dell' unita Master (invio su Split per diagnostica)
    #define REG_SPLIT_ENGINE_BOX_TOUCH_NATION       0x005D      // Identificazione Nazione da inviare sul touch per modificare dinamicamente gli oggetti e unità di misura sul touch a seconda della Nazione (USA/EUR)	

    #define REG_DIAG_SUB_SPLIT_QUANTITY             0x005E  // Dec. 094		r 
    #define REG_DIAG_SPLIT_RES_REQ_ADDRESS          0x005F  // Dec. 095		r   DiagnosticSplit.Reset_Req_Address -> indirizzo dello slave su cui effettuare il reset allarmi proveniente dallo slave master
	#define	REG_DIAG_SPLIT_TEST_ADDRESS				0x0060	// Dec. 096		r	#v16 // indirizzo dello split Master (dotato di touch x diagnostica) #v16
	#define	REG_DIAG_SPLIT_STATUS					0x0061	// Dec. 097		w	
	#define	REG_DIAG_SPLIT_LIQ_TEMP					0x0062	// Dec. 098		w
	#define	REG_DIAG_SPLIT_GAS_TEMP					0x0063	// Dec. 099		w
	#define	REG_DIAG_SPLIT_AIR_IN					0x0064	// Dec. 100		w
	#define	REG_DIAG_SPLIT_AIR_OUT					0x0065	// Dec. 101		w
	#define	REG_DIAG_SPLIT_FAN_SPEED				0x0066	// Dec. 102		w
	#define	REG_DIAG_SPLIT_EXP_VALVE				0x0067	// Dec. 103		w
	#define	REG_DIAG_SPLIT_GAS_PRESSURE				0x0068	// Dec. 104		w
	#define	REG_DIAG_SPLIT_LIQ_PRESSURE				0x0069	// Dec. 105		w
	#define	REG_DIAG_SPLIT_REQUEST_AIR_TEMP			0x006A	// Dec. 106		w
	#define	REG_DIAG_SPLIT_REQUEST_AIR_PRES			0x006B	// Dec. 107		w
	#define	REG_DIAG_SPLIT_ENG_MODE					0x006C	// Dec. 108		w	
	#define	REG_DIAG_SPLIT_CYCLE_TIME				0x006D	// Dec. 109		w

	#define	REG_DIAG_INVERTER_ALARM					0x006E	// Dec. 110		w	 
	#define	REG_DIAG_INVERTER_OUT_FREQ				0x006F	// Dec. 111		w	 
	#define	REG_DIAG_INVERTER_STATUS				0x0070	// Dec. 112		w	 
	#define	REG_DIAG_INVERTER_OUT_CURRENT			0x0071	// Dec. 113		w 
	#define	REG_DIAG_INVERTER_IN_VOLTAGE			0x0072	// Dec. 114		w	 
	#define	REG_DIAG_INVERTER_OUT_VOLTAGE			0x0073	// Dec. 115		w	 
	#define	REG_DIAG_INVERTER_IN_POWER				0x0074	// Dec. 116		w	
	#define	REG_DIAG_INVERTER_OUT_POWER				0x0075	// Dec. 117		w	
	#define	REG_DIAG_INVERTER_REQ_FREQ				0x0076	// Dec. 118		w	
	#define	REG_DIAG_INVERTER_MAX_REQ_FREQ			0x0077	// Dec. 119		w	
    #define REG_DIAG_EVAP_TEMP_VALVE    			0x0078	// Dec. 120		w

	#define REG_SPLIT_ERRORE_1_BOX_MOTORE			0x0079	// Dec. 121		w	registro con gli errori del box motore
	#define REG_SPLIT_ERRORE_PERS1_BOX_MOTORE		0x007A	// Dec. 122		w	registro con gli errori persistenti 1 del box motore
	#define REG_SPLIT_ERRORE_PERS2_BOX_MOTORE		0x007B	// Dec. 123		w	registro con gli errori persistenti 2 del box motore

	#define	REG_DIAG_INVERTER_POWERINV				0x007C	// Dec. 124		w	
	#define REG_SPLIT_ERRORE_2_BOX_MOTORE			0x007D	// Dec. 125		w	registro con gli errori del box motore
	#define REG_SPLIT_CUMULATIVE_ERROR				0x007E	// Dec. 126		w	registro con i cumulativi errori normali e persistenti

	#define REG_DIAG_SPLIT_CMD_ADDRESS				0x007F	// Dec. 127		w	


	#define REG_SPLIT_PWM_VALUE_OFF				0x0080		// Registro con il valore PWM per la velocità Spento
	#define REG_SPLIT_PWM_VALUE_NIGHT			0x0081		// Registro con il valore PWM per la velocità Nigth
	#define REG_SPLIT_PWM_VALUE_SPEED1			0x0082		// Registro con il valore PWM per la velocità 1
	#define REG_SPLIT_PWM_VALUE_SPEED2			0x0083		// Registro con il valore PWM per la velocità 2
	#define REG_SPLIT_PWM_VALUE_SPEED3			0x0084		// Registro con il valore PWM per la velocità 3
	#define REG_SPLIT_PWM_VALUE_SPEED4			0x0085		// Registro con il valore PWM per la velocità 4
	#define REG_SPLIT_PWM_VALUE_SPEED5			0x0086		// Registro con il valore PWM per la velocità 5
	#define REG_SPLIT_PWM_VALUE_SPEED6			0x0087		// Registro con il valore PWM per la velocità 6
	#define REG_SPLIT_PWM_VALUE_SPEED7			0x0088		// Registro con il valore PWM per la velocità 7

	#define REG_SPLIT_TEMP_LIMIT_NIGHT			0x0090		// Registro con il valore limite di temperatura per la velocità Night
	#define REG_SPLIT_TEMP_LIMIT_SPEED1			0x0091		// Registro con il valore limite di temperatura per la velocità 1
	#define REG_SPLIT_TEMP_LIMIT_SPEED2			0x0092		// Registro con il valore limite di temperatura per la velocità 2
	#define REG_SPLIT_TEMP_LIMIT_SPEED3			0x0093		// Registro con il valore limite di temperatura per la velocità 3
	#define REG_SPLIT_TEMP_LIMIT_SPEED4			0x0094		// Registro con il valore limite di temperatura per la velocità 4
	#define REG_SPLIT_TEMP_LIMIT_SPEED5			0x0095		// Registro con il valore limite di temperatura per la velocità 5
	#define REG_SPLIT_TEMP_LIMIT_SPEED6			0x0096		// Registro con il valore limite di temperatura per la velocità 6
	#define REG_SPLIT_TEMP_LIMIT_SPEED7			0x0097		// Registro con il valore limite di temperatura per la velocità 7

	#define	REG_SPLIT_AD_T1_0					0x0100		// Registro con il valore della conversione della temperatura 0 per il ch T1
	#define	REG_SPLIT_AD_T1_1					0x0101		// Registro con il valore della conversione della temperatura 1 per il ch T1
	#define	REG_SPLIT_AD_T1_2					0x0102		// Registro con il valore della conversione della temperatura 2 per il ch T1
	#define	REG_SPLIT_AD_T1_3					0x0103		// Registro con il valore della conversione della temperatura 3 per il ch T1
	#define	REG_SPLIT_AD_T1_4					0x0104		// Registro con il valore della conversione della temperatura 4 per il ch T1
	#define	REG_SPLIT_AD_T1_5					0x0105		// Registro con il valore della conversione della temperatura 5 per il ch T1
	#define	REG_SPLIT_AD_T1_6					0x0106		// Registro con il valore della conversione della temperatura 6 per il ch T1
	#define	REG_SPLIT_AD_T1_7					0x0107		// Registro con il valore della conversione della temperatura 7 per il ch T1
	#define	REG_SPLIT_AD_T1_8					0x0108		// Registro con il valore della conversione della temperatura 8 per il ch T1
	#define	REG_SPLIT_AD_T1_9					0x0109		// Registro con il valore della conversione della temperatura 9 per il ch T1
	#define	REG_SPLIT_AD_T1_10					0x010A		// Registro con il valore della conversione della temperatura 10 per il ch T1
	#define	REG_SPLIT_AD_T1_11					0x010B		// Registro con il valore della conversione della temperatura 11 per il ch T1

	#define	REG_SPLIT_AD_T2_0					0x0110		// Registro con il valore della conversione della temperatura 0 per il ch T2
	#define	REG_SPLIT_AD_T2_1					0x0111		// Registro con il valore della conversione della temperatura 1 per il ch T2
	#define	REG_SPLIT_AD_T2_2					0x0112		// Registro con il valore della conversione della temperatura 2 per il ch T2
	#define	REG_SPLIT_AD_T2_3					0x0113		// Registro con il valore della conversione della temperatura 3 per il ch T2
	#define	REG_SPLIT_AD_T2_4					0x0114		// Registro con il valore della conversione della temperatura 4 per il ch T2
	#define	REG_SPLIT_AD_T2_5					0x0115		// Registro con il valore della conversione della temperatura 5 per il ch T2
	#define	REG_SPLIT_AD_T2_6					0x0116		// Registro con il valore della conversione della temperatura 6 per il ch T2
	#define	REG_SPLIT_AD_T2_7					0x0117		// Registro con il valore della conversione della temperatura 7 per il ch T2
	#define	REG_SPLIT_AD_T2_8					0x0118		// Registro con il valore della conversione della temperatura 8 per il ch T2
	#define	REG_SPLIT_AD_T2_9					0x0119		// Registro con il valore della conversione della temperatura 9 per il ch T2
	#define	REG_SPLIT_AD_T2_10					0x011A		// Registro con il valore della conversione della temperatura 10 per il ch T2
	#define	REG_SPLIT_AD_T2_11					0x011B		// Registro con il valore della conversione della temperatura 11 per il ch T2

	#define	REG_SPLIT_AD_T3_0					0x0120		// Registro con il valore della conversione della temperatura 0 per il ch T3
	#define	REG_SPLIT_AD_T3_1					0x0121		// Registro con il valore della conversione della temperatura 1 per il ch T3
	#define	REG_SPLIT_AD_T3_2					0x0122		// Registro con il valore della conversione della temperatura 2 per il ch T3
	#define	REG_SPLIT_AD_T3_3					0x0123		// Registro con il valore della conversione della temperatura 3 per il ch T3
	#define	REG_SPLIT_AD_T3_4					0x0124		// Registro con il valore della conversione della temperatura 4 per il ch T3
	#define	REG_SPLIT_AD_T3_5					0x0125		// Registro con il valore della conversione della temperatura 5 per il ch T3
	#define	REG_SPLIT_AD_T3_6					0x0126		// Registro con il valore della conversione della temperatura 6 per il ch T3
	#define	REG_SPLIT_AD_T3_7					0x0127		// Registro con il valore della conversione della temperatura 7 per il ch T3
	#define	REG_SPLIT_AD_T3_8					0x0128		// Registro con il valore della conversione della temperatura 8 per il ch T3
	#define	REG_SPLIT_AD_T3_9					0x0129		// Registro con il valore della conversione della temperatura 9 per il ch T3
	#define	REG_SPLIT_AD_T3_10					0x012A		// Registro con il valore della conversione della temperatura 10 per il ch T3
	#define	REG_SPLIT_AD_T3_11					0x012B		// Registro con il valore della conversione della temperatura 11 per il ch T3

	#define	REG_SPLIT_AD_T4_0					0x0130		// Registro con il valore della conversione della temperatura 0 per il ch T4
	#define	REG_SPLIT_AD_T4_1					0x0131		// Registro con il valore della conversione della temperatura 1 per il ch T4
	#define	REG_SPLIT_AD_T4_2					0x0132		// Registro con il valore della conversione della temperatura 2 per il ch T4
	#define	REG_SPLIT_AD_T4_3					0x0133		// Registro con il valore della conversione della temperatura 3 per il ch T4
	#define	REG_SPLIT_AD_T4_4					0x0134		// Registro con il valore della conversione della temperatura 4 per il ch T4
	#define	REG_SPLIT_AD_T4_5					0x0135		// Registro con il valore della conversione della temperatura 5 per il ch T4
	#define	REG_SPLIT_AD_T4_6					0x0136		// Registro con il valore della conversione della temperatura 6 per il ch T4
	#define	REG_SPLIT_AD_T4_7					0x0137		// Registro con il valore della conversione della temperatura 7 per il ch T4
	#define	REG_SPLIT_AD_T4_8					0x0138		// Registro con il valore della conversione della temperatura 8 per il ch T4
	#define	REG_SPLIT_AD_T4_9					0x0139		// Registro con il valore della conversione della temperatura 9 per il ch T4
	#define	REG_SPLIT_AD_T4_10					0x013A		// Registro con il valore della conversione della temperatura 10 per il ch T4
	#define	REG_SPLIT_AD_T4_11					0x013B		// Registro con il valore della conversione della temperatura 11 per il ch T4

	#define	REG_SPLIT_KT_T1_0					0x0140		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 0
	#define	REG_SPLIT_KT_T1_1					0x0141		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 1
	#define	REG_SPLIT_KT_T1_2					0x0142		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 2
	#define	REG_SPLIT_KT_T1_3					0x0143		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 3
	#define	REG_SPLIT_KT_T1_4					0x0144		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 4
	#define	REG_SPLIT_KT_T1_5					0x0145		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 5
	#define	REG_SPLIT_KT_T1_6					0x0146		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 6
	#define	REG_SPLIT_KT_T1_7					0x0147		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 7
	#define	REG_SPLIT_KT_T1_8					0x0148		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 8
	#define	REG_SPLIT_KT_T1_9					0x0149		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 9
	#define	REG_SPLIT_KT_T1_10					0x014A		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 10

	#define	REG_SPLIT_KT_T2_0					0x0150		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 0
	#define	REG_SPLIT_KT_T2_1					0x0151		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 1
	#define	REG_SPLIT_KT_T2_2					0x0152		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 2
	#define	REG_SPLIT_KT_T2_3					0x0153		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 3
	#define	REG_SPLIT_KT_T2_4					0x0154		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 4
	#define	REG_SPLIT_KT_T2_5					0x0155		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 5
	#define	REG_SPLIT_KT_T2_6					0x0156		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 6
	#define	REG_SPLIT_KT_T2_7					0x0157		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 7
	#define	REG_SPLIT_KT_T2_8					0x0158		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 8
	#define	REG_SPLIT_KT_T2_9					0x0159		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 9
	#define	REG_SPLIT_KT_T2_10					0x015A		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 10

	#define	REG_SPLIT_KT_T3_0					0x0160		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 0
	#define	REG_SPLIT_KT_T3_1					0x0161		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 1
	#define	REG_SPLIT_KT_T3_2					0x0162		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 2
	#define	REG_SPLIT_KT_T3_3					0x0163		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 3
	#define	REG_SPLIT_KT_T3_4					0x0164		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 4
	#define	REG_SPLIT_KT_T3_5					0x0165		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 5
	#define	REG_SPLIT_KT_T3_6					0x0166		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 6
	#define	REG_SPLIT_KT_T3_7					0x0167		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 7
	#define	REG_SPLIT_KT_T3_8					0x0168		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 8
	#define	REG_SPLIT_KT_T3_9					0x0169		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 9
	#define	REG_SPLIT_KT_T3_10					0x016A		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 10

	#define	REG_SPLIT_KT_T4_0					0x0170		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 0
	#define	REG_SPLIT_KT_T4_1					0x0171		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 1
	#define	REG_SPLIT_KT_T4_2					0x0172		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 2
	#define	REG_SPLIT_KT_T4_3					0x0173		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 3
	#define	REG_SPLIT_KT_T4_4					0x0174		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 4
	#define	REG_SPLIT_KT_T4_5					0x0175		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 5
	#define	REG_SPLIT_KT_T4_6					0x0176		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 6
	#define	REG_SPLIT_KT_T4_7					0x0177		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 7
	#define	REG_SPLIT_KT_T4_8					0x0178		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 8
	#define	REG_SPLIT_KT_T4_9					0x0179		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 9
	#define	REG_SPLIT_KT_T4_10					0x017A		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 10

// Registri diagnostici SUB Split letti dal Master e inviati su Split Master per la visualizzazione su Touch

    #define REG_SUB_SPLIT_EVAP_TEMP_VALVE           0x017B  // Dec. 379		R   
    #define REG_SUB_SPLIT_SETP_SUPERHEAT            0x017C  // Dec. 380		R   
    #define REG_SUB_SPLIT_CUMULATIVE_ALARM			0x017D  // Dec. 381		R   
    #define REG_SUB_SPLIT_SUPERHEAT					0x017E  // Dec. 382		R   
    #define REG_SUB_SPLIT_SUPERHEAT_RISCALDAMENTO   0x017F  // Dec. 383		R   
    #define REG_SUB_SPLIT_RES_REQ_ADDRESS           0x0180  // Dec. 384		R   
	#define	REG_SUB_SPLIT_TEST_ADDRESS				0x0181	// Dec. 385		R	
	#define	REG_SUB_SPLIT_STATUS					0x0182	// Dec. 386		R	
	#define	REG_SUB_SPLIT_LIQ_TEMP					0x0183  // Dec. 387		R
	#define	REG_SUB_SPLIT_GAS_TEMP					0x0184	// Dec. 388		R
	#define	REG_SUB_SPLIT_AIR_IN					0x0185	// Dec. 389		R
	#define	REG_SUB_SPLIT_AIR_OUT					0x0186	// Dec. 390		R
	#define	REG_SUB_SPLIT_FAN_SPEED                 0x0187	// Dec. 391		R
	#define	REG_SUB_SPLIT_EXP_VALVE                 0x0188	// Dec. 392		R
	#define	REG_SUB_SPLIT_GAS_PRESSURE				0x0189	// Dec. 393		R
	#define	REG_SUB_SPLIT_LIQ_PRESSURE				0x018A	// Dec. 394		R
	#define	REG_SUB_SPLIT_REQUEST_AIR_TEMP			0x018B  // Dec. 395		R
	#define	REG_SUB_SPLIT_REQUEST_AIR_PRES			0x018C	// Dec. 396		R
	#define	REG_SUB_SPLIT_ENG_MODE					0x018D	// Dec. 397		R	
	#define	REG_SUB_SPLIT_CYCLE_TIME				0x018E	// Dec. 398		R
    #define REG_SUB_SPLIT_QUANTITY                  0x018F	// Dec. 399		R

    #define REG_SPLIT_EVAP_TEMP_VALVE               0x0190	// Dec. 400		R

    #define REG_SPLIT_ACC_INVERTER					0x0191	// Dec. 401		R
    #define REG_SPLIT_ACC_GASPRESS					0x0192	// Dec. 402		R

	#define	REG_MASTER_HW_VERSION					0x0193  // Dec. 403		R
	#define	REG_MASTER_FW_VERSION_HW				0x0194  // Dec. 404		R
	#define	REG_MASTER_FW_VERSION_FW				0x0195  // Dec. 405		R
	#define	REG_MASTER_FW_REVISION_FW				0x0196  // Dec. 406		R

	#define	REG_SPLIT_HW_VERSION					0x0197  // Dec. 407		R
	#define	REG_SPLIT_FW_VERSION_HW					0x0198  // Dec. 408		R
	#define	REG_SPLIT_FW_VERSION_FW					0x0199  // Dec. 409		R
	#define	REG_SPLIT_FW_REVISION_FW				0x019A  // Dec. 410		R

	#define	REG_SUB_SPLIT_HW_VERSION				0x019B  // Dec. 411		R
	#define	REG_SUB_SPLIT_FW_VERSION_HW				0x019C  // Dec. 412		R
	#define	REG_SUB_SPLIT_FW_VERSION_FW				0x019D  // Dec. 413		R
	#define	REG_SUB_SPLIT_FW_REVISION_FW			0x019E  // Dec. 414		R

	#define	REG_DIAG_SPLIT_HW_VERSION				0x019F  // Dec. 415		R
    #define REG_SUB_SPLIT_FLAGS_ENABLE_1_16         0x01A0  // Dec. 416     R
    #define REG_SUB_SPLIT_FLAGS_OFFLINE_1_16        0x01A1  // Dec. 417     R
    #define REG_SUB_SPLIT_FLAGS_ONLINE_1_16         0x01A2  // Dec. 418     R
    #define REG_SUB_SPLIT_FLAGS_ALARM_1_16          0x01A3  // Dec. 419     R
    #define REG_DIAG_SUB_SPLIT_FLAGS_ENABLE_1_16    0x01A4  // Dec. 420     R
    #define REG_DIAG_SUB_SPLIT_FLAGS_OFFLINE_1_16   0x01A5  // Dec. 421     R
    #define REG_DIAG_SUB_SPLIT_FLAGS_ONLINE_1_16    0x01A6  // Dec. 422     R
    #define REG_DIAG_SUB_SPLIT_FLAGS_ALARM_1_16     0x01A7  // Dec. 423     R

    #define REG_DIAG_SPLIT_TEMP_AMB_MIDDLE			0x01A9  // Dec. 425     R      // Indirizzo della temperatura ambiente mediata con sottoslave
    #define REG_DIAG_SPLIT_TEMP_AMB_REAL   			0x01B0  // Dec. 426     R      // Indirizzo della temperatura ambiente Reale (mediata con sottoslave o meno da FWSelection)

    #define REG_DIAG_SPLIT_FUNCTION_MODE            0x01B1  // Dec. 427     R
    #define REG_SUB_SPLIT_FUNCTION_MODE             0x01B2  // Dec. 428     R

	#define	REG_SUB_SPLIT_EXP_VALVE_ERR_POS         0x01B3	// Dec. 429		R
	#define	REG_SUB_SPLIT_EXP_VALVE_ERR_COD         0x01B4	// Dec. 430		R
	#define	REG_DIAG_SPLIT_EXP_VALVE_ERR_POS        0x01B5	// Dec. 431		R
	#define	REG_DIAG_SPLIT_EXP_VALVE_ERR_CODE       0x01B6	// Dec. 432		R

															// Liberi....
	#define	REG_DIAG_SPLIT_FW_VERSION_HW			0x0200  // Dec. 512		R
	#define	REG_DIAG_SPLIT_FW_VERSION_FW			0x0201  // Dec. 513		R
	#define	REG_DIAG_SPLIT_FW_REVISION_FW			0x0202  // Dec. 514		R

	#define	REG_SPLIT_CURRENT_MODE_ENGINE_BOX2		0x0203  // Dec. 515		R
	#define	REG_SPLIT_ENGINE_BOX2_T_COMP_OUT		0x0204  // Dec. 516		R
	#define	REG_SPLIT_ERRORE_1_BOX2_MOTORE			0x0205  // Dec. 517		R
	#define	REG_SPLIT_ERRORE_PERS1_BOX2_MOTORE		0x0206  // Dec. 518		R
	#define	REG_SPLIT_ERRORE_PERS2_BOX2_MOTORE		0x0207  // Dec. 519		R
	#define	REG_SPLIT_CUMULATIVE_ERROR2				0x0208  // Dec. 520		R
	#define	REG_SPLIT_STATUS_BOX2_MOTORE			0x0209  // Dec. 521		R
// .... Liberi da 0x020A.. a 0x020F
	#define	REG_SPLIT_DOUBLE_COMPRESSSOR_OK			0x0210  // Dec. 528		R
	
	#define REG_DIAG_SPLIT_SUPERHEAT				0x0211  // Dec. 529		R
	#define REG_DIAG_SPLIT_SUPERHEAT_RISCALDAMENTO	0x0212  // Dec. 530		R
	#define REG_SPLIT_SUPERHEAT						0x0213  // Dec. 531		R
	#define REG_SPLIT_SUPERHEAT_RISCALDAMENTO		0x0214  // Dec. 532		R

	#define REG_DIAG_SPLIT_SETP_SUPERHEAT       	0x0215  // Dec. 533		R
	#define REG_SPLIT_SETP_SUPERHEAT        		0x0216  // Dec. 534		R

	#define	REG_SPLIT_POWER_MAX_AVAILABLE			0x0218  // Dec. 536		R
	#define	REG_SPLIT_POWER_ECOMODE					0x0219  // Dec. 537		R
	#define	REG_SPLIT_ECOMODE_ENABLE				0x021A  // Dec. 538		R
	#define REG_SPLIT_INVERTER_QUANTITY				0x021B  // Dec. 539		R
	#define REG_DIAG_INVERTER_ADDRESS				0x021C  // Dec. 540		R
	#define REG_SPLIT_BOX_ABSORPTION				0x021D  // Dec. 541		R

	//#define REG_SPLIT_BOX_ABSORPTION				0x021D  // Dec. 541		R
	#define REG_SPLIT_ENGINE_BOX2_COMPRESSOR_SPEED	0x021E  // Dec. 542		R
	#define REG_SPLIT_MAX_DIAG_BOX_QUANTITY			0x021F  // Dec. 543		R

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
/* DA IMPLEMENTARE I COMANDI QUI SOTTO DA 556 a 567 */
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

    #define REG_SPLIT_UTA_DEHUMI_ON             	592
    #define REG_SPLIT_UTASP_ON                      593
    #define REG_SPLIT_UTA_SP                        594
    #define REG_SPLIT_UTA_SP_HUMI                   595

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

    #define REG_SPLIT_SET_BAUDRATE                  690	// Vaule: 1 = HiSpeed (115000Baud) 0 = LowSpeed (57600Baud)

    #define REG_DIAG_SPLIT_ONLY_FAN                 691
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

	#define REG_MASTER_COMERROR_CNTCOMERR                       864
    #define REG_MASTER_COMERROR_TIMEOUTCOMERR                   865
    #define REG_MASTER_COMERROR_CRC_COMERR                      866
    #define REG_MASTER_COMERROR_MODCNTCOMERR                    867
    #define REG_MASTER_COMERROR_MODTIMEOUTCOMERR                868
    #define REG_MASTER_COMERROR_MODCRC_COMERR                   869
    #define REG_MASTER_COMERROR_MODCNTCOMERR_U2                 870
    #define REG_MASTER_COMERROR_MODTIMEOUTCOMERR_U2             871
    #define REG_MASTER_COMERROR_MODCRC_COMERR_U2                872
    #define REG_MASTER_COMERROR_CNTCOMERR_U3                    873
    #define REG_MASTER_COMERROR_TIMEOUTCOMERR_U3                874
    #define REG_MASTER_COMERROR_CRC_COMERR_U3                   875

    #define REG_SPLIT_WR_EXTRACTOR_NUMBER                       876
    #define REG_SPLIT_WR_DELTAVEL_EXTRACTOR                     877
    #define REG_SPLIT_RD_EXTRACTOR_NUMBER                       878
    #define REG_SPLIT_RD_DELTAVEL_EXTRACTOR                     879
    #define REG_SPLIT_WR_EXTRACTOR_AUTOVEL                      880
    #define REG_SPLIT_WR_EXTRACTOR_ABIL                         881

    #define REG_DIAG_SPLIT_MODEL_VERSION                        882

    #define REG_SPLIT_FAN_M3H                                   883
    #define REG_SUB_SPLIT_FAN_M3H                               884
    #define REG_DIAG_SPLIT_FAN_M3H                              885

    #define REG_SPLIT_VENTIL_SELECTION                          886
    #define REG_SPLIT_M3H_MIN                                   887
    #define REG_SPLIT_M3H_MAX                                   888

    #define REG_SPLIT_AIR_POWER_OUTPUT                          889
    #define REG_SPLIT_AIR_POWER_BTU                             890
    #define REG_SUB_SPLIT_AIR_POWER_OUTPUT                      891
    #define REG_SUB_SPLIT_AIR_POWER_BTU                         892

    #define REG_DIAG_SPLIT_AIR_POWER_OUTPUT                     893
    #define REG_DIAG_SPLIT_AIR_POWER_BTU                        894

    #define REG_SPLIT_AND_SUB_AIR_POWER_OUTPUT                  895
    #define REG_SPLIT_AND_SUB_AIR_POWER_BTU                     896

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
	#define REG_SPLIT_RD_SET_DELTA_TEMP_MIN_DEHUMI               939
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

// registri modulo dimmer
	#define	REG_OUT_CH1						0x0001		// Valore dell'output CH1
	#define	REG_OUT_CH2						0x0002		// Valore dell'output CH2
	#define	REG_OUT_CH3						0x0003		// Valore dell'output CH3
	#define	REG_OUT_CH4						0x0004		// Valore dell'output CH4
	#define	REG_OUT_CH5						0x0005		// Valore dell'output CH5
	#define	REG_OUT_CH6						0x0006		// Valore dell'output CH6

	#define	REG_MODE_OUT					0x0007		// Ogni bit indica il settaggio della modalita di uscita (0=> On-Off; 1=> Dimmer)
	#define	REG_MODE_PULS					0x0008		// Ogni bit indica il setaggio dei pulsanti di ogni canale (0=> doppio pulsnate; 1=> tasto singolo)
	
	#define	REG_INC_OUT_CH1					0x0009		// Registro con il valore dell'incremento dell'uscita del CH1
	#define	REG_INC_OUT_CH2					0x000A		// Registro con il valore dell'incremento dell'uscita del CH2
	#define	REG_INC_OUT_CH3					0x000B		// Registro con il valore dell'incremento dell'uscita del CH3
	#define	REG_INC_OUT_CH4					0x000C		// Registro con il valore dell'incremento dell'uscita del CH4
	#define	REG_INC_OUT_CH5					0x000D		// Registro con il valore dell'incremento dell'uscita del CH5
	#define	REG_INC_OUT_CH6					0x000E		// Registro con il valore dell'incremento dell'uscita del CH6

	#define	REG_REFRESH_TIME_CH1			0x000F		// Registro con il valore del tempo di aggiornamento dell'uscita del CH 1
	#define	REG_REFRESH_TIME_CH2			0x0010		// Registro con il valore del tempo di aggiornamento dell'uscita del CH 2
	#define	REG_REFRESH_TIME_CH3			0x0011		// Registro con il valore del tempo di aggiornamento dell'uscita del CH 3
	#define	REG_REFRESH_TIME_CH4			0x0012		// Registro con il valore del tempo di aggiornamento dell'uscita del CH 4
	#define	REG_REFRESH_TIME_CH5			0x0013		// Registro con il valore del tempo di aggiornamento dell'uscita del CH 5
	#define	REG_REFRESH_TIME_CH6			0x0014		// Registro con il valore del tempo di aggiornamento dell'uscita del CH 6

	#define	REG_DIMMER_OUT_MATRIX_CH1		0x0015		// Registro con il numero del canale dei pulsanti che deve seguire l'uscita del CH1
	#define	REG_DIMMER_OUT_MATRIX_CH2		0x0016		// Registro con il numero del canale dei pulsanti che deve seguire l'uscita del CH1
	#define	REG_DIMMER_OUT_MATRIX_CH3		0x0017		// Registro con il numero del canale dei pulsanti che deve seguire l'uscita del CH1
	#define	REG_DIMMER_OUT_MATRIX_CH4		0x0018		// Registro con il numero del canale dei pulsanti che deve seguire l'uscita del CH1
	#define	REG_DIMMER_OUT_MATRIX_CH5		0x0019		// Registro con il numero del canale dei pulsanti che deve seguire l'uscita del CH1
	#define	REG_DIMMER_OUT_MATRIX_CH6		0x001A		// Registro con il numero del canale dei pulsanti che deve seguire l'uscita del CH1

	// registri comuni
    #define	REG_SUB_SPLIT_MODEL_VERSION     65439       // Identificativo del modulo (tipo di modulo) del subslave
	#define	REG_FW_VERSION					0xFFA0		// Versione del Fw, coincide con l'hardware
	#define	REG_FW_REVISION					0xFFA1		// Revisione del Fw
	#define	REG_SPLIT_MODEL_VERSION 		0xFFA2		// Identificativo del modulo (tipo di modulo)
	#define	REG_ADD_MODULO					0xFFA3		// Indirizzo del modulo
	#define	REG_SYSTEM_NUMBER_LO			0xFFA4		// System number (Low word)
	#define	REG_SYSTEM_NUMBER_HI			0xFFA5		// System number (high word)
	#define	REG_FABRICATION_DAY				0xFFA6		// Giorno della data di fabbricazione
	#define	REG_FABRICATION_MONTH			0xFFA7		// Mese della data di fabbricazione
	#define	REG_FABRICATION_YEAR			0xFFA8		// Anno della data di fabbricazione
	#define	REG_FLASH_CRC_16				0xFFA9		// CRC della flash
	#define	REG_SAVE_2_E2					0xFFAA		// salva i dati in E2

// Registri per funzioni speciali
	#define	REG_BOARD_AUTORESET				0xFFF0		// 65520d: Autoreset scheda (vedi Bootloader)
														// per poter eseguire l'autoreset inviare in sequenza i seguenti valori: 0x00, 0x55 (85d), 0xAA (170), 0x55AA (21930d)
	#define	REG_BOARD_SUBSLAVE_AUTORESET	0xFFF1		// 65521d: Reset scheda sottoslave (vedi Bootloader)
                                                        // per poter inviare il comando di reset al sottoslave inviare in sequenza i seguenti valori: 0x00, 0x55 (85d), 0xAA (170), 0x55AA (21930d)
	#define	REG_BOARD_BUSMERGE              0xFFF2		// 65522d: Fa entrare la scheda in modalità BUSMERGE per scambiare i dati tra BUS-M e BUS-S in modo trasparente
	#define	REG_BOARD_SPECIALFUNC3			0xFFF3		// 65523d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC4			0xFFF4		// 65524d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC5			0xFFF5		// 65525d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC6			0xFFF6		// 65526d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC7			0xFFF7		// 65527d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC8			0xFFF8		// 65528d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC9			0xFFF9		// 65529d: Libero per funzioni speciali
//.....
	#define	REG_BOARD_SPECIALFUNC15			0xFFFF		// 65535d: Libero per funzioni speciali




	// registri modulo slave
	#define	REG_MASTER_TEMP_1				0x0001
	#define	REG_MASTER_TEMP_2				0x0002
	#define	REG_MASTER_TEMP_3				0x0003
	#define	REG_MASTER_TEMP_4				0x0004
	#define	REG_MASTER_TEMP_5				0x0005
	#define	REG_MASTER_TEMP_6				0x0006
	#define	REG_MASTER_GAS_PRESSURE_G		0x0007
	#define	REG_MASTER_ADC_1				0x0008
	#define	REG_MASTER_ADC_2				0x0009
	#define	REG_MASTER_ADC_3				0x000A
	#define	REG_MASTER_ADC_4				0x000B
	#define	REG_MASTER_ADC_5				0x000C
	#define	REG_MASTER_ADC_6				0x000D
	#define	REG_MASTER_ADC_7				0x000E
	#define	REG_MASTER_ADC_8				0x000F
	#define	REG_MASTER_DIGIT_INPUT			0x0010
	#define	REG_INVERTER_ST_RELE			0x0011
	#define	REG_INVERTER_DAC_1				0x0012
	#define	REG_INVERTER_DAC_2				0x0013
	#define	REG_INVERTER_DAC_3				0x0014
	#define	REG_INVERTER_DAC_4				0x0015
	#define	REG_MASTER_GAS_PRESSURE_P		0x0016

	#define	REG_MASTER_AD_T1_0				0x0020		// Registro con il valore della conversione della temperatura 0 per il ch T1
	#define	REG_MASTER_AD_T1_1				0x0021		// Registro con il valore della conversione della temperatura 1 per il ch T1
	#define	REG_MASTER_AD_T1_2				0x0022		// Registro con il valore della conversione della temperatura 2 per il ch T1
	#define	REG_MASTER_AD_T1_3				0x0023		// Registro con il valore della conversione della temperatura 3 per il ch T1
	#define	REG_MASTER_AD_T1_4				0x0024		// Registro con il valore della conversione della temperatura 4 per il ch T1
	#define	REG_MASTER_AD_T1_5				0x0025		// Registro con il valore della conversione della temperatura 5 per il ch T1
	#define	REG_MASTER_AD_T1_6				0x0026		// Registro con il valore della conversione della temperatura 6 per il ch T1
	#define	REG_MASTER_AD_T1_7				0x0027		// Registro con il valore della conversione della temperatura 7 per il ch T1
	#define	REG_MASTER_AD_T1_8				0x0028		// Registro con il valore della conversione della temperatura 8 per il ch T1
	#define	REG_MASTER_AD_T1_9				0x0029		// Registro con il valore della conversione della temperatura 9 per il ch T1
	#define	REG_MASTER_AD_T1_10				0x002A		// Registro con il valore della conversione della temperatura 10 per il ch T1
	#define	REG_MASTER_AD_T1_11				0x002B		// Registro con il valore della conversione della temperatura 11 per il ch T1

	#define	REG_MASTER_AD_T2_0				0x002C		// Registro con il valore della conversione della temperatura 0 per il ch T2
	#define	REG_MASTER_AD_T2_1				0x002D		// Registro con il valore della conversione della temperatura 1 per il ch T2
	#define	REG_MASTER_AD_T2_2				0x002E		// Registro con il valore della conversione della temperatura 2 per il ch T2
	#define	REG_MASTER_AD_T2_3				0x002F		// Registro con il valore della conversione della temperatura 3 per il ch T2
	#define	REG_MASTER_AD_T2_4				0x0030		// Registro con il valore della conversione della temperatura 4 per il ch T2
	#define	REG_MASTER_AD_T2_5				0x0031		// Registro con il valore della conversione della temperatura 5 per il ch T2
	#define	REG_MASTER_AD_T2_6				0x0032		// Registro con il valore della conversione della temperatura 6 per il ch T2
	#define	REG_MASTER_AD_T2_7				0x0033		// Registro con il valore della conversione della temperatura 7 per il ch T2
	#define	REG_MASTER_AD_T2_8				0x0034		// Registro con il valore della conversione della temperatura 8 per il ch T2
	#define	REG_MASTER_AD_T2_9				0x0035		// Registro con il valore della conversione della temperatura 9 per il ch T2
	#define	REG_MASTER_AD_T2_10				0x0036		// Registro con il valore della conversione della temperatura 10 per il ch T2
	#define	REG_MASTER_AD_T2_11				0x0037		// Registro con il valore della conversione della temperatura 11 per il ch T2

	#define	REG_MASTER_AD_T3_0				0x0038		// Registro con il valore della conversione della temperatura 0 per il ch T3
	#define	REG_MASTER_AD_T3_1				0x0039		// Registro con il valore della conversione della temperatura 1 per il ch T3
	#define	REG_MASTER_AD_T3_2				0x003A		// Registro con il valore della conversione della temperatura 2 per il ch T3
	#define	REG_MASTER_AD_T3_3				0x003B		// Registro con il valore della conversione della temperatura 3 per il ch T3
	#define	REG_MASTER_AD_T3_4				0x003C		// Registro con il valore della conversione della temperatura 4 per il ch T3
	#define	REG_MASTER_AD_T3_5				0x003D		// Registro con il valore della conversione della temperatura 5 per il ch T3
	#define	REG_MASTER_AD_T3_6				0x003E		// Registro con il valore della conversione della temperatura 6 per il ch T3
	#define	REG_MASTER_AD_T3_7				0x003F		// Registro con il valore della conversione della temperatura 7 per il ch T3
	#define	REG_MASTER_AD_T3_8				0x0040		// Registro con il valore della conversione della temperatura 8 per il ch T3
	#define	REG_MASTER_AD_T3_9				0x0041		// Registro con il valore della conversione della temperatura 9 per il ch T3
	#define	REG_MASTER_AD_T3_10				0x0042		// Registro con il valore della conversione della temperatura 10 per il ch T3
	#define	REG_MASTER_AD_T3_11				0x0043		// Registro con il valore della conversione della temperatura 11 per il ch T3

	#define	REG_MASTER_AD_T4_0				0x0044		// Registro con il valore della conversione della temperatura 0 per il ch T4
	#define	REG_MASTER_AD_T4_1				0x0045		// Registro con il valore della conversione della temperatura 1 per il ch T4
	#define	REG_MASTER_AD_T4_2				0x0046		// Registro con il valore della conversione della temperatura 2 per il ch T4
	#define	REG_MASTER_AD_T4_3				0x0047		// Registro con il valore della conversione della temperatura 3 per il ch T4
	#define	REG_MASTER_AD_T4_4				0x0048		// Registro con il valore della conversione della temperatura 4 per il ch T4
	#define	REG_MASTER_AD_T4_5				0x0049		// Registro con il valore della conversione della temperatura 5 per il ch T4
	#define	REG_MASTER_AD_T4_6				0x004A		// Registro con il valore della conversione della temperatura 6 per il ch T4
	#define	REG_MASTER_AD_T4_7				0x004B		// Registro con il valore della conversione della temperatura 7 per il ch T4
	#define	REG_MASTER_AD_T4_8				0x004C		// Registro con il valore della conversione della temperatura 8 per il ch T4
	#define	REG_MASTER_AD_T4_9				0x004D		// Registro con il valore della conversione della temperatura 9 per il ch T4
	#define	REG_MASTER_AD_T4_10				0x004E		// Registro con il valore della conversione della temperatura 10 per il ch T4
	#define	REG_MASTER_AD_T4_11				0x004F		// Registro con il valore della conversione della temperatura 11 per il ch T4

	#define	REG_MASTER_AD_T5_0				0x0050		// Registro con il valore della conversione della temperatura 0 per il ch T5
	#define	REG_MASTER_AD_T5_1				0x0051		// Registro con il valore della conversione della temperatura 1 per il ch T5
	#define	REG_MASTER_AD_T5_2				0x0052		// Registro con il valore della conversione della temperatura 2 per il ch T5
	#define	REG_MASTER_AD_T5_3				0x0053		// Registro con il valore della conversione della temperatura 3 per il ch T5
	#define	REG_MASTER_AD_T5_4				0x0054		// Registro con il valore della conversione della temperatura 4 per il ch T5
	#define	REG_MASTER_AD_T5_5				0x0055		// Registro con il valore della conversione della temperatura 5 per il ch T5
	#define	REG_MASTER_AD_T5_6				0x0056		// Registro con il valore della conversione della temperatura 6 per il ch T5
	#define	REG_MASTER_AD_T5_7				0x0057		// Registro con il valore della conversione della temperatura 7 per il ch T5
	#define	REG_MASTER_AD_T5_8				0x0058		// Registro con il valore della conversione della temperatura 8 per il ch T5
	#define	REG_MASTER_AD_T5_9				0x0059		// Registro con il valore della conversione della temperatura 9 per il ch T5
	#define	REG_MASTER_AD_T5_10				0x005A		// Registro con il valore della conversione della temperatura 10 per il ch T5
	#define	REG_MASTER_AD_T5_11				0x005B		// Registro con il valore della conversione della temperatura 11 per il ch T5

	#define	REG_MASTER_AD_T6_0				0x005C		// Registro con il valore della conversione della temperatura 0 per il ch T6
	#define	REG_MASTER_AD_T6_1				0x005D		// Registro con il valore della conversione della temperatura 1 per il ch T6
	#define	REG_MASTER_AD_T6_2				0x005E		// Registro con il valore della conversione della temperatura 2 per il ch T6
	#define	REG_MASTER_AD_T6_3				0x005F		// Registro con il valore della conversione della temperatura 3 per il ch T6
	#define	REG_MASTER_AD_T6_4				0x0060		// Registro con il valore della conversione della temperatura 4 per il ch T6
	#define	REG_MASTER_AD_T6_5				0x0061		// Registro con il valore della conversione della temperatura 5 per il ch T6
	#define	REG_MASTER_AD_T6_6				0x0062		// Registro con il valore della conversione della temperatura 6 per il ch T6
	#define	REG_MASTER_AD_T6_7				0x0063		// Registro con il valore della conversione della temperatura 7 per il ch T6
	#define	REG_MASTER_AD_T6_8				0x0064		// Registro con il valore della conversione della temperatura 8 per il ch T6
	#define	REG_MASTER_AD_T6_9				0x0065		// Registro con il valore della conversione della temperatura 9 per il ch T6
	#define	REG_MASTER_AD_T6_10				0x0066		// Registro con il valore della conversione della temperatura 10 per il ch T6
	#define	REG_MASTER_AD_T6_11				0x0067		// Registro con il valore della conversione della temperatura 11 per il ch T6

	#define	REG_MASTER_KT_T1_0				0x0068		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 0
	#define	REG_MASTER_KT_T1_1				0x0069		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 1
	#define	REG_MASTER_KT_T1_2				0x006A		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 2
	#define	REG_MASTER_KT_T1_3				0x006B		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 3
	#define	REG_MASTER_KT_T1_4				0x006C		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 4
	#define	REG_MASTER_KT_T1_5				0x006D		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 5
	#define	REG_MASTER_KT_T1_6				0x006E		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 6
	#define	REG_MASTER_KT_T1_7				0x006F		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 7
	#define	REG_MASTER_KT_T1_8				0x0070		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 8
	#define	REG_MASTER_KT_T1_9				0x0071		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 9
	#define	REG_MASTER_KT_T1_10				0x0072		// Registro con il coefficente per la conversione della temperatura del ch T1 nel segmento 10

	#define	REG_MASTER_KT_T2_0				0x0073		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 0
	#define	REG_MASTER_KT_T2_1				0x0074		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 1
	#define	REG_MASTER_KT_T2_2				0x0075		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 2
	#define	REG_MASTER_KT_T2_3				0x0076		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 3
	#define	REG_MASTER_KT_T2_4				0x0077		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 4
	#define	REG_MASTER_KT_T2_5				0x0078		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 5
	#define	REG_MASTER_KT_T2_6				0x0079		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 6
	#define	REG_MASTER_KT_T2_7				0x007A		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 7
	#define	REG_MASTER_KT_T2_8				0x007B		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 8
	#define	REG_MASTER_KT_T2_9				0x007C		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 9
	#define	REG_MASTER_KT_T2_10				0x007D		// Registro con il coefficente per la conversione della temperatura del ch T2 nel segmento 10

	#define	REG_MASTER_KT_T3_0				0x007E		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 0
	#define	REG_MASTER_KT_T3_1				0x007F		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 1
	#define	REG_MASTER_KT_T3_2				0x0080		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 2
	#define	REG_MASTER_KT_T3_3				0x0081		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 3
	#define	REG_MASTER_KT_T3_4				0x0082		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 4
	#define	REG_MASTER_KT_T3_5				0x0083		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 5
	#define	REG_MASTER_KT_T3_6				0x0084		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 6
	#define	REG_MASTER_KT_T3_7				0x0085		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 7
	#define	REG_MASTER_KT_T3_8				0x0086		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 8
	#define	REG_MASTER_KT_T3_9				0x0087		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 9
	#define	REG_MASTER_KT_T3_10				0x0088		// Registro con il coefficente per la conversione della temperatura del ch T3 nel segmento 10

	#define	REG_MASTER_KT_T4_0				0x0089		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 0
	#define	REG_MASTER_KT_T4_1				0x008A		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 1
	#define	REG_MASTER_KT_T4_2				0x008B		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 2
	#define	REG_MASTER_KT_T4_3				0x008C		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 3
	#define	REG_MASTER_KT_T4_4				0x008D		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 4
	#define	REG_MASTER_KT_T4_5				0x008E		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 5
	#define	REG_MASTER_KT_T4_6				0x008F		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 6
	#define	REG_MASTER_KT_T4_7				0x0090		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 7
	#define	REG_MASTER_KT_T4_8				0x0091		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 8
	#define	REG_MASTER_KT_T4_9				0x0092		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 9
	#define	REG_MASTER_KT_T4_10				0x0093		// Registro con il coefficente per la conversione della temperatura del ch T4 nel segmento 10

	#define	REG_MASTER_KT_T5_0				0x0094		// Registro con il coefficente per la conversione della temperatura del ch T5 nel segmento 0
	#define	REG_MASTER_KT_T5_1				0x0095		// Registro con il coefficente per la conversione della temperatura del ch T5 nel segmento 1
	#define	REG_MASTER_KT_T5_2				0x0096		// Registro con il coefficente per la conversione della temperatura del ch T5 nel segmento 2
	#define	REG_MASTER_KT_T5_3				0x0097		// Registro con il coefficente per la conversione della temperatura del ch T5 nel segmento 3
	#define	REG_MASTER_KT_T5_4				0x0098		// Registro con il coefficente per la conversione della temperatura del ch T5 nel segmento 4
	#define	REG_MASTER_KT_T5_5				0x0099		// Registro con il coefficente per la conversione della temperatura del ch T5 nel segmento 5
	#define	REG_MASTER_KT_T5_6				0x009A		// Registro con il coefficente per la conversione della temperatura del ch T5 nel segmento 6
	#define	REG_MASTER_KT_T5_7				0x009B		// Registro con il coefficente per la conversione della temperatura del ch T5 nel segmento 7
	#define	REG_MASTER_KT_T5_8				0x009C		// Registro con il coefficente per la conversione della temperatura del ch T5 nel segmento 8
	#define	REG_MASTER_KT_T5_9				0x009D		// Registro con il coefficente per la conversione della temperatura del ch T5 nel segmento 9
	#define	REG_MASTER_KT_T5_10				0x009E		// Registro con il coefficente per la conversione della temperatura del ch T5 nel segmento 10

	#define	REG_MASTER_KT_T6_0				0x000F		// Registro con il coefficente per la conversione della temperatura del ch T6 nel segmento 0
	#define	REG_MASTER_KT_T6_1				0x0100		// Registro con il coefficente per la conversione della temperatura del ch T6 nel segmento 1
	#define	REG_MASTER_KT_T6_2				0x0101		// Registro con il coefficente per la conversione della temperatura del ch T6 nel segmento 2
	#define	REG_MASTER_KT_T6_3				0x0102		// Registro con il coefficente per la conversione della temperatura del ch T6 nel segmento 3
	#define	REG_MASTER_KT_T6_4				0x0103		// Registro con il coefficente per la conversione della temperatura del ch T6 nel segmento 4
	#define	REG_MASTER_KT_T6_5				0x0104		// Registro con il coefficente per la conversione della temperatura del ch T6 nel segmento 5
	#define	REG_MASTER_KT_T6_6				0x0105		// Registro con il coefficente per la conversione della temperatura del ch T6 nel segmento 6
	#define	REG_MASTER_KT_T6_7				0x0106		// Registro con il coefficente per la conversione della temperatura del ch T6 nel segmento 7
	#define	REG_MASTER_KT_T6_8				0x0107		// Registro con il coefficente per la conversione della temperatura del ch T6 nel segmento 8
	#define	REG_MASTER_KT_T6_9				0x0108		// Registro con il coefficente per la conversione della temperatura del ch T6 nel segmento 9
	#define	REG_MASTER_KT_T6_10				0x0109		// Registro con il coefficente per la conversione della temperatura del ch T6 nel segmento 10






// Registri d'informazione
	#define	REG_MASTER_FW_VERSION			0xFFA0		// Versione del Fw, coincide con l'hardware
	#define	REG_MASTER_FW_REVISION			0xFFA1		// Revisione del Fw
	#define	REG_MASTER_ID_MODULO			0xFFA2		// Identificativo del modulo (tipo di modulo)
	#define	REG_MASTER_ADD_MODULO			0xFFA3		// Indirizzo del modulo
	#define	REG_MASTER_SYSTEM_NUMBER_LO		0xFFA4		// System number (Low word)
	#define	REG_MASTER_SYSTEM_NUMBER_HI		0xFFA5		// System number (high word)
	#define	REG_MASTER_FABRICATION_DAY		0xFFA6		// Giorno della data di fabbricazione
	#define	REG_MASTER_FABRICATION_MONTH	0xFFA7		// Mese della data di fabbricazione
	#define	REG_MASTER_FABRICATION_YEAR		0xFFA8		// Anno della data di fabbricazione
	#define	REG_MASTER_FLASH_CRC_16			0xFFA9		// CRC della flash
	#define	REG_MASTER_SAVE_2_E2			0xFFAA		// salva i dati in E2
	#define	REG_MASTER_HOURS_OF_LIFE		0xFFAB		// Ore di vita del modulo Master


// Registri per funzioni speciali
	#define	REG_BOARD_AUTORESET				0xFFF0		// 65520d: Autoreset scheda (vedi Bootloader)
														// per poter eseguire l'autoreset inviare in sequenza i seguenti valori: 0x00, 0x55 (85d), 0xAA (170), 0x55AA (21930d)
	#define	REG_BOARD_SPECIALFUNC1			0xFFF1		// 65521d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC2			0xFFF2		// 65522d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC3			0xFFF3		// 65523d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC4			0xFFF4		// 65524d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC5			0xFFF5		// 65525d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC6			0xFFF6		// 65526d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC7			0xFFF7		// 65527d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC8			0xFFF8		// 65528d: Libero per funzioni speciali
	#define	REG_BOARD_SPECIALFUNC9			0xFFF9		// 65529d: Libero per funzioni speciali
//.....
	#define	REG_BOARD_SPECIALFUNC15			0xFFFF		// 65535d: Libero per funzioni speciali


	//----------------------------------
	// Strutture
	//----------------------------------
	typedef struct 
	{			 
		int	Mode;
		int	Temp_Evaporazione;
		int	PowerLimit;
		int	Power_EcoMode;			// Valore di limitazione Potenza in Eco Mode	Watt	
		int	OldPowerLimit;
		int	SplitStatus;
		int DefMode;
		int PowerOn;		
		int	SetPoint;                         	
		int	FanSpeed;
		int	AirTemp;
        int OutTemp;
        int	SystemDisable;
        int UTAMode;
        int FanMode;
		unsigned int CumulativeAlarm;
        int ValvePerc;	
        int ValvePriority;
        int Valve_Pid_Cooling;
        unsigned int On_Priority_Mode;
        unsigned int Valve_OnDemand_Min_Perc_Val;
        unsigned int Max_Lim_ON_Demand_Total_Score;
        unsigned int TimeOnDemandValveSwitch;
        unsigned int Fan_Power;
        unsigned int Total_Fan_Power;
        unsigned int Humidity;
                 int HumidityMiddle;
        unsigned int Fan_M3h;
        unsigned int Heater_Abil;
        unsigned int HeaterPwr;        
         //unsigned int Total_Fan_M3h;
        unsigned int Model_FW_Version;

        unsigned int AirPowerOutput;
        unsigned int AirPowerBTU;
        unsigned int TotalAirPowerOutput;
        unsigned int TotalAirPowerBTU;
        
        //Argo switch            
   		int TestAll_SuperHeat_SP_Cool;
   		int TestAll_SuperHeat_SP_Heat;
    	int	TestAll_TempBatt_SP_Cool;
    	int	TestAll_TempBatt_SP_Heat;
    	int	TestAll_MaxOpValve_Cool;
    	int	TestAll_MaxOpValve_Heat;
        int TestAll_Abil;
    	int	TestAll_MinOpValve_Cool;
    	int	TestAll_MinOpValve_Heat;
    	int	TestAll_MinOpValve_Frigo;
    	int	TestAll_MinOpValve_Freezer;
        unsigned int TestAll_SuperHeat_Heat_pGain;    		// proportional gain
        unsigned int TestAll_SuperHeat_Heat_iGain;			// integral gain
        unsigned int TestAll_SuperHeat_Heat_dGain;     		// derivative gain
        unsigned int TestAll_SuperHeat_Heat_Time_Exec_PID;        
        unsigned int TestAll_SuperHeat_Cool_pGain;    		// proportional gain
        unsigned int TestAll_SuperHeat_Cool_iGain;			// integral gain
        unsigned int TestAll_SuperHeat_Cool_dGain;     		// derivative gain
        unsigned int TestAll_SuperHeat_Cool_Time_Exec_PID;        
        //Argo switch                  
        
        unsigned int FrozenMode;

        //Argo switch FROZEN
   		int TestAll_SuperHeat_SP_Frigo;
   		int TestAll_SuperHeat_SP_Freezer;
    	int	TestAll_TempBatt_SP_Frigo;
    	int	TestAll_TempBatt_SP_Freezer;
    	int	TestAll_MaxOpValve_Frigo;
    	int	TestAll_MaxOpValve_Freezer;
        int TestAll_Frozen_Abil;
        //Argo switch FROZEN                  
        
        int Defrosting_Abil;
        int Defrosting_Step_Time;
        int Defrosting_Time;
        int Defrosting_Temp_Set;
        int Dripping_Time;
        unsigned int Valvola_Frozen;
        unsigned int Maintenance_Flag;
        unsigned int Maintenance_Set_Time;
        unsigned int Maintenance_Remaning_Time;

        int SuperHeat_Cool_Min_SetPoint;
        int SuperHeat_Cool_Max_Value_Correct;
        int SuperHeat_Heat_Min_SetPoint;
        int SuperHeat_Heat_Max_SetPoint;

        int    Dehumi_Abil;
        int    Set_Humi;
        int    Set_Delta_Temp_Min_Dehumi;
        int    Set_Fan_Speed_Dehumi;
        int    Set_Press_Dehumi;
        int    Set_Temp_Air_Out;
        int    Set_Temp_Air_Out_Dehumi;
        
	   struct 
	   {	
		   int		Pwr;
		   int		SetPoint;
		   int		FanMode;
		   int		SetPoint_F;
           int      UTA_Mode;
           int      Heater_Pwr;

	   } DiagWrite;				 
				 
	   struct 
	   {	
		   int		Pwr;
		   int		SetPoint;
		   int		FanMode;
		   int		SetPoint_F;

	   } DiagRead;				 

	   struct 
	   {	
		unsigned int    Status;
        unsigned int    Type;
		unsigned 		Enable:1;				// se è abilitato
		unsigned		OffLine:1;				// se non ha risposto
		unsigned		OnLine:1;
        unsigned        Alarm:1;
	   } ModDev;				       

		struct 
		{	unsigned int	Number;	
            unsigned int    Abil;
                     int	DeltaVel;
                     int    AutoVel;
		} Extractor;
        
 	    unsigned int	EcoModeEnable;			// Abilitazione Limitazione Potenza in Eco Mode
        unsigned int    OnlyFan;
        unsigned int    Set_CMD_Disable;        
		unsigned		EEV_End:1;				// flag di fine test EEV
		unsigned 		Enable:1;				// se è abilitato
		unsigned		OffLine:1;				// se non ha risposto
		unsigned		OnLine:1;
		//unsigned		CumulativeAlarm:1;		
	} TypRoom;
    
    
	typedef struct
	{			      
	   struct 
	   {	
				 int	Compressor_Output;
                 int    Max_Compressor_Output;
				 int	Compressor_Suction;
				 int	Condenser;
				 int	Sea_Water;
				 int	Liquid;
				 int	Gas;

                struct 
                {	
                          int	Compressor_Output;
                          int	Compressor_Suction;
                          int	Condenser;
                          int	Sea_Water;
                          int	Liquid;
                } PreviusVal;

                union ByteBit
                {
                     unsigned int Compact;        
                     struct 
                     {
                         unsigned	Compressor_Output:1;
                         unsigned	Compressor_Suction:1;
                         unsigned	Condenser:1;
                         unsigned	Sea_Water:1;
                         unsigned	Liquid:1;
                         unsigned	Compressor_Output2:1;
                     } Bit;        
                }; 

                union ByteBit Error;
                struct 
                {	
                          unsigned int	Compressor_Output;
                          unsigned int	Compressor_Suction;
                          unsigned int	Condenser;
                          unsigned int	Sea_Water;
                          unsigned int	Liquid;
                } CntError;                

                struct 
                {	
                          int	Compressor_Output;
                          int	Compressor_Suction;
                          int	Condenser;
                          int	Sea_Water;
                          int	Liquid;
                } Freeze;                
       } Temperature;
       
	   struct 
	   {	
        unsigned int	Gas;            // Pressione in mBar tubo grande (Gas)
		unsigned int	Liquid;			// Pressione in mBar tubo piccolo (Liquido)
		unsigned int	LiquidCond;     // Pressione in mBar Liquido Condensatore
                
                struct 
                {	
                 unsigned int	Gas;            // Pressione in mBar tubo grande (Gas)
                 unsigned int	Liquid;			// Pressione in mBar tubo piccolo (Liquido)
                 unsigned int	LiquidCond;     // Pressione in mBar Liquido Condensatore
                } PreviusVal;

                union ByteBit2
                {
                     unsigned int Compact;        
                     struct 
                     {
                         unsigned	Gas:1;
                         unsigned	Liquid:1;
                         unsigned	LiquidCond:1;
                     } Bit;        
                }; 

                union ByteBit2 Error;
/*                
                struct 
                {	
                 unsigned int	Gas;            // Pressione in mBar tubo grande (Gas)
                 unsigned int	Liquid;			// Pressione in mBar tubo piccolo (Liquido)
                 unsigned int	LiquidCond;     // Pressione in mBar Liquido Condensatore
                } Error;
*/
                struct 
                {	
                 unsigned int	Gas;            // Pressione in mBar tubo grande (Gas)
                 unsigned int	Liquid;			// Pressione in mBar tubo piccolo (Liquido)
                 unsigned int	LiquidCond;     // Pressione in mBar Liquido Condensatore
                } CntError;

                struct 
                {	
                 unsigned int	Gas;            // Pressione in mBar tubo grande (Gas)
                 unsigned int	Liquid;			// Pressione in mBar tubo piccolo (Liquido)
                 unsigned int	LiquidCond;     // Pressione in mBar Liquido Condensatore
                } Freeze;                
       } Pressure;

                 int	Ric_Temp;					// Temperatura richiesta al compressore
		unsigned int	Ric_Pressione;				// pressione richiesta al compressore
		unsigned int	HoldErr;					// valore di pressione nel calcolo precedente
				 int	Out_Inverter;
				 int	Request_Out_Inverter;		// richiesta da parte del programma di gestione dell'inverter
				 int	Inverter_Max_Power_Out;		// massima potenza disponibile per l'inverter
		unsigned int	TotalPowerBoxAbsorption;
        unsigned int	BoxAbsorption;
        unsigned int	MaxInverterAbsorption;
        unsigned int    Split_Total_Fan_Absorption;
        unsigned int    Split_Total_AirPowerOutput;        
        unsigned int    Split_Total_AirPowerBTU;
        unsigned int    System_Efficency_EER;
		unsigned int	PowerLimit;
				 int	Mode;						// modalità del box motore
													// Mode 0 => Off
													// Mode 1 => Shutting down
													// Mode 2 => Freddo
													// Mode 3 => Go Freddo
													// Mode 4 => Caldo
													// Mode 5 => Go Caldo
				 char	StatoCompressore;			// stato della macchina a stati di avvio della macchina
				 char	LastModeCompressor;			// Ultima modalità del compressore fino alla fase di ShutDown
				 char	IdMasterSplit;				// id dello split più gravoso
                 char   HeatingEngineStart;         // Flag per gestione fase di preriscaldamento motore (Braking))
                 int    SplitPowerOnCnt;            // Contatore numero Split con Power = ON
                 int    CompressorReqOn;            // Contatore numero Split che chiedono funzionamento
                 int    Frigo_Abil;
                 int    Freezer_Abil;
                 int    LoopWater_Abil;
                 int    Dehumi_Abil;
                 int    Abil_Defrosting;
                 int    SplitFrigo_On;
                 int    SplitFreezer_On;
                 char   CompressorIsMoreHot;        // Flag per gestire il preriscaldamento motore in fase OFF
				 int	Acceleration_Inverter;		// Accelerazione Inverter (DEBUG)
				 int	Acceleration_GasPressure;	// Accelerazione Pressione GAS (DEBUG)				 
		unsigned		InverterRaffrescamento:1;	// Modalità di funzionamento inverter in raffrescamento
		unsigned		InverterRiscaldamento:1;	// Modalità di funzionamento inverter in riscaldamento
		unsigned		GoRiscaldamento:1;			// Flag di richiesta avvio riscaldamento
		unsigned		GoRaffrescamento:1;			// Flag di richiesta avvio raffrescamento
		unsigned		GoShutDwComp:1;				// Flag di richiesta spegnimento
        unsigned        StopPump:1;
		unsigned		CumErr:1;					// Flag cumulativo di errore
		unsigned		PumpON_FluxTestFlag:1;		// Flag per gestione ON pompa per test flusso
		unsigned		PumpON_EEVTestFlag:1;		// Flag per gestione ON pompa per test EEV
		unsigned		DoubleCompressorOn:1;		// Flag per gestione presenza compressore parallelo		
		unsigned		ultimoerrore:1;					
		unsigned		SelectCompressor:1;			// Selezione compressore di Work
		unsigned		CoolingMode:1;
		unsigned		HeatingMode:1;
		unsigned		EngineIsOn:1;		
		unsigned int	FreqPompa_Acqua;			// Frequenza inverter pompa
		unsigned int	FreqPompa_Acqua_Perc;		// Frequenza inverter pompa in %		
		unsigned int	Split_Flags_Enable_1_16;
		unsigned int	Split_Flags_Enable_17_32;
		unsigned int	Split_Flags_Offline_1_16;
		unsigned int	Split_Flags_Offline_17_32;
		unsigned int	Split_Flags_Online_1_16;
		unsigned int	Split_Flags_Online_17_32;
		unsigned int	Split_Flags_Alarm_1_16;
		unsigned int	Split_Flags_Alarm_17_32;
        unsigned int	Split_Flags_EnableAndOffline_1_16;
        unsigned int	Split_Flags_EnableAndOffline_17_32;        
        
        unsigned int    SwitchPumpTime;
        unsigned int    SwitchPumpSelection;
		unsigned int    BoardAddress;
		unsigned char   DisplayDigit;     
        unsigned char   DisplayDigitDP;    
		unsigned char   DisplayDigitBlink;          
		unsigned char   ReverseDisplayDigit;       
        unsigned char   AllValveOpen100p;
		unsigned char   NoPayCastigationActivation;       
		unsigned char   EraseAllEEPROM;    
        unsigned char   AbilSuperHeatBox;
		unsigned char   ServiceMode;       
        
        unsigned int    Manual_Pump_Abil;
        unsigned int    Manual_Pump_Freq;
        unsigned int    DefMode;
        
        unsigned int    AddressGasLeaks;
		
        unsigned int	Split_ModDev_Flags_Enable_1_16;
		unsigned int	Split_ModDev_Flags_Enable_17_32;
		unsigned int	Split_ModDev_Flags_Offline_1_16;
		unsigned int	Split_ModDev_Flags_Offline_17_32;
		unsigned int	Split_ModDev_Flags_Online_1_16;
		unsigned int	Split_ModDev_Flags_Online_17_32;
		unsigned int	Split_ModDev_Flags_Alarm_1_16;
		unsigned int	Split_ModDev_Flags_Alarm_17_32;
        unsigned int	Split_ModDev_Flags_EnableAndOffline_1_16;
        unsigned int	Split_ModDev_Flags_EnableAndOffline_17_32;        
        
        unsigned int    AutoresetMagicNumber;        

        unsigned int    MaxErrorPersComp;
        unsigned int    MaxErrorPersPump;

        int	TempCompressorHi;
        int	TempRestartCompressorHi;
        
		struct 
		{	
            unsigned	CompressorHi:1;				// Flag di richiesta avvio procedura di recovery per alta temp
			unsigned	CompressorLo:1;				// Flag di richiesta avvio procedura di recovery per bassa temp
			unsigned	CondensatoreHi:1;			// Flag del ciclo di sbrinamento, quado ghiaccia il condensatore
			unsigned	CondensatoreLo:1;			// Flag di richiesta avvio procedura di recovery per alta temperatura condensatore
			unsigned	Gas_Recovery:1;				// Flag del ciclo di recupero del gas
			unsigned	Pressure_Lo:1;				// Flag di avvio procedura recovery per bassa pressione
            unsigned    SuperHeat:1;                // Flag x Errore superheat temporaneo
			unsigned	Pressure_Hi:1;				// Flag di avvio procedura recovery per alta pressione
			unsigned	Recharge_Oil:1;				// flag di richiesta ciclo di recupero dell'olio
			unsigned	GlobalStatusFlag:1;			// Flag x Cumulativo Errori 		
			unsigned	TouchStatusFlag:1;			// Flag x Cumulativo Errori solo visualizzazione su Touch 		
            unsigned int	Active_Error;           // Flag x Cumulativo Errori EngineBox, Split, UTA Box, UTA Split,	
            unsigned int	MasterRestart;		
            unsigned int	Defrosting;				// Flag x segnalazione motore in tato di defrosting,	
            unsigned	CondenserFouled:1;  
            unsigned    SPTemperatureFault:1;
            unsigned    TemperatureProbe:1;
            unsigned    Pressure_Probe:1;
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
#if (K_AbilMODBCAN==1)              
            unsigned int	ModCntComErr_U4;			
            unsigned int	ModTimeOutComErr_U4;		
            unsigned int	ModCRC_ComErr_U4;
#endif //#if (K_AbilMODBCAN==1)                         
		} ComError;             
        
		struct
		{	
            unsigned char CompressorHi;
			unsigned char CompressorLo;
			unsigned char CondensatoreHi;
			unsigned char CondensatoreLo;
			unsigned char GasRecovery;
			unsigned char FluxTest;
		} CntError;

		struct 
		{	
            unsigned	CompressorHi:1;				// Flag di richiesta avvio procedura di recovery per alta temp
			unsigned	CompressorLo:1;				// Flag di richiesta avvio procedura di recovery per bassa temp
			unsigned	CondensatoreHi:1;			// Flag del ciclo di sbrinamento, quado ghiaccia il condensatore
			unsigned	CondensatoreLo:1;			// Flag di richiesta avvio procedura di recovery per alta temperatura condensatore
			unsigned	Gas_Recovery:1;				// Flad del ciclo di recupero del gas
			unsigned	FloodSensor:1;				// stato dell'ingressio della sonda di allagamento
			unsigned	ThermicComp:1;				// stato della termica sul compressore
			unsigned	SeaWaterLo:1;				// Temp acqua di mare troppo fredda
			unsigned	SeaWaterHi:1;				// Temp acqua di mare troppo calda
			unsigned	CaricaRefrig:1;				// Flag che indica la mancanza di gas
			unsigned	WaterSeaFlux:1;				// Flag dell'allarme di flusso del condensatore
			unsigned	EEV_Open:1;					// Flag cumulativo di una EEV bloccata aperta nell'impianto
			unsigned	EEV_Close:1;				// Flag cumulativo di una EEV bloccata chiusa nell'impianto
			unsigned	BatteriaGhiacciata:1;		// Flag cumulativo di una batteria ghiacciata nell'impianto
			unsigned	Fan_Speed:1;				// Flag cumulativo di un ventilatore che non gira
			unsigned	Fan_OverLoad:1;				// Flag cumulativo di un ventilatore in sovraccarico
            unsigned    FireAlarm:1;                // Flag cumulativo di un allarme fuoco nella barca
			unsigned	Acceleration_Error:1;		// Flag x Errore di compresisone			
			unsigned	GlobalStatusFlag:1;			// Flag x Cumulativo Errori Persistenti	
			unsigned	LowOilLevel:1;			
			unsigned	CompInverterFault:1;		// Flag x Errore Inverter Compressore
			unsigned	PumpInverterFault:1;		// Flag x Errore Inverter Pompa	
			unsigned	CompMOff:1;					// Flag x Errore MOFF anomalo Compressore
			unsigned	PumpMOff:1;					// Flag x Errore MOFF anomalo pompa
			unsigned	Pump1Fault:1;				// Flag x Errore pompa 1 guasta
			unsigned	Pump2Fault:1;				// Flag x Errore pompa 2 guasta
            unsigned    SPTemperatureFault:1;       // Flag x Errore frigo o freezer non in temepratura
            unsigned    CoolWarm_Valve:1;           // Flag x Errore valvola di inversione
            unsigned    SuperHeat:1;                // Flag x Errore superheat
		} PersErr;

		struct
		{	
            unsigned	FluxReq:1;					// Flag che indica lo svolgimento del test o la richiesta
			unsigned	EEV_Req:1;					// Flag che indica lo svolgimento del test o la richiesta
			unsigned	EEV_Go:1;					// Flag che indica agli split di procedere con il test delle valvole
			unsigned	EEV_End:1;					// Flag cumulativo che indica che tutti gli split hanno terminato il test delle EEV
			//unsigned int PressPreRiscCondFluxTest;	// Pressione liquido (Piccolo) all'inizio del test di flusso
		} Test;

		struct
		{	
            unsigned	CompressorHi:1;
			unsigned	CompressorLo:1;
			unsigned	ThermicComp:1;
			unsigned	FloodSensor:1;
			unsigned	Flow:1;
			unsigned	ClearErrorReq:1;
            unsigned	ClearComErrorReq:1;
			unsigned	Antincendio:1;			
			unsigned	LowOilLevel:1;
		} In;

		struct
		{	
            unsigned	Free1:1;					// Uscita libera
			unsigned	Valvola_Caldo:1;			// Valvola caldo/freddo
			unsigned	Pompa_Acqua:1;				// Pompa di ricircolo acqua
			unsigned	Pompa_Acqua2:1;				// Pompa2 di ricircolo acqua				//;ReleAux1:1;					// Aux 1
			unsigned	PersErr:1;					// Allarme cumulativo Errori Persistenti	//ReleAux2:1;					// Aux 2
			unsigned	RechargeOil:1;				// uscita pompa olio
		} Out;
		
		struct
		{
			unsigned	TrigPump1:1;					// Uscita libera
			unsigned	TrigPump2:1;			// Valvola caldo/freddo
			unsigned	Pompa_Acqua:1;				// Pompa di ricircolo acqua
			unsigned	Pompa_Acqua2:1;				// Pompa2 di ricircolo acqua				//;ReleAux1:1;					// Aux 1
		} VirtualOut;		
        
        struct
		{
            int         Value;
			int         HeatingSP;                  // 
			int         HeatingCalc;                // 
            int         ControlSuperHeat;
            int         CoolingCalc;            
            int         Correct_Cooling;
		} SuperHeat;	

        struct
		{
            int         Value;            
			int         HeatingSP;                  // 
			int         HeatingCalc;                // 
		} PressSeaWater;	

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
            
            int         Defrosting_Abil;
            int         Defrosting_Step_Time;
            int         Defrosting_Time;
            int         Defrosting_Temp_Set;
            int         Dripping_Time; 

            int         MinOpValve_Cool;
            int         MinOpValve_Heat;
            int         MinOpValve_Frigo;
            int         MinOpValve_Freezer;    

            int         SuperHeat_Custom_Par_Heat_PID;
            int         SuperHeat_Custom_Par_Cool_PID;
            
            int         SuperHeat_Cool_Correct;
            int         SuperHeat_Heat_Correct;
            
            int         Split_Probe_Error;
            int         Master_Probe_Error;
            int         Compression_Error;
            int         Programming_Mode;
		} TestAllAbil;	
        
        
		unsigned int	Errori1;					// Errori NON persistenti
        unsigned int	Errori2;					// Errori NON persistenti
		unsigned int	ErroriPers1;				// Errori persistenti 1
		unsigned int	ErroriPers2;				// Errori persistenti 2
		unsigned int	SystemCycleTime;			// Tempo di esecuzione ciclo elaborazione in mS
		unsigned int	LastCompressorPowerStatus;       // Ultimo stato On/Off compressore  
				 int	BridgePowerLimit;
        unsigned int    Compressor2Master;
        unsigned int    SelectWorkingPump;
        unsigned int    DisableModComErrorCounter;
#if (K_AbilMODBCAN==1)          
        unsigned int    DisableModComErrorCounter_U4;
#endif //#if (K_AbilMODBCAN==1)                   
        unsigned int    ProgrammingModeEnable;
        
		
        unsigned int    HW_Version;					// REG_TOUCH_MASTER_HW_VERSION
		unsigned int	FW_Version_HW;				// REG_TOUCH_MASTER_FW_VERSION_HW                
		unsigned int	FW_Version_FW;				// REG_TOUCH_MASTER_FW_VERSION_FW
		unsigned int	FW_Revision_FW;				// REG_TOUCH_MASTER_FW_REVISION_FW			
	} TypEngineBox;


	typedef struct
	{
				 int 	Address;
                 int 	SubAddress;
				 int	CmdAddress;
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
                 int    EvapTempValve;                 
		unsigned int	CycleTime;
        unsigned int    Sub_Slave_Quantity;
				 int	Superheat;
				 int	SuperheatRiscaldamento;	
                 int    SetP_SuperHeat;
                 int    UTA_Abil;       
        unsigned int    CumulativeAlarm;
		unsigned int	HW_Version;				
		unsigned int	FW_Version_HW;			              
        unsigned int	FW_Version_FW;			
        unsigned int	FW_Revision_FW;	
        unsigned int    Model_FW_Version;
        unsigned int    Sub_Slave_Flags_Enable_1_16;
        unsigned int    Sub_Slave_Flags_Online_1_16;
        unsigned int    Sub_Slave_Flags_Offline_1_16;
        unsigned int    Sub_Slave_Flags_Alarm_1_16; 		
        unsigned int    OnlyFan;
        unsigned int    Set_CMD_Disable;
				 int    TempAmbMid;
                 int    TempAmbReal;
                 int    FunctionMode;  
                 int    FrozenMode;
                 
        unsigned int    Fan_Power;
        unsigned int    Fan_Current;
        unsigned int    Supply_Voltage;       
        
        unsigned int    MaxVentilMode;
        unsigned int    Humidity;
                 int    SuperHeat_Pid_Cool_Error;
        unsigned int    Temperature_Probe_Error;
        unsigned int    Heater_Abil;

        unsigned int    MinSetPoint;
        unsigned int    MaxSetPoint;       
        
        unsigned int    Fan_M3h;
        unsigned int    AirPowerOutput;
        unsigned int    AirPowerBTU;
        
        unsigned int    Touch_Page;
        
		unsigned 		Enable:1;				// se è abilitato
//		unsigned		OffLine:1;				// se non ha risposto

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
        
        struct 
        {			
            int			 CMD_SetP;				
            int			 CMD_SetP_F;					
            int			 SetPoint;					
            int			 FanSpeed;				
            int			 UTAMode;
                     int Send;		
                     int CMD_On_Off;
            unsigned int CMD_Fan;
            unsigned int Number;
        } OtherCabin;	        
	} TypDiagUnit;
    
    
	typedef struct
	{
        //int			 SystemDisable;				

        struct 
        {			
            int			 PowerOn;				
            int			 DefMode;					
            int			 SetPoint;					
            int			 FanSpeed;				
            unsigned int TrigInit;		
            unsigned int Power_EcoMode;
            unsigned int EcoModeEnable;
            unsigned int Address;
        } SetRoom;	
        
			struct 
			{			
				int			 PowerOn;				
				int			 DefMode;					
				int			 SetPoint;					
				int			 FanSpeed;				
				unsigned int SetHumi;		
			} SetUta;	

			struct 
			{			
				int			 Split;				
				int			 SubSplit;					
			} Diag;	

        
	} TypSyncro;

/*
	typedef struct
	{			 int	Req_Freq;		// frequenza richiesta da settare
				 int	Max_Req_Freq;	// limite MAX frequenza da settare
				 int	Out_Current;	// corrente in uscita
				 int	Out_Voltage;	// tensione in uscita
				 int	Out_Freq;		// frequenza in uscita
				 int	In_Voltage;		// tensione in ingresso
				 int	In_Power;		// Potenza in ingresso
				 int	Out_Power;		// Potenza in uscita
				 int    Max_Power_Inverter; // Limite potenza inverter proveniente da touch su slave master
				 int	StatusReg;		// registro letto dall'inverter
				 int	AlarmReg;		// registro letto dall'inverter
		struct 
		{	
			unsigned	FailFL:1;		// bit0 - 0: No Out;		1: Out in progress
			unsigned	Fail:1;			// bit1 - 0: Not tripped;	1: Tripped
			unsigned	Alarm:1;		// bit2 - 0: No Alarm;		1: Alarm issued
			unsigned	UnderVoltage:1;	// bit3 - 0: Normal;		1: Under Voltage
			unsigned	MotorSection:1;	// bit4 - 0: Motor THR1;	1: Motor THR2
			unsigned	PID_Control:1;	// bit5 - 0: Permitted;		1: Prohibited
			unsigned	AccDecPatt:1;	// bit6 - 0: Patt AD1;		1: Patt AD2
			unsigned	DC_Bracking:1;	// bit7 - 0: Off;			1: Forced DC Bracking
			unsigned	JogRun:1;		// bit8 - 0: Off;			1: Jog Run
			unsigned	FwRvRun:1;		// bit9 - 0: Forward Run;	1: Reverse Run
			unsigned	RunStop:1;		// bit10 - 0: Stop;			1: Run
			unsigned	CoastStop:1;	// bit11 - 0: ST On;		1: ST Off
			unsigned	EmergencyStp:1;	// bit12 - 0: Off;			1: Active
			unsigned	StandBy_ST:1;	// bit13 - 0: Startup;		1: Standby
			unsigned	StandBy:1;		// bit14 - 0: Startup;		1: Standby
		} Status;

		struct 
		{	
			unsigned	OverCurrent:1;		// bit0 - 0: Normal;		1: Alarm
			unsigned	InverterOverLoad:1;	// bit1 - 0: Normal;		1: Alarm
			unsigned	MotorOverLoad:1;	// bit2 - 0: Normal;		1: Alarm
			unsigned	OverHeat:1;			// bit3 - 0: Normal;		1: Alarm
			unsigned	OverVoltage:1;		// bit4 - 0: Normal;		1: Alarm
			unsigned	UnderVoltage:1;		// bit5 - 0: Normal;		1: Alarm
			unsigned	MainOverLoad:1;		// bit6 - 0: Normal;		1: Alarm
			unsigned	LowCurrent:1;		// bit7 - 0: Normal;		1: Alarm
			unsigned	OverTorque:1;		// bit8 - 0: Normal;		1: Alarm
			unsigned	BrakResOverLoad:1;	// bit9 - 0: Normal;		1: Alarm
			unsigned	CumOpHours:1;		// bit10- 0: Normal;		1: Alarm
			unsigned	OptionCommAlm:1;	// bit11- 0: Normal;		1: Alarm
			unsigned	SerialCommAlm:1;	// bit12- 0: Normal;		1: Alarm
			unsigned	MainVoltageErr:1;	// bit13- 0: Normal;		1: Alarm	Display MOFF flickering
			unsigned	ForceDecelarating:1;// bit14- 0: Normal;		1: Alarm
			unsigned	ForceStop:1;		// bit15- 0: Normal;		1: Alarm

		} Alarm;
		unsigned		OffLine:1;				// se non ha risposto
	} TypInverter;
*/
	
	typedef struct
	{			
//		unsigned		OffLine:1;				// se non ha risposto
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
		} FromEngBox;

	} TypInitSplit;
		

	//---------------------------------------
	// Variabili 
	//---------------------------------------
    
#define VariabiliGlobaliProtocolloComunicazione()	\
        int	DigitalInput = 0;                       \
        int StatoRele = 0;                          \
        int StatoIngressi = 0;                      \
        volatile TypTimer TimeOutPktRx;             \
        TypRoom Room[Max_Room];                     \
        TypEngineBox EngineBox[Max_Engine_Box];     \
        TypDiagUnit DiagnosticSplit;                \
        TypInitSplit Reinit;                        \
        TypSyncro Syncro;        


#define IncludeVariabiliGlobaliProtocolloComunicazione()    \
        extern int DigitalInput;                            \
        extern int StatoRele;                               \
        extern int StatoIngressi;                           \
        extern volatile TypTimer TimeOutPktRx;              \
        extern TypRoom Room[Max_Room];                      \
        extern TypEngineBox EngineBox[Max_Engine_Box];      \
        extern TypDiagUnit DiagnosticSplit;                 \
        extern TypInitSplit Reinit;                         \
        extern TypSyncro Syncro;            
    
    
    IncludeVariabiliGlobaliProtocolloComunicazione();    

/*
	// Timer 
	volatile TypTimer TimeOutPktRx;				// definisco il Timer per il TimeOut di Rx

	// Stanze
	TypRoom Room[Max_Room];					// definisco le variabili per le stanze

	// Box Motore
	TypEngineBox EngineBox[Max_Engine_Box];

	// Split in diagnostica
	TypDiagUnit DiagnosticSplit;		   

	// Inverter Compressore
	//TypInverter Comp_Inverter[K_Max_Inverter];	
	
	TypInitSplit Reinit;

    //Sincronizzazione zone alternative a bridge
	TypSyncro Syncro;
*/

	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	unsigned char WaitRx(int TimeOut_ms);
	unsigned char Send_WaitRx(char Address, char Comando, int Registro, int Data, int TimeOut_ms, char Retry);
	unsigned char Send_WriteReg(char Address, int Registro, int Data, int TimeOut_ms, char Retry);
	unsigned char SendBrc_WriteReg(char Cmd, int Registro, int Data, int TimeOut_ms, char Retry);

	unsigned FuncReadReg (char Address, int REG, int * StoreReg, unsigned OffLineFlag);
	unsigned FuncWriteReg (char Address, int REG, int SendData, unsigned OffLineFlag);
	unsigned FuncWriteBrcReg (char Comando, int REG, int SendData);

	void CheckModuli(void);
	void ResumeOffLine(char * Cnt);
	void CheckHotPlugSlave(char * Cnt);		

    void RefreshInPutRoom(TypRoom * DataStore, char Address, int Sched);
	//void RefreshOutPutRoom(void);
    void RefreshOutPutRoom(char Address);
	void RefreshInPutEngineBox(TypEngineBox * DataStore, char Address);
	void RefreshAllInPut(void);
	void RefreshAllModeRoom(void);
	
	void RefreshOutPutEngineBox(void);	//TypEngineBox * DataStore, char Address);
	void RefreshAllOutPut(void);
	
	void ReadDiagnosticSplitAddr(int addr);		// #v16
	void RefreshDiagnosticSplit(void);	
	void SendDiagnosticSplitData(int addr);
	void SendDiagnosticInverterData(int addr, int numInverter);
    void ReadDiagnosticSubSplitAddr(int addr);	
	void RefreshDiagnosticSubSplit(void);	  
	void ReadDiagnosticInverterAddr(int addr);

	// slave section
	int GetData (int Reg);
	char PutData (int Reg, int Value);
    
	//void Process_Cmd(TypBuffer * LastCmd);    
    
    void Change_Protocol_BUSM(int ProtocolIndex);
	void ReadDataFromSlaveMaster(void);
	void RefreshInPutMasterRoom(TypInitSplit * Reinit, char Address);
	void MasSlaveSyncroData(int Address);
	void Syncro2Value(int Address, int RegRd, int RegWr, int RegBridge, int * App1, int * OldApp1, int * App2, int * OldApp2, int * NewValue);
	void Syncro2Diag(int MasAddress, int SplitAddress,int RegRd, int RegWr, int RegSyncRD, int RegSyncWR, int * App1, int * OldApp1, int * App2, int * OldApp2, int * NewValue);
	void CheckAllRangeValue(void);	
	void CheckRangeValue(int * Var, int minVal, int maxVal, int defVal);

	void RefreshOutPutMasterRoom(void);
	void SendWorkingSplitData(unsigned int index);	
	void MountImageSlaveFlags(void);
    void OtherSplitSend(void);
    void Bridge2SplitSend(void);
    void SplitReadyCommand(void);
    void Valve_On_Demand(void);
    void RefreshAllSlaveModDev(void);
    void MountImageSlaveModDevFlags(void);    

#endif
