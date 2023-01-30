//----------------------------------------------------------------------------------
//	Progect name:	ProtocolloModBus.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			Emanuele
//	Date:			18/06/2011
//	Description:	Header delle funzioni che gestiscono la comunicazione ModBus con il sistema
//----------------------------------------------------------------------------------

#ifndef _PROTOCOLLO_MODBUS_H_
	#define _PROTOCOLLO_MODBUS_H_

#include "ProtocolloComunicazione.h"
#include "FWSelection.h"
	//----------------------------------------------------------------------------------
	//	Define
	//----------------------------------------------------------------------------------
	//#define	Add_TouchRoom1						0x01	// Indirizzo Touch
	#define	k_Add_Touch   						0x01	// Indirizzo Touch
	#define	Add_Comp_Inverter1					0x02	// Indirizzo Inverter Compressore primario
	#define	Add_Comp_Inverter2					0x03	// Indirizzo Inverter Compressore secondario
	#define	Add_Pump_Inverter1					0x04	// Indirizzo Inverter Pompa 
	#define	Add_Pump_Inverter2					0x05	// Indirizzo Inverter Pompa Loop 

	#define k_MAX_Cnt_Err_Touch					10		// N. Max tentativi di comunicazione con Touch Modbus prima di dichiararlo guasto
	#define k_MAX_Cnt_Err_Inverter				6		// Num. MAX di errori di comunicazione (TMT) consecutivi prima di segnalare Inverter Offline (flag PermanentOffline==1)


	//#define k_taglia_inverter					400		// Taglia potenza inverter in decine di Watt (400 -> 4Kw)
	//#define k_taglia_inverter					750		// Taglia potenza inverter in decine di Watt (750 -> 7.5Kw)
	//DEBUG ATTENZIONE: DINAMICO IN BASE A IMPIANTO IL DATO SOPRA DA SISTEMARE!!

	#define K_Max_Inverter						4		// Massimo numero di inverter gestibili (compressori parallelo + pompa + comp. UTA))

	#define K_MotorType_DA270A2F20L				1		// Identificativo per Motore mod.DA270A2F20L
	#define K_MotorType_DA550A3F10M				2		// Identificativo per Motore mod.DA550A3F10M
	#define K_MotorType_DA422A3F26M				3		// Identificativo per Motore mod.DA422A3F26M
	#define K_MotorType_DA422A3F27M				4		// Identificativo per Motore mod.DA422A3F27M
	#define K_MotorType_DA790A4F11UC1			5       // Identificativo per Motore mod.DA790A4F11UC1
	#define K_MotorType_DA330A2F20M             6       // Identificativo per Motore mod.DA330A2F20M
	#define K_MotorType_DA550A3F11MD			7		// Identificativo per Motore mod.DA550A3F11MD
	#define K_MotorType_DA640A3F20MAD           8       // Identificativo per Motore mod.DA640A3F20MAD
	#define K_MotorType_DA150A1F21N             9       // Identificativo per Motore mod.DADA150A1F21N
	#define K_MotorType_DA130A1F25F3            10      // Identificativo per Motore mod.DA130A1F25F3   //Uguale a DA150A1F21N


	#define K_PumpType_DEFAULT      			0		// Identificativo per Motore pompa tipo Default
    #define K_PumpType_MPH_441_FGACE5			1		// Identificativo per Motore pompa tipo MPH_441_FGACE5 330 L/min
	#define K_PumpType_MPH_453_FGACE5			2		// Identificativo per Motore pompa tipo MPH_453_FGACE5 500 L/min

	#define K_No_Braking_Temp					3500

	#define K_VoltOutMaxInverter				238				// Tensione MAX in uscita al motore

	#define K_TimeHwReset						60		// Tempo di attesa spegnimento Inverter (sec))


	#define MaxModChkRetries					3		// check
	#define MaxModChkWait						100//400		// check
	#define MaxModReadRetries					3//2		// Read
	#define	MaxModReadWait						100//50		// Read
	#define MaxModWriteRetries					3//2		// Write
	#define	MaxModWriteWait						100//50		// Write
	

	// comandi MODBUS
	#define	ReadModCoil							1		// Read Coils
	#define	ReadModCmd							3		// Read Holding Registers
	#define	ReadModInput						4		// Read Input Register
	#define	WriteModCoil						5		// Write Single Coil
	#define	WriteModCmd							6		// Write Single Register
	#define	WriteModListCmd						16		// Write Multiple Registers
/*
	// comandi
	#define	ReadModCmd							0x03
	#define	WriteModCmd							0x06
	#define	WriteModListCmd						0x10
*/

	// registri touch	(hanno un offset di uno, il tag 1 del touch ha indirizzo 00)
	#define	REG_TOUCH_PAGE						0x0001	//	Global
	#define	REG_TOUCH_PWR						0x0002	// registri di sistema del touch
	#define	REG_TOUCH_MODE						0x0003

	#define	REG_TOUCH_ROOM_TEMP					0x0010	//	1-w
	#define	REG_TOUCH_POWER_MOT					0x0011	//	1-w
	#define	REG_TOUCH_SP_ROOM					0x0012	//	1-r
	#define	REG_TOUCH_FAN_SPEED					0x0013	//	1-r
	#define	REG_TOUCH_FUNCTION_MODE				0x0014	//	1-r

	#define	REG_TOUCH_NET_VOLTAGE				0x0020	//	2-w
	#define	REG_TOUCH_SERVICE_BATTERY_VOLTAGE	0x0021	//	2-w
	#define	REG_TOUCH_INVERTER_POWER_OUT		0x0022	//	2-w
	#define	REG_TOUCH_MAX_PWR_LIMIT				0x0023	//	2-r
	#define	REG_TOUCH_POWER_ON_GENERATOR_LIMIT	0x0024	//	2-r
	#define	REG_TOUCH_POWER_SOURCE				0x0025	//	2-r

	#define	REG_TOUCH_DIMMER_1_A				0x0030	//	3-w
	#define	REG_TOUCH_DIMMER_1_B				0x0031	//	3-w
	#define	REG_TOUCH_DIMMER_2_A				0x0032	//	3-w
	#define	REG_TOUCH_DIMMER_2_B				0x0033	//	3-w
	#define	REG_TOUCH_DIMMER_3_A				0x0034	//	3-w
	#define	REG_TOUCH_DIMMER_3_B				0x0035	//	3-w

	#define	REG_TOUCH_INFRA_RED_MODE			0x0040	//	4-r
	#define	REG_TOUCH_PRESET_INFRA_RED			0x0041	//	4-r

	#define	REG_TOUCH_MATRICOLA_LO				0x0050	//	5-w
	#define	REG_TOUCH_MATRICOLA_HI				0x0051	//	5-w
	#define	REG_TOUCH_MASTER_FW_VERSION			0x0052	//	5-w
	#define	REG_TOUCH_SYS_ERROR_1				0x0053	//	5-w
	#define	REG_TOUCH_SYS_ERROR_2				0x0054	//	5-w

/*
	#define	REG_TOUCH_MANDATA_COMPRESSORE		0x0060	//	6-w
	#define	REG_TOUCH_RITORNO_COMPRESSORE		0x0061	//	6-w
	#define	REG_TOUCH_Temp_Condenser			0x0062	//	6-w
	#define	REG_TOUCH_SEA_WATER_IN_TEMP			0x0063	//	6-w
	#define	REG_TOUCH_SEA_WATER_OUT_TEMP		0x0064	//	6-w
	#define REG_TOUCH_PRESSIONE_P				0x0065	//	6-w
	#define REG_TOUCH_PRESSIONE_G				0x0066	//	6-w
	#define	REG_TOUCH_TEMP_RICHIESTA			0x0067	//	6-w
	#define	REG_TOUCH_PRESSIONE_RICHIESTA		0x0068	//	6-w
	#define	REG_TOUCH_STATO_COMPRESSORE			0x0069	//	6-w
*/
	#define	REG_TOUCH_HOURS_OF_LIFE				0x004F	//	6-w		// v8.4.16
	#define	REG_TOUCH_MANDATA_COMPRESSORE		0x0050	//	6-w
	#define	REG_TOUCH_RITORNO_COMPRESSORE		0x0051	//	6-w
	#define	REG_TOUCH_TEMP_CONDENSATORE			0x0052	//	6-w
	#define	REG_TOUCH_SEA_WATER_IN_TEMP			0x0053	//	6-w
	#define REG_TOUCH_PRESSIONE_P				0x0054	//	6-w
	#define REG_TOUCH_PRESSIONE_G				0x0055	//	6-w
	#define REG_TOUCH_COMPRESSOR_SPEED			0x0056	//	6-w
	#define	REG_TOUCH_TEMP_RICHIESTA			0x0057	//	6-w
	#define	REG_TOUCH_PRESSIONE_RICHIESTA		0x0058	//	6-w
	#define	REG_TOUCH_STATO_COMPRESSORE			0x0059	//	6-w
	#define	REG_TOUCH_ENGINE_BOX_MODE			0x005A	//	6-w
	#define	REG_TOUCH_MASTER_SPLIT				0x005B	//	6-w
	//#define	REG_TOUCH_CLEAR_ERROR				0x005C	//	6-r
	#define	REG_TOUCH_TEMP_P					0x005D	//	6-w
	#define	REG_TOUCH_TEMP_G					0x005E	//	6-w
	#define	REG_TOUCH_SYSTEM_CYCLE_TIME			0x005F	//	6-w		// Dec.95
    #define REG_TOUCH_ENGBOX_PRESS_LIQUID_COND  261



	#define	REG_TOUCH_PICCOLO_SPLIT_1			0x0070	//	7-w
	#define	REG_TOUCH_GRANDE_SPLIT_1			0x0071	//	7-w
	#define	REG_TOUCH_ARIA_INGRESSO_SPLIT_1		0x0072	//	7-w
	#define	REG_TOUCH_ARIA_USCITA_SPLIT_1		0x0073	//	7-w
	#define	REG_TOUCH_VENTILATORE_SPLIT_1		0x0074	//	7-w
	#define	REG_TOUCH_STATO_VALVOLA_SPLIT_1		0x0075	//	7-w
	#define	REG_TOUCH_TEMP_EVAP_SPLIT_1			0x0076	//	7-w
	#define	REG_TOUCH_PRESSIONE_G_SPLIT_1		0x0077	//	7-w
	#define	REG_TOUCH_RICH_PRESSIONE_SPLIT_1	0x0078	//	7-w

