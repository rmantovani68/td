//----------------------------------------------------------------------------------
//	Progect name:	Driver_Comunicazione.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			22/05/2011
//	Description:	Header delle funzioni pilota della comunicazione su bus
//----------------------------------------------------------------------------------
#ifndef _DRIVER_COMUNICAZIONE_SEC_H_
	#define _DRIVER_COMUNICAZIONE_SEC_H_
	
	//---------------------------------------
	// Define
	//---------------------------------------
	#define	FlagMyBusProt	0x00	// Id associato al protocollo ModBus
    #define k_My_Address_Sec    254     // Indirizzo dello slave per le comunicazioni con i SubSlave

    #define K_TimeInterCharSec  10  // setto il time out @ 10 ms

	#define VariabiliGlobaliDriverComunicazioneSec()	\
		volatile TypTimer TimeOutRxSec;	/* definisco il Timer per il TimeOut di Rx */                       \
		volatile TypBuffer BufferRxSec;	/* definisco una coda di comandi in ricezione di 3 */				\
		volatile TypBuffer BufferTxSec;	/* definisco i dati in uscita */									\
		volatile TypBuffer LastRxSec;	/* definisco la variabile contenente l'ultimo comando ricevuto */	\
		volatile TypMyBuffer MyBUS_BufferTxSec;    \
		volatile int WaitingReplaySec;          \
		volatile char iPckRxSec = 0;	/* indice del pacchetto in ricezione */

	#define IncludeVariabiliGlobaliDriverComunicazioneSec()\
		extern volatile TypTimer TimeOutRxSec;			\
		extern volatile TypBuffer BufferRxSec;			\
		extern volatile TypBuffer BufferTxSec;          \
		extern volatile TypBuffer LastRxSec;            \
		extern volatile TypMyBuffer MyBUS_BufferTxSec; \
		extern volatile int WaitingReplaySec;           \
		extern volatile char iPckRxSec;
	
	//---------------------------------------
	// Variabili
	//---------------------------------------
	IncludeVariabiliGlobaliDriverComunicazioneSec();
	
	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	char WriteBusSec(void);
	char WriteBusU3(void);
	void IntReadBusSec(volatile TypBuffer * Rx_Buff);
	void IntTimeOutRxSec(void);
	void Recive_Cmd_Sec(volatile TypBuffer * Rx_Buff, volatile TypBuffer * LastCmd);
	void Write_Cmd_Sec(volatile TypBuffer * Tx_Buff, char Address, char Comando, int Registro, int Data);
	void Write_brc_Cmd_Sec(volatile TypBuffer * Tx_Buff, char Comando, int Registro, int Data);

#endif
