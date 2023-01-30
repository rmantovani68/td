//----------------------------------------------------------------------------------
//	Progect name:	ProtocolloComunicazione.c
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			22/05/2011
//	Description:	Corpo delle funzioni che gestiscono la comunicazione con il sistema
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//	Include
//----------------------------------------------------------------------------------
#include "HWSelection.h"
#if (GB210==1) 
	#include <p24FJ256GB210.h>
#elif (DA210==1)
	#include <p24FJ256DA210.h>
#else
	#include <p24FJ256GB110.h>
#endif
#include <stdio.h>
#include "DefinePeriferiche.h"
#include "Timer.h"
#include "usart.h"
#include "ADC.h"
#include "PWM.h"
#include "EEPROM.h"
#include "Valvola_PassoPasso.h"
#include "Driver_Comunicazione.h"
#include "Driver_ComunicazioneSec.h"
#include "ProtocolloComunicazione.h"
#include "ProtocolloComunicazioneSec.h"
#include "Driver_ModBusSec.h"
#include "ProtocolloModBusSec.h"
#include "Core.h"
#include "delay.h"


extern volatile int CntComErrSec;
extern volatile int TimeOutComErrSec;
extern volatile int CRC_ComErrSec;
extern volatile int DisableModComErrorCounter;     

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
unsigned char WaitRxSec(int TimeOut_ms)
{
	TimeOutPktRxSec.Value = 0;			// resetto il timer per il time out della risposta
	TimeOutPktRxSec.TimeOut = 0;		//
	TimeOutPktRxSec.Time = TimeOut_ms;	// lo configuro come richiesto
	TimeOutPktRxSec.Enable = 1;			// lo faccio partire

    WaitingReplaySec = 1;
	while((TimeOutPktRxSec.TimeOut == 0) && (LastRxSec.Valid_Data == 0))	// attendo un evento
		continue;
    WaitingReplaySec = 0;
    
	TimeOutPktRxSec.Enable = 0;			// fermo il timer
	TimeOutPktRxSec.TimeOut = 0;		// resetto il time out

	if(LastRxSec.Valid_Data == 1)		// se ho ricevuto un dato valido 
	{
		if((LastRxSec.Mitt_Add == BufferTxSec.Dest_Add) &&
			(LastRxSec.Comando == BufferTxSec.Comando) &&
			(LastRxSec.Registro == BufferTxSec.Registro)) 	// verifico il pacchetto
			return 0;				// ritorno 0 se è ok
	}
	LastRxSec.Valid_Data = 0;		// scarto il pacchetto		$$$ -> copiato da FW Master 4.4.126
	return 1;						// ritorno 1
}

/*
unsigned char Send_WaitRx_Sec(unsigned char Address, char Comando, int Registro, int Data, int TimeOut_ms, char Retry)
{
	char Result = 0;
	char RetryCntSec = 0;
	while (RetryCntSec < Retry)
	{	
		RetryCntSec +=1;
		Write_Cmd_Sec(&BufferTxSec, Address, Comando, Registro, Data);
		Result = WaitRxSec(TimeOut_ms);
		if (Result == 0)	// se ricevo un pacchetto valido 
		{
			LastRxSec.Valid_Data = 0;		// scarto il pacchetto		$$$ -> copiato da FW Master 4.4.126
			return 0;
		}
        else 
            if(Room[Address].Enable) 
            {
                TimeOutComErrSec++;
                Me.ComError.TimeOutComErrSec++;
            }  
	}
    Room[Address].OffLine = 1;
	Room[Address].OnLine = 0;    
    if(Room[Address].Enable)
    {
        CntComErrSec++; 
        Me.ComError.CntComErrSec++;
    }
	LastRxSec.Valid_Data = 0;		// scarto il pacchetto		$$$ -> copiato da FW Master 4.4.126	
	return 1;				// se non ho nessuna risposta valida per dopo il numero di tentativi richiesti

}
*/
unsigned char Send_WriteReg_Sec(unsigned char Address, char Comando, int Registro, int Data, int TimeOut_ms, char Retry)
{
	char Result = 1;
	char RetryCntSec = 0;
	while (RetryCntSec < Retry)
	{
		RetryCntSec +=1;
		Write_Cmd_Sec(&BufferTxSec, Address, Comando /*WriteReg*/, Registro, Data);
		Result = WaitRxSec(TimeOut_ms);
		if (Result == 0)
		{
            if(Comando==WriteReg)
            {
                if(LastRxSec.Data == Ack)
                {
                   LastRxSec.Valid_Data = 0;
                   return 0;                     
                }
                else if(LastRxSec.Data == N_Ack)
                {
                   LastRxSec.Valid_Data = 0;
                   return 2;                           
                }                 
                else
                {
                   LastRxSec.Valid_Data = 0;
                   return 3;                           
                }
            }
            LastRxSec.Valid_Data = 0;
            return 0;                     
		}
        if(Result==1 && Room[Address].Enable)
        {
            TimeOutComErrSec++;
            Me.ComError.TimeOutComErrSec++;
        }  
        
		LastRxSec.Valid_Data = 0;
	}
    Room[Address].OffLine = 1;
	Room[Address].OnLine = 0;        
	LastRxSec.Valid_Data = 0;
    if(Room[Address].Enable)
    {
        CntComErrSec++; 
        Me.ComError.CntComErrSec++;
    }

	return 1;
}

unsigned char SendBrc_WriteReg_Sec(char Cmd, int Registro, int Data, int TimeOut_ms, char Retry)
{
	char RetryCntSec = 0;
	while (RetryCntSec < Retry)
	{
		RetryCntSec +=1;
		//Write_brc_Cmd_Sec(&BufferTxSec, Cmd, Registro, Data);
        Write_Cmd_Sec(&BufferTxSec, k_Broadcast_Addr, Cmd, Registro, Data);

		TimeOutPktRxSec.Value = 0;			// resetto il timer per il time out della risposta
		TimeOutPktRxSec.TimeOut = 0;		//
		TimeOutPktRxSec.Time = TimeOut_ms;	// lo configuro come richiesto
		TimeOutPktRxSec.Enable = 1;			// lo faccio partire
	
		while(TimeOutPktRxSec.TimeOut == 0)	// attendo un evento
			continue;
	
		TimeOutPktRxSec.Enable = 0;			// fermo il timer
		TimeOutPktRxSec.TimeOut = 0;		// resetto il time out
	}
	//LastRx.Valid_Data = 0;
	LastRxSec.Valid_Data = 0;	// $$$  LastRx.Valid_Data = 0;
	return 0;
}