//			0000000001111111111222222222233
//			1234567890123456789012345678901
	// registri Inverter
	#define REG_INVERTER_FREQUENCY_SET			0xFA01	// W - Scrive la frequenza richiesta
	#define	REG_INVERTER_COMMAND_SET			0xFA00	// W - scrive i comandi principali - RUN/STOP etc...
	#define	REG_INVERTER_TRIP_CODE_MONITOR		0xFC90	// R - codice di errore attuale dell'inverter
	#define	REG_INVERTER_ALARM					0xFC91	// R - stato dell'inverter
	#define	REG_INVERTER_OUT_FREQ				0xFD00	// RW - frequenza d'uscita (0.01 Hz)
	#define	REG_INVERTER_STATUS					0xFD01	// R - stato dell'inverter
	#define	REG_INVERTER_OUT_CURRENT			0xFD03	// R - Corrente d'uscita (0.01%)        0xFE03
	#define	REG_INVERTER_IN_VOLTAGE				0xFD04	// R - Tensione d'ingresso (0.01%)
	#define	REG_INVERTER_OUT_VOLTAGE			0xFD05	// R - Tensione d'uscita (0.01%)
	#define	REG_INVERTER_INPUT_POWER			0xFD29	// R - Potenza in ingresso in decine di W
	#define	REG_INVERTER_OUTPUT_POWER			0xFD30	// R - Potenza in uscita in decine di W

    #define REG_INVERTER_TIME_ACC				0x0009     // ACC	9 Tempo di accelerazione 1			Unit: 0.1Hz		
    #define REG_INVERTER_TIME_DEC				0x0010     // DEC	10 Tempo di	decelerazione 1			Unit: 0.1Hz	
    #define REG_INVERTER_MAX_FREQUENCY			0x0011     // FH	11	Maximum frequency
    #define REG_INVERTER_UPPER_LIM_FREQ			0x0012     // UL	12	Upper limit frequency
    #define REG_INVERTER_BASE_FREQUENCY			0x0014     // vL	14	Base frequency 1
    #define REG_INVERTER_VF_CONTROL_MODE		0x0015     // Pt	15	V/F control mode selection
	#define REG_INVERTER_AUTO_START_FREQ		0x0241	   // F241	Frequenza avviamento automatico
    #define REG_INVERTER_BRAKE_START_FREQ		0x0250     // F250	250	DC braking starting frequency
    #define REG_INVERTER_BRAKE_CURRENT_SET		0x0251     // Corrente di frenatura in % della corrente nominale inverter (0..100%)
    #define REG_INVERTER_BRAKE_TIME				0x0252     // F252	252	DC braking time
	#define REG_INVERTER_PWM_CARRIER			0x0300     // F300	300	DC braking time
    #define REG_INVERTER_AUTORESTART_CONTROL	0x0301     // F301	301	Auto-restart control selection
    #define REG_INVERTER_SUPPLY_VOLTAGE_CORRECT	0x0307	   // F307 Supply voltage correction Default=2 Unit: 0,1,2,3
	#define REG_INVERTER_AUTOTUNING				0x0400     // F400	400	Autotuning
	#define REG_INVERTER_AUTO_TORQUE_BOOST_VAL	0x0402     // F402	402	Automatic torque boost value
    #define REG_INVERTER_MOTOR_RATED_CAPACITY	0x0405     // F405	405	Motor rated capacity
    #define REG_INVERTER_BASE_FREQ_VOLTAGE		0x0409     // vLv	409	Base frequency voltage 1
    #define REG_INVERTER_MOTOR_RATED_CURRENT	0x0415     // F415	415	Motor rated current
    #define REG_INVERTER_MOTOR_RATED_SPEED		0x0417     // F417	417	Motor rated speed
    #define REG_INVERTER_MOTOR_SPECIFIC_COEFF2	0x0458     // F458	458	Motor specific coefficient 2
    #define REG_INVERTER_LOAD_INERTIA_MOMENT	0x0459     // F459	459	Load inertia moment ratio
    #define REG_INVERTER_MOTOR_SPECIFIC_COEFF3	0x0460     // F460	460	Motor specific coefficient 3
    #define REG_INVERTER_MOTOR_SPECIFIC_COEFF9	0x0495     // F495	495	Motor specific coefficient 9
    #define REG_INVERTER_ACCELERATION_TIME2		0x0500     // F500	500	Acceleration time 2
    #define REG_INVERTER_DECELERATION_TIME2		0x0501     // F501	501	Deceleration time 2
    #define REG_INVERTER_ACC_DEC_1e2_SW_FREQ	0x0505     // F505	505	Acceleration/deceleration 1 and 2 switching frequency
    #define REG_INVERTER_STALL_PREVENTION_L1	0x0601     // F601	601	Stall prevention level 1
	#define REG_INVERTER_RIL_FASE_SELECTION		0x0608	   // F608	608 0 Selezione rilevamento fase ingresso
	#define REG_UNDERVOLTAGE_TRIP_ALARM_SEL     0x0627	   // F627	627 0 Selezione intervento/allarme sottotensione 60%
	#define REG_INVERTER_UNIT_SELECTION			0x0701	   // F701 Selezione unità corrente/tensione Dspu		change parameters from % to nominal value	1   ??????
	#define REG_INVERTER_TIMEOUT_COMUNICATION	0x0803	   // F803 Time-out tempo di comunicazione		Unit: 0.1s
	#define REG_INVERTER_ACTION_TMT_COMUNICATION 0x0804	   // F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
    #define REG_INVERTER_FACTORY_SPEC_COEFF9P	0x0908     // F908	908	Factory specific coefficient 9P
    #define REG_INVERTER_FACTORY_SPEC_COEFF9D	0x0909     // F909	909	Factory specific coefficient 9D
    #define REG_INVERTER_STEPOUT_DET_CURR_LEV	0x0910     // F910	910	Step-out detection current level
    #define REG_INVERTER_STEPOUT_DET_TIME		0x0911     // F911	911	Step-out detection time
	#define REG_INVERTER_QAXIS_INDUCTANCE		0x0912	   // F912	912	q-axis inductance (Autotuning)
	#define REG_INVERTER_DAXIS_INDUCTANCE		0x0913	   // F913	913	d-axis inductance (Autotuning)
    #define REG_INVERTER_PM_CONTROL_SELECTION	0x0915     // F915	915	PM control selection
    #define REG_INVERTER_FACTORY_SPEC_COEFF9F	0x0916     // F916	916	Factory specific coefficient 9F
    #define REG_INVERTER_FACTORY_SPEC_COEFF9L	0x0921     // F921	921	Factory specific coefficient 9L


    #define K_VAL_INVERTER_RUN_COMMAND          0x8400  // Valore per comando RUN - bit 10 = 1
    #define K_VAL_INVERTER_STOP_COMMAND         0x8000  // Valore per comando STOP - bit 10 = 0
    #define K_VAL_INVERTER_BREAK_COMMAND        0x8080  // Valore per comando BREAK - bit7 = 1 
    #define K_VAL_INVERTER_RESET_FLT_COMMAND    0xA000	// Valore per comando RESET FAULT 

    #define K_VAL_INVERTER_STOP_AND_COAST		0x8800	// Valore per comando STOP + COAST STOP

    #define K_VAL_INVERTER_ERROR_CODE_Err4		23		// Valore per Errore Err4 -> CPU1 Fault (Reg FC90) 
    #define K_VAL_INVERTER_ERROR_CODE_Err5		24		// Valore per Errore Err5 -> CommError Fault (Reg FC90) 
	#define K_VAL_INVERTER_ERROR_CODE_UP1		30		// Valore per Errore UP1 -> Undervoltage trip/alarm selection 
    #define K_VAL_INVERTER_ERROR_CODE_Sout		47		// Valore per Errore SOut -> StepOut Fault (Reg FC90) 
	#define K_VAL_INVERTER_ERROR_CODE_E21		53		// Valore per Errore E21 -> CPU2 Fault (Reg FC90) 
    #define K_VAL_INVERTER_ERROR_CODE_E26		58		// Valore per Errore E26 -> CPU3 Fault (Reg FC90) 
	#define K_VAL_INVERTER_ERROR_CODE_E39		71		// Valore per Errore E39 -> AutoTuning Fault (Reg FC90) 



    #define K_HeatingEngineStart                10				// % Valore corrente di frenatura per riscaldamento motore fase START
    #define K_HeatingEngineWork                 10				// % Valore corrente di frenatura per riscaldamento motore fase WORK

//-------------------------------------------------------
// Valori di default per inizializzazione Inverters compressore
//-------------------------------------------------------

// NOTA: sistemare valori corretti su tutti i "-1"
   #define K_DA270A2F20L_INVERTER_MAX_FREQUENCY				240*100			// FH	11 Max Frequency					Unit: 0.01Hz											
   #define K_DA270A2F20L_INVERTER_UPPER_LIM_FREQ			240*100			// UL	12	Upper limit frequency			Unit: 0.01Hz
   #define K_DA270A2F20L_INVERTER_BASE_FREQUENCY			120*100			// vL	14	Base frequency 1				Unit: 0.01Hz
#if (K_SIMULATION_WORK==1)
   #define K_DA270A2F20L_INVERTER_VF_CONTROL_MODE			0				// Pt	15	V/F control mode selection		Unit: 1
#else
   #define K_DA270A2F20L_INVERTER_VF_CONTROL_MODE			6*1				// Pt	15	V/F control mode selection		Unit: 1
#endif
	#define K_DA270A2F20L_INVERTER_AUTO_START_FREQ			25*100			// F241	Frequenza avviamento automatico		Unit: 0.01Hz
   #define K_DA270A2F20L_INVERTER_BRAKE_START_FREQ			24*100			// F250	250	DC braking starting frequency	Unit: 0.01Hz
   #define K_DA270A2F20L_INVERTER_BRAKE_CURRENT_SET			100*1			// F251	Corrente di frenatura in % della corrente nominale inverter (0..100%) Unit: 1%
   #define K_DA270A2F20L_INVERTER_BRAKE_TIME				0*10			// F252	252	DC braking time					Unit: 0.1Sec
   #define K_DA270A2F20L_INVERTER_PWM_CARRIER				4*10			// F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz
   #define K_DA270A2F20L_INVERTER_AUTORESTART_CONTROL		4*1				// F301	301	Auto-restart control selection	Unit: 1
   #define K_DA270A2F20L_INVERTER_SUPPLY_VOLTAGE_CORRECTION	1				// F307 Supply voltage correction Default=2 Unit: 0,1,2,3
   #define K_DA270A2F20L_INVERTER_AUTOTUNING				0*1				// F400	400	Autotuning						Unit: 1
   #define K_DA270A2F20L_INVERTER_AUTO_TORQUE_BOOST_VAL		2.1*10//1*10			// F402	402	Automatic torque boost value	Unit: 0.10%
   #define K_DA270A2F20L_INVERTER_MOTOR_RATED_CAPACITY		2.2*100			// F405	405	Motor rated capacity			Unit: 0.01Kw
   #define K_DA270A2F20L_INVERTER_BASE_FREQ_VOLTAGE			143*10			// vLv	409	Base frequency voltage 1		Unit: 0.1V
   #define K_DA270A2F20L_INVERTER_MOTOR_RATED_CURRENT		9.9*100			// F415	415	Motor rated current				Unit: 0.01A
   #define K_DA270A2F20L_INVERTER_MOTOR_RATED_SPEED			3600			// F417	417	Motor rated speed				Unit: 1min-1 ******????
   #define K_DA270A2F20L_INVERTER_MOTOR_SPECIFIC_COEFF2		25*1			// F458	458	Motor specific coefficient 2	Unit: 1
   #define K_DA270A2F20L_INVERTER_LOAD_INERTIA_MOMENT		1*10			// F459	459	Load inertia moment ratio		Unit: 0.1
   #define K_DA270A2F20L_INVERTER_MOTOR_SPECIFIC_COEFF3		2*10			// F460	460	Motor specific coefficient 3	Unit: 0.1
   #define K_DA270A2F20L_INVERTER_MOTOR_SPECIFIC_COEFF9		100*1			// F495	495	Motor specific coefficient 9	Unit: 1%
   #define K_DA270A2F20L_INVERTER_ACCELERATION_TIME2		120*10			// F500	500	Acceleration time 2				Unit: 0.1Sec
   #define K_DA270A2F20L_INVERTER_DECELERATION_TIME2		120*10			// F501	501	Deceleration time 2				Unit: 0.1Sec
   #define K_DA270A2F20L_INVERTER_ACC_DEC_1e2_SW_FREQ		24*100			// F505	505	Acceleration/deceleration 1 and 2 switching frequency	Unit: 0.01Hz
   #define K_DA270A2F20L_INVERTER_STALL_PREVENTION_L1		150*1	//* 8.10		// F601	601	Stall prevention level 1		Unit: 1%
   #define K_DA270A2F20L_INVERTER_RIL_FASE_SELECTION		0				// F608	0 Selezione rilevamento fase ingresso	Unit: ??
   #define K_DA270A2F20L_INVERTER_UNIT_SELECTION			0//1				// F701 Selezione unità corrente/tensione		Unit: ?? Dspu???????
   #define K_DA270A2F20L_INVERTER_TIMEOUT_COMUNICATION		6*10			// F803 Time-out tempo di comunicazione		Unit: 0.1s
#if (K_SIMULATION_WORK==1)
   #define K_DA270A2F20L_INVERTER_ACTION_TMT_COMUNICATION	0				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#else
   #define K_DA270A2F20L_INVERTER_ACTION_TMT_COMUNICATION	2				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#endif
//   #define K_DA270A2F20L_INVERTER_FACTORY_SPEC_COEFF9P		0*1		//* non esiste!!!!			// F908	908	Factory specific coefficient 9P	Unit: 1%
   #define K_DA270A2F20L_INVERTER_FACTORY_SPEC_COEFF9D		20*1			// F909	909	Factory specific coefficient 9D	Unit: 1%
#if (K_STEPOUT_DISABLE==1)
   #define K_DA270A2F20L_INVERTER_STEPOUT_DET_CURR_LEV		100*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA270A2F20L_INVERTER_STEPOUT_DET_TIME			0.00*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#else
   #define K_DA270A2F20L_INVERTER_STEPOUT_DET_CURR_LEV		30*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA270A2F20L_INVERTER_STEPOUT_DET_TIME			0.07*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#endif
   #define K_DA270A2F20L_INVERTER_QAXIS_INDUCTANCE			4.07*100		// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA270A2F20L_INVERTER_DAXIS_INDUCTANCE			3.81*100		// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA270A2F20L_INVERTER_PM_CONTROL_SELECTION		3*1				// F915	915	PM control selection			Unit: 1
   #define K_DA270A2F20L_INVERTER_FACTORY_SPEC_COEFF9L		100*1			// F921	921	Factory specific coefficient 9L	Unit: 1%       


//DA550A3F-10M
   #define K_DA550A3F10M_INVERTER_MAX_FREQUENCY				200*100			// FH	11 Max Frequency					Unit: 0.01Hz											
   #define K_DA550A3F10M_INVERTER_UPPER_LIM_FREQ			200*100			// UL	12	Upper limit frequency			Unit: 0.01Hz
   #define K_DA550A3F10M_INVERTER_BASE_FREQUENCY			120*100			// vL	14	Base frequency 1				Unit: 0.01Hz
