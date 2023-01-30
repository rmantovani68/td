//----------------------------------------------------------------------------------
//	Progect name:	ProtocolloModBus.c
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			Emanuele
//	Date:			18/06/2011
//	Description:	Corpo delle funzioni che gestiscono la comunicazione ModBus con il sistema
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//	Include
//----------------------------------------------------------------------------------
#include "HWSelection.h"
#if (DA210==1)
	#include <p24FJ256DA210.h>
#else
	#include <p24FJ256GB210.h>
#endif

#include <stdio.h>
#include "DefinePeriferiche.h"
#include "Timer.h"
#include "usart.h"
#include "Driver_Comunicazione.h"
#include "ProtocolloComunicazione.h"
#include "Driver_ModBus.h"
#include "core.h"
#include "FWSelection.h"
#include "ProtocolloModBus.h"
#include "delay.h"
#include "Bridge1DataAddressMap.h"
#include "Bridge2DataAddressMap.h"
#include "DAC.h"


//extern volatile TypTimer TimerModRstComErr;
extern volatile int ModCntComErr;
extern volatile int ModTimeOutComErr;
extern unsigned char CurUsedProtocolId;		// Id del protocollo correntemente utilizzato
extern volatile int ModTimeOutComErr_U2;		
extern volatile int ModCntComErr_U2;			

extern volatile TypTimer TimerInverterPersError_C1;
extern volatile TypTimer TimerInverterPersError_C2;
extern volatile TypTimer TimerInverterPersError_Pump;
extern volatile TypTimer TimerInVoltageInverterOk_C1;
extern volatile TypTimer TimerInVoltageInverterOk_C2;



//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------

unsigned char WaitModRx(int TimeOut_ms)
{
	TimeOutModPktRx.Value = 0;				// resetto il timer per il time out della risposta
	TimeOutModPktRx.Time = TimeOut_ms;		// lo configuro come richiesto
	TimeOutModPktRx.Enable = 1;				// lo faccio partireOldApp1
	TimeOutModPktRx.TimeOut = 0;			//
    

	while((TimeOutModPktRx.TimeOut == 0) && (Mod_LastRx.Valid_Data == 0))	// attendo un evento
		continue;
   
    TimeOutModPktRx.Enable = 0;			// fermo il timer
    TimeOutModPktRx.TimeOut = 0;		// resetto il time out
    iPckModRx = 0;  
    

	if(Mod_LastRx.Valid_Data != 0)		// se ho ricevuto un dato valido 
	{
		if( (Mod_LastRx.Buffer[0] == Mod_BufferTx.Buffer[0]) &&	// verifico il pacchetto
			(Mod_LastRx.Buffer[1] == Mod_BufferTx.Buffer[1]))
			return 0;											// ritorno 0 se � ok
	}									// in caso contrario
    
    if(EngineBox[0].DisableModComErrorCounter==0)
    {
        if(CurUsedProtocolId==FlagModBusProt_BUSM)    
        {
            ModTimeOutComErr++;
            EngineBox[0].ComError.ModTimeOutComErr++;
        }
        if(CurUsedProtocolId==FlagModBusProt_BUSS)      
        {
            ModTimeOutComErr_U2++;
            EngineBox[0].ComError.ModTimeOutComErr_U2++;
        }    
    }
    
	return 1;							// ritorno 1
}

unsigned char Send_WaitModRx(char Address, char Comando, int Registro, int Data, int TimeOut_ms, char Retry)
{
	Result = 0;
	RetryCnt = 0;
	while (RetryCnt < Retry)
	{	
		RetryCnt +=1;
		if(Address == k_Add_Touch) 
			Mod_Write_Cmd(Address, Comando, Registro -1, Data);	// v4.4.117 !!!!
		else 
			Mod_Write_Cmd(Address, Comando, Registro, Data);
		Result = WaitModRx(TimeOut_ms);
		if (Result == 0)
        {
			Mod_LastRx.Valid_Data = 0;            
			DelayuSec(1000); //00);		// v.5.16.162 - Pausa per garantire una minima distanza tra un Tx e il successivo 
        	return 0;		// se ricevo un pacchetto valido 
        }
	}
	//TimerModRstComErr.Value = 0;       
    if(CurUsedProtocolId==FlagModBusProt_BUSM)    
    {
        ModCntComErr++;
        EngineBox[0].ComError.ModCntComErr++;
    }
    if(CurUsedProtocolId==FlagModBusProt_BUSS)      
    {
        ModCntComErr_U2++;
        EngineBox[0].ComError.ModCntComErr_U2++;
    }
    
	Mod_LastRx.Valid_Data = 0;
	return 1;				// se non ho nessuna risposta valida per dopo il numero di tentativi richiesti
}

unsigned char Send_WriteModReg(char Address, int Registro, int Data, int TimeOut_ms, char Retry, char Cmd)
{
	Result = 1;
	RetryCnt = 0;
	while (RetryCnt < Retry)
	{
		RetryCnt +=1;
		if(Address == k_Add_Touch) 
			Mod_Write_Cmd(Address, Cmd, Registro -1, Data);	// v4.4.117 !!!!
		else
			Mod_Write_Cmd(Address, Cmd, Registro, Data);
		Result = WaitModRx(TimeOut_ms);
		if ((Result == 0) &
			(Mod_LastRx.Buffer[2] == Mod_BufferTx.Buffer[2]) &
			(Mod_LastRx.Buffer[3] == Mod_BufferTx.Buffer[3]) &
			(Mod_LastRx.Buffer[4] == Mod_BufferTx.Buffer[4]) &
			(Mod_LastRx.Buffer[5] == Mod_BufferTx.Buffer[5])  )
		{
			Mod_LastRx.Valid_Data = 0;
			DelayuSec(1000); //00);		// v.5.16.162 - Pausa per garantire una minima distanza tra un Tx e il successivo 
			return 0;
		}
	}
	//TimerModRstComErr.Value = 0;    
    if(CurUsedProtocolId==FlagModBusProt_BUSM)    
    {
        ModCntComErr++;
        EngineBox[0].ComError.ModCntComErr++;
    }
    if(CurUsedProtocolId==FlagModBusProt_BUSS)     
    {
        ModCntComErr_U2++;
        EngineBox[0].ComError.ModCntComErr_U2++;
    }
    
	Mod_LastRx.Valid_Data = 0;
	return 1;
}

/*
void ReadModReg (char Address, int REG, int * StoreReg)
{
	if(!Room[Address-Add_TouchRoom1].Touch_OffLine)
	{
		if(!Send_WaitModRx(Address, ReadModCmd, REG, 1, MaxModReadWait, MaxModReadRetries))	// Il registro dei touch hanno un offset di 1
		{
			(*StoreReg) = Mod_LastRx.Data;
			Mod_LastRx.Valid_Data = 0;
		}
		else
		{
			Room[Address-Add_TouchRoom1].Touch_OffLine = 1;
		}
	}

}
*/

//-----------------------------------------------------------------
// Funzioni solo per Touch
//-----------------------------------------------------------------
/**/
void ReadModReg (char Address, int REG, int * StoreReg)
{
	char Result;
	if(!Touch.OffLine)
	{
		Result = Send_WaitModRx(Address, ReadModCmd, REG, 1, MaxModReadWait, MaxModReadRetries);
		if(Result == 0)	// Il registro dei touch hanno un offset di 1
		{
			(*StoreReg) = Mod_LastRx.Data;
			Mod_LastRx.Valid_Data = 0;
		}
		else
		{
			Touch.OffLine = 1;
		}
	}
}

char ReadModListReg (char DataIndex, int REG, int RegNum)
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

void WriteModReg (char Address, int REG, int SendData)
{
	unsigned char retcode;
/*	if(!Room[Address-Add_TouchRoom1].Touch_OffLine)
	{
		Room[Address-Add_TouchRoom1].Touch_OffLine = Send_WriteModReg(Address, REG, SendData, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Il registro dei touch hanno un offset di 1
	}*/
	retcode = Send_WriteModReg(Address, REG, SendData, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Il registro dei touch hanno un offset di 1
}

void WriteModListReg (char Address, int REG, int SendData)
{/*
	if(!Room[Address-Add_TouchRoom1].Touch_OffLine)
	{
		Room[Address-Add_TouchRoom1].Touch_OffLine = Send_WriteModReg(Address, REG, SendData, MaxModWriteWait, MaxModWriteRetries, WriteModListCmd);	// Il registro dei touch hanno un offset di 1
	}*/
}

int ExtracReg(char NumReg)
{
	// recupero un valore int dalla coda del buffer l'indice dei registri va da 1 al limite della richiesta
	Indice = 0;
	Data = 0;

	Indice = 1+(NumReg*2);
	Data = Mod_LastRx.Buffer[Indice]<<8;
	Data |= (Mod_LastRx.Buffer[Indice+1]&0x00FF);
	return Data;
}

void InsertReg(char NumReg, int Data)
{
	Indice = 0;
	
	Indice = 5+(NumReg*2);
	Mod_BufferTx.Buffer[Indice] = (char)((Data&0xFF00)>>8);
	Mod_BufferTx.Buffer[Indice+1] = (char)((Data&0x00FF));
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

void RefreshInverterPump(TypEngineBox * CompNumber, TypInverter * Inverter, int CompAddr, TypTimer * TimerPersError)
{
	static int sched=0;
	static int InverterErrorCode=0;	
    static int CntErrUnit = 0;
    static int CntMaxErrorPers = 0;
           int CorrectPercValueAbsorption_Inv=0;
           int K_Max_Error_Inverter_Continuous = 3;

    Change_Protocol_BUSS(K_ModBus_Baud38400);                                         // Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)
    
/*	
	Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_STATUS, 1, MaxModReadWait, MaxModReadRetries);
	if(!Inverter->OffLine)	
		Inverter->StatusReg = Mod_LastRx.Data;
*/

    Inverter->Req_Freq = CompNumber->FreqPompa_Acqua;

#if(K_AbilPumpInverter==1)            
    Inverter->Max_Req_Freq = K_LIM_MAX_FREQ_INV_PUMP;
#endif
    
    if(Inverter->PermanentOffLine)	
    {
        EngineBox[0].DisableModComErrorCounter=1;        
        Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_INPUT_POWER, 1, MaxModReadWait, 1);	
        EngineBox[0].DisableModComErrorCounter=0;        
    }
    else
        Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_INPUT_POWER, 1, MaxModReadWait, MaxModReadRetries);	

    if(Inverter->PermanentOffLine)	
        Inverter->Out_Power = 0;  //ArgoInverter
    
    if(!Inverter->OffLine)	
        //Inverter->In_Power = Mod_LastRx.Data;
        Inverter->Out_Power = Mod_LastRx.Data;  //ArgoPower

//Sommo correzione percentuale valori assorbimento    
#if(K_Pec_Sum_Inverter_Pump>0)
    if(Inverter->Out_Power>0)
    {
        CorrectPercValueAbsorption_Inv = (int)(((float)Inverter->Out_Power * (float)K_Pec_Sum_Inverter_Pump) / 100.0);
        Inverter->Out_Power = Inverter->Out_Power + CorrectPercValueAbsorption_Inv;
    }
#endif
   
/*	
    Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_OUTPUT_POWER, 1, MaxModReadWait, MaxModReadRetries);	
    if(!Inverter->OffLine)	
        Inverter->Out_Power = Mod_LastRx.Data;
*/


	if(!Inverter->OffLine)													
	{	

		Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_IN_VOLTAGE, 1, MaxModReadWait, MaxModReadRetries);
		if(!Inverter->OffLine)	
			Inverter->In_Voltage = Mod_LastRx.Data;
		/*
		Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_ALARM, 1, MaxModReadWait, MaxModReadRetries);	
		if(!Inverter->OffLine)
			Inverter->AlarmReg = Mod_LastRx.Data;		
        */
		Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_TRIP_CODE_MONITOR, 1, MaxModReadWait, MaxModReadRetries);	
		if(!Inverter->OffLine)	
			InverterErrorCode = Mod_LastRx.Data;		
        
		switch(sched)
		{
			case 0:
			Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_OUT_FREQ, 1, MaxModReadWait, MaxModReadRetries);	
			if(!Inverter->OffLine)	
				Inverter->Out_Freq = Mod_LastRx.Data;
			break;
			case 1:			
			Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_OUT_CURRENT, 1, MaxModReadWait, MaxModReadRetries);
			if(!Inverter->OffLine)	
				Inverter->Out_Current = Mod_LastRx.Data;
			break;
			case 2:
			Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_OUT_VOLTAGE, 1, MaxModReadWait, MaxModReadRetries);	
			if(!Inverter->OffLine)	
				Inverter->Out_Voltage = Mod_LastRx.Data;
			break;
/*
			case 3:
            Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_INPUT_POWER, 1, MaxModReadWait, MaxModReadRetries);	
			if(!Inverter->OffLine)	
				Inverter->In_Power = Mod_LastRx.Data;
			break;
*/
			/*
			case 4:
            Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_ALARM, 1, MaxModReadWait, MaxModReadRetries);	
            if(!Inverter->OffLine)
                Inverter->AlarmReg = Mod_LastRx.Data;
			break;
			*/
			case 5:
            Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_STATUS, 1, MaxModReadWait, MaxModReadRetries);
            if(!Inverter->OffLine)	
                Inverter->StatusReg = Mod_LastRx.Data;
			break;
		}

		sched++;
		if(sched>5)
			sched=0;
		
		// aggiorno i bit 
		Inverter->Status.FailFL = 			((Inverter->StatusReg & 0x0001)!=0);	// #bit 0
		Inverter->Status.Fail = 			((Inverter->StatusReg & 0x0002)!=0);	// #bit 1
		Inverter->Status.Alarm =			((Inverter->StatusReg & 0x0004)!=0);	// #bit 2
		Inverter->Status.UnderVoltage =		((Inverter->StatusReg & 0x0008)!=0);	// #bit 3
		Inverter->Status.MotorSection =		((Inverter->StatusReg & 0x0010)!=0);	// #bit 4
		Inverter->Status.PID_Control =		((Inverter->StatusReg & 0x0020)!=0);	// #bit 5
		Inverter->Status.AccDecPatt =		((Inverter->StatusReg & 0x0040)!=0);	// #bit 6
		Inverter->Status.DC_Bracking =		((Inverter->StatusReg & 0x0080)!=0);	// #bit 7
		Inverter->Status.JogRun = 			((Inverter->StatusReg & 0x0100)!=0);	// #bit 8
		Inverter->Status.FwRvRun = 			((Inverter->StatusReg & 0x0200)!=0);	// #bit 9
		Inverter->Status.RunStop = 			((Inverter->StatusReg & 0x0400)!=0);	// #bit 10
		Inverter->Status.CoastStop = 		((Inverter->StatusReg & 0x0800)!=0);	// #bit 11
		Inverter->Status.EmergencyStp = 	((Inverter->StatusReg & 0x1000)!=0);	// #bit 12
		Inverter->Status.StandBy_ST = 		((Inverter->StatusReg & 0x2000)!=0);	// #bit 13
		Inverter->Status.StandBy = 			((Inverter->StatusReg & 0x4000)!=0);	// #bit 14
/*
		Inverter->Alarm.OverCurrent =		((Inverter->AlarmReg & 0x0001)!=0);	// #bit 0
		Inverter->Alarm.InverterOverLoad =	((Inverter->AlarmReg & 0x0002)!=0);	// #bit 1
		Inverter->Alarm.MotorOverLoad =		((Inverter->AlarmReg & 0x0004)!=0);	// #bit 2
		Inverter->Alarm.OverHeat =			((Inverter->AlarmReg & 0x0008)!=0);	// #bit 3
		Inverter->Alarm.OverVoltage =		((Inverter->AlarmReg & 0x0010)!=0);	// #bit 4
		Inverter->Alarm.UnderVoltage =		((Inverter->AlarmReg & 0x0020)!=0);	// #bit 5
		Inverter->Alarm.MainOverLoad =		((Inverter->AlarmReg & 0x0040)!=0);	// #bit 6
		Inverter->Alarm.LowCurrent =		((Inverter->AlarmReg & 0x0080)!=0);	// #bit 7
		Inverter->Alarm.OverTorque =		((Inverter->AlarmReg & 0x0100)!=0);	// #bit 8
		Inverter->Alarm.BrakResOverLoad =	((Inverter->AlarmReg & 0x0200)!=0);	// #bit 9
		Inverter->Alarm.CumOpHours =		((Inverter->AlarmReg & 0x0400)!=0);	// #bit 10
		Inverter->Alarm.OptionCommAlm =		((Inverter->AlarmReg & 0x0800)!=0);	// #bit 11
		Inverter->Alarm.SerialCommAlm =		((Inverter->AlarmReg & 0x1000)!=0);	// #bit 12
		Inverter->Alarm.MainVoltageErr =	((Inverter->AlarmReg & 0x2000)!=0);	// #bit 13	Display MOFF flickering
		Inverter->Alarm.ForceDecelarating =	((Inverter->AlarmReg & 0x4000)!=0);	// #bit 14
		Inverter->Alarm.ForceStop =			((Inverter->AlarmReg & 0x8000)!=0);	// #bit 15
*/

		Inverter->AlarmReg = InverterErrorCode;
				
		// aggiorno il comando di run
		// Modifica v4.4.117: Gestione Run/Stop Inverter
		if(!Inverter->OffLine)	
		{
			//if(Inverter->Req_Freq>0)
			if(CompNumber->FreqPompa_Acqua>0)
				/*
#if(K_Abil_Double_Pump==1)
				if(EngineBox[0].VirtualOut.TrigPump1 || EngineBox[0].VirtualOut.TrigPump2)
					Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_STOP_AND_COAST, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Lo Mando in Free run
				else
#endif
				*/
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_RUN_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Lo mando in RUN
			else 
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_STOP_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Lo mando in STOP
                
		}
		
		// scrivo la frequenza da impostare
		if(!Inverter->OffLine)	
        {
			Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FREQUENCY_SET, CompNumber->FreqPompa_Acqua, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);
        }

	}

    if(Inverter->PermanentOffLine)
        Inverter->AlarmReg = 90;	//Forzo a 90 per segnalare sul touch allarme inverter offline
	
	
    TimerPersError->Enable = (CntMaxErrorPers > 0);
    
    if(!TimerPersError->Enable)
    {
        TimerPersError->Value =0;
        TimerPersError->TimeOut =0;
    }
    
    if(TimerPersError->TimeOut)
    {
        CntMaxErrorPers =0;
        TimerPersError->Value =0;
        TimerPersError->TimeOut =0;
    }
    
	// Invio comando di reset Fault
	// Solo se � scattato uno dei Fault specificati...
	if(InverterErrorCode!=0)
	{
        if(CntMaxErrorPers>=K_Max_Error_Inverter_Continuous)
        {
            EngineBox[0].MaxErrorPersPump = 1;
            CntMaxErrorPers = 0;
        }
        
		if(((InverterErrorCode==K_VAL_INVERTER_ERROR_CODE_Err4 || InverterErrorCode==K_VAL_INVERTER_ERROR_CODE_Err5 || InverterErrorCode==K_VAL_INVERTER_ERROR_CODE_Sout || 
            InverterErrorCode==K_VAL_INVERTER_ERROR_CODE_E21 || InverterErrorCode==K_VAL_INVERTER_ERROR_CODE_E26 || InverterErrorCode==K_VAL_INVERTER_ERROR_CODE_E39) && (CompNumber->Out_Inverter==0) && !EngineBox[0].MaxErrorPersPump) || (EngineBox[0].In.ClearErrorReq))		// Appena arrivo a zero con la freq. resetto l'errore...
        {
			Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_RESET_FLT_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Resetto flag di "Fault Reset -> registro 0xFA00
            if(InverterErrorCode!=K_VAL_INVERTER_ERROR_CODE_Err5)
                CntMaxErrorPers +=1;
            InverterErrorCode=0;            
        }        
	}
/*
	// Invio comando di reset Fault
	// Solo se � scattato uno dei Fault specificati...
	if(InverterErrorCode==K_VAL_INVERTER_ERROR_CODE_Err4 || InverterErrorCode==K_VAL_INVERTER_ERROR_CODE_Err5 || InverterErrorCode==K_VAL_INVERTER_ERROR_CODE_Sout || 
	   InverterErrorCode==K_VAL_INVERTER_ERROR_CODE_E21 || InverterErrorCode==K_VAL_INVERTER_ERROR_CODE_E26 || InverterErrorCode==K_VAL_INVERTER_ERROR_CODE_E39)
	{
		Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_RESET_FLT_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Resetto flag di "Fault Reset -> registro 0xFA00		
		//EngineBox[??].GoShutDwComp = 1;			
	}	
	
	
	// Se � scattato un qualsiasi errore sull'inverter 
	if(InverterErrorCode!=0)
	{
		if((EngineBox[0].In.ClearErrorReq))		// ed � stato premuto il tasto di reset fault sul Touch....
		{										// invio il comando di reset Fault sull'inverter
			InverterErrorCode=0;
			Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_RESET_FLT_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Resetto flag di "Fault Reset -> registro 0xFA00				
		}
	}
*/
	EngineBox[0].PersErr.PumpMOff = Inverter->In_Voltage==0 && (Inverter->Status.UnderVoltage);// || Inverter->Alarm.MainVoltageErr);
	
	EngineBox[0].PersErr.PumpInverterFault = InverterErrorCode!=0 || Inverter->PermanentOffLine || EngineBox[0].PersErr.PumpMOff || EngineBox[0].MaxErrorPersPump;

/*DEBUG       
    if(Comp_Inverter[2].OffLine)
        EngineBox[0].GoShutDwComp = 1;
*/
    
	
//---------------------------------------------------------------------------------------------------------------------    
// Gestione flag "PermanentOffLine" Inverter
//---------------------------------------------------------------------------------------------------------------------        
    
	if(Inverter->OffLine == 1)
	{
		if(Inverter->PermanentOffLine == 0)
		{
			CntErrUnit++;
			if(CntErrUnit> k_MAX_Cnt_Err_Inverter)
				Inverter->PermanentOffLine = 1;
			else
				Inverter->OffLine = 0;	// Slave 1 (primario)
		}
	}
	else
	{
    // Inizializzazione Inverter Pompa
    //----------------------------------
	if(Inverter->PermanentOffLine == 1)
    {
#if (K_AbilInverterInit==1)    
#if (K_AbilPumpInverter==1)
    	InitInverterPump(&Comp_Inverter[2], K_PumpModel_Selection, Add_Pump_Inverter1);
#endif            
#endif            
        EngineBox[0].MaxErrorPersPump = 0;
        CntMaxErrorPers =0;
        EngineBox[0].PersErr.PumpInverterFault = 0;
    }        
		CntErrUnit = 0;
		Inverter->PermanentOffLine = 0;
	}	
	
/* pensare a quando inviare il comando di reset Fault !!!!!!!!!!!!!!!!*/
		// Invio comando di reset Fault
	/*
	if((!Inverter->OffLine && LastOffline)) //|| Inverter->Status.EmergencyStp)
	{
		Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_RESET_FLT_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Resetto flag di "Fault Reset -> registro 0xFA00
		Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_STATUS, 1, MaxModReadWait, MaxModReadRetries);											// NON TOGLIERE!! Il comando precedente non da risposta e quindi questo comando "REG_INVERTER_STATUS" serve a far rispondere l'inverter e a generare un "!Offline"
		LastOffline = 0;
	}
	if(Inverter->OffLine)
		LastOffline = 1;	
	*/
}


