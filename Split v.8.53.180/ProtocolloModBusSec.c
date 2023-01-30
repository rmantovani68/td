//----------------------------------------------------------------------------------
//	Progect name:	ProtocolloModBus.c
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			18/06/2011
//	Description:	Corpo delle funzioni che gestiscono la comunicazione ModBus con il sistema
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
#include <libpic30.h>
#include <math.h>
#include "DefinePeriferiche.h"
#include "Timer.h"
#include "usart.h"
#include "ADC.h"
#include "PWM.h"
#include "EEPROM.h"
#include "Valvola_PassoPasso.h"
#include "Driver_Comunicazione.h"
#include "Driver_ComunicazioneSec.h"
#include "Driver_ModBusSec.h"
#include "ProtocolloComunicazione.h"
#include "ProtocolloComunicazioneSec.h"
#include "ProtocolloModBusSec.h"
#include "Core.h"
#include "PID.h"
#include "FWSelection.h"
#include "delay.h"

extern unsigned ComunicationLost;				// flag di comunicazione persa
extern volatile int ModCntComErrSec;
extern volatile int ModTimeOutComErrSec;
extern volatile int ModCRC_ComErrSec;
  

volatile int DisableModComErrorCounter = 0;   

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------

unsigned char WaitModRxSec(int TimeOut_ms)
{
	TimeOutModPktRxSec.Value = 0;				// resetto il timer per il time out della risposta
	TimeOutModPktRxSec.TimeOut = 0;			//
	TimeOutModPktRxSec.Time = TimeOut_ms;		// lo configuro come richiesto
	TimeOutModPktRxSec.Enable = 1;				// lo faccio partire

    WaitingReplaySec = 1;
	while((TimeOutModPktRxSec.TimeOut == 0) && (Mod_LastRxSec.Valid_Data == 0))	// attendo un evento
		continue;
    WaitingReplaySec = 0;
    
	if(Mod_LastRxSec.Valid_Data != 0)		// se ho ricevuto un dato valido 
	{
		TimeOutModPktRxSec.Enable = 0;			// fermo il timer
		TimeOutModPktRxSec.TimeOut = 0;		// resetto il time out
		if( (Mod_LastRxSec.Buffer[0] == Mod_BufferTxSec.Buffer[0]) &&	// verifico il pacchetto
			(Mod_LastRxSec.Buffer[1] == Mod_BufferTxSec.Buffer[1]))
			return 0;											// ritorno 0 se è ok
	}									// in caso contrario
	TimeOutModPktRxSec.Enable = 0;			// fermo il timer
	TimeOutModPktRxSec.TimeOut = 0;		// resetto il time out
    //if(Touch[0].Enable)        
    if(DisableModComErrorCounter==0)   
    {
        ModTimeOutComErrSec +=1;    
        Me.ComError.ModTimeOutComErrSec +=1;    
    } 
	return 1;							// ritorno 1
}

unsigned char Send_WaitModRxSec(unsigned char Address, char Comando, int Registro, int Data, int TimeOut_ms, char Retry, int AddressBased)
{
	char Result;
    
	RetryCntSec = 0;
	while (RetryCntSec < Retry)
	{	
		RetryCntSec +=1;
		Mod_Write_CmdSec(Address, Comando, Registro - AddressBased, Data);
		Result = WaitModRxSec(TimeOut_ms);
		if (Result == 0)
		{
			Mod_LastRxSec.Valid_Data = 0;
			//DelaymSec(2); //2);	// v.8.14.47 - Pausa per garantire una minima distanza tra un Tx e il successivo 	
            DelayuSec(k_ModWaitSendMessage);        // Nota: alla velocità piu bassa (38400bps) la pausa dovrebbe essere 910uS !!!
			return 0;		// se ricevo un pacchetto valido 
		}
		
	}
    //if(Touch[Address].Enable)     
    if(DisableModComErrorCounter==0)
    {
        ModCntComErrSec++;
        Me.ComError.ModCntComErrSec++;
    }
	Mod_LastRxSec.Valid_Data = 0;
	return 1;				// se non ho nessuna risposta valida per dopo il numero di tentativi richiesti
}

unsigned char Send_WriteModRegSec(unsigned char Address, int Registro, int Data, int TimeOut_ms, char Retry, char Cmd, int AddressBased)
{
	char Result;
    
	RetryCntSec = 0;
	while (RetryCntSec < Retry)
	{
		RetryCntSec +=1;
		Mod_Write_CmdSec(Address, Cmd, Registro - AddressBased, Data);
		Result = WaitModRxSec(TimeOut_ms);
		if ((Result == 0) &&
			(Mod_LastRxSec.Buffer[2] == Mod_BufferTxSec.Buffer[2]) &&
			(Mod_LastRxSec.Buffer[3] == Mod_BufferTxSec.Buffer[3]) &&
			(Mod_LastRxSec.Buffer[4] == Mod_BufferTxSec.Buffer[4]) &&
			(Mod_LastRxSec.Buffer[5] == Mod_BufferTxSec.Buffer[5])  )
		{
			Mod_LastRxSec.Valid_Data = 0;
			//DelaymSec(2);// 2);			// v.8.16.68 - Pausa per garantire una minima distanza tra un Tx e il successivo 		
            DelayuSec(k_ModWaitSendMessage);        // Nota: alla velocità piu bassa (38400bps) la pausa dovrebbe essere 910uS !!!
			return 0;
		}
	}
    //if(Touch[Address].Enable)        
    if(DisableModComErrorCounter==0)     
    {
        ModCntComErrSec++;
        Me.ComError.ModCntComErrSec++;
    }
	Mod_LastRxSec.Valid_Data = 0;
	return 1;
}


//-----------------------------------------------------------------
// Funzioni solo per Touch
//-----------------------------------------------------------------
void ReadModRegSec (char Address, int REG, volatile int * StoreReg)
{
	unsigned char TouchAddres;
	char Result;
	TouchAddres = Address-Add_TouchRoom;
	
	if(Touch[TouchAddres].OnLine)
	{
		Result = Send_WaitModRxSec(Address, ReadModCmd, REG, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased);
		if(Result == 0)	// Il registro dei touch hanno un offset di 1
		{
			(*StoreReg) = Mod_LastRxSec.Data;
			Mod_LastRxSec.Valid_Data = 0;
		}
		else
		{
			Touch[TouchAddres].OffLine = 1;
			Touch[TouchAddres].OnLine = 0;
			Mod_LastRxSec.Valid_Data = 0;
		}
	}
}

unsigned char ReadModListRegSec (char DataIndexSec, int REG, int RegNum)
{
	unsigned char TouchAddres;
	char Result;
	//TouchAddres = DataIndexSec-Add_TouchRoom;
	TouchAddres = DataIndexSec-Add_TouchRoom;
	//if(Touch[DataIndexSec].OnLine)
	if(Touch[TouchAddres].OnLine)
	{
		//Result = Send_WaitModRxSec(TouchAddres, ReadModCmd, REG, RegNum, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased);
		Result = Send_WaitModRxSec(DataIndexSec, ReadModCmd, REG, RegNum, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased);
		if(Result == 0)	// Il registro dei touch hanno un offset di 1
		{
			Mod_LastRxSec.Valid_Data = 0;
			return 0;
		}
		else
		{
			Touch[TouchAddres].OffLine = 1;
			Touch[TouchAddres].OnLine = 0;
			Mod_LastRxSec.Valid_Data = 0;
			return 1;
		}
	}
	return 1;
}

void WriteModRegSec (char Address, int REG, int SendData)
{
	unsigned char TouchAddres;
	char Result;
	TouchAddres = Address-Add_TouchRoom;

	if(Touch[TouchAddres].OnLine)
	{
		Result = Send_WriteModRegSec(Address, REG, SendData, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);
		if(Result == 1)
		{
			Touch[TouchAddres].OffLine = 1;
			Touch[TouchAddres].OnLine = 0;
		}
	}
}

void WriteModListRegSec (char Address, int REG, int SendData)
{
	unsigned char TouchAddres;
	char Result;
	TouchAddres = Address-Add_TouchRoom;	// Il registro dei touch hanno un offset di 1

	if(Touch[TouchAddres].OnLine)
	{
		Result = Send_WriteModRegSec(Address, REG, SendData, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModListCmd, K_ModBusZeroBased);
		if(Result == 1)
		{
			Touch[TouchAddres].OffLine = 1;
			Touch[TouchAddres].OnLine = 0;
		}
	}
}


//-----------------------------------------------------------------
// Funzioni solo per Inverter
//-----------------------------------------------------------------
void ReadModRegSecInv(char Address, int REG, int * StoreReg)
{
	char Result;

    if(Inverter.OffLine == 1)
    {
        Result = Send_WaitModRxSec(Address, ReadModCmd, REG, 1, MaxModReadWaitSec, 1, K_ModBusOneBased);
        if(Result == 0)	// Se l'inverter ha risposto
            Inverter.OffLine = 0;
    }
    if(Inverter.OffLine == 0)
    {
        Result = Send_WaitModRxSec(Address, ReadModCmd, REG, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusOneBased);
        if(Result == 0)	// Il registro dei touch hanno un offset di 1
        {
            (*StoreReg) = Mod_LastRxSec.Data;
            Mod_LastRxSec.Valid_Data = 0;
        }
        else
        {
            Inverter.OffLine = 1;
            Mod_LastRxSec.Valid_Data = 0;
        }
    }
}

void WriteModRegSecInv(char Address, int REG, int SendData)
{
	char Result;

 	if(!Inverter.OffLine)
	{
		Result = Send_WriteModRegSec(Address, REG, SendData, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusOneBased);
		if(Result == 1)
		{
			Inverter.OffLine = 1;
		}
	}
}

//-----------------------------------------------------------------
// Funzioni solo per ziehl Abegg modbus
//-----------------------------------------------------------------
void ReadModRegSecZiehgAbbModbusFan(char Address, int REG, int * StoreReg)
{
	char Result;

    Result = Send_WaitModRxSec(Address, ReadModCmd, REG, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusOneBased);
    if(Result == 0)	// Il registro dei touch hanno un offset di 1
    {
        (*StoreReg) = Mod_LastRxSec.Data;
        Mod_LastRxSec.Valid_Data = 0;
        Inverter.OffLine = 0;
    }
    else
    {
        Inverter.OffLine = 1;
        Mod_LastRxSec.Valid_Data = 0;
    }
}

void WriteModRegSecZiehgAbbModbusFan(char Address, int REG, int SendData)
{
	char Result;

 	if(!Inverter.OffLine)
	{
		Result = Send_WriteModRegSec(Address, REG, SendData, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusOneBased);
		if(Result == 1)
		{
			Inverter.OffLine = 1;
		}
	}
}

void ReadModInputSecZiehgAbbModbusFan (char Address, int REG, volatile int * StoreReg)
{
	char Result;
    
    Result = Send_WaitModRxSec(Address, ReadModInput, REG, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusOneBased);

    if(Result == 0)	// Il registro dei touch hanno un offset di 1
    {
        (*StoreReg) = Mod_LastRxSec.Data;
        Mod_LastRxSec.Valid_Data = 0;
        Inverter.OffLine = 0;
    }
    else
    {
        Inverter.OffLine = 1;
        Mod_LastRxSec.Valid_Data = 0;
    }
}

//-----------------------------------------------------------------
// Funzioni solo per EBM modbus
//-----------------------------------------------------------------
void ReadModRegSecEBMModbusFan(char Address, int REG, int * StoreReg)
{
	char Result;

    Result = Send_WaitModRxSec(Address, ReadModCmd, REG, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusOneBased);
    if(Result == 0)	// Il registro dei touch hanno un offset di 1
    {
        (*StoreReg) = Mod_LastRxSec.Data;
        Mod_LastRxSec.Valid_Data = 0;
        Inverter.OffLine = 0;
    }
    else
    {
        Inverter.OffLine = 1;
        Mod_LastRxSec.Valid_Data = 0;
    }
}

void WriteModRegSecEBMModbusFan(char Address, int REG, int SendData)
{
	char Result;

 	if(!Inverter.OffLine)
	{
		Result = Send_WriteModRegSec(Address, REG, SendData, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusOneBased);
		if(Result == 1)
		{
			Inverter.OffLine = 1;
		}
	}
}

void ReadModInputSecEBMModbusFan (char Address, int REG, volatile int * StoreReg)
{
	char Result;
    
    Result = Send_WaitModRxSec(Address, ReadModInput, REG, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusOneBased);

    if(Result == 0)	// Il registro dei touch hanno un offset di 1
    {
        (*StoreReg) = Mod_LastRxSec.Data;
        Mod_LastRxSec.Valid_Data = 0;
        Inverter.OffLine = 0;
    }
    else
    {
        Inverter.OffLine = 1;
        Mod_LastRxSec.Valid_Data = 0;
    }
}

//-----------------------------------------------------------------
// Funzioni solo per NetB
//-----------------------------------------------------------------
void ReadModRegSecNetB (char Address, int REG, volatile int * StoreReg)
{
	char Result;
	if(NetB[Address-Add_NetBRoom].OnLine)
	{
		Result = Send_WaitModRxSec(Address, ReadModCmd, REG, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased);
		if(Result == 0)	// Il registro dei touch hanno un offset di 1
		{
			(*StoreReg) = Mod_LastRxSec.Data;
			Mod_LastRxSec.Valid_Data = 0;
		}
		else
		{
			NetB[Address-Add_NetBRoom].OffLine = 1;
			NetB[Address-Add_NetBRoom].OnLine = 0;            
		}
	}
}