#if (K_SIMULATION_WORK==1)
   #define K_DA550A3F10M_INVERTER_VF_CONTROL_MODE			0				// Pt	15	V/F control mode selection		Unit: 1
#else
   #define K_DA550A3F10M_INVERTER_VF_CONTROL_MODE			6*1				// Pt	15	V/F control mode selection		Unit: 1
#endif
   #define K_DA550A3F10M_INVERTER_AUTO_START_FREQ			25*100			// F241	Frequenza avviamento automatico		Unit: 0.01Hz
   #define K_DA550A3F10M_INVERTER_BRAKE_START_FREQ			30*100			// F250	250	DC braking starting frequency	Unit: 0.01Hz
   #define K_DA550A3F10M_INVERTER_BRAKE_CURRENT_SET			100*1			// F251	Corrente di frenatura in % della corrente nominale inverter (0..100%) Unit: 1%
   #define K_DA550A3F10M_INVERTER_BRAKE_TIME				0*10			// F252	252	DC braking time					Unit: 0.1Sec
   #define K_DA550A3F10M_INVERTER_PWM_CARRIER				4*10			// F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz
   #define K_DA550A3F10M_INVERTER_AUTORESTART_CONTROL		4*1				// F301	301	Auto-restart control selection	Unit: 1
   #define K_DA550A3F10M_INVERTER_SUPPLY_VOLTAGE_CORRECTION	1				// F307 Supply voltage correction Default=2 Unit: 0,1,2,3
   #define K_DA550A3F10M_INVERTER_AUTOTUNING				0*1				// F400	400	Autotuning						Unit: 1
   #define K_DA550A3F10M_INVERTER_AUTO_TORQUE_BOOST_VAL		1.6*10			// F402	402	Automatic torque boost value	Unit: 0.10%
   #define K_DA550A3F10M_INVERTER_MOTOR_RATED_CAPACITY		3.7*100			// F405	405	Motor rated capacity			Unit: 0.01Kw
   #define K_DA550A3F10M_INVERTER_BASE_FREQ_VOLTAGE			114*10			// vLv	409	Base frequency voltage 1		Unit: 0.1V
   #define K_DA550A3F10M_INVERTER_MOTOR_RATED_CURRENT		22.2*100		// F415	415	Motor rated current				Unit: 0.01A
   #define K_DA550A3F10M_INVERTER_MOTOR_RATED_SPEED			3600			// F417	417	Motor rated speed				Unit: 1min-1 ******????
   #define K_DA550A3F10M_INVERTER_MOTOR_SPECIFIC_COEFF2		25*1			// F458	458	Motor specific coefficient 2	Unit: 1
   #define K_DA550A3F10M_INVERTER_LOAD_INERTIA_MOMENT		0.5*10			// F459	459	Load inertia moment ratio		Unit: 0.1
   #define K_DA550A3F10M_INVERTER_MOTOR_SPECIFIC_COEFF3		2*10			// F460	460	Motor specific coefficient 3	Unit: 0.1
   #define K_DA550A3F10M_INVERTER_MOTOR_SPECIFIC_COEFF9		100*1			// F495	495	Motor specific coefficient 9	Unit: 1%
   #define K_DA550A3F10M_INVERTER_ACCELERATION_TIME2		100*10			// F500	500	Acceleration time 2				Unit: 0.1Sec
   #define K_DA550A3F10M_INVERTER_DECELERATION_TIME2		100*10			// F501	501	Deceleration time 2				Unit: 0.1Sec
   #define K_DA550A3F10M_INVERTER_ACC_DEC_1e2_SW_FREQ		30*100			// F505	505	Acceleration/deceleration 1 and 2 switching frequency	Unit: 0.01Hz
   #define K_DA550A3F10M_INVERTER_STALL_PREVENTION_L1		150*1	//* 8.10		// F601	601	Stall prevention level 1		Unit: 1%
   #define K_DA550A3F10M_INVERTER_RIL_FASE_SELECTION		0				// F608	0 Selezione rilevamento fase ingresso	Unit: ??
   #define K_DA550A3F10M_INVERTER_UNIT_SELECTION			0//1				// F701 Selezione unità corrente/tensione		Unit: ?? Dspu???????
   #define K_DA550A3F10M_INVERTER_TIMEOUT_COMUNICATION		6*10			// F803 Time-out tempo di comunicazione		Unit: 0.1s
#if (K_SIMULATION_WORK==1)
   #define K_DA550A3F10M_INVERTER_ACTION_TMT_COMUNICATION	0				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#else
   #define K_DA550A3F10M_INVERTER_ACTION_TMT_COMUNICATION	2				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#endif
//   #define K_DA550A3F10M_INVERTER_FACTORY_SPEC_COEFF9P		0*1		//* non esiste!!!!			// F908	908	Factory specific coefficient 9P	Unit: 1%
   #define K_DA550A3F10M_INVERTER_FACTORY_SPEC_COEFF9D		20*1			// F909	909	Factory specific coefficient 9D	Unit: 1%
#if (K_STEPOUT_DISABLE==1)
   #define K_DA550A3F10M_INVERTER_STEPOUT_DET_CURR_LEV		100*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA550A3F10M_INVERTER_STEPOUT_DET_TIME			0.00*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#else
   #define K_DA550A3F10M_INVERTER_STEPOUT_DET_CURR_LEV		60*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA550A3F10M_INVERTER_STEPOUT_DET_TIME			0.07*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#endif
   #define K_DA550A3F10M_INVERTER_QAXIS_INDUCTANCE			2.26*100		// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA550A3F10M_INVERTER_DAXIS_INDUCTANCE			2.24*100		// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA550A3F10M_INVERTER_PM_CONTROL_SELECTION		3*1				// F915	915	PM control selection			Unit: 1
   #define K_DA550A3F10M_INVERTER_FACTORY_SPEC_COEFF9L		100*1			// F921	921	Factory specific coefficient 9L	Unit: 1%       


//DA422A3F26M
   #define K_DA422A3F26M_INVERTER_MAX_FREQUENCY				240*100			// FH	11 Max Frequency					Unit: 0.01Hz											
   #define K_DA422A3F26M_INVERTER_UPPER_LIM_FREQ			240*100			// UL	12	Upper limit frequency			Unit: 0.01Hz
   #define K_DA422A3F26M_INVERTER_BASE_FREQUENCY			120*100			// vL	14	Base frequency 1				Unit: 0.01Hz
#if (K_SIMULATION_WORK==1)
   #define K_DA422A3F26M_INVERTER_VF_CONTROL_MODE			0				// Pt	15	V/F control mode selection		Unit: 1
#else
   #define K_DA422A3F26M_INVERTER_VF_CONTROL_MODE			6*1				// Pt	15	V/F control mode selection		Unit: 1
#endif
   #define K_DA422A3F26M_INVERTER_AUTO_START_FREQ			25*100			// F241	Frequenza avviamento automatico		Unit: 0.01Hz
   #define K_DA422A3F26M_INVERTER_BRAKE_START_FREQ			20*100			// F250	250	DC braking starting frequency	Unit: 0.01Hz
   #define K_DA422A3F26M_INVERTER_BRAKE_CURRENT_SET			0*1	//100??		// F251	Corrente di frenatura in % della corrente nominale inverter (0..100%) Unit: 1%
   #define K_DA422A3F26M_INVERTER_BRAKE_TIME				0*10			// F252	252	DC braking time					Unit: 0.1Sec
   #define K_DA422A3F26M_INVERTER_PWM_CARRIER				4*10			// F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz
   #define K_DA422A3F26M_INVERTER_AUTORESTART_CONTROL		4*1				// F301	301	Auto-restart control selection	Unit: 1
   #define K_DA422A3F26M_INVERTER_SUPPLY_VOLTAGE_CORRECTION	1				// F307 Supply voltage correction Default=2 Unit: 0,1,2,3
   #define K_DA422A3F26M_INVERTER_AUTOTUNING				0*1				// F400	400	Autotuning						Unit: 1
   #define K_DA422A3F26M_INVERTER_AUTO_TORQUE_BOOST_VAL		1*10			// F402	402	Automatic torque boost value	Unit: 0.10%
   #define K_DA422A3F26M_INVERTER_MOTOR_RATED_CAPACITY		2.8*100			// F405	405	Motor rated capacity			Unit: 0.01Kw
   #define K_DA422A3F26M_INVERTER_BASE_FREQ_VOLTAGE			141*10			// vLv	409	Base frequency voltage 1		Unit: 0.1V
   #define K_DA422A3F26M_INVERTER_MOTOR_RATED_CURRENT		13.5*100		// F415	415	Motor rated current				Unit: 0.01A
   #define K_DA422A3F26M_INVERTER_MOTOR_RATED_SPEED			3600			// F417	417	Motor rated speed				Unit: 1min-1 ******????
   #define K_DA422A3F26M_INVERTER_MOTOR_SPECIFIC_COEFF2		25*1			// F458	458	Motor specific coefficient 2	Unit: 1
   #define K_DA422A3F26M_INVERTER_LOAD_INERTIA_MOMENT		0.5*10			// F459	459	Load inertia moment ratio		Unit: 0.1
   #define K_DA422A3F26M_INVERTER_MOTOR_SPECIFIC_COEFF3		2*10			// F460	460	Motor specific coefficient 3	Unit: 0.1
   #define K_DA422A3F26M_INVERTER_MOTOR_SPECIFIC_COEFF9		100*1			// F495	495	Motor specific coefficient 9	Unit: 1%
   #define K_DA422A3F26M_INVERTER_ACCELERATION_TIME2		120*10			// F500	500	Acceleration time 2				Unit: 0.1Sec
   #define K_DA422A3F26M_INVERTER_DECELERATION_TIME2		120*10			// F501	501	Deceleration time 2				Unit: 0.1Sec
   #define K_DA422A3F26M_INVERTER_ACC_DEC_1e2_SW_FREQ		20*100			// F505	505	Acceleration/deceleration 1 and 2 switching frequency	Unit: 0.01Hz
   #define K_DA422A3F26M_INVERTER_STALL_PREVENTION_L1		130*1			// F601	601	Stall prevention level 1		Unit: 1%
   #define K_DA422A3F26M_INVERTER_RIL_FASE_SELECTION		0				// F608	0 Selezione rilevamento fase ingresso	Unit: ??
   #define K_DA422A3F26M_INVERTER_UNIT_SELECTION			0//1				// F701 Selezione unità corrente/tensione		Unit: ?? Dspu???????
   #define K_DA422A3F26M_INVERTER_TIMEOUT_COMUNICATION		6*10			// F803 Time-out tempo di comunicazione		Unit: 0.1s
#if (K_SIMULATION_WORK==1)
   #define K_DA422A3F26M_INVERTER_ACTION_TMT_COMUNICATION	0				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#else
   #define K_DA422A3F26M_INVERTER_ACTION_TMT_COMUNICATION	2				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#endif
//   #define K_DA422A3F26M_INVERTER_FACTORY_SPEC_COEFF9P		0*1		//* non esiste!!!!			// F908	908	Factory specific coefficient 9P	Unit: 1%
   #define K_DA422A3F26M_INVERTER_FACTORY_SPEC_COEFF9D		20*1			// F909	909	Factory specific coefficient 9D	Unit: 1%
#if (K_STEPOUT_DISABLE==1)
   #define K_DA422A3F26M_INVERTER_STEPOUT_DET_CURR_LEV		100*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA422A3F26M_INVERTER_STEPOUT_DET_TIME			0.00*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#else
   #define K_DA422A3F26M_INVERTER_STEPOUT_DET_CURR_LEV		37*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA422A3F26M_INVERTER_STEPOUT_DET_TIME			0.07*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#endif
   #define K_DA422A3F26M_INVERTER_QAXIS_INDUCTANCE			3.3*100			// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA422A3F26M_INVERTER_DAXIS_INDUCTANCE			2.63*100		// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA422A3F26M_INVERTER_PM_CONTROL_SELECTION		3*1				// F915	915	PM control selection			Unit: 1
   #define K_DA422A3F26M_INVERTER_FACTORY_SPEC_COEFF9L		100*1			// F921	921	Factory specific coefficient 9L	Unit: 1%       

//DA422A3F27M
   #define K_DA422A3F27M_INVERTER_MAX_FREQUENCY				240*100			// FH	11 Max Frequency					Unit: 0.01Hz											
   #define K_DA422A3F27M_INVERTER_UPPER_LIM_FREQ			240*100			// UL	12	Upper limit frequency			Unit: 0.01Hz
   #define K_DA422A3F27M_INVERTER_BASE_FREQUENCY			120*100			// vL	14	Base frequency 1				Unit: 0.01Hz
#if (K_SIMULATION_WORK==1)
   #define K_DA422A3F27M_INVERTER_VF_CONTROL_MODE			0				// Pt	15	V/F control mode selection		Unit: 1
