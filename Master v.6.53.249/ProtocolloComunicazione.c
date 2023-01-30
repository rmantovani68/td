//----------------------------------------------------------------------------------
//	Progect name:	ProtocolloComunicazione.c
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			Emanuele
//	Date:			04/01/2014
//	Description:	Corpo delle funzioni che gestiscono la comunicazione con il sistema
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//	Include
//----------------------------------------------------------------------------------
#include "HWSelection.h"
#include "FWSelection.h"
#if (DA210==1)
	#include <p24FJ256DA210.h>
#else
	#include <p24FJ256GB210.h>
#endif

#include <stdio.h>
#include "DefinePeriferiche.h"
#include "EEPROM.h"
#include "Timer.h"
#include "usart.h"
#include "ADC.h"
#include "DAC.h"
#include "Driver_Comunicazione.h"
#include "Driver_ModBus.h"
#include "ProtocolloComunicazione.h"
#include "ProtocolloModBus.h"
#include "Bridge2DataAddressMap.h"
#include "Core.h"
#include "delay.h"
#include "PID.h"
//#include "PWM.h"
#if (K_AbilMODBCAN==1)  
#include "ModBusCAN.h"
#endif //#if (K_AbilMODBCAN==1)


extern TypRTC MyRTC;						// RTC di sistema
//extern volatile TypTimer TimerRstComErr;
extern volatile int CntComErr;
extern volatile int TimeOutComErr;
extern volatile int CRC_ComErr;
extern volatile int CommandNotImplemented;
extern volatile TypTimer TimerRestartAfterBlackOut;	// Timer per gestire la pausa alla riaccensione da blackout se i compressori erano in funzione 
extern unsigned char CurUsedProtocolId;		// Id del protocollo correntemente utilizzato
extern volatile TypTimer TimerValve_On_Demand_Limit;	
extern volatile TypTimer TimerValve_On_Demand_Switch;	


//IncludeVariabiliGlobaliEEPROM();
//VariabiliGlobaliProtocolloComunicazione();

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
unsigned char WaitRx(int TimeOut_ms)
{
	TimeOutPktRx.Value = 0;				// resetto il timer per il time out della risposta
	TimeOutPktRx.Time = TimeOut_ms;		// lo configuro come richiesto
	TimeOutPktRx.Enable = 1;			// lo faccio partire
	TimeOutPktRx.TimeOut = 0;			//


	while((TimeOutPktRx.TimeOut == 0) && (LastRx.Valid_Data == 0))	// attendo un evento
        continue;
    
    if(BufferTx.Comando!=Check)
    {
        if(TimeOutPktRx.TimeOut)
        {
            TimeOutComErr +=1; 
            EngineBox[0].ComError.TimeOutComErr++;
        }
    }
    
    TimeOutPktRx.Enable = 0;			// fermo il timer
	TimeOutPktRx.TimeOut = 0;			// resetto il time out
    iPckRx = 0;       
    
	if(LastRx.Valid_Data == 1)			// se ho ricevuto un dato valido 
	{
		if((LastRx.Mitt_Add == BufferTx.Dest_Add) &
			(LastRx.Comando == BufferTx.Comando) &
			(LastRx.Registro == BufferTx.Registro)) 	// verifico il pacchetto
			return 0;				// ritorno 0 se è ok
	}
	return 1;						// ritorno 1
}

unsigned char Send_WaitRx(char Address, char Comando, int Registro, int Data, int TimeOut_ms, char Retry)
{
	char Result = 0;
	char RetryCnt = 0;
	while (RetryCnt < Retry)
	{	
		RetryCnt +=1;
		Write_Cmd(&BufferTx, Address, Comando, Registro, Data);
		Result = WaitRx(TimeOut_ms);
		if (Result == 0)	// se ricevo un pacchetto valido 
		{
			LastRx.Valid_Data = 0;
			return 0;
		}
	}
    Room[Address].OffLine = 1;
	Room[Address].OnLine = 0;
    if(Room[Address].Enable==1)
    {
        CntComErr +=1; 
        EngineBox[0].ComError.CntComErr++;
    }
	LastRx.Valid_Data = 0;    
	return 1;				// se non ho nessuna risposta valida per dopo il numero di tentativi richiesti
}

unsigned char Send_WriteReg(char Address, int Registro, int Data, int TimeOut_ms, char Retry)
{
	char Result = 1;
	char RetryCnt = 0;
	while (RetryCnt < Retry)
	{
		RetryCnt +=1;
		Write_Cmd(&BufferTx, Address, WriteReg, Registro, Data);
		Result = WaitRx(TimeOut_ms);
		if ((Result == 0) & (LastRx.Data == Ack))
		{
			LastRx.Valid_Data = 0;
			return 0;
		}
		// Aggiunto in v5.17.167 per gestire risposte da comandi non implementati nello Slave (risposta )
		if ((Result == 0) & (LastRx.Data == N_Ack))
		{
			LastRx.Valid_Data = 0;
			return 2;
		}
	
		
		LastRx.Valid_Data = 0;
	}
    Room[Address].OffLine = 1;
	Room[Address].OnLine = 0;   
    if(Room[Address].Enable==1)
    {
        CntComErr +=1; 
        EngineBox[0].ComError.CntComErr++;
    }
	LastRx.Valid_Data = 0;
	return 1;
}

unsigned char SendBrc_WriteReg(char Cmd, int Registro, int Data, int TimeOut_ms, char Retry)
{
	//char Result = 1;
	char RetryCnt = 0;
	while (RetryCnt < Retry)
	{
		RetryCnt +=1;
		Write_brc_Cmd(&BufferTx, Cmd, Registro, Data);
 
        DelayuSec(TimeOut_ms);
       /*
		TimeOutPktRx.Value = 0;				// resetto il timer per il time out della risposta
		TimeOutPktRx.TimeOut = 0;			//
		TimeOutPktRx.Time = TimeOut_ms;		// lo configuro come richiesto
		TimeOutPktRx.Enable = 1;			// lo faccio partire
		while(TimeOutPktRx.TimeOut == 0)	// attendo un evento
			continue;
               
		TimeOutPktRx.Enable = 0;			// fermo il timer
		TimeOutPktRx.TimeOut = 0;			// resetto il time out
        */
	}
	LastRx.Valid_Data = 0;
	return 0;
}

unsigned FuncReadReg (char Address, int REG, int * StoreReg, unsigned OffLineFlag)
{
	if(!OffLineFlag)
	{
#if (K_EnableHiSpeedMyBus==K_Baud115200)
        Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
        Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
        Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    

		if(!Send_WaitRx(Address, ReadReg, REG, 0, MaxReadWait, MaxReadRetries))
		{
			(*StoreReg) = LastRx.Data;
			LastRx.Valid_Data = 0;
            DelayuSec(k_WaitSendMessage); //Argo Serve                                    //Pausa per garantire una minima distanza tra un Tx e il successivo 		                        
			return 0;
		}
	}
	LastRx.Valid_Data = 0;
	//TimerRstComErr.Value = 0;
	//CntComErr +=1;
	return 1;// se non ha risposto
}

unsigned FuncWriteReg (char Address, int REG, int SendData, unsigned OffLineFlag)
{
	unsigned char replay;
	if(!OffLineFlag)
	{	
#if (K_EnableHiSpeedMyBus==K_Baud115200)
        Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
        Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
        Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    
     
		replay=Send_WriteReg(Address, REG, SendData, MaxWriteWait, MaxWriteRetries);
		if((replay==0))
		{
			LastRx.Valid_Data = 0;
            DelayuSec(k_WaitSendMessage); //Argo Serve                                     //Pausa per garantire una minima distanza tra un Tx e il successivo 		                        
			return 0;
		}
		else if(replay==2)
		{
			LastRx.Valid_Data = 0;
			CommandNotImplemented=1;
            DelayuSec(k_WaitSendMessage); //Argo Serve                                     //Pausa per garantire una minima distanza tra un Tx e il successivo 		                        
			return 0;			
		}
	}
	LastRx.Valid_Data = 0;
	//CntComErr +=1;
	//TimerRstComErr.Value = 0;
	return 1;
}

unsigned FuncWriteBrcReg (char Comando, int REG, int SendData)
{    
#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    
 
	return SendBrc_WriteReg(Comando, REG, SendData, WriteBrcWait, WriteBrcRetries);
}

void CheckModuli(void)
{
	char Result = 0;
	char i = 0;
    
#if (K_EnableHiSpeedMyBus!=0)
	//FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_SET_BAUDRATE, 1);	// Invio sul BUS in broadcast il Baud: /Vaule: 1 = HiSpeed (115000Baud) 0 = LowSpeed (57600Baud)
	SendBrc_WriteReg(WriteSplitRegBrdCst, REG_SPLIT_SET_BAUDRATE, K_EnableHiSpeedMyBus, WriteBrcWait, WriteBrcRetries);
    
if(K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
if(K_EnableHiSpeedMyBus==K_Baud250000)
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    
#else
	//FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_SET_BAUDRATE, 0);	// Invio sul BUS in broadcast il Baud: /Vaule: 1 = HiSpeed (115000Baud) 0 = LowSpeed (57600Baud)    
	Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	
    
	// Scansiono la rete alla ricerca dei moduli di gestione delle valvole e degli split
	for(i=1;i<Max_Room; i++)
	{
		Result = Send_WaitRx(i, Check, 0, 0, MaxChkWait, MaxChkRetries);
		Room[i].OffLine = (Result != 0);
		LastRx.Valid_Data = 0;
		Room[i].Enable = !Room[i].OffLine;
		Room[i].OnLine = (Room[i].Enable == 1) && (Room[i].OffLine == 0);
        
        if(Room[i].OnLine)
            SendWorkingSplitData(i);		// Invio dati di lavoro agli Split (SuperHeat Setpoint, Working Pressure Hot & Cool)
	}
}

void ResumeOffLine(char * Cnt)
{
	char Result = 0;
	char CurrentId = 0;

#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    
    
	while((*Cnt) < Max_Room)				// Provo a recuperare i moduli di gestione Split
	{
		CurrentId = (*Cnt);
		if((Room[CurrentId].Enable == 1) & (Room[CurrentId].OffLine == 1))
		{
#if (K_EnableHiSpeedMyBus!=0)
            Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
            //FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_SET_BAUDRATE, 1);	// Invio sul BUS in broadcast il Baud: /Vaule: 1 = HiSpeed (115000Baud) 0 = LowSpeed (57600Baud)
            //FuncWriteReg (CurrentId, REG_SPLIT_SET_BAUDRATE, 1, 0);        // Invio sul BUS in broadcast il Baud: /Vaule: 1 = HiSpeed (115000Baud) 0 = LowSpeed (57600Baud)
            SendBrc_WriteReg(WriteSplitRegBrdCst, REG_SPLIT_SET_BAUDRATE, K_EnableHiSpeedMyBus, WriteBrcWait, WriteBrcRetries);
            if(K_EnableHiSpeedMyBus==K_Baud115200)
                Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
            if(K_EnableHiSpeedMyBus==K_Baud250000)
                Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	                              
			Result = Send_WaitRx(CurrentId, Check, 0, 0, MaxChkWait, MaxChkRetries);
			Room[CurrentId].OffLine = (Result != 0);
			LastRx.Valid_Data = 0;
            Room[CurrentId].OnLine = (Room[CurrentId].Enable == 1) && (Room[CurrentId].OffLine == 0);       
            if(Room[CurrentId].OnLine)       
                SendWorkingSplitData(CurrentId);		// Invio dati di lavoro agli Split (SuperHeat Setpoint, Working Pressure Hot & Cool)            
			(*Cnt) += 1;
			break;
		}
		Room[CurrentId].OnLine = (Room[CurrentId].Enable == 1) && (Room[CurrentId].OffLine == 0);
		(*Cnt) += 1;
	}
	if(((*Cnt) >= Max_Room) || ((*Cnt) < 1))
		(*Cnt) = 1;
}


void CheckHotPlugSlave(char * Cnt)
{
	char Result = 0;
	char CurrentId = 0;

#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    
    
	while((*Cnt) < Max_Room)				// Provo a recuperare i moduli di gestione Split
	{
		CurrentId = (*Cnt);
		if((Room[CurrentId].Enable == 0))
		{
            
#if (K_EnableHiSpeedMyBus!=0)
            Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
            //FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_SET_BAUDRATE, 1);	// Invio sul BUS in broadcast il Baud: /Vaule: 1 = HiSpeed (115000Baud) 0 = LowSpeed (57600Baud)
            //FuncWriteReg (CurrentId, REG_SPLIT_SET_BAUDRATE, 1, 0);        // Invio sul BUS in broadcast il Baud: /Vaule: 1 = HiSpeed (115000Baud) 0 = LowSpeed (57600Baud)
            SendBrc_WriteReg(WriteSplitRegBrdCst, REG_SPLIT_SET_BAUDRATE, K_EnableHiSpeedMyBus, WriteBrcWait, WriteBrcRetries);
            if(K_EnableHiSpeedMyBus==K_Baud115200)
                Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
            if(K_EnableHiSpeedMyBus==K_Baud250000)
                Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	                  
            
			Result = Send_WaitRx(CurrentId, Check, 0, 0, 5, 1);
			Room[CurrentId].OffLine = (Result != 0);
			Room[CurrentId].Enable = !Room[CurrentId].OffLine;	// Se la trovo la abilito.
			Room[CurrentId].OnLine = (Room[CurrentId].Enable == 1) && (Room[CurrentId].OffLine == 0);
            if(Room[CurrentId].OnLine)       
                SendWorkingSplitData(CurrentId);		// Invio dati di lavoro agli Split (SuperHeat Setpoint, Working Pressure Hot & Cool)            
			LastRx.Valid_Data = 0;
			(*Cnt) += 1;
			break;
		}
		(*Cnt) += 1;
	}
	if(((*Cnt) >= Max_Room) || ((*Cnt) < 1))
		(*Cnt) = 1;
}


// OK
void RefreshInPutRoom(TypRoom * DataStore, char Address, int Sched)
{
    int temp;
    
#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    
    
    
//#if (K_AbilMODBCAN==1)       
//	if(DataStore->OnLine == 1 && ModbCAN.SetRoom[Address].UpdateFlag==0)    		// se abilitato e OnLine & non è bloccato da un aggiornamento HMI CAN
//#else        
	if(DataStore->OnLine == 1)		// se abilitato e OnLine
//#endif        
	{
        if(Address==k_Split_Master_Add)
        {
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_POWER_LIMIT_REQ, &(DataStore->PowerLimit), DataStore->OffLine);
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_POWER_ECOMODE, &(DataStore->Power_EcoMode), DataStore->OffLine);
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_ECOMODE_ENABLE, &(DataStore->EcoModeEnable), DataStore->OffLine);		
    		DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_DEFINE_MODE, &(DataStore->DefMode), DataStore->OffLine);
    		DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_SYSTEM_DISABLE, &(DataStore->SystemDisable), DataStore->OffLine);  
#if(K_AbilScoreOnDemand==1)            
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_WR_ON_PRIORITY_MODE, &(DataStore->On_Priority_Mode), DataStore->OffLine);  
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_WR_VALVE_ONDEMAND_MIN_PERC_VAL, &(DataStore->Valve_OnDemand_Min_Perc_Val), DataStore->OffLine);  
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_WR_MAX_LIM_ONDEMAND_TOTAL_SCORE, &(DataStore->Max_Lim_ON_Demand_Total_Score), DataStore->OffLine);  
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_WR_TIME_ONDEMAND_VALVE_SWITCH, &(DataStore->TimeOnDemandValveSwitch), DataStore->OffLine);  
#endif
            
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_ABIL, &(DataStore->TestAll_Abil), DataStore->OffLine);  
            EngineBox[0].TestAllAbil.SuperHeat_SP_Cool = ((Room[k_Split_Master_Add].TestAll_Abil & 0x0001) != 0);
            EngineBox[0].TestAllAbil.SuperHeat_SP_Heat = ((Room[k_Split_Master_Add].TestAll_Abil & 0x0002) != 0);
            EngineBox[0].TestAllAbil.MaxOpValve_Cool   = ((Room[k_Split_Master_Add].TestAll_Abil & 0x0004) != 0);
            EngineBox[0].TestAllAbil.MaxOpValve_Heat   = ((Room[k_Split_Master_Add].TestAll_Abil & 0x0008) != 0);
            EngineBox[0].TestAllAbil.TempBatt_SP_Cool  = ((Room[k_Split_Master_Add].TestAll_Abil & 0x0010) != 0);
            EngineBox[0].TestAllAbil.TempBatt_SP_Heat  = ((Room[k_Split_Master_Add].TestAll_Abil & 0x0020) != 0);
            EngineBox[0].TestAllAbil.MinOpValve_Cool   = ((Room[k_Split_Master_Add].TestAll_Abil & 0x0040) != 0);
            EngineBox[0].TestAllAbil.MinOpValve_Heat   = ((Room[k_Split_Master_Add].TestAll_Abil & 0x0080) != 0);
            EngineBox[0].TestAllAbil.SuperHeat_Custom_Par_Heat_PID = ((Room[k_Split_Master_Add].TestAll_Abil & 0x0100) != 0);
            EngineBox[0].TestAllAbil.SuperHeat_Custom_Par_Cool_PID = ((Room[k_Split_Master_Add].TestAll_Abil & 0x0200) != 0);
            EngineBox[0].TestAllAbil.SuperHeat_Cool_Correct = ((Room[k_Split_Master_Add].TestAll_Abil & 0x0400) != 0);
            EngineBox[0].TestAllAbil.SuperHeat_Heat_Correct = ((Room[k_Split_Master_Add].TestAll_Abil & 0x0800) != 0);
            EngineBox[0].TestAllAbil.Split_Probe_Error = ((Room[k_Split_Master_Add].TestAll_Abil & 0x1000) != 0);
            EngineBox[0].TestAllAbil.Master_Probe_Error = ((Room[k_Split_Master_Add].TestAll_Abil & 0x2000) != 0);
            EngineBox[0].TestAllAbil.Compression_Error = ((Room[k_Split_Master_Add].TestAll_Abil & 0x4000) != 0);
            if(EngineBox[0].TestAllAbil.Programming_Mode = ((Room[k_Split_Master_Add].TestAll_Abil & 0x8000) != 0))
            {
                EngineBox[0].TestAllAbil.Programming_Mode = 0;
                EngineBox[0].ProgrammingModeEnable = 1;
            }

            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_SUPERHEAT_SP_COOL, &(DataStore->TestAll_SuperHeat_SP_Cool), DataStore->OffLine);          
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_SUPERHEAT_SP_HEAT, &(DataStore->TestAll_SuperHeat_SP_Heat), DataStore->OffLine);          
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_MAX_OPEN_VALVE_COOL, &(DataStore->TestAll_MaxOpValve_Cool), DataStore->OffLine);              
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_MAX_OPEN_VALVE_HEAT, &(DataStore->TestAll_MaxOpValve_Heat), DataStore->OffLine);              
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_WORK_TEMP_COOL, &(DataStore->TestAll_TempBatt_SP_Cool), DataStore->OffLine);  
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_WORK_TEMP_HEAT, &(DataStore->TestAll_TempBatt_SP_Heat), DataStore->OffLine);  
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_MIN_OPEN_VALVE_COOL, &(DataStore->TestAll_MinOpValve_Cool), DataStore->OffLine);              
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_MIN_OPEN_VALVE_HEAT, &(DataStore->TestAll_MinOpValve_Heat), DataStore->OffLine);              
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_SUPERHEAT_HEAT_PGAIN, &(DataStore->TestAll_SuperHeat_Heat_pGain), DataStore->OffLine);          
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_SUPERHEAT_HEAT_IGAIN, &(DataStore->TestAll_SuperHeat_Heat_iGain), DataStore->OffLine);          
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_SUPERHEAT_HEAT_DGAIN, &(DataStore->TestAll_SuperHeat_Heat_dGain), DataStore->OffLine);          
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID, &(DataStore->TestAll_SuperHeat_Heat_Time_Exec_PID), DataStore->OffLine);          
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_SUPERHEAT_COOL_PGAIN, &(DataStore->TestAll_SuperHeat_Cool_pGain), DataStore->OffLine);          
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_SUPERHEAT_COOL_IGAIN, &(DataStore->TestAll_SuperHeat_Cool_iGain), DataStore->OffLine);          
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_SUPERHEAT_COOL_DGAIN, &(DataStore->TestAll_SuperHeat_Cool_dGain), DataStore->OffLine);          
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID, &(DataStore->TestAll_SuperHeat_Cool_Time_Exec_PID), DataStore->OffLine);          

            if(EngineBox[0].Frigo_Abil==1 || EngineBox[0].Freezer_Abil==1 || EngineBox[0].LoopWater_Abil==1)
            {
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_FROZEN_ABIL, &(DataStore->TestAll_Frozen_Abil), DataStore->OffLine);  
                EngineBox[0].TestAllAbil.SuperHeat_SP_Frigo = ((Room[k_Split_Master_Add].TestAll_Frozen_Abil & 0x0001) != 0);
                EngineBox[0].TestAllAbil.SuperHeat_SP_Freezer = ((Room[k_Split_Master_Add].TestAll_Frozen_Abil & 0x0002) != 0);
                EngineBox[0].TestAllAbil.MaxOpValve_Frigo   = ((Room[k_Split_Master_Add].TestAll_Frozen_Abil & 0x0004) != 0);
                EngineBox[0].TestAllAbil.MaxOpValve_Freezer   = ((Room[k_Split_Master_Add].TestAll_Frozen_Abil & 0x0008) != 0);
                EngineBox[0].TestAllAbil.TempBatt_SP_Frigo  = ((Room[k_Split_Master_Add].TestAll_Frozen_Abil & 0x0010) != 0);
                EngineBox[0].TestAllAbil.TempBatt_SP_Freezer  = ((Room[k_Split_Master_Add].TestAll_Frozen_Abil & 0x0020) != 0);
                EngineBox[0].TestAllAbil.MinOpValve_Frigo   = ((Room[k_Split_Master_Add].TestAll_Frozen_Abil & 0x040) != 0);
                EngineBox[0].TestAllAbil.MinOpValve_Freezer   = ((Room[k_Split_Master_Add].TestAll_Frozen_Abil & 0x080) != 0);
            }
            if(EngineBox[0].Frigo_Abil==1)
            {
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_SUPERHEAT_SP_FRIGO, &(DataStore->TestAll_SuperHeat_SP_Frigo), DataStore->OffLine);          
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_MAX_OPEN_VALVE_FRIGO, &(DataStore->TestAll_MaxOpValve_Frigo), DataStore->OffLine);              
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_WORK_TEMP_FRIGO, &(DataStore->TestAll_TempBatt_SP_Frigo), DataStore->OffLine);  
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_MIN_OPEN_VALVE_FRIGO, &(DataStore->TestAll_MinOpValve_Frigo), DataStore->OffLine);              
            }
            if(EngineBox[0].Freezer_Abil==1)
            {
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_SUPERHEAT_SP_FREEZER, &(DataStore->TestAll_SuperHeat_SP_Freezer), DataStore->OffLine);          
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_MAX_OPEN_VALVE_FREEZER, &(DataStore->TestAll_MaxOpValve_Freezer), DataStore->OffLine);              
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_WORK_TEMP_FREEZER, &(DataStore->TestAll_TempBatt_SP_Freezer), DataStore->OffLine);  
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_MIN_OPEN_VALVE_FREEZER, &(DataStore->TestAll_MinOpValve_Freezer), DataStore->OffLine);              

                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_ABIL_DEFROSTING, &(DataStore->Defrosting_Abil), DataStore->OffLine);  
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_DEFROSTING_STEP_TIME, &(DataStore->Defrosting_Step_Time), DataStore->OffLine);          
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_DEFROSTING_TIME, &(DataStore->Defrosting_Time), DataStore->OffLine);              
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_DEFROSTING_TEMP_SET, &(DataStore->Defrosting_Temp_Set), DataStore->OffLine);  
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_DRIPPING_TIME, &(DataStore->Dripping_Time), DataStore->OffLine);  
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_VALVOLAFROZEN, &(DataStore->Valvola_Frozen), DataStore->OffLine);                  
            }       
            else if(EngineBox[0].LoopWater_Abil==1)
            {
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_SUPERHEAT_SP_FREEZER, &(DataStore->TestAll_SuperHeat_SP_Freezer), DataStore->OffLine);          
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_MAX_OPEN_VALVE_FREEZER, &(DataStore->TestAll_MaxOpValve_Freezer), DataStore->OffLine);              
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_WORK_TEMP_FREEZER, &(DataStore->TestAll_TempBatt_SP_Freezer), DataStore->OffLine);  
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TESTALL_MIN_OPEN_VALVE_FREEZER, &(DataStore->TestAll_MinOpValve_Freezer), DataStore->OffLine);              
            }       
            
            temp = DataStore->Maintenance_Flag;
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_MAINTENANCE_FLAG, &(DataStore->Maintenance_Flag), DataStore->OffLine);  
            if(temp != DataStore->Maintenance_Flag)
                StoreInt(DataStore->Maintenance_Flag, E2_Maintenance_Flag);                

            temp = DataStore->Maintenance_Set_Time;            
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_TOUCH_MAINTENANCE_SET_TIME, &(DataStore->Maintenance_Set_Time), DataStore->OffLine);
            DataStore->Maintenance_Set_Time = DataStore->Maintenance_Set_Time*24;
            if(temp != DataStore->Maintenance_Set_Time)
                StoreInt(DataStore->Maintenance_Set_Time, E2_Maintenance_Set_Time);                

            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_SUPERHEAT_COOL_MIN_SETPOINT, &(DataStore->SuperHeat_Cool_Min_SetPoint), DataStore->OffLine);
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_SUPERHEAT_COOL_MAX_VALUE_CORRECT, &(DataStore->SuperHeat_Cool_Max_Value_Correct), DataStore->OffLine);
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_SUPERHEAT_HEAT_MIN_SETPOINT, &(DataStore->SuperHeat_Heat_Min_SetPoint), DataStore->OffLine);       
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_SUPERHEAT_HEAT_MAX_SETPOINT, &(DataStore->SuperHeat_Heat_Max_SetPoint), DataStore->OffLine);       

            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_THRESHOLD_COMPRESSOR_HI, &(temp), DataStore->OffLine);	
            if(temp>=6900)
                EngineBox[0].TempCompressorHi = temp;
            
            //DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_SET_TEMP_AIR_OUT, &(DataStore->Set_Temp_Air_Out), DataStore->OffLine);        

            if(EngineBox[0].Dehumi_Abil==1)
            {
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_SET_HUMI, &(DataStore->Set_Humi), DataStore->OffLine);	
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_SET_DELTA_TEMP_MIN_DEHUMI, &(DataStore->Set_Delta_Temp_Min_Dehumi), DataStore->OffLine);	
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_SET_FAN_SPEED_DEHUMI, &(DataStore->Set_Fan_Speed_Dehumi), DataStore->OffLine);	
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_SET_PRESS_DEHUMI, &(DataStore->Set_Press_Dehumi), DataStore->OffLine);	
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_SET_TEMP_AIR_OUT_DEHUMI, &(DataStore->Set_Temp_Air_Out_Dehumi), DataStore->OffLine);	                
            }
        }
	
        DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_POWER, &(DataStore->PowerOn), DataStore->OffLine);
		DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_FUNCTION_MODE, &(DataStore->Mode), DataStore->OffLine);

		DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_VENTIL_MODE, &(DataStore->FanSpeed), DataStore->OffLine);	// Velocità di ventilazione 0->Spento; 1->Night; 2->Speed1; ...; 8->Speed7; 9->Auto;

        if((DataStore->OnlyFan != 1))  //Se non sono un Only fan normale
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_CURRENT_SP, &(DataStore->SetPoint), DataStore->OffLine);	// SP corrente per il calcolo della velocità in modalità auto

        if((DataStore->Model_FW_Version == 1) || (DataStore->Model_FW_Version == 2) || (DataStore->OnlyFan >= 4))  //Se sono UTA o se Only Fan Speciale (sfrutto registro UTA per funzioni ventilazione)
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_UTA_MODE, &(DataStore->UTAMode), DataStore->OffLine);	// Modalità UTA cabina

        //DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_SET_CMD_DISABLE, &(DataStore->Set_CMD_Disable), DataStore->OffLine);	// SP corrente per il calcolo della velocità in modalità auto        
        //DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_FAN_POWER, &(DataStore->Fan_Power), DataStore->OffLine);	// Modalità UTA cabina
        //DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_AND_SUB_FAN_POWER, &(DataStore->Total_Fan_Power), DataStore->OffLine);	// Modalità UTA cabina
#if (K_AbilMODBCAN==1)          
        DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_FAN_MODE, &(DataStore->FanMode), DataStore->OffLine);	// SP corrente per il calcolo della velocità in modalità auto        
#else   
        //DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_FAN_MODE, &(DataStore->FanMode), DataStore->OffLine);	// SP corrente per il calcolo della velocità in modalità auto                
#endif //#if (K_AbilMODBCAN==1)           
        