unsigned FuncReadReg_Sec (char Address, int REG, volatile int * StoreReg, unsigned OffLineFlag)
{
#if(K_EnableModBus_Sec==1)
	if(!OffLineFlag)
	{
		if(Send_WaitModRxSec(Address, ReadModCmd, REG, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusOneBased)==0)
		{
            (*StoreReg) = Mod_LastRxSec.Data;
			Mod_LastRxSec.Valid_Data = 0;
            //DelayuSec(K_TimeModInterCharSec);    //c'e' già in "Send_WaitModRxSec"   // Pausa per garantire una minima distanza tra un Tx e il successivo (minmo 3.5volte 1 carattere @ Baudrate corrente	            
			return 0;
		}
	}
	Mod_LastRxSec.Valid_Data = 0;                   // scarto il pacchetto	
	return 1;                                       // se non ha risposto
#else    
	if(!OffLineFlag)
	{
		if(Send_WriteReg_Sec(Address, ReadReg, REG, 0, MaxReadWait, MaxReadRetries)==0)
		{
			(*StoreReg) = LastRxSec.Data;
			LastRxSec.Valid_Data = 0;
            //DelayuSec(10000);                            // Pausa per garantire una minima distanza tra un Tx e il successivo 		            
			return 0;
		}
	}
	LastRxSec.Valid_Data = 0;		// scarto il pacchetto		$$$ -> copiato da FW Master 4.4.126
	return 1;// se non ha risposto
#endif
}

unsigned FuncWriteReg_Sec(char Address, int REG, int SendData, unsigned OffLineFlag)
{
#if(K_EnableModBus_Sec==1)
	if(!OffLineFlag)
	{   
		if(Send_WriteModRegSec(Address, REG, SendData, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusOneBased)==0)
		{
			Mod_LastRxSec.Valid_Data = 0;           	
			// C'è già nella funzione "Send_WriteModRegSec" ->    DelayuSec(K_TimeModInterCharSec);       // Pausa per garantire una minima distanza tra un Tx e il successivo (minmo 3.5volte 1 carattere @ Baudrate corrente	                        
			return 0;						
		}
	}
	Mod_LastRxSec.Valid_Data = 0;                   // scarto il pacchetto		
	return 1;    
#else    
	if(!OffLineFlag)
	{
		if(Send_WriteReg_Sec(Address, WriteReg, REG, SendData, MaxWriteWait, MaxWriteRetries)==0)
		{
			LastRxSec.Valid_Data = 0;		// scarto il pacchetto		$$$ -> copiato da FW Master 4.4.126
			//DelayuSec(10000);         	// v.8.14.47 - Pausa per garantire una minima distanza tra un Tx e il successivo 		                        
			return 0;						// $$$ -> copiato da FW Master 4.4.126
		}
	}
	LastRxSec.Valid_Data = 0;		// scarto il pacchetto		$$$ -> copiato da FW Master 4.4.126
	return 1;
#endif
}

unsigned FuncWriteBrcReg_Sec(char Comando, int REG, int SendData)
{
#if(K_EnableModBus_Sec==1)
    DisableModComErrorCounter = 1;

    Send_WriteModRegSec(k_Broadcast_Addr_Sec, REG, SendData, MaxModWriteBrcWaitSec, MaxModWriteBrcRetriesSec, WriteModCmd, K_ModBusOneBased);
	Mod_LastRxSec.Valid_Data = 0;          
    DisableModComErrorCounter = 0;
	return 0;       
#else        
    unsigned char ret;
    //$$$
    ret = SendBrc_WriteReg_Sec(Comando, REG, SendData, WriteBrcWait, WriteBrcRetries);    
	//DelayuSec(10000);     	// v.8.14.47 - Pausa per garantire una minima distanza tra un Tx e il successivo 		                
	return ret;
#endif
            
}

unsigned FuncCheckReg_Sec(char Address)
{
#if(K_EnableModBus_Sec==1)
    DisableModComErrorCounter = 1;
    if(Send_WaitModRxSec(Address, ReadModCmd, 1, 1, MaxModChkSubWaitSec, MaxModChkSubRetriesSec, K_ModBusOneBased)==0)
    {
        Mod_LastRxSec.Valid_Data = 0;           // scarto il pacchetto	
        //DelayuSec(K_TimeModInterCharSec);    //C'e' già in "Send_WaitModRxSec"    // Pausa per garantire una minima distanza tra un Tx e il successivo (minmo 3.5volte 1 carattere @ Baudrate corrente	
        DisableModComErrorCounter = 0;
        return 0;
    }
	Mod_LastRxSec.Valid_Data = 0;                   // scarto il pacchetto	
    DisableModComErrorCounter = 0;
	return 1;                                       // se non ha risposto
#else        
    if(Send_WriteReg_Sec(Address, Check, 0, 0, MaxChkWait, MaxChkRetries)==0)
    {
        LastRxSec.Valid_Data = 0;		// scarto il pacchetto		$$$ -> copiato da FW Master 4.4.126
        //DelayuSec(10000);            	// v.8.14.47 - Pausa per garantire una minima distanza tra un Tx e il successivo 		                        
        return 0;						// $$$ -> copiato da FW Master 4.4.126
    }

	LastRxSec.Valid_Data = 0;		// scarto il pacchetto		$$$ -> copiato da FW Master 4.4.126
	return 1;
#endif
}