#else
   #define K_DA422A3F27M_INVERTER_VF_CONTROL_MODE			6*1				// Pt	15	V/F control mode selection		Unit: 1
#endif
   #define K_DA422A3F27M_INVERTER_AUTO_START_FREQ			25*100			// F241	Frequenza avviamento automatico		Unit: 0.01Hz
   #define K_DA422A3F27M_INVERTER_BRAKE_START_FREQ			20*100			// F250	250	DC braking starting frequency	Unit: 0.01Hz
   #define K_DA422A3F27M_INVERTER_BRAKE_CURRENT_SET			0*1	//100??		// F251	Corrente di frenatura in % della corrente nominale inverter (0..100%) Unit: 1%
   #define K_DA422A3F27M_INVERTER_BRAKE_TIME				0*10			// F252	252	DC braking time					Unit: 0.1Sec
   #define K_DA422A3F27M_INVERTER_PWM_CARRIER				4*10			// F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz
   #define K_DA422A3F27M_INVERTER_AUTORESTART_CONTROL		4*1				// F301	301	Auto-restart control selection	Unit: 1
   #define K_DA422A3F27M_INVERTER_SUPPLY_VOLTAGE_CORRECTION	1				// F307 Supply voltage correction Default=2 Unit: 0,1,2,3
   #define K_DA422A3F27M_INVERTER_AUTOTUNING				0*1				// F400	400	Autotuning						Unit: 1
   #define K_DA422A3F27M_INVERTER_AUTO_TORQUE_BOOST_VAL		1.1*10			// F402	402	Automatic torque boost value	Unit: 0.10%
   #define K_DA422A3F27M_INVERTER_MOTOR_RATED_CAPACITY		2.9*100			// F405	405	Motor rated capacity			Unit: 0.01Kw
   #define K_DA422A3F27M_INVERTER_BASE_FREQ_VOLTAGE			256*10			// vLv	409	Base frequency voltage 1		Unit: 0.1V
   #define K_DA422A3F27M_INVERTER_MOTOR_RATED_CURRENT		7.9*100         // F415	415	Motor rated current				Unit: 0.01A
   #define K_DA422A3F27M_INVERTER_MOTOR_RATED_SPEED			3600			// F417	417	Motor rated speed				Unit: 1min-1 ******????
   #define K_DA422A3F27M_INVERTER_MOTOR_SPECIFIC_COEFF2		25*1			// F458	458	Motor specific coefficient 2	Unit: 1
   #define K_DA422A3F27M_INVERTER_LOAD_INERTIA_MOMENT		0.5*10			// F459	459	Load inertia moment ratio		Unit: 0.1
   #define K_DA422A3F27M_INVERTER_MOTOR_SPECIFIC_COEFF3		2*10			// F460	460	Motor specific coefficient 3	Unit: 0.1
   #define K_DA422A3F27M_INVERTER_MOTOR_SPECIFIC_COEFF9		100*1			// F495	495	Motor specific coefficient 9	Unit: 1%
   #define K_DA422A3F27M_INVERTER_ACCELERATION_TIME2		120*10			// F500	500	Acceleration time 2				Unit: 0.1Sec
   #define K_DA422A3F27M_INVERTER_DECELERATION_TIME2		120*10			// F501	501	Deceleration time 2				Unit: 0.1Sec
   #define K_DA422A3F27M_INVERTER_ACC_DEC_1e2_SW_FREQ		20*100			// F505	505	Acceleration/deceleration 1 and 2 switching frequency	Unit: 0.01Hz
   #define K_DA422A3F27M_INVERTER_STALL_PREVENTION_L1		83*1			// F601	601	Stall prevention level 1		Unit: 1%
   #define K_DA422A3F27M_INVERTER_RIL_FASE_SELECTION		0				// F608	0 Selezione rilevamento fase ingresso	Unit: ??
   #define K_DA422A3F27M_INVERTER_UNIT_SELECTION			0//1				// F701 Selezione unità corrente/tensione		Unit: ?? Dspu???????
   #define K_DA422A3F27M_INVERTER_TIMEOUT_COMUNICATION		6*10			// F803 Time-out tempo di comunicazione		Unit: 0.1s
#if (K_SIMULATION_WORK==1)
   #define K_DA422A3F27M_INVERTER_ACTION_TMT_COMUNICATION	0				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#else
   #define K_DA422A3F27M_INVERTER_ACTION_TMT_COMUNICATION	2				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#endif
//   #define K_DA422A3F27M_INVERTER_FACTORY_SPEC_COEFF9P		0*1		//* non esiste!!!!			// F908	908	Factory specific coefficient 9P	Unit: 1%
   #define K_DA422A3F27M_INVERTER_FACTORY_SPEC_COEFF9D		20*1			// F909	909	Factory specific coefficient 9D	Unit: 1%
#if (K_STEPOUT_DISABLE==1)
   #define K_DA422A3F27M_INVERTER_STEPOUT_DET_CURR_LEV		100*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA422A3F27M_INVERTER_STEPOUT_DET_TIME			0.00*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#else
   #define K_DA422A3F27M_INVERTER_STEPOUT_DET_CURR_LEV		41*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA422A3F27M_INVERTER_STEPOUT_DET_TIME			0.07*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#endif
   #define K_DA422A3F27M_INVERTER_QAXIS_INDUCTANCE			12.96*100			// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA422A3F27M_INVERTER_DAXIS_INDUCTANCE			11.74*100		// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA422A3F27M_INVERTER_PM_CONTROL_SELECTION		3*1				// F915	915	PM control selection			Unit: 1
   #define K_DA422A3F27M_INVERTER_FACTORY_SPEC_COEFF9L		100*1			// F921	921	Factory specific coefficient 9L	Unit: 1%       

//DA790A4F11UC1
   #define K_DA790A4F11UC1_INVERTER_MAX_FREQUENCY				200*100			// FH	11 Max Frequency					Unit: 0.01Hz											
   #define K_DA790A4F11UC1_INVERTER_UPPER_LIM_FREQ				200*100			// UL	12	Upper limit frequency			Unit: 0.01Hz
   #define K_DA790A4F11UC1_INVERTER_BASE_FREQUENCY				120*100			// vL	14	Base frequency 1				Unit: 0.01Hz
#if (K_SIMULATION_WORK==1)
   #define K_DA790A4F11UC1_INVERTER_VF_CONTROL_MODE				0//6*1				// Pt	15	V/F control mode selection		Unit: 1
#else
   #define K_DA790A4F11UC1_INVERTER_VF_CONTROL_MODE				6*1				// Pt	15	V/F control mode selection		Unit: 1
#endif
   #define K_DA790A4F11UC1_INVERTER_AUTO_START_FREQ				25*100			// F241	Frequenza avviamento automatico		Unit: 0.01Hz
   #define K_DA790A4F11UC1_INVERTER_BRAKE_START_FREQ			30*100			// F250	250	DC braking starting frequency	Unit: 0.01Hz
   #define K_DA790A4F11UC1_INVERTER_BRAKE_CURRENT_SET			0*1				// F251	Corrente di frenatura in % della corrente nominale inverter (0..100%) Unit: 1%
   #define K_DA790A4F11UC1_INVERTER_BRAKE_TIME					0*10			// F252	252	DC braking time					Unit: 0.1Sec
   #define K_DA790A4F11UC1_INVERTER_PWM_CARRIER					4*10			// F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz
   #define K_DA790A4F11UC1_INVERTER_AUTORESTART_CONTROL			4*1				// F301	301	Auto-restart control selection	Unit: 1
   #define K_DA790A4F11UC1_INVERTER_SUPPLY_VOLTAGE_CORRECTION	1				// F307 Supply voltage correction Default=2 Unit: 0,1,2,3
   #define K_DA790A4F11UC1_INVERTER_AUTOTUNING					0*1				// F400	400	Autotuning						Unit: 1
   #define K_DA790A4F11UC1_INVERTER_AUTO_TORQUE_BOOST_VAL		1.9*10//2.5*10			// F402	402	Automatic torque boost value	Unit: 0.10%
   #define K_DA790A4F11UC1_INVERTER_MOTOR_RATED_CAPACITY		7.5*100			// F405	405	Motor rated capacity			Unit: 0.01Kw
   #define K_DA790A4F11UC1_INVERTER_BASE_FREQ_VOLTAGE			297*10			// vLv	409	Base frequency voltage 1		Unit: 0.1V
   #define K_DA790A4F11UC1_INVERTER_MOTOR_RATED_CURRENT			15.7*100		// F415	415	Motor rated current				Unit: 0.01A
   #define K_DA790A4F11UC1_INVERTER_MOTOR_RATED_SPEED			3600			// F417	417	Motor rated speed				Unit: 1min-1 ******????
   #define K_DA790A4F11UC1_INVERTER_MOTOR_SPECIFIC_COEFF2		25*1			// F458	458	Motor specific coefficient 2	Unit: 1
   #define K_DA790A4F11UC1_INVERTER_LOAD_INERTIA_MOMENT			0.5*10			// F459	459	Load inertia moment ratio		Unit: 0.1
   #define K_DA790A4F11UC1_INVERTER_MOTOR_SPECIFIC_COEFF3		2*10			// F460	460	Motor specific coefficient 3	Unit: 0.1
   #define K_DA790A4F11UC1_INVERTER_MOTOR_SPECIFIC_COEFF9		100*1			// F495	495	Motor specific coefficient 9	Unit: 1%
   #define K_DA790A4F11UC1_INVERTER_ACCELERATION_TIME2			100*10			// F500	500	Acceleration time 2				Unit: 0.1Sec
   #define K_DA790A4F11UC1_INVERTER_DECELERATION_TIME2			100*10			// F501	501	Deceleration time 2				Unit: 0.1Sec
   #define K_DA790A4F11UC1_INVERTER_ACC_DEC_1e2_SW_FREQ			30*100			// F505	505	Acceleration/deceleration 1 and 2 switching frequency	Unit: 0.01Hz
   #define K_DA790A4F11UC1_INVERTER_STALL_PREVENTION_L1			100*1			// F601	601	Stall prevention level 1		Unit: 1%
   #define K_DA790A4F11UC1_INVERTER_RIL_FASE_SELECTION			0				// F608	0 Selezione rilevamento fase ingresso	Unit: ??
   #define K_DA790A4F11UC1_INVERTER_UNIT_SELECTION				0//1			// F701 Selezione unità corrente/tensione		Unit: ?? Dspu???????
   #define K_DA790A4F11UC1_INVERTER_TIMEOUT_COMUNICATION		6*10			// F803 Time-out tempo di comunicazione		Unit: 0.1s
#if (K_SIMULATION_WORK==1)
   #define K_DA790A4F11UC1_INVERTER_ACTION_TMT_COMUNICATION		0				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#else
   #define K_DA790A4F11UC1_INVERTER_ACTION_TMT_COMUNICATION		2				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#endif
//   #define K_DA790A4F11UC1_INVERTER_FACTORY_SPEC_COEFF9P		0*1		//* non esiste!!!!			// F908	908	Factory specific coefficient 9P	Unit: 1%
   #define K_DA790A4F11UC1_INVERTER_FACTORY_SPEC_COEFF9D		20*1			// F909	909	Factory specific coefficient 9D	Unit: 1%
#if (K_STEPOUT_DISABLE==1)
   #define K_DA790A4F11UC1_INVERTER_STEPOUT_DET_CURR_LEV		100*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA790A4F11UC1_INVERTER_STEPOUT_DET_TIME			0.00*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#else
   #define K_DA790A4F11UC1_INVERTER_STEPOUT_DET_CURR_LEV		24*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA790A4F11UC1_INVERTER_STEPOUT_DET_TIME			0.07*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#endif
   #define K_DA790A4F11UC1_INVERTER_QAXIS_INDUCTANCE			5.75*100		// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA790A4F11UC1_INVERTER_DAXIS_INDUCTANCE			2.58*100		// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA790A4F11UC1_INVERTER_PM_CONTROL_SELECTION		3*1				// F915	915	PM control selection			Unit: 1
   #define K_DA790A4F11UC1_INVERTER_FACTORY_SPEC_COEFF9L		100*1			// F921	921	Factory specific coefficient 9L	Unit: 1%       


// DA330A2F20M
   #define K_DA330A2F20M_INVERTER_MAX_FREQUENCY				240*100			// FH	11 Max Frequency					Unit: 0.01Hz											
   #define K_DA330A2F20M_INVERTER_UPPER_LIM_FREQ			240*100			// UL	12	Upper limit frequency			Unit: 0.01Hz
   #define K_DA330A2F20M_INVERTER_BASE_FREQUENCY			120*100			// vL	14	Base frequency 1				Unit: 0.01Hz
#if (K_SIMULATION_WORK==1)
   #define K_DA330A2F20M_INVERTER_VF_CONTROL_MODE			0				// Pt	15	V/F control mode selection		Unit: 1
#else
   #define K_DA330A2F20M_INVERTER_VF_CONTROL_MODE			6*1				// Pt	15	V/F control mode selection		Unit: 1
