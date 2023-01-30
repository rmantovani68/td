//----------------------------------------------------------------------------------
//	Progect name:	Driver_ModBus_U4.h
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			TDM Team
//	Date:			11/04/2022
//	Description:	Header delle funzioni pilota della comunicazione MODBUS su UART4
//----------------------------------------------------------------------------------
#if (K_AbilMODBCAN==1)  

#ifndef _DRIVER_MODBUS_U4_H_
	#define _DRIVER_MODBUS_U4_H_
	
	//---------------------------------------
	// Define
	//---------------------------------------
	#define ModPolGenCRC16	0xA001	// polinomio generatore a 9 bit così da ottenere un resto a 8 bit
	#define	FlagModBusProt_BUS4	0x04	// Id associato al protocollo ModBus UART4
	#define ByteLenModBuff	80 //40		// lunghezza in byte dei buffer modbus - v8.4.16 Aumentata lunghezza da 40 a 80 byte per evitare sforamenti buffer TX verso Touch in caso di piu di 15 dati da inviare!

    #define K_TimeModInterChar       10      // setto il time out @ 10 ms

    #define K_Enable_TX_ModBus_U4_Interrupt    1



	#define VariabiliGlobaliDriverModBus_U4()	\
		volatile TypTimer TimeOutModRx_U4;		/* definisco il Timer per il TimeOut di Rx */						\
		volatile TypModBuffer Mod_BufferRx_U4;	/* definisco una coda di comandi in ricezione di 3 */				\
		volatile TypModBuffer Mod_BufferTx_U4;	/* definisco i dati in uscita */									\
		volatile TypModBuffer Mod_LastRx_U4;	/* definisco la variabile contenente l'ultimo comando ricevuto */	\
		volatile char iPckModRx_U4=0;			/* indice del pacchetto in ricezione */

	#define IncludeVariabiliGlobaliDriverModBus_U4()    \
		extern volatile TypTimer TimeOutModRx_U4;		\
		extern volatile TypModBuffer Mod_BufferRx_U4;	\
		extern volatile TypModBuffer Mod_BufferTx_U4;	\
		extern volatile TypModBuffer Mod_LastRx_U4;     \
		extern volatile char iPckModRx_U4;
	
	//---------------------------------------
	// Struttura buffer
	//---------------------------------------
/*
	typedef struct
	{
		char	Buffer[ByteLenModBuff];	// carattere di start
		char	nByte;					// numero di byte ricevuti
		int		Data;					// dato decodificato
		char	Valid_Data;				// il dato è valido
        int     CntByte;        
	} TypModBuffer;
*/	
	//---------------------------------------
	// Variabili
	//---------------------------------------
	IncludeVariabiliGlobaliDriverModBus_U4();

	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	unsigned int Mod_BaseCRC16_U4(unsigned int Old_CRC, unsigned char NewChr);
	unsigned int Mod_Generate_CRC_U4(TypModBuffer * RtxBuff);
	void Mod_CopyBuffer_U4(TypModBuffer * Destinazione, TypModBuffer * Originale);
	char Mod_WriteBus_U4(void);
	void Mod_IntReadBus_U4(int RxData);
	void Mod_IntTimeOutRx_U4(void);
	void Mod_Recive_Cmd_U4(void);
	void Mod_Write_Cmd_U4(char Address, char Comando, int Registro, int Data);    
    
#endif
    
#endif //#if (K_AbilMODBCAN==1)  
    
