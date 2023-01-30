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

//extern volatile TypTimer TimerModRstComErr;
extern volatile int ModTimeOutComErr_U4;		
extern volatile int ModCntComErr_U4;			
extern volatile TypTimer StableValueChangeHMI;         // Timer x gestione accettazione cambio valori da HMI Scheiber

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------


void Init_ModbCAN(void)
{            
    if(Check_IfModbCANIsPresent()!=0)
    {
        Send_InitDataToModbCAN();	
        ModbCAN.Ready=1;
    }
}

int Check_IfModbCANIsPresent(void)
{
	int OfflineFlag=0;
	int ReadWord;
	int ReadWord2;
		
    OfflineFlag = Send_WaitModRx_U4(k_MODBCAN_Addr, ReadModCmd, REG_CAN_CONVERTER_TIMEOUT_FLAG, 1, MaxModReadWait, MaxModReadRetries);
    if(OfflineFlag==0)					// se OfflineFlag == 0 ConvertiroreCAN presente
        ReadWord = ExtracReg_U4(1);

	
    OfflineFlag = Send_WaitModRx_U4(k_MODBCAN_Addr, ReadModCmd, REG_MODBCAN_ALIVE_MAJVER_HMI, 1, MaxModReadWait, MaxModReadRetries);
    if(OfflineFlag==0)					// se OfflineFlag == 0 ConvertiroreCAN presente
        ReadWord2 = ExtracReg_U4(1);

    ModbCAN.OnLine = !OfflineFlag;
	
	if(OfflineFlag==0)              // Se ConvertiroreCAN presente 
    {
		if(ReadWord==0) 
            return 1;				// Ritorno 1 se BUS CAN non in timeout
        else
            return 2;               // Ritorno 2 se BUS CAN in timeout
    }
	else 
		return 0;                   // altrimenti se ConvertiroreCAN non presente ritorno 0
    
}

void Send_InitDataToModbCAN(void)
{
    unsigned int temp[10];
    unsigned int Offs=0;
    unsigned int x;
    
	temp[0] = EngineBox[0].FW_Version_HW;
	temp[1] = EngineBox[0].FW_Version_FW = Versione_Fw;				// REG_TOUCH_MASTER_FW_VERSION_FW
	temp[2] = EngineBox[0].FW_Revision_FW = Revisione_Fw;             // REG_TOUCH_MASTER_FW_REVISION_FW
    temp[3] = K_MODBCAN_ALIVE_CRCLO;
    temp[4] = K_MODBCAN_ALIVE_CRCHI;
    
    InsertReg_U4(1, temp[0]);										// REG_MODBCAN_ALIVE_MAJVER_MOD0
    InsertReg_U4(2, temp[1]);										// REG_MODBCAN_ALIVE_MINVER_MOD0
    InsertReg_U4(3, temp[2]);										// REG_MODBCAN_ALIVE_PATVER_MOD0
    InsertReg_U4(4, temp[3]);										// REG_MODBCAN_ALIVE_CRCHI_MOD0
    InsertReg_U4(5, temp[4]);										// REG_MODBCAN_ALIVE_CRCLO_MOD0         INIVIO TUTTI I CAMPI TRANNE L'ULTIMO CHE SCATENEREBBE L'ALIVE LATO CAN
        
    for(x=1; x<=K_ModbCanRoomNumber; x++)                           // Inizializzo tutti i possibili moduli (16) anche se non presenti)
    {
        //if(Room[x].OnLine)
        //{
            Offs = (x-1) * K_ModbCanSizeOfTxData;
            Send_WaitModRx_U4(k_MODBCAN_Addr, WriteModListCmd, REG_MODBCAN_ALIVE_MAJVER_MOD0+Offs, 4/*5*/, MaxModWriteWait, MaxModWriteRetries);         //INIVIO TUTTI I CAMPI TRANNE L'ULTIMO CHE SCATENEREBBE L'ALIVE LATO CAN
        //}
            
            PackInitDataForCAN(x);
    }    
    
}

