//----------------------------------------------------------------------------------
//	Progect name:	ProtocolloModBusSec.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			18/06/2011
//	Description:	Header delle funzioni che gestiscono la comunicazione ModBus con il sistema
//----------------------------------------------------------------------------------
#include "FWSelection.h"
#ifndef _PROTOCOLLO_MODBUS_SEC_H_
	#define _PROTOCOLLO_MODBUS_SEC_H_
	
	//----------------------------------------------------------------------------------
	//	Define
	//----------------------------------------------------------------------------------
#if(K_EnableModBus_Sec==0)
	#define	Add_TouchRoom						1
#else
	#define	Add_TouchRoom						32
#endif
	#define	Add_NetBRoom						30  	// Indirizzo MODBUS Pannello NetBuilding Mod.MCT4KKT (Address default = 30 -> 0x1E)
	#define	Add_SyxtSRoom						31  	// Indirizzo MODBUS Pannello NetBuilding Mod.MCT4KKT (Address default = 30 -> 0x1E)

    #define k_Broadcast_Addr_Sec                250

	#define	Add_Fan_Inverter					2       // Indirizzo Inverter FAN
    #define	Add_Fan_EBM_ModBus                  10     // Indirizzo Vemtola ModBUS            
    #define	Add_Fan_ZihegAbb_ModBus             247     // Indirizzo Vemtola ModBUS            

    #define K_ModBusZeroBased                   1       // Offset indirizzi per Device MODBUS "ZeroBased" (inizia da Address=0) es.: Touch Carel/Exor e NetB NetBuilding
    #define K_ModBusOneBased                    0       // Offset indirizzi per Device MODBUS "OneBased" (inizia da Address=1) es.: Inverer Thoshiba

	#define MaxModChkRetriesSec					1//3		// check
	#define MaxModChkWaitSec					20//500//100//400		// check

	#define MaxModChkSubRetriesSec              1//3		// check                Check x Schede Subslave
	#define MaxModChkSubWaitSec					20//100//400		// check        Check x Schede Subslave

	#define MaxModReadRetriesSec				3//2		// Read
	#define	MaxModReadWaitSec					500//100//600//1100//100//50		// Read
	#define MaxModWriteRetriesSec				3//2		// Write
	#define	MaxModWriteWaitSec					500//100//600//1100//100//50		// Write

	#define	MaxModWriteBrcRetriesSec            1 		// Broadcast
	#define	MaxModWriteBrcWaitSec               20//5		// Broadcast

	#define	k_ModWaitSendMessage                2200    // Pausa in uS per garantire una minima distanza tra un Tx e il successivo 		                        

	#define k_MAX_Cnt_Err_Touch					10		// N. Max tentativi di comunicazione con Touch Modbus prima di dichiararlo guasto
	#define k_MAX_Cnt_Err_SyxthS				10		// N. Max tentativi di comunicazione con Touch Modbus prima di dichiararlo guasto
	#define k_MAX_Cnt_Err_NetB					5		// N. Max tentativi di comunicazione con Touch Modbus prima di dichiararlo guasto
	
	#define K_DEVICE_MODBUS_NUMBER              3		// Numero MAX di Device Modbus

/*
	#define K_DEVICE_MODBUS_EXOR                1		// Device Modbus = Touch Exor/Carel
	#define K_DEVICE_MODBUS_SYXS                2		// Device Modbus = Touch SyxthSense
	#define K_DEVICE_MODBUS_NETB                3		// Device Modbus = Panel Netbuilding
*/
	// comandi MODBUS
	#define	ReadModCoil							1   //0x01	// Read Coil Status 
	#define	ReadModInputStatus					2   //0x02	// Read Input Status        NEW (Not implemented)
	#define	ReadModCmd							3   //0x03	// Read Holding Register
	#define	ReadModInput						4   //0x04	// Read Input Register
	#define	WriteModForceSingleCoil				5   //0x05	// Force Single Coil        NEW (Not implemented)
	#define	WriteModCmd							6   //0x06	// Preset Single Register
	#define	ReadModStatus                       7   //0x07	// Read Status              NEW (Not implemented)
	#define	WriteModForceMultipleCoil			15  //0x0F	// Force Multiple Coil      NEW (Not implemented)
	#define	WriteModListCmd						16  //0x10	// Preset Multiple register


	// registri touch	(hanno un offset di uno, il tag 1 del touch ha indirizzo 00)
	#define	REG_TOUCH_PAGE						0x0001	//	Global
	#define	REG_TOUCH_PWR						0x0002	// registri di sistema del touch
	#define	REG_TOUCH_MODE						0x0003

	#define	REG_TOUCH_ROOM_TEMP					0x0010	//	2-w     16d
	#define	REG_TOUCH_POWER_MOT					0x0011	//	2-w     17d
	#define	REG_TOUCH_CUMULATIVE_ALARM			0x0012	//	2-w
	#define	REG_TOUCH_SP_ROOM					0x0013	//	2-r
	#define	REG_TOUCH_FAN_SPEED					0x0014	//	2-r
	#define	REG_TOUCH_FUNCTION_MODE				0x0015	//	2-r
	#define	REG_TOUCH_SP_ROOM_F					0x0016	//	2-r     22

	#define	REG_TOUCH_UTA_MODE					0x0017	//	2-r     22


	#define	REG_TOUCH_MAX_PWR_LIMIT				0x0020	//	3-r

	//#define REG_TOUCH_MAX_PWR_LIMIT			0x00030	//	4-r

	#define	REG_TOUCH_MATRICOLA_LO				0x0040	//	5-w
	#define	REG_TOUCH_MATRICOLA_HI				0x0041	//	5-w
	#define	REG_TOUCH_MASTER_FW_VERSION			0x0042	//	5-w
	#define	REG_TOUCH_SYS_ERROR_1				0x0043	//	5-w
	#define	REG_TOUCH_SYS_PERSERR_1				0x0044	//	5-w
	#define	REG_TOUCH_SYS_ERROR_3				0x0045	//	5-w
	#define	REG_TOUCH_SYS_PERSERR_2				0x0046	//	5-w
	#define	REG_TOUCH_SYS_ERROR_2				0x0047	//	5-w


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

	#define	REG_TOUCH_PICCOLO_SPLIT_1			0x0060	//	7-w
	#define	REG_TOUCH_GRANDE_SPLIT_1			0x0061	//	7-w
	#define	REG_TOUCH_ARIA_INGRESSO_SPLIT_1		0x0062	//	7-w
	#define	REG_TOUCH_ARIA_USCITA_SPLIT_1		0x0063	//	7-w
	#define	REG_TOUCH_VENTILATORE_SPLIT_1		0x0064	//	7-w
	#define	REG_TOUCH_STATO_VALVOLA_SPLIT_1		0x0065	//	7-w
	#define	REG_TOUCH_PRESSIONE_G_SPLIT_1		0x0066	//	7-w
	#define	REG_TOUCH_PRESSIONE_P_SPLIT_1		0x0067	//	7-w
	#define	REG_TOUCH_TEMP_EVAP_SPLIT_1			0x0068	//	7-w
	#define	REG_TOUCH_RICH_PRESSIONE_SPLIT_1	0x0069	//	7-w
	#define	REG_TOUCH_ENGINE_BOX_MODE_SPLIT_1	0x006A	//	7-w			Dec. 106
	#define	REG_TOUCH_CYCLE_TIME_SPLIT_1		0x006B	//	7-w			Dec. 107