#if(K_AbilValveOnDemand==1 || K_AbilScoreOnDemand==1)        
        DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_VALVOLA_CUR_POS_PERC, &(DataStore->ValvePerc), DataStore->OffLine);	// Valvola in percentuale
        DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_VALVOLA_PID_COOL_PERC_POS, &(DataStore->Valve_Pid_Cooling), DataStore->OffLine);	// Valvola in percentuale
#endif            
                
        if(Sched==Address)
        {
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_CUMULATIVE_ALARM, &(DataStore->CumulativeAlarm), DataStore->OffLine);				
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_CURRENT_EVAP_TEMP, &(DataStore->Temp_Evaporazione), DataStore->OffLine);
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_TEMP_AMB_REAL, &(DataStore->AirTemp), DataStore->OffLine);	
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_TEMP_ARIA_USCITA, &(DataStore->OutTemp), DataStore->OffLine);	
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_FROZEN_MODE, &(DataStore->FrozenMode), DataStore->OffLine);  
            if(DataStore->FrozenMode == 1)  
                EngineBox[0].Frigo_Abil = 1;
            
            if(DataStore->FrozenMode == 2)  
                EngineBox[0].Freezer_Abil = 1;

            if(DataStore->FrozenMode == 3)  
                EngineBox[0].LoopWater_Abil = 1;

            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_ONLY_FAN, &(DataStore->OnlyFan), DataStore->OffLine);	// SP corrente per il calcolo della velocità in modalità auto        

            // Lettura dei flags di stato del Device Modbus collegato allo slave (TYPE: 0=Nessun Device, 1=Touch, 2=NetBuilding, 3=SyxthSense)
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_MODDEV_TYPE, &(DataStore->ModDev.Type), DataStore->OffLine);

            //Struttura per accendere automaticamente estrattore con altra unità
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_EXTRACTOR_NUMBER, &(DataStore->Extractor.Number), DataStore->OffLine);

            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_AND_SUB_AIR_POWER_OUTPUT, &(DataStore->TotalAirPowerOutput), DataStore->OffLine);	// Modalità UTA cabina
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_AND_SUB_AIR_POWER_BTU, &(DataStore->TotalAirPowerBTU), DataStore->OffLine);	// Modalità UTA cabina
            //DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_FAN_M3H, &(DataStore->Total_Fan_M3h), DataStore->OffLine);	// DA IMPLEMENTARE
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_AND_SUB_FAN_POWER, &(DataStore->Total_Fan_Power), DataStore->OffLine);	// Modalità UTA cabina

            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_MODEL_VERSION, &(DataStore->Model_FW_Version), DataStore->OffLine);	// Modalità UTA cabina

            //DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_READ_HUMIDITY, &(DataStore->Humidity), DataStore->OffLine);  
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_READ_HUMIDITY_MIDDLE, &(DataStore->HumidityMiddle), DataStore->OffLine);  

            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_HEATER_ABIL, &(DataStore->Heater_Abil), DataStore->OffLine); 
            
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_DEHUMI_ABIL, &(DataStore->Dehumi_Abil), DataStore->OffLine);   
            if(DataStore->Dehumi_Abil==1)
                EngineBox[0].Dehumi_Abil = 1;
        }
        if(DataStore->Heater_Abil)
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_HEATER_PWR, &(DataStore->HeaterPwr), DataStore->OffLine);  
            

        // Lettura dei flags di stato del Device Modbus collegato allo slave (TYPE: 0=Nessun Device, 1=Touch, 2=NetBuilding, 3=SyxthSense)
        //DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_MODDEV_TYPE, &(DataStore->ModDev.Type), DataStore->OffLine);
		DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_MODDEV_FLAGS_STATUS, &(DataStore->ModDev.Status), DataStore->OffLine);
        DataStore->ModDev.Alarm =   (DataStore->ModDev.Status & 0x0001) != 0;
        DataStore->ModDev.OnLine =  (DataStore->ModDev.Status & 0x0002) != 0;
        DataStore->ModDev.OffLine = (DataStore->ModDev.Status & 0x0004) != 0; 
        DataStore->ModDev.Enable =  (DataStore->ModDev.Status & 0x0008) != 0;
		
		DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_STATUS, &(DataStore->SplitStatus), DataStore->OffLine);
		if((DataStore->SplitStatus & 0x0001) != 0)
			EngineBox[0].In.ClearErrorReq =1;
		if((DataStore->SplitStatus & 0x0002) != 0)
			EngineBox[0].PersErr.EEV_Open =1;
		if((DataStore->SplitStatus & 0x0004) != 0)
			EngineBox[0].PersErr.EEV_Close =1;
		if((DataStore->SplitStatus & 0x0008) != 0)
			EngineBox[0].PersErr.BatteriaGhiacciata =1;
		if(((DataStore->SplitStatus & 0x0010) != 0) && (DataStore->Model_FW_Version == 8))  //Se la ventola è in errore e sono una ATEX Fan
			EngineBox[0].PersErr.Fan_Speed =1;
		if((DataStore->SplitStatus & 0x0020) != 0)
			EngineBox[0].PersErr.FireAlarm =1;
		//if((DataStore->SplitStatus & 0x0040) == 0)
        //EngineBox[0].Test.EEV_End = 0;
		if((DataStore->SplitStatus & 0x0040) != 0)
			EngineBox[0].AddressGasLeaks = Address;
		if((DataStore->SplitStatus & 0x0200) != 0)
			EngineBox[0].PersErr.Fan_OverLoad =1;
		if((DataStore->SplitStatus & 0x1000) != 0)
			EngineBox[0].Error.SPTemperatureFault =1;  
		if((DataStore->SplitStatus & 0x8000) != 0)
            EngineBox[0].In.ClearComErrorReq =1;

/*
		if(DataStore->PowerLimit != DataStore->OldPowerLimit)
		{	DataStore->OldPowerLimit = DataStore->PowerLimit;
			EngineBox[0].Inverter_Max_Power_Out = DataStore->PowerLimit;
		}
*/
        //Struttura per leggere dati altra cabina e scriverli su split
        if(DiagnosticSplit.OtherCabin.Send==0)
        {
    		DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_WR_OTHER_CABIN_NUMBER, &(DiagnosticSplit.OtherCabin.Number), DataStore->OffLine);
            if(DiagnosticSplit.OtherCabin.Number>0)
            {
                //Mando verso slave dati altro split
                Room[Address].OffLine = FuncWriteReg (Address, REG_SPLIT_OTHER_CABIN_ST_TEMPERATURE, Room[DiagnosticSplit.OtherCabin.Number].AirTemp, Room[Address].OffLine);
                Room[Address].OffLine = FuncWriteReg (Address, REG_SPLIT_OTHER_CABIN_ST_ON_OFF, Room[DiagnosticSplit.OtherCabin.Number].PowerOn, Room[Address].OffLine);
                Room[Address].OffLine = FuncWriteReg (Address, REG_SPLIT_OTHER_CABIN_ST_SETP, Room[DiagnosticSplit.OtherCabin.Number].SetPoint, Room[Address].OffLine);
                Room[Address].OffLine = FuncWriteReg (Address, REG_SPLIT_OTHER_CABIN_ST_FAN, Room[DiagnosticSplit.OtherCabin.Number].FanSpeed, Room[Address].OffLine);
                Room[Address].OffLine = FuncWriteReg (Address, REG_SPLIT_OTHER_CABIN_ONLY_FAN, Room[DiagnosticSplit.OtherCabin.Number].OnlyFan, Room[Address].OffLine);                                
		        if((Room[DiagnosticSplit.OtherCabin.Number].Model_FW_Version == 1) || (Room[DiagnosticSplit.OtherCabin.Number].Model_FW_Version == 2) || (Room[DiagnosticSplit.OtherCabin.Number].OnlyFan >= 4))  //Se sono UTA o se Only Fan Speciale (sfrutto registro UTA per funzioni ventilazione)
					Room[Address].OffLine = FuncWriteReg (Address, REG_SPLIT_OTHER_CABIN_ST_UTA_MODE, Room[DiagnosticSplit.OtherCabin.Number].UTAMode, Room[Address].OffLine);
                //guardo se è stato richiesto un send
                DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_OTHER_CABIN_CMD_SEND, &(DiagnosticSplit.OtherCabin.Send), DataStore->OffLine);
                if(DiagnosticSplit.OtherCabin.Send==1)
                {
                    DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_OTHER_CABIN_CMD_ON_OFF, &(DiagnosticSplit.OtherCabin.CMD_On_Off), DataStore->OffLine);
                    DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_OTHER_CABIN_CMD_SETP, &(DiagnosticSplit.OtherCabin.CMD_SetP), DataStore->OffLine);
                    DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_OTHER_CABIN_CMD_SETP_F, &(DiagnosticSplit.OtherCabin.CMD_SetP_F), DataStore->OffLine);
                    DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_OTHER_CABIN_CMD_FAN, &(DiagnosticSplit.OtherCabin.CMD_Fan), DataStore->OffLine);
			        if((Room[DiagnosticSplit.OtherCabin.Number].Model_FW_Version == 1) || (Room[DiagnosticSplit.OtherCabin.Number].Model_FW_Version == 2) || (Room[DiagnosticSplit.OtherCabin.Number].OnlyFan >= 4))  //Se sono UTA o se Only Fan Speciale (sfrutto registro UTA per funzioni ventilazione)
	                    DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_OTHER_CABIN_CMD_UTA_MODE, &(DiagnosticSplit.OtherCabin.UTAMode), DataStore->OffLine);
				}
            }
        }
        //Struttura per accendere automaticamente estrattore con altra unità
        //DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_EXTRACTOR_NUMBER, &(DataStore->Extractor.Number), DataStore->OffLine);
        if(DataStore->Extractor.Number!=0)
        {
            Room[DataStore->Extractor.Number].Extractor.Abil = 1;
            DataStore->OffLine = FuncReadReg (Address, REG_SPLIT_RD_DELTAVEL_EXTRACTOR, &(DataStore->Extractor.DeltaVel), DataStore->OffLine);
            temp = (DataStore->FanSpeed + DataStore->Extractor.DeltaVel) * (DataStore->Mode!=0);
            if(temp>8)
                temp = 8;
            if(temp>Room[DataStore->Extractor.Number].Extractor.AutoVel)
                Room[DataStore->Extractor.Number].Extractor.AutoVel = temp;
        }
	}
}

void RefreshInPutMasterRoom(TypInitSplit * Reinit, char Address)
{
		Room[Address].OffLine = FuncReadReg (Address, REG_SPLIT_TO_TRIGGER_INIT, &(Reinit->ToEngBox.TrigInit), Room[Address].OffLine);			
		if(Reinit->ToEngBox.TrigInit!=0 && !Room[Address].OffLine)
		{
			Room[Address].OffLine = FuncReadReg (Address, REG_SPLIT_TO_INIT_SP_ROOM, &(Reinit->ToEngBox.SetPoint), Room[Address].OffLine);		
			Room[Address].OffLine = FuncReadReg (Address, REG_SPLIT_TO_INIT_FAN_SPEED, &(Reinit->ToEngBox.FanSpeed), Room[Address].OffLine);		
			Room[Address].OffLine = FuncReadReg (Address, REG_SPLIT_TO_INIT_POWER, &(Reinit->ToEngBox.Power), Room[Address].OffLine);		
			Room[Address].OffLine = FuncReadReg (Address, REG_SPLIT_TO_INIT_SP_ROOM_F, &(Reinit->ToEngBox.SetPoint_F), Room[Address].OffLine);		
			//Room[Address].OffLine = FuncReadReg (Address, REG_SPLIT_TO_INIT_DEFMODE, &(Reinit->ToEngBox.DefMode), Room[Address].OffLine);		
			//Room[Address].OffLine = FuncReadReg (Address, REG_SPLIT_TO_TRIGGER_INIT, &(Reinit->ToEngBox.TrigInit), Room[Address].OffLine);			

			Reinit->FromEngBox.SetPoint = Reinit->ToEngBox.SetPoint;		
			Reinit->FromEngBox.FanSpeed = Reinit->ToEngBox.FanSpeed;		
			Reinit->FromEngBox.Power = Reinit->ToEngBox.Power;		
			Reinit->FromEngBox.SetPoint_F = Reinit->ToEngBox.SetPoint_F;		
			Reinit->FromEngBox.TrigInit = Reinit->ToEngBox.TrigInit;
		}			
		else
		{
			Reinit->FromEngBox.TrigInit = 0;
		}
#if(K_AbilBridge1==1 || K_AbilBridge2==1)	
		if(Bridge[1].Reinit.TrigInit!=0)
		{
			Reinit->FromEngBox.SetPoint = Bridge[1].Reinit.SetPoint;		
			Reinit->FromEngBox.FanSpeed = Bridge[1].Reinit.FanSpeed;		
			Reinit->FromEngBox.Power = Bridge[1].Reinit.Power;				
			Reinit->FromEngBox.TrigInit = Bridge[1].Reinit.TrigInit;
			
    #if(K_HiSpeedU1ModBUS==1)            
            Change_Protocol_BUSM(K_ModBus_Baud115200);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
    #else
            Change_Protocol_BUSM(K_ModBus_Baud57600);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
    #endif
			InsertReg(1, 0);						
			Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, REG_BRIDGE2_DIAG_ALL_SPLIT_WRITE_UPDATE, 1, MaxModReadWait, MaxModReadRetries);
		}
#endif
			//Dati in lettura generici da split n.1
			//Room[Address].OffLine = FuncReadReg (Address, REG_SPLIT_UTA_CLEAR_ERROR_BOX, &UTA_H.ClearErrorBox, Room[Address].OffLine);	
        Room[Address].OffLine = FuncReadReg (Address, REG_SPLIT_DOUBLE_PUMP_SWITCH_TIME, &EngineBox[0].SwitchPumpTime, Room[Address].OffLine);
        Room[Address].OffLine = FuncReadReg (Address, REG_SPLIT_DOUBLE_PUMP_SELECTION, &EngineBox[0].SwitchPumpSelection, Room[Address].OffLine);
        
        Room[Address].OffLine = FuncReadReg (Address, REG_SPLIT_MANUAL_PUMP_ABIL, &EngineBox[0].Manual_Pump_Abil, Room[Address].OffLine);
        Room[Address].OffLine = FuncReadReg (Address, REG_SPLIT_MANUAL_PUMP_FREQ, &EngineBox[0].Manual_Pump_Freq, Room[Address].OffLine);
        
}
// --------------------------------------------------------------------------------------------------------
// Legge l'indirizzo dello split da monitorare (dallo split master che lo ha letto dal touch)
// --------------------------------------------------------------------------------------------------------
void ReadDiagnosticSplitAddr(int addr)		// #v16
{
#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    
    
	Room[addr].OffLine = FuncReadReg (addr, REG_DIAG_SPLIT_TEST_ADDRESS, &(DiagnosticSplit.Address), Room[addr].OffLine);
	Room[addr].OffLine = FuncReadReg (addr, REG_DIAG_SPLIT_CMD_ADDRESS, &(DiagnosticSplit.CmdAddress), Room[addr].OffLine);

	Room[addr].OffLine = FuncReadReg (addr, REG_SPLIT_TOUCH_PAGE, &(DiagnosticSplit.Touch_Page), Room[addr].OffLine);	
}

// --------------------------------------------------------------------------------------------------------
// Legge l'indirizzo dello SubSplit da monitorare (dallo split master che lo ha letto dal touch)
// --------------------------------------------------------------------------------------------------------
void ReadDiagnosticSubSplitAddr(int addr)
{
#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    


	Room[addr].OffLine = FuncReadReg (addr, REG_SUB_SPLIT_TEST_ADDRESS, &(DiagnosticSplit.SubAddress), Room[addr].OffLine);
}

// --------------------------------------------------------------------------------------------------------
// Legge indirizzi vari da monitorare (dallo split master che lo ha letto dal touch)
// --------------------------------------------------------------------------------------------------------
void ReadDiagnosticInverterAddr(int addr)
{
#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    


	Room[addr].OffLine = FuncReadReg (addr, REG_DIAG_INVERTER_ADDRESS, &(Comp_Inverter[0].DiagAddress), Room[addr].OffLine);
	
}

// --------------------------------------------------------------------------------------------------------
// Legge i dati diagnostici dallo split selezionato sul touch dello split master (DiagnosticSplit.Address)
// --------------------------------------------------------------------------------------------------------
void RefreshDiagnosticSplit(void)		// #v16
{	
#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    

    
	if((DiagnosticSplit.Address > 0) && (DiagnosticSplit.Address <= Max_Room) && (DiagnosticSplit.SubAddress == 0) && (Room[DiagnosticSplit.Address].OnLine == 1)) //&& !Room[DiagnosticSplit.Address].OffLine) //&& (Touch.Page == 11))
	{
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_STATUS, &(DiagnosticSplit.SplitStatus), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_TEMP_BULBO_GRANDE, &(DiagnosticSplit.Temp_Gas), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_TEMP_BULBO_PICCOLO, &(DiagnosticSplit.Temp_Liq), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_TEMP_ARIA_USCITA, &(DiagnosticSplit.Temp_AirOut), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_TEMP_AMBIENTE, &(DiagnosticSplit.Temp_AirIn), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_FAN_TACHO, &(DiagnosticSplit.FanSpeed), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_VALVOLA_CUR_POS, &(DiagnosticSplit.Exp_Valve), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_VALVOLA_ERR_POS, &(DiagnosticSplit.Exp_Valve_Err), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_VALVOLA_ERR_COD, &(DiagnosticSplit.Exp_Valve_Err_Code), Room[DiagnosticSplit.Address].OffLine);		
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_PRES_BULBO_GRANDE, &(DiagnosticSplit.Gas_Pressure), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_PRES_BULBO_PICCOLO, &(DiagnosticSplit.Liq_Pressure), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_CURRENT_EVAP_TEMP, &(DiagnosticSplit.Req_AirTemp), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_CURRENT_EVAP_PRESURE, &(DiagnosticSplit.Req_Pressure), Room[DiagnosticSplit.Address].OffLine);
		//Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_STATUS_BOX_MOTORE, &(DiagnosticSplit.Split_Eng_Mode), Room[DiagnosticSplit.Address].OffLine);
		DiagnosticSplit.Split_Eng_Mode = EngineBox[0].Mode;
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_DIAG_SPLIT_CYCLE_TIME, &(DiagnosticSplit.CycleTime), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_QUANTITY, &(DiagnosticSplit.Sub_Slave_Quantity), Room[DiagnosticSplit.Address].OffLine);		
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_EVAP_TEMP_VALVE , &(DiagnosticSplit.EvapTempValve), Room[DiagnosticSplit.Address].OffLine);

		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_TEMP_AMB_MIDDLE , &(DiagnosticSplit.TempAmbMid), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_TEMP_AMB_REAL , &(DiagnosticSplit.TempAmbReal), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_FUNCTION_MODE , &(DiagnosticSplit.FunctionMode), Room[DiagnosticSplit.Address].OffLine);        
        
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_FLAGS_ENABLE_1_16 , &(DiagnosticSplit.Sub_Slave_Flags_Enable_1_16), Room[DiagnosticSplit.Address].OffLine);
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_FLAGS_OFFLINE_1_16 , &(DiagnosticSplit.Sub_Slave_Flags_Offline_1_16), Room[DiagnosticSplit.Address].OffLine);
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_FLAGS_ONLINE_1_16 , &(DiagnosticSplit.Sub_Slave_Flags_Online_1_16), Room[DiagnosticSplit.Address].OffLine);
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_FLAGS_ALARM_1_16 , &(DiagnosticSplit.Sub_Slave_Flags_Alarm_1_16), Room[DiagnosticSplit.Address].OffLine);

        
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_SETP_SUPERHEAT , &(DiagnosticSplit.SetP_SuperHeat), Room[DiagnosticSplit.Address].OffLine);		
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_SUPERHEAT , &(DiagnosticSplit.Superheat), Room[DiagnosticSplit.Address].OffLine);
		//Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_SUPERHEAT_RISCALDAMENTO , &(DiagnosticSplit.SuperheatRiscaldamento), Room[DiagnosticSplit.Address].OffLine);		

		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_UTA_ABIL, &(DiagnosticSplit.UTA_Abil), Room[DiagnosticSplit.Address].OffLine);	

		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_RD_FROZEN_MODE, &(DiagnosticSplit.FrozenMode), Room[DiagnosticSplit.Address].OffLine);	
        
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_ONLY_FAN, &(DiagnosticSplit.OnlyFan), Room[DiagnosticSplit.Address].OffLine);    
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_SET_CMD_DISABLE, &(DiagnosticSplit.Set_CMD_Disable), Room[DiagnosticSplit.Address].OffLine);	
		
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_FAN_POWER, &(DiagnosticSplit.Fan_Power), Room[DiagnosticSplit.Address].OffLine);	
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_FAN_CURRENT, &(DiagnosticSplit.Fan_Current), Room[DiagnosticSplit.Address].OffLine);	
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_SUPPLY_VOLTAGE, &(DiagnosticSplit.Supply_Voltage), Room[DiagnosticSplit.Address].OffLine);	

        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_HW_VERSION, &(DiagnosticSplit.HW_Version), Room[DiagnosticSplit.Address].OffLine);
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_FW_VERSION_HW, &(DiagnosticSplit.FW_Version_HW), Room[DiagnosticSplit.Address].OffLine);
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_FW_VERSION_FW, &(DiagnosticSplit.FW_Version_FW), Room[DiagnosticSplit.Address].OffLine);
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_FW_REVISION_FW, &(DiagnosticSplit.FW_Revision_FW), Room[DiagnosticSplit.Address].OffLine);        
               
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_RD_MAX_VENTIL_MODE, &(DiagnosticSplit.MaxVentilMode), Room[DiagnosticSplit.Address].OffLine);        

        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_DIAG_SPLIT_RD_MIN_SET_POINT, &(DiagnosticSplit.MinSetPoint), Room[DiagnosticSplit.Address].OffLine);                
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_DIAG_SPLIT_RD_MAX_SET_POINT, &(DiagnosticSplit.MaxSetPoint), Room[DiagnosticSplit.Address].OffLine);                
        
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_SUPERH_COOL_PID_ERROR, &(DiagnosticSplit.SuperHeat_Pid_Cool_Error), Room[DiagnosticSplit.Address].OffLine);        

        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_HEATER_ABIL, &(DiagnosticSplit.Heater_Abil), Room[DiagnosticSplit.Address].OffLine);

        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_RD_TEMPERATURE_PROBE_ERROR, &(DiagnosticSplit.Temperature_Probe_Error), Room[DiagnosticSplit.Address].OffLine);        
        

        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_READ_HUMIDITY, &(DiagnosticSplit.Humidity), Room[DiagnosticSplit.Address].OffLine);        

        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_MODEL_VERSION, &(DiagnosticSplit.Model_FW_Version), Room[DiagnosticSplit.Address].OffLine);        

        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_FAN_M3H, &(DiagnosticSplit.Fan_M3h), Room[DiagnosticSplit.Address].OffLine);        

        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_AIR_POWER_OUTPUT, &(DiagnosticSplit.AirPowerOutput), Room[DiagnosticSplit.Address].OffLine);
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_AIR_POWER_BTU, &(DiagnosticSplit.AirPowerBTU), Room[DiagnosticSplit.Address].OffLine);

        
        if(DiagnosticSplit.Touch_Page==62)
        {
            Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_CNTCOMERRSEC, &(DiagnosticSplit.ComError.CntComErrSec), Room[DiagnosticSplit.Address].OffLine);
            Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_TIMEOUTCOMERRSEC, &(DiagnosticSplit.ComError.TimeOutComErrSec), Room[DiagnosticSplit.Address].OffLine);
            Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_TIMEOUTINTERCHARCOMERRSEC, &(DiagnosticSplit.ComError.TimeOutInterCharComErrSec), Room[DiagnosticSplit.Address].OffLine);
            Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_CRC_COMERRSEC, &(DiagnosticSplit.ComError.CRC_ComErrSec), Room[DiagnosticSplit.Address].OffLine);
            Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_MODCNTCOMERRSEC, &(DiagnosticSplit.ComError.ModCntComErrSec), Room[DiagnosticSplit.Address].OffLine);
            Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_MODTIMEOUTCOMERRSEC, &(DiagnosticSplit.ComError.ModTimeOutComErrSec), Room[DiagnosticSplit.Address].OffLine);
            Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_MODTIMEOUTINTERCHARCOMERRSEC, &(DiagnosticSplit.ComError.ModTimeOutInterCharComErrSec), Room[DiagnosticSplit.Address].OffLine);
            Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_MODCRC_COMERRSEC, &(DiagnosticSplit.ComError.ModCRC_ComErrSec), Room[DiagnosticSplit.Address].OffLine);            
        }
	}
	
/*
	if((DiagnosticSplit.Address > 0) && (DiagnosticSplit.Address <27))
	{
		FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_STATUS, &(DiagnosticSplit.SplitStatus), 0);
		FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_TEMP_BULBO_GRANDE, &(DiagnosticSplit.Temp_Gas), 0);
		FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_TEMP_BULBO_PICCOLO, &(DiagnosticSplit.Temp_Liq), 0);
		FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_TEMP_ARIA_USCITA, &(DiagnosticSplit.Temp_AirOut), 0);
		FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_TEMP_AMBIENTE, &(DiagnosticSplit.Temp_AirIn), 0);
		FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_FAN_TACHO, &(DiagnosticSplit.FanSpeed), 0);
		FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_VALVOLA_CUR_POS, &(DiagnosticSplit.Exp_Valve), 0);
		FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_PRES_BULBO_GRANDE, &(DiagnosticSplit.Gas_Pressure), 0);
		FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_PRES_BULBO_PICCOLO, &(DiagnosticSplit.Liq_Pressure), 0);
		FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_CURRENT_EVAP_TEMP, &(DiagnosticSplit.Req_AirTemp), 0);
		FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_CURRENT_EVAP_PRESURE, &(DiagnosticSplit.Req_Pressure), 0);
		FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_STATUS_BOX_MOTORE, &(DiagnosticSplit.Split_Eng_Mode), 0);
///		FuncReadReg (DiagnosticSplit.Address, REG_DIAG_SPLIT_CYCLE_TIME, &(DiagnosticSplit.CycleTime), 0);
	}
*/
}