int Check_IfModbCANGoOnline(TypModbCAN * ModbCANflag)
{
	#define K_Max_CAN_Offline_CNT		5
    static int LoseActivity=0;
	    
    if(ModbCANflag->PermanentOffLine)
        EngineBox[0].DisableModComErrorCounter_U4=1;
	ModbCANflag->OffLine = Send_WaitModRx_U4(k_MODBCAN_Addr, ReadModCmd, REG_CAN_CONVERTER_TIMEOUT_FLAG, 1, 20, 1);
    
    if(ModbCANflag->OffLine==0)                             // se OfflineFlag == 0 ConvertiroreCAN presente
        ModbCANflag->CANBUS_Activity = ExtracReg_U4(1);    // 1 = BUS CAN Ok    0 == BUS CAN in timeout
    
    //ARGOTRAPPOLA  
    //if(ModbCANflag->CANBUS_Activity == 0)
    //    ModbCANflag->OffLine=0;
    
    
    ModbCANflag->OnLine = !ModbCANflag->OffLine; 
    EngineBox[0].DisableModComErrorCounter_U4=0;
    
    if(ModbCANflag->OnLine==1 && ModbCANflag->CANBUS_Activity==0)
    {
        LoseActivity=1;        
    }

    if(ModbCANflag->OnLine==1 && ModbCANflag->CANBUS_Activity==1 &&  LoseActivity==1)
    {
        LoseActivity=0;
        Init_ModbCAN();
    }
	
    if(ModbCANflag->OnLine==0)					// se OfflineFlag == 0 scheda presente
	{
		if(ModbCANflag->PermanentOffLine == 0)
		{
			ModbCANflag->OffLineCounter += 1;
			if(ModbCANflag->OffLineCounter>=K_Max_CAN_Offline_CNT)
			{
				ModbCANflag->PermanentOffLine = 1;
                ModbCANflag->Ready = 0;
                ModbCANflag->InitializeModbCAN = 0;
			}
		}
		return 0;
    }
    else if(ModbCANflag->Ready==1)
	{
		ModbCANflag->OffLineCounter = 0;
		ModbCANflag->PermanentOffLine = 0;
        ModbCANflag->InitializeModbCAN = 0;
		return 0;
	}
	else if(ModbCANflag->OnLine==1 && ModbCANflag->PermanentOffLine == 1)
	{
		ModbCANflag->OffLineCounter = 0;
		//ModbCANflag->PermanentOffLine = 0;
		return 1;
	}    

}



void RefreshDataForCAN(int RoomIndex)
{
    int Buffer[4];
    int temp,temp2,temp3,temp4,temp5;
    int Offs;

    
    temp = (int)round((float)(Room[RoomIndex].AirTemp)/10.0);
    temp2 = (int)ValueIn2ValueOut(temp, -100, 400, 40, 140);
       
    temp3 = (ModbCAN.SetRoom[RoomIndex].NewData0>>4) & 0x0F;    // Estraggo "MODE"
    temp4 = ModbCAN.SetRoom[RoomIndex].NewData0 & 0x0F;         // Estraggo "FAN Speed"
    temp5 = ModbCAN.SetRoom[RoomIndex].NewData1 & 0x3F;         // Estraggo "T. SetPoint"
    
    //if(temp3>1)     // ARGOTRAPPOLA
    //    temp3=1;

    /*
    LimitDataValue(0, 1, &temp3);                               // Limito "MODE" tra 0 (OFF) e 1 (ON-AUTO)
    LimitDataValue(0, 8, &temp4);                               // Limito "FANSPEED" tra 0 (AUTO) e 8 (Speed7):     Auto, Night, Speed1, Speed2, Speed3, Speed4 ,Speed5 ,Speed6 ,Speed7
    LimitDataValue(36, 60, &temp5);                             // Limito "T.SetPoint" tra 36 (18°C) e 60 (30°C):   18..30°C
    */

    ModbCAN.SetRoom[RoomIndex].NewData0 = (temp3<<4) | temp4;   // Ricreo REG_MODBCAN_STATUS_MODE_FANS_MOD0 con valori limitati
    ModbCAN.SetRoom[RoomIndex].NewData1 = temp5;                // Ricreo REG_MODBCAN_STATUS_AUX_SETP_MOD0 con valori limitati

    Buffer[0] = ModbCAN.SetRoom[RoomIndex].NewData0;            // REG_MODBCAN_STATUS_MODE_FANS_MOD0
    Buffer[1] = ModbCAN.SetRoom[RoomIndex].NewData1;            // REG_MODBCAN_STATUS_AUX_SETP_MOD0
    
    Buffer[2] = temp2;
    Buffer[3] = Room[RoomIndex].CumulativeAlarm;
    
    Offs = (RoomIndex-1) * K_ModbCanSizeOfTxData;    
    InsertReg_U4(1, Buffer[0]);										// REG_MODBCAN_STATUS_MODE_FANS_MOD0
    InsertReg_U4(2, Buffer[1]);										// REG_MODBCAN_STATUS_AUX_SETP_MOD0
    InsertReg_U4(3, Buffer[2]);										// REG_MODBCAN_STATUS_TEMPER_MOD0
    InsertReg_U4(4, 0);             								// REG_MODBCAN_STATUS_SCHEDID_MOD0
    InsertReg_U4(5, Buffer[3]);										// REG_MODBCAN_STATUS_ERRCODE_MOD0    
    Send_WaitModRx_U4(k_MODBCAN_Addr, WriteModListCmd, REG_MODBCAN_STATUS_MODE_FANS_MOD0+Offs, K_ModbCanSizeOfTxData, MaxModWriteWait, MaxModWriteRetries); 

    /* TTOARGOMORENNI
    Offs = (RoomIndex-1) * 2;    
    InsertReg_U4(1, Buffer[0]);										// REG_MODBCAN_STATUS_MODE_FANS_MOD0
    InsertReg_U4(2, Buffer[1]);										// REG_MODBCAN_STATUS_AUX_SETP_MOD0    
    Send_WaitModRx_U4(k_MODBCAN_Addr, WriteModListCmd, REG_MODBCAN_COMMAND_MODE_FANS_MOD0+Offs, 2, MaxModWriteWait, MaxModWriteRetries); 
    */
    
    
    /*
    WriteModReg_U4(k_MODBCAN_Addr, REG_MODBCAN_STATUS_MODE_FANS_MOD0+Offs, Buffer[0]);
    WriteModReg_U4(k_MODBCAN_Addr, REG_MODBCAN_STATUS_AUX_SETP_MOD0+Offs, Buffer[1]);
    WriteModReg_U4(k_MODBCAN_Addr, REG_MODBCAN_STATUS_TEMPER_MOD0+Offs, Buffer[2]);
    WriteModReg_U4(k_MODBCAN_Addr, REG_MODBCAN_STATUS_SCHEDID_MOD0+Offs, 0);
    WriteModReg_U4(k_MODBCAN_Addr, REG_MODBCAN_STATUS_ERRCODE_MOD0+Offs, Buffer[3]);
     */
}