void CheckModuli_Sec(void)
{
	char Result = 0;
	unsigned char i;         //!!!

#if (K_EnableModBus_Sec==1)
    #if (K_EnableHiSpeedMyBus_Sec==1)
        Change_ProtocolBusSlave(K_ModBus_Baud57600, K_Modbus_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_SET_BAUDRATE, 1);	// Invio sul BUS in broadcast il Baud: /Vaule: 1 = HiSpeed (115000Baud) 0 = LowSpeed (57600Baud)
        Change_ProtocolBusSlave(K_ModBus_Baud115200, K_Modbus_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #else
        Change_ProtocolBusSlave(K_ModBus_Baud57600, K_Modbus_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #endif	    
    
#else
    #if (K_EnableHiSpeedMyBus_Sec==1)
        Change_ProtocolBusSlave(K_MyBus_Baud57600, K_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_SET_BAUDRATE, 1);	// Invio sul BUS in broadcast il Baud: /Vaule: 1 = HiSpeed (115000Baud) 0 = LowSpeed (57600Baud)
        Change_ProtocolBusSlave(K_MyBus_Baud115200, K_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #else
        Change_ProtocolBusSlave(K_MyBus_Baud57600, K_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #endif	    
#endif
        
	// Scansiono la rete alla ricerca dei moduli di gestione delle valvole e degli split
	Me.Sub_Slave_Quantity = 0;
	
	for(i=1;i<Max_Unit_Sec; i++)        //!!!
	{
		//if(i == Me.My_Address)
		//	i++;
		//Result = Send_WaitRx_Sec(i, Check, 0, 0, MaxChkWait, MaxChkRetries);
        //Result = Send_WriteReg_Sec(i, ReadReg, 0, 0, MaxChkWait, MaxChkRetries);
        Result = FuncCheckReg_Sec(i); 
		Room[i].OffLine = (Result != 0);
		//LastRx.Valid_Data = 0;
		LastRxSec.Valid_Data = 0;	// $$$   LastRx.Valid_Data = 0;
		Room[i].Enable = !Room[i].OffLine;
        Room[i].OnLine = (Room[i].Enable == 1) && (Room[i].OffLine == 0);
		if(Room[i].Enable)		// se ho almeno uno split
		{
			EnableMySecBus =1;			// segnalo al sistema di attivare la scansione
			Me.Sub_Slave_Quantity++;	// Conto quanti Sub Slave ci sono
		}
	}

}

void CheckHotPlugSlave_Sec(unsigned char * Cnt)
{
	char Result = 0;
	unsigned char CurrentId = 0;

	while((*Cnt) < Max_Unit_Sec)				// Provo a recuperare i moduli di gestione Split
	{
		CurrentId = (*Cnt);
		if((Room[CurrentId].Enable == 0))
		{
#if (K_EnableModBus_Sec==1)
    #if (K_EnableHiSpeedMyBus_Sec==1)
        Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_SET_BAUDRATE, 1);	// Invio sul BUS in broadcast il Baud: /Vaule: 1 = HiSpeed (115000Baud) 0 = LowSpeed (57600Baud)
        Change_ProtocolBusSlave(K_ModBus_Baud115200, K_ModBus_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #else
        Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #endif	    
    
#else
    #if (K_EnableHiSpeedMyBus_Sec==1)
        Change_ProtocolBusSlave(K_MyBus_Baud57600, K_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_SET_BAUDRATE, 1);	// Invio sul BUS in broadcast il Baud: /Vaule: 1 = HiSpeed (115000Baud) 0 = LowSpeed (57600Baud)
        Change_ProtocolBusSlave(K_MyBus_Baud115200, K_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #else
        Change_ProtocolBusSlave(K_MyBus_Baud57600, K_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #endif	    
#endif	              
			//Result = Send_WaitRx_Sec(CurrentId, Check, 0, 0, 5, 1);
            //Result = Send_WriteReg_Sec(CurrentId, ReadReg, 0, 0, 5, 1);
            Result = FuncCheckReg_Sec(CurrentId); 
            Room[CurrentId].OffLine = (Result != 0);
			Room[CurrentId].Enable = !Room[CurrentId].OffLine;	// Se la trovo la abilito.
            if(Room[CurrentId].Enable)		// se ho almeno uno split
            {
                EnableMySecBus =1;			// segnalo al sistema di attivare la scansione
                Me.Sub_Slave_Quantity++;	// Conto quanti Sub Slave ci sono
            }               
			Room[CurrentId].OnLine = (Room[CurrentId].Enable == 1) && (Room[CurrentId].OffLine == 0);
			LastRxSec.Valid_Data = 0;       // <---- ERRORE (Ma serve??) !!!!! era   LastRx.Valid_Data
			(*Cnt) += 1;
			break;
		}
		(*Cnt) += 1;
	}
	if(((*Cnt) >= Max_Unit_Sec) || ((*Cnt) < 0))
		(*Cnt) = 1;
}

void ResumeOffLine_Sec(unsigned char * Cnt)
{       
	char Result = 0;
	unsigned char CurrentId = 0;
	if(EnableMySecBus)
	{
        while((*Cnt) < Max_Unit_Sec)				// Provo a recuperare i moduli di gestione Split
        {
            CurrentId = (*Cnt);
            if((Room[CurrentId].Enable == 1) & (Room[CurrentId].OffLine == 1))
            {
#if (K_EnableModBus_Sec==1)
    #if (K_EnableHiSpeedMyBus_Sec==1)
        Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_SET_BAUDRATE, 1);	// Invio sul BUS in broadcast il Baud: /Vaule: 1 = HiSpeed (115000Baud) 0 = LowSpeed (57600Baud)
        Change_ProtocolBusSlave(K_ModBus_Baud115200, K_ModBus_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #else
        Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #endif	    
    
#else
    #if (K_EnableHiSpeedMyBus_Sec==1)
        Change_ProtocolBusSlave(K_MyBus_Baud57600, K_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_SET_BAUDRATE, 1);	// Invio sul BUS in broadcast il Baud: /Vaule: 1 = HiSpeed (115000Baud) 0 = LowSpeed (57600Baud)
        Change_ProtocolBusSlave(K_MyBus_Baud115200, K_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #else
        Change_ProtocolBusSlave(K_MyBus_Baud57600, K_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #endif	    
#endif	                        
                //Result = Send_WaitRx_Sec((*Cnt), Check, 0, 0, MaxChkWait, MaxChkRetries);
                //Result = Send_WriteReg_Sec((*Cnt), ReadReg, 0, 0, MaxChkWait, MaxChkRetries);
                Result = FuncCheckReg_Sec(CurrentId); 
                Room[CurrentId].OffLine = (Result != 0);
                LastRx.Valid_Data = 0;
                Room[CurrentId].OnLine = (Room[CurrentId].Enable == 1) && (Room[CurrentId].OffLine == 0);
                (*Cnt) += 1;
                break;
            }
            Room[CurrentId].OnLine = (Room[CurrentId].Enable == 1) && (Room[CurrentId].OffLine == 0);
            (*Cnt) += 1;
        }
        if(((*Cnt) >= Max_Unit_Sec) || ((*Cnt) < 0))
            (*Cnt) = 0;        
    }

}



void RefreshInPutRoom_Sec(volatile TypSplitSlave * DataStore, char Address, int SchedRead)
{
	int x;
	
	if(DataStore->OnLine == 1)		// se OnLine
	{
        DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_FAN_TACHO, &(DataStore->FanSpeed), DataStore->OffLine);		// stato attuale della valvola
        DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_VALVOLA_CUR_POS, &(DataStore->ExpValve_Act), DataStore->OffLine);		// stato attuale della valvola
        DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_SUPERHEAT_RISCALDAMENTO, &(DataStore->SuperheatRiscaldamento), DataStore->OffLine);		// stato attuale della valvola		
        DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_SETP_SUPERHEAT, &(DataStore->SetP_Superheat), DataStore->OffLine);		// stato attuale della valvola			        		
        DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_VALVOLA_ERR_POS, &(DataStore->ExpValve_Err), DataStore->OffLine);		// stato attuale della valvola			        
        DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_VALVOLA_ERR_COD, &(DataStore->ExpValve_Err_Code), DataStore->OffLine);		// stato attuale della valvola			        
        DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_VALVOLA_PID_COOL_PERC_POS, &(DataStore->ExpValve_Pid_Cooling), DataStore->OffLine);		// stato attuale della valvola	
        DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_SUPERH_COOL_PID_ERROR, &(DataStore->SuperHeat_Pid_Cool_Error), DataStore->OffLine);		// stato attuale della valvola	
        DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_RD_TEMPERATURE_PROBE_ERROR, &(DataStore->Temperature_Probe_error), DataStore->OffLine);		// stato attuale della valvola	
        DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_MODEL_VERSION, &(DataStore->Model_FW_Version), DataStore->OffLine);		// stato attuale della valvola	

        DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_FAN_M3H, &(DataStore->Fan_M3h), DataStore->OffLine);

        DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_AIR_POWER_OUTPUT, &(DataStore->AirPowerOutput), DataStore->OffLine);
        DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_AIR_POWER_BTU, &(DataStore->AirPowerBTU), DataStore->OffLine);
        
        DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_READ_HUMIDITY, &(DataStore->Humidity), DataStore->OffLine);            

        if(SchedRead==0)
        {
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_CUMULATIVE_ALARM, &(DataStore->CumulativeAlarm), DataStore->OffLine);		// stato attuale degli errori attivi subslave
            //DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_FAN_TACHO, &(DataStore->FanSpeed), DataStore->OffLine);		// stato attuale della valvola
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_CURRENT_EVAP_TEMP, &(DataStore->Evap_Temp), DataStore->OffLine);		
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_CURRENT_EVAP_PRESURE, &(DataStore->Evap_Press), DataStore->OffLine);	
        }
        if(SchedRead==1)
        {
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_TEMP_AMBIENTE, &(DataStore->Temp_Amb), DataStore->OffLine);			// temperatura ambiente
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_TEMP_ARIA_USCITA, &(DataStore->Temp_AirOut), DataStore->OffLine);		// temperatura aria in uscita
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_TEMP_BULBO_GRANDE, &(DataStore->Temp_Big), DataStore->OffLine);		// temperatura tubo grande
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_TEMP_BULBO_PICCOLO, &(DataStore->Temp_Small), DataStore->OffLine);		// temperatura tubo piccolo
        }
        if(SchedRead==2)
        {
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_FUNCTION_MODE, &(DataStore->Function_Mode), DataStore->OffLine);		// modalità di lavoro richiesta. Sovrascritta da touch se presente
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_VENTIL_MODE, &(DataStore->Ventil_Mode), DataStore->OffLine);			// modalità di ventilazione richiesta. Sovrascritta dal touch se presente
            //DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_VALVOLA_CUR_POS, &(DataStore->ExpValve_Act), DataStore->OffLine);		// stato attuale della valvola
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_EVAP_TEMP_VALVE, &(DataStore->EvapTempValve), DataStore->OffLine);		
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_SUPERHEAT, &(DataStore->Superheat), DataStore->OffLine);		// stato attuale della valvola
        }
        if(SchedRead==3)
        {
            //DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_SUPERHEAT_RISCALDAMENTO, &(DataStore->SuperheatRiscaldamento), DataStore->OffLine);		// stato attuale della valvola			
            //DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_SETP_SUPERHEAT, &(DataStore->SetP_Superheat), DataStore->OffLine);		// stato attuale della valvola			
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_STATUS, &(DataStore->SplitStatus), DataStore->OffLine);
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_HW_VERSION, &(DataStore->HW_Version), DataStore->OffLine);	
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_FW_VERSION_HW, &(DataStore->FW_Version_HW), DataStore->OffLine);	
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_FW_VERSION_FW, &(DataStore->FW_Version_FW), DataStore->OffLine);	
        }   
        if(SchedRead==4)
        {
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_FW_REVISION_FW, &(DataStore->FW_Revision_FW), DataStore->OffLine);	        
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_DIAG_SPLIT_CYCLE_TIME, &(DataStore->CycleTime), DataStore->OffLine);	        
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_FAN_POWER, &(DataStore->Fan_Power), DataStore->OffLine);		// temperatura aria in uscita
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_FAN_CURRENT, &(DataStore->Fan_Current), DataStore->OffLine);		// temperatura aria in uscita
            DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_SUPPLY_VOLTAGE, &(DataStore->Supply_Voltage), DataStore->OffLine);		// temperatura aria in uscita
        }
		//DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_PRES_BULBO_GRANDE, &(DataStore->Pres_Big), DataStore->OffLine);		// pressione tubo grande
		//DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_PRES_BULBO_PICCOLO, &(DataStore->Pres_Small), DataStore->OffLine);		// pressione tubo piccolo
		//DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_CURRENT_SP, &(DataStore->SP), DataStore->OffLine);						// SP richiesto
		//DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_CURRENT_EVAP_TEMP, &(DataStore->Evap_Temp), DataStore->OffLine);		// temperatura di evaporazione richiesta
		//DataStore->OffLine = FuncReadReg_Sec (Address, REG_SPLIT_VALVOLA_SET_POS, &(DataStore->ExpValve_Req), DataStore->OffLine);		// apertura richiesta alla valvola
		
		for(x=0; x<2000; x++);	// piccolo ritardo! Serve? Fare prove per verificare l'utilità (Forse un giorno....)
	}
}