unsigned char ReadModListRegSecNetB (unsigned char DataIndexSec, int REG, int RegNum)
{
	char NetBAddres;
	NetBAddres = (DataIndexSec+Add_NetBRoom);
	if(NetB[DataIndexSec].OnLine)
	{
		if(Send_WaitModRxSec(NetBAddres, ReadModCmd, REG, RegNum, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
		{
			NetB[DataIndexSec].OffLine = 1;
			NetB[DataIndexSec].OnLine = 0;                        
			Mod_LastRxSec.Valid_Data = 0;
			return 1;
		}
		else
		{
			Mod_LastRxSec.Valid_Data = 0;
			return 0;
		}
	}
	return 1;
}

void WriteModRegSecNetB (unsigned char Address, int REG, int SendData)
{
	unsigned char NetBAddres;
	char Result;
	NetBAddres = Address-Add_NetBRoom;

	if(NetB[NetBAddres].OnLine)
	{
		Result = Send_WriteModRegSec(Address, REG, SendData, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Il registro dei touch hanno un offset di 1
		if(Result == 1)
		{
			NetB[NetBAddres].OffLine = 1;
			NetB[NetBAddres].OnLine = 0;
		}
	}
}

void WriteCoilModRegSecNetB (unsigned char Address, int REG, int SendData)
{
	unsigned char NetBAddres;
	char Result;
	NetBAddres = Address-Add_NetBRoom;

	if(NetB[NetBAddres].OnLine)
	{
		Result = Send_WriteModRegSec(Address, REG, SendData, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModForceSingleCoil, K_ModBusZeroBased);	// Il registro dei touch hanno un offset di 1
		if(Result == 1)
		{
			NetB[NetBAddres].OffLine = 1;
			NetB[NetBAddres].OnLine = 0;
		}
	}
}


void ReadCoilModRegSecNetB (char Address, int REG, int * StoreReg)
{
	char Result;
	if(NetB[Address-Add_NetBRoom].OnLine)
	{
		Result = Send_WaitModRxSec(Address, ReadModCoil, REG, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased);
		if(Result == 0)	// Il registro dei touch hanno un offset di 1
		{
			(*StoreReg) = Mod_LastRxSec.Data;
			Mod_LastRxSec.Valid_Data = 0;
		}
		else
		{
			NetB[Address-Add_NetBRoom].OffLine = 1;
			NetB[Address-Add_NetBRoom].OnLine = 0;            
		}
	}
}


void WriteModListRegSecNetB (unsigned char Address, int REG, int SendData)
{
	unsigned char NetBAddres;
	char Result;
	NetBAddres = Address-Add_NetBRoom;	// Il registro dei touch hanno un offset di 1

	if(NetB[NetBAddres].OnLine)
	{
		Result = Send_WriteModRegSec(Address, REG, SendData, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModListCmd, K_ModBusZeroBased);	// Il registro dei touch hanno un offset di 1
		if(Result == 1)
		{
			NetB[NetBAddres].OffLine = 1;
			NetB[NetBAddres].OnLine = 0;
		}
	}     
}






int ExtracRegSec(char NumReg)
{
	// recupero un valore int dalla coda del buffer l'indice dei registri va da 1 al limite della richiesta
	unsigned char Indice;
	int Data;

	Indice = 1+(NumReg*2);
	/////Data = Mod_LastRxSec.Buffer[Indice]<<8;
	Data = ((Mod_LastRxSec.Buffer[Indice]<<8)&0xFF00);	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
	Data |= (Mod_LastRxSec.Buffer[Indice+1]&0x00FF);
	return Data;
}

void InsertRegSec(char NumReg, int Data)
{
    unsigned char Indice;
    
	Indice = 5+(NumReg*2);
	/////Mod_BufferTxSec.Buffer[Indice] = (char)((Data&0xFF00)>>8);
	Mod_BufferTxSec.Buffer[Indice] = (char)(((Data&0xFF00)>>8)&0x00FF);	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
	Mod_BufferTxSec.Buffer[Indice+1] = (char)((Data&0x00FF));
}




void InitInverterFan(TypInverter * Inverter, int InvAddr)
{
    Change_ProtocolBusSlave(K_ModBus_Baud38400, K_ModBus_Generic);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud

    Inverter->OffLine = 0;
    
    WriteModRegSecInv (InvAddr, REG_INVERTER_TIMEOUT_COMUNICATION,K_FAN_INVERTER_TIMEOUT_COMUNICATION);    
    WriteModRegSecInv (InvAddr, REG_INVERTER_ACTION_TMT_COMUNICATION,K_FAN_INVERTER_ACTION_TMT_COMUNICATION);    
    
    //WriteModRegSecInv (InvAddr, REG_INVERTER_TIME_ACC,K_FAN_INVERTER_TIME_ACC);    
    //WriteModRegSecInv (InvAddr, REG_INVERTER_TIME_DEC,K_FAN_INVERTER_TIME_DEC);    
    //WriteModRegSecInv (InvAddr, REG_INVERTER_MAX_FREQUENCY,K_FAN_INVERTER_MAX_FREQUENCY);    
#if(K_InverterMax65HZ==0)   
    WriteModRegSecInv (InvAddr, REG_INVERTER_UPPER_LIM_FREQ,K_FAN_INVERTER_UPPER_LIM_FREQ_50HZ);    
#else
    WriteModRegSecInv (InvAddr, REG_INVERTER_UPPER_LIM_FREQ,K_FAN_INVERTER_UPPER_LIM_FREQ_65HZ);    
#endif
    //WriteModRegSecInv (InvAddr, REG_INVERTER_BASE_FREQUENCY,K_FAN_INVERTER_BASE_FREQUENCY);    
    //WriteModRegSecInv (InvAddr, REG_INVERTER_VF_CONTROL_MODE,K_FAN_INVERTER_VF_CONTROL_MODE);    
    WriteModRegSecInv (InvAddr, REG_INVERTER_AUTO_START_FREQ,K_FAN_INVERTER_AUTO_START_FREQ);    
    //WriteModRegSecInv (InvAddr, REG_INVERTER_BASE_FREQ_VOLTAGE,K_FAN_INVERTER_BASE_FREQ_VOLTAGE);    
    //WriteModRegSecInv (InvAddr, REG_INVERTER_MOTOR_RATED_SPEED,K_FAN_INVERTER_MOTOR_RATED_SPEED);    
}




void RefreshInverterFan(TypInverter * Inverter, int InvAddr)
{	
    Change_ProtocolBusSlave(K_ModBus_Baud38400, K_ModBus_Generic);                            // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    
    ReadModRegSecInv(InvAddr, REG_INVERTER_TRIP_CODE_MONITOR, &(Inverter->ErrorCode));
    if(!Inverter->OffLine)
    {
        ReadModRegSecInv(InvAddr, REG_INVERTER_OUT_FREQ, &(Inverter->Out_Freq));
        ReadModRegSecInv(InvAddr, REG_INVERTER_INPUT_POWER, &(Inverter->Out_Power));
        ReadModRegSecInv(InvAddr, REG_INVERTER_OUT_CURRENT, &(Inverter->Out_Current));


            // aggiorno il comando di run
            // Modifica v4.4.117: Gestione Run/Stop Inverter

        if(Inverter->FreqFan>0)	
            WriteModRegSecInv(InvAddr, REG_INVERTER_COMMAND_SET,K_VAL_INVERTER_RUN_COMMAND);
        else 
            WriteModRegSecInv(InvAddr, REG_INVERTER_COMMAND_SET,K_VAL_INVERTER_STOP_COMMAND);             

    // scrivo la frequenza da impostare
        WriteModRegSecInv(InvAddr, REG_INVERTER_FREQUENCY_SET,Inverter->FreqFan);             


        // Invio comando di reset Fault
        // Solo se è scattato uno dei Fault specificati...
        if(Inverter->ErrorCode==K_VAL_INVERTER_ERROR_CODE_Err4 || Inverter->ErrorCode==K_VAL_INVERTER_ERROR_CODE_Err5 || Inverter->ErrorCode==K_VAL_INVERTER_ERROR_CODE_Sout || 
           Inverter->ErrorCode==K_VAL_INVERTER_ERROR_CODE_E21 || Inverter->ErrorCode==K_VAL_INVERTER_ERROR_CODE_E26 || Inverter->ErrorCode==K_VAL_INVERTER_ERROR_CODE_E39)
        {
            WriteModRegSecInv(InvAddr, REG_INVERTER_COMMAND_SET,K_VAL_INVERTER_RESET_FLT_COMMAND);             
        }	

        // Se è scattato un qualsiasi errore sull'inverter 
        if(Inverter->ErrorCode!=0)
        {
            if(((Me.RoomMaster_Status & 0x0100) && (DiagnosticSplit.Reset_Req_Address == Me.My_Address) && (DiagnosticSplit.SubAddress == 0)) || 
            ((Me.RoomMaster_Status & 0x0100) && (DiagnosticSplit.SubAddress == Me.My_Address) && Me.I_Am_SubSlave))
            {
                WriteModRegSecInv(InvAddr, REG_INVERTER_COMMAND_SET,K_VAL_INVERTER_RESET_FLT_COMMAND);             
            }
        }
    }
	Inverter->FanInverterFault = Inverter->ErrorCode!=0 || Inverter->OffLine;
}

void RefreshZiehgAbbModbusFan(TypInverter * Inverter, int InvAddr)
{	
    Change_ProtocolBusSlave(K_ModBus_Baud38400, K_ModBus_Generic);                            // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    
    ReadModInputSecZiehgAbbModbusFan(InvAddr, REG_MOD_ZIEGABB_OP_COND1, &(Inverter->ErrorCode));
    if(!Inverter->OffLine)
    {
        ReadModInputSecZiehgAbbModbusFan(InvAddr, REG_MOD_ZIEGABB_SPEED_ACTUAL, &(Me.Fan_Tacho));
        
        Me.Fan_Tacho = Filter(&FilterFan_Tacho, Me.Fan_Tacho, K_Campioni_Filter_FanTacho);

        // scrivo la velocità da impostare
        WriteModRegSecZiehgAbbModbusFan(InvAddr, REG_MOD_ZIEGABB_SPEED_CONTROL, ValueToPercent(Me.Pwm_Value,255));             

    }
	Inverter->FanInverterFault = /*Inverter->ErrorCode!=0 || */Inverter->OffLine;
}

void RefreshEBMModbusFan(TypInverter * Inverter, int InvAddr)
{	
    unsigned int VelRead=0;    
    int VelWrite=0;
    int ValPerc=0;
    Change_ProtocolBusSlave(K_ModBus_Baud19200, K_ModBus_Generic);                            // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud 
    
    ReadModInputSecEBMModbusFan(InvAddr, REG_MOD_EBM_MOTOR_STATUS, &(Inverter->ErrorCode));
    if(!Inverter->OffLine)
    {
        ReadModInputSecEBMModbusFan(InvAddr, REG_MOD_EBM_ACT_SPEED, &(VelRead));
        
        if(Me.Ventil_Selection==K_EBM_3200RPM_ModBus)
             Me.Fan_Tacho = (unsigned int)(((float)VelRead/64000.0)*3200);
        
        ValPerc = ValueToPercent(Me.Pwm_Value,255);
        if(ValPerc>0)
            VelWrite = (int)(round((float)ValPerc * 655.35));
        else
            VelWrite = 0;

        // scrivo la velocità da impostare
        WriteModRegSecEBMModbusFan(InvAddr, REG_MOD_EBM_SETPOINT, VelWrite);             

    }
	Inverter->FanInverterFault = /*Inverter->ErrorCode!=0 || */Inverter->OffLine;
}


//--------------------------------------------
// Routines di gestione dei Touch
//--------------------------------------------
void Search_Touch(void)
{
	char Result;
    unsigned int i = 0;

    if(Me.EnableHiSpeedModBusTouch_Sec==1)        
        Change_ProtocolBusSlave(K_ModBus_Baud115200, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    else
        Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud

    
	// Scansiono la rete alla ricerca dei touch
	for(i=0; i<MaxTouch; i++)
	{
		Result = Send_WaitModRxSec((Add_TouchRoom+i), ReadModCmd, REG_TOUCH_PAGE, 1, MaxModChkWaitSec, MaxModChkRetriesSec, K_ModBusZeroBased);
		Touch[i].Enable = (Result == 0) ;
		Touch[i].OffLine = (Result != 0);
        Touch[i].OnLine = (Touch[i].Enable == 1) && (Touch[i].OffLine == 0);
		Mod_LastRxSec.Valid_Data = 0;

        if(Touch[i].OnLine) 
        {
            Touch[i].CntErrTouch = 0;
            Touch[i].PermanentOffline = 0;
            Me.ModBusDeviceDetected=K_ModDeviceExor;
            InitTouch();
        } 
	}
}

/*
void Resume_TouchOffLine(char * Cnt)
{
	Result = 0;
	
	// Provo a recuperare i moduli Touch
	if((Touch[(*Cnt)].Enable == 1) && (Touch[(*Cnt)].OffLine == 1))
	{
		Result = Send_WaitModRxSec((Add_TouchRoom +(*Cnt)), ReadModCmd, REG_TOUCH_PAGE, 1, MaxModChkWaitSec, MaxModChkRetriesSec, K_ModBusZeroBased);
		Touch[(*Cnt)].OffLine = (Result != 0);
		
		if(Touch[(*Cnt)].OffLine == 0)
		{
			Touch[(*Cnt)].OnLine = (Touch[(*Cnt)].Enable == 1) && (Touch[(*Cnt)].OffLine == 0);
			InitTouch();
		}
		
		Mod_LastRxSec.Valid_Data = 0;
	}
	(*Cnt) += 1;
	if((*Cnt) >= MaxTouch)
		(*Cnt) = 0;
    
    Touch[(*Cnt)].OnLine = (Touch[(*Cnt)].Enable == 1) && (Touch[(*Cnt)].OffLine == 0);

}
*/

void CheckHotPlugTouch(unsigned char * Cnt)
{
	char Result = 0;
	unsigned char CurrentId = 0;

	while((*Cnt) < MaxTouch)				// Provo a recuperare i Touch
	{
		CurrentId = (*Cnt);
		if((Touch[CurrentId].Enable == 0))
		{
            if(Me.EnableHiSpeedModBusTouch_Sec==1)
                Change_ProtocolBusSlave(K_ModBus_Baud115200, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
            else
                Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud    

            DisableModComErrorCounter = 1;
            Result = Send_WaitModRxSec((Add_TouchRoom +(*Cnt)), ReadModCmd, REG_TOUCH_PAGE, 1, MaxModChkWaitSec, 1, K_ModBusZeroBased);
            DisableModComErrorCounter = 0;
            
            Touch[(*Cnt)].OffLine = (Result != 0);                
			Touch[(*Cnt)].Enable = !Touch[(*Cnt)].OffLine;	// Se lo trovo lo abilito.                
                
            Touch[(*Cnt)].OnLine = (Touch[(*Cnt)].Enable == 1) && (Touch[(*Cnt)].OffLine == 0);     
            
            if(Touch[(*Cnt)].OnLine)		// se ho trovato il touch e mi rispnde online
            {
               Touch[(*Cnt)].CntErrTouch = 0;
               Touch[(*Cnt)].PermanentOffline = 0;
               Me.ModDeviceType = K_ModDeviceExor;                
                InitTouch();
            }     
            
			(*Cnt) += 1;
			break;
		}
		(*Cnt) += 1;
	}
	if(((*Cnt) >= MaxTouch) || ((*Cnt) < 0))
		(*Cnt) = 0;
}



void Resume_TouchOffLine(unsigned char * Cnt)
{
	char Result;
	
	// Provo a recuperare i moduli Touch
	if((Touch[(*Cnt)].Enable == 1) && (Touch[(*Cnt)].OffLine == 1))
	{
        
        if(Me.EnableHiSpeedModBusTouch_Sec==1)        
            Change_ProtocolBusSlave(K_ModBus_Baud115200, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
        else        
            Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
        
		Result = Send_WaitModRxSec((Add_TouchRoom +(*Cnt)), ReadModCmd, REG_TOUCH_PAGE, 1, MaxModChkWaitSec, MaxModChkRetriesSec, K_ModBusZeroBased);
		Touch[(*Cnt)].OffLine = (Result != 0);
		
        
        if(Touch[(*Cnt)].OffLine == 1)
        {
            if(Touch[(*Cnt)].PermanentOffline == 0)
            {
                Touch[(*Cnt)].CntErrTouch++;
                if(Touch[(*Cnt)].CntErrTouch > k_MAX_Cnt_Err_Touch)
                    Touch[(*Cnt)].PermanentOffline = 1;
            }
        }
        else
        {
            Touch[(*Cnt)].CntErrTouch = 0;
            Touch[(*Cnt)].OnLine = (Touch[(*Cnt)].Enable == 1) && (Touch[(*Cnt)].OffLine == 0);
            if(Touch[(*Cnt)].PermanentOffline == 1)
            {        
                Touch[(*Cnt)].PermanentOffline = 0;
                InitTouch();                
            }
        }      
		
		Mod_LastRxSec.Valid_Data = 0;
	}
	(*Cnt) += 1;
	if((*Cnt) >= MaxTouch)
		(*Cnt) = 0;  
}
/*
void Resume_TouchOffLine(void)
{

	static int CntErrTouch = 0;

	if(Touch.OffLine == 1)
	{
		if(Touch.PermanentOffline == 0)
		{
			CntErrTouch++;
			if(CntErrTouch > k_MAX_Cnt_Err_Touch)
				Touch.PermanentOffline = 1;
			else
				Touch.OffLine = 0;
		}
	}
	else
	{
		CntErrTouch = 0;
		Touch.PermanentOffline = 0;
	}
}
*/


// Legge da Touch
void RefreshTouchInPut(char TouchAddres)
{
	int Temp = 0;
    unsigned char DataIndexSec;
	
	DataIndexSec = TouchAddres-Add_TouchRoom;
		
    // v8.4.22 (07/08/2015 02:16) - Spostata lettura registri pagina Home sempre (fuori dai case)

    ReadModRegSec (TouchAddres, REG_TOUCH_PAGE, &(Touch[DataIndexSec].Current_Page));
    if(Touch[DataIndexSec].Current_Page==0) 
        InitTouch();
    if(Touch[DataIndexSec].OnLine)
    {     
        ReadModListRegSec (TouchAddres, REG_TOUCH_MODE, 1);
        if(Touch[DataIndexSec].OnLine)
        {
            Touch[0].DefMode		= RecheckDataReadTouch(ExtracRegSec(1), Touch[0].DefMode, &Touch[0].Previous.DefMode, &Touch[0].CheckCounter.DefMode);   //ExtracRegSec(1);	//REG_TOUCH_PWR  
        }           
        //ReadModRegSec (TouchAddres, REG_TOUCH_MODE, &(Touch[0].DefMode));
    }                  
    //ReadModListRegSec (TouchAddres, REG_TOUCH_PAGE, 22);            

    ReadModListRegSec (TouchAddres, REG_TOUCH_PWR, 1);
    if(Touch[DataIndexSec].OnLine)
    {
        Touch[0].Script_Split_Pwr		= RecheckDataReadTouch(ExtracRegSec(1), Touch[0].Script_Split_Pwr, &Touch[0].Previous.Script_Split_Pwr, &Touch[0].CheckCounter.Script_Split_Pwr);   //ExtracRegSec(1);	//REG_TOUCH_PWR  
    }

    ReadModListRegSec (TouchAddres, REG_TOUCH_SP_ROOM, 5);
    if(Touch[DataIndexSec].OnLine)
    {
        Touch[0].SetPoint       		= RecheckDataReadTouch(ExtracRegSec(1), Touch[0].SetPoint, &Touch[0].Previous.SetPoint, &Touch[0].CheckCounter.SetPoint);   //ExtracRegSec(1);	//REG_TOUCH_SP_ROOM
        Touch[0].FanMode        		= RecheckDataReadTouch(ExtracRegSec(2), Touch[0].FanMode, &Touch[0].Previous.FanMode, &Touch[0].CheckCounter.FanMode);   //ExtracRegSec(2);	//REG_TOUCH_FAN_SPEED

        Touch[0].Script_SetPoint_F  	= RecheckDataReadTouch(ExtracRegSec(4), Touch[0].Script_SetPoint_F, &Touch[0].Previous.Script_SetPoint_F, &Touch[0].CheckCounter.Script_SetPoint_F);   //ExtracRegSec(4);	//REG_TOUCH_SP_ROOM_F
        Touch[0].Uta_Mode             	= RecheckDataReadTouch(ExtracRegSec(5), Touch[0].Uta_Mode, &Touch[0].Previous.Uta_Mode, &Touch[0].CheckCounter.Uta_Mode);   //ExtracRegSec(5);	//REG_TOUCH_UTA_MODE            
    }

#if(K_Force_Lim_Hi_SP_Split==1)
    if(Me.DefMode==CoreRiscaldamento)
    {
        if(Syncronize.SetPoint>K_Forced_Lim_Hi_SP_Split_Hot)
            Syncronize.SetPoint = K_Forced_Lim_Hi_SP_Split_Hot;

        if(Touch[0].SetPoint>K_Forced_Lim_Hi_SP_Split_Hot)
        {
            Touch[0].SetPoint=K_Forced_Lim_Hi_SP_Split_Hot;
            WriteModRegSec (TouchAddres, REG_TOUCH_SP_ROOM, Touch[0].SetPoint);
        }
    }
#endif
    SyncronizeDataNew(&Syncronize.Script_Split_Pwr, &Syncronize.Old.Script_Split_Pwr, &Touch[0].Script_Split_Pwr, &Syncronize.Engine.Script_Split_Pwr, REG_TOUCH_PWR, 1);				 	
    SyncronizeDataNew(&Syncronize.SetPoint, &Syncronize.Old.SetPoint, &Touch[0].SetPoint, &Syncronize.Engine.SetPoint, REG_TOUCH_SP_ROOM, 2);							
    SyncronizeDataNew(&Syncronize.FanMode, &Syncronize.Old.FanMode, &Touch[0].FanMode, &Syncronize.Engine.FanMode, REG_TOUCH_FAN_SPEED, 3);				
    SyncronizeDataNew(&Syncronize.Script_SetPoint_F, &Syncronize.Old.Script_SetPoint_F, &Touch[0].Script_SetPoint_F, &Syncronize.Engine.Script_SetPoint_F, REG_TOUCH_SP_ROOM_F, 4);
    SyncronizeDataNew(&Syncronize.Uta_Mode, &Syncronize.Old.Uta_Mode, &Touch[0].Uta_Mode, &Syncronize.Engine.Uta_Mode, REG_TOUCH_UTA_MODE, 5);
#if(K_Heater_Abil==1)
   ReadModListRegSec (TouchAddres, REG_TOUCH_HEATER_PWR, 1);
    if(Touch[DataIndexSec].OnLine)
    {
        Touch[0].HeaterPwr  = ExtracRegSec(1);	//REG_TOUCH_HEATER_PWR
    }

    SyncronizeDataNew(&Syncronize.HeaterPwr, &Syncronize.Old.HeaterPwr, &Touch[0].HeaterPwr, &Syncronize.Engine.HeaterPwr, REG_TOUCH_HEATER_PWR, 19);
#endif    
/*        
        TimerMaxTimeCmdAddress.Enable = DiagnosticSplit.OldValueCmdAddress != DiagnosticSplit.CmdAddress;
        if(!TimerMaxTimeCmdAddress.Enable)
        {
            TimerMaxTimeCmdAddress.Value = 0;
            TimerMaxTimeCmdAddress.TimeOut = 0;
        }
*/            
        if(Me.My_Address==k_Split_Master_Add)
        {
            if(DiagnosticSplit.CmdAddress==DiagnosticSplit.Ready_CMD_Address)
                DiagnosticSplit.DataReady = 1;

            ReadModListRegSec (TouchAddres, REG_TOUCH_SPLIT_SET_POWER, 4);
            if(Touch[DataIndexSec].OnLine)
            {
                Touch[0].DiagWrite.Pwr			= RecheckDataReadTouch(ExtracRegSec(1), Touch[0].DiagWrite.Pwr, &Touch[0].Previous.DiagWrite.Pwr, &Touch[0].CheckCounter.DiagWrite.Pwr);   //ExtracRegSec(1);	//REG_TOUCH_SPLIT_SET_POWER
                Touch[0].DiagWrite.SetPoint		= RecheckDataReadTouch(ExtracRegSec(2), Touch[0].DiagWrite.SetPoint, &Touch[0].Previous.DiagWrite.SetPoint, &Touch[0].CheckCounter.DiagWrite.SetPoint);   //ExtracRegSec(1);	//REG_TOUCH_SP_ROOM//ExtracRegSec(2);	//REG_TOUCH_SPLIT_SET_SETPOINT
                Touch[0].DiagWrite.FanMode		= RecheckDataReadTouch(ExtracRegSec(3), Touch[0].DiagWrite.FanMode, &Touch[0].Previous.DiagWrite.FanMode, &Touch[0].CheckCounter.DiagWrite.FanMode);   //ExtracRegSec(3);	//REG_TOUCH_SPLIT_SET_FANSPEED
                Touch[0].DiagWrite.SetPoint_F	= RecheckDataReadTouch(ExtracRegSec(4), Touch[0].DiagWrite.SetPoint_F, &Touch[0].Previous.DiagWrite.SetPoint_F, &Touch[0].CheckCounter.DiagWrite.SetPoint_F);   //ExtracRegSec(4);	//REG_TOUCH_SPLIT_SET_SETPOINT_F
            }

            SyncronizeDataOld(&Syncronize.DiagWritePwr, &Syncronize.Old.DiagWritePwr, &Touch[0].DiagWrite.Pwr, &Syncronize.Engine.DiagWritePwr, REG_TOUCH_SPLIT_SET_POWER, 7);				 	
            SyncronizeDataOld(&Syncronize.DiagWriteSetPoint, &Syncronize.Old.DiagWriteSetPoint, &Touch[0].DiagWrite.SetPoint, &Syncronize.Engine.DiagWriteSetPoint, REG_TOUCH_SPLIT_SET_SETPOINT, 8);				
            SyncronizeDataOld(&Syncronize.DiagWriteFanMode, &Syncronize.Old.DiagWriteFanMode, &Touch[0].DiagWrite.FanMode, &Syncronize.Engine.DiagWriteFanMode, REG_TOUCH_SPLIT_SET_FANSPEED, 9);				
            SyncronizeDataOld(&Syncronize.DiagWriteSetPoint_F, &Syncronize.Old.DiagWriteSetPoint_F, &Touch[0].DiagWrite.SetPoint_F, &Syncronize.Engine.DiagWriteSetPoint_F, REG_TOUCH_SPLIT_SET_SETPOINT_F, 10);				

            ReadModListRegSec (Add_TouchRoom, REG_TOUCH_SPLIT_SET_HEATER_PWR, 1);
             if(Touch[DataIndexSec].OnLine)
             {
                 Touch[0].DiagWrite.HeaterPwr	= RecheckDataReadTouch(ExtracRegSec(1), Touch[0].DiagWrite.HeaterPwr, &Touch[0].Previous.DiagWrite.HeaterPwr, &Touch[0].CheckCounter.DiagWrite.HeaterPwr);
             }

            SyncronizeDataOld(&Syncronize.DiagWriteHeaterPwr, &Syncronize.Old.DiagWriteHeaterPwr, &Touch[0].DiagWrite.HeaterPwr, &Syncronize.Engine.DiagWriteHeaterPwr, REG_TOUCH_SPLIT_SET_HEATER_PWR, 19);				
                
            //ReadModRegSec (TouchAddres, REG_TOUCH_MODE, &(Touch[0].DefMode));                                        
            //SyncronizeDataOld(&Syncronize.DefMode, &Syncronize.Old.DefMode, &Touch[0].DefMode, &Syncronize.Engine.DefMode, REG_TOUCH_MODE, 11);	
            SyncronizeDataBridge(&Syncronize.DefMode, &Syncronize.Old.DefMode, &Touch[0].DefMode, &Syncronize.Engine.DefMode, REG_TOUCH_MODE, 11);	

            ReadModListRegSec (TouchAddres, REG_TOUCH_DIAG_UTA_MODE, 1);
            {
                Touch[0].DiagWrite.Uta_Mode		= RecheckDataReadTouch(ExtracRegSec(1), Touch[0].DiagWrite.Uta_Mode, &Touch[0].Previous.DiagWrite.Uta_Mode, &Touch[0].CheckCounter.DiagWrite.Uta_Mode);   //ExtracRegSec(1);	//REG_TOUCH_DIAG_UTA_MODE
                //Touch[0].Uta_Power		= ExtracRegSec(1);	//REG_TOUCH_UTA_POWER
                //Touch[0].DiagWrite.Uta_Mode		= ExtracRegSec(2);	//REG_TOUCH_UTA_MODE
                //Touch[0].Uta_SP_Room	= ExtracRegSec(3);	//REG_TOUCH_UTA_SP_ROOM
                //Touch[0].Uta_SP_Humi	= ExtracRegSec(4);	//REG_TOUCH_UTA_SP_HUMI
                //Touch[0].Uta_Fan_Speed	= ExtracRegSec(5);	//REG_TOUCH_UTA_FAN_SPEED
                //Touch[0].Uta_SP_Room_F	= ExtracRegSec(7);	//REG_TOUCH_UTA_SP_ROOM_F
            }

            //SyncronizeDataOld(&Syncronize.Uta_Power, &Syncronize.Old.Uta_Power, &Touch[0].Uta_Power, &Syncronize.Engine.Uta_Power, REG_TOUCH_UTA_POWER, 12);	
            SyncronizeDataOld(&Syncronize.DiagUta_Mode, &Syncronize.Old.DiagUta_Mode, &Touch[0].DiagWrite.Uta_Mode, &Syncronize.Engine.DiagUta_Mode, REG_TOUCH_DIAG_UTA_MODE, 13);	
            //SyncronizeDataOld(&Syncronize.Uta_SP_Room, &Syncronize.Old.Uta_SP_Room, &Touch[0].Uta_SP_Room, &Syncronize.Engine.Uta_SP_Room, REG_TOUCH_UTA_SP_ROOM, 14);	
            //SyncronizeDataOld(&Syncronize.Uta_SP_Humi, &Syncronize.Old.Uta_SP_Humi, &Touch[0].Uta_SP_Humi, &Syncronize.Engine.Uta_SP_Humi, REG_TOUCH_UTA_SP_HUMI, 15);	
            //SyncronizeDataOld(&Syncronize.Uta_Fan_Speed, &Syncronize.Old.Uta_Fan_Speed, &Touch[0].Uta_Fan_Speed, &Syncronize.Engine.Uta_Fan_Speed, REG_TOUCH_UTA_FAN_SPEED, 16);	
            //SyncronizeDataOld(&Syncronize.Uta_SP_Room_F, &Syncronize.Old.Uta_SP_Room_F, &Touch[0].Uta_SP_Room_F, &Syncronize.Engine.Uta_SP_Room_F, REG_TOUCH_UTA_SP_ROOM_F, 17);	

            if(EngineBox.AllValveOpen100p==1)                                                           //Se ho il flag di valvole aperte 100% ...
                WriteModRegSec (TouchAddres, REG_TOUCH_SPLIT_SYSTEM_DISABLE, 1); //...lo forzo sul touch.                                            

            //ReadModRegSec (TouchAddres, REG_TOUCH_SPLIT_SYSTEM_DISABLE, &(Touch[0].SystemDisable));
            ReadModListRegSec (TouchAddres, REG_TOUCH_SPLIT_SYSTEM_DISABLE, 1);
            if(Touch[DataIndexSec].OnLine)
            {
                Touch[0].SystemDisable		= RecheckDataReadTouch(ExtracRegSec(1), Touch[0].SystemDisable, &Touch[0].Previous.SystemDisable, &Touch[0].CheckCounter.SystemDisable);
            }           

            //SyncronizeDataOld(&Syncronize.SystemDisable, &Syncronize.Old.SystemDisable, &Touch[0].SystemDisable, &Syncronize.Engine.SystemDisable, REG_TOUCH_SPLIT_SYSTEM_DISABLE, 18);	
            SyncronizeDataBridge(&Syncronize.SystemDisable, &Syncronize.Old.SystemDisable, &Touch[0].SystemDisable, &Syncronize.Engine.SystemDisable, REG_TOUCH_SPLIT_SYSTEM_DISABLE, 18);	

        }			
        ReadModListRegSec (TouchAddres, REG_TOUCH_ECOMODE_ENABLE, 2);			
        if(Touch[DataIndexSec].OnLine)
        {
            Me.EcoModeEnable	= ExtracRegSec(1);	//REG_TOUCH_ECOMODE_ENABLE
            Me.Power_EcoMode	= ExtracRegSec(2);	//REG_TOUCH_POWER_ECOMODE
        }		

        //SyncronizeDataNew(&Syncronize.EcoModeEnable, &Syncronize.Old.EcoModeEnable, &Me.EcoModeEnable, &Syncronize.Engine.EcoModeEnable, REG_TOUCH_ECOMODE_ENABLE, 6);
        //SyncronizeDataNew(&Syncronize.Power_EcoMode, &Syncronize.Old.Power_EcoMode, &Me.Power_EcoMode, &Syncronize.Engine.Power_EcoMode, REG_TOUCH_POWER_ECOMODE, 7);
        SyncronizeDataBridge(&Syncronize.EcoModeEnable, &Syncronize.Old.EcoModeEnable, &Me.EcoModeEnable, &Syncronize.Engine.EcoModeEnable, REG_TOUCH_ECOMODE_ENABLE, 6);
        SyncronizeDataBridge(&Syncronize.Power_EcoMode, &Syncronize.Old.Power_EcoMode, &Me.Power_EcoMode, &Syncronize.Engine.Power_EcoMode, REG_TOUCH_POWER_ECOMODE, 7);
        ReadModListRegSec (TouchAddres, REG_TOUCH_DIAG_BOX_ADDRESS, 3);			
        if(Touch[DataIndexSec].OnLine)
        {
            EngineBox.DiagAddress	= ExtracRegSec(1);	//REG_TOUCH_DIAG_BOX_ADDRESS
            DiagnosticInverter.DiagAddress	= ExtracRegSec(3);	//REG_TOUCH_INVERTER_ADDRESS
        }		

        ReadModListRegSec (TouchAddres, REG_TOUCH_INIT_SP_ROOM, 5);

        if(Touch[DataIndexSec].OnLine)
        {
            Reinit.ToEngBox.SetPoint = ExtracRegSec(1);	// REG_TOUCH_INIT_SP_ROOM	      
            Reinit.ToEngBox.FanSpeed = ExtracRegSec(2);	// REG_TOUCH_INIT_FAN_SPEED	      
            Reinit.ToEngBox.Power = ExtracRegSec(3);		// REG_TOUCH_INIT_POWER	      
            Reinit.ToEngBox.SetPoint_F = ExtracRegSec(4);	// REG_TOUCH_INIT_SP_ROOM_F	      
            Reinit.ToEngBox.TrigInit = ExtracRegSec(5);	// REG_TOUCH_TRIGGER_INIT	      
        }

        ReadModRegSec (TouchAddres, REG_TOUCH_MAX_PWR_LIMIT, &(Touch[DataIndexSec].PowerLimit));
        Me.PowerLimit = Touch[DataIndexSec].PowerLimit;			// setto il registro con i valori da aggiornare
        //Me.PowerLimit= InitPowerLimit;		//ATTENZIONE:VALORE FORZATO PER DISATTIVARE RIDUZIONE DI POTENZA!!!!!

        ReadModRegSec (TouchAddres, REG_TOUCH_SPLIT_TEST_ADDRESS, &(DiagnosticSplit.Address));

        ReadModRegSec (TouchAddres, REG_TOUCH_SPLIT_CMD_ADDRESS, &(DiagnosticSplit.CmdAddress));
        if(!Touch[DataIndexSec].OnLine)
        {
            DiagnosticSplit.CmdAddress = 0;
            DiagnosticSplit.DataReady = 0;
        }

        if(DiagnosticSplit.CmdAddress!=DiagnosticSplit.Ready_CMD_Address)
            DiagnosticSplit.DataReady = 0;

    //Touch[DataIndexSec].FunctionMode = Touch[DataIndexSec].Script_Split_Pwr * Me.DefMode * (EngineBox.SystemDisable==0);	  

        ReadModListRegSec (TouchAddres, REG_TOUCH_OTHER_CABIN_CMD_ON_OFF, 7);

        if(Touch[DataIndexSec].OnLine)
        {
            Me.OtherCabin.CMD_On_Off = ExtracRegSec(1);	// REG_TOUCH_OTHER_CABIN_CMD_ON_OFF	      
            Me.OtherCabin.CMD_SetP = ExtracRegSec(2);      // REG_TOUCH_OTHER_CABIN_CMD_SETP	      
            Me.OtherCabin.CMD_SetP_F = ExtracRegSec(3);	// REG_TOUCH_OTHER_CABIN_CMD_SETP_F	      
            Me.OtherCabin.CMD_Fan = ExtracRegSec(4);       // REG_TOUCH_OTHER_CABIN_CMD_FAN	      
            Me.OtherCabin.CMD_Send = ExtracRegSec(5);      // REG_TOUCH_OTHER_CABIN_CMD_SEND	  
            EngineBox.DoublePumpSwitchTime = ExtracRegSec(6);      // REG_TOUCH_DOUBLE_PUMP_SWITCH_TIME	  
            EngineBox.DoublePumpSelection = ExtracRegSec(7);      // REG_TOUCH_DOUBLE_PUMP_SELECTION	                  
        }

        ReadModRegSec (TouchAddres, REG_TOUCH_OTHER_CABIN_CMD_UTA_MODE, &(Me.OtherCabin.CMD_UTAMode));

        ReadModListRegSec (TouchAddres, REG_TOUCH_MANUAL_PUMP_ABIL, 2);

        if(Touch[DataIndexSec].OnLine)
        {
            EngineBox.Manual_Pump_Abil = ExtracRegSec(1);	// REG_TOUCH_MANUAL_PUMP_ABIL	      
            EngineBox.Manual_Pump_Freq = ExtracRegSec(2)*100;      // REG_TOUCH_MANUAL_PUMP_FREQ	      
        }

        ReadModListRegSec (TouchAddres, REG_TOUCH_TESTALL_ABIL, 7);
        if(Touch[DataIndexSec].OnLine)
        {            
            Touch[0].TestAll_Abil              = ExtracRegSec(1);                  // REG_TOUCH_TESTALL_ABIL                 294
            Touch[0].TestAll_SuperHeat_SP_Cool = ExtracRegSec(2);                  // REG_TOUCH_TESTALL_SUPERHEAT_SP_COOL    295
            Touch[0].TestAll_SuperHeat_SP_Heat = ExtracRegSec(3);                  // REG_TOUCH_TESTALL_SUPERHEAT_SP_HEAT    296
            Touch[0].TestAll_MaxOpValve_Cool   = ExtracRegSec(4);                  // REG_TOUCH_TESTALL_MAX_OPEN_VALVE_COOL  297
            Touch[0].TestAll_MaxOpValve_Heat   = ExtracRegSec(5);                  // REG_TOUCH_TESTALL_MAX_OPEN_VALVE_HEAT  298
            Touch[0].TestAll_TempBatt_SP_Cool  = ExtracRegSec(6);                  // REG_TOUCH_TESTALL_WORK_TEMP_COOL       299
            Touch[0].TestAll_TempBatt_SP_Heat  = ExtracRegSec(7);                  // REG_TOUCH_TESTALL_WORK_TEMP_HEAT       300
        }

        ReadModListRegSec (TouchAddres, REG_TOUCH_TESTALL_MIN_OPEN_VALVE_COOL, 4);            
        if(Touch[DataIndexSec].OnLine)
        {            
            Touch[0].TestAll_MinOpValve_Cool        = ExtracRegSec(1);                  // REG_TOUCH_TESTALL_MIN_OPEN_VALVE_COOL
            Touch[0].TestAll_MinOpValve_Heat        = ExtracRegSec(2);                  // REG_TOUCH_TESTALL_MIN_OPEN_VALVE_HEAT
            Touch[0].TestAll_MinOpValve_Frigo       = ExtracRegSec(3);                  // REG_TOUCH_TESTALL_MIN_OPEN_VALVE_FRIGO
            Touch[0].TestAll_MinOpValve_Freezer     = ExtracRegSec(4);                 // REG_TOUCH_TESTALL_MIN_OPEN_VALVE_FREEZER
        }            

        if(EngineBox.Frozen_Abil!=0)
        {
            ReadModListRegSec (TouchAddres, REG_TOUCH_TESTALL_FROZEN_ABIL, 7);
            if(Touch[DataIndexSec].OnLine)
            {            
                Touch[0].TestALL_Frozen_Abil            = ExtracRegSec(1);                  // REG_TOUCH_TESTALL_FROZEN_ABIL       
                Touch[0].TestAll_SuperHeat_SP_Frigo     = ExtracRegSec(2);                  // REG_TOUCH_TESTALL_SUPERHEAT_SP_FRIGO
                Touch[0].TestAll_SuperHeat_SP_Freezer   = ExtracRegSec(3);                  // REG_TOUCH_TESTALL_SUPERHEAT_SP_FREEZER 
                Touch[0].TestAll_MaxOpValve_Frigo       = ExtracRegSec(4);                  // REG_TOUCH_TESTALL_MAX_OPEN_VALVE_FRIGO
                Touch[0].TestAll_MaxOpValve_Freezer     = ExtracRegSec(5);                  // REG_TOUCH_TESTALL_MAX_OPEN_VALVE_FREEZER
                Touch[0].TestAll_TempBatt_SP_Frigo      = ExtracRegSec(6);                  // REG_TOUCH_TESTALL_WORK_TEMP_FRIGO
                Touch[0].TestAll_TempBatt_SP_Freezer    = ExtracRegSec(7);                  // REG_TOUCH_TESTALL_WORK_TEMP_FREEZER
            }

            ReadModListRegSec (TouchAddres, REG_TOUCH_ABIL_DEFROSTING, 6);
            if(Touch[DataIndexSec].OnLine)
            {            
                Touch[0].Abil_Defrosting	            = ExtracRegSec(1);                  // REG_TOUCH_ABIL_DEFROSTING     
                Touch[0].Defrosting_Step_Time           = ExtracRegSec(2);                  // REG_TOUCH_DEFROSTING_STEP_TIME
                Touch[0].Defrosting_Time                = ExtracRegSec(3);                  // REG_TOUCH_DEFROSTING_TIME     
                Touch[0].Defrosting_Temp_Set            = ExtracRegSec(4);                  // REG_TOUCH_DEFROSTING_TEMP_SET 
                Touch[0].Dripping_Time                  = ExtracRegSec(5);                  // REG_TOUCH_DRIPPING_TIME    
                Touch[0].Valvola_Frozen                 = ExtracRegSec(6);                  // REG_TOUCH_VALVOLA_FROZEN
            }
        }

        ReadModListRegSec (TouchAddres, REG_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN, 8);
        if(Touch[DataIndexSec].OnLine)
        {            
            Touch[0].TestAll_SuperHeat_Heat_pGain        = ExtracRegSec(1);                  // REG_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN
            Touch[0].TestAll_SuperHeat_Heat_iGain        = ExtracRegSec(2);                  // REG_TOUCH_TESTALL_SUPERHEAT_HEAT_IGAIN
            Touch[0].TestAll_SuperHeat_Heat_dGain        = ExtracRegSec(3);                  // REG_TOUCH_TESTALL_SUPERHEAT_HEAT_DGAIN
            Touch[0].TestAll_SuperHeat_Heat_Time_Exec_PID    = ExtracRegSec(4);                  // REG_TOUCH_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID
            Touch[0].TestAll_SuperHeat_Cool_pGain        = ExtracRegSec(5);                  // REG_TOUCH_TESTALL_SUPERHEAT_COOL_PGAIN
            Touch[0].TestAll_SuperHeat_Cool_iGain        = ExtracRegSec(6);                  // REG_TOUCH_TESTALL_SUPERHEAT_COOL_IGAIN
            Touch[0].TestAll_SuperHeat_Cool_dGain        = ExtracRegSec(7);                  // REG_TOUCH_TESTALL_SUPERHEAT_COOL_DGAIN
            Touch[0].TestAll_SuperHeat_Cool_Time_Exec_PID    = ExtracRegSec(8);                  // REG_TOUCH_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID
        }

        ReadModListRegSec (TouchAddres, REG_TOUCH_MAINTENANCE_FLAG, 2);
        if(Touch[DataIndexSec].OnLine)
        {            
            Touch[0].Maintenance_Flag        = ExtracRegSec(1);                  // REG_TOUCH_MAINTENANCE_FLAG
            Touch[0].Maintenance_Set_Time    = ExtracRegSec(2);                  // REG_TOUCH_MAINTENANCE_SET_TIME
        }

        ReadModListRegSec (TouchAddres, REG_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT, 4);
        if(Touch[DataIndexSec].OnLine)
        {            
            Touch[0].SuperHeat_Cool_Min_SetPoint       = ExtracRegSec(1);                  // REG_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT
            Touch[0].SuperHeat_Cool_Max_Value_Correct  = ExtracRegSec(2);                  // REG_TOUCH_SUPERHEAT_COOL_MAX_VALUE_CORRECT
            Touch[0].SuperHeat_Heat_Min_SetPoint       = ExtracRegSec(3);                  // REG_TOUCH_SUPERHEAT_HEAT_MIN_SETPOINT
            Touch[0].SuperHeat_Heat_Max_SetPoint       = ExtracRegSec(4);                  // REG_TOUCH_SUPERHEAT_HEAT_MAX_SETPOINT
        }

        ReadModListRegSec (Add_TouchRoom, REG_TOUCH_THRESHOLD_COMPRESSOR_HI, 1);
        if(Touch[DataIndexSec].OnLine)
        {            
            Touch[0].Threshold_Compressor_Hi        = ExtracRegSec(1);                  // REG_TOUCH_THRESHOLD_COMPRESSOR_HI
        }

        ReadModRegSec (TouchAddres, REG_TOUCH_SET_TEMP_AIR_OUT, &(Touch[0].Set_Temp_Air_Out));
            
        if(EngineBox.Dehumi_Abil==1)
        {
            ReadModListRegSec (TouchAddres, REG_TOUCH_SET_HUMI, 5);
            if(Touch[DataIndexSec].OnLine)
            {            
                Touch[0].Set_Humi                = ExtracRegSec(1);
                Touch[0].Set_Delta_Temp_Min_Dehumi = ExtracRegSec(2);
                Touch[0].Set_Fan_Speed_Dehumi    = ExtracRegSec(3);
                Touch[0].Set_Press_Dehumi        = ExtracRegSec(4);
                Touch[0].Set_Temp_Air_Out_Dehumi = ExtracRegSec(5);
            }
        }   
/*            
        ReadModListRegSec (TouchAddres, REG_TOUCH_ON_PRIORITY_MODE, 4);

        if(Touch[DataIndexSec].OnLine)
        {
            Touch[0].On_Priority_Mode = ExtracRegSec(1);                // REG_TOUCH_ON_PRIORITY_MODE
            Touch[0].ValveOnDemand_Min_Percent_Val = ExtracRegSec(2);   // REG_TOUCH_VALVE_ONDEMAND_MIN_PERC_VAL
            Touch[0].Max_Lim_ON_Demand_Total_Score = ExtracRegSec(3);   // REG_TOUCH_MAX_LIM_ONDEMAND_TOTAL_SCORE
            Touch[0].TimeOnDemandValveSwitch = ExtracRegSec(4);         // REG_TOUCH_TIME_ONDEMAND_VALVE_SWITCH
        }
*/                   

    ReadModRegSec (TouchAddres, REG_TOUCH_PAGE, &(Touch[DataIndexSec].Current_Page));
    switch((char)Touch[DataIndexSec].Current_Page)
    {
        case 0:
            InitTouch();
            break;
        case 2:		// pagina 2 -> Home
            /*
            if(!Touch[DataIndexSec].OffLine)
            {
                if(Send_WaitModRxSec(TouchAddres, ReadModCmd, REG_TOUCH_PWR, 2, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
                    Touch[DataIndexSec].OffLine = 1;
                else
                {
                    Touch[DataIndexSec].Script_Split_Pwr = ExtracRegSec(1);	// REG_TOUCH_PWR
                    Touch[DataIndexSec].DefMode = ExtracRegSec(2);			// REG_TOUCH_MODE
                }
                Mod_LastRxSec.Valid_Data = 0;

                if(Send_WaitModRxSec(TouchAddres, ReadModCmd, REG_TOUCH_SP_ROOM, 3, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
                    Touch[DataIndexSec].OffLine = 1;
                else
                {
                    Touch[DataIndexSec].SetPoint = ExtracRegSec(1);
                    Touch[DataIndexSec].FanMode = ExtracRegSec(2);
                    Touch[DataIndexSec].FunctionMode = ExtracRegSec(3);
                }
                Mod_LastRxSec.Valid_Data = 0;
            }
             */
            break;

        case 3:	// pagina 3 -> Power Manager
            /*
            // DEBUG Bridge
            if(EngineBox.PowerLimit!=EngineBox.OldPowerLimit)								// Se è arrivato un aggiornamento dal Master
            {
                WriteModRegSec (TouchAddres, REG_TOUCH_MAX_PWR_LIMIT, EngineBox.PowerLimit);	// Scrivo valore arrivato dal Master
                EngineBox.OldPowerLimit = EngineBox.PowerLimit;								// Aggiorno OLD
            }				
            // DEBUG Bridge						
            if(!Touch[DataIndexSec].OffLine)														// Leggo registro "PowerLimit" dal touch
            {
                if(Send_WaitModRxSec(TouchAddres, ReadModCmd, REG_TOUCH_MAX_PWR_LIMIT, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
                    Touch[DataIndexSec].OffLine = 1;
                else
                {
                    Touch[DataIndexSec].PowerLimit = ExtracRegSec(1);				// Leggo dal Touch
                    Me.PowerLimit = Touch[DataIndexSec].PowerLimit;			// e aggiorno "Me" con i valore letto (sarà letto dal Master
                }
                Mod_LastRxSec.Valid_Data = 0;
            }
             */
            /* Estratto dalla pagina per refresh continuo 
            if(!Touch[DataIndexSec].OffLine)
            {
                if(Send_WaitModRxSec(TouchAddres, ReadModCmd, REG_TOUCH_MAX_PWR_LIMIT, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
                    Touch[DataIndexSec].OffLine = 1;
                else
                {
                    Touch[DataIndexSec].PowerLimit = ExtracRegSec(1);
                    Me.PowerLimit = Touch[DataIndexSec].PowerLimit;			// setto il registro con i valori da aggiornare

                }
                Mod_LastRxSec.Valid_Data = 0;
            }		*/		
            break;

        case 4:		// pagina 4 -> Service Login
            break;	

        case 5:		// pagina 5 -> Faults1
            break;

        case 6:		// pagina 6 -> Faults2
        case 17:    // DEBUG errore compressione    
            ReadModRegSec (TouchAddres, REG_TOUCH_CLEAR_ERROR, &(Temp));

            if((Temp & 0x0001)!=0)
            {	
                Me.Error.ClearErrReq = 1;
                Temp = 0;
            }
            break;

        case 7:		// pagina 7 -> Faults3		
            ReadModRegSec (TouchAddres, REG_TOUCH_SPLIT_CLEAR_ERROR, &(Temp));

            if((Temp & 0x0001)!=0)
            {	
                //Me.Error.Battery_Defrost = 0;
                Me.Error.EEV_Open = 0;
                Me.Error.EEV_Close = 0;
                Me.Error.BatteriaGhiacciata = 0;
                Me.Error.Fan_Fault = 0;
                Me.Error.Fan_OverLoad = 0;
                Me.Error.EEV_Test_End = 0;
                Me.Error.SPTemperatureFault = 0;
                Me.Error.ClearSplitErrReq = 1;
                Temp = 0;
            }				
            break;

        case 8:		// pagina 8 -> Faults4			//Ex.Pag.8
        case 62:    // Errori com slave                
            ReadModRegSec (TouchAddres, REG_TOUCH_DIAGNOSTIC_CLEAR_ERROR, &(Temp));

            if((Temp & 0x0001)!=0)
            {	
                Me.Error.ClearDiagnosticSplitErrReq = 1;
                Temp = 0;
            }					

            if(My_Address == k_Split_Master_Add && !Me.I_Am_SubSlave)
            {
                ReadModRegSec (TouchAddres, REG_TOUCH_SUB_SPLIT_TEST_ADDRESS, &(DiagnosticSplit.SubAddress));

                if(!Touch[DataIndexSec].OnLine)
                    DiagnosticSplit.SubAddress = 0;
            }
            break;

        case 9:		// pagina 9 -> Diagnostica Box motore			//Ex.Pag.9
            break;

        case 10:		// pagina 10 -> Diagnostica Split				//Ex.Pag.10
            break;

        case 11:		// pagina 11 -> Diagnostica Other Split			//Ex.Pag.11 #v16
            if(My_Address == k_Split_Master_Add && !Me.I_Am_SubSlave)
            {					
                ReadModRegSec (TouchAddres, REG_TOUCH_SUB_SPLIT_TEST_ADDRESS, &(DiagnosticSplit.SubAddress));

                if(!Touch[DataIndexSec].OnLine)
                DiagnosticSplit.SubAddress = 0;
            }				
            break;

        case 12:		// pagina 12 -> Diagnostica Inverter			//Ex.Pag.12
            break;

        case 13:		// pagina 13 -> VNC			
            break;				

        case 14:		// pagina 14-> Power Monitor
            break;
        case 15:		// pagina 15-> Engine Detail
            if(My_Address == k_Split_Master_Add && !Me.I_Am_SubSlave)
            {					
                ReadModRegSec (TouchAddres, REG_TOUCH_SUB_SPLIT_TEST_ADDRESS, &(DiagnosticSplit.SubAddress));

                if(!Touch[DataIndexSec].OnLine)
                DiagnosticSplit.SubAddress = 0;
            }					
            break;
        case 16:		// pagina 16-> Air Handler Detail				
            break;	

        case 21:    // DEBUG errore compressione    
            ReadModRegSec (TouchAddres, REG_TOUCH_UTA_BOX_CLEAR_ERROR, &(Temp));
            if((Temp & 0x0002)!=0)
            {	
                EngineBox.UTA_H.ClearError = 1;
                Temp = 0;
            }		
            break;
        case 26:		// pagina 26-> Set Split
            if(My_Address == k_Split_Master_Add && !Me.I_Am_SubSlave)
            {					
                ReadModRegSec (TouchAddres, REG_TOUCH_SUB_SPLIT_TEST_ADDRESS, &(DiagnosticSplit.SubAddress));

                if(!Touch[DataIndexSec].OnLine)
                DiagnosticSplit.SubAddress = 0;
            }					
            break;		

        case 52:
            ReadModRegSec (TouchAddres, REG_TOUCH_CLEAR_ERROR, &(Temp));

            if((Temp & 0x0001)!=0)
            {	
                Me.Error.ClearComErr = 1;
                Temp = 0;
            }
            break;
    }

}

void RefreshAllTouchInPut(void)
{
    unsigned int i = 0;
    
    if(Me.EnableHiSpeedModBusTouch_Sec==1)        
        Change_ProtocolBusSlave(K_ModBus_Baud115200, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    else        
        Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    
	/////for(i=1; i<=MaxTouch; i++)
	for(i=Add_TouchRoom; i< (Add_TouchRoom+MaxTouch); i++)
		RefreshTouchInPut(i);

	//if(!Me.NetBSelect)	//Se sono un touch...
	if(Me.My_Address == k_Split_Master_Add && !Me.I_Am_SubSlave)	 	//Se sono uno slave Master..
    {
		//Me.DefMode = Touch[0].DefMode;									//Non ricevo il "Me.DefMode" da Master, ma lo leggo da touch
#if(K_Abil_DigIn_SystemDisable==1)            
        Me.InputSystemDisable = iDigiIn3;   
        EngineBox.SystemDisable = Touch[0].SystemDisable || Me.InputSystemDisable || EngineBox.AllValveOpen100p;
#else                
        EngineBox.SystemDisable = Touch[0].SystemDisable || EngineBox.AllValveOpen100p;
#endif      
                
	}
	if(Me.I_Am_SubSlave)												//Se sono un SubSlave
		Touch[0].FunctionMode = Me.DefMode * (EngineBox.SystemDisable==0);								//Scrivo il FunctionMode nella struct del touch per poterlo rileggere corretto quando serve (Essendo che il SubSlave non può avere touch).
}


// Scrive su Touch
void RefreshTouchOutPut(char TouchAddres)
{
	unsigned int Inverter_InCurr;
	unsigned int Inverter_OutCurr;
	unsigned int Inverter_MaxCurr;
	unsigned int Inverter_InVolt;
	unsigned int Inverter_OutVolt;
	unsigned int Inverter_Max_Power_Reduce;
	unsigned int Split_Exp_Valve_Perc;
    int Split_Exp_Valve_Perc_Neg;
	unsigned int temp=0;
    unsigned char DataIndexSec;

 	DataIndexSec = TouchAddres-Add_TouchRoom;
	if(Touch[DataIndexSec].OnLine)
	{	       
        int vK_Power_Inverter;
        int vK_Supply_Inverter;

        vK_Power_Inverter = DiagnosticInverter.Power_Inverter;
        vK_Supply_Inverter = Me.Inverter_Supply;//K_Supply_Inverter;
					
		EngineBox.PowerLimit = Me.PowerLimit;
		
		InsertRegSec(1, 1*(DiagnosticSplit.DataReady==0));					// REG_TOUCH_DIAG_DATA_PROCESS
		InsertRegSec(2, 1*(temp!=0));										// REG_TOUCH_BUZZER_ERROR
		WriteModListRegSec (TouchAddres, REG_TOUCH_DIAG_DATA_PROCESS, 2);
		
		WriteModRegSec (TouchAddres, REG_TOUCH_COUNTER_COMUNICATION,8); //3);
        
		WriteModRegSec (TouchAddres, REG_TOUCH_MAX_FANVENTIL_MODE, Me.MaxFanVentil_Mode);
        
		
		//WriteModRegSec (TouchAddres, REG_TOUCH_DIAG_DATA_PROCESS, 1*(DiagnosticSplit.DataReady==0));			

        
        if(Me.Error.ReadMasReset)
        {
            if(Me.Error.ClearErrReq)
            {
                WriteModRegSec (TouchAddres, REG_TOUCH_CLEAR_ERROR, 0);
                Me.Error.ClearErrReq = 0;
            }
            if(Me.Error.ClearComErr)
            {
                WriteModRegSec (TouchAddres, REG_TOUCH_CLEAR_ERROR, 0);
                Me.Error.ClearComErr = 0;
            }
            if(Me.Error.ClearDiagnosticSplitErrReq)
            {
                WriteModRegSec (TouchAddres, REG_TOUCH_DIAGNOSTIC_CLEAR_ERROR, 0);
                Me.Error.ClearDiagnosticSplitErrReq = 0;
            }
            Me.Error.ReadMasReset = 0;
		}
		if(Me.Error.ClearSplitErrReq)
			WriteModRegSec (TouchAddres, REG_TOUCH_SPLIT_CLEAR_ERROR, 0);
		        
		if(EngineBox.UTA_H.ClearError)
			WriteModRegSec (TouchAddres, REG_TOUCH_UTA_BOX_CLEAR_ERROR, 0);
        
        if(Me.OtherCabin.ReadOk==1)
        {
            WriteModRegSec (TouchAddres, REG_TOUCH_OTHER_CABIN_CMD_SEND, 0);
            Me.OtherCabin.ReadOk=0;
        }
		
        if((EngineBox.Abil_Defrosting & 0x0004)!=0)
        {
            WriteModRegSec (TouchAddres, REG_TOUCH_ABIL_DEFROSTING, (Touch[0].Abil_Defrosting & 0xfffb));
        }
		
		// NEW v8.4.22
		if(Me.My_Address == k_Split_Master_Add && !Me.I_Am_SubSlave)								// Se sono uno slave Master...
			WriteModRegSec (TouchAddres, REG_TOUCH_I_AM_MASTER_SPLIT, 1);		// Attivo flag su Touch per gestione Hide/View pagine riservate a MasterSlave
		else
		{
			WriteModRegSec (TouchAddres, REG_TOUCH_I_AM_MASTER_SPLIT, 0);		// Dsattivo flag su Touch
			WriteModRegSec (TouchAddres, REG_TOUCH_MODE, Me.DefMode);			// Se non sono slit master scrivo sul touch il Me.DefMode che arriva da Spplit master che verrà riletto dalla routine di lettura per essere processato da script che elabora functionmode.
//Prova
			//if(Touch[0].Script_Split_Pwr)
			//WriteModRegSec (TouchAddres, REG_TOUCH_FUNCTION_MODE, Touch[0].DefMode*Touch[0].Script_Split_Pwr);			
//Prova			
			
		}
		
		
		WriteModRegSec (TouchAddres, REG_TOUCH_NATION, EngineBox.Touch_Nation);	// Invio la Nazione sul touch per modificare dinamicamente gli oggetti e unità di misura sul touch a seconda della Nazione (USA/EUR)	
		
		WriteModRegSec (TouchAddres, REG_TOUCH_DOUBLE_COMPRESSOR, EngineBox.DoubleCompressorOn);	// Invio la Nazione sul touch per modificare dinamicamente gli oggetti e unità di misura sul touch a seconda della Nazione (USA/EUR)	

		WriteModRegSec (TouchAddres, REG_TOUCH_ECOMODE_STATUS, EngineBox.Eco_Mode_Status);	// Invio lo stato di EcoMOde
		
		//WriteModRegSec (TouchAddres, REG_TOUCH_UTA_ABIL, Touch[0].Uta_Abil);				// Invio lo stato di Abilitazione UTA
		
/*
		InsertRegSec(1, EngineBox.Split.FlagsEnable_1_16);                                        // REG_SPLIT_FLAGS_ENABLE_1_16
		InsertRegSec(2, EngineBox.Split.FlagsOffLine_1_16);                                        // REG_SPLIT_FLAGS_OFFLINE_1_16
		InsertRegSec(3, EngineBox.Split.FlagsOnLine_1_16);                                        // REG_SPLIT_FLAGS_ONLINE_1_16
		InsertRegSec(4, EngineBox.Split.FlagsAlarm_1_16);                                        // REG_SPLIT_FLAGS_ALARM_1_16
		InsertRegSec(5, EngineBox.Split.FlagsEnable_17_32);                                        // REG_SPLIT_FLAGS_ENABLE_17_32
		InsertRegSec(6, EngineBox.Split.FlagsOffLine_17_32);                                        // REG_SPLIT_FLAGS_OFFLINE_17_32
		InsertRegSec(7, EngineBox.Split.FlagsOnLine_17_32);                                        // REG_SPLIT_FLAGS_ONLINE_17_32
		InsertRegSec(8, EngineBox.Split.FlagsAlarm_17_32);                                        // REG_SPLIT_FLAGS_ALARM_17_32        
		WriteModListRegSec (TouchAddres, REG_TOUCH_FLAGS_ENABLE_1_16, 8); 
*/
        
        if(Me.My_Address == k_Split_Master_Add && !Me.I_Am_SubSlave)								// Se sono uno slave Master...
        {        
#if(SplitMode == K_I_Am_OnlyTouchComm)
			WriteModRegSec (TouchAddres, REG_TOUCH_ONLY_COMUNICATION, 1);		// Attivo flag su Touch per gestione Hide/View pagine riservate a MasterSlave
#endif

            InsertRegSec(1, EngineBox.Split.FlagsEnable_1_16);                                        // REG_SPLIT_FLAGS_ENABLE_1_16
            InsertRegSec(2, EngineBox.Split.FlagsOffLine_1_16);                                        // REG_SPLIT_FLAGS_OFFLINE_1_16
            InsertRegSec(3, EngineBox.Split.FlagsOnLine_1_16);                                        // REG_SPLIT_FLAGS_ONLINE_1_16
            InsertRegSec(4, EngineBox.Split.FlagsAlarm_1_16);                                        // REG_SPLIT_FLAGS_ALARM_1_16
            InsertRegSec(5, EngineBox.Split.FlagsEnable_17_32);                                        // REG_SPLIT_FLAGS_ENABLE_17_32
            InsertRegSec(6, EngineBox.Split.FlagsOffLine_17_32);                                        // REG_SPLIT_FLAGS_OFFLINE_17_32
            InsertRegSec(7, EngineBox.Split.FlagsOnLine_17_32);                                        // REG_SPLIT_FLAGS_ONLINE_17_32
            InsertRegSec(8, EngineBox.Split.FlagsAlarm_17_32);                                        // REG_SPLIT_FLAGS_ALARM_17_32        

            InsertRegSec(9, EngineBox.Temp_Mandata);                                        // REG_TOUCH_GRAPH_TEMP_OUTPUT_C1
            InsertRegSec(10, EngineBox.Temp_Compressor_Output_C2);                                        // REG_TOUCH_GRAPH_TEMP_OUTPUT_C2
            InsertRegSec(11, EngineBox.Temp_Ritorno);                                        // REG_TOUCH_GRAPH_TEMP_SUCTION
            InsertRegSec(12, EngineBox.Temp_Condensatore);                                        // REG_TOUCH_GRAPH_TEMP_CONDENSER
            InsertRegSec(13, EngineBox.Temp_Acqua_Mare);                                        // REG_TOUCH_GRAPH_TEMP_SEA_WATER
            InsertRegSec(14, EngineBox.Temp_Collettore_Piccolo);                                        // REG_TOUCH_GRAPH_TEMP_LIQUID
            InsertRegSec(15, EngineBox.Pressione_Gas_G);                                        // REG_TOUCH_GRAPH_PRES_GAS
            InsertRegSec(16, EngineBox.Pressione_Liq_P);                                        // REG_TOUCH_GRAPH_PRES_LIQUID
            InsertRegSec(17, EngineBox.Ric_Pressione);                                        // REG_TOUCH_GRAPH_PRES_REQUIRED
            InsertRegSec(18, (unsigned int)( ((float)EngineBox.Compressor_Speed / (float)255) * 100.0));                                        // REG_TOUCH_GRAPH_SPEED_C1
            InsertRegSec(19, (unsigned int)( ((float)EngineBox.Compressor_Speed_C2 / (float)255) * 100.0));                                     // REG_TOUCH_GRAPH_SPEED_C2
            Inverter_InVolt = (unsigned int)(((float)EngineBox.InVolt_C1 * (float)vK_Supply_Inverter)/100.0);	// Perc. -> Volt (centesimi di V)
            Inverter_OutCurr = (unsigned int)( (float)EngineBox.OutPower_C1*10.0 / (float)K_VoltOutMaxInverter * 100.0);	// Perc. -> Ampere (centesimi di A)        
            InsertRegSec(20, Inverter_InVolt);                                        // REG_TOUCH_GRAPH_VOLT_IN_C1
            InsertRegSec(21, Inverter_OutCurr);                                        // REG_TOUCH_GRAPH_CURR_OUT_C1
            Inverter_InVolt = (unsigned int)(((float)EngineBox.InVolt_C2 * (float)vK_Supply_Inverter)/100.0);	// Perc. -> Volt (centesimi di V)
            Inverter_OutCurr = (unsigned int)( (float)EngineBox.OutPower_C2*10.0 / (float)K_VoltOutMaxInverter * 100.0);	// Perc. -> Ampere (centesimi di A)        
            InsertRegSec(22, Inverter_InVolt);                                        // REG_TOUCH_GRAPH_VOLT_IN_C2
            InsertRegSec(23, Inverter_OutCurr);                                        // REG_TOUCH_GRAPH_CURR_OUT_C2
            WriteModListRegSec (TouchAddres, REG_TOUCH_FLAGS_ENABLE_1_16, 23);
            
            InsertRegSec(1, Me.DoublePump_Abil);                                        // REG_TOUCH_ABIL_DOUBLE_PUMP
            InsertRegSec(2, Me.SplitMode_Abil);                                         // REG_TOUCH_SPLITMODE_ABIL
            WriteModListRegSec (TouchAddres, REG_TOUCH_ABIL_DOUBLE_PUMP, 2);
            
            InsertRegSec(1, EngineBox.AbilValveOnDemand);                                        // REG_TOUCH_ABIL_VALVE_ONDEMAND
            InsertRegSec(2, EngineBox.AbilScoreOnDemand);                                         // REG_TOUCH_ABIL_SCORE_ONDEMAND
            WriteModListRegSec (TouchAddres, REG_TOUCH_ABIL_VALVE_ONDEMAND, 2);

            InsertRegSec(1, EngineBox.Split.ModDev.FlagsEnable_1_16);                   // REG_TOUCH_MODDEV_FLAGS_ENABLE_1_16
            InsertRegSec(2, EngineBox.Split.ModDev.FlagsOffLine_1_16);                  // REG_TOUCH_MODDEV_FLAGS_OFFLINE_1_16
            InsertRegSec(3, EngineBox.Split.ModDev.FlagsOnLine_1_16);                   // REG_TOUCH_MODDEV_FLAGS_ONLINE_1_16
            InsertRegSec(4, EngineBox.Split.ModDev.FlagsAlarm_1_16);                    // REG_TOUCH_MODDEV_FLAGS_ALARM_1_16
            InsertRegSec(5, EngineBox.Split.ModDev.FlagsEnable_17_32);                  // REG_TOUCH_MODDEV_FLAGS_ENABLE_17_32
            InsertRegSec(6, EngineBox.Split.ModDev.FlagsOffLine_17_32);                 // REG_TOUCH_MODDEV_FLAGS_OFFLINE_17_32
            InsertRegSec(7, EngineBox.Split.ModDev.FlagsOnLine_17_32);                  // REG_TOUCH_MODDEV_FLAGS_ONLINE_17_32
            InsertRegSec(8, EngineBox.Split.ModDev.FlagsAlarm_17_32);                   // REG_TOUCH_MODDEV_FLAGS_ALARM_17_32        
            InsertRegSec(9, EngineBox.Split.ModDev.AddrType);                           // REG_TOUCH_MODDEV_AND_ADDR_TYPE               tipo di Device (Byte LO) e suo address (Byte HI)) 
            WriteModListRegSec (TouchAddres, REG_TOUCH_MODDEV_FLAGS_ENABLE_1_16, 9);            
            
            InsertRegSec(1, EngineBox.Total_AirPowerOutput);                   // REG_TOUCH_TOTAL_AIR_POWER_OTUPUT
            InsertRegSec(2, EngineBox.Total_AirPower_BTU);                     // REG_TOUCH_TOTAL_AIR_POWER_BTU
            InsertRegSec(3, EngineBox.System_Efficency_EER);                   // REG_TOUCH_SYSTEM_EFFICENCY_EER
            WriteModListRegSec (TouchAddres, REG_TOUCH_TOTAL_AIR_POWER_OTUPUT, 3);

            InsertRegSec(1, EngineBox.PowerBoxAbsorption);										// REG_TOUCH_BOX_ABSORPTION
            WriteModListRegSec (TouchAddres, REG_TOUCH_BOX_ABSORPTION, 1);		

            InsertRegSec(1, EngineBox.AddressGasLeaks);										// REG_TOUCH_ADDRESS_GAS_LEAKS
            WriteModListRegSec (TouchAddres, REG_TOUCH_ADDRESS_GAS_LEAKS, 1);		

			

        }
        
        InsertRegSec(1, DiagnosticSplit.Sub_Slave_Flags_Enable_1_16);                                      // REG_TOUCH_SUB_SL_FLAGS_ENABLE_1_16
		InsertRegSec(2, DiagnosticSplit.Sub_Slave_Flags_Offline_1_16);                                     // REG_TOUCH_SUB_SL_FLAGS_OFFLINE_1_16
		InsertRegSec(3, DiagnosticSplit.Sub_Slave_Flags_Online_1_16);                                      // REG_TOUCH_SUB_SL_FLAGS_ONLINE_1_16
		InsertRegSec(4, DiagnosticSplit.Sub_Slave_Flags_Alarm_1_16);                                       // REG_TOUCH_SUB_SL_FLAGS_ALARMS_1_16
		InsertRegSec(5, Me.OtherCabin.Number);                                                             // REG_TOUCH_NUMBER_OTHER_CABIN
		InsertRegSec(6, Me.OtherCabin.Temperature);                                                        // REG_TOUCH_OTHER_CABIN_ST_TEMP
		InsertRegSec(7, Me.OtherCabin.ST_On_Off);                                                          // REG_TOUCH_OTHER_CABIN_ST_ON_OFF
		InsertRegSec(8, Me.OtherCabin.ST_SetP);                                                            // REG_TOUCH_OTHER_CABIN_ST_SETP
		InsertRegSec(9, Me.OtherCabin.ST_Fan);                                                             // REG_TOUCH_OTHER_CABIN_ST_FAN
        
		WriteModListRegSec (TouchAddres, REG_TOUCH_SUB_SL_FLAGS_ENABLE_1_16, 9);
        
        WriteModRegSec (TouchAddres, REG_TOUCH_OTHER_CABIN_ONLY_FAN, Me.OtherCabin.Only_Fan);

        WriteModRegSec (TouchAddres, REG_TOUCH_OTHER_CABIN_ST_UTA_MODE, Me.OtherCabin.ST_UTAMode);
        
        EngineBox.Errori3 = 0;//(EngineBox.Errori3 & 0x00FF);
				
        if(ComunicationLost)
            EngineBox.Errori3 = (EngineBox.Errori3 | 0x0001);
        if(Me.Room.Split_Flags_Alarm_1_16>0)
            EngineBox.Errori3 = (EngineBox.Errori3 | 0x0002);
        if(Me.Error.TemperatureProbe)		// aggiungo gli errori dello split
            EngineBox.Errori3 = (EngineBox.Errori3 | 0x0200);
        if(Me.Error.Battery_Defrost)
            EngineBox.Errori3 = (EngineBox.Errori3 | 0x0400);
        if(Me.Error.EEV_Open)
            EngineBox.Errori3 = (EngineBox.Errori3 | 0x0800);
        if(Me.Error.EEV_Close)
            EngineBox.Errori3 = (EngineBox.Errori3 | 0x1000);
        if(Me.Error.BatteriaGhiacciata)
            EngineBox.Errori3 = (EngineBox.Errori3 | 0x2000);
        if(Me.Error.Fan_Fault)
            EngineBox.Errori3 = (EngineBox.Errori3 | 0x4000);
        if(Me.Error.Fan_OverLoad)
            EngineBox.Errori3 = (EngineBox.Errori3 | 0x8000);

        InsertRegSec(1, EngineBox.Errori1);					// REG_TOUCH_SYS_ERROR_1
        InsertRegSec(2, EngineBox.ErroriPers1);				// REG_TOUCH_SYS_PERSERR_1
        InsertRegSec(3, EngineBox.Errori3);					// REG_TOUCH_SYS_ERROR_3
        InsertRegSec(4, EngineBox.ErroriPers2);				// REG_TOUCH_SYS_ERROR_3

        WriteModListRegSec (TouchAddres, REG_TOUCH_SYS_ERROR_1, 4);
        
        WriteModRegSec (TouchAddres, REG_TOUCH_SYS_ERROR_2, EngineBox.Errori2);                

        InsertRegSec(1, EngineBox.Inverter1CodeFault);			// REG_TOUCH_INVERTER1_CODE_ALARM
        InsertRegSec(2, EngineBox.Inverter2CodeFault);			// REG_TOUCH_INVERTER2_CODE_ALARM
        InsertRegSec(3, EngineBox.Inverter3CodeFault);			// REG_TOUCH_INVERTER3_CODE_ALARM
        InsertRegSec(4, EngineBox.Inverter4CodeFault);			// REG_TOUCH_INVERTER4_CODE_ALARM

        WriteModListRegSec (TouchAddres, REG_TOUCH_INVERTER1_CODE_ALARM, 4);

        WriteModRegSec (TouchAddres, REG_TOUCH_FROZEN_ABIL, EngineBox.Frozen_Abil);                

        WriteModRegSec (TouchAddres, REG_TOUCH_MAINTENANCE_REMANING_TIME, Touch[0].Maintenance_Remaning_Time);                                

        WriteModRegSec (TouchAddres, REG_TOUCH_DEHUMI_ABIL, EngineBox.Dehumi_Abil);                

        if(EngineBox.Maintenance_ReadMasReset)
        {
            Touch[0].Maintenance_Flag &= ~0x0002;
            EngineBox.Maintenance_ReadMasReset = 0;
            WriteModRegSec (TouchAddres, REG_TOUCH_MAINTENANCE_FLAG, Touch[0].Maintenance_Flag);
        }
        //WriteModRegSec (TouchAddres, REG_TOUCH_SPLIT_FROZEN_MODE, Me.Frozen.Mode);                
						
		ReadModRegSec (TouchAddres, REG_TOUCH_PAGE, &(Touch[DataIndexSec].Current_Page));
		switch((char)Touch[DataIndexSec].Current_Page)
		{
			case 0:		// pagina 0 -> Wait system initializing
				break;

			case 2:		// pagina 2 -> Home
				InsertRegSec(1, Me.TempAmbTouchVisual);						// REG_TOUCH_ROOM_TEMP
				//InsertRegSec(3, EngineBox.ErroriPers1);				// REG_TOUCH_CUMULATIVE_ALARM
				if(EngineBox.DoubleCompressorOn!=0)
				{
                    if(My_Address==k_Split_Master_Add)
                        temp = ((EngineBox.CumulativeError | EngineBox.CumulativeError_C2/* | (Me.CumulativeAlarm << 1)*/) * !ComunicationLost) + (2 * ComunicationLost);
                    else
                        temp = 0;
                    
                    if(EngineBox.DoubleCompressorOn==3)
    					InsertRegSec(2, EngineBox.Compressor_Speed);		// REG_TOUCH_POWER_MOT
                    else
    					InsertRegSec(2, (EngineBox.Compressor_Speed + EngineBox.Compressor_Speed_C2)/2);		// REG_TOUCH_POWER_MOT
					InsertRegSec(3, temp);		// REG_TOUCH_CUMULATIVE_ALARM
				}
				else
				{
                    if(My_Address==k_Split_Master_Add)
    					temp = (EngineBox.CumulativeError * !ComunicationLost) + (2 * ComunicationLost);
	                else
                        temp = 0;
					InsertRegSec(2, EngineBox.Compressor_Speed);		// REG_TOUCH_POWER_MOT
					InsertRegSec(3, temp);								// REG_TOUCH_CUMULATIVE_ALARM
				}
				WriteModListRegSec (TouchAddres, REG_TOUCH_ROOM_TEMP, 3);						
				
				if(Me.My_Address!=k_Split_Master_Add)
				{
					InsertRegSec(1, EngineBox.SystemDisable);										// REG_TOUCH_CUMULATIVE_ALARM
					WriteModListRegSec (TouchAddres, REG_TOUCH_SPLIT_SYSTEM_DISABLE, 1);						
				}
				
				WriteModRegSec (TouchAddres, REG_TOUCH_MASTER_RESTART_TIME, EngineBox.MasterRestartTime);
				
				WriteModRegSec (TouchAddres, REG_TOUCH_SYS_ERROR_1, EngineBox.Errori1);
				break;

			case 3:		// pagina 3 -> Power Manager
				// Calcolo potenza limitata in centesimi di Kw partendo da % slider (Touch[0].PowerLimit) ceh è in scala 0...255
				// solo per visualizzazione su Touch
				//							0..255					Watt
				Inverter_Max_Power_Reduce = (unsigned int)((float)Touch[0].PowerLimit/2.55 * (float)DiagnosticInverter.PowerMax_Available/1000.0);
				WriteModRegSec (TouchAddres, REG_TOUCH_MAX_POWER_REDUCE, Inverter_Max_Power_Reduce);		

				InsertRegSec(1, EngineBox.PowerBoxAbsorption);										// REG_TOUCH_BOX_ABSORPTION
				WriteModListRegSec (TouchAddres, REG_TOUCH_BOX_ABSORPTION, 1);					
				break;			

			case 4:			// pagina 4 -> Service Login	// Ex. Pag.4
				break;
			case 5:			// pagina 5 -> Faults1
			case 6:			// pagina 6 -> Faults2
			case 7:			// pagina 7 -> Faults3				
			case 8:			// pagina 8 -> Faults4			// Ex. Pag. 8
				/*
				EngineBox.Errori3 = 0;//(EngineBox.Errori3 & 0x00FF);
				
				if(ComunicationLost)
					EngineBox.Errori3 = (EngineBox.Errori3 | 0x0001);
				if(Me.Error.Battery_Defrost)		// aggiungo gli errori dello split
					EngineBox.Errori3 = (EngineBox.Errori3 | 0x0400);
				if(Me.Error.EEV_Open)
					EngineBox.Errori3 = (EngineBox.Errori3 | 0x0800);
				if(Me.Error.EEV_Close)
					EngineBox.Errori3 = (EngineBox.Errori3 | 0x1000);
				if(Me.Error.BatteriaGhiacciata)
					EngineBox.Errori3 = (EngineBox.Errori3 | 0x2000);
				if(Me.Error.Fan_Fault)
					EngineBox.Errori3 = (EngineBox.Errori3 | 0x4000);
				if(Me.Error.Fan_OverLoad)
					EngineBox.Errori3 = (EngineBox.Errori3 | 0x8000);
				

				InsertRegSec(1, EngineBox.Errori1);					// REG_TOUCH_SYS_ERROR_1
				InsertRegSec(2, EngineBox.ErroriPers1);				// REG_TOUCH_SYS_PERSERR_1
				InsertRegSec(3, EngineBox.Errori3);					// REG_TOUCH_SYS_ERROR_3
				InsertRegSec(4, EngineBox.ErroriPers2);				// REG_TOUCH_SYS_ERROR_3
				
				WriteModListRegSec (TouchAddres, REG_TOUCH_SYS_ERROR_1, 4);
				*/
				WriteModRegSec (TouchAddres, REG_TOUCH_SPLIT_STATUS, DiagnosticSplit.SplitStatus);
                WriteModRegSec (TouchAddres, REG_TOUCH_UTA_ABIL, DiagnosticSplit.UTA_Abil);
                WriteModRegSec (TouchAddres, REG_TOUCH_DIAG_ONLY_FAN, DiagnosticSplit.Only_Fan);
                WriteModRegSec (TouchAddres, REG_TOUCH_DIAG_FROZEN_MODE, DiagnosticSplit.FrozenMode);

				InsertRegSec(1, DiagnosticSplit.Sub_Slave_Quantity);				// REG_TOUCH_SUB_SPLIT_QUANTITY				
				WriteModListRegSec (TouchAddres, REG_TOUCH_SUB_SPLIT_QUANTITY, 1);				

				InsertRegSec(1, DiagnosticSplit.Model_FW_Version);				// REG_TOUCH_DIAG_SPLIT_MODEL_FW_VERSION				
				InsertRegSec(2, DiagnosticSplit.Fan_M3h);                       // REG_TOUCH_DIAG_SPLIT_FAN_M3H				
				WriteModListRegSec (TouchAddres, REG_TOUCH_DIAG_SPLIT_MODEL_FW_VERSION, 2);		
                
				InsertRegSec(1, DiagnosticSplit.AirPowerOutput);				// REG_TOUCH_DIAG_SPLIT_AIR_POWER_OUTPUT
				InsertRegSec(2, DiagnosticSplit.AirPowerBTU);                   // REG_TOUCH_DIAG_SPLIT_AIR_POWER_BTU				
				WriteModListRegSec (TouchAddres, REG_TOUCH_DIAG_SPLIT_AIR_POWER_OUTPUT, 2);		
				break;

			case 9:			// pagina 9 -> Diagnostica Box motore	// Ex. Pag.9

				InsertRegSec(1, EngineBox.HoursOfLife);				// REG_TOUCH_HOURS_OF_LIFE v8.4.16 -> invio ore di vita unita master x diagnostica
				if(EngineBox.DiagAddress==1)
					InsertRegSec(2, EngineBox.Temp_Mandata);				// REG_TOUCH_MANDATA_COMPRESSORE
				else if(EngineBox.DiagAddress==2)
					InsertRegSec(2, EngineBox.Temp_Compressor_Output_C2);				// REG_TOUCH_MANDATA_COMPRESSORE
				InsertRegSec(3, EngineBox.Temp_Ritorno);				// REG_TOUCH_RITORNO_COMPRESSORE
				InsertRegSec(4, EngineBox.Temp_Condensatore);			// REG_TOUCH_TEMP_CONDENSATORE
				InsertRegSec(5, EngineBox.Temp_Acqua_Mare);			// REG_TOUCH_SEA_WATER_IN_TEMP
				InsertRegSec(6, EngineBox.Pressione_Liq_P);			// REG_TOUCH_PRESSIONE_P
				InsertRegSec(7, EngineBox.Pressione_Gas_G);			// REG_TOUCH_PRESSIONE_G
				if(EngineBox.DiagAddress==1)
					InsertRegSec(8, EngineBox.Compressor_Speed);			// REG_TOUCH_COMPRESSOR_SPEED
				else if(EngineBox.DiagAddress==2)
					InsertRegSec(8, EngineBox.Compressor_Speed_C2);			// REG_TOUCH_COMPRESSOR_SPEED
				//InsertRegSec(9, EngineBox.Pressure_Req);			// REG_TOUCH_TEMP_RICHIESTA
				InsertRegSec(10, EngineBox.Ric_Pressione);				// REG_TOUCH_PRESSIONE_RICHIESTA
				if(EngineBox.DiagAddress==1)
				{
					InsertRegSec(11, EngineBox.StatoCompressore);			// REG_TOUCH_STATO_COMPRESSORE
					InsertRegSec(12, EngineBox.FunctionMode);				// REG_TOUCH_ENGINE_BOX_MODE
				}
				else if(EngineBox.DiagAddress==2)
				{
					InsertRegSec(11, EngineBox.StatoCompressore_C2);			// REG_TOUCH_STATO_COMPRESSORE
					InsertRegSec(12, EngineBox.FunctionMode_C2);				// REG_TOUCH_ENGINE_BOX_MODE
				}
				InsertRegSec(13, EngineBox.IdMasterSplit);			// REG_TOUCH_MASTER_SPLIT
				//Saltare locazione 14 per clear error in posizione // REG_TOUCH_CLEAR_ERROR				0x005C
				InsertRegSec(15, EngineBox.Temp_Collettore_Piccolo);	// REG_TOUCH_TEMP_P
				InsertRegSec(16, EngineBox.Temp_Collettore_Grande);	// REG_TOUCH_TEMP_G
				InsertRegSec(17, EngineBox.SystemCycleTime);			// REG_TOUCH_SYSTEM_CYCLE_TIME v8.4.16 -> invio tempo di ciclo unita master x diagnostica
				WriteModListRegSec (TouchAddres, REG_TOUCH_HOURS_OF_LIFE, 17);

                InsertRegSec(1, EngineBox.HW_Version);                 // REG_TOUCH_MASTER_HW_VERSION
                InsertRegSec(2, EngineBox.FW_Version_HW);              // REG_TOUCH_MASTER_FW_VERSION_HW                
                InsertRegSec(3, EngineBox.FW_Version_FW);              // REG_TOUCH_MASTER_FW_VERSION_FW
                InsertRegSec(4, EngineBox.FW_Revision_FW);             // REG_TOUCH_MASTER_FW_REVISION_FW
                WriteModListRegSec (TouchAddres, REG_TOUCH_MASTER_HW_VERSION, 4);    


                InsertRegSec(1, EngineBox.Pump_Speed);             // REG_TOUCH_POWER_PUMP_DIAG
                WriteModListRegSec (TouchAddres, REG_TOUCH_POWER_PUMP_DIAG, 1);    
				
                InsertRegSec(1, EngineBox.Max_Comp_Quantity);             // REG_TOUCH_POWER_PUMP_DIAG
                WriteModListRegSec (TouchAddres, REG_TOUCH_MAX_DIAG_BOX_QUANTITY, 1);    

                InsertRegSec(1, EngineBox.Pump_Speed_Perc);             // REG_TOUCH_POWER_PUMP_PERC
                WriteModListRegSec (TouchAddres, REG_TOUCH_POWER_PUMP_PERC, 1);    
				
				WriteModRegSec (TouchAddres, REG_TOUCH_MASTER_RESTART_TIME, EngineBox.MasterRestartTime);			

				WriteModRegSec (TouchAddres, REG_TOUCH_ENGBOX_PRESS_LIQUID_COND, EngineBox.Pressure_LiquidCond);			

                if(Me.DefMode==CoreRiscaldamento)
				{
                    InsertRegSec(1, DiagnosticSplit.SuperheatRiscaldamento);                         // REG_TOUCH_SUPERHEAT
                    InsertRegSec(2, DiagnosticSplit.SetP_Superheat); //Da vederecome sarà
                }				
                else if(Me.DefMode==CoreRaffrescamento)
                {
					InsertRegSec(1, EngineBox.SuperHeat_CoolingCalc);
                    InsertRegSec(2, EngineBox.K_SetPoint_Cooling_SuperHeat);
                }
                else
                {
                    InsertRegSec(1, 0);
                    InsertRegSec(2, 0);
                }
                WriteModListRegSec (TouchAddres, REG_TOUCH_SUPERHEAT, 2);
				//InsertRegSec(1, EngineBox.);             // REG_TOUCH_MASTER_FW_REVISION_FW
                //WriteModListRegSec (TouchAddres, REG_TOUCH_POWER_PUMP_DIAG, 4);                
                
                WriteModRegSec (TouchAddres, REG_TOUCH_BOX_TEMPERATURE_PROBE_ERROR, EngineBox.Temperature_Probe_Error);			
                
/*
				InsertRegSec(1, EngineBox.Temp_Mandata);				// REG_TOUCH_MANDATA_COMPRESSORE
				InsertRegSec(2, EngineBox.Temp_Ritorno);				// REG_TOUCH_RITORNO_COMPRESSORE
				InsertRegSec(3, EngineBox.Temp_Condensatore);			// REG_TOUCH_TEMP_CONDENSATORE
				InsertRegSec(4, EngineBox.Temp_Acqua_Mare);			// REG_TOUCH_SEA_WATER_IN_TEMP
				InsertRegSec(5, EngineBox.Pressione_Liq_P);			// REG_TOUCH_PRESSIONE_P
				InsertRegSec(6, EngineBox.Pressione_Gas_G);			// REG_TOUCH_PRESSIONE_G
				InsertRegSec(7, EngineBox.Compressor_Speed);			// REG_TOUCH_COMPRESSOR_SPEED
				WriteModListRegSec (TouchAddres, REG_TOUCH_MANDATA_COMPRESSORE, 7);
				//InsertRegSec(8, EngineBox.Pressure_Req);			// REG_TOUCH_TEMP_RICHIESTA
			
				InsertRegSec(1, EngineBox.Ric_Pressione);				// REG_TOUCH_PRESSIONE_RICHIESTA
				InsertRegSec(2, EngineBox.StatoCompressore);			// REG_TOUCH_STATO_COMPRESSORE
				InsertRegSec(3, EngineBox.FunctionMode);				// REG_TOUCH_ENGINE_BOX_MODE
				InsertRegSec(4, EngineBox.IdMasterSplit);				// REG_TOUCH_MASTER_SPLIT
				//Saltare locazione 13 per clear error in posizione // REG_TOUCH_CLEAR_ERROR				0x005C
				InsertRegSec(6, EngineBox.Temp_Collettore_Piccolo);	// REG_TOUCH_TEMP_P
				InsertRegSec(7, EngineBox.Temp_Collettore_Grande);	// REG_TOUCH_TEMP_G
				//InsertRegSec(8, EngineBox.SystemCycleTime);			// REG_TOUCH_SYSTEM_CYCLE_TIME v8.4.16 -> invio tempo di ciclo unita master x diagnistica
				InsertRegSec(8, temp++);
				WriteModListRegSec (TouchAddres, REG_TOUCH_PRESSIONE_RICHIESTA, 8);
*/			
				break;

			case 10:		// pagina 10 -> Diagnostica Room 1	// Ex. Pag.10
				// Calcolo percentuale apertura valvola 0..100%
				temp = K_MaxExcursionValve; //ValvolaAperta;
				Split_Exp_Valve_Perc = (unsigned int)( ((float)Me.ExpValve_Act / (float)temp) * 100.0); 
				if(Split_Exp_Valve_Perc>100)
					Split_Exp_Valve_Perc=100;
				
				InsertRegSec(1, Me.Temperature.Liquid);				// REG_TOUCH_PICCOLO_SPLIT_1
				InsertRegSec(2, Me.Temperature.Gas);					// REG_TOUCH_GRANDE_SPLIT_1
				InsertRegSec(3, Me.Temperature.Ambient);					// REG_TOUCH_ARIA_INGRESSO_SPLIT_1
				InsertRegSec(4, Me.Temperature.AirOut);				// REG_TOUCH_ARIA_USCITA_SPLIT_1
				InsertRegSec(5, Me.Fan_Tacho);					// REG_TOUCH_VENTILATORE_SPLIT_1
				//InsertRegSec(6, Me.ExpValve_Act);				// REG_TOUCH_STATO_VALVOLA_SPLIT_1
				InsertRegSec(6, Split_Exp_Valve_Perc);				// REG_TOUCH_STATO_VALVOLA_SPLIT_1
				
				//InsertRegSec(7, Me.Pres_Big);				// REG_TOUCH_PRESSIONE_G_SPLIT_1
				//InsertRegSec(8, Me.Pres_Small);				// REG_TOUCH_PRESSIONE_P_SPLIT_1
				InsertRegSec(7, EngineBox.Pressione_Gas_G);	// REG_TOUCH_PRESSIONE_G_SPLIT_1	//Metto pressioni box motore perchè non presenti sonde di pressione salvo casi eccezionali da valutare
				InsertRegSec(8, EngineBox.Pressione_Liq_P);	// REG_TOUCH_PRESSIONE_P_SPLIT_1	//Metto pressioni box motore perchè non presenti sonde di pressione salvo casi eccezionali da valutare
				InsertRegSec(9, Me.EvapTempValve);					// REG_TOUCH_TEMP_EVAP_SPLIT_1	// sottraggo PerformanceLossBattery per visualizzazione correta dato richiesto su touch
				InsertRegSec(10, Me.Evap_Press);				// REG_TOUCH_RICH_PRESSIONE_SPLIT_1
				InsertRegSec(11, EngineBox.RealFuncMode);				// REG_TOUCH_ENGINE_BOX_MODE_SPLIT_1
				InsertRegSec(12, Me.CycleTime);				// REG_TOUCH_CYCLE_TIME_SPLIT_1
				WriteModListRegSec (TouchAddres, REG_TOUCH_PICCOLO_SPLIT_1, 12);
                /*
				//if(EngineBox.FunctionMode==EngineBox_Caldo || EngineBox.FunctionMode == EngineBox_GoCaldo || EngineBox.FunctionMode_C2==EngineBox_Caldo || EngineBox.FunctionMode_C2==EngineBox_GoCaldo)
                if(Me.DefMode==CoreRiscaldamento)
				{	
                    InsertRegSec(1, Me.SuperheatRiscaldamento);                         // REG_TOUCH_SUPERHEAT
                    //InsertRegSec(2, K_SetPoint_SuperHeatRiscaldamento); //da verificare come sarà!!                    
                    InsertRegSec(2, Me.SuperheatRiscaldamentoSP);
                }
				else
                {
					InsertRegSec(1, Me.Superheat);
                    InsertRegSec(2, ValvePID.SetP); //DEBUG
                }
                 * */
				//InsertRegSec(2, Me.Pwm_Req); //DEBUG
				//InsertRegSec(2, EngineBox.Temp_Ritorno - PressureGasToTempR410A(EngineBox.Pressione_Liq_P));//DEBUG
                //InsertRegSec(3, (int)(ValvePID.VKdPID*100));//DEBUG
                //InsertRegSec(3, Me.Fan_MaxSpeed);//DEBUG			
                InsertRegSec(4, Me.CntValveFault);  				
                WriteModListRegSec (TouchAddres, REG_TOUCH_SUPERHEAT, 4); 

                InsertRegSec(1, Me.HW_Version);                                        // REG_TOUCH_SPLIT_HW_VERSION
                InsertRegSec(2, Me.FW_Version_HW);                                     // REG_TOUCH_SPLIT_FW_VERSION_HW                
                InsertRegSec(3, Me.FW_Version_FW);                                     // REG_TOUCH_SPLIT_FW_VERSION_FW
                InsertRegSec(4, Me.FW_Revision_FW);                                    // REG_TOUCH_SPLIT_FW_REVISION_FW
                WriteModListRegSec (TouchAddres, REG_TOUCH_SPLIT_HW_VERSION, 4);             
				
				break;
                

			case 11:		// pagina 11 -> Diagnostic  Split	//Ex.Pag.11 		#v16
            case 15:
			//case 26:
				// Calcolo percentuale apertura valvola 0..100%
				temp = K_MaxExcursionValve; //ValvolaAperta;
				//Split_Exp_Valve_Perc = (unsigned int)((float)DiagnosticSplit.Exp_Valve / (float)temp * 100.0); 
                Split_Exp_Valve_Perc = (unsigned int)( round((float)DiagnosticSplit.Exp_Valve / (float)temp * 100.0)) ; 
				if(Split_Exp_Valve_Perc>100)
					Split_Exp_Valve_Perc=100;
				
				InsertRegSec(1, DiagnosticSplit.Temp_Liq);					// REG_TOUCH_SPLIT_LIQ_TEMP
				InsertRegSec(2, DiagnosticSplit.Temp_Gas);					// REG_TOUCH_SPLIT_GAS_TEMP
				InsertRegSec(3, DiagnosticSplit.Temp_AirIn);				// REG_TOUCH_SPLIT_AIR_IN
				InsertRegSec(4, DiagnosticSplit.Temp_AirOut);				// REG_TOUCH_SPLIT_AIR_OUT
				InsertRegSec(5, DiagnosticSplit.FanSpeed);					// REG_TOUCH_SPLIT_FAN_SPEED
				//InsertRegSec(6, DiagnosticSplit.Exp_Valve);				// REG_TOUCH_SPLIT_EXP_VALVE
				InsertRegSec(6, Split_Exp_Valve_Perc);				// REG_TOUCH_SPLIT_EXP_VALVE
				//InsertRegSec(7, DiagnosticSplit.Gas_Pressure);				// REG_TOUCH_SPLIT_GAS_PRESSURE
				//InsertRegSec(8, DiagnosticSplit.Liq_Pressure);				// REG_TOUCH_SPLIT_LIQ_PRESSURE
				InsertRegSec(9, DiagnosticSplit.EvapTempValve);			// REG_TOUCH_SPLIT_REQUEST_AIR_TEMP	
				InsertRegSec(10, DiagnosticSplit.Req_Pressure);			// REG_TOUCH_SPLIT_REQUEST_AIR_PRES
				InsertRegSec(11, EngineBox.RealFuncMode);							//DiagnosticSplit.Split_Eng_Mode);			// REG_TOUCH_SPLIT_ENG_MODE
				InsertRegSec(12, DiagnosticSplit.SplitStatus);				// REG_TOUCH_SPLIT_STATUS
				InsertRegSec(13, DiagnosticSplit.CycleTime);				// REG_TOUCH_SPLIT_TEST_CYCLE_TIME				
				WriteModListRegSec (TouchAddres, REG_TOUCH_SPLIT_LIQ_TEMP, 13);

				Split_Exp_Valve_Perc_Neg = (int)((float)DiagnosticSplit.Exp_Valve_Err / (float)temp * 100.0); 
                
				if(Split_Exp_Valve_Perc_Neg<-100)
					Split_Exp_Valve_Perc_Neg=-100;                
				if(Split_Exp_Valve_Perc_Neg>100)
					Split_Exp_Valve_Perc_Neg=100;                
				InsertRegSec(1, Split_Exp_Valve_Perc_Neg);	//REG_TOUCH_DIAG_SPLIT_EXP_VALVE_ERR
				InsertRegSec(2, DiagnosticSplit.Exp_Valve_Err_Code);	//REG_TOUCH_DIAG_SPLIT_EXP_VALVE_CODE
				WriteModListRegSec (TouchAddres, REG_TOUCH_DIAG_SPLIT_EXP_VALVE_ERR, 2);

				InsertRegSec(1, EngineBox.Pressione_Gas_G);				// REG_TOUCH_SPLIT_GAS_PRESSURE	//Metto pressioni box motore perchè non presenti sonde di pressione salvo casi eccezionali da valutare
				InsertRegSec(2, EngineBox.Pressione_Liq_P);				// REG_TOUCH_SPLIT_LIQ_PRESSURE	//Metto pressioni box motore perchè non presenti sonde di pressione salvo casi eccezionali da valutare
				WriteModListRegSec (TouchAddres, REG_TOUCH_PRESSIONE_G_SPLIT_1, 2);
                
                WriteModRegSec (TouchAddres, REG_TOUCH_TEMP_P, EngineBox.Temp_Collettore_Piccolo);		//	REG_TOUCH_TEMP_P				Temp_Tubo_Piccolo_Liquido	93	

				
				InsertRegSec(1, DiagnosticSplit.Sub_Slave_Quantity);				// REG_TOUCH_SUB_SPLIT_QUANTITY				
				WriteModListRegSec (TouchAddres, REG_TOUCH_SUB_SPLIT_QUANTITY, 1);	
                
                InsertRegSec(1, DiagnosticSplit.HW_Version);               // REG_TOUCH_DIAG_SPLIT_HW_VERSION
                InsertRegSec(2, DiagnosticSplit.FW_Version_HW);            // REG_TOUCH_DIAG_SPLIT_FW_VERSION_HW                
                InsertRegSec(3, DiagnosticSplit.FW_Version_FW);            // REG_TOUCH_DIAG_SPLIT_FW_VERSION_FW
                InsertRegSec(4, DiagnosticSplit.FW_Revision_FW);           // REG_TOUCH_DIAG_SPLIT_FW_REVISION_FW
                WriteModListRegSec (TouchAddres, REG_TOUCH_DIAG_SPLIT_HW_VERSION, 4);  

				InsertRegSec(1, DiagnosticSplit.Model_FW_Version);				// REG_TOUCH_DIAG_SPLIT_MODEL_FW_VERSION				
				InsertRegSec(2, DiagnosticSplit.Fan_M3h);                       // REG_TOUCH_DIAG_SPLIT_FAN_M3H				
				WriteModListRegSec (TouchAddres, REG_TOUCH_DIAG_SPLIT_MODEL_FW_VERSION, 2);				

				InsertRegSec(1, DiagnosticSplit.AirPowerOutput);				// REG_TOUCH_DIAG_SPLIT_AIR_POWER_OUTPUT
				InsertRegSec(2, DiagnosticSplit.AirPowerBTU);                   // REG_TOUCH_DIAG_SPLIT_AIR_POWER_BTU				
				WriteModListRegSec (TouchAddres, REG_TOUCH_DIAG_SPLIT_AIR_POWER_OUTPUT, 2);		

				//if(EngineBox.FunctionMode==EngineBox_Caldo || EngineBox.FunctionMode == EngineBox_GoCaldo || EngineBox.FunctionMode_C2==EngineBox_Caldo || EngineBox.FunctionMode_C2==EngineBox_GoCaldo)
                if(Me.DefMode==CoreRiscaldamento)
				{
                    InsertRegSec(1, DiagnosticSplit.SuperheatRiscaldamento);                         // REG_TOUCH_SUPERHEAT
                    InsertRegSec(2, DiagnosticSplit.SetP_Superheat); //Da vederecome sarà
                }				
                else
                {
					InsertRegSec(1, DiagnosticSplit.Superheat);                //InsertRegSec(2, (int)(ValvePID.VKiPID*1000)); //DEBUG
                    InsertRegSec(2, DiagnosticSplit.SetP_Superheat); //DEBUG
                }
                //InsertRegSec(2, EngineBox.Temp_Ritorno - PressureGasToTempR410A(EngineBox.Pressione_Liq_P));//DEBUG
                //InsertRegSec(3, (int)(ValvePID.VKdPID*100));//DEBUG
                //InsertRegSec(3, Me.Fan_MaxSpeed);//DEBUG			
                InsertRegSec(4, Me.CntValveFault);  				
                WriteModListRegSec (TouchAddres, REG_TOUCH_SUPERHEAT, 4); 	
                
                WriteModRegSec (TouchAddres, REG_TOUCH_UTA_ABIL, DiagnosticSplit.UTA_Abil);
                WriteModRegSec (TouchAddres, REG_TOUCH_DIAG_ONLY_FAN, DiagnosticSplit.Only_Fan);
                WriteModRegSec (TouchAddres, REG_TOUCH_SET_CMD_SPLIT_DISABLE, DiagnosticSplit.Set_CMD_Disable); 
                WriteModRegSec (TouchAddres, REG_TOUCH_DIAG_FUNCTIONMODE, DiagnosticSplit.FunctionMode); 
                WriteModRegSec (TouchAddres, REG_TOUCH_DIAG_FROZEN_MODE, DiagnosticSplit.FrozenMode);
                
                
				WriteModRegSec (TouchAddres, REG_TOUCH_ENGBOX_PRESS_LIQUID_COND, EngineBox.Pressure_LiquidCond);	
                                
                InsertRegSec(1, DiagnosticSplit.Fan_Power);                     // REG_TOUCH_DIAG_SPLIT_FAN_POWER
                InsertRegSec(2, DiagnosticSplit.Fan_Current);                   // REG_TOUCH_DIAG_SPLIT_FAN_CURRENT                
                InsertRegSec(3, DiagnosticSplit.Supply_Voltage);                   // REG_TOUCH_DIAG_SPLIT_SUPPLY_VOLTAGE
                WriteModListRegSec (TouchAddres, REG_TOUCH_DIAG_SPLIT_FAN_POWER, 3);                  

                if(Me.DefMode==CoreRiscaldamento)
                    WriteModRegSec (TouchAddres, REG_TOUCH_ENGBOX_SUPERH_PID_ERROR, EngineBox.SuperH_Pid_Error);
                else
                    WriteModRegSec (TouchAddres, REG_TOUCH_ENGBOX_SUPERH_PID_ERROR, DiagnosticSplit.Superheat_Cool_PID_Error);

                WriteModRegSec (TouchAddres, REG_TOUCH_DIAG_SPLIT_TEMPERATURE_PROBE_ERROR, DiagnosticSplit.Temperature_Probe_Error);			

                WriteModRegSec (Add_TouchRoom, REG_TOUCH_DIAG_HUMIDITY, DiagnosticSplit.Humidity);		//
				break;	

/*			Spostato insieme al case 16 (Nuova pagina "Power Monitor")
			case 12:		// pagina 15 -> Diagnostic Inverter data			//Ex.Pag.12
				InsertRegSec(1, DiagnosticInverter.Alarm);					// REG_TOUCH_INVERTER_ALARM
				InsertRegSec(2, DiagnosticInverter.OutFreq);				// REG_TOUCH_INVERTER_OUT_FREQ	
				InsertRegSec(3, DiagnosticInverter.Status);				// REG_TOUCH_INVERTER_STATUS
				InsertRegSec(4, DiagnosticInverter.OutCurr);				// REG_TOUCH_INVERTER_OUT_CURR
				InsertRegSec(5, DiagnosticInverter.InVolt);				// REG_TOUCH_INVERTER_IN_VOLT
				InsertRegSec(6, DiagnosticInverter.OutVolt);				// REG_TOUCH_INVERTER_OUT_VOLT
				InsertRegSec(7, DiagnosticInverter.InPower);				// REG_TOUCH_INVERTER_IN_POWER
				InsertRegSec(8, DiagnosticInverter.OutPower);				// REG_TOUCH_INVERTER_OUT_POWER
				InsertRegSec(9, DiagnosticInverter.ReqFreq);				// REG_TOUCH_INVERTER_REQ_FREQ
				InsertRegSec(10, DiagnosticInverter.MaxReqFreq);			// REG_TOUCH_INVERTER_MAX_REQ_FREQ
				WriteModListRegSec (TouchAddres, REG_TOUCH_INVERTER_ALARM, 10);
				break;	
*/				
			case 13:		// pagina 13-> VNC
				break;				
				
			case 14:		// pagina 14-> EngineBox Detail					
            case 46:        // Service Pump
				WriteModRegSec (TouchAddres, REG_TOUCH_PWR, Touch[0].Script_Split_Pwr);		//	Pwr	2	
						
				InsertRegSec(1, EngineBox.Temp_Mandata);					//	REG_TOUCH_MANDATA_COMPRESSORE	Mandata_compressore			80
				InsertRegSec(2, EngineBox.Temp_Ritorno);					//	REG_TOUCH_RITORNO_COMPRESSORE	Ritorno_compressore			81
				InsertRegSec(3, EngineBox.Temp_Condensatore);				//	REG_TOUCH_TEMP_CONDENSATORE		Temp_condensatore			82
				InsertRegSec(4, EngineBox.Temp_Acqua_Mare);				//	REG_TOUCH_SEA_WATER_IN_TEMP		Sea_Water_Temp				83	
				InsertRegSec(5, EngineBox.Pressione_Liq_P);				//	REG_TOUCH_PRESSIONE_P			Pressione_P					84
				InsertRegSec(6, EngineBox.Pressione_Gas_G);				//	REG_TOUCH_PRESSIONE_G			Pressione_G					85						
				if(EngineBox.DiagAddress==1)
					InsertRegSec(7, EngineBox.Compressor_Speed);			// REG_TOUCH_COMPRESSOR_SPEED
				else if(EngineBox.DiagAddress==2)
					InsertRegSec(7, EngineBox.Compressor_Speed_C2);			// REG_TOUCH_COMPRESSOR_SPEED
				WriteModListRegSec (TouchAddres, REG_TOUCH_MANDATA_COMPRESSORE, 7);		
						
						
				InsertRegSec(1, EngineBox.Temp_Collettore_Piccolo);		//	REG_TOUCH_TEMP_P				Temp_Tubo_Piccolo_Liquido	93
				InsertRegSec(2, EngineBox.Temp_Collettore_Grande);					//	REG_TOUCH_TEMP_G				Temp_Tubo_Grande_Gas		94
				WriteModListRegSec (TouchAddres, REG_TOUCH_TEMP_P, 2);		

				// Estrapolazione stato rele pompa e stato Test di flusso da "Errori1"
                if((Me.DoublePump_Abil & 0x0001)!=0)
                {
                    temp = (1*((((EngineBox.ReleSt & 0x0001)!=0) || ((EngineBox.ReleSt & 0x0004)!=0)) && !((EngineBox.Errori1 & 0x0020)!=0))) | 
                            (3*((EngineBox.Errori1 & 0x0020)!=0));		// bit.5 -> BoxMotore[0].Test.FluxReq (arriva da Master)
                }
                else
                {
                    temp = (1*(((EngineBox.ReleSt & 0x0004)!=0) && !((EngineBox.Errori1 & 0x0020)!=0))) | 
                            (3*((EngineBox.Errori1 & 0x0020)!=0));		// bit.5 -> BoxMotore[0].Test.FluxReq (arriva da Master)
                }
				InsertRegSec(1, temp);					//	REG_TOUCH_SEA_WATER_PUMP	
				// Estrapolazione stato Flussostato pompa e stato Guasto flussostato da "ErroriPers1"
				temp = (1*(((EngineBox.InputSt & 0x0001)!=0) && !((EngineBox.ErroriPers1 & 0x0400)!=0))) |
						(2*((EngineBox.ErroriPers1 & 0x0400)!=0));		// bit.10 -> PersErr.WaterSeaFlux (arriva da Master)
				InsertRegSec(2, temp);					//	REG_TOUCH_FLOW_STATE
				WriteModListRegSec (TouchAddres, REG_TOUCH_SEA_WATER_PUMP, 2);				
				
			break;
/*
			case 15:		// pagina 15-> Engine Detail
				temp = K_MaxExcursionValve; //ValvolaAperta;
				Split_Exp_Valve_Perc = (unsigned int)( ((float)Me.ExpValve_Act / (float)temp) * 100.0); 				
				if(Split_Exp_Valve_Perc>100)
					Split_Exp_Valve_Perc=100;
				
				InsertRegSec(1, EngineBox.Pressione_Liq_P);				//	REG_TOUCH_PRESSIONE_P			Pressione_P					84
				InsertRegSec(2, EngineBox.Pressione_Gas_G);				//	REG_TOUCH_PRESSIONE_G			Pressione_G					85	
				WriteModListRegSec (TouchAddres, REG_TOUCH_PRESSIONE_P, 2);			
						
				WriteModRegSec (TouchAddres, REG_TOUCH_TEMP_P, EngineBox.Temp_Collettore_Piccolo);		//	REG_TOUCH_TEMP_P				Temp_Tubo_Piccolo_Liquido	93	
						
				InsertRegSec(1, Me.Temperature.Liquid);					//	REG_TOUCH_PICCOLO_SPLIT_1		Piccolo_Split_1				96						
				InsertRegSec(2, Me.Temperature.Gas);						//	REG_TOUCH_GRANDE_SPLIT_1		Grande_Split_1				97
				InsertRegSec(3, Me.Temperature.Ambient);						//	REG_TOUCH_ARIA_INGRESSO_SPLIT_1	Aria_Ingresso_Split_1		98
				InsertRegSec(4, Me.Temperature.AirOut);					//	REG_TOUCH_ARIA_USCITA_SPLIT_1	Aria_Uscita_Split_1			99
				InsertRegSec(5, Me.Fan_Tacho);						//	REG_TOUCH_VENTILATORE_SPLIT_1	FanSpeed_Split_1			100
				//InsertRegSec(6, Me.ExpValve_Act);					//	REG_TOUCH_STATO_VALVOLA_SPLIT_1	Stato_Valvola_Split_1		101
				InsertRegSec(6, Split_Exp_Valve_Perc);					//	REG_TOUCH_STATO_VALVOLA_SPLIT_1	Stato_Valvola_Split_1		101
				WriteModListRegSec (TouchAddres, REG_TOUCH_PICCOLO_SPLIT_1, 6);

			break;
*/			
			case 12:		// pagina 15 -> Diagnostic Inverter data			//Ex.Pag.12			
			case 16:		// pagina 16-> Power Monitor	
				
				// Dati restituiti da Inverter	=> Potenza in centesimi di Kw (es. 5Kw = 500)
				//								=> Frequenza in centesimi di Hz (es. 120Hz = 12000)
				//								=> Tensione in centesimi di % Perc. (es. 90% = 9000)
				//								=> Corrente in centesimi di % Perc. (es. 35% = 3500)				
				//   Amp         =     Watt         /     Volt
				

				//DEBUG
				vK_Power_Inverter = DiagnosticInverter.Power_Inverter;
				//vK_Supply_Inverter = K_Supply_Inverter;
				
				/*
				DiagnosticInverter.InPower = 400;	// 320 = 3200W
				DiagnosticInverter.OutCurr = 10000;	// 5000 = 50.00%
				//DiagnosticInverter.InVolt = 9700;	// 9700 = 97.00%
				//DiagnosticInverter.OutVolt = 6500;	// 6500 = 65.00%	
				*/
				//DEBUG
				
				Inverter_MaxCurr = vK_Power_Inverter / vK_Supply_Inverter;			// Max_Power_Inverter / Valim_InverterVcc -> (Vac * SQR(2) o SQR(3) se trifase)
				Inverter_InCurr = (unsigned int)( (float)DiagnosticInverter.InPower*10.0 / (float)vK_Supply_Inverter * 100.0);	// I = P(W)*100 / Volt -> Corrente in centesimi di A
				//Inverter_OutCurr = (unsigned int)(((float)DiagnosticInverter.OutCurr * (float)Inverter_MaxCurr)/100.0);	// Perc. -> Ampere (centesimi di A)
				Inverter_OutCurr = (unsigned int)( (float)DiagnosticInverter.OutPower*10.0 / (float)K_VoltOutMaxInverter * 100.0);	// Perc. -> Ampere (centesimi di A)
				
				Inverter_InVolt = (unsigned int)(((float)DiagnosticInverter.InVolt * (float)vK_Supply_Inverter)/100.0);	// Perc. -> Volt (centesimi di V)
				Inverter_OutVolt = (unsigned int)(((float)DiagnosticInverter.OutVolt * (float)vK_Supply_Inverter)/100.0);	// Perc. -> Volt (centesimi di V)
				
				InsertRegSec(1, DiagnosticInverter.Alarm);					// REG_TOUCH_INVERTER_ALARM			bits
				InsertRegSec(2, DiagnosticInverter.OutFreq);				// REG_TOUCH_INVERTER_OUT_FREQ		Hz (centesimi)
				InsertRegSec(3, DiagnosticInverter.Status);				// REG_TOUCH_INVERTER_STATUS		bits
				InsertRegSec(4, Inverter_OutCurr/*DiagnosticInverter.OutCurr*/);				// REG_TOUCH_INVERTER_OUT_CURR		% Perc. (centesimi) di Max Corrente (Taglia inverter in KW / Volt Inverter (400V)  
				InsertRegSec(5, Inverter_InVolt/*DiagnosticInverter.InVolt*/);				// REG_TOUCH_INVERTER_IN_VOLT		% Perc. (centesimi)	di Max Volt alimentazione inverter 3F (400V))
				InsertRegSec(6, Inverter_OutVolt/*DiagnosticInverter.OutVolt*/);				// REG_TOUCH_INVERTER_OUT_VOLT		% Perc. (centesimi)	di Max Volt alimentazione inverter 3F (400V))
				InsertRegSec(7, DiagnosticInverter.InPower);				// REG_TOUCH_INVERTER_IN_POWER		Kw (centesimi)
				InsertRegSec(8, DiagnosticInverter.OutPower);				// REG_TOUCH_INVERTER_OUT_POWER		Kw (centesimi)
				InsertRegSec(9, DiagnosticInverter.ReqFreq);				// REG_TOUCH_INVERTER_REQ_FREQ		Hz (centesimi)
				InsertRegSec(10, DiagnosticInverter.MaxReqFreq);			// REG_TOUCH_INVERTER_MAX_REQ_FREQ	Hz (centesimi)
				WriteModListRegSec (TouchAddres, REG_TOUCH_INVERTER_ALARM, 10);
				
				InsertRegSec(1, DiagnosticInverter.Max_Inverter);										// REG_TOUCH_INVERTER_QUANTITY
				WriteModListRegSec (TouchAddres, REG_TOUCH_INVERTER_QUANTITY, 1);
                
                WriteModRegSec (TouchAddres, REG_TOUCH_INVERTER_400V, 1*(Me.Inverter_Supply==400)+2*(Me.Inverter_Supply==566)+3*(Me.Inverter_Supply==283));
			break;					
/*
			case 17:		// pagina 17-> Debug Errore compressione
				InsertRegSec(1, EngineBox.Acceleration_Inverter);				// REG_TOUCH_ACC_INV
				InsertRegSec(2, EngineBox.Acceleration_GasPressure);			// REG_TOUCH_ACC_GAS
				WriteModListRegSec (TouchAddres, REG_TOUCH_ACC_INV, 2);                
                WriteModRegSec (TouchAddres, REG_TOUCH_SYS_PERSERR_2, EngineBox.ErroriPers2);      
                
                WriteModRegSec (TouchAddres, REG_TOUCH_PRESSIONE_G, EngineBox.Pressione_Gas_G);  
                WriteModRegSec (TouchAddres, REG_TOUCH_COMPRESSOR_SPEED, EngineBox.Compressor_Speed);             
            break;
*/			
			case 20:
				InsertRegSec(1, EngineBox.UTA_H.CumulativeAlarm);					// REG_TOUCH_UTA_CUM_ALARM
				WriteModListRegSec (TouchAddres, REG_TOUCH_UTA_CUM_ALARM, 1);
			break;
			
	        case 21:
				InsertRegSec(1, EngineBox.UTA_H.EngineBoxErr1);					// REG_TOUCH_UTA_ERROR_1
				InsertRegSec(2, EngineBox.UTA_H.EngineBoxPersErr1);				// REG_TOUCH_UTA_PERSERROR_1
				WriteModListRegSec (TouchAddres, REG_TOUCH_UTA_ERROR_1, 2);
			break;
				
          case 24:

				//InsertRegSec(1, EngineBox.HoursOfLife);				// REG_TOUCH_HOURS_OF_LIFE v8.4.16 -> invio ore di vita unita master x diagnostica
				InsertRegSec(2, EngineBox.UTA_H.EngineTempOut);				// REG_TOUCH_MANDATA_COMPRESSORE
				InsertRegSec(3, EngineBox.UTA_H.EngineTempIn);				// REG_TOUCH_RITORNO_COMPRESSORE
				InsertRegSec(4, EngineBox.UTA_H.EngineTempBatt);			// REG_TOUCH_TEMP_CONDENSATORE
				InsertRegSec(5, EngineBox.UTA_H.EngineTempAir);			// REG_TOUCH_SEA_WATER_IN_TEMP
				InsertRegSec(6, EngineBox.UTA_H.EnginePressLiq);			// REG_TOUCH_PRESSIONE_P
				InsertRegSec(7, EngineBox.UTA_H.EnginePressGas);			// REG_TOUCH_PRESSIONE_G
				InsertRegSec(8, EngineBox.UTA_H.EngineCompSpeed);			// REG_TOUCH_COMPRESSOR_SPEED
				//InsertRegSec(9, EngineBox.Pressure_Req);			// REG_TOUCH_TEMP_RICHIESTA
				InsertRegSec(10, EngineBox.UTA_H.EngineReqPressure);				// REG_TOUCH_PRESSIONE_RICHIESTA
				InsertRegSec(11, EngineBox.UTA_H.StatoCompressore);			// REG_TOUCH_STATO_COMPRESSORE
				InsertRegSec(12, EngineBox.UTA_H.Mode);				// REG_TOUCH_ENGINE_BOX_MODE
				InsertRegSec(13, EngineBox.UTA_H.IdMasterSplit);			// REG_TOUCH_MASTER_SPLIT
				//Saltare locazione 14 per clear error in posizione // REG_TOUCH_CLEAR_ERROR				0x005C
				InsertRegSec(15, EngineBox.UTA_H.EngineTempLiq);	// REG_TOUCH_TEMP_P
				InsertRegSec(16, EngineBox.UTA_H.EngineTempGas);	// REG_TOUCH_TEMP_G
				InsertRegSec(17, EngineBox.SystemCycleTime);			// REG_TOUCH_SYSTEM_CYCLE_TIME v8.4.16 -> invio tempo di ciclo unita master x diagnostica
				WriteModListRegSec (TouchAddres, REG_TOUCH_HOURS_OF_LIFE, 17);

                InsertRegSec(1, EngineBox.UTA_H.HW_Ver);                 // REG_TOUCH_MASTER_HW_VERSION
                InsertRegSec(2, EngineBox.UTA_H.FW_Ver_HW);              // REG_TOUCH_MASTER_FW_VERSION_HW                
                InsertRegSec(3, EngineBox.UTA_H.FW_Ver_FW);              // REG_TOUCH_MASTER_FW_VERSION_FW
                InsertRegSec(4, EngineBox.UTA_H.FW_Rev_FW);             // REG_TOUCH_MASTER_FW_REVISION_FW
                WriteModListRegSec (TouchAddres, REG_TOUCH_MASTER_HW_VERSION, 4);    
				
                InsertRegSec(1, EngineBox.UTA_H.EngineFanSpeed);             // REG_TOUCH_POWER_PUMP_DIAG
                WriteModListRegSec (TouchAddres, REG_TOUCH_POWER_PUMP_DIAG, 1);    
 
				//InsertRegSec(1, EngineBox.);             // REG_TOUCH_MASTER_FW_REVISION_FW
                //WriteModListRegSec (TouchAddres, REG_TOUCH_POWER_PUMP_DIAG, 4);                
                
            break;
   
            
            case 25:
				// Calcolo percentuale apertura valvola 0..100%
				temp = K_MaxExcursionValve; //ValvolaAperta;
				Split_Exp_Valve_Perc = (unsigned int)( ((float)EngineBox.UTA_H.Split_Exp_Valve / (float)temp) * 100.0); 
				if(Split_Exp_Valve_Perc>100)
					Split_Exp_Valve_Perc=100;
				
				InsertRegSec(1, EngineBox.UTA_H.Split_LiquidTemp);				// REG_TOUCH_PICCOLO_SPLIT_1
				InsertRegSec(2, EngineBox.UTA_H.Split_GasTemp);					// REG_TOUCH_GRANDE_SPLIT_1
				InsertRegSec(3, EngineBox.UTA_H.Split_AirInTemp);					// REG_TOUCH_ARIA_INGRESSO_SPLIT_1
				InsertRegSec(4, EngineBox.UTA_H.Split_AitOutTemp);				// REG_TOUCH_ARIA_USCITA_SPLIT_1
				////////InsertRegSec(5, Me.Fan_Tacho);					// REG_TOUCH_VENTILATORE_SPLIT_1
				//InsertRegSec(6, Me.ExpValve_Act);				// REG_TOUCH_STATO_VALVOLA_SPLIT_1
				InsertRegSec(6, Split_Exp_Valve_Perc);				// REG_TOUCH_STATO_VALVOLA_SPLIT_1
				
				//InsertRegSec(7, Me.Pres_Big);				// REG_TOUCH_PRESSIONE_G_SPLIT_1
				//InsertRegSec(8, Me.Pres_Small);				// REG_TOUCH_PRESSIONE_P_SPLIT_1
				InsertRegSec(7, EngineBox.UTA_H.EnginePressGas);	// REG_TOUCH_PRESSIONE_G_SPLIT_1	//Metto pressioni box motore perchè non presenti sonde di pressione salvo casi eccezionali da valutare
				InsertRegSec(8, EngineBox.UTA_H.EnginePressLiq);	// REG_TOUCH_PRESSIONE_P_SPLIT_1	//Metto pressioni box motore perchè non presenti sonde di pressione salvo casi eccezionali da valutare
				InsertRegSec(9, EngineBox.UTA_H.Split_ReqAirTemp);					// REG_TOUCH_TEMP_EVAP_SPLIT_1	// sottraggo PerformanceLossBattery per visualizzazione correta dato richiesto su touch
				InsertRegSec(10, EngineBox.UTA_H.Split_ReqGasPress);				// REG_TOUCH_RICH_PRESSIONE_SPLIT_1
				InsertRegSec(11, EngineBox.UTA_H.Mode);				// REG_TOUCH_ENGINE_BOX_MODE_SPLIT_1
				////////InsertRegSec(12, Me.CycleTime);				// REG_TOUCH_CYCLE_TIME_SPLIT_1
				WriteModListRegSec (TouchAddres, REG_TOUCH_PICCOLO_SPLIT_1, 12);
               
				InsertRegSec(1, EngineBox.UTA_H.Split_Superheat);						
                WriteModListRegSec (TouchAddres, REG_TOUCH_SUPERHEAT, 1); 

                InsertRegSec(1, EngineBox.UTA_H.Split_HW_Ver);                                        // REG_TOUCH_SPLIT_HW_VERSION
                InsertRegSec(2, EngineBox.UTA_H.Split_FW_Ver_HW);                                     // REG_TOUCH_SPLIT_FW_VERSION_HW                
                InsertRegSec(3, EngineBox.UTA_H.Split_FW_Ver_FW);                                     // REG_TOUCH_SPLIT_FW_VERSION_FW
                InsertRegSec(4, EngineBox.UTA_H.Split_FW_Rev_FW);                                    // REG_TOUCH_SPLIT_FW_REVISION_FW
                WriteModListRegSec (TouchAddres, REG_TOUCH_SPLIT_HW_VERSION, 4);             

            break;
			
			case 26:
                WriteModRegSec (TouchAddres, REG_TOUCH_SPLIT_AIR_IN, DiagnosticSplit.TempAmbReal);		//	REG_TOUCH_SPLIT_AIR_IN				Temp_Tubo_Piccolo_Liquido	93	
							                
                WriteModRegSec (TouchAddres, REG_TOUCH_UTA_ABIL, DiagnosticSplit.UTA_Abil);
                WriteModRegSec (TouchAddres, REG_TOUCH_DIAG_ONLY_FAN, DiagnosticSplit.Only_Fan);
                WriteModRegSec (TouchAddres, REG_TOUCH_SET_CMD_SPLIT_DISABLE, DiagnosticSplit.Set_CMD_Disable);  
                WriteModRegSec (TouchAddres, REG_TOUCH_DIAG_FROZEN_MODE, DiagnosticSplit.FrozenMode);
                WriteModRegSec (TouchAddres, REG_TOUCH_DIAG_MAX_VENTIL_MODE, DiagnosticSplit.MaxVentilMode);
                WriteModRegSec (TouchAddres, REG_TOUCH_DIAG_SPLIT_HEATER_ABIL, DiagnosticSplit.Heater_Abil);    //Abilito il riscaldatore se abilitato da FwSelection           
                WriteModRegSec (TouchAddres, REG_TOUCH_DIAG_MIN_SET_POINT, DiagnosticSplit.MinSetPoint);
                WriteModRegSec (TouchAddres, REG_TOUCH_DIAG_MAX_SET_POINT, DiagnosticSplit.MaxSetPoint);

				InsertRegSec(1, DiagnosticSplit.Model_FW_Version);				// REG_TOUCH_DIAG_SPLIT_MODEL_FW_VERSION				
				WriteModListRegSec (TouchAddres, REG_TOUCH_DIAG_SPLIT_MODEL_FW_VERSION, 1);		

				WriteModRegSec (TouchAddres, REG_TOUCH_SPLIT_TEST_CYCLE_TIME, DiagnosticSplit.CycleTime);
				break;							
                
                
			case 47:
                InsertRegSec(1, Me.ExpValve_Cur_Number_Priority);                                  // REG_TOUCH_VALVOLA_CUR_NUMB_PRIORITY                
                InsertRegSec(2, Me.ExpValve_Room_Number_Priority);                                 // REG_TOUCH_VALVOLA_ROOM_NUMB_PRIORITY
                InsertRegSec(3, Me.ExpValve_Sum_Tot_Percent);                                      // REG_TOUCH_VALVOLA_SUM_TOT_PERCENT
                InsertRegSec(4, Me.ExpValve_Sum_Req_Percent);                                      // REG_TOUCH_VALVOLA_SUM_REQ_PERCENT                
                InsertRegSec(5, Me.ExpValve_On_Demand_Lim_Max);                                    // REG_TOUCH_VALVOLA_ONDEMAND_LIM_MAX
                InsertRegSec(6, Me.ExpValve_On_Demand_Request);                                    // REG_TOUCH_VALVOLA_MAX_PERC_PRIORITY                
                WriteModListRegSec (TouchAddres, REG_TOUCH_VALVOLA_CUR_NUMB_PRIORITY, 6);             
				break;							
                
            case 48:
                InsertRegSec(1, EngineBox.Total_Fan_Absorption);                                   // REG_TOUCH_SPLIT_TOTAL_FAN_ABSORPTION                
                InsertRegSec(2, EngineBox.Compressor1_Absorption);                                 // REG_TOUCH_COMPRESSOR1_ABSORPTION
                InsertRegSec(3, EngineBox.Compressor2_Absorption);                                 // REG_TOUCH_COMPRESSOR2_ABSORPTION
                InsertRegSec(4, EngineBox.Sea_Water_Pump_Absorption);                              // REG_TOUCH_SEA_WATER_PUMP_ABSORPTION                
                WriteModListRegSec (TouchAddres, REG_TOUCH_SPLIT_TOTAL_FAN_ABSORPTION, 4);             
                
				InsertRegSec(1, EngineBox.PowerBoxAbsorption);										// REG_TOUCH_BOX_ABSORPTION
				WriteModListRegSec (TouchAddres, REG_TOUCH_BOX_ABSORPTION, 1);					                
                break;
                
            case 49:
                temp = TempToPressureGasR410A(Touch[0].TestAll_TempBatt_SP_Cool);
                InsertRegSec(1, temp);                                   // REG_TOUCH_TESTALL_WORK_PRESS_COOL           
                temp = TempToPressureGasR410A(Touch[0].TestAll_TempBatt_SP_Heat);
                InsertRegSec(2, temp);                                   // REG_TOUCH_TESTALL_WORK_PRESS_HEAT
                WriteModListRegSec (TouchAddres, REG_TOUCH_TESTALL_WORK_PRESS_COOL, 2);             
                break;                  
                
            case 50:
                temp = TempToPressureGasR410A(Touch[0].TestAll_TempBatt_SP_Frigo);
                InsertRegSec(1, temp);                                   // REG_TOUCH_TESTALL_WORK_PRESS_FRIGO           
                temp = TempToPressureGasR410A(Touch[0].TestAll_TempBatt_SP_Freezer);
                InsertRegSec(2, temp);                                   // REG_TOUCH_TESTALL_WORK_PRESS_FReezer
                WriteModListRegSec (TouchAddres, REG_TOUCH_TESTALL_WORK_PRESS_FRIGO, 2);             
//                WriteModRegSec (TouchAddres, REG_TOUCH_DIAG_FROZEN_MODE, DiagnosticSplit.FrozenMode);
                break;                  
                
            case 52:                
                InsertRegSec(1, EngineBox.ComError.CntComErr);                          //  REG_TOUCH_MASTER_COMERROR_CNTCOMERR
                InsertRegSec(2, EngineBox.ComError.TimeOutComErr);                  
                InsertRegSec(3, EngineBox.ComError.CRC_ComErr);                   
                InsertRegSec(4, EngineBox.ComError.ModCntComErr);                    
                InsertRegSec(5, EngineBox.ComError.ModTimeOutComErr);
                InsertRegSec(6, EngineBox.ComError.ModCRC_ComErr);            
                InsertRegSec(7, EngineBox.ComError.ModCntComErr_U2);                  
                InsertRegSec(8, EngineBox.ComError.ModTimeOutComErr_U2);              
                InsertRegSec(9, EngineBox.ComError.ModCRC_ComErr_U2);                 
                InsertRegSec(10, EngineBox.ComError.CntComErr_U3);                   
                InsertRegSec(11, EngineBox.ComError.TimeOutComErr_U3);               
                InsertRegSec(12, EngineBox.ComError.CRC_ComErr_U3);                     // REG_TOUCH_MASTER_COMERROR_CRC_COMERR_U3
                WriteModListRegSec (Add_TouchRoom, REG_TOUCH_MASTER_COMERROR_CNTCOMERR, 12);            
                
				WriteModRegSec (TouchAddres, REG_TOUCH_SYSTEM_CYCLE_TIME, EngineBox.SystemCycleTime);                
            break;                
                
            case 62:                
                InsertRegSec(1, DiagnosticSplit.ComError.CntComErrSec);                          //  REG_TOUCH_DIAG_SPLIT_CNTCOMERRSEC
                InsertRegSec(2, DiagnosticSplit.ComError.TimeOutComErrSec);                  
                InsertRegSec(3, DiagnosticSplit.ComError.TimeOutInterCharComErrSec);                   
                InsertRegSec(4, DiagnosticSplit.ComError.CRC_ComErrSec);                    
                InsertRegSec(5, DiagnosticSplit.ComError.ModCntComErrSec);
                InsertRegSec(6, DiagnosticSplit.ComError.ModTimeOutComErrSec);            
                InsertRegSec(7, DiagnosticSplit.ComError.ModTimeOutInterCharComErrSec);                  
                InsertRegSec(8, DiagnosticSplit.ComError.ModCRC_ComErrSec);              
                WriteModListRegSec (Add_TouchRoom, REG_TOUCH_DIAG_SPLIT_CNTCOMERRSEC, 8);            

				WriteModRegSec (TouchAddres, REG_TOUCH_SPLIT_TEST_CYCLE_TIME, DiagnosticSplit.CycleTime);                
            break;                
		}
	}
}

void RefreshAllTouchOutPut(void)
{
    unsigned int i = 0;
    
    if(Me.EnableHiSpeedModBusTouch_Sec==1)        
        Change_ProtocolBusSlave(K_ModBus_Baud115200, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    else        
        Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    
	/////for(i=1; i<=MaxTouch; i++)
	for(i=Add_TouchRoom; i< (Add_TouchRoom+MaxTouch); i++)
		RefreshTouchOutPut(i);
}

void InitTouch(void)
{
	unsigned int i = 0;
	
    if(Me.EnableHiSpeedModBusTouch_Sec==1)        
        Change_ProtocolBusSlave(K_ModBus_Baud115200, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    else        
        Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    
	for(i=0; i<MaxTouch; i++)
	/////for(i=Add_TouchRoom; i< (Add_TouchRoom+MaxTouch); i++)
	{
		//if(Touch[i].Enable && !Touch[i].OffLine)
        
        if(Touch[i].OnLine)
            CheckIfTouchIsReady(i);        
        
        if(Touch[i].OnLine)
		{
/*
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SP_ROOM, 2000);
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SP_ROOM_F, 6400);
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_FAN_SPEED, 9);
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_PWR, 0);
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_MODE, CoreRiscaldamento);
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_MAX_PWR_LIMIT, InitPowerLimit);
*/
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SP_ROOM, Touch[0].SetPoint);
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SP_ROOM_F, Touch[0].Script_SetPoint_F);
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_FAN_SPEED, Touch[0].FanMode);
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_PWR, Touch[0].Script_Split_Pwr);
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_MODE, Touch[0].DefMode);
			//WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_FUNCTION_MODE, Touch[0].DefMode*Touch[0].Script_Split_Pwr);
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_MAX_PWR_LIMIT, Touch[0].PowerLimit);	
	
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_MATRICOLA_LO, 0x3210);
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_MATRICOLA_HI, 0x7654);
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_MASTER_FW_VERSION, Versione_Fw);
			
			//WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SPLIT_TEST_ADDRESS, 2);			
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_DIAG_BOX_ADDRESS, 1);		
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_INVERTER_ADDRESS, 1);		
			
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_ECOMODE_ENABLE, Me.EcoModeEnable);		
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_POWER_ECOMODE, Me.Power_EcoMode);								
			
			//WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SPLIT_CMD_ADDRESS, 0);//2);
			
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SPLIT_TEST_ADDRESS, 1);			
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SPLIT_CMD_ADDRESS, 1);	
	
            
			WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_ROOM_ADDRESS, My_Address);
            
            WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TRIGGER_INIT, 0);
            WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_OTHER_CABIN_CMD_SEND, 0);
