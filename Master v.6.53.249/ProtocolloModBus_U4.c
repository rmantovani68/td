//----------------------------------------------------------------------------------
//	Progect name:	ProtocolloModBus_U4.c
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			TDM Team
//	Date:			11/04/2022
//	Description:	Corpo delle funzioni che gestiscono la comunicazione ModBus su UART4
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//	Include
//----------------------------------------------------------------------------------
#include "FWSelection.h"
#if (K_AbilMODBCAN==1)  

#include "HWSelection.h"
#include <p24FJ256GB210.h>

#include <stdio.h>
#include "DefinePeriferiche.h"
#include "Timer.h"
#include "usart.h"
#include "Driver_Comunicazione.h"
#include "ProtocolloComunicazione.h"
#include "Driver_ModBus.h"
#include "Driver_ModBus_U4.h"
#include "core.h"
//#include "FWSelection.h"
#include "ProtocolloModBus.h"
#include "ProtocolloModBus_U4.h"
#include "delay.h"
#include "Bridge1DataAddressMap.h"
#include "Bridge2DataAddressMap.h"
#include "DAC.h"
#include "ModBusCAN.h"


extern volatile int ModTimeOutComErr_U4;		
extern volatile int ModCntComErr_U4;			


//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------

unsigned char WaitModRx_U4(int TimeOut_ms)
{
	TimeOutModPktRx_U4.Value = 0;				// resetto il timer per il time out della risposta
	TimeOutModPktRx_U4.Time = TimeOut_ms;		// lo configuro come richiesto
	TimeOutModPktRx_U4.Enable = 1;				// lo faccio partireOldApp1
	TimeOutModPktRx_U4.TimeOut = 0;			//
    

	while((TimeOutModPktRx_U4.TimeOut == 0) && (Mod_LastRx_U4.Valid_Data == 0))	// attendo un evento
		continue;
   
    TimeOutModPktRx_U4.Enable = 0;			// fermo il timer
    TimeOutModPktRx_U4.TimeOut = 0;		// resetto il time out
    iPckModRx_U4 = 0;  
    

	if(Mod_LastRx_U4.Valid_Data != 0)		// se ho ricevuto un dato valido 
	{
		if( (Mod_LastRx_U4.Buffer[0] == Mod_BufferTx_U4.Buffer[0]) &&	// verifico il pacchetto
			(Mod_LastRx_U4.Buffer[1] == Mod_BufferTx_U4.Buffer[1]))
			return 0;											// ritorno 0 se è ok
	}									// in caso contrario
    
    if(EngineBox[0].DisableModComErrorCounter_U4==0)
    {
            ModTimeOutComErr_U4++;
            EngineBox[0].ComError.ModTimeOutComErr_U4++;
    }
    
	return 1;							// ritorno 1
}

unsigned char Send_WaitModRx_U4(char Address, char Comando, int Registro, int Data, int TimeOut_ms, char Retry)
{
	Result_U4 = 0;
	RetryCnt_U4 = 0;
	while (RetryCnt_U4 < Retry)
	{	
		RetryCnt_U4 +=1;
        Mod_Write_Cmd_U4(Address, Comando, Registro, Data);
		Result_U4 = WaitModRx_U4(TimeOut_ms);
		if (Result_U4 == 0)
        {
			Mod_LastRx_U4.Valid_Data = 0;            
			DelayuSec(1000); //00);		// v.5.16.162 - Pausa per garantire una minima distanza tra un Tx e il successivo 
        	return 0;		// se ricevo un pacchetto valido 
        }
	}
	//TimerModRstComErr_U4.Value = 0;       
    ModCntComErr_U4++;
    EngineBox[0].ComError.ModCntComErr_U4++;
    
	Mod_LastRx_U4.Valid_Data = 0;
	return 1;				// se non ho nessuna risposta valida per dopo il numero di tentativi richiesti
}

