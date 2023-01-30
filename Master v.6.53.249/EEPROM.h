//----------------------------------------------------------------------------------
//	Progect name:	EEPROM.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			Emanuele
//	Date:			22/05/2011
//	Description:	Header con la tabella di allocazione della E2 del micro
//----------------------------------------------------------------------------------

/*
Memory = 256 Bytes
+------+----------------+-----------------------------------------------+
|Addres|	Name		|		Function								|
+------+----------------+-----------------------------------------------+
| 0x00 | CRC_Hi_Flash   | CRC high byte per il controllo della flash    |
+------+----------------+-----------------------------------------------+
| 0x01 | CRC_Lo_Flash   | CRC high byte per il controllo della flash    |
+------+----------------+-----------------------------------------------+
| 0x02 | System_SN_1    | 1° byte del serial number dell'impianto       |
+------+----------------+-----------------------------------------------+
| 0x03 | System_SN_2    | 2° byte del serial number dell'impianto       |
+------+----------------+-----------------------------------------------+
| 0x04 | System_SN_3    | 3° byte del serial number dell'impianto       |
+------+----------------+-----------------------------------------------+
| 0x05 | System_SN_4    | 4° byte del serial number dell'impianto       |
+------+----------------+-----------------------------------------------+
| 0x06 | Fab_Day        | Giorno della fabbricazione                    |
+------+----------------+-----------------------------------------------+
| 0x07 | Fab_Month      | Mese di fabbricazione                         |
+------+----------------+-----------------------------------------------+
| 0x08 | Fab_Year       | Anno di fabbricazione                         |
+------+----------------+-----------------------------------------------+
.	   .				.												.
.	   .				.												.
+------+----------------+-----------------------------------------------+
| 0x10 | AD_T1_0_Hi		| Byte Alto con il valore AD della temp notevole|
+------+----------------+-----------------------------------------------+
| 0x11 | AD_T1_0_Lo		| Byte Basso con il valore AD della temp notevole|
+------+----------------+-----------------------------------------------+
.	   .				.												.
.	   .				.												.
+------+----------------+-----------------------------------------------+
| 0x60 | Kt_T1_0_Hi		| Byte Alto con il coefficenrte del segmento 1  |
+------+----------------+-----------------------------------------------+
| 0x61 | Kt_T1_0_Lo		| Byte Alto con il coefficenrte del segmento 1	|
+------+----------------+-----------------------------------------------+
.	   .				.												.
.	   .				.												.
+------+----------------+-----------------------------------------------+
| 0xB0 |                |                                               |
+------+----------------+-----------------------------------------------+
.	   .				.												.
.	   .				.												.
+------+----------------+-----------------------------------------------+
| 0xFF |                |                                               |
+------+----------------+-----------------------------------------------+
*/
#ifndef _EEPROM_
	#define _EEPROM_


	#define	K_Test_EEPROM_ReadWrite     0x1234	// Numero magico per effettuare il test di Read/Write ad ogni partenza (vedi Routine "CheckEEPROM")
	#define	K_Test_EEPROM_Retention     0x5678  // Numero magico per effettuare il test di verifica "Data Retention" ad ogni partenza ma che è scritto solo una volta durante la fase di HW_Test
	#define	K_Reset_EEPROM_Value        0xFFFF  // Valore di "reset" locazioni EEPROM

    #define	K_Test_EEPROM_TestPattern1  0xAAAA  // Numero magico per effettuare il test di verifica "Data Retention" ad ogni partenza ma che è scritto solo una volta durante la fase di HW_Test
    #define	K_Test_EEPROM_TestPattern2  0x5555  // Numero magico per effettuare il test di verifica "Data Retention" ad ogni partenza ma che è scritto solo una volta durante la fase di HW_Test

    #define	K_EEPROM_ByteSize           0x800   // Dimensione EEPROM in Byte -> 16Kbit -> 2048byte -> 8page x 256 byte
    #define	K_EEPROM_WordSize           0x400   // Dimensione EEPROM in Word -> 16Kbit -> 1024word -> 8page x 128 word


	//----------------------------------------------------------------------------------
	//	DEFINE indirizzi E2
	//----------------------------------------------------------------------------------
	#define EEPROM_Add				0xA0	// indirizzo di base della EEPROM esterna
	
	//----------------------------------------------------------------------------------
	//	DEFINE indirizzi E2
	//----------------------------------------------------------------------------------
	#define	CRC_Hi_Flash		0x00	// CRC high byte per il controllo della flash
	#define	CRC_Lo_Flash		0x01	// CRC high byte per il controllo della flash
	#define	System_SN_1			0x02	// 1° byte del serial number dell'impianto
	#define	System_SN_2			0x03	// 2° byte del serial number dell'impianto
	#define	System_SN_3			0x04	// 3° byte del serial number dell'impianto
	#define	System_SN_4			0x05	// 4° byte del serial number dell'impianto
	#define	Fab_Day				0x06	// Giorno della fabbricazione
	#define	Fab_Month			0x07	// Mese di fabbricazione
	#define	Fab_Year			0x08	// Anno di fabbricazione
	
	#define	E2_Addr_Hours		0x0A	// Conteggio ore di vita
	#define	E2_Addr_Hours_Lo	0x0B

	#define	E2_Last_SystemDisable		0x0C	// Controllo l'ultimo stato di System Disable
	#define	E2_Last_SystemDisable_Lo	0x0D

