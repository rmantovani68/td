//----------------------------------------------------------------------------------
//	Progect name:	Driver_Comunicazione.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			Emanuele
//	Date:			18/06/2011
//	Description:	Header delle funzioni pilota della comunicazione su bus
//----------------------------------------------------------------------------------
#ifndef _DRIVER_MODBUS_H_
	#define _DRIVER_MODBUS_H_
	
	//---------------------------------------
	// Define
	//---------------------------------------
	#define ModPolGenCRC16	0xA001	// polinomio generatore a 9 bit così da ottenere un resto a 8 bit
	#define	FlagModBusProt_BUSM	0x01	// Id associato al protocollo ModBus UART1
	#define	FlagModBusProt_BUSS	0x02	// Id associato al protocollo ModBus UART2
	#define ByteLenModBuff	80 //40		// lunghezza in byte dei buffer modbus - v8.4.16 Aumentata lunghezza da 40 a 80 byte per evitare sforamenti buffer TX verso Touch in caso di piu di 15 dati da inviare!

    #define K_TimeModInterChar       10      // setto il time out @ 10 ms

    #define K_Enable_TX_ModBus_Interrupt    1



	#define VariabiliGlobaliDriverModBus()	\
		volatile TypTimer TimeOutModRx;		/* definisco il Timer per il TimeOut di Rx */						\
		volatile TypModBuffer Mod_BufferRx;	/* definisco una coda di comandi in ricezione di 3 */				\
		volatile TypModBuffer Mod_BufferTx;	/* definisco i dati in uscita */									\
		volatile TypModBuffer Mod_LastRx;	/* definisco la variabile contenente l'ultimo comando ricevuto */	\
		volatile char iPckModRx=0;				/* indice del pacchetto in ricezione */

	#define IncludeVariabiliGlobaliDriverModBus()	\
		extern volatile TypTimer TimeOutModRx;		\
		extern volatile TypModBuffer Mod_BufferRx;	\
		extern volatile TypModBuffer Mod_BufferTx;	\
		extern volatile TypModBuffer Mod_LastRx;	\
		extern volatile char iPckModRx;
	
	//---------------------------------------
	// Struttura buffer
	//---------------------------------------
	typedef struct
	{
		char	Buffer[ByteLenModBuff];	// carattere di start
		char	nByte;					// numero di byte ricevuti
		int		Data;					// dato decodificato
		char	Valid_Data;				// il dato è valido
        int     CntByte;        
	} TypModBuffer;
	
	//---------------------------------------
	// Variabili
	//---------------------------------------
	IncludeVariabiliGlobaliDriverModBus();

	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	unsigned int Mod_BaseCRC16(unsigned int Old_CRC, unsigned char NewChr);
	unsigned int Mod_Generate_CRC(TypModBuffer * RtxBuff);
	void Mod_CopyBuffer(TypModBuffer * Destinazione, TypModBuffer * Originale);
	char Mod_WriteBus(void);
	void Mod_IntReadBus(int RxData);
	void Mod_IntTimeOutRx(void);
	void Mod_Recive_Cmd(void);
	void Mod_Write_Cmd(char Address, char Comando, int Registro, int Data);

#endif