void PackDataForCAN(int RoomIndex)
{
    int Buffer[4];
    int temp,temp2,temp3,temp4,temp5;

    temp = (int)round((float)(Room[RoomIndex].AirTemp)/10.0);
    temp2 = (int)ValueIn2ValueOut(temp, -100, 400, 40, 140);

    temp = (int)round((float)(Room[RoomIndex].SetPoint)/10.0);
    temp3 = (int)ValueIn2ValueOut(temp, 180, 300, 36, 60);

    temp4 = Room[RoomIndex].FanMode;
    if(temp4>=K_MaxTouchFanSpeed) temp4=0;               // 9 = FanAuto (TDM)   0 = FanAuto (Scheiber)
    
    /*
    switch(EngineBox[0].DefMode)
    {
        case CoreOff:
            temp5 = 0;
        break;
        case CoreAuto:
            temp5 = 1;
        break;
        case CoreRiscaldamento:
            temp5 = 3;
        break;
        case CoreRaffrescamento:
            temp5 = 2;
        break;
        case CoreVentilazione:
            temp5 = 5;
        break;
        case CoreDehumi:
            temp5 = 4;
        break;
    }
    
    temp5 = temp5 * (Room[RoomIndex].PowerOn==1) * (Room[1].SystemDisable==0);
    */
    temp5 = (Room[RoomIndex].PowerOn==1); // * (Room[1].SystemDisable==0);
    
    if(ModbCAN.SetRoom[RoomIndex].UpdateFlag)
    {
        Buffer[0] = ModbCAN.SetRoom[RoomIndex].NewData0;            // REG_MODBCAN_STATUS_MODE_FANS_MOD0
        Buffer[1] = ModbCAN.SetRoom[RoomIndex].NewData1;            // REG_MODBCAN_STATUS_AUX_SETP_MOD0
    }
    else
    {
        Buffer[0] = (temp5<<4) | temp4;
        Buffer[1] = (0<<6) | temp3;
    }
    
    Buffer[2] = temp2;
    Buffer[3] = Room[RoomIndex].CumulativeAlarm;
    
    
    InsertReg_U4(1, Buffer[0]);										// REG_MODBCAN_STATUS_MODE_FANS_MOD0
    InsertReg_U4(2, Buffer[1]);										// REG_MODBCAN_STATUS_AUX_SETP_MOD0
    InsertReg_U4(3, Buffer[2]);										// REG_MODBCAN_STATUS_TEMPER_MOD0
    InsertReg_U4(4, 0);             								// REG_MODBCAN_STATUS_SCHEDID_MOD0
    InsertReg_U4(5, Buffer[3]);										// REG_MODBCAN_STATUS_ERRCODE_MOD0
    
}