//-----------------------------------------------------------------------------------------------
// Aggiorna (Lettura/Scrittura) alcuni parametri dell'inverter Compressore 
//
// Addr: 2 = Compressore primario
// Addr: 3 = Compressore parallelo 
//
// Parametri Inverter VF-S15 o VF-PS1 da settare per poter comunicare in ModBus:
//       VFS15      VFPS1
// CMOD: 2          2       -> Modalit� comando RS485
// FMOD: 4          5       -> Modalit� impostazione frequenza RS485
// F800: 5          2       -> Baud Rate RS485 38400 bps
// F801: 0          0       -> Parit� RS485 nessuna
// F802: 0..247     0.247   -> Indirizzo Inverter RS485
// F829: 1          1       -> Protocollo ModBus RTU
// 
//-----------------------------------------------------------------------------------------------
void RefreshInverterComp(int pointer, TypEngineBox * CompNumber, TypInverter * Inverter, int CompAddr, TypTimer * TimerPersError)
{	
	static int LastOffline[2] = {1,1};
	static int sched[2] = {0,0};
	static int InverterErrorCode[2] = {0,0};
    static int CntErrUnit[2] = {0,0};
    static int CntMaxErrorPers[2] = {0,0};
           int CorrectPercValueAbsorption_Inv = 0;
           int K_Max_Error_Inverter_Continuous = 3;
           	
    Change_Protocol_BUSS(K_ModBus_Baud38400);                                         // Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)

    Inverter->Req_Freq = DAC2FreqInverterComp(CompNumber->Out_Inverter);	// Converto il valore Out_inverter in scala DAC_1 (0..255 -> 0..10V x Inverter compressore)
                                                                                // in valore frequenza da inviare via ModBus
    Inverter->Max_Req_Freq = DAC2FreqInverterComp(EngineBox[0].Inverter_Max_Power_Out);					// Converto il valore da scala DAC_1 (0..255 -> 0..10V x Inverter compressore)
    
	if(CompNumber->Temperature.Compressor_Output < K_No_Braking_Temp-200)
		CompNumber->HeatingEngineStart = 1;
	else if(CompNumber->Temperature.Compressor_Output > K_No_Braking_Temp)
		CompNumber->HeatingEngineStart = 0;
	
/*	
	Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_STATUS, 1, MaxModReadWait, MaxModReadRetries);
	if(!Inverter->OffLine)	
		Inverter->StatusReg = Mod_LastRx.Data;
*/
    if(Inverter->PermanentOffLine)	
    {
        EngineBox[0].DisableModComErrorCounter=1;
        Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_INPUT_POWER, 1, MaxModReadWait, 1);	
        EngineBox[0].DisableModComErrorCounter=0;
    }
    else
        Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_INPUT_POWER, 1, MaxModReadWait, MaxModReadRetries);	

    //if(Inverter->OffLine==1)	
    if(Inverter->PermanentOffLine)	
        Inverter->Out_Power = 0;  //ArgoInverter

    if(!Inverter->OffLine)	
        //Inverter->In_Power = Mod_LastRx.Data;
        Inverter->Out_Power = Mod_LastRx.Data;  //ArgoPower

//Sommo correzione percentuale valori assorbimento    
#if(K_Pec_Sum_Inverter_Comp>0)
    if(Inverter->Out_Power>0)
    {
        CorrectPercValueAbsorption_Inv = (int)(((float)Inverter->Out_Power * (float)K_Pec_Sum_Inverter_Comp) / 100.0);
        
        Inverter->Out_Power = Inverter->Out_Power + CorrectPercValueAbsorption_Inv;
    }
#endif
/*    
    Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_OUTPUT_POWER, 1, MaxModReadWait, MaxModReadRetries);	
    if(!Inverter->OffLine)	
        Inverter->Out_Power = Mod_LastRx.Data;
*/
	if(!Inverter->OffLine)													
	{        
        Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_IN_VOLTAGE, 1, MaxModReadWait, MaxModReadRetries);
		if(!Inverter->OffLine)	
			Inverter->In_Voltage = Mod_LastRx.Data;
        
		/*
		Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_ALARM, 1, MaxModReadWait, MaxModReadRetries);	
		if(!Inverter->OffLine)
			Inverter->AlarmReg = Mod_LastRx.Data;		
        */

		Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_TRIP_CODE_MONITOR, 1, MaxModReadWait, MaxModReadRetries);	
		if(!Inverter->OffLine)	
			InverterErrorCode[pointer] = Mod_LastRx.Data;	
        
        Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_OUT_CURRENT, 1, MaxModReadWait, MaxModReadRetries);
        if(!Inverter->OffLine)	
            Inverter->Out_Current = Mod_LastRx.Data;        

		switch(sched[pointer])
		{
			case 0:
			Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_OUT_FREQ, 1, MaxModReadWait, MaxModReadRetries);	
			if(!Inverter->OffLine)	
				Inverter->Out_Freq = Mod_LastRx.Data;
			break;
			//case 1:			
			//Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_OUT_CURRENT, 1, MaxModReadWait, MaxModReadRetries);
			//if(!Inverter->OffLine)	
			//	Inverter->Out_Current = Mod_LastRx.Data;
			//break;
			case 2:
			Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_OUT_VOLTAGE, 1, MaxModReadWait, MaxModReadRetries);	
			if(!Inverter->OffLine)	
				Inverter->Out_Voltage = Mod_LastRx.Data;
			break;
/*
			case 3:
			Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_INPUT_POWER, 1, MaxModReadWait, MaxModReadRetries);	
			if(!Inverter->OffLine)	
				Inverter->In_Power = Mod_LastRx.Data;
			break;
*/
  			/*
			case 4:
            Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_ALARM, 1, MaxModReadWait, MaxModReadRetries);	
            if(!Inverter->OffLine)
                Inverter->AlarmReg = Mod_LastRx.Data;
			break;
			*/
			case 5:
            Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_STATUS, 1, MaxModReadWait, MaxModReadRetries);
            if(!Inverter->OffLine)	
                Inverter->StatusReg = Mod_LastRx.Data;
			break;	
		}
	
		sched[pointer]++;
		if(sched[pointer]>5)
			sched[pointer]=0;
	

		// aggiorno i bit 
		Inverter->Status.FailFL = 			((Inverter->StatusReg & 0x0001)!=0);	// #bit 0
		Inverter->Status.Fail = 			((Inverter->StatusReg & 0x0002)!=0);	// #bit 1
		Inverter->Status.Alarm =			((Inverter->StatusReg & 0x0004)!=0);	// #bit 2
		Inverter->Status.UnderVoltage =		((Inverter->StatusReg & 0x0008)!=0);	// #bit 3
		Inverter->Status.MotorSection =		((Inverter->StatusReg & 0x0010)!=0);	// #bit 4
		Inverter->Status.PID_Control =		((Inverter->StatusReg & 0x0020)!=0);	// #bit 5
		Inverter->Status.AccDecPatt =		((Inverter->StatusReg & 0x0040)!=0);	// #bit 6
		Inverter->Status.DC_Bracking =		((Inverter->StatusReg & 0x0080)!=0);	// #bit 7
		Inverter->Status.JogRun = 			((Inverter->StatusReg & 0x0100)!=0);	// #bit 8
		Inverter->Status.FwRvRun = 			((Inverter->StatusReg & 0x0200)!=0);	// #bit 9
		Inverter->Status.RunStop = 			((Inverter->StatusReg & 0x0400)!=0);	// #bit 10
		Inverter->Status.CoastStop = 		((Inverter->StatusReg & 0x0800)!=0);	// #bit 11
		Inverter->Status.EmergencyStp = 	((Inverter->StatusReg & 0x1000)!=0);	// #bit 12
		Inverter->Status.StandBy_ST = 		((Inverter->StatusReg & 0x2000)!=0);	// #bit 13
		Inverter->Status.StandBy = 			((Inverter->StatusReg & 0x4000)!=0);	// #bit 14
/*
		Inverter->Alarm.OverCurrent =		((Inverter->AlarmReg & 0x0001)!=0);	// #bit 0
		Inverter->Alarm.InverterOverLoad =	((Inverter->AlarmReg & 0x0002)!=0);	// #bit 1
		Inverter->Alarm.MotorOverLoad =		((Inverter->AlarmReg & 0x0004)!=0);	// #bit 2
		Inverter->Alarm.OverHeat =			((Inverter->AlarmReg & 0x0008)!=0);	// #bit 3
		Inverter->Alarm.OverVoltage =		((Inverter->AlarmReg & 0x0010)!=0);	// #bit 4
		Inverter->Alarm.UnderVoltage =		((Inverter->AlarmReg & 0x0020)!=0);	// #bit 5
		Inverter->Alarm.MainOverLoad =		((Inverter->AlarmReg & 0x0040)!=0);	// #bit 6
		Inverter->Alarm.LowCurrent =		((Inverter->AlarmReg & 0x0080)!=0);	// #bit 7
		Inverter->Alarm.OverTorque =		((Inverter->AlarmReg & 0x0100)!=0);	// #bit 8
		Inverter->Alarm.BrakResOverLoad =	((Inverter->AlarmReg & 0x0200)!=0);	// #bit 9
		Inverter->Alarm.CumOpHours =		((Inverter->AlarmReg & 0x0400)!=0);	// #bit 10
		Inverter->Alarm.OptionCommAlm =		((Inverter->AlarmReg & 0x0800)!=0);	// #bit 11
		Inverter->Alarm.SerialCommAlm =		((Inverter->AlarmReg & 0x1000)!=0);	// #bit 12
		Inverter->Alarm.MainVoltageErr =	((Inverter->AlarmReg & 0x2000)!=0);	// #bit 13	Display MOFF flickering
		Inverter->Alarm.ForceDecelarating =	((Inverter->AlarmReg & 0x4000)!=0);	// #bit 14
		Inverter->Alarm.ForceStop =			((Inverter->AlarmReg & 0x8000)!=0);	// #bit 15
*/

		Inverter->AlarmReg = InverterErrorCode[pointer];
		
		// aggiorno il comando di run
		// Modifica v4.4.117: Gestione Run/Stop Inverter
		if(!Inverter->OffLine)	
		{
			//if(Inverter->Req_Freq>0)
			if(CompNumber->Out_Inverter>0)		
			{
#if (K_EnableBrakeCurrent==1)				
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_TIME, Inverter->StepOutCurrentWarningWork, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);
#endif	
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_RUN_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Lo mando in RUN
			}
			else 
            {		
#if (K_EnableBrakeCurrent==1)
				if(CompNumber->HeatingEngineStart)
				{
					Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_TIME, Inverter->StepOutCurrentWarningDB, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);						
					Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_BREAK_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);
				}   
				else
#endif					
					Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_STOP_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Lo mando in STOP				
			}
						
/*				
                if(!EngineBox[0].HeatingEngineStart)
                {
                    if(((EngineBox[0].SplitPowerOnCnt>0) && !CompNumber->GoRiscaldamento && !CompNumber->GoRaffrescamento && !CompNumber->PersErr.GlobalStatusFlag && ((EngineBox[0].DefMode == CoreRiscaldamento) || (EngineBox[0].DefMode == CoreRaffrescamento))) || // Vado in Breaking Current se ho almeno uno split acceso su touch e il motore si � spento per SetPoint soddisfatto o per errore NON permanente;
							(EngineBox[0].CompressorIsMoreHot && ((EngineBox[0].SplitPowerOnCnt==0) || CompNumber->PersErr.GlobalStatusFlag)))							// Vado in Breaking Curent con Post RIscaldamento (per spegnimento volontario dell'unit� oppure per spegnimento in caso di errore permanente)
					{	
						if(CompNumber->Temperature.Compressor_Output<K_No_Braking_Temp-100)
						{
							Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_TIME, Inverter->StepOutCurrentWarningDB, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);						
							Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_BREAK_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);
						}
						else if(CompNumber->Temperature.Compressor_Output>K_No_Braking_Temp)
						{
							Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_STOP_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Lo mando in STOP
						}
					}
                    else
                        Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_STOP_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Lo mando in STOP
                }
*/				     
                
		}
		
		// scrivo la frequenza da impostare
		if(!Inverter->OffLine)	
        {
			Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FREQUENCY_SET, Inverter->Req_Freq, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);
        }

	}
	
    if(Inverter->PermanentOffLine)
        Inverter->AlarmReg = 90;	//Forzo a 90 per segnalare sul touch allarme inverter offline
	
    TimerPersError->Enable = (CntMaxErrorPers[pointer] > 0);
    
    if(!TimerPersError->Enable)
    {
        TimerPersError->Value =0;
        TimerPersError->TimeOut =0;
    }
    
    if(TimerPersError->TimeOut)
    {
        CntMaxErrorPers[pointer] =0;
        TimerPersError->Value =0;
        TimerPersError->TimeOut =0;
    }
    
	// Invio comando di reset Fault
	// Solo se � scattato uno dei Fault specificati...
	if(InverterErrorCode[pointer]!=0)
	{
        if(CntMaxErrorPers[pointer]>=K_Max_Error_Inverter_Continuous)
        {
            EngineBox[pointer].MaxErrorPersComp = 1;
            CntMaxErrorPers[pointer] = 0;
        }
        
		if(((InverterErrorCode[pointer]==K_VAL_INVERTER_ERROR_CODE_Err4 || InverterErrorCode[pointer]==K_VAL_INVERTER_ERROR_CODE_Err5 || InverterErrorCode[pointer]==K_VAL_INVERTER_ERROR_CODE_Sout || 
            InverterErrorCode[pointer]==K_VAL_INVERTER_ERROR_CODE_E21 || InverterErrorCode[pointer]==K_VAL_INVERTER_ERROR_CODE_E26 || InverterErrorCode[pointer]==K_VAL_INVERTER_ERROR_CODE_E39 || InverterErrorCode[pointer]==K_VAL_INVERTER_ERROR_CODE_UP1) && (CompNumber->Out_Inverter==0) && !EngineBox[pointer].MaxErrorPersComp) || (EngineBox[0].In.ClearErrorReq))		// Appena arrivo a zero con la freq. resetto l'errore...
        {
            if(CompNumber->Out_Inverter==0)		// Appena arrivo a zero con la freq. resetto l'errore...
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_RESET_FLT_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Resetto flag di "Fault Reset -> registro 0xFA00
            if(InverterErrorCode[pointer]!=K_VAL_INVERTER_ERROR_CODE_Err5)
                CntMaxErrorPers[pointer] +=1;
            InverterErrorCode[pointer]=0;	
            if(EngineBox[pointer].EngineIsOn)
                EngineBox[pointer].GoShutDwComp = 1;            
        }        
	}
	
	EngineBox[pointer].PersErr.CompMOff = Inverter->In_Voltage==0 && (Inverter->Status.UnderVoltage);// || Inverter->Alarm.MainVoltageErr);
	
	EngineBox[pointer].PersErr.CompInverterFault = InverterErrorCode[pointer]!=0 || Inverter->PermanentOffLine || EngineBox[pointer].PersErr.CompMOff || EngineBox[pointer].MaxErrorPersComp;
	if(EngineBox[pointer].PersErr.CompInverterFault && EngineBox[pointer].EngineIsOn)			// controllo se il compressore sta girando 	
    {
		EngineBox[pointer].GoShutDwComp = 1;
        if(EngineBox[pointer].Out_Inverter > Min_Speed)
            EngineBox[pointer].Out_Inverter = Min_Speed;
    }
    
//---------------------------------------------------------------------------------------------------------------------    
// Gestione flag "PermanentOffLine" Inverter
//---------------------------------------------------------------------------------------------------------------------        
    
	if(Inverter->OffLine == 1)
	{
		if(Inverter->PermanentOffLine == 0)
		{
			CntErrUnit[pointer]++;
			if(CntErrUnit[pointer]> k_MAX_Cnt_Err_Inverter)
				Inverter->PermanentOffLine = 1;
			else
				Inverter->OffLine = 0;	// Slave 1 (primario)
		}
	}
	else
	{
        if(Inverter->PermanentOffLine == 1)
        {
#if (K_AbilInverterInit==1)    
    //----------------------------------            
    // Inizializzazioni Inverters Compressori
    //----------------------------------
            if(CompAddr==Add_Comp_Inverter1)
                InitInverterComp(&Comp_Inverter[0], K_MotorModel_Selection1, Add_Comp_Inverter1);
#if (K_AbilCompressor2==1)	// Se � presente il secondo inverter compressore....
#if(K_OnlyOneInverterComp2==0)
            if(CompAddr==Add_Comp_Inverter2)
                InitInverterComp(&Comp_Inverter[1], K_MotorModel_Selection2, Add_Comp_Inverter2);    
#endif
#endif            
#endif

#if(K_OnlyOneInverterComp2==1)
            EngineBox[0].MaxErrorPersComp = 0;
            CntMaxErrorPers[0] = 0;
            EngineBox[0].PersErr.CompInverterFault = 0;

            EngineBox[1].MaxErrorPersComp = 0;
            CntMaxErrorPers[1] = 0;
            EngineBox[1].PersErr.CompInverterFault = 0;
#else
            if(CompAddr==Add_Comp_Inverter1)
            {
                EngineBox[0].MaxErrorPersComp = 0;
                CntMaxErrorPers[0] = 0;
                EngineBox[0].PersErr.CompInverterFault = 0;
            }
            if(CompAddr==Add_Comp_Inverter2)
            {
                EngineBox[1].MaxErrorPersComp = 0;
                CntMaxErrorPers[1] = 0;
                EngineBox[1].PersErr.CompInverterFault = 0;
            }
#endif
        }
            CntErrUnit[pointer] = 0;
            Inverter->PermanentOffLine = 0;
	}	
	
/* pensare a quando inviare il comando di reset Fault !!!!!!!!!!!!!!!!*/
	/*
	// Invio comando di reset Fault
	if((!Inverter->OffLine && LastOffline[pointer])) //|| Inverter->Status.EmergencyStp)
	{
		Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_RESET_FLT_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Resetto flag di "Fault Reset -> registro 0xFA00
		Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_STATUS, 1, MaxModReadWait, MaxModReadRetries);											// NON TOGLIERE!! Il comando precedente non da risposta e quindi questo comando "REG_INVERTER_STATUS" serve a far rispondere l'inverter e a generare un "!Offline"
		LastOffline[pointer]  = 0;
	}
	if(Inverter->OffLine)
		LastOffline[pointer] = 1;
	*/
}



#if (K_AbilInverterInit==1)
	#if (K_AbilPumpInverter==1)
void InitInverterPump(TypInverter * Inverter, int MotorType, int CompAddr)
{
    Change_Protocol_BUSS(K_ModBus_Baud38400);                             // Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)    
    
	Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_STATUS, 1, MaxModReadWait, MaxModReadRetries);
	
	if(!Inverter->OffLine)													
	{	
        
        
        switch(MotorType)
        {
            case K_PumpType_MPH_441_FGACE5:
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MAX_FREQUENCY,        K_PUMP_MPH_441_FGACE5_INVERTER_MAX_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      		
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UPPER_LIM_FREQ,       K_PUMP_MPH_441_FGACE5_INVERTER_UPPER_LIM_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      // UL	12	Upper limit frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQUENCY,       K_PUMP_MPH_441_FGACE5_INVERTER_BASE_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vL	14	Base frequency 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_VF_CONTROL_MODE,      K_PUMP_MPH_441_FGACE5_INVERTER_VF_CONTROL_MODE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Pt	15	V/F control mode selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_START_FREQ,      K_PUMP_MPH_441_FGACE5_INVERTER_AUTO_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);   // F241	Frequenza avviamento automatico
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PWM_CARRIER,          K_PUMP_MPH_441_FGACE5_INVERTER_PWM_CARRIER, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz					
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CAPACITY, K_PUMP_MPH_441_FGACE5_INVERTER_MOTOR_RATED_CAPACITY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F405	405	Motor rated capacity
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQ_VOLTAGE,    K_PUMP_MPH_441_FGACE5_INVERTER_BASE_FREQ_VOLTAGE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vLv	409	Base frequency voltage 1		
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CURRENT,  K_PUMP_MPH_441_FGACE5_INVERTER_MOTOR_RATED_CURRENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F415	415	Motor rated current				
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_SPEED,    K_PUMP_MPH_441_FGACE5_INVERTER_MOTOR_RATED_SPEED, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F417	417	Motor rated speed								
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_TIMEOUT_COMUNICATION,   K_PUMP_DEFAULT_INVERTER_TIMEOUT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F803 Time-out tempo di comunicazione		Unit: 0.1s
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACTION_TMT_COMUNICATION,K_PUMP_DEFAULT_INVERTER_ACTION_TMT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto										
				//Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STALL_PREVENTION_L1,  K_PUMP_MPH_441_FGACE5_INVERTER_INVERTER_STALL_PREVENTION_L1, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F601	601	Stall prevention level 1
                if(K_Supply_Inverter==566)
                    Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_RIL_FASE_SELECTION,   K_PUMP_DEFAULT_INVERTER_RIL_FASE_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F803 Time-out tempo di comunicazione		Unit: 0.1s
                break;
			
            case K_PumpType_MPH_453_FGACE5:           
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MAX_FREQUENCY,        K_PUMP_MPH_453_FGACE5_INVERTER_MAX_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      		
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UPPER_LIM_FREQ,       K_PUMP_MPH_453_FGACE5_INVERTER_UPPER_LIM_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      // UL	12	Upper limit frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQUENCY,       K_PUMP_MPH_453_FGACE5_INVERTER_BASE_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vL	14	Base frequency 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_VF_CONTROL_MODE,      K_PUMP_MPH_453_FGACE5_INVERTER_VF_CONTROL_MODE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Pt	15	V/F control mode selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_START_FREQ,      K_PUMP_MPH_453_FGACE5_INVERTER_AUTO_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);   // F241	Frequenza avviamento automatico
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PWM_CARRIER,          K_PUMP_MPH_453_FGACE5_INVERTER_PWM_CARRIER, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz					
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CAPACITY, K_PUMP_MPH_453_FGACE5_INVERTER_MOTOR_RATED_CAPACITY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F405	405	Motor rated capacity
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQ_VOLTAGE,    K_PUMP_MPH_453_FGACE5_INVERTER_BASE_FREQ_VOLTAGE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vLv	409	Base frequency voltage 1		
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CURRENT,  K_PUMP_MPH_453_FGACE5_INVERTER_MOTOR_RATED_CURRENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F415	415	Motor rated current				
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_SPEED,    K_PUMP_MPH_453_FGACE5_INVERTER_MOTOR_RATED_SPEED, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F417	417	Motor rated speed								
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_TIMEOUT_COMUNICATION,   K_PUMP_DEFAULT_INVERTER_TIMEOUT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F803 Time-out tempo di comunicazione		Unit: 0.1s
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACTION_TMT_COMUNICATION,K_PUMP_DEFAULT_INVERTER_ACTION_TMT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto										
				//Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STALL_PREVENTION_L1,  K_PUMP_MPH_453_FGACE5_INVERTER_INVERTER_STALL_PREVENTION_L1, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F601	601	Stall prevention level 1
                if(K_Supply_Inverter==566)
                    Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_RIL_FASE_SELECTION,   K_PUMP_DEFAULT_INVERTER_RIL_FASE_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F803 Time-out tempo di comunicazione		Unit: 0.1s
                break;

            case K_PumpType_DEFAULT:
            default:           
                //Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_TIME_ACC,        K_PUMP_DEFAULT_INVERTER_TIME_ACC, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      		
                //Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_TIME_DEC,        K_PUMP_DEFAULT_INVERTER_TIME_DEC, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      		
				
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MAX_FREQUENCY,        K_PUMP_DEFAULT_INVERTER_MAX_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      		
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UPPER_LIM_FREQ,       K_PUMP_DEFAULT_INVERTER_UPPER_LIM_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      // UL	12	Upper limit frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQUENCY,       K_PUMP_DEFAULT_INVERTER_BASE_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vL	14	Base frequency 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_VF_CONTROL_MODE,      K_PUMP_DEFAULT_INVERTER_VF_CONTROL_MODE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Pt	15	V/F control mode selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_START_FREQ,      K_PUMP_DEFAULT_INVERTER_AUTO_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);   // F241	Frequenza avviamento automatico
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PWM_CARRIER,          K_PUMP_DEFAULT_INVERTER_PWM_CARRIER, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz					
                //Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CAPACITY, K_PUMP_DEFAULT_INVERTER_MOTOR_RATED_CAPACITY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F405	405	Motor rated capacity
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQ_VOLTAGE,    K_PUMP_DEFAULT_INVERTER_BASE_FREQ_VOLTAGE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vLv	409	Base frequency voltage 1		
                //Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CURRENT,  K_PUMP_DEFAULT_INVERTER_MOTOR_RATED_CURRENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F415	415	Motor rated current				
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_SPEED,    K_PUMP_DEFAULT_INVERTER_MOTOR_RATED_SPEED, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F417	417	Motor rated speed								
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_TIMEOUT_COMUNICATION,   K_PUMP_DEFAULT_INVERTER_TIMEOUT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F803 Time-out tempo di comunicazione		Unit: 0.1s
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACTION_TMT_COMUNICATION,K_PUMP_DEFAULT_INVERTER_ACTION_TMT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto										
				//Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STALL_PREVENTION_L1,  K_PUMP_DEFAULT_INVERTER_INVERTER_STALL_PREVENTION_L1, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F601	601	Stall prevention level 1
                if(K_Supply_Inverter==566)
                    Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_RIL_FASE_SELECTION,   K_PUMP_DEFAULT_INVERTER_RIL_FASE_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F803 Time-out tempo di comunicazione		Unit: 0.1s
                break;
        } 
    }
}
	#endif