// Diagnostic Slave data    #v16
	#define	REG_TOUCH_SPLIT_LIQ_TEMP			0x0070	// Dec. 112		w
	#define	REG_TOUCH_SPLIT_GAS_TEMP			0x0071	// Dec. 113		w
	#define	REG_TOUCH_SPLIT_AIR_IN				0x0072	// Dec. 114		w
	#define	REG_TOUCH_SPLIT_AIR_OUT				0x0073	// Dec. 115		w
	#define	REG_TOUCH_SPLIT_FAN_SPEED			0x0074	// Dec. 116		w
	#define	REG_TOUCH_SPLIT_EXP_VALVE			0x0075	// Dec. 117		w
	#define	REG_TOUCH_SPLIT_GAS_PRESSURE		0x0076	// Dec. 118		w
	#define	REG_TOUCH_SPLIT_LIQ_PRESSURE		0x0077	// Dec. 119		w
	#define	REG_TOUCH_SPLIT_REQUEST_AIR_TEMP	0x0078	// Dec. 120		w
	#define	REG_TOUCH_SPLIT_REQUEST_AIR_PRES	0x0079	// Dec. 121		w
	#define	REG_TOUCH_SPLIT_ENG_MODE			0x007A	// Dec. 122		w
	#define	REG_TOUCH_SPLIT_STATUS				0x007B	// Dec. 123		w
	#define	REG_TOUCH_SPLIT_TEST_CYCLE_TIME		0x007C	// Dec. 124		w
	#define	REG_TOUCH_SPLIT_TEST_ADDRESS		0x007D	// Dec. 125		r	

// Diagnostic Inverter data
	#define	REG_TOUCH_INVERTER_ALARM			0x007E	// Dec. 126		w
	#define	REG_TOUCH_INVERTER_OUT_FREQ			0x007F	// Dec. 127		w
	#define	REG_TOUCH_INVERTER_STATUS			0x0080	// Dec. 128		w
	#define	REG_TOUCH_INVERTER_OUT_CURR			0x0081	// Dec. 129		w
	#define	REG_TOUCH_INVERTER_IN_VOLT			0x0082	// Dec. 130		w
	#define	REG_TOUCH_INVERTER_OUT_VOLT			0x0083	// Dec. 131		w	
	#define	REG_TOUCH_INVERTER_IN_POWER			0x0084	// Dec. 132		w	
	#define	REG_TOUCH_INVERTER_OUT_POWER		0x0085	// Dec. 133		w	
	#define	REG_TOUCH_INVERTER_REQ_FREQ			0x0086	// Dec. 134		w
	#define	REG_TOUCH_INVERTER_MAX_REQ_FREQ		0x0087	// Dec. 135		w

	#define	REG_TOUCH_SUPERHEAT         		0x0088	// Dec. 136		w
	#define	REG_TOUCH_SETP_SUPERHEAT            0x0089	// Dec. 137		w
//	#define	REG_TOUCH_KD                 		0x008A	// Dec. 138		w
	#define	REG_TOUCH_CNTVALVEFAULT             0x008B	// Dec. 139		w

	#define	REG_TOUCH_I_AM_MASTER_SPLIT         0x008C	// Dec. 140		w
    #define REG_TOUCH_MAX_POWER_REDUCE          0x008D	// Dec. 141		w
    #define REG_TOUCH_SEA_WATER_PUMP            0x008E	// Dec. 142		w
    #define REG_TOUCH_FLOW_STATE                0x008F	// Dec. 143		w

    #define REG_TOUCH_SPLIT_CLEAR_ERROR         0x0090	// Dec. 144		w

    #define REG_TOUCH_DIAGNOSTIC_CLEAR_ERROR    0x0091	// Dec. 145		w
	#define	REG_TOUCH_CLEAR_ERROR				0x0092	// Dec. 146		w

    #define	REG_TOUCH_SUB_SPLIT_TEST_ADDRESS    0x0093  // Dec. 147		R
    #define	REG_TOUCH_SUB_SPLIT_QUANTITY        0x0094  // Dec. 148		R
    #define	REG_TOUCH_NATION                    0x0095  // Dec. 149		R
	#define	REG_TOUCH_AD5_AUX1                  0x0096  // Dec. 150		R
	#define	REG_TOUCH_AD6_AUX2                  0x0097  // Dec. 151		R

	#define	REG_TOUCH_MASTER_HW_VERSION         0x0098  // Dec. 152		R
	#define	REG_TOUCH_MASTER_FW_VERSION_HW      0x0099  // Dec. 153		R
	#define	REG_TOUCH_MASTER_FW_VERSION_FW      0x009A  // Dec. 154		R
	#define	REG_TOUCH_MASTER_FW_REVISION_FW     0x009B  // Dec. 155		R

	#define	REG_TOUCH_SPLIT_HW_VERSION			0x009C  // Dec. 156		R
	#define	REG_TOUCH_SPLIT_FW_VERSION_HW		0x009D  // Dec. 157		R
	#define	REG_TOUCH_SPLIT_FW_VERSION_FW		0x009E  // Dec. 158		R
	#define	REG_TOUCH_SPLIT_FW_REVISION_FW		0x009F  // Dec. 159		R

	#define	REG_TOUCH_DIAG_SPLIT_HW_VERSION		0x00A0  // Dec. 160		R
	#define	REG_TOUCH_DIAG_SPLIT_FW_VERSION_HW	0x00A1  // Dec. 161		R
	#define	REG_TOUCH_DIAG_SPLIT_FW_VERSION_FW	0x00A2  // Dec. 162		R
	#define	REG_TOUCH_DIAG_SPLIT_FW_REVISION_FW	0x00A3  // Dec. 163		R

//	#define REG_TOUCH_COMMAND_SPLIT_ADDRESS		164		
//	#define REG_TOUCH_MAX_COMMAND_SPLIT_QUANTITY	165		
	#define REG_TOUCH_DIAG_BOX_ADDRESS			166
	#define REG_TOUCH_MAX_DIAG_BOX_QUANTITY		167	
	#define	REG_TOUCH_INVERTER_ADDRESS			168
	#define	REG_TOUCH_INVERTER_QUANTITY			169
	#define	REG_TOUCH_INIT_SP_ROOM				170
	#define	REG_TOUCH_INIT_FAN_SPEED			171
	#define	REG_TOUCH_INIT_POWER				172
	#define	REG_TOUCH_INIT_SP_ROOM_F			173
	#define	REG_TOUCH_TRIGGER_INIT				174
	#define	REG_TOUCH_POWER_PUMP_DIAG			175
	#define	REG_TOUCH_BOX_ABSORPTION			176
	#define REG_TOUCH_ECOMODE_ENABLE			177
	#define REG_TOUCH_POWER_ECOMODE				178
	#define REG_TOUCH_DOUBLE_COMPRESSOR			179			

	#define REG_TOUCH_ECOMODE_STATUS			180

	#define REG_TOUCH_UTA_POWER					181
	#define REG_TOUCH_DIAG_UTA_MODE				182
	#define REG_TOUCH_UTA_SP_ROOM				183
	#define REG_TOUCH_UTA_SP_HUMI				184
	#define REG_TOUCH_UTA_FAN_SPEED				185
	#define REG_TOUCH_UTA_BOX_CLEAR_ERROR		186
	#define REG_TOUCH_UTA_SP_ROOM_F				187
	#define REG_TOUCH_UTA_CUM_ALARM				188
	#define REG_TOUCH_UTA_ABIL					189
	#define REG_TOUCH_UTA_ERROR_1				190
	#define REG_TOUCH_UTA_PERSERROR_1			191
