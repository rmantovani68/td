//----------------------------------------------------------------------------------
//	Progect name:	EEPROM.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			Emanuele
//	Date:			22/05/2011
//	Description:	Corpo delle funzioni per l'allocazione della E2 del micro
//----------------------------------------------------------------------------------
#include "HWSelection.h"
#if (DA210==1)
	#include <p24FJ256DA210.h>
#else
	#include <p24FJ256GB210.h>
#endif

#include <stdio.h>
#include "DefinePeriferiche.h"
#include "delay.h"
#include "ADC.h"
#include "I2C.h"
//#include "PWM.h"
//#include "Valvola_PassoPasso.h"
#include "EEPROM.h"
#include "Core.h"
#include "ProtocolloModBus.h"
#include "ProtocolloComunicazione.h"


extern TypRTC MyRTC;						// RTC di sistema
extern volatile TypTimer	TimerSwitchCompressor;		// Timer per time switch secondo compressore
extern volatile TypTimer    TimerSwitchPump;            // Timer per time switch seconda pompa

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
char HiByte(int Data)
{
	return (char)((Data & 0xFF00) >> 8);
}

char LoByte(int Data)
{
	return (char)(Data & 0x00FF);
}

int ToInt(char HiByte, char LoByte)
{
//	return (((int)HiByte) <<8 )| (LoByte&0x00FF);
	return ((((int)HiByte) <<8 ) & 0xFF00) | (LoByte&0x00FF);	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
}

void StoreInt(int Data, int Address)
{
	unsigned char ByteLow, ByteHigh;

	ByteLow = LoByte(Data);
	ByteHigh = HiByte(Data);
	
	SetRegI2C(EEPROM_Add, Address, ByteHigh);
	SetRegI2C(EEPROM_Add, Address +1, ByteLow);
}

int LoadInt(int Address)
{
	char ByteLow, ByteHigh;

	ByteHigh = ReadRegI2C(EEPROM_Add, Address);
	ByteLow = ReadRegI2C(EEPROM_Add, Address +1);
	
	return ToInt(ByteHigh, ByteLow);
}