#if(K_AutoTempFanControl==1)
            WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_ONLY_FAN, 2);
#elif(SplitMode==K_I_Am_InverterFAN || SplitMode==K_I_Am_StandardFAN || SplitMode==K_I_Am_Atex_Fan || SplitMode==K_I_Am_On_Off_Fan)
            WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_ONLY_FAN, 1);
#else
            WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_ONLY_FAN, 0);            
#endif
#if(K_Heater_Abil==1)            
            WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_HEATER_ABIL, 1);    //Abilito il riscaldatore se abilitato da FwSelection   
#else
            WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_HEATER_ABIL, 0);    //Abilito il riscaldatore se abilitato da FwSelection               
#endif
            
            WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SPLIT_FROZEN_MODE, Me.Frozen.Mode);                
            
			if(Me.My_Address == k_Split_Master_Add)								// Se sono uno slave Master...
			{
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_I_AM_MASTER_SPLIT, 1);		// Attivo flag su Touch per gestione Hide/View pagine riservate a MasterSlave

#if(SplitMode == K_I_Am_OnlyTouchComm)
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_ONLY_COMUNICATION, 1);
#endif
                
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_UTA_SP_ROOM, Touch[0].Uta_SP_Room);		
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_UTA_FAN_SPEED, Touch[0].Uta_Fan_Speed);								
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_UTA_MODE, Touch[0].Uta_Mode);		
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_UTA_POWER, Touch[0].Uta_Power);								
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_UTA_SP_HUMI, Touch[0].Uta_SP_Humi);		
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_UTA_SP_ROOM_F, Touch[0].Uta_SP_Room_F);
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SPLIT_SYSTEM_DISABLE, Touch[0].SystemDisable);			
                DiagnosticSplit.Ready_UtaMode = Touch[0].Uta_Mode;

				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SPLIT_SET_SETPOINT, Touch[0].SetPoint);
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SPLIT_SET_SETPOINT_F, Touch[0].Script_SetPoint_F);
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SPLIT_SET_FANSPEED, Touch[0].FanMode);
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SPLIT_SET_POWER, Touch[0].Script_Split_Pwr);
                DiagnosticSplit.Ready_SetPoint = Touch[0].SetPoint;
                DiagnosticSplit.Ready_SetPoint_F = Touch[0].Script_SetPoint_F;
                DiagnosticSplit.Ready_FanMode = Touch[0].FanMode;
                DiagnosticSplit.Ready_PowerOn = Touch[0].Script_Split_Pwr;

                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SPLIT_SET_HEATER_PWR, Touch[0].HeaterPwr);

				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_INIT_SP_ROOM, Touch[0].SetPoint);
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_INIT_SP_ROOM_F, Touch[0].Script_SetPoint_F);
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_INIT_FAN_SPEED, Touch[0].FanMode);
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_INIT_POWER, Touch[0].Script_Split_Pwr);

				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_DOUBLE_PUMP_SWITCH_TIME, EngineBox.DoublePumpSwitchTime);
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_DOUBLE_PUMP_SELECTION, EngineBox.DoublePumpSelection);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_INVERTER_400V, 1*(Me.Inverter_Supply==400)+2*(Me.Inverter_Supply==566)+3*(Me.Inverter_Supply==283));
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_ABIL_DOUBLE_PUMP, Me.DoublePump_Abil);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SPLITMODE_ABIL, Me.SplitMode_Abil);
                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_ABIL_VALVE_ONDEMAND, EngineBox.AbilValveOnDemand);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_ABIL_SCORE_ONDEMAND, EngineBox.AbilScoreOnDemand);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_ON_PRIORITY_MODE, Touch[0].On_Priority_Mode);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_VALVE_ONDEMAND_MIN_PERC_VAL, Touch[0].ValveOnDemand_Min_Percent_Val);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_MAX_LIM_ONDEMAND_TOTAL_SCORE, Touch[0].Max_Lim_ON_Demand_Total_Score);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TIME_ONDEMAND_VALVE_SWITCH, Touch[0].TimeOnDemandValveSwitch);

                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_ABIL, Touch[0].TestAll_Abil);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_SUPERHEAT_SP_COOL, Touch[0].TestAll_SuperHeat_SP_Cool);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_SUPERHEAT_SP_HEAT, Touch[0].TestAll_SuperHeat_SP_Heat);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_MAX_OPEN_VALVE_COOL, Touch[0].TestAll_MaxOpValve_Cool);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_MAX_OPEN_VALVE_HEAT, Touch[0].TestAll_MaxOpValve_Heat);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_WORK_TEMP_COOL, Touch[0].TestAll_TempBatt_SP_Cool);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_WORK_TEMP_HEAT, Touch[0].TestAll_TempBatt_SP_Heat);

                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_FROZEN_ABIL, Touch[0].TestALL_Frozen_Abil);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_SUPERHEAT_SP_FRIGO, Touch[0].TestAll_SuperHeat_SP_Frigo);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_SUPERHEAT_SP_FREEZER, Touch[0].TestAll_SuperHeat_SP_Freezer);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_MAX_OPEN_VALVE_FRIGO, Touch[0].TestAll_MaxOpValve_Frigo);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_MAX_OPEN_VALVE_FREEZER, Touch[0].TestAll_MaxOpValve_Freezer);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_WORK_TEMP_FRIGO, Touch[0].TestAll_TempBatt_SP_Frigo);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_WORK_TEMP_FREEZER, Touch[0].TestAll_TempBatt_SP_Freezer);

                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_ABIL_DEFROSTING, Touch[0].Abil_Defrosting);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_DEFROSTING_STEP_TIME, Touch[0].Defrosting_Step_Time);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_DEFROSTING_TIME, Touch[0].Defrosting_Time);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_DEFROSTING_TEMP_SET, Touch[0].Defrosting_Temp_Set);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_DRIPPING_TIME, Touch[0].Dripping_Time);    
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_VALVOLA_FROZEN, Touch[0].Valvola_Frozen);  
                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_MIN_OPEN_VALVE_COOL, Touch[0].TestAll_MinOpValve_Cool);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_MIN_OPEN_VALVE_HEAT, Touch[0].TestAll_MinOpValve_Heat);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_MIN_OPEN_VALVE_FRIGO, Touch[0].TestAll_MinOpValve_Frigo);
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_MIN_OPEN_VALVE_FREEZER, Touch[0].TestAll_MinOpValve_Freezer);
                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN, Touch[0].TestAll_SuperHeat_Heat_pGain);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_SUPERHEAT_HEAT_IGAIN, Touch[0].TestAll_SuperHeat_Heat_iGain);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_SUPERHEAT_HEAT_DGAIN, Touch[0].TestAll_SuperHeat_Heat_dGain);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID, Touch[0].TestAll_SuperHeat_Heat_Time_Exec_PID);                

                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_SUPERHEAT_COOL_PGAIN, Touch[0].TestAll_SuperHeat_Cool_pGain);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_SUPERHEAT_COOL_IGAIN, Touch[0].TestAll_SuperHeat_Cool_iGain);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_SUPERHEAT_COOL_DGAIN, Touch[0].TestAll_SuperHeat_Cool_dGain);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID, Touch[0].TestAll_SuperHeat_Cool_Time_Exec_PID);                

                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_MAINTENANCE_FLAG, Touch[0].Maintenance_Flag);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_MAINTENANCE_SET_TIME, Touch[0].Maintenance_Set_Time);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_MAINTENANCE_REMANING_TIME, Touch[0].Maintenance_Remaning_Time);                

                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT, Touch[0].SuperHeat_Cool_Min_SetPoint);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SUPERHEAT_COOL_MAX_VALUE_CORRECT, Touch[0].SuperHeat_Cool_Max_Value_Correct);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SUPERHEAT_HEAT_MIN_SETPOINT, Touch[0].SuperHeat_Heat_Min_SetPoint);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SUPERHEAT_HEAT_MAX_SETPOINT, Touch[0].SuperHeat_Heat_Max_SetPoint);                

                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_THRESHOLD_COMPRESSOR_HI, Touch[0].Threshold_Compressor_Hi);                

                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SET_HUMI, Touch[0].Set_Humi);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SET_DELTA_TEMP_MIN_DEHUMI, Touch[0].Set_Delta_Temp_Min_Dehumi);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SET_FAN_SPEED_DEHUMI, Touch[0].Set_Fan_Speed_Dehumi);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SET_PRESS_DEHUMI, Touch[0].Set_Press_Dehumi);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SET_TEMP_AIR_OUT, Touch[0].Set_Temp_Air_Out);   
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SET_TEMP_AIR_OUT_DEHUMI, Touch[0].Set_Temp_Air_Out_Dehumi);   
                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SET_MIN_SET_POINT, K_LimLo_SPLIT_SP);   
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SET_MAX_SET_POINT, K_LimHi_SPLIT_SP);   

                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SPLIT_HW_VERSION, Me.HW_Version);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SPLIT_FW_VERSION_HW, Me.FW_Version_HW);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SPLIT_FW_VERSION_FW, Me.FW_Version_FW);                
                WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_SPLIT_FW_REVISION_FW, Me.FW_Revision_FW);                
			}
			else
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_I_AM_MASTER_SPLIT, 0);		// Dsattivo flag su Touch
			