// --------------------------------------------------------------------------------------------------------
// Invia i dati diagnostici dello slave da monitorare sullo split master
// --------------------------------------------------------------------------------------------------------
void SendDiagnosticSplitData(int addr)		// #v16
{
#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    

    
	if(Room[addr].OnLine) 
	{
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_STATUS, DiagnosticSplit.SplitStatus, Room[addr].OffLine);					// REG_SPLIT_STATUS
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_GAS_TEMP, DiagnosticSplit.Temp_Gas, Room[addr].OffLine);				// REG_SPLIT_TEMP_BULBO_GRANDE
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_LIQ_TEMP, DiagnosticSplit.Temp_Liq, Room[addr].OffLine);				// REG_SPLIT_TEMP_BULBO_PICCOLO 
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_AIR_OUT, DiagnosticSplit.Temp_AirOut, Room[addr].OffLine);			// REG_SPLIT_TEMP_ARIA_USCITA
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_AIR_IN, DiagnosticSplit.Temp_AirIn, Room[addr].OffLine);				// REG_SPLIT_TEMP_AMBIENTE
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_FAN_SPEED, DiagnosticSplit.FanSpeed, Room[addr].OffLine);				// REG_SPLIT_FAN_TACHO
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_EXP_VALVE, DiagnosticSplit.Exp_Valve, Room[addr].OffLine);			// REG_SPLIT_VALVOLA_CUR_POS
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_EXP_VALVE_ERR_POS, DiagnosticSplit.Exp_Valve_Err, Room[addr].OffLine);			// REG_SPLIT_VALVOLA_CUR_POS
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_EXP_VALVE_ERR_CODE, DiagnosticSplit.Exp_Valve_Err_Code, Room[addr].OffLine);			// REG_SPLIT_VALVOLA_CUR_POS        
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_GAS_PRESSURE, DiagnosticSplit.Gas_Pressure, Room[addr].OffLine);		// REG_SPLIT_PRES_BULBO_GRANDE
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_LIQ_PRESSURE, DiagnosticSplit.Liq_Pressure, Room[addr].OffLine);		// REG_SPLIT_PRES_BULBO_PICCOLO 
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_REQUEST_AIR_TEMP, DiagnosticSplit.Req_AirTemp, Room[addr].OffLine);	// REG_SPLIT_CURRENT_EVAP_TEMP
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_REQUEST_AIR_PRES, DiagnosticSplit.Req_Pressure, Room[addr].OffLine);	// REG_SPLIT_CURRENT_EVAP_PRESURE
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_ENG_MODE, DiagnosticSplit.Split_Eng_Mode, Room[addr].OffLine);		// REG_SPLIT_STATUS_BOX_MOTORE	
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_CYCLE_TIME, DiagnosticSplit.CycleTime, Room[addr].OffLine);			// REG_DIAG_SPLIT_CYCLE_TIME	

		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SUB_SPLIT_QUANTITY, DiagnosticSplit.Sub_Slave_Quantity, Room[addr].OffLine);// REG_DIAG_SPLIT_CYCLE_TIME	
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_EVAP_TEMP_VALVE, DiagnosticSplit.EvapTempValve, Room[addr].OffLine);		// REG_DIAG_EVAP_TEMP_VALVE
        
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_TEMP_AMB_MIDDLE, DiagnosticSplit.TempAmbMid, Room[addr].OffLine);		// REG_DIAG_SPLIT_TEMP_AMB_MIDDLE
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_TEMP_AMB_REAL, DiagnosticSplit.TempAmbReal, Room[addr].OffLine);		// REG_DIAG_SPLIT_TEMP_AMB_REAL
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_FUNCTION_MODE, DiagnosticSplit.FunctionMode, Room[addr].OffLine);		// REG_DIAG_SPLIT_TEMP_AMB_REAL        
        
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SUB_SPLIT_FLAGS_ENABLE_1_16, DiagnosticSplit.Sub_Slave_Flags_Enable_1_16, Room[addr].OffLine);
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SUB_SPLIT_FLAGS_OFFLINE_1_16, DiagnosticSplit.Sub_Slave_Flags_Offline_1_16, Room[addr].OffLine);
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SUB_SPLIT_FLAGS_ONLINE_1_16, DiagnosticSplit.Sub_Slave_Flags_Online_1_16, Room[addr].OffLine);
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SUB_SPLIT_FLAGS_ALARM_1_16, DiagnosticSplit.Sub_Slave_Flags_Alarm_1_16, Room[addr].OffLine);
        
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_SUPERHEAT, DiagnosticSplit.Superheat, Room[addr].OffLine);
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_SUPERHEAT_RISCALDAMENTO, EngineBox[0].SuperHeat.Value/*DiagnosticSplit.SuperheatRiscaldamento*/, Room[addr].OffLine);
        if(EngineBox[0].DefMode==CoreRiscaldamento)
            Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_SETP_SUPERHEAT, SuperHeatPID.SetP/*DiagnosticSplit.SetP_SuperHeat*/, Room[addr].OffLine);
        else
            Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_SETP_SUPERHEAT, DiagnosticSplit.SetP_SuperHeat, Room[addr].OffLine);
        
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_UTA_ABIL, DiagnosticSplit.UTA_Abil, Room[addr].OffLine);
        
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_FROZEN_MODE, DiagnosticSplit.FrozenMode, Room[addr].OffLine);
        
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_FAN_POWER, DiagnosticSplit.Fan_Power, Room[addr].OffLine);		
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_FAN_CURRENT, DiagnosticSplit.Fan_Current, Room[addr].OffLine);		
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_SUPPLY_VOLTAGE, DiagnosticSplit.Supply_Voltage, Room[addr].OffLine);		

		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_HW_VERSION, DiagnosticSplit.HW_Version, Room[addr].OffLine);		
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_FW_VERSION_HW, DiagnosticSplit.FW_Version_HW, Room[addr].OffLine);		
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_FW_VERSION_FW, DiagnosticSplit.FW_Version_FW, Room[addr].OffLine);		
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_FW_REVISION_FW, DiagnosticSplit.FW_Revision_FW, Room[addr].OffLine);	

        Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_ONLY_FAN, DiagnosticSplit.OnlyFan, Room[addr].OffLine);	
        Room[addr].OffLine = FuncWriteReg (addr, REG_SPLIT_DIAG_SET_CMD_DISABLE, DiagnosticSplit.Set_CMD_Disable, Room[addr].OffLine);	
        
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_WR_MAX_VENTIL_MODE, DiagnosticSplit.MaxVentilMode, Room[addr].OffLine);        

		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_WR_MIN_SET_POINT, DiagnosticSplit.MinSetPoint, Room[addr].OffLine);        
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_WR_MAX_SET_POINT, DiagnosticSplit.MaxSetPoint, Room[addr].OffLine);        

		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_SUPERH_COOL_PID_ERROR, DiagnosticSplit.SuperHeat_Pid_Cool_Error, Room[addr].OffLine);
        
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_HEATER_ABIL, DiagnosticSplit.Heater_Abil, Room[addr].OffLine);

		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_TEMPERATURE_PROBE_ERROR, DiagnosticSplit.Temperature_Probe_Error, Room[addr].OffLine);        
        
        Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_WR_HUMIDITY, DiagnosticSplit.Humidity, Room[addr].OffLine);        

		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_MODEL_VERSION, DiagnosticSplit.Model_FW_Version, Room[addr].OffLine);

		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_FAN_M3H, DiagnosticSplit.Fan_M3h, Room[addr].OffLine);
        
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_AIR_POWER_OUTPUT, DiagnosticSplit.AirPowerOutput, Room[addr].OffLine);
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_AIR_POWER_BTU, DiagnosticSplit.AirPowerBTU, Room[addr].OffLine);        

        if(DiagnosticSplit.Touch_Page==62)
        {
            Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_CNTCOMERRSEC, DiagnosticSplit.ComError.CntComErrSec, Room[addr].OffLine);                    
            Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_TIMEOUTCOMERRSEC, DiagnosticSplit.ComError.TimeOutComErrSec, Room[addr].OffLine);                    
            Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_TIMEOUTINTERCHARCOMERRSEC, DiagnosticSplit.ComError.TimeOutInterCharComErrSec, Room[addr].OffLine);                    
            Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_CRC_COMERRSEC, DiagnosticSplit.ComError.CRC_ComErrSec, Room[addr].OffLine);                    
            Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_MODCNTCOMERRSEC, DiagnosticSplit.ComError.ModCntComErrSec, Room[addr].OffLine);                    
            Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_MODTIMEOUTCOMERRSEC, DiagnosticSplit.ComError.ModTimeOutComErrSec, Room[addr].OffLine);                    
            Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_MODTIMEOUTINTERCHARCOMERRSEC, DiagnosticSplit.ComError.ModTimeOutInterCharComErrSec, Room[addr].OffLine);                    
            Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_SPLIT_MODCRC_COMERRSEC, DiagnosticSplit.ComError.ModCRC_ComErrSec, Room[addr].OffLine);                    
        }
	}
}



// --------------------------------------------------------------------------------------------------------
// Legge i dati diagnostici dallo Subsplit selezionato sul touch dello split master (DiagnosticSubSplit.Address)
// --------------------------------------------------------------------------------------------------------
void RefreshDiagnosticSubSplit(void)		
{
#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    

		
	if((DiagnosticSplit.Address > 0) && (DiagnosticSplit.Address <= Max_Room) && (DiagnosticSplit.SubAddress != 0) && (Room[DiagnosticSplit.Address].OnLine == 1))
	{
		Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_STATUS, &(DiagnosticSplit.SplitStatus), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_GAS_TEMP, &(DiagnosticSplit.Temp_Gas), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_LIQ_TEMP, &(DiagnosticSplit.Temp_Liq), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_AIR_OUT, &(DiagnosticSplit.Temp_AirOut), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_AIR_IN, &(DiagnosticSplit.Temp_AirIn), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_FAN_SPEED, &(DiagnosticSplit.FanSpeed), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_EXP_VALVE, &(DiagnosticSplit.Exp_Valve), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_EXP_VALVE_ERR_POS, &(DiagnosticSplit.Exp_Valve_Err), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_EXP_VALVE_ERR_COD, &(DiagnosticSplit.Exp_Valve_Err_Code), Room[DiagnosticSplit.Address].OffLine);		
        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_GAS_PRESSURE, &(DiagnosticSplit.Gas_Pressure), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_LIQ_PRESSURE, &(DiagnosticSplit.Liq_Pressure), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_REQUEST_AIR_TEMP, &(DiagnosticSplit.Req_AirTemp), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_REQUEST_AIR_PRES, &(DiagnosticSplit.Req_Pressure), Room[DiagnosticSplit.Address].OffLine);
		//Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_STATUS_BOX_MOTORE, &(DiagnosticSplit.Split_Eng_Mode), Room[DiagnosticSplit.Address].OffLine);
		DiagnosticSplit.Split_Eng_Mode = EngineBox[0].Mode;
		Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_CYCLE_TIME, &(DiagnosticSplit.CycleTime), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_QUANTITY, &(DiagnosticSplit.Sub_Slave_Quantity), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_EVAP_TEMP_VALVE , &(DiagnosticSplit.EvapTempValve), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_TEMP_AMB_MIDDLE , &(DiagnosticSplit.TempAmbMid), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_TEMP_AMB_REAL , &(DiagnosticSplit.TempAmbReal), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_FUNCTION_MODE , &(DiagnosticSplit.FunctionMode), Room[DiagnosticSplit.Address].OffLine);
   
        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_FLAGS_ENABLE_1_16 , &(DiagnosticSplit.Sub_Slave_Flags_Enable_1_16), Room[DiagnosticSplit.Address].OffLine);
        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_FLAGS_OFFLINE_1_16 , &(DiagnosticSplit.Sub_Slave_Flags_Offline_1_16), Room[DiagnosticSplit.Address].OffLine);
        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_FLAGS_ONLINE_1_16 , &(DiagnosticSplit.Sub_Slave_Flags_Online_1_16), Room[DiagnosticSplit.Address].OffLine);
        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_FLAGS_ALARM_1_16 , &(DiagnosticSplit.Sub_Slave_Flags_Alarm_1_16), Room[DiagnosticSplit.Address].OffLine);

        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_SETP_SUPERHEAT , &(DiagnosticSplit.SetP_SuperHeat), Room[DiagnosticSplit.Address].OffLine);
		Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_SUPERHEAT , &(DiagnosticSplit.Superheat), Room[DiagnosticSplit.Address].OffLine);
		//Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_SUPERHEAT_RISCALDAMENTO , &(DiagnosticSplit.SuperheatRiscaldamento), Room[DiagnosticSplit.Address].OffLine);		

        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_UTA_ABIL, &(DiagnosticSplit.UTA_Abil), Room[DiagnosticSplit.Address].OffLine);		

        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_ONLY_FAN, &(DiagnosticSplit.OnlyFan), Room[DiagnosticSplit.Address].OffLine);	
        
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_FAN_POWER, &(DiagnosticSplit.Fan_Power), Room[DiagnosticSplit.Address].OffLine);	
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_FAN_CURRENT, &(DiagnosticSplit.Fan_Current), Room[DiagnosticSplit.Address].OffLine);	
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_SUPPLY_VOLTAGE, &(DiagnosticSplit.Supply_Voltage), Room[DiagnosticSplit.Address].OffLine);	
        
        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_HW_VERSION, &(DiagnosticSplit.HW_Version), Room[DiagnosticSplit.Address].OffLine);
        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_FW_VERSION_HW, &(DiagnosticSplit.FW_Version_HW), Room[DiagnosticSplit.Address].OffLine);
        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_FW_VERSION_FW, &(DiagnosticSplit.FW_Version_FW), Room[DiagnosticSplit.Address].OffLine);
        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_FW_REVISION_FW, &(DiagnosticSplit.FW_Revision_FW), Room[DiagnosticSplit.Address].OffLine);        

        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_RD_MAX_VENTIL_MODE, &(DiagnosticSplit.MaxVentilMode), Room[DiagnosticSplit.Address].OffLine);                

        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_DIAG_SPLIT_RD_MIN_SET_POINT, &(DiagnosticSplit.MinSetPoint), Room[DiagnosticSplit.Address].OffLine);                
        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_DIAG_SPLIT_RD_MAX_SET_POINT, &(DiagnosticSplit.MaxSetPoint), Room[DiagnosticSplit.Address].OffLine);                
        
        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_SUPERH_COOL_PID_ERROR, &(DiagnosticSplit.SuperHeat_Pid_Cool_Error), Room[DiagnosticSplit.Address].OffLine);                
    
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_HEATER_ABIL, &(DiagnosticSplit.Heater_Abil), Room[DiagnosticSplit.Address].OffLine);
        
        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_RD_TEMPERATURE_PROBE_ERROR, &(DiagnosticSplit.Temperature_Probe_Error), Room[DiagnosticSplit.Address].OffLine);        
        
        Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_READ_HUMIDITY, &(DiagnosticSplit.Humidity), Room[DiagnosticSplit.Address].OffLine);        

        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_MODEL_VERSION, &(DiagnosticSplit.Model_FW_Version), Room[DiagnosticSplit.Address].OffLine);      

        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_FAN_M3H, &(DiagnosticSplit.Fan_M3h), Room[DiagnosticSplit.Address].OffLine);      

        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_AIR_POWER_OUTPUT, &(DiagnosticSplit.AirPowerOutput), Room[DiagnosticSplit.Address].OffLine);      
        Room[DiagnosticSplit.Address].OffLine = FuncReadReg (DiagnosticSplit.Address, REG_SUB_SPLIT_AIR_POWER_BTU, &(DiagnosticSplit.AirPowerBTU), Room[DiagnosticSplit.Address].OffLine);      
        
        if(DiagnosticSplit.Touch_Page==62)
        {
            Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_CNTCOMERRSEC, &(DiagnosticSplit.ComError.CntComErrSec), Room[DiagnosticSplit.Address].OffLine);
            Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_TIMEOUTCOMERRSEC, &(DiagnosticSplit.ComError.TimeOutComErrSec), Room[DiagnosticSplit.Address].OffLine);
            Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_TIMEOUTINTERCHARCOMERRSEC, &(DiagnosticSplit.ComError.TimeOutInterCharComErrSec), Room[DiagnosticSplit.Address].OffLine);
            Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_CRC_COMERRSEC, &(DiagnosticSplit.ComError.CRC_ComErrSec), Room[DiagnosticSplit.Address].OffLine);
            Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_MODCNTCOMERRSEC, &(DiagnosticSplit.ComError.ModCntComErrSec), Room[DiagnosticSplit.Address].OffLine);
            Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_MODTIMEOUTCOMERRSEC, &(DiagnosticSplit.ComError.ModTimeOutComErrSec), Room[DiagnosticSplit.Address].OffLine);
            Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_MODTIMEOUTINTERCHARCOMERRSEC, &(DiagnosticSplit.ComError.ModTimeOutInterCharComErrSec), Room[DiagnosticSplit.Address].OffLine);
            Room[DiagnosticSplit.Address].OffLine =  FuncReadReg (DiagnosticSplit.Address, REG_SPLIT_MODCRC_COMERRSEC, &(DiagnosticSplit.ComError.ModCRC_ComErrSec), Room[DiagnosticSplit.Address].OffLine);            
        }
	}	
}




// --------------------------------------------------------------------------------------------------------
// Invia i dati diagnostici dell' inverter da monitorare sullo split master
// --------------------------------------------------------------------------------------------------------
void SendDiagnosticInverterData(int addr, int numInverter)
{
#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    

    
	if(!Room[addr].OffLine) 
	{
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_INVERTER_ALARM, Comp_Inverter[numInverter-1].AlarmReg, Room[addr].OffLine);					// REG_INVERTER_ALARM
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_INVERTER_OUT_FREQ, Comp_Inverter[numInverter-1].Out_Freq, Room[addr].OffLine);				// REG_INVERTER_OUT_FREQ
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_INVERTER_STATUS, Comp_Inverter[numInverter-1].StatusReg, Room[addr].OffLine);				// REG_INVERTER_STATUS 
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_INVERTER_OUT_CURRENT, Comp_Inverter[numInverter-1].Out_Current, Room[addr].OffLine);		// REG_INVERTER_OUT_CURRENT
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_INVERTER_IN_VOLTAGE, Comp_Inverter[numInverter-1].In_Voltage, Room[addr].OffLine);			// REG_INVERTER_IN_VOLTAGE
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_INVERTER_OUT_VOLTAGE, Comp_Inverter[numInverter-1].Out_Voltage, Room[addr].OffLine);		// REG_INVERTER_OUT_VOLTAGE
		//Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_INVERTER_IN_POWER, Comp_Inverter[numInverter-1].In_Power, Room[addr].OffLine);				// REG_INVERTER_INPUT_POWER
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_INVERTER_OUT_POWER, Comp_Inverter[numInverter-1].Out_Power, Room[addr].OffLine);			// REG_INVERTER_OUTPUT_POWER
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_INVERTER_REQ_FREQ, Comp_Inverter[numInverter-1].Req_Freq, Room[addr].OffLine);				// EngineBox[0].Out_Inverter
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_INVERTER_MAX_REQ_FREQ, Comp_Inverter[numInverter-1].Max_Req_Freq, Room[addr].OffLine);		// EngineBox[0].Inverter_Max_Power_Out
		Room[addr].OffLine = FuncWriteReg (addr, REG_DIAG_INVERTER_POWERINV, Comp_Inverter[numInverter-1].Power_Inverter, Room[addr].OffLine);				
	}

}


void RefreshAllModeRoom(void)
{
    static int RoomMode = CoreRiscaldamento;
    
#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    
  

    //FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_DEFINE_MODE, Room[k_Split_Master_Add].DefMode);	// Invio sul BUS in broadcast la Modalità di funzionamento letta dalla funzione "RefreshInPutRoom" dello Split con indirizzo 1 (che per convenzione in questo caso è l'unico che ha il Touch e decide Riscaldamento/Raffrescamento)
#ifdef K_MasterAutoRoom    
    if(Room[K_MasterAutoRoom].AirTemp > (Room[K_MasterAutoRoom].SetPoint + K_MasterAutoIst))
        RoomMode = CoreRaffrescamento;
    if(Room[K_MasterAutoRoom].AirTemp < (Room[K_MasterAutoRoom].SetPoint - K_MasterAutoIst))
        RoomMode = CoreRiscaldamento; 

    if(Room[k_Split_Master_Add].DefMode==CoreAuto)
    {        
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_DEFINE_MODE, RoomMode);
        EngineBox[0].DefMode = RoomMode;
    }
    else
#endif
    {
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_DEFINE_MODE, Room[k_Split_Master_Add].DefMode);
        EngineBox[0].DefMode = Room[k_Split_Master_Add].DefMode;
    }
}

void RefreshAllInPut(void)
{	
	char i=1;
    static int Scheduler = 1;

#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    
  
    
	EngineBox[0].In.ClearErrorReq =0;               //Azzero prima di rileggere stati
    EngineBox[0].In.ClearComErrorReq =0;
	EngineBox[0].PersErr.EEV_Open =0;
	EngineBox[0].PersErr.EEV_Close =0;
	EngineBox[0].PersErr.BatteriaGhiacciata =0;
	EngineBox[0].PersErr.Fan_Speed =0;
	EngineBox[0].PersErr.Fan_OverLoad =0;
	//EngineBox[0].Test.EEV_End =1;
    EngineBox[0].PersErr.FireAlarm =0;  
    EngineBox[0].Error.SPTemperatureFault =0;
    EngineBox[0].AddressGasLeaks =0;
 
    
	RefreshInPutMasterRoom(&Reinit, 1);
    
    for(i=1; i<Max_Room; i++)
    {
        RefreshInPutRoom(&Room[i], i, Scheduler);
        if(Scheduler == i && Room[i].OnLine==0)
            Scheduler++;
    }
    
    if(++Scheduler>=Max_Room)
        Scheduler=1;
 
    
	//Comp_Inverter[0].Max_Power_Inverter = (unsigned int)(((float)Room[k_Split_Master_Add].PowerLimit * (float)k_taglia_inverter)/255.0);		// Memorizzo limite potenza proveniente da scheda slave master (addr=1) //DA SISTEMARE POWER LIMIT!!!!!!!
	//Comp_Inverter[0].Max_Power_Inverter = 400;
}

void RefreshOutPutRoom(char Address)
{
        if(Room[Address].Extractor.Abil==1)
        {
            Room[Address].OffLine = FuncWriteReg (Address, REG_SPLIT_WR_EXTRACTOR_ABIL, Room[Address].Extractor.Abil, Room[Address].OffLine);
            Room[Address].OffLine = FuncWriteReg (Address, REG_SPLIT_WR_EXTRACTOR_AUTOVEL, Room[Address].Extractor.AutoVel, Room[Address].OffLine);
            Room[Address].Extractor.AutoVel=0;
        }
}

void RefreshOutPutEngineBox(void)		//TypEngineBox * DataStore, char Address)
{    
    static int Scheduler = 1;    
    unsigned int temp;
    
#if (K_AbilCompressor2==0)		// K_AbilCompressor2 = 1 -> Compressore doppio	
	StatoRele =
	(EngineBox[0].Out.PersErr               << 0) |
	(EngineBox[0].Out.Valvola_Caldo			<< 1) |
	(EngineBox[0].Out.Pompa_Acqua			<< 2) |
#if (K_AbilRechargeOil==0)
    (EngineBox[0].Out.Pompa_Acqua2			<< 3);
#else
	(EngineBox[0].Out.Pompa_Acqua2			<< 3) |
	(EngineBox[0].Out.RechargeOil			<< 4);
#endif
#else	
	StatoRele =
#if(K_OnlyOneInverterComp2==1)
    (EngineBox[1].SelectCompressor			<< 0) |
#elif (K_Abil_Double_Pump==1)			
    (EngineBox[0].Out.Pompa_Acqua2			<< 0) |
#else
	(EngineBox[0].Out.PersErr				<< 0) |
#endif        
	(EngineBox[0].Out.Valvola_Caldo			<< 1) |
	(EngineBox[0].Out.Pompa_Acqua			<< 2) |
	(EngineBox[0].Out.RechargeOil			<< 3) |
	(EngineBox[1].Out.RechargeOil			<< 4); 
#if(K_OnlyOneInverterComp2==1 || K_Abil_Double_Pump==1)
    DAC_4 = 130*EngineBox[0].Out.PersErr;   //per relè finder bobina 6V cod. 34.51.7.005.0010
#endif
#endif	
	
	
	Comp_Inverter[0].Power_Inverter = K_Power_InverterC1;			// Potenza Taglia Inverter compressore 1		Watt
    EngineBox[0].MaxInverterAbsorption = Comp_Inverter[0].Power_Inverter/10;    //Potenza trattata per limitazione singolo compressore
	Comp_Inverter[1].Power_Inverter = K_Power_InverterC2;			// Potenza Taglia Inverter compressore 2		Watt
    EngineBox[1].MaxInverterAbsorption = Comp_Inverter[1].Power_Inverter/10;    //Potenza trattata per limitazione singolo compressore
	Comp_Inverter[2].Power_Inverter = K_Power_InverterPump;		// Potenza Taglia Inverter Pompa				Watt
	Comp_Inverter[0].PowerMax_Available = K_PowerMax_Available;		// Valore limitazione Potenza totale			Watt
	Comp_Inverter[0].Power_EcoMode = Room[k_Split_Master_Add].Power_EcoMode;	// Valore di limitazione Potenza in Eco Mode	Watt	
#if(K_Ing1_ModeSelection==2)
	Comp_Inverter[0].EcoModeEnable = Room[k_Split_Master_Add].EcoModeEnable || iDigiIn1;
#else
	Comp_Inverter[0].EcoModeEnable = Room[k_Split_Master_Add].EcoModeEnable;
#endif	
	// Era in "RefreshAllInPut()"
    if(Room[k_Split_Master_Add].OnLine)
    {
        Comp_Inverter[0].Max_Power_Inverter = (unsigned int)(((float)Room[k_Split_Master_Add].PowerLimit * ((float)Comp_Inverter[0].PowerMax_Available)/2550.0));

        if(Comp_Inverter[0].EcoModeEnable)
            Comp_Inverter[0].Max_Power_Inverter = Comp_Inverter[0].Power_EcoMode;	
    }
	
	
	StatoIngressi = (EngineBox[0].In.Flow	<< 0);
	
	DAC_1 = EngineBox[0].Out_Inverter;

	//DEBUGGONE
	DAC_2 = EngineBox[1].Out_Inverter;			// 02/05/2016 - Uscita 0..10V Comp2 su DAC2
#if(K_AbilPumpInverter==1 || K_AbilPumpDAC==1)
	DAC_3 = FreqInverterPump2DAC(EngineBox[0].FreqPompa_Acqua);			// 02/05/2016 - Uscita 0..10V Pompa su DAC3
#endif
	
	//DEBUGGONE
				
#if(K_AbilPumpInverter==1 || K_AbilPumpDAC==1)	
	EngineBox[0].FreqPompa_Acqua_Perc = ValueToPercent(EngineBox[0].FreqPompa_Acqua, K_LIM_MAX_FREQ_INV_PUMP);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_WATER_PUMP_SPEED_PERC, EngineBox[0].FreqPompa_Acqua_Perc);	
#else
#if (K_Abil_Double_Pump==1)			
    EngineBox[0].FreqPompa_Acqua_Perc = 100*(EngineBox[0].Out.Pompa_Acqua||EngineBox[0].Out.Pompa_Acqua2);
#else
	EngineBox[0].FreqPompa_Acqua_Perc = 100*EngineBox[0].Out.Pompa_Acqua;
#endif
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_WATER_PUMP_SPEED_PERC, EngineBox[0].FreqPompa_Acqua_Perc);	
	
#endif
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_COMPRESSOR_SPEED, EngineBox[0].Out_Inverter);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_WATER_PUMP_SPEED, EngineBox[0].FreqPompa_Acqua);
	//FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_FLUX, EngineBox[0].);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_CURRENT_MODE_ENGINE_BOX, EngineBox[0].Mode);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_CURRENT_REQ_PRESSURE, EngineBox[0].Ric_Pressione);              // Master                               Slave
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_T_COMP_OUT, EngineBox[0].Temperature.Compressor_Output);     // REG_SPLIT_ENGINE_BOX_T_COMP_OUT       REG_SPLIT_ENGINE_BOX_TEMP_MANDATA
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_T_COMP_SUCTION, EngineBox[0].Temperature.Compressor_Suction);// REG_SPLIT_ENGINE_BOX_T_COMP_SUCTION       REG_SPLIT_ENGINE_BOX_TEMP_RITORNO
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_T_SEA_WATER, EngineBox[0].Temperature.Sea_Water);           // REG_SPLIT_ENGINE_BOX_T_SEA_WATER    REG_SPLIT_ENGINE_BOX_TEMP_ACQUA_MARE    
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_T_CONDENSER, EngineBox[0].Temperature.Condenser);           // REG_SPLIT_ENGINE_BOX_T_CONDENSER
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_T_GAS, EngineBox[0].Temperature.Gas);                       // REG_SPLIT_ENGINE_BOX_T_GAS
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_T_LIQUID, EngineBox[0].Temperature.Liquid);                 // REG_SPLIT_ENGINE_BOX_T_LIQUID
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_P_GAS, EngineBox[0].Pressure.Gas);                   // REG_SPLIT_ENGINE_BOX_P_GAS           REG_SPLIT_ENGINE_BOX_PRESSURE_BIG
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_P_LIQUID, EngineBox[0].Pressure.Liquid);             // REG_SPLIT_ENGINE_BOX_P_LIQUID
	//FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_P_VRV_BOX, EngineBox[0].);
	//FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_P_AUX, EngineBox[0].);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_RELE_SATUS, StatoRele);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_INPUT_STATUS, StatoIngressi);
	//FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_SYS_ID_LO, EngineBox[0].);
	//FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_SYS_ID_HI, EngineBox[0].);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ERRORE_1_BOX_MOTORE, EngineBox[0].Errori1);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ERRORE_2_BOX_MOTORE, EngineBox[0].Errori2);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ERRORE_PERS1_BOX_MOTORE, EngineBox[0].ErroriPers1);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ERRORE_PERS2_BOX_MOTORE, EngineBox[0].ErroriPers2);    
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_CUMULATIVE_ERROR, ((EngineBox[0].PersErr.GlobalStatusFlag<<1) +  EngineBox[0].Error.TouchStatusFlag)); 

	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_INVERTER1_ERROR, Comp_Inverter[0].AlarmReg); 
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_INVERTER2_ERROR, Comp_Inverter[1].AlarmReg); 
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_INVERTER3_ERROR, Comp_Inverter[2].AlarmReg); 
	//FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_INVERTER4_ERROR, Comp_Inverter[3].AlarmReg); 
	
	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_STATUS_BOX_MOTORE, EngineBox[0].StatoCompressore);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ID_SPLIT_MASTER, EngineBox[0].IdMasterSplit);
	//FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_POWER_LIMIT, EngineBox[0].Inverter_Max_Power_Out);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_CYCLE_TIME, EngineBox[0].SystemCycleTime); 	// Tempo di ciclo elaborazione in mS x diagnostica
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_HOURS_OF_LIFE, MyRTC.HoursOfLife); 			// Ore di vita del Master x diagnostica

	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_TOUCH_NATION, K_Touch_Nation);				// Identificativo Nazione da inviare sui touch 
	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_MASTER_HW_VERSION, EngineBox[0].HW_Version);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_MASTER_FW_VERSION_HW, EngineBox[0].FW_Version_HW);    
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_MASTER_FW_VERSION_FW, EngineBox[0].FW_Version_FW);    
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_MASTER_FW_REVISION_FW, EngineBox[0].FW_Revision_FW);    
	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_DIAG_SPLIT_RES_REQ_ADDRESS, DiagnosticSplit.Address);			// Invio verso gli slave dell'indirizzo dello split su cui resettare gli allarmi proveniente dallo split master
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_MASTER_STATUS, Room[1].SplitStatus);
	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SUB_SPLIT_TEST_ADDRESS, DiagnosticSplit.SubAddress);
    
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_DIAG_SPLIT_TOUCH_PAGE, DiagnosticSplit.Touch_Page);

	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ACC_INVERTER, EngineBox[0].Acceleration_Inverter);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ACC_GASPRESS, EngineBox[0].Acceleration_GasPressure);

    

	// Invio dati per Doppio compressore