#endif
	#define K_DA330A2F20M_INVERTER_AUTO_START_FREQ			25*100			// F241	Frequenza avviamento automatico		Unit: 0.01Hz
   #define K_DA330A2F20M_INVERTER_BRAKE_START_FREQ			24*100			// F250	250	DC braking starting frequency	Unit: 0.01Hz
   #define K_DA330A2F20M_INVERTER_BRAKE_CURRENT_SET			100*1			// F251	Corrente di frenatura in % della corrente nominale inverter (0..100%) Unit: 1%                  0 ????
   #define K_DA330A2F20M_INVERTER_BRAKE_TIME				0*10			// F252	252	DC braking time					Unit: 0.1Sec
   #define K_DA330A2F20M_INVERTER_PWM_CARRIER				4*10			// F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz                                                     12 ????
   #define K_DA330A2F20M_INVERTER_AUTORESTART_CONTROL		4*1				// F301	301	Auto-restart control selection	Unit: 1
   #define K_DA330A2F20M_INVERTER_SUPPLY_VOLTAGE_CORRECTION	1				// F307 Supply voltage correction Default=2 Unit: 0,1,2,3
   #define K_DA330A2F20M_INVERTER_AUTOTUNING				0*1				// F400	400	Autotuning						Unit: 1
   #define K_DA330A2F20M_INVERTER_AUTO_TORQUE_BOOST_VAL		1.9*10//2.1*10	// F402	402	Automatic torque boost value	Unit: 0.10%                                         ??????
   #define K_DA330A2F20M_INVERTER_MOTOR_RATED_CAPACITY		2.9*100			// F405	405	Motor rated capacity			Unit: 0.01Kw                                                2.2 **
   #define K_DA330A2F20M_INVERTER_BASE_FREQ_VOLTAGE			136*10			// vLv	409	Base frequency voltage 1		Unit: 0.1V                                                  143  **
   #define K_DA330A2F20M_INVERTER_MOTOR_RATED_CURRENT		14.7*100			// F415	415	Motor rated current				Unit: 0.01A                                                 9.9 **
   #define K_DA330A2F20M_INVERTER_MOTOR_RATED_SPEED			3600			// F417	417	Motor rated speed				Unit: 1min-1 ******????
   #define K_DA330A2F20M_INVERTER_MOTOR_SPECIFIC_COEFF2		25*1			// F458	458	Motor specific coefficient 2	Unit: 1
   #define K_DA330A2F20M_INVERTER_LOAD_INERTIA_MOMENT		0.5*10			// F459	459	Load inertia moment ratio		Unit: 0.1                                                   1 **
   #define K_DA330A2F20M_INVERTER_MOTOR_SPECIFIC_COEFF3		2*10			// F460	460	Motor specific coefficient 3	Unit: 0.1
   #define K_DA330A2F20M_INVERTER_MOTOR_SPECIFIC_COEFF9		100*1			// F495	495	Motor specific coefficient 9	Unit: 1%
   #define K_DA330A2F20M_INVERTER_ACCELERATION_TIME2		120*10			// F500	500	Acceleration time 2				Unit: 0.1Sec
   #define K_DA330A2F20M_INVERTER_DECELERATION_TIME2		120*10			// F501	501	Deceleration time 2				Unit: 0.1Sec
   #define K_DA330A2F20M_INVERTER_ACC_DEC_1e2_SW_FREQ		24*100			// F505	505	Acceleration/deceleration 1 and 2 switching frequency	Unit: 0.01Hz
   #define K_DA330A2F20M_INVERTER_STALL_PREVENTION_L1		126*1	//* 8.10		// F601	601	Stall prevention level 1		Unit: 1%                                            150 **
   #define K_DA330A2F20M_INVERTER_RIL_FASE_SELECTION		0				// F608	0 Selezione rilevamento fase ingresso	Unit: ??
   #define K_DA330A2F20M_INVERTER_UNIT_SELECTION			0//1				// F701 Selezione unità corrente/tensione		Unit: ?? Dspu???????                                ????
   #define K_DA330A2F20M_INVERTER_TIMEOUT_COMUNICATION		6*10			// F803 Time-out tempo di comunicazione		Unit: 0.1s
#if (K_SIMULATION_WORK==1)
   #define K_DA330A2F20M_INVERTER_ACTION_TMT_COMUNICATION	0				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#else
   #define K_DA330A2F20M_INVERTER_ACTION_TMT_COMUNICATION	2				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#endif
//   #define K_DA330A2F20M_INVERTER_FACTORY_SPEC_COEFF9P		0*1		//* non esiste!!!!			// F908	908	Factory specific coefficient 9P	Unit: 1%
   #define K_DA330A2F20M_INVERTER_FACTORY_SPEC_COEFF9D		20*1			// F909	909	Factory specific coefficient 9D	Unit: 1%
#if (K_STEPOUT_DISABLE==1)
   #define K_DA330A2F20M_INVERTER_STEPOUT_DET_CURR_LEV		100*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA330A2F20M_INVERTER_STEPOUT_DET_TIME			0.00*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#else
   #define K_DA330A2F20M_INVERTER_STEPOUT_DET_CURR_LEV		63*100			// F910	910	Step-out detection current level	Unit: 0.01%                                             30 *
   #define K_DA330A2F20M_INVERTER_STEPOUT_DET_TIME			0.07*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#endif
   #define K_DA330A2F20M_INVERTER_QAXIS_INDUCTANCE			3.77*100//4.07*100		// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry                                            ????
   #define K_DA330A2F20M_INVERTER_DAXIS_INDUCTANCE			3.12*100//3.81*100		// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry                                            ????
   #define K_DA330A2F20M_INVERTER_PM_CONTROL_SELECTION		3*1				// F915	915	PM control selection			Unit: 1
// 916 ????
   #define K_DA330A2F20M_INVERTER_FACTORY_SPEC_COEFF9L		100*1			// F921	921	Factory specific coefficient 9L	Unit: 1%                                                150 100 default ????


//DA550A3F-11MD
   #define K_DA550A3F11MD_INVERTER_MAX_FREQUENCY			200*100			// FH	11 Max Frequency					Unit: 0.01Hz											
   #define K_DA550A3F11MD_INVERTER_UPPER_LIM_FREQ			200*100			// UL	12	Upper limit frequency			Unit: 0.01Hz
   #define K_DA550A3F11MD_INVERTER_BASE_FREQUENCY			120*100			// vL	14	Base frequency 1				Unit: 0.01Hz
#if (K_SIMULATION_WORK==1)
   #define K_DA550A3F11MD_INVERTER_VF_CONTROL_MODE			0				// Pt	15	V/F control mode selection		Unit: 1
#else
   #define K_DA550A3F11MD_INVERTER_VF_CONTROL_MODE			6*1				// Pt	15	V/F control mode selection		Unit: 1
#endif
   #define K_DA550A3F11MD_INVERTER_AUTO_START_FREQ			25*100			// F241	Frequenza avviamento automatico		Unit: 0.01Hz
   #define K_DA550A3F11MD_INVERTER_BRAKE_START_FREQ			25*100			// F250	250	DC braking starting frequency	Unit: 0.01Hz
   #define K_DA550A3F11MD_INVERTER_BRAKE_CURRENT_SET		100*1			// F251	Corrente di frenatura in % della corrente nominale inverter (0..100%) Unit: 1%
   #define K_DA550A3F11MD_INVERTER_BRAKE_TIME				0*10			// F252	252	DC braking time					Unit: 0.1Sec
   #define K_DA550A3F11MD_INVERTER_PWM_CARRIER				12*10//4*10			// F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz
   #define K_DA550A3F11MD_INVERTER_AUTORESTART_CONTROL		4*1				// F301	301	Auto-restart control selection	Unit: 1
   #define K_DA550A3F11MD_INVERTER_SUPPLY_VOLTAGE_CORRECTION	1				// F307 Supply voltage correction Default=2 Unit: 0,1,2,3
   #define K_DA550A3F11MD_INVERTER_AUTOTUNING				0*1				// F400	400	Autotuning						Unit: 1
   #define K_DA550A3F11MD_INVERTER_AUTO_TORQUE_BOOST_VAL	0.3*10		//??	// F402	402	Automatic torque boost value	Unit: 0.10%
   #define K_DA550A3F11MD_INVERTER_MOTOR_RATED_CAPACITY		3.75*100		// F405	405	Motor rated capacity			Unit: 0.01Kw
   #define K_DA550A3F11MD_INVERTER_BASE_FREQ_VOLTAGE		292*10			// vLv	409	Base frequency voltage 1		Unit: 0.1V
   #define K_DA550A3F11MD_INVERTER_MOTOR_RATED_CURRENT		8.7*100         // F415	415	Motor rated current				Unit: 0.01A
   #define K_DA550A3F11MD_INVERTER_MOTOR_RATED_SPEED		3600			// F417	417	Motor rated speed				Unit: 1min-1 ******????
   #define K_DA550A3F11MD_INVERTER_MOTOR_SPECIFIC_COEFF2	25*1			// F458	458	Motor specific coefficient 2	Unit: 1
   #define K_DA550A3F11MD_INVERTER_LOAD_INERTIA_MOMENT		1.0*10			// F459	459	Load inertia moment ratio		Unit: 0.1
   #define K_DA550A3F11MD_INVERTER_MOTOR_SPECIFIC_COEFF3	2*10			// F460	460	Motor specific coefficient 3	Unit: 0.1
   #define K_DA550A3F11MD_INVERTER_MOTOR_SPECIFIC_COEFF9	100*1			// F495	495	Motor specific coefficient 9	Unit: 1%
   #define K_DA550A3F11MD_INVERTER_ACCELERATION_TIME2		100*10			// F500	500	Acceleration time 2				Unit: 0.1Sec
   #define K_DA550A3F11MD_INVERTER_DECELERATION_TIME2		100*10			// F501	501	Deceleration time 2				Unit: 0.1Sec
   #define K_DA550A3F11MD_INVERTER_ACC_DEC_1e2_SW_FREQ		30*100			// F505	505	Acceleration/deceleration 1 and 2 switching frequency	Unit: 0.01Hz
   #define K_DA550A3F11MD_INVERTER_STALL_PREVENTION_L1		150*1			// F601	601	Stall prevention level 1		Unit: 1%
   #define K_DA550A3F11MD_INVERTER_RIL_FASE_SELECTION		0				// F608	0 Selezione rilevamento fase ingresso	Unit: ??
   #define K_DA550A3F11MD_INVERTER_UNIT_SELECTION			0//1				// F701 Selezione unità corrente/tensione		Unit: ?? Dspu???????
   #define K_DA550A3F11MD_INVERTER_TIMEOUT_COMUNICATION		6*10			// F803 Time-out tempo di comunicazione		Unit: 0.1s
#if (K_SIMULATION_WORK==1)
   #define K_DA550A3F11MD_INVERTER_ACTION_TMT_COMUNICATION	0				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#else
   #define K_DA550A3F11MD_INVERTER_ACTION_TMT_COMUNICATION	2				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#endif
//   #define K_DA550A3F11MD_INVERTER_FACTORY_SPEC_COEFF9P		0*1         //* non esiste!!!!			// F908	908	Factory specific coefficient 9P	Unit: 1%
   #define K_DA550A3F11MD_INVERTER_FACTORY_SPEC_COEFF9D		20*1			// F909	909	Factory specific coefficient 9D	Unit: 1%
#if (K_STEPOUT_DISABLE==1)
   #define K_DA550A3F11MD_INVERTER_STEPOUT_DET_CURR_LEV		100*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA550A3F11MD_INVERTER_STEPOUT_DET_TIME			0.00*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#else
   #define K_DA550A3F11MD_INVERTER_STEPOUT_DET_CURR_LEV		60*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA550A3F11MD_INVERTER_STEPOUT_DET_TIME			0.07*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#endif
   #define K_DA550A3F11MD_INVERTER_QAXIS_INDUCTANCE			9.00*100	//??	// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA550A3F11MD_INVERTER_DAXIS_INDUCTANCE			8.30*100	//??	// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry
   //#define K_DA550A3F11MD_INVERTER_PM_CONTROL_SELECTION		3*1		//?????	NON SPECIFICATI	// F915	915	PM control selection			Unit: 1
   #define K_DA550A3F11MD_INVERTER_FACTORY_SPEC_COEFF9F		50*1            // F916	916	PM control selection			Unit: 1
   //#define K_DA550A3F11MD_INVERTER_FACTORY_SPEC_COEFF9L		100*1	//?????	NON SPECIFICATI		// F921	921	Factory specific coefficient 9L	Unit: 1%       