void UnPackDataFromCAN(int RoomIndex)
{
    int temp,temp3,temp4,temp5;

    /*
    temp = (ModbCAN.SetRoom[RoomIndex].NewData0 & 0x00F0) >> 4;
    
    switch(temp)
    {                   // VIGANOTA: N.B. Per il momento la modalità "AUTO" e "DEHUMI" selezionate sull' HMI
                        // vengono settate come Ventilazione (in attesa di decidere come gestirle))
        case 0:
            temp5 = CoreOff;                        // 0
        break;
        case 1:
            temp5 = CoreVentilazione; //CoreAuto;                       // 2
        break;
        case 2:
            temp5 = CoreRaffrescamento;             // 8
        break;
        case 3:
            temp5 = CoreRiscaldamento;              // 4
        break;
        case 4:
            temp5 = CoreVentilazione; //CoreDehumi;                     // 32
        break;
        case 5:
            temp5 = CoreVentilazione;               // 16
        break;
    }
    */
    
    temp = ModbCAN.SetRoom[RoomIndex].NewData1 & 0x003F;                // Estraggo dato "SetPoint"
    if(temp<36) temp=36;                                                // Limito Valori tra 18°C (36)
    else    
        if(temp>60) temp=60;                                            // e 30°C (60)
    temp3 = (int)(ValueIn2ValueOut(temp, 36, 60, 180, 300) * 10.0);     // e riscalo valori in centesimi
              
    temp4 = (ModbCAN.SetRoom[RoomIndex].NewData0 & 0x000F);
    if(temp4>K_MaxTouchFanSpeed) temp4=K_MaxTouchFanSpeed;
    else
        if(temp4==0) temp4=K_MaxTouchFanSpeed;//9;
    
    temp5 = (ModbCAN.SetRoom[RoomIndex].NewData0 & 0x00F0) >> 4;  
    ModbCAN.SetRoom[RoomIndex].PowerOn = temp5!=0;     // Se Mode != OFF

    //if(temp5!=0)
    //    ModbCAN.SetRoom[RoomIndex].DefMode = temp5;
    
    ModbCAN.SetRoom[RoomIndex].SetPoint = temp3;
    ModbCAN.SetRoom[RoomIndex].FanSpeed = temp4;
    ModbCAN.SetRoom[RoomIndex].AuxHeater = (ModbCAN.SetRoom[RoomIndex].NewData1 & 0x0040) != 0;
       
}

void PackInitDataForCAN(int RoomIndex)
{
    int Buffer[4];
    int temp,temp2,temp3,temp4,temp5,Offs;

    temp = (int)round((float)(Room[RoomIndex].AirTemp)/10.0);
    temp2 = (int)ValueIn2ValueOut(temp, -100, 400, 40, 140);

    temp = (int)round((float)(Room[RoomIndex].SetPoint)/10.0);
    temp3 = (int)ValueIn2ValueOut(temp, 180, 300, 36, 60);

    temp4 = Room[RoomIndex].FanMode;
    if(temp4>=K_MaxTouchFanSpeed) temp4=0;               // 9 = FanAuto (TDM)   0 = FanAuto (Scheiber)
    
    temp5 = (Room[RoomIndex].PowerOn==1); // * (Room[1].SystemDisable==0);
    
    Buffer[0] = (temp5<<4) | temp4;
    Buffer[1] = (0<<6) | temp3;
    
    Buffer[2] = temp2;
    Buffer[3] = Room[RoomIndex].CumulativeAlarm;
        
    Offs = (RoomIndex-1) * K_ModbCanSizeOfTxData;    
    InsertReg_U4(1, Buffer[0]);										// REG_MODBCAN_STATUS_MODE_FANS_MOD0
    InsertReg_U4(2, Buffer[1]);										// REG_MODBCAN_STATUS_AUX_SETP_MOD0
    InsertReg_U4(3, Buffer[2]);										// REG_MODBCAN_STATUS_TEMPER_MOD0
    InsertReg_U4(4, 0);             								// REG_MODBCAN_STATUS_SCHEDID_MOD0
    InsertReg_U4(5, Buffer[3]);										// REG_MODBCAN_STATUS_ERRCODE_MOD0    
    Send_WaitModRx_U4(k_MODBCAN_Addr, WriteModListCmd, REG_MODBCAN_STATUS_MODE_FANS_MOD0+Offs, K_ModbCanSizeOfTxData, MaxModWriteWait, MaxModWriteRetries); 
    
}

int CheckChangeValueRoomToHMI(int RoomIndex)
{
    int RetVal=0;
    
    if(ModbCAN.EngineBox.SystemDisable!=Room[1].SystemDisable)
    {
        ModbCAN.EngineBox.SystemDisable = Room[1].SystemDisable;
        RetVal=1;
    }

    /*
    if(ModbCAN.EngineBox.DefMode!=EngineBox[0].DefMode)
    {
        ModbCAN.EngineBox.DefMode=EngineBox[0].DefMode;
        RetVal=2;
    }
     */ 
    
    if(ModbCAN.Room[RoomIndex].AirTemp!=Room[RoomIndex].AirTemp)
    {
        ModbCAN.Room[RoomIndex].AirTemp=Room[RoomIndex].AirTemp;
        RetVal=3;
    }
    
    if(ModbCAN.Room[RoomIndex].CumulativeAlarm!=Room[RoomIndex].CumulativeAlarm)
    {
        ModbCAN.Room[RoomIndex].CumulativeAlarm=Room[RoomIndex].CumulativeAlarm;
        RetVal=4;
    }
    
    if(ModbCAN.Room[RoomIndex].PowerOn!=Room[RoomIndex].PowerOn)
    {
        ModbCAN.Room[RoomIndex].PowerOn=Room[RoomIndex].PowerOn;
        RetVal=5;
    }

    if(ModbCAN.Room[RoomIndex].SetPoint!=Room[RoomIndex].SetPoint)
    {
        ModbCAN.Room[RoomIndex].SetPoint=Room[RoomIndex].SetPoint;
        RetVal=6;
    }

    if(ModbCAN.Room[RoomIndex].FanSpeed!=Room[RoomIndex].FanMode)
    {
        ModbCAN.Room[RoomIndex].FanSpeed=Room[RoomIndex].FanMode;
        RetVal=7;
    }
    
    return RetVal;
}