#if(K_AbilCompressor2==1)	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_CURRENT_MODE_ENGINE_BOX2, EngineBox[1].Mode);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX2_T_COMP_OUT, EngineBox[1].Temperature.Compressor_Output);     // REG_SPLIT_ENGINE_BOX_T_COMP_OUT       REG_SPLIT_ENGINE_BOX_TEMP_MANDATA
//	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ERRORE_1_BOX2_MOTORE, EngineBox[1].Errori1);
//	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ERRORE_PERS1_BOX2_MOTORE, EngineBox[1].ErroriPers1);
//	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ERRORE_PERS2_BOX2_MOTORE, EngineBox[1].ErroriPers2);    
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_CUMULATIVE_ERROR2, ((EngineBox[1].PersErr.GlobalStatusFlag<<1) + EngineBox[1].Error.TouchStatusFlag)); 
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_STATUS_BOX2_MOTORE, EngineBox[1].StatoCompressore);
#if(K_OnlyOneInverterComp2==1)        
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_DOUBLE_COMPRESSSOR_OK, 3);
#else
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_DOUBLE_COMPRESSSOR_OK, EngineBox[0].DoubleCompressorOn);
#endif
	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX2_COMPRESSOR_SPEED, EngineBox[1].Out_Inverter);
#endif
	// Invio dati per gestione limitazione potenza	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_POWER_MAX_AVAILABLE, Comp_Inverter[0].PowerMax_Available);	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_INVERTER_QUANTITY, K_Max_Inverter);	

	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_BOX_ABSORPTION, EngineBox[0].TotalPowerBoxAbsorption);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_MAX_DIAG_BOX_QUANTITY, Max_Engine_Box);
	
	
	// DEBUG per testare scambio dati con scheda Bridge
	//FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_POWER_LIMIT, EngineBox[0].PowerLimit); //Room[1].PowerLimit);	
	// DEBUG per testare scambio dati con scheda Bridge	

    if(Reinit.FromEngBox.TrigInit>0)
    {
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_FROM_INIT_SP_ROOM, Reinit.FromEngBox.SetPoint);	
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_FROM_INIT_FAN_SPEED, Reinit.FromEngBox.FanSpeed);	
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_FROM_INIT_POWER, Reinit.FromEngBox.Power);	
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_FROM_INIT_SP_ROOM_F, Reinit.FromEngBox.SetPoint_F);	
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_FROM_INIT_DEFMODE, Reinit.FromEngBox.DefMode);	
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_FROM_TRIGGER_INIT, Reinit.FromEngBox.TrigInit);	
    }
    
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_ECOMODE_STATUS, Comp_Inverter[0].EcoModeEnable);	

#if(K_Ing2_ModeSelection==1)	
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_SYSTEM_DISABLE, Room[k_Split_Master_Add].SystemDisable || EngineBox[0].AllValveOpen100p==1) || iDigiIn2; 	
#else
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_SYSTEM_DISABLE, Room[k_Split_Master_Add].SystemDisable || EngineBox[0].AllValveOpen100p==1);    
#endif
    //	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_UTA_SP, Bridge[1].SetUta.SetPoint);	
	
/*
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_UTA_ABIL, K_AbilUTA);	
    
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_UTASP_ON, UTA_H.UTASP_On*(EngineBox[0].DefMode==CoreRiscaldamento));		
*/
	temp = (K_Time_Restart_After_Blackout-TimerRestartAfterBlackOut.Value);
	//FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_MASTER_RESTART_TIME, (K_Time_Restart_After_Blackout-TimerRestartAfterBlackOut.Value));
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_MASTER_RESTART_TIME, temp);

	//GL FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP);	
	//GL FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP);	
	//GL FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP);	
	
	
	MountImageSlaveFlags();
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_FLAGS_ENABLE_1_16, EngineBox[0].Split_Flags_Enable_1_16);	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_FLAGS_ENABLE_17_32, EngineBox[0].Split_Flags_Enable_17_32);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_FLAGS_OFFLINE_1_16, EngineBox[0].Split_Flags_Offline_1_16);	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_FLAGS_OFFLINE_17_32, EngineBox[0].Split_Flags_Offline_17_32);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_FLAGS_ONLINE_1_16, EngineBox[0].Split_Flags_Online_1_16);	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_FLAGS_ONLINE_17_32, EngineBox[0].Split_Flags_Online_17_32);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_FLAGS_ALARM_1_16, EngineBox[0].Split_Flags_Alarm_1_16);	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_FLAGS_ALARM_17_32, EngineBox[0].Split_Flags_Alarm_17_32);
	
	MountImageSlaveModDevFlags();
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_MODDEV_FLAGS_ENABLE_1_16, EngineBox[0].Split_ModDev_Flags_Enable_1_16);	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_MODDEV_FLAGS_ENABLE_17_32, EngineBox[0].Split_ModDev_Flags_Enable_17_32);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_MODDEV_FLAGS_OFFLINE_1_16, EngineBox[0].Split_ModDev_Flags_Offline_1_16);	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_MODDEV_FLAGS_OFFLINE_17_32, EngineBox[0].Split_ModDev_Flags_Offline_17_32);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_MODDEV_FLAGS_ONLINE_1_16, EngineBox[0].Split_ModDev_Flags_Online_1_16);	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_MODDEV_FLAGS_ONLINE_17_32, EngineBox[0].Split_ModDev_Flags_Online_17_32);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_MODDEV_FLAGS_ALARM_1_16, EngineBox[0].Split_ModDev_Flags_Alarm_1_16);	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_MODDEV_FLAGS_ALARM_17_32, EngineBox[0].Split_ModDev_Flags_Alarm_17_32);
    if(++Scheduler>=Max_Room)
        Scheduler=1;
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_MODDEV_AND_ADDR_TYPE, Scheduler<<8 | Room[Scheduler].ModDev.Type);       // Invio in modo schedulato (1 per giro) il tipo di Device (Byte LO) e suo address (Byte HI))    
    
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_INV_INVOLT_C1, Comp_Inverter[0].In_Voltage);	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_INV_OUTPOWER_C1, Comp_Inverter[0].Out_Power);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_INV_INVOLT_C2, Comp_Inverter[1].In_Voltage);	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_INV_OUTPOWER_C2, Comp_Inverter[1].Out_Power);
    
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGBOX_SUPERH_HEATCALC, EngineBox[0].SuperHeat.HeatingCalc);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGBOX_PRESS_LIQUID_COND, EngineBox[0].Pressure.LiquidCond);

	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGBOX_SUPERH_PID_ERROR, (int)SuperHeatPID.Error);
    
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_ENGBOX_ALL_VALVEOPEN100P, EngineBox[0].AllValveOpen100p);
    
#if(K_MaxTouchFanSpeed>0)
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_MAX_FAN_SPEED, K_MaxTouchFanSpeed);
#endif
    
#if(K_AbilScoreOnDemand==1)
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_RD_VALVE_ONDEMAND_MIN_PERC_VAL, Room[k_Split_Master_Add].Valve_OnDemand_Min_Perc_Val);    
#endif    

#if(K_AbilCoolingSuperHeat==1)
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_SUPERHEAT_COOLINGCALC, EngineBox[0].SuperHeat.CoolingCalc);    
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_K_SETPOINT_COOLING_SUPERHEAT, (Room[k_Split_Master_Add].SuperHeat_Cool_Min_SetPoint*(EngineBox[0].TestAllAbil.SuperHeat_Cool_Correct==1))/*K_SetPoint_Cooling_SuperHeat*/);    
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_SUPERHEAT_CORRECT_COOLING, EngineBox[0].SuperHeat.Correct_Cooling);    
#endif    
    
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_CONDENSER_PRESS_ABIL, K_Condenser_Pressure_Abil);      
    
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_TOTAL_FAN_ABSORPTION, EngineBox[0].Split_Total_Fan_Absorption);      
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_COMPRESSOR1_ABSORPTION, Comp_Inverter[0].Out_Power);      
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_COMPRESSOR2_ABSORPTION, Comp_Inverter[1].Out_Power);   

#if(K_AbilPumpInverter==1)	
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_SEA_WATER_PUMP_ABSORPTION, Comp_Inverter[2].Out_Power);      
#else
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_SEA_WATER_PUMP_ABSORPTION, (K_Absorption_Pump_OnOff*(EngineBox[0].Out.Pompa_Acqua==1)));
#endif    
    
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_PRESSURE_LO_LIQ, PressioneLo_Liq_P);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_PRESSURE_LO_GAS, PressioneLo_Gas_G);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_PRESSURE_HI_LIQ, PressioneHi_Liq_P);
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_PRESSURE_HI_GAS, PressioneHi_Gas_G);    
           
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_ABIL, Room[k_Split_Master_Add].TestAll_Abil);    

    if(EngineBox[0].TestAllAbil.TempBatt_SP_Heat)
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_WORK_PRESS_HEAT, Room[k_Split_Master_Add].TestAll_TempBatt_SP_Heat);    

    if(EngineBox[0].TestAllAbil.SuperHeat_SP_Cool)
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL, Room[k_Split_Master_Add].TestAll_SuperHeat_SP_Cool);    

    if(EngineBox[0].TestAllAbil.TempBatt_SP_Cool)
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_WORK_PRESS_COOL, Room[k_Split_Master_Add].TestAll_TempBatt_SP_Cool);    

    if(EngineBox[0].TestAllAbil.MaxOpValve_Cool)
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMAXFREDDO, Room[k_Split_Master_Add].TestAll_MaxOpValve_Cool);                 

    if(EngineBox[0].TestAllAbil.MinOpValve_Cool)
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMINFREDDO, Room[k_Split_Master_Add].TestAll_MinOpValve_Cool);                 

    if(EngineBox[0].TestAllAbil.MinOpValve_Heat)
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMINCALDO, Room[k_Split_Master_Add].TestAll_MinOpValve_Heat);                 
    
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_FROZEN_ABIL, (EngineBox[0].Frigo_Abil*1)+(EngineBox[0].Freezer_Abil*2)+(EngineBox[0].LoopWater_Abil*4));                   
        
    if((EngineBox[0].Frigo_Abil == 1) || (EngineBox[0].Freezer_Abil == 1))
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_FROZEN_ON, (EngineBox[0].SplitFrigo_On*1)+(EngineBox[0].SplitFreezer_On*2));  
    
    
    
    if(EngineBox[0].Frigo_Abil==1 || EngineBox[0].Freezer_Abil==1 || EngineBox[0].LoopWater_Abil==1)    
       FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_FROZEN_ABIL, Room[k_Split_Master_Add].TestAll_Frozen_Abil);    
    else
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_FROZEN_ABIL, 0);    
    
    if(EngineBox[0].Frigo_Abil==1)    
    {
        if(EngineBox[0].TestAllAbil.TempBatt_SP_Frigo)
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_WORK_PRESS_FRIGO, Room[k_Split_Master_Add].TestAll_TempBatt_SP_Frigo);    

        if(EngineBox[0].TestAllAbil.SuperHeat_SP_Frigo)
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_SUPERHEAT_SP_FRIGO, Room[k_Split_Master_Add].TestAll_SuperHeat_SP_Frigo);    

        if(EngineBox[0].TestAllAbil.MaxOpValve_Frigo)
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMAXFREDDO_FRIGO, Room[k_Split_Master_Add].TestAll_MaxOpValve_Frigo);                 

        if(EngineBox[0].TestAllAbil.MinOpValve_Frigo)
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMINFREDDO_FRIGO, Room[k_Split_Master_Add].TestAll_MinOpValve_Frigo);                 
    }
   
    if(EngineBox[0].Freezer_Abil==1)    
    {
        if(EngineBox[0].TestAllAbil.TempBatt_SP_Freezer)
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_WORK_PRESS_FREEZER, Room[k_Split_Master_Add].TestAll_TempBatt_SP_Freezer);    

        if(EngineBox[0].TestAllAbil.SuperHeat_SP_Freezer)
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_SUPERHEAT_SP_FREEZER, Room[k_Split_Master_Add].TestAll_SuperHeat_SP_Freezer);    

        if(EngineBox[0].TestAllAbil.MaxOpValve_Freezer)
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMAXFREDDO_FREEZER, Room[k_Split_Master_Add].TestAll_MaxOpValve_Freezer);                 

        if(EngineBox[0].TestAllAbil.MinOpValve_Freezer)
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMINFREDDO_FREEZER, Room[k_Split_Master_Add].TestAll_MinOpValve_Freezer);                 
    }   

    if(EngineBox[0].LoopWater_Abil==1)    
    {
        if(EngineBox[0].TestAllAbil.TempBatt_SP_Freezer)
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_WORK_PRESS_FREEZER, Room[k_Split_Master_Add].TestAll_TempBatt_SP_Freezer);    

        if(EngineBox[0].TestAllAbil.SuperHeat_SP_Freezer)
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_SUPERHEAT_SP_FREEZER, Room[k_Split_Master_Add].TestAll_SuperHeat_SP_Freezer);    

        if(EngineBox[0].TestAllAbil.MaxOpValve_Freezer)
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMAXFREDDO_FREEZER, Room[k_Split_Master_Add].TestAll_MaxOpValve_Freezer);                 

        if(EngineBox[0].TestAllAbil.MinOpValve_Freezer)
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMINFREDDO_FREEZER, Room[k_Split_Master_Add].TestAll_MinOpValve_Freezer);                 
    }   

    if(EngineBox[0].Freezer_Abil==1)    
    {
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_ABIL_DEFROSTING, Room[k_Split_Master_Add].Defrosting_Abil);    
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_DEFROSTING_STEP_TIME, Room[k_Split_Master_Add].Defrosting_Step_Time);    
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_DEFROSTING_TIME, Room[k_Split_Master_Add].Defrosting_Time);    
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_DEFROSTING_TEMP_SET, Room[k_Split_Master_Add].Defrosting_Temp_Set);                 
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_DRIPPING_TIME, Room[k_Split_Master_Add].Dripping_Time);     
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_DRIPPING_TIME, Room[k_Split_Master_Add].Dripping_Time);     
            FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_VALVELIM_VALVOLAFROZEN, Room[k_Split_Master_Add].Valvola_Frozen);           
    }   
    if(EngineBox[0].TestAllAbil.SuperHeat_Custom_Par_Cool_PID)
    {
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL_PGAIN, Room[k_Split_Master_Add].TestAll_SuperHeat_Cool_pGain);                 
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL_IGAIN, Room[k_Split_Master_Add].TestAll_SuperHeat_Cool_iGain);                 
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL_DGAIN, Room[k_Split_Master_Add].TestAll_SuperHeat_Cool_dGain);                 
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID, Room[k_Split_Master_Add].TestAll_SuperHeat_Cool_Time_Exec_PID);
    }
    
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_MAINTENANCE_REMANING_TIME, Room[k_Split_Master_Add].Maintenance_Remaning_Time);

    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_SUPERHEAT_HEAT_MIN_SETPOINT, Room[k_Split_Master_Add].SuperHeat_Heat_Min_SetPoint);
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_SUPERHEAT_HEAT_MAX_SETPOINT, Room[k_Split_Master_Add].SuperHeat_Heat_Max_SetPoint);

    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_DIAG_SPLIT_SUPERHEAT_RISCALDAMENTO, EngineBox[0].SuperHeat.Value);

    temp = ((EngineBox[0].Pressure.Error.Compact << 8) & 0x0700) | (EngineBox[0].Temperature.Error.Compact & 0x003F);
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_BOX_PROBE_ERROR, temp);
    
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_MASTER_COMERROR_CNTCOMERR, EngineBox[0].ComError.CntComErr);                   
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_MASTER_COMERROR_TIMEOUTCOMERR, EngineBox[0].ComError.TimeOutComErr);                   
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_MASTER_COMERROR_CRC_COMERR, EngineBox[0].ComError.CRC_ComErr);                   
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_MASTER_COMERROR_MODCNTCOMERR, EngineBox[0].ComError.ModCntComErr);                   
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_MASTER_COMERROR_MODTIMEOUTCOMERR, EngineBox[0].ComError.ModTimeOutComErr);                   
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_MASTER_COMERROR_MODCRC_COMERR, EngineBox[0].ComError.ModCRC_ComErr);                   
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_MASTER_COMERROR_MODCNTCOMERR_U2, EngineBox[0].ComError.ModCntComErr_U2);                   
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_MASTER_COMERROR_MODTIMEOUTCOMERR_U2, EngineBox[0].ComError.ModTimeOutComErr_U2);                   
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_MASTER_COMERROR_MODCRC_COMERR_U2, EngineBox[0].ComError.ModCRC_ComErr_U2);                   
    //FuncWriteBrcReg (WriteSplitRegBrdCst, REG_MASTER_COMERROR_CNTCOMERR_U3, EngineBox[0].ComError.CntComErr_U3);                   
    //FuncWriteBrcReg (WriteSplitRegBrdCst, REG_MASTER_COMERROR_TIMEOUTCOMERR_U3, EngineBox[0].ComError.TimeOutComErr_U3);                   
    //FuncWriteBrcReg (WriteSplitRegBrdCst, REG_MASTER_COMERROR_CRC_COMERR_U3, EngineBox[0].ComError.CRC_ComErr_U3);                   

	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_TOTAL_AIR_POWER_OUTPUT, EngineBox[0].Split_Total_AirPowerOutput);      
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_TOTAL_AIR_POWER_BTU, EngineBox[0].Split_Total_AirPowerBTU);      
	FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SYSTEM_EFFICENCY_EER, EngineBox[0].System_Efficency_EER);      
    
    int MaxFreq=0;
#if(K_AbilCompressor2)    
    if(Comp_Inverter[0].Out_Freq>Comp_Inverter[1].Out_Freq)
        MaxFreq = Comp_Inverter[0].Out_Freq;
    else
        MaxFreq = Comp_Inverter[1].Out_Freq;
#else
        MaxFreq = Comp_Inverter[0].Out_Freq;
#endif    
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_INVERTER_UPPER_OUT_FREQ, MaxFreq);

    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_THRESHOLD_COMPRESSOR_HI, EngineBox[0].TempCompressorHi);  
    
    //FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_RD_SET_TEMP_AIR_OUT, Room[k_Split_Master_Add].Set_Temp_Air_Out);  

    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_SET_HUMI, Room[k_Split_Master_Add].Set_Humi);  
    
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_DEHUMI_ABIL, EngineBox[0].Dehumi_Abil);  
    
    if(EngineBox[0].Dehumi_Abil==1)
    {
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_SET_HUMI, Room[k_Split_Master_Add].Set_Humi);  
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_SET_DELTA_TEMP_MIN_DEHUMI, Room[k_Split_Master_Add].Set_Delta_Temp_Min_Dehumi);  
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_SET_FAN_SPEED_DEHUMI, Room[k_Split_Master_Add].Set_Fan_Speed_Dehumi);  
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_SET_PRESS_DEHUMI, Room[k_Split_Master_Add].Set_Press_Dehumi);  
        FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_WR_SET_TEMP_AIR_OUT_DEHUMI, Room[k_Split_Master_Add].Set_Temp_Air_Out_Dehumi);  
    }  
    
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_DIAG_SPLIT_WR_GAS_LEAKS, EngineBox[0].AddressGasLeaks);      
    
    
} 

void RefreshAllOutPut(void)
{
	char i=1;
		
#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    

    
	//for(i=0; i<Max_Engine_Box; i++)	
	//	RefreshOutPutEngineBox(&EngineBox[i], i);
		RefreshOutPutEngineBox();	//&EngineBox[0], i);
        
    for(i=1; i<Max_Room; i++)
    {
        RefreshOutPutRoom(i);
    }
}

void SplitReadyCommand(void)
{
#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    
         
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_READY_CMD_ADDRESS, DiagnosticSplit.CmdAddress);	//Dico che il master è pronto a passare i dati freschi
/*
    Room[k_Split_Master_Add].OffLine = FuncWriteReg (k_Split_Master_Add, REG_SPLIT_READY_OFFLINE, 1*(!Room[DiagnosticSplit.CmdAddress].OnLine), Room[k_Split_Master_Add].OffLine);				

    if(Room[DiagnosticSplit.CmdAddress].OnLine)
    {
        Room[k_Split_Master_Add].OffLine = FuncWriteReg (k_Split_Master_Add, REG_SPLIT_READY_POWER, Room[DiagnosticSplit.CmdAddress].PowerOn, Room[k_Split_Master_Add].OffLine);				
        Room[k_Split_Master_Add].OffLine = FuncWriteReg (k_Split_Master_Add, REG_SPLIT_READY_CURRENT_SP, Room[DiagnosticSplit.CmdAddress].SetPoint, Room[k_Split_Master_Add].OffLine);				
        Room[k_Split_Master_Add].OffLine = FuncWriteReg (k_Split_Master_Add, REG_SPLIT_READY_CURRENT_SP_F, Room[DiagnosticSplit.CmdAddress].SetPoint, Room[k_Split_Master_Add].OffLine);				
        Room[k_Split_Master_Add].OffLine = FuncWriteReg (k_Split_Master_Add, REG_SPLIT_READY_CURRENT_FANMODE, Room[DiagnosticSplit.CmdAddress].FanMode, Room[k_Split_Master_Add].OffLine);				
        Room[k_Split_Master_Add].OffLine = FuncWriteReg (k_Split_Master_Add, REG_SPLIT_READY_CURRENT_UTAMODE, Room[DiagnosticSplit.CmdAddress].UTAMode, Room[k_Split_Master_Add].OffLine);				
        
        Room[k_Split_Master_Add].OffLine = FuncWriteReg (k_Split_Master_Add, REG_DIAG_SPLIT_MODDEV_TYPE, Room[DiagnosticSplit.CmdAddress].ModDev.Type, Room[k_Split_Master_Add].OffLine);				        
    }
*/
}



//===========================================================================================
//*******************************************************************************************
//	SLAVE
//*******************************************************************************************
//===========================================================================================

void Process_Cmd(volatile TypBuffer * LastCmd)
{
 
	if(LastRx.Valid_Data == 1)
	{
		switch (LastRx.Comando)	// interpreto il comando
		{
			case Prg:
				while(BufferTx.Valid_Data == 1)	// attendo 
					continue;
				Write_Cmd(&BufferTx, LastRx.Mitt_Add, LastRx.Comando, 0, LastRx.Data);
				break;

			case ReadReg:
				while(BufferTx.Valid_Data == 1)	// attendo 
					continue;
				Write_Cmd(&BufferTx, LastRx.Mitt_Add, LastRx.Comando, LastRx.Registro, GetData(LastRx.Registro));
				break;

			case WriteReg:
				while(BufferTx.Valid_Data == 1)	// attendo 
					continue;
				char Esito = 0;
				Esito = PutData(LastRx.Registro, LastRx.Data);	// Tento la scrittura del registro
				if(Esito == 0)			// in base all'esito rispondo
				{	Write_Cmd(&BufferTx, LastRx.Mitt_Add, LastRx.Comando, LastRx.Registro, Ack);	}
				else
				{	Write_Cmd(&BufferTx, LastRx.Mitt_Add, LastRx.Comando, LastRx.Registro, N_Ack);	}
				break;

			case Check:
				while(BufferTx.Valid_Data == 1)	// attendo 
					continue;
				Write_Cmd(&BufferTx, LastRx.Mitt_Add, LastRx.Comando, 0, Ack);
				break;

		}
		LastRx.Valid_Data = 0;
	}
   
}