#if K_DEVELOP==1
            ReadModRegSec (Add_TouchRoom, REG_TOUCH_PAGE, &(Touch[0].Current_Page));
            if(Me.My_Address==0)        
            {
                if(Touch[0].Current_Page==0)
                    WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_PAGE, 10);	// Aggiunto in v8.6.36 
            }
			else if(Touch[0].Current_Page==0)
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_PAGE, 2);	// Modificato in v8.4.16 
#else            
			if(Touch[0].Current_Page==0)
				WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_PAGE, 2);	// Modificato in v8.4.16 
#endif               

		}
	}
}

//------------------------------------------------
// Routines per il controllo dello stato di "Ready"
// dell'eventuale touch collegato
// Ritorna 0 == OK
// Ritorna 1 == Errore di scrittura
//------------------------------------------------
int CheckIfTouchIsReady(unsigned char i)
{
    int x;
    int temp;
    
    for(x=0; x<30; x++)     // Tento MAX 30 volte
    {
        WriteModRegSec ((i+1), REG_TOUCH_MASTER_FW_VERSION, Versione_Fw);	
        DelaymSec(100);
        ReadModRegSec ((i+1), REG_TOUCH_MASTER_FW_VERSION, &(temp));
        if(temp==Versione_Fw)        
        {
            DelaymSec(200);
            return 0;
        }
    }

    Touch[i].OnLine = 0;
    Touch[i].OffLine = 1;    
    return 1;   // Se non riesco a scriverci... ritorno errore
    
}