int CheckChangeValueRoomFromHMI(int RoomIndex)
{
    int RetVal=0;
    int Mode,FanS,SetP,IsLimit=0,Offs;       
    
    if(ModbCAN.SetRoom[RoomIndex].OldData0 != ModbCAN.SetRoom[RoomIndex].NewData0)
    {
        /**/
        Mode = (ModbCAN.SetRoom[RoomIndex].NewData0>>4) & 0x0F;     // Estraggo "MODE"
        FanS = ModbCAN.SetRoom[RoomIndex].NewData0 & 0x0F;          // Estraggo "FAN Speed"
        IsLimit = LimitDataValue(0, 1, &Mode);                                // Limito "MODE" tra 0 (OFF) e 1 (ON-AUTO)
        IsLimit = LimitDataValue(0, (K_MaxTouchFanSpeed-1), &FanS);                                // Limito "FANSPEED" tra 0 (AUTO) e 8 (Speed7):     Auto, Night, Speed1, Speed2, Speed3, Speed4 ,Speed5 ,Speed6 ,Speed7
        ModbCAN.SetRoom[RoomIndex].NewData0 = (Mode << 4) | FanS;   // Repak "MODE"+"Fan Speed"
         
        ModbCAN.SetRoom[RoomIndex].OldData0 = ModbCAN.SetRoom[RoomIndex].NewData0;
        RetVal=1;
    }
                
    if(ModbCAN.SetRoom[RoomIndex].OldData1 != ModbCAN.SetRoom[RoomIndex].NewData1)
    {
        /**/
        SetP = ModbCAN.SetRoom[RoomIndex].NewData1 & 0x3F;          // Estraggo "T. SetPoint"
        IsLimit = LimitDataValue(36, 60, &SetP);                              // Limito "T.SetPoint" tra 36 (18°C) e 60 (30°C):   18..30°C
        ModbCAN.SetRoom[RoomIndex].NewData1 = SetP & 0x3F;          // Repak "T. SetPoint"
         
        ModbCAN.SetRoom[RoomIndex].OldData1 = ModbCAN.SetRoom[RoomIndex].NewData1;
        RetVal=1;
    }
        
    if(IsLimit==1)
    {
        //TTOARGOMORENNI 
       Offs = (RoomIndex-1) * 2;    
       InsertReg_U4(1, ModbCAN.SetRoom[RoomIndex].NewData0);										// REG_MODBCAN_STATUS_MODE_FANS_MOD0
       InsertReg_U4(2, ModbCAN.SetRoom[RoomIndex].NewData1);										// REG_MODBCAN_STATUS_AUX_SETP_MOD0    
       Send_WaitModRx_U4(k_MODBCAN_Addr, WriteModListCmd, REG_MODBCAN_COMMAND_MODE_FANS_MOD0+Offs, 2, MaxModWriteWait, MaxModWriteRetries);             

       LoadTimer(&StableValueChangeHMI, K_StableValueChangeHMI, 0, 1);     // Riarmo Timer
    }
    
    
    return RetVal;
}