int	GetData(int Reg)
{
	switch(Reg)
	{
		case REG_MASTER_TEMP_1:
			return EngineBox[0].Temperature.Compressor_Output;	//Temperatura_1;

		case REG_MASTER_TEMP_2:
			return EngineBox[0].Temperature.Compressor_Suction;	//Temperatura_2;

		case REG_MASTER_TEMP_3:
			return EngineBox[0].Temperature.Condenser;			//Temperatura_3;

		case REG_MASTER_TEMP_4:
			return EngineBox[0].Temperature.Sea_Water;			//Temperatura_4;

		case REG_MASTER_TEMP_5:
			return EngineBox[0].Temperature.Liquid;	//Temperatura_5;

		case REG_MASTER_TEMP_6:
			return EngineBox[0].Temperature.Gas;		//Temperatura_6;

		case REG_MASTER_GAS_PRESSURE_G:
			return EngineBox[0].Pressure.Gas;			//Gas_Pressure;

		case REG_MASTER_ADC_1:
			return Adc_Temp[1];

		case REG_MASTER_ADC_2:
			return Adc_Temp[2];

		case REG_MASTER_ADC_3:
			return Adc_Temp[3];

		case REG_MASTER_ADC_4:
			return Adc_Temp[4];

		case REG_MASTER_ADC_5:
			return Adc_Temp[5];

		case REG_MASTER_ADC_6:
			return Adc_Temp[6];

		case REG_MASTER_ADC_7:
			return Adc_Temp[7];

		case REG_MASTER_ADC_8:
			return Adc_Temp[8];

		case REG_MASTER_DIGIT_INPUT:
			return DigitalInput;

		case REG_INVERTER_ST_RELE:
			return StatoRele;

		case REG_INVERTER_DAC_1:
			return DAC_1;

		case REG_INVERTER_DAC_2:
			return DAC_2;

		case REG_INVERTER_DAC_3:
			return DAC_3;

		case REG_INVERTER_DAC_4:
			return DAC_4;

		case REG_MASTER_GAS_PRESSURE_P:
			return EngineBox[0].Pressure.Liquid;



		case REG_MASTER_AD_T1_0:		// AD_T1[0];
			return 0; //AD_T1;

		case REG_MASTER_AD_T1_1:
			return 0; // AD_T1[1];

		case REG_MASTER_AD_T1_2:
			return 0; // AD_T1[2];

		case REG_MASTER_AD_T1_3:
			return 0; // AD_T1[3];

		case REG_MASTER_AD_T1_4:
			return 0; // AD_T1[4];

		case REG_MASTER_AD_T1_5:
			return 0; // AD_T1[5];

		case REG_MASTER_AD_T1_6:
			return 0; // AD_T1[6];

		case REG_MASTER_AD_T1_7:
			return 0; // AD_T1[7];

		case REG_MASTER_AD_T1_8:
			return 0; // AD_T1[8];

		case REG_MASTER_AD_T1_9:
			return 0; // AD_T1[9];

		case REG_MASTER_AD_T1_10:
			return 0; // AD_T1[10];

		case REG_MASTER_AD_T1_11:
			return 0; // AD_T1[11];

		case REG_MASTER_AD_T2_0:	// AD_T2[0];
			return 0; // AD_T2[0];

		case REG_MASTER_AD_T2_1:
			return 0; // AD_T2[1];

		case REG_MASTER_AD_T2_2:
			return 0; // AD_T2[2];

		case REG_MASTER_AD_T2_3:
			return 0; // AD_T2[3];

		case REG_MASTER_AD_T2_4:
			return 0; // AD_T2[4];

		case REG_MASTER_AD_T2_5:
			return 0; // AD_T2[5];

		case REG_MASTER_AD_T2_6:
			return 0; // AD_T2[6];

		case REG_MASTER_AD_T2_7:
			return 0; // AD_T2[7];

		case REG_MASTER_AD_T2_8:
			return 0; // AD_T2[8];

		case REG_MASTER_AD_T2_9:
			return 0; // AD_T2[9];

		case REG_MASTER_AD_T2_10:
			return 0; // AD_T2[10];

		case REG_MASTER_AD_T2_11:
			return 0; // AD_T2[11];

		case REG_MASTER_AD_T3_0:		// AD_T3[0]
			return 0; // AD_T3[0];

		case REG_MASTER_AD_T3_1:
			return 0; // AD_T3[1];

		case REG_MASTER_AD_T3_2:
			return 0; // AD_T3[2];

		case REG_MASTER_AD_T3_3:
			return 0; // AD_T3[3];

		case REG_MASTER_AD_T3_4:
			return 0; // AD_T3[4];

		case REG_MASTER_AD_T3_5:
			return 0; // AD_T3[5];

		case REG_MASTER_AD_T3_6:
			return 0; // AD_T3[6];

		case REG_MASTER_AD_T3_7:
			return 0; // AD_T3[7];

		case REG_MASTER_AD_T3_8:
			return 0; // AD_T3[8];

		case REG_MASTER_AD_T3_9:
			return 0; // AD_T3[9];

		case REG_MASTER_AD_T3_10:
			return 0; // AD_T3[10];

		case REG_MASTER_AD_T3_11:
			return 0; // AD_T3[11];

		case REG_MASTER_AD_T4_0:		// AD_T4[0]
			return 0; // AD_T4[0];

		case REG_MASTER_AD_T4_1:
			return 0; // AD_T4[1];

		case REG_MASTER_AD_T4_2:
			return 0; // AD_T4[2];

		case REG_MASTER_AD_T4_3:
			return 0; // AD_T4[3];

		case REG_MASTER_AD_T4_4:
			return 0; // AD_T4[4];

		case REG_MASTER_AD_T4_5:
			return 0; // AD_T4[5];

		case REG_MASTER_AD_T4_6:
			return 0; // AD_T4[6];

		case REG_MASTER_AD_T4_7:
			return 0; // AD_T4[7];

		case REG_MASTER_AD_T4_8:
			return 0; // AD_T4[8];

		case REG_MASTER_AD_T4_9:
			return 0; // AD_T4[9];

		case REG_MASTER_AD_T4_10:
			return 0; // AD_T4[10];

		case REG_MASTER_AD_T4_11:
			return 0; // AD_T4[11];

		case REG_MASTER_AD_T5_0:		// AD_T5[0]
			return 0; // AD_T5[0];

		case REG_MASTER_AD_T5_1:
			return 0; // AD_T5[1];

		case REG_MASTER_AD_T5_2:
			return 0; // AD_T5[2];

		case REG_MASTER_AD_T5_3:
			return 0; // AD_T5[3];

		case REG_MASTER_AD_T5_4:
			return 0; // AD_T5[4];

		case REG_MASTER_AD_T5_5:
			return 0; // AD_T5[5];

		case REG_MASTER_AD_T5_6:
			return 0; // AD_T5[6];

		case REG_MASTER_AD_T5_7:
			return 0; // AD_T5[7];

		case REG_MASTER_AD_T5_8:
			return 0; // AD_T5[8];

		case REG_MASTER_AD_T5_9:
			return 0; // AD_T5[9];

		case REG_MASTER_AD_T5_10:
			return 0; // AD_T5[10];

		case REG_MASTER_AD_T5_11:
			return 0; // AD_T5[11];
/*
		case REG_MASTER_AD_T6_0:		// AD_T6[0];
			return AD_T6[0];

		case REG_MASTER_AD_T6_1:
			return AD_T6[1];

		case REG_MASTER_AD_T6_2:
			return AD_T6[2];

		case REG_MASTER_AD_T6_3:
			return AD_T6[3];

		case REG_MASTER_AD_T6_4:
			return AD_T6[4];

		case REG_MASTER_AD_T6_5:
			return AD_T6[5];

		case REG_MASTER_AD_T6_6:
			return AD_T6[6];

		case REG_MASTER_AD_T6_7:
			return AD_T6[7];

		case REG_MASTER_AD_T6_8:
			return AD_T6[8];

		case REG_MASTER_AD_T6_9:
			return AD_T6[9];

		case REG_MASTER_AD_T6_10:
			return AD_T6[10];

		case REG_MASTER_AD_T6_11:
			return AD_T6[11];
*/
		case REG_MASTER_KT_T1_0:		// Kt_T1[0]
			return 0; // Kt_T1[0];

		case REG_MASTER_KT_T1_1:
			return 0; // Kt_T1[1];

		case REG_MASTER_KT_T1_2:
			return 0; // Kt_T1[2];

		case REG_MASTER_KT_T1_3:
			return 0; // Kt_T1[3];

		case REG_MASTER_KT_T1_4:
			return 0; // Kt_T1[4];

		case REG_MASTER_KT_T1_5:
			return 0; // Kt_T1[5];

		case REG_MASTER_KT_T1_6:
			return 0; // Kt_T1[6];

		case REG_MASTER_KT_T1_7:
			return 0; // Kt_T1[7];

		case REG_MASTER_KT_T1_8:
			return 0; // Kt_T1[8];

		case REG_MASTER_KT_T1_9:
			return 0; // Kt_T1[9];

		case REG_MASTER_KT_T1_10:
			return 0; // Kt_T1[10];

		case REG_MASTER_KT_T2_0:		// Kt_T2[0];
			return 0; // Kt_T2[0];

		case REG_MASTER_KT_T2_1:
			return 0; // Kt_T2[1];

		case REG_MASTER_KT_T2_2:
			return 0; // Kt_T2[2];

		case REG_MASTER_KT_T2_3:
			return 0; // Kt_T2[3];

		case REG_MASTER_KT_T2_4:
			return 0; // Kt_T2[4];

		case REG_MASTER_KT_T2_5:
			return 0; // Kt_T2[5];

		case REG_MASTER_KT_T2_6:
			return 0; // Kt_T2[6];

		case REG_MASTER_KT_T2_7:
			return 0; // Kt_T2[7];

		case REG_MASTER_KT_T2_8:
			return 0; // Kt_T2[8];

		case REG_MASTER_KT_T2_9:
			return 0; // Kt_T2[9];

		case REG_MASTER_KT_T2_10:
			return 0; // Kt_T2[10];

		case REG_MASTER_KT_T3_0:		//Kt_T3[0];
			return 0; // Kt_T3[0];

		case REG_MASTER_KT_T3_1:
			return 0; // Kt_T3[1];

		case REG_MASTER_KT_T3_2:
			return 0; // Kt_T3[2];

		case REG_MASTER_KT_T3_3:
			return 0; // Kt_T3[3];

		case REG_MASTER_KT_T3_4:
			return 0; // Kt_T3[4];

		case REG_MASTER_KT_T3_5:
			return 0; // Kt_T3[5];

		case REG_MASTER_KT_T3_6:
			return 0; // Kt_T3[6];

		case REG_MASTER_KT_T3_7:
			return 0; // Kt_T3[7];

		case REG_MASTER_KT_T3_8:
			return 0; // Kt_T3[8];

		case REG_MASTER_KT_T3_9:
			return 0; // Kt_T3[9];

		case REG_MASTER_KT_T3_10:
			return 0; // Kt_T3[10];

		case REG_MASTER_KT_T4_0:		// Kt_T4[0];
			return 0; // Kt_T4[0];

		case REG_MASTER_KT_T4_1:
			return 0; // Kt_T4[1];

		case REG_MASTER_KT_T4_2:
			return 0; // Kt_T4[2];

		case REG_MASTER_KT_T4_3:
			return 0; // Kt_T4[3];

		case REG_MASTER_KT_T4_4:
			return 0; // Kt_T4[4];

		case REG_MASTER_KT_T4_5:
			return 0; // Kt_T4[5];

		case REG_MASTER_KT_T4_6:
			return 0; // Kt_T4[6];

		case REG_MASTER_KT_T4_7:
			return 0; // Kt_T4[7];

		case REG_MASTER_KT_T4_8:
			return 0; // Kt_T4[8];

		case REG_MASTER_KT_T4_9:
			return 0; // Kt_T4[9];

		case REG_MASTER_KT_T4_10:
			return 0; // Kt_T4[10];

		case REG_MASTER_KT_T5_0:		// Kt_T5[0];
			return 0; // Kt_T5[0];

		case REG_MASTER_KT_T5_1:
			return 0; // Kt_T5[1];

		case REG_MASTER_KT_T5_2:
			return 0; // Kt_T5[2];

		case REG_MASTER_KT_T5_3:
			return 0; // Kt_T5[3];

		case REG_MASTER_KT_T5_4:
			return 0; // Kt_T5[4];

		case REG_MASTER_KT_T5_5:
			return 0; // Kt_T5[5];

		case REG_MASTER_KT_T5_6:
			return 0; // Kt_T5[6];

		case REG_MASTER_KT_T5_7:
			return 0; // Kt_T5[7];

		case REG_MASTER_KT_T5_8:
			return 0; // Kt_T5[8];

		case REG_MASTER_KT_T5_9:
			return 0; // Kt_T5[9];

		case REG_MASTER_KT_T5_10:
			return 0; // Kt_T5[10];
/*
		case REG_MASTER_KT_T6_0:		// Kt_T6[0];
			return Kt_T6[0];

		case REG_MASTER_KT_T6_1:
			return Kt_T6[1];

		case REG_MASTER_KT_T6_2:
			return Kt_T6[2];

		case REG_MASTER_KT_T6_3:
			return Kt_T6[3];

		case REG_MASTER_KT_T6_4:
			return Kt_T6[4];

		case REG_MASTER_KT_T6_5:
			return Kt_T6[5];

		case REG_MASTER_KT_T6_6:
			return Kt_T6[6];

		case REG_MASTER_KT_T6_7:
			return Kt_T6[7];

		case REG_MASTER_KT_T6_8:
			return Kt_T6[8];

		case REG_MASTER_KT_T6_9:
			return Kt_T6[9];

		case REG_MASTER_KT_T6_10:
			return Kt_T6[10];
*/
		case REG_MASTER_FW_VERSION:				// Registri d'informazione
			return Versione_Fw;

		case REG_MASTER_FW_REVISION:
			return Revisione_Fw;

		case REG_MASTER_ID_MODULO:
			return Id_Modulo;

		case REG_MASTER_ADD_MODULO:
			return Indirizzo_Modulo;

		case REG_MASTER_SYSTEM_NUMBER_LO:
			return ToInt(SN2, SN1);

		case REG_MASTER_SYSTEM_NUMBER_HI:
			return ToInt(SN4, SN3);

		case REG_MASTER_FABRICATION_DAY:
			return GiornoFabbricazione;

		case REG_MASTER_FABRICATION_MONTH:
			return MeseFabbricazione;

		case REG_MASTER_FABRICATION_YEAR:
			return AnnoFabbricazione;

		case REG_MASTER_FLASH_CRC_16:
			return CRC_Flash;

		case REG_MASTER_SAVE_2_E2:
			return WriteE2Status;

		case REG_MASTER_HOURS_OF_LIFE:
			return MyRTC.HoursOfLife;

		default:
			return 0xFFFF;
	}
}


char PutData(int Reg, int Value)
{
	static int LastValue[4]={0,0,0,0};
	static char LastValPt=0;
	
	switch(Reg)
	{
		case REG_INVERTER_ST_RELE:
			StatoRele = Value;
			return 0;

		case REG_INVERTER_DAC_1:
			DAC_1 = Value;
			return 0;

		case REG_INVERTER_DAC_2:
			DAC_2 = Value;
			return  0;

		case REG_INVERTER_DAC_3:
			DAC_3 = Value;
			return 0;

		case REG_INVERTER_DAC_4:
			DAC_4 = Value;
			return 0;


		case REG_MASTER_AD_T1_0:		// AD_T1[0];
			//AD_T1[0] = Value;
			return 0;

		case REG_MASTER_AD_T1_1:
			//AD_T1[1] = Value;
			return 0;

		case REG_MASTER_AD_T1_2:
			//AD_T1[2] = Value;
			return 0;

		case REG_MASTER_AD_T1_3:
			//AD_T1[3] = Value;
			return 0;

		case REG_MASTER_AD_T1_4:
			//AD_T1[4] = Value;
			return 0;

		case REG_MASTER_AD_T1_5:
			//AD_T1[5] = Value;
			return 0;

		case REG_MASTER_AD_T1_6:
			//AD_T1[6] = Value;
			return 0;

		case REG_MASTER_AD_T1_7:
			//AD_T1[7] = Value;
			return 0;

		case REG_MASTER_AD_T1_8:
			//AD_T1[8] = Value;
			return 0;

		case REG_MASTER_AD_T1_9:
			//AD_T1[9] = Value;
			return 0;

		case REG_MASTER_AD_T1_10:
			//AD_T1[10] = Value;
			return 0;

		case REG_MASTER_AD_T1_11:
			//AD_T1[11] = Value;
			return 0;

		case REG_MASTER_AD_T2_0:	// AD_T2[0];
			//AD_T2[0] = Value;
			return 0;

		case REG_MASTER_AD_T2_1:
			//AD_T2[1] = Value;
			return 0;

		case REG_MASTER_AD_T2_2:
			//AD_T2[2] = Value;
			return 0;

		case REG_MASTER_AD_T2_3:
			//AD_T2[3] = Value;
			return 0;

		case REG_MASTER_AD_T2_4:
			//AD_T2[4] = Value;
			return 0;

		case REG_MASTER_AD_T2_5:
			//AD_T2[5] = Value;
			return 0;

		case REG_MASTER_AD_T2_6:
			//AD_T2[6] = Value;
			return 0;

		case REG_MASTER_AD_T2_7:
			//AD_T2[7] = Value;
			return 0;

		case REG_MASTER_AD_T2_8:
			//AD_T2[8] = Value;
			return 0;

		case REG_MASTER_AD_T2_9:
			//AD_T2[9] = Value;
			return 0;

		case REG_MASTER_AD_T2_10:
			//AD_T2[10] = Value;
			return 0;

		case REG_MASTER_AD_T2_11:
			//AD_T2[11] = Value;
			return 0;

		case REG_MASTER_AD_T3_0:		// AD_T3[0]
			//AD_T3[0] = Value;
			return 0;

		case REG_MASTER_AD_T3_1:
			//AD_T3[1] = Value;
			return 0;

		case REG_MASTER_AD_T3_2:
			//AD_T3[2] = Value;
			return 0;

		case REG_MASTER_AD_T3_3:
			//AD_T3[3] = Value;
			return 0;

		case REG_MASTER_AD_T3_4:
			//AD_T3[4] = Value;
			return 0;

		case REG_MASTER_AD_T3_5:
			//AD_T3[5] = Value;
			return 0;

		case REG_MASTER_AD_T3_6:
			//AD_T3[6] = Value;
			return 0;

		case REG_MASTER_AD_T3_7:
			//AD_T3[7] = Value;
			return 0;

		case REG_MASTER_AD_T3_8:
			//AD_T3[8] = Value;
			return 0;

		case REG_MASTER_AD_T3_9:
			//AD_T3[9] = Value;
			return 0;

		case REG_MASTER_AD_T3_10:
			//AD_T3[10] = Value;
			return 0;

		case REG_MASTER_AD_T3_11:
			//AD_T3[11] = Value;
			return 0;

		case REG_MASTER_AD_T4_0:		// AD_T4[0]
			//AD_T4[0] = Value;
			return 0;

		case REG_MASTER_AD_T4_1:
			//AD_T4[1] = Value;
			return 0;

		case REG_MASTER_AD_T4_2:
			//AD_T4[2] = Value;
			return 0;

		case REG_MASTER_AD_T4_3:
			//AD_T4[3] = Value;
			return 0;

		case REG_MASTER_AD_T4_4:
			//AD_T4[4] = Value;
			return 0;

		case REG_MASTER_AD_T4_5:
			//AD_T4[5] = Value;
			return 0;

		case REG_MASTER_AD_T4_6:
			//AD_T4[6] = Value;
			return 0;

		case REG_MASTER_AD_T4_7:
			//AD_T4[7] = Value;
			return 0;

		case REG_MASTER_AD_T4_8:
			//AD_T4[8] = Value;
			return 0;

		case REG_MASTER_AD_T4_9:
			//AD_T4[9] = Value;
			return 0;

		case REG_MASTER_AD_T4_10:
			//AD_T4[10] = Value;
			return 0;

		case REG_MASTER_AD_T4_11:
			//AD_T4[11] = Value;
			return 0;

		case REG_MASTER_AD_T5_0:		// AD_T5[0]
			//AD_T5[0] = Value;
			return 0;

		case REG_MASTER_AD_T5_1:
			//AD_T5[1] = Value;
			return 0;

		case REG_MASTER_AD_T5_2:
			//AD_T5[2] = Value;
			return 0;

		case REG_MASTER_AD_T5_3:
			//AD_T5[3] = Value;
			return 0;

		case REG_MASTER_AD_T5_4:
			//AD_T5[4] = Value;
			return 0;

		case REG_MASTER_AD_T5_5:
			//AD_T5[5] = Value;
			return 0;

		case REG_MASTER_AD_T5_6:
			//AD_T5[6] = Value;
			return 0;

		case REG_MASTER_AD_T5_7:
			//AD_T5[7] = Value;
			return 0;

		case REG_MASTER_AD_T5_8:
			//AD_T5[8] = Value;
			return 0;

		case REG_MASTER_AD_T5_9:
			//AD_T5[9] = Value;
			return 0;

		case REG_MASTER_AD_T5_10:
			//AD_T5[10] = Value;
			return 0;

		case REG_MASTER_AD_T5_11:
			//AD_T5[11] = Value;
			return 0;
/*
		case REG_MASTER_AD_T6_0:		// AD_T6[0];
			//AD_T6[0] = Value;
			return 0;

		case REG_MASTER_AD_T6_1:
			//AD_T6[1] = Value;
			return 0;

		case REG_MASTER_AD_T6_2:
			//AD_T6[2] = Value;
			return 0;

		case REG_MASTER_AD_T6_3:
			//AD_T6[3] = Value;
			return 0;

		case REG_MASTER_AD_T6_4:
			//AD_T6[4] = Value;
			return 0;

		case REG_MASTER_AD_T6_5:
			//AD_T6[5] = Value;
			return 0;

		case REG_MASTER_AD_T6_6:
			//AD_T6[6] = Value;
			return 0;

		case REG_MASTER_AD_T6_7:
			//AD_T6[7] = Value;
			return 0;

		case REG_MASTER_AD_T6_8:
			//AD_T6[8] = Value;
			return 0;

		case REG_MASTER_AD_T6_9:
			//AD_T6[9] = Value;
			return 0;

		case REG_MASTER_AD_T6_10:
			//AD_T6[10] = Value;
			return 0;

		case REG_MASTER_AD_T6_11:
			AD_T6[11] = Value;
			return 0;
*/
		case REG_MASTER_KT_T1_0:		// Kt_T1[0]
			//AD_Kt_T1[0] = Value;
			return 0;

		case REG_MASTER_KT_T1_1:
			//AD_Kt_T1[1] = Value;
			return 0;

		case REG_MASTER_KT_T1_2:
			//AD_Kt_T1[2] = Value;
			return 0;

		case REG_MASTER_KT_T1_3:
			//AD_Kt_T1[3] = Value;
			return 0;

		case REG_MASTER_KT_T1_4:
			//AD_Kt_T1[4] = Value;
			return 0;

		case REG_MASTER_KT_T1_5:
			//AD_Kt_T1[5] = Value;
			return 0;

		case REG_MASTER_KT_T1_6:
			//AD_Kt_T1[6] = Value;
			return 0;

		case REG_MASTER_KT_T1_7:
			//AD_Kt_T1[7] = Value;
			return 0;

		case REG_MASTER_KT_T1_8:
			//AD_Kt_T1[8] = Value;
			return 0;

		case REG_MASTER_KT_T1_9:
			//Kt_T1[9] = Value;
			return 0;

		case REG_MASTER_KT_T1_10:
			//Kt_T1[10] = Value;
			return 0;

		case REG_MASTER_KT_T2_0:		// Kt_T2[0];
			//Kt_T2[0] = Value;
			return 0;

		case REG_MASTER_KT_T2_1:
			//Kt_T2[1] = Value;
			return 0;

		case REG_MASTER_KT_T2_2:
			//Kt_T2[2] = Value;
			return 0;

		case REG_MASTER_KT_T2_3:
			//Kt_T2[3] = Value;
			return 0;

		case REG_MASTER_KT_T2_4:
			//Kt_T2[4] = Value;
			return 0;

		case REG_MASTER_KT_T2_5:
			//Kt_T2[5] = Value;
			return 0;

		case REG_MASTER_KT_T2_6:
			//Kt_T2[6] = Value;
			return 0;

		case REG_MASTER_KT_T2_7:
			//Kt_T2[7] = Value;
			return 0;

		case REG_MASTER_KT_T2_8:
			//Kt_T2[8] = Value;
			return 0;

		case REG_MASTER_KT_T2_9:
			//Kt_T2[9] = Value;
			return 0;

		case REG_MASTER_KT_T2_10:
			//Kt_T2[10] = Value;
			return 0;

		case REG_MASTER_KT_T3_0:		//Kt_T3[0];
			//Kt_T3[0] = Value;
			return 0;

		case REG_MASTER_KT_T3_1:
			//Kt_T3[1] = Value;
			return 0;

		case REG_MASTER_KT_T3_2:
			//Kt_T3[2] = Value;
			return 0;

		case REG_MASTER_KT_T3_3:
			//Kt_T3[3] = Value;
			return 0;

		case REG_MASTER_KT_T3_4:
			//Kt_T3[4] = Value;
			return 0;

		case REG_MASTER_KT_T3_5:
			//Kt_T3[5] = Value;
			return 0;

		case REG_MASTER_KT_T3_6:
			//Kt_T3[6] = Value;
			return 0;

		case REG_MASTER_KT_T3_7:
			//Kt_T3[7] = Value;
			return 0;

		case REG_MASTER_KT_T3_8:
			//Kt_T3[8] = Value;
			return 0;

		case REG_MASTER_KT_T3_9:
			//Kt_T3[9] = Value;
			return 0;

		case REG_MASTER_KT_T3_10:
			//Kt_T3[10] = Value;
			return 0;

		case REG_MASTER_KT_T4_0:		// Kt_T4[0];
			//Kt_T4[0] = Value;
			return 0;

		case REG_MASTER_KT_T4_1:
			//Kt_T4[1] = Value;
			return 0;

		case REG_MASTER_KT_T4_2:
			//Kt_T4[2] = Value;
			return 0;

		case REG_MASTER_KT_T4_3:
			//Kt_T4[3] = Value;
			return 0;

		case REG_MASTER_KT_T4_4:
			//Kt_T4[4] = Value;
			return 0;

		case REG_MASTER_KT_T4_5:
			//Kt_T4[5] = Value;
			return 0;

		case REG_MASTER_KT_T4_6:
			//Kt_T4[6] = Value;
			return 0;

		case REG_MASTER_KT_T4_7:
			//Kt_T4[7] = Value;
			return 0;

		case REG_MASTER_KT_T4_8:
			//Kt_T4[8] = Value;
			return 0;

		case REG_MASTER_KT_T4_9:
			//Kt_T4[9] = Value;
			return 0;

		case REG_MASTER_KT_T4_10:
			//Kt_T4[10] = Value;
			return 0;

		case REG_MASTER_KT_T5_0:		// Kt_T5[0];
			//Kt_T5[0] = Value;
			return 0;

		case REG_MASTER_KT_T5_1:
			//Kt_T5[1] = Value;
			return 0;

		case REG_MASTER_KT_T5_2:
			//Kt_T5[2] = Value;
			return 0;

		case REG_MASTER_KT_T5_3:
			//Kt_T5[3] = Value;
			return 0;

		case REG_MASTER_KT_T5_4:
			//Kt_T5[4] = Value;
			return 0;

		case REG_MASTER_KT_T5_5:
			//Kt_T5[5] = Value;
			return 0;

		case REG_MASTER_KT_T5_6:
			//Kt_T5[6] = Value;
			return 0;

		case REG_MASTER_KT_T5_7:
			//Kt_T5[7] = Value;
			return 0;

		case REG_MASTER_KT_T5_8:
			//Kt_T5[8] = Value;
			return 0;

		case REG_MASTER_KT_T5_9:
			//Kt_T5[9] = Value;
			return 0;

		case REG_MASTER_KT_T5_10:
			//Kt_T5[10] = Value;
			return 0;
/*
		case REG_MASTER_KT_T6_0:		// Kt_T6[0];
			Kt_T6[0] = Value;
			return 0;

		case REG_MASTER_KT_T6_1:
			Kt_T6[1] = Value;
			return 0;

		case REG_MASTER_KT_T6_2:
			Kt_T6[2] = Value;
			return 0;

		case REG_MASTER_KT_T6_3:
			Kt_T6[3] = Value;
			return 0;

		case REG_MASTER_KT_T6_4:
			Kt_T6[4] = Value;
			return 0;

		case REG_MASTER_KT_T6_5:
			Kt_T6[5] = Value;
			return 0;

		case REG_MASTER_KT_T6_6:
			Kt_T6[6] = Value;
			return 0;

		case REG_MASTER_KT_T6_7:
			Kt_T6[7] = Value;
			return 0;

		case REG_MASTER_KT_T6_8:
			Kt_T6[8] = Value;
			return 0;

		case REG_MASTER_KT_T6_9:
			Kt_T6[9] = Value;
			return 0;

		case REG_MASTER_KT_T6_10:
			Kt_T6[10] = Value;
			return 0;
*/

		case REG_MASTER_SYSTEM_NUMBER_LO:
			SN1 = LoByte(Value);
			SN2 = HiByte(Value);
			return 0;

		case REG_MASTER_SYSTEM_NUMBER_HI:
			SN3 = LoByte(Value);
			SN4 = HiByte(Value);
			return 0;

		case REG_MASTER_FABRICATION_DAY:
			GiornoFabbricazione = Value;
			return 0;

		case REG_MASTER_FABRICATION_MONTH:
			MeseFabbricazione = Value;
			return 0;

		case REG_MASTER_FABRICATION_YEAR:
			AnnoFabbricazione = Value;
			return 0;

		case REG_MASTER_FLASH_CRC_16:
			CRC_Flash = Value;
			return 0;

		case REG_MASTER_SAVE_2_E2:
			WriteE2Status = Value;
			return 0;

		case REG_MASTER_HOURS_OF_LIFE:
			MyRTC.HoursOfLife = Value;
			return 0;

		case REG_BOARD_AUTORESET:	
            EngineBox[0].AutoresetMagicNumber = Value;			
			return 0;						
			
		default:
			return 1;
	}
}


// Legge i dati correnti impostati sullo Split Master.

void ReadDataFromSlaveMaster(void)
{
	RefreshInPutRoom(&Room[k_Split_Master_Add], k_Split_Master_Add, k_Split_Master_Add);
}