//------------------------------------------------
// Routines di gestione dei Pannelli NetBuilding
//------------------------------------------------
void Search_NetB(void)
{
	char Result;
    unsigned int i = 0;    
    
    Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud

	// Scansiono la rete alla ricerca dei Pannelli
	for(i=0; i<MaxNetB; i++)
	{
		Result = Send_WaitModRxSec((Add_NetBRoom+i), ReadModCmd, REG_NETB_FW_REL, 1, MaxModChkWaitSec, MaxModChkRetriesSec, K_ModBusZeroBased);
		NetB[i].Enable = (Result == 0) ;
		NetB[i].OffLine = (Result != 0);
        NetB[i].OnLine = (NetB[i].Enable == 1) && (NetB[i].OffLine == 0);                
		Mod_LastRxSec.Valid_Data = 0;

        if(NetB[i].OnLine)
        {
            NetB[i].CntErrTouch = 0;
            NetB[i].PermanentOffline = 0;
            Me.ModBusDeviceDetected=K_ModDeviceNetB;                        
            InitNetB();
        }     
	}
}

void Resume_NetBOffLine(unsigned char * Cnt)
{
	char Result;
    static int CntCycleOnLine=0;    
	
	// Provo a recuperare i moduli NetB
	if((NetB[(*Cnt)].Enable == 1) && (NetB[(*Cnt)].OffLine == 1))
	{     
        Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    
		Result = Send_WaitModRxSec((Add_NetBRoom +(*Cnt)), ReadModCmd, REG_NETB_FW_REL, 1, MaxModChkWaitSec, MaxModChkRetriesSec, K_ModBusZeroBased);
        
        if(Result==0)
            CntCycleOnLine++;
        else
            CntCycleOnLine=0;
        
        if(CntCycleOnLine >= 10)
        {        
            NetB[(*Cnt)].OffLine = (Result != 0);
            //NetB[(*Cnt)].OnLine = (NetB[(*Cnt)].OffLine == 0);            
            CntCycleOnLine=0;
        }               
            
		Mod_LastRxSec.Valid_Data = 0;
/*
        if(NetB[(*Cnt)].OnLine)
        {
            Me.MaxFanVentil_Mode = K_DefaultFanSpeed_NetB;
            Me.ModDeviceType = K_ModDeviceNetB;
            InitNetB();
        }     
*/        
        
        if(NetB[(*Cnt)].OffLine == 1)
        {
            if(NetB[(*Cnt)].PermanentOffline == 0 && CntCycleOnLine==0)
            {
                NetB[(*Cnt)].CntErrTouch++;
                if(NetB[(*Cnt)].CntErrTouch > k_MAX_Cnt_Err_NetB)
                    NetB[(*Cnt)].PermanentOffline = 1;
            }
        }
        else
        {
            Me.MaxFanVentil_Mode = K_DefaultFanSpeed_NetB;
            Me.ModDeviceType = K_ModDeviceNetB;
            NetB[(*Cnt)].OnLine = (NetB[(*Cnt)].Enable == 1) && (NetB[(*Cnt)].OffLine == 0);
            if(NetB[(*Cnt)].CntErrTouch>2)
                InitNetB();
            
            NetB[(*Cnt)].CntErrTouch = 0;
            if(NetB[(*Cnt)].PermanentOffline == 1)
            {        
                NetB[(*Cnt)].PermanentOffline = 0;
            }
        }        
	}
	(*Cnt) += 1;
	if((*Cnt) >= MaxNetB)
		(*Cnt) = 0;
}