//-----------------------------------------------------------------------------------------------
// Inizializza i parametri dell'inverter (Scrittura) per il corretto funzionamento del Compressore 
// Addr: 2 = Compressore primario
// Addr: 3 = Compressore parallelo 
//
// Parametri Inverter VF-S15 o VF-PS1 da settare per poter comunicare in ModBus:
//       VFS15      VFPS1
// CMOD: 2          2       -> Modalit� comando RS485
// FMOD: 4          5       -> Modalit� impostazione frequenza RS485
// F800: 5          2       -> Baud Rate RS485 38400 bps
// F801: 0          0       -> Parit� RS485 nessuna
// F802: 0..247     0.247   -> Indirizzo Inverter RS485
// F829: 1          1       -> Protocollo ModBus RTU
// 
//-----------------------------------------------------------------------------------------------

void InitInverterComp(TypInverter * Inverter, int MotorType, int CompAddr)
{	
    Change_Protocol_BUSS(K_ModBus_Baud38400);                             // Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)
    
	Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_STATUS, 1, MaxModReadWait, MaxModReadRetries);
	
	if(!Inverter->OffLine)													
	{	
        switch(MotorType)
        {
            case K_MotorType_DA270A2F20L:
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MAX_FREQUENCY,        K_DA270A2F20L_INVERTER_MAX_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UPPER_LIM_FREQ,       K_DA270A2F20L_INVERTER_UPPER_LIM_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      // UL	12	Upper limit frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQUENCY,       K_DA270A2F20L_INVERTER_BASE_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vL	14	Base frequency 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_VF_CONTROL_MODE,      K_DA270A2F20L_INVERTER_VF_CONTROL_MODE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Pt	15	V/F control mode selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_START_FREQ,      K_DA270A2F20L_INVERTER_AUTO_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);   // F241	Frequenza avviamento automatico
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_START_FREQ,     K_DA270A2F20L_INVERTER_BRAKE_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F250	250	DC braking starting frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_CURRENT_SET,    K_DA270A2F20L_INVERTER_BRAKE_CURRENT_SET, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Corrente di frenatura in % della corrente nominale inverter (0..100%)
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_TIME,           K_DA270A2F20L_INVERTER_BRAKE_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F252	252	DC braking time		
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PWM_CARRIER,  K_DA270A2F20L_INVERTER_PWM_CARRIER, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz	
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTORESTART_CONTROL,  K_DA270A2F20L_INVERTER_AUTORESTART_CONTROL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F301	301	Auto-restart control selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_SUPPLY_VOLTAGE_CORRECT,  K_DA270A2F20L_INVERTER_SUPPLY_VOLTAGE_CORRECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F307 Supply voltage correction Default=2 Unit: 0,1,2,3
#if (K_AUTOTUNING_OFF == 1)
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTOTUNING,  K_DA270A2F20L_INVERTER_AUTOTUNING, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F400	400	Autotuning						Unit: 1				
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_TORQUE_BOOST_VAL,  K_DA270A2F20L_INVERTER_AUTO_TORQUE_BOOST_VAL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F402	402	Automatic torque boost value			
#endif                
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CAPACITY, K_DA270A2F20L_INVERTER_MOTOR_RATED_CAPACITY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F405	405	Motor rated capacity
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQ_VOLTAGE,    K_DA270A2F20L_INVERTER_BASE_FREQ_VOLTAGE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vLv	409	Base frequency voltage 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CURRENT,  K_DA270A2F20L_INVERTER_MOTOR_RATED_CURRENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F415	415	Motor rated current
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_SPEED,    K_DA270A2F20L_INVERTER_MOTOR_RATED_SPEED, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F417	417	Motor rated speed
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF2,K_DA270A2F20L_INVERTER_MOTOR_SPECIFIC_COEFF2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F458	458	Motor specific coefficient 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_LOAD_INERTIA_MOMENT,  K_DA270A2F20L_INVERTER_LOAD_INERTIA_MOMENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F459	459	Load inertia moment ratio
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF3,K_DA270A2F20L_INVERTER_MOTOR_SPECIFIC_COEFF3, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F460	460	Motor specific coefficient 3
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF9,K_DA270A2F20L_INVERTER_MOTOR_SPECIFIC_COEFF9, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F495	495	Motor specific coefficient 9
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACCELERATION_TIME2,   K_DA270A2F20L_INVERTER_ACCELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F500	500	Acceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DECELERATION_TIME2,   K_DA270A2F20L_INVERTER_DECELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F501	501	Deceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACC_DEC_1e2_SW_FREQ,  K_DA270A2F20L_INVERTER_ACC_DEC_1e2_SW_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F505	505	Acceleration/deceleration 1 and 2 switching frequency
                //Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STALL_PREVENTION_L1,  K_DA270A2F20L_INVERTER_STALL_PREVENTION_L1, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F601	601	Stall prevention level 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_RIL_FASE_SELECTION,   K_DA270A2F20L_INVERTER_RIL_FASE_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F608	608 0 Selezione rilevamento fase ingresso
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UNIT_SELECTION,       K_DA270A2F20L_INVERTER_UNIT_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // f701 Selezione unit� corrente/tensione Dspu		change parameters from % to nominal value	1   ??????
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_TIMEOUT_COMUNICATION,   K_DA270A2F20L_INVERTER_TIMEOUT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F803 Time-out tempo di comunicazione		Unit: 0.1s
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACTION_TMT_COMUNICATION,K_DA270A2F20L_INVERTER_ACTION_TMT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto						
//                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9P, K_DA270A2F20L_INVERTER_FACTORY_SPEC_COEFF9P, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F908	908	Factory specific coefficient 9P
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9D, K_DA270A2F20L_INVERTER_FACTORY_SPEC_COEFF9D, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F909	909	Factory specific coefficient 9D
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_CURR_LEV, K_DA270A2F20L_INVERTER_STEPOUT_DET_CURR_LEV, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F910	910	Step-out detection current level
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_TIME,     K_DA270A2F20L_INVERTER_STEPOUT_DET_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F911	911	Step-out detection time
#if (K_AUTOTUNING_OFF == 1)		
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_QAXIS_INDUCTANCE,     K_DA270A2F20L_INVERTER_QAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DAXIS_INDUCTANCE,     K_DA270A2F20L_INVERTER_DAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry			
#endif
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_UNDERVOLTAGE_TRIP_ALARM_SEL, K_GENERAL_INVERTER_UNDERVOLTAGE_ERROR, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            

                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PM_CONTROL_SELECTION, K_DA270A2F20L_INVERTER_PM_CONTROL_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F915	915	PM control selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9L, K_DA270A2F20L_INVERTER_FACTORY_SPEC_COEFF9L, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            
                
				Inverter->StepOutCurrentWarningDB = K_GENERAL_INVERTER_STEPOUT_DISABLE;
				Inverter->StepOutCurrentWarningWork = K_GENERAL_INVERTER_STEPOUT_ENABLE;		
            break;
            case K_MotorType_DA550A3F10M:
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MAX_FREQUENCY,        K_DA550A3F10M_INVERTER_MAX_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UPPER_LIM_FREQ,       K_DA550A3F10M_INVERTER_UPPER_LIM_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      // UL	12	Upper limit frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQUENCY,       K_DA550A3F10M_INVERTER_BASE_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vL	14	Base frequency 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_VF_CONTROL_MODE,      K_DA550A3F10M_INVERTER_VF_CONTROL_MODE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Pt	15	V/F control mode selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_START_FREQ,      K_DA550A3F10M_INVERTER_AUTO_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);   // F241	Frequenza avviamento automatico
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_START_FREQ,     K_DA550A3F10M_INVERTER_BRAKE_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F250	250	DC braking starting frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_CURRENT_SET,    K_DA550A3F10M_INVERTER_BRAKE_CURRENT_SET, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Corrente di frenatura in % della corrente nominale inverter (0..100%)
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_TIME,           K_DA550A3F10M_INVERTER_BRAKE_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F252	252	DC braking time
					Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PWM_CARRIER,  K_DA550A3F10M_INVERTER_PWM_CARRIER, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz					
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTORESTART_CONTROL,  K_DA550A3F10M_INVERTER_AUTORESTART_CONTROL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F301	301	Auto-restart control selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_SUPPLY_VOLTAGE_CORRECT,  K_DA550A3F10M_INVERTER_SUPPLY_VOLTAGE_CORRECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F307 Supply voltage correction Default=2 Unit: 0,1,2,3
#if (K_AUTOTUNING_OFF == 1)
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTOTUNING,  K_DA550A3F10M_INVERTER_AUTOTUNING, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F400	400	Autotuning						Unit: 1				
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_TORQUE_BOOST_VAL,  K_DA550A3F10M_INVERTER_AUTO_TORQUE_BOOST_VAL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F402	402	Automatic torque boost value			
#endif	                
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CAPACITY, K_DA550A3F10M_INVERTER_MOTOR_RATED_CAPACITY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F405	405	Motor rated capacity
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQ_VOLTAGE,    K_DA550A3F10M_INVERTER_BASE_FREQ_VOLTAGE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vLv	409	Base frequency voltage 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CURRENT,  K_DA550A3F10M_INVERTER_MOTOR_RATED_CURRENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F415	415	Motor rated current
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_SPEED,    K_DA550A3F10M_INVERTER_MOTOR_RATED_SPEED, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F417	417	Motor rated speed
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF2,K_DA550A3F10M_INVERTER_MOTOR_SPECIFIC_COEFF2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F458	458	Motor specific coefficient 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_LOAD_INERTIA_MOMENT,  K_DA550A3F10M_INVERTER_LOAD_INERTIA_MOMENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F459	459	Load inertia moment ratio
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF3,K_DA550A3F10M_INVERTER_MOTOR_SPECIFIC_COEFF3, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F460	460	Motor specific coefficient 3
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF9,K_DA550A3F10M_INVERTER_MOTOR_SPECIFIC_COEFF9, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F495	495	Motor specific coefficient 9
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACCELERATION_TIME2,   K_DA550A3F10M_INVERTER_ACCELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F500	500	Acceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DECELERATION_TIME2,   K_DA550A3F10M_INVERTER_DECELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F501	501	Deceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACC_DEC_1e2_SW_FREQ,  K_DA550A3F10M_INVERTER_ACC_DEC_1e2_SW_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F505	505	Acceleration/deceleration 1 and 2 switching frequency
					Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STALL_PREVENTION_L1,  K_DA550A3F10M_INVERTER_STALL_PREVENTION_L1, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F601	601	Stall prevention level 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_RIL_FASE_SELECTION,   K_DA550A3F10M_INVERTER_RIL_FASE_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F608	608 0 Selezione rilevamento fase ingresso
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UNIT_SELECTION,       K_DA550A3F10M_INVERTER_UNIT_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // f701 Selezione unit� corrente/tensione Dspu		change parameters from % to nominal value	1   ??????
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_TIMEOUT_COMUNICATION,   K_DA550A3F10M_INVERTER_TIMEOUT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F803 Time-out tempo di comunicazione		Unit: 0.1s
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACTION_TMT_COMUNICATION,K_DA550A3F10M_INVERTER_ACTION_TMT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto										
	            //    Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9P, K_DA550A3F10M_INVERTER_FACTORY_SPEC_COEFF9P, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F908	908	Factory specific coefficient 9P
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9D, K_DA550A3F10M_INVERTER_FACTORY_SPEC_COEFF9D, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F909	909	Factory specific coefficient 9D
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_CURR_LEV, K_DA550A3F10M_INVERTER_STEPOUT_DET_CURR_LEV, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F910	910	Step-out detection current level
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_TIME,     K_DA550A3F10M_INVERTER_STEPOUT_DET_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F911	911	Step-out detection time
#if (K_AUTOTUNING_OFF == 1)		
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_QAXIS_INDUCTANCE,     K_DA550A3F10M_INVERTER_QAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DAXIS_INDUCTANCE,     K_DA550A3F10M_INVERTER_DAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry			
#endif	
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_UNDERVOLTAGE_TRIP_ALARM_SEL, K_GENERAL_INVERTER_UNDERVOLTAGE_ERROR, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            

                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PM_CONTROL_SELECTION, K_DA550A3F10M_INVERTER_PM_CONTROL_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F915	915	PM control selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9L, K_DA550A3F10M_INVERTER_FACTORY_SPEC_COEFF9L, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            

				Inverter->StepOutCurrentWarningDB = K_GENERAL_INVERTER_STEPOUT_DISABLE;
				Inverter->StepOutCurrentWarningWork = K_GENERAL_INVERTER_STEPOUT_ENABLE;				
				
            break;
            case K_MotorType_DA422A3F26M:
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MAX_FREQUENCY,        K_DA422A3F26M_INVERTER_MAX_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UPPER_LIM_FREQ,       K_DA422A3F26M_INVERTER_UPPER_LIM_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      // UL	12	Upper limit frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQUENCY,       K_DA422A3F26M_INVERTER_BASE_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vL	14	Base frequency 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_VF_CONTROL_MODE,      K_DA422A3F26M_INVERTER_VF_CONTROL_MODE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Pt	15	V/F control mode selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_START_FREQ,      K_DA422A3F26M_INVERTER_AUTO_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);   // F241	Frequenza avviamento automatico
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_START_FREQ,     K_DA422A3F26M_INVERTER_BRAKE_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F250	250	DC braking starting frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_CURRENT_SET,    K_DA422A3F26M_INVERTER_BRAKE_CURRENT_SET, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Corrente di frenatura in % della corrente nominale inverter (0..100%)
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_TIME,           K_DA422A3F26M_INVERTER_BRAKE_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F252	252	DC braking time
					Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PWM_CARRIER,  K_DA422A3F26M_INVERTER_PWM_CARRIER, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz					
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTORESTART_CONTROL,  K_DA422A3F26M_INVERTER_AUTORESTART_CONTROL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F301	301	Auto-restart control selection				
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_SUPPLY_VOLTAGE_CORRECT,  K_DA422A3F26M_INVERTER_SUPPLY_VOLTAGE_CORRECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F307 Supply voltage correction Default=2 Unit: 0,1,2,3
#if (K_AUTOTUNING_OFF == 1)
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTOTUNING,  K_DA422A3F26M_INVERTER_AUTOTUNING, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F400	400	Autotuning						Unit: 1				
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_TORQUE_BOOST_VAL,  K_DA422A3F26M_INVERTER_AUTO_TORQUE_BOOST_VAL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F402	402	Automatic torque boost value			
#endif	
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CAPACITY, K_DA422A3F26M_INVERTER_MOTOR_RATED_CAPACITY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F405	405	Motor rated capacity
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQ_VOLTAGE,    K_DA422A3F26M_INVERTER_BASE_FREQ_VOLTAGE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vLv	409	Base frequency voltage 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CURRENT,  K_DA422A3F26M_INVERTER_MOTOR_RATED_CURRENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F415	415	Motor rated current
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_SPEED,    K_DA422A3F26M_INVERTER_MOTOR_RATED_SPEED, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F417	417	Motor rated speed
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF2,K_DA422A3F26M_INVERTER_MOTOR_SPECIFIC_COEFF2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F458	458	Motor specific coefficient 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_LOAD_INERTIA_MOMENT,  K_DA422A3F26M_INVERTER_LOAD_INERTIA_MOMENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F459	459	Load inertia moment ratio
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF3,K_DA422A3F26M_INVERTER_MOTOR_SPECIFIC_COEFF3, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F460	460	Motor specific coefficient 3
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF9,K_DA422A3F26M_INVERTER_MOTOR_SPECIFIC_COEFF9, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F495	495	Motor specific coefficient 9
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACCELERATION_TIME2,   K_DA422A3F26M_INVERTER_ACCELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F500	500	Acceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DECELERATION_TIME2,   K_DA422A3F26M_INVERTER_DECELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F501	501	Deceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACC_DEC_1e2_SW_FREQ,  K_DA422A3F26M_INVERTER_ACC_DEC_1e2_SW_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F505	505	Acceleration/deceleration 1 and 2 switching frequency
					Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STALL_PREVENTION_L1,  K_DA422A3F26M_INVERTER_STALL_PREVENTION_L1, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F601	601	Stall prevention level 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_RIL_FASE_SELECTION,   K_DA422A3F26M_INVERTER_RIL_FASE_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F608	608 0 Selezione rilevamento fase ingresso
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UNIT_SELECTION,       K_DA422A3F26M_INVERTER_UNIT_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // f701 Selezione unit� corrente/tensione Dspu		change parameters from % to nominal value	1   ??????
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_TIMEOUT_COMUNICATION,   K_DA422A3F26M_INVERTER_TIMEOUT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F803 Time-out tempo di comunicazione		Unit: 0.1s
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACTION_TMT_COMUNICATION,K_DA422A3F26M_INVERTER_ACTION_TMT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto										
	            //    Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9P, K_DA422A3F26M_INVERTER_FACTORY_SPEC_COEFF9P, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F908	908	Factory specific coefficient 9P
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9D, K_DA422A3F26M_INVERTER_FACTORY_SPEC_COEFF9D, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F909	909	Factory specific coefficient 9D
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_CURR_LEV, K_DA422A3F26M_INVERTER_STEPOUT_DET_CURR_LEV, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F910	910	Step-out detection current level
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_TIME,     K_DA422A3F26M_INVERTER_STEPOUT_DET_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F911	911	Step-out detection time		   
#if (K_AUTOTUNING_OFF == 1)		
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_QAXIS_INDUCTANCE,     K_DA422A3F26M_INVERTER_QAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DAXIS_INDUCTANCE,     K_DA422A3F26M_INVERTER_DAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry			
#endif		
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_UNDERVOLTAGE_TRIP_ALARM_SEL, K_GENERAL_INVERTER_UNDERVOLTAGE_ERROR, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            

                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PM_CONTROL_SELECTION, K_DA422A3F26M_INVERTER_PM_CONTROL_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F915	915	PM control selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9L, K_DA422A3F26M_INVERTER_FACTORY_SPEC_COEFF9L, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            

				Inverter->StepOutCurrentWarningDB = K_GENERAL_INVERTER_STEPOUT_DISABLE;
				Inverter->StepOutCurrentWarningWork = K_GENERAL_INVERTER_STEPOUT_ENABLE;				
                
            break;

            case K_MotorType_DA422A3F27M:
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MAX_FREQUENCY,        K_DA422A3F27M_INVERTER_MAX_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UPPER_LIM_FREQ,       K_DA422A3F27M_INVERTER_UPPER_LIM_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      // UL	12	Upper limit frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQUENCY,       K_DA422A3F27M_INVERTER_BASE_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vL	14	Base frequency 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_VF_CONTROL_MODE,      K_DA422A3F27M_INVERTER_VF_CONTROL_MODE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Pt	15	V/F control mode selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_START_FREQ,      K_DA422A3F27M_INVERTER_AUTO_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);   // F241	Frequenza avviamento automatico
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_START_FREQ,     K_DA422A3F27M_INVERTER_BRAKE_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F250	250	DC braking starting frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_CURRENT_SET,    K_DA422A3F27M_INVERTER_BRAKE_CURRENT_SET, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Corrente di frenatura in % della corrente nominale inverter (0..100%)
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_TIME,           K_DA422A3F27M_INVERTER_BRAKE_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F252	252	DC braking time
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PWM_CARRIER,          K_DA422A3F27M_INVERTER_PWM_CARRIER, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz					
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTORESTART_CONTROL,  K_DA422A3F27M_INVERTER_AUTORESTART_CONTROL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F301	301	Auto-restart control selection				
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_SUPPLY_VOLTAGE_CORRECT,  K_DA422A3F27M_INVERTER_SUPPLY_VOLTAGE_CORRECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F307 Supply voltage correction Default=2 Unit: 0,1,2,3                
#if (K_AUTOTUNING_OFF == 1)
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTOTUNING,  K_DA422A3F27M_INVERTER_AUTOTUNING, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F400	400	Autotuning						Unit: 1				
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_TORQUE_BOOST_VAL,  K_DA422A3F27M_INVERTER_AUTO_TORQUE_BOOST_VAL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F402	402	Automatic torque boost value			
#endif	
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CAPACITY, K_DA422A3F27M_INVERTER_MOTOR_RATED_CAPACITY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F405	405	Motor rated capacity
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQ_VOLTAGE,    K_DA422A3F27M_INVERTER_BASE_FREQ_VOLTAGE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vLv	409	Base frequency voltage 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CURRENT,  K_DA422A3F27M_INVERTER_MOTOR_RATED_CURRENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F415	415	Motor rated current
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_SPEED,    K_DA422A3F27M_INVERTER_MOTOR_RATED_SPEED, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F417	417	Motor rated speed
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF2,K_DA422A3F27M_INVERTER_MOTOR_SPECIFIC_COEFF2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F458	458	Motor specific coefficient 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_LOAD_INERTIA_MOMENT,  K_DA422A3F27M_INVERTER_LOAD_INERTIA_MOMENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F459	459	Load inertia moment ratio
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF3,K_DA422A3F27M_INVERTER_MOTOR_SPECIFIC_COEFF3, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F460	460	Motor specific coefficient 3
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF9,K_DA422A3F27M_INVERTER_MOTOR_SPECIFIC_COEFF9, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F495	495	Motor specific coefficient 9
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACCELERATION_TIME2,   K_DA422A3F27M_INVERTER_ACCELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F500	500	Acceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DECELERATION_TIME2,   K_DA422A3F27M_INVERTER_DECELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F501	501	Deceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACC_DEC_1e2_SW_FREQ,  K_DA422A3F27M_INVERTER_ACC_DEC_1e2_SW_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F505	505	Acceleration/deceleration 1 and 2 switching frequency
					Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STALL_PREVENTION_L1,  K_DA422A3F27M_INVERTER_STALL_PREVENTION_L1, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F601	601	Stall prevention level 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_RIL_FASE_SELECTION,   K_DA422A3F27M_INVERTER_RIL_FASE_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F608	608 0 Selezione rilevamento fase ingresso
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UNIT_SELECTION,       K_DA422A3F27M_INVERTER_UNIT_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // f701 Selezione unit� corrente/tensione Dspu		change parameters from % to nominal value	1   ??????
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_TIMEOUT_COMUNICATION,   K_DA422A3F27M_INVERTER_TIMEOUT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F803 Time-out tempo di comunicazione		Unit: 0.1s
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACTION_TMT_COMUNICATION,K_DA422A3F27M_INVERTER_ACTION_TMT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto										
	            //    Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9P, K_DA422A3F27M_INVERTER_FACTORY_SPEC_COEFF9P, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F908	908	Factory specific coefficient 9P
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9D, K_DA422A3F27M_INVERTER_FACTORY_SPEC_COEFF9D, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F909	909	Factory specific coefficient 9D
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_CURR_LEV, K_DA422A3F27M_INVERTER_STEPOUT_DET_CURR_LEV, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F910	910	Step-out detection current level
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_TIME,     K_DA422A3F27M_INVERTER_STEPOUT_DET_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F911	911	Step-out detection time		   
#if (K_AUTOTUNING_OFF == 1)		
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_QAXIS_INDUCTANCE,     K_DA422A3F27M_INVERTER_QAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DAXIS_INDUCTANCE,     K_DA422A3F27M_INVERTER_DAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry			
#endif		
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_UNDERVOLTAGE_TRIP_ALARM_SEL, K_GENERAL_INVERTER_UNDERVOLTAGE_ERROR, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            

                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PM_CONTROL_SELECTION, K_DA422A3F27M_INVERTER_PM_CONTROL_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F915	915	PM control selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9L, K_DA422A3F27M_INVERTER_FACTORY_SPEC_COEFF9L, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            

				Inverter->StepOutCurrentWarningDB = K_GENERAL_INVERTER_STEPOUT_DISABLE;
				Inverter->StepOutCurrentWarningWork = K_GENERAL_INVERTER_STEPOUT_ENABLE;         
            break;    

			case K_MotorType_DA790A4F11UC1:
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MAX_FREQUENCY,        K_DA790A4F11UC1_INVERTER_MAX_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UPPER_LIM_FREQ,       K_DA790A4F11UC1_INVERTER_UPPER_LIM_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      // UL	12	Upper limit frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQUENCY,       K_DA790A4F11UC1_INVERTER_BASE_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vL	14	Base frequency 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_VF_CONTROL_MODE,      K_DA790A4F11UC1_INVERTER_VF_CONTROL_MODE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Pt	15	V/F control mode selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_START_FREQ,      K_DA790A4F11UC1_INVERTER_AUTO_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);   // F241	Frequenza avviamento automatico
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_START_FREQ,     K_DA790A4F11UC1_INVERTER_BRAKE_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F250	250	DC braking starting frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_CURRENT_SET,    K_DA790A4F11UC1_INVERTER_BRAKE_CURRENT_SET, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Corrente di frenatura in % della corrente nominale inverter (0..100%)
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_TIME,           K_DA790A4F11UC1_INVERTER_BRAKE_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F252	252	DC braking time
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PWM_CARRIER,  K_DA790A4F11UC1_INVERTER_PWM_CARRIER, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz					
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTORESTART_CONTROL,  K_DA790A4F11UC1_INVERTER_AUTORESTART_CONTROL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F301	301	Auto-restart control selection				
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_SUPPLY_VOLTAGE_CORRECT,  K_DA790A4F11UC1_INVERTER_SUPPLY_VOLTAGE_CORRECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F307 Supply voltage correction Default=2 Unit: 0,1,2,3
#if (K_AUTOTUNING_OFF == 1)
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTOTUNING,  K_DA790A4F11UC1_INVERTER_AUTOTUNING, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F400	400	Autotuning						Unit: 1				
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_TORQUE_BOOST_VAL,  K_DA790A4F11UC1_INVERTER_AUTO_TORQUE_BOOST_VAL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F402	402	Automatic torque boost value			
#endif	 
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CAPACITY, K_DA790A4F11UC1_INVERTER_MOTOR_RATED_CAPACITY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F405	405	Motor rated capacity
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQ_VOLTAGE,    K_DA790A4F11UC1_INVERTER_BASE_FREQ_VOLTAGE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vLv	409	Base frequency voltage 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CURRENT,  K_DA790A4F11UC1_INVERTER_MOTOR_RATED_CURRENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F415	415	Motor rated current
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_SPEED,    K_DA790A4F11UC1_INVERTER_MOTOR_RATED_SPEED, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F417	417	Motor rated speed
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF2,K_DA790A4F11UC1_INVERTER_MOTOR_SPECIFIC_COEFF2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F458	458	Motor specific coefficient 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_LOAD_INERTIA_MOMENT,  K_DA790A4F11UC1_INVERTER_LOAD_INERTIA_MOMENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F459	459	Load inertia moment ratio
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF3,K_DA790A4F11UC1_INVERTER_MOTOR_SPECIFIC_COEFF3, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F460	460	Motor specific coefficient 3
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF9,K_DA790A4F11UC1_INVERTER_MOTOR_SPECIFIC_COEFF9, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F495	495	Motor specific coefficient 9
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACCELERATION_TIME2,   K_DA790A4F11UC1_INVERTER_ACCELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F500	500	Acceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DECELERATION_TIME2,   K_DA790A4F11UC1_INVERTER_DECELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F501	501	Deceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACC_DEC_1e2_SW_FREQ,  K_DA790A4F11UC1_INVERTER_ACC_DEC_1e2_SW_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F505	505	Acceleration/deceleration 1 and 2 switching frequency
					Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STALL_PREVENTION_L1,  K_DA790A4F11UC1_INVERTER_STALL_PREVENTION_L1, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F601	601	Stall prevention level 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_RIL_FASE_SELECTION,   K_DA790A4F11UC1_INVERTER_RIL_FASE_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F608	608 0 Selezione rilevamento fase ingresso
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UNIT_SELECTION,       K_DA790A4F11UC1_INVERTER_UNIT_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // f701 Selezione unit� corrente/tensione Dspu		change parameters from % to nominal value	1   ??????
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_TIMEOUT_COMUNICATION,   K_DA790A4F11UC1_INVERTER_TIMEOUT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F803 Time-out tempo di comunicazione		Unit: 0.1s
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACTION_TMT_COMUNICATION,K_DA790A4F11UC1_INVERTER_ACTION_TMT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto										
	            //    Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9P, K_DA790A4F11UC1_INVERTER_FACTORY_SPEC_COEFF9P, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F908	908	Factory specific coefficient 9P
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9D, K_DA790A4F11UC1_INVERTER_FACTORY_SPEC_COEFF9D, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F909	909	Factory specific coefficient 9D
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_CURR_LEV, K_DA790A4F11UC1_INVERTER_STEPOUT_DET_CURR_LEV, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F910	910	Step-out detection current level
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_TIME,     K_DA790A4F11UC1_INVERTER_STEPOUT_DET_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F911	911	Step-out detection time		   
#if (K_AUTOTUNING_OFF == 1)		
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_QAXIS_INDUCTANCE,     K_DA790A4F11UC1_INVERTER_QAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DAXIS_INDUCTANCE,     K_DA790A4F11UC1_INVERTER_DAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry			
#endif		
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_UNDERVOLTAGE_TRIP_ALARM_SEL, K_GENERAL_INVERTER_UNDERVOLTAGE_ERROR, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            

                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PM_CONTROL_SELECTION, K_DA790A4F11UC1_INVERTER_PM_CONTROL_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F915	915	PM control selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9L, K_DA790A4F11UC1_INVERTER_FACTORY_SPEC_COEFF9L, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            

				Inverter->StepOutCurrentWarningDB = K_GENERAL_INVERTER_STEPOUT_DISABLE;
				Inverter->StepOutCurrentWarningWork = K_GENERAL_INVERTER_STEPOUT_ENABLE;				
                				
			break;
            case K_MotorType_DA330A2F20M:
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MAX_FREQUENCY,        K_DA330A2F20M_INVERTER_MAX_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UPPER_LIM_FREQ,       K_DA330A2F20M_INVERTER_UPPER_LIM_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      // UL	12	Upper limit frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQUENCY,       K_DA330A2F20M_INVERTER_BASE_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vL	14	Base frequency 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_VF_CONTROL_MODE,      K_DA330A2F20M_INVERTER_VF_CONTROL_MODE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Pt	15	V/F control mode selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_START_FREQ,      K_DA330A2F20M_INVERTER_AUTO_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);   // F241	Frequenza avviamento automatico
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_START_FREQ,     K_DA330A2F20M_INVERTER_BRAKE_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F250	250	DC braking starting frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_CURRENT_SET,    K_DA330A2F20M_INVERTER_BRAKE_CURRENT_SET, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Corrente di frenatura in % della corrente nominale inverter (0..100%)
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_TIME,           K_DA330A2F20M_INVERTER_BRAKE_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F252	252	DC braking time		
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PWM_CARRIER,  K_DA330A2F20M_INVERTER_PWM_CARRIER, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz	
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTORESTART_CONTROL,  K_DA330A2F20M_INVERTER_AUTORESTART_CONTROL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F301	301	Auto-restart control selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_SUPPLY_VOLTAGE_CORRECT,  K_DA330A2F20M_INVERTER_SUPPLY_VOLTAGE_CORRECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F307 Supply voltage correction Default=2 Unit: 0,1,2,3
#if (K_AUTOTUNING_OFF == 1)
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTOTUNING,  K_DA330A2F20M_INVERTER_AUTOTUNING, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F400	400	Autotuning						Unit: 1				
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_TORQUE_BOOST_VAL,  K_DA330A2F20M_INVERTER_AUTO_TORQUE_BOOST_VAL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F402	402	Automatic torque boost value			
#endif                
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CAPACITY, K_DA330A2F20M_INVERTER_MOTOR_RATED_CAPACITY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F405	405	Motor rated capacity
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQ_VOLTAGE,    K_DA330A2F20M_INVERTER_BASE_FREQ_VOLTAGE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vLv	409	Base frequency voltage 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CURRENT,  K_DA330A2F20M_INVERTER_MOTOR_RATED_CURRENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F415	415	Motor rated current
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_SPEED,    K_DA330A2F20M_INVERTER_MOTOR_RATED_SPEED, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F417	417	Motor rated speed
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF2,K_DA330A2F20M_INVERTER_MOTOR_SPECIFIC_COEFF2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F458	458	Motor specific coefficient 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_LOAD_INERTIA_MOMENT,  K_DA330A2F20M_INVERTER_LOAD_INERTIA_MOMENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F459	459	Load inertia moment ratio
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF3,K_DA330A2F20M_INVERTER_MOTOR_SPECIFIC_COEFF3, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F460	460	Motor specific coefficient 3
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF9,K_DA330A2F20M_INVERTER_MOTOR_SPECIFIC_COEFF9, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F495	495	Motor specific coefficient 9
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACCELERATION_TIME2,   K_DA330A2F20M_INVERTER_ACCELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F500	500	Acceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DECELERATION_TIME2,   K_DA330A2F20M_INVERTER_DECELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F501	501	Deceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACC_DEC_1e2_SW_FREQ,  K_DA330A2F20M_INVERTER_ACC_DEC_1e2_SW_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F505	505	Acceleration/deceleration 1 and 2 switching frequency
                //Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STALL_PREVENTION_L1,  K_DA330A2F20M_INVERTER_STALL_PREVENTION_L1, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F601	601	Stall prevention level 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_RIL_FASE_SELECTION,   K_DA330A2F20M_INVERTER_RIL_FASE_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F608	608 0 Selezione rilevamento fase ingresso
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UNIT_SELECTION,       K_DA330A2F20M_INVERTER_UNIT_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // f701 Selezione unit� corrente/tensione Dspu		change parameters from % to nominal value	1   ??????
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_TIMEOUT_COMUNICATION,   K_DA330A2F20M_INVERTER_TIMEOUT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F803 Time-out tempo di comunicazione		Unit: 0.1s
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACTION_TMT_COMUNICATION,K_DA330A2F20M_INVERTER_ACTION_TMT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto						
//                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9P, K_DA330A2F20M_INVERTER_FACTORY_SPEC_COEFF9P, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F908	908	Factory specific coefficient 9P
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9D, K_DA330A2F20M_INVERTER_FACTORY_SPEC_COEFF9D, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F909	909	Factory specific coefficient 9D
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_CURR_LEV, K_DA330A2F20M_INVERTER_STEPOUT_DET_CURR_LEV, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F910	910	Step-out detection current level
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_TIME,     K_DA330A2F20M_INVERTER_STEPOUT_DET_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F911	911	Step-out detection time
#if (K_AUTOTUNING_OFF == 1)		
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_QAXIS_INDUCTANCE,     K_DA330A2F20M_INVERTER_QAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DAXIS_INDUCTANCE,     K_DA330A2F20M_INVERTER_DAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry			
#endif
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_UNDERVOLTAGE_TRIP_ALARM_SEL, K_GENERAL_INVERTER_UNDERVOLTAGE_ERROR, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            

                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PM_CONTROL_SELECTION, K_DA330A2F20M_INVERTER_PM_CONTROL_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F915	915	PM control selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9L, K_DA330A2F20M_INVERTER_FACTORY_SPEC_COEFF9L, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            
                
				Inverter->StepOutCurrentWarningDB = K_GENERAL_INVERTER_STEPOUT_DISABLE;
				Inverter->StepOutCurrentWarningWork = K_GENERAL_INVERTER_STEPOUT_ENABLE;		
            break;
            case K_MotorType_DA550A3F11MD:
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MAX_FREQUENCY,        K_DA550A3F11MD_INVERTER_MAX_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UPPER_LIM_FREQ,       K_DA550A3F11MD_INVERTER_UPPER_LIM_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      // UL	12	Upper limit frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQUENCY,       K_DA550A3F11MD_INVERTER_BASE_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vL	14	Base frequency 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_VF_CONTROL_MODE,      K_DA550A3F11MD_INVERTER_VF_CONTROL_MODE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Pt	15	V/F control mode selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_START_FREQ,      K_DA550A3F11MD_INVERTER_AUTO_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);   // F241	Frequenza avviamento automatico
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_START_FREQ,     K_DA550A3F11MD_INVERTER_BRAKE_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F250	250	DC braking starting frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_CURRENT_SET,    K_DA550A3F11MD_INVERTER_BRAKE_CURRENT_SET, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Corrente di frenatura in % della corrente nominale inverter (0..100%)
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_TIME,           K_DA550A3F11MD_INVERTER_BRAKE_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F252	252	DC braking time
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PWM_CARRIER,  K_DA550A3F11MD_INVERTER_PWM_CARRIER, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz					
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTORESTART_CONTROL,  K_DA550A3F11MD_INVERTER_AUTORESTART_CONTROL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F301	301	Auto-restart control selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_SUPPLY_VOLTAGE_CORRECT,  K_DA550A3F11MD_INVERTER_SUPPLY_VOLTAGE_CORRECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F307 Supply voltage correction Default=2 Unit: 0,1,2,3
#if (K_AUTOTUNING_OFF == 1)
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTOTUNING,  K_DA550A3F11MD_INVERTER_AUTOTUNING, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F400	400	Autotuning						Unit: 1				
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_TORQUE_BOOST_VAL,  K_DA550A3F11MD_INVERTER_AUTO_TORQUE_BOOST_VAL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F402	402	Automatic torque boost value			
#endif	                
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CAPACITY, K_DA550A3F11MD_INVERTER_MOTOR_RATED_CAPACITY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F405	405	Motor rated capacity
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQ_VOLTAGE,    K_DA550A3F11MD_INVERTER_BASE_FREQ_VOLTAGE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vLv	409	Base frequency voltage 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CURRENT,  K_DA550A3F11MD_INVERTER_MOTOR_RATED_CURRENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F415	415	Motor rated current
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_SPEED,    K_DA550A3F11MD_INVERTER_MOTOR_RATED_SPEED, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F417	417	Motor rated speed
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF2,K_DA550A3F11MD_INVERTER_MOTOR_SPECIFIC_COEFF2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F458	458	Motor specific coefficient 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_LOAD_INERTIA_MOMENT,  K_DA550A3F11MD_INVERTER_LOAD_INERTIA_MOMENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F459	459	Load inertia moment ratio
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF3,K_DA550A3F11MD_INVERTER_MOTOR_SPECIFIC_COEFF3, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F460	460	Motor specific coefficient 3
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF9,K_DA550A3F11MD_INVERTER_MOTOR_SPECIFIC_COEFF9, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F495	495	Motor specific coefficient 9
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACCELERATION_TIME2,   K_DA550A3F11MD_INVERTER_ACCELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F500	500	Acceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DECELERATION_TIME2,   K_DA550A3F11MD_INVERTER_DECELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F501	501	Deceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACC_DEC_1e2_SW_FREQ,  K_DA550A3F11MD_INVERTER_ACC_DEC_1e2_SW_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F505	505	Acceleration/deceleration 1 and 2 switching frequency
					Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STALL_PREVENTION_L1,  K_DA550A3F11MD_INVERTER_STALL_PREVENTION_L1, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F601	601	Stall prevention level 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_RIL_FASE_SELECTION,   K_DA550A3F11MD_INVERTER_RIL_FASE_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F608	608 0 Selezione rilevamento fase ingresso
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UNIT_SELECTION,       K_DA550A3F11MD_INVERTER_UNIT_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // f701 Selezione unit� corrente/tensione Dspu		change parameters from % to nominal value	1   ??????
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_TIMEOUT_COMUNICATION,   K_DA550A3F11MD_INVERTER_TIMEOUT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F803 Time-out tempo di comunicazione		Unit: 0.1s
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACTION_TMT_COMUNICATION,K_DA550A3F11MD_INVERTER_ACTION_TMT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto										
	            //    Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9P, K_DA550A3F11MD_INVERTER_FACTORY_SPEC_COEFF9P, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F908	908	Factory specific coefficient 9P
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9D, K_DA550A3F11MD_INVERTER_FACTORY_SPEC_COEFF9D, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F909	909	Factory specific coefficient 9D
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_CURR_LEV, K_DA550A3F11MD_INVERTER_STEPOUT_DET_CURR_LEV, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F910	910	Step-out detection current level
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_TIME,     K_DA550A3F11MD_INVERTER_STEPOUT_DET_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F911	911	Step-out detection time
#if (K_AUTOTUNING_OFF == 1)		
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_QAXIS_INDUCTANCE,     K_DA550A3F11MD_INVERTER_QAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DAXIS_INDUCTANCE,     K_DA550A3F11MD_INVERTER_DAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry			
#endif	
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_UNDERVOLTAGE_TRIP_ALARM_SEL, K_GENERAL_INVERTER_UNDERVOLTAGE_ERROR, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            

                // Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PM_CONTROL_SELECTION, K_DA550A3F11MD_INVERTER_PM_CONTROL_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F915	915	PM control selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9F, K_DA550A3F11MD_INVERTER_FACTORY_SPEC_COEFF9F, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      // F916	916	PM control selection			Unit: 1
                // Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9L, K_DA550A3F11MD_INVERTER_FACTORY_SPEC_COEFF9L, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            

				Inverter->StepOutCurrentWarningDB = K_GENERAL_INVERTER_STEPOUT_DISABLE;
				Inverter->StepOutCurrentWarningWork = K_GENERAL_INVERTER_STEPOUT_ENABLE;				
				
            break;                                    

            case K_MotorType_DA640A3F20MAD:
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MAX_FREQUENCY,        K_DA640A3F20MAD_INVERTER_MAX_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UPPER_LIM_FREQ,       K_DA640A3F20MAD_INVERTER_UPPER_LIM_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      // UL	12	Upper limit frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQUENCY,       K_DA640A3F20MAD_INVERTER_BASE_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vL	14	Base frequency 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_VF_CONTROL_MODE,      K_DA640A3F20MAD_INVERTER_VF_CONTROL_MODE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Pt	15	V/F control mode selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_START_FREQ,      K_DA640A3F20MAD_INVERTER_AUTO_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);   // F241	Frequenza avviamento automatico
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_START_FREQ,     K_DA640A3F20MAD_INVERTER_BRAKE_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F250	250	DC braking starting frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_CURRENT_SET,    K_DA640A3F20MAD_INVERTER_BRAKE_CURRENT_SET, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Corrente di frenatura in % della corrente nominale inverter (0..100%)
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_TIME,           K_DA640A3F20MAD_INVERTER_BRAKE_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F252	252	DC braking time
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PWM_CARRIER,  K_DA640A3F20MAD_INVERTER_PWM_CARRIER, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz					
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTORESTART_CONTROL,  K_DA640A3F20MAD_INVERTER_AUTORESTART_CONTROL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F301	301	Auto-restart control selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_SUPPLY_VOLTAGE_CORRECT,  K_DA640A3F20MAD_INVERTER_SUPPLY_VOLTAGE_CORRECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F307 Supply voltage correction Default=2 Unit: 0,1,2,3
#if (K_AUTOTUNING_OFF == 1)
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTOTUNING,  K_DA640A3F20MAD_INVERTER_AUTOTUNING, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F400	400	Autotuning						Unit: 1				
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_TORQUE_BOOST_VAL,  K_DA640A3F20MAD_INVERTER_AUTO_TORQUE_BOOST_VAL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F402	402	Automatic torque boost value			
#endif	                
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CAPACITY, K_DA640A3F20MAD_INVERTER_MOTOR_RATED_CAPACITY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F405	405	Motor rated capacity
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQ_VOLTAGE,    K_DA640A3F20MAD_INVERTER_BASE_FREQ_VOLTAGE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vLv	409	Base frequency voltage 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CURRENT,  K_DA640A3F20MAD_INVERTER_MOTOR_RATED_CURRENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F415	415	Motor rated current
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_SPEED,    K_DA640A3F20MAD_INVERTER_MOTOR_RATED_SPEED, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F417	417	Motor rated speed
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF2,K_DA640A3F20MAD_INVERTER_MOTOR_SPECIFIC_COEFF2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F458	458	Motor specific coefficient 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_LOAD_INERTIA_MOMENT,  K_DA640A3F20MAD_INVERTER_LOAD_INERTIA_MOMENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F459	459	Load inertia moment ratio
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF3,K_DA640A3F20MAD_INVERTER_MOTOR_SPECIFIC_COEFF3, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F460	460	Motor specific coefficient 3
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF9,K_DA640A3F20MAD_INVERTER_MOTOR_SPECIFIC_COEFF9, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F495	495	Motor specific coefficient 9
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACCELERATION_TIME2,   K_DA640A3F20MAD_INVERTER_ACCELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F500	500	Acceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DECELERATION_TIME2,   K_DA640A3F20MAD_INVERTER_DECELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F501	501	Deceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACC_DEC_1e2_SW_FREQ,  K_DA640A3F20MAD_INVERTER_ACC_DEC_1e2_SW_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F505	505	Acceleration/deceleration 1 and 2 switching frequency
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STALL_PREVENTION_L1,  K_DA640A3F20MAD_INVERTER_STALL_PREVENTION_L1, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F601	601	Stall prevention level 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_RIL_FASE_SELECTION,   K_DA640A3F20MAD_INVERTER_RIL_FASE_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F608	608 0 Selezione rilevamento fase ingresso
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UNIT_SELECTION,       K_DA640A3F20MAD_INVERTER_UNIT_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // f701 Selezione unit� corrente/tensione Dspu		change parameters from % to nominal value	1   ??????
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_TIMEOUT_COMUNICATION,   K_DA640A3F20MAD_INVERTER_TIMEOUT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F803 Time-out tempo di comunicazione		Unit: 0.1s
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACTION_TMT_COMUNICATION,K_DA640A3F20MAD_INVERTER_ACTION_TMT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto										
	            //    Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9P, K_DA640A3F20MAD_INVERTER_FACTORY_SPEC_COEFF9P, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F908	908	Factory specific coefficient 9P
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9D, K_DA640A3F20MAD_INVERTER_FACTORY_SPEC_COEFF9D, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F909	909	Factory specific coefficient 9D
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_CURR_LEV, K_DA640A3F20MAD_INVERTER_STEPOUT_DET_CURR_LEV, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F910	910	Step-out detection current level
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_TIME,     K_DA640A3F20MAD_INVERTER_STEPOUT_DET_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F911	911	Step-out detection time
#if (K_AUTOTUNING_OFF == 1)		
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_QAXIS_INDUCTANCE,     K_DA640A3F20MAD_INVERTER_QAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DAXIS_INDUCTANCE,     K_DA640A3F20MAD_INVERTER_DAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry			
#endif	
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_UNDERVOLTAGE_TRIP_ALARM_SEL, K_GENERAL_INVERTER_UNDERVOLTAGE_ERROR, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            

                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PM_CONTROL_SELECTION, K_DA640A3F20MAD_INVERTER_PM_CONTROL_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F915	915	PM control selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9F, K_DA640A3F20MAD_INVERTER_FACTORY_SPEC_COEFF9F, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      // F916	916	PM control selection			Unit: 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9L, K_DA640A3F20MAD_INVERTER_FACTORY_SPEC_COEFF9L, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            

				Inverter->StepOutCurrentWarningDB = K_GENERAL_INVERTER_STEPOUT_DISABLE;
				Inverter->StepOutCurrentWarningWork = K_GENERAL_INVERTER_STEPOUT_ENABLE;				
            break;                                    
            
            case K_MotorType_DA150A1F21N:
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MAX_FREQUENCY,        K_DA150A1F21N_INVERTER_MAX_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UPPER_LIM_FREQ,       K_DA150A1F21N_INVERTER_UPPER_LIM_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      // UL	12	Upper limit frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQUENCY,       K_DA150A1F21N_INVERTER_BASE_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vL	14	Base frequency 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_VF_CONTROL_MODE,      K_DA150A1F21N_INVERTER_VF_CONTROL_MODE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Pt	15	V/F control mode selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_START_FREQ,      K_DA150A1F21N_INVERTER_AUTO_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);   // F241	Frequenza avviamento automatico
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_START_FREQ,     K_DA150A1F21N_INVERTER_BRAKE_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F250	250	DC braking starting frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_CURRENT_SET,    K_DA150A1F21N_INVERTER_BRAKE_CURRENT_SET, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Corrente di frenatura in % della corrente nominale inverter (0..100%)
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_TIME,           K_DA150A1F21N_INVERTER_BRAKE_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F252	252	DC braking time		
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PWM_CARRIER,  K_DA150A1F21N_INVERTER_PWM_CARRIER, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz	
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTORESTART_CONTROL,  K_DA150A1F21N_INVERTER_AUTORESTART_CONTROL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F301	301	Auto-restart control selection
#if (K_AUTOTUNING_OFF == 1)
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTOTUNING,  K_DA150A1F21N_INVERTER_AUTOTUNING, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F400	400	Autotuning						Unit: 1				
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_TORQUE_BOOST_VAL,  K_DA150A1F21N_INVERTER_AUTO_TORQUE_BOOST_VAL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F402	402	Automatic torque boost value			
#endif                
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CAPACITY, K_DA150A1F21N_INVERTER_MOTOR_RATED_CAPACITY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F405	405	Motor rated capacity
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQ_VOLTAGE,    K_DA150A1F21N_INVERTER_BASE_FREQ_VOLTAGE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vLv	409	Base frequency voltage 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CURRENT,  K_DA150A1F21N_INVERTER_MOTOR_RATED_CURRENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F415	415	Motor rated current
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_SPEED,    K_DA150A1F21N_INVERTER_MOTOR_RATED_SPEED, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F417	417	Motor rated speed
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF2,K_DA150A1F21N_INVERTER_MOTOR_SPECIFIC_COEFF2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F458	458	Motor specific coefficient 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_LOAD_INERTIA_MOMENT,  K_DA150A1F21N_INVERTER_LOAD_INERTIA_MOMENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F459	459	Load inertia moment ratio
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF3,K_DA150A1F21N_INVERTER_MOTOR_SPECIFIC_COEFF3, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F460	460	Motor specific coefficient 3
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF9,K_DA150A1F21N_INVERTER_MOTOR_SPECIFIC_COEFF9, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F495	495	Motor specific coefficient 9
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACCELERATION_TIME2,   K_DA150A1F21N_INVERTER_ACCELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F500	500	Acceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DECELERATION_TIME2,   K_DA150A1F21N_INVERTER_DECELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F501	501	Deceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACC_DEC_1e2_SW_FREQ,  K_DA150A1F21N_INVERTER_ACC_DEC_1e2_SW_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F505	505	Acceleration/deceleration 1 and 2 switching frequency
                //Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STALL_PREVENTION_L1,  K_DA150A1F21N_INVERTER_STALL_PREVENTION_L1, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F601	601	Stall prevention level 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_RIL_FASE_SELECTION,   K_DA150A1F21N_INVERTER_RIL_FASE_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F608	608 0 Selezione rilevamento fase ingresso
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UNIT_SELECTION,       K_DA150A1F21N_INVERTER_UNIT_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // f701 Selezione unit� corrente/tensione Dspu		change parameters from % to nominal value	1   ??????
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_TIMEOUT_COMUNICATION,   K_DA150A1F21N_INVERTER_TIMEOUT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F803 Time-out tempo di comunicazione		Unit: 0.1s
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACTION_TMT_COMUNICATION,K_DA150A1F21N_INVERTER_ACTION_TMT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto						
//                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9P, K_DA150A1F21N_INVERTER_FACTORY_SPEC_COEFF9P, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F908	908	Factory specific coefficient 9P
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9D, K_DA150A1F21N_INVERTER_FACTORY_SPEC_COEFF9D, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F909	909	Factory specific coefficient 9D
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_CURR_LEV, K_DA150A1F21N_INVERTER_STEPOUT_DET_CURR_LEV, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F910	910	Step-out detection current level
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_TIME,     K_DA150A1F21N_INVERTER_STEPOUT_DET_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F911	911	Step-out detection time
#if (K_AUTOTUNING_OFF == 1)		
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_QAXIS_INDUCTANCE,     K_DA150A1F21N_INVERTER_QAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DAXIS_INDUCTANCE,     K_DA150A1F21N_INVERTER_DAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry			
#endif
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_UNDERVOLTAGE_TRIP_ALARM_SEL, K_GENERAL_INVERTER_UNDERVOLTAGE_ERROR, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            

                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PM_CONTROL_SELECTION, K_DA150A1F21N_INVERTER_PM_CONTROL_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F915	915	PM control selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9L, K_DA150A1F21N_INVERTER_FACTORY_SPEC_COEFF9L, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            
                
				Inverter->StepOutCurrentWarningDB = K_GENERAL_INVERTER_STEPOUT_DISABLE;
				Inverter->StepOutCurrentWarningWork = K_GENERAL_INVERTER_STEPOUT_ENABLE;		
            break;			

            case K_MotorType_DA130A1F25F3:
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MAX_FREQUENCY,        K_DA130A1F25F3_INVERTER_MAX_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UPPER_LIM_FREQ,       K_DA130A1F25F3_INVERTER_UPPER_LIM_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);      // UL	12	Upper limit frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQUENCY,       K_DA130A1F25F3_INVERTER_BASE_FREQUENCY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vL	14	Base frequency 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_VF_CONTROL_MODE,      K_DA130A1F25F3_INVERTER_VF_CONTROL_MODE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Pt	15	V/F control mode selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_START_FREQ,      K_DA130A1F25F3_INVERTER_AUTO_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);   // F241	Frequenza avviamento automatico
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_START_FREQ,     K_DA130A1F25F3_INVERTER_BRAKE_START_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F250	250	DC braking starting frequency
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_CURRENT_SET,    K_DA130A1F25F3_INVERTER_BRAKE_CURRENT_SET, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // Corrente di frenatura in % della corrente nominale inverter (0..100%)
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_TIME,           K_DA130A1F25F3_INVERTER_BRAKE_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F252	252	DC braking time		
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PWM_CARRIER,  K_DA130A1F25F3_INVERTER_PWM_CARRIER, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F300	Frequenza portante PWM	Unit: 2,0..16,0 Khz	
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTORESTART_CONTROL,  K_DA130A1F25F3_INVERTER_AUTORESTART_CONTROL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F301	301	Auto-restart control selection
#if (K_AUTOTUNING_OFF == 1)
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTOTUNING,  K_DA130A1F25F3_INVERTER_AUTOTUNING, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F400	400	Autotuning						Unit: 1				
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_AUTO_TORQUE_BOOST_VAL,  K_DA130A1F25F3_INVERTER_AUTO_TORQUE_BOOST_VAL, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);			// F402	402	Automatic torque boost value			
#endif                
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CAPACITY, K_DA130A1F25F3_INVERTER_MOTOR_RATED_CAPACITY, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F405	405	Motor rated capacity
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BASE_FREQ_VOLTAGE,    K_DA130A1F25F3_INVERTER_BASE_FREQ_VOLTAGE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // vLv	409	Base frequency voltage 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_CURRENT,  K_DA130A1F25F3_INVERTER_MOTOR_RATED_CURRENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F415	415	Motor rated current
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_RATED_SPEED,    K_DA130A1F25F3_INVERTER_MOTOR_RATED_SPEED, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F417	417	Motor rated speed
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF2,K_DA130A1F25F3_INVERTER_MOTOR_SPECIFIC_COEFF2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F458	458	Motor specific coefficient 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_LOAD_INERTIA_MOMENT,  K_DA130A1F25F3_INVERTER_LOAD_INERTIA_MOMENT, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F459	459	Load inertia moment ratio
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF3,K_DA130A1F25F3_INVERTER_MOTOR_SPECIFIC_COEFF3, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F460	460	Motor specific coefficient 3
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_MOTOR_SPECIFIC_COEFF9,K_DA130A1F25F3_INVERTER_MOTOR_SPECIFIC_COEFF9, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F495	495	Motor specific coefficient 9
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACCELERATION_TIME2,   K_DA130A1F25F3_INVERTER_ACCELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F500	500	Acceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DECELERATION_TIME2,   K_DA130A1F25F3_INVERTER_DECELERATION_TIME2, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F501	501	Deceleration time 2
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACC_DEC_1e2_SW_FREQ,  K_DA130A1F25F3_INVERTER_ACC_DEC_1e2_SW_FREQ, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F505	505	Acceleration/deceleration 1 and 2 switching frequency
                //Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STALL_PREVENTION_L1,  K_DA130A1F25F3_INVERTER_STALL_PREVENTION_L1, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F601	601	Stall prevention level 1
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_RIL_FASE_SELECTION,   K_DA130A1F25F3_INVERTER_RIL_FASE_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F608	608 0 Selezione rilevamento fase ingresso
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_UNIT_SELECTION,       K_DA130A1F25F3_INVERTER_UNIT_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // f701 Selezione unit� corrente/tensione Dspu		change parameters from % to nominal value	1   ??????
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_TIMEOUT_COMUNICATION,   K_DA130A1F25F3_INVERTER_TIMEOUT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F803 Time-out tempo di comunicazione		Unit: 0.1s
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_ACTION_TMT_COMUNICATION,K_DA130A1F25F3_INVERTER_ACTION_TMT_COMUNICATION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F804 Azione time-out comunicazione		Unit: 0, 1, 2 -> Decellerazione fino all'arresto						
//                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9P, K_DA130A1F25F3_INVERTER_FACTORY_SPEC_COEFF9P, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F908	908	Factory specific coefficient 9P
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9D, K_DA130A1F25F3_INVERTER_FACTORY_SPEC_COEFF9D, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F909	909	Factory specific coefficient 9D
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_CURR_LEV, K_DA130A1F25F3_INVERTER_STEPOUT_DET_CURR_LEV, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F910	910	Step-out detection current level
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_TIME,     K_DA130A1F25F3_INVERTER_STEPOUT_DET_TIME, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F911	911	Step-out detection time
#if (K_AUTOTUNING_OFF == 1)		
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_QAXIS_INDUCTANCE,     K_DA130A1F25F3_INVERTER_QAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F912	912	q-axis inductance (Autotuning)	Unit: 0.01 Henry
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_DAXIS_INDUCTANCE,     K_DA130A1F25F3_INVERTER_DAXIS_INDUCTANCE, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// F913	913	d-axis inductance (Autotuning)	Unit: 0.01 Henry			
#endif
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_UNDERVOLTAGE_TRIP_ALARM_SEL, K_GENERAL_INVERTER_UNDERVOLTAGE_ERROR, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            

                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_PM_CONTROL_SELECTION, K_DA130A1F25F3_INVERTER_PM_CONTROL_SELECTION, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F915	915	PM control selection
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_FACTORY_SPEC_COEFF9L, K_DA130A1F25F3_INVERTER_FACTORY_SPEC_COEFF9L, MaxModWriteWait, MaxModWriteRetries, WriteModCmd); // F921	921	Factory specific coefficient 9L            
                
				Inverter->StepOutCurrentWarningDB = K_GENERAL_INVERTER_STEPOUT_DISABLE;
				Inverter->StepOutCurrentWarningWork = K_GENERAL_INVERTER_STEPOUT_ENABLE;		
            break;			
        }
    }    
}
#endif