void MasSlaveSyncroData(int Address)
{	
	static int EqualRead1[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	static int EqualRead2[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	
	static int OldEqualRead1[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	static int OldEqualRead2[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	
	//---------------------------	
	// SetPoint
	//---------------------------
	Syncro2Diag(Address, DiagnosticSplit.CmdAddress,REG_DIAG_SPLIT_RD_SET_SETPOINT, REG_DIAG_SPLIT_WR_SET_SETPOINT, REG_SPLIT_SYNC_RD_SETP, REG_SPLIT_SYNC_WR_SETP, &EqualRead1[0], &OldEqualRead1[0], &EqualRead2[0], &OldEqualRead2[0], &Room[Address].DiagWrite.SetPoint);
		
	//---------------------------	
	// Power
	//---------------------------	
	Syncro2Diag(Address, DiagnosticSplit.CmdAddress,REG_DIAG_SPLIT_RD_SET_POWER, REG_DIAG_SPLIT_WR_SET_POWER, REG_SPLIT_SYNC_RD_PWR, REG_SPLIT_SYNC_WR_PWR, &EqualRead1[1], &OldEqualRead1[1], &EqualRead2[1], &OldEqualRead2[1], &Room[Address].DiagWrite.Pwr);	

	//---------------------------	
	// FAN Speed
	//---------------------------
	Syncro2Diag(Address, DiagnosticSplit.CmdAddress,REG_DIAG_SPLIT_RD_SET_FANSPEED, REG_DIAG_SPLIT_WR_SET_FANSPEED, REG_SPLIT_SYNC_RD_FANMODE, REG_SPLIT_SYNC_WR_FANMODE, &EqualRead1[2], &OldEqualRead1[2], &EqualRead2[2], &OldEqualRead2[2], &Room[Address].DiagWrite.FanMode);	
	
	//---------------------------	
	// SetPoint F
	//---------------------------
	Syncro2Diag(Address, DiagnosticSplit.CmdAddress,REG_DIAG_SPLIT_RD_SET_SETPOINT_F, REG_DIAG_SPLIT_WR_SET_SETPOINT_F, REG_SPLIT_SYNC_RD_SETPF, REG_SPLIT_SYNC_WR_SETPF, &EqualRead1[3], &OldEqualRead1[3], &EqualRead2[3], &OldEqualRead2[3], &Room[Address].DiagWrite.SetPoint_F);	
	
    
	//---------------------------	
	// UTA Abil
	//---------------------------
	Syncro2Diag(Address, DiagnosticSplit.CmdAddress,REG_DIAG_SPLIT_RD_UTA_MODE, REG_DIAG_SPLIT_WR_UTA_MODE, REG_SPLIT_SYNC_RD_UTA_MODE, REG_SPLIT_SYNC_WR_UTA_MODE, &EqualRead1[8], &OldEqualRead1[8], &EqualRead2[8], &OldEqualRead2[8], &Room[Address].DiagWrite.UTA_Mode);

	//---------------------------	
	// Heater Abil
	//---------------------------
	Syncro2Diag(Address, DiagnosticSplit.CmdAddress,REG_DIAG_SPLIT_RD_HEATER_PWR, REG_DIAG_SPLIT_WR_HEATER_PWR, REG_SPLIT_SYNC_RD_HEATER_PWR, REG_SPLIT_SYNC_WR_HEATER_PWR, &EqualRead1[13], &OldEqualRead1[13], &EqualRead2[13], &OldEqualRead2[13], &Room[Address].DiagWrite.Heater_Pwr);	

#if (K_AbilBridge2==1)  			    
	//---------------------------	
	// Define Mode
	//---------------------------
	Syncro2Value(k_Split_Master_Add, REG_SPLIT_RD_MODE, REG_SPLIT_WR_MODE, REG_BRIDGE2_DIAG_SPLIT_WRITE_MODE, &EqualRead1[4], &OldEqualRead1[4], &EqualRead2[4], &OldEqualRead2[4], &Bridge[1].SetRoom.DefMode);

	//---------------------------	
	// Eco Mode Limit
	//---------------------------
	Syncro2Value(k_Split_Master_Add, REG_SPLIT_SYNC_RD_PWRECOM, REG_SPLIT_SYNC_WR_PWRECOM, REG_BRIDGE2_ECO_MODE_PWR_LIMIT, &EqualRead1[5], &OldEqualRead1[5], &EqualRead2[5], &OldEqualRead2[5], &Bridge[1].SetRoom.Power_EcoMode);

	//---------------------------	
	// Eco Mode Enable
	//---------------------------
	Syncro2Value(k_Split_Master_Add, REG_SPLIT_SYNC_RD_ECOMEN, REG_SPLIT_SYNC_WR_ECOMEN, REG_BRIDGE2_ECO_MODE_ENABLE, &EqualRead1[6], &OldEqualRead1[6], &EqualRead2[6], &OldEqualRead2[6], &Bridge[1].SetRoom.EcoModeEnable);

   //---------------------------	
	// SYSTEM DISABLE
	//---------------------------
	Syncro2Value(k_Split_Master_Add, REG_SPLIT_RD_SYSTEM_DISABLE, REG_SPLIT_WR_SYSTEM_DISABLE, REG_BRIDGE2_SYSTEM_DISABLE, &EqualRead1[12], &OldEqualRead1[12], &EqualRead2[12], &OldEqualRead2[12], &Bridge[1].EngineBox.SystemDisable);
     
	//---------------------------	
	// UTA Power
	//---------------------------
	//Syncro2Value(k_Split_Master_Add, REG_SPLIT_RD_UTA_POWER, REG_SPLIT_WR_UTA_POWER, REG_BRIDGE2_UTA_WRITE_PWR, &EqualRead1[7], &OldEqualRead1[7], &EqualRead2[7], &OldEqualRead2[7], &Syncro.SetUta.PowerOn);

	//---------------------------	
	// UTA Mode
	//---------------------------
	//Syncro2Value(k_Split_Master_Add, REG_SPLIT_RD_UTA_MODE, REG_SPLIT_WR_UTA_MODE, REG_BRIDGE2_UTA_WRITE_MODE, &EqualRead1[8], &OldEqualRead1[8], &EqualRead2[8], &OldEqualRead2[8], &Bridge[1].SetUta.DefMode);

	//---------------------------	
	// UTA SetPoint
	//---------------------------
	//Syncro2Value(k_Split_Master_Add, REG_SPLIT_RD_UTA_SP_ROOM, REG_SPLIT_WR_UTA_SP_ROOM, REG_BRIDGE2_UTA_WRITE_SP_ROOM, &EqualRead1[9], &OldEqualRead1[9], &EqualRead2[9], &OldEqualRead2[9], &Bridge[1].SetUta.SetPoint);

	//---------------------------	
	// UTA Humidity Set
	//---------------------------
	//Syncro2Value(k_Split_Master_Add, REG_SPLIT_RD_UTA_SP_HUMI, REG_SPLIT_WR_UTA_SP_HUMI, REG_BRIDGE2_UTA_WRITE_SP_HUMI, &EqualRead1[10], &OldEqualRead1[10], &EqualRead2[10], &OldEqualRead2[10], &Bridge[1].SetUta.SetHumi);

	//---------------------------	
	// UTA Fan Speed
	//---------------------------
	//Syncro2Value(k_Split_Master_Add, REG_SPLIT_RD_UTA_FAN_SPEED, REG_SPLIT_WR_UTA_FAN_SPEED, REG_BRIDGE2_UTA_WRITE_FAN_SPEED, &EqualRead1[11], &OldEqualRead1[11], &EqualRead2[11], &OldEqualRead2[11], &Syncro.SetUta.FanSpeed);
#endif
    
    OtherSplitSend();   //Scrivo inizializzazione evenualmente arrivata da altro split
    
#if (K_AbilBridge2==1)
    Bridge2SplitSend();   //Scrivo inizializzazione evenualmente arrivata da bridge
#endif
}

void Syncro2Value(int Address, int RegRd, int RegWr, int RegBridge, int * App1, int * OldApp1, int * App2, int * OldApp2, int * NewValue)
{	
#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    

    
    if(Room[Address].OnLine)
        Room[Address].OffLine = FuncReadReg (Address, RegRd, &(*App1), Room[Address].OffLine);		

	if((*OldApp1)!=(*App1))	
	{
   		
    #if(K_HiSpeedU1ModBUS==1)            
            Change_Protocol_BUSM(K_ModBus_Baud115200);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
    #else
            Change_Protocol_BUSM(K_ModBus_Baud57600);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
    #endif        
		InsertReg(1, (*App1));		
		Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, RegBridge, 1, MaxModReadWait, MaxModReadRetries);

		(*NewValue) = (*App1);		
		(*OldApp1) = (*App1);
	}   
	else if((*NewValue)!=(*OldApp2)&&Bridge[1].OffLine==0)	
	{
		Room[Address].OffLine = FuncWriteReg (Address, RegWr, (*NewValue), Room[Address].OffLine);					// REG_SPLIT_STATUS
		(*OldApp2) = (*NewValue);
	}
	//(*OldApp1) = (*App1);
	//(*OldApp2) = NewValue;
}

void Syncro2Diag(int MasAddress, int SplitAddress,int RegRd, int RegWr, int RegSyncRD, int RegSyncWR, int * App1, int * OldApp1, int * App2, int * OldApp2, int * NewValue)
{
	
	int Appoggio1 = 0;
	int Appoggio2 = 0;
	int NuovoValore = 0;
	
#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    
  
    
 	Room[MasAddress].OffLine = FuncReadReg (MasAddress, RegRd, &(Appoggio1), Room[MasAddress].OffLine);		
	if(Room[MasAddress].OnLine)
        (*App1) = Appoggio1;
	
	if(Room[SplitAddress].OnLine)
	{
		Room[SplitAddress].OffLine = FuncReadReg (SplitAddress, RegSyncRD, &(Appoggio2), Room[SplitAddress].OffLine);		
		(*App2) = Appoggio2;
	}
	if((*App1)!=(*OldApp1))	
	{
		(*NewValue) = (*App1);
	}
	else if((*App2)!=(*OldApp2))
	{
		Appoggio2 = (*App2);
		Room[MasAddress].OffLine = FuncWriteReg (MasAddress, RegWr, (Appoggio2), Room[MasAddress].OffLine);					// REG_SPLIT_STATUS
		
	}
	(*OldApp1) = (*App1);
	(*OldApp2) = (*App2);

	if(Room[SplitAddress].OnLine) 
	{
		NuovoValore = (*NewValue);
		Room[SplitAddress].OffLine = FuncWriteReg (SplitAddress, RegSyncWR, (NuovoValore), Room[SplitAddress].OffLine);					// REG_SPLIT_STATUS
	}
}

void CheckAllRangeValue(void)
{
	// Inserire tutte le variabili da inviare agli slave (Broadcast) di cui si vuole controllare (ed eventualmente correggere) il valore
	/*
	CheckRangeValue(&Reinit.FromEngBox.SetPoint, 1800, 3000, 2500);	
	CheckRangeValue(&Reinit.FromEngBox.FanSpeed, 0, 9, 9);					// Fan Speed 0..9 (9=Auto)
	CheckRangeValue(&Reinit.FromEngBox.Power, 0, 1, 0);						// Power On/Off 0..1 (0=Off))
	CheckRangeValue(&Reinit.FromEngBox.SetPoint_F, 1800, 3000, 2500);		// SetPF = ??..??
	CheckRangeValue(&Reinit.FromEngBox.DefMode, 0, 16, 0);					// DefMode = 0..16 (0=Off)
	CheckRangeValue(&Reinit.FromEngBox.TrigInit, 0, 1, 0);					// TrigInit = 0..1 (0=NoInit)
	*/
	//CheckRangeValue(&Reinit.FromEngBox.TrigInit, 0, 1, 0);					// TrigInit = 0..1 (0=NoInit)

}

// Controlla il valore della variabile "Var" e se fuori limiti corregge con "defVal"
void CheckRangeValue(int * Var, int minVal, int maxVal, int defVal)
{
	
	if((*Var) > maxVal || (*Var) < minVal)
	{
		(*Var) = defVal;
	}
	
}
/*
void RefreshOutPutMasterRoom(void)
{
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ENGINECOMPSPEED, UTA_H.EngineCompSpeed, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ENGINEWATERPUMP, UTA_H.EngineFanSpeed, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ENGINEREQPRESSURE, UTA_H.EngineReqPressure, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ENGINETEMPOUT, UTA_H.EngineTempOut, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ENGINETEMPIN, UTA_H.EngineTempIn, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ENGINETEMPAIR, UTA_H.EngineTempAir, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ENGINETEMPBATT, UTA_H.EngineTempBatt, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ENGINETEMPGAS, UTA_H.EngineTempGas, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ENGINETEMPLIQ, UTA_H.EngineTempLiq, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ENGINEPRESSGAS, UTA_H.EnginePressGas, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ENGINEPRESSLIQ, UTA_H.EnginePressLiq, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ENGINEBOXERR1, UTA_H.EngineBoxErr1, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ENGINEBOXPERSERR1, UTA_H.EngineBoxPersErr1, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ENGINEBOXSPLITERR1, UTA_H.EngineBoxSplitErr1, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ABSORPTION, UTA_H.Absorption, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_CUMULATIVEALARM, UTA_H.CumulativeAlarm, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_SPLIT_LIQUIDTEMP, UTA_H.Split.LiquidTemp, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_SPLIT_GASTEMP, UTA_H.Split.GasTemp, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_SPLIT_AIRINTEMP, UTA_H.Split.AirInTemp, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_SPLIT_AITOUTTEMP, UTA_H.Split.AitOutTemp, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_SPLIT_EXP_VALVE, UTA_H.Split.Exp_Valve, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_SPLIT_HUMI, UTA_H.Split.Humi, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_SPLIT_REQAIRTEMP, UTA_H.Split.ReqAirTemp, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_SPLIT_REQGASPRESS, UTA_H.Split.ReqGasPress, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_SPLIT_SUPERHEAT, UTA_H.Split.Superheat, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_SPLIT_HW_VER, UTA_H.Split.HW_Ver, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_SPLIT_FW_VER_HW, UTA_H.Split.FW_Ver_HW, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_SPLIT_FW_VER_FW, UTA_H.Split.FW_Ver_FW, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_SPLIT_FW_REV_FW, UTA_H.Split.FW_Rev_FW, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_HW_VER, UTA_H.HW_Ver, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_FW_VER_HW, UTA_H.FW_Ver_HW, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_FW_VER_FW, UTA_H.FW_Ver_FW, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_FW_REV_FW, UTA_H.FW_Rev_FW, Room[1].OffLine);
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_UTASP_ON, UTA_H.UTASP_On, Room[1].OffLine); 
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_COMP_STATUS, UTA_H.StatoCompressore, Room[1].OffLine); 
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ENGINE_BOX_ID_MAS_SPLIT, UTA_H.IdMasterSplit, Room[1].OffLine); 
    Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_UTAH_ENGINE_MODE, UTA_H.Mode, Room[1].OffLine); 
	
	
     
}
*/

void SendWorkingSplitData(unsigned int index)
{
    /*
	static index=0;
	int found=0;
	
	while(!found)
	{
		if(++index>=Real_Max_Room) 
			index=0;
		if(Room[index].Enable)
			found=1;
	}
	*/
	
	switch(index)
	{
		case 1:
            //Parametri che vann giu comunque allo slave n.1 indipendentemente se abilitato o meno l'init.
			if(!Room[index].OffLine) 
			{
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_INVERTER_SUPPLY, K_Supply_Inverter, Room[index].OffLine);
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DOUBLE_PUMP_ABIL, ((K_AbilPumpInverter<<1) +  K_Abil_Double_Pump), Room[index].OffLine);
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MODE_ABIL, K_AbilSplitMode, Room[index].OffLine);

                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_RD_ABIL_VALVE_ONDEMAND, K_AbilValveOnDemand, Room[index].OffLine);
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_RD_ABIL_SCORE_ONDEMAND, K_AbilScoreOnDemand, Room[index].OffLine);
            }            
#if(K_AbilSendData_Room1 == 1)	
			if(!Room[index].OffLine) 
			{
 
#if(K_AbilDinamicSuperHeat_Room1!=0)
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room1, Room[index].OffLine);					
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);					
#endif              
#ifdef K_SuperHeat_SP_Room1 
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room1, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room1               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room1, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room1
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room1, Room[index].OffLine);	
#endif                
#endif
#ifdef K_MinPressCool_SP_Room1               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room1, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room1               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room1, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room1               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room1, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room1               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room1, Room[index].OffLine);
#endif                
#ifdef K_PerformanceLossBattery_SP_Room1               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room1, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room1               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room1, Room[index].OffLine);	
#endif                

#ifdef K_Temp_Amb_Mid_Heating_Room1
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room1, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room1                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room1, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room1                
#ifdef K_ValveLim_PercMaxOpenValve_Room1 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room1,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room1,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room1,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room1,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room1,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room1,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room1,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room1,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room1,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room1,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room1,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room1,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room1,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room1,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room1
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room1;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room1,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room1,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room1,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room1,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room1,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room1,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room1,	Room[index].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room1,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room1,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room1,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room1,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room1,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room1,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room1,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room1,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room1,	Room[index].OffLine);	      
#endif
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room1,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room1,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room1
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room1,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room1                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room1,	Room[index].OffLine);	      
#endif                
                
#ifdef K_Extractor_Number_Room1                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room1,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room1                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room1,	Room[index].OffLine);
#endif                
			}
#endif
		break;
		case 2:
#if(K_AbilSendData_Room2 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room2!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room2, Room[index].OffLine);					
#else
					Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);					
#endif              
#ifdef K_SuperHeat_SP_Room2 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room2, Room[index].OffLine);					
#endif

#ifdef K_WorkPressCool_SP_Room2               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room2, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room2
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room2, Room[index].OffLine);	
#endif                 
#endif
#ifdef K_MinPressCool_SP_Room2               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room2, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room2               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room2, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room2               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room2, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room2               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room2, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room2               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room2, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room2               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room2, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room2
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room2, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room2                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room2, Room[index].OffLine); 
#endif                 
                
                
// Invio dati personalizzati per gestione valvola Room2                
#ifdef K_ValveLim_PercMaxOpenValve_Room2 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room2,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room2,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room2,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room2,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room2,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room2,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room2,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room2,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room2,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room2,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room2,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room2,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room2,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room2,	Room[index].OffLine);	      
#endif  		
#ifdef K_ValveON_Demand_Priority_Room2
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room2;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);                
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room2,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room2,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room2,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room2,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room2,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room2,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room2,	Room[index].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room2,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room2,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room2,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room2,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room2,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room2,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room2,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room2,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room2,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room2,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room2,	Room[index].OffLine);	      
#endif                
#ifdef K_NetB_LCD_Intensity_Room2
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room2,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room2                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room2,	Room[index].OffLine);	      
#endif                
                
#ifdef K_Extractor_Number_Room2                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room2,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room2                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room2,	Room[index].OffLine);
#endif                

		}
#endif
		break;
		case 3:
#if(K_AbilSendData_Room3 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room3!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room3, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);					
#endif              
#ifdef K_SuperHeat_SP_Room3 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room3, Room[index].OffLine);
#endif
                
#ifdef K_WorkPressCool_SP_Room3               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room3, Room[index].OffLine);
#ifndef K_MinPressCool_SP_Room3
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room3, Room[index].OffLine);	
#endif                 
#endif
#ifdef K_MinPressCool_SP_Room3               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room3, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room3               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room3, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room3               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room3, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room3               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room3, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room3               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room3, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room3               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room3, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room3
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room3, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room3                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room3, Room[index].OffLine); 
#endif                 
                
                
// Invio dati personalizzati per gestione valvola Room1                
#ifdef K_ValveLim_PercMaxOpenValve_Room3 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room3,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room3,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room3,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room3,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room3,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room3,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room3,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room3,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room3,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room3,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room3,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room3,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room3,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room3,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room3
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room3;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room3,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room3,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room3,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room3,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room3,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room3,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room3,	Room[index].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room3,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room3,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room3,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room3,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room3,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room3,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room3,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room3,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room3,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room3,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room3,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room3,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room3
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room3,	Room[index].OffLine);	      
#endif                
	
#ifdef K_Extractor_Number_Room3                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room3,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room3                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room3,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 4:
#if(K_AbilSendData_Room4 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room4!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room4, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);					
#endif              
#ifdef K_SuperHeat_SP_Room4 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room4, Room[index].OffLine);
#endif
                
#ifdef K_WorkPressCool_SP_Room4               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room4, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room4
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room4, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room4               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room4, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room4               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room4, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room4               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room4, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room4               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room4, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room4               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room4, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room4               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room4, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room4
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room4, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room4                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room4, Room[index].OffLine); 
#endif                 
                
                
// Invio dati personalizzati per gestione valvola Room4                
#ifdef K_ValveLim_PercMaxOpenValve_Room4 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room4,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room4,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room4,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room4,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room4,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room4,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room4,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room4,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room4,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room4,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room4,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room4,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room4,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room4,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room4
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room4;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room4,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room4,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room4,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room4,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room4,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room4,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room4,	Room[index].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room4,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room4,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room4,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room4,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room4,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room4,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room4,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room4,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room4,	Room[index].OffLine);	      
#endif
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room4,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room4,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room4,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room4
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room4,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room4                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room4,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room4                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room4,	Room[index].OffLine);
#endif                
	
 			}
#endif
		break;
		case 5:
#if(K_AbilSendData_Room5 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room5!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room5, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);					
#endif              
#ifdef K_SuperHeat_SP_Room5 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room5, Room[index].OffLine);
#endif
                
#ifdef K_WorkPressCool_SP_Room5               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room5, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room5
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room5, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room5               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room5, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room5               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room5, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room5               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room5, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room5               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room5, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room5               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room5, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room5               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room5, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room5
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room5, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room5                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room5, Room[index].OffLine); 
#endif                 
                
                
// Invio dati personalizzati per gestione valvola Room5                
#ifdef K_ValveLim_PercMaxOpenValve_Room5 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room5,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room5,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room5,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room5,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room5,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room5,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room5,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room5,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room5,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room5,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room5,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room5,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room5,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room5,	Room[index].OffLine);	      
#endif
#ifdef K_ValveON_Demand_Priority_Room5
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room5;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
  				
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room5,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room5,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room5,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room5,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room5,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room5,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room5,	Room[index].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room5,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room5,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room5,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room5,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room5,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room5,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room5,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room5,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room5,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room5,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room5,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room5,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room5
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room5,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room5                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room5,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room5                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room5,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 6:
#if(K_AbilSendData_Room6 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room6!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room6, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);						
#endif              
#ifdef K_SuperHeat_SP_Room6 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room6, Room[index].OffLine);
#endif
                
#ifdef K_WorkPressCool_SP_Room6               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room6, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room6
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room6, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room6               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room6, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room6               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room6, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room6               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room6, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room6               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room6, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room6               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room6, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room6               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room6, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room6
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room6, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room6                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room6, Room[index].OffLine); 
#endif                 
                
                
// Invio dati personalizzati per gestione valvola Room6                
#ifdef K_ValveLim_PercMaxOpenValve_Room6 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room6,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room6,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room6,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room6,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room6,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room6,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room6,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room6,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room6,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room6,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room6,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room6,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room6,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room6,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room6
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room6;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room6,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room6,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room6,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room6,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room6,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room6,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room6,	Room[in5ex].OffLine);	      
#endif  				
                                
#ifdef K_FanSpeedOff_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room6,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room6,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room6,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room6,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room6,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room6,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room6,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room6,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room6,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room6,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room6,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room6,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room6
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room6,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room6                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room6,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room6                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room6,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 7:
#if(K_AbilSendData_Room7 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room7!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room7, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room7 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room7, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room7               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room7, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room7
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room7, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room7               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room7, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room7               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room7, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room7               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room7, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room7               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room7, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room7               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room7, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room7               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room7, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room7
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room7, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room7                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room7, Room[index].OffLine); 
#endif                 
                
                
// Invio dati personalizzati per gestione valvola Room7                
#ifdef K_ValveLim_PercMaxOpenValve_Room7 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room7,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room7,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room7,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room7,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room7,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room7,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room7,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room7,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room7,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room7,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room7,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room7,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room7,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room7,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room7
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room7;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room7,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room7,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room7,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room7,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room7,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room7,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room7,	Room[in5e5].OffLine);	      
#endif  				
                                              
#ifdef K_FanSpeedOff_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room7,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room7,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room7,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room7,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room7,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room7,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room7,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room7,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room7,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room7,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room7,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room7,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room7
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room7,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room7                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room7,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room7                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room7,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 8:
#if(K_AbilSendData_Room8 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room8!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room8, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room8 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room8, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room8               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room8, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room8
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room8, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room8               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room8, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room8               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room8, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room8               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room8, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room8               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room8, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room8               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room8, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room8               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room8, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room8
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room8, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room8                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room8, Room[index].OffLine); 
#endif                 
              
                
// Invio dati personalizzati per gestione valvola Room8                
#ifdef K_ValveLim_PercMaxOpenValve_Room8 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room8,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room8,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room8,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room8,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room8,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room8,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room8,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room8,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room8,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room8,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room8,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room8,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room8,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room8,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room8
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room8;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room8,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room8,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room8,	Room[index].OffLine);	      
#endif  				
                
#ifdef K_Min_Current_Fan_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room8,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room8,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room8,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room8,	Room[in5e5].OffLine);	      
#endif  				
                                              
#ifdef K_FanSpeedOff_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room8,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room8,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room8,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room8,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room8,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room8,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room8,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room8,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room8,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room8,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room8,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room8,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room8
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room8,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room8                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room8,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room8                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room8,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 9:
#if(K_AbilSendData_Room9 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room9!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room9, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room9 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room9, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room9               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room9, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room9
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room9, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room9               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room9, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room9               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room9, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room9               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room9, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room9               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room9, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room9               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room9, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room9               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room9, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room9
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room9, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room9                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room9, Room[index].OffLine); 
#endif                 
               
                
// Invio dati personalizzati per gestione valvola Room9                
#ifdef K_ValveLim_PercMaxOpenValve_Room9 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room9,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room9,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room9,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room9,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room9,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room9,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room9,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room9,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room9,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room9,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room9,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room9,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room9,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room9,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room9
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room9;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room9,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room9,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room9,	Room[index].OffLine);	      
#endif  				
                
#ifdef K_Min_Current_Fan_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room9,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room9,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room9,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room9,	Room[in5e5].OffLine);	      
#endif  				
                                              
#ifdef K_FanSpeedOff_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room9,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room9,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room9,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room9,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room9,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room9,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room9,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room9,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room9,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room9,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room9,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room9,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room9
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room9,	Room[index].OffLine);
#endif                


#ifdef K_Extractor_Number_Room9                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room9,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room9                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room9,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 10:
#if(K_AbilSendData_Room10 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room10!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room10, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room10 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room10, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room10               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room10, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room10
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room10, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room10               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room10, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room10               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room10, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room10               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room10, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room10               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room10, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room10               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room10, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room10               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room10, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room10
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room10, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room10                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room10, Room[index].OffLine); 
#endif                 
                
                
// Invio dati personalizzati per gestione valvola Room10                
#ifdef K_ValveLim_PercMaxOpenValve_Room10 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room10,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room10,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room10,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room10,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room10,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room10,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room10,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room10,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room10,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room10,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room10,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room10,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room10,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room10,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room10
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room10;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room10, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room10,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room10,	Room[index].OffLine);	      
#endif  				
                
#ifdef K_Min_Current_Fan_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room10,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room10,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room10,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room10,	Room[in5e5].OffLine);	      
#endif  				
                                              