void UpdateRoomDataFromCAN(int RoomIndex)
{               
   
    
    if(Room[RoomIndex].Enable)
    {
        
        Room[RoomIndex].OffLine = FuncWriteReg (RoomIndex, REG_SPLIT_FROM_INIT_SP_ROOM, ModbCAN.SetRoom[RoomIndex].SetPoint, Room[RoomIndex].OffLine);					// REG_SPLIT_STATUS
        Room[RoomIndex].SetPoint = ModbCAN.SetRoom[RoomIndex].SetPoint;
        Room[RoomIndex].OffLine = FuncWriteReg (RoomIndex, REG_SPLIT_FROM_INIT_FAN_SPEED, ModbCAN.SetRoom[RoomIndex].FanSpeed, Room[RoomIndex].OffLine);					// REG_SPLIT_STATUS
        Room[RoomIndex].FanSpeed = ModbCAN.SetRoom[RoomIndex].FanSpeed;
        Room[RoomIndex].OffLine = FuncWriteReg (RoomIndex, REG_SPLIT_FROM_INIT_POWER, ModbCAN.SetRoom[RoomIndex].PowerOn, Room[RoomIndex].OffLine);					// REG_SPLIT_STATUS
        Room[RoomIndex].PowerOn = ModbCAN.SetRoom[RoomIndex].PowerOn;
        //Room[RoomIndex].OffLine = FuncWriteReg (RoomIndex, REG_SPLIT_FROM_INIT_UTAMODE, ModbCAN.SetRoom[RoomIndex].DefMode, Room[RoomIndex].OffLine);            
        //Room[RoomIndex].OffLine = FuncWriteReg (RoomIndex, REG_SPLIT_FROM_INIT_AUXHEATER, ModbCAN.SetRoom[RoomIndex].AuxHeater, Room[RoomIndex].OffLine);					// REG_SPLIT_STATUS
        Room[RoomIndex].OffLine = FuncWriteReg (RoomIndex, REG_SPLIT_OTHER_CABIN_TRIGGER_INIT, 8, Room[RoomIndex].OffLine);					// REG_SPLIT_STATUS      
    }	
    /*
	if(Room[k_Split_Master_Add].Enable)
	{
		Room[k_Split_Master_Add].OffLine = FuncWriteReg (k_Split_Master_Add, REG_SPLIT_WR_MODE, ModbCAN.SetRoom[RoomIndex].DefMode, Room[k_Split_Master_Add].OffLine);					// REG_SPLIT_STATUS		
        Room[k_Split_Master_Add].DefMode = ModbCAN.SetRoom[RoomIndex].DefMode;
	}    
     */
    
/*    
    //---------------------------	
	// SetPoint
	//---------------------------
	if(Room[1].OnLine)
	{
	//	Room[1].OffLine = FuncWriteReg (1, REG_DIAG_SPLIT_WR_SET_SETPOINT, ModbCAN.SetRoom[RoomIndex].SetPoint, Room[1].OffLine);					// REG_SPLIT_STATUS		
	}
	if(Room[RoomIndex].OnLine) 
	{
		Room[RoomIndex].OffLine = FuncWriteReg (RoomIndex, REG_SPLIT_SYNC_WR_SETP, ModbCAN.SetRoom[RoomIndex].SetPoint, Room[RoomIndex].OffLine);					// REG_SPLIT_STATUS
	}

	//Syncro2Diag(k_Split_Master_Add, RoomIndex, REG_DIAG_SPLIT_RD_SET_SETPOINT, REG_DIAG_SPLIT_WR_SET_SETPOINT, REG_SPLIT_SYNC_RD_SETP, REG_SPLIT_SYNC_WR_SETP, &EqualRead1[0], &OldEqualRead1[0], &EqualRead2[0], &OldEqualRead2[0], &ModbCAN.SetRoom[RoomIndex].SetPoint);
		
	//---------------------------	
	// Power
	//---------------------------	
	if(Room[1].OnLine)
	{
	//	Room[1].OffLine = FuncWriteReg (1, REG_DIAG_SPLIT_WR_SET_POWER, ModbCAN.SetRoom[RoomIndex].PowerOn, Room[1].OffLine);					// REG_SPLIT_STATUS		
	}
	if(Room[RoomIndex].OnLine) 
	{
		Room[RoomIndex].OffLine = FuncWriteReg (RoomIndex, REG_SPLIT_SYNC_WR_PWR, ModbCAN.SetRoom[RoomIndex].PowerOn, Room[RoomIndex].OffLine);					// REG_SPLIT_STATUS
	}
	//Syncro2Diag(k_Split_Master_Add, RoomIndex, REG_DIAG_SPLIT_RD_SET_POWER, REG_DIAG_SPLIT_WR_SET_POWER, REG_SPLIT_SYNC_RD_PWR, REG_SPLIT_SYNC_WR_PWR, &EqualRead1[1], &OldEqualRead1[1], &EqualRead2[1], &OldEqualRead2[1], &ModbCAN.SetRoom[RoomIndex].PowerOn);	

	//---------------------------	
	// FAN Speed
	//---------------------------
	if(Room[1].OnLine)
	{
	//	Room[1].OffLine = FuncWriteReg (1, REG_DIAG_SPLIT_WR_SET_FANSPEED, ModbCAN.SetRoom[RoomIndex].FanSpeed, Room[1].OffLine);					// REG_SPLIT_STATUS		
	}
	if(Room[RoomIndex].OnLine) 
	{
		Room[RoomIndex].OffLine = FuncWriteReg (RoomIndex, REG_SPLIT_SYNC_WR_FANMODE, ModbCAN.SetRoom[RoomIndex].FanSpeed, Room[RoomIndex].OffLine);					// REG_SPLIT_STATUS
	}
	//Syncro2Diag(k_Split_Master_Add, RoomIndex, REG_DIAG_SPLIT_RD_SET_FANSPEED, REG_DIAG_SPLIT_WR_SET_FANSPEED, REG_SPLIT_SYNC_RD_FANMODE, REG_SPLIT_SYNC_WR_FANMODE, &EqualRead1[2], &OldEqualRead1[2], &EqualRead2[2], &OldEqualRead2[2], &ModbCAN.SetRoom[RoomIndex].FanSpeed);	

	//---------------------------	
	// Define Mode
	//---------------------------
	if(Room[1].OnLine)
	{
		Room[1].OffLine = FuncWriteReg (1, REG_SPLIT_WR_MODE, ModbCAN.SetRoom[RoomIndex].DefMode, Room[1].OffLine);					// REG_SPLIT_STATUS		
	}    
	//Syncro2Value(k_Split_Master_Add, REG_SPLIT_RD_MODE, REG_SPLIT_WR_MODE, REG_BRIDGE2_DIAG_SPLIT_WRITE_MODE, &EqualRead1[4], &OldEqualRead1[4], &EqualRead2[4], &OldEqualRead2[4], &ModbCAN.SetRoom[RoomIndex].DefMode);
    

 */ 
    
    
    
   //---------------------------	
	// SYSTEM DISABLE
	//---------------------------
	//Syncro2Value(k_Split_Master_Add, REG_SPLIT_RD_SYSTEM_DISABLE, REG_SPLIT_WR_SYSTEM_DISABLE, REG_BRIDGE2_SYSTEM_DISABLE, &EqualRead1[12], &OldEqualRead1[12], &EqualRead2[12], &OldEqualRead2[12], &Bridge[1].EngineBox.SystemDisable);
    
    /*
    Room[RoomIndex].OffLine = FuncWriteReg (RoomIndex, REG_SPLIT_FROM_INIT_SP_ROOM, ModbCAN.SetRoom[RoomIndex].SetPoint, Room[RoomIndex].OffLine);					// REG_SPLIT_STATUS
    Room[RoomIndex].OffLine = FuncWriteReg (RoomIndex, REG_SPLIT_FROM_INIT_FAN_SPEED, ModbCAN.SetRoom[RoomIndex].FanSpeed, Room[RoomIndex].OffLine);					// REG_SPLIT_STATUS
    Room[RoomIndex].OffLine = FuncWriteReg (RoomIndex, REG_SPLIT_FROM_INIT_POWER, ModbCAN.SetRoom[RoomIndex].PowerOn, Room[RoomIndex].OffLine);					// REG_SPLIT_STATUS
    //Room[RoomIndex].OffLine = FuncWriteReg (RoomIndex, REG_SPLIT_FROM_INIT_AUXHEATER, ModbCAN.SetRoom[RoomIndex].AuxHeater, Room[RoomIndex].OffLine);					// REG_SPLIT_STATUS
    Room[RoomIndex].OffLine = FuncWriteReg (RoomIndex, REG_SPLIT_OTHER_CABIN_TRIGGER_INIT, 8, Room[RoomIndex].OffLine);					// REG_SPLIT_STATUS      
    */
}



