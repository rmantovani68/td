//----------------------------------------------------------------------------------
//	Progect name:	ProtocolloModBus_U4.h
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			TDM Team
//	Date:			11/04/2022
//	Description:	Header delle funzioni che gestiscono la comunicazione ModBus su UART4
//----------------------------------------------------------------------------------
#include "FWSelection.h"
#if (K_AbilMODBCAN==1)  

#ifndef _PROTOCOLLO_MODBUS_U4_H_
	#define _PROTOCOLLO_MODBUS_U4_H_


#include "ProtocolloComunicazione.h"
//#include "FWSelection.h"

	//----------------------------------------------------------------------------------
	//	Define
	//----------------------------------------------------------------------------------

	//----------------------------------
	// Strutture
	//----------------------------------
	
	//---------------------------------------
	// Variabili 
	//---------------------------------------
	// Timer 
    
#define VariabiliGlobaliProtocolloModBus_U4()       \
	char Result_U4 = 0;                             \
	char RetryCnt_U4 = 0;                           \
	char Indice_U4 = 0;                             \
	int Data_U4 = 0;                                \
	char i_U4=0;                                    \
    volatile TypTimer TimeOutModPktRx_U4;


#define IncludeVariabiliGlobaliProtocolloModBus_U4()	\
	extern char Result_U4;                              \
	extern char RetryCnt_U4;                            \
	extern char Indice_U4;                              \
	extern int Data_U4;                                 \
	extern char i_U4;                                   \
    extern volatile TypTimer TimeOutModPktRx_U4;

    
    IncludeVariabiliGlobaliProtocolloModBus_U4();     
	


	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	unsigned char WaitModRx_U4(int TimeOut_ms);
	unsigned char Send_WaitModRx_U4(char Address, char Comando, int Registro, int Data, int TimeOut_ms, char Retry);
	unsigned char Send_WriteModReg_U4(char Address, int Registro, int Data, int TimeOut_ms, char Retry, char Cmd);
	
	void ReadModReg_U4(char Address, int REG, int * StoreReg);
	char ReadModListReg_U4(char DataIndex, int REG, int RegNum);
	void WriteModReg_U4(char Address, int REG, int SendData);
	void WriteModListReg_U4(char Address, int REG, int SendData);
	int ExtracReg_U4(char NumReg);
	void InsertReg_U4(char NumReg, int Data);
	//void InsertReg2_U4(char NumReg, int Data);
	
    
#endif
    
#endif //#if (K_AbilMODBCAN==1)     