void InverterCheckVoltage(int pointer, TypInverter * Inverter, TypTimer * TimerVoltage)
{
#if(K_Supply_Inverter==283 || K_Supply_Inverter==566)    
        int ValMin = (int)((float)k_Voltage_Inverter_Min_Ok / (float)k_Voltage_Inverter_Nominal * 10000.0);
        int ValMax = (int)((float)k_Voltage_Inverter_Max_Ok / (float)k_Voltage_Inverter_Nominal * 10000.0);
        
        if(Inverter->Ok_Voltage==0)
        {
            TimerVoltage->Time = K_Time_Resume_Ok_Voltage;
            TimerVoltage->Enable = (Inverter->In_Voltage>ValMin) && (Inverter->In_Voltage<ValMax) && (Inverter->Ok_Voltage==0);

            if(!TimerVoltage->Enable)
            {
                TimerVoltage->TimeOut = 0;
                TimerVoltage->Value = 0;
                
            }

            if(TimerVoltage->TimeOut)
            {
                Inverter->Ok_Voltage = 1;
                TimerVoltage->TimeOut = 0;
                TimerVoltage->Value = 0;
            }
        }
        else
        {
            TimerVoltage->Time = K_Time_Error_Ok_Voltage;
            TimerVoltage->Enable = ((Inverter->In_Voltage<ValMin) || (Inverter->In_Voltage>ValMax)) && (Inverter->Ok_Voltage==1);

            if(!TimerVoltage->Enable)
            {
                TimerVoltage->TimeOut = 0;
                TimerVoltage->Value = 0;
                
            }

            if(TimerVoltage->TimeOut)
            {
                Inverter->Ok_Voltage = 0;
                if(EngineBox[pointer].EngineIsOn)
                    EngineBox[pointer].GoShutDwComp = 1;
                TimerVoltage->TimeOut = 0;
                TimerVoltage->Value = 0;
            }           
        }
        if(Inverter->PermanentOffLine)
            Inverter->Ok_Voltage = 1;
#else
        Inverter->Ok_Voltage = 1;
#endif
}

