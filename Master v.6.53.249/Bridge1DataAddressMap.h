//----------------------------------------------------------------------------------
//	Progect name:	Bridge1DataAddressMap.h
//	Device:			PIC18F4620 @ 10Mhz (PLLx4)
//	Autor:			Termodinamica Team
//	Date:			10/04/2016
//	Description:	Header per define Mappa di scambio dati con scheda Bridge1 (MASTER UTA-H)
//----------------------------------------------------------------------------------

#ifndef _BRIDGE1_DATA_ADDRESS_MAP_H_
	#define _BRIDGE1_DATA_ADDRESS_MAP_H_
	
	//----------------------------------------------------------------------------------
	//	Define
	//----------------------------------------------------------------------------------

	// registri scheda bridge1 (Logica verso Master UTA-H)

	#define	REG_BRIDGE1_STATUS_FLAGS							0				// Bridge Status Flags
																				// bit 0:  Master Data Init Start (R/W): Viene settato a 1 dal Master quando inizia 
																				//		   la fase di scrittura dei dati di inizializzazione da inviare al SideB. 
																				//		   Durante questa fase le eventuali scritture dal SideB sono bloccate dal Bridge.
																				// bit 1:  Bridge SideB Data Init Start (R/W): Viene settato a 1 dal Master quando ha finito
																				//		   la fase 1 (scrittura dati di init bit0==1) avvisando il Bridge di iniziare l'invio con check
																				//		   di avvenuto recepimento del dato verso la logica SideB (DL).
																				//		   Verrà resettato insieme al bit0 una volta terminato l'invio con verifica di tutti i dati del 
																				//		   Buffer Bridge verso la logica SideB.
																				// bit 2:  not used  
																				// bit 3:  not used  
																				// bit 4:  not used  
																				// bit 5:  not used   
																				// bit 6:  not used  
																				// bit 7:  not used  
																				// bit 8:  not used  
																				// bit 9:  not used  
																				// bit 10: not used
																				// bit 11: not used
																				// bit 12: not used
																				// bit 13: not used
																				// bit 14: not used
																				// bit 15: not used