#ifdef K_FanSpeedOff_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room10,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room10,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room10,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room10,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room10,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room10,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room10,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room10,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room10,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room10,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room10,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room10,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room10
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room10,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room10                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room10,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room10                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room10,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 11:
#if(K_AbilSendData_Room11 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room11!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room11, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room11 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room11, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room11               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room11, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room11
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room11, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room11               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room11, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room11               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room11, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room11               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room11, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room11               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room11, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room11               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room11, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room11               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room11, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room11
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room11, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room11                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room11, Room[index].OffLine); 
#endif                 
               
                
// Invio dati personalizzati per gestione valvola Room11                
#ifdef K_ValveLim_PercMaxOpenValve_Room11 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room11,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room11,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room11,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room11,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room11,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room11,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room11,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room11,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room11,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room11,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room11,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room11,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room11,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room11,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room11
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room11;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room11, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room11,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room11,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room11,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room11,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room11,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room11,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room11,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room11,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room11,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room11,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room11,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room11,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room11,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room11,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room11,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room11,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room11,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room11,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room11
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room11,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room11                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room11,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room11                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room11,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 12:
#if(K_AbilSendData_Room12 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room12!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room12, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room12 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room12, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room12               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room12, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room12
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room12, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room12               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room12, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room12               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room12, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room12               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room12, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room12               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room12, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room12               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room12, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room12               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room12, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room12
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room12, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room12                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room12, Room[index].OffLine); 
#endif                 
                
                
// Invio dati personalizzati per gestione valvola Room12                
#ifdef K_ValveLim_PercMaxOpenValve_Room12 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room12,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room12,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room12,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room12,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room12,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room12,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room12,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room12,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room12,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room12,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room12,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room12,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room12,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room12,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room12
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room12;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room12, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room12,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room12,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room12,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room12,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room12,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room12,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room12,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room12,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room12,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room12,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room12,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room12,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room12,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room12,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room12,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room12,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room12,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room12,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room12
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room12,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room12                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room12,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room12                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room12,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 13:
#if(K_AbilSendData_Room13 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room13!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room13, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room13 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room13, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room13               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room13, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room13
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room13, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room13               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room13, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room13               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room13, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room13               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room13, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room13               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room13, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room13               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room13, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room13               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room13, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room13
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room13, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room13                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room13, Room[index].OffLine); 
#endif                 
             
                
// Invio dati personalizzati per gestione valvola Room13                
#ifdef K_ValveLim_PercMaxOpenValve_Room13 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room13,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room13,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room13,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room13,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room13,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room13,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room13,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room13,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room13,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room13,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room13,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room13,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room13,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room13,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room13
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room13;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room13, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room13,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room13,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room13,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room13,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room13,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room13,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room13,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room13,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room13,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room13,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room13,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room13,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room13,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room13,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room13,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room13,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room13,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room13,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room13
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room13,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room13                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room13,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room13                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room13,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 14:
#if(K_AbilSendData_Room14 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room14!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room14, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room14 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room14, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room14               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room14, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room14
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room14, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room14               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room14, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room14               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room14, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room14               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room14, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room14               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room14, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room14               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room14, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room14               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room14, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room14
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room14, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room14                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room14, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room14                
#ifdef K_ValveLim_PercMaxOpenValve_Room14 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room14,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room14,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room14,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room14,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room14,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room14,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room14,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room14,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room14,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room14,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room14,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room14,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room14,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room14,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room14
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room14;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room14, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room14,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room14,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room14,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room14,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room14,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room14,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room14,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room14,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room14,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room14,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room14,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room14,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room14,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room14,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room14,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room14,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room14,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room14,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room14
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room14,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room14                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room14,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room14                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room14,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 15:
#if(K_AbilSendData_Room15 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room15!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room15, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room15 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room15, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room15               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room15, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room15
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room15, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room15               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room15, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room15               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room15, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room15               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room15, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room15               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room15, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room15               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room15, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room15               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room15, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room15
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room15, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room15                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room15, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room15                
#ifdef K_ValveLim_PercMaxOpenValve_Room15 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room15,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room15,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room15,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room15,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room15,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room15,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room15,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room15,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room15,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room15,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room15,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room15,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room15,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room15,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room15
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room15;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room15, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room15,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room15,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room15,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room15,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room15,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room15,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room15,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room15,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room15,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room15,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room15,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room15,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room15,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room15,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room15,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room15,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room15,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room15,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room15
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room15,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room15                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room15,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room15                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room15,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 16:
#if(K_AbilSendData_Room16 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room16!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room16, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room16 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room16, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room16               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room16, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room16
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room16, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room16               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room16, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room16               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room16, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room16               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room16, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room16               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room16, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room16               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room16, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room16               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room16, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room16
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room16, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room16                       
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room16, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room16                
#ifdef K_ValveLim_PercMaxOpenValve_Room16 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room16,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room16,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room16,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room16,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room16,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room16,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room16,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room16,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room16,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room16,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room16,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room16,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room16,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room16,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room16
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room16;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room16, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room16,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room16,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room16,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room16,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room16,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room16,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room16,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room16,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room16,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room16,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room16,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room16,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room16,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room16,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room16,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room16,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room16,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room16,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room16
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room16,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room16                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room16,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room16                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room16,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 17:
#if(K_AbilSendData_Room17 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room17!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room17, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room17 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room17, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room17               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room17, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room17
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room17, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room17               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room17, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room17               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room17, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room17               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room17, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room17               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room17, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room17               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room17, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room17               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room17, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room17
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room17, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room17                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room17, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room17                
#ifdef K_ValveLim_PercMaxOpenValve_Room17 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room17,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room17,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room17,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room17,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room17,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room17,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room17,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room17,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room17,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room17,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room17,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room17,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room17,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room17,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room17
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room17;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room17, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room17,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room17,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room17,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room17,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room17,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room17,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room17,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room17,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room17,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room17,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room17,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room17,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room17,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room17,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room17,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room17,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room17,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room17,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room17
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room17,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room17                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room17,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room17                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room17,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 18:
#if(K_AbilSendData_Room18 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room18!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room18, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room18 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room18, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room18               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room18, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room18
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room18, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room18               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room18, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room18               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room18, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room18               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room18, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room18               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room18, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room18               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room18, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room18               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room18, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room18
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room18, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room18                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room18, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room18                
#ifdef K_ValveLim_PercMaxOpenValve_Room18 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room18,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room18,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room18,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room18,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room18,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room18,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room18,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room18,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room18,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room18,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room18,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room18,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room18,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room18,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room18
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room18;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room18, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room18,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room18,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room18,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room18,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room18,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room18,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room18,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room18,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room18,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room18,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room18,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room18,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room18,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room18,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room18,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room18,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room18,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room18,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room18
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room18,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room18                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room18,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room18                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room18,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 19:
#if(K_AbilSendData_Room19 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room19!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room19, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room19 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room19, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room19               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room19, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room19
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room19, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room19               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room19, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room19               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room19, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room19               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room19, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room19               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room19, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room19               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room19, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room19               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room19, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room19
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room19, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room19                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room19, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room19                
#ifdef K_ValveLim_PercMaxOpenValve_Room19 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room19,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room19,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room19,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room19,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room19,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room19,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room19,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room19,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room19,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room19,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room19,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room19,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room19,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room19,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room19
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room19;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room19, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room19,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room19,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room19,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room19,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room19,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room19,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room19,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room19,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room19,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room19,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room19,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room19,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room19,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room19,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room19,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room19,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room19,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room19,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room19
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room19,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room19                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room19,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room19                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room19,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 20:
#if(K_AbilSendData_Room20 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room20!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room20, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);					
#endif              
#ifdef K_SuperHeat_SP_Room20 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room20, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room20               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room20, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room20
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room20, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room20               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room20, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room20               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room20, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room20               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room20, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room20               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room20, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room20               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room20, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room20               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room20, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room20
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room20, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room20                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room20, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room20                
#ifdef K_ValveLim_PercMaxOpenValve_Room20 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room20,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room20,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room20,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room20,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room20,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room20,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room20,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room20,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room20,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room20,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room20,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room20,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room20,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room20,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room20
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room20;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room20, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room20,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room20,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room20,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room20,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room20,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room20,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room20,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room20,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room20,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room20,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room20,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room20,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room20,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room20,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room20,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room20,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room20,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room20,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room20
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room20,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room20                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room20,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room20                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room20,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 21:
#if(K_AbilSendData_Room21 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room21!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room21, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room21 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room21, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room21               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room21, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room21
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room21, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room21               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room21, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room21               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room21, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room21               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room21, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room21               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room21, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room21               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room21, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room21               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room21, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room21
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room21, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room21                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room21, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room21                
#ifdef K_ValveLim_PercMaxOpenValve_Room21 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room21,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room21,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room21,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room21,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room21,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room21,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room21,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room21,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room21,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room21,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room21,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room21,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room21,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room21,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room21
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room21;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room21, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room21,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room21,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room21,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room21,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room21,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room21,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room21,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room21,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room21,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room21,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room21,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room21,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room21,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room21,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room21,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room21,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room21,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room21,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room21
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room21,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room21                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room21,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room21                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room21,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 22:
#if(K_AbilSendData_Room22 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room22!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room22, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room22 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room22, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room22               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room22, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room22
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room22, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room22               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room22, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room22               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room22, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room22               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room22, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room22               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room22, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room22               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room22, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room22               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room22, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room22
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room22, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room22                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room22, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room22                
#ifdef K_ValveLim_PercMaxOpenValve_Room22 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room22,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room22,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room22,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room22,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room22,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room22,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room22,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room22,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room22,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room22,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room22,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room22,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room22,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room22,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room22
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room22;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room22, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room22,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room22,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room22,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room22,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room22,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room22,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room22,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room22,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room22,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room22,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room22,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room22,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room22,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room22,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room22,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room22,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room22,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room22,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room22
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room22,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room22                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room22,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room22                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room22,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 23:
#if(K_AbilSendData_Room23 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room23!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room23, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room23 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room23, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room23               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room23, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room23
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room23, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room23               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room23, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room23               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room23, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room23               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room23, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room23               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room23, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room23               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room23, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room23               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room23, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room23
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room23, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room23                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room23, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room23                
#ifdef K_ValveLim_PercMaxOpenValve_Room23 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room23,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room23,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room23,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room23,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room23,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room23,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room23,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room23,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room23,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room23,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room23,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room23,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room23,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room23,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room23
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room23;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room23, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room23,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room23,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room23,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room23,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room23,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room23,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room23,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room23,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room23,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room23,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room23,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room23,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room23,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room23,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room23,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room23,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room23,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room23,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room23
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room23,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room23                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room23,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room23                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room23,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 24:
#if(K_AbilSendData_Room24 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room24!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room24, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room24 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room24, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room24               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room24, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room24
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room24, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room24               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room24, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room24               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room24, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room24               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room24, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room24               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room24, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room24               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room24, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room24               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room24, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room24
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room24, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room24                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room24, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room24                
#ifdef K_ValveLim_PercMaxOpenValve_Room24 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room24,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room24,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room24,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room24,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room24,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room24,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room24,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room24,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room24,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room24,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room24,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room24,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room24,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room24,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room24
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room24;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room24, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room24,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room24,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room24,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room24,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room24,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room24,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room24,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room24,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room24,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room24,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room24,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room24,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room24,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room24,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room24,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room24,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room24,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room24,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room24
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room24,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room24                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room24,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room24                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room24,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 25:
#if(K_AbilSendData_Room25 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room25!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room25, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room25 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room25, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room25               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room25, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room25
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room25, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room25               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room25, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room25               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room25, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room25               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room25, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room25               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room25, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room25               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room25, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room25               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room25, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room25
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room25, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room25                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room25, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room25                
#ifdef K_ValveLim_PercMaxOpenValve_Room25 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room25,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room25,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room25,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room25,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room25,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room25,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room25,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room25,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room25,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room25,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room25,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room25,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room25,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room25,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room25
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room25;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room25, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room25,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room25,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room25,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room25,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room25,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room25,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room25,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room25,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room25,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room25,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room25,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room25,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room25,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room25,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room25,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room25,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room25,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room25,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room25
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room25,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room25                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room25,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room25                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room25,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 26:
#if(K_AbilSendData_Room26 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room26!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room26, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room26 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room26, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room26               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room26, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room26
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room26, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room26               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room26, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room26               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room26, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room26               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room26, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room26               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room26, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room26               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room26, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room26               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room26, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room26
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room26, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room26                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room26, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room26                
#ifdef K_ValveLim_PercMaxOpenValve_Room26 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room26,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room26,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room26,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room26,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room26,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room26,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room26,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room26,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room26,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room26,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room26,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room26,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room26,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room26,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room26
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room26;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room26, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room26,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room26,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room26,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room26,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room26,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room26,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room26,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room26,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room26,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room26,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room26,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room26,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room26,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room26,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room26,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room26,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room26,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room26,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room26
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room26,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room26                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room26,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room26                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room26,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 27:
#if(K_AbilSendData_Room27 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room27!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room27, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room27 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room27, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room27               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room27, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room27
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room27, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room27               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room27, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room27               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room27, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room27               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room27, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room27               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room27, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room27               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room27, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room27               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room27, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room27
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room27, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room27                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room27, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room27                
#ifdef K_ValveLim_PercMaxOpenValve_Room27 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room27,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room27,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room27,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room27,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room27,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room27,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room27,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room27,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room27,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room27,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room27,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room27,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room27,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room27,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room27
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room27;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room27, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room27,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room27,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room27,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room27,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room27,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room27,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room27,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room27,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room27,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room27,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room27,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room27,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room27,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room27,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room27,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room27,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room27,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room27,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room27
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room27,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room27                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room27,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room27                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room27,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 28:
#if(K_AbilSendData_Room28 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room28!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room28, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room28 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room28, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room28               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room28, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room28
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room28, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room28               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room28, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room28               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room28, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room28               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room28, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room28               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room28, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room28               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room28, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room28               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room28, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room28
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room28, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room28                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room28, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room28                
#ifdef K_ValveLim_PercMaxOpenValve_Room28 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room28,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room28,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room28,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room28,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room28,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room28,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room28,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room28,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room28,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room28,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room28,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room28,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room28,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room28,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room28
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room28;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room28, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room28,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room28,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room28,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room28,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room28,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room28,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room28,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room28,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room28,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room28,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room28,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room28,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room28,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room28,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room28,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room28,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room28,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room28,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room28
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room28,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room28                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room28,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room28                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room28,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 29:
#if(K_AbilSendData_Room29 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room29!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room29, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room29 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room29, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room29               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room29, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room29
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room29, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room29               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room29, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room29               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room29, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room29               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room29, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room29               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room29, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room29               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room29, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room29               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room29, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room29
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room29, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room29                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room29, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room29                
#ifdef K_ValveLim_PercMaxOpenValve_Room29 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room29,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room29,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room29,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room29,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room29,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room29,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room29,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room29,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room29,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room29,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room29,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room29,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room29,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room29,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room29
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room29;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room29, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room29,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room29,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room29,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room29,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room29,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room29,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room29,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room29,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room29,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room29,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room29,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room29,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room29,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room29,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room29,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room29,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room29,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room29,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room29
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room29,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room29                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room29,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room29                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room29,	Room[index].OffLine);
#endif                

 			}
#endif
		break;
		case 30:
#if(K_AbilSendData_Room30 == 1)	
			if(!Room[index].OffLine) 
			{
#if(K_AbilDinamicSuperHeat_Room30!=0)
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, K_AbilDinamicSuperHeat_Room30, Room[index].OffLine);					
#else
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DINAMIC_SUPERHEAT, 0, Room[index].OffLine);	
#endif              
#ifdef K_SuperHeat_SP_Room30 
                    Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_SUPERHEAT, K_SuperHeat_SP_Room30, Room[index].OffLine);					
#endif
                
#ifdef K_WorkPressCool_SP_Room30               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_COOL, K_WorkPressCool_SP_Room30, Room[index].OffLine);	
#ifndef K_MinPressCool_SP_Room30
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_WorkPressCool_SP_Room30, Room[index].OffLine);	
#endif 
#endif
#ifdef K_MinPressCool_SP_Room30               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_COOL, K_MinPressCool_SP_Room30, Room[index].OffLine);	
#endif
#ifdef K_WorkPressHeat_SP_Room30               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_WORK_PRESS_HEAT, K_WorkPressHeat_SP_Room30, Room[index].OffLine);
#endif
#ifdef K_MinPressHeat_SP_Room30               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_MIN_PRESS_HEAT, K_MinPressHeat_SP_Room30, Room[index].OffLine);
#endif
#ifdef K_TempCorrectSuperHeat_AirOut_Room30               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, K_TempCorrectSuperHeat_AirOut_Room30, Room[index].OffLine);
#endif                                
#ifdef K_PerformanceLossBattery_SP_Room30               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, K_PerformanceLossBattery_SP_Room30, Room[index].OffLine);	
#endif                
#ifdef K_Other_Cabin_Number_Room30               
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_OTHER_CABIN_NUMBER, K_Other_Cabin_Number_Room30, Room[index].OffLine);	
#endif                
#ifdef K_Temp_Amb_Mid_Heating_Room30
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, K_Temp_Amb_Mid_Heating_Room30, Room[index].OffLine); 
#endif 
#ifdef K_Temp_Amb_Mid_Cooling_Room30                        
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, K_Temp_Amb_Mid_Cooling_Room30, Room[index].OffLine); 
#endif                 
                
// Invio dati personalizzati per gestione valvola Room30                
#ifdef K_ValveLim_PercMaxOpenValve_Room30 
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE,		K_ValveLim_PercMaxOpenValve_Room30,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaAperta_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAAPERTA,		K_ValveLim_ValvolaAperta_Room30,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaChiusa_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLACHIUSA,		K_ValveLim_ValvolaChiusa_Room30,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValvolaOff_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLAOFF,			K_ValveLim_ValvolaOff_Room30,			Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_Valve_Min_Go_Off_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF,		K_ValveLim_Valve_Min_Go_Off_Room30,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommCool_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL,	K_ValveLim_ValveLostCommCool_Room30,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveLostCommHot_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT,		K_ValveLim_ValveLostCommHot_Room30,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxCaldo_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXCALDO,		K_ValveLim_AperturaMaxCaldo_Room30,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinCaldo_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINCALDO,		K_ValveLim_AperturaMinCaldo_Room30,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_InitValvolaFreddo_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO,	K_ValveLim_InitValvolaFreddo_Room30,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMaxFreddo_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO,	K_ValveLim_AperturaMaxFreddo_Room30,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_AperturaMinFreddo_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_APERTURAMINFREDDO,	K_ValveLim_AperturaMinFreddo_Room30,		Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMinFreddo_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, K_ValveLim_ValveNoWorkMinFreddo_Room30,	Room[index].OffLine);	       
#endif  				
#ifdef K_ValveLim_ValveNoWorkMaxFreddo_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, K_ValveLim_ValveNoWorkMaxFreddo_Room30,	Room[index].OffLine);	      
#endif  				
#ifdef K_ValveON_Demand_Priority_Room30
				Room[index].ValvePriority = K_ValveON_Demand_Priority_Room30;
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Room[index].ValvePriority,	Room[index].OffLine);
#else
                Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, 0,	Room[index].OffLine);
#endif  				                
                
//Invio dati per velocità ventola
                
#ifdef K_FanVentil_Selection_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_VENTIL_SELECTION, K_FanVentil_Selection_Room30, Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Min_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MIN, K_FanM3h_Min_Room30,	Room[index].OffLine);	      
#endif  				

#ifdef K_FanM3h_Max_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_M3H_MAX, K_FanM3h_Max_Room30,	Room[index].OffLine);	      
#endif  				

#ifdef K_Min_Current_Fan_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MIN_CURRENT_FAN, K_Min_Current_Fan_Room30,	Room[index].OffLine);	      
#endif  				

#ifdef K_Max_Current_Fan_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_MAX_CURRENT_FAN, K_Max_Current_Fan_Room30,	Room[index].OffLine);	      
#endif  				

#ifdef K_Efficiency_Fan_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_EFFICIENCY_FAN, K_Efficiency_Fan_Room30,	Room[index].OffLine);	      
#endif  				

#ifdef K_Supply_Fan_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_SUPPLY_FAN, K_Supply_Fan_Room30,	Room[in5e5].OffLine);	      
#endif  				
                
#ifdef K_FanSpeedOff_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_OFF, K_FanSpeedOff_Room30,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeedNight_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_NIGHT, K_FanSpeedNight_Room30,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed1_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED1, K_FanSpeed1_Room30,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed2_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED2, K_FanSpeed2_Room30,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed3_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED3, K_FanSpeed3_Room30,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed4_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED4, K_FanSpeed4_Room30,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed5_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED5, K_FanSpeed5_Room30,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed6_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED6, K_FanSpeed6_Room30,	Room[index].OffLine);	      
#endif  				
#ifdef K_FanSpeed7_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_PWM_VALUE_SPEED7, K_FanSpeed7_Room30,	Room[index].OffLine);	      
#endif  				
//Invio dati per On-Off timer Ventola in NewStyle caldo
#ifdef K_Fan_New_Style_Hot_Time_On_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, K_Fan_New_Style_Hot_Time_On_Room30,	Room[index].OffLine);	      
#endif
#ifdef K_Fan_New_Style_Hot_Time_Off_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, K_Fan_New_Style_Hot_Time_Off_Room30,	Room[index].OffLine);	      
#endif
#ifdef K_NetB_LCD_Intensity_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_NETB_LCD_INTENSITY, K_NetB_LCD_Intensity_Room30,	Room[index].OffLine);	      
#endif                
#ifdef K_Fan_Current_Limit_Room30
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_FAN_CURRENT_LIMIT, K_Fan_Current_Limit_Room30,	Room[index].OffLine);
#endif                

#ifdef K_Extractor_Number_Room30                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_EXTRACTOR_NUMBER, K_Extractor_Number_Room30,	Room[index].OffLine);
#endif                
#ifdef K_Extractor_DeltaVel_Room30                
				Room[index].OffLine = FuncWriteReg (index, REG_SPLIT_WR_DELTAVEL_EXTRACTOR, K_Extractor_DeltaVel_Room30,	Room[index].OffLine);
#endif                

 			}
#endif
		break;

		
	}
	
	//if(index>=Real_Max_Room) 
	//	index=0;


	
	
}


void MountImageSlaveFlags(void)
{
    EngineBox[0].Split_Flags_Enable_1_16 = 0;
	EngineBox[0].Split_Flags_Enable_1_16 = Room[16].Enable << 15 | Room[15].Enable << 14 | Room[14].Enable << 13 | Room[13].Enable << 12 | 
										   Room[12].Enable << 11 | Room[11].Enable << 10 | Room[10].Enable << 9 | Room[9].Enable << 8 | 
										   Room[8].Enable << 7 | Room[7].Enable << 6 | Room[6].Enable << 5 | Room[5].Enable << 4 | 
										   Room[4].Enable << 3 | Room[3].Enable << 2 | Room[2].Enable << 1 | Room[1].Enable;
    EngineBox[0].Split_Flags_Enable_17_32 = 0;
	EngineBox[0].Split_Flags_Enable_17_32 =Bridge[1].Enable << 15 | /*Room[31].Enable << 14 | */Room[30].Enable << 13 | Room[29].Enable << 12 | 
										   Room[28].Enable << 11 | Room[27].Enable << 10 | Room[26].Enable << 9 | Room[25].Enable << 8 | 
										   Room[24].Enable << 7 | Room[23].Enable << 6 | Room[22].Enable << 5 | Room[21].Enable << 4 | 
										   Room[20].Enable << 3 | Room[19].Enable << 2 | Room[18].Enable << 1 | Room[17].Enable;	
	EngineBox[0].Split_Flags_Offline_1_16 = 0;
	EngineBox[0].Split_Flags_Offline_1_16 =Room[16].OffLine << 15 | Room[15].OffLine << 14 | Room[14].OffLine << 13 | Room[13].OffLine << 12 | 
										   Room[12].OffLine << 11 | Room[11].OffLine << 10 | Room[10].OffLine << 9 | Room[9].OffLine << 8 | 
										   Room[8].OffLine << 7 | Room[7].OffLine << 6 | Room[6].OffLine << 5 | Room[5].OffLine << 4 | 
										   Room[4].OffLine << 3 | Room[3].OffLine << 2 | Room[2].OffLine << 1 | Room[1].OffLine;
    EngineBox[0].Split_Flags_Offline_17_32 = 0;
	EngineBox[0].Split_Flags_Offline_17_32=Bridge[1].PermanentOffLine << 15 | /*Room[31].OffLine << 14 | */Room[30].OffLine << 13 | Room[29].OffLine << 12 | 
										   Room[28].OffLine << 11 | Room[27].OffLine << 10 | Room[26].OffLine << 9 | Room[25].OffLine << 8 | 
										   Room[24].OffLine << 7 | Room[23].OffLine << 6 | Room[22].OffLine << 5 | Room[21].OffLine << 4 | 
										   Room[20].OffLine << 3 | Room[19].OffLine << 2 | Room[18].OffLine << 1 | Room[17].OffLine;
    EngineBox[0].Split_Flags_EnableAndOffline_1_16 = 0;
	EngineBox[0].Split_Flags_EnableAndOffline_1_16 =(Room[16].OffLine&&Room[16].Enable) << 15 | (Room[15].OffLine&&Room[15].Enable) << 14 | (Room[14].OffLine&&Room[14].Enable) << 13 | (Room[13].OffLine&&Room[13].Enable) << 12 | 
										   (Room[12].OffLine&&Room[12].Enable) << 11 | (Room[11].OffLine&&Room[11].Enable) << 10 | (Room[10].OffLine&&Room[10].Enable) << 9 | (Room[9].OffLine&&Room[9].Enable) << 8 | 
										   (Room[8].OffLine&&Room[8].Enable) << 7 | (Room[7].OffLine&&Room[7].Enable) << 6 | (Room[6].OffLine&&Room[6].Enable) << 5 | (Room[5].OffLine&&Room[5].Enable) << 4 | 
										   (Room[4].OffLine&&Room[4].Enable) << 3 | (Room[3].OffLine&&Room[3].Enable) << 2 | (Room[2].OffLine&&Room[2].Enable) << 1 | (Room[1].OffLine&&Room[1].Enable);
    EngineBox[0].Split_Flags_EnableAndOffline_17_32 = 0;
	EngineBox[0].Split_Flags_EnableAndOffline_17_32=(Bridge[1].PermanentOffLine&&Bridge[1].Enable) << 15 | /*(Room[31].OffLine&&Room[31].Enable) << 14 | */(Room[30].OffLine&&Room[30].Enable) << 13 | (Room[29].OffLine&&Room[29].Enable) << 12 | 
										   (Room[28].OffLine&&Room[28].Enable) << 11 | (Room[27].OffLine&&Room[27].Enable) << 10 | (Room[26].OffLine&&Room[26].Enable) << 9 | (Room[25].OffLine&&Room[25].Enable) << 8 | 
										   (Room[24].OffLine&&Room[24].Enable) << 7 | (Room[23].OffLine&&Room[23].Enable) << 6 | (Room[22].OffLine&&Room[22].Enable) << 5 | (Room[21].OffLine&&Room[21].Enable) << 4 | 
										   (Room[20].OffLine&&Room[20].Enable) << 3 | (Room[19].OffLine&&Room[19].Enable) << 2 | (Room[18].OffLine&&Room[18].Enable) << 1 | (Room[17].OffLine&&Room[17].Enable);
    
	EngineBox[0].Split_Flags_Online_1_16 = 0;	
	EngineBox[0].Split_Flags_Online_1_16 = Room[16].OnLine << 15 | Room[15].OnLine << 14 | Room[14].OnLine << 13 | Room[13].OnLine << 12 | 
										   Room[12].OnLine << 11 | Room[11].OnLine << 10 | Room[10].OnLine << 9 | Room[9].OnLine << 8 | 
										   Room[8].OnLine << 7 | Room[7].OnLine << 6 | Room[6].OnLine << 5 | Room[5].OnLine << 4 | 
										   Room[4].OnLine << 3 | Room[3].OnLine << 2 | Room[2].OnLine << 1 | Room[1].OnLine;
    EngineBox[0].Split_Flags_Online_17_32 = 0;
	EngineBox[0].Split_Flags_Online_17_32 =Bridge[1].OnLine << 15 | /*Room[31].OnLine << 14 | */Room[30].OnLine << 13 | Room[29].OnLine << 12 | 
										   Room[28].OnLine << 11 | Room[27].OnLine << 10 | Room[26].OnLine << 9 | Room[25].OnLine << 8 | 
										   Room[24].OnLine << 7 | Room[23].OnLine << 6 | Room[22].OnLine << 5 | Room[21].OnLine << 4 | 
										   Room[20].OnLine << 3 | Room[19].OnLine << 2 | Room[18].OnLine << 1 | Room[17].OnLine;
	EngineBox[0].Split_Flags_Alarm_1_16 = 0;	
	EngineBox[0].Split_Flags_Alarm_1_16 =  Room[16].CumulativeAlarm << 15 | Room[15].CumulativeAlarm << 14 | Room[14].CumulativeAlarm << 13 | Room[13].CumulativeAlarm << 12 | 
										   Room[12].CumulativeAlarm << 11 | Room[11].CumulativeAlarm << 10 | Room[10].CumulativeAlarm << 9 | Room[9].CumulativeAlarm << 8 | 
										   Room[8].CumulativeAlarm << 7 | Room[7].CumulativeAlarm << 6 | Room[6].CumulativeAlarm << 5 | Room[5].CumulativeAlarm << 4 | 
										   Room[4].CumulativeAlarm << 3 | Room[3].CumulativeAlarm << 2 | Room[2].CumulativeAlarm << 1 | Room[1].CumulativeAlarm;
    EngineBox[0].Split_Flags_Alarm_17_32 = 0;
#if (K_AbilMODBCAN==1)     
                                            // N.B: "Room[32].CumulativeAlarm" contiene -> ModbCAN.CANBUS_Activity==0 && ModbCAN.OnLine==1 (vedi Main.c riga 610 circa)
	EngineBox[0].Split_Flags_Alarm_17_32 = Room[32].CumulativeAlarm << 15 | /*Room[31].CumulativeAlarm << 14 | */Room[30].CumulativeAlarm << 13 | Room[29].CumulativeAlarm << 12 | 
										   Room[28].CumulativeAlarm << 11 | Room[27].CumulativeAlarm << 10 | Room[26].CumulativeAlarm << 9 | Room[25].CumulativeAlarm << 8 | 
										   Room[24].CumulativeAlarm << 7 | Room[23].CumulativeAlarm << 6 | Room[22].CumulativeAlarm << 5 | Room[21].CumulativeAlarm << 4 | 
										   Room[20].CumulativeAlarm << 3 | Room[19].CumulativeAlarm << 2 | Room[18].CumulativeAlarm << 1 | Room[17].CumulativeAlarm;
#else
	EngineBox[0].Split_Flags_Alarm_17_32 = /*Room[32].CumulativeAlarm << 15 | Room[31].CumulativeAlarm << 14 | */Room[30].CumulativeAlarm << 13 | Room[29].CumulativeAlarm << 12 | 
										   Room[28].CumulativeAlarm << 11 | Room[27].CumulativeAlarm << 10 | Room[26].CumulativeAlarm << 9 | Room[25].CumulativeAlarm << 8 | 
										   Room[24].CumulativeAlarm << 7 | Room[23].CumulativeAlarm << 6 | Room[22].CumulativeAlarm << 5 | Room[21].CumulativeAlarm << 4 | 
										   Room[20].CumulativeAlarm << 3 | Room[19].CumulativeAlarm << 2 | Room[18].CumulativeAlarm << 1 | Room[17].CumulativeAlarm;

#endif //#if (K_AbilMODBCAN==1)      
				
	
}