/*
void DC_Braking(TypEngineBox * CompNumber, TypInverter * Inverter, int CompAddr, int onoff)
{
    if(CompNumber->Out_Inverter==0)
    {
        Change_Protocol_BUSS(K_ModBus_Baud38400);							// Imposto protocollo (1=MyBus, 2=ModBus, 3=ModBus @57600)        
        Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_STATUS, 1, MaxModReadWait, MaxModReadRetries);
        
        if(onoff==1)
        {
			
			if(CompNumber->Temperature.Compressor_Output<K_No_Braking_Temp-100)
			{
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_STEPOUT_DET_TIME, Inverter->StepOutCurrentWarningDB, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);						
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_BREAK_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);
			}
			else if(CompNumber->Temperature.Compressor_Output>K_No_Braking_Temp)
			{
				Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_STOP_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);	// Lo mando in STOP
			}
        }
        else
        {
            if(!Inverter->OffLine)	
                Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_COMMAND_SET, K_VAL_INVERTER_STOP_COMMAND, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);
        }
                
        //Change_Protocol_BUSS(K_MyBus_Baud57600);							// Imposto protocollo (1=MyBus, 2=ModBus, 3=ModBus @57600)
    }
}


void Set_Braking_Current(TypInverter * Inverter, int CompAddr, int CurrentValuePerc)
{
    Change_Protocol_BUSS(K_ModBus_Baud38400);							// Imposto protocollo (1=MyBus, 2=ModBus, 3=ModBus @57600)
	Inverter->OffLine = Send_WaitModRx(CompAddr, ReadModCmd, REG_INVERTER_STATUS, 1, MaxModReadWait, MaxModReadRetries);
    
    if(!Inverter->OffLine)	
        Inverter->OffLine = Send_WriteModReg(CompAddr, REG_INVERTER_BRAKE_CURRENT_SET, CurrentValuePerc, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);
    //Change_Protocol_BUSS(K_MyBus_Baud57600);							// Imposto protocollo (1=MyBus, 2=ModBus)
}
*/


void Search_Touch(void)
{/*
	Result = 0;

	// Scansiono la rete alla ricerca dei touch
	for(i=0; i<Max_Room; i++)
	{
		Result = Send_WaitModRx(Add_TouchRoom1+i, ReadModCmd, REG_TOUCH_PAGE, 1, MaxModChkWait, MaxModChkRetries);
		Room[i].Touch_OffLine = (Result != 0);
		Mod_LastRx.Valid_Data = 0;
	}
*/
}

/*void Resume_TouchOffLine(char * Cnt)
{
	Result = 0;
	
	// Provo a recuperare i moduli Touch
	if((Room[(*Cnt)].Enable == 1) && (Room[(*Cnt)].Touch_OffLine == 1))
	{
		Result = Send_WaitModRx(Add_TouchRoom1 +(*Cnt), ReadModCmd, REG_TOUCH_PAGE, 1, MaxModChkWait, MaxModChkRetries);
		Room[(*Cnt)].Touch_OffLine = (Result != 0);
		Mod_LastRx.Valid_Data = 0;
	}
	(*Cnt) += 1;
	if((*Cnt) >= Max_Room)
		(*Cnt) = 0;

}*/
/**/
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

void RefreshTouchInPut(char TouchAddres)
{/*
	DataIndex = TouchAddres-Add_TouchRoom1;
	if((Room[DataIndex].Enable | Dimmer[DataIndex].Enable)& !Room[DataIndex].Touch_OffLine)		// se abilitato e OnLine
	{
		ReadModReg (TouchAddres, REG_TOUCH_PAGE, &(Room[DataIndex].Touch_Current_Page));
		switch((char)Room[DataIndex].Touch_Current_Page)
		{
			case 1:		// pagina 1 -> Home
				if(!Room[DataIndex].Touch_OffLine)
				{
					if(Send_WaitModRx(TouchAddres, ReadModCmd, REG_TOUCH_SP_ROOM, 3, MaxModReadWait, MaxModReadRetries))	// Il registro dei touch hanno un offset di 1
						Room[DataIndex].Touch_OffLine = 1;
					else
					{
						Room[DataIndex].Set_Point = ExtracReg(1);
						Room[DataIndex].Velocita_Ventilatore = ExtracReg(2);
						Room[DataIndex].Mode = ExtracReg(3);
					}
					Mod_LastRx.Valid_Data = 0;
				}
				break;

			case 2:		// pagina 2 -> Inverter alimentazione
				if(!Room[DataIndex].Touch_OffLine)
				{
					if(Send_WaitModRx(TouchAddres, ReadModCmd, REG_TOUCH_MAX_PWR_LIMIT, 3, MaxModReadWait, MaxModReadRetries))	// Il registro dei touch hanno un offset di 1
						Room[DataIndex].Touch_OffLine = 1;
					else
					{
						if(!Room[DataIndex].RequestPowerLimitRefresh)
						{
							int ReadBuffTmp;
							ReadBuffTmp = ExtracReg(1);

							if(ReadBuffTmp != EngineBox[0].Inverter_Max_Power_Out)
							{
								EngineBox[0].Inverter_Max_Power_Out = ReadBuffTmp;
								for(int i = 0; i<Max_Room; i++)
								{
									if(Room[i].Enable)
										Room[i].RequestPowerLimitRefresh = 1;
								}
							}
						}
					}
					Mod_LastRx.Valid_Data = 0;
				}
				break;

			case 3:		// pagina 3 -> Dimmer
				if(!Room[DataIndex].Touch_OffLine & Dimmer[DataIndex].Enable) // & !Dimmer[DataIndex].RequestRefresh)
				{
					if(Send_WaitModRx(TouchAddres, ReadModCmd, REG_TOUCH_DIMMER_1_A, 6, MaxModReadWait, MaxModReadRetries))	// Il registro dei touch hanno un offset di 1
						Room[DataIndex].Touch_OffLine = 1;
					else
					{
						int ReadBuffTmp;
						if(!Dimmer[DataIndex].RefreshT & !Dimmer[DataIndex].RequestRefresh)
						{
							// canale 1
							if(Dimmer[DataIndex].Ch1 != Dimmer[DataIndex].HoldCh1)		// se il dimmer ha cambiato valore
							{
								Dimmer[DataIndex].HoldCh1 = Dimmer[DataIndex].Ch1;		// aggiorno i touch
								Dimmer[DataIndex].RefreshT = 1;
							}
							else
							{
								ReadBuffTmp = ExtracReg(1);
								if(ReadBuffTmp != Dimmer[DataIndex].Ch1)
								{
									Dimmer[DataIndex].Ch1 = ReadBuffTmp;
									Dimmer[DataIndex].HoldCh1 = ReadBuffTmp;
									Dimmer[DataIndex].RequestRefresh = 1;
								}
							}
						
							// canale 2
							if(Dimmer[DataIndex].Ch2 != Dimmer[DataIndex].HoldCh2)		// se il dimmer ha cambiato valore
							{
								Dimmer[DataIndex].HoldCh2 = Dimmer[DataIndex].Ch2;		// aggiorno i touch
								Dimmer[DataIndex].RefreshT = 1;
							}
							else
							{
								ReadBuffTmp = ExtracReg(2);
								if(ReadBuffTmp != Dimmer[DataIndex].Ch2)
								{
									Dimmer[DataIndex].Ch2 = ReadBuffTmp;
									Dimmer[DataIndex].HoldCh2 = ReadBuffTmp;
									Dimmer[DataIndex].RequestRefresh = 1;
								}
							}

							// canale 3
							if(Dimmer[DataIndex].Ch3 != Dimmer[DataIndex].HoldCh3)		// se il dimmer ha cambiato valore
							{
								Dimmer[DataIndex].HoldCh3 = Dimmer[DataIndex].Ch3;		// aggiorno i touch
								Dimmer[DataIndex].RefreshT = 1;
							}
							else
							{
								ReadBuffTmp = ExtracReg(3);
								if(ReadBuffTmp != Dimmer[DataIndex].Ch3)
								{
									Dimmer[DataIndex].Ch3 = ReadBuffTmp;
									Dimmer[DataIndex].HoldCh3 = ReadBuffTmp;
									Dimmer[DataIndex].RequestRefresh = 1;
								}
							}

							// canale 4
							if(Dimmer[DataIndex].Ch4 != Dimmer[DataIndex].HoldCh4)		// se il dimmer ha cambiato valore
							{
								Dimmer[DataIndex].HoldCh4 = Dimmer[DataIndex].Ch4;		// aggiorno i touch
								Dimmer[DataIndex].RefreshT = 1;
							}
							else
							{
								ReadBuffTmp = ExtracReg(4);
								if(ReadBuffTmp != Dimmer[DataIndex].Ch4)
								{
									Dimmer[DataIndex].Ch4 = ReadBuffTmp;
									Dimmer[DataIndex].HoldCh4 = ReadBuffTmp;
									Dimmer[DataIndex].RequestRefresh = 1;
								}
							}

							// canale 5
							if(Dimmer[DataIndex].Ch5 != Dimmer[DataIndex].HoldCh5)		// se il dimmer ha cambiato valore
							{
								Dimmer[DataIndex].HoldCh5 = Dimmer[DataIndex].Ch5;		// aggiorno i touch
								Dimmer[DataIndex].RefreshT = 1;
							}
							else
							{
								ReadBuffTmp = ExtracReg(5);
								if(ReadBuffTmp != Dimmer[DataIndex].Ch5)
								{
									Dimmer[DataIndex].Ch5 = ReadBuffTmp;
									Dimmer[DataIndex].HoldCh5 = ReadBuffTmp;
									Dimmer[DataIndex].RequestRefresh = 1;
								}
							}

							// canale 6
							if(Dimmer[DataIndex].Ch6 != Dimmer[DataIndex].HoldCh6)		// se il dimmer ha cambiato valore
							{
								Dimmer[DataIndex].HoldCh6 = Dimmer[DataIndex].Ch6;		// aggiorno i touch
								Dimmer[DataIndex].RefreshT = 1;
							}
							else
							{
								ReadBuffTmp = ExtracReg(6);
								if(ReadBuffTmp != Dimmer[DataIndex].Ch6)
								{
									Dimmer[DataIndex].Ch6 = ReadBuffTmp;
									Dimmer[DataIndex].HoldCh6 = ReadBuffTmp;
									Dimmer[DataIndex].RequestRefresh = 1;
								}
							}
						}
					}
					Mod_LastRx.Valid_Data = 0;
				}
				break;

			case 4:		// pagina 4 -> Infrarosso
				if(!Room[DataIndex].Touch_OffLine)
				{
					if(Send_WaitModRx(TouchAddres, ReadModCmd, REG_TOUCH_INFRA_RED_MODE, 2, MaxModReadWait, MaxModReadRetries))	// Il registro dei touch hanno un offset di 1
						Room[DataIndex].Touch_OffLine = 1;
					else
					{
						Room[DataIndex].Infrarosso_Mode = ExtracReg(1);
						//Room[DataIndex].REG_TOUCH_PRESET_INFRA_RED = ExtracReg(2);
					}
					Mod_LastRx.Valid_Data = 0;
				}
				break;

			case 5:		// pagina 5 -> Diagnostica generale
				break;

			case 6:		// pagina 6 -> Diagnostica Box motore
				break;

			case 7:		// pagina 7 -> Diagnostica Room 1
				break;

			case 8:		// pagina 8 -> Diagnostica Room 2
				break;

			case 9:		// pagina 9 -> Diagnostica Room 3
				break;

			case 10:	// pagina 10 -> Diagnostica Room 4
				break;

			case 11:	// pagina 11 -> Diagnostica Room 5
				break;

			case 12:	// pagina 12 -> Diagnostica Room 6
				break;

			case 13:	// pagina 13 -> Diagnostica Room 7
				break;

			case 14:	// pagina 14 -> Diagnostica Room 8
				break;

			case 15:	// pagina 15 -> Scheduler
				break;
		}
	}
*/
}