void LoadAllE2_Data(void)
{
	char i=0;

	//@ oErrLed=LED_OFF;        ??????

	CRC_Flash = LoadInt(CRC_Hi_Flash);
	SN1 = ReadRegI2C(EEPROM_Add, System_SN_1);
	SN2 = ReadRegI2C(EEPROM_Add, System_SN_2);
	SN3 = ReadRegI2C(EEPROM_Add, System_SN_3);
	SN4 = ReadRegI2C(EEPROM_Add, System_SN_4);
	GiornoFabbricazione = ReadRegI2C(EEPROM_Add, Fab_Day);
	MeseFabbricazione = ReadRegI2C(EEPROM_Add, Fab_Month);
	AnnoFabbricazione = ReadRegI2C(EEPROM_Add, Fab_Year);    
    
	MyRTC.HoursOfLife = LoadInt(E2_Addr_Hours);		// Carico ore conteggiate da E2		
	if(MyRTC.HoursOfLife==0xFFFF)					// Se valore letto da E2 = FFFF allora EEPROM vergine e quindi
	{
		MyRTC.HoursOfLife=0;
		StoreInt(MyRTC.HoursOfLife, E2_Addr_Hours);	// Azzero ore conteggiate in E2
	}	
	
	Room[1].SystemDisable = LoadInt(E2_Last_SystemDisable);		// Carico ore conteggiate da E2		
	if(Room[1].SystemDisable==0xFFFF)					// Se valore letto da E2 = FFFF allora EEPROM vergine e quindi
	{
		Room[1].SystemDisable=0;
		StoreInt(Room[1].SystemDisable, E2_Last_SystemDisable);	// Azzero ore conteggiate in E2
	}	    
    
	EngineBox[0].LastCompressorPowerStatus = LoadInt(E2_LastCompressorPowerStatus);	// Carico ultimo stato compressore
	if(EngineBox[0].LastCompressorPowerStatus==0xFFFF)                              // Se valore letto da E2 = FFFF allora EEPROM vergine e quindi
	{
		EngineBox[0].LastCompressorPowerStatus=0;
		StoreInt(EngineBox[0].LastCompressorPowerStatus, E2_LastCompressorPowerStatus);	
	}  

	EngineBox[0].Compressor2Master = LoadInt(E2_LastCompressor2Master);				// Carico ultimo stato switch compressore
	if(EngineBox[0].Compressor2Master==0xFFFF)                              // Se valore letto da E2 = FFFF allora EEPROM vergine e quindi
	{
		EngineBox[0].Compressor2Master=0;
		StoreInt(EngineBox[0].Compressor2Master, E2_LastCompressor2Master);	
	}

	TimerSwitchCompressor.Value = LoadInt(E2_LastTimerSwitchCompressor);			// Carico ultimo valore timer x switch compressore
	if(TimerSwitchCompressor.Value==0xFFFF)					// Se valore letto da E2 = FFFF allora EEPROM vergine e quindi
	{
		TimerSwitchCompressor.Value=0;
		StoreInt(TimerSwitchCompressor.Value, E2_LastTimerSwitchCompressor);	
	}	

    TimerSwitchPump.Value = LoadInt(E2_LastTimerSwitchPump);                        // Carico ultimo valore timer x switch pompe
	if(TimerSwitchPump.Value==0xFFFF)					// Se valore letto da E2 = FFFF allora EEPROM vergine e quindi
	{
		TimerSwitchPump.Value=0;
		StoreInt(TimerSwitchPump.Value, E2_LastTimerSwitchPump);	
	}	

	EngineBox[0].SelectWorkingPump = LoadInt(E2_LastSelectWorkingPump);				// Carico ultimo stato switch pompe
	if(EngineBox[0].SelectWorkingPump==0xFFFF)                              // Se valore letto da E2 = FFFF allora EEPROM vergine e quindi
	{
		EngineBox[0].SelectWorkingPump=0;
		StoreInt(EngineBox[0].SelectWorkingPump, E2_LastSelectWorkingPump);	
	}  
    
    Room[k_Split_Master_Add].Maintenance_Flag = LoadInt(E2_Maintenance_Flag);				// Carico ultimo stato switch pompe
	if(Room[k_Split_Master_Add].Maintenance_Flag==0xFFFF)					// Se valore letto da E2 = FFFF allora EEPROM vergine e quindi
	{
		Room[k_Split_Master_Add].Maintenance_Flag=0;
		StoreInt(Room[k_Split_Master_Add].Maintenance_Flag, E2_Maintenance_Flag);	// Azzero ore conteggiate in E2
	}	

    Room[k_Split_Master_Add].Maintenance_Set_Time = LoadInt(E2_Maintenance_Set_Time);				// Carico ultimo stato switch pompe
	if(Room[k_Split_Master_Add].Maintenance_Set_Time==0xFFFF)					// Se valore letto da E2 = FFFF allora EEPROM vergine e quindi
	{
		Room[k_Split_Master_Add].Maintenance_Set_Time=k_Max_HoursOfLife;
		StoreInt(Room[k_Split_Master_Add].Maintenance_Set_Time, E2_Maintenance_Set_Time);	// Azzero ore conteggiate in E2
	}	

	//@ oErrLed=LED_ON;    ???????
   
}

void RefreshE2_Data(void)
{
	if(WriteE2Status != 0)
	{

		if(CRC_Flash != (LoadInt(CRC_Hi_Flash)))
			{StoreInt(CRC_Flash, CRC_Hi_Flash);}

		if(SN1 != (ReadRegI2C(EEPROM_Add, System_SN_1)))
			{SetRegI2C(EEPROM_Add, System_SN_1, SN1);}

		if(SN2 != (ReadRegI2C(EEPROM_Add, System_SN_2)))
			{SetRegI2C(EEPROM_Add, System_SN_2, SN2);}

		if(SN3 != (ReadRegI2C(EEPROM_Add, System_SN_3)))
			{SetRegI2C(EEPROM_Add, System_SN_3, SN3);}

		if(SN4 != (ReadRegI2C(EEPROM_Add, System_SN_4)))
			{SetRegI2C(EEPROM_Add, System_SN_4, SN4);}

		if(GiornoFabbricazione != (ReadRegI2C(EEPROM_Add, Fab_Day)))
			{SetRegI2C(EEPROM_Add, Fab_Day, GiornoFabbricazione);}

		if(MeseFabbricazione != (ReadRegI2C(EEPROM_Add, Fab_Month)))
			{SetRegI2C(EEPROM_Add, Fab_Month, MeseFabbricazione);}

		if(AnnoFabbricazione != (ReadRegI2C(EEPROM_Add, Fab_Year)))
			{SetRegI2C(EEPROM_Add, Fab_Year, AnnoFabbricazione);}

		
//		if(EngineBox[0].LastCompressorPowerStatus != (LoadInt(E2_LastCompressorPowerStatus)))
//			{StoreInt(EngineBox[0].LastCompressorPowerStatus, E2_LastCompressorPowerStatus);}	// Memorizzo ultimo stato compressore			
	

		WriteE2Status = 0;
    }
}