void RefreshAllInPut_Sec(void)
{	
	unsigned char i=0;
    int Mode = Me.Function_Mode; //Carico la mia modalità
    int EvapTemp = Me.Evap_Temp;    //Carico la mia evap temp
    long int TempAmbMiddle = Me.Temperature.Ambient;
    long int NumberSubSlaveOnline = 0;
    static int SlaveSchedRead = 0;
    int SubSlaveSumPercValve = 0;
    int SubSlaveSumValvePidCooling = 0;      
    float TotalFanPower = 0;
    float TotalAirPowerOutput = 0;
    float TotalAirPowerBTU = 0;
    long int HumiMiddle = Me.Humidity;
    
    int HumiPresent=0;
    
	if(EnableMySecBus)
	{
#if (K_EnableModBus_Sec==1)
    #if (K_EnableHiSpeedMyBus_Sec==1)
            Change_ProtocolBusSlave(K_ModBus_Baud115200, K_ModBus_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #else
            Change_ProtocolBusSlave(K_ModBus_Baud57600K_ModBus_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #endif	         
#else        
    #if (K_EnableHiSpeedMyBus_Sec==1)
            Change_ProtocolBusSlave(K_MyBus_Baud115200, K_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #else
            Change_ProtocolBusSlave(K_MyBus_Baud57600, K_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #endif	         
#endif        

        for(i=0; i<Max_Unit_Sec; i++)
		{
  			RefreshInPutRoom_Sec(&Room[i], i, SlaveSchedRead);
            
            if(Room[i].OnLine==1)
            {
                NumberSubSlaveOnline++;
                TempAmbMiddle = TempAmbMiddle + Room[i].Temp_Amb;
                if(Room[i].Humidity!=0)
                {
                    HumiMiddle = HumiMiddle + Room[i].Humidity;
                    HumiPresent++;
                }
                SubSlaveSumPercValve = SubSlaveSumPercValve + ValueToPercent(Room[i].ExpValve_Act, K_MaxExcursionValve);
                SubSlaveSumValvePidCooling = SubSlaveSumValvePidCooling + Room[i].ExpValve_Pid_Cooling;
                TotalFanPower = TotalFanPower + (float)Room[i].Fan_Power;
                TotalAirPowerOutput = TotalAirPowerOutput + (float)Room[i].AirPowerOutput;
                TotalAirPowerBTU = TotalAirPowerBTU + (float)Room[i].AirPowerBTU;
                if(Room[i].Function_Mode==CoreRaffrescamento)// estraggo modalità sottoslave e se i miei sottoslave chiedono funzionamento, lo mando al master
                {
                    Mode = CoreRaffrescamento;           //se il sottoslave chiede di raffrescare mando modalità freddo al master
                    if(EvapTemp > (Room[i].Evap_Temp))  //se la temperatura di evaporazione del sottoslave è minore, la carico
                        EvapTemp = Room[i].Evap_Temp;
                }
                else if(Room[i].Function_Mode==CoreRiscaldamento)
                {
                    Mode = CoreRiscaldamento;            //se il sottoslave chiede di riscaldare mando modalità caldo al master
                    if(EvapTemp < (Room[i].Evap_Temp))  //se la temperatura di evaporazione del sottoslave è maggiore, la carico
                        EvapTemp = Room[i].Evap_Temp;
                }
            }
		}
        SlaveSchedRead += 1;
        if(SlaveSchedRead>4)
            SlaveSchedRead=0;    
            
        Me.MyOrSubSplitMode = Mode;
        Me.MyOrSubSplitEvap_Temp = EvapTemp;
        Me.TempAmbMiddle = (int)(TempAmbMiddle/(1+NumberSubSlaveOnline));
        Me.HumidityMiddle = (int)(HumiMiddle/(1+HumiPresent));        
        Me.SubSlaveSumPercValve = SubSlaveSumPercValve;
        Me.SubSlaveSumValvePidCooling = SubSlaveSumValvePidCooling;
        Me.SubSlaveTotalFanPower = (unsigned int) round((TotalFanPower/100.0));
        Me.SubSlaveTotalAirPowerOutput = (unsigned int) round((TotalAirPowerOutput));
        Me.SubSlaveTotalAirPowerBTU = (unsigned int) round((TotalAirPowerBTU));
        MountImageSlaveFlags();
	}
}

void RefreshOutPutRoom_Sec(int SchedRead)
{
    int temp;
    
#if((K_UTAEnable==1 || SplitMode==K_I_Am_InverterFAN || SplitMode==K_I_Am_StandardFAN || SplitMode==K_I_Am_Atex_Fan || SplitMode==K_I_Am_On_Off_Fan) && K_Force_SubSlave_Abil==1 && K_SubSlave_Abil_Only_Fan==1)
	FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_DEFINE_MODE, CoreVentilazione*Touch[0].Script_Split_Pwr * (EngineBox.SystemDisable==0));
#elif(K_Force_DigiIn1_Ventil==1)
    if(iDigiIn1)
        temp = 1;
    else
        temp = Touch[0].Script_Split_Pwr;
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_DEFINE_MODE, Me.DefMode* temp * (EngineBox.SystemDisable==0));
#elif(K_LoopPumpEnable==1)
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_DEFINE_MODE, Me.DefMode* Touch[0].Script_Split_Pwr * (EngineBox.SystemDisable==0) * (Me.Error.Flow==0));
#else
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_DEFINE_MODE, Me.DefMode* Touch[0].Script_Split_Pwr * (EngineBox.SystemDisable==0));
#endif
	FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_CURRENT_REQ_PRESSURE, EngineBox.Ric_Pressione);
	FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_P_GAS, EngineBox.Pressione_Gas_G);
	FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_P_LIQUID, EngineBox.Pressione_Liq_P);
	FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGBOX_PRESS_LIQUID_COND, EngineBox.Pressure_LiquidCond);    
	FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ERRORE_1_BOX_MOTORE, EngineBox.Errori1);
	FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ERRORE_2_BOX_MOTORE, EngineBox.Errori2);
	FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ERRORE_PERS1_BOX_MOTORE, EngineBox.ErroriPers1);
	FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ERRORE_PERS2_BOX_MOTORE, EngineBox.ErroriPers2);    
	FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SUB_SPLIT_TEST_ADDRESS, DiagnosticSplit.SubAddress); 	// Tempo di ciclo elaborazione in mS x diagnostica
	FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_MASTER_STATUS, Me.RoomMaster_Status*(DiagnosticSplit.Reset_Req_Address == Me.My_Address)); 	// Tempo di ciclo elaborazione in mS x diagnostica
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_T_COMP_OUT, EngineBox.Temp_Mandata);
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX2_T_COMP_OUT, EngineBox.Temp_Compressor_Output_C2);
    