void Send_DataToModbCAN(void)
{
    unsigned int Offs=0;
    unsigned int x,Result=0;
        
    for(x=1; x<=K_ModbCanRoomNumber; x++)
    {

        if(StableValueChangeHMI.TimeOut==1 && ModbCAN.SetRoom[x].UpdateFlag==0)         // solo se ho finito di "smanacciare" sulla HMI....
            Result = CheckChangeValueRoomToHMI(x);  // verifico se ci sono variazioni dallo Slave
        if((Room[x].OnLine && Result!=0) || (Room[x].OnLine && ModbCAN.SetRoom[x].UpdateFlag==1 && StableValueChangeHMI.TimeOut==1))
        {
            Offs = (x-1) * K_ModbCanSizeOfTxData;
            PackDataForCAN(x);     
            Send_WaitModRx_U4(k_MODBCAN_Addr, WriteModListCmd, REG_MODBCAN_STATUS_MODE_FANS_MOD0+Offs, K_ModbCanSizeOfTxData, MaxModWriteWait, MaxModWriteRetries); 
            
            /* TTOARGOMORENNI 
            Offs = (x-1) * 2;    
            InsertReg_U4(1, ModbCAN.SetRoom[x].NewData0);										// REG_MODBCAN_STATUS_MODE_FANS_MOD0
            InsertReg_U4(2, ModbCAN.SetRoom[x].NewData1);										// REG_MODBCAN_STATUS_AUX_SETP_MOD0    
            Send_WaitModRx_U4(k_MODBCAN_Addr, WriteModListCmd, REG_MODBCAN_COMMAND_MODE_FANS_MOD0+Offs, 2, MaxModWriteWait, MaxModWriteRetries); 
             */
        }
    }
}