void CheckHotPlugNetB(unsigned char * Cnt)
{
	char Result = 0;
	unsigned char CurrentId = 0;
    static int CntCycleOnLine=0;

	while((*Cnt) < MaxNetB)				// Provo a recuperare i NetB
	{
		CurrentId = (*Cnt);
		if((NetB[CurrentId].Enable == 0))
		{

            Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud

            DisableModComErrorCounter = 1;
            Result = Send_WaitModRxSec((Add_NetBRoom +(*Cnt)), ReadModCmd, REG_NETB_FW_REL, 1, MaxModChkWaitSec, 1, K_ModBusZeroBased);
            DisableModComErrorCounter = 0;
            
            if(Result==0)
                CntCycleOnLine++;
            else
                CntCycleOnLine=0;
                
            if(CntCycleOnLine >= 10)
            {
                NetB[(*Cnt)].OffLine = (Result != 0);                
                NetB[(*Cnt)].Enable = !NetB[(*Cnt)].OffLine;	// Se lo trovo lo abilito.    
                NetB[(*Cnt)].OnLine = (NetB[(*Cnt)].Enable == 1) && (NetB[(*Cnt)].OffLine == 0);  
                CntCycleOnLine=0;
            }   
            //NetB[(*Cnt)].OnLine = (NetB[(*Cnt)].Enable == 1) && (NetB[(*Cnt)].OffLine == 0);     
            
            //if(NetB[(*Cnt)].OnLine)		// se ho trovato il touch e mi rispnde online
            if(NetB[(*Cnt)].OnLine)
            {
                NetB[(*Cnt)].CntErrTouch = 0;
                NetB[(*Cnt)].PermanentOffline = 0;
                Me.MaxFanVentil_Mode = K_DefaultFanSpeed_NetB;
                Me.ModDeviceType = K_ModDeviceNetB;
                InitNetB();
            }     
            
			(*Cnt) += 1;
			break;
		}
		(*Cnt) += 1;
	}
	if(((*Cnt) >= MaxNetB) || ((*Cnt) < 0))
		(*Cnt) = 0;
}


void RefreshNetBInPut(char NetBAddres)
{
	unsigned char DataIndexSec;
    
    DataIndexSec = NetBAddres-Add_NetBRoom;
    
    WriteCoilModRegSecNetB (Add_NetBRoom, COIL_NetB_ABILPROG, 1);    // Abilito programmazione registri via BUS
    
	SyncronizeDataOnOff_NetB(&Syncronize.Script_Split_Pwr, &Syncronize.Old.Script_Split_Pwr, &Touch[0].Script_Split_Pwr, &Syncronize.Engine.Script_Split_Pwr, REG_NETB_IMP_VEL_FAN, 0);				 	
	SyncronizeDataSetP_NetB(&Syncronize.SetPoint, &Syncronize.Old.SetPoint, &Touch[0].SetPoint, &Syncronize.Engine.SetPoint, REG_NETB_TEMP_SETP_10, 0);				
	SyncronizeDataVel_NetB(&Syncronize.FanMode, &Syncronize.Old.FanMode, &NetB[0].FanMode, &Syncronize.Engine.FanMode, REG_NETB_IMP_VEL_FAN, 0);				  

	ReadModRegSecNetB (NetBAddres, REG_NETB_TEMP_SETP_10, &(NetB[DataIndexSec].SetPoint));
    if(NetB[DataIndexSec].OnLine)
        NetB[DataIndexSec].SetPoint *= 10;															// Il setpoint che arriva dal pannello e' in decimi di grado

	ReadModRegSecNetB (NetBAddres, REG_NETB_IMP_VEL_FAN, &(NetB[DataIndexSec].FanMode));

	//ReadModRegSecNetB (NetBAddres, REG_NETB_TEMP_ATT_10, &(NetB[DataIndexSec].Temperature));  //ATTENZIONE: VALUTARE SE ISTANTANEA O MEDIATA!!!
    ReadModRegSecNetB (NetBAddres, REG_NETB_TEMP_IST_10, &(NetB[DataIndexSec].Temperature));	
    NetB[DataIndexSec].Temperature *= 10;														// La temperatura che arriva dal pannello e' in decimi di grado
	
    //ReadModRegSecNetB (NetBAddres, REG_NETB_PRESSURE_10, &(NetB[DataIndexSec].Pressure));
	
    //ReadModRegSecNetB (NetBAddres, COIL_NetB_SEASON, &(NetB[DataIndexSec].FunctionMode));
	//NetB[DataIndexSec].FunctionMode = ((NetB[DataIndexSec].FunctionMode & 0x0100) != 0); 		// Estraggo flag stagione: Raffrescamento=0 o Riscaldamento=1


/*
	if(NetB[DataIndexSec].OnLine)
	{	
		if(Send_WaitModRxSec(NetBAddres, ReadModCmd, REG_NETB_TEMP_SETP_10, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
			NetB[DataIndexSec].OffLine = 1;
			NetB[DataIndexSec].OnLine = 0;        
		else
			NetB[DataIndexSec].SetPoint = ExtracRegSec(1)*10; // Il setpoint che arriva dal pannello e' in decimi di grado

		if(Send_WaitModRxSec(NetBAddres, ReadModCmd, REG_NETB_IMP_VEL_FAN, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
			NetB[DataIndexSec].OffLine = 1;
		else
			NetB[DataIndexSec].FanMode = ExtracRegSec(1);

		if(Send_WaitModRxSec(NetBAddres, ReadModCmd, REG_NETB_TEMP_ATT_10, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
			NetB[DataIndexSec].OffLine = 1;
		else
			NetB[DataIndexSec].Temperature = ExtracRegSec(1)*10; // La temperatura che arriva dal pannello e' in decimi di grado

		if(Send_WaitModRxSec(NetBAddres, ReadModCmd, REG_NETB_PRESSURE_10, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
			NetB[DataIndexSec].OffLine = 1;
		else
			NetB[DataIndexSec].Pressure = ExtracRegSec(1); 		// La pressione che arriva dal pannello e' in decimi

		if(Send_WaitModRxSec(NetBAddres, ReadModCoil, COIL_NetB_SEASON, 16, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
			NetB[DataIndexSec].OffLine = 1;
		else
			NetB[DataIndexSec].FunctionMode = ((ExtracRegSec(1) & 0x0100) != 0); 		// Estraggo flag stagione: Raffrescamento=0 o Riscaldamento=1
	}
*/
}

void RefreshAllNetBInPut(void)
{
    unsigned int i = 0;    

    Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    
	for(i=Add_NetBRoom; i < (Add_NetBRoom+MaxNetB); i++)
		RefreshNetBInPut(i);
}

void RefreshNetBOutPut(char NetBAddres)
{
//	int SysError = 0;
/*
	DataIndexSec = NetBAddres-Add_NetBRoom;
	if(NetB[DataIndexSec].Enable & !NetB[DataIndexSec].OffLine)
	{	


//	.......

	}
*/
}

void RefreshAllNetBOutPut(void)
{   
    int temp = 0;
    Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
       
	//NetB[0].OffLine = Send_WriteModRegSec(Add_NetBRoom, COIL_NetB_ABILPROG, 1, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModForceSingleCoil, K_ModBusZeroBased);	// Abilito programmazione registri via BUS

    WriteCoilModRegSecNetB (Add_NetBRoom, COIL_NetB_ABILPROG, 1);    // Abilito programmazione registri via BUS
    
    ReadCoilModRegSecNetB (Add_NetBRoom, COIL_NetB_SEASON, &temp);
    
    if((temp==0)&&(Me.DefMode==CoreRiscaldamento))
        WriteCoilModRegSecNetB (Add_NetBRoom, COIL_NetB_SEASON, 1);
    else if((temp==1)&&(Me.DefMode==CoreRaffrescamento))
        WriteCoilModRegSecNetB (Add_NetBRoom, COIL_NetB_SEASON, 0);

        
    ReadModRegSecNetB (Add_NetBRoom, REG_NETB_LCD_INTENS, &temp);
    
    if(temp!=Me.NetB_LCD_Intens)
        WriteModRegSecNetB (Add_NetBRoom, REG_NETB_LCD_INTENS, Me.NetB_LCD_Intens);

#if(K_New_NetB==1)
    ReadModRegSecNetB (Add_NetBRoom, REG_NETB_NIGHTMODE_LCD_INTENSITY, &temp);
    
    if(temp!=Me.NetB_LCD_Intens)
        WriteModRegSecNetB (Add_NetBRoom, REG_NETB_NIGHTMODE_LCD_INTENSITY, Me.NetB_LCD_Intens);
#endif
    
    WriteModRegSecNetB (Add_NetBRoom, REG_NETB_REMOTE_TEMPERATURE, Me.TempAmbTouchVisual/10);
}

void InitNetB(void)
{ 

    Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud

    //NetB[0].OffLine = Send_WriteModRegSec(Add_NetBRoom, REG_NETB_TEMP_MIN, K_LimLo_SPLIT_SP/100, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Il registro dei touch hanno un offset di 1
    //NetB[0].OffLine = Send_WriteModRegSec(Add_NetBRoom, REG_NETB_TEMP_MAX, K_LimHi_SPLIT_SP/100, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Il registro dei touch hanno un offset di 1
    WriteCoilModRegSecNetB (Add_NetBRoom, COIL_NetB_ABILPROG, 1);    // Abilito programmazione registri via BUS

    WriteModRegSecNetB (Add_NetBRoom, REG_NETB_TEMP_MIN, 1800/*K_LimLo_SPLIT_SP*//100);
    WriteModRegSecNetB (Add_NetBRoom, REG_NETB_TEMP_MAX, K_LimHi_SPLIT_SP/100);
    
    WriteModRegSecNetB (Add_NetBRoom, REG_NETB_TEMP_SETP_10, Touch[0].SetPoint/10);    
    
    WriteModRegSecNetB (Add_NetBRoom, REG_NETB_LCD_INTENS_TIME, 15);    

#if(K_New_NetB==1)
    WriteModRegSecNetB (Add_NetBRoom, REG_NETB_NIGHTMODE_HOUR_START, 0);    
  
    WriteModRegSecNetB (Add_NetBRoom, REG_NETB_NIGHTMODE_HOUR_STOP, 0);    
#endif
    
    ApplyVel_NetB(&Touch[0].Script_Split_Pwr, &Touch[0].FanMode, REG_NETB_IMP_VEL_FAN);
    
/*
	for(i=0; i<MaxNetB; i++)
	{
		if(NetB[i].Enable && !NetB[i].OffLine)
		{

//			WriteModRegSecNetB ((Add_NetBRoom +i), REG_NETB_..., valore);
//			WriteModRegSecNetB ((Add_NetBRoom +i), REG_NETB_..., valore);
//			WriteModRegSecNetB ((Add_NetBRoom +i), REG_NETB_..., valore);
//			WriteModRegSecNetB ((Add_NetBRoom +i), REG_NETB_..., valore);
		
		}
	}
*/
}

