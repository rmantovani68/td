    //----------------------------------------------------------------------------------
//	Progect name:	ProtocolloComunicazione.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			22/05/2011
//	Description:	Header delle funzioni che gestiscono la comunicazione con il sistema
//----------------------------------------------------------------------------------

#ifndef _PROTOCOLLO_COMUNICAZIONE_SEC_H_
	#define _PROTOCOLLO_COMUNICAZIONE_SEC_H_
	
	#include "Core.h"
	//----------------------------------------------------------------------------------
	//	Define
	//----------------------------------------------------------------------------------
	#define MaxChkRetries	1//3		// check
	#define MaxChkWait		5//100//200		// check
	#define MaxReadRetries	3 		// Read
	#define	MaxReadWait		200//50 		// Read
	#define MaxWriteRetries	3 		// Write
	#define	MaxWriteWait	200		// Write
	#define	WriteBrcRetries	1 		// Broadcast
	#define	WriteBrcWait	1//10		// Broadcast

	#define	k_WaitSendMessage   1000    // Pausa in uS per garantire una minima distanza tra un Tx e il successivo 		                        


	#define VariabiliGlobaliProtocolloComunicazioneSec()	\
		volatile TypTimer TimeOutPktRxSec	/* definisco il Timer per il TimeOut di Rx */

	#define IncludeVariabiliGlobaliProtocolloComunicazioneSec()		\
		extern volatile TypTimer TimeOutPktRxSec


	//----------------------------------
	// Strutture
	//----------------------------------

	//---------------------------------------
	// Variabili 
	//---------------------------------------
	IncludeVariabiliGlobaliProtocolloComunicazioneSec();
	//IncludeVariabiliGlobaliCore();

	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	unsigned char WaitRxSec(int TimeOut_ms);
	//unsigned char Send_WaitRx_Sec(unsigned char Address, char Comando, int Registro, int Data, int TimeOut_ms, char Retry);
	unsigned char Send_WriteReg_Sec(unsigned char Address, char Comando, int Registro, int Data, int TimeOut_ms, char Retry);
	unsigned char SendBrc_WriteReg_Sec(char Cmd, int Registro, int Data, int TimeOut_ms, char Retry);

	unsigned FuncReadReg_Sec (char Address, int REG, volatile int * StoreReg, unsigned OffLineFlag);
	unsigned FuncWriteReg_Sec (char Address, int REG, int SendData, unsigned OffLineFlag);
	unsigned FuncWriteBrcReg_Sec (char Comando, int REG, int SendData);
    unsigned FuncCheckReg_Sec (char Address);

	void CheckModuli_Sec(void);
	void ResumeOffLine_Sec(unsigned char * Cnt);

	void RefreshInPutRoom_Sec(volatile TypSplitSlave * DataStore, char Address, int SchedRead);
	void RefreshAllInPut_Sec(void);
	void RefreshOutPutRoom_Sec(int SlaveSchedRead);
	void RefreshAllOutPut_Sec(void);
    void CheckHotPlugSlave_Sec(unsigned char * Cnt);    
    void MountImageSlaveFlags(void);
    int CheckIfTouchIsReady(unsigned char i);

#endif