void Get_DataFromModbCAN(void)
{
    unsigned int Offs=0;
    unsigned int x,Result;
    static int ChangeTrigger[K_ModbCanRoomNumber+1]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        
    for(x=1; x<=K_ModbCanRoomNumber; x++)
    {
        if(Room[x].OnLine)
        {
            Offs = (x-1) * K_ModbCanSizeOfRxData;
            ReadModReg_U4(k_MODBCAN_Addr, REG_MODBCAN_COMMAND_MODE_FANS_MOD0+Offs, &ModbCAN.SetRoom[x].NewData0);       // Leggo nuovo dato da convertitore MODCAN
            ReadModReg_U4(k_MODBCAN_Addr, REG_MODBCAN_COMMAND_AUX_SETP_MOD0+Offs, &ModbCAN.SetRoom[x].NewData1);        // Leggo nuovo dato da convertitore MODCAN
            
            Result = CheckChangeValueRoomFromHMI(x);                // Controllo E LIMITO se ci sono variazioni sui dati

            if(Result==1)
            {
                LoadTimer(&StableValueChangeHMI, K_StableValueChangeHMI, 0, 1);     // Riarmo Timer
                //ChangeTrigger[x] = 1;                                                  // Trigger variazioni rilevate
                ModbCAN.SetRoom[x].UpdateFlag=1;
                RefreshDataForCAN(x);                                               // Carico i dati cambiati da riinviare su HMI
                //ARGOCAZZONE
                //Send_WaitModRx_U4(k_MODBCAN_Addr, WriteModListCmd, REG_MODBCAN_STATUS_MODE_FANS_MOD0+Offs, K_ModbCanSizeOfTxData, MaxModWriteWait, MaxModWriteRetries); 
            }

            //if(ChangeTrigger[x]==1 && StableValueChangeHMI.TimeOut==1)
            if(ModbCAN.SetRoom[x].UpdateFlag==1 && StableValueChangeHMI.TimeOut==1)
            {            
                UnPackDataFromCAN(x);   
                //UpdateRoomDataFromCAN(x);
                //ModbCAN.SetRoom[x].UpdateFlag=1;    // Segnalo che è da aggiornare
                //ModbCAN.ChangedValueFromHMI=1;      // Segnalo che è da aggiornare
                //ChangeTrigger[x] = 0;
            }
            
        }
    }    
} 

void Send_UpdateDataToRoom(void)
{
    unsigned int x;
    static int trig=0;
    static int add=0;
#define K_MAX_CNT_DELAY_UPDATE_DATA_CAN 6//4

    for(x=1; x<=K_ModbCanRoomNumber; x++)
    {
        
        if(StableValueChangeHMI.TimeOut==0)
            ModbCAN.SetRoom[x].CntTrig=0;
        
        if(Room[x].OnLine && ModbCAN.SetRoom[x].UpdateFlag && ModbCAN.SetRoom[x].CntTrig==0 && StableValueChangeHMI.TimeOut==1)     
        {
            UpdateRoomDataFromCAN(x);           // Aggiorno eventuali variazioni sullo Slave
            ModbCAN.SetRoom[x].CntTrig=1;
            //ModbCAN.SetRoom[x].UpdateFlag=0;               
        }
        
        if(ModbCAN.SetRoom[x].CntTrig>0)
        {
            if(ModbCAN.SetRoom[x].CntTrig++>K_MAX_CNT_DELAY_UPDATE_DATA_CAN)
            {
                ModbCAN.SetRoom[x].CntTrig=0;
                ModbCAN.SetRoom[x].UpdateFlag=0;                               
            }
        }
    }
     
    /*
    if(StableValueChangeHMI.TimeOut==1)             // solo se ho finito di "smanacciare" sulla HMI....
    {
        for(x=1; x<=K_ModbCanRoomNumber; x++)
        {
            if(Room[x].OnLine && ModbCAN.SetRoom[x].UpdateFlag && ModbCAN.SetRoom[x].CntTrig==0)     
            {
                UpdateRoomDataFromCAN(x);           // Aggiorno eventuali variazioni sullo Slave
                ModbCAN.SetRoom[x].CntTrig=1;
                //ModbCAN.SetRoom[x].UpdateFlag=0;               
            }
            if(ModbCAN.SetRoom[x].CntTrig>0)
            {
                if(ModbCAN.SetRoom[x].CntTrig++>K_MAX_CNT_DELAY_UPDATE_DATA_CAN)
                {
                    ModbCAN.SetRoom[x].CntTrig=0;
                    ModbCAN.SetRoom[x].UpdateFlag=0;                               
                }
            }
        }
    }
    */
} 


void Send_AliveCANFrame_ToModbCAN(void)
{
    unsigned int Offs=0;
    unsigned int x;
        
    for(x=1; x<=K_ModbCanRoomNumber; x++)
    {
        if(Room[x].OnLine)
        {
            Offs = (x-1) * K_ModbCanSizeOfTxData;
            WriteModReg_U4(k_MODBCAN_Addr, REG_MODBCAN_ALIVE_CRCLO_MOD0+Offs, 0xFF);
        }
    }

}


int LimitDataValue(int ValMin, int ValMax, int *Reg)
{
    if(*Reg<ValMin) 
    {
        *Reg=ValMin;
        return 1;
    }
    else
    {
        if(*Reg>ValMax) 
        {
            *Reg=ValMax;
            return 1;
        }
    }
    
    return 0;
}

#endif //#if (K_AbilMODBCAN==1) 