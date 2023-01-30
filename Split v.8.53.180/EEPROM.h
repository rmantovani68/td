//----------------------------------------------------------------------------------
//	Progect name:	EEPROM.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			22/05/2011
//	Description:	Header con la tabella di allocazione della E2 del micro
//----------------------------------------------------------------------------------

/*
 * 
 * EEPROM utilizzazta: 24LC16B
 * Marca: Microchip
 * Quantità memoria: 16Kbits -> 16384 / 8 = 2048 Bytes
 * N. Blocchi: 8 da 256 Bytes 
 * Cicli erase/write: 1,000,000 
 * 
*/
#include "HWSelection.h"
#if (GB210==1) 
	#include <p24FJ256GB210.h>
#elif (DA210==1)
	#include <p24FJ256DA210.h>
#else
	#include <p24FJ256GB110.h>
#endif

#ifndef _EEPROM_
	#define _EEPROM_


	#define	K_Test_EEPROM_ReadWrite     0x1234	// Numero magico per effettuare il test di Read/Write ad ogni partenza (vedi Routine "CheckEEPROM")
	#define	K_Test_EEPROM_Retention     0x5678  // Numero magico per effettuare il test di verifica "Data Retention" ad ogni partenza ma che è scritto solo una volta durante la fase di HW_Test
	#define	K_Reset_EEPROM_Value        0xFFFF  // Valore di "reset" locazioni EEPROM

    #define	K_Test_EEPROM_TestPattern1  0xAAAA  // Numero magico per effettuare il test di verifica "Data Retention" ad ogni partenza ma che è scritto solo una volta durante la fase di HW_Test
    #define	K_Test_EEPROM_TestPattern2  0x5555  // Numero magico per effettuare il test di verifica "Data Retention" ad ogni partenza ma che è scritto solo una volta durante la fase di HW_Test

#if (GB210==1 || DA210==1)
    #define	K_EEPROM_ByteSize           0x800   // Dimensione EEPROM in Byte -> 16Kbit -> 2048byte -> 8page x 256 byte
    #define	K_EEPROM_WordSize           K_EEPROM_ByteSize/2   // Dimensione EEPROM in Word -> 16Kbit -> 1024word -> 8page x 128 word
    #define	K_EEPROM_PageSize           0x100   // Dimensione pagina EEPROM in Byte -> 256 byte

    #define	K_EEPROM_Primary_Sector     1       // Numero settore primario per il salvataggio dei dati      otto settori disponibili 0..7
    #define	K_EEPROM_Secondary_Sector   7       // Numero settore secondario per il salvataggio dei dati
#else
    #define	K_EEPROM_ByteSize           0x200//0x800   // Dimensione EEPROM in Byte -> 16Kbit -> 2048byte -> 8page x 256 byte
    #define	K_EEPROM_WordSize           K_EEPROM_ByteSize/2   // Dimensione EEPROM in Word -> 16Kbit -> 1024word -> 8page x 128 word
    #define	K_EEPROM_PageSize           0x100   // Dimensione pagina EEPROM in Byte -> 256 byte

    #define	K_EEPROM_Primary_Sector     0//1       // Numero settore primario per il salvataggio dei dati      otto settori disponibili 0..7
    #define	K_EEPROM_Secondary_Sector   1//7       // Numero settore secondario per il salvataggio dei dati