//K_MotorType_DA640A3F-20MAD
   #define K_DA640A3F20MAD_INVERTER_MAX_FREQUENCY			240*100			// FH	11 Max Frequency					Unit: 0.01Hz											
   #define K_DA640A3F20MAD_INVERTER_UPPER_LIM_FREQ			240*100			// UL	12	Upper limit frequency			Unit: 0.01Hz
   #define K_DA640A3F20MAD_INVERTER_BASE_FREQUENCY			120*100			// vL	14	Base frequency 1				Unit: 0.01Hz
#if (K_SIMULATION_WORK==1)
   #define K_DA640A3F20MAD_INVERTER_VF_CONTROL_MODE			0				// Pt	15	V/F control mode selection		Unit: 1
#else
   #define K_DA640A3F20MAD_INVERTER_VF_CONTROL_MODE			6*1				// Pt	15	V/F control mode selection		Unit: 1
#endif
   #define K_DA640A3F20MAD_INVERTER_AUTO_START_FREQ			25*100			// F241	Frequenza avviamento automatico		Unit: 0.01Hz
   #define K_DA640A3F20MAD_INVERTER_BRAKE_START_FREQ		25*100			// F250	250	DC braking starting frequency	Unit: 0.01Hz
   #define K_DA640A3F20MAD_INVERTER_BRAKE_CURRENT_SET		0*1             // F251	Corrente di frenatura in % della corrente nominale inverter (0..100%) Unit: 1%
   #define K_DA640A3F20MAD_INVERTER_BRAKE_TIME				0*10			// F252	252	DC braking time					Unit: 0.1Sec
   #define K_DA640A3F20MAD_INVERTER_PWM_CARRIER				12*10//4*10			// F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz
   #define K_DA640A3F20MAD_INVERTER_AUTORESTART_CONTROL		4*1				// F301	301	Auto-restart control selection	Unit: 1
   #define K_DA640A3F20MAD_INVERTER_SUPPLY_VOLTAGE_CORRECTION	1				// F307 Supply voltage correction Default=2 Unit: 0,1,2,3
   #define K_DA640A3F20MAD_INVERTER_AUTOTUNING				0*1				// F400	400	Autotuning						Unit: 1
   #define K_DA640A3F20MAD_INVERTER_AUTO_TORQUE_BOOST_VAL	1.0*10//1.2*10		//??	// F402	402	Automatic torque boost value	Unit: 0.10%
   #define K_DA640A3F20MAD_INVERTER_MOTOR_RATED_CAPACITY	5.7*100		// F405	405	Motor rated capacity			Unit: 0.01Kw
   #define K_DA640A3F20MAD_INVERTER_BASE_FREQ_VOLTAGE		179*10			// vLv	409	Base frequency voltage 1		Unit: 0.1V
   #define K_DA640A3F20MAD_INVERTER_MOTOR_RATED_CURRENT		21.6*100         // F415	415	Motor rated current				Unit: 0.01A
   #define K_DA640A3F20MAD_INVERTER_MOTOR_RATED_SPEED		3600			// F417	417	Motor rated speed				Unit: 1min-1 ******????
   #define K_DA640A3F20MAD_INVERTER_MOTOR_SPECIFIC_COEFF2	25*1			// F458	458	Motor specific coefficient 2	Unit: 1
   #define K_DA640A3F20MAD_INVERTER_LOAD_INERTIA_MOMENT		0.5*10			// F459	459	Load inertia moment ratio		Unit: 0.1
   #define K_DA640A3F20MAD_INVERTER_MOTOR_SPECIFIC_COEFF3	2*10			// F460	460	Motor specific coefficient 3	Unit: 0.1
   #define K_DA640A3F20MAD_INVERTER_MOTOR_SPECIFIC_COEFF9	100*1			// F495	495	Motor specific coefficient 9	Unit: 1%
   #define K_DA640A3F20MAD_INVERTER_ACCELERATION_TIME2		120*10			// F500	500	Acceleration time 2				Unit: 0.1Sec
   #define K_DA640A3F20MAD_INVERTER_DECELERATION_TIME2		120*10			// F501	501	Deceleration time 2				Unit: 0.1Sec
   #define K_DA640A3F20MAD_INVERTER_ACC_DEC_1e2_SW_FREQ		25*100			// F505	505	Acceleration/deceleration 1 and 2 switching frequency	Unit: 0.01Hz
   #define K_DA640A3F20MAD_INVERTER_STALL_PREVENTION_L1		150*1			// F601	601	Stall prevention level 1		Unit: 1%
   #define K_DA640A3F20MAD_INVERTER_RIL_FASE_SELECTION		0				// F608	0 Selezione rilevamento fase ingresso	Unit: ??
   #define K_DA640A3F20MAD_INVERTER_UNIT_SELECTION			0//1				// F701 Selezione unità corrente/tensione		Unit: ?? Dspu???????
   #define K_DA640A3F20MAD_INVERTER_TIMEOUT_COMUNICATION	6*10			// F803 Time-out tempo di comunicazione		Unit: 0.1s
#if (K_SIMULATION_WORK==1)
   #define K_DA640A3F20MAD_INVERTER_ACTION_TMT_COMUNICATION	0				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#else
   #define K_DA640A3F20MAD_INVERTER_ACTION_TMT_COMUNICATION	2				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#endif
//   #define K_DA640A3F20MAD_INVERTER_FACTORY_SPEC_COEFF9P		0*1         //* non esiste!!!!			// F908	908	Factory specific coefficient 9P	Unit: 1%
   #define K_DA640A3F20MAD_INVERTER_FACTORY_SPEC_COEFF9D	20*1			// F909	909	Factory specific coefficient 9D	Unit: 1%
#if (K_STEPOUT_DISABLE==1)
   #define K_DA640A3F20MAD_INVERTER_STEPOUT_DET_CURR_LEV		100*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA640A3F20MAD_INVERTER_STEPOUT_DET_TIME			0.00*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#else
   #define K_DA640A3F20MAD_INVERTER_STEPOUT_DET_CURR_LEV	100*100//30*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA640A3F20MAD_INVERTER_STEPOUT_DET_TIME		0.07*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#endif
   #define K_DA640A3F20MAD_INVERTER_QAXIS_INDUCTANCE		4.50*100	//??	// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA640A3F20MAD_INVERTER_DAXIS_INDUCTANCE		3.56*100	//??	// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA640A3F20MAD_INVERTER_PM_CONTROL_SELECTION	3*1		//?????	NON SPECIFICATI	// F915	915	PM control selection			Unit: 1
   #define K_DA640A3F20MAD_INVERTER_FACTORY_SPEC_COEFF9F	50*1            // F916	916	PM control selection			Unit: 1
   #define K_DA640A3F20MAD_INVERTER_FACTORY_SPEC_COEFF9L	100*1	//?????	NON SPECIFICATI		// F921	921	Factory specific coefficient 9L	Unit: 1%       

//K_MotorType_DA150A1F21N
   #define K_DA150A1F21N_INVERTER_MAX_FREQUENCY				240*100			// FH	11 Max Frequency					Unit: 0.01Hz											
   #define K_DA150A1F21N_INVERTER_UPPER_LIM_FREQ			240*100			// UL	12	Upper limit frequency			Unit: 0.01Hz
   #define K_DA150A1F21N_INVERTER_BASE_FREQUENCY			120*100			// vL	14	Base frequency 1				Unit: 0.01Hz
#if (K_SIMULATION_WORK==1)
   #define K_DA150A1F21N_INVERTER_VF_CONTROL_MODE			0				// Pt	15	V/F control mode selection		Unit: 1
#else
   #define K_DA150A1F21N_INVERTER_VF_CONTROL_MODE			6*1				// Pt	15	V/F control mode selection		Unit: 1
#endif
	#define K_DA150A1F21N_INVERTER_AUTO_START_FREQ			25*100			// F241	Frequenza avviamento automatico		Unit: 0.01Hz
   #define K_DA150A1F21N_INVERTER_BRAKE_START_FREQ			24*100			// F250	250	DC braking starting frequency	Unit: 0.01Hz
   #define K_DA150A1F21N_INVERTER_BRAKE_CURRENT_SET			100*1			// F251	Corrente di frenatura in % della corrente nominale inverter (0..100%) Unit: 1%
   #define K_DA150A1F21N_INVERTER_BRAKE_TIME				0*10			// F252	252	DC braking time					Unit: 0.1Sec
   #define K_DA150A1F21N_INVERTER_PWM_CARRIER				4*10			// F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz
   #define K_DA150A1F21N_INVERTER_AUTORESTART_CONTROL		4*1				// F301	301	Auto-restart control selection	Unit: 1
   #define K_DA150A1F21N_INVERTER_AUTOTUNING				0*1				// F400	400	Autotuning						Unit: 1
   #define K_DA150A1F21N_INVERTER_AUTO_TORQUE_BOOST_VAL		1.5*10			// F402	402	Automatic torque boost value	Unit: 0.10%
   #define K_DA150A1F21N_INVERTER_MOTOR_RATED_CAPACITY		1*100			// F405	405	Motor rated capacity			Unit: 0.01Kw
   #define K_DA150A1F21N_INVERTER_BASE_FREQ_VOLTAGE			129*10			// vLv	409	Base frequency voltage 1		Unit: 0.1V
   #define K_DA150A1F21N_INVERTER_MOTOR_RATED_CURRENT		4.8*100			// F415	415	Motor rated current				Unit: 0.01A
   #define K_DA150A1F21N_INVERTER_MOTOR_RATED_SPEED			3600			// F417	417	Motor rated speed				Unit: 1min-1 ******????
   #define K_DA150A1F21N_INVERTER_MOTOR_SPECIFIC_COEFF2		25*1			// F458	458	Motor specific coefficient 2	Unit: 1
   #define K_DA150A1F21N_INVERTER_LOAD_INERTIA_MOMENT		1*10			// F459	459	Load inertia moment ratio		Unit: 0.1
   #define K_DA150A1F21N_INVERTER_MOTOR_SPECIFIC_COEFF3		2*10			// F460	460	Motor specific coefficient 3	Unit: 0.1
   #define K_DA150A1F21N_INVERTER_MOTOR_SPECIFIC_COEFF9		100*1			// F495	495	Motor specific coefficient 9	Unit: 1%
   #define K_DA150A1F21N_INVERTER_ACCELERATION_TIME2		120*10			// F500	500	Acceleration time 2				Unit: 0.1Sec
   #define K_DA150A1F21N_INVERTER_DECELERATION_TIME2		120*10			// F501	501	Deceleration time 2				Unit: 0.1Sec
   #define K_DA150A1F21N_INVERTER_ACC_DEC_1e2_SW_FREQ		24*100			// F505	505	Acceleration/deceleration 1 and 2 switching frequency	Unit: 0.01Hz
   #define K_DA150A1F21N_INVERTER_STALL_PREVENTION_L1		150*1	//* 8.10		// F601	601	Stall prevention level 1		Unit: 1%
   #define K_DA150A1F21N_INVERTER_RIL_FASE_SELECTION		0				// F608	0 Selezione rilevamento fase ingresso	Unit: ??
   #define K_DA150A1F21N_INVERTER_UNIT_SELECTION			0//1				// F701 Selezione unità corrente/tensione		Unit: ?? Dspu???????
   #define K_DA150A1F21N_INVERTER_TIMEOUT_COMUNICATION		6*10			// F803 Time-out tempo di comunicazione		Unit: 0.1s
#if (K_SIMULATION_WORK==1)
   #define K_DA150A1F21N_INVERTER_ACTION_TMT_COMUNICATION	0				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#else
   #define K_DA150A1F21N_INVERTER_ACTION_TMT_COMUNICATION	2				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#endif
//   #define K_DA150A1F21N_INVERTER_FACTORY_SPEC_COEFF9P		0*1		//* non esiste!!!!			// F908	908	Factory specific coefficient 9P	Unit: 1%
   #define K_DA150A1F21N_INVERTER_FACTORY_SPEC_COEFF9D		20*1			// F909	909	Factory specific coefficient 9D	Unit: 1%
#if (K_STEPOUT_DISABLE==1)
   #define K_DA150A1F21N_INVERTER_STEPOUT_DET_CURR_LEV		100*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA150A1F21N_INVERTER_STEPOUT_DET_TIME			0.00*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#else
   #define K_DA150A1F21N_INVERTER_STEPOUT_DET_CURR_LEV		30*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA150A1F21N_INVERTER_STEPOUT_DET_TIME			0.07*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#endif
   #define K_DA150A1F21N_INVERTER_QAXIS_INDUCTANCE			11.38*100		// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA150A1F21N_INVERTER_DAXIS_INDUCTANCE			9.79*100		// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA150A1F21N_INVERTER_PM_CONTROL_SELECTION		3*1				// F915	915	PM control selection			Unit: 1
   #define K_DA150A1F21N_INVERTER_FACTORY_SPEC_COEFF9L		100*1			// F921	921	Factory specific coefficient 9L	Unit: 1%       

