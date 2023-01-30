//----------------------------------------------------------------------------------
//	Progect name:	EEPROM.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			22/05/2011
//	Description:	Corpo delle funzioni per l'allocazione della E2 del micro
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
#include "DefinePeriferiche.h"
#include "delay.h"
#include "ADC.h"
#include "I2C.h"
#include "PWM.h"
#include "Valvola_PassoPasso.h"
#include "EEPROM.h"
#include "Core.h"
#include "ProtocolloModBusSec.h"
#include "ProtocolloComunicazione.h"
#include "Driver_ModBusSec.h"

//----------------------------------------------------------------------------------
//	Variabili
//----------------------------------------------------------------------------------
unsigned int VirtualE2PageBuff[K_EEPROM_ByteSize];


void StoreIntVirt(int Data, int Address)
{
	unsigned char ByteLow, ByteHigh;

	ByteLow = LoByte(Data);
	ByteHigh = HiByte(Data);
    
    VirtualE2PageBuff[Address] = ByteHigh;
    VirtualE2PageBuff[Address+1] = ByteLow;
}

int LoadIntVirt(int Address)
{
	char ByteLow, ByteHigh;

	ByteHigh = VirtualE2PageBuff[Address];
	ByteLow = VirtualE2PageBuff[Address+1];
	
	return ToInt(ByteHigh, ByteLow);
}

unsigned int Store_Offset_SectorPage(int SectorPointer)
{
    unsigned int AddrOffset;
    
    //AddrOffset = (SectorPointer << 8) & 0x0700; 
    
    switch(SectorPointer)
    {
        case 0:
            AddrOffset = 0;
        break;
        case 1:
            AddrOffset = 0x100;
        break;
        case 2:
            AddrOffset = 0x200;
        break;
        case 3:
            AddrOffset = 0x300;
        break;
        case 4:
            AddrOffset = 0x400;
        break;
        case 5:
            AddrOffset = 0x500;
        break;
        case 6:
            AddrOffset = 0x600;
        break;
        case 7:
            AddrOffset = 0x700;
        break;
    }
    
    return AddrOffset;
}

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

char HiMidByte(long Data)
{
	return (char)((Data & 0xFF000000) >> 24); //32);
}

char LoMidByte(long Data)
{
	return (char)((Data & 0x00FF0000) >> 16);
}


int ToInt(char HiByte, char LoByte)
{
//	return (((int)HiByte) <<8 )| (LoByte&0x00FF);
	return ((((int)HiByte) <<8 ) & 0xFF00) | (LoByte&0x00FF);	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
}

void StoreInt(int Data, int Address)
{
	unsigned char ByteLow, ByteHigh;
    unsigned char RetVal;

	ByteLow = LoByte(Data);
	ByteHigh = HiByte(Data);
    
    RetVal = SetRegI2C(EEPROM_Add, Address, ByteHigh);
    RetVal = SetRegI2C(EEPROM_Add, Address +1, ByteLow);   
    
    // Aggiunto x gestire buffer virtuale EEPROM
    VirtualE2PageBuff[Address] = ByteHigh;
    VirtualE2PageBuff[Address+1] = ByteLow;        
}

int LoadInt(int Address, int UseVirtualBuffer)
{
	char ByteLow, ByteHigh;

    if(UseVirtualBuffer)
    {
        ByteHigh = VirtualE2PageBuff[Address];
        ByteLow = VirtualE2PageBuff[Address+1];           
    }   
    else
    {
        ByteHigh = ReadRegI2C(EEPROM_Add, Address);
        ByteLow = ReadRegI2C(EEPROM_Add, Address +1);         
    }   

	return ToInt(ByteHigh, ByteLow);
}




long ToLong(char HiMidByte, char LoMidByte, char HiByte, char LoByte)
{
//	return (((int)HiByte) <<8 )| (LoByte&0x00FF);
	return  ((((long)HiMidByte) <<24) & 0xFF000000) | ((((long)LoMidByte) <<16) & 0x00FF0000) | ((((long)HiByte) <<8) & 0x0000FF00) | (LoByte&0x00FF);	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
}


void StoreLong(long Data, int Address)
{
	unsigned char ByteMidLow, ByteMidHigh, ByteLow, ByteHigh;

	ByteLow = LoByte(Data);
	ByteHigh = HiByte(Data);
	ByteMidLow = LoMidByte(Data);
	ByteMidHigh = HiMidByte(Data);
	
	SetRegI2C(EEPROM_Add, Address, ByteHigh);
	SetRegI2C(EEPROM_Add, Address +1, ByteLow);
	SetRegI2C(EEPROM_Add, Address +2, ByteMidHigh);
	SetRegI2C(EEPROM_Add, Address +3, ByteMidLow);    
    
    // Aggiunto x gestire buffer virtuale EEPROM
    VirtualE2PageBuff[Address] = ByteHigh;
    VirtualE2PageBuff[Address+1] = ByteLow;           
    VirtualE2PageBuff[Address+2] = ByteMidHigh;
    VirtualE2PageBuff[Address+3] = ByteMidLow;           
}


long LoadLong(int Address, int UseVirtualBuffer)
{
	char ByteLow, ByteHigh, ByteMidLow, ByteMidHigh;
        
    if(UseVirtualBuffer)
    {
        ByteHigh = VirtualE2PageBuff[Address];
        ByteLow = VirtualE2PageBuff[Address+1];           
        ByteMidHigh = VirtualE2PageBuff[Address+2];
        ByteMidLow = VirtualE2PageBuff[Address+3];           
    }   
    else
    {
        ByteHigh = ReadRegI2C(EEPROM_Add, Address);
        ByteLow = ReadRegI2C(EEPROM_Add, Address +1);
        ByteMidHigh = ReadRegI2C(EEPROM_Add, Address +2);
        ByteMidLow = ReadRegI2C(EEPROM_Add, Address +3);        
    }       
	
	return ToLong(ByteMidHigh, ByteMidLow, ByteHigh, ByteLow);
}



/*
void LoadAllE2_Data(void)
{
	//char i=0;

	//oErrLed=LED_ON;

	CRC_Flash = LoadInt(CRC_Hi_Flash);
	SN1 = ReadRegI2C(EEPROM_Add, System_SN_1);
	SN2 = ReadRegI2C(EEPROM_Add, System_SN_2);
	SN3 = ReadRegI2C(EEPROM_Add, System_SN_3);
	SN4 = ReadRegI2C(EEPROM_Add, System_SN_4);
	GiornoFabbricazione = ReadRegI2C(EEPROM_Add, Fab_Day);
	MeseFabbricazione = ReadRegI2C(EEPROM_Add, Fab_Month);
	AnnoFabbricazione = ReadRegI2C(EEPROM_Add, Fab_Year);

	// Inizializzazione parametri di lavoro da K e non più da caricamento EEPROM

	//oErrLed=LED_OFF;
}
*/

/*
void RefreshE2_Data(void)
{
    
    
    
	if(WriteE2Status != 0)
	{
		unsigned char i = 0;
		int E2Data = 0;

		//oErrLed=LED_ON;


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

		
		
		WriteE2Status = 0;
		//oErrLed=LED_OFF;
	}
	

}
*/	

//--------------------------------------------------------------------------------------------------------------------------------
// Esegue il refresh dei registri che hanno una posizione in E2 se il relativo valore è cambiato
// In doppia locazione (settore speculare) per avere una maggiore sicurezza di avere una tabella dati integra in caso di 
// spegnimento scheda durante la fase di refresh.
//--------------------------------------------------------------------------------------------------------------------------------
void RefreshE2_TouchValue_DoubleSector(int UseVirtualBuffer)
{
    RefreshE2_TouchValue(K_EEPROM_Primary_Sector, UseVirtualBuffer);
    RefreshE2_TouchValue(K_EEPROM_Secondary_Sector, UseVirtualBuffer);
}

//--------------------------------------------------------------------------------------------------------------------------------
// Esegue il caricamento dei dati da EEPROM nei relativi registri se il CRC dati ne conferma l'integrità.
// In caso di CRC corrotto il caricamento viene tentato anche in un secondo settore speculare.
// Se fallisce anche il secondo vengono caricati i valori di default.
//--------------------------------------------------------------------------------------------------------------------------------
void LoadE2_TouchValue_DoubleSector(void)
{
    LoadE2_TouchValue(K_EEPROM_Primary_Sector, K_EEPROM_Secondary_Sector); 
}