/*
	#define	REG_BRIDGE1_WORD_1									1				// 
	#define	REG_BRIDGE1_WORD_2									2				// 
	#define	REG_BRIDGE1_WORD_3									3				// 
	#define	REG_BRIDGE1_WORD_4									4				// 
*/

	// Range Registri in Lettura

	#define	REG_BRIDGE1_UTA_WRITE_PWR							5 //25				// UTA Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE1_UTA_WRITE_MODE							6 //26				// "UTA Unit Function Mode: 
																				// 16 = Ventilation
																				// 64 = Dehumi
																				// 128 = Heat treatment"
	#define	REG_BRIDGE1_UTA_WRITE_SP_ROOM						7 //27				// "UTA Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C
																				// Range: 18..30°C"
	#define	REG_BRIDGE1_UTA_WRITE_SP_HUMI						8 //28				// "UTA Unit Humidity SetPoint: 
																				// dehumidification capacity - 10 - 25 – 50%  es. 500 = 50.0% "                                      	   	
	#define	REG_BRIDGE1_UTA_WRITE_FAN_SPEED						9 //29				// "UTA Unit Fan Speed: 
																				// 1 = Vel.MIN
																				// 2 = Vel.MED
																				// 3 = Vel.MAX
																				// 4 = Vel.Auto"	
	#define	REG_BRIDGE1_UTA_H_CLEAR_ERROR						10	//30		// UTA H Unit Error Reset: 1=Reset
	#define	REG_BRIDGE1_UTA_H_CLEAR_SPLITERROR1					11				// UTA H Split Unit Error Reset: 1=Reset
	#define	REG_BRIDGE1_UTA_H_ABIL								12 				// UTA H Abilitation from Touch: 1=Abil






	// Range Registri in Scrittura

	#define	REG_BRIDGE1_UTA_H_ENGINE_COMPRESSOR_SPEED			100 //160				// "UTA H Engine Box Inverter Compressor Speed: (percentage)
																				// Range: 0..100%"
	#define	REG_BRIDGE1_UTA_H_ENGINE_WATER_PUMP_SPEED			101 //161				// "UTA H Engine Box Inverter Water Pump Speed: (percentage)
																				// Range: 0..100%"
	#define	REG_BRIDGE1_UTA_H_ENGINE_REQ_PRESSURE				102 //162				// UTA H Engine Box Max Air Handler Required Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar
	#define	REG_BRIDGE1_UTA_H_ENGINE_T_OUTPUT					103 //163				// UTA H Engine Box Output Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE1_UTA_H_ENGINE_T_SUCTION					104 //164				// UTA H Engine Box Suction Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE1_UTA_H_ENGINE_T_AIR						105 //165				// UTA H Engine Box Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE1_UTA_H_ENGINE_T_BATTERY					106 //166				// UTA H Engine Box Battery Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE1_UTA_H_ENGINE_T_GAS						107 //167				// UTA H Engine Box Gas Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE1_UTA_H_ENGINE_T_LIQUID					108 //168				// UTA H Engine Box Liquid Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE1_UTA_H_ENGINE_P_GAS						109 //169				// UTA H Engine Box Gas Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar
	#define	REG_BRIDGE1_UTA_H_ENGINE_P_LIQUID					110 //170				// UTA H Engine Box Liquid Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar
	#define	REG_BRIDGE1_UTA_H_ENGINE_BOX_ERROR1					111 //171				// "UTA H Engine Box Error/Status Flags 1:
																				// bit 0: High temperature compressor
																				// bit 1: Low temperature compressor
																				// bit 2: High temperature battery
																				// bit 3: Low temperature battery
																				// bit 4: Gas Recovery
																				// bit 5: not used
																				// bit 6: Pre test Expansion Valve
																				// bit 7: Test Expansion Valve
																				// bit 8: Low pressure
																				// bit 9: High pressure
																				// bit 10: Oil recharge
																				// bit 11: not used
																				// bit 12: not used
																				// bit 13: not used
																				// bit 14: not used
																				// bit 15: not used
	#define	REG_BRIDGE1_UTA_H_ENGINE_BOX_PERSERROR1				112 //172				// "UTA H Engine Box Persistent Error Flags 2:
																				// bit 0: High temperature compressor persistent Error
																				// bit 1: Low temperature compressor persistent Error
																				// bit 2: High temperature battery persistent Error
																				// bit 3: Low temperature battery persistent Error
																				// bit 4: Gas Recovery persistent Error
																				// bit 5: Flood alarm persistent Error
																				// bit 6: Overtemp compressor persistent Error
																				// bit 7: Low temperature air persistent Error
																				// bit 8: High temperature air persistent Error
																				// bit 9: Check refrigerant charge persistent Error
																				// bit 10: not used
																				// bit 11: reserved
																				// bit 12: reserved
																				// bit 13: reserved
																				// bit 14: reserved
																				// bit 15: reserved
	#define	REG_BRIDGE1_UTA_H_ENGINE_BOX_SPLITERROR1			113				// "UTA H Engine Box Split Error Flags 1:
	#define	REG_BRIDGE1_UTA_H_ABSORPTION						114 //188		// Instantaneous absorption control of the UTA (Ampere)
	#define	REG_BRIDGE1_UTA_H_CUMULATIVE_ALARM					115				// Cumulativo Allarmi x UTA


	#define	REG_UTA_H_READ_LIQ_TEMP								116	// 61	UTA H Unit Liquid Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_UTA_H_READ_GAS_TEMP								117	// 62	UTA H Unit Gas Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_UTA_H_READ_AIR_IN								118	// 63	UTA H Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_UTA_H_READ_AIR_OUT								119	// 64	UTA H Unit Output Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_UTA_H_READ_EXP_VALVE							120	// 65	"UTA H Unit Expansion Valve: (percentage)	Range: 0..100%   es. 500 = 50.0%"
	#define	REG_UTA_H_READ_HUMI									121	// 66	"UTA H Unit Humidity value: (percentage) Range: 0..100%    es. 500 = 50.0%"
	#define	REG_UTA_H_READ_REQUEST_AIR_TEMP						122	// 67	UTA H Unit Requested Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_UTA_H_READ_REQUEST_GAS_PRES						123	// 68	UTA H Unit Requested Gas Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar
	#define	REG_UTA_H_READ_SUPERHEAT							124	

	#define	REG_UTA_H_READ_SPLIT_HW_VERS						125	
	#define	REG_UTA_H_READ_SPLIT_FW_VERS_HW						126	
	#define	REG_UTA_H_READ_SPLIT_FW_VERS_FW						127	
	#define	REG_UTA_H_READ_SPLIT_FW_REV_FW						128	
	#define	REG_UTA_H_READ_BOX_HW_VERS							129	
	#define	REG_UTA_H_READ_BOX_FW_VERS_HW						130	
	#define	REG_UTA_H_READ_BOX_FW_VERS_FW						131	
	#define	REG_UTA_H_READ_BOX_FW_REV_FW						132	

	#define	REG_UTA_H_ON_UTA_SP                                 133	//

	#define	REG_UTA_H_ENGINE_BOX_COMP_STATUS                    134	//
	#define	REG_UTA_H_ENGINE_BOX_ID_MAS_SPLIT                   135	//
	#define	REG_UTA_H_ENGINE_BOX_ENGINE_MODE                    136	//



#endif