//K_MotorType_DA130A1F25F3
   #define K_DA130A1F25F3_INVERTER_MAX_FREQUENCY			240*100			// FH	11 Max Frequency					Unit: 0.01Hz											
   #define K_DA130A1F25F3_INVERTER_UPPER_LIM_FREQ			240*100			// UL	12	Upper limit frequency			Unit: 0.01Hz
   #define K_DA130A1F25F3_INVERTER_BASE_FREQUENCY			120*100			// vL	14	Base frequency 1				Unit: 0.01Hz
#if (K_SIMULATION_WORK==1)
   #define K_DA130A1F25F3_INVERTER_VF_CONTROL_MODE			0				// Pt	15	V/F control mode selection		Unit: 1
#else
   #define K_DA130A1F25F3_INVERTER_VF_CONTROL_MODE			6*1				// Pt	15	V/F control mode selection		Unit: 1
#endif
	#define K_DA130A1F25F3_INVERTER_AUTO_START_FREQ			25*100			// F241	Frequenza avviamento automatico		Unit: 0.01Hz
   #define K_DA130A1F25F3_INVERTER_BRAKE_START_FREQ			24*100			// F250	250	DC braking starting frequency	Unit: 0.01Hz
   #define K_DA130A1F25F3_INVERTER_BRAKE_CURRENT_SET		100*1			// F251	Corrente di frenatura in % della corrente nominale inverter (0..100%) Unit: 1%
   #define K_DA130A1F25F3_INVERTER_BRAKE_TIME				0*10			// F252	252	DC braking time					Unit: 0.1Sec
   #define K_DA130A1F25F3_INVERTER_PWM_CARRIER				4*10			// F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz
   #define K_DA130A1F25F3_INVERTER_AUTORESTART_CONTROL		4*1				// F301	301	Auto-restart control selection	Unit: 1
   #define K_DA130A1F25F3_INVERTER_AUTOTUNING				0*1				// F400	400	Autotuning						Unit: 1
   #define K_DA130A1F25F3_INVERTER_AUTO_TORQUE_BOOST_VAL	1.2*10			// F402	402	Automatic torque boost value	Unit: 0.10%
   #define K_DA130A1F25F3_INVERTER_MOTOR_RATED_CAPACITY		1*100			// F405	405	Motor rated capacity			Unit: 0.01Kw
   #define K_DA130A1F25F3_INVERTER_BASE_FREQ_VOLTAGE		110*10			// vLv	409	Base frequency voltage 1		Unit: 0.1V
   #define K_DA130A1F25F3_INVERTER_MOTOR_RATED_CURRENT		4.8*100			// F415	415	Motor rated current				Unit: 0.01A
   #define K_DA130A1F25F3_INVERTER_MOTOR_RATED_SPEED		3600			// F417	417	Motor rated speed				Unit: 1min-1 ******????
   #define K_DA130A1F25F3_INVERTER_MOTOR_SPECIFIC_COEFF2	25*1			// F458	458	Motor specific coefficient 2	Unit: 1
   #define K_DA130A1F25F3_INVERTER_LOAD_INERTIA_MOMENT		1*10			// F459	459	Load inertia moment ratio		Unit: 0.1
   #define K_DA130A1F25F3_INVERTER_MOTOR_SPECIFIC_COEFF3	2*10			// F460	460	Motor specific coefficient 3	Unit: 0.1
   #define K_DA130A1F25F3_INVERTER_MOTOR_SPECIFIC_COEFF9	100*1			// F495	495	Motor specific coefficient 9	Unit: 1%
   #define K_DA130A1F25F3_INVERTER_ACCELERATION_TIME2		120*10			// F500	500	Acceleration time 2				Unit: 0.1Sec
   #define K_DA130A1F25F3_INVERTER_DECELERATION_TIME2		120*10			// F501	501	Deceleration time 2				Unit: 0.1Sec
   #define K_DA130A1F25F3_INVERTER_ACC_DEC_1e2_SW_FREQ		24*100			// F505	505	Acceleration/deceleration 1 and 2 switching frequency	Unit: 0.01Hz
   #define K_DA130A1F25F3_INVERTER_STALL_PREVENTION_L1		150*1	//* 8.10		// F601	601	Stall prevention level 1		Unit: 1%
   #define K_DA130A1F25F3_INVERTER_RIL_FASE_SELECTION		0				// F608	0 Selezione rilevamento fase ingresso	Unit: ??
   #define K_DA130A1F25F3_INVERTER_UNIT_SELECTION			0//1				// F701 Selezione unità corrente/tensione		Unit: ?? Dspu???????
   #define K_DA130A1F25F3_INVERTER_TIMEOUT_COMUNICATION		6*10			// F803 Time-out tempo di comunicazione		Unit: 0.1s
#if (K_SIMULATION_WORK==1)
   #define K_DA130A1F25F3_INVERTER_ACTION_TMT_COMUNICATION	0				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#else
   #define K_DA130A1F25F3_INVERTER_ACTION_TMT_COMUNICATION	2				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#endif
//   #define K_DA130A1F25F3_INVERTER_FACTORY_SPEC_COEFF9P		0*1		//* non esiste!!!!			// F908	908	Factory specific coefficient 9P	Unit: 1%
   #define K_DA130A1F25F3_INVERTER_FACTORY_SPEC_COEFF9D		20*1			// F909	909	Factory specific coefficient 9D	Unit: 1%
#if (K_STEPOUT_DISABLE==1)
   #define K_DA130A1F25F3_INVERTER_STEPOUT_DET_CURR_LEV		100*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA130A1F25F3_INVERTER_STEPOUT_DET_TIME			0.00*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#else
   #define K_DA130A1F25F3_INVERTER_STEPOUT_DET_CURR_LEV		30*100			// F910	910	Step-out detection current level	Unit: 0.01%
   #define K_DA130A1F25F3_INVERTER_STEPOUT_DET_TIME			0.07*100		// F911	911	Step-out detection time			Unit: 0.01Sec
#endif
   #define K_DA130A1F25F3_INVERTER_QAXIS_INDUCTANCE			7.88*100		// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA130A1F25F3_INVERTER_DAXIS_INDUCTANCE			6.68*100		// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry
   #define K_DA130A1F25F3_INVERTER_PM_CONTROL_SELECTION		3*1				// F915	915	PM control selection			Unit: 1
   #define K_DA130A1F25F3_INVERTER_FACTORY_SPEC_COEFF9L		100*1			// F921	921	Factory specific coefficient 9L	Unit: 1%       

   #define K_GENERAL_INVERTER_STEPOUT_DISABLE				0*100			// F911	911	Step-out detection time			Unit: 0.01Sec
   #define K_GENERAL_INVERTER_STEPOUT_ENABLE				0.07*100		// F911	911	Step-out detection time			Unit: 0.01Sec

   #define K_GENERAL_INVERTER_UNDERVOLTAGE_ERROR            1*1             // F627 627 

//-------------------------------------------------------
// Valori di default per inizializzazione Inverter Pompa
//-------------------------------------------------------
//Default
   //#define K_PUMP_DEFAULT_INVERTER_TIME_ACC                         10*10//35*10			// ACC	9 Tempo di accelerazione 1			Unit: 0.1Hz											
   //#define K_PUMP_DEFAULT_INVERTER_TIME_DEC                         10*10//35*10			// DEC	10 Tempo di	decelerazione 1			Unit: 0.1Hz											

   #define K_PUMP_DEFAULT_INVERTER_MAX_FREQUENCY					50*100//55*100			// FH	11 Max Frequency					Unit: 0.01Hz											
   #define K_PUMP_DEFAULT_INVERTER_UPPER_LIM_FREQ					50*100//			// UL	12	Upper limit frequency			Unit: 0.01Hz
   #define K_PUMP_DEFAULT_INVERTER_BASE_FREQUENCY					50*100			// vL	14	Base frequency 1				Unit: 0.01Hz
   #define K_PUMP_DEFAULT_INVERTER_VF_CONTROL_MODE					0				// Pt	15	V/F control mode selection		Unit: 1

   #define K_PUMP_DEFAULT_INVERTER_AUTO_START_FREQ					15*100			// F241	Frequenza avviamento automatico		Unit: 0.01Hz

   #define K_PUMP_DEFAULT_INVERTER_PWM_CARRIER      				4*10			// F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz

//#define K_PUMP_DEFAULT_INVERTER_MOTOR_RATED_CAPACITY			1.5*100			// F405	405	Motor rated capacity			Unit: 0.01Kw
   #define K_PUMP_DEFAULT_INVERTER_BASE_FREQ_VOLTAGE				230*10			// vLv	409	Base frequency voltage 1		Unit: 0.1V
   //#define K_PUMP_DEFAULT_INVERTER_MOTOR_RATED_CURRENT			6.5*100		// F415	415	Motor rated current				Unit: 0.01A
   #define K_PUMP_DEFAULT_INVERTER_MOTOR_RATED_SPEED				2800			// F417	417	Motor rated speed				Unit: 1min-1 ******????
   //#define K_PUMP_DEFAULT_INVERTER_INVERTER_STALL_PREVENTION_L1	92*1			// F601	601	Stall prevention level 1		Unit: 1%
   #define K_PUMP_DEFAULT_INVERTER_RIL_FASE_SELECTION               0				// F608	0 Selezione rilevamento fase ingresso	Unit: ??

   #define K_PUMP_DEFAULT_INVERTER_TIMEOUT_COMUNICATION             6*10			// F803 Time-out tempo di comunicazione		Unit: 0.1s
#if (K_SIMULATION_WORK==1)
   #define K_PUMP_DEFAULT_INVERTER_ACTION_TMT_COMUNICATION			0				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#else
   #define K_PUMP_DEFAULT_INVERTER_ACTION_TMT_COMUNICATION			2				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#endif

//MPH_441_FGACE5
   #define K_PUMP_MPH_441_FGACE5_INVERTER_MAX_FREQUENCY				50*100			// FH	11 Max Frequency					Unit: 0.01Hz											
   #define K_PUMP_MPH_441_FGACE5_INVERTER_UPPER_LIM_FREQ			50*100			// UL	12	Upper limit frequency			Unit: 0.01Hz
   #define K_PUMP_MPH_441_FGACE5_INVERTER_BASE_FREQUENCY			50*100			// vL	14	Base frequency 1				Unit: 0.01Hz
   #define K_PUMP_MPH_441_FGACE5_INVERTER_VF_CONTROL_MODE			0				// Pt	15	V/F control mode selection		Unit: 1

   #define K_PUMP_MPH_441_FGACE5_INVERTER_AUTO_START_FREQ			15*100			// F241	Frequenza avviamento automatico		Unit: 0.01Hz

   #define K_PUMP_MPH_441_FGACE5_INVERTER_PWM_CARRIER               4*10			// F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz

   #define K_PUMP_MPH_441_FGACE5_INVERTER_MOTOR_RATED_CAPACITY      0.75*100		// F405	405	Motor rated capacity			Unit: 0.01Kw
   #define K_PUMP_MPH_441_FGACE5_INVERTER_BASE_FREQ_VOLTAGE			220*10			// vLv	409	Base frequency voltage 1		Unit: 0.1V
   #define K_PUMP_MPH_441_FGACE5_INVERTER_MOTOR_RATED_CURRENT		3.4*100         // F415	415	Motor rated current				Unit: 0.01A
   #define K_PUMP_MPH_441_FGACE5_INVERTER_MOTOR_RATED_SPEED			2800			// F417	417	Motor rated speed				Unit: 1min-1 ******????
   //#define K_PUMP_MPH_441_FGACE5_INVERTER_INVERTER_STALL_PREVENTION_L1	92*1			// F601	601	Stall prevention level 1		Unit: 1%