void RefreshAllTouchInPut(void)
{/*
	for(i=1; i<= Max_Room; i++)
		RefreshTouchInPut(i);
*/
}

void RefreshTouchOutPut(char TouchAddres)
{/*
	int DataIndex = 0;
	DataIndex = TouchAddres-Add_TouchRoom1;
	if((Room[DataIndex].Enable | Dimmer[DataIndex].Enable) & !Room[DataIndex].Touch_OffLine)		// se abilitato e OnLine
	{
		int SysError = 0;

		if(Room[DataIndex].RequestPowerLimitRefresh)				// controllo sempre l'aggionamento del limite
		{
			WriteModReg (TouchAddres, REG_TOUCH_MAX_PWR_LIMIT, EngineBox[0].Inverter_Max_Power_Out);
			if(!Room[DataIndex].Touch_OffLine)
				Room[DataIndex].RequestPowerLimitRefresh = 0;
		}

		switch((char)Room[DataIndex].Touch_Current_Page)
		{
			case 1:		// pagina 1 -> Home
				InsertReg(1, Room[DataIndex].Temp_Aria_In);		// REG_TOUCH_ROOM_TEMP
				InsertReg(2, EngineBox[0].Out_Inverter);		// REG_TOUCH_POWER_MOT
				WriteModListReg (TouchAddres, REG_TOUCH_ROOM_TEMP, 2);
				break;

			case 2:		// pagina 2 -> Inverter alimentazione
//				InsertReg(1, Room[DataIndex].Temp_Aria_In);		// REG_TOUCH_NET_VOLTAGE
//				InsertReg(2, Room[DataIndex].Temp_Aria_In);		// REG_TOUCH_SERVICE_BATTERY_VOLTAGE
//				InsertReg(3, Room[DataIndex].Temp_Aria_In);		// REG_TOUCH_INVERTER_POWER_OUT
//				WriteModListReg (TouchAddres, REG_TOUCH_NET_VOLTAGE, 3);
//				break;

			case 3:		// pagina 3 -> Dimmer
				if(Dimmer[DataIndex].Enable & Dimmer[DataIndex].RefreshT)
				{
					InsertReg(1, Dimmer[DataIndex].Ch1);
					InsertReg(2, Dimmer[DataIndex].Ch2);
					InsertReg(3, Dimmer[DataIndex].Ch3);
					InsertReg(4, Dimmer[DataIndex].Ch4);
					InsertReg(5, Dimmer[DataIndex].Ch5);
					InsertReg(6, Dimmer[DataIndex].Ch6);
					WriteModListReg (TouchAddres, REG_TOUCH_DIMMER_1_A, 6);
					if(!Room[DataIndex].Touch_OffLine)
						Dimmer[DataIndex].RefreshT = 0;
				}
				break;


			case 4:		// pagina 4 -> Infrarosso
				break;

			case 5:		// pagina 5 -> Diagnostica generale
				SysError | = 0;		// errore refrigerante
				SysError << 1;
				SysError | = ((EngineBox[0].Stato_Ingressi & 0x0002) != 0);		// allagamento
				SysError << 1;
				SysError | = 0;		// ostruzione pompa
				SysError << 1;
				SysError | = ((EngineBox[0].Stato_Ingressi & 0x0001) != 0);		// Termica compressore
				SysError << 1;

				SysError | = !Room[3].Split_OffLine;
				SysError << 1;
				SysError | = !Room[2].Split_OffLine;
				SysError << 1;
				SysError | = !Room[1].Split_OffLine;
				SysError << 1;
				SysError | = !Room[0].Split_OffLine;
				SysError << 1;

				SysError | = !Room[3].Valvola_OffLine;
				SysError << 1;
				SysError | = !Room[2].Valvola_OffLine;
				SysError << 1;
				SysError | = !Room[1].Valvola_OffLine;
				SysError << 1;
				SysError | = !Room[0].Valvola_OffLine;
				SysError << 1;
				
				SysError | = !Room[3].Touch_OffLine;
				SysError << 1;
				SysError | = !Room[2].Touch_OffLine;
				SysError << 1;
				SysError | = !Room[1].Touch_OffLine;
				SysError << 1;
				SysError | = !Room[0].Touch_OffLine;

				InsertReg(1, SysError);								// REG_TOUCH_SYS_ERROR_1
				InsertReg(2, 0x0000);								// REG_TOUCH_SYS_ERROR_2
				WriteModListReg (TouchAddres, REG_TOUCH_SYS_ERROR_1, 2);
				break;

			case 6:		// pagina 6 -> Diagnostica Box motore
				InsertReg(1, EngineBox[0].Temperature.Compressor_Output);	// REG_TOUCH_MANDATA_COMPRESSORE
				InsertReg(2, EngineBox[0].Temperature.Compressor_Suction);	// REG_TOUCH_RITORNO_COMPRESSORE
				InsertReg(3, EngineBox[0].Temperature.Condenser);			// REG_TOUCH_Temp_Condenser
				InsertReg(4, EngineBox[0].Temperature.Sea_Water);				// REG_TOUCH_SEA_WATER_IN_TEMP
				InsertReg(5, 0):	//EngineBox[0].Temperature.Sea_Water);	// REG_TOUCH_SEA_WATER_OUT_TEMP
				InsertReg(6, EngineBox[0].Pressione_Gas_P);				// REG_TOUCH_PRESSIONE_P
				InsertReg(7, EngineBox[0].Pressure.Gas);				// REG_TOUCH_PRESSIONE_G
				InsertReg(8, EngineBox[0].Ric_Temp);					// REG_TOUCH_TEMP_RICHIESTA
				InsertReg(9, EngineBox[0].Ric_Pressione);				// REG_TOUCH_PRESSIONE_RICHIESTA
				InsertReg(10, EngineBox[0].StatoCompressore);			// REG_TOUCH_STATO_COMPRESSORE
				WriteModListReg (TouchAddres, REG_TOUCH_MANDATA_COMPRESSORE, 10);
				WriteModReg (TouchAddres, REG_TOUCH_POWER_MOT, EngineBox[0].Out_Inverter);
				break;

			case 7:		// pagina 7 -> Diagnostica Room 1
				#if (Max_Room > 0)
					InsertReg(1, Room[0].Temp_Piccolo);					// REG_TOUCH_PICCOLO_SPLIT_1
					InsertReg(2, Room[0].Temp_Grande);					// REG_TOUCH_GRANDE_SPLIT_1
					InsertReg(3, Room[0].Temp_Aria_In);					// REG_TOUCH_ARIA_INGRESSO_SPLIT_1
					InsertReg(4, Room[0].Temp_Aria_Out);				// REG_TOUCH_ARIA_USCITA_SPLIT_1
					InsertReg(5, Room[0].Velocita_Ventilatore);			// REG_TOUCH_VENTILATORE_SPLIT_1
					InsertReg(6, Room[0].Posizione_Valvola);			// REG_TOUCH_STATO_VALVOLA_SPLIT_1
					InsertReg(7, Room[0].Temp_Evaporazione);			// REG_TOUCH_TEMP_EVAP_SPLIT_1
					InsertReg(8, Room[0].Pressure_G);					// REG_TOUCH_PRESSIONE_G_SPLIT_1
					InsertReg(9, PressureGas(Room[0].Temp_Evaporazione));// REG_TOUCH_RICH_PRESSIONE_SPLIT_1
					WriteModListReg (TouchAddres, REG_TOUCH_PICCOLO_SPLIT_1, 9);
				#endif
				break;

			case 8:		// pagina 8 -> Diagnostica Room 2
				#if (Max_Room > 1)
					InsertReg(1, Room[1].Temp_Piccolo);					// REG_TOUCH_PICCOLO_SPLIT_2
					InsertReg(2, Room[1].Temp_Grande);					// REG_TOUCH_GRANDE_SPLIT_2
					InsertReg(3, Room[1].Temp_Aria_In);					// REG_TOUCH_ARIA_INGRESSO_SPLIT_2
					InsertReg(4, Room[1].Temp_Aria_Out);				// REG_TOUCH_ARIA_USCITA_SPLIT_2
					InsertReg(5, Room[1].Velocita_Ventilatore);			// REG_TOUCH_VENTILATORE_SPLIT_2
					InsertReg(6, Room[1].Posizione_Valvola);			// REG_TOUCH_STATO_VALVOLA_SPLIT_2
					InsertReg(7, Room[1].Temp_Evaporazione);			// REG_TOUCH_TEMP_EVAP_SPLIT_2
					InsertReg(8, Room[1].Pressure_G);					// REG_TOUCH_PRESSIONE_G_SPLIT_2
					InsertReg(9, PressureGas(Room[1].Temp_Evaporazione));// REG_TOUCH_RICH_PRESSIONE_SPLIT_2
					WriteModListReg (TouchAddres, REG_TOUCH_PICCOLO_SPLIT_2, 9);
				#endif
				break;

			case 9:		// pagina 9 -> Diagnostica Room 3
				#if (Max_Room > 2)
					InsertReg(1, Room[2].Temp_Piccolo);					// REG_TOUCH_PICCOLO_SPLIT_3
					InsertReg(2, Room[2].Temp_Grande);					// REG_TOUCH_GRANDE_SPLIT_3
					InsertReg(3, Room[2].Temp_Aria_In);					// REG_TOUCH_ARIA_INGRESSO_SPLIT_3
					InsertReg(4, Room[2].Temp_Aria_Out);				// REG_TOUCH_ARIA_USCITA_SPLIT_3
					InsertReg(5, Room[2].Velocita_Ventilatore);			// REG_TOUCH_VENTILATORE_SPLIT_3
					InsertReg(6, Room[2].Posizione_Valvola);			// REG_TOUCH_STATO_VALVOLA_SPLIT_3
					InsertReg(7, Room[2].Temp_Evaporazione);			// REG_TOUCH_TEMP_EVAP_SPLIT_3
					InsertReg(8, Room[2].Pressure_G);					// REG_TOUCH_PRESSIONE_G_SPLIT_3
					InsertReg(9, PressureGas(Room[2].Temp_Evaporazione));// REG_TOUCH_RICH_PRESSIONE_SPLIT_3
					WriteModListReg (TouchAddres, REG_TOUCH_PICCOLO_SPLIT_3, 9);
				#endif
				break;

			case 10:	// pagina 10 -> Diagnostica Room 4
				#if (Max_Room > 3)
					InsertReg(1, Room[3].Temp_Piccolo);					// REG_TOUCH_PICCOLO_SPLIT_4
					InsertReg(2, Room[3].Temp_Grande);					// REG_TOUCH_GRANDE_SPLIT_4
					InsertReg(3, Room[3].Temp_Aria_In);					// REG_TOUCH_ARIA_INGRESSO_SPLIT_4
					InsertReg(4, Room[3].Temp_Aria_Out);				// REG_TOUCH_ARIA_USCITA_SPLIT_4
					InsertReg(5, Room[3].Velocita_Ventilatore);			// REG_TOUCH_VENTILATORE_SPLIT_4
					InsertReg(6, Room[3].Posizione_Valvola);			// REG_TOUCH_STATO_VALVOLA_SPLIT_4
					InsertReg(7, Room[3].Temp_Evaporazione);			// REG_TOUCH_TEMP_EVAP_SPLIT_4
					InsertReg(8, Room[3].Pressure_G);					// REG_TOUCH_PRESSIONE_G_SPLIT_4
					InsertReg(9, PressureGas(Room[3].Temp_Evaporazione));// REG_TOUCH_RICH_PRESSIONE_SPLIT_4
					WriteModListReg (TouchAddres, REG_TOUCH_PICCOLO_SPLIT_4, 9);
				#endif
				break;

			case 11:	// pagina 11 -> Diagnostica Room 5
				#if (Max_Room > 4)
					InsertReg(1, Room[4].Temp_Piccolo);					// REG_TOUCH_PICCOLO_SPLIT_5
					InsertReg(2, Room[4].Temp_Grande);					// REG_TOUCH_GRANDE_SPLIT_5
					InsertReg(3, Room[4].Temp_Aria_In);					// REG_TOUCH_ARIA_INGRESSO_SPLIT_5
					InsertReg(4, Room[4].Temp_Aria_Out);				// REG_TOUCH_ARIA_USCITA_SPLIT_5
					InsertReg(5, Room[4].Velocita_Ventilatore);			// REG_TOUCH_VENTILATORE_SPLIT_5
					InsertReg(6, Room[4].Posizione_Valvola);			// REG_TOUCH_STATO_VALVOLA_SPLIT_5
					InsertReg(7, Room[4].Temp_Evaporazione);			// REG_TOUCH_TEMP_EVAP_SPLIT_5
					InsertReg(8, Room[4].Pressure_G);					// REG_TOUCH_PRESSIONE_G_SPLIT_5
					InsertReg(9, PressureGas(Room[4].Temp_Evaporazione));// REG_TOUCH_RICH_PRESSIONE_SPLIT_5
					WriteModListReg (TouchAddres, REG_TOUCH_PICCOLO_SPLIT_5, 9);
				#endif
				break;

			case 12:	// pagina 12 -> Diagnostica Room 6
				#if (Max_Room > 5)
					InsertReg(1, Room[5].Temp_Piccolo);					// REG_TOUCH_PICCOLO_SPLIT_6
					InsertReg(2, Room[5].Temp_Grande);					// REG_TOUCH_GRANDE_SPLIT_6
					InsertReg(3, Room[5].Temp_Aria_In);					// REG_TOUCH_ARIA_INGRESSO_SPLIT_6
					InsertReg(4, Room[5].Temp_Aria_Out);				// REG_TOUCH_ARIA_USCITA_SPLIT_6
					InsertReg(5, Room[5].Velocita_Ventilatore);			// REG_TOUCH_VENTILATORE_SPLIT_6
					InsertReg(6, Room[5].Posizione_Valvola);			// REG_TOUCH_STATO_VALVOLA_SPLIT_6
					InsertReg(7, Room[5].Temp_Evaporazione);			// REG_TOUCH_TEMP_EVAP_SPLIT_6
					InsertReg(8, Room[5].Pressure_G);					// REG_TOUCH_PRESSIONE_G_SPLIT_6
					InsertReg(9, PressureGas(Room[5].Temp_Evaporazione));// REG_TOUCH_RICH_PRESSIONE_SPLIT_6
					WriteModListReg (TouchAddres, REG_TOUCH_PICCOLO_SPLIT_6, 9);
				#endif
				break;

			case 13:	// pagina 13 -> Diagnostica Room 7
				#if (Max_Room > 6)
					InsertReg(1, Room[6].Temp_Piccolo);					// REG_TOUCH_PICCOLO_SPLIT_7
					InsertReg(2, Room[6].Temp_Grande);					// REG_TOUCH_GRANDE_SPLIT_7
					InsertReg(3, Room[6].Temp_Aria_In);					// REG_TOUCH_ARIA_INGRESSO_SPLIT_7
					InsertReg(4, Room[6].Temp_Aria_Out);				// REG_TOUCH_ARIA_USCITA_SPLIT_7
					InsertReg(5, Room[6].Velocita_Ventilatore);			// REG_TOUCH_VENTILATORE_SPLIT_7
					InsertReg(6, Room[6].Posizione_Valvola);			// REG_TOUCH_STATO_VALVOLA_SPLIT_7
					InsertReg(7, Room[6].Temp_Evaporazione);			// REG_TOUCH_TEMP_EVAP_SPLIT_7
					InsertReg(8, Room[6].Pressure_G);					// REG_TOUCH_PRESSIONE_G_SPLIT_7
					InsertReg(9, PressureGas(Room[6].Temp_Evaporazione));// REG_TOUCH_RICH_PRESSIONE_SPLIT_7
					WriteModListReg (TouchAddres, REG_TOUCH_PICCOLO_SPLIT_7, 9);
				#endif
				break;

			case 14:	// pagina 14 -> Diagnostica Room 8
				#if (Max_Room > 7)
					InsertReg(1, Room[7].Temp_Piccolo);					// REG_TOUCH_PICCOLO_SPLIT_8
					InsertReg(2, Room[7].Temp_Grande);					// REG_TOUCH_GRANDE_SPLIT_8
					InsertReg(3, Room[7].Temp_Aria_In);					// REG_TOUCH_ARIA_INGRESSO_SPLIT_8
					InsertReg(4, Room[7].Temp_Aria_Out);				// REG_TOUCH_ARIA_USCITA_SPLIT_8
					InsertReg(5, Room[7].Velocita_Ventilatore);			// REG_TOUCH_VENTILATORE_SPLIT_8
					InsertReg(6, Room[7].Posizione_Valvola);			// REG_TOUCH_STATO_VALVOLA_SPLIT_8
					InsertReg(7, Room[7].Temp_Evaporazione);			// REG_TOUCH_TEMP_EVAP_SPLIT_8
					InsertReg(8, Room[7].Pressure_G);					// REG_TOUCH_PRESSIONE_G_SPLIT_8
					InsertReg(9, PressureGas(Room[7].Temp_Evaporazione));// REG_TOUCH_RICH_PRESSIONE_SPLIT_8
					WriteModListReg (TouchAddres, REG_TOUCH_PICCOLO_SPLIT_8, 9);
				#endif
				break;

			case 15:	// pagina 15 -> Scheduler
				break;
		}
	}
*/
}

void RefreshAllTouchOutPut(void)
{/*
	for(i=1; i<=Max_Room; i++)
		RefreshTouchOutPut(i);
*/
}

/*
void InitTouch(void)
{
	for(i=0; i<Max_Room; i++)
	{
		if(Room[i].Enable && !Room[i].Touch_OffLine)
		{
			WriteModReg (Add_TouchRoom1 +i, REG_TOUCH_SP_ROOM, 2000);
			WriteModReg (Add_TouchRoom1 +i, REG_TOUCH_FAN_SPEED, 1);
			WriteModReg (Add_TouchRoom1 +i, REG_TOUCH_PWR, 0);
			WriteModReg (Add_TouchRoom1 +i, REG_TOUCH_MODE, 2);
			WriteModReg (Add_TouchRoom1 +i, REG_TOUCH_MAX_PWR_LIMIT, 255);
			WriteModReg (Add_TouchRoom1 +i, REG_TOUCH_POWER_ON_GENERATOR_LIMIT, 255);
			WriteModReg (Add_TouchRoom1 +i, REG_TOUCH_POWER_SOURCE, 1);
			WriteModReg (Add_TouchRoom1 +i, REG_TOUCH_INFRA_RED_MODE, 1);
			WriteModReg (Add_TouchRoom1 +i, REG_TOUCH_PRESET_INFRA_RED, 1);
	
			WriteModReg (Add_TouchRoom1 +i, REG_TOUCH_MATRICOLA_LO, 0x3210);
			WriteModReg (Add_TouchRoom1 +i, REG_TOUCH_MATRICOLA_HI, 0x7654);
			WriteModReg (Add_TouchRoom1 +i, REG_TOUCH_MASTER_FW_VERSION, 2);
		}
	}
 }
*/
/**/
void InitTouch(void)
{
	int StopWait = 0;
	int x;
	
	//oErrLed = LED_ON;
	
	//while((TimerStartUp.TimeOut == 0) && (StopWait == 0))
	while(StopWait == 0)			// Attendo che il touch sia OnLine.....
	{
		Touch.OffLine = 0;
		ReadModReg (k_Add_Touch, REG_TOUCH_PAGE, &(Touch.Page));
		//ReadTouchReg (Add_Touch, REG_TOUCH_PAGE, &(Touch.Page));	// leggo la pagina
		if(Touch.OffLine == 0) StopWait = 1;
		
		//oLedEEV = !oLedEEV;				// Lampeggio LED Giallo per segnalare attesa Touch OnLine
		oErrLed = !oErrLed;
				
		DelaymSec(100); // for(x=0; x<9000; x++);			// attesa xx mS...
	}

	oErrLed = LED_OFF;

	if(!Touch.OffLine)		// se Touch OnLine aggiorno valori letti da EEPROM
	{	
		WriteModReg (k_Add_Touch, 226, 8); //3);        
		WriteModReg (k_Add_Touch, 140, 1);		// Attivo flag su Touch per gestione Hide/View pagine riservate a MasterSlave
		WriteModReg (k_Add_Touch, 149, 2);	// Invio la Nazione sul touch per modificare dinamicamente gli oggetti e unit� di misura sul touch a seconda della Nazione (USA/EUR)	      
		WriteModReg (k_Add_Touch, REG_TOUCH_PAGE, 9);//2);		//BOX	
		
		//GL InitTouchE2Value();
	}	
	

}	



void Init_Bridge(int bridgeNumber)
{
#if(K_HiSpeedU1ModBUS==1)            
    Change_Protocol_BUSM(K_ModBus_Baud115200);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#else
    Change_Protocol_BUSM(K_ModBus_Baud57600);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#endif
            
	switch(bridgeNumber)
	{
        /*
		case 1:			// Scheda Bridge n.1 - Verso Master UTA-H
			if(Check_IfBridgeIsPresent(1))
			{
				Send_InitDataToBridge(1);
				WaitEnd_DataFromBridge(1);
			}
		break;
		*/
		case 2:			// Scheda Bridge n.2 - Verso DL
			if(Check_IfBridgeIsPresent(2))
			{
                if(Bridge[1].InitializeBridge==0)
                {
                    Send_InitDataToBridge(&Bridge[1], 2);	
                }
				WaitEnd_DataFromBridge(&Bridge[1], 2);	
			}
		break;
	}
}


		
int Check_IfBridgeIsPresent(int bridgeNumber)
{
	int OfflineFlag=0;
	int ReadWord;
	
#if(K_HiSpeedU1ModBUS==1)            
    Change_Protocol_BUSM(K_ModBus_Baud115200);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#else
    Change_Protocol_BUSM(K_ModBus_Baud57600);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#endif
	//DelaymSec(20);
	
	switch(bridgeNumber)
	{
        /*
		case 1:
			OfflineFlag = Send_WaitModRx(k_Bridge1_Addr, ReadModCmd, REG_BRIDGE1_STATUS_FLAGS, 1, MaxModReadWait, MaxModReadRetries);
			if(OfflineFlag==0)					// se OfflineFlag == 0 scheda presente
				ReadWord = ExtracReg(1);
		break;
		*/
		case 2:
			OfflineFlag = Send_WaitModRx(k_Bridge2_Addr, ReadModCmd, REG_BRIDGE2_STATUS_FLAGS, 1, MaxModReadWait, MaxModReadRetries);
			if(OfflineFlag==0)					// se OfflineFlag == 0 scheda presente
				ReadWord = ExtracReg(1);
		break;		
	}
	
	if(OfflineFlag==0)	
		return 1;				// Se scheda presente Ritorno 1
	else 
		return 0;				// altrimenti Ritorno 0
}

