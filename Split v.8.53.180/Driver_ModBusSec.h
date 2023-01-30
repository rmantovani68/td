//----------------------------------------------------------------------------------
//	Progect name:	Driver_Comunicazione.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			18/06/2011
//	Description:	Header delle funzioni pilota della comunicazione su bus
//----------------------------------------------------------------------------------
#ifndef _DRIVER_MODBUS_SEC_H_
	#define _DRIVER_MODBUS_SEC_H_
	
	//---------------------------------------
	// Define
	//---------------------------------------
	#define ModPolGenCRC16	0xA001      // polinomio generatore a 9 bit così da ottenere un resto a 8 bit
	#define	FlagModBusProt	0x01        // Id associato al protocollo ModBus
	#define ByteLenModBuffSec       256//80  //40		// lunghezza in byte dei buffer modbus - v8.4.16 Aumentata lunghezza da 40 a 80 byte per evitare sforamenti buffer TX verso Touch in caso di piu di 15 dati da inviare!
	#define K_TimeModInterCharSec	10//5//2   // Tempo massimo di pausa (in mS) intercarattere: dovrebbe essere impostato ad almeno 3.5 volte il tempo di un byte @ BaudRate corrente
                                        // @9600  bps -> 1 byte = 1,04mS x 3.5 = 3.64mS
                                        // @19200 bps -> 1 byte = 520uS x 3.5 = 1.82mS
                                        // @38400 bps -> 1 byte = 260uS x 3.5 = 910uS   <- Velocità minima usata da noi
                                        // @57600 bps -> 1 byte = 174uS x 3.5 = 609uS
                                        // @115200 bps -> 1 byte = 87uS x 3.5 = 305uS   <- Velocità massima usata da noi

	#define VariabiliGlobaliDriverModBusSec()	\
		volatile TypTimer TimeOutModRxSec;		/* definisco il Timer per il TimeOut di Rx */						\
		volatile TypModBufferSec Mod_BufferRxSec;	/* definisco una coda di comandi in ricezione di 3 */				\
		volatile TypModBufferSec Mod_BufferTxSec;	/* definisco i dati in uscita */									\
		volatile TypModBufferSec Mod_LastRxSec;	/* definisco la variabile contenente l'ultimo comando ricevuto */	\
		volatile unsigned char iPckModRxSec;				/* indice del pacchetto in ricezione */

	#define IncludeVariabiliGlobaliDriverModBusSec()	\
		extern volatile TypTimer TimeOutModRxSec;		\
		extern volatile TypModBufferSec Mod_BufferRxSec;	\
		extern volatile TypModBufferSec Mod_BufferTxSec;	\
		extern volatile TypModBufferSec Mod_LastRxSec;	\
		extern volatile unsigned char iPckModRxSec;
	
	//---------------------------------------
	// Struttura buffer
	//---------------------------------------
	typedef struct
	{
		char	Buffer[ByteLenModBuffSec];	// carattere di start
		char	nByte;					// numero di byte ricevuti
		int		Data;					// dato decodificato
		char	Valid_Data;				// il dato è valido
		char	CntByte;				// contatore byte inviati.. (vedi modifica TX sotto interrupt)
	} TypModBufferSec;
	
	//---------------------------------------
	// Variabili
	//---------------------------------------
	IncludeVariabiliGlobaliDriverModBusSec();

	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	unsigned int Mod_BaseCRC16Sec(unsigned int Old_CRC, unsigned char NewChr);
	unsigned int Mod_Generate_CRCSec(volatile TypModBufferSec * RtxBuff);
	void Mod_CopyBufferSec(volatile TypModBufferSec * Destinazione, volatile TypModBufferSec * Originale);
	char Mod_WriteBusSec(void);
	void Mod_IntReadBusSec(void);
	void Mod_IntTimeOutRxSec(void);
	void Mod_Recive_CmdSec(void);
	void Mod_Write_CmdSec(char Address, char Comando, int Registro, int Data);

#endif