void Reinit_DataSplitToTouch(void)
{	
	int Trig=0;
    int TrigOther=0;
    
    if(Me.EnableHiSpeedModBusTouch_Sec==1)        
        Change_ProtocolBusSlave(K_ModBus_Baud115200, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    else        
        Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    
#if(K_UTAEnable==0 && SplitMode!=K_I_Am_InverterFAN && SplitMode!=K_I_Am_ManualRegulation && SplitMode!=K_I_Am_StandardFAN && SplitMode!=K_I_Am_Atex_Fan && SplitMode!=K_I_Am_On_Off_Fan && SplitMode!=K_I_Am_OnlyTouchComm && SplitMode!=K_I_Am_Frigo && SplitMode!=K_I_Am_Freezer && SplitMode!=K_I_Am_LoopWater)
    if(Reinit.FromEngBox.TrigInit < K_LimLo_TOUCH_TRIGGER_INIT || Reinit.FromEngBox.TrigInit > K_LimHi_TOUCH_TRIGGER_INIT)
        Reinit.FromEngBox.TrigInit = 0;	             			

	Trig=Reinit.FromEngBox.TrigInit;
#else
    if(Reinit.FromEngBox.TrigInit!=0)
    {
        Reinit.FromEngBox.TrigInit=0;
        if(Touch[0].OnLine)
            WriteModRegSec (Add_TouchRoom, REG_TOUCH_TRIGGER_INIT, 0);
    }
#endif	
    TrigOther = Me.OtherCabin.Trigger_Init;
	if((Trig & 0x0001)==1 || (Trig & 0x0008)==8 || (TrigOther & 0x0008)==8)
	{
        if(Touch[0].OnLine)
		{
            WriteModRegSec (Add_TouchRoom, REG_TOUCH_PWR, Reinit.FromEngBox.Power);		
             //WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_FUNCTION_MODE, Reinit.FromEngBox.DefMode*Reinit.FromEngBox.Power);
            WriteModRegSec (Add_TouchRoom, REG_TOUCH_TRIGGER_INIT, Reinit.FromEngBox.TrigInit &= 0xFFFE);
        }
        else
        {
             Syncronize.Script_Split_Pwr = Reinit.FromEngBox.Power;
             Syncronize.Old.Script_Split_Pwr = Reinit.FromEngBox.Power;
             Syncronize.Engine.Script_Split_Pwr = Reinit.FromEngBox.Power;
             Touch[0].Script_Split_Pwr = Reinit.FromEngBox.Power;
             Reinit.FromEngBox.TrigInit &= 0xFFFE;
        }
	}
	
	if((Trig & 0x0002)==2 || (Trig & 0x0008)==8 || (TrigOther & 0x0008)==8)	
	{
        if(Touch[0].OnLine)
		{
            WriteModRegSec (Add_TouchRoom, REG_TOUCH_SP_ROOM, Reinit.FromEngBox.SetPoint);		
            WriteModRegSec (Add_TouchRoom, REG_TOUCH_SP_ROOM_F, Reinit.FromEngBox.SetPoint_F);		
            WriteModRegSec (Add_TouchRoom, REG_TOUCH_TRIGGER_INIT, Reinit.FromEngBox.TrigInit &= 0xFFFD);				
        }
        else
        {
             Syncronize.SetPoint = Reinit.FromEngBox.SetPoint;
             Syncronize.Old.SetPoint = Reinit.FromEngBox.SetPoint;
             Syncronize.Engine.SetPoint = Reinit.FromEngBox.SetPoint;
             Touch[0].SetPoint = Reinit.FromEngBox.SetPoint;
             
             Syncronize.Script_SetPoint_F = Reinit.FromEngBox.SetPoint_F;
             Syncronize.Old.Script_SetPoint_F = Reinit.FromEngBox.SetPoint_F;
             Syncronize.Engine.Script_SetPoint_F = Reinit.FromEngBox.SetPoint_F;
             Touch[0].Script_SetPoint_F = Reinit.FromEngBox.SetPoint_F;
             Reinit.FromEngBox.TrigInit &= 0xFFFD;
        }
	}
	
	if((Trig & 0x0004)==4 || (Trig & 0x0008)==8 || (TrigOther & 0x0008)==8)
	{
        if(Touch[0].OnLine)
		{
            WriteModRegSec (Add_TouchRoom, REG_TOUCH_FAN_SPEED, Reinit.FromEngBox.FanSpeed);		
            WriteModRegSec (Add_TouchRoom, REG_TOUCH_TRIGGER_INIT, Reinit.FromEngBox.TrigInit &= 0xFFF3);				
        }
        else
        {
             Syncronize.FanMode = Reinit.FromEngBox.FanSpeed;
             Syncronize.Old.FanMode = Reinit.FromEngBox.FanSpeed;
             Syncronize.Engine.FanMode = Reinit.FromEngBox.FanSpeed; 
             Touch[0].FanMode = Reinit.FromEngBox.FanSpeed; 
             
             Reinit.FromEngBox.TrigInit &= 0xFFF3;
        }
	}
	if((Trig & 0x0010)==10)
	{
		WriteModRegSec (Add_TouchRoom, REG_TOUCH_FUNCTION_MODE, Reinit.FromEngBox.DefMode);	
		//WriteModRegSec ((Add_TouchRoom +i), REG_TOUCH_FUNCTION_MODE, Reinit.FromEngBox.DefMode*Reinit.FromEngBox.Power);				
		WriteModRegSec (Add_TouchRoom, REG_TOUCH_TRIGGER_INIT, Reinit.FromEngBox.TrigInit &= 0xFFEF);	
	}		
#if(SplitMode==K_I_Am_UTA_and_InvFAN || SplitMode==K_I_Am_StandardUTA)
	if((TrigOther & 0x0008)==8 && Reinit.FromEngBox.UTA_Mode!=-1)
	{
        if(Touch[0].OnLine)
		{
            WriteModRegSec (Add_TouchRoom, REG_TOUCH_UTA_MODE, Reinit.FromEngBox.UTA_Mode);		
        }
        else
        {
             Syncronize.Uta_Mode = Reinit.FromEngBox.UTA_Mode;
             Syncronize.Old.Uta_Mode = Reinit.FromEngBox.UTA_Mode;
             Syncronize.Engine.Uta_Mode = Reinit.FromEngBox.UTA_Mode; 
             Touch[0].Uta_Mode = Reinit.FromEngBox.UTA_Mode; 
             Syncronize.Uta_Mode = Reinit.FromEngBox.UTA_Mode;
        }
	}
#endif    

#if(K_Heater_Abil==1)
	if((TrigOther & 0x0008)==8 && Reinit.FromEngBox.HeaterPwr!=-1)
	{
        if(Touch[0].OnLine)
		{
            WriteModRegSec (Add_TouchRoom, REG_TOUCH_HEATER_PWR, Reinit.FromEngBox.HeaterPwr);		
        }
        else
        {
             Syncronize.HeaterPwr = Reinit.FromEngBox.HeaterPwr;
             Syncronize.Old.HeaterPwr = Reinit.FromEngBox.HeaterPwr;
             Syncronize.Engine.HeaterPwr = Reinit.FromEngBox.HeaterPwr; 
             Touch[0].HeaterPwr = Reinit.FromEngBox.HeaterPwr; 
             Syncronize.HeaterPwr = Reinit.FromEngBox.HeaterPwr;
        }
	}
#endif    
    
    Me.OtherCabin.Trigger_Init = 0;
}

void Reinit_DataSplitToNetB(void)
{	
	int Trig;
	int ValFan;
    int TrigOther;
	
    Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    
    if(Reinit.FromEngBox.TrigInit < K_LimLo_TOUCH_TRIGGER_INIT || Reinit.FromEngBox.TrigInit > K_LimHi_TOUCH_TRIGGER_INIT)
        Reinit.FromEngBox.TrigInit = 0;	             			

#if(K_Disable_Trig_Init==1)
    if(Reinit.FromEngBox.TrigInit!=0)
        Reinit.FromEngBox.TrigInit=0;
#endif    
    
	Trig=Reinit.FromEngBox.TrigInit;

    TrigOther = Me.OtherCabin.Trigger_Init;	
    
    
/*
	if((Reinit.FromEngBox.FanSpeed==1) || (Reinit.FromEngBox.FanSpeed==2) || (Reinit.FromEngBox.FanSpeed==3))
	ValFan = 1;
	else if((Reinit.FromEngBox.FanSpeed==4) || (Reinit.FromEngBox.FanSpeed==5) || (Reinit.FromEngBox.FanSpeed==6))
	ValFan = 2;
	else if((Reinit.FromEngBox.FanSpeed==7) || (Reinit.FromEngBox.FanSpeed==8))
	ValFan = 3;
	else if((Reinit.FromEngBox.FanSpeed==9))
	ValFan = 4;
*/
    
    ValFan = Reinit.FromEngBox.FanSpeed;
    
	if((Trig & 0x0001)==1)
	{
        if(NetB[0].OnLine)
		{
			//NetB[0].OffLine = Send_WriteModRegSec(Add_NetBRoom, REG_NETB_IMP_VEL_FAN, ValFan*Reinit.FromEngBox.Power, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Il registro dei touch hanno un offset di 1
            WriteModRegSecNetB (Add_NetBRoom, REG_NETB_IMP_VEL_FAN, ValFan*Reinit.FromEngBox.Power); 
            Reinit.FromEngBox.TrigInit &= 0xFFFE;
        }
        else
        {
             Syncronize.Script_Split_Pwr = Reinit.FromEngBox.Power;
             Syncronize.Old.Script_Split_Pwr = Reinit.FromEngBox.Power;
             Syncronize.Engine.Script_Split_Pwr = Reinit.FromEngBox.Power;
             Touch[0].Script_Split_Pwr = Reinit.FromEngBox.Power;
             Reinit.FromEngBox.TrigInit &= 0xFFFE;
        }
	}

	if((Trig & 0x0002)==2 || (Trig & 0x0008)==8 || (TrigOther & 0x0008)==8)	
	{
        if(NetB[0].OnLine)
		{
            //NetB[0].OffLine = Send_WriteModRegSec(Add_NetBRoom, REG_NETB_TEMP_SETP_10, Reinit.FromEngBox.SetPoint/10, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Il registro dei touch hanno un offset di 1            
            WriteModRegSecNetB (Add_NetBRoom, REG_NETB_TEMP_SETP_10, Reinit.FromEngBox.SetPoint/10); 
            Reinit.FromEngBox.TrigInit &= 0xFFFD;				
    	}
        else
        {
             Syncronize.SetPoint = Reinit.FromEngBox.SetPoint;
             Syncronize.Old.SetPoint = Reinit.FromEngBox.SetPoint;
             Syncronize.Engine.SetPoint = Reinit.FromEngBox.SetPoint;
             Touch[0].SetPoint = Reinit.FromEngBox.SetPoint;
             Reinit.FromEngBox.TrigInit &= 0xFFFD;
        }
    }

	if((Trig & 0x0004)==4 || (Trig & 0x0008)==8 || (TrigOther & 0x0008)==8)
	{
		if(NetB[0].OnLine)
		{
			//NetB[0].OffLine = Send_WriteModRegSec(Add_NetBRoom, REG_NETB_IMP_VEL_FAN, ValFan*Reinit.FromEngBox.Power, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Il registro dei touch hanno un offset di 1
            WriteModRegSecNetB (Add_NetBRoom, REG_NETB_IMP_VEL_FAN, ValFan*Reinit.FromEngBox.Power); 
            Reinit.FromEngBox.TrigInit &= 0xFFF3;
		}
        else
        {
             Syncronize.FanMode = Reinit.FromEngBox.FanSpeed;
             Syncronize.Old.FanMode = Reinit.FromEngBox.FanSpeed;
             Syncronize.Engine.FanMode = Reinit.FromEngBox.FanSpeed; 
             Touch[0].FanMode = Reinit.FromEngBox.FanSpeed; 
             NetB[0].FanMode = Reinit.FromEngBox.FanSpeed; 
             
             Syncronize.Script_Split_Pwr = Reinit.FromEngBox.Power;
             Syncronize.Old.Script_Split_Pwr = Reinit.FromEngBox.Power;
             Syncronize.Engine.Script_Split_Pwr = Reinit.FromEngBox.Power;
             Touch[0].Script_Split_Pwr = Reinit.FromEngBox.Power;

             Reinit.FromEngBox.TrigInit &= 0xFFF3;
        }
	}
    /*    
	if((Trig & 0x0008)==8)	
	{
		if(NetB[0].OnLine)
		{
			//NetB[0].OffLine = Send_WriteModRegSec(Add_NetBRoom, REG_NETB_TEMP_SETP_10, Reinit.FromEngBox.SetPoint/10, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Il registro dei touch hanno un offset di 1
			//NetB[0].OffLine = Send_WriteModRegSec(Add_NetBRoom, REG_NETB_IMP_VEL_FAN, ValFan*Reinit.FromEngBox.Power, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Il registro dei touch hanno un offset di 1
            WriteModRegSecNetB (Add_NetBRoom, REG_NETB_TEMP_SETP_10, Reinit.FromEngBox.SetPoint/10); 
            WriteModRegSecNetB (Add_NetBRoom, REG_NETB_IMP_VEL_FAN, ValFan*Reinit.FromEngBox.Power); 
            Reinit.FromEngBox.TrigInit &= 0xFFF3;
		}
	}
    */
    Me.OtherCabin.Trigger_Init = 0;
}

void SyncronizeDataOld(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer)
{
	static int firstTimeFlag[30];
	
	
	if((*OldData)==0 && firstTimeFlag[pointer]==0)	//Inizializzo se sono a primo giro di programma
	{	
		(*OldData) = (*Data);
		firstTimeFlag[pointer] = 1;
	}

		if((*Data)!=(*OldData))								// Se è arrivato un aggiornamento dal Master
		{
			WriteModRegSec (Add_TouchRoom, RegAddr, (*Data));	// Scrivo valore arrivato dal Master
			(*OldData) = (*Data);								// Aggiorno OLD
		}
	
	// DEBUG Bridge						
	if(Touch[0].OnLine)														// Leggo registro "PowerLimit" dal touch
	{
		if(DiagnosticSplit.DataReady==1)
		{		
			//ReadModRegSec(Add_TouchRoom, RegAddr, &(*TouchData));

			(*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
			(*Data) = (*TouchData);
			(*OldData) = (*TouchData);
		}
	}
	else
	{
		(*TouchData) = (*Data);
	}
	
}

void SyncronizeDataNew(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer)
{
	static int firstTimeFlag[20];
	static int OldTouchData[20];
	static int ReadyData[20];
	static int CntOkData[20];
	static int TouchNewData[20];
	static int timer=0;
           int MaxDataPointer;
	
	
	if((*OldData)==0 && firstTimeFlag[pointer]==0)	//Inizializzo se sono a primo giro di programma
	{	
		(*OldData) = (*Data);
		firstTimeFlag[pointer] = 1;
	}

    if(Reinit.FromEngBox.TrigInit!=0 || Me.OtherCabin.Trigger_Init!=0)
    {
        ReadyData[pointer] = 0;	
        CntOkData[pointer]=0;

    }
    if(Touch[0].OnLine)
        MaxDataPointer = 10;//5;//10;
    else
        MaxDataPointer = 10;//15;
            
	if(ReadyData[pointer] == 0)
	{
		CntOkData[pointer]++;
		if(CntOkData[pointer]>=MaxDataPointer)
		{
			ReadyData[pointer] = 1;	
			CntOkData[pointer]=0;
		}
	}
	
	//ReadyData[pointer] = 1;
	if(DiagnosticSplit.DataProcess==0)
	{
        if(Touch[0].OnLine)														// Leggo registro "PowerLimit" dal touch
		{
			ReadModRegSec(Add_TouchRoom, RegAddr, &(*TouchData));
        }           
    }

	if(DiagnosticSplit.DataProcess==1)
	{
		
		if(Touch[0].OnLine)														// Leggo registro "PowerLimit" dal touch
		{

		//ReadModRegSec(Add_TouchRoom, RegAddr, &(*TouchData));

            if((*TouchData)!=OldTouchData[pointer])
                timer++;

            if((*TouchData)!=TouchNewData[pointer])
                timer=0;

            TouchNewData[pointer]=(*TouchData);		

            if(((*TouchData)!=OldTouchData[pointer]) && (timer>=5))
			{
				(*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
				(*Data) = (*TouchData);
				(*OldData) = (*TouchData); 
				OldTouchData[pointer] = (*TouchData);
				ReadyData[pointer] = 0;	
				CntOkData[pointer]=0;
                timer=0;                
			}
		}
		else
		{
            if(ReadyData[pointer] == 1)
                (*TouchData) = (*Data);
		}
		//(*TouchData) = (*Data);
		
		if((*Data)!=(*OldData)&&(ReadyData[pointer] == 1))								// Se è arrivato un aggiornamento dal Master
		{

			if(Touch[0].OnLine)														// Leggo registro "PowerLimit" dal touch
			{
				//if(DiagnosticSplit.DataReady==1)
					WriteModRegSec (Add_TouchRoom, RegAddr, (*Data));	// Scrivo valore arrivato dal Master
			}
 
			(*OldData) = (*Data);								// Aggiorno OLD
			OldTouchData[pointer] = (*Data);
			(*SendMasData) = (*Data);
		}			
	}
    else
    {
        //if(Touch[0].OnLine)														// Leggo registro "PowerLimit" dal touch
		{
            (*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
            (*Data) = (*TouchData);
            (*OldData) = (*TouchData); 
            OldTouchData[pointer] = (*TouchData);
            //ReadyData[pointer] = 0;	
            //CntOkData[pointer]=0;
		}
        //else
        //if(!Touch[0].OnLine)														// Leggo registro "PowerLimit" dal touch
        //{
        //    ReadyData[pointer] = 0;	
        //    CntOkData[pointer]=0;          
        //}
#if(k_Abil_Panel_Shared==1)        
        //else
        //{
            ReadyData[pointer] = 1;	
			CntOkData[pointer]=0;
        //}
#endif
    }
}

void SyncronizeDataBridge(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer)
{
	static int firstTimeFlag[30];
	
	
	if((*OldData)==0 && firstTimeFlag[pointer]==0)	//Inizializzo se sono a primo giro di programma
	{	
		(*OldData) = (*Data);
		firstTimeFlag[pointer] = 1;
	}

		if((*Data)!=(*OldData))								// Se è arrivato un aggiornamento dal Master
		{
			WriteModRegSec (Add_TouchRoom, RegAddr, (*Data));	// Scrivo valore arrivato dal Master
			(*OldData) = (*Data);								// Aggiorno OLD
		}
	
	// DEBUG Bridge						
	if(Touch[0].OnLine)														// Leggo registro "PowerLimit" dal touch
	{
			//ReadModRegSec(Add_TouchRoom, RegAddr, &(*TouchData));

			(*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
			(*Data) = (*TouchData);
			(*OldData) = (*TouchData);
	}
	else
	{
		(*TouchData) = (*Data);
	}
	
}

void SyncronizeDataNew_NetB(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer)
{
	static int firstTimeFlag[1];
	static int OldTouchData[1];
	static int ReadyData[1];
	static int CntOkData[1];
	static int TouchNewData[1];
	static int timer=0;
	int temp;
	
	
	if((*OldData)==0 && firstTimeFlag[pointer]==0)	//Inizializzo se sono a primo giro di programma
	{	
		(*OldData) = (*Data);
		firstTimeFlag[pointer] = 1;
	}
	
	if(ReadyData[pointer] == 0)
	{
		CntOkData[pointer]++;
		if(CntOkData[pointer]>=10)
		{
			ReadyData[pointer] = 1;	
			CntOkData[pointer]=0;
		}
	}
	
	
	if(NetB[0].OnLine)														// Leggo registro "PowerLimit" dal touch
	{

		ReadModRegSecNetB (Add_NetBRoom, RegAddr, &(temp));
		(*TouchData) = temp;						// Leggo dal Touch
		
		if((*TouchData)!=OldTouchData[pointer])
			timer++;
				
		if((*TouchData)!=TouchNewData[pointer])
			timer=0;
		
		TouchNewData[pointer]=(*TouchData);		
		
		if(((*TouchData)!=OldTouchData[pointer]) && (timer>=20))
		{
			(*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
			(*Data) = (*TouchData);
			(*OldData) = (*TouchData); 
			OldTouchData[pointer] = (*TouchData);
			ReadyData[pointer] = 0;	
			CntOkData[pointer]=0;
			timer=0;
		}

		Mod_LastRxSec.Valid_Data = 0;
	}
	
	if((*Data)!=(*OldData)&&(ReadyData[pointer] == 1))								// Se è arrivato un aggiornamento dal Master
	{
		if(NetB[0].OnLine)
		{
			//NetB[0].OffLine = Send_WriteModRegSec(Add_NetBRoom, RegAddr, (*Data), MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Il registro dei touch hanno un offset di 1
			WriteModRegSecNetB (Add_NetBRoom, RegAddr, (*Data)); 
		//		Inverter->OffLine = Send_WriteModRegSec(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_RUN_COMMAND, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Lo mando in RUN
			
		}		
		
		(*OldData) = (*Data);								// Aggiorno OLD
		OldTouchData[pointer] = (*Data);
		(*SendMasData) = (*Data);
	}				
	
}


void SyncronizeDataSetP_NetB(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer)
{
	static int firstTimeFlag[1];
	static int OldTouchData[1];
	static int ReadyData[1];
	static int CntOkData[1];
	static int TouchNewData[1];
	static int timer=0;
           int MaxDataPointer;
           int temp;
	
	
	
	if((*OldData)==0 && firstTimeFlag[pointer]==0)	//Inizializzo se sono a primo giro di programma
	{	
		(*OldData) = (*Data);
		firstTimeFlag[pointer] = 1;
	}
    
    if(Reinit.FromEngBox.TrigInit!=0 || Me.OtherCabin.Trigger_Init!=0)
    {
        ReadyData[pointer] = 0;	
        CntOkData[pointer]=0;
    }
	
    if(NetB[0].OnLine)
        MaxDataPointer = 20;//10;
    else
        MaxDataPointer = 10;//15;

    if(ReadyData[pointer] == 0)
	{
		CntOkData[pointer]++;
		if(CntOkData[pointer]>=MaxDataPointer)
		{
			ReadyData[pointer] = 1;	
			CntOkData[pointer]=0;
		}
	}

	if(DiagnosticSplit.DataProcess==0)
	{
        //if(Send_WaitModRxSec(Add_NetBRoom, ReadModCmd, RegAddr, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
        //    NetB[0].OffLine = 1;
        if(NetB[0].OnLine)														// Leggo registro "PowerLimit" dal touch
		{
            ReadModRegSecNetB (Add_NetBRoom, RegAddr, &(temp));
            (*TouchData) = temp/10;                         // Perdo il decimale
            (*TouchData) = (*TouchData)*100;				// Converto in 100° di grado
        }           
    }
	if(DiagnosticSplit.DataProcess==1)
	{
        if(NetB[0].OnLine)														// Leggo registro "PowerLimit" dal touch
        {
            //if(Send_WaitModRxSec(Add_NetBRoom, ReadModCmd, RegAddr, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
            //    NetB[0].OffLine = 1;
            ReadModRegSecNetB (Add_NetBRoom, RegAddr, &(temp));

            (*TouchData) = temp/10;                         // Perdo il decimale
            (*TouchData) = (*TouchData)*100;				// Converto in 100° di grado

            if((*TouchData)!=OldTouchData[pointer])
                timer++;

            if((*TouchData)!=TouchNewData[pointer])
                timer=0;

            TouchNewData[pointer]=(*TouchData);		

            if(((*TouchData)!=OldTouchData[pointer]) && (timer>=20))
            {
                (*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
                (*Data) = (*TouchData);
                (*OldData) = (*TouchData); 
                OldTouchData[pointer] = (*TouchData);
                ReadyData[pointer] = 0;	
                CntOkData[pointer]=0;
                timer=0;
            }

            Mod_LastRxSec.Valid_Data = 0;
        }
		else
		{
            if(ReadyData[pointer] == 1)
                (*TouchData) = (*Data);
		}
        
        if((*Data)!=(*OldData)&&(ReadyData[pointer] == 1))								// Se è arrivato un aggiornamento dal Master
        {
            if(NetB[0].OnLine)
            {
                //NetB[0].OffLine = Send_WriteModRegSec(Add_NetBRoom, RegAddr, (*Data)/10, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Il registro dei touch hanno un offset di 1
                WriteModRegSecNetB (Add_NetBRoom, RegAddr, (*Data)/10);     
            //		Inverter->OffLine = Send_WriteModRegSec(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_RUN_COMMAND, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Lo mando in RUN

            }		

            (*OldData) = (*Data);								// Aggiorno OLD
            OldTouchData[pointer] = (*Data);
            (*SendMasData) = (*Data);
        }				
	}
    else
    {
        //if(Touch[0].OnLine)														// Leggo registro "PowerLimit" dal touch
		{
            (*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
            (*Data) = (*TouchData);
            (*OldData) = (*TouchData); 
            OldTouchData[pointer] = (*TouchData);
            //ReadyData[pointer] = 0;	
            //CntOkData[pointer]=0;
		}
        //else
        //if(NetB[0].OffLine)														// Leggo registro "PowerLimit" dal touch
        //{
            ReadyData[pointer] = 0;	
            CntOkData[pointer]=0;          
        //}
#if(k_Abil_Panel_Shared==1)        
        //else
        //{
            ReadyData[pointer] = 1;	
			CntOkData[pointer]=0;
        //}
#endif
    }           
}

void SyncronizeDataOnOff_NetB(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer)
{
	static int firstTimeFlag[1];
	static int OldTouchData[1];
	static int ReadyData[1];
	static int CntOkData[1];
	static int temp=0;
	static int TouchNewData[1];
	static int timer=0;
    int MaxDataPointer;
	
	
	if((*OldData)==0 && firstTimeFlag[pointer]==0)	//Inizializzo se sono a primo giro di programma
	{	
		(*OldData) = (*Data);
		firstTimeFlag[pointer] = 1;
	}

    if(Reinit.FromEngBox.TrigInit!=0 || Me.OtherCabin.Trigger_Init!=0)
    {
        ReadyData[pointer] = 0;	
        CntOkData[pointer]=0;
    }	

    if(NetB[0].OnLine)
        MaxDataPointer = 20;//5;//10;
    else
        MaxDataPointer = 10;//15;
    
	if(ReadyData[pointer] == 0)
	{
		CntOkData[pointer]++;
		if(CntOkData[pointer]>=MaxDataPointer)
		{
			ReadyData[pointer] = 1;	
			CntOkData[pointer]=0;
		}
	}

	if(DiagnosticSplit.DataProcess==0)
	{       
        //if(Send_WaitModRxSec(Add_NetBRoom, ReadModCmd, RegAddr, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
        //    NetB[0].OffLine = 1;
        //else
        //    temp = ExtracRegSec(1);				// Leggo dal Touch
        
        if(NetB[0].OnLine)
        {
            ReadModRegSecNetB (Add_NetBRoom, RegAddr, &(temp));
        }
        
    }	
	if(DiagnosticSplit.DataProcess==1)
	{
        if(NetB[0].OnLine)														// Leggo registro "PowerLimit" dal touch
        {
            ReadModRegSecNetB (Add_NetBRoom, RegAddr, &(temp));
            
            if(temp==0)
                (*TouchData) = 0;
            else
                (*TouchData) = 1;

                if((*TouchData)!=OldTouchData[pointer])
                timer++;

            if((*TouchData)!=TouchNewData[pointer])
                timer=0;

            TouchNewData[pointer]=(*TouchData);		

            if(((*TouchData)!=OldTouchData[pointer]) && (timer>=20))
            {
                (*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
                (*Data) = (*TouchData);
                (*OldData) = (*TouchData); 
                OldTouchData[pointer] = (*TouchData);
                ReadyData[pointer] = 0;	
                CntOkData[pointer]=0;
                timer=0;
            }

            Mod_LastRxSec.Valid_Data = 0;
        }
		else
		{
            if(ReadyData[pointer] == 1)
            {
                (*TouchData) = (*Data);
                (*SendMasData) = (*Data);
                (*OldData) = (*Data);
                OldTouchData[pointer] = (*Data);
            }                
		}
        if((*Data)!=(*OldData)&&(ReadyData[pointer] == 1))								// Se è arrivato un aggiornamento dal Master
        {
            /*
            if(!NetB[0].OffLine)
            {
                NetB[0].OffLine = Send_WriteModRegSec(Add_NetBRoom, RegAddr, (*Data), MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Il registro dei touch hanno un offset di 1
            //		Inverter->OffLine = Send_WriteModRegSec(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_RUN_COMMAND, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Lo mando in RUN

            }		
            */
            (*OldData) = (*Data);								// Aggiorno OLD
            OldTouchData[pointer] = (*Data);
            (*SendMasData) = (*Data);

            ApplyVel_NetB(&Syncronize.Script_Split_Pwr, &Syncronize.FanMode, REG_NETB_IMP_VEL_FAN);
        }				
    }
    else
    {
        if(NetB[0].OnLine)														// Leggo registro "PowerLimit" dal touch
		{
            (*TouchData) = 1*(temp>0);            
            (*SendMasData) = 1*(temp>0);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
            (*Data) = 1*(temp>0);
            (*OldData) = 1*(temp>0); 
            OldTouchData[pointer] = 1*(temp>0);
            //ReadyData[pointer] = 0;	
            //CntOkData[pointer]=0;
		}
        else
        {
            (*TouchData) = (*Data);
            (*SendMasData) = (*Data);
            (*OldData) = (*Data);
            OldTouchData[pointer] = (*Data);                
        }
        
        //if(NetB[0].OffLine)														// Leggo registro "PowerLimit" dal touch
        //{
        //    ReadyData[pointer] = 0;	
        //    CntOkData[pointer]=0;          
        //}
#if(k_Abil_Panel_Shared==1)        
        //else
        //{
            ReadyData[pointer] = 1;	
			CntOkData[pointer]=0;
        //}
#endif
    }	
}

void SyncronizeDataVel_NetB(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer)
{
	static int firstTimeFlag[1];
	static int OldTouchData[1];
	static int ReadyData[1];
	static int CntOkData[1];
	static int temp=0;
	static int OkData=4;
	static int TouchNewData[1];
	static int timer=0;	
           int MaxDataPointer;
	
	if((*OldData)==0 && firstTimeFlag[pointer]==0)	//Inizializzo se sono a primo giro di programma
	{	
		(*OldData) = (*Data);
		//OkData = (*Data);
		firstTimeFlag[pointer] = 1;
	}

    if(Reinit.FromEngBox.TrigInit!=0 || Me.OtherCabin.Trigger_Init!=0)
    {
        ReadyData[pointer] = 0;	
        CntOkData[pointer]=0;
    }

    if(NetB[0].OnLine)
        MaxDataPointer = 20;//5;//10;
    else
        MaxDataPointer = 30;//15;
	
	if(ReadyData[pointer] == 0)
	{
		CntOkData[pointer]++;
		if(CntOkData[pointer]>=MaxDataPointer)  
		{
			ReadyData[pointer] = 1;	
			CntOkData[pointer]=0;
		}
	}

	if(DiagnosticSplit.DataProcess==0)
	{		
        if(NetB[0].OnLine)														// Leggo registro "PowerLimit" dal touch
        {        
            ReadModRegSecNetB (Add_NetBRoom, RegAddr, &(temp));
        
            if(temp==0)
                temp = Syncronize.FanMode;
        }
    }
	if(DiagnosticSplit.DataProcess==1)
	{		
        if(NetB[0].OnLine)														// Leggo registro "PowerLimit" dal touch
        {
            ReadModRegSecNetB (Add_NetBRoom, RegAddr, &(temp));

            if(temp!=0)
                OkData = temp;

            if(OkData!=OldTouchData[pointer])
                timer++;

            if(OkData!=TouchNewData[pointer])
                timer=0;

            TouchNewData[pointer]=OkData;		

            if((OkData!=OldTouchData[pointer]) && (timer>=30))         
            {
                (*SendMasData) = OkData;				// e aggiorno "Me" con i valore letto (sarà letto dal Master
                (*Data) = OkData;
                (*OldData) = OkData; 
                OldTouchData[pointer] = OkData;
                ReadyData[pointer] = 0;	
                CntOkData[pointer]=0;
                timer=0;
            }

            Mod_LastRxSec.Valid_Data = 0;
        }
		else
		{
            if(ReadyData[pointer] == 1)
            {
                (*TouchData) = (*Data);
                (*SendMasData) = (*Data);
                (*OldData) = (*Data);
                OldTouchData[pointer] = (*Data);
            }
		}

        if((*Data)!=(*OldData)&&(ReadyData[pointer] == 1)&&(OkData==OldTouchData[pointer]))								// Se è arrivato un aggiornamento dal Master
        {
            /*
            if(!NetB[0].OffLine)
            {
                NetB[0].OffLine = Send_WriteModRegSec(Add_NetBRoom, RegAddr, (*Data), MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Il registro dei touch hanno un offset di 1
            //		Inverter->OffLine = Send_WriteModRegSec(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_RUN_COMMAND, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Lo mando in RUN

            }		
            */
            (*OldData) = (*Data);								// Aggiorno OLD
            OldTouchData[pointer] = (*Data);
            (*SendMasData) = (*Data);
            if(temp==0)
                OkData = (*Data);

            ApplyVel_NetB(&Syncronize.Script_Split_Pwr, &Syncronize.FanMode, REG_NETB_IMP_VEL_FAN);
        }				
    }
    else
    {		
        if(NetB[0].OnLine)														// Leggo registro "PowerLimit" dal touch
        {
            (*SendMasData) = temp;				// e aggiorno "Me" con i valore letto (sarà letto dal Master
            (*Data) = temp;
            (*OldData) = temp; 
            OldTouchData[pointer] = temp;
            /*
            (*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
            (*Data) = (*TouchData);
            (*OldData) = (*TouchData); 
            OldTouchData[pointer] = (*TouchData);
            */
            //ReadyData[pointer] = 0;	
            //CntOkData[pointer]=0;
        }
        else
        {
            (*TouchData) = (*Data);
            (*SendMasData) = (*Data);
            (*OldData) = (*Data);
            OldTouchData[pointer] = (*Data);                
        }

        //if(NetB[0].OffLine)														// Leggo registro "PowerLimit" dal touch
        //{
        //    ReadyData[pointer] = 0;	
        //    CntOkData[pointer]=0;          
        //}
#if(k_Abil_Panel_Shared==1)        
        //else
        //{
            ReadyData[pointer] = 1;	
			CntOkData[pointer]=0;
        //}
#endif
    }
}

void ApplyVel_NetB(volatile int * OnOff, volatile int * Vel, int RegAddr)
{
	
	static int velocita;
	//static int OldVelocita;
	
	velocita = (*Vel)*(*OnOff);
	
	//if(velocita!=OldVelocita)
	//{
		if(NetB[0].OnLine)
		{
            WriteModRegSecNetB (Add_NetBRoom, RegAddr, velocita);     
			//OldVelocita=velocita;   
		}
		
	//}	
}




//-----------------------------------------------------------------
// Funzioni solo per SyxthSense
//-----------------------------------------------------------------

void Reinit_DataSplitToSyxtS(void)
{
	int Trig;
    int OnOffReverse;
    int TrigOther;
	
    Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    
    if(Reinit.FromEngBox.TrigInit < K_LimLo_TOUCH_TRIGGER_INIT || Reinit.FromEngBox.TrigInit > K_LimHi_TOUCH_TRIGGER_INIT)
        Reinit.FromEngBox.TrigInit = 0;	             			

#if(K_Disable_Trig_Init==1)
    if(Reinit.FromEngBox.TrigInit!=0)
        Reinit.FromEngBox.TrigInit=0;
#endif             			

	Trig=Reinit.FromEngBox.TrigInit;

    TrigOther = Me.OtherCabin.Trigger_Init;	
    
    OnOffReverse = ~Reinit.FromEngBox.Power & 0x0001;
    
	if((Trig & 0x0001)==1 || (Trig & 0x0008)==8 || (TrigOther & 0x0008)==8)
	{
        if(SyxtS[0].OnLine)
		{
            WriteCoilModRegSecSyxtS (Add_SyxtSRoom, COIL_SYXTS_OFF_MODE_OVRR, OnOffReverse); 
            Reinit.FromEngBox.TrigInit &= 0xFFFE;
        }
        else
        {
             Syncronize.Script_Split_Pwr = Reinit.FromEngBox.Power;
             Syncronize.Old.Script_Split_Pwr = Reinit.FromEngBox.Power;
             Syncronize.Engine.Script_Split_Pwr = Reinit.FromEngBox.Power;
             Touch[0].Script_Split_Pwr = Reinit.FromEngBox.Power;
             Reinit.FromEngBox.TrigInit &= 0xFFFE;
        }
	}

	if((Trig & 0x0002)==2 || (Trig & 0x0008)==8 || (TrigOther & 0x0008)==8)	
	{
        if(SyxtS[0].OnLine)
		{
            if((SyxtS[0].FwVer_Upper_423==1) && (EngineBox.Touch_Nation==1))
                WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_NOM_SETP, Reinit.FromEngBox.SetPoint_F/10); 
            else
                WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_NOM_SETP, Reinit.FromEngBox.SetPoint/10); 
            Reinit.FromEngBox.TrigInit &= 0xFFFD;				
    	}
        else
        {
             Syncronize.SetPoint = Reinit.FromEngBox.SetPoint;
             Syncronize.Old.SetPoint = Reinit.FromEngBox.SetPoint;
             Syncronize.Engine.SetPoint = Reinit.FromEngBox.SetPoint;
             Touch[0].SetPoint = Reinit.FromEngBox.SetPoint;
             if((SyxtS[0].FwVer_Upper_423==1) && (EngineBox.Touch_Nation==1))
             {
                Syncronize.Script_SetPoint_F = Reinit.FromEngBox.SetPoint_F;
                Syncronize.Old.Script_SetPoint_F = Reinit.FromEngBox.SetPoint_F;
                Syncronize.Engine.Script_SetPoint_F = Reinit.FromEngBox.SetPoint_F;
                Touch[0].Script_SetPoint_F = Reinit.FromEngBox.SetPoint_F;
             }
             Reinit.FromEngBox.TrigInit &= 0xFFFD;
        }
    }

	if((Trig & 0x0004)==4 || (Trig & 0x0008)==8 || (TrigOther & 0x0008)==8)
	{
		if(SyxtS[0].OnLine)
		{
            WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_FAN_SPEED_OVRR_ADV, Reinit.FromEngBox.FanSpeed); 
            Reinit.FromEngBox.TrigInit &= 0xFFF3;
		}
        else
        {
             Syncronize.FanMode = Reinit.FromEngBox.FanSpeed;
             Syncronize.Old.FanMode = Reinit.FromEngBox.FanSpeed;
             Syncronize.Engine.FanMode = Reinit.FromEngBox.FanSpeed; 
             Touch[0].FanMode = Reinit.FromEngBox.FanSpeed; 
             SyxtS[0].FanMode = Reinit.FromEngBox.FanSpeed; 
             Reinit.FromEngBox.TrigInit &= 0xFFF3;
        }
	}
    Me.OtherCabin.Trigger_Init = 0;    
}



void ReadModRegSecSyxtS (char Address, int REG, volatile int * StoreReg)
{
	char Result;
	if(SyxtS[Address-Add_SyxtSRoom].OnLine)
	{
		Result = Send_WaitModRxSec(Address, ReadModCmd, REG, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusOneBased);
		if(Result == 0)	// Il registro dei touch hanno un offset di 1
		{
			(*StoreReg) = Mod_LastRxSec.Data;
			Mod_LastRxSec.Valid_Data = 0;
		}
		else
		{
			SyxtS[Address-Add_SyxtSRoom].OffLine = 1;
			SyxtS[Address-Add_SyxtSRoom].OnLine = 0;            
		}
	}
}

void ReadModInputSecSyxtS (char Address, int REG, volatile int * StoreReg)
{
	char Result;
	if(SyxtS[Address-Add_SyxtSRoom].OnLine)
	{
		Result = Send_WaitModRxSec(Address, ReadModInput, REG, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusOneBased);
		if(Result == 0)	// Il registro dei touch hanno un offset di 1
		{
			(*StoreReg) = Mod_LastRxSec.Data;
			Mod_LastRxSec.Valid_Data = 0;
		}
		else
		{
			SyxtS[Address-Add_SyxtSRoom].OffLine = 1;
			SyxtS[Address-Add_SyxtSRoom].OnLine = 0;            
		}
	}
}



unsigned char ReadModListRegSecSyxtS (unsigned char DataIndexSec, int REG, int RegNum)
{
	char SyxtSAddres;
	SyxtSAddres = (DataIndexSec+Add_SyxtSRoom);
	if(SyxtS[DataIndexSec].OnLine)
	{
		if(Send_WaitModRxSec(SyxtSAddres, ReadModCmd, REG, RegNum, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusOneBased))	// Il registro dei touch hanno un offset di 1
		{
			SyxtS[DataIndexSec].OffLine = 1;
			SyxtS[DataIndexSec].OnLine = 0;                        
			Mod_LastRxSec.Valid_Data = 0;
			return 1;
		}
		else
		{
			Mod_LastRxSec.Valid_Data = 0;
			return 0;
		}
	}
	return 1;
}


void WriteModRegSecSyxtS (unsigned char Address, int REG, int SendData)
{
	unsigned char SyxtSAddres;
	char Result;
	SyxtSAddres = Address-Add_SyxtSRoom;

	if(SyxtS[SyxtSAddres].OnLine)
	{
		Result = Send_WriteModRegSec(Address, REG, SendData, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusOneBased);	// Il registro dei touch hanno un offset di 1
		if(Result == 1)
		{
			SyxtS[SyxtSAddres].OffLine = 1;
			SyxtS[SyxtSAddres].OnLine = 0;
		}
	}
}

void WriteCoilModRegSecSyxtS (unsigned char Address, int REG, int SendData)
{
	unsigned char SyxtSAddres;
	char Result;
	SyxtSAddres = Address-Add_SyxtSRoom;

	if(SyxtS[SyxtSAddres].OnLine)
	{
		Result = Send_WriteModRegSec(Address, REG, SendData, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModForceSingleCoil, K_ModBusOneBased);	// Il registro dei touch hanno un offset di 1
		if(Result == 1)
		{
			SyxtS[SyxtSAddres].OffLine = 1;
			SyxtS[SyxtSAddres].OnLine = 0;
		}
	}
}

void WriteModListRegSecSyxtS (unsigned char Address, int REG, int SendData)
{
	unsigned char SyxtSAddres;
	char Result;
	SyxtSAddres = Address-Add_SyxtSRoom;	// Il registro dei touch hanno un offset di 1

	if(SyxtS[SyxtSAddres].OnLine)
	{
		Result = Send_WriteModRegSec(Address, REG, SendData, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModListCmd, K_ModBusOneBased);	// Il registro dei touch hanno un offset di 1
		if(Result == 1)
		{
			SyxtS[SyxtSAddres].OffLine = 1;
			SyxtS[SyxtSAddres].OnLine = 0;
		}
	}     
}

void ReadCoilModRegSecSyxtS (char Address, int REG, int * StoreReg)
{
	char Result;
	if(SyxtS[Address-Add_SyxtSRoom].OnLine)
	{
		Result = Send_WaitModRxSec(Address, ReadModCoil, REG, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusOneBased);
		if(Result == 0)	// Il registro dei touch hanno un offset di 1
		{
			(*StoreReg) = Mod_LastRxSec.Data;
			Mod_LastRxSec.Valid_Data = 0;
		}
		else
		{
			SyxtS[Address-Add_SyxtSRoom].OffLine = 1;
			SyxtS[Address-Add_SyxtSRoom].OnLine = 0;            
		}
	}
}

void ReadInputStModRegSecSyxtS (char Address, int REG, int * StoreReg)
{
	char Result;
	if(SyxtS[Address-Add_SyxtSRoom].OnLine)
	{
		Result = Send_WaitModRxSec(Address, ReadModInputStatus, REG, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusOneBased);
		if(Result == 0)	// Il registro dei touch hanno un offset di 1
		{
			(*StoreReg) = Mod_LastRxSec.Data;
			Mod_LastRxSec.Valid_Data = 0;
		}
		else
		{
			SyxtS[Address-Add_SyxtSRoom].OffLine = 1;
			SyxtS[Address-Add_SyxtSRoom].OnLine = 0;            
		}
	}
}

void RefreshAllSyxtSInPut(void)
{
    unsigned int i = 0;    

    Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    
	for(i=Add_SyxtSRoom; i < (Add_SyxtSRoom+MaxSyxtS); i++)
		RefreshSyxtSInPut(i);
}



void RefreshSyxtSInPut(char SyxtSAddres)
{
	unsigned char DataIndexSec;
    
    DataIndexSec = SyxtSAddres-Add_SyxtSRoom;
    
	SyncronizeDataOnOff_SyxtS(&Syncronize.Script_Split_Pwr, &Syncronize.Old.Script_Split_Pwr, &Touch[0].Script_Split_Pwr, &Syncronize.Engine.Script_Split_Pwr, COIL_SYXTS_OFF_MODE_OVRR, 0);				 	
    if((SyxtS[0].FwVer_Upper_423==1) && (EngineBox.Touch_Nation==1))
    	SyncronizeDataSetP_SyxtS(&Syncronize.Script_SetPoint_F, &Syncronize.Old.Script_SetPoint_F, &Touch[0].Script_SetPoint_F, &Syncronize.Engine.Script_SetPoint_F, REGH_SYXTS_NOM_SETP, REGI_SYXTS_CURC_SETP, 0);
    else
    	SyncronizeDataSetP_SyxtS(&Syncronize.SetPoint, &Syncronize.Old.SetPoint, &Touch[0].SetPoint, &Syncronize.Engine.SetPoint, REGH_SYXTS_NOM_SETP, REGI_SYXTS_CURC_SETP, 0);				

	SyncronizeDataVel_SyxtS(&Syncronize.FanMode, &Syncronize.Old.FanMode, &SyxtS[0].FanMode, &Syncronize.Engine.FanMode, REGH_SYXTS_FAN_SPEED_OVRR_ADV, 0);							

#if(K_Heater_Abil==1)
    SyncronizeDataHeater_SyxtS(&Syncronize.HeaterPwr, &Syncronize.Old.HeaterPwr, &SyxtS[0].HeaterPwr, &Syncronize.Engine.HeaterPwr, COIL_SYXTS_AC_STATUS, REGH_SYXTS_OVERR_AC, 0);
#endif    
    

	ReadModInputSecSyxtS(SyxtSAddres, REGI_SYXTS_CURC_SETP, &(SyxtS[DataIndexSec].SetPoint));
    if(SyxtS[DataIndexSec].OnLine)
    {
        SyxtS[DataIndexSec].SetPoint *= 10;															// Il setpoint che arriva dal pannello e' in decimi di grado
        if((SyxtS[0].FwVer_Upper_423==1) && (EngineBox.Touch_Nation==1))
        {
            SyxtS[DataIndexSec].SetPoint = (int)(((float)SyxtS[DataIndexSec].SetPoint - 3200)*5.0/9.0);
        }        
    }

	ReadModRegSecSyxtS (SyxtSAddres, REGH_SYXTS_FAN_SPEED_OVRR_ADV, &(SyxtS[DataIndexSec].FanMode));
	ReadModInputSecSyxtS (SyxtSAddres, REGI_SYXTS_BI_TEMP, &(SyxtS[DataIndexSec].Temperature));
    if(SyxtS[DataIndexSec].OnLine)
    {  
        SyxtS[DataIndexSec].Temperature *= 10;															// La temperatura che arriva dal pannello e' in decimi di grado    
        if((SyxtS[0].FwVer_Upper_423==1) && (EngineBox.Touch_Nation==1))
        {
            SyxtS[DataIndexSec].Temperature = (int)(((float)SyxtS[DataIndexSec].Temperature - 3200)*5.0/9.0);
        }
    }
}


void SyxtS2Touch(void)
{
    if(SyxtS[0].FanMode!=0)
        Touch[0].FanMode = SyxtS[0].FanMode;

    if(SyxtS[0].OnLine)
    {
        Me.PanelTempAmb = SyxtS[0].Temperature;        
        Touch[0].SetPoint = SyxtS[0].SetPoint;

#if(K_Heater_Abil==1)
        Touch[0].HeaterPwr = SyxtS[0].HeaterPwr;
#endif

        if((SyxtS[0].FwVer_Upper_423==1) && (EngineBox.Touch_Nation==1))
        {
            Syncronize.SetPoint = Touch[0].SetPoint;
            Syncronize.Old.SetPoint = Touch[0].SetPoint;
            Syncronize.Engine.SetPoint = Touch[0].SetPoint;            
        }    
        //Touch[0].FunctionMode = Me.DefMode * (Touch[0].FanMode!=0) * (EngineBox.SystemDisable==0);
    }
    
}


void Search_SyxtS(void)
{
	char Result;    
	Result = 0;

    int temp=0;

    Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud    

    Result = Send_WaitModRxSec(Add_SyxtSRoom, ReadModInput, REGI_SYXTS_FW_REL, 1, MaxModChkWaitSec, MaxModChkRetriesSec, K_ModBusOneBased);

    if(Result==1)
    {
        Change_ProtocolBusSlave(K_ModBus_Baud9600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud, 5=ModBus@9600Baud

        Result = Send_WaitModRxSec(1, ReadModInput, REGI_SYXTS_FW_REL, 1, MaxModChkWaitSec, MaxModChkRetriesSec, K_ModBusOneBased);
        if(Result==0)     
        {
            SyxtS[0].OffLine = Send_WriteModRegSec(1, REGH_SYXTS_MODBUS_ADDRESS, Add_SyxtSRoom, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusOneBased);	// Il registro dei touch hanno un offset di 1
            SyxtS[0].OffLine = Send_WriteModRegSec(1, REGH_SYXTS_MODBUS_BAUD, 3, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusOneBased);	// Il registro dei touch hanno un offset di 1
            SyxtS[0].OffLine = Send_WriteModRegSec(1, REGH_SYXTS_MODBUS_PARITY, 0, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusOneBased);	// Il registro dei touch hanno un offset di 1
            SyxtS[0].OffLine = Send_WriteModRegSec(1, REGH_SYXTS_MODBUS_STOPBIT, 0, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusOneBased);	// Il registro dei touch hanno un offset di 1
            SyxtS[0].OffLine = Send_WriteModRegSec(1, REGH_SYXTS_E2_UPDATE, 1, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusOneBased);	// Il registro dei touch hanno un offset di 1
            DelaymSec(100);
            SyxtS[0].OffLine = Send_WriteModRegSec(1, REGH_SYXTS_FORCE_RESET, 1, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusOneBased);	// Il registro dei touch hanno un offset di 1
            DelaymSec(6000);
        }
        Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud, 5=ModBus@9600Baud

        Result = Send_WaitModRxSec(Add_SyxtSRoom, ReadModInput, REGI_SYXTS_FW_REL, 1, MaxModChkWaitSec, MaxModChkRetriesSec, K_ModBusOneBased);
    }    
    
    SyxtS[0].Enable = (Result == 0) ;
    SyxtS[0].OffLine = (Result != 0);
    SyxtS[0].OnLine = (SyxtS[0].Enable == 1) && (SyxtS[0].OffLine == 0);           
    Mod_LastRxSec.Valid_Data = 0;

    if(SyxtS[0].OnLine) 
    {
#if(k_Abil_Panel_Shared==1)
        ReadModInputSecSyxtS (Add_SyxtSRoom, REGI_SYXTS_FW_REL, &(temp));
        if(temp>=423)
            SyxtS[0].FwVer_Upper_423 = 1;
#endif        
        SyxtS[0].CntErrTouch = 0;
        SyxtS[0].PermanentOffline = 0;
        Me.ModBusDeviceDetected=K_ModDeviceSyxtS;
        InitSyxtS();
    }    
}
            

void InitSyxtS(void)
{
    int OnOffReverse;    
    int Result;    
    int temp;

    Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud, 5=ModBus@9600Baud
#if(k_Abil_Panel_Shared==1)
    ReadModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_ENABLE_MODE_ICON, &(Result));
    //if(Result != 4)
    
    ReadModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_MIN_SETP, &(temp));
    if(((temp < K_LimLo_SPLIT_SP_F/10) && (EngineBox.Touch_Nation==1)) || ((temp > K_LimHi_SPLIT_SP/10) && (EngineBox.Touch_Nation==2)) || (Result != 4))
#endif
    {
        ReadModInputSecSyxtS (Add_SyxtSRoom, REGI_SYXTS_FW_REL, &(temp));
        if(temp>=423)
            SyxtS[0].FwVer_Upper_423 = 1;

        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_SETP_UNIT, 0);       
#if(k_Abil_Panel_Shared==1)
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_SENSOR3_SOURCE, 0);
#else
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_SENSOR3_SOURCE, 1);     
#endif

        if((SyxtS[0].FwVer_Upper_423==1) && (EngineBox.Touch_Nation==1))
        {
            WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_MIN_SETP, K_LimLo_SPLIT_SP_F/10);     
            WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_MAX_SETP, K_LimHi_SPLIT_SP_F/10);                 
        }
        else
        {
            WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_MIN_SETP, K_LimLo_SPLIT_SP/10);     
            WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_MAX_SETP, K_LimHi_SPLIT_SP/10);     
        }

        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_COLOUR_RING_MODE, 10);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_FAN_MAN_CMODE, 9);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_FAN_DISPLAY, 4);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_FAN_SPEED_OVRR, 6);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_ENABLE_MODE_ICON, 4);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_ENAB_LIGHT_SYMBOL, 0);     