//	#define REG_TOUCH_UTA_SPLIT_ERROR			192
//	#define REG_TOUCH_UTA_SPLIT_CLEAR_ERROR		193

	#define REG_TOUCH_SPLIT_SET_POWER			194
	#define REG_TOUCH_SPLIT_SET_SETPOINT		195
	#define REG_TOUCH_SPLIT_SET_FANSPEED		196
	#define REG_TOUCH_SPLIT_SET_SETPOINT_F		197

	#define REG_TOUCH_SPLIT_CMD_ADDRESS			198

	#define REG_TOUCH_SPLIT_SYSTEM_DISABLE		199

	#define REG_TOUCH_POWER_PUMP_PERC			200

	#define REG_TOUCH_ROOM_ADDRESS				201
	#define REG_TOUCH_MASTER_RESTART_TIME		202

	#define REG_TOUCH_FLAGS_ENABLE_1_16			203
	#define REG_TOUCH_FLAGS_OFFLINE_1_16		204
	#define REG_TOUCH_FLAGS_ONLINE_1_16			205
	#define REG_TOUCH_FLAGS_ALARM_1_16			206
	#define REG_TOUCH_FLAGS_ENABLE_17_32		207
	#define REG_TOUCH_FLAGS_OFFLINE_17_32		208
	#define REG_TOUCH_FLAGS_ONLINE_17_32		209
	#define REG_TOUCH_FLAGS_ALARM_17_32			210

	#define REG_TOUCH_GRAPH_TEMP_OUTPUT_C1		211
    #define REG_TOUCH_GRAPH_TEMP_OUTPUT_C2		212
    #define REG_TOUCH_GRAPH_TEMP_SUCTION		213
    #define REG_TOUCH_GRAPH_TEMP_CONDENSER		214
    #define REG_TOUCH_GRAPH_TEMP_SEA_WATER		215
    #define REG_TOUCH_GRAPH_TEMP_LIQUID 		216
    #define REG_TOUCH_GRAPH_PRES_GAS    		217
    #define REG_TOUCH_GRAPH_PRES_LIQUID    		218
    #define REG_TOUCH_GRAPH_PRES_REQUIRED    	219
    #define REG_TOUCH_GRAPH_SPEED_C1    		220
    #define REG_TOUCH_GRAPH_SPEED_C2    		221
    #define REG_TOUCH_GRAPH_VOLT_IN_C1    		222
    #define REG_TOUCH_GRAPH_CURR_OUT_C1    		223
    #define REG_TOUCH_GRAPH_VOLT_IN_C2    		224
    #define REG_TOUCH_GRAPH_CURR_OUT_C2    		225

	#define	REG_TOUCH_COUNTER_COMUNICATION		226
	#define	REG_TOUCH_DIAG_DATA_PROCESS			227
	#define	REG_TOUCH_BUZZER_ERROR				228

	#define	REG_TOUCH_SUB_SL_FLAGS_ENABLE_1_16	229
	#define	REG_TOUCH_SUB_SL_FLAGS_OFFLINE_1_16	230
	#define	REG_TOUCH_SUB_SL_FLAGS_ONLINE_1_16	231
	#define	REG_TOUCH_SUB_SL_FLAGS_ALARMS_1_16	232

    #define REG_TOUCH_NUMBER_OTHER_CABIN        233
    #define REG_TOUCH_OTHER_CABIN_ST_TEMP       234
    #define REG_TOUCH_OTHER_CABIN_ST_ON_OFF     235
    #define REG_TOUCH_OTHER_CABIN_ST_SETP       236
    #define REG_TOUCH_OTHER_CABIN_ST_FAN        237
    #define REG_TOUCH_OTHER_CABIN_CMD_ON_OFF    238
    #define REG_TOUCH_OTHER_CABIN_CMD_SETP      239
    #define REG_TOUCH_OTHER_CABIN_CMD_SETP_F    240
    #define REG_TOUCH_OTHER_CABIN_CMD_FAN       241
    #define REG_TOUCH_OTHER_CABIN_CMD_SEND      242

    #define REG_TOUCH_DOUBLE_PUMP_SWITCH_TIME   243
    #define REG_TOUCH_DOUBLE_PUMP_SELECTION     244

    #define REG_TOUCH_INVERTER1_CODE_ALARM      245
    #define REG_TOUCH_INVERTER2_CODE_ALARM      246
    #define REG_TOUCH_INVERTER3_CODE_ALARM      247
    #define REG_TOUCH_INVERTER4_CODE_ALARM      248

    #define REG_TOUCH_DIAG_ONLY_FAN             249
    #define REG_TOUCH_ONLY_FAN                  250
    #define REG_TOUCH_OTHER_CABIN_ONLY_FAN      251

    #define REG_TOUCH_INVERTER_400V             252

    #define REG_TOUCH_SET_CMD_SPLIT_DISABLE     253

    #define REG_TOUCH_ABIL_DOUBLE_PUMP          254

    #define REG_TOUCH_SPLITMODE_ABIL            255

    #define REG_TOUCH_DIAG_FUNCTIONMODE         256

    #define REG_TOUCH_DIAG_SPLIT_EXP_VALVE_ERR  257
    #define REG_TOUCH_DIAG_SPLIT_EXP_VALVE_CODE 258

    #define REG_TOUCH_HEATER_ABIL               259
    #define REG_TOUCH_HEATER_PWR                260

    #define REG_TOUCH_ENGBOX_PRESS_LIQUID_COND  261

    #define REG_TOUCH_MAX_FANVENTIL_MODE        262

    #define REG_TOUCH_MANUAL_PUMP_ABIL          263

    #define REG_TOUCH_MANUAL_PUMP_FREQ          264

    #define REG_TOUCH_VALVOLA_CUR_NUMB_PRIORITY    265
    #define REG_TOUCH_VALVOLA_ROOM_NUMB_PRIORITY   266
    #define REG_TOUCH_VALVOLA_SUM_TOT_PERCENT      267
    #define REG_TOUCH_VALVOLA_SUM_REQ_PERCENT      268
    #define REG_TOUCH_VALVOLA_ONDEMAND_LIM_MAX     269
    #define REG_TOUCH_VALVOLA_MAX_PERC_PRIORITY    270

    #define REG_TOUCH_ONLY_COMUNICATION            271

    #define REG_TOUCH_ABIL_VALVE_ONDEMAND          272  // WR
    #define REG_TOUCH_ABIL_SCORE_ONDEMAND          273  // WR
    #define REG_TOUCH_ON_PRIORITY_MODE             274  // RD
    #define REG_TOUCH_VALVE_ONDEMAND_MIN_PERC_VAL  275  // RD
    #define REG_TOUCH_MAX_LIM_ONDEMAND_TOTAL_SCORE 276  // RD
    #define REG_TOUCH_TIME_ONDEMAND_VALVE_SWITCH   277  // RD

    #define REG_TOUCH_SPLIT_TOTAL_FAN_ABSORPTION   278 
    #define REG_TOUCH_COMPRESSOR1_ABSORPTION       279 
    #define REG_TOUCH_COMPRESSOR2_ABSORPTION       280
    #define REG_TOUCH_SEA_WATER_PUMP_ABSORPTION    281 
    #define REG_TOUCH_DIAG_SPLIT_FAN_POWER         282 
    #define REG_TOUCH_DIAG_SPLIT_FAN_CURRENT       283 
    #define REG_TOUCH_DIAG_SPLIT_SUPPLY_VOLTAGE       284

	#define REG_TOUCH_MODDEV_FLAGS_ENABLE_1_16     285
	#define REG_TOUCH_MODDEV_FLAGS_OFFLINE_1_16    286
	#define REG_TOUCH_MODDEV_FLAGS_ONLINE_1_16     287
	#define REG_TOUCH_MODDEV_FLAGS_ALARM_1_16      288
	#define REG_TOUCH_MODDEV_FLAGS_ENABLE_17_32    289
	#define REG_TOUCH_MODDEV_FLAGS_OFFLINE_17_32   290
	#define REG_TOUCH_MODDEV_FLAGS_ONLINE_17_32    291
	#define REG_TOUCH_MODDEV_FLAGS_ALARM_17_32     292
    #define REG_TOUCH_MODDEV_AND_ADDR_TYPE         293

    #define REG_TOUCH_TESTALL_ABIL                 294
    #define REG_TOUCH_TESTALL_SUPERHEAT_SP_COOL    295
    #define REG_TOUCH_TESTALL_SUPERHEAT_SP_HEAT    296
    #define REG_TOUCH_TESTALL_MAX_OPEN_VALVE_COOL  297
    #define REG_TOUCH_TESTALL_MAX_OPEN_VALVE_HEAT  298
    #define REG_TOUCH_TESTALL_WORK_TEMP_COOL       299
    #define REG_TOUCH_TESTALL_WORK_TEMP_HEAT       300
    #define REG_TOUCH_TESTALL_WORK_PRESS_COOL      301
    #define REG_TOUCH_TESTALL_WORK_PRESS_HEAT      302

    #define REG_TOUCH_TESTALL_FROZEN_ABIL            303
    #define REG_TOUCH_TESTALL_SUPERHEAT_SP_FRIGO     304
    #define REG_TOUCH_TESTALL_SUPERHEAT_SP_FREEZER   305
    #define REG_TOUCH_TESTALL_MAX_OPEN_VALVE_FRIGO   306
    #define REG_TOUCH_TESTALL_MAX_OPEN_VALVE_FREEZER 307
    #define REG_TOUCH_TESTALL_WORK_TEMP_FRIGO        308
    #define REG_TOUCH_TESTALL_WORK_TEMP_FREEZER      309
    #define REG_TOUCH_TESTALL_WORK_PRESS_FRIGO       310
    #define REG_TOUCH_TESTALL_WORK_PRESS_FREEZER     311

	#define REG_TOUCH_FROZEN_ABIL                    312
    #define REG_TOUCH_DIAG_FROZEN_MODE               313
	
    #define REG_TOUCH_ABIL_DEFROSTING                314
    #define REG_TOUCH_DEFROSTING_STEP_TIME           315
    #define REG_TOUCH_DEFROSTING_TIME                316
    #define REG_TOUCH_DEFROSTING_TEMP_SET            317
    #define REG_TOUCH_DRIPPING_TIME                  318
    #define REG_TOUCH_VALVOLA_FROZEN                 319

    #define REG_TOUCH_TESTALL_MIN_OPEN_VALVE_COOL    320
    #define REG_TOUCH_TESTALL_MIN_OPEN_VALVE_HEAT    321
    #define REG_TOUCH_TESTALL_MIN_OPEN_VALVE_FRIGO   322
    #define REG_TOUCH_TESTALL_MIN_OPEN_VALVE_FREEZER 323

    #define REG_TOUCH_SPLIT_FROZEN_MODE              324
    
    #define REG_TOUCH_DIAG_MAX_VENTIL_MODE           325

    #define REG_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN        326
    #define REG_TOUCH_TESTALL_SUPERHEAT_HEAT_IGAIN        327
    #define REG_TOUCH_TESTALL_SUPERHEAT_HEAT_DGAIN        328
    #define REG_TOUCH_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID  329

    #define REG_TOUCH_TESTALL_SUPERHEAT_COOL_PGAIN        330
    #define REG_TOUCH_TESTALL_SUPERHEAT_COOL_IGAIN        331
    #define REG_TOUCH_TESTALL_SUPERHEAT_COOL_DGAIN        332
    #define REG_TOUCH_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID  333

    #define REG_TOUCH_MAINTENANCE_FLAG                    334
    #define REG_TOUCH_MAINTENANCE_SET_TIME                335
    #define REG_TOUCH_MAINTENANCE_REMANING_TIME           336

    #define REG_TOUCH_ENGBOX_SUPERH_PID_ERROR             337

    #define REG_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT         338
    #define REG_TOUCH_SUPERHEAT_COOL_MAX_VALUE_CORRECT    339
    #define REG_TOUCH_SUPERHEAT_HEAT_MIN_SETPOINT         340
    #define REG_TOUCH_SUPERHEAT_HEAT_MAX_SETPOINT         341

    #define REG_TOUCH_BOX_TEMPERATURE_PROBE_ERROR         342
    #define REG_TOUCH_DIAG_SPLIT_TEMPERATURE_PROBE_ERROR  343

	#define REG_TOUCH_MASTER_COMERROR_CNTCOMERR             344
    #define REG_TOUCH_MASTER_COMERROR_TIMEOUTCOMERR         345          
    #define REG_TOUCH_MASTER_COMERROR_CRC_COMERR            346      
    #define REG_TOUCH_MASTER_COMERROR_MODCNTCOMERR          347
    #define REG_TOUCH_MASTER_COMERROR_MODTIMEOUTCOMERR      348      
    #define REG_TOUCH_MASTER_COMERROR_MODCRC_COMERR         349      
    #define REG_TOUCH_MASTER_COMERROR_MODCNTCOMERR_U2       350      
    #define REG_TOUCH_MASTER_COMERROR_MODTIMEOUTCOMERR_U2   351      
    #define REG_TOUCH_MASTER_COMERROR_MODCRC_COMERR_U2      352      
    #define REG_TOUCH_MASTER_COMERROR_CNTCOMERR_U3          353      
    #define REG_TOUCH_MASTER_COMERROR_TIMEOUTCOMERR_U3      354      
    #define REG_TOUCH_MASTER_COMERROR_CRC_COMERR_U3         355      

    #define REG_TOUCH_DIAG_SPLIT_MODEL_FW_VERSION           356     

    #define REG_TOUCH_DIAG_SPLIT_FAN_M3H                    357

    #define REG_TOUCH_DIAG_SPLIT_AIR_POWER_OUTPUT           358
    #define REG_TOUCH_DIAG_SPLIT_AIR_POWER_BTU              359

    #define REG_TOUCH_TOTAL_AIR_POWER_OTUPUT                360
    #define REG_TOUCH_TOTAL_AIR_POWER_BTU                   361
    #define REG_TOUCH_SYSTEM_EFFICENCY_EER                  362

    #define REG_TOUCH_OTHER_CABIN_CMD_UTA_MODE              363
    #define REG_TOUCH_OTHER_CABIN_ST_UTA_MODE               364

    #define REG_TOUCH_THRESHOLD_COMPRESSOR_HI               365

    #define REG_TOUCH_DIAG_SPLIT_CNTCOMERRSEC               366
    #define REG_TOUCH_DIAG_SPLIT_TIMEOUTCOMERRSEC           367
    #define REG_TOUCH_DIAG_SPLIT_TIMEOUTINTERCHARCOMERRSEC  368
    #define REG_TOUCH_DIAG_SPLIT_CRC_COMERRSEC              369
    #define REG_TOUCH_DIAG_SPLIT_MODCNTCOMERRSEC            370
    #define REG_TOUCH_DIAG_SPLIT_MODTIMEOUTCOMERRSEC        371
    #define REG_TOUCH_DIAG_SPLIT_MODTIMEOUTINTERCHCOMERRSEC 372
    #define REG_TOUCH_DIAG_SPLIT_MODCRC_COMERRSEC           373

    #define REG_TOUCH_DIAG_HUMIDITY                         374

    #define REG_TOUCH_DIAG_SPLIT_HEATER_ABIL                375
    #define REG_TOUCH_SPLIT_SET_HEATER_PWR                  376

    #define REG_TOUCH_SET_TEMP_AIR_OUT                      377

    #define REG_TOUCH_DEHUMI_ABIL                           378
    #define REG_TOUCH_SET_HUMI                              379
    #define REG_TOUCH_SET_DELTA_TEMP_MIN_DEHUMI             380
    #define REG_TOUCH_SET_FAN_SPEED_DEHUMI                  381
    #define REG_TOUCH_SET_PRESS_DEHUMI                      382
    #define REG_TOUCH_SET_TEMP_AIR_OUT_DEHUMI               383

    #define REG_TOUCH_DIAG_MIN_SET_POINT                    384
    #define REG_TOUCH_DIAG_MAX_SET_POINT                    385
    #define REG_TOUCH_SET_MIN_SET_POINT                     386
    #define REG_TOUCH_SET_MAX_SET_POINT                     387

    #define REG_TOUCH_ADDRESS_GAS_LEAKS                     388