#if((K_UTAEnable==1 || SplitMode==K_I_Am_InverterFAN || SplitMode==K_I_Am_StandardFAN || SplitMode==K_I_Am_Atex_Fan || SplitMode==K_I_Am_On_Off_Fan) && K_UTA_SubSlave_Delta_Vel>0 && K_Force_SubSlave_Abil==1)
    temp = Me.Ventil_Mode+K_UTA_SubSlave_Delta_Vel;
    if(temp > Ventil_7)
        temp = Ventil_7;
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VENTIL_MODE, temp);
#else
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VENTIL_MODE, Me.Ventil_Mode);    
#endif
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_CURRENT_SP, Me.SP);

    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_T_COMP_SUCTION, EngineBox.Temp_Ritorno);	
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_T_SEA_WATER, EngineBox.Temp_Acqua_Mare);

    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_CUMULATIVE_ERROR, EngineBox.CumulativeError);    	
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_CUMULATIVE_ERROR2, EngineBox.CumulativeError_C2);    	

    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_CURRENT_MODE_ENGINE_BOX, EngineBox.FunctionMode);
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_CURRENT_MODE_ENGINE_BOX2, EngineBox.FunctionMode_C2);

    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_DOUBLE_COMPRESSSOR_OK, EngineBox.DoubleCompressorOn);
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ID_SPLIT_MASTER, EngineBox.IdMasterSplit);

    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_YOU_ARE_SUBSLAVE, 1);	// Segnalo agli SubSlave che sono SubSlave
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_MY_SPLITMASTER_ADDRESS, Me.My_Address);	//Dico il mio indirizzo ai miei sottoslave.
		
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SUB_SPLIT_TEMP_AMB_MID, Me.TempAmbRealSelection);	
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING, Me.TempAmb_Middle_Heating);	
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING, Me.TempAmb_Middle_Cooling);	

    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_COMPRESSOR_SPEED, EngineBox.Compressor_Speed);
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX2_COMPRESSOR_SPEED, EngineBox.Compressor_Speed_C2);
    
	FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGBOX_SUPERH_HEATCALC, EngineBox.SuperH_HeatCalc);

	FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY, Me.ExpValve_On_Demand_Priority);    
	FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVOLA_CUR_NUMB_PRIORITY, Me.ExpValve_Cur_Number_Priority);    
	FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVOLA_MAX_PERC_PRIORITY, Me.ExpValve_On_Demand_Request);        
    
	FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_SUPERHEAT_CORRECT_COOLING, EngineBox.SuperHeat_Correct_Cooling);        
    
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_FROZEN_ON, EngineBox.Frozen_On);            
    
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_ABIL_DEFROSTING, EngineBox.Abil_Defrosting);        
    
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_INVERTER_UPPER_OUT_FREQ, EngineBox.Inverter_Upper_Out_Freq);            

    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_THRESHOLD_COMPRESSOR_HI, EngineBox.Threshold_Compressor_Hi);            

