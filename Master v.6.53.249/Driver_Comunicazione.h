//----------------------------------------------------------------------------------
//	Progect name:	Driver_Comunicazione.h
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			Emanuele
//	Date:			04/01/2014
//	Description:	Header delle funzioni pilota della comunicazione su bus
//----------------------------------------------------------------------------------
#ifndef _DRIVER_COMUNICAZIONE_H_
	#define _DRIVER_COMUNICAZIONE_H_
	
#include "Timer.h"
	
	//---------------------------------------
	// Define
	//---------------------------------------
	#define PolGenCRC16             0x01A5	// polinomio generatore a 9 bit così da ottenere un resto a 8 bit
	#define	FlagMyBusProt_BUSM      0x00	// Id associato al protocollo MyBus UART1	
	#define	FlagMyBusProt_BUSS      0x03	// Id associato al protocollo MyBus UART2	

    #define K_TimeInterChar         10      // setto il time out @ 10 ms

    #define K_Enable_TX_MyBus_Interrupt    1


	//---------------------------------------
	// Struttura buffer
	//---------------------------------------
	typedef struct
	{
		unsigned char	Start_Chr;	// carattere di start
		unsigned char	Dest_Add;	// byte con l'indirizzo del destinatario
		unsigned char	Mitt_Add;	// byte con l'indirizzo del mitente
		unsigned char	Comando;	// byte con il codice del comando da eseguire
		unsigned int	Registro;	// integer con l'indirizzo del registro (0 se opzionale)
		unsigned int	Data;		// integer con i dati del registro (0 se opzionale)
		unsigned int	CRC16;		// integer con il codice di controllo CRC16
		unsigned char	Valid_Data;	// byte che indica se il dato è valido ed è da procesare
		unsigned char	Buffer[12]; //             
		unsigned char	nByte;		// numero di byte ricevuti
		unsigned char	CntByte;	// contatore byte inviati.. (vedi modifica TX sotto interrupt)
	} TypBuffer;

	//---------------------------------------
	// Struttura buffer
	//---------------------------------------
	typedef struct
	{
		unsigned char	Buffer[12];             // carattere di start
		unsigned char	nByte;					// numero di byte ricevuti
		unsigned int	Data;					// dato decodificato
		unsigned char	Valid_Data;				// il dato è valido
		unsigned char	CntByte;				// contatore byte inviati.. (vedi modifica TX sotto interrupt)
	} TypMyBuffer;        

	#define VariabiliGlobaliDriverComunicazione()	\
		volatile TypTimer TimeOutRx;	/* definisco il Timer per il TimeOut di Rx */						\
		volatile TypBuffer BufferRx;	/* definisco una coda di comandi in ricezione di 3 */				\
		volatile TypBuffer BufferTx;	/* definisco i dati in uscita */									\
		volatile TypBuffer LastRx;		/* definisco la variabile contenente l'ultimo comando ricevuto */	\
		volatile TypMyBuffer MyBUS_BufferTx;                                                                \
		volatile char iPckRx = 0;		/* indice del pacchetto in ricezione */

	#define IncludeVariabiliGlobaliDriverComunicazione()\
		extern volatile TypTimer TimeOutRx;				\
		extern volatile TypBuffer BufferRx;				\
		extern volatile TypBuffer BufferTx;				\
		extern volatile TypBuffer LastRx;				\
		extern volatile TypMyBuffer MyBUS_BufferTx;     \
        extern volatile char iPckRx;


	
	//---------------------------------------
	// Variabili
	//---------------------------------------
	IncludeVariabiliGlobaliDriverComunicazione();

	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	int BaseCRC16(int Old_CRC, unsigned char NewChr);
	int GenerateCRC(volatile TypBuffer * RxBuff);
	void CopyBuffer(volatile TypBuffer * Destinazione, volatile TypBuffer * Originale);
	void IntReadBus(volatile TypBuffer * Rx_Buff, int RxData);
    void Process_Cmd(volatile TypBuffer * LastCmd);    
	void IntTimeOutRx(void);
	void Recive_Cmd(volatile TypBuffer * Rx_Buff, volatile TypBuffer * LastCmd);
	void Write_Cmd(volatile TypBuffer * Tx_Buff, char Address, char Comando, int Registro, int Data);
	void Write_brc_Cmd(volatile TypBuffer * Tx_Buff, char Comando, int Registro, int Data);

#endif