//			0000000001111111111222222222233
//			1234567890123456789012345678901
	// registri Inverter
	#define REG_INVERTER_FREQUENCY_SET			0xFA01	// W - Scrive la frequenza richiesta
	#define	REG_INVERTER_COMMAND_SET			0xFA00	// W - scrive i comandi principali - RUN/STOP etc...
	#define	REG_INVERTER_TRIP_CODE_MONITOR		0xFC90	// R - codice di errore attuale dell'inverter
	#define	REG_INVERTER_ALARM					0xFC91	// R - stato dell'inverter
	#define	REG_INVERTER_OUT_FREQ				0xFD00	// RW - frequenza d'uscita (0.01 Hz)
	#define	REG_INVERTER_STATUS					0xFD01	// R - stato dell'inverter
	#define	REG_INVERTER_OUT_CURRENT			0xFD03	// R - Corrente d'uscita (0.01%)
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
    #define REG_INVERTER_FACTORY_SPEC_COEFF9L	0x0921     // F921	921	Factory specific coefficient 9L


    #define K_VAL_INVERTER_RUN_COMMAND          0x8400  // Valore per comando RUN - bit 10 = 1
    #define K_VAL_INVERTER_STOP_COMMAND         0x8000  // Valore per comando STOP - bit 10 = 0
    #define K_VAL_INVERTER_BREAK_COMMAND        0x8080  // Valore per comando BREAK - bit7 = 1 
    #define K_VAL_INVERTER_RESET_FLT_COMMAND    0xA000	// Valore per comando RESET FAULT 

    #define K_VAL_INVERTER_ERROR_CODE_Err4		23		// Valore per Errore Err4 -> CPU1 Fault (Reg FC90) 
    #define K_VAL_INVERTER_ERROR_CODE_Err5		24		// Valore per Errore Err5 -> CommError Fault (Reg FC90) 
    #define K_VAL_INVERTER_ERROR_CODE_Sout		47		// Valore per Errore SOut -> StepOut Fault (Reg FC90) 
	#define K_VAL_INVERTER_ERROR_CODE_E21		53		// Valore per Errore E21 -> CPU2 Fault (Reg FC90) 
    #define K_VAL_INVERTER_ERROR_CODE_E26		58		// Valore per Errore E26 -> CPU3 Fault (Reg FC90) 
	#define K_VAL_INVERTER_ERROR_CODE_E39		71		// Valore per Errore E39 -> AutoTuning Fault (Reg FC90) 



	// registri FAN ModBUS ZiehgAbb
    // Holding - RW
    #define REG_MOD_ZIEGABB_SPEED_CONTROL       2       // R/W - HoldingRegister - speed control of the device   0 to 65535 1/m   (The interpretation of the value depends on control mode register (h4) Bit [3:0].)
    #define REG_MOD_ZIEGABB_COM_PARAM           3       // R/W - HoldingRegister - Communication parameters for serial MODBUS communication.
    #define REG_MOD_ZIEGABB_CONTROL_MODE        4       // R/W - HoldingRegister - Control mode defines how the device is controlled by the user.
                                                        //      3:0 bit:    0: Control by E1 (0 - 10 V / PWM) * (Default=0))
                                                        //                  1: Speed control register h2 (absolute)
                                                        //                  2: Speed control register h2 (fractional 0 - 32767 = 0 - 100 %) *
                                                        //                  3: Speed control register h2 (fractional 0 - 100 = 0 - 100 %) *

	
    // Input - R
    #define REG_MOD_ZIEGABB_OP_COND1            10      // R - InputRegister - Operation condition 1
    #define REG_MOD_ZIEGABB_OP_COND2            11      // R - InputRegister - Operation condition 2
    #define REG_MOD_ZIEGABB_SPEED_ACTUAL        14      // R - InputRegister - Display actual speed    0 to 65535 1/m

	// registri FAN ModBUS EBMPapst
    // Holding - RW
    #define REG_MOD_EBM_SETPOINT            0xD001  // R/W - HoldingRegister - speed control of the device   0 to 65535 1/m   (The interpretation of the value depends on control mode register (h4) Bit [3:0].)

	
    // Input - R
    #define REG_MOD_EBM_ACT_SPEED           0xD010      // R - InputRegister
    #define REG_MOD_EBM_MOTOR_STATUS        0xD011      // R - InputRegister