//--------------------------------------------------------------------------------------------------------------------------------
// Esegue il refresh dei registri che hanno una posizione in E2 se il relativo valore è cambiato
//--------------------------------------------------------------------------------------------------------------------------------
void RefreshE2_TouchValue(int SectorPointer, int UseVirtualBuffer)
{
	int flag=0;
	unsigned int CRC_calc;
    unsigned int AddrOffset;
    long temp;
        
    //AddrOffset = (SectorPointer << 8) & 0x0700; 
    
    switch(SectorPointer)
    {
        case 0:
            AddrOffset = 0;
        break;
        case 1:
            AddrOffset = 0x100;
        break;
        case 2:
            AddrOffset = 0x200;
        break;
        case 3:
            AddrOffset = 0x300;
        break;
        case 4:
            AddrOffset = 0x400;
        break;
        case 5:
            AddrOffset = 0x500;
        break;
        case 6:
            AddrOffset = 0x600;
        break;
        case 7:
            AddrOffset = 0x700;
        break;
    }
    

	if(Touch[0].HeaterPwr != (LoadInt(E2_SPLIT_Heater_Pwr_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].HeaterPwr, E2_SPLIT_Heater_Pwr_Hi+AddrOffset); flag=1;}		
    
	if(EngineBox.DoublePumpSwitchTime != (LoadInt(E2_SPLIT_PumpSwitchTime_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(EngineBox.DoublePumpSwitchTime, E2_SPLIT_PumpSwitchTime_Hi+AddrOffset); flag=1;}		

	if(EngineBox.DoublePumpSelection != (LoadInt(E2_SPLIT_PumpSelection_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(EngineBox.DoublePumpSelection, E2_SPLIT_PumpSelection_Hi+AddrOffset); flag=1;}		
	
	if(Touch[0].SystemDisable != (LoadInt(E2_SPLIT_SystemDisable_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].SystemDisable, E2_SPLIT_SystemDisable_Hi+AddrOffset); flag=1;}		
		
	if(Touch[0].Uta_Power != (LoadInt(E2_SPLIT_Uta_Power_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Uta_Power, E2_SPLIT_Uta_Power_Hi+AddrOffset); flag=1;}	
	
	if(Touch[0].Uta_Mode != (LoadInt(E2_SPLIT_Uta_Mode_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Uta_Mode, E2_SPLIT_Uta_Mode_Hi+AddrOffset); flag=1;}	
	
	if(Touch[0].Uta_SP_Room != (LoadInt(E2_SPLIT_Uta_SP_Room_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Uta_SP_Room, E2_SPLIT_Uta_SP_Room_Hi+AddrOffset); flag=1;}	
	
	if(Touch[0].Uta_SP_Humi != (LoadInt(E2_SPLIT_Uta_SP_Humi_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Uta_SP_Humi, E2_SPLIT_Uta_SP_Humi_Hi+AddrOffset); flag=1;}	
	
	if(Touch[0].Uta_Fan_Speed != (LoadInt(E2_SPLIT_Uta_Fan_Speed_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Uta_Fan_Speed, E2_SPLIT_Uta_Fan_Speed_Hi+AddrOffset); flag=1;}	
		
	if(Touch[0].Uta_SP_Room_F != (LoadInt(E2_SPLIT_Uta_SP_Room_F_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Uta_SP_Room_F, E2_SPLIT_Uta_SP_Room_F_Hi+AddrOffset); flag=1;}	

	if(Me.EcoModeEnable != (LoadInt(E2_SPLIT_ECOMODEEN_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Me.EcoModeEnable, E2_SPLIT_ECOMODEEN_Hi+AddrOffset); flag=1;}	
	
	if(Me.Power_EcoMode != (LoadInt(E2_SPLIT_PWR_ECOMODE_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Me.Power_EcoMode, E2_SPLIT_PWR_ECOMODE_Hi+AddrOffset); flag=1;}		

	if(EngineBox.Touch_Nation != (LoadInt(E2_SPLIT_NATION_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(EngineBox.Touch_Nation, E2_SPLIT_NATION_Hi+AddrOffset); flag=1;}	
	
	if(Touch[0].FanMode != (LoadInt(E2_SPLIT_FanMode_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].FanMode, E2_SPLIT_FanMode_Hi+AddrOffset); flag=1;}

	if(Touch[0].SetPoint != (LoadInt(E2_SPLIT_SP_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].SetPoint, E2_SPLIT_SP_Hi+AddrOffset); flag=1;}

	if(Touch[0].DefMode != (LoadInt(E2_SPLIT_Mode_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].DefMode, E2_SPLIT_Mode_Hi+AddrOffset); flag=1;}

	if(Touch[0].Script_Split_Pwr != (LoadInt(E2_SPLIT_Pwr_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Script_Split_Pwr, E2_SPLIT_Pwr_Hi+AddrOffset); flag=1;}

	if(Touch[0].PowerLimit != (LoadInt(E2_PWR_Limit_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].PowerLimit, E2_PWR_Limit_Hi+AddrOffset); flag=1;}

	if(Touch[0].Script_SetPoint_F != (LoadInt(E2_SPLIT_SP_F_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Script_SetPoint_F, E2_SPLIT_SP_F_Hi+AddrOffset); flag=1;}

    
	if(Touch[0].On_Priority_Mode != (LoadInt(E2_SPLIT_ON_PRI_MODE_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].On_Priority_Mode, E2_SPLIT_ON_PRI_MODE_Hi+AddrOffset); flag=1;}
       
	if(Touch[0].ValveOnDemand_Min_Percent_Val != (LoadInt(E2_VALVE_ONDEM_MIN_PERC_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].ValveOnDemand_Min_Percent_Val, E2_VALVE_ONDEM_MIN_PERC_Hi+AddrOffset); flag=1;}
    
	if(Touch[0].Max_Lim_ON_Demand_Total_Score != (LoadInt(E2_MAXLIM_ONDEM_TOTSCORE_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Max_Lim_ON_Demand_Total_Score, E2_MAXLIM_ONDEM_TOTSCORE_Hi+AddrOffset); flag=1;}
    
	if(Touch[0].TimeOnDemandValveSwitch != (LoadInt(E2_TIME_ONDEM_VALVE_SW_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TimeOnDemandValveSwitch, E2_TIME_ONDEM_VALVE_SW_Hi+AddrOffset); flag=1;}
    

	if(Touch[0].TestAll_Abil != (LoadInt(E2_TOUCH_TESTALL_ABIL_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_Abil, E2_TOUCH_TESTALL_ABIL_Hi+AddrOffset); flag=1;}

	if(Touch[0].TestAll_SuperHeat_SP_Cool != (LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_SP_COOL_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_SuperHeat_SP_Cool, E2_TOUCH_TESTALL_SUPERHEAT_SP_COOL_Hi+AddrOffset); flag=1;}

	if(Touch[0].TestAll_SuperHeat_SP_Heat != (LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_SP_HEAT_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_SuperHeat_SP_Heat, E2_TOUCH_TESTALL_SUPERHEAT_SP_HEAT_Hi+AddrOffset); flag=1;}

	if(Touch[0].TestAll_MaxOpValve_Cool != (LoadInt(E2_TOUCH_TESTALL_MAX_OPEN_VALVE_COOL_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_MaxOpValve_Cool, E2_TOUCH_TESTALL_MAX_OPEN_VALVE_COOL_Hi+AddrOffset); flag=1;}

	if(Touch[0].TestAll_MaxOpValve_Heat != (LoadInt(E2_TOUCH_TESTALL_MAX_OPEN_VALVE_HEAT_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_MaxOpValve_Heat, E2_TOUCH_TESTALL_MAX_OPEN_VALVE_HEAT_Hi+AddrOffset); flag=1;}

	if(Touch[0].TestAll_TempBatt_SP_Cool != (LoadInt(E2_TOUCH_TESTALL_WORK_TEMP_COOL_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_TempBatt_SP_Cool, E2_TOUCH_TESTALL_WORK_TEMP_COOL_Hi+AddrOffset); flag=1;}

	if(Touch[0].TestAll_TempBatt_SP_Heat != (LoadInt(E2_TOUCH_TESTALL_WORK_TEMP_HEAT_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_TempBatt_SP_Heat, E2_TOUCH_TESTALL_WORK_TEMP_HEAT_Hi+AddrOffset); flag=1;}
   
    
	if(Touch[0].TestALL_Frozen_Abil != (LoadInt(E2_TOUCH_TESTALL_FROZEN_ABIL_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestALL_Frozen_Abil, E2_TOUCH_TESTALL_FROZEN_ABIL_Hi+AddrOffset); flag=1;}
    
	if(Touch[0].TestAll_SuperHeat_SP_Frigo != (LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_SP_FRIGO_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_SuperHeat_SP_Frigo, E2_TOUCH_TESTALL_SUPERHEAT_SP_FRIGO_Hi+AddrOffset); flag=1;}
    
	if(Touch[0].TestAll_SuperHeat_SP_Freezer != (LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_SP_FREEZER_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_SuperHeat_SP_Freezer, E2_TOUCH_TESTALL_SUPERHEAT_SP_FREEZER_Hi+AddrOffset); flag=1;}
    
	if(Touch[0].TestAll_MaxOpValve_Frigo != (LoadInt(E2_TOUCH_TESTALL_MAXOPVALVE_FRIGO_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_MaxOpValve_Frigo, E2_TOUCH_TESTALL_MAXOPVALVE_FRIGO_Hi+AddrOffset); flag=1;}
    
	if(Touch[0].TestAll_MaxOpValve_Freezer != (LoadInt(E2_TOUCH_TESTALL_MAXOPVALVE_FREEZER_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_MaxOpValve_Freezer, E2_TOUCH_TESTALL_MAXOPVALVE_FREEZER_Hi+AddrOffset); flag=1;}
    
	if(Touch[0].TestAll_TempBatt_SP_Frigo != (LoadInt(E2_TOUCH_TESTALL_TEMPBATT_SP_FRIGO_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_TempBatt_SP_Frigo, E2_TOUCH_TESTALL_TEMPBATT_SP_FRIGO_Hi+AddrOffset); flag=1;}
    
	if(Touch[0].TestAll_TempBatt_SP_Freezer != (LoadInt(E2_TOUCH_TESTALL_TEMPBATT_SP_FREEZER_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_TempBatt_SP_Freezer, E2_TOUCH_TESTALL_TEMPBATT_SP_FREEZER_Hi+AddrOffset); flag=1;}

	if(Touch[0].Abil_Defrosting != (LoadInt(E2_TOUCH_ABIL_DEFROSTING_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Abil_Defrosting, E2_TOUCH_ABIL_DEFROSTING_Hi+AddrOffset); flag=1;}

	if(Touch[0].Defrosting_Step_Time != (LoadInt(E2_TOUCH_DEFROSTING_STEP_TIME_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Defrosting_Step_Time, E2_TOUCH_DEFROSTING_STEP_TIME_Hi+AddrOffset); flag=1;}

	if(Touch[0].Defrosting_Time != (LoadInt(E2_TOUCH_DEFROSTING_TIME_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Defrosting_Time, E2_TOUCH_DEFROSTING_TIME_Hi+AddrOffset); flag=1;}

	if(Touch[0].Defrosting_Temp_Set != (LoadInt(E2_TOUCH_DEFROSTING_TEMP_SET_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Defrosting_Temp_Set, E2_TOUCH_DEFROSTING_TEMP_SET_Hi+AddrOffset); flag=1;}

	if(Touch[0].Dripping_Time != (LoadInt(E2_TOUCH_DRIPPING_TIME_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Dripping_Time, E2_TOUCH_DRIPPING_TIME_Hi+AddrOffset); flag=1;}
    
	if(Touch[0].Valvola_Frozen != (LoadInt(E2_TOUCH_VALVOLA_FROZEN_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Valvola_Frozen, E2_TOUCH_VALVOLA_FROZEN_Hi+AddrOffset); flag=1;}
    
#if (K_AbilSplitDefrostingCycle==1)					
    temp = LoadLong(E2_DEFROSTING_CYCLE_WORK_VALUE_MidHi+AddrOffset, UseVirtualBuffer);
	if((TimerDefrostingCycleWork.Value > (temp+300)) || (TimerDefrostingCycleWork.Value < temp))
		{StoreLong(TimerDefrostingCycleWork.Value, E2_DEFROSTING_CYCLE_WORK_VALUE_MidHi+AddrOffset); flag=1;}
#endif

	if(Touch[0].TestAll_MinOpValve_Cool != (LoadInt(E2_TOUCH_TESTALL_MINOPVALVE_COOL_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_MinOpValve_Cool, E2_TOUCH_TESTALL_MINOPVALVE_COOL_Hi+AddrOffset); flag=1;}
    
	if(Touch[0].TestAll_MinOpValve_Heat != (LoadInt(E2_TOUCH_TESTALL_MINOPVALVE_HEAT_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_MinOpValve_Heat, E2_TOUCH_TESTALL_MINOPVALVE_HEAT_Hi+AddrOffset); flag=1;}

	if(Touch[0].TestAll_MinOpValve_Frigo != (LoadInt(E2_TOUCH_TESTALL_MINOPVALVE_FRIGO_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_MinOpValve_Frigo, E2_TOUCH_TESTALL_MINOPVALVE_FRIGO_Hi+AddrOffset); flag=1;}

   	if(Touch[0].TestAll_MinOpValve_Freezer != (LoadInt(E2_TOUCH_TESTALL_MINOPVALVE_FREEZER_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_MinOpValve_Freezer, E2_TOUCH_TESTALL_MINOPVALVE_FREEZER_Hi+AddrOffset); flag=1;}

	if(Touch[0].TestAll_SuperHeat_Heat_pGain != (LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_SuperHeat_Heat_pGain, E2_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN_Hi+AddrOffset); flag=1;}

	if(Touch[0].TestAll_SuperHeat_Heat_iGain != (LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_HEAT_IGAIN_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_SuperHeat_Heat_iGain, E2_TOUCH_TESTALL_SUPERHEAT_HEAT_IGAIN_Hi+AddrOffset); flag=1;}

	if(Touch[0].TestAll_SuperHeat_Heat_dGain != (LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_HEAT_DGAIN_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_SuperHeat_Heat_dGain, E2_TOUCH_TESTALL_SUPERHEAT_HEAT_DGAIN_Hi+AddrOffset); flag=1;}

	if(Touch[0].TestAll_SuperHeat_Heat_Time_Exec_PID != (LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_SuperHeat_Heat_Time_Exec_PID, E2_TOUCH_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID_Hi+AddrOffset); flag=1;}
 
	if(Touch[0].TestAll_SuperHeat_Cool_pGain != (LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_COOL_PGAIN_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_SuperHeat_Cool_pGain, E2_TOUCH_TESTALL_SUPERHEAT_COOL_PGAIN_Hi+AddrOffset); flag=1;}

	if(Touch[0].TestAll_SuperHeat_Cool_iGain != (LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_COOL_IGAIN_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_SuperHeat_Cool_iGain, E2_TOUCH_TESTALL_SUPERHEAT_COOL_IGAIN_Hi+AddrOffset); flag=1;}

	if(Touch[0].TestAll_SuperHeat_Cool_dGain != (LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_COOL_DGAIN_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_SuperHeat_Cool_dGain, E2_TOUCH_TESTALL_SUPERHEAT_COOL_DGAIN_Hi+AddrOffset); flag=1;}

	if(Touch[0].TestAll_SuperHeat_Cool_Time_Exec_PID != (LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].TestAll_SuperHeat_Cool_Time_Exec_PID, E2_TOUCH_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID_Hi+AddrOffset); flag=1;}

	if(Touch[0].Maintenance_Flag != (LoadInt(E2_TOUCH_MAINTENANCE_FLAG_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Maintenance_Flag, E2_TOUCH_MAINTENANCE_FLAG_Hi+AddrOffset); flag=1;}

	if(Touch[0].Maintenance_Set_Time != (LoadInt(E2_TOUCH_MAINTENANCE_SET_TIME_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Maintenance_Set_Time, E2_TOUCH_MAINTENANCE_SET_TIME_Hi+AddrOffset); flag=1;}

	if(Touch[0].SuperHeat_Cool_Min_SetPoint != (LoadInt(E2_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].SuperHeat_Cool_Min_SetPoint, E2_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT_Hi+AddrOffset); flag=1;}

	if(Touch[0].SuperHeat_Cool_Max_Value_Correct != (LoadInt(E2_TOUCH_SUPERHEAT_COOL_MAX_VALUE_CORRECT_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].SuperHeat_Cool_Max_Value_Correct, E2_TOUCH_SUPERHEAT_COOL_MAX_VALUE_CORRECT_Hi+AddrOffset); flag=1;}

	if(Touch[0].SuperHeat_Heat_Min_SetPoint != (LoadInt(E2_TOUCH_SUPERHEAT_HEAT_MIN_SETPOINT_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].SuperHeat_Heat_Min_SetPoint, E2_TOUCH_SUPERHEAT_HEAT_MIN_SETPOINT_Hi+AddrOffset); flag=1;}
    
	if(Touch[0].SuperHeat_Heat_Max_SetPoint != (LoadInt(E2_TOUCH_SUPERHEAT_HEAT_MAX_SETPOINT_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].SuperHeat_Heat_Max_SetPoint, E2_TOUCH_SUPERHEAT_HEAT_MAX_SETPOINT_Hi+AddrOffset); flag=1;}

	if(Touch[0].Threshold_Compressor_Hi != (LoadInt(E2_TOUCH_THRESHOLD_COMPRESSOR_HI_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Threshold_Compressor_Hi, E2_TOUCH_THRESHOLD_COMPRESSOR_HI_Hi+AddrOffset); flag=1;}

	if(Touch[0].Set_Humi != (LoadInt(E2_TOUCH_SPLIT_SET_HUMI_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Set_Humi, E2_TOUCH_SPLIT_SET_HUMI_Hi+AddrOffset); flag=1;}

	if(Touch[0].Set_Delta_Temp_Min_Dehumi != (LoadInt(E2_TOUCH_SET_DELTA_TEMP_MIN_DEHUMI_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Set_Delta_Temp_Min_Dehumi, E2_TOUCH_SET_DELTA_TEMP_MIN_DEHUMI_Hi+AddrOffset); flag=1;}

	if(Touch[0].Set_Fan_Speed_Dehumi != (LoadInt(E2_TOUCH_SET_FAN_SPEED_DEHUMI_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Set_Fan_Speed_Dehumi, E2_TOUCH_SET_FAN_SPEED_DEHUMI_Hi+AddrOffset); flag=1;}

	if(Touch[0].Set_Press_Dehumi != (LoadInt(E2_TOUCH_SET_PRESS_DEHUMI_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Set_Press_Dehumi, E2_TOUCH_SET_PRESS_DEHUMI_Hi+AddrOffset); flag=1;}

	if(Touch[0].Set_Temp_Air_Out != (LoadInt(E2_TOUCH_SET_TEMP_AIR_OUT_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Set_Temp_Air_Out, E2_TOUCH_SET_TEMP_AIR_OUT_Hi+AddrOffset); flag=1;}
 
	if(Touch[0].Set_Temp_Air_Out_Dehumi != (LoadInt(E2_TOUCH_SET_TEMP_AIR_OUT_DEHUMI_Hi+AddrOffset, UseVirtualBuffer)))
		{StoreInt(Touch[0].Set_Temp_Air_Out_Dehumi, E2_TOUCH_SET_TEMP_AIR_OUT_DEHUMI_Hi+AddrOffset); flag=1;}
    
	if(flag==1)
	{
		CRC_calc = Generate_CRC_E2(E2_START_OF_SPACE_Hi+AddrOffset, E2_END_OF_SPACE_Lo+AddrOffset, UseVirtualBuffer);
		StoreInt(CRC_calc, E2_CRC_TouchVal_Hi+AddrOffset);
	}

}


//--------------------------------------------------------------------------------------------------------------------------------
// Esegue il caricamento dei dati da EEPROM nei relativi registri se il CRC dati ne conferma l'integrità.
// Altrimenti vengono caricati i valori di default.
//--------------------------------------------------------------------------------------------------------------------------------
void LoadE2_TouchValue(int PrimarySectorPointer, int SecondarySectorPointer)
{
	unsigned int CRC_Eeprom1;
	unsigned int CRC_calc1;
	unsigned int CRC_Eeprom2;
	unsigned int CRC_calc2;
    unsigned int AddrOffset=0;
    unsigned int AddrOffset1;
    unsigned int AddrOffset2;
    
    
    //AddrOffset1 = (PrimarySectorPointer << 8) & 0x0700; 
    //AddrOffset2 = (SecondarySectorPointer << 8) & 0x0700; 
    
    switch(PrimarySectorPointer)
    {
        case 0:
            AddrOffset1 = 0;
        break;
        case 1:
            AddrOffset1 = 0x100;
        break;
        case 2:
            AddrOffset1 = 0x200;
        break;
        case 3:
            AddrOffset1 = 0x300;
        break;
        case 4:
            AddrOffset1 = 0x400;
        break;
        case 5:
            AddrOffset1 = 0x500;
        break;
        case 6:
            AddrOffset1 = 0x600;
        break;
        case 7:
            AddrOffset1 = 0x700;
        break;
    }
    
    switch(SecondarySectorPointer)
    {
        case 0:
            AddrOffset2 = 0;
        break;
        case 1:
            AddrOffset2 = 0x100;
        break;
        case 2:
            AddrOffset2 = 0x200;
        break;
        case 3:
            AddrOffset2 = 0x300;
        break;
        case 4:
            AddrOffset2 = 0x400;
        break;
        case 5:
            AddrOffset2 = 0x500;
        break;
        case 6:
            AddrOffset2 = 0x600;
        break;
        case 7:
            AddrOffset2 = 0x700;
        break;
    }    


	CRC_Eeprom1 = LoadInt(E2_CRC_TouchVal_Hi+AddrOffset1, K_UseRealE2);
	CRC_calc1 = Generate_CRC_E2(E2_START_OF_SPACE_Hi+AddrOffset1, E2_END_OF_SPACE_Lo+AddrOffset1, K_UseRealE2);
	
	CRC_Eeprom2 = LoadInt(E2_CRC_TouchVal_Hi+AddrOffset2, K_UseRealE2);
	CRC_calc2 = Generate_CRC_E2(E2_START_OF_SPACE_Hi+AddrOffset2, E2_END_OF_SPACE_Lo+AddrOffset2, K_UseRealE2);
    
    if(CRC_calc1 == CRC_Eeprom1)
        AddrOffset = AddrOffset1;
    else if(CRC_calc2 == CRC_Eeprom2)
        AddrOffset = AddrOffset2;

	if(CRC_calc1 == CRC_Eeprom1 || CRC_calc2 == CRC_Eeprom2)
	{
        Touch[0].HeaterPwr = LoadInt(E2_SPLIT_Heater_Pwr_Hi+AddrOffset, K_UseRealE2);    
        EngineBox.DoublePumpSwitchTime = LoadInt(E2_SPLIT_PumpSwitchTime_Hi+AddrOffset, K_UseRealE2);
        EngineBox.DoublePumpSelection = LoadInt(E2_SPLIT_PumpSelection_Hi+AddrOffset, K_UseRealE2);    
        Touch[0].SystemDisable = LoadInt(E2_SPLIT_SystemDisable_Hi+AddrOffset, K_UseRealE2);
        Touch[0].Uta_Power = LoadInt(E2_SPLIT_Uta_Power_Hi+AddrOffset, K_UseRealE2);
        Touch[0].Uta_Mode = LoadInt(E2_SPLIT_Uta_Mode_Hi+AddrOffset, K_UseRealE2);
        Touch[0].Uta_SP_Room = LoadInt(E2_SPLIT_Uta_SP_Room_Hi+AddrOffset, K_UseRealE2);
        Touch[0].Uta_SP_Humi = LoadInt(E2_SPLIT_Uta_SP_Humi_Hi+AddrOffset, K_UseRealE2);
        Touch[0].Uta_Fan_Speed = LoadInt(E2_SPLIT_Uta_Fan_Speed_Hi+AddrOffset, K_UseRealE2);
        Touch[0].Uta_SP_Room_F = LoadInt(E2_SPLIT_Uta_SP_Room_F_Hi+AddrOffset, K_UseRealE2);
        Me.EcoModeEnable = LoadInt(E2_SPLIT_ECOMODEEN_Hi+AddrOffset, K_UseRealE2);      
        Me.Power_EcoMode = LoadInt(E2_SPLIT_PWR_ECOMODE_Hi+AddrOffset, K_UseRealE2);	
        EngineBox.Touch_Nation = LoadInt(E2_SPLIT_NATION_Hi+AddrOffset, K_UseRealE2);	
        Touch[0].FanMode = LoadInt(E2_SPLIT_FanMode_Hi+AddrOffset, K_UseRealE2);
        Touch[0].SetPoint = LoadInt(E2_SPLIT_SP_Hi+AddrOffset, K_UseRealE2);
        Touch[0].DefMode = LoadInt(E2_SPLIT_Mode_Hi+AddrOffset, K_UseRealE2);
        Touch[0].Script_Split_Pwr = LoadInt(E2_SPLIT_Pwr_Hi+AddrOffset, K_UseRealE2);
        Touch[0].PowerLimit = LoadInt(E2_PWR_Limit_Hi+AddrOffset, K_UseRealE2);
        Touch[0].Script_SetPoint_F = LoadInt(E2_SPLIT_SP_F_Hi+AddrOffset, K_UseRealE2);        
        
        Touch[0].On_Priority_Mode = LoadInt(E2_SPLIT_ON_PRI_MODE_Hi+AddrOffset, K_UseRealE2);
        Touch[0].ValveOnDemand_Min_Percent_Val = LoadInt(E2_VALVE_ONDEM_MIN_PERC_Hi+AddrOffset, K_UseRealE2);
        Touch[0].Max_Lim_ON_Demand_Total_Score = LoadInt(E2_MAXLIM_ONDEM_TOTSCORE_Hi+AddrOffset, K_UseRealE2);
        Touch[0].TimeOnDemandValveSwitch = LoadInt(E2_TIME_ONDEM_VALVE_SW_Hi+AddrOffset, K_UseRealE2);

        
        Touch[0].TestAll_Abil = LoadInt(E2_TOUCH_TESTALL_ABIL_Hi+AddrOffset, K_UseRealE2) & ~0x8000; // Forzo azzeramento flag di "ProgrammingMode"       
        Touch[0].TestAll_SuperHeat_SP_Cool = LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_SP_COOL_Hi+AddrOffset, K_UseRealE2);
        Touch[0].TestAll_SuperHeat_SP_Heat = LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_SP_HEAT_Hi+AddrOffset, K_UseRealE2);
        Touch[0].TestAll_MaxOpValve_Cool = LoadInt(E2_TOUCH_TESTALL_MAX_OPEN_VALVE_COOL_Hi+AddrOffset, K_UseRealE2);
        Touch[0].TestAll_MaxOpValve_Heat = LoadInt(E2_TOUCH_TESTALL_MAX_OPEN_VALVE_HEAT_Hi+AddrOffset, K_UseRealE2);
        Touch[0].TestAll_TempBatt_SP_Cool = LoadInt(E2_TOUCH_TESTALL_WORK_TEMP_COOL_Hi+AddrOffset, K_UseRealE2);
        Touch[0].TestAll_TempBatt_SP_Heat = LoadInt(E2_TOUCH_TESTALL_WORK_TEMP_HEAT_Hi+AddrOffset, K_UseRealE2);        
        
        Touch[0].TestALL_Frozen_Abil = LoadInt(E2_TOUCH_TESTALL_FROZEN_ABIL_Hi+AddrOffset, K_UseRealE2);
        Touch[0].TestAll_SuperHeat_SP_Frigo = LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_SP_FRIGO_Hi+AddrOffset, K_UseRealE2);
        Touch[0].TestAll_SuperHeat_SP_Freezer = LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_SP_FREEZER_Hi+AddrOffset, K_UseRealE2);
        Touch[0].TestAll_MaxOpValve_Frigo = LoadInt(E2_TOUCH_TESTALL_MAXOPVALVE_FRIGO_Hi+AddrOffset, K_UseRealE2);
        Touch[0].TestAll_MaxOpValve_Freezer = LoadInt(E2_TOUCH_TESTALL_MAXOPVALVE_FREEZER_Hi+AddrOffset, K_UseRealE2);
        Touch[0].TestAll_TempBatt_SP_Frigo = LoadInt(E2_TOUCH_TESTALL_TEMPBATT_SP_FRIGO_Hi+AddrOffset, K_UseRealE2);
        Touch[0].TestAll_TempBatt_SP_Freezer = LoadInt(E2_TOUCH_TESTALL_TEMPBATT_SP_FREEZER_Hi+AddrOffset, K_UseRealE2);

        Touch[0].Abil_Defrosting = LoadInt(E2_TOUCH_ABIL_DEFROSTING_Hi+AddrOffset, K_UseRealE2);
        Touch[0].Defrosting_Step_Time = LoadInt(E2_TOUCH_DEFROSTING_STEP_TIME_Hi+AddrOffset, K_UseRealE2);
        Touch[0].Defrosting_Time = LoadInt(E2_TOUCH_DEFROSTING_TIME_Hi+AddrOffset, K_UseRealE2);
        Touch[0].Defrosting_Temp_Set = LoadInt(E2_TOUCH_DEFROSTING_TEMP_SET_Hi+AddrOffset, K_UseRealE2);
        Touch[0].Dripping_Time = LoadInt(E2_TOUCH_DRIPPING_TIME_Hi+AddrOffset, K_UseRealE2);
        Touch[0].Valvola_Frozen = LoadInt(E2_TOUCH_VALVOLA_FROZEN_Hi+AddrOffset, K_UseRealE2);
        
        TimerDefrostingCycleWork.Value = LoadLong(E2_DEFROSTING_CYCLE_WORK_VALUE_MidHi+AddrOffset, K_UseRealE2);

        Touch[0].TestAll_MinOpValve_Cool = LoadInt(E2_TOUCH_TESTALL_MINOPVALVE_COOL_Hi+AddrOffset, K_UseRealE2);        
        Touch[0].TestAll_MinOpValve_Heat = LoadInt(E2_TOUCH_TESTALL_MINOPVALVE_HEAT_Hi+AddrOffset, K_UseRealE2);        
        Touch[0].TestAll_MinOpValve_Frigo = LoadInt(E2_TOUCH_TESTALL_MINOPVALVE_FRIGO_Hi+AddrOffset, K_UseRealE2);        
        Touch[0].TestAll_MinOpValve_Freezer = LoadInt(E2_TOUCH_TESTALL_MINOPVALVE_FREEZER_Hi+AddrOffset, K_UseRealE2);        
              
        Touch[0].TestAll_SuperHeat_Heat_pGain = LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN_Hi+AddrOffset, K_UseRealE2);  
        Touch[0].TestAll_SuperHeat_Heat_iGain = LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_HEAT_IGAIN_Hi+AddrOffset, K_UseRealE2);  
        Touch[0].TestAll_SuperHeat_Heat_dGain = LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_HEAT_DGAIN_Hi+AddrOffset, K_UseRealE2);  
        Touch[0].TestAll_SuperHeat_Heat_Time_Exec_PID = LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID_Hi+AddrOffset, K_UseRealE2);          

        Touch[0].TestAll_SuperHeat_Cool_pGain = LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_COOL_PGAIN_Hi+AddrOffset, K_UseRealE2);  
        Touch[0].TestAll_SuperHeat_Cool_iGain = LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_COOL_IGAIN_Hi+AddrOffset, K_UseRealE2);  
        Touch[0].TestAll_SuperHeat_Cool_dGain = LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_COOL_DGAIN_Hi+AddrOffset, K_UseRealE2);  
        Touch[0].TestAll_SuperHeat_Cool_Time_Exec_PID = LoadInt(E2_TOUCH_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID_Hi+AddrOffset, K_UseRealE2);          
        
        Touch[0].Maintenance_Flag = LoadInt(E2_TOUCH_MAINTENANCE_FLAG_Hi+AddrOffset, K_UseRealE2);
        Touch[0].Maintenance_Set_Time = LoadInt(E2_TOUCH_MAINTENANCE_SET_TIME_Hi+AddrOffset, K_UseRealE2);        

        Touch[0].SuperHeat_Cool_Min_SetPoint = LoadInt(E2_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT_Hi+AddrOffset, K_UseRealE2);        
        Touch[0].SuperHeat_Cool_Max_Value_Correct = LoadInt(E2_TOUCH_SUPERHEAT_COOL_MAX_VALUE_CORRECT_Hi+AddrOffset, K_UseRealE2);        
        Touch[0].SuperHeat_Heat_Min_SetPoint = LoadInt(E2_TOUCH_SUPERHEAT_HEAT_MIN_SETPOINT_Hi+AddrOffset, K_UseRealE2);        
        Touch[0].SuperHeat_Heat_Max_SetPoint = LoadInt(E2_TOUCH_SUPERHEAT_HEAT_MAX_SETPOINT_Hi+AddrOffset, K_UseRealE2);        

        Touch[0].Threshold_Compressor_Hi = LoadInt(E2_TOUCH_THRESHOLD_COMPRESSOR_HI_Hi+AddrOffset, K_UseRealE2);          

        Touch[0].Set_Humi = LoadInt(E2_TOUCH_SPLIT_SET_HUMI_Hi+AddrOffset, K_UseRealE2);          
        Touch[0].Set_Delta_Temp_Min_Dehumi = LoadInt(E2_TOUCH_SET_DELTA_TEMP_MIN_DEHUMI_Hi+AddrOffset, K_UseRealE2);          
        Touch[0].Set_Fan_Speed_Dehumi = LoadInt(E2_TOUCH_SET_FAN_SPEED_DEHUMI_Hi+AddrOffset, K_UseRealE2);          
        Touch[0].Set_Press_Dehumi = LoadInt(E2_TOUCH_SET_PRESS_DEHUMI_Hi+AddrOffset, K_UseRealE2);          
        Touch[0].Set_Temp_Air_Out = LoadInt(E2_TOUCH_SET_TEMP_AIR_OUT_Hi+AddrOffset, K_UseRealE2);          
        Touch[0].Set_Temp_Air_Out_Dehumi = LoadInt(E2_TOUCH_SET_TEMP_AIR_OUT_DEHUMI_Hi+AddrOffset, K_UseRealE2);          
	}
    else
    {                                                               // Se entrambi i CRC dei due settori non sono integri carico i Default
		Touch[0].HeaterPwr = K_Def_SPLIT_HeaterPwr;
        EngineBox.DoublePumpSwitchTime = K_Def_SPLIT_PumpSwitchTime;
		EngineBox.DoublePumpSelection = K_Def_SPLIT_PumpSelection;
		Touch[0].SystemDisable = K_Def_SPLIT_SystemDisable;
		Touch[0].Uta_Power = K_Def_SPLIT_Uta_Power;
		Touch[0].Uta_Mode = K_Def_SPLIT_Uta_Mode;
		Touch[0].Uta_SP_Room = K_Def_SPLIT_Uta_SP_Room;
		Touch[0].Uta_SP_Humi = K_Def_SPLIT_Uta_SP_Humi;
		Touch[0].Uta_Fan_Speed = K_Def_SPLIT_Uta_Fan_Speed;
		Touch[0].Uta_SP_Room_F = K_Def_SPLIT_Uta_SP_Room_F;
		Me.EcoModeEnable = K_Def_SPLIT_EcoModeEn;        
		Me.Power_EcoMode = K_Def_SPLIT_PwrEcoMode;
		EngineBox.Touch_Nation = K_Def_SPLIT_Nation;
        if(Me.NetBSelect)
            Touch[0].FanMode = K_DefaultFanSpeed_NetB;
        else if (Me.SyxtSSelect)
            Touch[0].FanMode = K_DefaultFanSpeed_SyxtS;
        else
            Touch[0].FanMode = K_Def_SPLIT_FanMode;    
		Touch[0].SetPoint = K_Def_SPLIT_SP;
		Touch[0].DefMode = K_Def_SPLIT_Mode;
		Touch[0].Script_Split_Pwr = K_Def_SPLIT_Pwr;
		Touch[0].PowerLimit = K_Def_PWR_Limit;
		Touch[0].Script_SetPoint_F = K_Def_SPLIT_SP_F;            
        
        Touch[0].On_Priority_Mode = K_Def_On_Priority_Mode;
        Touch[0].ValveOnDemand_Min_Percent_Val = K_Def_ValveOnDemand_Min_Percent_Val;
        Touch[0].Max_Lim_ON_Demand_Total_Score = K_Def_Max_Lim_ON_Demand_Total_Score;
        Touch[0].TimeOnDemandValveSwitch = K_Def_TimeOnDemandValveSwitch;        
        
        Touch[0].TestAll_Abil = K_Def_TOUCH_TESTALL_ABIL;
        Touch[0].TestAll_SuperHeat_SP_Cool = K_Def_TOUCH_TESTALL_SUPERHEAT_SP_COOL;
        Touch[0].TestAll_SuperHeat_SP_Heat = K_Def_TOUCH_TESTALL_SUPERHEAT_SP_HEAT;
        Touch[0].TestAll_MaxOpValve_Cool = K_Def_TOUCH_TESTALL_MAX_OPEN_VALVE_COOL;
        Touch[0].TestAll_MaxOpValve_Heat = K_Def_TOUCH_TESTALL_MAX_OPEN_VALVE_HEAT;
        Touch[0].TestAll_TempBatt_SP_Cool = K_Def_TOUCH_TESTALL_WORK_TEMP_COOL;
        Touch[0].TestAll_TempBatt_SP_Heat = K_Def_TOUCH_TESTALL_WORK_TEMP_HEAT;        
        
        Touch[0].TestALL_Frozen_Abil = K_Def_TOUCH_TESTALL_FROZEN_ABIL;        
        Touch[0].TestAll_SuperHeat_SP_Frigo = K_Def_TOUCH_TESTALL_SUPERHEAT_SP_FRIGO;
        Touch[0].TestAll_SuperHeat_SP_Freezer = K_Def_TOUCH_TESTALL_SUPERHEAT_SP_FREEZER;
        Touch[0].TestAll_MaxOpValve_Frigo = K_Def_TOUCH_TESTALL_MAXOPVALVE_FRIGO;
        Touch[0].TestAll_MaxOpValve_Freezer = K_Def_TOUCH_TESTALL_MAXOPVALVE_FREEZER;
        Touch[0].TestAll_TempBatt_SP_Frigo = K_Def_TOUCH_TESTALL_TEMPBATT_SP_FRIGO;
        Touch[0].TestAll_TempBatt_SP_Freezer = K_Def_TOUCH_TESTALL_TEMPBATT_SP_FREEZER;     
        
        Touch[0].Abil_Defrosting = K_Def_TOUCH_ABIL_DEFROSTING;
        Touch[0].Defrosting_Step_Time = K_Def_TOUCH_DEFROSTING_STEP_TIME;
        Touch[0].Defrosting_Time = K_Def_TOUCH_DEFROSTING_TIME;
        Touch[0].Defrosting_Temp_Set = K_Def_TOUCH_DEFROSTING_TEMP_SET;
        Touch[0].Dripping_Time = K_Def_TOUCH_DRIPPING_TIME;        
        Touch[0].Valvola_Frozen = K_Def_TOUCH_VALVOLA_FROZEN;        

        Touch[0].TestAll_MinOpValve_Cool = K_Def_TOUCH_TESTALL_MINOPVALVE_COOL;
        Touch[0].TestAll_MinOpValve_Heat = K_Def_TOUCH_TESTALL_MINOPVALVE_HEAT;
        Touch[0].TestAll_MinOpValve_Frigo = K_Def_TOUCH_TESTALL_MINOPVALVE_FRIGO;
        Touch[0].TestAll_MinOpValve_Freezer = K_Def_TOUCH_TESTALL_MINOPVALVE_FREEZER;
        
        TimerDefrostingCycleWork.Value = 0;                
        
        Touch[0].TestAll_SuperHeat_Heat_pGain = K_Def_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN;  
        Touch[0].TestAll_SuperHeat_Heat_iGain = K_Def_TOUCH_TESTALL_SUPERHEAT_HEAT_IGAIN;  
        Touch[0].TestAll_SuperHeat_Heat_dGain = K_Def_TOUCH_TESTALL_SUPERHEAT_HEAT_DGAIN;  
        Touch[0].TestAll_SuperHeat_Heat_Time_Exec_PID = K_Def_TOUCH_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID;        

        Touch[0].TestAll_SuperHeat_Cool_pGain = K_Def_TOUCH_TESTALL_SUPERHEAT_COOL_PGAIN;  
        Touch[0].TestAll_SuperHeat_Cool_iGain = K_Def_TOUCH_TESTALL_SUPERHEAT_COOL_IGAIN;  
        Touch[0].TestAll_SuperHeat_Cool_dGain = K_Def_TOUCH_TESTALL_SUPERHEAT_COOL_DGAIN;  
        Touch[0].TestAll_SuperHeat_Cool_Time_Exec_PID = K_Def_TOUCH_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID; 
        
        Touch[0].Maintenance_Flag = K_Def_TOUCH_MAINTENANCE_FLAG;
        Touch[0].Maintenance_Set_Time = K_Def_TOUCH_MAINTENANCE_SET_TIME;        

        Touch[0].SuperHeat_Cool_Min_SetPoint = K_Def_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT;        
        Touch[0].SuperHeat_Cool_Max_Value_Correct = K_Def_TOUCH_SUPERHEAT_COOL_MAX_VALUE_CORRECT;        
        Touch[0].SuperHeat_Heat_Min_SetPoint = K_Def_TOUCH_SUPERHEAT_HEAT_MIN_SETPOINT;        
        Touch[0].SuperHeat_Heat_Max_SetPoint = K_Def_TOUCH_SUPERHEAT_HEAT_MAX_SETPOINT;        

        Touch[0].Threshold_Compressor_Hi = K_Def_TOUCH_THRESHOLD_COMPRESSOR_HI;

        Touch[0].Set_Humi                = K_Def_TOUCH_SET_HUMI;
        Touch[0].Set_Delta_Temp_Min_Dehumi  = K_Def_TOUCH_SET_DELTA_TEMP_MIN_DEHUMI;
        Touch[0].Set_Fan_Speed_Dehumi          = K_Def_TOUCH_SET_FAN_SPEED_DEHUMI;
        Touch[0].Set_Press_Dehumi        = K_Def_TOUCH_SET_PRESS_DEHUMI;
        Touch[0].Set_Temp_Air_Out        = K_Def_TOUCH_SET_TEMP_AIR_OUT;
        Touch[0].Set_Temp_Air_Out_Dehumi = K_Def_TOUCH_SET_TEMP_AIR_OUT_DEHUMI;
    }
    
    RefreshE2_TouchValue_DoubleSector(K_UseRealE2);     // Aggiorna in EEPROM eventuali variazioni di parametri 
    LoadE2VirtualPageBuffer();                          // Carica nel buffer virtuale l'immagine del settore EEPROM utilizzato    
    
	Syncronize.SetPoint = Touch[0].SetPoint;
	Syncronize.Old.SetPoint = Touch[0].SetPoint;
	Syncronize.Engine.SetPoint = Touch[0].SetPoint;	
    DiagnosticSplit.Ready_SetPoint = Touch[0].SetPoint;

	Syncronize.FanMode = Touch[0].FanMode;
	Syncronize.Old.FanMode = Touch[0].FanMode;
	Syncronize.Engine.FanMode = Touch[0].FanMode;	
	NetB[0].FanMode = Touch[0].FanMode;	
	SyxtS[0].FanMode = Touch[0].FanMode;	
	DiagnosticSplit.Ready_FanMode = Touch[0].FanMode;	
    
	Syncronize.Script_Split_Pwr = Touch[0].Script_Split_Pwr;
	Syncronize.Old.Script_Split_Pwr = Touch[0].Script_Split_Pwr;
	Syncronize.Engine.Script_Split_Pwr = Touch[0].Script_Split_Pwr;	
    DiagnosticSplit.Ready_PowerOn = Touch[0].Script_Split_Pwr;    
	
	Syncronize.Script_SetPoint_F = Touch[0].Script_SetPoint_F;
	Syncronize.Old.Script_SetPoint_F = Touch[0].Script_SetPoint_F;
	Syncronize.Engine.Script_SetPoint_F = Touch[0].Script_SetPoint_F;	
	DiagnosticSplit.Ready_SetPoint_F = Touch[0].Script_SetPoint_F;	

	Syncronize.Uta_Mode = Touch[0].Uta_Mode;
	Syncronize.Old.Uta_Mode = Touch[0].Uta_Mode;
	Syncronize.Engine.Uta_Mode = Touch[0].Uta_Mode;	
	DiagnosticSplit.Ready_UtaMode = Touch[0].Uta_Mode;	
    
    Syncronize.DefMode = Touch[0].DefMode;
    Syncronize.Old.DefMode = Touch[0].DefMode;
    Syncronize.Engine.DefMode = Touch[0].DefMode;

    Syncronize.SystemDisable = Touch[0].SystemDisable;
    Syncronize.Old.SystemDisable = Touch[0].SystemDisable;
    Syncronize.Engine.SystemDisable = Touch[0].SystemDisable;
    EngineBox.SystemDisable = Touch[0].SystemDisable;

    Syncronize.EcoModeEnable = Me.EcoModeEnable;
    Syncronize.Old.EcoModeEnable = Me.EcoModeEnable;
    Syncronize.Engine.EcoModeEnable = Me.EcoModeEnable;
    
    Syncronize.Power_EcoMode = Me.Power_EcoMode;
    Syncronize.Old.Power_EcoMode = Me.Power_EcoMode;
    Syncronize.Engine.Power_EcoMode = Me.Power_EcoMode;     
    
	Syncronize.HeaterPwr = Touch[0].HeaterPwr;
	Syncronize.Old.HeaterPwr = Touch[0].HeaterPwr;
	Syncronize.Engine.HeaterPwr = Touch[0].HeaterPwr;	
}


unsigned int Generate_CRC_E2(int StartE2Data, int EndE2Data, int UseVirtualBuffer)
{
	unsigned int CRC_16=0xFFFF;
	unsigned int i = 0;
	unsigned char e2val;
	
	
	for(i=StartE2Data; i <= EndE2Data; i++)
	{
        if(UseVirtualBuffer)
            e2val = VirtualE2PageBuff[i];
        else
            e2val = ReadRegI2C(EEPROM_Add, i);
		CRC_16 = Mod_BaseCRC16Sec(CRC_16, e2val);
	}
	return CRC_16;
}



void CheckValueRange(void)
{
/*	
	CheckVarValue(&Syncronize.Script_Split_Pwr, K_LimLo_SPLIT_Pwr, K_LimHi_SPLIT_Pwr, K_Def_SPLIT_Pwr);	
	CheckVarValue(&Syncronize.SetPoint, K_LimLo_SPLIT_SP, K_LimHi_SPLIT_SP, K_Def_SPLIT_SP);
	CheckVarValue(&Syncronize.FanMode, K_LimLo_SPLIT_FanMode, K_LimHi_SPLIT_FanMode, K_Def_SPLIT_FanMode);
	CheckVarValue(&Syncronize.Script_SetPoint_F, K_LimLo_SPLIT_SP_F, K_LimHi_SPLIT_SP_F, K_Def_SPLIT_SP_F);	
*/

    
if(Me.NetBSelect)
{
    Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
 
    CheckVarValueNewNetB(&Touch[0].Script_Split_Pwr, &Syncronize.Script_Split_Pwr, K_LimLo_SPLIT_Pwr, K_LimHi_SPLIT_Pwr, K_Def_SPLIT_Pwr, REG_NETB_IMP_VEL_FAN, E2_SPLIT_Pwr_Hi);	
    CheckVarValueSetPNetB(&Touch[0].SetPoint, &Syncronize.SetPoint, K_LimLo_SPLIT_SP, K_LimHi_SPLIT_SP, K_Def_SPLIT_SP, REG_NETB_TEMP_SETP_10, E2_SPLIT_SP_Hi);
	CheckVarValueNewNetB(&Touch[0].FanMode, &Syncronize.FanMode, K_LimLo_SPLIT_FanMode, 4, 1, REG_NETB_IMP_VEL_FAN, E2_SPLIT_FanMode_Hi);
}
else if(Me.SyxtSSelect) 
{
    Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud

	CheckVarValueOnOffSyxtS(&Touch[0].Script_Split_Pwr, &Syncronize.Script_Split_Pwr, K_LimLo_SPLIT_Pwr, K_LimHi_SPLIT_Pwr, K_Def_SPLIT_Pwr, COIL_SYXTS_OFF_MODE_OVRR, E2_SPLIT_Pwr_Hi);	
    if((SyxtS[0].FwVer_Upper_423==1) && (EngineBox.Touch_Nation==1))
        CheckVarValueSetPSyxtS(&Touch[0].Script_SetPoint_F, &Syncronize.Script_SetPoint_F, K_LimLo_SPLIT_SP_F, K_LimHi_SPLIT_SP_F, K_Def_SPLIT_SP_F, REGH_SYXTS_NOM_SETP, E2_SPLIT_SP_F_Hi);
    else
        CheckVarValueSetPSyxtS(&Touch[0].SetPoint, &Syncronize.SetPoint, K_LimLo_SPLIT_SP, K_LimHi_SPLIT_SP, K_Def_SPLIT_SP, REGH_SYXTS_NOM_SETP, E2_SPLIT_SP_Hi);
    CheckVarValueNewSyxtS(&Touch[0].FanMode, &Syncronize.FanMode, K_LimLo_SPLIT_FanMode, Me.MaxFanVentil_Mode, Me.MaxFanVentil_Mode, REGH_SYXTS_FAN_SPEED_OVRR_ADV, E2_SPLIT_FanMode_Hi);
}
else
{
    if(Me.EnableHiSpeedModBusTouch_Sec==1)        
        Change_ProtocolBusSlave(K_ModBus_Baud115200, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
    else
        Change_ProtocolBusSlave(K_ModBus_Baud57600, K_ModBus_Panel);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud  
    
	CheckVarValueNew(&Touch[0].Script_Split_Pwr, &Syncronize.Script_Split_Pwr, K_LimLo_SPLIT_Pwr, K_LimHi_SPLIT_Pwr, K_Def_SPLIT_Pwr, REG_TOUCH_PWR, E2_SPLIT_Pwr_Hi);	
    CheckVarValueNew(&Touch[0].SetPoint, &Syncronize.SetPoint, K_LimLo_SPLIT_SP, K_LimHi_SPLIT_SP, K_Def_SPLIT_SP, REG_TOUCH_SP_ROOM, E2_SPLIT_SP_Hi);
    CheckVarValueNew(&Touch[0].FanMode, &Syncronize.FanMode, K_LimLo_SPLIT_FanMode, Me.MaxFanVentil_Mode, Me.MaxFanVentil_Mode, REG_TOUCH_FAN_SPEED, E2_SPLIT_FanMode_Hi);
}
    
    
	CheckVarValueNew(&Touch[0].Script_SetPoint_F, &Syncronize.Script_SetPoint_F, K_LimLo_SPLIT_SP_F, K_LimHi_SPLIT_SP_F, K_Def_SPLIT_SP_F, REG_TOUCH_SP_ROOM_F, E2_SPLIT_SP_F_Hi);	
#if(K_Heater_Abil==1)   
	CheckVarValueNew(&Touch[0].HeaterPwr, &Syncronize.HeaterPwr, K_LimLo_SPLIT_HeaterPwr, K_LimHi_SPLIT_HeaterPwr, K_Def_SPLIT_HeaterPwr, REG_TOUCH_HEATER_PWR, E2_SPLIT_Heater_Pwr_Hi);	
#endif
    
    CheckUtaModeValue();	
	if(Me.My_Address==k_Split_Master_Add)
	{
		CheckVarValue(&Syncronize.SystemDisable, K_LimLo_SPLIT_SystemDisable, K_LimHi_SPLIT_SystemDisable, K_Def_SPLIT_SystemDisable, REG_TOUCH_SPLIT_SYSTEM_DISABLE);
		CheckVarValue(&Syncronize.Uta_Power, K_LimLo_SPLIT_Uta_Power, K_LimHi_SPLIT_Uta_Power, K_Def_SPLIT_Uta_Power, REG_TOUCH_UTA_POWER);
		//CheckVarValue(&Syncronize.Uta_Mode, K_LimLo_SPLIT_Uta_Mode, K_LimHi_SPLIT_Uta_Mode, K_Def_SPLIT_Uta_Mode, REG_TOUCH_UTA_MODE);
		CheckVarValue(&Syncronize.Uta_SP_Room, K_LimLo_SPLIT_Uta_SP_Room, K_LimHi_SPLIT_Uta_SP_Room, K_Def_SPLIT_Uta_SP_Room, REG_TOUCH_UTA_SP_ROOM);
		CheckVarValue(&Syncronize.Uta_SP_Humi, K_LimLo_SPLIT_Uta_SP_Humi, K_LimHi_SPLIT_Uta_SP_Humi, K_Def_SPLIT_Uta_SP_Humi, REG_TOUCH_UTA_SP_HUMI);
		CheckVarValue(&Syncronize.Uta_Fan_Speed, K_LimLo_SPLIT_Uta_Fan_Speed, K_LimHi_SPLIT_Uta_Fan_Speed, K_Def_SPLIT_Uta_Fan_Speed, REG_TOUCH_UTA_FAN_SPEED);
		CheckVarValue(&Syncronize.Uta_SP_Room_F, K_LimLo_SPLIT_Uta_SP_Room_F, K_LimHi_SPLIT_Uta_SP_Room_F, K_Def_SPLIT_Uta_SP_Room_F, REG_TOUCH_UTA_SP_ROOM_F);
		CheckVarValue(&Syncronize.EcoModeEnable, K_LimLo_SPLIT_EcoModeEn, K_LimHi_SPLIT_EcoModeEn, K_Def_SPLIT_EcoModeEn, REG_TOUCH_ECOMODE_ENABLE);

		CheckVarValue(&Syncronize.Power_EcoMode, K_LimLo_SPLIT_PwrEcoMode, K_LimHi_SPLIT_PwrEcoMode, K_Def_SPLIT_PwrEcoMode, REG_TOUCH_POWER_ECOMODE);
		//CheckVarValue(&EngineBox.Touch_Nation, K_LimLo_SPLIT_Nation, K_LimHi_SPLIT_Nation, K_Def_SPLIT_Nation);
		CheckDefModeValue();
		CheckPowerLimitValue();		
		CheckVarValue(&EngineBox.DoublePumpSwitchTime, K_LimLo_SPLIT_PumpSwitchTime, K_LimHi_SPLIT_PumpSwitchTime, K_Def_SPLIT_PumpSwitchTime, REG_TOUCH_DOUBLE_PUMP_SWITCH_TIME);
        CheckPumpSelectionValue();
                
        CheckVarValue(&Touch[0].On_Priority_Mode, K_LimLo_SPLIT_On_Priority_Mode, K_LimHi_SPLIT_On_Priority_Mode, K_Def_On_Priority_Mode, REG_TOUCH_ON_PRIORITY_MODE);
        CheckVarValue(&Touch[0].ValveOnDemand_Min_Percent_Val, K_LimLo_SPLIT_ValveOnDemand_Min_Percent_Val, K_LimHi_SPLIT_ValveOnDemand_Min_Percent_Val, K_Def_ValveOnDemand_Min_Percent_Val, REG_TOUCH_VALVE_ONDEMAND_MIN_PERC_VAL);
        CheckVarValue(&Touch[0].Max_Lim_ON_Demand_Total_Score, K_LimLo_SPLIT_Max_Lim_ON_Demand_Total_Score, K_LimHi_SPLIT_Max_Lim_ON_Demand_Total_Score, K_Def_Max_Lim_ON_Demand_Total_Score, REG_TOUCH_MAX_LIM_ONDEMAND_TOTAL_SCORE);
        CheckVarValue(&Touch[0].TimeOnDemandValveSwitch, K_LimLo_SPLIT_TimeOnDemandValveSwitch, K_LimHi_SPLIT_TimeOnDemandValveSwitch, K_Def_TimeOnDemandValveSwitch, REG_TOUCH_TIME_ONDEMAND_VALVE_SWITCH);

		//CheckVarValue(&Touch[0].TestAll_Abil, K_LimLo_TOUCH_TESTALL_ABIL, K_LimHi_TOUCH_TESTALL_ABIL, K_Def_TOUCH_TESTALL_ABIL, REG_TOUCH_TESTALL_ABIL);  //TOLTO PERCHè USIAMO TUTTI I BIT
		CheckVarValue(&Touch[0].TestAll_SuperHeat_SP_Cool, K_LimLo_TOUCH_TESTALL_SUPERHEAT_SP_COOL, K_LimHi_TOUCH_TESTALL_SUPERHEAT_SP_COOL, K_Def_TOUCH_TESTALL_SUPERHEAT_SP_COOL, REG_TOUCH_TESTALL_SUPERHEAT_SP_COOL);
		CheckVarValue(&Touch[0].TestAll_SuperHeat_SP_Heat, K_LimLo_TOUCH_TESTALL_SUPERHEAT_SP_HEAT, K_LimHi_TOUCH_TESTALL_SUPERHEAT_SP_HEAT, K_Def_TOUCH_TESTALL_SUPERHEAT_SP_HEAT, REG_TOUCH_TESTALL_SUPERHEAT_SP_HEAT);
		CheckVarValue(&Touch[0].TestAll_MaxOpValve_Cool, K_LimLo_TOUCH_TESTALL_MAX_OPEN_VALVE_COOL, K_LimHi_TOUCH_TESTALL_MAX_OPEN_VALVE_COOL, K_Def_TOUCH_TESTALL_MAX_OPEN_VALVE_COOL, REG_TOUCH_TESTALL_MAX_OPEN_VALVE_COOL);
		CheckVarValue(&Touch[0].TestAll_MaxOpValve_Heat, K_LimLo_TOUCH_TESTALL_MAX_OPEN_VALVE_HEAT, K_LimHi_TOUCH_TESTALL_MAX_OPEN_VALVE_HEAT, K_Def_TOUCH_TESTALL_MAX_OPEN_VALVE_HEAT, REG_TOUCH_TESTALL_MAX_OPEN_VALVE_HEAT);
		CheckVarValue(&Touch[0].TestAll_TempBatt_SP_Cool, K_LimLo_TOUCH_TESTALL_WORK_TEMP_COOL, K_LimHi_TOUCH_TESTALL_WORK_TEMP_COOL, K_Def_TOUCH_TESTALL_WORK_TEMP_COOL, REG_TOUCH_TESTALL_WORK_TEMP_COOL);
		CheckVarValue(&Touch[0].TestAll_TempBatt_SP_Heat, K_LimLo_TOUCH_TESTALL_WORK_TEMP_HEAT, K_LimHi_TOUCH_TESTALL_WORK_TEMP_HEAT, K_Def_TOUCH_TESTALL_WORK_TEMP_HEAT, REG_TOUCH_TESTALL_WORK_TEMP_HEAT);
        
        
        CheckVarValue(&Touch[0].TestALL_Frozen_Abil,          K_LimLo_TOUCH_TESTALL_FROZEN_ABIL         , K_LimHi_TOUCH_TESTALL_FROZEN_ABIL         , K_Def_TOUCH_TESTALL_FROZEN_ABIL         , REG_TOUCH_TESTALL_FROZEN_ABIL           );
        CheckVarValue(&Touch[0].TestAll_SuperHeat_SP_Frigo,   K_LimLo_TOUCH_TESTALL_SUPERHEAT_SP_FRIGO  , K_LimHi_TOUCH_TESTALL_SUPERHEAT_SP_FRIGO  , K_Def_TOUCH_TESTALL_SUPERHEAT_SP_FRIGO  , REG_TOUCH_TESTALL_SUPERHEAT_SP_FRIGO    );
        CheckVarValue(&Touch[0].TestAll_SuperHeat_SP_Freezer, K_LimLo_TOUCH_TESTALL_SUPERHEAT_SP_FREEZER, K_LimHi_TOUCH_TESTALL_SUPERHEAT_SP_FREEZER, K_Def_TOUCH_TESTALL_SUPERHEAT_SP_FREEZER, REG_TOUCH_TESTALL_SUPERHEAT_SP_FREEZER  );
        CheckVarValue(&Touch[0].TestAll_MaxOpValve_Frigo	, K_LimLo_TOUCH_TESTALL_MAX_OPEN_VALVE_FRIGO    , K_LimHi_TOUCH_TESTALL_MAX_OPEN_VALVE_FRIGO    , K_Def_TOUCH_TESTALL_MAXOPVALVE_FRIGO    , REG_TOUCH_TESTALL_MAX_OPEN_VALVE_FRIGO  );
        CheckVarValue(&Touch[0].TestAll_MaxOpValve_Freezer	, K_LimLo_TOUCH_TESTALL_MAX_OPEN_VALVE_FREEZER  , K_LimHi_TOUCH_TESTALL_MAX_OPEN_VALVE_FREEZER  , K_Def_TOUCH_TESTALL_MAXOPVALVE_FREEZER  , REG_TOUCH_TESTALL_MAX_OPEN_VALVE_FREEZER);
        CheckVarValue(&Touch[0].TestAll_TempBatt_SP_Frigo	, K_LimLo_TOUCH_TESTALL_TEMPBATT_SP_FRIGO   , K_LimHi_TOUCH_TESTALL_TEMPBATT_SP_FRIGO   , K_Def_TOUCH_TESTALL_TEMPBATT_SP_FRIGO   , REG_TOUCH_TESTALL_WORK_TEMP_FRIGO       );
        CheckVarValue(&Touch[0].TestAll_TempBatt_SP_Freezer	, K_LimLo_TOUCH_TESTALL_TEMPBATT_SP_FREEZER , K_LimHi_TOUCH_TESTALL_TEMPBATT_SP_FREEZER , K_Def_TOUCH_TESTALL_TEMPBATT_SP_FREEZER , REG_TOUCH_TESTALL_WORK_TEMP_FREEZER     );


        CheckVarValue(&Touch[0].Abil_Defrosting	,       K_LimLo_TOUCH_ABIL_DEFROSTING     , K_LimHi_TOUCH_ABIL_DEFROSTING     , K_Def_TOUCH_ABIL_DEFROSTING     , REG_TOUCH_ABIL_DEFROSTING     );
        CheckVarValue(&Touch[0].Defrosting_Step_Time,   K_LimLo_TOUCH_DEFROSTING_STEP_TIME, K_LimHi_TOUCH_DEFROSTING_STEP_TIME, K_Def_TOUCH_DEFROSTING_STEP_TIME, REG_TOUCH_DEFROSTING_STEP_TIME);
        CheckVarValue(&Touch[0].Defrosting_Time	,       K_LimLo_TOUCH_DEFROSTING_TIME     , K_LimHi_TOUCH_DEFROSTING_TIME     , K_Def_TOUCH_DEFROSTING_TIME     , REG_TOUCH_DEFROSTING_TIME     );
        CheckVarValue(&Touch[0].Defrosting_Temp_Set	,   K_LimLo_TOUCH_DEFROSTING_TEMP_SET , K_LimHi_TOUCH_DEFROSTING_TEMP_SET , K_Def_TOUCH_DEFROSTING_TEMP_SET , REG_TOUCH_DEFROSTING_TEMP_SET );
        CheckVarValue(&Touch[0].Dripping_Time		,   K_LimLo_TOUCH_DRIPPING_TIME       , K_LimHi_TOUCH_DRIPPING_TIME       , K_Def_TOUCH_DRIPPING_TIME       , REG_TOUCH_DRIPPING_TIME       );
        CheckVarValue(&Touch[0].Valvola_Frozen		,   K_LimLo_TOUCH_VALVOLA_FROZEN      , K_LimHi_TOUCH_VALVOLA_FROZEN      , K_Def_TOUCH_VALVOLA_FROZEN      , REG_TOUCH_VALVOLA_FROZEN      );
        

        CheckVarValue(&Touch[0].TestAll_MinOpValve_Cool	    , K_LimLo_TOUCH_TESTALL_MIN_OPEN_VALVE_COOL    , K_LimHi_TOUCH_TESTALL_MIN_OPEN_VALVE_COOL    , K_Def_TOUCH_TESTALL_MINOPVALVE_COOL    , REG_TOUCH_TESTALL_MIN_OPEN_VALVE_COOL    );
        CheckVarValue(&Touch[0].TestAll_MinOpValve_Heat	    , K_LimLo_TOUCH_TESTALL_MIN_OPEN_VALVE_HEAT    , K_LimHi_TOUCH_TESTALL_MIN_OPEN_VALVE_HEAT    , K_Def_TOUCH_TESTALL_MINOPVALVE_HEAT    , REG_TOUCH_TESTALL_MIN_OPEN_VALVE_HEAT    );
        CheckVarValue(&Touch[0].TestAll_MinOpValve_Frigo	, K_LimLo_TOUCH_TESTALL_MIN_OPEN_VALVE_FRIGO   , K_LimHi_TOUCH_TESTALL_MIN_OPEN_VALVE_FRIGO   , K_Def_TOUCH_TESTALL_MINOPVALVE_FRIGO   , REG_TOUCH_TESTALL_MIN_OPEN_VALVE_FRIGO   );
        CheckVarValue(&Touch[0].TestAll_MinOpValve_Freezer	, K_LimLo_TOUCH_TESTALL_MIN_OPEN_VALVE_FREEZER , K_LimHi_TOUCH_TESTALL_MIN_OPEN_VALVE_FREEZER , K_Def_TOUCH_TESTALL_MINOPVALVE_FREEZER , REG_TOUCH_TESTALL_MIN_OPEN_VALVE_FREEZER );

		CheckVarValue(&Touch[0].TestAll_SuperHeat_Heat_pGain, K_LimLo_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN, K_LimHi_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN, K_Def_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN, REG_TOUCH_TESTALL_SUPERHEAT_HEAT_PGAIN);
		CheckVarValue(&Touch[0].TestAll_SuperHeat_Heat_iGain, K_LimLo_TOUCH_TESTALL_SUPERHEAT_HEAT_IGAIN, K_LimHi_TOUCH_TESTALL_SUPERHEAT_HEAT_IGAIN, K_Def_TOUCH_TESTALL_SUPERHEAT_HEAT_IGAIN, REG_TOUCH_TESTALL_SUPERHEAT_HEAT_IGAIN);
		CheckVarValue(&Touch[0].TestAll_SuperHeat_Heat_dGain, K_LimLo_TOUCH_TESTALL_SUPERHEAT_HEAT_DGAIN, K_LimHi_TOUCH_TESTALL_SUPERHEAT_HEAT_DGAIN, K_Def_TOUCH_TESTALL_SUPERHEAT_HEAT_DGAIN, REG_TOUCH_TESTALL_SUPERHEAT_HEAT_DGAIN);
		CheckVarValue(&Touch[0].TestAll_SuperHeat_Heat_Time_Exec_PID, K_LimLo_TOUCH_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID, K_LimHi_TOUCH_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID, K_Def_TOUCH_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID, REG_TOUCH_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID);

		CheckVarValue(&Touch[0].TestAll_SuperHeat_Cool_pGain, K_LimLo_TOUCH_TESTALL_SUPERHEAT_COOL_PGAIN, K_LimHi_TOUCH_TESTALL_SUPERHEAT_COOL_PGAIN, K_Def_TOUCH_TESTALL_SUPERHEAT_COOL_PGAIN, REG_TOUCH_TESTALL_SUPERHEAT_COOL_PGAIN);
		CheckVarValue(&Touch[0].TestAll_SuperHeat_Cool_iGain, K_LimLo_TOUCH_TESTALL_SUPERHEAT_COOL_IGAIN, K_LimHi_TOUCH_TESTALL_SUPERHEAT_COOL_IGAIN, K_Def_TOUCH_TESTALL_SUPERHEAT_COOL_IGAIN, REG_TOUCH_TESTALL_SUPERHEAT_COOL_IGAIN);
		CheckVarValue(&Touch[0].TestAll_SuperHeat_Cool_dGain, K_LimLo_TOUCH_TESTALL_SUPERHEAT_COOL_DGAIN, K_LimHi_TOUCH_TESTALL_SUPERHEAT_COOL_DGAIN, K_Def_TOUCH_TESTALL_SUPERHEAT_COOL_DGAIN, REG_TOUCH_TESTALL_SUPERHEAT_COOL_DGAIN);
		CheckVarValue(&Touch[0].TestAll_SuperHeat_Cool_Time_Exec_PID, K_LimLo_TOUCH_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID, K_LimHi_TOUCH_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID, K_Def_TOUCH_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID, REG_TOUCH_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID);

		CheckVarValue(&Touch[0].Maintenance_Flag, K_LimLo_TOUCH_MAINTENANCE_FLAG, K_LimHi_TOUCH_MAINTENANCE_FLAG, K_Def_TOUCH_MAINTENANCE_FLAG, REG_TOUCH_MAINTENANCE_FLAG);
		CheckVarValue(&Touch[0].Maintenance_Set_Time, K_LimLo_TOUCH_MAINTENANCE_SET_TIME, K_LimHi_TOUCH_MAINTENANCE_SET_TIME, K_Def_TOUCH_MAINTENANCE_SET_TIME, REG_TOUCH_MAINTENANCE_SET_TIME);

        CheckVarValue(&Reinit.ToEngBox.TrigInit	, K_LimLo_TOUCH_TRIGGER_INIT , K_LimHi_TOUCH_TRIGGER_INIT , K_Def_TOUCH_TRIGGER_INIT , REG_TOUCH_TRIGGER_INIT );

		CheckVarValue(&Touch[0].SuperHeat_Cool_Min_SetPoint, K_LimLo_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT, K_LimHi_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT, K_Def_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT, REG_TOUCH_SUPERHEAT_COOL_MIN_SETPOINT);
		CheckVarValue(&Touch[0].SuperHeat_Cool_Max_Value_Correct, K_LimLo_TOUCH_SUPERHEAT_COOL_MAX_VALUE_CORRECT, K_LimHi_TOUCH_SUPERHEAT_COOL_MAX_VALUE_CORRECT, K_Def_TOUCH_SUPERHEAT_COOL_MAX_VALUE_CORRECT, REG_TOUCH_SUPERHEAT_COOL_MAX_VALUE_CORRECT);
		CheckVarValue(&Touch[0].SuperHeat_Heat_Min_SetPoint, K_LimLo_TOUCH_SUPERHEAT_HEAT_MIN_SETPOINT, K_LimHi_TOUCH_SUPERHEAT_HEAT_MIN_SETPOINT, K_Def_TOUCH_SUPERHEAT_HEAT_MIN_SETPOINT, REG_TOUCH_SUPERHEAT_HEAT_MIN_SETPOINT);
		CheckVarValue(&Touch[0].SuperHeat_Heat_Max_SetPoint, K_LimLo_TOUCH_SUPERHEAT_HEAT_MAX_SETPOINT, K_LimHi_TOUCH_SUPERHEAT_HEAT_MAX_SETPOINT, K_Def_TOUCH_SUPERHEAT_HEAT_MAX_SETPOINT, REG_TOUCH_SUPERHEAT_HEAT_MAX_SETPOINT);

		CheckVarValue(&Touch[0].Threshold_Compressor_Hi, K_LimLo_TOUCH_THRESHOLD_COMPRESSOR_HI, K_LimHi_TOUCH_THRESHOLD_COMPRESSOR_HI, K_Def_TOUCH_THRESHOLD_COMPRESSOR_HI, REG_TOUCH_THRESHOLD_COMPRESSOR_HI);

		CheckVarValue(&Touch[0].Set_Humi, K_LimLo_TOUCH_SET_HUMI, K_LimHi_TOUCH_SET_HUMI, K_Def_TOUCH_SET_HUMI, REG_TOUCH_SET_HUMI);
		CheckVarValue(&Touch[0].Set_Delta_Temp_Min_Dehumi, K_LimLo_TOUCH_SET_DELTA_TEMP_MIN_DEHUMI, K_LimHi_TOUCH_SET_DELTA_TEMP_MIN_DEHUMI, K_Def_TOUCH_SET_DELTA_TEMP_MIN_DEHUMI, REG_TOUCH_SET_DELTA_TEMP_MIN_DEHUMI);
		CheckVarValue(&Touch[0].Set_Fan_Speed_Dehumi, K_LimLo_TOUCH_SET_FAN_SPEED_DEHUMI, K_LimHi_TOUCH_SET_FAN_SPEED_DEHUMI, K_Def_TOUCH_SET_FAN_SPEED_DEHUMI, REG_TOUCH_SET_FAN_SPEED_DEHUMI);
		CheckVarValue(&Touch[0].Set_Press_Dehumi, K_LimLo_TOUCH_SET_PRESS_DEHUMI, K_LimHi_TOUCH_SET_PRESS_DEHUMI, K_Def_TOUCH_SET_PRESS_DEHUMI, REG_TOUCH_SET_PRESS_DEHUMI);
		CheckVarValue(&Touch[0].Set_Temp_Air_Out, K_LimLo_TOUCH_SET_TEMP_AIR_OUT, K_LimHi_TOUCH_SET_TEMP_AIR_OUT, K_Def_TOUCH_SET_TEMP_AIR_OUT, REG_TOUCH_SET_TEMP_AIR_OUT);
		CheckVarValue(&Touch[0].Set_Temp_Air_Out_Dehumi, K_LimLo_TOUCH_SET_TEMP_AIR_OUT_DEHUMI, K_LimHi_TOUCH_SET_TEMP_AIR_OUT_DEHUMI, K_Def_TOUCH_SET_TEMP_AIR_OUT_DEHUMI, REG_TOUCH_SET_TEMP_AIR_OUT_DEHUMI);
	}
}

void CheckVarValue(volatile int * Var, int LimLo, int LimHi, int DefVal, int RegVal)
{
	if(*Var > LimHi || *Var < LimLo)
    {   
		*Var = DefVal;
        if(Touch[0].OnLine)
            WriteModRegSec (Add_TouchRoom, RegVal, DefVal);	// Scrivo valore arrivato dal Master
    }
}

void CheckVarValueNew(volatile int * TouchVar, volatile int * Var, int LimLo, int LimHi, int DefVal, int RegVal, int EEpAdd)
{
	if(*TouchVar > LimHi || *TouchVar < LimLo)
    {        
        int EepromVal = LoadIntVirt(EEpAdd+Me.EEPROM_SectorPage);
        if(EepromVal > LimHi || EepromVal < LimLo)
            EepromVal = DefVal;
        *Var = EepromVal;

        if(Touch[0].OnLine)
            WriteModRegSec (Add_TouchRoom, RegVal, EepromVal);	// Scrivo valore arrivato dal Master
        //else
            *TouchVar = EepromVal; 
    }
}

void CheckVarValueNewNetB(volatile int * TouchVar, volatile int * Var, int LimLo, int LimHi, int DefVal, int RegVal, int EEpAdd)
{
	if(*TouchVar > LimHi || *TouchVar < LimLo)
    {
        int EepromVal = LoadIntVirt(EEpAdd+Me.EEPROM_SectorPage);
        if(EepromVal > LimHi || EepromVal < LimLo)
            EepromVal = DefVal;
        *Var = EepromVal;

        if(NetB[0].OnLine)
            WriteModRegSecNetB (Add_NetBRoom, RegVal, EepromVal);	// Scrivo valore arrivato dal Master 
        //else
            *TouchVar = EepromVal;
    }
}

void CheckVarValueSetPNetB(volatile int * TouchVar, volatile int * Var, int LimLo, int LimHi, int DefVal, int RegVal, int EEpAdd)
{
	if(*TouchVar > LimHi || *TouchVar < LimLo)
    {
        int EepromVal = LoadIntVirt(EEpAdd+Me.EEPROM_SectorPage);
        if(EepromVal > LimHi || EepromVal < LimLo)
            EepromVal = DefVal;
        *Var = EepromVal;

        if(NetB[0].OnLine)
            WriteModRegSecNetB (Add_NetBRoom, RegVal, EepromVal/10);	// Scrivo valore arrivato dal Master 
        //else
            *TouchVar = EepromVal;
    }
}

void CheckVarValueNewSyxtS(volatile int * TouchVar, volatile int * Var, int LimLo, int LimHi, int DefVal, int RegVal, int EEpAdd)    
{
	if(*TouchVar > LimHi || *TouchVar < LimLo)
    {
        int EepromVal = LoadIntVirt(EEpAdd+Me.EEPROM_SectorPage);
        if(EepromVal > LimHi || EepromVal < LimLo)
            EepromVal = DefVal;
        *Var = EepromVal;

        if(SyxtS[0].OnLine)
            WriteModRegSecSyxtS (Add_SyxtSRoom, RegVal, EepromVal); 
        //else
            *TouchVar = EepromVal; 
    }
}

void CheckVarValueSetPSyxtS(volatile int * TouchVar, volatile int * Var, int LimLo, int LimHi, int DefVal, int RegVal, int EEpAdd)    
{
	if(*TouchVar > LimHi || *TouchVar < LimLo)
    {
        int EepromVal = LoadIntVirt(EEpAdd+Me.EEPROM_SectorPage);
        if(EepromVal > LimHi || EepromVal < LimLo)
            EepromVal = DefVal;
        *Var = EepromVal;

        if(SyxtS[0].OnLine)
            WriteModRegSecSyxtS (Add_SyxtSRoom, RegVal, EepromVal/10); 
        //else
            *TouchVar = EepromVal; 
    }
}

void CheckVarValueOnOffSyxtS(volatile int * TouchVar, volatile int * Var, int LimLo, int LimHi, int DefVal, int RegVal, int EEpAdd)    
{
	if(*TouchVar > LimHi || *TouchVar < LimLo)
    {
        int EepromVal = LoadIntVirt(EEpAdd+Me.EEPROM_SectorPage);
        if(EepromVal > LimHi || EepromVal < LimLo)
            EepromVal = DefVal;
        *Var = EepromVal;
        
        if(SyxtS[0].OnLine)
            WriteCoilModRegSecSyxtS (Add_SyxtSRoom, RegVal, ~EepromVal & 0x0001); 
        //else
            *TouchVar = EepromVal; 
    }
}

void CheckDefModeValue(void)
{
	if(Syncronize.DefMode != CoreRiscaldamento && Syncronize.DefMode != CoreRaffrescamento && Syncronize.DefMode != CoreVentilazione && Syncronize.DefMode != CoreAuto)
    {
		Syncronize.DefMode = CoreVentilazione;
        if(Touch[0].OnLine)
            WriteModRegSec (Add_TouchRoom, REG_TOUCH_MODE, Syncronize.DefMode);	// Scrivo valore arrivato dal Master
    }
}

void CheckUtaModeValue(void)
{
	if(Syncronize.Uta_Mode != CoreVentilazione && Syncronize.Uta_Mode != CoreUtaAuto)
    {
		Syncronize.Uta_Mode = CoreVentilazione;
        if(Touch[0].OnLine)
            WriteModRegSec (Add_TouchRoom, REG_TOUCH_UTA_MODE, Syncronize.Uta_Mode);	// Scrivo valore arrivato dal Master
    }
}

void CheckPowerLimitValue(void)
{
	if(Touch[0].PowerLimit > K_LimHi_PWR_Limit || Touch[0].PowerLimit < K_LimLo_PWR_Limit)
    {
		Touch[0].PowerLimit = K_Def_PWR_Limit;
	
        if(Touch[0].OnLine)
    		WriteModRegSec (Add_TouchRoom, REG_TOUCH_MAX_PWR_LIMIT, Touch[0].PowerLimit);	
    }
}

void CheckPumpSelectionValue(void)
{
	if(EngineBox.DoublePumpSelection != PumpSelection_Auto && EngineBox.DoublePumpSelection != PumpSelection_1 && EngineBox.DoublePumpSelection != PumpSelection_2)
		EngineBox.DoublePumpSelection = PumpSelection_Auto;
}


//--------------------------------------------------------------------------------------------------------------------------------
// Esegue un test di Scrittura e rilettura dato sulla prima locazione di tutti gli 8 banchi EEPROM
// Se il test fallisce viene segnalato con il flag "Me.Error.EEPROM_ReadWrite == 1"
//--------------------------------------------------------------------------------------------------------------------------------
 void CheckEEPROM_ReadWrite(void)
 {
    int MagicNumber = K_Test_EEPROM_ReadWrite;
    int Test[8];
    int Passed=0;
    int Retry=10;
    
    while(!Passed && Retry>0)
    {
        StoreInt(MagicNumber, Test_EEPROM_ReadWrite_B0);
        StoreInt(MagicNumber, Test_EEPROM_ReadWrite_B1);
        StoreInt(MagicNumber, Test_EEPROM_ReadWrite_B2);
        StoreInt(MagicNumber, Test_EEPROM_ReadWrite_B3);
        StoreInt(MagicNumber, Test_EEPROM_ReadWrite_B4);
        StoreInt(MagicNumber, Test_EEPROM_ReadWrite_B5);
        StoreInt(MagicNumber, Test_EEPROM_ReadWrite_B6);
        StoreInt(MagicNumber, Test_EEPROM_ReadWrite_B7);
        Test[0] = LoadInt(Test_EEPROM_ReadWrite_B0, K_UseRealE2);
        Test[1] = LoadInt(Test_EEPROM_ReadWrite_B1, K_UseRealE2);
        Test[2] = LoadInt(Test_EEPROM_ReadWrite_B2, K_UseRealE2);
        Test[3] = LoadInt(Test_EEPROM_ReadWrite_B3, K_UseRealE2);
        Test[4] = LoadInt(Test_EEPROM_ReadWrite_B4, K_UseRealE2);
        Test[5] = LoadInt(Test_EEPROM_ReadWrite_B5, K_UseRealE2);
        Test[6] = LoadInt(Test_EEPROM_ReadWrite_B6, K_UseRealE2);
        Test[7] = LoadInt(Test_EEPROM_ReadWrite_B7, K_UseRealE2);

        if(Test[0] != MagicNumber || Test[1] != MagicNumber || Test[2] != MagicNumber || Test[3] != MagicNumber ||
           Test[4] != MagicNumber || Test[5] != MagicNumber || Test[6] != MagicNumber || Test[7] != MagicNumber)
        {
            DelaymSec(50);
            I2C_Init();
            Retry--;
        }
        else Passed=1;
    }
    
    if(Passed==0) Me.Error.EEPROM_ReadWrite = 1;           // Segnalo Errore EEPROM 

 }  
 
 
//--------------------------------------------------------------------------------------------------------------------------------
// Esegue un test di ritenzione del dato sull'ultima locazione di tutti gli 8 banchi EEPROM
// Se il test fallisce viene segnalato con il flag "Me.Error.EEPROM_Retention == 1"
//--------------------------------------------------------------------------------------------------------------------------------
void CheckEEPROM_Retention(void)
{
    unsigned int E2Value;
    int MagicNumber = K_Test_EEPROM_Retention;
    
    E2Value = LoadInt(Test_EEPROM_Retention_B0, K_UseRealE2);
    if(E2Value != K_Test_EEPROM_Retention)
    {
        Me.Error.EEPROM_Retention = 1;                      // Segnalo Errore EEPROM 
        StoreInt(MagicNumber, Test_EEPROM_Retention_B0);    // e inizializzo con "MagicNumber"      
    }
    
    E2Value = LoadInt(Test_EEPROM_Retention_B1, K_UseRealE2);
    if(E2Value != K_Test_EEPROM_Retention)
    {
        Me.Error.EEPROM_Retention = 1;                      // Segnalo Errore EEPROM 
        StoreInt(MagicNumber, Test_EEPROM_Retention_B1);    // e inizializzo con "MagicNumber"      
    }
    
    E2Value = LoadInt(Test_EEPROM_Retention_B2, K_UseRealE2);
    if(E2Value != K_Test_EEPROM_Retention)
    {
        Me.Error.EEPROM_Retention = 1;                      // Segnalo Errore EEPROM 
        StoreInt(MagicNumber, Test_EEPROM_Retention_B2);    // e inizializzo con "MagicNumber"      
    }
    
    E2Value = LoadInt(Test_EEPROM_Retention_B3, K_UseRealE2);
    if(E2Value != K_Test_EEPROM_Retention)
    {
        Me.Error.EEPROM_Retention = 1;                      // Segnalo Errore EEPROM 
        StoreInt(MagicNumber, Test_EEPROM_Retention_B3);    // e inizializzo con "MagicNumber"      
    }
    
    E2Value = LoadInt(Test_EEPROM_Retention_B4, K_UseRealE2);
    if(E2Value != K_Test_EEPROM_Retention)
    {
        Me.Error.EEPROM_Retention = 1;                      // Segnalo Errore EEPROM 
        StoreInt(MagicNumber, Test_EEPROM_Retention_B4);    // e inizializzo con "MagicNumber"      
    }
    
    E2Value = LoadInt(Test_EEPROM_Retention_B5, K_UseRealE2);
    if(E2Value != K_Test_EEPROM_Retention)
    {
        Me.Error.EEPROM_Retention = 1;                      // Segnalo Errore EEPROM 
        StoreInt(MagicNumber, Test_EEPROM_Retention_B5);    // e inizializzo con "MagicNumber"      
    }
    
    E2Value = LoadInt(Test_EEPROM_Retention_B6, K_UseRealE2);
    if(E2Value != K_Test_EEPROM_Retention)
    {
        Me.Error.EEPROM_Retention = 1;                      // Segnalo Errore EEPROM 
        StoreInt(MagicNumber, Test_EEPROM_Retention_B6);    // e inizializzo con "MagicNumber"      
    }
    
    E2Value = LoadInt(Test_EEPROM_Retention_B7, K_UseRealE2);
    if(E2Value != K_Test_EEPROM_Retention)
    {
        Me.Error.EEPROM_Retention = 1;                      // Segnalo Errore EEPROM 
        StoreInt(MagicNumber, Test_EEPROM_Retention_B7);    // e inizializzo con "MagicNumber"      
    }
    
}


void InitEEPROM_Retention(void)
{
    int MagicNumber = K_Test_EEPROM_Retention;
    
    StoreInt(MagicNumber, Test_EEPROM_Retention_B0);
    StoreInt(MagicNumber, Test_EEPROM_Retention_B1);
    StoreInt(MagicNumber, Test_EEPROM_Retention_B2);
    StoreInt(MagicNumber, Test_EEPROM_Retention_B3);
    StoreInt(MagicNumber, Test_EEPROM_Retention_B4);
    StoreInt(MagicNumber, Test_EEPROM_Retention_B5);
    StoreInt(MagicNumber, Test_EEPROM_Retention_B6);
    StoreInt(MagicNumber, Test_EEPROM_Retention_B7);    
}


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
        *(Buffer+temp) = LoadInt(temp*2, K_UseRealE2); 
    }
}


void CheckEEPROMCell(void)
{
    int temp;
    int ReadVal;

    for(temp=0; temp<K_EEPROM_WordSize; temp++)
    {
        StoreInt(K_Test_EEPROM_TestPattern1, temp*2);              
    }    
    for(temp=0; temp<K_EEPROM_WordSize; temp++)
    {
        ReadVal = LoadInt(temp*2, K_UseRealE2); 
        if(ReadVal != K_Test_EEPROM_TestPattern1)
            Me.Error.EEPROM_Cell = 1;                      // Segnalo Errore EEPROM 
    }    
    
    for(temp=0; temp<K_EEPROM_WordSize; temp++)
    {
        StoreInt(K_Test_EEPROM_TestPattern2, temp*2);      
    }    
    for(temp=0; temp<K_EEPROM_WordSize; temp++)
    {
        ReadVal = LoadInt(temp*2, K_UseRealE2); 
        if(ReadVal != K_Test_EEPROM_TestPattern2)
            Me.Error.EEPROM_Cell = 1;                      // Segnalo Errore EEPROM 
    }    
    
    for(temp=0; temp<K_EEPROM_WordSize; temp++)
    {
        StoreInt(K_Reset_EEPROM_Value, temp*2);      
    }    
    for(temp=0; temp<K_EEPROM_WordSize; temp++)
    {
        ReadVal = LoadInt(temp*2, K_UseRealE2); 
        if(ReadVal != K_Reset_EEPROM_Value)
            Me.Error.EEPROM_Cell = 1;                      // Segnalo Errore EEPROM 
    }    
}

void CheckEEPROMCellSector(void)
{
    int temp;
    int ReadVal;

    int StartAddress;
    int StopAddress;
    int StartAddress2;
    int StopAddress2;
    
            
    StartAddress = (K_EEPROM_Primary_Sector << 8) & 0x0700; 
    StopAddress = (StartAddress + 256);

    StartAddress2 = (K_EEPROM_Secondary_Sector << 8) & 0x0700; 
    StopAddress2 = StartAddress2 + 256;
     
    for(temp=StartAddress; temp<StopAddress; temp+=2)
    {
        StoreInt(K_Test_EEPROM_TestPattern1, temp);              
    }
    for(temp=StartAddress; temp<StopAddress; temp+=2)
    {
        ReadVal = LoadInt(temp, K_UseRealE2); 
        if(ReadVal != K_Test_EEPROM_TestPattern1)
            Me.Error.EEPROM_Cell = 1;                      // Segnalo Errore EEPROM 
    }    
    
    for(temp=StartAddress; temp<StopAddress; temp+=2)
    {
        StoreInt(K_Test_EEPROM_TestPattern2, temp);      
    }
    for(temp=StartAddress; temp<StopAddress; temp+=2)
    {    
        ReadVal = LoadInt(temp, K_UseRealE2); 
        if(ReadVal != K_Test_EEPROM_TestPattern2)
            Me.Error.EEPROM_Cell = 1;                      // Segnalo Errore EEPROM 
    }    
    
    for(temp=StartAddress; temp<StopAddress; temp+=2)
    {
        StoreInt(K_Reset_EEPROM_Value, temp);      
    }
    for(temp=StartAddress; temp<StopAddress; temp+=2)
    {
        ReadVal = LoadInt(temp, K_UseRealE2); 
        if(ReadVal != K_Reset_EEPROM_Value)
            Me.Error.EEPROM_Cell = 1;                      // Segnalo Errore EEPROM 
    }    

    for(temp=StartAddress2; temp<StopAddress2; temp+=2)
    {
        StoreInt(K_Test_EEPROM_TestPattern1, temp);              
    }
    for(temp=StartAddress2; temp<StopAddress2; temp+=2)
    {    
        ReadVal = LoadInt(temp, K_UseRealE2); 
        if(ReadVal != K_Test_EEPROM_TestPattern1)
            Me.Error.EEPROM_Cell = 1;                      // Segnalo Errore EEPROM 
    }    
    
    for(temp=StartAddress2; temp<StopAddress2; temp+=2)
    {
        StoreInt(K_Test_EEPROM_TestPattern2, temp);      
    }
    for(temp=StartAddress2; temp<StopAddress2; temp+=2)
    {    
        ReadVal = LoadInt(temp, K_UseRealE2); 
        if(ReadVal != K_Test_EEPROM_TestPattern2)
            Me.Error.EEPROM_Cell = 1;                      // Segnalo Errore EEPROM 
    }    
    
    for(temp=StartAddress2; temp<StopAddress2; temp+=2)
    {
        StoreInt(K_Reset_EEPROM_Value, temp);      
    }
    for(temp=StartAddress2; temp<StopAddress2; temp+=2)
    {    
        ReadVal = LoadInt(temp, K_UseRealE2); 
        if(ReadVal != K_Reset_EEPROM_Value)
            Me.Error.EEPROM_Cell = 1;                      // Segnalo Errore EEPROM 
    }    
}

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------

void LoadE2VirtualPageBuffer(void)
{
    int Address;
    
    for(Address=0; Address<K_EEPROM_ByteSize; Address++)
    {
        VirtualE2PageBuff[Address] = ReadRegI2C(EEPROM_Add, Address);
    }    
}