//MPH_453_FGACE5
   #define K_PUMP_MPH_453_FGACE5_INVERTER_MAX_FREQUENCY				50*100			// FH	11 Max Frequency					Unit: 0.01Hz											
   #define K_PUMP_MPH_453_FGACE5_INVERTER_UPPER_LIM_FREQ			50*100			// UL	12	Upper limit frequency			Unit: 0.01Hz
   #define K_PUMP_MPH_453_FGACE5_INVERTER_BASE_FREQUENCY			50*100			// vL	14	Base frequency 1				Unit: 0.01Hz
   #define K_PUMP_MPH_453_FGACE5_INVERTER_VF_CONTROL_MODE			0				// Pt	15	V/F control mode selection		Unit: 1

   #define K_PUMP_MPH_453_FGACE5_INVERTER_AUTO_START_FREQ			15*100			// F241	Frequenza avviamento automatico		Unit: 0.01Hz

   #define K_PUMP_MPH_453_FGACE5_INVERTER_PWM_CARRIER               4*10			// F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz

   #define K_PUMP_MPH_453_FGACE5_INVERTER_MOTOR_RATED_CAPACITY      2.2*100 		// F405	405	Motor rated capacity			Unit: 0.01Kw
   #define K_PUMP_MPH_453_FGACE5_INVERTER_BASE_FREQ_VOLTAGE			220*10			// vLv	409	Base frequency voltage 1		Unit: 0.1V
   #define K_PUMP_MPH_453_FGACE5_INVERTER_MOTOR_RATED_CURRENT		8.3*100         // F415	415	Motor rated current				Unit: 0.01A
   #define K_PUMP_MPH_453_FGACE5_INVERTER_MOTOR_RATED_SPEED			2895			// F417	417	Motor rated speed				Unit: 1min-1 ******????
   //#define K_PUMP_MPH_453_FGACE5_INVERTER_INVERTER_STALL_PREVENTION_L1	92*1			// F601	601	Stall prevention level 1		Unit: 1%

	//----------------------------------
	// Strutture
	//----------------------------------
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
                 int    Filtered_Out_Current;
                 int    Highest_Filtered_Out_Current;
				 int	StatusReg;		// registro letto dall'inverter
				 int	AlarmReg;		// registro letto dall'inverter
		unsigned int	StepOutCurrentWarningDB;
		unsigned int	StepOutCurrentWarningWork;
		
		unsigned int	Power_Inverter;				// Potenza Taglia Inverter						Watt
		unsigned int	PowerMax_Available;			// Valore limitazione Potenza totale			Watt
		unsigned int	Power_EcoMode;	
		unsigned int	DiagAddress;
		unsigned int	Ok_Voltage;        
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
		unsigned int	EcoModeEnable;		// Flag per abilitazione limitazione Potenza in ECO Mode		
		unsigned		OffLine:1;					// se non ha risposto
		unsigned		PermanentOffLine:1;			// se non ha risposto per xx volte (Timeout)        
        
	} TypInverter;
	
	/*
	typedef struct
	{		
			unsigned int EngineCompSpeed;				// REG_BRIDGE1_UTA_H_ENGINE_COMPRESSOR_SPEED			100 //160				// "UTA H Engine Box Inverter Compressor Speed: (percentage)		Range: 0..100%"                                  
			unsigned int EngineFanSpeed;				// REG_BRIDGE1_UTA_H_ENGINE_WATER_PUMP_SPEED			101 //161				// "UTA H Engine Box Inverter Water Pump Speed: (percentage)		Range: 0..100%"                                  
			unsigned int EngineReqPressure;				// REG_BRIDGE1_UTA_H_ENGINE_REQ_PRESSURE				102 //162				// UTA H Engine Box Max Air Handler Required Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar                  
			int			 EngineTempOut;					// REG_BRIDGE1_UTA_H_ENGINE_T_OUTPUT					103 //163				// UTA H Engine Box Output Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C                         
			int			 EngineTempIn;					// REG_BRIDGE1_UTA_H_ENGINE_T_SUCTION					104 //164				// UTA H Engine Box Suction Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C                        
			int			 EngineTempAir;					// REG_BRIDGE1_UTA_H_ENGINE_T_AIR						105 //165				// UTA H Engine Box Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C                            
			int			 EngineTempBatt;				// REG_BRIDGE1_UTA_H_ENGINE_T_BATTERY					106 //166				// UTA H Engine Box Battery Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C                        
			int			 EngineTempGas;					// REG_BRIDGE1_UTA_H_ENGINE_T_GAS						107 //167				// UTA H Engine Box Gas Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C                            
			int			 EngineTempLiq;					// REG_BRIDGE1_UTA_H_ENGINE_T_LIQUID					108 //168				// UTA H Engine Box Liquid Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C                         
			unsigned int EnginePressGas;				// REG_BRIDGE1_UTA_H_ENGINE_P_GAS						109 //169				// UTA H Engine Box Gas Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar                                       
			unsigned int EnginePressLiq;				// REG_BRIDGE1_UTA_H_ENGINE_P_LIQUID					110 //170				// UTA H Engine Box Liquid Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar                                    
			unsigned int EngineBoxErr1;					// REG_BRIDGE1_UTA_H_ENGINE_BOX_ERROR1					111 //171				// "UTA H Engine Box Error/Status Flags 1:                                                                               
			unsigned int EngineBoxPersErr1;				// REG_BRIDGE1_UTA_H_ENGINE_BOX_PERSERROR1				112 //172				// "UTA H Engine Box Persistent Error Flags 2:                                                                           
			unsigned int EngineBoxSplitErr1;			// REG_BRIDGE1_UTA_H_ENGINE_BOX_SPLITERROR1				113 //172				// "UTA H Engine Box Persistent Error Flags 2:                                                                           
			unsigned int Absorption;					// REG_BRIDGE1_UTA_H_ABSORPTION							114 //188				// Instantaneous absorption control of the UTA (Ampere)                                                              
			unsigned int ClearErrorBox;					// REG_BRIDGE1_UTA_H_CLEAR_ERROR						10
			unsigned int ClearErrorSplit;				// REG_BRIDGE1_UTA_H_CLEAR_SPLITERROR1					11
			unsigned int CumulativeAlarm;				// REG_BRIDGE1_UTA_H_CUMULATIVE_ALARM					115
		
            unsigned int UTASP_On;
            
			unsigned int HW_Ver;						// REG_UTA_H_READ_BOX_HW_VERS							129		
			unsigned int FW_Ver_HW;						// REG_UTA_H_READ_BOX_FW_VERS_HW						130		
			unsigned int FW_Ver_FW;						// REG_UTA_H_READ_BOX_FW_VERS_FW						131		
			unsigned int FW_Rev_FW;						// REG_UTA_H_READ_BOX_FW_REV_FW							132	

			unsigned int StatoCompressore;				// REG_UTA_H_ENGINE_BOX_COMP_STATUS						134 
			unsigned int IdMasterSplit;					// REG_UTA_H_ENGINE_BOX_ID_MAS_SPLIT					135 
			unsigned int Mode;							// REG_UTA_H_ENGINE_BOX_ENGINE_MODE						136 
			
			struct 
			{			
				int			 LiquidTemp;				// REG_UTA_H_READ_LIQ_TEMP								116		
				int			 GasTemp;					// REG_UTA_H_READ_GAS_TEMP								117	
				int			 AirInTemp;					// REG_UTA_H_READ_AIR_IN								118		
				int			 AitOutTemp;				// REG_UTA_H_READ_AIR_OUT								119		
				unsigned int Exp_Valve;					// REG_UTA_H_READ_EXP_VALVE								120		
				unsigned int Humi;						// REG_UTA_H_READ_HUMI									121		
				int			 ReqAirTemp;				// REG_UTA_H_READ_REQUEST_AIR_TEMP						122		
				unsigned int ReqGasPress;				// REG_UTA_H_READ_REQUEST_GAS_PRES						123		
				int			 Superheat;					// REG_UTA_H_READ_SUPERHEAT								124		
				unsigned int HW_Ver;					// REG_UTA_H_READ_SPLIT_HW_VERS							125		
				unsigned int FW_Ver_HW;					// REG_UTA_H_READ_SPLIT_FW_VERS_HW						126		
				unsigned int FW_Ver_FW;					// REG_UTA_H_READ_SPLIT_FW_VERS_FW						127		
				unsigned int FW_Rev_FW;					// REG_UTA_H_READ_SPLIT_FW_REV_FW						128			
			} Split;
    struct 
			{			
				int			 PowerOn;				
				int			 DefMode;					
				int			 SetPoint;	
				int			 SetUta;
				int			 FanSpeed;		
			} SetRoom;			
	} TypUTAH;	
	*/

	typedef struct
	{
		unsigned		Enable:1;					// se è abilitato
		unsigned		OffLine:1;					// se non ha risposto
		unsigned		OnLine:1;					// 
		unsigned		BridgeReady:1;				// 
        unsigned		InitializeBridge:1;			// 
		unsigned		PermanentOffLine:1;			// se non ha risposto entro "OffLineCounter"
		unsigned int	OffLineCounter;				// Numero di volte che non lo devo vedere Online per definirlo Offline
		
		struct 
			{			
				int			 Power;				
				int			 DefMode;					
				int			 SetPoint;					
				int			 FanSpeed;				
				unsigned int TrigInit;						
			} Reinit;		

			struct 
			{			
				int			 PowerOn;				
				int			 DefMode;					
				int			 SetPoint;					
				int			 FanSpeed;				
				int			 HeaterPwr;              
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

			struct 
			{			
				int			 SystemDisable;				
			} EngineBox;				
					
	} TypBridge;		

	
		typedef struct
	{			 
                 int    Page;
                 int	SetPoint;
				 int	FunctionMode;
				 int	FanMode;
				 int	Current_Page;			// pagina corrente del touch
				 int	DefMode;
                 int    Split_Pwr;
                 int    Split_Mode;
			unsigned 	Enable:1;				// se è abilitato
			unsigned	OffLine:1;				// se non ha risposto
            unsigned	PermanentOffline:1;		// Gestione Errore di comunicazione Touch
	} TypTouch;
	
	//---------------------------------------
	// Variabili 
	//---------------------------------------
	// Timer 
    
#define VariabiliGlobaliProtocolloModBus()      \
	TypInverter Comp_Inverter[K_Max_Inverter];  \
	char Result = 0;                            \
	char RetryCnt = 0;                          \
	char Indice = 0;                            \
	int Data = 0;                               \
	char i=0;                                   \
    volatile TypTimer TimeOutModPktRx;          \
    volatile TypBridge Bridge[2];               \
    volatile TypTouch Touch;		


#define IncludeVariabiliGlobaliProtocolloModBus()\
	extern TypInverter Comp_Inverter[K_Max_Inverter];   \
	extern char Result;                                 \
	extern char RetryCnt;                               \
	extern char Indice;                                 \
	extern int Data;                                    \
	extern char i;                                      \
    extern volatile TypTimer TimeOutModPktRx;           \
    extern volatile TypBridge Bridge[2];                \
    extern volatile TypTouch Touch;		

    
    IncludeVariabiliGlobaliProtocolloModBus();     
	


	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	unsigned char WaitModRx(int TimeOut_ms);
	unsigned char Send_WaitModRx(char Address, char Comando, int Registro, int Data, int TimeOut_ms, char Retry);
	unsigned char Send_WriteModReg(char Address, int Registro, int Data, int TimeOut_ms, char Retry, char Cmd);
	
	void ReadModReg (char Address, int REG, int * StoreReg);
	char ReadModListReg (char DataIndex, int REG, int RegNum);
	void WriteModReg (char Address, int REG, int SendData);
	void WriteModListReg (char Address, int REG, int SendData);
	int ExtracReg(char NumReg);
	void InsertReg(char NumReg, int Data);
	//void InsertReg2(char NumReg, int Data);
	
	void Search_Touch(void);
	//void Resume_TouchOffLine(char * Cnt);
	void Resume_TouchOffLine(void);

	void RefreshTouchInPut(char TouchAddres);
	void RefreshAllTouchInPut(void);

	void RefreshTouchOutPut(char TouchAddres);
	void RefreshAllTouchOutPut(void);

	void InitTouch(void);

	void RefreshInverterComp(int pointer, TypEngineBox * CompNumber, TypInverter * Inverter, int CompAddr, TypTimer * TimerPersError);
	void RefreshInverterPump(TypEngineBox * CompNumber, TypInverter * Inverter, int CompAddr, TypTimer * TimerPersError);
	///void DC_Braking(TypEngineBox * CompNumber, TypInverter * Inverter, int CompAddr, int onoff);
    ///void Set_Braking_Current(TypInverter * Inverter, int CompAddr,int CurrentValuePerc);
	void InitInverterComp(TypInverter * Inverter, int MotorType, int CompAddr);
	void InitInverterPump(TypInverter * Inverter, int MotorType, int CompAddr);	
    void InverterCheckVoltage(int pointer, TypInverter * Inverter, TypTimer * TimerVoltage);

	void Init_Bridge(int bridgeNumber);
	int Check_IfBridgeIsPresent(int bridgeNumber);
	void Send_InitDataToBridge(TypBridge * bridgeFlag, int bridgeNumber);
		
	int SendInitBufferToSideB(void);					// Invio con check dei dati del buffer verso il SideB
	int CheckMasterInitFLag(void);						// Check bit0 Word0 del buffer -> Master Data Init Start
	int CheckMasterSyncFLag(void);						// Check bit1 Word0 del buffer -> Bridge SideB Data Init Start
	void Send_DataToBridge(TypBridge * bridgeFlag, int bridgeNumber);	
	void Get_DataFromBridge(TypBridge * bridgeFlag, int bridgeNumber);	
	//void UpdateNewDataBridge(int SlaveValue, int * BridgeValue);
	//void SyncDataFromBridge(void);
	void WaitEnd_DataFromBridge(TypBridge * bridgeFlag, int bridgeNumber);
	int Check_IfBridgeGoOnline(TypBridge * bridgeFlag, unsigned int bridgeAddr);
	void SendQueryArg(int RoomStart, int Address);			// Son tutti Argomenni con il culo degli altri......
	void InitQueryArg(int RoomStart, int Address);			// Son tutti Argomenni con il culo degli altri......
#endif