#if(K_Heater_Abil==1)
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_ENAB_AC_SYMBOL, 1);     
#else
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_ENAB_AC_SYMBOL, 0);     
#endif        
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_TEMP_UNIT, 0);           // mettere 1 se si vuole abilitare la scelta F/C da display
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_LOCK_MODE, 0);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_HUMI_DISPLAY, 0);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_ENABLE_OFF_ICON, 1);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_NIGHT_FAN_SPEED, 1);     
        //WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_BACKL_BRIGHT, 1);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_LANGUAGE, 0);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_SENSOR3_TEXT, 16);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_DISAB_ECOB_CANCEL, 1);     

        // Reinit di tutti i registri di Override (Solo per sicurezza!! Sono già disattivati di default)    
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_OVERR_MODE_ICON, 0);     
        //WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_OVERR_AC, 0);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_OVERR_LIGHTS, 0);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_OVERR_LOCK_MODE, 0);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_OVERR_RELAY, 0);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_OVERR_RELAY, 0);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_OVERR_RELAY, 0);     
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_E2_UPDATE, 1);     
        DelaymSec(100);
        //WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_FORCE_RESET, 1);     
        Send_WriteModRegSec(Add_SyxtSRoom, REGH_SYXTS_FORCE_RESET, 1, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusOneBased);	// Il registro dei touch hanno un offset di 1

        DelaymSec(6000);
    }
    
#if(k_Abil_Panel_Shared==1)
    WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_SENSOR3_SOURCE, 0);
    DelaymSec(2000);
#endif

    
    if((SyxtS[0].FwVer_Upper_423==1) && (EngineBox.Touch_Nation==1))
    {
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_SENS3_NET_TEMP, (int)round((((((float)Me.TempAmbTouchVisual)*9.0/5.0)+3200.0)/10.0)));
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_NOM_SETP, Touch[0].Script_SetPoint_F/10);     
    }
    else
    {
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_SENS3_NET_TEMP, Me.TempAmbTouchVisual/10);         
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_NOM_SETP, Touch[0].SetPoint/10);     
    }
    WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_FAN_SPEED_OVRR_ADV, Touch[0].FanMode);         
    OnOffReverse = ~Touch[0].Script_Split_Pwr & 0x0001;  
    WriteCoilModRegSecSyxtS (Add_SyxtSRoom, COIL_SYXTS_OFF_MODE_OVRR, OnOffReverse);

#if(K_Heater_Abil==1)
    WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_OVERR_AC, (1*(Touch[0].HeaterPwr==1))+(2*(Touch[0].HeaterPwr==0)));
    DelaymSec(20);
    WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_OVERR_AC, 0);    
#endif    
}


void RefreshAllSyxtSOutPut(void)
{
    Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
               
    if((SyxtS[0].FwVer_Upper_423==1) && (EngineBox.Touch_Nation==1))
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_SENS3_NET_TEMP, (int)round((((((float)Me.TempAmbTouchVisual)*9.0/5.0)+3200.0)/10.0)));
    else
        WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_SENS3_NET_TEMP, Me.TempAmbTouchVisual/10);         
#if(k_Abil_Panel_Shared==0)        
    WriteCoilModRegSecSyxtS (Add_SyxtSRoom, COIL_SYXTS_ECO_MODE_OVRR, EngineBox.Eco_Mode_Status);
    
    WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_OVERR_LOCK_MODE, (1*EngineBox.SystemDisable)+(2*!EngineBox.SystemDisable));
#endif
    WriteModRegSecSyxtS (Add_SyxtSRoom, REGH_SYXTS_OVERR_MODE_ICON, (2*(Me.DefMode==CoreRiscaldamento))+(3*(Me.DefMode==CoreRaffrescamento))+(4*(Me.DefMode==CoreVentilazione)));  //1=Auto, 2=Heating, 3=Cooling, 4=Ventilation
    
    
}


void Resume_SyxtSOffLine(unsigned char * Cnt)
{
	char Result;
    static int CntCycleOnLine=0;    
    int temp = 0;
	
	// Provo a recuperare i moduli 
	if((SyxtS[(*Cnt)].Enable == 1) && (SyxtS[(*Cnt)].OffLine == 1))
	{     
        Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    
        Result = Send_WaitModRxSec((Add_SyxtSRoom +(*Cnt)), ReadModInput, REGI_SYXTS_FW_REL, 1, MaxModChkWaitSec, MaxModChkRetriesSec, K_ModBusOneBased);
        
        if(Result==0)
            CntCycleOnLine++;
        else
            CntCycleOnLine=0;
        
        if(CntCycleOnLine >= 2)
        {        
            SyxtS[(*Cnt)].OffLine = (Result != 0);
            //SyxtS[(*Cnt)].OnLine = (SyxtS[(*Cnt)].OffLine == 0);            
            CntCycleOnLine=0;
        }               
            
		Mod_LastRxSec.Valid_Data = 0;
/*
        if(SyxtS[(*Cnt)].OnLine)
        {
            Me.MaxFanVentil_Mode = K_DefaultFanSpeed_SyxtS;
            Me.ModDeviceType = K_ModDeviceSyxtS;            
            InitSyxtS();
        }  
*/        

        if(SyxtS[(*Cnt)].OffLine == 1)
        {
            if(SyxtS[(*Cnt)].PermanentOffline == 0 && CntCycleOnLine==0)
            {
                SyxtS[(*Cnt)].CntErrTouch++;
                if(SyxtS[(*Cnt)].CntErrTouch > k_MAX_Cnt_Err_NetB)
                    SyxtS[(*Cnt)].PermanentOffline = 1;
            }
        }
        else
        {
            Me.MaxFanVentil_Mode = K_DefaultFanSpeed_SyxtS;
            Me.ModDeviceType = K_ModDeviceSyxtS;            
            SyxtS[(*Cnt)].OnLine = (SyxtS[(*Cnt)].Enable == 1) && (SyxtS[(*Cnt)].OffLine == 0);
            if(SyxtS[(*Cnt)].CntErrTouch>2)
            {
#if(k_Abil_Panel_Shared==1)
            ReadModInputSecSyxtS (Add_SyxtSRoom, REGI_SYXTS_FW_REL, &(temp));
            if(temp>=423)
                SyxtS[0].FwVer_Upper_423 = 1;
#endif                                    
                InitSyxtS();
            }
            
            SyxtS[(*Cnt)].CntErrTouch = 0;
            if(SyxtS[(*Cnt)].PermanentOffline == 1)
            {        
                SyxtS[(*Cnt)].PermanentOffline = 0;
            }
        }        
 	}
	(*Cnt) += 1;
	if((*Cnt) >= MaxSyxtS)
		(*Cnt) = 0;
}

void CheckHotPlugSyxtS(unsigned char * Cnt)
{
	char Result = 0;
	unsigned char CurrentId = 0;
    static int CntCycleOnLine=0;
    int temp = 0;

	while((*Cnt) < MaxSyxtS)				// Provo a recuperare i SyxtS
	{
		CurrentId = (*Cnt);
		if((SyxtS[CurrentId].Enable == 0))
		{

            Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud

            DisableModComErrorCounter = 1;
            Result = Send_WaitModRxSec((Add_SyxtSRoom +(*Cnt)), ReadModInput, REGI_SYXTS_FW_REL, 1, MaxModChkWaitSec, MaxModChkRetriesSec, K_ModBusOneBased);
            
            DisableModComErrorCounter = 0;
            
            if(Result==0)
                CntCycleOnLine++;
            else
                CntCycleOnLine=0;
                
            if(CntCycleOnLine >= 2)
            {
                SyxtS[(*Cnt)].OffLine = (Result != 0);                
                SyxtS[(*Cnt)].Enable = !SyxtS[(*Cnt)].OffLine;	// Se lo trovo lo abilito.    
                SyxtS[(*Cnt)].OnLine = (SyxtS[(*Cnt)].Enable == 1) && (SyxtS[(*Cnt)].OffLine == 0);                
                CntCycleOnLine=0;
            }   

            if(SyxtS[(*Cnt)].OnLine)
            {
#if(k_Abil_Panel_Shared==1)
                ReadModInputSecSyxtS (Add_SyxtSRoom, REGI_SYXTS_FW_REL, &(temp));
                if(temp>=423)
                    SyxtS[0].FwVer_Upper_423 = 1;
#endif        
                SyxtS[(*Cnt)].CntErrTouch = 0;
                SyxtS[(*Cnt)].PermanentOffline = 0;
                Me.MaxFanVentil_Mode = K_DefaultFanSpeed_SyxtS;
                Me.ModDeviceType = K_ModDeviceSyxtS;
                InitSyxtS();
            }     
            
			(*Cnt) += 1;
			break;
		}
		(*Cnt) += 1;
	}
	if(((*Cnt) >= MaxSyxtS) || ((*Cnt) < 0))
		(*Cnt) = 0;
}


void SyncronizeDataSetP_SyxtS(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddrW, int RegAddrI, int pointer)
{
	static int firstTimeFlag[1];
	static int OldTouchData[1];
	static int ReadyData[1];
	static int CntOkData[1];
	static int TouchNewData[1];
	static int timer=0;
           int MaxDataPointer;
           int temp;
	
	
	
	if((*OldData)==0 && firstTimeFlag[pointer]==0)	//Inizializzo se sono a primo giro di programma
	{	
		(*OldData) = (*Data);
		firstTimeFlag[pointer] = 1;
	}
    
    if(Reinit.FromEngBox.TrigInit!=0 || Me.OtherCabin.Trigger_Init!=0)
    {
        ReadyData[pointer] = 0;	
        CntOkData[pointer]=0;
    }
	
    if(SyxtS[0].OnLine)
        MaxDataPointer = 20;//10;
    else
        MaxDataPointer = 10;//15;

    if(ReadyData[pointer] == 0)
	{
		CntOkData[pointer]++;
		if(CntOkData[pointer]>=MaxDataPointer)
		{
			ReadyData[pointer] = 1;	
			CntOkData[pointer]=0;
		}
	}

	if(DiagnosticSplit.DataProcess==0)
	{
        //if(Send_WaitModRxSec(Add_SyxtSRoom, ReadModCmd, RegAddr, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
        //    SyxtS[0].OffLine = 1;
        if(SyxtS[0].OnLine)														// Leggo registro "PowerLimit" dal touch
		{
            ReadModInputSecSyxtS (Add_SyxtSRoom, RegAddrI, &(temp));
            (*TouchData) = temp/10;                         // Perdo il decimale
            (*TouchData) = (*TouchData)*100;				// Converto in 100° di grado
        }           
    }
	if(DiagnosticSplit.DataProcess==1)
	{
        if(SyxtS[0].OnLine)														// Leggo registro "PowerLimit" dal touch
        {
            //if(Send_WaitModRxSec(Add_SyxtSRoom, ReadModCmd, RegAddr, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
            //    SyxtS[0].OffLine = 1;
            ReadModInputSecSyxtS (Add_SyxtSRoom, RegAddrI, &(temp));

            (*TouchData) = temp/10;                         // Perdo il decimale
            (*TouchData) = (*TouchData)*100;				// Converto in 100° di grado

            if((*TouchData)!=OldTouchData[pointer])
                timer++;

            if((*TouchData)!=TouchNewData[pointer])
                timer=0;

            TouchNewData[pointer]=(*TouchData);		

            if(((*TouchData)!=OldTouchData[pointer]) && (timer>=10))//20))
            {
                (*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
                (*Data) = (*TouchData);
                (*OldData) = (*TouchData); 
                OldTouchData[pointer] = (*TouchData);
                ReadyData[pointer] = 0;	
                CntOkData[pointer]=0;
                timer=0;
            }

            Mod_LastRxSec.Valid_Data = 0;
        }
		else
		{
            if(ReadyData[pointer] == 1)
                (*TouchData) = (*Data);
		}
        
        if((*Data)!=(*OldData)&&(ReadyData[pointer] == 1))								// Se è arrivato un aggiornamento dal Master
        {
            if(SyxtS[0].OnLine)
            {
                //SyxtS[0].OffLine = Send_WriteModRegSec(Add_SyxtSRoom, RegAddr, (*Data)/10, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Il registro dei touch hanno un offset di 1
                WriteModRegSecSyxtS (Add_SyxtSRoom, RegAddrW, (*Data)/10);     
            //		Inverter->OffLine = Send_WriteModRegSec(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_RUN_COMMAND, MaxModWriteWaitSec, MaxModWriteRetriesSec, WriteModCmd, K_ModBusZeroBased);	// Lo mando in RUN

            }		

            (*OldData) = (*Data);								// Aggiorno OLD
            OldTouchData[pointer] = (*Data);
            (*SendMasData) = (*Data);
        }				
	}
    else
    {
        //if(Touch[0].OnLine)														// Leggo registro "PowerLimit" dal touch
		{
            (*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
            (*Data) = (*TouchData);
            (*OldData) = (*TouchData); 
            OldTouchData[pointer] = (*TouchData);
            //ReadyData[pointer] = 0;	
            //CntOkData[pointer]=0;
		}
        //else
        //if(!SyxtS[0].Enable)														// Leggo registro "PowerLimit" dal touch
        //{
        //    ReadyData[pointer] = 0;	//NON ERA COMMENTATO IN V.8.47.161
        //    CntOkData[pointer]=0;          
        //}
#if(k_Abil_Panel_Shared==1)        
        //else
        //{
            ReadyData[pointer] = 1;	
			CntOkData[pointer]=0;
        //}
#endif
    }           
}

void SyncronizeDataOnOff_SyxtS(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer)
{
	static int firstTimeFlag[1];
	static int OldTouchData[1];
	static int ReadyData[1];
	static int CntOkData[1];
	static int TouchNewData[1];
	static int timer=0;
           int MaxDataPointer;
           int temp=0;
	
	
	
	if((*OldData)==0 && firstTimeFlag[pointer]==0)	//Inizializzo se sono a primo giro di programma
	{	
		(*OldData) = (*Data);
		firstTimeFlag[pointer] = 1;
	}
    
    if(Reinit.FromEngBox.TrigInit!=0 || Me.OtherCabin.Trigger_Init!=0)
    {
        ReadyData[pointer] = 0;	
        CntOkData[pointer]=0;
    }
	
    if(SyxtS[0].OnLine)
        MaxDataPointer = 20;//10;
    else
        MaxDataPointer = 10;//15;

    if(ReadyData[pointer] == 0)
	{
		CntOkData[pointer]++;
		if(CntOkData[pointer]>=MaxDataPointer)
		{
			ReadyData[pointer] = 1;	
			CntOkData[pointer]=0;
		}
	}

	if(DiagnosticSplit.DataProcess==0)
	{
        if(SyxtS[0].OnLine)														// Leggo registro "PowerLimit" dal touch
		{
            ReadCoilModRegSecSyxtS (Add_SyxtSRoom, RegAddr, &(temp));
            (*TouchData) = ~temp & 0x0001;              
        }           
    }
	if(DiagnosticSplit.DataProcess==1)
	{
        if(SyxtS[0].OnLine)														// Leggo registro "PowerLimit" dal touch
        {
            ReadCoilModRegSecSyxtS (Add_SyxtSRoom, RegAddr, &(temp));

            (*TouchData) = ~temp & 0x0001;              

            if((*TouchData)!=OldTouchData[pointer])
                timer++;

            if((*TouchData)!=TouchNewData[pointer])
                timer=0;

            TouchNewData[pointer]=(*TouchData);		

            if(((*TouchData)!=OldTouchData[pointer]) && (timer>=10))
            {
                (*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
                (*Data) = (*TouchData);
                (*OldData) = (*TouchData); 
                OldTouchData[pointer] = (*TouchData);
                ReadyData[pointer] = 0;	
                CntOkData[pointer]=0;
                timer=0;
            }

            Mod_LastRxSec.Valid_Data = 0;
        }
		else
		{
            if(ReadyData[pointer] == 1)
                (*TouchData) = (*Data);
		}
        
        if((*Data)!=(*OldData)&&(ReadyData[pointer] == 1))								// Se è arrivato un aggiornamento dal Master
        {
            if(SyxtS[0].OnLine)
            {
                temp = ~(*Data) & 0x0001;              
                WriteCoilModRegSecSyxtS (Add_SyxtSRoom, RegAddr, temp);     
            }		

            (*OldData) = (*Data);								// Aggiorno OLD
            OldTouchData[pointer] = (*Data);
            (*SendMasData) = (*Data);
        }				
	}
    else
    {
        //if(Touch[0].OnLine)														// Leggo registro "PowerLimit" dal touch
		{
            (*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
            (*Data) = (*TouchData);
            (*OldData) = (*TouchData); 
            OldTouchData[pointer] = (*TouchData);
            //ReadyData[pointer] = 0;	
            //CntOkData[pointer]=0;
		}
        //else
        //if(!SyxtS[0].Enable)														// Leggo registro "PowerLimit" dal touch
        //{
        //    ReadyData[pointer] = 0;	
        //    CntOkData[pointer]=0;          
        //}
#if(k_Abil_Panel_Shared==1)        
        //else
        //{
            ReadyData[pointer] = 1;	
			CntOkData[pointer]=0;
        //}
#endif
    }
}

void SyncronizeDataVel_SyxtS(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int pointer)
{
	static int firstTimeFlag[1];
	static int OldTouchData[1];
	static int ReadyData[1];
	static int CntOkData[1];
	static int TouchNewData[1];
	static int timer=0;
           int MaxDataPointer;
           int temp;
           
	if((*OldData)==0 && firstTimeFlag[pointer]==0)	//Inizializzo se sono a primo giro di programma
	{	
		(*OldData) = (*Data);
		firstTimeFlag[pointer] = 1;
	}
    
    if(Reinit.FromEngBox.TrigInit!=0 || Me.OtherCabin.Trigger_Init!=0)
    {
        ReadyData[pointer] = 0;	
        CntOkData[pointer]=0;
    }
	
    if(SyxtS[0].OnLine)
        MaxDataPointer = 20;//10;
    else
        MaxDataPointer = 10;//15;

    if(ReadyData[pointer] == 0)
	{
		CntOkData[pointer]++;
		if(CntOkData[pointer]>=MaxDataPointer)
		{
			ReadyData[pointer] = 1;	
			CntOkData[pointer]=0;
		}
	}

	if(DiagnosticSplit.DataProcess==0)
	{
        //if(Send_WaitModRxSec(Add_SyxtSRoom, ReadModCmd, RegAddr, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
        //    SyxtS[0].OffLine = 1;
        if(SyxtS[0].OnLine)														// Leggo registro "PowerLimit" dal touch
		{
            ReadModRegSecSyxtS (Add_SyxtSRoom, RegAddr, &(temp));
            (*TouchData) = temp;
        }           
    }
	if(DiagnosticSplit.DataProcess==1)
	{
        if(SyxtS[0].OnLine)														// Leggo registro "PowerLimit" dal touch
        {
            //if(Send_WaitModRxSec(Add_SyxtSRoom, ReadModCmd, RegAddr, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
            //    SyxtS[0].OffLine = 1;
            ReadModRegSecSyxtS (Add_SyxtSRoom, RegAddr, &(temp));
            (*TouchData) = temp;

            if((*TouchData)!=OldTouchData[pointer])
                timer++;

            if((*TouchData)!=TouchNewData[pointer])
                timer=0;

            TouchNewData[pointer]=(*TouchData);		

            if(((*TouchData)!=OldTouchData[pointer]) && (timer>=10))
            {
                (*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
                (*Data) = (*TouchData);
                (*OldData) = (*TouchData); 
                OldTouchData[pointer] = (*TouchData);
                ReadyData[pointer] = 0;	
                CntOkData[pointer]=0;
                timer=0;
            }

            Mod_LastRxSec.Valid_Data = 0;
        }
		else
		{
            if(ReadyData[pointer] == 1)
                (*TouchData) = (*Data);
		}
        
        if((*Data)!=(*OldData)&&(ReadyData[pointer] == 1))								// Se è arrivato un aggiornamento dal Master
        {
            if(SyxtS[0].OnLine)
            {
                WriteModRegSecSyxtS (Add_SyxtSRoom, RegAddr, (*Data));     
            }		

            (*OldData) = (*Data);								// Aggiorno OLD
            OldTouchData[pointer] = (*Data);
            (*SendMasData) = (*Data);
        }				
	}
    else
    {
        //if(Touch[0].OnLine)														// Leggo registro "PowerLimit" dal touch
		{
            (*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
            (*Data) = (*TouchData);
            (*OldData) = (*TouchData); 
            OldTouchData[pointer] = (*TouchData);
            //ReadyData[pointer] = 0;	
            //CntOkData[pointer]=0;
		}
        //else
        //if(!SyxtS[0].Enable)														// Leggo registro "PowerLimit" dal touch
        //{
        //    ReadyData[pointer] = 0;	
        //    CntOkData[pointer]=0;          
        //}
#if(k_Abil_Panel_Shared==1)        
        //else
        //{
            ReadyData[pointer] = 1;	
			CntOkData[pointer]=0;
        //}
#endif
    }           
}

void SyncronizeDataHeater_SyxtS(volatile int * Data, volatile int * OldData, volatile int * TouchData, volatile int * SendMasData, int RegAddr, int RegAddrWr, int pointer)
{
	static int firstTimeFlag[1];
	static int OldTouchData[1];
	static int ReadyData[1];
	static int CntOkData[1];
	static int TouchNewData[1];
	static int timer=0;
           int MaxDataPointer;
           int temp;
           
	if((*OldData)==0 && firstTimeFlag[pointer]==0)	//Inizializzo se sono a primo giro di programma
	{	
		(*OldData) = (*Data);
		firstTimeFlag[pointer] = 1;
	}
    
    if(Reinit.FromEngBox.TrigInit!=0 || Me.OtherCabin.Trigger_Init!=0)
    {
        ReadyData[pointer] = 0;	
        CntOkData[pointer]=0;
    }
	
    if(SyxtS[0].OnLine)
        MaxDataPointer = 20;//10;
    else
        MaxDataPointer = 10;//15;

    if(ReadyData[pointer] == 0)
	{
		CntOkData[pointer]++;
		if(CntOkData[pointer]>=MaxDataPointer)
		{
			ReadyData[pointer] = 1;	
			CntOkData[pointer]=0;
		}
	}

	if(DiagnosticSplit.DataProcess==0)
	{
        //if(Send_WaitModRxSec(Add_SyxtSRoom, ReadModCmd, RegAddr, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
        //    SyxtS[0].OffLine = 1;
        if(SyxtS[0].OnLine)														// Leggo registro "PowerLimit" dal touch
		{
            ReadInputStModRegSecSyxtS (Add_SyxtSRoom, RegAddr, &(temp));
            (*TouchData) = temp;
        }           
    }
	if(DiagnosticSplit.DataProcess==1)
	{
        if(SyxtS[0].OnLine)														// Leggo registro "PowerLimit" dal touch
        {
            //if(Send_WaitModRxSec(Add_SyxtSRoom, ReadModCmd, RegAddr, 1, MaxModReadWaitSec, MaxModReadRetriesSec, K_ModBusZeroBased))	// Il registro dei touch hanno un offset di 1
            //    SyxtS[0].OffLine = 1;
            ReadInputStModRegSecSyxtS (Add_SyxtSRoom, RegAddr, &(temp));
            (*TouchData) = temp;

            if((*TouchData)!=OldTouchData[pointer])
                timer++;

            if((*TouchData)!=TouchNewData[pointer])
                timer=0;

            TouchNewData[pointer]=(*TouchData);		

            if(((*TouchData)!=OldTouchData[pointer]) && (timer>=10))
            {
                (*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
                (*Data) = (*TouchData);
                (*OldData) = (*TouchData); 
                OldTouchData[pointer] = (*TouchData);
                ReadyData[pointer] = 0;	
                CntOkData[pointer]=0;
                timer=0;
            }

            Mod_LastRxSec.Valid_Data = 0;
        }
		else
		{
            if(ReadyData[pointer] == 1)
                (*TouchData) = (*Data);
		}
        
        if((*Data)!=(*OldData)&&(ReadyData[pointer] == 1))								// Se è arrivato un aggiornamento dal Master
        {
            if(SyxtS[0].OnLine)
            {
                if((*Data)==0)
                    WriteModRegSecSyxtS (Add_SyxtSRoom, RegAddrWr, 2);
                else if((*Data)==1)
                    WriteModRegSecSyxtS (Add_SyxtSRoom, RegAddrWr, 1);
            }		

            (*OldData) = (*Data);								// Aggiorno OLD
            OldTouchData[pointer] = (*Data);
            (*SendMasData) = (*Data);
            DelaymSec(100);
            WriteModRegSecSyxtS (Add_SyxtSRoom, RegAddrWr, 0);
        }				
	}
    else
    {
        //if(Touch[0].OnLine)														// Leggo registro "PowerLimit" dal touch
		{
            (*SendMasData) = (*TouchData);				// e aggiorno "Me" con i valore letto (sarà letto dal Master
            (*Data) = (*TouchData);
            (*OldData) = (*TouchData); 
            OldTouchData[pointer] = (*TouchData);
            //ReadyData[pointer] = 0;	
            //CntOkData[pointer]=0;
		}
        //else
        //if(!SyxtS[0].Enable)														// Leggo registro "PowerLimit" dal touch
        //{
        //    ReadyData[pointer] = 0;	
        //    CntOkData[pointer]=0;          
        //}
#if(k_Abil_Panel_Shared==1)        
        //else
        //{
        //    ReadyData[pointer] = 1;	
		//	CntOkData[pointer]=0;
        //}
#endif
    }           
}

//(conver > (floor(conver)+0.5f)) ? ceil(conver) : floor(conver);
float round(float fvalue)
{
    float ftemp;
    
    ftemp = floor(fvalue)+0.5;
    
    if(fvalue > ftemp)
        ftemp = ceil(fvalue);
    else
        ftemp = floor(fvalue);
    
    return  ftemp;
}

/*
//-------------------------------------------------------------------------------------------------------------------
// Rilevamento automatico del device modbus collegato: Se rilevato device viene setato il flag "Me.ModBusDeviceDetected"
//          0= NoDevice connected, 
//          1=Touch Exor/Carel connected, 
//          2=Touch Syxthsense connected, 
//          3=NetBuilding Panel connected
//-------------------------------------------------------------------------------------------------------------------
void SearchModbusDevice(void)
{
    //static unsigned int DeviceScanPointer=0;

    if(Touch[0].OnLine==0 &&  SyxtS[0].OnLine==0 && NetB[0].OnLine==0)
        Me.ModBusDeviceDetected = 0;
    
    if(Me.ModBusDeviceDetected == 0)
    {
        Search_Touch();                 // Me.ModBusDeviceDetected = K_ModDeviceExor (1)
        if(Me.ModBusDeviceDetected == 0)
        {
            Me.EnableHiSpeedModBusTouch_Sec = !Me.EnableHiSpeedModBusTouch_Sec;      // Togglo flag
            Search_Touch();                 // Me.ModBusDeviceDetected = K_ModDeviceExor (1)
        }   
        
        if(Me.ModBusDeviceDetected == K_ModDeviceExor)
        {
            Me.EnableHiSpeedMyBus_Sec = Me.EnableHiSpeedModBusTouch_Sec;    // Se touch è collegato allineo la velocità del MyBUS
        }
        
        //InitTouch();
    }
        
    if(Me.ModBusDeviceDetected == 0)
    {
        Search_SyxtS();                 // Me.ModBusDeviceDetected = K_ModDeviceSyxtS (2)
        //InitSyxtS();
    }

    if(Me.ModBusDeviceDetected == 0)
    {
        Search_NetB();                  // Me.ModBusDeviceDetected = K_ModDeviceNetB (3)
        //InitNetB();
    }
    
   
}
 */


//-------------------------------------------------------------------------------------------------------------------
// Rilevamento automatico del device modbus collegato: Se rilevato device viene setato il flag "Me.ModBusDeviceDetected"
//          0= NoDevice connected, 
//          1=Touch Exor/Carel connected, 
//          2=Touch Syxthsense connected, 
//          3=NetBuilding Panel connected
//
//          Input Device: 
//          0= Touch Exor/Carel @57600 Baud
//          1= Touch Exor/Carel @115200 Baud
//          2= Touch Syxthsense #57600 Baud
//          3= NetBuilding Panel #57600 Baud
//-------------------------------------------------------------------------------------------------------------------
void SearchModbusDeviceIndex(int * Device)
{
    //static unsigned int DeviceScanPointer=0;

    if((*Device) > K_DEVICE_MODBUS_NUMBER)
        (*Device)=0;
/*   
    if(Touch[0].OnLine==0 &&  SyxtS[0].OnLine==0 && NetB[0].OnLine==0)
        Me.ModBusDeviceDetected = 0;
*/ 
    if(Me.ModDeviceEnableAndOffLine==0) // Se non mai trovato nessun device modbus....
        DisableModComErrorCounter = 1;            
    
    if(Me.ModBusDeviceDetected == 0 && (*Device)==0)
    {
        Me.EnableHiSpeedModBusTouch_Sec = 0; // LowSpeed
        Search_Touch();                 // Me.ModBusDeviceDetected = K_ModDeviceExor (1)
        
        if(Me.ModBusDeviceDetected == K_ModDeviceExor)
        {
            Me.EnableHiSpeedMyBus_Sec = Me.EnableHiSpeedModBusTouch_Sec;    // Se touch è collegato allineo la velocità del MyBUS
        }
    }
    else if(Me.ModBusDeviceDetected == 0 && (*Device)==1)
    {
        Me.EnableHiSpeedModBusTouch_Sec = 1; // HiSpeed
        Search_Touch();                 // Me.ModBusDeviceDetected = K_ModDeviceExor (1)
        
        if(Me.ModBusDeviceDetected == K_ModDeviceExor)
        {
            Me.EnableHiSpeedMyBus_Sec = Me.EnableHiSpeedModBusTouch_Sec;    // Se touch è collegato allineo la velocità del MyBUS
        }
    }
    else if(Me.ModBusDeviceDetected == 0 && (*Device)==2)
    {
        Search_SyxtS();                 // Me.ModBusDeviceDetected = K_ModDeviceSyxtS (2)
        
        if(Me.ModBusDeviceDetected == K_ModDeviceSyxtS)
        {
            Me.EnableHiSpeedMyBus_Sec = K_EnableHiSpeedMyBus_Sec;    // Se touch è collegato allineo la velocità del MyBUS
        }
        
    }
    else if(Me.ModBusDeviceDetected == 0 && (*Device)==3)
    {
        Search_NetB();                  // Me.ModBusDeviceDetected = K_ModDeviceNetB (3)

        if(Me.ModBusDeviceDetected == K_ModDeviceNetB)
        {
            Me.EnableHiSpeedMyBus_Sec = K_EnableHiSpeedMyBus_Sec;    // Se touch è collegato allineo la velocità del MyBUS
        }
    }
   
    DisableModComErrorCounter = 0;        
    
    if(Me.ModBusDeviceDetected==K_ModDeviceExor)
    {
        Me.SyxtSSelect = 0;
        Me.NetBSelect = 0;
    }
    else if(Me.ModBusDeviceDetected==K_ModDeviceNetB)
    {
        Me.NetBSelect = 1;
        Me.SyxtSSelect = 0;
    }
    else if(Me.ModBusDeviceDetected==K_ModDeviceSyxtS)
    {
        Me.SyxtSSelect = 1;
        Me.NetBSelect = 0;
    }

   
}



int RecheckDataReadTouch(int ReadNow, int VarData, int * PreviousVarData, unsigned int * ReadDataCounter)
{
#define K_ReadMax 2
#define RecheckReadTouch 0
    
#if(RecheckReadTouch==1)
    if(ReadNow == VarData)
    {
        (*ReadDataCounter)=0;
        (*PreviousVarData) = ReadNow;
        return ReadNow;
    }
    
    if(ReadNow!=(*PreviousVarData))
        (*ReadDataCounter)=0;

    (*ReadDataCounter)++;
    
    if((*ReadDataCounter)<K_ReadMax)
    {
        (*PreviousVarData) = ReadNow;
        return VarData;
    }
    else
    {
        (*PreviousVarData) = ReadNow;
        return ReadNow;        
    }
#else
        return ReadNow;        
#endif    
}