unsigned char Send_WriteModReg_U4(char Address, int Registro, int Data, int TimeOut_ms, char Retry, char Cmd)
{
	Result_U4 = 1;
	RetryCnt_U4 = 0;
	while (RetryCnt_U4 < Retry)
	{
		RetryCnt_U4 +=1;
        Mod_Write_Cmd_U4(Address, Cmd, Registro, Data);
		Result_U4 = WaitModRx_U4(TimeOut_ms);
		if ((Result_U4 == 0) &&
			(Mod_LastRx_U4.Buffer[2] == Mod_BufferTx_U4.Buffer[2]) &&
			(Mod_LastRx_U4.Buffer[3] == Mod_BufferTx_U4.Buffer[3]) &&
			(Mod_LastRx_U4.Buffer[4] == Mod_BufferTx_U4.Buffer[4]) &&
			(Mod_LastRx_U4.Buffer[5] == Mod_BufferTx_U4.Buffer[5])  )
		{
			Mod_LastRx_U4.Valid_Data = 0;
			DelayuSec(1000); //00);		// v.5.16.162 - Pausa per garantire una minima distanza tra un Tx e il successivo 
			return 0;
		}
	}
	//TimerModRstComErr_U4.Value = 0;    
    ModCntComErr_U4++;
    EngineBox[0].ComError.ModCntComErr_U4++;
    
	Mod_LastRx_U4.Valid_Data = 0;
	return 1;
}


void ReadModReg_U4(char Address, int REG, int * StoreReg)
{
	char Result_U4;
	if(!ModbCAN.OffLine)
	{
		Result_U4 = Send_WaitModRx_U4(Address, ReadModCmd, REG, 1, MaxModReadWait, MaxModReadRetries);
		if(Result_U4 == 0)	// Il registro dei touch hanno un offset di 1
		{
			(*StoreReg) = Mod_LastRx_U4.Data;
			Mod_LastRx_U4.Valid_Data = 0;
		}
		else
		{
			ModbCAN.OffLine = 1;
		}
	}
}

char ReadModListReg_U4(char DataIndex, int REG, int RegNum)
{
/*	char TouchAddres;
	TouchAddres = DataIndex+Add_TouchRoom1;
	if(!Room[DataIndex].Touch_OffLine)
	{
		if(Send_WaitModRx(TouchAddres, ReadModCmd, REG, RegNum, MaxModReadWait, MaxModReadRetries))	// Il registro dei touch hanno un offset di 1
		{
			Room[DataIndex].Touch_OffLine = 1;
			Mod_LastRx.Valid_Data = 0;
			return 1;
		}
		else
		{
			Mod_LastRx.Valid_Data = 0;
			return 0;
		}
	}
	return 1;*/
}

void WriteModReg_U4(char Address, int REG, int SendData)
{
	unsigned char retcode;
/*	if(!Room[Address-Add_TouchRoom1].Touch_OffLine)
	{
		Room[Address-Add_TouchRoom1].Touch_OffLine = Send_WriteModReg(Address, REG, SendData, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Il registro dei touch hanno un offset di 1
	}*/
	retcode = Send_WriteModReg_U4(Address, REG, SendData, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Il registro dei touch hanno un offset di 1
}

void WriteModListReg_U4(char Address, int REG, int SendData)
{/*
	if(!Room[Address-Add_TouchRoom1].Touch_OffLine)
	{
		Room[Address-Add_TouchRoom1].Touch_OffLine = Send_WriteModReg(Address, REG, SendData, MaxModWriteWait, MaxModWriteRetries, WriteModListCmd);	// Il registro dei touch hanno un offset di 1
	}*/
}

int ExtracReg_U4(char NumReg)
{
	// recupero un valore int dalla coda del buffer l'indice dei registri va da 1 al limite della richiesta
	Indice_U4 = 0;
	Data_U4 = 0;

	Indice_U4 = 1+(NumReg*2);
	Data_U4 = Mod_LastRx_U4.Buffer[Indice_U4]<<8;
	Data_U4 |= (Mod_LastRx_U4.Buffer[Indice_U4+1]&0x00FF);
	return Data_U4;
}

void InsertReg_U4(char NumReg, int Data)
{
	Indice_U4 = 0;
	
	Indice_U4 = 5+(NumReg*2);
	Mod_BufferTx_U4.Buffer[Indice_U4] = (char)((Data&0xFF00)>>8);
	Mod_BufferTx_U4.Buffer[Indice_U4+1] = (char)((Data&0x00FF));
}
/*
void InsertReg2(char NumReg, int Data)
{
	Indice = 0;
	
	Indice = 5+((NumReg+1)*2);
	Mod_BufferTx.Buffer[Indice] = (char)((Data&0xFF00)>>8);
	Mod_BufferTx.Buffer[Indice+1] = (char)((Data&0x00FF));
}
*/


#endif //#if (K_AbilMODBCAN==1) 