#endif


    #define	K_UseRealE2                 0       // Switch per utilizzare le funzioni Read/Write sulla EEPROM reale
    #define	K_UseVirtualE2              1       // Switch per utilizzare le funzioni di Read dal buffer di EEPROM virtuale (per velocizzare le operazioni di lettura durante il calcolo del CRC)



	//----------------------------------------------------------------------------------
	//	DEFINE indirizzi E2 - Banco 0
	//----------------------------------------------------------------------------------
	#define EEPROM_Add                  0xA0	// indirizzo di base della EEPROM esterna       b7 b6 b5 b4 b3 b2 b1 b0
                                                //                                              |        |  |     |  |
                                                //                                               base addr    page   R/W        (R/W\ -> 1= Read  0=Write)
                                                //                                              1  0  1  0  x  x  x  0/1  
                                                //                                              
                                                //                                              Ex.: A0 -> Scrittura in pagina 0
                                                //                                              Ex.: A1 -> Lettura in pagina 0
                                                //                                              Ex.: A2 -> Scrittura in pagina 1
                                                //                                              Ex.: A9 -> Lettura in pagina 4
                                                //                                              Ex.: AE -> Scrittura in pagina 7
	
	#define	Test_EEPROM_ReadWrite_B0    0x00	// Byte riservato al test di Scrittura/Lettura che si effettua a tutte le accensioni

    #define	CRC_Hi_Flash                0x00	// CRC high byte per il controllo della flash
	#define	CRC_Lo_Flash                0x01	// CRC high byte per il controllo della flash
	#define	System_SN_1                 0x02	// 1° byte del serial number dell'impianto
	#define	System_SN_2                 0x03	// 2° byte del serial number dell'impianto
	#define	System_SN_3                 0x04	// 3° byte del serial number dell'impianto
	#define	System_SN_4                 0x05	// 4° byte del serial number dell'impianto
	#define	Fab_Day                     0x06	// Giorno della fabbricazione
	#define	Fab_Month                   0x07	// Mese di fabbricazione
	#define	Fab_Year                    0x08	// Anno di fabbricazione
	


	#define E2_START_OF_SPACE_Hi        0x1E    // Locazione spare per identificare inizio spazio EEPROM (comodo per calcolo CRC)
	#define E2_START_OF_SPACE_Lo        0x1F
	#define E2_SPLIT_Heater_Pwr_Hi      0x20    //CD
	#define E2_SPLIT_Heater_Pwr_Lo      0x21
	#define E2_SPLIT_PumpSwitchTime_Hi	0x22
	#define E2_SPLIT_PumpSwitchTime_Lo	0x23
	#define E2_SPLIT_PumpSelection_Hi	0x24
	#define E2_SPLIT_PumpSelection_Lo	0x25
	#define E2_SPLIT_SystemDisable_Hi	0x26		// Touch[0].SystemDisable
	#define E2_SPLIT_SystemDisable_Lo	0x27		//
	#define E2_SPLIT_Uta_Power_Hi		0x28		// Touch[0].Uta_Power
	#define E2_SPLIT_Uta_Power_Lo		0x29		// 
	#define	E2_SPLIT_Uta_Mode_Hi		0x2A		// Touch[0].Uta_Mode
	#define	E2_SPLIT_Uta_Mode_Lo		0x2B		// 
	#define	E2_SPLIT_Uta_SP_Room_Hi		0x2C		// Touch[0].Uta_SP_Room
	#define	E2_SPLIT_Uta_SP_Room_Lo		0x2D		// 
	#define	E2_SPLIT_Uta_SP_Humi_Hi		0x2E		// Touch[0].Uta_SP_Humi
	#define	E2_SPLIT_Uta_SP_Humi_Lo		0x2F		// 
	#define	E2_SPLIT_Uta_Fan_Speed_Hi	0x30		// Touch[0].Uta_Fan_Speed
	#define	E2_SPLIT_Uta_Fan_Speed_Lo	0x31		// 
	#define	E2_SPLIT_Uta_SP_Room_F_Hi	0x32		// Touch[0].Uta_SP_Room_F
	#define	E2_SPLIT_Uta_SP_Room_F_Lo	0x33		// 						
	#define	E2_SPLIT_ECOMODEEN_Hi       0x34        //E1
	#define	E2_SPLIT_ECOMODEEN_Lo       0x35
	#define	E2_SPLIT_PWR_ECOMODE_Hi     0x36
	#define	E2_SPLIT_PWR_ECOMODE_Lo     0x37
    #define E2_SPLIT_NATION_Hi          0x38        //EF
    #define E2_SPLIT_NATION_Lo          0x39
	#define	E2_SPLIT_FanMode_Hi         0x3A			// Touch[0].FanMode
	#define	E2_SPLIT_FanMode_Lo         0x3B
	#define	E2_SPLIT_SP_Hi              0x3C			// Touch[0].SetPoint
	#define	E2_SPLIT_SP_Lo              0x3D
	#define	E2_SPLIT_Mode_Hi            0x3E			// Touch[0].DefMode
	#define	E2_SPLIT_Mode_Lo            0x3F
	#define	E2_SPLIT_Pwr_Hi             0x40			// Touch[0].Script_Split_Pwr
	#define	E2_SPLIT_Pwr_Lo             0x41	
	#define	E2_PWR_Limit_Hi             0x42			// Touch[0].PowerLimit
	#define	E2_PWR_Limit_Lo             0x43			 
	#define	E2_SPLIT_SP_F_Hi            0x44			// Touch[0].Script_SetPoint_F
	#define	E2_SPLIT_SP_F_Lo            0x45

	#define	E2_SPLIT_ON_PRI_MODE_Hi     0x46			// 
	#define	E2_SPLIT_ON_PRI_MODE_Lo     0x47												
	#define	E2_VALVE_ONDEM_MIN_PERC_Hi  0x48			// 
	#define	E2_VALVE_ONDEM_MIN_PERC_Lo  0x49												
	#define	E2_MAXLIM_ONDEM_TOTSCORE_Hi 0x4A			// 
	#define	E2_MAXLIM_ONDEM_TOTSCORE_Lo 0x4B												
	#define	E2_TIME_ONDEM_VALVE_SW_Hi   0x4C			// 
	#define	E2_TIME_ONDEM_VALVE_SW_Lo   0x4D						


	#define	E2_TOUCH_TESTALL_ABIL_Hi                0x4E						
	#define	E2_TOUCH_TESTALL_ABIL_Lo                0x4F						
	#define	E2_TOUCH_TESTALL_SUPERHEAT_SP_COOL_Hi   0x50						
	#define	E2_TOUCH_TESTALL_SUPERHEAT_SP_COOL_Lo   0x51						
	#define	E2_TOUCH_TESTALL_SUPERHEAT_SP_HEAT_Hi   0x52						
	#define	E2_TOUCH_TESTALL_SUPERHEAT_SP_HEAT_Lo   0x53						
	#define	E2_TOUCH_TESTALL_MAX_OPEN_VALVE_COOL_Hi 0x54						
	#define	E2_TOUCH_TESTALL_MAX_OPEN_VALVE_COOL_Lo 0x55						
	#define	E2_TOUCH_TESTALL_MAX_OPEN_VALVE_HEAT_Hi 0x56						
	#define	E2_TOUCH_TESTALL_MAX_OPEN_VALVE_HEAT_Lo 0x57						
	#define	E2_TOUCH_TESTALL_WORK_TEMP_COOL_Hi      0x58						
	#define	E2_TOUCH_TESTALL_WORK_TEMP_COOL_Lo      0x59						
	#define	E2_TOUCH_TESTALL_WORK_TEMP_HEAT_Hi      0x5A						
	#define	E2_TOUCH_TESTALL_WORK_TEMP_HEAT_Lo      0x5B						


	#define	E2_TOUCH_TESTALL_FROZEN_ABIL_Hi         0x5C						
	#define	E2_TOUCH_TESTALL_FROZEN_ABIL_Lo         0x5D						
	#define	E2_TOUCH_TESTALL_SUPERHEAT_SP_FRIGO_Hi  0x5E						
	#define	E2_TOUCH_TESTALL_SUPERHEAT_SP_FRIGO_Lo  0x5F						
	#define	E2_TOUCH_TESTALL_SUPERHEAT_SP_FREEZER_Hi    0x60						
	#define	E2_TOUCH_TESTALL_SUPERHEAT_SP_FREEZER_Lo    0x61						
	#define	E2_TOUCH_TESTALL_MAXOPVALVE_FRIGO_Hi    0x62						
	#define	E2_TOUCH_TESTALL_MAXOPVALVE_FRIGO_Lo    0x63						
	#define	E2_TOUCH_TESTALL_MAXOPVALVE_FREEZER_Hi  0x64						
	#define	E2_TOUCH_TESTALL_MAXOPVALVE_FREEZER_Lo  0x65						
	#define	E2_TOUCH_TESTALL_TEMPBATT_SP_FRIGO_Hi   0x66						
	#define	E2_TOUCH_TESTALL_TEMPBATT_SP_FRIGO_Lo   0x67						
	#define	E2_TOUCH_TESTALL_TEMPBATT_SP_FREEZER_Hi 0x68						
	#define	E2_TOUCH_TESTALL_TEMPBATT_SP_FREEZER_Lo 0x69						

	#define	E2_TOUCH_ABIL_DEFROSTING_Hi             0x6A
	#define	E2_TOUCH_ABIL_DEFROSTING_Lo             0x6B
	#define	E2_TOUCH_DEFROSTING_STEP_TIME_Hi        0x6C
	#define	E2_TOUCH_DEFROSTING_STEP_TIME_Lo        0x6D
	#define	E2_TOUCH_DEFROSTING_TIME_Hi             0x6E
	#define	E2_TOUCH_DEFROSTING_TIME_Lo             0x6F
	#define	E2_TOUCH_DEFROSTING_TEMP_SET_Hi         0x70
	#define	E2_TOUCH_DEFROSTING_TEMP_SET_Lo         0x71
	#define	E2_TOUCH_DRIPPING_TIME_Hi               0x72
	#define	E2_TOUCH_DRIPPING_TIME_Lo               0x73

	#define	E2_DEFROSTING_CYCLE_WORK_VALUE_MidHi    0x74
	#define	E2_DEFROSTING_CYCLE_WORK_VALUE_MidLo    0x75
	#define	E2_DEFROSTING_CYCLE_WORK_VALUE_Hi       0x76
	#define	E2_DEFROSTING_CYCLE_WORK_VALUE_Lo       0x77
	#define	E2_TOUCH_VALVOLA_FROZEN_Hi              0x78
    #define	E2_TOUCH_VALVOLA_FROZEN_Lo              0x79
	#define	E2_TOUCH_TESTALL_MINOPVALVE_COOL_Hi     0x7A						
	#define	E2_TOUCH_TESTALL_MINOPVALVE_COOL_Lo     0x7B						
	#define	E2_TOUCH_TESTALL_MINOPVALVE_HEAT_Hi     0x7C						
	#define	E2_TOUCH_TESTALL_MINOPVALVE_HEAT_Lo     0x7D						
	#define	E2_TOUCH_TESTALL_MINOPVALVE_FRIGO_Hi    0x7E						
	#define	E2_TOUCH_TESTALL_MINOPVALVE_FRIGO_Lo    0x7F						
	#define	E2_TOUCH_TESTALL_MINOPVALVE_FREEZER_Hi  0x80						
	#define	E2_TOUCH_TESTALL_MINOPVALVE_FREEZER_Lo  0x81						


	#define	E2_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN_Hi     0x82						
	#define	E2_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN_Lo     0x83						
	#define	E2_TOUCH_TESTALL_SUPERHEAT_HEAT_IGAIN_Hi     0x84						
	#define	E2_TOUCH_TESTALL_SUPERHEAT_HEAT_IGAIN_Lo     0x85						
	#define	E2_TOUCH_TESTALL_SUPERHEAT_HEAT_DGAIN_Hi     0x86						
	#define	E2_TOUCH_TESTALL_SUPERHEAT_HEAT_DGAIN_Lo     0x87						
	#define	E2_TOUCH_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID_Hi 0x88						
	#define	E2_TOUCH_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID_Lo 0x89						

	#define	E2_TOUCH_TESTALL_SUPERHEAT_COOL_PGAIN_Hi     0x8A
	#define	E2_TOUCH_TESTALL_SUPERHEAT_COOL_PGAIN_Lo     0x8B
	#define	E2_TOUCH_TESTALL_SUPERHEAT_COOL_IGAIN_Hi     0x8C
	#define	E2_TOUCH_TESTALL_SUPERHEAT_COOL_IGAIN_Lo     0x8D
	#define	E2_TOUCH_TESTALL_SUPERHEAT_COOL_DGAIN_Hi     0x8E
	#define	E2_TOUCH_TESTALL_SUPERHEAT_COOL_DGAIN_Lo     0x8F
	#define	E2_TOUCH_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID_Hi 0x90
	#define	E2_TOUCH_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID_Lo 0x91
    
	#define	E2_TOUCH_MAINTENANCE_FLAG_Hi                 0x92
	#define	E2_TOUCH_MAINTENANCE_FLAG_Lo                 0x93
	#define	E2_TOUCH_MAINTENANCE_SET_TIME_Hi             0x94
	#define	E2_TOUCH_MAINTENANCE_SET_TIME_Lo             0x95

	#define	E2_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT_Hi      0x96
	#define	E2_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT_Lo      0x97
	#define	E2_TOUCH_SUPERHEAT_COOL_MAX_VALUE_CORRECT_Hi 0x98
	#define	E2_TOUCH_SUPERHEAT_COOL_MAX_VALUE_CORRECT_Lo 0x99
	#define	E2_TOUCH_SUPERHEAT_HEAT_MIN_SETPOINT_Hi      0x9A
	#define	E2_TOUCH_SUPERHEAT_HEAT_MIN_SETPOINT_Lo      0x9B
	#define	E2_TOUCH_SUPERHEAT_HEAT_MAX_SETPOINT_Hi      0x9C
	#define	E2_TOUCH_SUPERHEAT_HEAT_MAX_SETPOINT_Lo      0x9D

	#define	E2_TOUCH_THRESHOLD_COMPRESSOR_HI_Hi          0x9E						
	#define	E2_TOUCH_THRESHOLD_COMPRESSOR_HI_Lo          0x9F	

	#define	E2_TOUCH_SPLIT_SET_HUMI_Hi                   0xA0						
	#define	E2_TOUCH_SPLIT_SET_HUMI_Lo                   0xA1	

	#define	E2_TOUCH_SET_DELTA_TEMP_MIN_DEHUMI_Hi        0xA2						
	#define	E2_TOUCH_SET_DELTA_TEMP_MIN_DEHUMI_Lo        0xA3	

	#define	E2_TOUCH_SET_FAN_SPEED_DEHUMI_Hi             0xA4						
	#define	E2_TOUCH_SET_FAN_SPEED_DEHUMI_Lo             0xA5	

	#define	E2_TOUCH_SET_PRESS_DEHUMI_Hi                 0xA6						
	#define	E2_TOUCH_SET_PRESS_DEHUMI_Lo                 0xA7	

    #define	E2_TOUCH_SET_TEMP_AIR_OUT_Hi                 0xA8						
	#define	E2_TOUCH_SET_TEMP_AIR_OUT_Lo                 0xA9	

    #define	E2_TOUCH_SET_TEMP_AIR_OUT_DEHUMI_Hi          0xAA						
	#define	E2_TOUCH_SET_TEMP_AIR_OUT_DEHUMI_Lo          0xAB	



    // Free
	//#define	E2_          0xAC
    //...
	//#define	E2_          0xF7

    #define E2_Addr_Hours_Hi            0xF8
    #define E2_Addr_Hours_Lo            0xF9

	#define	E2_END_OF_SPACE_Hi          0xFA			// Locazione spare per identificare fine spazio EEPROM (comodo per calcolo CRC)
	#define	E2_END_OF_SPACE_Lo          0xFB												

	#define	E2_CRC_TouchVal_Hi          0xFC			// CRC
	#define	E2_CRC_TouchVal_Lo          0xFD												


	#define	Test_EEPROM_Retention_B0    0xFE            // Byte riservato al test di Rilettura "MagicNumber" per verificare che la EEPROM mantenga il dato a scheda disalimentata

	//----------------------------------------------------------------------------------
	//	DEFINE indirizzi E2 - Banco 1
	//----------------------------------------------------------------------------------
	#define	Test_EEPROM_ReadWrite_B1    0x100	// Byte riservato al test di Scrittura/Lettura che si effettua a tutte le accensioni
    //....
	#define	Test_EEPROM_Retention_B1    0x1FE	// Byte riservato al test di Rilettura "MagicNumber" per verificare che la EEPROM mantenga il dato a scheda disalimentata
	//----------------------------------------------------------------------------------
	//	DEFINE indirizzi E2 - Banco 2
	//----------------------------------------------------------------------------------
	#define	Test_EEPROM_ReadWrite_B2    0x200	// Byte riservato al test di Scrittura/Lettura che si effettua a tutte le accensioni
    //....
	#define	Test_EEPROM_Retention_B2    0x2FE	// Byte riservato al test di Rilettura "MagicNumber" per verificare che la EEPROM mantenga il dato a scheda disalimentata
	//----------------------------------------------------------------------------------
	//	DEFINE indirizzi E2 - Banco 3
	//----------------------------------------------------------------------------------
	#define	Test_EEPROM_ReadWrite_B3    0x300	// Byte riservato al test di Scrittura/Lettura che si effettua a tutte le accensioni
    //....
	#define	Test_EEPROM_Retention_B3    0x3FE	// Byte riservato al test di Rilettura "MagicNumber" per verificare che la EEPROM mantenga il dato a scheda disalimentata
	//----------------------------------------------------------------------------------
	//	DEFINE indirizzi E2 - Banco 4
	//----------------------------------------------------------------------------------
	#define	Test_EEPROM_ReadWrite_B4    0x400	// Byte riservato al test di Scrittura/Lettura che si effettua a tutte le accensioni
    //....
	#define	Test_EEPROM_Retention_B4    0x4FE	// Byte riservato al test di Rilettura "MagicNumber" per verificare che la EEPROM mantenga il dato a scheda disalimentata
	//----------------------------------------------------------------------------------
	//	DEFINE indirizzi E2 - Banco 5
	//----------------------------------------------------------------------------------
	#define	Test_EEPROM_ReadWrite_B5    0x500	// Byte riservato al test di Scrittura/Lettura che si effettua a tutte le accensioni
    //....
	#define	Test_EEPROM_Retention_B5    0x5FE	// Byte riservato al test di Rilettura "MagicNumber" per verificare che la EEPROM mantenga il dato a scheda disalimentata
	//----------------------------------------------------------------------------------
	//	DEFINE indirizzi E2 - Banco 6
	//----------------------------------------------------------------------------------
	#define	Test_EEPROM_ReadWrite_B6    0x600	// Byte riservato al test di Scrittura/Lettura che si effettua a tutte le accensioni
    //....
	#define	Test_EEPROM_Retention_B6    0x6FE	// Byte riservato al test di Rilettura "MagicNumber" per verificare che la EEPROM mantenga il dato a scheda disalimentata
	//----------------------------------------------------------------------------------
	//	DEFINE indirizzi E2 - Banco 7
	//----------------------------------------------------------------------------------
	#define	Test_EEPROM_ReadWrite_B7    0x700	// Byte riservato al test di Scrittura/Lettura che si effettua a tutte le accensioni
    //....
	#define	Test_EEPROM_Retention_B7    0x7FE	// Byte riservato al test di Rilettura "MagicNumber" per verificare che la EEPROM mantenga il dato a scheda disalimentata




	#define VariabiliGlobaliEEPROM()	\
		volatile int WriteE2Status = 0;	\
		volatile int CRC_Flash = 0;		\
		volatile char SN1 = 0;			\
		volatile char SN2 = 0;			\
		volatile char SN3 = 0;			\
		volatile char SN4 = 0;			\
		volatile int GiornoFabbricazione = 0;	\
		volatile int MeseFabbricazione = 0;		\
		volatile int AnnoFabbricazione = 0;

	#define IncludeVariabiliGlobaliEEPROM()		\
		extern volatile int WriteE2Status;		\
		extern volatile int CRC_Flash;			\
		extern volatile char SN1;				\
		extern volatile char SN2;				\
		extern volatile char SN3;				\
		extern volatile char SN4;				\
		extern volatile int GiornoFabbricazione;	\
		extern volatile int MeseFabbricazione;		\
		extern volatile int AnnoFabbricazione;

	//----------------------------------------------------------------------------------
	//	Variabili
	//----------------------------------------------------------------------------------
	IncludeVariabiliGlobaliEEPROM();
	//IncludeVariabiliGlobaliADC();
	//IncludeVariabiliGlobaliPWM();
	//IncludeVariabiliGlobaliValvolaExp();

	//----------------------------------------------------------------------------------
	//	Prototipi delle funzioni
	//----------------------------------------------------------------------------------
	char HiByte(int Data);
	char LoByte(int Data);
	int ToInt(char HiByte, char LoByte);
	void StoreInt(int Data, int Address);
	int LoadInt(int Address, int UseVirtualBuffer);
	//void LoadAllE2_Data(void);
	//void RefreshE2_Data();
    
    void RefreshE2_TouchValue(int SectorPointer, int UseVirtualBuffer);
    void LoadE2_TouchValue(int PrimarySectorPointer, int SecondarySectorPointer);
    void RefreshE2_TouchValue_DoubleSector(int UseVirtualBuffer);
    void LoadE2_TouchValue_DoubleSector(void);
    
    unsigned int Generate_CRC_E2(int StartE2Data, int EndE2Data, int UseVirtualBuffer);
	
    void CheckValueRange(void);
	void CheckVarValueNewNetB(volatile int * TouchVar, volatile int * Var, int LimLo, int LimHi, int DefVal, int RegVal, int EEpAdd);
    void CheckVarValueSetPNetB(volatile int * TouchVar, volatile int * Var, int LimLo, int LimHi, int DefVal, int RegVal, int EEpAdd);
    void CheckVarValueNewSyxtS(volatile int * TouchVar, volatile int * Var, int LimLo, int LimHi, int DefVal, int RegVal, int EEpAdd); 
    void CheckVarValueSetPSyxtS(volatile int * TouchVar, volatile int * Var, int LimLo, int LimHi, int DefVal, int RegVal, int EEpAdd);
    void CheckVarValueOnOffSyxtS(volatile int * TouchVar, volatile int * Var, int LimLo, int LimHi, int DefVal, int RegVal, int EEpAdd);    
	void CheckVarValue(volatile int * Var, int LimLo, int LimHi, int DefVal, int RegVal);
    void CheckVarValueNew(volatile int * TouchVar, volatile int * Var, int LimLo, int LimHi, int DefVal, int RegVal, int EEpAdd);    
	void CheckDefModeValue(void);
    void CheckUtaModeValue(void);
	void CheckPowerLimitValue(void);
    void CheckPumpSelectionValue(void);

    void CheckEEPROM_ReadWrite(void);	
    void CheckEEPROM_Retention(void);	
    void InitEEPROM_Retention(void);    
    void EraseEEPROM(void);    
    void LoadAllEEPROM_Debug(int * Buffer);
    void CheckEEPROMCellSector(void);
    
    void LoadE2VirtualPageBuffer(void);
    void StoreIntVirt(int Data, int Address);
    int LoadIntVirt(int Address);  
    
    unsigned int Store_Offset_SectorPage(int SectorPointer);
#endif
