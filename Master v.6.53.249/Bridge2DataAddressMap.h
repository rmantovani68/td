//----------------------------------------------------------------------------------
//	Progect name:	Bridge2DataAddressMap.h
//	Device:			PIC18F4620 @ 10Mhz (PLLx4)
//	Autor:			Termodinamica Team
//	Date:			10/04/2016
//	Description:	Header per define Mappa di scambio dati con scheda Bridge2 (DL)
//----------------------------------------------------------------------------------

#ifndef _BRIDGE2_DATA_ADDRESS_MAP_H_
	#define _BRIDGE2_DATA_ADDRESS_MAP_H_
	
	//----------------------------------------------------------------------------------
	//	Define
	//----------------------------------------------------------------------------------

	// registri scheda bridge2 (Logica verso DL)

	#define	REG_BRIDGE2_STATUS_FLAGS							0				// Bridge Status Flags
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
	#define	REG_BRIDGE2_DIAG_ALL_SPLIT_WRITE_PWR				1				// ALL Unit Power On/Off  0=OFF 1=ON

	#define	REG_BRIDGE2_DIAG_ALL_SPLIT_WRITE_SP_ROOM			3				// "ALL Unit SetPoint: (hundredths of degrees Celsius) ex: 2350 = 23.50 °C
																				// Range: 18..30°C"
	#define	REG_BRIDGE2_DIAG_ALL_SPLIT_WRITE_FAN_SPEED			4				// "ALL Unit Fan Speed: 
																				// 1 = Vel.Night
																				// 2 = Vel.1
																				// 3 = Vel.2
																				// 4 = Vel.3
																				// 5 = Vel.4
																				// 6 = Vel.5
																				// 7 = Vel.6
																				// 8 = Vel.7
																				// 9 = Vel.Auto"
	#define	REG_BRIDGE2_DIAG_ALL_SPLIT_WRITE_UPDATE				5				// ALL Unit Update flag: 
																				// 1=Send Update PWR
																				// 2=Send Update SetPoint
																				// 4=Send Update Fan Speed
																				// 8=Send Update All
	#define	REG_BRIDGE2_DIAG_SPLIT_WRITE_MODE					6				// *** "Unit Function Mode: 
																				// 4 = Heating
																				// 8 = Cooling
																				// 16 = Ventilation"

	#define	REG_BRIDGE2_DIAG_SPLIT_WRITE_SELECTION				10				// *** Diag Slave Unit Selection Write Address
	#define	REG_BRIDGE2_DIAG_SPLIT_WRITE_PWR					11				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_DIAG_SPLIT_WRITE_SP_ROOM				12				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C
																				// Range: 18..30°C"
	#define	REG_BRIDGE2_DIAG_SPLIT_WRITE_FAN_SPEED				13				// "Unit Fan Speed: 
																				// 1 = Vel.Night
																				// 2 = Vel.1
																				// 3 = Vel.2
																				// 4 = Vel.3
																				// 5 = Vel.4
																				// 6 = Vel.5
																				// 7 = Vel.6
																				// 8 = Vel.7
																				// 9 = Vel.Auto"
	#define	REG_BRIDGE2_ECO_MODE_PWR_LIMIT						14				// Eco Mode Power Limit Value (Kw)   (hundredths of Kw)  es. 400 = 4.00Kw
	#define	REG_BRIDGE2_ECO_MODE_ENABLE							15				// Eco Mode Enable On/Off  (0=OFF 1=ON) – Priority over Power Limit Value
                                        	
	#define	REG_BRIDGE2_HEATER_PWR  							17				// Heater Enable On/Off  (0=OFF 1=ON)
                                        	   	
	#define	REG_BRIDGE2_UTA_WRITE_PWR							25				// UTA Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_UTA_WRITE_MODE							26				// "UTA Unit Function Mode: 
																				// UTA Unit Function Mode: 
																				// 1 = Ventilation
																				// 2 = Heat treatment
																				// 4 = Dehumi
	#define	REG_BRIDGE2_UTA_WRITE_SP_ROOM						27				// "UTA Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C
																				// Range: 18..30°C"
	#define	REG_BRIDGE2_UTA_WRITE_SP_HUMI						28				// "UTA Unit Humidity SetPoint: 
																				// dehumidification capacity - MIN - MED ? MAX
																				// 1 = MIN
																				// 2 = MED
																				// 3 = MAX                                	   	
	#define	REG_BRIDGE2_UTA_WRITE_FAN_SPEED						29				// "UTA Unit Fan Speed: 
																				// 1 = Vel.MIN
																				// 2 = Vel.MED
																				// 3 = Vel.MAX
	
	#define REG_BRIDGE2_SYSTEM_DISABLE							30				//0 = System On 
																				//1 = System Off	
	
	#define	REG_BRIDGE2_UTA_READ_AIR_IN							63				// UTA H Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_UTA_READ_AIR_OUT						64				// UTA H Unit Output Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C

	#define	REG_BRIDGE2_DIAG_SPLIT_TEST_ADDRESS					106				// Diagnostic Split Request Address
	#define	REG_BRIDGE2_DIAG_SPLIT_ADDRESS_READ_CONFIRMED		107				// Diagnostic Split Request Address Received to Bridge

	#define	REG_BRIDGE2_DIAG_SPLIT_READ_AIR_IN					110				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_DIAG_SPLIT_STATUS						111				// Diagnostic Split Comunication Status: 
																				// 1 = Unit not present
																				// 2 = Unit enabled
																				// 4 = Unit Offline
																				// 8 = Unit in Error
                                        	   	
	#define	REG_BRIDGE2_SPLIT_ENGINE_BOX_COMPRESSOR_SPEED		140				// "Engine Box Inverter Compressor Speed: (percentage)
																				// Range: 0..100%   es. 500 = 50.0%"
	#define	REG_BRIDGE2_SPLIT_ENGINE_BOX_WATER_PUMP_SPEED		141				// "Engine Box Inverter Water Pump Speed: (percentage)
																				// Range: 0..100%   es. 500 = 50.0%"
	#define	REG_BRIDGE2_SPLIT_CURRENT_REQ_PRESSURE				142				// Engine Box Max Air Handler Required Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar

	#define	REG_BRIDGE2_SPLIT_ENGINE_BOX_T_SEA_WATER			145				// Engine Box Sea Water Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C

	#define	REG_BRIDGE2_SPLIT_ENGINE_BOX_P_GAS					149				// Engine Box Gas Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar
	#define	REG_BRIDGE2_SPLIT_ENGINE_BOX_P_LIQUID				150				// Engine Box Liquid Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar
	#define	REG_BRIDGE2_SPLIT_ENGINE_BOX_P_CONDENSER			151				// Engine Box Condenser Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar

	#define	REG_BRIDGE2_SPLIT_ENGINE_BOX_COMPRESSOR2_SPEED		155				// "Engine Box Inverter Compressor2 Speed: (percentage)

                                        	   	
	#define	REG_BRIDGE2_UTA_H_ENGINE_COMPRESSOR_SPEED			160				// "UTA H Engine Box Inverter Compressor Speed: (percentage)
																				// Range: 0..100%"
	#define	REG_BRIDGE2_UTA_H_ENGINE_REQ_PRESSURE				162				// UTA H Engine Box Max Air Handler Required Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar
	#define	REG_BRIDGE2_UTA_H_ENGINE_P_GAS						169				// UTA H Engine Box Gas Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar
	#define	REG_BRIDGE2_UTA_H_ENGINE_P_LIQUID					170				// UTA H Engine Box Liquid Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar

	#define	REG_BRIDGE2_ABSORPTION_WATT_AC_COMPRESSOR_1			182				// Instantaneous absorption control of the AC compressor 1 (WATT)
	#define	REG_BRIDGE2_ABSORPTION_WATT_AC_COMPRESSOR_2			183				// Instantaneous absorption control of the AC compressor 2 (WATT)
	#define	REG_BRIDGE2_ABSORPTION_WATT_SEA_WATER_PUMP			184				// Instantaneous absorption control of the AC Sea Water Pump (WATT)

	#define	REG_BRIDGE2_ABSORPTION_AC_COMPRESSOR_1				185				// Instantaneous absorption control of the AC compressor 1 (Ampere)
	#define	REG_BRIDGE2_ABSORPTION_AC_COMPRESSOR_2				186				// Instantaneous absorption control of the AC compressor 2 (Ampere)
	#define	REG_BRIDGE2_ABSORPTION_SEA_WATER_PUMP				187				// Instantaneous absorption control of the AC Sea Water Pump (Ampere)
	#define	REG_BRIDGE2_ABSORPTION_WATT_AC_TOTAL				188				// Instantaneous absorption control of the AC Total (WATT)
	#define	REG_BRIDGE2_ABSORPTION_AC_TOTAL     				189				// Instantaneous absorption control of the AC Total (Ampere)
				
	#define	REG_BRIDGE2_ACTIVE_ERROR							190				// General Active Error:
																				// bit 0: Engine Box Error
																				// bit 1: Split Error
																				// bit 2: UTA Box Error
																				// bit 3: UTA Split Error
																				// bit 4: not used
																				// bit 5: not used
																				// bit 6: not used
																				// bit 7: not used
																				// bit 8: not used
																				// bit 9: not used
																				// bit 10: not used
																				// bit 11: not used
																				// bit 12: not used
																				// bit 13: not used
																				// bit 14: not used
																				// bit 15: not used




	#define	REG_BRIDGE2_SPLIT_01_READ_PWR						200				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_01_READ_SP_ROOM					201				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_01_READ_FAN_SPEED					202				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_01_READ_AIR_IN					203				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_01_READ_STATUS					204				// Diagnostic Split Comunication Status: 
	#define	REG_BRIDGE2_SPLIT_01_UTA_MODE						205				// Free 1
	#define	REG_BRIDGE2_SPLIT_01_READ_HEATER_PWR				206				// Free 2
	#define	REG_BRIDGE2_SPLIT_01_READ_HUMIDITY_MIDDLE   		207				// Free 3 
	#define	REG_BRIDGE2_SPLIT_01_READ_FREE4						208				// Free 4 
	#define	REG_BRIDGE2_SPLIT_01_READ_FREE5						209				// Free 5 

	#define	REG_BRIDGE2_SPLIT_02_READ_PWR						210				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_02_READ_SP_ROOM					211				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_02_READ_FAN_SPEED					212				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_02_READ_AIR_IN					213				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_02_READ_STATUS					214				// Free Location
	#define	REG_BRIDGE2_SPLIT_02_UTA_MODE						215				// Free 1
	#define	REG_BRIDGE2_SPLIT_02_READ_HEATER_PWR				216				// Free 2
	#define	REG_BRIDGE2_SPLIT_02_READ_HUMIDITY_MIDDLE   		217				// Free 3 
	#define	REG_BRIDGE2_SPLIT_02_READ_FREE4						218				// Free 4 
	#define	REG_BRIDGE2_SPLIT_02_READ_FREE5						219				// Free 5 

	#define	REG_BRIDGE2_SPLIT_03_READ_PWR						220				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_03_READ_SP_ROOM					221				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_03_READ_FAN_SPEED					222				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_03_READ_AIR_IN					223				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_03_READ_STATUS					224				// Free Location
	#define	REG_BRIDGE2_SPLIT_03_UTA_MODE						225				// Free 1
	#define	REG_BRIDGE2_SPLIT_03_READ_HEATER_PWR				226				// Free 2
	#define	REG_BRIDGE2_SPLIT_03_READ_HUMIDITY_MIDDLE   		227				// Free 3 
	#define	REG_BRIDGE2_SPLIT_03_READ_FREE4						228				// Free 4 
	#define	REG_BRIDGE2_SPLIT_03_READ_FREE5						229				// Free 5 		
	
	#define	REG_BRIDGE2_SPLIT_04_READ_PWR						230				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_04_READ_SP_ROOM					231				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_04_READ_FAN_SPEED					232				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_04_READ_AIR_IN					233				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_04_READ_STATUS					234				// Free Location
	#define	REG_BRIDGE2_SPLIT_04_UTA_MODE						235				// Free 1
	#define	REG_BRIDGE2_SPLIT_04_READ_HEATER_PWR				236				// Free 2
	#define	REG_BRIDGE2_SPLIT_04_READ_HUMIDITY_MIDDLE   		237				// Free 3 
	#define	REG_BRIDGE2_SPLIT_04_READ_FREE4						238				// Free 4 
	#define	REG_BRIDGE2_SPLIT_04_READ_FREE5						239				// Free 5 

	#define	REG_BRIDGE2_SPLIT_05_READ_PWR						240				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_05_READ_SP_ROOM					241				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_05_READ_FAN_SPEED					242				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_05_READ_AIR_IN					243				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_05_READ_STATUS					244				// Free Location
	#define	REG_BRIDGE2_SPLIT_05_UTA_MODE						245				// Free 1
	#define	REG_BRIDGE2_SPLIT_05_READ_HEATER_PWR				246				// Free 2
	#define	REG_BRIDGE2_SPLIT_05_READ_HUMIDITY_MIDDLE   		247				// Free 3 
	#define	REG_BRIDGE2_SPLIT_05_READ_FREE4						248				// Free 4 
	#define	REG_BRIDGE2_SPLIT_05_READ_FREE5						249				// Free 5 

	#define	REG_BRIDGE2_SPLIT_06_READ_PWR						250				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_06_READ_SP_ROOM					251				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_06_READ_FAN_SPEED					252				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_06_READ_AIR_IN					253				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_06_READ_STATUS					254				// Free Location
	#define	REG_BRIDGE2_SPLIT_06_UTA_MODE						255				// Free 1
	#define	REG_BRIDGE2_SPLIT_06_READ_HEATER_PWR				256				// Free 2
	#define	REG_BRIDGE2_SPLIT_06_READ_HUMIDITY_MIDDLE   		257				// Free 3 
	#define	REG_BRIDGE2_SPLIT_06_READ_FREE4						258				// Free 4 
	#define	REG_BRIDGE2_SPLIT_06_READ_FREE5						259				// Free 5 

	#define	REG_BRIDGE2_SPLIT_07_READ_PWR						260				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_07_READ_SP_ROOM					261				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_07_READ_FAN_SPEED					262				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_07_READ_AIR_IN					263				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_07_READ_STATUS					264				// Free Location
	#define	REG_BRIDGE2_SPLIT_07_UTA_MODE						265				// Free 1
	#define	REG_BRIDGE2_SPLIT_07_READ_HEATER_PWR				266				// Free 2
	#define	REG_BRIDGE2_SPLIT_07_READ_HUMIDITY_MIDDLE   		267				// Free 3 
	#define	REG_BRIDGE2_SPLIT_07_READ_FREE4						268				// Free 4 
	#define	REG_BRIDGE2_SPLIT_07_READ_FREE5						269				// Free 5 

	#define	REG_BRIDGE2_SPLIT_08_READ_PWR						270				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_08_READ_SP_ROOM					271				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_08_READ_FAN_SPEED					272				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_08_READ_AIR_IN					273				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_08_READ_STATUS					274				// Free Location
	#define	REG_BRIDGE2_SPLIT_08_UTA_MODE						275				// Free 1
	#define	REG_BRIDGE2_SPLIT_08_READ_HEATER_PWR				276				// Free 2
	#define	REG_BRIDGE2_SPLIT_08_READ_HUMIDITY_MIDDLE   		277				// Free 3 
	#define	REG_BRIDGE2_SPLIT_08_READ_FREE4						278				// Free 4 
	#define	REG_BRIDGE2_SPLIT_08_READ_FREE5						279				// Free 5 

	#define	REG_BRIDGE2_SPLIT_09_READ_PWR						280				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_09_READ_SP_ROOM					281				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_09_READ_FAN_SPEED					282				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_09_READ_AIR_IN					283				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_09_READ_STATUS					284				// Free Location
	#define	REG_BRIDGE2_SPLIT_09_UTA_MODE						285				// Free 1
	#define	REG_BRIDGE2_SPLIT_09_READ_HEATER_PWR				286				// Free 2
	#define	REG_BRIDGE2_SPLIT_09_READ_HUMIDITY_MIDDLE   		287				// Free 3 
	#define	REG_BRIDGE2_SPLIT_09_READ_FREE4						288				// Free 4 
	#define	REG_BRIDGE2_SPLIT_09_READ_FREE5						289				// Free 5 

	#define	REG_BRIDGE2_SPLIT_10_READ_PWR						290				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_10_READ_SP_ROOM					291				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_10_READ_FAN_SPEED					292				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_10_READ_AIR_IN					293				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_10_READ_STATUS					294				// Free Location
	#define	REG_BRIDGE2_SPLIT_10_UTA_MODE						295				// Free 1
	#define	REG_BRIDGE2_SPLIT_10_READ_HEATER_PWR				296				// Free 2
	#define	REG_BRIDGE2_SPLIT_10_READ_HUMIDITY_MIDDLE   		297				// Free 3 
	#define	REG_BRIDGE2_SPLIT_10_READ_FREE4						298				// Free 4 
	#define	REG_BRIDGE2_SPLIT_10_READ_FREE5						299				// Free 5 

	#define	REG_BRIDGE2_SPLIT_11_READ_PWR						300				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_11_READ_SP_ROOM					301				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_11_READ_FAN_SPEED					302				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_11_READ_AIR_IN					303				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_11_READ_STATUS					304				// Free Location
	#define	REG_BRIDGE2_SPLIT_11_UTA_MODE						305				// Free 1
	#define	REG_BRIDGE2_SPLIT_11_READ_HEATER_PWR				306				// Free 2
	#define	REG_BRIDGE2_SPLIT_11_READ_HUMIDITY_MIDDLE   		307				// Free 3 
	#define	REG_BRIDGE2_SPLIT_11_READ_FREE4						308				// Free 4 
	#define	REG_BRIDGE2_SPLIT_11_READ_FREE5						309				// Free 5 

	#define	REG_BRIDGE2_SPLIT_12_READ_PWR						310				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_12_READ_SP_ROOM					311				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_12_READ_FAN_SPEED					312				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_12_READ_AIR_IN					313				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_12_READ_STATUS					314				// Free Location
	#define	REG_BRIDGE2_SPLIT_12_UTA_MODE						315				// Free 1
	#define	REG_BRIDGE2_SPLIT_12_READ_HEATER_PWR				316				// Free 2
	#define	REG_BRIDGE2_SPLIT_12_READ_HUMIDITY_MIDDLE   		317				// Free 3 
	#define	REG_BRIDGE2_SPLIT_12_READ_FREE4						318				// Free 4 
	#define	REG_BRIDGE2_SPLIT_12_READ_FREE5						319				// Free 5 

	#define	REG_BRIDGE2_SPLIT_13_READ_PWR						320				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_13_READ_SP_ROOM					321				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_13_READ_FAN_SPEED					322				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_13_READ_AIR_IN					323				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_13_READ_STATUS					324				// Free Location
	#define	REG_BRIDGE2_SPLIT_13_UTA_MODE						325				// Free 1
	#define	REG_BRIDGE2_SPLIT_13_READ_HEATER_PWR				326				// Free 2
	#define	REG_BRIDGE2_SPLIT_13_READ_HUMIDITY_MIDDLE   		327				// Free 3 
	#define	REG_BRIDGE2_SPLIT_13_READ_FREE4						328				// Free 4 
	#define	REG_BRIDGE2_SPLIT_13_READ_FREE5						329				// Free 5

	#define	REG_BRIDGE2_SPLIT_14_READ_PWR						330				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_14_READ_SP_ROOM					331				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_14_READ_FAN_SPEED					332				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_14_READ_AIR_IN					333				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_14_READ_STATUS					334				// Free Location
	#define	REG_BRIDGE2_SPLIT_14_UTA_MODE						335				// Free 1
	#define	REG_BRIDGE2_SPLIT_14_READ_HEATER_PWR				336				// Free 2
	#define	REG_BRIDGE2_SPLIT_14_READ_HUMIDITY_MIDDLE   		337				// Free 3 
	#define	REG_BRIDGE2_SPLIT_14_READ_FREE4						338				// Free 4 
	#define	REG_BRIDGE2_SPLIT_14_READ_FREE5						339				// Free 5 

	#define	REG_BRIDGE2_SPLIT_15_READ_PWR						340				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_15_READ_SP_ROOM					341				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_15_READ_FAN_SPEED					342				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_15_READ_AIR_IN					343				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_15_READ_STATUS					344				// Free Location
	#define	REG_BRIDGE2_SPLIT_15_UTA_MODE						345				// Free 1
	#define	REG_BRIDGE2_SPLIT_15_READ_HEATER_PWR				346				// Free 2
	#define	REG_BRIDGE2_SPLIT_15_READ_HUMIDITY_MIDDLE   		347				// Free 3 
	#define	REG_BRIDGE2_SPLIT_15_READ_FREE4						348				// Free 4 
	#define	REG_BRIDGE2_SPLIT_15_READ_FREE5						349				// Free 5 

	#define	REG_BRIDGE2_SPLIT_16_READ_PWR						350				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_16_READ_SP_ROOM					351				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_16_READ_FAN_SPEED					352				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_16_READ_AIR_IN					353				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_16_READ_STATUS					354				// Free Location
	#define	REG_BRIDGE2_SPLIT_16_UTA_MODE						355				// Free 1
	#define	REG_BRIDGE2_SPLIT_16_READ_HEATER_PWR				356				// Free 2
	#define	REG_BRIDGE2_SPLIT_16_READ_HUMIDITY_MIDDLE   		357				// Free 3 
	#define	REG_BRIDGE2_SPLIT_16_READ_FREE4						358				// Free 4 
	#define	REG_BRIDGE2_SPLIT_16_READ_FREE5						359				// Free 5 

	#define	REG_BRIDGE2_SPLIT_17_READ_PWR						360				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_17_READ_SP_ROOM					361				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_17_READ_FAN_SPEED					362				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_17_READ_AIR_IN					363				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_17_READ_STATUS					364				// Free Location
	#define	REG_BRIDGE2_SPLIT_17_UTA_MODE						365				// Free 1
	#define	REG_BRIDGE2_SPLIT_17_READ_HEATER_PWR				366				// Free 2
	#define	REG_BRIDGE2_SPLIT_17_READ_HUMIDITY_MIDDLE   		367				// Free 3 
	#define	REG_BRIDGE2_SPLIT_17_READ_FREE4						368				// Free 4 
	#define	REG_BRIDGE2_SPLIT_17_READ_FREE5						369				// Free 5 

	#define	REG_BRIDGE2_SPLIT_18_READ_PWR						370				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_18_READ_SP_ROOM					371				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_18_READ_FAN_SPEED					372				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_18_READ_AIR_IN					373				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_18_READ_STATUS					374				// Free Location
	#define	REG_BRIDGE2_SPLIT_18_UTA_MODE						375				// Free 1
	#define	REG_BRIDGE2_SPLIT_18_READ_HEATER_PWR				376				// Free 2
	#define	REG_BRIDGE2_SPLIT_18_READ_HUMIDITY_MIDDLE   		377				// Free 3 
	#define	REG_BRIDGE2_SPLIT_18_READ_FREE4						378				// Free 4 
	#define	REG_BRIDGE2_SPLIT_18_READ_FREE5						379				// Free 5 

	#define	REG_BRIDGE2_SPLIT_19_READ_PWR						380				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_19_READ_SP_ROOM					381				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_19_READ_FAN_SPEED					382				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_19_READ_AIR_IN					383				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_19_READ_STATUS					384				// Free Location
	#define	REG_BRIDGE2_SPLIT_19_UTA_MODE						385				// Free 1
	#define	REG_BRIDGE2_SPLIT_19_READ_HEATER_PWR				386				// Free 2
	#define	REG_BRIDGE2_SPLIT_19_READ_HUMIDITY_MIDDLE   		387				// Free 3 
	#define	REG_BRIDGE2_SPLIT_19_READ_FREE4						388				// Free 4 
	#define	REG_BRIDGE2_SPLIT_19_READ_FREE5						389				// Free 5 

	#define	REG_BRIDGE2_SPLIT_20_READ_PWR						390				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_20_READ_SP_ROOM					391				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_20_READ_FAN_SPEED					392				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_20_READ_AIR_IN					393				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_20_READ_STATUS					394				// Free Location
	#define	REG_BRIDGE2_SPLIT_20_UTA_MODE						395				// Free 1
	#define	REG_BRIDGE2_SPLIT_20_READ_HEATER_PWR				396				// Free 2
	#define	REG_BRIDGE2_SPLIT_20_READ_HUMIDITY_MIDDLE   		397				// Free 3 
	#define	REG_BRIDGE2_SPLIT_20_READ_FREE4						398				// Free 4 
	#define	REG_BRIDGE2_SPLIT_20_READ_FREE5						399				// Free 5 

	#define	REG_BRIDGE2_SPLIT_21_READ_PWR						400				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_21_READ_SP_ROOM					401				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_21_READ_FAN_SPEED					402				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_21_READ_AIR_IN					403				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_21_READ_STATUS					404				// Free Location
	#define	REG_BRIDGE2_SPLIT_21_UTA_MODE						405				// Free 1
	#define	REG_BRIDGE2_SPLIT_21_READ_HEATER_PWR				406				// Free 2
	#define	REG_BRIDGE2_SPLIT_21_READ_HUMIDITY_MIDDLE   		407				// Free 3 
	#define	REG_BRIDGE2_SPLIT_21_READ_FREE4						408				// Free 4 
	#define	REG_BRIDGE2_SPLIT_21_READ_FREE5						409				// Free 5 

	#define	REG_BRIDGE2_SPLIT_22_READ_PWR						410				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_22_READ_SP_ROOM					411				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_22_READ_FAN_SPEED					412				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_22_READ_AIR_IN					413				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_22_READ_STATUS					414				// Free Location
	#define	REG_BRIDGE2_SPLIT_22_UTA_MODE						415				// Free 1
	#define	REG_BRIDGE2_SPLIT_22_READ_HEATER_PWR				416				// Free 2
	#define	REG_BRIDGE2_SPLIT_22_READ_HUMIDITY_MIDDLE   		417				// Free 3 
	#define	REG_BRIDGE2_SPLIT_22_READ_FREE4						418				// Free 4 
	#define	REG_BRIDGE2_SPLIT_22_READ_FREE5						419				// Free 5 

	#define	REG_BRIDGE2_SPLIT_23_READ_PWR						420				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_23_READ_SP_ROOM					421				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_23_READ_FAN_SPEED					422				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_23_READ_AIR_IN					423				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_23_READ_STATUS					424				// Free Location
	#define	REG_BRIDGE2_SPLIT_23_UTA_MODE						425				// Free 1
	#define	REG_BRIDGE2_SPLIT_23_READ_HEATER_PWR				426				// Free 2
	#define	REG_BRIDGE2_SPLIT_23_READ_HUMIDITY_MIDDLE   		427				// Free 3 
	#define	REG_BRIDGE2_SPLIT_23_READ_FREE4						428				// Free 4 
	#define	REG_BRIDGE2_SPLIT_23_READ_FREE5						429				// Free 5 

	#define	REG_BRIDGE2_SPLIT_24_READ_PWR						430				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_24_READ_SP_ROOM					431				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_24_READ_FAN_SPEED					432				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_24_READ_AIR_IN					433				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_24_READ_STATUS					434				// Free Location
	#define	REG_BRIDGE2_SPLIT_24_UTA_MODE						435				// Free 1
	#define	REG_BRIDGE2_SPLIT_24_READ_HEATER_PWR				436				// Free 2
	#define	REG_BRIDGE2_SPLIT_24_READ_HUMIDITY_MIDDLE   		437				// Free 3 
	#define	REG_BRIDGE2_SPLIT_24_READ_FREE4						438				// Free 4 
	#define	REG_BRIDGE2_SPLIT_24_READ_FREE5						439				// Free 5 

	#define	REG_BRIDGE2_SPLIT_25_READ_PWR						440				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_25_READ_SP_ROOM					441				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_25_READ_FAN_SPEED					442				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_25_READ_AIR_IN					443				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_25_READ_STATUS					444				// Free Location
	#define	REG_BRIDGE2_SPLIT_25_UTA_MODE						445				// Free 1
	#define	REG_BRIDGE2_SPLIT_25_READ_HEATER_PWR				446				// Free 2
	#define	REG_BRIDGE2_SPLIT_25_READ_HUMIDITY_MIDDLE   		447				// Free 3 
	#define	REG_BRIDGE2_SPLIT_25_READ_FREE4						448				// Free 4 
	#define	REG_BRIDGE2_SPLIT_25_READ_FREE5						449				// Free 5 

	#define	REG_BRIDGE2_SPLIT_26_READ_PWR						450				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_26_READ_SP_ROOM					451				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_26_READ_FAN_SPEED					452				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_26_READ_AIR_IN					453				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_26_READ_STATUS					454				// Free Location
	#define	REG_BRIDGE2_SPLIT_26_UTA_MODE						455				// Free 1
	#define	REG_BRIDGE2_SPLIT_26_READ_HEATER_PWR				456				// Free 2
	#define	REG_BRIDGE2_SPLIT_26_READ_HUMIDITY_MIDDLE   		457				// Free 3 
	#define	REG_BRIDGE2_SPLIT_26_READ_FREE4						458				// Free 4 
	#define	REG_BRIDGE2_SPLIT_26_READ_FREE5						459				// Free 5 

	#define	REG_BRIDGE2_SPLIT_27_READ_PWR						460				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_27_READ_SP_ROOM					461				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_27_READ_FAN_SPEED					462				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_27_READ_AIR_IN					463				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_27_READ_STATUS					464				// Free Location
	#define	REG_BRIDGE2_SPLIT_27_UTA_MODE						465				// Free 1
	#define	REG_BRIDGE2_SPLIT_27_READ_HEATER_PWR				466				// Free 2
	#define	REG_BRIDGE2_SPLIT_27_READ_HUMIDITY_MIDDLE   		467				// Free 3 
	#define	REG_BRIDGE2_SPLIT_27_READ_FREE4						468				// Free 4 
	#define	REG_BRIDGE2_SPLIT_27_READ_FREE5						469				// Free 5 

	#define	REG_BRIDGE2_SPLIT_28_READ_PWR						470				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_28_READ_SP_ROOM					471				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_28_READ_FAN_SPEED					472				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_28_READ_AIR_IN					473				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_28_READ_STATUS					474				// Free Location
	#define	REG_BRIDGE2_SPLIT_28_UTA_MODE						475				// Free 1
	#define	REG_BRIDGE2_SPLIT_28_READ_HEATER_PWR				476				// Free 2
	#define	REG_BRIDGE2_SPLIT_28_READ_HUMIDITY_MIDDLE   		477				// Free 3 
	#define	REG_BRIDGE2_SPLIT_28_READ_FREE4						478				// Free 4 
	#define	REG_BRIDGE2_SPLIT_28_READ_FREE5						479				// Free 5 

	#define	REG_BRIDGE2_SPLIT_29_READ_PWR						480				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_29_READ_SP_ROOM					481				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_29_READ_FAN_SPEED					482				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_29_READ_AIR_IN					483				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_29_READ_STATUS					484				// Free Location
	#define	REG_BRIDGE2_SPLIT_29_UTA_MODE						485				// Free 1
	#define	REG_BRIDGE2_SPLIT_29_READ_HEATER_PWR				486				// Free 2
	#define	REG_BRIDGE2_SPLIT_29_READ_HUMIDITY_MIDDLE   		487				// Free 3 
	#define	REG_BRIDGE2_SPLIT_29_READ_FREE4						488				// Free 4 
	#define	REG_BRIDGE2_SPLIT_29_READ_FREE5						489				// Free 5 

	#define	REG_BRIDGE2_SPLIT_30_READ_PWR						490				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_30_READ_SP_ROOM					491				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_30_READ_FAN_SPEED					492				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_30_READ_AIR_IN					493				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_30_READ_STATUS					494				// Free Location
	#define	REG_BRIDGE2_SPLIT_30_UTA_MODE						495				// Free 1
	#define	REG_BRIDGE2_SPLIT_30_READ_HEATER_PWR				496				// Free 2
	#define	REG_BRIDGE2_SPLIT_30_READ_HUMIDITY_MIDDLE   		497				// Free 3 
	#define	REG_BRIDGE2_SPLIT_30_READ_FREE4						498				// Free 4 
	#define	REG_BRIDGE2_SPLIT_30_READ_FREE5						499				// Free 5 

	#define	REG_BRIDGE2_SPLIT_31_READ_PWR						500				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_31_READ_SP_ROOM					501				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_31_READ_FAN_SPEED					502				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_31_READ_AIR_IN					503				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_31_READ_STATUS					504				// Free Location
	#define	REG_BRIDGE2_SPLIT_31_UTA_MODE						505				// Free 1
	#define	REG_BRIDGE2_SPLIT_31_READ_HEATER_PWR				506				// Free 2
	#define	REG_BRIDGE2_SPLIT_31_READ_HUMIDITY_MIDDLE   		507				// Free 3 
	#define	REG_BRIDGE2_SPLIT_31_READ_FREE4						508				// Free 4 
	#define	REG_BRIDGE2_SPLIT_31_READ_FREE5						509				// Free 5 

	#define	REG_BRIDGE2_SPLIT_32_READ_PWR						510				// Unit Power On/Off  0=OFF 1=ON
	#define	REG_BRIDGE2_SPLIT_32_READ_SP_ROOM					511				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 °C // Range: 18..30°C"							
	#define	REG_BRIDGE2_SPLIT_32_READ_FAN_SPEED					512				// "Unit Fan Speed: 
	#define	REG_BRIDGE2_SPLIT_32_READ_AIR_IN					513				// Unit Input Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 °C
	#define	REG_BRIDGE2_SPLIT_32_READ_STATUS					514				// Free Location
	#define	REG_BRIDGE2_SPLIT_32_UTA_MODE						515				// Free 1
	#define	REG_BRIDGE2_SPLIT_32_READ_HEATER_PWR				516				// Free 2
	#define	REG_BRIDGE2_SPLIT_32_READ_HUMIDITY_MIDDLE   		517				// Free 3 
	#define	REG_BRIDGE2_SPLIT_32_READ_FREE4						518				// Free 4 
	#define	REG_BRIDGE2_SPLIT_32_READ_FREE5						519				// Free 5 
																
#endif