void RefreshE2_CompressorStatus(void)
{ 
    unsigned int temp;
    
    if(Room[1].SystemDisable != (LoadInt(E2_Last_SystemDisable)))
	{
        StoreInt(Room[1].SystemDisable, E2_Last_SystemDisable);   // Memorizzo ultimo stato compressore
    }	
    
	if(EngineBox[0].LastCompressorPowerStatus != (LoadInt(E2_LastCompressorPowerStatus)))
	{
        StoreInt(EngineBox[0].LastCompressorPowerStatus, E2_LastCompressorPowerStatus);   // Memorizzo ultimo stato compressore
    }	
	
	if(EngineBox[0].Compressor2Master != (LoadInt(E2_LastCompressor2Master)))
	{
        StoreInt(EngineBox[0].Compressor2Master, E2_LastCompressor2Master);   // Memorizzo ultimo stato switch compressore
    }	   
        
    temp = LoadInt(E2_LastTimerSwitchCompressor);
    if((TimerSwitchCompressor.Value > (temp+300)) ||               //salvo ogni 5 minuti il valore
    (TimerSwitchCompressor.Value < temp))
    {
        StoreInt(TimerSwitchCompressor.Value,E2_LastTimerSwitchCompressor);
    }     

    temp = LoadInt(E2_LastTimerSwitchPump);
    if((TimerSwitchPump.Value > (temp+300)) ||               //salvo ogni 5 minuti il valore
    (TimerSwitchPump.Value < temp))
    {
        StoreInt(TimerSwitchPump.Value,E2_LastTimerSwitchPump);
    }     
    
/*  Vesione che salva su timeout timer dedicato 
    temp = LoadInt(E2_LastTimerSwitchCompressor);
    if((TimerSwitchCompressor.Value != temp) && UpdateTimer.Timeout)               //salvo ogni 5 minuti il valore
    {
        StoreInt(TimerSwitchCompressor.Value,E2_LastTimerSwitchCompressor);
    }     
*/      
    
    if(EngineBox[0].SelectWorkingPump != (LoadInt(E2_LastSelectWorkingPump)))
	{
        StoreInt(EngineBox[0].SelectWorkingPump, E2_LastSelectWorkingPump);				// Carico ultimo stato switch pompe
    }

}


unsigned int Generate_CRC_E2(int StartE2Data, int EndE2Data)
{
	unsigned int CRC_16=0xFFFF;
	unsigned int i = 0;
	unsigned char e2val;
	
	
	for(i=StartE2Data; i <= EndE2Data; i++)
	{
		e2val = ReadRegI2C(EEPROM_Add, i);
		CRC_16 = Mod_BaseCRC16(CRC_16, e2val);
	}
	return CRC_16;
}