//-------------------------------------------------------
// Valori di default per inizializzazione Inverter FAN
//-------------------------------------------------------
   #define K_FAN_INVERTER_TIME_ACC						35*10			// ACC	9 Tempo di accelerazione 1			Unit: 0.1Hz											
   #define K_FAN_INVERTER_TIME_DEC						35*10			// DEC	10 Tempo di	decelerazione 1			Unit: 0.1Hz											

   #define K_FAN_INVERTER_MAX_FREQUENCY					50*100//55*100			// FH	11 Max Frequency					Unit: 0.01Hz											
   #define K_FAN_INVERTER_UPPER_LIM_FREQ_50HZ   		50*100//			// UL	12	Upper limit frequency			Unit: 0.01Hz
   #define K_FAN_INVERTER_UPPER_LIM_FREQ_65HZ   		65*100//			// UL	12	Upper limit frequency			Unit: 0.01Hz
   #define K_FAN_INVERTER_BASE_FREQUENCY				50*100			// vL	14	Base frequency 1				Unit: 0.01Hz
   #define K_FAN_INVERTER_VF_CONTROL_MODE				0				// Pt	15	V/F control mode selection		Unit: 1

   #define K_FAN_INVERTER_AUTO_START_FREQ				15*100			// F241	Frequenza avviamento automatico		Unit: 0.01Hz

   //#define K_FAN_INVERTER_MOTOR_RATED_CAPACITY		1.5*100			// F405	405	Motor rated capacity			Unit: 0.01Kw
   #define K_FAN_INVERTER_BASE_FREQ_VOLTAGE				230*10			// vLv	409	Base frequency voltage 1		Unit: 0.1V
   //#define K_FAN_INVERTER_MOTOR_RATED_CURRENT			6.5*100		// F415	415	Motor rated current				Unit: 0.01A
   #define K_FAN_INVERTER_MOTOR_RATED_SPEED				2800			// F417	417	Motor rated speed				Unit: 1min-1 ******????
   //#define K_FAN_INVERTER_INVERTER_STALL_PREVENTION_L192*1			// F601	601	Stall prevention level 1		Unit: 1%


   #define K_FAN_INVERTER_TIMEOUT_COMUNICATION			6*10			// F803 Time-out tempo di comunicazione		Unit: 0.1s
#if (K_SIMULATION_WORK==1)
   #define K_FAN_INVERTER_ACTION_TMT_COMUNICATION		0				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#else
   #define K_FAN_INVERTER_ACTION_TMT_COMUNICATION		2				// F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto
#endif





//---------------------------------------------------------------------------------------------------------------------------------
//	Registri ModBus Pannello NetBuilding				Descrizione															Tipo
//---------------------------------------------------------------------------------------------------------------------------------
// Tipo Coil Register
	#define	COIL_NetB_KEYLOCK					1	// Blocco Tastiera: 1=Bloccato											R/W
	#define	COIL_NetB_SEASON					2	// Stagione: Raffescamento=0 Riscaldamento=1							R/W
	#define	COIL_NetB_CONFORT					3	// Confort/Mantenimento: 1=Confort										R/W
	#define	COIL_NetB_CONFFORCE				4	// Forzatura a Confort Fisso: 1=Confort Fisso 							R/W
	#define	COIL_NetB_ABILPROG					5	// Consenso alla programmazione: 1=COnsenso abilitato 					R/W
/* Vedi DOC "Termoregolatori MCT-MCU - Protocollo ModBus RTU Rev. 2.6" (Solo cartaceo)
 * Per implementazione ulteriori registri COIL (1..18)
	#define	COIL_NetB_					6	// : 							R/W
	#define	COIL_NetB_					7	// : 							R/W
	#define	COIL_NetB_					8	// : 							R/W
	#define	COIL_NetB_					9	// : 							R/W
	#define	COIL_NetB_					10	// : 							R/W
	#define	COIL_NetB_					11	// : 							R/W
	#define	COIL_NetB_					12	// : 							R/W
	#define	COIL_NetB_					13	// : 							R/W
	#define	COIL_NetB_					14	// : 							R/W
	#define	COIL_NetB_					15	// : 							R/W
	#define	COIL_NetB_					16	// : 							R/W
	#define	COIL_NetB_					17	// : 							R/W
	#define	COIL_NetB_					18	// : 							R/W
*/