/*
    #define	E2_LastCompressorPowerStatus	0x0C    // ULtimo stato compressore
    #define	E2_LastCompressor2Master		0x0D    // ULtimo stato flag switch compressore
    #define	E2_LastTimerSwitchCompressor	0x0E    // ULtimo valore timer per switch compressore
*/


	#define	AD_T1_0_Hi			0x10
	#define	AD_T1_0_Lo			0x11
	#define	AD_T1_1_Hi			0x12
	#define	AD_T1_1_Lo			0x13
	#define	AD_T1_2_Hi			0x14
	#define	AD_T1_2_Lo			0x15
	#define	AD_T1_3_Hi			0x16
	#define	AD_T1_3_Lo			0x17
	#define	AD_T1_4_Hi			0x18
	#define	AD_T1_4_Lo			0x19
	#define	AD_T1_5_Hi			0x1A
	#define	AD_T1_5_Lo			0x1B
	#define	AD_T1_6_Hi			0x1C
	#define	AD_T1_6_Lo			0x1D
	#define	AD_T1_7_Hi			0x1E
	#define	AD_T1_7_Lo			0x1F
	#define	AD_T1_8_Hi			0x20
	#define	AD_T1_8_Lo			0x21
	#define	AD_T1_9_Hi			0x22
	#define	AD_T1_9_Lo			0x23
	#define	AD_T1_10_Hi			0x24
	#define	AD_T1_10_Lo			0x25
	#define	AD_T1_11_Hi			0x26
	#define	AD_T1_11_Lo			0x27

	#define	AD_T2_0_Hi			0x28
	#define	AD_T2_0_Lo			0x29
	#define	AD_T2_1_Hi			0x2A
	#define	AD_T2_1_Lo			0x2B
	#define	AD_T2_2_Hi			0x2C
	#define	AD_T2_2_Lo			0x2D
	#define	AD_T2_3_Hi			0x2E
	#define	AD_T2_3_Lo			0x2F
	#define	AD_T2_4_Hi			0x30
	#define	AD_T2_4_Lo			0x31
	#define	AD_T2_5_Hi			0x32
	#define	AD_T2_5_Lo			0x33
	#define	AD_T2_6_Hi			0x34
	#define	AD_T2_6_Lo			0x35
	#define	AD_T2_7_Hi			0x36
	#define	AD_T2_7_Lo			0x37
	#define	AD_T2_8_Hi			0x38
	#define	AD_T2_8_Lo			0x39
	#define	AD_T2_9_Hi			0x3A
	#define	AD_T2_9_Lo			0x3B
	#define	AD_T2_10_Hi			0x3C
	#define	AD_T2_10_Lo			0x3D
	#define	AD_T2_11_Hi			0x3E
	#define	AD_T2_11_Lo			0x3F
	
	#define	AD_T3_0_Hi			0x40
	#define	AD_T3_0_Lo			0x41
	#define	AD_T3_1_Hi			0x42
	#define	AD_T3_1_Lo			0x43
	#define	AD_T3_2_Hi			0x44
	#define	AD_T3_2_Lo			0x45
	#define	AD_T3_3_Hi			0x46
	#define	AD_T3_3_Lo			0x47
	#define	AD_T3_4_Hi			0x48
	#define	AD_T3_4_Lo			0x49
	#define	AD_T3_5_Hi			0x4A
	#define	AD_T3_5_Lo			0x4B
	#define	AD_T3_6_Hi			0x4C
	#define	AD_T3_6_Lo			0x4D
	#define	AD_T3_7_Hi			0x4E
	#define	AD_T3_7_Lo			0x4F
	#define	AD_T3_8_Hi			0x50
	#define	AD_T3_8_Lo			0x51
	#define	AD_T3_9_Hi			0x52
	#define	AD_T3_9_Lo			0x53
	#define	AD_T3_10_Hi			0x54
	#define	AD_T3_10_Lo			0x55
	#define	AD_T3_11_Hi			0x56
	#define	AD_T3_11_Lo			0x57

	#define	AD_T4_0_Hi			0x58
	#define	AD_T4_0_Lo			0x59
	#define	AD_T4_1_Hi			0x5A
	#define	AD_T4_1_Lo			0x5B
	#define	AD_T4_2_Hi			0x5C
	#define	AD_T4_2_Lo			0x5D
	#define	AD_T4_3_Hi			0x5E
	#define	AD_T4_3_Lo			0x5F
	#define	AD_T4_4_Hi			0x60
	#define	AD_T4_4_Lo			0x61
	#define	AD_T4_5_Hi			0x62
	#define	AD_T4_5_Lo			0x63
	#define	AD_T4_6_Hi			0x64
	#define	AD_T4_6_Lo			0x65
	#define	AD_T4_7_Hi			0x66
	#define	AD_T4_7_Lo			0x67
	#define	AD_T4_8_Hi			0x68
	#define	AD_T4_8_Lo			0x69
	#define	AD_T4_9_Hi			0x6A
	#define	AD_T4_9_Lo			0x6B
	#define	AD_T4_10_Hi			0x6C
	#define	AD_T4_10_Lo			0x6D
	#define	AD_T4_11_Hi			0x6E
	#define	AD_T4_11_Lo			0x6F

	#define	Kt_T1_0_Hi			0x70
	#define	Kt_T1_0_Lo			0x71
	#define	Kt_T1_1_Hi			0x72
	#define	Kt_T1_1_Lo			0x73
	#define	Kt_T1_2_Hi			0x74
	#define	Kt_T1_2_Lo			0x75
	#define	Kt_T1_3_Hi			0x76
	#define	Kt_T1_3_Lo			0x77
	#define	Kt_T1_4_Hi			0x78
	#define	Kt_T1_4_Lo			0x79
	#define	Kt_T1_5_Hi			0x7A
	#define	Kt_T1_5_Lo			0x7B
	#define	Kt_T1_6_Hi			0x7C
	#define	Kt_T1_6_Lo			0x7D
	#define	Kt_T1_7_Hi			0x7E
	#define	Kt_T1_7_Lo			0x7F
	#define	Kt_T1_8_Hi			0x80
	#define	Kt_T1_8_Lo			0x81
	#define	Kt_T1_9_Hi			0x82
	#define	Kt_T1_9_Lo			0x83
	#define	Kt_T1_10_Hi			0x84
	#define	Kt_T1_10_Lo			0x85

	#define	Kt_T2_0_Hi			0x86
	#define	Kt_T2_0_Lo			0x87
	#define	Kt_T2_1_Hi			0x88
	#define	Kt_T2_1_Lo			0x89
	#define	Kt_T2_2_Hi			0x8A
	#define	Kt_T2_2_Lo			0x8B
	#define	Kt_T2_3_Hi			0x8C
	#define	Kt_T2_3_Lo			0x8D
	#define	Kt_T2_4_Hi			0x8E
	#define	Kt_T2_4_Lo			0x8F
	#define	Kt_T2_5_Hi			0x90
	#define	Kt_T2_5_Lo			0x91
	#define	Kt_T2_6_Hi			0x92
	#define	Kt_T2_6_Lo			0x93
	#define	Kt_T2_7_Hi			0x94
	#define	Kt_T2_7_Lo			0x95
	#define	Kt_T2_8_Hi			0x96
	#define	Kt_T2_8_Lo			0x97
	#define	Kt_T2_9_Hi			0x98
	#define	Kt_T2_9_Lo			0x99
	#define	Kt_T2_10_Hi			0x9A
	#define	Kt_T2_10_Lo			0x9B

	#define	Kt_T3_0_Hi			0x9C
	#define	Kt_T3_0_Lo			0x9D
	#define	Kt_T3_1_Hi			0x9E
	#define	Kt_T3_1_Lo			0x9F
	#define	Kt_T3_2_Hi			0xA0
	#define	Kt_T3_2_Lo			0xA1
	#define	Kt_T3_3_Hi			0xA2
	#define	Kt_T3_3_Lo			0xA3
	#define	Kt_T3_4_Hi			0xA4
	#define	Kt_T3_4_Lo			0xA5
	#define	Kt_T3_5_Hi			0xA6
	#define	Kt_T3_5_Lo			0xA7
	#define	Kt_T3_6_Hi			0xA8
	#define	Kt_T3_6_Lo			0xA9
	#define	Kt_T3_7_Hi			0xAA
	#define	Kt_T3_7_Lo			0xAB
	#define	Kt_T3_8_Hi			0xAC
	#define	Kt_T3_8_Lo			0xAD
	#define	Kt_T3_9_Hi			0xAE
	#define	Kt_T3_9_Lo			0xAF
	#define	Kt_T3_10_Hi			0xB0
	#define	Kt_T3_10_Lo			0xB1

	#define	Kt_T4_0_Hi			0xB2
	#define	Kt_T4_0_Lo			0xB3
	#define	Kt_T4_1_Hi			0xB4
	#define	Kt_T4_1_Lo			0xB5
	#define	Kt_T4_2_Hi			0xB6
	#define	Kt_T4_2_Lo			0xB7
	#define	Kt_T4_3_Hi			0xB8
	#define	Kt_T4_3_Lo			0xB9
	#define	Kt_T4_4_Hi			0xBA
	#define	Kt_T4_4_Lo			0xBB
	#define	Kt_T4_5_Hi			0xBC
	#define	Kt_T4_5_Lo			0xBD
	#define	Kt_T4_6_Hi			0xBE
	#define	Kt_T4_6_Lo			0xBF
	#define	Kt_T4_7_Hi			0xC0
	#define	Kt_T4_7_Lo			0xC1
	#define	Kt_T4_8_Hi			0xC2
	#define	Kt_T4_8_Lo			0xC3
	#define	Kt_T4_9_Hi			0xC4
	#define	Kt_T4_9_Lo			0xC5
	#define	Kt_T4_10_Hi			0xC6
	#define	Kt_T4_10_Lo			0xC7

	#define	AD_T5_0_Hi			0xC8
	#define	AD_T5_0_Lo			0xC9
	#define	AD_T5_1_Hi			0xCA
	#define	AD_T5_1_Lo			0xCB
	#define	AD_T5_2_Hi			0xCC
	#define	AD_T5_2_Lo			0xCD
	#define	AD_T5_3_Hi			0xCE
	#define	AD_T5_3_Lo			0xCF
	#define	AD_T5_4_Hi			0xD0
	#define	AD_T5_4_Lo			0xD1
	#define	AD_T5_5_Hi			0xD2
	#define	AD_T5_5_Lo			0xD3
	#define	AD_T5_6_Hi			0xD4
	#define	AD_T5_6_Lo			0xD5
	#define	AD_T5_7_Hi			0xD6
	#define	AD_T5_7_Lo			0xD7
	#define	AD_T5_8_Hi			0xD8
	#define	AD_T5_8_Lo			0xD9
	#define	AD_T5_9_Hi			0xDA
	#define	AD_T5_9_Lo			0xDB
	#define	AD_T5_10_Hi			0xDC
	#define	AD_T5_10_Lo			0xDD
	#define	AD_T5_11_Hi			0xDE
	#define	AD_T5_11_Lo			0xDF

	#define	Kt_T5_0_Hi			0xE0
	#define	Kt_T5_0_Lo			0xE1
	#define	Kt_T5_1_Hi			0xE2
	#define	Kt_T5_1_Lo			0xE3
	#define	Kt_T5_2_Hi			0xE4
	#define	Kt_T5_2_Lo			0xE5
	#define	Kt_T5_3_Hi			0xE6
	#define	Kt_T5_3_Lo			0xE7
	#define	Kt_T5_4_Hi			0xE8
	#define	Kt_T5_4_Lo			0xE9
	#define	Kt_T5_5_Hi			0xEA
	#define	Kt_T5_5_Lo			0xEB
	#define	Kt_T5_6_Hi			0xEC
	#define	Kt_T5_6_Lo			0xED
	#define	Kt_T5_7_Hi			0xEE
	#define	Kt_T5_7_Lo			0xEF
	#define	Kt_T5_8_Hi			0xF0
	#define	Kt_T5_8_Lo			0xF1
	

    #define	E2_Maintenance_Flag             0xF2    // Tempo di NoPay
    #define	E2_Maintenance_Flag_Lo          0xF3    // Tempo di NoPay

    #define	E2_Maintenance_Set_Time         0xF4    // Tempo di NoPay
    #define	E2_Maintenance_Set_Time_Lo      0xF5    // Tempo di NoPay

    #define	E2_LastCompressorPowerStatus	0xF6    // ULtimo stato compressore
    #define	E2_LastCompressorPowerStatus_Lo	0xF7    // ULtimo stato compressore

	#define	E2_LastCompressor2Master		0xF8    // ULtimo stato flag switch compressore
	#define	E2_LastCompressor2Master_Lo		0xF9    // ULtimo stato flag switch compressore

	#define	E2_LastTimerSwitchCompressor	0xFA    // ULtimo valore timer per switch compressore
	#define	E2_LastTimerSwitchCompressor_Lo	0xFB    // ULtimo valore timer per switch compressore

	#define	E2_LastTimerSwitchPump      	0xFC    // ULtimo valore timer per switch pompe
	#define	E2_LastTimerSwitchPump_Lo       0xFD    // ULtimo valore timer per switch pompe

	#define	E2_LastSelectWorkingPump        0xFE    // ULtimo stato flag switch pompe
	#define	E2_LastSelectWorkingPump_Lo     0xFF    // ULtimo stato flag switch pompe


	#define VariabiliGlobaliEEPROM()	\
		volatile int WriteE2Status = 0;	\
		volatile int CRC_Flash = 0;		\
		volatile char SN1 = 0;			\
		volatile char SN2 = 0;			\
		volatile char SN3 = 0;			\
		volatile char SN4 = 0;			\
		volatile int GiornoFabbricazione = 0;	\
		volatile int MeseFabbricazione = 0;		\
		volatile int AnnoFabbricazione = 0

	#define IncludeVariabiliGlobaliEEPROM()		\
		extern volatile int WriteE2Status;		\
		extern volatile int CRC_Flash;			\
		extern volatile char SN1;				\
		extern volatile char SN2;				\
		extern volatile char SN3;				\
		extern volatile char SN4;				\
		extern volatile int GiornoFabbricazione;	\
		extern volatile int MeseFabbricazione;		\
		extern volatile int AnnoFabbricazione

	//----------------------------------------------------------------------------------
	//	Variabili
	//----------------------------------------------------------------------------------
	IncludeVariabiliGlobaliEEPROM();

	//----------------------------------------------------------------------------------
	//	Prototipi delle funzioni
	//----------------------------------------------------------------------------------
	char HiByte(int Data);
	char LoByte(int Data);
	int ToInt(char HiByte, char LoByte);
	void StoreInt(int Data, int Address);
	int LoadInt(int Address);
	void LoadAllE2_Data(void);
	void RefreshE2_Data(void);
    void RefreshE2_CompressorStatus(void);    
    
	unsigned int Generate_CRC_E2(int StartE2Data, int EndE2Data);
	void CheckValueRange(void);
	void CheckVarValue(int * Var, int LimLo, int LimHi, int DefVal);
	void CheckDefModeValue(void);
    void CheckUtaModeValue(void);
	void CheckPowerLimitValue(void);
    void CheckPumpSelectionValue(void);

    void EraseEEPROM(void);    
    void LoadAllEEPROM_Debug(int * Buffer);
	
#endif