void CheckValueRange(void)
{
/*
	CheckVarValueNew(&Touch[0].Script_Split_Pwr, &Syncronize.Script_Split_Pwr, K_LimLo_SPLIT_Pwr, K_LimHi_SPLIT_Pwr, K_Def_SPLIT_Pwr);	
	CheckVarValueNew(&Touch[0].SetPoint, &Syncronize.SetPoint, K_LimLo_SPLIT_SP, K_LimHi_SPLIT_SP, K_Def_SPLIT_SP);
	CheckVarValueNew(&Touch[0].FanMode, &Syncronize.FanMode, K_LimLo_SPLIT_FanMode, K_LimHi_SPLIT_FanMode, K_Def_SPLIT_FanMode);
	CheckVarValueNew(&Touch[0].Script_SetPoint_F, &Syncronize.Script_SetPoint_F, K_LimLo_SPLIT_SP_F, K_LimHi_SPLIT_SP_F, K_Def_SPLIT_SP_F);	
    CheckUtaModeValue();	
	if(Me.My_Address==k_Split_Master_Add)
	{
		CheckVarValue(&Syncronize.SystemDisable, K_LimLo_SPLIT_SystemDisable, K_LimHi_SPLIT_SystemDisable, K_Def_SPLIT_SystemDisable);
		CheckVarValue(&Syncronize.Uta_Power, K_LimLo_SPLIT_Uta_Power, K_LimHi_SPLIT_Uta_Power, K_Def_SPLIT_Uta_Power);
		//CheckVarValue(&Syncronize.Uta_Mode, K_LimLo_SPLIT_Uta_Mode, K_LimHi_SPLIT_Uta_Mode, K_Def_SPLIT_Uta_Mode);
		CheckVarValue(&Syncronize.Uta_SP_Room, K_LimLo_SPLIT_Uta_SP_Room, K_LimHi_SPLIT_Uta_SP_Room, K_Def_SPLIT_Uta_SP_Room);
		CheckVarValue(&Syncronize.Uta_SP_Humi, K_LimLo_SPLIT_Uta_SP_Humi, K_LimHi_SPLIT_Uta_SP_Humi, K_Def_SPLIT_Uta_SP_Humi);
		CheckVarValue(&Syncronize.Uta_Fan_Speed, K_LimLo_SPLIT_Uta_Fan_Speed, K_LimHi_SPLIT_Uta_Fan_Speed, K_Def_SPLIT_Uta_Fan_Speed);
		CheckVarValue(&Syncronize.Uta_SP_Room_F, K_LimLo_SPLIT_Uta_SP_Room_F, K_LimHi_SPLIT_Uta_SP_Room_F, K_Def_SPLIT_Uta_SP_Room_F);
		CheckVarValue(&Syncronize.EcoModeEnable, K_LimLo_SPLIT_EcoModeEn, K_LimHi_SPLIT_EcoModeEn, K_Def_SPLIT_EcoModeEn);
		CheckVarValue(&Syncronize.Power_EcoMode, K_LimLo_SPLIT_PwrEcoMode, K_LimHi_SPLIT_PwrEcoMode, K_Def_SPLIT_PwrEcoMode);
		//CheckVarValue(&EngineBox.Touch_Nation, K_LimLo_SPLIT_Nation, K_LimHi_SPLIT_Nation, K_Def_SPLIT_Nation);
		CheckDefModeValue();
		CheckPowerLimitValue();		
		CheckVarValue(&EngineBox.DoublePumpSwitchTime, K_LimLo_SPLIT_PumpSwitchTime, K_LimHi_SPLIT_PumpSwitchTime, K_Def_SPLIT_PumpSwitchTime);
        CheckPumpSelectionValue();
	}	
	*/
}

/*
void CheckVarValue(int * Var, int LimLo, int LimHi, int DefVal)
{
	if(*Var > LimHi || *Var < LimLo)
		*Var = DefVal;
}

void CheckVarValueNew(int * TouchVar, int * Var, int LimLo, int LimHi, int DefVal)
{
	if(*TouchVar > LimHi || *TouchVar < LimLo)
		*Var = DefVal;
}


void CheckDefModeValue(void)
{
	if(Syncronize.DefMode != CoreRiscaldamento && Syncronize.DefMode != CoreRaffrescamento && Syncronize.DefMode != CoreVentilazione)
		Syncronize.DefMode = CoreVentilazione;
}

void CheckUtaModeValue(void)
{
	if(Syncronize.Uta_Mode != CoreVentilazione && Syncronize.Uta_Mode != CoreUtaAuto)
		Syncronize.Uta_Mode = CoreVentilazione;
}

void CheckPowerLimitValue(void)
{
	if(Touch[0].PowerLimit > K_LimHi_PWR_Limit || Touch[0].PowerLimit < K_LimLo_PWR_Limit)
		Touch[0].PowerLimit = K_Def_PWR_Limit;
	
	//if(Touch[0].Enable && !Touch[0].OffLine)
    if(Touch[0].OnLine)
		WriteModReg (Add_TouchRoom, REG_TOUCH_MAX_PWR_LIMIT, Touch[0].PowerLimit);	
}

void CheckPumpSelectionValue(void)
{
	if(EngineBox.DoublePumpSelection != PumpSelection_Auto && EngineBox.DoublePumpSelection != PumpSelection_1 && EngineBox.DoublePumpSelection != PumpSelection_2)
		EngineBox.DoublePumpSelection = PumpSelection_Auto;
}
*/

void EraseEEPROM(void)
{
    int temp;
    
    for(temp=0; temp<K_EEPROM_WordSize; temp++)
    {
        StoreInt(K_Reset_EEPROM_Value, temp*2);
    }
}

void LoadAllEEPROM_Debug(int * Buffer)
{
    int temp;

    for(temp=0; temp<K_EEPROM_WordSize; temp++)
    {
        *(Buffer+temp) = LoadInt(temp*2); 
    }
}