// Tipo Holding Register
	#define	REG_NETB_ADDR						1	// Indirizzo Nodo (range 1..247)										R/W 
	#define	REG_NETB_FW_VER					2	// Numero versione Firmware												R   
	#define	REG_NETB_FW_REL					3	// Numero rilascio Firmware												R   
	#define	REG_NETB_TEMP_ATT_16				4	// Temperatura Attuale (elaborata) in sedicesimi						R   
	#define	REG_NETB_TEMP_IST_16				5	// Temperatura Istantanea in sedicesimi									R   
	#define	REG_NETB_TEMP_INT_16				6	// Temperatura interna in sedicesimi									R   
	#define	REG_NETB_HUMI_REL					7	// Umidita relativa (0..100% RH)										R   
	#define	REG_NETB_ON_OFF					8	// Accensione-Spegnimento (0=Spento, 3=Auto, 4=crono)					R/W 
	#define	REG_NETB_TEMP_COMFORT				9	// Temperatura comfort (5..45°C)										R/W 
	#define	REG_NETB_TEMP_MAINT				10	// Temperatura mantenimento (5..45°C)									R/W 
	#define	REG_NETB_TEMP_ANTIFREEZE			11	// Temperatura antigelo (0..18°C 0=disabil)								R/W 
	#define	REG_NETB_TEMP_MAX					12	//* Temperatura Massima (20..45°C)										R/W 
	#define	REG_NETB_TEMP_MAX_AUTO				13	// Temperatura Massima in comm. auto (imp. 4 tubi)						R/W 
	#define	REG_NETB_TEMP_MIN					14	//* Temperatura Minima (5..30°C)										R/W 
	#define	REG_NETB_IMP_VEL_FAN				15	//* Impostazione velocita Fancoil (0..4 0=OFF, 4=Auto)					R/W 
	#define	REG_NETB_PERC_MAX_PID				16	// Percentuale soglia massima regolazione PID (0..100%)					R/W 
	#define	REG_NETB_FACT_CORR_TEMP			17	// Fattore di correzione della temperatura (0..199 decimi °C)			R/W
	#define	REG_NETB_FACT_COMP_TEMP			18	// Fattore di compensazione della temperatura (0..199 decimi °C)		R/W
	#define	REG_NETB_LCD_INTENS				19	// Intensita display (0..100%)											R/W
	#define	REG_NETB_LCD_INTENS_TIME			20	// Tempo intensita display (1..255 sec.)								R/W
	#define	REG_NETB_PROP_GAIN					21	// Guadagno proporzionale (1..100)										R/W
	#define	REG_NETB_INTEGR_GAIN				22	// Guadagno integrale (1..100)											R/W
	#define	REG_NETB_INTEGR_TIME				23	// Tempo integrale (2..255 sec.)										R/W
	#define	REG_NETB_OUT_PID					24	// Uscita PID (1..100%)													R/W
	#define	REG_NETB_TEMP_SETP					25	// Setpoint temperatura													R/W
	#define	REG_NETB_LOCKED_MODE				26	// Funzionamento in bloccato (2=manuale, 3=auto)						R/W
	#define	REG_NETB_MANUAL_VEL				27	// Velocita in manuale (0..4 o 0..10 dipende dal tipo di scheda)		R/W
	#define	REG_NETB_OP_STATUS					28	// Stato funzionamento (0=OFF rem, 1=OFF loc, 2=manu, 3=auto, 4=crono)	R/W 
	#define	REG_NETB_SP_COMM_AUTO				29	// Soglia per commutazione automatica (1..127 decimi °C)				R/W
	#define	REG_NETB_IR_IN_DELAY				30	// Ritardo ingresso infrarosso											R/W
	#define	REG_NETB_BAUD_SETUP				31	// Set velocita seriale (1=9600, 2=19200, 3=38400, 4=57600, 5=115000)	R/W
	#define	REG_NETB_DATA_BIT					32	// Bit dati seriale														R
	#define	REG_NETB_PARITY_ERR				33	// Parita seriale (0=none)												R
	#define	REG_NETB_STOP_BIT					34	// Bit di stop seriale													R
	#define	REG_NETB_IST_SETP					35	// Isteresi sul SetPoint in funzionamento ON-OFF (0..99 decimi °C)		R/W
	#define	REG_NETB_HUMI_SETP					36	// Setpoint umidita (0=regolazione T, >0 = regolazione T e U)			R/W
	#define	REG_NETB_DEW_POINT_16				37	// Punto di rugiada (solo MCU) in sedicesimi							R
	#define	REG_NETB_DEW_POINT_10				38	// Punto di rugiada (solo MCU) in decimi								R
	#define	REG_NETB_EXT_TEMP_16				39	// Temperatura sonda esterna STF1 in sedicesimi							R
	#define	REG_NETB_EXT_TEMP_10				40	// Temperatura sonda esterna STF1 in decimi								R
	#define	REG_NETB_TEMP_ATT_10				41	//* Temperatura Attuale (elaborata) in decimi							R 
	#define	REG_NETB_TEMP_IST_10				42	// Temperatura Istantanea in decimi										R 				
	#define	REG_NETB_TEMP_INT_10				43	// Temperatura interna in decimi										R 				
	#define	REG_NETB_TEMP_COMFORT_10			44	// Temperatura compfort in decimi di grado (50..450°C)					R/W
	#define	REG_NETB_TEMP_MAINT_10				45	// Temperatura mantenimento in decimi di grado (50..450°C)				R/W
	#define	REG_NETB_TEMP_SETP_10				46	//* Setpoint temperatura in decimi di grado								R/W 
	#define	REG_NETB_HUMI_REL_10				47	//* Umidita relativa in decimi di grado (%RH)							R
	#define	REG_NETB_PRESSURE_10				48	// Pressione in decimi (hPa)											R
	#define	REG_NETB_BIOREG_10					49	// Valore bioregolazione in decimi (%)									R
	#define	REG_NETB_BIOREG_SETP_10             50	// Setpoint bioregolazione in decimi (%)								R/W
	#define	REG_NETB_DEHUMIDIF_SETP             51	// Setpoint Deumidificazione (%RH)										R/W
	#define	REG_NETB_HUMIDIF_SETP				52	// Setpoint Umidificazione (%RH)										R/W
	#define	REG_NETB_ENTALPIA_100				53	// Valore Entalpia in centesimi											R
	#define	REG_NETB_WET_SKIN_10				54	// Valore Wet Skin in decimi di grado									R
	#define	REG_NETB_REMOTE_TEMPERATURE			55	// Valore di temperatura remota in decimi di grado	(funziona se nel menù del touch - Sonta esterna = principale rem) R/W

	#define	REG_NETB_NIGHTMODE_LCD_INTENSITY	61	// Percentuale luminosità display per funzione LED Night Control R/W
	#define	REG_NETB_NIGHTMODE_HOUR_START		62	// Ora iniziale funzione LED Night Control R/W
	#define	REG_NETB_NIGHTMODE_HOUR_STOP       	63	// Ora finale funzione LED Night Control R/W
//PArametro luminosità default

    #define	K_NetB_LCD_Intensity                12



//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	Registri ModBus Pannello SyxthSense Mod.SRI-70      Descrizione															Tipo							Valore da inizializzare
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Tipo Coils Registers (FUNCTION CODE 01)
	#define	COIL_SYXTS_OFF_MODE_OVRR        100	// Off Mode Override        0..1                                        R/W								On/Off	
	#define	COIL_SYXTS_ECO_MODE_OVRR        101	// ECO Mode Override        0..1										R/W								On/Off
	#define	COIL_SYXTS_SHOW_FROST_ICON      102	// Show Frost Icon        	0..1										R/W								On/Off

// Tipo Discrete Inputs Registers (FUNCTION CODE 02)
	#define	COIL_SYXTS_AC_STATUS            104	// AC Status                0..1                                       R								On/Off	

// Tipo Read Input Registers (FUNCTION CODE 04)
	#define	REGI_SYXTS_BI_TEMP             100 // Built-In Temperature Measurement                                     R - Sign16   
	#define	REGI_SYXTS_RI1_MEASURE         101 // Resistive Input 1 Measurement                                        R   
	#define	REGI_SYXTS_RI2_MEASURE         102 // Resistive Input 2 Measurement                                        R   
	#define	REGI_SYXTS_CURC_SETP           103 // Current Calculated Setpoint (°C)                                     R   
	#define	REGI_SYXTS_INT_CURM            104 // Interface Current Mode                                               R   
	#define	REGI_SYXTS_REL_HUMI            105 // Relative Humidity Measurement (with RH option)                       R   
	#define	REGI_SYXTS_ALARM_STATUS        106 // Alarm State                                                          R   
	#define	REGI_SYXTS_DIS_INPUTR          107 // Discrete Input Registers                                             R   
	#define	REGI_SYXTS_SCHED_TIME          108 // Next Schedule Switching Time (only SRI-P70)                          R   
	#define	REGI_SYXTS_SCHED_DAY           109 // Next Schedule Switching Day (only SRI-P70)                           R   
	#define	REGI_SYXTS_SCHED_VALUE         110 // Next Schedule Switching Value (only SRI-P70)                         R   
	#define	REGI_SYXTS_CUR_FSPEED          111 // Current Fan Speed                                                    R   
	#define	REGI_SYXTS_MODE_ICON_STATUS    112 // Mode Icon Status                                                     R   
	#define	REGI_SYXTS_FW_REL              200 // Firmware Version                                                     R