#if(K_Channel_Gas_Regulation==1)
    FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_CHANNEL_GAS_REGULATION, Me.Channel_Gas_Regulation);    
#endif
    if(SchedRead==0)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_SUPERHEAT, Me.SuperHeat_SP);
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_MIN_PRESS_COOL, Me.Freddo_TempBatt_Min);
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_WORK_PRESS_COOL, Me.Freddo_TempBatt_SP);
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VENTIL_SELECTION, Me.Ventil_Selection);
    }
    
    if(SchedRead==1)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_WORK_PRESS_HEAT, Me.Caldo_TempBatt_SP);
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_MIN_PRESS_HEAT, Me.Caldo_TempBatt_Min);
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_PERFORMANCELOSSBATTERY, Me.PerformanceLossBattery_SP);		
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_UTA_MODE, Touch[0].Uta_Mode);		
    }   
    
    if(SchedRead==2)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_DINAMIC_SUPERHEAT, Me.Dinamic_SuperHeat);               
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVELIM_PERCMAXOPENVALVE, Me.ValveLim.MasterRx.PercMaxOpenValve);               
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVELIM_VALVOLAAPERTA, Me.ValveLim.MasterRx.ValvolaAperta);               
    }    
    if(SchedRead==3)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVELIM_VALVOLACHIUSA, Me.ValveLim.MasterRx.ValvolaChiusa);               
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVELIM_VALVOLAOFF, Me.ValveLim.MasterRx.ValvolaOff);               
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF, Me.ValveLim.MasterRx.Valve_Min_Go_Off);               
    }
    if(SchedRead==4)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL, Me.ValveLim.MasterRx.ValveLostCommCool);  
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT, Me.ValveLim.MasterRx.ValvoLostCommHot);          
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVELIM_APERTURAMAXCALDO, Me.ValveLim.MasterRx.AperturaMaxCaldo);               
    }
    if(SchedRead==5)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVELIM_APERTURAMINCALDO, Me.ValveLim.MasterRx.AperturaMinCaldo);               
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVELIM_INITVALVOLAFREDDO, Me.ValveLim.MasterRx.InitValvolaFreddo);               
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVELIM_APERTURAMAXFREDDO, Me.ValveLim.MasterRx.AperturaMaxFreddo);               
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMINFREDDO, EngineBox.TestAll_MinOpValve_Cool);               
    }
    if(SchedRead==6)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVELIM_APERTURAMINFREDDO, Me.ValveLim.MasterRx.AperturaMinFreddo);               
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO, Me.ValveLim.MasterRx.ValveNoWorkMinFreddo);               
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO, Me.ValveLim.MasterRx.ValveNoWorkMaxFreddo);          
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMINCALDO, EngineBox.TestAll_MinOpValve_Heat);
    }    
    if(SchedRead==7)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON, Me.FanNewStyleTimeOn);               
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF, Me.FanNewStyleTimeOff);
#if(K_Heater_Abil==1)        
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_SYNC_WR_HEATER_PWR, Touch[0].HeaterPwr);               
#endif
        //FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGBOX_PRESS_LIQUID_COND, EngineBox.PressW_HeatCalc);    
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGBOX_SUPERH_HEATCALC, EngineBox.SuperH_HeatCalc);
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_VALVELIM_VALVOLAFROZEN, Me.ValveLim.MasterRx.ValvolaFrozen);

    }

    if(SchedRead==8)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGBOX_ALL_VALVEOPEN100P, EngineBox.AllValveOpen100p);               
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_PWM_VALUE_OFF, PWMValue[0]);        
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_PWM_VALUE_NIGHT, PWMValue[1]);        
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_PWM_VALUE_SPEED1, PWMValue[2]);        
    }

    if(SchedRead==9)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_PWM_VALUE_SPEED2, PWMValue[3]);        
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_PWM_VALUE_SPEED3, PWMValue[4]);        
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_PWM_VALUE_SPEED4, PWMValue[5]);        
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_PWM_VALUE_SPEED5, PWMValue[6]);        
    }

    if(SchedRead==10)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_PWM_VALUE_SPEED6, PWMValue[7]);        
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_PWM_VALUE_SPEED7, PWMValue[8]);        
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_MAX_FAN_SPEED, Me.MaxFanVentil_Mode);        
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT, Me.CorrectSuperHeatAirOut_Local);      
    }

    if(SchedRead==11)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_RD_VALVE_ONDEMAND_MIN_PERC_VAL, Me.ValveOnDemand_Min_Percent_Val);      
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_CONDENSER_PRESS_ABIL, EngineBox.Condenser_Press_Abil);              
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_PRESSURE_LO_LIQ, EngineBox.PressureLoLiqValue);              
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_PRESSURE_LO_GAS, EngineBox.PressureLoGasValue);              
    }

    if(SchedRead==12)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_PRESSURE_HI_LIQ, EngineBox.PressureHiLiqValue);              
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_PRESSURE_HI_GAS, EngineBox.PressureHiGasValue);       
		FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_FAN_CURRENT_LIMIT, Me.Fan_Current_Limit);
		FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_ABIL, EngineBox.TestAll_Abil);
    }

    if(SchedRead==13)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_WORK_PRESS_HEAT, EngineBox.TestAll_TempBatt_SP_Heat);              
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL, EngineBox.TestAll_SuperHeat_SP_Cool);       
		FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_WORK_PRESS_COOL, EngineBox.TestAll_TempBatt_SP_Cool);
		FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMAXFREDDO, EngineBox.TestAll_MaxOpValve_Cool);
    }

    if(SchedRead==14)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL_PGAIN, EngineBox.TestAll_SuperHeat_Cool_pGain);              
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL_IGAIN, EngineBox.TestAll_SuperHeat_Cool_iGain);       
		FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL_DGAIN, EngineBox.TestAll_SuperHeat_Cool_dGain);
		FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID, EngineBox.TestAll_SuperHeat_Cool_Time_Exec_PID);
    }

    if(SchedRead==15)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_M3H_MIN, Me.M3h_Min);              
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_M3H_MAX, Me.M3h_Max);       
		FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_MIN_CURRENT_FAN, Me.Min_Current_Fan);
		FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_MAX_CURRENT_FAN, Me.Max_Current_Fan);
    }

    if(SchedRead==16)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_EFFICIENCY_FAN, Me.Efficiency_Fan);              
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_SUPPLY_FAN, Me.Supply_Fan);       
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_ECOMODE_STATUS, EngineBox.Eco_Mode_Status);       
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_SET_TEMP_AIR_OUT, EngineBox.Set_Temp_Air_Out); 
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_DEHUMI_ABIL, EngineBox.Dehumi_Abil); 
    
    }

    if(SchedRead==17)
    {
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_SET_HUMI, EngineBox.Set_Humi);   
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_SET_DELTA_TEMP_MIN_DEHUMI, EngineBox.Set_Delta_Temp_Min_Dehumi);   
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_SET_FAN_SPEED_DEHUMI, EngineBox.Set_Fan_Speed_Dehumi);   
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_SET_PRESS_DEHUMI, EngineBox.Set_Press_Dehumi);   
        FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_WR_SET_TEMP_AIR_OUT_DEHUMI, EngineBox.Set_Temp_Air_Out_Dehumi);   
    }
   
	//FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_FUNCTION_MODE, SlaveMode);    //deprecata per redere sottoslave indipendente
	//FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_CURRENT_EVAP_TEMP, Me.Evap_Temp); //deprecata per rendere sottoslave indipendente
	//FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_COMPRESSOR_SPEED, EngineBox.Compressor_Speed);
	//FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_T_COMP_OUT, EngineBox.Temp_Mandata);
	//FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_T_CONDENSER, EngineBox.Temp_Condensatore);
	//FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_T_GAS, EngineBox.Temp_Collettore_Grande);
	//FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_T_LIQUID, EngineBox.Temp_Collettore_Piccolo);
	//FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_RELE_SATUS, EngineBox.ReleSt);
	//FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_INPUT_STATUS, EngineBox.InputSt);
	//FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_STATUS_BOX_MOTORE, EngineBox.StatoCompressore);
	//FuncWriteBrcReg_Sec (WriteSplitRegBrdCst, REG_SPLIT_ENGINE_BOX_CYCLE_TIME, EngineBox.SystemCycleTime); 	// Tempo di ciclo elaborazione in mS x diagnostica

}

