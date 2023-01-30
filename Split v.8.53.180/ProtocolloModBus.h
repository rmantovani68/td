//----------------------------------------------------------------------------------
//	Progect name:	ProtocolloModBus.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			18/06/2011
//	Description:	Header delle funzioni che gestiscono la comunicazione ModBus con il sistema
//----------------------------------------------------------------------------------

#include "HWSelection.h"
#if (GB210==1) 
	#include <p24FJ256GB210.h>
#elif (DA210==1)
	#include <p24FJ256DA210.h>
#else
	#include <p24FJ256GB110.h>
#endif
#ifndef _PROTOCOLLO_MODBUS_H_
	#define _PROTOCOLLO_MODBUS_H_
	
	//----------------------------------------------------------------------------------
	//	Define
	//----------------------------------------------------------------------------------
/*
	#define	Add_TouchRoom						0x01
	#define	Add_NetBRoom						0x1E  	// Indirizzo MODBUS Pannello NetBuilding Mod.MCT4KKT (Address default = 30 -> 0x1E)
*/
	#define MaxModChkRetries					3		// check
	#define MaxModChkWait						100		// check
	#define MaxModReadRetries					3//2		// Read
	#define	MaxModReadWait						100//50		// Read
	#define MaxModWriteRetries					3//2		// Write
	#define	MaxModWriteWait						100//50		// Write
	
	// comandi MODBUS
	#define	ReadModCoil							1   //0x01	// Read Coil Status 
	#define	ReadModInputStatus					2   //0x02	// Read Input Status        NEW (Not implemented)
	#define	ReadModCmd							3   //0x03	// Read Holding Register
	#define	ReadModInput						4   //0x04	// Read Input Register
	#define	WriteModForceSingleCoil				5   //0x05	// Force Single Coil        NEW (Not implemented)
	#define	WriteModCmd							6   //0x06	// Preset Single Register
	#define	ReadModStatus                       7   //0x07	// Read Status              NEW (Not implemented)
	#define	WriteModForceMultipleCoil			15  //0x0F	// Force Multiple Coil      NEW (Not implemented)
	#define	WriteModListCmd						16  //0x10	// Preset Multiple register

		/*volatile char Result = 0;							\*/

	#define VariabiliGlobaliProtocolloModBus()				\
		volatile TypTimer TimeOutModPktRx;					\
		volatile TypExchange BridgeBufferA;					\
		volatile TypExchange BridgeBufferB;					\
		volatile unsigned char Mod_PreBufferTx[K_MaxReg*2];	\
		volatile char RetryCnt = 0;							\
		volatile char i=0;									\
		volatile char DataIndex = 0;						\
		volatile TypBusError BusError;
/*
		volatile char Indice = 0;							\
		volatile int Data = 0;								\
*/
		/*extern volatile char Result;								\*/

	#define IncludeVariabiliGlobaliProtocolloModBus()				\
		extern volatile TypTimer TimeOutModPktRx;					\
		extern volatile TypExchange BridgeBufferA;					\
		extern volatile TypExchange BridgeBufferB;					\
		extern volatile unsigned char Mod_PreBufferTx[K_MaxReg*2];	\
		extern volatile char RetryCnt;								\
		extern volatile char i;										\
		extern volatile char DataIndex;								\
		extern volatile TypBusError BusError;
/*
		extern volatile char Indice;								\
		extern volatile int Data;									\ 
 */

	//----------------------------------
	// Strutture
	//----------------------------------
#if (GB210==1 || DA210==1)
#define	K_MaxReg	1024 //256
#else
#define	K_MaxReg	550 //256
#endif

/*
	typedef struct
	{
		int             RegData[K_MaxReg];
		unsigned char 	RegData_ChangeFlagBUS[K_MaxReg];				
		unsigned char 	RegData_UnreadFlagBUS[K_MaxReg];								
	} TypExchange;
*/
	typedef struct
	{
		int             Data[K_MaxReg];
		int				OldData[K_MaxReg];
		unsigned char 	cFlag[K_MaxReg];	
		unsigned char	LastWrite[K_MaxReg];		
	} TypExchange;
	
	
	typedef struct
	{
		struct
		{	
			unsigned int	CRCError;
			unsigned int	LenError;	
			unsigned int	CmdError;				
			unsigned int	SyncError;
			unsigned int	TmtError;			
			unsigned int	ValidData;
		} SideA;
		
		struct
		{	
			unsigned int	CRCError;
			unsigned int	LenError;	
			unsigned int	CmdError;				
			unsigned int	SyncError;			
			unsigned int	TmtError;			
			unsigned int	ValidData;
		} SideB;		
	} TypBusError;


	//---------------------------------------
	// Variabili 
	//---------------------------------------
	IncludeVariabiliGlobaliDriverModBus();
	IncludeVariabiliGlobaliProtocolloModBus();
	//IncludeVariabiliGlobaliCore();

	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
	unsigned char WaitModRx(int TimeOut_ms);
	unsigned char Send_WaitModRx(char Address, char Comando, int Registro, int Data, int TimeOut_ms, char Retry);
	unsigned char Send_WriteModReg(char Address, int Registro, int Data, int TimeOut_ms, char Retry, char Cmd);
	
	void ReadModReg (char Address, int REG, int * StoreReg);
	//unsigned char ReadModListReg (char DataIndex, int REG, int RegNum);
	void WriteModReg (char Address, int REG, int SendData);
	//void WriteModListReg (char Address, int REG, int SendData);
	/*
	void ReadModRegNetB (char Address, int REG, int * StoreReg);
	unsigned char ReadModListRegNetB (char DataIndex, int REG, int RegNum);
	void WriteModRegNetB (char Address, int REG, int SendData);
	void WriteModListRegNetB (char Address, int REG, int SendData);
	*/
	int ExtracReg(char NumReg);
	void InsertReg(char NumReg, int Data);

	/*
	void Search_Touch(void);
	void Resume_TouchOffLine(char * Cnt);

	void RefreshTouchInPut(char TouchAddres);
	void RefreshAllTouchInPut(void);

	void RefreshTouchOutPut(char TouchAddres);
	void RefreshAllTouchOutPut(void);

	void InitTouch(void);

	// Aggiunti il 04/01/2014 per gestire pannelli NetBuilding
	void Search_NetB(void);
	void Resume_NetBOffLine(char * Cnt);

	void RefreshNetBInPut(char NetBAddres);
	void RefreshAllNetBInPut(void);

	void RefreshNetBOutPut(char NetBAddres);
	void RefreshAllNetBOutPut(void);

	void InitNetB(void);
	 */

    //int	Mod_GetData(int Reg);
    char Mod_PutData(int Reg, int Value);   
	//GL char Mod_PutDataMulti(int Reg, int StartBuff, unsigned char ByteLen);
	int Mod_GetDataMulti(int Reg, int DataLen);
    void Mod_Process_Cmd(void);    
	void Mod_Reply_Cmd(char Address, char Comando, int Registro, int Value, int nData);
	//GL void CheckBufferSync(void);
	//GL int SendInitBufferToSideB(void);
	//GL int SendInitBufferToSideBdue(void);	
	//GL int CheckMasterInitFLag(void);
	//GL int CheckMasterSyncFLag(void);
	//GL void CheckCommTimeoutFlag_SideA(void);
	//GL void CheckCommTimeoutFlag_SideB(void);
	
#endif