int Check_IfBridgeGoOnline(TypBridge * bridgeFlag, unsigned int bridgeAddr)
{
	#define K_Max_Offline_CNT		3
	
#if(K_HiSpeedU1ModBUS==1)            
    Change_Protocol_BUSM(K_ModBus_Baud115200);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#else
    Change_Protocol_BUSM(K_ModBus_Baud57600);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#endif
    
    if(bridgeFlag->PermanentOffLine)
        EngineBox[0].DisableModComErrorCounter=1;
	bridgeFlag->OffLine = Send_WaitModRx(bridgeAddr, ReadModCmd, 0, 1, 20/*MaxModReadWait*/, 1/*MaxModReadRetries*/);
    bridgeFlag->OnLine = !bridgeFlag->OffLine; 
    EngineBox[0].DisableModComErrorCounter=0;

    
	if(bridgeFlag->OnLine==0)					// se OfflineFlag == 0 scheda presente
	{
		if(bridgeFlag->PermanentOffLine == 0)
		{
			bridgeFlag->OffLineCounter += 1;
			if(bridgeFlag->OffLineCounter>=K_Max_Offline_CNT)
			{
				bridgeFlag->PermanentOffLine = 1;
                bridgeFlag->BridgeReady = 0;
                bridgeFlag->InitializeBridge = 0;
			}
		}
		return 0;
	}
    else if(bridgeFlag->BridgeReady==1)
	{
		bridgeFlag->OffLineCounter = 0;
		bridgeFlag->PermanentOffLine = 0;
        bridgeFlag->InitializeBridge = 0;
		return 0;
	}
	else if(bridgeFlag->OnLine==1 && bridgeFlag->PermanentOffLine == 1)
	{
		bridgeFlag->OffLineCounter = 0;
		//bridgeFlag->PermanentOffLine = 0;
		return 1;
	}
}

void Send_InitDataToBridge(TypBridge * bridgeFlag, int bridgeNumber)
{
	int temp[9]={0,0,0,0,0,0,0,0};
	
#if(K_HiSpeedU1ModBUS==1)            
    Change_Protocol_BUSM(K_ModBus_Baud115200);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#else
    Change_Protocol_BUSM(K_ModBus_Baud57600);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#endif
    
	switch(bridgeNumber)
	{
		case 1:			// Invio dei dati di inizializzazione verso Bridge 1 (Master UTA-H)
						// Srittura flag bit0 in REG0 per avvisare bridge che sto per inviare i dati di init
						// questo inibisce sul bridge le eventuali scritture lato DL
			InsertReg(1, 1);			// REG_BRIDGE1_STATUS_FLAGS 0				// Bridge Status Flags           
			Send_WaitModRx(k_Bridge1_Addr, WriteModListCmd, 0, 1, MaxModWriteWait, MaxModWriteRetries);		
			DelaymSec(1000);					
//-------------------------------------------------------------------------------------------------------
			//iniziare con INIT qui

			
			
			
			//Finire con INIT qui
//-------------------------------------------------------------------------------------------------------					
			DelaymSec(5);				
			InsertReg(1, 3);						//Reg0 REG_BRIDGE1_STATUS_FLAGS			// Bridge Status Flags                                                                               
			Send_WaitModRx(k_Bridge1_Addr, WriteModListCmd, 0, 1, MaxModWriteWait, MaxModWriteRetries);
			//DelaymSec(5);			
			break;
		
		case 2:			// Invio dei dati di inizializzazione verso Bridge 2 (Logica DL)	-> 190 dati in 3 blocchi da 64 word			
			// Srittura flag bit0 in REG0 per avvisare bridge che sto per inviare i dati di init
			// questo inibisce sul bridge le eventuali scritture lato DL
            
/*            
			InsertReg(1	, 1);			// 0				// Bridge Status Flags           
			Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, REG_BRIDGE2_STATUS_FLAGS, 1, MaxModWriteWait, MaxModWriteRetries);		
*/
			Send_WriteModReg(k_Bridge2_Addr, REG_BRIDGE2_STATUS_FLAGS, 1, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);
            
			DelaymSec(1000);		
//-------------------------------------------------------------------------------------------------------
			//iniziare con INIT qui
			
			
#if (K_EnableHiSpeedMyBus==K_Baud115200)
            Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
            Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
            Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	              
            
			Room[1].OffLine = FuncReadReg (1, REG_SPLIT_RD_MODE, &temp[1], Room[1].OffLine);	

#if(K_HiSpeedU1ModBUS==1)            
            Change_Protocol_BUSM(K_ModBus_Baud115200);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#else
            Change_Protocol_BUSM(K_ModBus_Baud57600);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#endif

			InsertReg(1, temp[1]);										// 6 REG_BRIDGE2_DIAG_SPLIT_WRITE_MODE
			Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, REG_BRIDGE2_DIAG_SPLIT_WRITE_MODE, 1, MaxModWriteWait, MaxModWriteRetries); 

#if (K_EnableHiSpeedMyBus==K_Baud115200)
            Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
            Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
            Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	              

			Room[1].OffLine = FuncReadReg (1, REG_SPLIT_SYNC_RD_PWRECOM, &temp[1], Room[1].OffLine);	
			Room[1].OffLine = FuncReadReg (1, REG_SPLIT_SYNC_RD_ECOMEN, &temp[2], Room[1].OffLine);	

#if(K_HiSpeedU1ModBUS==1)            
            Change_Protocol_BUSM(K_ModBus_Baud115200);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#else
            Change_Protocol_BUSM(K_ModBus_Baud57600);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#endif

			InsertReg(1, temp[1]);										// 14 REG_BRIDGE2_ECO_MODE_PWR_LIMIT
			InsertReg(2, temp[2]);										// 15 REG_BRIDGE2_ECO_MODE_ENABLE
			Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, REG_BRIDGE2_ECO_MODE_PWR_LIMIT, 2, MaxModWriteWait, MaxModWriteRetries); 
	
#if (K_EnableHiSpeedMyBus==K_Baud115200)
            Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
            Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
            Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	              

			Room[1].OffLine = FuncReadReg (1, REG_SPLIT_RD_UTA_POWER, &temp[1], Room[1].OffLine);	
			Room[1].OffLine = FuncReadReg (1, REG_SPLIT_RD_UTA_MODE, &temp[2], Room[1].OffLine);	
			Room[1].OffLine = FuncReadReg (1, REG_SPLIT_RD_UTA_SP_ROOM, &temp[3], Room[1].OffLine);	
			Room[1].OffLine = FuncReadReg (1, REG_SPLIT_RD_UTA_SP_HUMI, &temp[4], Room[1].OffLine);	
			Room[1].OffLine = FuncReadReg (1, REG_SPLIT_RD_UTA_FAN_SPEED, &temp[5], Room[1].OffLine);
			Room[1].OffLine = FuncReadReg (1, REG_SPLIT_RD_SYSTEM_DISABLE, &temp[6], Room[1].OffLine);
			
#if(K_HiSpeedU1ModBUS==1)            
            Change_Protocol_BUSM(K_ModBus_Baud115200);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#else
            Change_Protocol_BUSM(K_ModBus_Baud57600);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#endif
            
			InsertReg(1, temp[1]);										// 25 REG_BRIDGE2_UTA_WRITE_PWR
			InsertReg(2, temp[2]);										// 26 REG_BRIDGE2_UTA_WRITE_MODE
			InsertReg(3, temp[3]);										// 27 REG_BRIDGE2_UTA_WRITE_SP_ROOM
			InsertReg(4, temp[4]);										// 28 REG_BRIDGE2_UTA_WRITE_SP_HUMI
			InsertReg(5, temp[5]);										// 29 REG_BRIDGE2_UTA_WRITE_FAN_SPEED
			InsertReg(6, temp[6]);										// 30 REG_BRIDGE2_SYSTEM_DISABLE
			Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, REG_BRIDGE2_UTA_WRITE_PWR, 6, MaxModWriteWait, MaxModWriteRetries); 			

			InitQueryArg(1, REG_BRIDGE2_SPLIT_01_READ_PWR);
			InitQueryArg(2, REG_BRIDGE2_SPLIT_02_READ_PWR);
			InitQueryArg(3, REG_BRIDGE2_SPLIT_03_READ_PWR);
			InitQueryArg(4, REG_BRIDGE2_SPLIT_04_READ_PWR);
			InitQueryArg(5, REG_BRIDGE2_SPLIT_05_READ_PWR);
			InitQueryArg(6, REG_BRIDGE2_SPLIT_06_READ_PWR);
			InitQueryArg(7, REG_BRIDGE2_SPLIT_07_READ_PWR);
			InitQueryArg(8, REG_BRIDGE2_SPLIT_08_READ_PWR);
			InitQueryArg(9, REG_BRIDGE2_SPLIT_09_READ_PWR);
			InitQueryArg(10, REG_BRIDGE2_SPLIT_10_READ_PWR);
            InitQueryArg(11, REG_BRIDGE2_SPLIT_11_READ_PWR);
            InitQueryArg(12, REG_BRIDGE2_SPLIT_12_READ_PWR);
			InitQueryArg(13, REG_BRIDGE2_SPLIT_13_READ_PWR);
            InitQueryArg(14, REG_BRIDGE2_SPLIT_14_READ_PWR);
            InitQueryArg(15, REG_BRIDGE2_SPLIT_15_READ_PWR);
			InitQueryArg(16, REG_BRIDGE2_SPLIT_16_READ_PWR);
            InitQueryArg(17, REG_BRIDGE2_SPLIT_17_READ_PWR);
            InitQueryArg(18, REG_BRIDGE2_SPLIT_18_READ_PWR);
            InitQueryArg(19, REG_BRIDGE2_SPLIT_19_READ_PWR);
            InitQueryArg(20, REG_BRIDGE2_SPLIT_20_READ_PWR);
            InitQueryArg(21, REG_BRIDGE2_SPLIT_21_READ_PWR);
            InitQueryArg(22, REG_BRIDGE2_SPLIT_22_READ_PWR);
            InitQueryArg(23, REG_BRIDGE2_SPLIT_23_READ_PWR);
            InitQueryArg(24, REG_BRIDGE2_SPLIT_24_READ_PWR);
            InitQueryArg(25, REG_BRIDGE2_SPLIT_25_READ_PWR);
            InitQueryArg(26, REG_BRIDGE2_SPLIT_26_READ_PWR);
            InitQueryArg(27, REG_BRIDGE2_SPLIT_27_READ_PWR);
            InitQueryArg(28, REG_BRIDGE2_SPLIT_28_READ_PWR);
            InitQueryArg(29, REG_BRIDGE2_SPLIT_29_READ_PWR);
            InitQueryArg(30, REG_BRIDGE2_SPLIT_30_READ_PWR);
            
			//Finire con INIT qui
//-------------------------------------------------------------------------------------------------------	
			// Srittura flag bit1 in REG0 per avvisare bridge che ho finito di inviare i dati di init
			// questo ripristina le scritture lato DL
			DelaymSec(5);															// Pausa 5mS	
/*
			InsertReg(1	, 3);			// 0				// Bridge Status Flags           
			Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, REG_BRIDGE2_STATUS_FLAGS, 1, MaxModReadWait, MaxModReadRetries);		
*/
			Send_WriteModReg(k_Bridge2_Addr, REG_BRIDGE2_STATUS_FLAGS, 3, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);

            
            
            bridgeFlag->InitializeBridge = 1;
			break;		
			
	}
}

/*
int Send_InitDataToBridgeAndCheck(int Addr, int Reg, int WriteValue)
{
	int Offline=0;
	int ReadValue;
	int Ready=0;
	int RetryLoop=0;
	#define	K_MAX_RETRY_BRIDGE_INIT		200
	
	while(!Ready)
	{
		Offline = Send_WriteModReg(k_Bridge2_Addr, REG_BRIDGE2_STATUS_FLAGS,  WriteValue, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);  
		DelaymSec(5);									// Pausa 5mS			
		Offline = Send_WaitModRx(k_Bridge2_Addr, ReadModCmd, REG_BRIDGE2_STATUS_FLAGS, 1, MaxModReadWait, MaxModReadRetries);
		DelaymSec(5);									// Pausa 5mS			
		if(!Offline)
			ReadValue = Mod_LastRx.Data;	
		else
			return 0;						// Non mi ha risposto per "MaxModWriteRetries" volte quindi esco 			
		if(ReadValue == WriteValue)
			Ready=1;
		if(RetryLoop++ > K_MAX_RETRY_BRIDGE_INIT)
			return 0;						// Non sono riuscito ad inizializzare entro "K_MAX_RETRY_BRIDGE_INIT" volte esco 
	}
	
	return 1;
}
*/


void Send_DataToBridge(TypBridge * bridgeFlag, int bridgeNumber)
{
	int temp[9]={0,0,0,0,0,0,0,0,0};

    static int Sched=0;
#if(K_HiSpeedU1ModBUS==1)            
    Change_Protocol_BUSM(K_ModBus_Baud115200);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#else
    Change_Protocol_BUSM(K_ModBus_Baud57600);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#endif

	switch(bridgeNumber)
	{
        /*
		case 1:			// Invio dei dati verso Bridge 1 (Master UTA-H)


			Change_Protocol_BUSM( K_MyBus_Baud57600);
			Room[1].OffLine = FuncReadReg (1, REG_SPLIT_RD_UTA_POWER, &temp[1], Room[1].OffLine);	
			Room[1].OffLine = FuncReadReg (1, REG_SPLIT_RD_UTA_MODE, &temp[2], Room[1].OffLine);	
			Room[1].OffLine = FuncReadReg (1, REG_SPLIT_RD_UTA_SP_ROOM, &temp[3], Room[1].OffLine);	
			Room[1].OffLine = FuncReadReg (1, REG_SPLIT_RD_UTA_SP_HUMI, &temp[4], Room[1].OffLine);	
			Room[1].OffLine = FuncReadReg (1, REG_SPLIT_RD_UTA_FAN_SPEED, &temp[5], Room[1].OffLine);	

#if(K_HiSpeedU1ModBUS==1)            
            Change_Protocol_BUSM(K_ModBus_Baud115200);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#else
            Change_Protocol_BUSM(K_ModBus_Baud57600);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#endif

  			InsertReg(1, temp[1]);										// 5 REG_BRIDGE1_UTA_WRITE_PWR
			InsertReg(2, temp[2]);										// 6 REG_BRIDGE1_UTA_WRITE_MODE
			InsertReg(3, temp[3]);										// 7 REG_BRIDGE1_UTA_WRITE_SP_ROOM
			InsertReg(4, temp[4]);										// 8 REG_BRIDGE1_UTA_WRITE_SP_HUMI
			InsertReg(5, temp[5]);										// 9 REG_BRIDGE1_UTA_WRITE_FAN_SPEED
			InsertReg(6, temp[6]);										// 10 REG_BRIDGE1_UTA_H_CLEAR_ERROR
			InsertReg(7, temp[7]);										// 11 REG_BRIDGE1_UTA_H_CLEAR_SPLITERROR1
			InsertReg(8, temp[8]);										// 12 REG_BRIDGE1_UTA_H_ABIL
			Send_WaitModRx(k_Bridge1_Addr, WriteModListCmd, REG_BRIDGE1_UTA_WRITE_PWR, 8, MaxModReadWait, MaxModReadRetries); 																			// 1 = Vel.MIN

			InsertReg(1, UTA_H.ClearErrorBox);		// 10 REG_BRIDGE1_UTA_H_CLEAR_ERROR		
			Send_WaitModRx(k_Bridge1_Addr, WriteModListCmd, REG_BRIDGE1_UTA_H_CLEAR_ERROR, 1, MaxModReadWait, MaxModReadRetries); 																			// 1 = Vel.MIN

			//InsertReg(REG_BRIDGE1_STATUS_FLAGS, 0);
			//InsertReg(REG_BRIDGE1_WORD_1, 0);		//??
			//InsertReg(REG_BRIDGE1_WORD_1, Room[1].PowerLimit); //EngineBox[0].BridgePowerLimit);		//??
			//InsertReg(REG_BRIDGE1_WORD_2, 0);		//??
			//InsertReg(REG_BRIDGE1_WORD_3, 0);		//??

		break;
		*/
		case 2:			// Invio dei dati verso Bridge 2	
			
			//bridgeFlag->OffLine = Send_WaitModRx(k_Bridge2_Addr, ReadModCmd, REG_BRIDGE2_SPLIT_ENGINE_BOX_COMPRESSOR_SPEED, 1, MaxModReadWait, MaxModReadRetries);


            
            if((bridgeFlag->OffLine==0) && (Sched==0))
            {				       
                InsertReg(1, ValueToPercentDecimal(EngineBox[0].Out_Inverter, 255));								// 140 REG_BRIDGE2_SPLIT_ENGINE_BOX_COMPRESSOR_SPEED				// "Engine Box Inverter Compressor Speed: (percentage)                                                    		
#if(K_AbilPumpInverter==1 || K_AbilPumpDAC==1)
                InsertReg(2, ValueToPercentDecimal(EngineBox[0].FreqPompa_Acqua, K_LIM_MAX_FREQ_INV_PUMP));		// 141 REG_BRIDGE2_SPLIT_ENGINE_BOX_WATER_PUMP_SPEED				// "Engine Box Inverter Water Pump Speed: (percentage)                                                    		
#else
                InsertReg(2, 0);		// 141 REG_BRIDGE2_SPLIT_ENGINE_BOX_WATER_PUMP_SPEED				// "Engine Box Inverter Water Pump Speed: (percentage)                                                    		            
#endif			
                InsertReg(3, EngineBox[0].Ric_Pressione);													// 142 REG_BRIDGE2_SPLIT_CURRENT_REQ_PRESSURE				// Engine Box Max Air Handler Required Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar             		
                bridgeFlag->OffLine = Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, REG_BRIDGE2_SPLIT_ENGINE_BOX_COMPRESSOR_SPEED, 3, MaxModReadWait, MaxModReadRetries);         
            }

            if((bridgeFlag->OffLine==0) && (Sched==1))
            {				                       
                InsertReg(1, EngineBox[0].Temperature.Sea_Water);							// 145 REG_BRIDGE2_SPLIT_ENGINE_BOX_T_SEA_WATER				// Engine Box Sea Water Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 �C                 		
                bridgeFlag->OffLine = Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, REG_BRIDGE2_SPLIT_ENGINE_BOX_T_SEA_WATER, 1, MaxModReadWait, MaxModReadRetries);         
            }

            if((bridgeFlag->OffLine==0) && (Sched==2))
            {				       
                InsertReg(1, EngineBox[0].Pressure.Gas);		// 149 REG_BRIDGE2_SPLIT_ENGINE_BOX_P_GAS				// Engine Box Gas Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar                                  		
                InsertReg(2, EngineBox[0].Pressure.Liquid);		// 150 REG_BRIDGE2_SPLIT_ENGINE_BOX_P_LIQUID				// Engine Box Liquid Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar                               		
                InsertReg(3, EngineBox[0].Pressure.LiquidCond);	// 151 REG_BRIDGE2_SPLIT_ENGINE_BOX_P_CONDENSER			// Engine Box Condenser Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar                               		
                bridgeFlag->OffLine = Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, REG_BRIDGE2_SPLIT_ENGINE_BOX_P_GAS, 3, MaxModReadWait, MaxModReadRetries);         
            }

            if((bridgeFlag->OffLine==0) && (Sched==3))
            {				       
                InsertReg(1, ValueToPercentDecimal(EngineBox[1].Out_Inverter, 255));		// 155 REG_BRIDGE2_SPLIT_ENGINE_BOX_COMPRESSOR2_SPEED				// "Engine Box Inverter Compressor Speed: (percentage)                                                    		
                bridgeFlag->OffLine = Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, REG_BRIDGE2_SPLIT_ENGINE_BOX_COMPRESSOR2_SPEED, 1, MaxModReadWait, MaxModReadRetries);         
            }

            if((bridgeFlag->OffLine==0) && (Sched==4))
			{				       
				InsertReg(1, Comp_Inverter[0].Out_Power);		// 185 REG_BRIDGE2_ABSORPTION_WATT_AC_COMPRESSOR_1				// Instantaneous absorption control of the AC compressor 1 (Ampere)                                       		
				InsertReg(2, Comp_Inverter[1].Out_Power);		// 186 REG_BRIDGE2_ABSORPTION_WATT_AC_COMPRESSOR_2				// Instantaneous absorption control of the AC compressor 2 (Ampere)                                       		
				InsertReg(3, Comp_Inverter[2].Out_Power);		// 187 REG_BRIDGE2_ABSORPTION_WATT_SEA_WATER_PUMP				// Instantaneous absorption control of the AC Sea Water Pump (Ampere)                                     		
				//InsertReg(4, EngineBox[0].TotalPowerBoxAbsorption);// 188 REG_BRIDGE2_ABSORPTION_WATT_AC_TOTAL				    // Instantaneous absorption control of the AC Total (WATT)           
/*
#if(K_Supply_Inverter==400)
                InsertReg(5, (unsigned int)( (float)(EngineBox[0].TotalPowerBoxAbsorption) / (float)(K_Supply_Inverter) / 1.73 / 0.9));// 189 REG_BRIDGE2_ABSORPTION_AC_TOTAL   // Instantaneous absorption control of the AC Total (Ampere)
#else
                InsertReg(5, (unsigned int)( (float)EngineBox[0].TotalPowerBoxAbsorption*10.0 / (float)K_Supply_Inverter * 100.0) );   // 189 REG_BRIDGE2_ABSORPTION_AC_TOTAL   // Instantaneous absorption control of the AC Total (Ampere)                
#endif
*/
				bridgeFlag->OffLine = Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, REG_BRIDGE2_ABSORPTION_WATT_AC_COMPRESSOR_1, 3, MaxModReadWait, MaxModReadRetries);         
			}

            if((bridgeFlag->OffLine==0) && (Sched==5))
            {				       
                InsertReg(1, (unsigned int)( (float)Comp_Inverter[0].Out_Power*10.0 / (float)K_VoltOutMaxInverter * 100.0) );		// 185 REG_BRIDGE2_ABSORPTION_AC_COMPRESSOR_1				// Instantaneous absorption control of the AC compressor 1 (Ampere)                                       		
                InsertReg(2, (unsigned int)( (float)Comp_Inverter[1].Out_Power*10.0 / (float)K_VoltOutMaxInverter * 100.0) );		// 186 REG_BRIDGE2_ABSORPTION_AC_COMPRESSOR_2				// Instantaneous absorption control of the AC compressor 2 (Ampere)                                       		
                InsertReg(3, (unsigned int)( (float)Comp_Inverter[2].Out_Power*10.0 / (float)K_VoltOutMaxInverter * 100.0) );		// 187 REG_BRIDGE2_ABSORPTION_SEA_WATER_PUMP				// Instantaneous absorption control of the AC Sea Water Pump (Ampere)                                     		
 				InsertReg(4, Comp_Inverter[0].Out_Power+Comp_Inverter[1].Out_Power+Comp_Inverter[2].Out_Power);                 	// 188 REG_BRIDGE2_ABSORPTION_WATT_AC_TOTAL				    // Instantaneous absorption control of the AC Total (WATT)
                InsertReg(5, (unsigned int)( (((float)Comp_Inverter[0].Out_Power*10.0)+((float)Comp_Inverter[1].Out_Power*10.0)+((float)Comp_Inverter[2].Out_Power*10.0)) / (float)K_VoltOutMaxInverter * 100.0) );		// 189 REG_BRIDGE2_ABSORPTION_AC_TOTAL   // Instantaneous absorption control of the AC Total (Ampere)

				//InsertReg(4, (unsigned int)( (float)Comp_Inverter[3].Out_Power*10.0 / (float)K_VoltOutMaxInverter * 100.0) );		// 188 REG_BRIDGE2_ABSORPTION_UTA				// Instantaneous absorption control of the UTA (Ampere)         			
                ////InsertReg(4, (unsigned int)( (float)UTA_H.Absorption*10.0 / (float)K_VoltOutMaxInverter * 100.0) );		// 188 REG_BRIDGE2_ABSORPTION_UTA				// Instantaneous absorption control of the UTA (Ampere)         			
                bridgeFlag->OffLine = Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, REG_BRIDGE2_ABSORPTION_AC_COMPRESSOR_1, 5, MaxModReadWait, MaxModReadRetries);         
            }

            if((bridgeFlag->OffLine==0) && (Sched==6))
            {				       
                InsertReg(1, EngineBox[0].Error.Active_Error);	// 190 REG_BRIDGE2_ACTIVE_ERROR				// "General Active Error:  
                bridgeFlag->OffLine = Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, REG_BRIDGE2_ACTIVE_ERROR, 1, MaxModReadWait, MaxModReadRetries);
            }
			
			SendQueryArg(1, REG_BRIDGE2_SPLIT_01_READ_PWR);
			SendQueryArg(2, REG_BRIDGE2_SPLIT_02_READ_PWR);
			SendQueryArg(3, REG_BRIDGE2_SPLIT_03_READ_PWR);
			SendQueryArg(4, REG_BRIDGE2_SPLIT_04_READ_PWR);
			SendQueryArg(5, REG_BRIDGE2_SPLIT_05_READ_PWR);
			SendQueryArg(6, REG_BRIDGE2_SPLIT_06_READ_PWR);
			SendQueryArg(7, REG_BRIDGE2_SPLIT_07_READ_PWR);
			SendQueryArg(8, REG_BRIDGE2_SPLIT_08_READ_PWR);
			SendQueryArg(9, REG_BRIDGE2_SPLIT_09_READ_PWR);
			SendQueryArg(10, REG_BRIDGE2_SPLIT_10_READ_PWR);
            SendQueryArg(11, REG_BRIDGE2_SPLIT_11_READ_PWR);
            SendQueryArg(12, REG_BRIDGE2_SPLIT_12_READ_PWR);
			SendQueryArg(13, REG_BRIDGE2_SPLIT_13_READ_PWR);
            SendQueryArg(14, REG_BRIDGE2_SPLIT_14_READ_PWR);
            SendQueryArg(15, REG_BRIDGE2_SPLIT_15_READ_PWR);
			SendQueryArg(16, REG_BRIDGE2_SPLIT_16_READ_PWR);
            SendQueryArg(17, REG_BRIDGE2_SPLIT_17_READ_PWR);
            SendQueryArg(18, REG_BRIDGE2_SPLIT_18_READ_PWR);
            SendQueryArg(19, REG_BRIDGE2_SPLIT_19_READ_PWR);
            SendQueryArg(20, REG_BRIDGE2_SPLIT_20_READ_PWR);
            SendQueryArg(21, REG_BRIDGE2_SPLIT_21_READ_PWR);
            SendQueryArg(22, REG_BRIDGE2_SPLIT_22_READ_PWR);
            SendQueryArg(23, REG_BRIDGE2_SPLIT_23_READ_PWR);
            SendQueryArg(24, REG_BRIDGE2_SPLIT_24_READ_PWR);
            SendQueryArg(25, REG_BRIDGE2_SPLIT_25_READ_PWR);
            SendQueryArg(26, REG_BRIDGE2_SPLIT_26_READ_PWR);
            SendQueryArg(27, REG_BRIDGE2_SPLIT_27_READ_PWR);
            SendQueryArg(28, REG_BRIDGE2_SPLIT_28_READ_PWR);
            SendQueryArg(29, REG_BRIDGE2_SPLIT_29_READ_PWR);
            SendQueryArg(30, REG_BRIDGE2_SPLIT_30_READ_PWR);
			
            if(Sched++ > 6)
               Sched = 0;
            
			break;		
		
		case 3:
		break;		
	}
}