// Tipo Holding Registers (FUNCTION CODE 03 -> READ, FUNCTION CODE 06 -> WRITE SINGLE, FUNCTION CODE 16 -> WRITE MULTIPLE)
	#define	REGH_SYXTS_NOM_SETP            100	// Nominal Setpoint                                                     R/W								Da impostare con il SETPOINT
	#define	REGH_SYXTS_SETP_UNIT           101	// Setpoint Unit ->   0=Temperatura (Default), 1=Percentuale            R/W								0
	#define	REGH_SYXTS_SENSOR3_SOURCE      102	// Sensor 3 Source -> 0 = Buillt-In S. (Default)  1 = Network S.		R/W								1
	#define	REGH_SYXTS_SENS3_NET_TEMP      103	// Sensor 3 Network Temperature                                         R/W								Da impostare con la temperatura amb.
	#define	REGH_SYXTS_MIN_SETP            104	// Minimum Setpoint     0..990 -> 0..99.0°C                             R/W								180
	#define	REGH_SYXTS_MAX_SETP            105	// Maximum Setpoint     0..990 -> 0..99.0°C                             R/W								300
	#define	REGH_SYXTS_COLOUR_RING_MODE    106	// Colour Ring Mode     0..10  -> 10=Auto                               R/W								10		Auto: si colora in base al delta setpoint-temperatura
	#define	REGH_SYXTS_SENSOR1_SOURCE      107	// Sensor 1 Source                                                      R/W								
	#define	REGH_SYXTS_SENS1_NET_TEMP      108	// Sensor 1 Network Temperature                                         R/W   
	#define	REGH_SYXTS_FAN_MAN_CMODE       109	// Fan Manual Control Mode  0..9 -> 0=Disabled(Default)  9=Advanced     R/W								9
	#define	REGH_SYXTS_FAN_DISPLAY         110	// Fan Display              0..5 -> 4 = Coloured Bar & Fan              R/W								4
	#define	REGH_SYXTS_FAN_SPEED_OVRR      111	// Fan Speed Override       0..6                                        R/W								6
	#define	REGH_SYXTS_ENABLE_MODE_ICON    112	// Enable Mode Icon         0..6 -> 4 = Auto/Hhg/Clg/Fan                R/W								4
	#define	REGH_SYXTS_113                 113	//                                                                      R/W   
	#define	REGH_SYXTS_114                 114	//                                                                      R/W   
	#define	REGH_SYXTS_ENAB_LIGHT_SYMBOL   115	// Enable Lights Symbol		0 = Disabled (Default)                      R/W								0    
	#define	REGH_SYXTS_ENAB_AC_SYMBOL      116	// Enable AC Symbol			0 = Disabled (Default)                      R/W								0
	#define	REGH_SYXTS_LOCK_MODE           117	// Lock Mode                0..3   0 = Lock mode Disabled (Default)     R/W   
	#define	REGH_SYXTS_TEMP_UNIT           118	// Temperature Units        0=°C (Default)  1=Fahrenheit                R/W								0 
    #define	REGH_SYXTS_119                 119	//                                                                      R/W								
	#define	REGH_SYXTS_120                 120	//                                                                      R/W   
	#define	REGH_SYXTS_121                 121	//                                                                      R/W   
	#define	REGH_SYXTS_122                 122	//                                                                      R/W   
	#define	REGH_SYXTS_HUMI_DISPLAY        123	// Humidity Display			0 = Disabled                                R/W								0
	#define	REGH_SYXTS_OVERR_MODE_ICON     124	// Override Mode Icon       0..4                                        R/W								Da impostare con la MODE corrente
	#define	REGH_SYXTS_125                 125	//                                                                      R/W   
	#define	REGH_SYXTS_BACKL_BRIGHT        126	// Backlight Brightness     0..20 (5=Default)                           R/W								1
	#define	REGH_SYXTS_127                 127	//                                                                      R/W   
	#define	REGH_SYXTS_128                 128	//                                                                      R/W   
	#define	REGH_SYXTS_129                 129	//                                                                      R/W   
	#define	REGH_SYXTS_OVERR_AC            130	// Override A/C                                                         R/W                             0  
	#define	REGH_SYXTS_OVERR_LIGHTS        131	// Override Lights                                                      R/W                             0
	#define	REGH_SYXTS_OVERR_LOCK_MODE     132	// Override Lock Mode		0 = Disabled (Default)                      R/W								Nel funzionamento  1= Visualizza lucchetto 2= cancella lucchetto
	#define	REGH_SYXTS_ENABLE_OFF_ICON     133	// Enable OFF Icon  0=Disabled (Default) 1=Enabled (Home Screen)        R/W								1
	#define	REGH_SYXTS_FAN_SPEED_OVRR_ADV  134	// Fan Speed Override (Advanced Mode)    1..8                           R/W								Scrivere/Leggere la velocità attuale 1..8  1=Night .. 8=Auto 
	#define	REGH_SYXTS_NIGHT_FAN_SPEED     135	// Night Speed - Set Fan Speed in Night Mode   0..6                     R/W								1
	#define	REGH_SYXTS_OVERR_RELAY         136	// Relay Network Override                                               R/W   
	#define	REGH_SYXTS_137                 137	//                                                                      R/W   
	#define	REGH_SYXTS_LANGUAGE            138	// Language					0 = English (Default)                       R/W								0
	#define	REGH_SYXTS_139                 139	//                                                                      R/W   
	#define	REGH_SYXTS_140                 140	//                                                                      R/W   
	#define	REGH_SYXTS_SENSOR3_TEXT        141	// Sensor 3 Text            16 = Cabin                                  R/W                             16
	#define	REGH_SYXTS_142                 142	//                                                                      R/W   
	#define	REGH_SYXTS_143                 143	//                                                                      R/W   
	#define	REGH_SYXTS_DISAB_ECOB_CANCEL   159	//                                                                      R/W   

	#define	REGH_SYXTS_MODBUS_ADDRESS      300	// Modbus Address   0..247                                              R/W                             30
	#define	REGH_SYXTS_MODBUS_BAUD         301	// Modbus Baud Rate 0=9600..3=57600, 4=76800                            R/W                             3
	#define	REGH_SYXTS_MODBUS_PARITY       302	// Modbus Parity   0=None                                               R/W                             0
	#define	REGH_SYXTS_MODBUS_STOPBIT      303	// Modbus Address  0=1Stop                                              R/W                             1


    #define	REGH_SYXTS_FORCE_RESET         400	// Force Reset                  1 = Force Reset                         R/W   
	#define	REGH_SYXTS_E2_UPDATE           401	// Non Volatile Memory Update   1 = Update                              R/W   
	#define	REGH_SYXTS_FORCE_DEFAULT       403	// Force Factory Defaults       1 = Force Factory Defaults              R/W   





	#define VariabiliGlobaliProtocolloModBusSec()	\
		volatile TypTimer TimeOutModPktRxSec;		\
		volatile char RetryCntSec = 0;				\
		volatile char DataIndexSec = 0;			\
        volatile TypInverter Inverter;          \
		volatile TypSyncroData Syncronize;


	#define IncludeVariabiliGlobaliProtocolloModBusSec()	\
		extern volatile TypTimer TimeOutModPktRxSec;		\
		extern volatile char RetryCntSec;					\
		extern volatile char DataIndexSec;					\
        extern volatile TypInverter Inverter;           \
		extern volatile TypSyncroData Syncronize;


	//----------------------------------
	// Strutture
	//----------------------------------

	typedef struct
	{
		int	SetPoint;
		int	FanMode;
		int	Script_SetPoint_F;
		int Script_Split_Pwr;
		int EcoModeEnable;
		int Power_EcoMode;
		int	DiagWriteSetPoint;
		int	DiagWriteFanMode;
		int	DiagWriteSetPoint_F;
		int DiagWritePwr;
        int DiagUta_Mode;
        int DiagWriteHeaterPwr;        
        
		
		int DefMode;
		int Uta_Power;
		int Uta_Mode;
		int Uta_SP_Room;
		int Uta_SP_Humi;
		int Uta_Fan_Speed;
		int Uta_SP_Room_F;
		int SystemDisable;
        int HeaterPwr;
		
		struct 
		{	
			int	SetPoint;
			int	FanMode;
			int	Script_SetPoint_F;
			int Script_Split_Pwr;
			int EcoModeEnable;
			int Power_EcoMode;
			int	DiagWriteSetPoint;
			int	DiagWriteFanMode;
			int	DiagWriteSetPoint_F;
			int DiagWritePwr;
            int DiagUta_Mode;
            int DiagWriteHeaterPwr;
			
			int DefMode;
			int Uta_Power;
			int Uta_Mode;
			int Uta_SP_Room;
			int Uta_SP_Humi;
			int Uta_Fan_Speed;
			int Uta_SP_Room_F;			

			int SystemDisable;
            int HeaterPwr;
		} Old;
		
		struct 
		{	
			int	SetPoint;
			int	FanMode;
			int	Script_SetPoint_F;
			int Script_Split_Pwr;
			int EcoModeEnable;
			int Power_EcoMode;			
			int	DiagWriteSetPoint;
			int	DiagWriteFanMode;
			int	DiagWriteSetPoint_F;
			int DiagWritePwr;
            int DiagUta_Mode;
            int DiagWriteHeaterPwr;
			
			int DefMode;
			int Uta_Power;
			int Uta_Mode;
			int Uta_SP_Room;
			int Uta_SP_Humi;
			int Uta_Fan_Speed;
			int Uta_SP_Room_F;			

			int SystemDisable;
            int HeaterPwr;
		} Engine;			
	} TypSyncroData;
    
    
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
				 int	ErrorCode;		// registro letto dall'inverter
                 int    FreqFan;
				 int	AlarmReg;		// registro letto dall'inverter
		unsigned int	StepOutCurrentWarningDB;
		unsigned int	StepOutCurrentWarningWork;
		
		unsigned int	Power_Inverter;				// Potenza Taglia Inverter						Watt
		unsigned int	PowerMax_Available;			// Valore limitazione Potenza totale			Watt
		unsigned int	Power_EcoMode;	
		unsigned int	DiagAddress;
	
        unsigned		FanInverterFault:1;
        
		unsigned int	EcoModeEnable;		// Flag per abilitazione limitazione Potenza in ECO Mode		
		unsigned		OffLine:1;					// se non ha risposto
	} TypInverter;    
    
    
    
	//---------------------------------------
	// Variabili 
	//---------------------------------------
	//IncludeVariabiliGlobaliDriverModBusSec();
	IncludeVariabiliGlobaliProtocolloModBusSec();
	//IncludeVariabiliGlobaliCore();  
    
    
	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	unsigned char WaitModRxSec(int TimeOut_ms);
	unsigned char Send_WaitModRxSec(unsigned char Address, char Comando, int Registro, int Data, int TimeOut_ms, char Retry, int AddressBased);
	unsigned char Send_WriteModRegSec(unsigned char Address, int Registro, int Data, int TimeOut_ms, char Retry, char Cmd, int AddressBased);
	
	void ReadModRegSec (char Address, int REG, volatile int * StoreReg);
	unsigned char ReadModListRegSec (char DataIndexSec, int REG, int RegNum);
	void WriteModRegSec (char Address, int REG, int SendData);
	void WriteModListRegSec (char Address, int REG, int SendData);
	void ReadModRegSecNetB (char Address, int REG, volatile int * StoreReg);
	unsigned char ReadModListRegSecNetB (unsigned char DataIndexSec, int REG, int RegNum);
	void WriteModRegSecNetB (unsigned char Address, int REG, int SendData);
    void WriteCoilModRegSecNetB (unsigned char Address, int REG, int SendData);    
	void WriteModListRegSecNetB (unsigned char Address, int REG, int SendData);
    void ReadCoilModRegSecNetB (char Address, int REG, int * StoreReg);        
    
	int ExtracRegSec(char NumReg);
	void InsertRegSec(char NumReg, int Data);

	void Search_Touch(void);
	void Resume_TouchOffLine(unsigned char * Cnt);

	void RefreshTouchInPut(char TouchAddres);
	void RefreshAllTouchInPut(void);

	void RefreshTouchOutPut(char TouchAddres);
	void RefreshAllTouchOutPut(void);

	void InitTouch(void);

	// Aggiunti il 04/01/2014 per gestire pannelli NetBuilding
	void Search_NetB(void);
	void Resume_NetBOffLine(unsigned char * Cnt);

	void RefreshNetBInPut(char NetBAddres);
	void RefreshAllNetBInPut(void);

	void RefreshNetBOutPut(char NetBAddres);
	void RefreshAllNetBOutPut(void);

	void InitNetB(void);
	void Reinit_DataSplitToTouch(void);
	void Reinit_DataSplitToNetB(void);

	void SyncronizeDataOld(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer);
	void SyncronizeDataNew(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer);
    void SyncronizeDataBridge(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer);
	void SyncronizeDataNew_NetB(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer);
	void SyncronizeDataSetP_NetB(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer);
	void SyncronizeDataOnOff_NetB(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer);
	void SyncronizeDataVel_NetB(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer);
	void ApplyVel_NetB(volatile int * OnOff, volatile int * Vel, int RegAddr);
    
    void RefreshInverterFan(TypInverter * Inverter, int InvAddr);
    void InitInverterFan(TypInverter * Inverter, int InvAddr);
    
    void ReadModRegSecInv(char Address, int REG, int * StoreReg);    
    void WriteModRegSecInv(char Address, int REG, int SendData);
    void CheckHotPlugTouch(unsigned char * Cnt);
    void CheckHotPlugNetB(unsigned char * Cnt);

    void Reinit_DataSplitToSyxtS(void);
    void ReadModRegSecSyxtS (char Address, int REG, volatile int * StoreReg);
    void ReadModInputSecSyxtS (char Address, int REG, volatile int * StoreReg);
    unsigned char ReadModListRegSecSyxtS (unsigned char DataIndexSec, int REG, int RegNum);
    void WriteModRegSecSyxtS (unsigned char Address, int REG, int SendData);
    void WriteCoilModRegSecSyxtS (unsigned char Address, int REG, int SendData);
    void WriteModListRegSecSyxtS (unsigned char Address, int REG, int SendData);
    void ReadCoilModRegSecSyxtS (char Address, int REG, int * StoreReg);
    void RefreshAllSyxtSInPut(void);
    void RefreshSyxtSInPut(char SyxtSAddres);
    void SyxtS2Touch(void);
    void Search_SyxtS(void);
    void InitSyxtS(void);    
    void RefreshAllSyxtSOutPut(void);
    void Resume_SyxtSOffLine(unsigned char * Cnt);
    void CheckHotPlugSyxtS(unsigned char * Cnt);
    void SyncronizeDataSetP_SyxtS(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddrW, int RegAddrI, int pointer);
    void SyncronizeDataOnOff_SyxtS(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer);
    void SyncronizeDataVel_SyxtS(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer);
    void SyncronizeDataHeater_SyxtS(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int RegAddrWr, int pointer);
    float round(float fvalue);        
    void SearchModbusDevice(void);
    void SearchModbusDeviceIndex(int * Device);
    int RecheckDataReadTouch(int ReadNow, int VarData, int * PreviousVarData, unsigned int * ReadDataCounter);
    void ReadInputStModRegSecSyxtS (char Address, int REG, int * StoreReg);    
    
#endif