void RefreshAllOutPut_Sec(void)
{
//	char i=0;
    static int SlaveSchedRead = 0;
 
	if(EnableMySecBus)
	{
#if (K_EnableModBus_Sec==1)
    #if (K_EnableHiSpeedMyBus_Sec==1)
            Change_ProtocolBusSlave(K_ModBus_Baud115200, K_ModBus_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #else
            Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #endif	         
#else        
    #if (K_EnableHiSpeedMyBus_Sec==1)
            Change_ProtocolBusSlave(K_MyBus_Baud115200, K_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #else
            Change_ProtocolBusSlave(K_MyBus_Baud57600, K_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    #endif	         
#endif    	    
        
        
		RefreshOutPutRoom_Sec(SlaveSchedRead);
        SlaveSchedRead += 1;
        if(SlaveSchedRead>17)
            SlaveSchedRead=0;

    /*
		for(i=0; i<Max_Dimmer_Sec; i++)
			RefreshOutPutDimmer_Sec(&Dimmer[i], i);
	*/
	}
}

void MountImageSlaveFlags(void)
{

	Me.Room.Split_Flags_Enable_1_16 = Room[16].Enable << 15 | Room[15].Enable << 14 | Room[14].Enable << 13 | Room[13].Enable << 12 | 
										   Room[12].Enable << 11 | Room[11].Enable << 10 | Room[10].Enable << 9 | Room[9].Enable << 8 | 
										   Room[8].Enable << 7 | Room[7].Enable << 6 | Room[6].Enable << 5 | Room[5].Enable << 4 | 
										   Room[4].Enable << 3 | Room[3].Enable << 2 | Room[2].Enable << 1 | Room[1].Enable;	
	
	Me.Room.Split_Flags_Offline_1_16 =Room[16].OffLine << 15 | Room[15].OffLine << 14 | Room[14].OffLine << 13 | Room[13].OffLine << 12 | 
										   Room[12].OffLine << 11 | Room[11].OffLine << 10 | Room[10].OffLine << 9 | Room[9].OffLine << 8 | 
										   Room[8].OffLine << 7 | Room[7].OffLine << 6 | Room[6].OffLine << 5 | Room[5].OffLine << 4 | 
										   Room[4].OffLine << 3 | Room[3].OffLine << 2 | Room[2].OffLine << 1 | Room[1].OffLine;
	
	Me.Room.Split_Flags_Online_1_16 = Room[16].OnLine << 15 | Room[15].OnLine << 14 | Room[14].OnLine << 13 | Room[13].OnLine << 12 | 
										   Room[12].OnLine << 11 | Room[11].OnLine << 10 | Room[10].OnLine << 9 | Room[9].OnLine << 8 | 
										   Room[8].OnLine << 7 | Room[7].OnLine << 6 | Room[6].OnLine << 5 | Room[5].OnLine << 4 | 
										   Room[4].OnLine << 3 | Room[3].OnLine << 2 | Room[2].OnLine << 1 | Room[1].OnLine;
	
	Me.Room.Split_Flags_Alarm_1_16 =  Room[16].CumulativeAlarm << 15 | Room[15].CumulativeAlarm << 14 | Room[14].CumulativeAlarm << 13 | Room[13].CumulativeAlarm << 12 | 
										   Room[12].CumulativeAlarm << 11 | Room[11].CumulativeAlarm << 10 | Room[10].CumulativeAlarm << 9 | Room[9].CumulativeAlarm << 8 | 
										   Room[8].CumulativeAlarm << 7 | Room[7].CumulativeAlarm << 6 | Room[6].CumulativeAlarm << 5 | Room[5].CumulativeAlarm << 4 | 
										   Room[4].CumulativeAlarm << 3 | Room[3].CumulativeAlarm << 2 | Room[2].CumulativeAlarm << 1 | Room[1].CumulativeAlarm;
    
	Me.Room.Split_Flags_EnableAndOffline_1_16 =(Room[16].OffLine&&Room[16].Enable) << 15 | (Room[15].OffLine&&Room[15].Enable) << 14 | (Room[14].OffLine&&Room[14].Enable) << 13 | (Room[13].OffLine&&Room[13].Enable) << 12 | 
										   (Room[12].OffLine&&Room[12].Enable) << 11 | (Room[11].OffLine&&Room[11].Enable) << 10 | (Room[10].OffLine&&Room[10].Enable) << 9 | (Room[9].OffLine&&Room[9].Enable) << 8 | 
										   (Room[8].OffLine&&Room[8].Enable) << 7 | (Room[7].OffLine&&Room[7].Enable) << 6 | (Room[6].OffLine&&Room[6].Enable) << 5 | (Room[5].OffLine&&Room[5].Enable) << 4 | 
										   (Room[4].OffLine&&Room[4].Enable) << 3 | (Room[3].OffLine&&Room[3].Enable) << 2 | (Room[2].OffLine&&Room[2].Enable) << 1 | (Room[1].OffLine&&Room[1].Enable);

		
	
}