void MountImageSlaveModDevFlags(void)
{
    EngineBox[0].Split_ModDev_Flags_Enable_1_16 = 0;
	EngineBox[0].Split_ModDev_Flags_Enable_1_16 = Room[16].ModDev.Enable << 15 | Room[15].ModDev.Enable << 14 | Room[14].ModDev.Enable << 13 | Room[13].ModDev.Enable << 12 | 
										   Room[12].ModDev.Enable << 11 | Room[11].ModDev.Enable << 10 | Room[10].ModDev.Enable << 9 | Room[9].ModDev.Enable << 8 | 
										   Room[8].ModDev.Enable << 7 | Room[7].ModDev.Enable << 6 | Room[6].ModDev.Enable << 5 | Room[5].ModDev.Enable << 4 | 
										   Room[4].ModDev.Enable << 3 | Room[3].ModDev.Enable << 2 | Room[2].ModDev.Enable << 1 | Room[1].ModDev.Enable;
    EngineBox[0].Split_ModDev_Flags_Enable_17_32 = 0;
	EngineBox[0].Split_ModDev_Flags_Enable_17_32 =/*Room[32].ModDev.Enable << 15 | Room[31].ModDev.Enable << 14 | */Room[30].ModDev.Enable << 13 | Room[29].ModDev.Enable << 12 | 
										   Room[28].ModDev.Enable << 11 | Room[27].ModDev.Enable << 10 | Room[26].ModDev.Enable << 9 | Room[25].ModDev.Enable << 8 | 
										   Room[24].ModDev.Enable << 7 | Room[23].ModDev.Enable << 6 | Room[22].ModDev.Enable << 5 | Room[21].ModDev.Enable << 4 | 
										   Room[20].ModDev.Enable << 3 | Room[19].ModDev.Enable << 2 | Room[18].ModDev.Enable << 1 | Room[17].ModDev.Enable;	
	EngineBox[0].Split_ModDev_Flags_Offline_1_16 = 0;
	EngineBox[0].Split_ModDev_Flags_Offline_1_16 = Room[16].ModDev.OffLine << 15 | Room[15].ModDev.OffLine << 14 | Room[14].ModDev.OffLine << 13 | Room[13].ModDev.OffLine << 12 | 
										   Room[12].ModDev.OffLine << 11 | Room[11].ModDev.OffLine << 10 | Room[10].ModDev.OffLine << 9 | Room[9].ModDev.OffLine << 8 | 
										   Room[8].ModDev.OffLine << 7 | Room[7].ModDev.OffLine << 6 | Room[6].ModDev.OffLine << 5 | Room[5].ModDev.OffLine << 4 | 
										   Room[4].ModDev.OffLine << 3 | Room[3].ModDev.OffLine << 2 | Room[2].ModDev.OffLine << 1 | Room[1].ModDev.OffLine;
    EngineBox[0].Split_ModDev_Flags_Offline_17_32 = 0;
	EngineBox[0].Split_ModDev_Flags_Offline_17_32=/*Room[32].ModDev.OffLine << 15 | Room[31].ModDev.OffLine << 14 | */Room[30].ModDev.OffLine << 13 | Room[29].ModDev.OffLine << 12 | 
										   Room[28].ModDev.OffLine << 11 | Room[27].ModDev.OffLine << 10 | Room[26].ModDev.OffLine << 9 | Room[25].ModDev.OffLine << 8 | 
										   Room[24].ModDev.OffLine << 7 | Room[23].ModDev.OffLine << 6 | Room[22].ModDev.OffLine << 5 | Room[21].ModDev.OffLine << 4 | 
										   Room[20].ModDev.OffLine << 3 | Room[19].ModDev.OffLine << 2 | Room[18].ModDev.OffLine << 1 | Room[17].ModDev.OffLine;
    EngineBox[0].Split_ModDev_Flags_EnableAndOffline_1_16 = 0;
	EngineBox[0].Split_ModDev_Flags_EnableAndOffline_1_16 =(Room[16].ModDev.OffLine&&Room[16].ModDev.Enable) << 15 | (Room[15].ModDev.OffLine&&Room[15].ModDev.Enable) << 14 | (Room[14].ModDev.OffLine&&Room[14].ModDev.Enable) << 13 | (Room[13].ModDev.OffLine&&Room[13].ModDev.Enable) << 12 | 
										   (Room[12].ModDev.OffLine&&Room[12].ModDev.Enable) << 11 | (Room[11].ModDev.OffLine&&Room[11].ModDev.Enable) << 10 | (Room[10].ModDev.OffLine&&Room[10].ModDev.Enable) << 9 | (Room[9].ModDev.OffLine&&Room[9].ModDev.Enable) << 8 | 
										   (Room[8].ModDev.OffLine&&Room[8].ModDev.Enable) << 7 | (Room[7].ModDev.OffLine&&Room[7].ModDev.Enable) << 6 | (Room[6].ModDev.OffLine&&Room[6].ModDev.Enable) << 5 | (Room[5].ModDev.OffLine&&Room[5].ModDev.Enable) << 4 | 
										   (Room[4].ModDev.OffLine&&Room[4].ModDev.Enable) << 3 | (Room[3].ModDev.OffLine&&Room[3].ModDev.Enable) << 2 | (Room[2].ModDev.OffLine&&Room[2].ModDev.Enable) << 1 | (Room[1].ModDev.OffLine&&Room[1].ModDev.Enable);
    EngineBox[0].Split_ModDev_Flags_EnableAndOffline_17_32 = 0;
	EngineBox[0].Split_ModDev_Flags_EnableAndOffline_17_32=/*(Room[32].ModDev.OffLine&&Room[32].ModDev.Enable) << 15 | (Room[31].ModDev.OffLine&&Room[31].ModDev.Enable) << 14 | */(Room[30].ModDev.OffLine&&Room[30].ModDev.Enable) << 13 | (Room[29].ModDev.OffLine&&Room[29].ModDev.Enable) << 12 | 
										   (Room[28].ModDev.OffLine&&Room[28].ModDev.Enable) << 11 | (Room[27].ModDev.OffLine&&Room[27].ModDev.Enable) << 10 | (Room[26].ModDev.OffLine&&Room[26].ModDev.Enable) << 9 | (Room[25].ModDev.OffLine&&Room[25].ModDev.Enable) << 8 | 
										   (Room[24].ModDev.OffLine&&Room[24].ModDev.Enable) << 7 | (Room[23].ModDev.OffLine&&Room[23].ModDev.Enable) << 6 | (Room[22].ModDev.OffLine&&Room[22].ModDev.Enable) << 5 | (Room[21].ModDev.OffLine&&Room[21].ModDev.Enable) << 4 | 
										   (Room[20].ModDev.OffLine&&Room[20].ModDev.Enable) << 3 | (Room[19].ModDev.OffLine&&Room[19].ModDev.Enable) << 2 | (Room[18].ModDev.OffLine&&Room[18].ModDev.Enable) << 1 | (Room[17].ModDev.OffLine&&Room[17].ModDev.Enable);
	EngineBox[0].Split_ModDev_Flags_Online_1_16 = 0;
	EngineBox[0].Split_ModDev_Flags_Online_1_16 = Room[16].ModDev.OnLine << 15 | Room[15].ModDev.OnLine << 14 | Room[14].ModDev.OnLine << 13 | Room[13].ModDev.OnLine << 12 | 
										   Room[12].ModDev.OnLine << 11 | Room[11].ModDev.OnLine << 10 | Room[10].ModDev.OnLine << 9 | Room[9].ModDev.OnLine << 8 | 
										   Room[8].ModDev.OnLine << 7 | Room[7].ModDev.OnLine << 6 | Room[6].ModDev.OnLine << 5 | Room[5].ModDev.OnLine << 4 | 
										   Room[4].ModDev.OnLine << 3 | Room[3].ModDev.OnLine << 2 | Room[2].ModDev.OnLine << 1 | Room[1].ModDev.OnLine;
    EngineBox[0].Split_ModDev_Flags_Online_17_32 = 0;
	EngineBox[0].Split_ModDev_Flags_Online_17_32 =/*Room[32].ModDev.OnLine << 15 | Room[31].ModDev.OnLine << 14 | */Room[30].ModDev.OnLine << 13 | Room[29].ModDev.OnLine << 12 | 
										   Room[28].ModDev.OnLine << 11 | Room[27].ModDev.OnLine << 10 | Room[26].ModDev.OnLine << 9 | Room[25].ModDev.OnLine << 8 | 
										   Room[24].ModDev.OnLine << 7 | Room[23].ModDev.OnLine << 6 | Room[22].ModDev.OnLine << 5 | Room[21].ModDev.OnLine << 4 | 
										   Room[20].ModDev.OnLine << 3 | Room[19].ModDev.OnLine << 2 | Room[18].ModDev.OnLine << 1 | Room[17].ModDev.OnLine;
	EngineBox[0].Split_ModDev_Flags_Alarm_1_16 = 0;
	EngineBox[0].Split_ModDev_Flags_Alarm_1_16 =  Room[16].ModDev.Alarm << 15 | Room[15].ModDev.Alarm << 14 | Room[14].ModDev.Alarm << 13 | Room[13].ModDev.Alarm << 12 | 
										   Room[12].ModDev.Alarm << 11 | Room[11].ModDev.Alarm << 10 | Room[10].ModDev.Alarm << 9 | Room[9].ModDev.Alarm << 8 | 
										   Room[8].ModDev.Alarm << 7 | Room[7].ModDev.Alarm << 6 | Room[6].ModDev.Alarm << 5 | Room[5].ModDev.Alarm << 4 | 
										   Room[4].ModDev.Alarm << 3 | Room[3].ModDev.Alarm << 2 | Room[2].ModDev.Alarm << 1 | Room[1].ModDev.Alarm;
    EngineBox[0].Split_ModDev_Flags_Alarm_17_32 = 0;
	EngineBox[0].Split_ModDev_Flags_Alarm_17_32 = /*Room[32].ModDev.Alarm << 15 | Room[31].ModDev.Alarm << 14 | */Room[30].ModDev.Alarm << 13 | Room[29].ModDev.Alarm << 12 | 
										   Room[28].ModDev.Alarm << 11 | Room[27].ModDev.Alarm << 10 | Room[26].ModDev.Alarm << 9 | Room[25].ModDev.Alarm << 8 | 
										   Room[24].ModDev.Alarm << 7 | Room[23].ModDev.Alarm << 6 | Room[22].ModDev.Alarm << 5 | Room[21].ModDev.Alarm << 4 | 
										   Room[20].ModDev.Alarm << 3 | Room[19].ModDev.Alarm << 2 | Room[18].ModDev.Alarm << 1 | Room[17].ModDev.Alarm;
		
	
}

void OtherSplitSend(void)
{
#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    

    
    if(DiagnosticSplit.OtherCabin.Send==1)
    {
		Room[DiagnosticSplit.OtherCabin.Number].OffLine = FuncWriteReg (DiagnosticSplit.OtherCabin.Number, REG_SPLIT_FROM_INIT_POWER, DiagnosticSplit.OtherCabin.CMD_On_Off, Room[DiagnosticSplit.OtherCabin.Number].OffLine);
		Room[DiagnosticSplit.OtherCabin.Number].OffLine = FuncWriteReg (DiagnosticSplit.OtherCabin.Number, REG_SPLIT_FROM_INIT_SP_ROOM, DiagnosticSplit.OtherCabin.CMD_SetP, Room[DiagnosticSplit.OtherCabin.Number].OffLine);
		Room[DiagnosticSplit.OtherCabin.Number].OffLine = FuncWriteReg (DiagnosticSplit.OtherCabin.Number, REG_SPLIT_FROM_INIT_SP_ROOM_F, DiagnosticSplit.OtherCabin.CMD_SetP_F, Room[DiagnosticSplit.OtherCabin.Number].OffLine);
		Room[DiagnosticSplit.OtherCabin.Number].OffLine = FuncWriteReg (DiagnosticSplit.OtherCabin.Number, REG_SPLIT_FROM_INIT_FAN_SPEED, DiagnosticSplit.OtherCabin.CMD_Fan, Room[DiagnosticSplit.OtherCabin.Number].OffLine);
		if((Room[DiagnosticSplit.OtherCabin.Number].Model_FW_Version == 1) || (Room[DiagnosticSplit.OtherCabin.Number].Model_FW_Version == 2) || (Room[DiagnosticSplit.OtherCabin.Number].OnlyFan >= 4))  //Se sono UTA o se Only Fan Speciale (sfrutto registro UTA per funzioni ventilazione)
			Room[DiagnosticSplit.OtherCabin.Number].OffLine = FuncWriteReg (DiagnosticSplit.OtherCabin.Number, REG_SPLIT_FROM_INIT_UTAMODE, DiagnosticSplit.OtherCabin.UTAMode, Room[DiagnosticSplit.OtherCabin.Number].OffLine);
		Room[DiagnosticSplit.OtherCabin.Number].OffLine = FuncWriteReg (DiagnosticSplit.OtherCabin.Number, REG_SPLIT_OTHER_CABIN_TRIGGER_INIT, 8*(DiagnosticSplit.OtherCabin.Send==1), Room[DiagnosticSplit.OtherCabin.Number].OffLine);    //Simulo l'invio dell'inizializzazione ma solo verso una cabina nella quale ho chiesto l'inizializzazione.
		DiagnosticSplit.OtherCabin.Send=0;  //Rendo pronto il tutto per nuova inizializzazione	
    }
}

void Bridge2SplitSend(void)
{
    if(Bridge[1].SetRoom.Address!=0)
    {
#if (K_EnableHiSpeedMyBus==K_Baud115200)
    Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
    Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
    Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	    
      
        if(Room[Bridge[1].SetRoom.Address].Enable)
        {
            Room[Bridge[1].SetRoom.Address].OffLine = FuncWriteReg (Bridge[1].SetRoom.Address, REG_SPLIT_FROM_INIT_SP_ROOM, Bridge[1].SetRoom.SetPoint, Room[Bridge[1].SetRoom.Address].OffLine);					// REG_SPLIT_STATUS
            Room[Bridge[1].SetRoom.Address].OffLine = FuncWriteReg (Bridge[1].SetRoom.Address, REG_SPLIT_FROM_INIT_FAN_SPEED, Bridge[1].SetRoom.FanSpeed, Room[Bridge[1].SetRoom.Address].OffLine);					// REG_SPLIT_STATUS
            Room[Bridge[1].SetRoom.Address].OffLine = FuncWriteReg (Bridge[1].SetRoom.Address, REG_SPLIT_FROM_INIT_POWER, Bridge[1].SetRoom.PowerOn, Room[Bridge[1].SetRoom.Address].OffLine);
            //VEDERE SUCCESSIVAMENTE COME GESTIRE FARHENEIT
            //Room[Bridge[1].SetRoom.Address].OffLine = FuncWriteReg (Bridge[1].SetRoom.Address, REG_SPLIT_FROM_INIT_SP_ROOM_F, Bridge[1].SetRoom.SetPoint_F, Room[Bridge[1].SetRoom.Address].OffLine);
                Room[Bridge[1].SetRoom.Address].OffLine = FuncWriteReg (Bridge[1].SetRoom.Address, REG_SPLIT_FROM_INIT_HEATER_PWR, Bridge[1].SetRoom.HeaterPwr, Room[Bridge[1].SetRoom.Address].OffLine);            
                Room[Bridge[1].SetRoom.Address].OffLine = FuncWriteReg (Bridge[1].SetRoom.Address, REG_SPLIT_FROM_INIT_UTAMODE, Bridge[1].SetUta.DefMode, Room[Bridge[1].SetRoom.Address].OffLine);            

            Room[Bridge[1].SetRoom.Address].OffLine = FuncWriteReg (Bridge[1].SetRoom.Address, REG_SPLIT_OTHER_CABIN_TRIGGER_INIT, 8, Room[Bridge[1].SetRoom.Address].OffLine);
        }	
        
        Bridge[1].SetRoom.Address=0;    //Sicurezza per dire che ho letto il dato e azzerato lo stato in caso di rimbalzo flag.
    #if(K_HiSpeedU1ModBUS==1)            
        Change_Protocol_BUSM(K_ModBus_Baud115200);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
    #else
        Change_Protocol_BUSM(K_ModBus_Baud57600);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
    #endif        

        if(Bridge[1].SetUta.DefMode!=-1) //Se è stato impostato, lo metto a FFFF per dire vedere se il prossimo giro è stato cambiato.
        {
            InsertReg(1, -1);						
            Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, REG_BRIDGE2_UTA_WRITE_MODE, 1, MaxModReadWait, MaxModReadRetries);
        }            

        if(Bridge[1].SetRoom.HeaterPwr!=-1) //Se è stato impostato, lo metto a FFFF per dire vedere se il prossimo giro è stato cambiato.
        {
            InsertReg(1, -1);						
            Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, REG_BRIDGE2_HEATER_PWR, 1, MaxModReadWait, MaxModReadRetries);
        }            

        InsertReg(1, 0);						
        Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, REG_BRIDGE2_DIAG_SPLIT_WRITE_SELECTION, 1, MaxModReadWait, MaxModReadRetries);
    }        
}


//------------------------------------------------------------------------------
// Cambia il protocollo corrente con quello specificato: (compreso BaudRate)
// ProtocolIndex: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud, 5=MyBus@250000Baud
//------------------------------------------------------------------------------
void Change_Protocol_BUSM(int ProtocolIndex)
{
    static int LastProtocol = -1;    
    
	switch(ProtocolIndex)
	{
		case K_MyBus_Baud115200:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;                
                oRTS_Master = 0;
                set_baudrateU1_115200();                   // @ 115000Bps
                DelaymSec(20); //2);
                ClearUart1Err();            		
            }
            CurUsedProtocolId = FlagMyBusProt_BUSM;         // Protocollo MyBUS UART1
		break;   
        
		case K_MyBus_Baud57600:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;              
                oRTS_Master = 0;
                set_baudrateU1_57600();                        // @ 57600Bps
                DelaymSec(20); //2);
                ClearUart1Err();            		
            }
            CurUsedProtocolId = FlagMyBusProt_BUSM;         // Protocollo MyBUS UART1
		break;

		case K_ModBus_Baud38400:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;          
                oRTS_Master = 0;
                set_baudrateU1_38400();    					// @ 38400Bps
                DelaymSec(20); //2);
                ClearUart1Err();            		
            }
            CurUsedProtocolId = FlagModBusProt_BUSM;         // Protocollo MODBUS UART1
		break;
		
		case K_ModBus_Baud57600:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;              
                oRTS_Master = 0;
                set_baudrateU1_57600();                        // @ 57600Bps
                DelaymSec(20); //2);
                ClearUart1Err();            		
            }
            CurUsedProtocolId = FlagModBusProt_BUSM;         // Protocollo MODBUS UART1
		break;		
        
		case K_ModBus_Baud115200:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;              
                oRTS_Master = 0;
                set_baudrateU1_115200();                   // @ 115000Bps
                DelaymSec(20); //2);
                ClearUart1Err();            		
            }
            CurUsedProtocolId = FlagModBusProt_BUSM;         // Protocollo MODBUS UART1
        break;	        

        
		case K_MyBus_Baud250000:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;                
                oRTS_Master = 0;
                set_baudrateU1_250000();                   // @ 250000Bps
                DelaymSec(20); //2);
                ClearUart1Err();            		
            }
            CurUsedProtocolId = FlagMyBusProt_BUSM;         // Protocollo MyBUS UART1
		break;   
        
        
		default:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;              
                oRTS_Master = 0;
                set_baudrateU1_57600();                        // @ 57600Bps
                DelaymSec(20); //2);
                ClearUart1Err();     
            }
            CurUsedProtocolId = FlagMyBusProt_BUSM;         // Protocollo MyBUS UART1
        break;			
	}
}



//------------------------------------------------------------------------------
// Cambia il protocollo corrente con quello specificato: (compreso BaudRate)
// ProtocolIndex: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
//------------------------------------------------------------------------------
void Change_Protocol_BUSS(int ProtocolIndex)
{
    static int LastProtocol = -1;    
    
	switch(ProtocolIndex)
	{
		case K_MyBus_Baud115200:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;                
                oRTS_Slave = 0;
                set_baudrateU2_115200();                   // @ 115000Bps / 125000Bps
                DelaymSec(20); //2);
                ClearUart2Err();            		
            }
            CurUsedProtocolId = FlagMyBusProt_BUSS;         // Protocollo MyBUS UART2
		break;   
        
		case K_MyBus_Baud57600:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;              
                oRTS_Slave = 0;
                set_baudrateU2_57600();                            // @ 57600Bps
                DelaymSec(20); //2);
                ClearUart2Err();            		
            }
            CurUsedProtocolId = FlagMyBusProt_BUSS;         // Protocollo MyBUS UART2
		break;

		case K_ModBus_Baud38400:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;          
                oRTS_Slave = 0;
                set_baudrateU2_38400();                            // @ 38400Bps
                DelaymSec(20); //2);
                ClearUart2Err();            		
            }
            CurUsedProtocolId = FlagModBusProt_BUSS;        // Protocollo MODBUS UART2
		break;
		
		case K_ModBus_Baud57600:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;              
                oRTS_Slave = 0;
                set_baudrateU2_57600();                            // @ 57600Bps 
                DelaymSec(20); //2);
                ClearUart2Err();            		
            }
            CurUsedProtocolId = FlagModBusProt_BUSS;        // Protocollo MODBUS UART2
		break;		
        
		case K_ModBus_Baud115200:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;              
                oRTS_Slave = 0;
                set_baudrateU2_115200();                   // @ 115000Bps / 125000Bps
                DelaymSec(20); //2);
                ClearUart2Err();            		
            }
            CurUsedProtocolId = FlagModBusProt_BUSS;        // Protocollo MODBUS UART2
        break;	        

		default:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;              
                oRTS_Slave = 0;
                set_baudrateU2_38400();                            // @ 38400Bps
                DelaymSec(20); //2);
                ClearUart2Err();     
            }
            CurUsedProtocolId = FlagModBusProt_BUSS;        // Protocollo MODBUS UART2
        break;			
	}		
}

#if(K_AbilValveOnDemand==1)
void Valve_On_Demand(void)
{
    
    //Sopra i 360 per 2 minuti, limito
    //sotto i 310 per 2 minuti, tolgo limitazione
    static int  RoomPriority = 0;
    static int  RoomNumber = 0;
    static int  LastRoomNumber = 0;
    static int  FirstRoomNumber = 0;
    //static int  WorkRoomNumber = 0;
    static int  LimitOn = 0;
           int  SumValvePercent = 0;
           int  SumValvePercentNoPriority = 0;
           int  SumValvePidCooling = 0;
           int  MaxLevelPriority = 0;
    unsigned int i,p;       

    for(i=0; i<Max_Room; i++)
    {
        if(Room[i].OnLine == 1)
        {
            SumValvePercent = SumValvePercent + Room[i].ValvePerc;
            SumValvePidCooling = SumValvePidCooling + Room[i].Valve_Pid_Cooling;

            if((Room[i].Mode == CoreRaffrescamento) && (Room[i].ValvePriority==0))
                SumValvePercentNoPriority = SumValvePercentNoPriority + Room[i].ValvePerc;

            if(MaxLevelPriority<Room[i].ValvePriority)
                MaxLevelPriority = Room[i].ValvePriority;
            
        }   
    }
/*           
    TimerValve_On_Demand_Limit.Enable = ((LimitOn==1) && (SumValvePercent<K_Min_Lim_OFF_Demand_Perc_Valve)) ||
                                        ((LimitOn==0) && (SumValvePercent>K_Max_Lim_ON_Demand_Perc_Valve));
            
    if(!TimerValve_On_Demand_Limit.Enable)
    {
        TimerValve_On_Demand_Limit.Value = 0;
        TimerValve_On_Demand_Limit.TimeOut = 0;
    }
    if(TimerValve_On_Demand_Limit.TimeOut)
    {
        if(LimitOn==0)
            LimitOn=1;
        else
            LimitOn=0;
    }
*/

    if(SumValvePidCooling < K_Min_Lim_OFF_Demand_Perc_Valve)
        LimitOn=0;
    if(SumValvePidCooling > K_Max_Lim_ON_Demand_Perc_Valve)
        LimitOn=1;                   

    if((LimitOn==1) && EngineBox[0].CoolingMode && (MaxLevelPriority!=0))
    {
        if(TimerValve_On_Demand_Switch.TimeOut)
        {
            RoomNumber = 0;
            for(p=0; p<MaxLevelPriority; p++)
            {
                if(++RoomPriority>MaxLevelPriority)
                    RoomPriority = 1;

                for(i=0; i<Max_Room; i++)
                {
                    if((Room[i].OnLine == 1) && (Room[i].Mode == CoreRaffrescamento) && (Room[i].ValvePriority==RoomPriority))
                    {
                        RoomNumber = i;
                        break;
                    }
                }

                if((Room[i].OnLine == 1) && (Room[i].Mode == CoreRaffrescamento) && (Room[i].ValvePriority==RoomPriority))
                {
                    break;
                }                   
            }          
            TimerValve_On_Demand_Switch.TimeOut = 0;
            TimerValve_On_Demand_Switch.Value = 0;
        }
    }
    else
    {
        RoomPriority = 0;
        RoomNumber = 0;
        //WorkRoomNumber = 0;
        LastRoomNumber = 0;
        LimitOn=0;
        TimerValve_On_Demand_Switch.TimeOut = 1;
        TimerValve_On_Demand_Switch.Value = 0;
    }
    
    if((Room[RoomNumber].Mode != CoreRaffrescamento)&&(Room[RoomNumber].ValvePriority==RoomPriority))
        TimerValve_On_Demand_Switch.TimeOut = 1;
        
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_VALVOLA_CUR_NUMB_PRIORITY, RoomPriority);	// 
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_VALVOLA_ROOM_NUMB_PRIORITY, RoomNumber);	//     
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_VALVOLA_ONDEMAND_LIM_MAX, K_Max_Lim_ON_Demand_Perc_Valve);	//     
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_VALVOLA_MAX_PERC_PRIORITY, K_Max_Lim_ON_Demand_Perc_Valve-SumValvePercentNoPriority);	//     
    //FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_VALVOLA_MAX_PERC_PRIORITY, K_Max_Lim_ON_Demand_Perc_Valve-SumValvePercent);	//          

    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_VALVOLA_SUM_REQ_PERCENT, SumValvePidCooling);	//     
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_VALVOLA_SUM_TOT_PERCENT, SumValvePercent);	// 
}
#endif

#if(K_AbilScoreOnDemand==1)
void Score_On_Demand(void)
{
    
    //Sopra i 360 per 2 minuti, limito
    //sotto i 310 per 2 minuti, tolgo limitazione
    static int  RoomPriority = 0;
    static int  RoomNumber = 0;
    static int  LastRoomNumber = 0;
    static int  FirstRoomNumber = 0;
    //static int  WorkRoomNumber = 0;
    static int  LimitOn = 0;
           int  SumValvePercent = 0;
           int  SumValvePercentNoPriority = 0;
           int  SumValvePidCooling = 0;
           int  MaxLevelPriority = 0;
           int  TempScore = 0;
           int  SumDeltaScore = 0;
           int  SumOutScore = 0;
           int  SumTotalScore = 0;
           
    unsigned int i,p;       

    for(i=0; i<Max_Room; i++)
    {
        if(Room[i].OnLine == 1)
        {
            SumValvePercent = SumValvePercent + Room[i].ValvePerc;
            SumValvePidCooling = SumValvePidCooling + Room[i].Valve_Pid_Cooling;

            if((Room[i].Mode == CoreRaffrescamento) && (Room[i].ValvePriority==0))
                SumValvePercentNoPriority = SumValvePercentNoPriority + Room[i].ValvePerc;

            if(MaxLevelPriority<Room[i].ValvePriority)
                MaxLevelPriority = Room[i].ValvePriority;
            
            if(Room[i].Mode == CoreRaffrescamento)
            {
                TempScore = (Room[i].AirTemp - Room[i].SetPoint)/100;
                if(TempScore>0)
                    SumDeltaScore = SumDeltaScore + TempScore;
                
                TempScore = (Room[i].OutTemp - K_MaxAirOut)/100;
                if(TempScore>0)
                    SumOutScore = SumOutScore + TempScore;
            }
        }   
    }
    
    SumTotalScore = SumDeltaScore + SumOutScore;

    TimerValve_On_Demand_Switch.Time = Room[k_Split_Master_Add].TimeOnDemandValveSwitch*60;
    
    if(SumTotalScore < (Room[k_Split_Master_Add].Max_Lim_ON_Demand_Total_Score-5))//K_Min_Lim_OFF_Demand_Total_Score)
        LimitOn=0;
    if(SumTotalScore > Room[k_Split_Master_Add].Max_Lim_ON_Demand_Total_Score)//K_Max_Lim_ON_Demand_Total_Score)
        LimitOn=1;                   

    if((LimitOn==1) && EngineBox[0].CoolingMode && (MaxLevelPriority!=0) && (Room[k_Split_Master_Add].On_Priority_Mode==1))
    {
        if(TimerValve_On_Demand_Switch.TimeOut)
        {
            RoomNumber = 0;
            for(p=0; p<MaxLevelPriority; p++)
            {
                if(++RoomPriority>MaxLevelPriority)
                    RoomPriority = 1;

                for(i=0; i<Max_Room; i++)
                {
                    if((Room[i].OnLine == 1) && (Room[i].Mode == CoreRaffrescamento) && (Room[i].ValvePriority==RoomPriority))
                    {
                        RoomNumber = i;
                        break;
                    }
                }

                if((Room[i].OnLine == 1) && (Room[i].Mode == CoreRaffrescamento) && (Room[i].ValvePriority==RoomPriority))
                {
                    break;
                }                   
            }          
            TimerValve_On_Demand_Switch.TimeOut = 0;
            TimerValve_On_Demand_Switch.Value = 0;
        }
    }
    else
    {
        RoomPriority = 0;
        RoomNumber = 0;
        //WorkRoomNumber = 0;
        LastRoomNumber = 0;
        LimitOn=0;
        TimerValve_On_Demand_Switch.TimeOut = 1;
        TimerValve_On_Demand_Switch.Value = 0;
    }
    
    if((Room[RoomNumber].Mode != CoreRaffrescamento)&&(Room[RoomNumber].ValvePriority==RoomPriority))
        TimerValve_On_Demand_Switch.TimeOut = 1;
        
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_VALVOLA_CUR_NUMB_PRIORITY, RoomPriority);	// 
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_VALVOLA_ROOM_NUMB_PRIORITY, RoomNumber);	//     
    //FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_VALVOLA_ONDEMAND_LIM_MAX, K_Max_Lim_ON_Demand_Perc_Valve);	//     
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_VALVOLA_MAX_PERC_PRIORITY, 100);//K_Max_Lim_ON_Demand_Perc_Valve-SumValvePercentNoPriority);	//     

    //FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_VALVOLA_SUM_REQ_PERCENT, SumValvePidCooling);	//     
    FuncWriteBrcReg (WriteSplitRegBrdCst, REG_SPLIT_VALVOLA_SUM_TOT_PERCENT, SumTotalScore);	// 
}
#endif