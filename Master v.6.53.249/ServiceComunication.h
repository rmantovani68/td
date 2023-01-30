//----------------------------------------------------------------------------------
//	Progect name:	ServiceComunication.c
//	Device:			PIC24FJ256GB210 @ 16Mhz (FRCPLL)
//	Autor:			TDM Team
//	Date:			31/12/2017
//	Description:	Header delle funzioni che gestiscono la comunicazione con protocolli Asiliari per Debug su UART3
//----------------------------------------------------------------------------------

#ifndef _SERVICE_COMUNICATION_H_
	#define _SERVICE_COMUNICATION_H_
	
	//#include "Driver_Comunicazione.h"


	//----------------------------------------------------------------------------------
	//	Define per gestione protocollo "PLX-DAQ" per EXCEL
	//----------------------------------------------------------------------------------
	//#define Indirizzo_Modulo	Master_Default_Address
    #define ByteLenServiceBusBuff  1024
/*
	#define	k_Split_Master_Add	1			// Indirizzo Slave master 
    #define k_Broadcast_Addr    0

	#define	AbilRX_All			0           // 1 = Abilita la modalità risponditutto per il range di indirizzi specificato da "MinRX_All" e "MaxRX_All" (solo per DEBUG))
	#define	MinRX_All			3           
	#define	MaxRX_All			6//16	


	// header
	#define	StartPack			0x55		// per l'auto BaudRate
	#define	Ack					0x8000		// dato di ack
	#define	N_Ack				0xFFFF		// dato di non ack
	
	// comandi
	#define	Prg					0x01
	#define	ReadReg				0x02	
	#define	WriteRegBrdCst		0x03
	#define	WriteReg			0x04
	#define	WriteSplitRegBrdCst	0x05
	#define	WriteDimmRegBrdCst	0x06
	#define	WriteEngRegBrdCst	0x07
	#define	Check				0x08

	// registri locali
	#define	REG_SPLIT_TEMP_BULBO_GRANDE			0x0001		// Temperatura bulbo grande
	#define	REG_SPLIT_TEMP_BULBO_PICCOLO		0x0002		// Temperatura bulbo piccolo
	#define	REG_SPLIT_TEMP_ARIA_USCITA			0x0003		// Temperatura arian in uscita
	#define	REG_SPLIT_TEMP_AMBIENTE				0x0004		// Temperatura aria ambiente
	#define REG_SPLIT_PRES_BULBO_GRANDE			0x0005		// Pressione bulbo grande
	#define REG_SPLIT_PRES_BULBO_PICCOLO		0x0006		// Pressione bulbo piccolo
	#define REG_SPLIT_I_FAN						0x0007		// Corrente ventilatore
	#define REG_SPLIT_YOU_ARE_SUBSLAVE			0x0008		// Flag che dice ai sub split slave che sono split slave.
    #define REG_SPLIT_MY_SPLITMASTER_ADDRESS    0x0009      // Indirizzo del mio Slave Master
    #define REG_SPLIT_TEMP_AMB_MIDDLE		    0x000A      // Indirizzo della temperatura ambiente mediata con sottoslave
    #define REG_SPLIT_TEMP_AMB_REAL 		    0x000B      // Indirizzo della temperatura ambiente reale utilizzata (mediata con sottoslave o no in mbas ea FW Selection)



	#define GlobalVarServiceComunicationProtocol()	\
		volatile unsigned char	 Master_Default_Address = 0

	#define IncludeGlobalVarServiceComunicationProtocol()\
		extern volatile unsigned char	 Master_Default_Address

	//---------------------------------------
	// Variabili 
	//---------------------------------------
	IncludeGlobalVarServiceComunicationProtocol();
	//IncludeVariabiliGlobaliADC();
	//IncludeVariabiliGlobaliEEPROM();
	//IncludeVariabiliGlobaliPWM();
	//IncludeVariabiliGlobaliValvolaExp();
	//IncludeVariabiliGlobaliCore();

 */ 

	#define GlobalVarServiceComunication()	\
		volatile TypServiceBusBuffer ServiceBusBufferTxU3;  \
		volatile TypServiceBusBuffer ServiceBusBufferTxU4;  \
        volatile TypServiceBusBuffer ServiceBusBufferRxU3;  \
        volatile TypServiceBusBuffer ServiceBusBufferRxU4;  

	#define IncludeGlobalVarServiceComunication()	\
		extern volatile TypServiceBusBuffer ServiceBusBufferTxU3;   \
        extern volatile TypServiceBusBuffer ServiceBusBufferTxU4;   \
        extern volatile TypServiceBusBuffer ServiceBusBufferRxU3;   \
        extern volatile TypServiceBusBuffer ServiceBusBufferRxU4;
	
	//---------------------------------------
	// Struttura buffer
	//---------------------------------------
	typedef struct
	{
		char	Buffer[ByteLenServiceBusBuff];	// Buffer caratteri 
		unsigned int nByte;                     // numero di byte ricevuti/trasmessi
		int		Data;                           // dato decodificato
        int *   PtData;                         // Puntatore al dato da aggiornare
		char	Valid_Data;                     // il dato è valido
		unsigned int 	CntByte;				// contatore byte inviati/ricevuti.. (vedi modifica TX sotto interrupt)
        int Cella1;
        int Cella2;
        int Cella3;
	} TypServiceBusBuffer;
    
    
    
    IncludeGlobalVarServiceComunication();    
    
    
	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	void Change_BaudeUART3(int Baudindex);
    void PutStringToUART3(int Val1, int Val2, int Val3, int Val4, int Val5, int Val6, unsigned int Val7);
    void SendBufferToUART3(void);
    void PutDataForUART3(int Val1, int Val2, int Val3, unsigned int Val4, unsigned int Val5, unsigned int Val6, unsigned int Val7);
	void Change_BaudeUART4(int Baudindex);
    void PutStringToUART4(int Val1, int Val2, int Val3, int Val4, int Val5, int Val6, unsigned int Val7);
    void SendBufferToUART4(void);
    void PutDataForUART4(int Val1, int Val2, int Val3, int Val4, int Val5, unsigned int Val6, unsigned int Val7);
    void IntReadServiceBusU3(volatile TypServiceBusBuffer * Rx_Buff);
    void Process_ServiceU3Cmd(volatile TypServiceBusBuffer * Rx_Buff);
    void IntReadServiceBusU4(volatile TypServiceBusBuffer * Rx_Buff);
    void Process_ServiceU4Cmd(volatile TypServiceBusBuffer * Rx_Buff);
    int StringToInt(char * Buffer);

	
#endif