void SendQueryArg(int RoomStart, int Address)
{
	int temp=0;
	if(Room[RoomStart].Enable==1)
    {
		temp = (!Room[RoomStart].Enable << 0) |	//Se Non abilitata
				(Room[RoomStart].Enable << 1) |	//Se Abilitata
				((Room[RoomStart].OffLine && Room[RoomStart].Enable) << 2) |// Se Abilitata e Offline
				((Room[RoomStart].SplitStatus != 0) << 3);//Se in errore		

		InsertReg(1, Room[RoomStart].PowerOn);					// 201 REG_BRIDGE2_SPLIT_01_READ_PWR			
		InsertReg(2, Room[RoomStart].SetPoint);					// 202 REG_BRIDGE2_SPLIT_01_READ_SP_ROOM			
		InsertReg(3, Room[RoomStart].FanSpeed);					// 203 REG_BRIDGE2_SPLIT_01_READ_FAN_SPEED			
		InsertReg(4, Room[RoomStart].AirTemp);					// 204 REG_BRIDGE2_SPLIT_01_READ_AIR_IN			
		InsertReg(5, temp);            							// 205 REG_BRIDGE2_SPLIT_01_READ_STATUS	
        if((Room[RoomStart].Model_FW_Version == 1) || (Room[RoomStart].Model_FW_Version == 2))  //Se sono UTA
        {
            InsertReg(6, Room[RoomStart].UTAMode);            		// 206 REG_BRIDGE2_SPLIT_01_UTA_MODE		        

            Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, Address, 6, MaxModReadWait, MaxModReadRetries);			
        }
        else
        {
            Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, Address, 5, MaxModReadWait, MaxModReadRetries);			
        }
        if(Room[RoomStart].Heater_Abil==1)
        {
            InsertReg(1, Room[RoomStart].HeaterPwr);					// 206 REG_BRIDGE2_SPLIT_01_READ_HEATER_PWR
            Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, (Address+6), 1, MaxModReadWait, MaxModReadRetries);            
        }
        if(Room[RoomStart].HumidityMiddle>0)
        {
            InsertReg(1, Room[RoomStart].HumidityMiddle);					// 207 REG_BRIDGE2_SPLIT_01_READ_HUMIDITY_MIDDLE
            Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, (Address+7), 1, MaxModReadWait, MaxModReadRetries);            
        }
    }
    /*
    else
    {
		temp = (!Room[RoomStart].Enable << 0) |	//Se Non abilitata
				(Room[RoomStart].Enable << 1) |	//Se Abilitata
				((Room[RoomStart].OffLine && Room[RoomStart].Enable) << 2) |// Se Abilitata e Offline
				((Room[RoomStart].SplitStatus != 0) << 3);//Se in errore		
		InsertReg(1, temp);            							// 205 REG_BRIDGE2_SPLIT_01_READ_STATUS		
        Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, Address+4, 1, MaxModReadWait, MaxModReadRetries);			        
    }
    */
}		

void InitQueryArg(int RoomStart, int Address)
{
	int temp=0;
    temp = (!Room[RoomStart].Enable << 0) |	//Se Non abilitata
            (Room[RoomStart].Enable << 1) |	//Se Abilitata
            ((Room[RoomStart].OffLine && Room[RoomStart].Enable) << 2) |// Se Abilitata e Offline
            ((Room[RoomStart].SplitStatus != 0) << 3);//Se in errore		
    InsertReg(1, temp);            							// 205 REG_BRIDGE2_SPLIT_01_READ_STATUS		
    Send_WaitModRx(k_Bridge2_Addr, WriteModListCmd, Address+4, 1, MaxModReadWait, MaxModReadRetries);			        
}		


void Get_DataFromBridge(TypBridge * bridgeFlag, int bridgeNumber)
{
    
#if(K_HiSpeedU1ModBUS==1)            
    Change_Protocol_BUSM(K_ModBus_Baud115200);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#else
    Change_Protocol_BUSM(K_ModBus_Baud57600);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#endif
	
	switch(bridgeNumber)
	{
        /*
		case 1:			// Lettura dei dati da Bridge 1 (Master UTA-H)
			
			bridgeFlag->OffLine = Send_WaitModRx(k_Bridge1_Addr, ReadModCmd, REG_BRIDGE1_UTA_H_ENGINE_COMPRESSOR_SPEED, 18, MaxModReadWait, MaxModReadRetries);
			if(bridgeFlag->OffLine==0)
			{
				UTA_H.EngineCompSpeed = ExtracReg(1);				// REG_BRIDGE1_UTA_H_ENGINE_COMPRESSOR_SPEED			100 //160				// "UTA H Engine Box Inverter Compressor Speed: (percentage)		Range: 0..100%"                                  
				UTA_H.EngineFanSpeed = ExtracReg(2);				// REG_BRIDGE1_UTA_H_ENGINE_WATER_PUMP_SPEED			101 //161				// "UTA H Engine Box Inverter Water Pump Speed: (percentage)		Range: 0..100%"                                  
				UTA_H.EngineReqPressure = ExtracReg(3);				// REG_BRIDGE1_UTA_H_ENGINE_REQ_PRESSURE				102 //162				// UTA H Engine Box Max Air Handler Required Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar                  
				UTA_H.EngineTempOut = ExtracReg(4);					// REG_BRIDGE1_UTA_H_ENGINE_T_OUTPUT					103 //163				// UTA H Engine Box Output Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 �C                         
				UTA_H.EngineTempIn = ExtracReg(5);					// REG_BRIDGE1_UTA_H_ENGINE_T_SUCTION					104 //164				// UTA H Engine Box Suction Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 �C                        
				UTA_H.EngineTempAir = ExtracReg(6);					// REG_BRIDGE1_UTA_H_ENGINE_T_AIR						105 //165				// UTA H Engine Box Air Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 �C                            
				UTA_H.EngineTempBatt = ExtracReg(7);				// REG_BRIDGE1_UTA_H_ENGINE_T_BATTERY					106 //166				// UTA H Engine Box Battery Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 �C                        
				UTA_H.EngineTempGas = ExtracReg(8);					// REG_BRIDGE1_UTA_H_ENGINE_T_GAS						107 //167				// UTA H Engine Box Gas Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 �C                            
				UTA_H.EngineTempLiq = ExtracReg(9);					// REG_BRIDGE1_UTA_H_ENGINE_T_LIQUID					108 //168				// UTA H Engine Box Liquid Temperature:  (hundredths of degrees Celsius) es: 2350 = 23.50 �C                         
				UTA_H.EnginePressGas = ExtracReg(10);				// REG_BRIDGE1_UTA_H_ENGINE_P_GAS						109 //169				// UTA H Engine Box Gas Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar                                       
				UTA_H.EnginePressLiq = ExtracReg(11);				// REG_BRIDGE1_UTA_H_ENGINE_P_LIQUID					110 //170				// UTA H Engine Box Liquid Pressure:  (thousandths of Bar) es: 22543 = 22.543 Bar                                    
				UTA_H.EngineBoxErr1 = ExtracReg(12);				// REG_BRIDGE1_UTA_H_ENGINE_BOX_ERROR1					111 //171				// "UTA H Engine Box Error/Status Flags 1:                                                                               
				UTA_H.EngineBoxPersErr1 = ExtracReg(13);			// REG_BRIDGE1_UTA_H_ENGINE_BOX_PERSERROR1				112 //172	
				UTA_H.EngineBoxSplitErr1 = ExtracReg(14);			// REG_BRIDGE1_UTA_H_ENGINE_BOX_SPLITERROR1				113                                                                 
				UTA_H.Absorption = ExtracReg(15);					// REG_BRIDGE1_UTA_H_ABSORPTION							114 //188				// Instantaneous absorption control of the UTA (Ampere)                                                              
				UTA_H.CumulativeAlarm = ExtracReg(16);				// REG_BRIDGE1_UTA_H_CUMULATIVE_ALARM					115						// Cumulativo Allarmi x UTA                                                                                          
				UTA_H.Split.LiquidTemp = ExtracReg(17);				// REG_UTA_H_READ_LIQ_TEMP								116		
				UTA_H.Split.GasTemp = ExtracReg(18);				// REG_UTA_H_READ_GAS_TEMP								117	
			}

			bridgeFlag->OffLine = Send_WaitModRx(k_Bridge1_Addr, ReadModCmd, REG_UTA_H_READ_AIR_IN, 19, MaxModReadWait, MaxModReadRetries);
			if(bridgeFlag->OffLine==0)
			{
				UTA_H.Split.AirInTemp = ExtracReg(1);				// REG_UTA_H_READ_AIR_IN								118		
				UTA_H.Split.AitOutTemp = ExtracReg(2);				// REG_UTA_H_READ_AIR_OUT								119		
				UTA_H.Split.Exp_Valve = ExtracReg(3);				// REG_UTA_H_READ_EXP_VALVE								120		
				UTA_H.Split.Humi = ExtracReg(4);					// REG_UTA_H_READ_HUMI									121		
				UTA_H.Split.ReqAirTemp = ExtracReg(5);				// REG_UTA_H_READ_REQUEST_AIR_TEMP						122		
				UTA_H.Split.ReqGasPress = ExtracReg(6);             // REG_UTA_H_READ_REQUEST_GAS_PRES						123		
				UTA_H.Split.Superheat = ExtracReg(7);				// REG_UTA_H_READ_SUPERHEAT								124		
				UTA_H.Split.HW_Ver = ExtracReg(8);					// REG_UTA_H_READ_SPLIT_HW_VERS							125		
				UTA_H.Split.FW_Ver_HW = ExtracReg(9);				// REG_UTA_H_READ_SPLIT_FW_VERS_HW						126		
				UTA_H.Split.FW_Ver_FW = ExtracReg(10);				// REG_UTA_H_READ_SPLIT_FW_VERS_FW						127		
				UTA_H.Split.FW_Rev_FW = ExtracReg(11);				// REG_UTA_H_READ_SPLIT_FW_REV_FW						128		
				UTA_H.HW_Ver = ExtracReg(12);						// REG_UTA_H_READ_BOX_HW_VERS							129		
				UTA_H.FW_Ver_HW = ExtracReg(13);					// REG_UTA_H_READ_BOX_FW_VERS_HW						130		
				UTA_H.FW_Ver_FW = ExtracReg(14);					// REG_UTA_H_READ_BOX_FW_VERS_FW						131		
				UTA_H.FW_Rev_FW = ExtracReg(15);					// REG_UTA_H_READ_BOX_FW_REV_FW							132		
				UTA_H.UTASP_On = ExtracReg(16);                     // REG_UTA_H_ON_UTA_SP                                  133		
				UTA_H.StatoCompressore = ExtracReg(17);             // REG_UTA_H_ENGINE_BOX_COMP_STATUS				134                            133		
				UTA_H.IdMasterSplit = ExtracReg(18);                // REG_UTA_H_ENGINE_BOX_ID_MAS_SPLIT               135                   133		
				UTA_H.Mode = ExtracReg(19);							// REG_UTA_H_ENGINE_BOX_ENGINE_MODE                         136         133		               
			}
			
		break;
		*/
		case 2:			// Lettura dei dati da Bridge 2 (Logica DL)	->

			bridgeFlag->OffLine = Send_WaitModRx(k_Bridge2_Addr, ReadModCmd, REG_BRIDGE2_DIAG_ALL_SPLIT_WRITE_PWR, 6, MaxModReadWait, MaxModReadRetries);
			if(bridgeFlag->OffLine==0)
			{				
				bridgeFlag->Reinit.Power = 	ExtracReg(1);		//REG_BRIDGE2_DIAG_ALL_SPLIT_WRITE_PWR			// 1				// ALL Unit Power On/Off  0=OFF 1=ON                                                                 	                                                                       	
				bridgeFlag->Reinit.SetPoint =  ExtracReg(3);	//REG_BRIDGE2_DIAG_ALL_SPLIT_WRITE_SP_ROOM		// 3				// "ALL Unit SetPoint: (hundredths of degrees Celsius) ex: 2350 = 23.50 �C                           	
				bridgeFlag->Reinit.FanSpeed = 	ExtracReg(4);	//REG_BRIDGE2_DIAG_ALL_SPLIT_WRITE_FAN_SPEED	// 4				// "ALL Unit Fan Speed:                                                                              	
				bridgeFlag->Reinit.TrigInit = 	ExtracReg(5);	//REG_BRIDGE2_DIAG_ALL_SPLIT_WRITE_UPDATE		// 5				// "ALL Unit Update flag:     
				bridgeFlag->SetRoom.DefMode = 	ExtracReg(6);	//REG_BRIDGE2_DIAG_SPLIT_WRITE_MODE				// 6				// "ALL Unit Update flag:     
			}
			
            bridgeFlag->OffLine = Send_WaitModRx(k_Bridge2_Addr, ReadModCmd, REG_BRIDGE2_UTA_WRITE_MODE, 1, MaxModReadWait, MaxModReadRetries);
			if(bridgeFlag->OffLine==0)
			{						
				bridgeFlag->SetUta.DefMode = ExtracReg(1);			// REG_BRIDGE2_UTA_WRITE_MODE 26				// "UTA Unit Function Mode:    
            }
            
            bridgeFlag->OffLine = Send_WaitModRx(k_Bridge2_Addr, ReadModCmd, REG_BRIDGE2_HEATER_PWR, 1, MaxModReadWait, MaxModReadRetries);
			if(bridgeFlag->OffLine==0)
			{						
				bridgeFlag->SetRoom.HeaterPwr = ExtracReg(1);			// REG_BRIDGE2_UTA_WRITE_MODE 26				// "UTA Unit Function Mode:    
            }			
			
			bridgeFlag->OffLine = Send_WaitModRx(k_Bridge2_Addr, ReadModCmd, REG_BRIDGE2_DIAG_SPLIT_WRITE_SELECTION, 6, MaxModReadWait, MaxModReadRetries);
			if(bridgeFlag->OffLine==0)
			{				
				bridgeFlag->SetRoom.Address = 	ExtracReg(1);			// REG_BRIDGE2_DIAG_SPLIT_WRITE_SELECTION 10				// Unit Power On/Off  0=OFF 1=ON                                                                     	
				bridgeFlag->SetRoom.PowerOn = 	ExtracReg(2);			// REG_BRIDGE2_DIAG_SPLIT_WRITE_PWR 11				// "Unit Function Mode:                                                                              	
				bridgeFlag->SetRoom.SetPoint = 	ExtracReg(3);		// REG_BRIDGE2_DIAG_SPLIT_WRITE_SP_ROOM 12				// "Unit SetPoint: (hundredths of degrees Celsius) es: 2350 = 23.50 �C                               	
				bridgeFlag->SetRoom.FanSpeed = 	ExtracReg(4);		// REG_BRIDGE2_DIAG_SPLIT_WRITE_FAN_SPEED 13				// "Unit Fan Speed:                                                                                  	                                                                                   	
				bridgeFlag->SetRoom.Power_EcoMode = ExtracReg(5);				// REG_BRIDGE2_ECO_MODE_PWR_LIMIT 14				// Eco Mode Power Limit Value (Kw)   (hundredths of Kw)  es. 400 = 4.00Kw                            	
				bridgeFlag->SetRoom.EcoModeEnable = ExtracReg(6);				// REG_BRIDGE2_ECO_MODE_ENABLE 15				// Eco Mode Enable On/Off  (0=OFF 1=ON) ? Priority over Power Limit Value   
			}
			
            bridgeFlag->OffLine = Send_WaitModRx(k_Bridge2_Addr, ReadModCmd, REG_BRIDGE2_SYSTEM_DISABLE, 1, MaxModReadWait, MaxModReadRetries);
			if(bridgeFlag->OffLine==0)
			{						
				bridgeFlag->EngineBox.SystemDisable = ExtracReg(1);	// REG_BRIDGE2_SYSTEM_DISABLE 30
			}

/*			            			
			bridgeFlag->OffLine = Send_WaitModRx(k_Bridge2_Addr, ReadModCmd, REG_BRIDGE2_DIAG_SPLIT_TEST_ADDRESS, 1, MaxModReadWait, MaxModReadRetries);
			if(bridgeFlag->OffLine==0)
			{
				bridgeFlag->Diag.Split = ExtracReg(1);					// 106 REG_BRIDGE2_DIAG_SPLIT_TEST_ADDRESS				// Diagnostic Split Request Address                                                            					
			}							
*/
		break;
	}
	
}

/*
void UpdateNewDataBridge(int SlaveValue, int * BridgeValue)
{	
	if(SlaveValue != (*BridgeValue))
		(*BridgeValue) = SlaveValue;
}
*/
/*
void SyncDataFromBridge(void)
{
	UpdateNewDataBridge(Room[1].PowerLimit, &EngineBox[0].PowerLimit);
//	UpdateNewDataBridge(Room[1].PowerLimit, EngineBox[0].PowerLimit);
//	UpdateNewDataBridge(Room[1].PowerLimit, EngineBox[0].PowerLimit);
//	UpdateNewDataBridge(Room[1].PowerLimit, EngineBox[0].PowerLimit);
	
}
 */



void WaitEnd_DataFromBridge(TypBridge * bridgeFlag, int bridgeNumber)
{
	int RepCode;
	int Value=1;
    static int CntOnline=0;
	#define K_Wait_Cnt_Cycle	3
	
#if(K_HiSpeedU1ModBUS==1)            
    Change_Protocol_BUSM(K_ModBus_Baud115200);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#else
    Change_Protocol_BUSM(K_ModBus_Baud57600);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
#endif
     
	switch(bridgeNumber)
	{
        /*
		case 1:			// Lettura dei dati da Bridge 1 (Master UTA-H)		
			while(Value!=0 && PausaCompressore.TimeOut==0)						// Attendo che il bridge abbia finito di inizializzare il buffer SideB (bit0==0 e bit1==0)
			{
				RepCode = Send_WaitModRx(k_Bridge1_Addr, ReadModCmd, REG_BRIDGE1_STATUS_FLAGS, 1, MaxModReadWait, MaxModReadRetries);
				if(RepCode==0)
				{
					Value = ExtracReg(1);		// Leggo Word0 Buffer bit0 e bit1
				}
				DelaymSec(K_Wait_Check_Pause);					// Attesa pr non stressare troppo il Bridge che sta sincronizzando il SideB
			}
		break;
		*/
		case 2:			// Lettura dei dati da Bridge 2 (Logica DL)	-> 190 dati in 3 blocchi da 64 word
            /*
			while(Value!=0 && PausaCompressore.TimeOut==0)						// Attendo che il bridge abbia finito di inizializzare il buffer SideB (bit0==0 e bit1==0)
			{
				RepCode = Send_WaitModRx(k_Bridge2_Addr, ReadModCmd, REG_BRIDGE2_STATUS_FLAGS, 1, MaxModReadWait, MaxModReadRetries);
				if(RepCode==0)
				{
					Value = ExtracReg(1);		// Leggo Word0 Buffer bit0 e bit1
				}
				DelaymSec(K_Wait_Check_Pause);				
			}
            */
            
            RepCode = Send_WaitModRx(k_Bridge2_Addr, ReadModCmd, REG_BRIDGE2_STATUS_FLAGS, 1, MaxModReadWait, MaxModReadRetries);
            if(RepCode==0)
            {
                Value = ExtracReg(1);		// Leggo Word0 Buffer bit0 e bit1
                CntOnline += 1;
            }
            if(Value==0 || CntOnline>=K_Wait_Cnt_Cycle)
            {
                bridgeFlag->BridgeReady=1;
                //bridgeFlag->InitializeBridge=0;
                if(CntOnline>=K_Wait_Cnt_Cycle)
                    Send_WriteModReg(k_Bridge2_Addr, REG_BRIDGE2_STATUS_FLAGS, 0, MaxModWriteWait, MaxModWriteRetries, WriteModCmd);
                    
                CntOnline = 0;
            }
		break;
	}
}
