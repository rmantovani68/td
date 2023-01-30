//----------------------------------------------------------------------------------
//	Progect name:	ProtocolloComunicazione.c
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			TDM Team
//	Date:			04/01/2014
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
#include <libpic30.h>
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
#include "Core.h"
#include "ProtocolloModBusSec.h"
#include "Driver_ModBusSec.h"
#include "FWSelection.h"
#include "PID.h"
#include "delay.h"

extern unsigned char CurUsedProtocolId;	// Id del protocollo correntemente utilizzato
extern volatile TypTimer TmrLastRequest;


//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
void SetMyAddress(char My_Add)
{
	My_Address = My_Add;
}

char GetMyAddress(void)
{
	return My_Address;
}

void ResetComFlags(void)
{
	LastRx.Valid_Data = 0;			// scarto il pacchetto
    TmrLastRequest.Value = 0;                   // Comunication Lost...
    TmrLastRequest.TimeOut = 0;    
}

void Process_Cmd(volatile TypBuffer * LastCmd)
{
	if(LastCmd->Valid_Data == 1)
	{
		char Esito = 0;
       
#if(K_Address_Reply>0&&K_Address_Reply!=K_Address_MasterDefault)
        if((LastCmd->Mitt_Add==K_Address_Consolle) || (LastCmd->Mitt_Add==K_Address_MasterDefault-K_Address_Reply) || (Me.I_Am_SubSlave==1))
#elif(K_Address_Reply==K_Address_MasterDefault)
        if((LastCmd->Mitt_Add==K_Address_Consolle) || (LastCmd->Mitt_Add==K_Address_MasterDefault) || (Me.I_Am_SubSlave==1))            
#endif
        {
            switch (LastCmd->Comando)	// interpreto il comando
            {
                case Prg:
                    while(BufferTx.Valid_Data == 1)	// attendo 
                        continue;
                    if(LastCmd->Dest_Add != k_Broadcast_Addr)	// Se non è stato ricevuto in broadcast, rispondo al mittente
                        Write_Cmd(&BufferTx,LastCmd->Mitt_Add, LastCmd->Comando, 0, LastCmd->Data);
                    break;

                case ReadReg:
                    while(BufferTx.Valid_Data == 1)	// attendo 
                        continue;
                    if(LastCmd->Dest_Add != k_Broadcast_Addr)	// Se non è stato ricevuto in broadcast, rispondo al mittente
                        Write_Cmd(&BufferTx,LastCmd->Mitt_Add, LastCmd->Comando, LastCmd->Registro, GetData(LastCmd->Registro));
                    break;

                case WriteReg:
                    while(BufferTx.Valid_Data == 1)	// attendo 
                        continue;
                    Esito = PutData(LastCmd->Registro, LastCmd->Data);	// Tento la scrittura del registro
                    if(LastCmd->Dest_Add != k_Broadcast_Addr)	// Se non è stato ricevuto in broadcast, rispondo al mittente
                    {
                        if(Esito == 0)			// in base all'esito rispondo
                        {	Write_Cmd(&BufferTx,LastCmd->Mitt_Add, LastCmd->Comando, LastCmd->Registro, Ack);	}
                        else
                        {	Write_Cmd(&BufferTx,LastCmd->Mitt_Add, LastCmd->Comando, LastCmd->Registro, N_Ack);	}
                    }
                    break;

                case WriteRegBrdCst:
                case WriteSplitRegBrdCst:
                    Esito = PutData(LastCmd->Registro, LastCmd->Data);	// Tento la scrittura del registro
                    //DEBUGGONE
                    //Me.BroadcastRxCounter++;
                    //DEBUGGONE
                    break;

                case Check:
                    while(BufferTx.Valid_Data == 1)	// attendo 
                        continue;
                    Write_Cmd(&BufferTx,LastCmd->Mitt_Add, LastCmd->Comando, 0, Ack);
                    break;

            }
            ResetComFlags();        
        }
		LastCmd->Valid_Data = 0;
	}
}

int	GetData(volatile int Reg)
{
	int retVal = 0;
    int temp = 0;    

	switch(Reg)
	{
		case REG_SPLIT_TEMP_BULBO_GRANDE:			// valori sonde già convertiti
			return Me.Temperature.Gas;
		case REG_SPLIT_TEMP_BULBO_PICCOLO:
			return Me.Temperature.Liquid;
		case REG_SPLIT_TEMP_ARIA_USCITA:
			return Me.Temperature.AirOut;
		case REG_SPLIT_TEMP_AMBIENTE:
			return Me.Temperature.Ambient;
		case REG_SPLIT_PRES_BULBO_GRANDE:
			return Me.Pres_Big;
		case REG_SPLIT_PRES_BULBO_PICCOLO:
			return Me.Pres_Small;
		//case REG_SPLIT_I_FAN:
			//return Me.Fan_Power;
		case REG_SPLIT_TEMP_AMB_MIDDLE:
			return Me.TempAmbMiddle;
		case REG_SPLIT_TEMP_AMB_REAL:
			return Me.TempAmbTouchVisual;
			

		case REG_SPLIT_TEMP_BULBO_GRANDE_ADC:		// valori grezzi acquisizione adc
			return Adc_Temp[1]; //Adc_T_Gas;
		case REG_SPLIT_TEMP_BULBO_PICCOLO_ADC:
			return Adc_Temp[2]; //Adc_T_Liquid;
		case REG_SPLIT_TEMP_ARIA_USCITA_ADC:
			return Adc_Temp[3]; //Adc_T_Out;
		case REG_SPLIT_TEMP_AMBIENTE_ADC:
			return Adc_Temp[4]; //Adc_T_Amb;
		case REG_SPLIT_PRES_BULBO_GRANDE_ADC:
			return Adc_Temp[5]; //Adc_P_Grande;
		case REG_SPLIT_PRES_BULBO_PICCOLO_ADC:
			return Adc_Temp[6]; //Adc_P_Piccolo;
		case REG_SPLIT_I_FAN_ADC:
			return Adc_Temp[7]; //Adc_I_Fan;

		case REG_SPLIT_DIGIT_INPUT:					// stato digital input
			return Me.Digi_In;
		case REG_SPLIT_DIGIT_OUTPUT:				// stato digital out
			return Me.Digi_Out;

		case REG_SPLIT_PWM_REQ_FAN_VALUE:			// gestione ventola
			return Me.Pwm_Req;
		case REG_SPLIT_PWM_ACT_FAN_VALUE:
			return Me.Pwm_Value;
		case REG_SPLIT_FAN_TACHO:
#if(SplitMode==K_I_Am_LoopWater)                
#if(K_LoopDoublePumpEnable==1)    
            return iDigitPumpLoop1 | (Me.Error.Flow<<1) | (K_LoopDoublePumpEnable<<2) | (iDigitPumpLoop2<<3) | (Me.Error.Flow2<<4);
#else
            return iDigitPumpLoop1 | (Me.Error.Flow<<1);
#endif
#else
			return Me.Fan_Tacho;
#endif
		case  REG_SPLIT_VENTIL_MODE:				// gestione modalità operativa
			return Me.Ventil_Mode;
		case REG_SPLIT_FUNCTION_MODE:
//#if(SplitMode!=K_I_Am_InverterFAN && SplitMode!=K_I_Am_StandardFAN && SplitMode!=K_I_Am_OnlyTouchComm)
#if(SplitMode!=K_I_Am_OnlyTouchComm)
            if(EnableMySecBus)
                return Me.MyOrSubSplitMode;
            else
                return Me.Function_Mode;
#else
            return 0;
#endif
		case REG_SPLIT_CURRENT_SP:
			return Me.SP;
		case REG_SPLIT_CURRENT_EVAP_TEMP:
            if(EnableMySecBus)
                return Me.MyOrSubSplitEvap_Temp;
            else
                return Me.Evap_Temp; 
            
		case REG_SPLIT_ONLY_FAN:
#if(SplitMode==K_I_Am_InverterFAN || SplitMode==K_I_Am_ManualRegulation || SplitMode==K_I_Am_StandardFAN || SplitMode==K_I_Am_Atex_Fan || SplitMode==K_I_Am_On_Off_Fan)
#if(K_DiffPress_Auto_Fan_Control==1)
            return 4;
#elif(K_DiffPress_Fan_Control==1)
            return 3;
#elif(K_AutoTempFanControl==1)
            return 2;
#else
            return 1;
#endif
#else
            return 0; 
#endif

		case REG_SPLIT_SET_CMD_DISABLE:
#if(SplitMode==K_I_Am_ManualRegulation || K_Channel_Gas_Regulation==1 || K_DiffPress_Auto_Fan_Control==1)
            return 1;
#else
            return 0; 
#endif            

		case REG_SPLIT_UTA_MODE:
			return Touch[0].Uta_Mode;
            
		case REG_SPLIT_FAN_MODE:
			return Touch[0].FanMode;

		case REG_SPLIT_DEFINE_MODE:
			//return Me.DefMode;
            return Touch[0].DefMode;
        case REG_SPLIT_POWER:
            return Touch[0].Script_Split_Pwr;    

		case REG_SPLIT_HEATER_PWR:
			return Touch[0].HeaterPwr;            
						
		case REG_SPLIT_CUMULATIVE_ALARM:
			return Me.CumulativeAlarm;			
			
		case REG_SPLIT_CURRENT_EVAP_PRESURE:
			return Me.Evap_Press;
            
 		case REG_SPLIT_WR_OTHER_CABIN_NUMBER:	
			return Me.OtherCabin.Number;	   
            
 		case REG_SPLIT_OTHER_CABIN_CMD_ON_OFF:	
			return Me.OtherCabin.CMD_On_Off;	   

 		case REG_SPLIT_OTHER_CABIN_CMD_SETP:	
			return Me.OtherCabin.CMD_SetP;	   

 		case REG_SPLIT_OTHER_CABIN_CMD_SETP_F:	
			return Me.OtherCabin.CMD_SetP_F;	   

 		case REG_SPLIT_OTHER_CABIN_CMD_FAN:	
			return Me.OtherCabin.CMD_Fan;	
            
 		case REG_SPLIT_OTHER_CABIN_CMD_UTA_MODE:	
			return Me.OtherCabin.CMD_UTAMode;	   

 		case REG_SPLIT_RD_EXTRACTOR_NUMBER:	
			return Me.Extractor.Number;

 		case REG_SPLIT_RD_DELTAVEL_EXTRACTOR:
			return Me.Extractor.DeltaVel;            
                        
 		case REG_SPLIT_OTHER_CABIN_CMD_SEND:
            if(Me.OtherCabin.CMD_Send!=0)   //Se è attiva la richiesta di send, azzero sul touch il bit e passo il dato al master per inizializzare.
                Me.OtherCabin.ReadOk=1;
			return Me.OtherCabin.CMD_Send;
            
		case REG_SPLIT_SYSTEM_DISABLE:          
                return EngineBox.SystemDisable;
            
        case REG_SPLIT_DOUBLE_PUMP_SWITCH_TIME:
            return EngineBox.DoublePumpSwitchTime;
            
        case REG_SPLIT_DOUBLE_PUMP_SELECTION:
            return EngineBox.DoublePumpSelection;
            
        case REG_SPLIT_MANUAL_PUMP_ABIL:
            return EngineBox.Manual_Pump_Abil;

        case REG_SPLIT_MANUAL_PUMP_FREQ:
            return EngineBox.Manual_Pump_Freq;            
                                               
		case REG_SPLIT_VALVOLA_CUR_POS:				// gestione valvola
			return Me.ExpValve_Act;
            
		case REG_SPLIT_VALVOLA_CUR_POS_PERC:				// gestione valvola
            if(!Me.I_Am_SubSlave)
                temp = ValueToPercent(Me.ExpValve_Act, K_MaxExcursionValve) + Me.SubSlaveSumPercValve;
            else
                temp = ValueToPercent(Me.ExpValve_Act, K_MaxExcursionValve);
			return temp;
            
		case REG_SPLIT_VALVOLA_PID_COOL_PERC_POS:           // gestione valvola
            if(!Me.I_Am_SubSlave)
                temp = ValueToPercent(Me.ExpValve_Pid_Cooling, K_MaxExcursionValve) + Me.SubSlaveSumValvePidCooling;
            else
                temp = ValueToPercent(Me.ExpValve_Pid_Cooling, K_MaxExcursionValve);
			return temp;
            
        case REG_SPLIT_FAN_POWER:            
            return Me.Fan_Power;
            
        case REG_SPLIT_AND_SUB_FAN_POWER:
#if(SplitMode==K_I_Am_Freezer && K_AbilSplitDefrostingCycle==1)    
            return (unsigned int)(round((float)Me.Fan_Power/100.0)) + Me.SubSlaveTotalFanPower + Me.OutDefrosPower;
#else            
            return (unsigned int)(round((float)Me.Fan_Power/100.0)) + Me.SubSlaveTotalFanPower;
#endif
            
        case REG_SPLIT_AND_SUB_AIR_POWER_OUTPUT:
            return Me.AirPowerOutput + Me.SubSlaveTotalAirPowerOutput;

        case REG_SPLIT_AND_SUB_AIR_POWER_BTU:
            return Me.AirPowerBTU + Me.SubSlaveTotalAirPowerBTU;
            
        case REG_SPLIT_FAN_CURRENT:      
            return Me.Fan_Current;  
            
        case REG_SPLIT_SUPPLY_VOLTAGE:      
            return Me.Supply_Voltage;
            
        case REG_SPLIT_HEATER_ABIL:      
#if(K_Heater_Abil==1)
            return 1;
#else
            return 0;
#endif 
            
		case REG_SPLIT_VALVOLA_SET_POS:
			return Me.ExpValve_Req;
		case REG_SPLIT_VALVOLA_ERR_POS:				// gestione valvola
			return Me.ExpValve_Err;
		case REG_SPLIT_VALVOLA_ERR_COD:				// gestione valvola
			return Me.ExpValve_Err_Code;
		case REG_SPLIT_VALV_STP_REF_TIME:
			return StepRefreshTime;
		case REG_SPLIT_VALV_MAX_STEP:
			return ValvolaMaxStepNum;
		case REG_SPLIT_CORRENTE_MAX_VALV:
			return 0x0000;
		case REG_SPLIT_CORRENTE_MIN_VALV:
			return 0x0000;
		case REG_SPLIT_THR_MIN_AMP_VALV:
			return 0x0000;
		case REG_SPLIT_THR_MAX_AMP_VALV:
			return 0x0000;


		case REG_SPLIT_ENGINE_BOX_COMPRESSOR_SPEED:	// gestione info box motore
			return EngineBox.Compressor_Speed;
		case REG_SPLIT_ENGINE_BOX_WATER_PUMP_SPEED:
			return EngineBox.Pump_Speed;
		case REG_SPLIT_ENGINE_BOX_FLUX:
			return EngineBox.Water_Flux;
		case REG_SPLIT_CURRENT_MODE_ENGINE_BOX:
			return EngineBox.FunctionMode;
		case REG_SPLIT_CURRENT_REQ_PRESSURE:
			return EngineBox.Ric_Pressione;
		case REG_SPLIT_ENGINE_BOX_T_COMP_OUT:
			return EngineBox.Temp_Mandata;
		case REG_SPLIT_ENGINE_BOX_T_COMP_SUCTION:
			return EngineBox.Temp_Ritorno;
		case REG_SPLIT_ENGINE_BOX_T_SEA_WATER:
			return EngineBox.Temp_Acqua_Mare;
		case REG_SPLIT_ENGINE_BOX_T_CONDENSER:
			return EngineBox.Temp_Condensatore;
		case REG_SPLIT_ENGINE_BOX_T_GAS:
			return EngineBox.Temp_Collettore_Grande;
		case REG_SPLIT_ENGINE_BOX_T_LIQUID:
			return EngineBox.Temp_Collettore_Piccolo;
		case REG_SPLIT_ENGINE_BOX_P_GAS:
			return EngineBox.Pressione_Gas_G;
		case REG_SPLIT_ENGINE_BOX_P_LIQUID:
			return EngineBox.Pressione_Liq_P;
		case REG_SPLIT_ENGINE_BOX_PRESSURE_VRV_BOX:
			return EngineBox.Pressione_VRV_Box;
		case REG_SPLIT_ENGINE_BOX_PRESSURE_AUX:
			return EngineBox.Pressione_Aux;
		case REG_SPLIT_ENGINE_BOX_RELE_SATUS:
			return EngineBox.ReleSt;
		case REG_SPLIT_ENGINE_BOX_INPUT_STATUS:
			return EngineBox.InputSt;
		case REG_SPLIT_ENGINE_BOX_SYS_ID_LO:
			return EngineBox.SysNumLo;
		case REG_SPLIT_ENGINE_BOX_SYS_ID_HI:
			return EngineBox.SysNumHi;
		case REG_SPLIT_ERRORE_1_BOX_MOTORE:
			return EngineBox.Errori1;
		case REG_SPLIT_ERRORE_2_BOX_MOTORE:
			return EngineBox.Errori2;
		case REG_SPLIT_ERRORE_PERS1_BOX_MOTORE:
			return EngineBox.ErroriPers1;
		case REG_SPLIT_ERRORE_PERS2_BOX_MOTORE:
			return EngineBox.ErroriPers2;
		case REG_SPLIT_STATUS_BOX_MOTORE:
			return EngineBox.StatoCompressore;
		case REG_SPLIT_ID_SPLIT_MASTER:
			return EngineBox.IdMasterSplit;
		case REG_SPLIT_POWER_LIMIT:
			return EngineBox.PowerLimit;
		case REG_SPLIT_POWER_LIMIT_REQ:
			return Me.PowerLimit;
		case REG_SPLIT_STATUS:
			if(Me.Error.ClearErrReq == 1 || Me.Error.ClearDiagnosticSplitErrReq == 1 || Me.Error.ClearComErr == 1)
                Me.Error.ReadMasReset = 1;
			return Me.SplitStatus;
		case REG_SPLIT_ENGINE_BOX_CYCLE_TIME:
			return EngineBox.SystemCycleTime;
		case REG_SPLIT_ENGINE_BOX_HOURS_OF_LIFE:
			return EngineBox.HoursOfLife;

		case REG_DIAG_SUB_SPLIT_QUANTITY:
			return DiagnosticSplit.Sub_Slave_Quantity;
		case REG_DIAG_SPLIT_TEST_ADDRESS:			// #v16
			return DiagnosticSplit.Address;			// Indirizzo slave da monitorare
		case REG_DIAG_SPLIT_STATUS:					
			return DiagnosticSplit.SplitStatus;		
		case REG_DIAG_SPLIT_LIQ_TEMP:					
			return DiagnosticSplit.Temp_Liq;	
		case REG_DIAG_SPLIT_GAS_TEMP:					
			return DiagnosticSplit.Temp_Gas;	
		case REG_DIAG_SPLIT_AIR_IN:					
			return DiagnosticSplit.Temp_AirIn;	
		case REG_DIAG_SPLIT_AIR_OUT:					
			return DiagnosticSplit.Temp_AirOut;	
		case REG_DIAG_SPLIT_FAN_SPEED:					
			return DiagnosticSplit.FanSpeed;	
		case REG_DIAG_SPLIT_EXP_VALVE:					
			return DiagnosticSplit.Exp_Valve;	
		case REG_DIAG_SPLIT_GAS_PRESSURE:					
			return DiagnosticSplit.Gas_Pressure;	
		case REG_DIAG_SPLIT_LIQ_PRESSURE:					
			return DiagnosticSplit.Liq_Pressure;	
		case REG_DIAG_SPLIT_REQUEST_AIR_TEMP:					
			return DiagnosticSplit.Req_AirTemp;	
		case REG_DIAG_SPLIT_REQUEST_AIR_PRES:					
			return DiagnosticSplit.Req_Pressure;	
		case REG_DIAG_SPLIT_ENG_MODE:					
			return DiagnosticSplit.Split_Eng_Mode;
		case REG_DIAG_SPLIT_CYCLE_TIME:					
			return Me.CycleTime;
            
        case REG_DIAG_SPLIT_FAN_POWER:
            return DiagnosticSplit.Fan_Power;
        case REG_DIAG_SPLIT_FAN_CURRENT:      
            return DiagnosticSplit.Fan_Current;            
        case REG_DIAG_SPLIT_SUPPLY_VOLTAGE:      
            return DiagnosticSplit.Supply_Voltage;            

		case REG_DIAG_INVERTER_ALARM:	
			return DiagnosticInverter.Alarm;		
		case REG_DIAG_INVERTER_OUT_FREQ:
			return DiagnosticInverter.OutFreq;				
		case REG_DIAG_INVERTER_STATUS:		
			return DiagnosticInverter.Status;		 
		case REG_DIAG_INVERTER_OUT_CURRENT:	
			return DiagnosticInverter.OutCurr;	
		case REG_DIAG_INVERTER_IN_VOLTAGE:	
			return DiagnosticInverter.InVolt;	
		case REG_DIAG_INVERTER_OUT_VOLTAGE:	
			return DiagnosticInverter.OutVolt;	
		case REG_DIAG_INVERTER_IN_POWER:	
			return DiagnosticInverter.InPower;
		case REG_DIAG_INVERTER_OUT_POWER:	
			return DiagnosticInverter.OutPower;
		case REG_DIAG_INVERTER_REQ_FREQ:	
			return DiagnosticInverter.ReqFreq;
		case REG_DIAG_INVERTER_MAX_REQ_FREQ:	
			return DiagnosticInverter.MaxReqFreq;

		case REG_SPLIT_PWM_VALUE_OFF:				// valori predefiniti delle velocità
			return PWMValue[0];
		case REG_SPLIT_PWM_VALUE_NIGHT:
			return PWMValue[1];
		case REG_SPLIT_PWM_VALUE_SPEED1:
			return PWMValue[2];
		case REG_SPLIT_PWM_VALUE_SPEED2:
			return PWMValue[3];
		case REG_SPLIT_PWM_VALUE_SPEED3:
			return PWMValue[4];
		case REG_SPLIT_PWM_VALUE_SPEED4:
			return PWMValue[5];
		case REG_SPLIT_PWM_VALUE_SPEED5:
			return PWMValue[6];
		case REG_SPLIT_PWM_VALUE_SPEED6:
			return PWMValue[7];
		case REG_SPLIT_PWM_VALUE_SPEED7:
			return PWMValue[8];

            			
		case REG_SUB_SPLIT_EVAP_TEMP_VALVE:	
			return Room[DiagnosticSplit.SubAddress].EvapTempValve;
		case REG_SUB_SPLIT_SETP_SUPERHEAT:	
			return Room[DiagnosticSplit.SubAddress].SetP_Superheat;
		case REG_SUB_SPLIT_CUMULATIVE_ALARM:	
			return Room[DiagnosticSplit.SubAddress].CumulativeAlarm;
		case REG_SUB_SPLIT_SUPERHEAT:	
			return Room[DiagnosticSplit.SubAddress].Superheat;
		case REG_SUB_SPLIT_SUPERHEAT_RISCALDAMENTO:	
			return Room[DiagnosticSplit.SubAddress].SuperheatRiscaldamento;			
//		case REG_SUB_SPLIT_RES_REQ_ADDRESS:
//			return Room[DiagnosticSplit.SubAddress].SplitStatus;
		case REG_SUB_SPLIT_TEST_ADDRESS:	
			return DiagnosticSplit.SubAddress;		
		case REG_SUB_SPLIT_STATUS:	
			return Room[DiagnosticSplit.SubAddress].SplitStatus;	

		case REG_SUB_SPLIT_LIQ_TEMP:		
			return Room[DiagnosticSplit.SubAddress].Temp_Small;
		case REG_SUB_SPLIT_GAS_TEMP:		
			return Room[DiagnosticSplit.SubAddress].Temp_Big;		
		case REG_SUB_SPLIT_AIR_IN:		
			return Room[DiagnosticSplit.SubAddress].Temp_Amb;		
		case REG_SUB_SPLIT_AIR_OUT:		
			return Room[DiagnosticSplit.SubAddress].Temp_AirOut;		
		case REG_SUB_SPLIT_FAN_SPEED:
			return Room[DiagnosticSplit.SubAddress].FanSpeed;
		case REG_SUB_SPLIT_EXP_VALVE:
			return Room[DiagnosticSplit.SubAddress].ExpValve_Act;		
		case REG_SUB_SPLIT_EXP_VALVE_ERR_POS:
			return Room[DiagnosticSplit.SubAddress].ExpValve_Err;		
		case REG_SUB_SPLIT_EXP_VALVE_ERR_COD:
			return Room[DiagnosticSplit.SubAddress].ExpValve_Err_Code;		
		case REG_SUB_SPLIT_GAS_PRESSURE:		
			return Room[DiagnosticSplit.SubAddress].Pres_Big;		
		case REG_SUB_SPLIT_LIQ_PRESSURE:	
			return Room[DiagnosticSplit.SubAddress].Pres_Small;		
		case REG_SUB_SPLIT_REQUEST_AIR_TEMP:
			return Room[DiagnosticSplit.SubAddress].Evap_Temp;		
		case REG_SUB_SPLIT_REQUEST_AIR_PRES:
			return Room[DiagnosticSplit.SubAddress].Evap_Press;		
		case REG_SUB_SPLIT_ENG_MODE:		
			return EngineBox.FunctionMode;		
		case REG_SUB_SPLIT_CYCLE_TIME:			
			return	Room[DiagnosticSplit.SubAddress].CycleTime; 
		case REG_SUB_SPLIT_FUNCTION_MODE:			
			return	Room[DiagnosticSplit.SubAddress].Function_Mode;             
		case REG_SUB_SPLIT_QUANTITY:
			return Me.Sub_Slave_Quantity;
		case REG_SPLIT_EVAP_TEMP_VALVE:
			return Me.EvapTempValve;	
            
		case REG_SUB_SPLIT_FAN_POWER:
			return Room[DiagnosticSplit.SubAddress].Fan_Power;		
		case REG_SUB_SPLIT_FAN_CURRENT:
			return Room[DiagnosticSplit.SubAddress].Fan_Current;		
		case REG_SUB_SPLIT_SUPPLY_VOLTAGE:
			return Room[DiagnosticSplit.SubAddress].Supply_Voltage;		

        case REG_SUB_SPLIT_RD_TEMPERATURE_PROBE_ERROR:
            return Room[DiagnosticSplit.SubAddress].Temperature_Probe_error;
			
		case REG_SPLIT_POWER_ECOMODE:
			return Me.Power_EcoMode;			
		case REG_SPLIT_ECOMODE_ENABLE:
			return Me.EcoModeEnable;			
		case REG_DIAG_INVERTER_ADDRESS:
			return DiagnosticInverter.DiagAddress;		
            
        case REG_SPLIT_HW_VERSION:
            return Me.HW_Version;	            
        case REG_SPLIT_FW_VERSION_HW:
		    return Me.FW_Version_HW;		            
        case REG_SPLIT_FW_VERSION_FW:
            return Me.FW_Version_FW;            
        case REG_SPLIT_FW_REVISION_FW:        
            return Me.FW_Revision_FW;	        

		case REG_SPLIT_SUPERHEAT:	    
			return Me.Superheat;
        case REG_SPLIT_SUPERHEAT_RISCALDAMENTO:	    
			return Me.SuperheatRiscaldamento;	
            
        case REG_SPLIT_SETP_SUPERHEAT:
            if(EngineBox.FunctionMode==EngineBox_Caldo || EngineBox.FunctionMode == EngineBox_GoCaldo || EngineBox.FunctionMode_C2==EngineBox_Caldo || EngineBox.FunctionMode_C2==EngineBox_GoCaldo)
                return K_SetPoint_SuperHeatRiscaldamento;
            else
                return ValvePID.SetP;
            
        case REG_SUB_SPLIT_FLAGS_ENABLE_1_16:	    
			return Me.Room.Split_Flags_Enable_1_16;	
        case REG_SUB_SPLIT_FLAGS_OFFLINE_1_16:	    
			return Me.Room.Split_Flags_Offline_1_16;	
        case REG_SUB_SPLIT_FLAGS_ONLINE_1_16:	    
			return Me.Room.Split_Flags_Online_1_16;	 
        case REG_SUB_SPLIT_FLAGS_ALARM_1_16:	    
			return Me.Room.Split_Flags_Alarm_1_16;	

		case REG_SUB_SPLIT_SUPERH_COOL_PID_ERROR:
			return Room[DiagnosticSplit.SubAddress].SuperHeat_Pid_Cool_Error;		

        case REG_SPLIT_UTA_ABIL:
#if(K_UTAEnable==1)
            return 1;
#else
            return 0;
#endif            
               
        case REG_SUB_SPLIT_HW_VERSION:
            return Room[DiagnosticSplit.SubAddress].HW_Version;	            
        case REG_SUB_SPLIT_FW_VERSION_HW:
		    return Room[DiagnosticSplit.SubAddress].FW_Version_HW;		            
        case REG_SUB_SPLIT_FW_VERSION_FW:
            return Room[DiagnosticSplit.SubAddress].FW_Version_FW;            
        case REG_SUB_SPLIT_FW_REVISION_FW:        
            return Room[DiagnosticSplit.SubAddress].FW_Revision_FW;	             
        case REG_SUB_SPLIT_MODEL_VERSION:        
            return Room[DiagnosticSplit.SubAddress].Model_FW_Version;	
            
        case REG_SUB_SPLIT_FAN_M3H:        
            return Room[DiagnosticSplit.SubAddress].Fan_M3h;	         

        case REG_SUB_SPLIT_AIR_POWER_OUTPUT:        
            return Room[DiagnosticSplit.SubAddress].AirPowerOutput;	         

        case REG_SUB_SPLIT_AIR_POWER_BTU:        
            return Room[DiagnosticSplit.SubAddress].AirPowerBTU;	
            
        case REG_SUB_SPLIT_READ_HUMIDITY:
            return Room[DiagnosticSplit.SubAddress].Humidity;                       

        case REG_SPLIT_TO_INIT_SP_ROOM:        
            return Reinit.ToEngBox.SetPoint;	             
        case REG_SPLIT_TO_INIT_FAN_SPEED:        
            return Reinit.ToEngBox.FanSpeed;	             
        case REG_SPLIT_TO_INIT_POWER:        
            return Reinit.ToEngBox.Power;	             
        case REG_SPLIT_TO_INIT_SP_ROOM_F:        
            return Reinit.ToEngBox.SetPoint_F;	
       case REG_SPLIT_TO_INIT_DEFMODE:        
            return Reinit.ToEngBox.DefMode;				
        case REG_SPLIT_TO_TRIGGER_INIT:        
            return Reinit.ToEngBox.TrigInit;	
			
        case REG_SPLIT_SYNC_RD_SETP:        
            return Syncronize.Engine.SetPoint;	             			
        case REG_SPLIT_SYNC_RD_FANMODE:        
            return Syncronize.Engine.FanMode;	             					
        case REG_SPLIT_SYNC_RD_SETPF:        
            return Syncronize.Engine.Script_SetPoint_F;	             						
        case REG_SPLIT_SYNC_RD_PWR:        
            return Syncronize.Engine.Script_Split_Pwr;	      

       case REG_SPLIT_SYNC_RD_UTA_MODE:        
            return Syncronize.Engine.Uta_Mode;	      

       case REG_SPLIT_SYNC_RD_HEATER_PWR:        
            return Syncronize.Engine.HeaterPwr;
			
        case REG_SPLIT_SYNC_RD_ECOMEN:        
            return Syncronize.Engine.EcoModeEnable;	             			
        case REG_SPLIT_SYNC_RD_PWRECOM:        
            return Syncronize.Engine.Power_EcoMode;	             						
			
        case REG_DIAG_SPLIT_RD_SET_POWER:		
            return Syncronize.Engine.DiagWritePwr;	             						
        case REG_DIAG_SPLIT_RD_SET_SETPOINT:	
            return Syncronize.Engine.DiagWriteSetPoint;	             						
        case REG_DIAG_SPLIT_RD_SET_FANSPEED:	
            return Syncronize.Engine.DiagWriteFanMode;	             						
        case REG_DIAG_SPLIT_RD_SET_SETPOINT_F:
            return Syncronize.Engine.DiagWriteSetPoint_F;	         

        case REG_DIAG_SPLIT_RD_HEATER_PWR:
            return Syncronize.Engine.DiagWriteHeaterPwr;	         
            
       case REG_DIAG_SPLIT_RD_UTA_MODE:
            return Syncronize.Engine.DiagUta_Mode;	         
			
		case REG_SPLIT_RD_MODE:
			return Syncronize.Engine.DefMode;			
		case REG_SPLIT_RD_UTA_POWER:
			return Syncronize.Engine.Uta_Power;			
		//case REG_SPLIT_RD_UTA_MODE:
		//	return Syncronize.Engine.Uta_Mode;			
		case REG_SPLIT_RD_UTA_SP_ROOM:
			return Syncronize.Engine.Uta_SP_Room;			
		case REG_SPLIT_RD_UTA_SP_HUMI:
			return Syncronize.Engine.Uta_SP_Humi;			
		case REG_SPLIT_RD_UTA_FAN_SPEED:
			return Syncronize.Engine.Uta_Fan_Speed;			
		case REG_SPLIT_RD_UTA_SP_ROOM_F:
			return Syncronize.Engine.Uta_SP_Room_F;				
		
		case REG_SPLIT_RD_SYSTEM_DISABLE:
			return Syncronize.Engine.SystemDisable;				

		case REG_DIAG_SPLIT_CMD_ADDRESS:
            return DiagnosticSplit.CmdAddress;
/*             
            if(((Touch[0].DiagWrite.Pwr == DiagnosticSplit.Ready_PowerOn) &&
               ((Touch[0].DiagWrite.SetPoint == DiagnosticSplit.Ready_SetPoint) || (DiagnosticSplit.Only_Fan==1)) &&
               ((Touch[0].DiagWrite.FanMode == DiagnosticSplit.Ready_FanMode) || (DiagnosticSplit.FrozenMode!=0) || ((DiagnosticSplit.ModDevType==K_ModDeviceNetB)&&(DiagnosticSplit.Ready_PowerOn==0))) &&
               ((Touch[0].DiagWrite.Uta_Mode == DiagnosticSplit.Ready_UtaMode) || (DiagnosticSplit.UTA_Abil==0)) &&     
               //(Touch[0].DiagWrite.SetPoint_F == DiagnosticSplit.Ready_SetPoint_F) &&     
               (DiagnosticSplit.CmdAddress != 0))||DiagnosticSplit.OffLine || TimerMaxTimeCmdAddress.TimeOut) 
            {
                
                DiagnosticSplit.OldValueCmdAddress = DiagnosticSplit.CmdAddress;
                TimerMaxTimeCmdAddress.Value = 0;
                TimerMaxTimeCmdAddress.TimeOut = 0;                
                return DiagnosticSplit.CmdAddress;	
            }
            else
            {
                return DiagnosticSplit.OldValueCmdAddress;
            }	
*/            			
		case REG_SPLIT_UTA_CLEAR_ERROR_BOX:
			retVal = EngineBox.UTA_H.ClearError;
			EngineBox.UTA_H.ClearError = 0;
			return retVal;	
            
        case REG_SPLIT_WR_ON_PRIORITY_MODE:
			return Touch[0].On_Priority_Mode;
                
        case REG_SPLIT_WR_VALVE_ONDEMAND_MIN_PERC_VAL:
			return Touch[0].ValveOnDemand_Min_Percent_Val;
                
        case REG_SPLIT_WR_MAX_LIM_ONDEMAND_TOTAL_SCORE:
			return Touch[0].Max_Lim_ON_Demand_Total_Score;
                
        case REG_SPLIT_WR_TIME_ONDEMAND_VALVE_SWITCH:
			return Touch[0].TimeOnDemandValveSwitch;
                    		
        case REG_SPLIT_MODDEV_TYPE:                     // TYPE: 0=Nessun Device, 1=Touch, 2=NetBuilding, 3=SyxthSense
            return Me.ModDeviceType;
            
        case REG_SPLIT_MODDEV_FLAGS_STATUS:             // Status: // Status: Bit3=ENABLE, Bit2=OFFLINE, Bit1=ONLINE, Bit0=Alarm
            return Me.ModDeviceStatus;            
            
            
        case REG_SPLIT_RD_TESTALL_ABIL:
            return Touch[0].TestAll_Abil;
            
        case REG_SPLIT_RD_TESTALL_SUPERHEAT_SP_COOL:
            return Touch[0].TestAll_SuperHeat_SP_Cool;
            
        case REG_SPLIT_RD_TESTALL_SUPERHEAT_SP_HEAT:
            return Touch[0].TestAll_SuperHeat_SP_Heat;            

        case REG_SPLIT_RD_TESTALL_MAX_OPEN_VALVE_COOL:
            return Touch[0].TestAll_MaxOpValve_Cool;
            
        case REG_SPLIT_RD_TESTALL_MAX_OPEN_VALVE_HEAT:
            return Touch[0].TestAll_MaxOpValve_Heat;
            			
        case REG_SPLIT_RD_TESTALL_WORK_TEMP_COOL:
            return Touch[0].TestAll_TempBatt_SP_Cool;
            
        case REG_SPLIT_RD_TESTALL_WORK_TEMP_HEAT:
            return Touch[0].TestAll_TempBatt_SP_Heat;
            
        case REG_SPLIT_RD_FROZEN_MODE:
            return Me.Frozen.Mode;

        case REG_SPLIT_RD_TESTALL_FROZEN_ABIL:
            return Touch[0].TestALL_Frozen_Abil;
            
        case REG_SPLIT_RD_TESTALL_SUPERHEAT_SP_FRIGO:
            return Touch[0].TestAll_SuperHeat_SP_Frigo;
            
        case REG_SPLIT_RD_TESTALL_SUPERHEAT_SP_FREEZER:
            return Touch[0].TestAll_SuperHeat_SP_Freezer;
            
        case REG_SPLIT_RD_TESTALL_MAX_OPEN_VALVE_FRIGO:
            return Touch[0].TestAll_MaxOpValve_Frigo;
            
        case REG_SPLIT_RD_TESTALL_MAX_OPEN_VALVE_FREEZER:
            return Touch[0].TestAll_MaxOpValve_Freezer;
            
        case REG_SPLIT_RD_TESTALL_WORK_TEMP_FRIGO:
            return Touch[0].TestAll_TempBatt_SP_Frigo;
            
        case REG_SPLIT_RD_TESTALL_WORK_TEMP_FREEZER:
            return Touch[0].TestAll_TempBatt_SP_Freezer;

        case REG_SPLIT_RD_TESTALL_MIN_OPEN_VALVE_COOL:
            return Touch[0].TestAll_MinOpValve_Cool;

        case REG_SPLIT_RD_TESTALL_MIN_OPEN_VALVE_HEAT:
            return Touch[0].TestAll_MinOpValve_Heat;

        case REG_SPLIT_RD_TESTALL_MIN_OPEN_VALVE_FRIGO:
            return Touch[0].TestAll_MinOpValve_Frigo;

        case REG_SPLIT_RD_TESTALL_MIN_OPEN_VALVE_FREEZER:
            return Touch[0].TestAll_MinOpValve_Freezer;
            
        case REG_SPLIT_RD_ABIL_DEFROSTING:
            return Touch[0].Abil_Defrosting;
        
        case REG_SPLIT_RD_DEFROSTING_STEP_TIME:
            return Touch[0].Defrosting_Step_Time;
        
        case REG_SPLIT_RD_DEFROSTING_TIME:
            return Touch[0].Defrosting_Time;
        
        case REG_SPLIT_RD_DEFROSTING_TEMP_SET:
            return Touch[0].Defrosting_Temp_Set;
        
        case REG_SPLIT_RD_DRIPPING_TIME:
            return Touch[0].Dripping_Time;
            
        case REG_SPLIT_RD_VALVOLAFROZEN:
            return Touch[0].Valvola_Frozen;
            
        case REG_SPLIT_RD_MAX_VENTIL_MODE:
#if(SplitMode==K_I_Am_InverterFAN || SplitMode==K_I_Am_StandardFAN)
#if(K_DiffPress_Fan_Control==1 || K_AutoTempFanControl==1)
            return 9;
#else
        if(Me.Extractor.Abil!=0)
            return 9;
        else
            return 8;
#endif
#else
            return Me.MaxFanVentil_Mode;
#endif

        case REG_DIAG_SPLIT_RD_MIN_SET_POINT:
            return K_LimLo_SPLIT_SP;
            
        case REG_DIAG_SPLIT_RD_MAX_SET_POINT:
            return K_LimHi_SPLIT_SP;

        case REG_SPLIT_READ_HUMIDITY:
#if(K_Humidity_Read==1)
            return Me.Humidity;           
#else
            return 0;           
#endif

        case REG_SPLIT_READ_HUMIDITY_MIDDLE:
            if(EnableMySecBus)
                return Me.HumidityMiddle;
            else
                return Me.Humidity;

        case REG_SPLIT_RD_TESTALL_SUPERHEAT_HEAT_PGAIN:
            return Touch[0].TestAll_SuperHeat_Heat_pGain;

        case REG_SPLIT_RD_TESTALL_SUPERHEAT_HEAT_IGAIN:
            return Touch[0].TestAll_SuperHeat_Heat_iGain;

        case REG_SPLIT_RD_TESTALL_SUPERHEAT_HEAT_DGAIN:
            return Touch[0].TestAll_SuperHeat_Heat_dGain;

        case REG_SPLIT_RD_TESTALL_SUPERHEAT_HEAT_TIME_EXEC_PID:
            return Touch[0].TestAll_SuperHeat_Heat_Time_Exec_PID;

        case REG_SPLIT_RD_TESTALL_SUPERHEAT_COOL_PGAIN:
            return Touch[0].TestAll_SuperHeat_Cool_pGain;

        case REG_SPLIT_RD_TESTALL_SUPERHEAT_COOL_IGAIN:
            return Touch[0].TestAll_SuperHeat_Cool_iGain;

        case REG_SPLIT_RD_TESTALL_SUPERHEAT_COOL_DGAIN:
            return Touch[0].TestAll_SuperHeat_Cool_dGain;

        case REG_SPLIT_RD_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID:
            return Touch[0].TestAll_SuperHeat_Cool_Time_Exec_PID;

        case REG_SPLIT_RD_MAINTENANCE_FLAG:
            temp = Touch[0].Maintenance_Flag;
            if((Touch[0].Maintenance_Flag & 0x0002)!=0)
                EngineBox.Maintenance_ReadMasReset = 1;
            return temp;
            
        case REG_SPLIT_RD_TOUCH_MAINTENANCE_SET_TIME:
            return Touch[0].Maintenance_Set_Time;

        case REG_SPLIT_SUPERH_COOL_PID_ERROR:
            return ValvePID.Error;
            
        case REG_SPLIT_RD_SUPERHEAT_COOL_MIN_SETPOINT:
            return Touch[0].SuperHeat_Cool_Min_SetPoint;

        case REG_SPLIT_RD_SUPERHEAT_COOL_MAX_VALUE_CORRECT:
            return Touch[0].SuperHeat_Cool_Max_Value_Correct;

        case REG_SPLIT_RD_SUPERHEAT_HEAT_MIN_SETPOINT:
            return Touch[0].SuperHeat_Heat_Min_SetPoint;
            
        case REG_SPLIT_RD_SUPERHEAT_HEAT_MAX_SETPOINT:
            return Touch[0].SuperHeat_Heat_Max_SetPoint;
            
        case REG_SPLIT_RD_TEMPERATURE_PROBE_ERROR:
            return Me.Temperature.Error.Compact;
            
        case REG_SPLIT_FAN_M3H:
            return Me.Fan_M3h;
            
        case REG_SPLIT_AIR_POWER_OUTPUT:
            return Me.AirPowerOutput;

        case REG_SPLIT_AIR_POWER_BTU:
            return Me.AirPowerBTU;

        case REG_SPLIT_RD_DEHUMI_ABIL:
#if(K_Humidity_Read==1 && K_Humidity_Correct==1)
            return 1;
#else
            return 0;
#endif            
        case REG_SPLIT_RD_SET_TEMP_AIR_OUT:
            return Touch[0].Set_Temp_Air_Out;

        case REG_SPLIT_RD_SET_HUMI:
            return Touch[0].Set_Humi;

        case REG_SPLIT_RD_SET_DELTA_TEMP_MIN_DEHUMI:
            return Touch[0].Set_Delta_Temp_Min_Dehumi;

        case REG_SPLIT_RD_SET_FAN_SPEED_DEHUMI:
            return Touch[0].Set_Fan_Speed_Dehumi;
            
        case REG_SPLIT_RD_SET_PRESS_DEHUMI:
            return Touch[0].Set_Press_Dehumi;

        case REG_SPLIT_RD_SET_TEMP_AIR_OUT_DEHUMI:
            return Touch[0].Set_Temp_Air_Out_Dehumi;
            
        case REG_SPLIT_TOUCH_PAGE:
            return Touch[0].Current_Page;
        case REG_SPLIT_CNTCOMERRSEC:
            return Me.ComError.CntComErrSec;
        case REG_SPLIT_TIMEOUTCOMERRSEC:
            return Me.ComError.TimeOutComErrSec;
        case REG_SPLIT_TIMEOUTINTERCHARCOMERRSEC:
            return Me.ComError.TimeOutInterCharComErrSec;
        case REG_SPLIT_CRC_COMERRSEC:
            return Me.ComError.CRC_ComErrSec;
        case REG_SPLIT_MODCNTCOMERRSEC:
            return Me.ComError.ModCntComErrSec;
        case REG_SPLIT_MODTIMEOUTCOMERRSEC:
            return Me.ComError.ModTimeOutComErrSec;
        case REG_SPLIT_MODTIMEOUTINTERCHARCOMERRSEC:
            return Me.ComError.ModTimeOutInterCharComErrSec;
        case REG_SPLIT_MODCRC_COMERRSEC:
            return Me.ComError.ModCRC_ComErrSec;
            

		case REG_SPLIT_FW_VERSION:						// info generali
			return Versione_Fw;
		case REG_SPLIT_FW_REVISION:
			return Revisione_Fw;
		case REG_SPLIT_MODEL_VERSION:
			return SplitMode;
		case REG_SPLIT_ADD_MODULO:
			return My_Address;
		case REG_SPLIT_SYSTEM_NUMBER_LO:
			return ToInt(SN2, SN1);
		case REG_SPLIT_SYSTEM_NUMBER_HI:
			return ToInt(SN4, SN3);
		case REG_SPLIT_FABRICATION_DAY:
			return GiornoFabbricazione;
		case REG_SPLIT_FABRICATION_MONTH:
			return MeseFabbricazione;
		case REG_SPLIT_FABRICATION_YEAR:
			return AnnoFabbricazione;
		case REG_SPLIT_CRC_16:
			return CRC_Flash;
		case REG_SPLIT_SAVE_2_E2:
			return WriteE2Status;
            
		default:
			return 0xFFFF;
	}
}

// Ritorna 0 se scrittura eseguita (registro esisente), 1 se non eseguito (registro non implementato)
char PutData(volatile int Reg, volatile int Value)
{
    int temp=0;    
	switch(Reg)
	{
		case REG_SPLIT_YOU_ARE_SUBSLAVE:
			Me.I_Am_SubSlave = Value;
			return 0;		
			
		case REG_SPLIT_MY_SPLITMASTER_ADDRESS:
			Me.My_SplitMaster_Address = Value;
			return 0;		
		
		case REG_SPLIT_DIGIT_OUTPUT:		// digital output
			Me.Digi_Out = Value;
			return 0;
/*
		case REG_SPLIT_PWM_REQ_FAN_VALUE:	// gestione ventola
			Me.Pwm_Req = Value;
			return 0;
		case REG_SPLIT_PWM_ACT_FAN_VALUE:
				Me.Pwm_Value = Value;
			return 0;
*/
		case REG_SPLIT_VENTIL_MODE:			// gestione modalità
			Me.Ventil_Mode = Value;
			return 0;
/*            
		case REG_SPLIT_FUNCTION_MODE:
			Me.Function_Mode = Value;
			return 0;
*/
  		case REG_SPLIT_CURRENT_SP:
			Me.SP = Value;
			return 0;
		case REG_SPLIT_CURRENT_EVAP_TEMP:
			Me.Evap_Temp = Value;
			return 0;

		case REG_SPLIT_MASTER_STATUS:
			Me.RoomMaster_Status = Value;
			return 0;
			
		case REG_SPLIT_DEFINE_MODE:
#if(SplitMode!=K_I_Am_Frigo && SplitMode!=K_I_Am_Freezer)
			if(Me.I_Am_SubSlave)	
                Me.DefMode = Value;											//La modalità di funzionamento me la comunica il Master (letta da Split-Master o in mod auto da split che comanda).
#if(SplitMode!=K_I_Am_InverterFAN && SplitMode!=K_I_Am_StandardFAN && SplitMode!=K_I_Am_Atex_Fan && SplitMode!=K_I_Am_On_Off_Fan)
            else if((Value==CoreVentilazione) || (Value==CoreRaffrescamento) || (Value==CoreRiscaldamento))// if(My_Address != k_Split_Master_Add || Me.I_Am_SubSlave)	//Se non sono Split-Master o se sono SubSlave
                Me.DefMode = Value;
#endif
#endif
			return 0;

		case REG_SPLIT_UTA_MODE:
			Touch[0].Uta_Mode = Value;
			return 0;

/*
		case REG_SPLIT_VALVOLA_SET_POS:		// gestione valvola
			Me.ExpValve_Req = Value;
			return 0;
		case REG_SPLIT_VALV_STP_REF_TIME:
			StepRefreshTime = Value;
			return 0;
		case REG_SPLIT_VALV_MAX_STEP:
			ValvolaMaxStepNum = Value;
			return 0;
            
		case REG_SPLIT_THR_MIN_AMP_VALV:
			DigitalOutput = Value;
			return 0;
		case REG_SPLIT_THR_MAX_AMP_VALV:
			DigitalOutput = Value;
			return 0;
*/
		case REG_SPLIT_ENGINE_BOX_WATER_PUMP_SPEED_PERC:
			EngineBox.Pump_Speed_Perc = Value;
			return 0;						
		case REG_SPLIT_ENGINE_BOX_COMPRESSOR_SPEED:	// gestione info box motore
			EngineBox.Compressor_Speed = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_WATER_PUMP_SPEED:
			EngineBox.Pump_Speed = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_FLUX:
			EngineBox.Water_Flux = Value;
			return 0;
		case REG_SPLIT_CURRENT_MODE_ENGINE_BOX:
			EngineBox.FunctionMode = Value;
			return 0;
		case REG_SPLIT_CURRENT_REQ_PRESSURE:
			EngineBox.Ric_Pressione = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_T_COMP_OUT:
			EngineBox.Temp_Mandata = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_T_COMP_SUCTION:
			EngineBox.Temp_Ritorno = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_T_SEA_WATER:
			EngineBox.Temp_Acqua_Mare = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_T_CONDENSER:
			EngineBox.Temp_Condensatore = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_T_GAS:
			EngineBox.Temp_Collettore_Grande = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_T_LIQUID:
			EngineBox.Temp_Collettore_Piccolo = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_P_GAS:
			EngineBox.Pressione_Gas_G = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_P_LIQUID:
			EngineBox.Pressione_Liq_P = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_PRESSURE_VRV_BOX:
			EngineBox.Pressione_VRV_Box = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_PRESSURE_AUX:
			EngineBox.Pressione_Aux = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_RELE_SATUS:
			EngineBox.ReleSt = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_INPUT_STATUS:
			EngineBox.InputSt = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_SYS_ID_LO:
			EngineBox.SysNumLo = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_SYS_ID_HI:
			EngineBox.SysNumHi = Value;
			return 0;
		case REG_SPLIT_ERRORE_1_BOX_MOTORE:
			EngineBox.Errori1 = Value;
			return 0;            
		case REG_SPLIT_ERRORE_2_BOX_MOTORE:
			EngineBox.Errori2 = Value;
			return 0;
		case REG_SPLIT_ERRORE_PERS1_BOX_MOTORE:
			EngineBox.ErroriPers1 = Value;
			return 0;
		case REG_SPLIT_ERRORE_PERS2_BOX_MOTORE:
			EngineBox.ErroriPers2 = Value;
			return 0;
		case REG_SPLIT_CUMULATIVE_ERROR:
			EngineBox.CumulativeError = Value;
			return 0;
		case REG_SPLIT_STATUS_BOX_MOTORE:
			EngineBox.StatoCompressore = Value;
			return 0;
		case REG_SPLIT_ID_SPLIT_MASTER:
			EngineBox.IdMasterSplit = Value;
			return 0;
		case REG_SPLIT_POWER_LIMIT:
			EngineBox.PowerLimit = Value;
			return 0;
		case REG_SPLIT_POWER_LIMIT_REQ:
			Me.PowerLimit = Value;
			return 0;
		case REG_SPLIT_STATUS:
			Me.SplitStatus = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_CYCLE_TIME:
			EngineBox.SystemCycleTime = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_HOURS_OF_LIFE:
			EngineBox.HoursOfLife = Value;
			return 0;
		case REG_SPLIT_ENGINE_BOX_TOUCH_NATION:
			EngineBox.Touch_Nation = Value;
			return 0;
			
		case REG_DIAG_SUB_SPLIT_QUANTITY:
			DiagnosticSplit.Sub_Slave_Quantity = Value;
			return 0;				
		case REG_DIAG_SPLIT_RES_REQ_ADDRESS:		
			DiagnosticSplit.Reset_Req_Address = Value;	// indirizzo dello slave su cui effettuare il reset allarmi proveniente dallo slave master
			return 0;			
		case REG_DIAG_SPLIT_TEST_ADDRESS:		// #v16
			DiagnosticSplit.Address = Value;	// Indirizzo slave da monitorare
			return 0;
		case REG_DIAG_SPLIT_STATUS:					
			DiagnosticSplit.SplitStatus = Value;	
			return 0;	
		case REG_DIAG_SPLIT_LIQ_TEMP:					
			DiagnosticSplit.Temp_Liq = Value;	
			return 0;
		case REG_DIAG_SPLIT_GAS_TEMP:					
			DiagnosticSplit.Temp_Gas = Value;	
			return 0;
		case REG_DIAG_SPLIT_AIR_IN:					
			DiagnosticSplit.Temp_AirIn = Value;	
			return 0;
		case REG_DIAG_SPLIT_AIR_OUT:					
			DiagnosticSplit.Temp_AirOut = Value;	
			return 0;
		case REG_DIAG_SPLIT_FAN_SPEED:					
			DiagnosticSplit.FanSpeed = Value;
			return 0;	
		case REG_DIAG_SPLIT_EXP_VALVE:					
			DiagnosticSplit.Exp_Valve = Value;	
			return 0;
		case REG_DIAG_SPLIT_EXP_VALVE_ERR_POS:					
			DiagnosticSplit.Exp_Valve_Err = Value;	
			return 0;
		case REG_DIAG_SPLIT_EXP_VALVE_ERR_CODE:					
			DiagnosticSplit.Exp_Valve_Err_Code = Value;	
			return 0;
            
		case REG_DIAG_SPLIT_GAS_PRESSURE:					
			DiagnosticSplit.Gas_Pressure = Value;	
			return 0;
		case REG_DIAG_SPLIT_LIQ_PRESSURE:					
			DiagnosticSplit.Liq_Pressure = Value;	
			return 0;
		case REG_DIAG_SPLIT_REQUEST_AIR_TEMP:					
			DiagnosticSplit.Req_AirTemp = Value;	
			return 0;
		case REG_DIAG_SPLIT_REQUEST_AIR_PRES:					
			DiagnosticSplit.Req_Pressure = Value;	
			return 0;
		case REG_DIAG_SPLIT_ENG_MODE:					
			DiagnosticSplit.Split_Eng_Mode = Value;
			return 0;
		case REG_DIAG_SPLIT_CYCLE_TIME:					
			DiagnosticSplit.CycleTime = Value;
			return 0;
		case REG_DIAG_SPLIT_FUNCTION_MODE:					
			DiagnosticSplit.FunctionMode = Value;
			return 0;

		case REG_DIAG_INVERTER_ALARM:	
			DiagnosticInverter.Alarm = Value;
			return 0;		
		case REG_DIAG_INVERTER_OUT_FREQ:
			DiagnosticInverter.OutFreq = Value;				
			return 0;
		case REG_DIAG_INVERTER_STATUS:		
			DiagnosticInverter.Status = Value;		 
			return 0;
		case REG_DIAG_INVERTER_OUT_CURRENT:	
			DiagnosticInverter.OutCurr = Value;	
			return 0;
		case REG_DIAG_INVERTER_IN_VOLTAGE:	
			DiagnosticInverter.InVolt = Value;	
			return 0;
		case REG_DIAG_INVERTER_OUT_VOLTAGE:	
			DiagnosticInverter.OutVolt = Value;	
			return 0;
		case REG_DIAG_INVERTER_IN_POWER:	
			DiagnosticInverter.InPower = Value;	
			return 0;
		case REG_DIAG_INVERTER_OUT_POWER:	
			DiagnosticInverter.OutPower = Value;	
			return 0;
		case REG_DIAG_INVERTER_REQ_FREQ:	
			DiagnosticInverter.ReqFreq = Value;	
			return 0;
		case REG_DIAG_INVERTER_MAX_REQ_FREQ:	
			DiagnosticInverter.MaxReqFreq = Value;	
			return 0;

        case REG_DIAG_EVAP_TEMP_VALVE:
            DiagnosticSplit.EvapTempValve = Value;
            return 0;
            
		case REG_SPLIT_READY_CMD_ADDRESS:					
			DiagnosticSplit.Ready_CMD_Address = Value;
			return 0;
            
		case REG_SPLIT_READY_POWER:					
			DiagnosticSplit.Ready_PowerOn = Value;
			return 0;

		case REG_SPLIT_READY_CURRENT_SP:					
			DiagnosticSplit.Ready_SetPoint = Value;
			return 0;
            
		case REG_SPLIT_READY_CURRENT_SP_F:					
			DiagnosticSplit.Ready_SetPoint_F = Value;
			return 0;

		case REG_SPLIT_READY_CURRENT_FANMODE:					
			DiagnosticSplit.Ready_FanMode = Value;
			return 0;
            
		case REG_SPLIT_READY_CURRENT_UTAMODE:					
			DiagnosticSplit.Ready_UtaMode = Value;
			return 0;
            
        case REG_SPLIT_ACC_INVERTER:
            EngineBox.Acceleration_Inverter = Value;
            return 0;
            
        case REG_SPLIT_ACC_GASPRESS:
            EngineBox.Acceleration_GasPressure = Value;
            return 0;            
        
		case REG_SPLIT_VENTIL_SELECTION:
			Me.Ventil_Selection = Value;
            if(!Me.I_Am_SubSlave)
                Init_PWM_Value();
			return 0;
            
		case REG_SPLIT_PWM_VALUE_OFF:			// gestione valori pwm velocità predefinite
			PWMValue[0] = Value;
			return 0;
		case REG_SPLIT_PWM_VALUE_NIGHT:
			PWMValue[1] = Value;
			return 0;
		case REG_SPLIT_PWM_VALUE_SPEED1:
			PWMValue[2] = Value;
			return 0;
		case REG_SPLIT_PWM_VALUE_SPEED2:
			PWMValue[3] = Value;
			return 0;
		case REG_SPLIT_PWM_VALUE_SPEED3:
			PWMValue[4] = Value;
			return 0;
		case REG_SPLIT_PWM_VALUE_SPEED4:
			PWMValue[5] = Value;
			return 0;
		case REG_SPLIT_PWM_VALUE_SPEED5:
			PWMValue[6] = Value;
			return 0;
		case REG_SPLIT_PWM_VALUE_SPEED6:
			PWMValue[7] = Value;
			return 0;
		case REG_SPLIT_PWM_VALUE_SPEED7:
			PWMValue[8] = Value;
			return 0;
			
		case REG_SPLIT_M3H_MIN:
			Me.M3h_Min = Value;
			return 0;
            
		case REG_SPLIT_M3H_MAX:
			Me.M3h_Max = Value;
			return 0;

		case REG_SPLIT_MIN_CURRENT_FAN:
			Me.Min_Current_Fan = Value;
			return 0;

		case REG_SPLIT_MAX_CURRENT_FAN:
			Me.Max_Current_Fan = Value;
			return 0;

		case REG_SPLIT_EFFICIENCY_FAN:
			Me.Efficiency_Fan = Value;
			return 0;

		case REG_SPLIT_SUPPLY_FAN:
			Me.Supply_Fan = Value;
			return 0;

		case REG_SPLIT_WR_THRESHOLD_COMPRESSOR_HI:
            if(Value<K_LimLo_TOUCH_THRESHOLD_COMPRESSOR_HI || Value>K_LimHi_TOUCH_THRESHOLD_COMPRESSOR_HI)
                Value = K_Def_TOUCH_THRESHOLD_COMPRESSOR_HI;
			EngineBox.Threshold_Compressor_Hi = Value;
			return 0;
 
		case REG_SUB_SPLIT_TEST_ADDRESS:	
			//if(My_Address != k_Split_Master_Add && !Me.I_Am_SubSlave)
			if(My_Address != k_Split_Master_Add || Me.I_Am_SubSlave)
				DiagnosticSplit.SubAddress = Value;	// Indirizzo slave da monitorare			
			return 0;			
			
		case REG_DIAG_INVERTER_POWERINV:
			DiagnosticInverter.Power_Inverter = Value;
			return 0;					
		case REG_SPLIT_POWER_MAX_AVAILABLE:
			DiagnosticInverter.PowerMax_Available = Value;
			return 0;					
		case REG_SPLIT_INVERTER_QUANTITY:
			DiagnosticInverter.Max_Inverter = Value;
			return 0;					
		case REG_DIAG_INVERTER_ADDRESS:
			DiagnosticInverter.DiagAddress = Value;
			return 0;					
			
		case REG_SPLIT_BOX_ABSORPTION:
			EngineBox.PowerBoxAbsorption = Value;
			return 0;	
			
		case REG_SPLIT_ENGINE_BOX2_COMPRESSOR_SPEED:
			EngineBox.Compressor_Speed_C2 = Value;
			return 0;	
			
		case REG_SPLIT_MAX_DIAG_BOX_QUANTITY:
			EngineBox.Max_Comp_Quantity = Value;
			return 0;	
			
        case REG_DIAG_SPLIT_WR_SET_POWER:		
            Syncronize.DiagWritePwr = Value;	             						
			return 0;	
        case REG_DIAG_SPLIT_WR_SET_SETPOINT:	
            Syncronize.DiagWriteSetPoint = Value;	             						
			return 0;	
        case REG_DIAG_SPLIT_WR_SET_FANSPEED:	
            Syncronize.DiagWriteFanMode = Value;	             						
			return 0;	
        case REG_DIAG_SPLIT_WR_SET_SETPOINT_F:
            Syncronize.DiagWriteSetPoint_F = Value;				
			return 0;	
            
        case REG_DIAG_SPLIT_WR_UTA_MODE:
            Syncronize.DiagUta_Mode = Value;				
			return 0;	
            
        case REG_DIAG_SPLIT_WR_HEATER_PWR:
            Syncronize.DiagWriteHeaterPwr = Value;				
			return 0;	
			
		case REG_SPLIT_WR_MODE:
            if(Value == CoreAuto || Value == CoreRiscaldamento || Value == CoreRaffrescamento || Value == CoreVentilazione)            
                Syncronize.DefMode = Value;
			return 0;			
		case REG_SPLIT_WR_UTA_POWER:
			Syncronize.Uta_Power = Value;
			return 0;			
		//case REG_SPLIT_WR_UTA_MODE:
		//	Syncronize.Uta_Mode = Value;
		//	return 0;			
            
		case REG_SPLIT_WR_UTA_SP_ROOM:
            if(Value >= K_LimLo_SPLIT_Uta_SP_Room && Value <= K_LimHi_SPLIT_Uta_SP_Room)
                Syncronize.Uta_SP_Room = Value;
			return 0;			
		case REG_SPLIT_WR_UTA_SP_HUMI:
            if(Value >= K_LimLo_SPLIT_Uta_SP_Humi && Value <= K_LimHi_SPLIT_Uta_SP_Humi)            
                Syncronize.Uta_SP_Humi = Value;
			return 0;			
		case REG_SPLIT_WR_UTA_FAN_SPEED:
            if(Value >= K_LimLo_SPLIT_Uta_Fan_Speed && Value <= K_LimHi_SPLIT_Uta_Fan_Speed)            
                Syncronize.Uta_Fan_Speed = Value;
			return 0;			
		case REG_SPLIT_WR_UTA_SP_ROOM_F:
            if(Value >= K_LimLo_SPLIT_Uta_SP_Room_F && Value <= K_LimHi_SPLIT_Uta_SP_Room_F)            
                Syncronize.Uta_SP_Room_F = Value;
			return 0;			

		case REG_SPLIT_WR_SYSTEM_DISABLE:
            if(Value >= K_LimLo_SPLIT_SystemDisable && Value <= K_LimHi_SPLIT_SystemDisable)            
                Syncronize.SystemDisable = Value;
			return 0;			
			
			
		case REG_SPLIT_WR_SUPERHEAT:
			//GL Me.SuperHeat_FromMaster = Value;
			Me.SuperHeat_SP_Local = Value;
			return 0;			
            
		case REG_SPLIT_WR_CORRECT_SUPERHEAT_AIROUT:
			Me.CorrectSuperHeatAirOut_Local = Value;
			return 0;			            
			
		case REG_SPLIT_WR_WORK_PRESS_COOL:
			//GL Me.Freddo_TempBatt_FromMaster = Value;
			Me.Freddo_TempBatt_SP_Local = Value;
			return 0;		
            
		case REG_SPLIT_WR_MIN_PRESS_COOL:
			Me.Freddo_TempBatt_Min = Value;
			return 0;		
    			
		case REG_SPLIT_WR_WORK_PRESS_HEAT:
			//GL Me.Caldo_TempBatt_FromMaster = Value;
			Me.Caldo_TempBatt_SP_Local = Value;
			return 0;			

		case REG_SPLIT_WR_MIN_PRESS_HEAT:
			Me.Caldo_TempBatt_Min = Value;
			return 0;		
			
		case REG_SPLIT_WR_PERFORMANCELOSSBATTERY:	
			Me.PerformanceLossBattery_SP = Value;
			return 0;	
            
		case REG_SPLIT_WR_DINAMIC_SUPERHEAT:	
			Me.Dinamic_SuperHeat = Value;
			return 0;	

		case REG_SPLIT_WR_INVERTER_SUPPLY:	
			Me.Inverter_Supply = Value;
			return 0;	

		case REG_SPLIT_WR_DOUBLE_PUMP_ABIL:	
			Me.DoublePump_Abil = Value;
			return 0;	

		case REG_SPLIT_WR_MODE_ABIL:	
			Me.SplitMode_Abil = Value;
			return 0;	
            
		case REG_SPLIT_WR_FAN_NEW_STYLE_TIME_ON:	
			Me.FanNewStyleTimeOn = Value;
			return 0;	

        case REG_SPLIT_WR_FAN_NEW_STYLE_TIME_OFF:	
			Me.FanNewStyleTimeOff = Value;
			return 0;	          
            
        case REG_SPLIT_WR_NETB_LCD_INTENSITY:	
			Me.NetB_LCD_Intens = Value;
			return 0;	                                  

        case REG_SPLIT_WR_FAN_CURRENT_LIMIT:	
			Me.Fan_Current_Limit = Value;
			return 0;	                                  
            
                        
        // FREE    
		//case REG_SPLIT_WR_TEMP_AMB_MIDDLE_ON:	
		//	Me.TempAmb_Middle_On = Value;
		//	return 0;	
            
		case REG_SPLIT_WR_OTHER_CABIN_NUMBER:	
			Me.OtherCabin.Number = Value;
			return 0;	
            
		case REG_SPLIT_OTHER_CABIN_ST_TEMPERATURE:	
			Me.OtherCabin.Temperature = Value;
			return 0;	

		case REG_SPLIT_OTHER_CABIN_ST_ON_OFF:	
			Me.OtherCabin.ST_On_Off = Value;
			return 0;	

		case REG_SPLIT_OTHER_CABIN_ST_SETP:	
			Me.OtherCabin.ST_SetP = Value;
			return 0;	

		case REG_SPLIT_OTHER_CABIN_ST_FAN:	
			Me.OtherCabin.ST_Fan = Value;
			return 0;	
            
		case REG_SPLIT_OTHER_CABIN_ST_UTA_MODE:	
			Me.OtherCabin.ST_UTAMode = Value;
			return 0;	

		case REG_SPLIT_OTHER_CABIN_ONLY_FAN:	
			Me.OtherCabin.Only_Fan = Value;
			return 0;	

		case REG_SPLIT_OTHER_CABIN_TRIGGER_INIT:	
			Me.OtherCabin.Trigger_Init = Value;
			return 0;	

        case REG_SPLIT_WR_EXTRACTOR_NUMBER:	
			Me.Extractor.Number = Value;
			return 0;

        case REG_SPLIT_WR_DELTAVEL_EXTRACTOR:	
			Me.Extractor.DeltaVel = Value;
			return 0;    
            
        case REG_SPLIT_WR_EXTRACTOR_AUTOVEL:	
			Me.Extractor.AutoVel = Value;
			return 0;                

        case REG_SPLIT_WR_EXTRACTOR_ABIL:	
			Me.Extractor.Abil = Value;
			return 0;
            
        case REG_SPLIT_SET_BAUDRATE:
            if(Value==K_Baud57600)
                set_baudrateU1_57600();
            if(Value==K_Baud115200)
                set_baudrateU1_115200();		
            if(Value==K_Baud250000)
                set_baudrateU1_250000();            
            return 0;	    
            
			
		case REG_SPLIT_FLAGS_ENABLE_1_16:
			EngineBox.Split.FlagsEnable_1_16 = Value;
			return 0;			
		case REG_SPLIT_FLAGS_OFFLINE_1_16:
			EngineBox.Split.FlagsOffLine_1_16 = Value;
			return 0;			
		case REG_SPLIT_FLAGS_ONLINE_1_16:
			EngineBox.Split.FlagsOnLine_1_16 = Value;
			return 0;			
		case REG_SPLIT_FLAGS_ALARM_1_16:
			EngineBox.Split.FlagsAlarm_1_16 = Value;
			return 0;			
		case REG_SPLIT_FLAGS_ENABLE_17_32:
			EngineBox.Split.FlagsEnable_17_32 = Value;
			return 0;			
		case REG_SPLIT_FLAGS_OFFLINE_17_32:
			EngineBox.Split.FlagsOffLine_17_32 = Value;
			return 0;			
		case REG_SPLIT_FLAGS_ONLINE_17_32:
			EngineBox.Split.FlagsOnLine_17_32 = Value;
			return 0;			
		case REG_SPLIT_FLAGS_ALARM_17_32:
			EngineBox.Split.FlagsAlarm_17_32 = Value;
			return 0;			
			
        case REG_SPLIT_INV_INVOLT_C1:
			EngineBox.InVolt_C1 = Value;
			return 0;			
        case REG_SPLIT_INV_OUTPOWER_C1:
			EngineBox.OutPower_C1 = Value;
			return 0;			         
        case REG_SPLIT_INV_INVOLT_C2:
			EngineBox.InVolt_C2 = Value;
			return 0;			
        case REG_SPLIT_INV_OUTPOWER_C2:
			EngineBox.OutPower_C2 = Value;
			return 0;
			
        case REG_SPLIT_INVERTER1_ERROR:
			EngineBox.Inverter1CodeFault = Value;
			return 0;

        case REG_SPLIT_INVERTER2_ERROR:
			EngineBox.Inverter2CodeFault = Value;
			return 0;

        case REG_SPLIT_INVERTER3_ERROR:
			EngineBox.Inverter3CodeFault = Value;
			return 0;

        case REG_SPLIT_INVERTER4_ERROR:
			EngineBox.Inverter4CodeFault = Value;
			return 0;
			
		case REG_SPLIT_UTA_ABIL:
			Touch[0].Uta_Abil = Value;
			return 0;	
			
		case REG_SPLIT_MASTER_RESTART_TIME:	
			EngineBox.MasterRestartTime = Value;
			return 0;	

		case REG_SPLIT_SYSTEM_DISABLE:
			//if(My_Address != k_Split_Master_Add)		// Eliminata eccezione per far spegnere anche split master con "System Disable" da Touch
            if(My_Address != k_Split_Master_Add || Me.I_Am_SubSlave)	//Se non sono Split-Master o se sono SubSlave
#if(K_Bypass_System_Disable==1)
                EngineBox.SystemDisable = 0;
#else
				EngineBox.SystemDisable = Value;
#endif
			return 0;	
			
		case REG_SPLIT_MODDEV_FLAGS_ENABLE_1_16:
			EngineBox.Split.ModDev.FlagsEnable_1_16 = Value;
			return 0;			
		case REG_SPLIT_MODDEV_FLAGS_OFFLINE_1_16:
			EngineBox.Split.ModDev.FlagsOffLine_1_16 = Value;
			return 0;			
		case REG_SPLIT_MODDEV_FLAGS_ONLINE_1_16:
			EngineBox.Split.ModDev.FlagsOnLine_1_16 = Value;
			return 0;			
		case REG_SPLIT_MODDEV_FLAGS_ALARM_1_16:
			EngineBox.Split.ModDev.FlagsAlarm_1_16 = Value;
			return 0;			
		case REG_SPLIT_MODDEV_FLAGS_ENABLE_17_32:
			EngineBox.Split.ModDev.FlagsEnable_17_32 = Value;
			return 0;			
		case REG_SPLIT_MODDEV_FLAGS_OFFLINE_17_32:
			EngineBox.Split.ModDev.FlagsOffLine_17_32 = Value;
			return 0;			
		case REG_SPLIT_MODDEV_FLAGS_ONLINE_17_32:
			EngineBox.Split.ModDev.FlagsOnLine_17_32 = Value;
			return 0;			
		case REG_SPLIT_MODDEV_FLAGS_ALARM_17_32:
			EngineBox.Split.ModDev.FlagsAlarm_17_32 = Value;
			return 0;			

        case REG_SPLIT_MODDEV_AND_ADDR_TYPE:
			EngineBox.Split.ModDev.AddrType = Value;
			return 0;		                        
            
		case REG_SPLIT_UTAH_ENGINECOMPSPEED:			
			EngineBox.UTA_H.EngineCompSpeed = Value;	            
			return 0;			                        
		case REG_SPLIT_UTAH_ENGINEWATERPUMP:			
			EngineBox.UTA_H.EngineFanSpeed = Value;	              			
			return 0;			                        
		case REG_SPLIT_UTAH_ENGINEREQPRESSURE:	    
			EngineBox.UTA_H.EngineReqPressure = Value;	                            
			return 0;			                        
		case REG_SPLIT_UTAH_ENGINETEMPOUT:				
			EngineBox.UTA_H.EngineTempOut = Value;                     			
			return 0;			                        
		case REG_SPLIT_UTAH_ENGINETEMPIN:					
			EngineBox.UTA_H.EngineTempIn = Value;			                          
			return 0;			                        
		case REG_SPLIT_UTAH_ENGINETEMPAIR:					
			EngineBox.UTA_H.EngineTempAir = Value;			                          
			return 0;			                        
		case REG_SPLIT_UTAH_ENGINETEMPBATT:				
			EngineBox.UTA_H.EngineTempBatt = Value;		                          
			return 0;			                        
		case REG_SPLIT_UTAH_ENGINETEMPGAS:				
			EngineBox.UTA_H.EngineTempGas = Value;		                          
			return 0;			                        
		case REG_SPLIT_UTAH_ENGINETEMPLIQ:					
			EngineBox.UTA_H.EngineTempLiq = Value;		              			
			return 0;			                        
		case REG_SPLIT_UTAH_ENGINEPRESSGAS:			
			EngineBox.UTA_H.EnginePressGas = Value;		                          
			return 0;			                        
		case REG_SPLIT_UTAH_ENGINEPRESSLIQ:				
			EngineBox.UTA_H.EnginePressLiq = Value;		              			
			return 0;			                        
		case REG_SPLIT_UTAH_ENGINEBOXERR1:				
			EngineBox.UTA_H.EngineBoxErr1 = Value;			                          
			return 0;			                        
		case REG_SPLIT_UTAH_ENGINEBOXPERSERR1:	    
			EngineBox.UTA_H.EngineBoxPersErr1 = Value;	                          
			return 0;			                        
		case REG_SPLIT_UTAH_ENGINEBOXSPLITERR1:	    
			EngineBox.UTA_H.EngineBoxSplitErr1 = Value;	                          
			return 0;			                        
		case REG_SPLIT_UTAH_ABSORPTION:						
			EngineBox.UTA_H.Absorption = Value;				                          
			return 0;			                        
		case REG_SPLIT_UTAH_CUMULATIVEALARM:		    
			EngineBox.UTA_H.CumulativeAlarm = Value;		                          
			return 0;			                        
		case REG_SPLIT_UTAH_SPLIT_LIQUIDTEMP:				
			EngineBox.UTA_H.Split_LiquidTemp = Value;		                          
			return 0;			                        
		case REG_SPLIT_UTAH_SPLIT_GASTEMP:					
			EngineBox.UTA_H.Split_GasTemp = Value;			                          
			return 0;			                        
											                               			                                                       
		case REG_SPLIT_UTAH_SPLIT_AIRINTEMP:				
			EngineBox.UTA_H.Split_AirInTemp = Value;			                          
			return 0;			                        
		case REG_SPLIT_UTAH_SPLIT_AITOUTTEMP:			
			EngineBox.UTA_H.Split_AitOutTemp = Value;		                          
			return 0;			                        
		case REG_SPLIT_UTAH_SPLIT_EXP_VALVE:				
			EngineBox.UTA_H.Split_Exp_Valve = Value;		              			
			return 0;			                        
		case REG_SPLIT_UTAH_SPLIT_HUMI:						
			EngineBox.UTA_H.Split_Humi = Value;				              			
			return 0;			                        
		case REG_SPLIT_UTAH_SPLIT_REQAIRTEMP:		    
			EngineBox.UTA_H.Split_ReqAirTemp = Value;		                          
			return 0;			                        
		case REG_SPLIT_UTAH_SPLIT_REQGASPRESS:	    
			EngineBox.UTA_H.Split_ReqGasPress = Value;	                          
			return 0;			                        
		case REG_SPLIT_UTAH_SPLIT_SUPERHEAT:				
			EngineBox.UTA_H.Split_Superheat = Value;		              			
			return 0;			                        
		case REG_SPLIT_UTAH_SPLIT_HW_VER:					
			EngineBox.UTA_H.Split_HW_Ver = Value;			                          
			return 0;			                        
		case REG_SPLIT_UTAH_SPLIT_FW_VER_HW:			
			EngineBox.UTA_H.Split_FW_Ver_HW = Value;	              			
			return 0;			                        
		case REG_SPLIT_UTAH_SPLIT_FW_VER_FW:			
			EngineBox.UTA_H.Split_FW_Ver_FW = Value;	              			
			return 0;			                        
		case REG_SPLIT_UTAH_SPLIT_FW_REV_FW:			
			EngineBox.UTA_H.Split_FW_Rev_FW = Value;	              			
			return 0;			                        
		case REG_SPLIT_UTAH_HW_VER:								
			EngineBox.UTA_H.HW_Ver = Value;					                          
			return 0;			                        
		case REG_SPLIT_UTAH_FW_VER_HW:						
			EngineBox.UTA_H.FW_Ver_HW = Value;			              			
			return 0;			                        
		case REG_SPLIT_UTAH_FW_VER_FW:						
			EngineBox.UTA_H.FW_Ver_FW = Value;			              			
			return 0;			                        
		case REG_SPLIT_UTAH_FW_REV_FW:						
			EngineBox.UTA_H.FW_Rev_FW = Value;			              			
			return 0;			                        
		case REG_SPLIT_UTAH_UTASP_ON:							
			EngineBox.UTA_H.UTASP_On = Value;				                          
			return 0;			                        

		case REG_SPLIT_UTAH_COMP_STATUS:							
			EngineBox.UTA_H.StatoCompressore = Value;				                          
			return 0;			                        

		case REG_SPLIT_UTAH_ENGINE_BOX_ID_MAS_SPLIT:							
			EngineBox.UTA_H.IdMasterSplit = Value;				                          
			return 0;			                        

		case REG_SPLIT_UTAH_ENGINE_MODE:							
			EngineBox.UTA_H.Mode = Value;				                          
			return 0;			                        		
            
        case REG_SPLIT_PRESSURE_LO_LIQ:
			EngineBox.PressureLoLiqValue = Value;				                          
			return 0;			                        		
        case REG_SPLIT_PRESSURE_LO_GAS:
			EngineBox.PressureLoGasValue = Value;				                          
			return 0;			                        		
        case REG_SPLIT_PRESSURE_HI_LIQ:
			EngineBox.PressureHiLiqValue = Value;				                          
			return 0;			                        		
        case REG_SPLIT_PRESSURE_HI_GAS:
			EngineBox.PressureHiGasValue = Value;				                          
			return 0;			                        		
                                    			
		case REG_MASTER_HW_VERSION:			
            EngineBox.HW_Version = Value;
			return 0;			            
        case REG_MASTER_FW_VERSION_HW:
            EngineBox.FW_Version_HW = Value;
			return 0;			            
        case REG_MASTER_FW_VERSION_FW:		
            EngineBox.FW_Version_FW = Value;
			return 0;	            
        case REG_MASTER_FW_REVISION_FW:		
            EngineBox.FW_Revision_FW = Value;
			return 0;	                               
            
            
        case REG_DIAG_SPLIT_HW_VERSION:		
            DiagnosticSplit.HW_Version = Value;
			return 0;	            
        case REG_DIAG_SPLIT_FW_VERSION_HW:		
            DiagnosticSplit.FW_Version_HW = Value;
			return 0;	            
        case REG_DIAG_SPLIT_FW_VERSION_FW:	
            DiagnosticSplit.FW_Version_FW = Value;
			return 0;	            
        case REG_DIAG_SPLIT_FW_REVISION_FW:	    
            DiagnosticSplit.FW_Revision_FW = Value;
			return 0;
            
        case REG_DIAG_SPLIT_MODEL_VERSION:	    
            DiagnosticSplit.Model_FW_Version = Value;
			return 0;
            
        case REG_DIAG_SPLIT_FAN_M3H:	    
            DiagnosticSplit.Fan_M3h = Value;
			return 0;            

        case REG_DIAG_SPLIT_AIR_POWER_OUTPUT:	    
            DiagnosticSplit.AirPowerOutput = Value;
			return 0;            

        case REG_DIAG_SPLIT_AIR_POWER_BTU:	    
            DiagnosticSplit.AirPowerBTU = Value;
			return 0;            

        case REG_DIAG_SPLIT_TOUCH_PAGE:
            DiagnosticSplit.Touch_Page = Value;
			return 0;            
            
        case REG_DIAG_SPLIT_CNTCOMERRSEC:
            DiagnosticSplit.ComError.CntComErrSec = Value;
			return 0;            
            
        case REG_DIAG_SPLIT_TIMEOUTCOMERRSEC:
            DiagnosticSplit.ComError.TimeOutComErrSec = Value;
			return 0;           
            
        case REG_DIAG_SPLIT_TIMEOUTINTERCHARCOMERRSEC:
            DiagnosticSplit.ComError.TimeOutInterCharComErrSec = Value;
			return 0;            
            
        case REG_DIAG_SPLIT_CRC_COMERRSEC:
            DiagnosticSplit.ComError.CRC_ComErrSec = Value;
			return 0;            
            
        case REG_DIAG_SPLIT_MODCNTCOMERRSEC:
            DiagnosticSplit.ComError.ModCntComErrSec = Value;
			return 0;            
            
        case REG_DIAG_SPLIT_MODTIMEOUTCOMERRSEC:
            DiagnosticSplit.ComError.ModTimeOutComErrSec = Value;
			return 0;            
            
        case REG_DIAG_SPLIT_MODTIMEOUTINTERCHARCOMERRSEC:
            DiagnosticSplit.ComError.ModTimeOutInterCharComErrSec = Value;
			return 0;            
            
        case REG_DIAG_SPLIT_MODCRC_COMERRSEC:
            DiagnosticSplit.ComError.ModCRC_ComErrSec = Value;
			return 0;            

        case REG_SPLIT_DIAG_ONLY_FAN:
            DiagnosticSplit.Only_Fan = Value;
			return 0;
        case REG_DIAG_SPLIT_TEMP_AMB_MIDDLE:
            DiagnosticSplit.TempAmbMiddle = Value;
			return 0;
        case REG_DIAG_SPLIT_TEMP_AMB_REAL:
            DiagnosticSplit.TempAmbReal = Value;
			return 0;
            
        case REG_SPLIT_DIAG_SET_CMD_DISABLE:
            DiagnosticSplit.Set_CMD_Disable = Value;
			return 0;            
 
       case REG_SPLIT_VALVELIM_PERCMAXOPENVALVE:            
            Me.ValveLim.MasterRx.PercMaxOpenValve = Value;   
			return 0;   
         
        case REG_SPLIT_VALVELIM_VALVOLAAPERTA:
            Me.ValveLim.MasterRx.ValvolaAperta = Value;
			return 0;            
            
        case REG_SPLIT_VALVELIM_VALVOLACHIUSA:
            Me.ValveLim.MasterRx.ValvolaChiusa = Value;
			return 0;            
            
        case REG_SPLIT_VALVELIM_VALVOLAOFF:
            Me.ValveLim.MasterRx.ValvolaOff = Value;
			return 0;            
            
        case REG_SPLIT_VALVELIM_VALVE_MIN_GO_OFF:
            Me.ValveLim.MasterRx.Valve_Min_Go_Off = Value;
			return 0;            
            
        case REG_SPLIT_VALVELIM_VALVELOSTCOMMCOOL:
            Me.ValveLim.MasterRx.ValveLostCommCool = Value;
			return 0;            
            
        case REG_SPLIT_VALVELIM_VALVOLOSTCOMMHOT:
            Me.ValveLim.MasterRx.ValvoLostCommHot = Value;
			return 0;            
            
        case REG_SPLIT_VALVELIM_APERTURAMAXCALDO:
            Me.ValveLim.MasterRx.AperturaMaxCaldo_Local = Value;
			return 0;            
            
        case REG_SPLIT_VALVELIM_APERTURAMINCALDO:
            Me.ValveLim.MasterRx.AperturaMinCaldo_Local = Value;
			return 0;            
            
        case REG_SPLIT_VALVELIM_INITVALVOLAFREDDO:
            Me.ValveLim.MasterRx.InitValvolaFreddo = Value;
			return 0;            
            
        case REG_SPLIT_VALVELIM_APERTURAMAXFREDDO:
            Me.ValveLim.MasterRx.AperturaMaxFreddo_Local = Value;
			return 0;            
            
        case REG_SPLIT_VALVELIM_APERTURAMINFREDDO:
            Me.ValveLim.MasterRx.AperturaMinFreddo_Local = Value;
			return 0;            
            
        case REG_SPLIT_VALVELIM_VALVENOWORKMINFREDDO:
            Me.ValveLim.MasterRx.ValveNoWorkMinFreddo = Value;
			return 0;            
            
        case REG_SPLIT_VALVELIM_VALVENOWORKMAXFREDDO:
            Me.ValveLim.MasterRx.ValveNoWorkMaxFreddo = Value;
			return 0;            
              
       case REG_SPLIT_VALVELIM_VALVOLAFROZEN:            
            Me.ValveLim.MasterRx.ValvolaFrozen = Value;   
			return 0;   

        case REG_SPLIT_VALVOLA_ONDEMAND_PRIORITY:
            Me.ExpValve_On_Demand_Priority = Value;
			return 0;            
            
        case REG_SPLIT_VALVOLA_CUR_NUMB_PRIORITY:
            Me.ExpValve_Cur_Number_Priority = Value;
			return 0;            

        case REG_SPLIT_VALVOLA_ROOM_NUMB_PRIORITY:
            Me.ExpValve_Room_Number_Priority = Value;
			return 0;            

        case REG_SPLIT_VALVOLA_MAX_PERC_PRIORITY:
            /*
            if(EnableMySecBus==1 && !Me.I_Am_SubSlave)
                temp = (int)((float)Value/(float)(Me.Sub_Slave_Quantity+1));
            else*/
                temp = Value;
            Me.ExpValve_On_Demand_Request = temp;
			return 0;  
            
        case REG_SPLIT_VALVOLA_SUM_TOT_PERCENT:
            Me.ExpValve_Sum_Tot_Percent = Value;
			return 0;  

        case REG_SPLIT_VALVOLA_SUM_REQ_PERCENT:
            Me.ExpValve_Sum_Req_Percent = Value;
			return 0;  

        case REG_SPLIT_VALVOLA_ONDEMAND_LIM_MAX:
            Me.ExpValve_On_Demand_Lim_Max = Value;
			return 0;  
            
		case REG_SPLIT_WR_TEMP_AMB_MIDDLE_HEATING:	
			Me.TempAmb_Middle_Heating = Value;
			return 0;	            

		case REG_SPLIT_WR_TEMP_AMB_MIDDLE_COOLING:	
			Me.TempAmb_Middle_Cooling = Value;
			return 0;	                                             
            
            
        case REG_SPLIT_RD_ABIL_VALVE_ONDEMAND:
			EngineBox.AbilValveOnDemand = Value;
			return 0;	                                             

        case REG_SPLIT_RD_ABIL_SCORE_ONDEMAND:
			EngineBox.AbilScoreOnDemand = Value;
			return 0;	                                             

        case REG_SPLIT_RD_VALVE_ONDEMAND_MIN_PERC_VAL:
			Me.ValveOnDemand_Min_Percent_Val = Value;
			return 0;	                                             
            
            
            case REG_DIAG_SUB_SPLIT_FLAGS_ENABLE_1_16:		
            DiagnosticSplit.Sub_Slave_Flags_Enable_1_16 = Value;
			return 0;	            
        case REG_DIAG_SUB_SPLIT_FLAGS_OFFLINE_1_16:		
            DiagnosticSplit.Sub_Slave_Flags_Offline_1_16 = Value;
			return 0;	            
        case REG_DIAG_SUB_SPLIT_FLAGS_ONLINE_1_16:	
            DiagnosticSplit.Sub_Slave_Flags_Online_1_16 = Value;
			return 0;	            
        case REG_DIAG_SUB_SPLIT_FLAGS_ALARM_1_16:	    
            DiagnosticSplit.Sub_Slave_Flags_Alarm_1_16 = Value;
			return 0;	           
 
       case REG_DIAG_SPLIT_SUPERHEAT:	    
			DiagnosticSplit.Superheat = Value;
			return 0;
        case REG_DIAG_SPLIT_SUPERHEAT_RISCALDAMENTO:	    
			DiagnosticSplit.SuperheatRiscaldamento = Value;
			return 0;						
            
        case REG_DIAG_SPLIT_SETP_SUPERHEAT:	    
			DiagnosticSplit.SetP_Superheat = Value;
			return 0;			
            
        case REG_DIAG_SPLIT_UTA_ABIL:	    
			DiagnosticSplit.UTA_Abil = Value;
			return 0;			
            
        case REG_DIAG_SPLIT_FAN_POWER:	    
			DiagnosticSplit.Fan_Power = Value;
			return 0;			
            
        case REG_DIAG_SPLIT_FAN_CURRENT:	    
			DiagnosticSplit.Fan_Current = Value;
			return 0;			

        case REG_DIAG_SPLIT_SUPPLY_VOLTAGE:	    
			DiagnosticSplit.Supply_Voltage = Value;
			return 0;			
            
        case REG_DIAG_SPLIT_FROZEN_MODE:	    
			DiagnosticSplit.FrozenMode = Value;
			return 0;			            
            
            
        case REG_SUB_SPLIT_TEMP_AMB_MID:
            Me.TempAmbMiddle = Value;
            return 0;
            
        case REG_SPLIT_SUPERHEAT_COOLINGCALC:
            EngineBox.SuperHeat_CoolingCalc = Value;
            return 0;
            
        case REG_SPLIT_K_SETPOINT_COOLING_SUPERHEAT:
            EngineBox.K_SetPoint_Cooling_SuperHeat = Value;
            return 0;
            
        case REG_SPLIT_SUPERHEAT_CORRECT_COOLING:
            EngineBox.SuperHeat_Correct_Cooling = Value;
            return 0;

        case REG_SPLIT_WR_CONDENSER_PRESS_ABIL:
            EngineBox.Condenser_Press_Abil = Value;
            return 0;

        case REG_SPLIT_TOTAL_FAN_ABSORPTION:
            EngineBox.Total_Fan_Absorption = Value;
            return 0;

        case REG_SPLIT_COMPRESSOR1_ABSORPTION:
            EngineBox.Compressor1_Absorption = Value;
            return 0;
            
        case REG_SPLIT_COMPRESSOR2_ABSORPTION:
            EngineBox.Compressor2_Absorption = Value;
            return 0;
            
        case REG_SPLIT_SEA_WATER_PUMP_ABSORPTION:
            EngineBox.Sea_Water_Pump_Absorption = Value;
            return 0;
            
        case REG_SPLIT_TOTAL_AIR_POWER_OUTPUT:
            EngineBox.Total_AirPowerOutput = Value;
            return 0;

        case REG_SPLIT_TOTAL_AIR_POWER_BTU:
            EngineBox.Total_AirPower_BTU = Value;
            return 0;            
           
        case REG_SYSTEM_EFFICENCY_EER:
            EngineBox.System_Efficency_EER = Value;
            return 0;            

        case REG_INVERTER_UPPER_OUT_FREQ:
            EngineBox.Inverter_Upper_Out_Freq = Value;
            return 0;            
            
        case REG_SPLIT_WR_TESTALL_ABIL:
            EngineBox.TestAll_Abil = Value;          
            return 0;
        case REG_SPLIT_WR_TESTALL_WORK_PRESS_HEAT:
            EngineBox.TestAll_TempBatt_SP_Heat = Value;
            return 0;
        case REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL:
            EngineBox.TestAll_SuperHeat_SP_Cool = Value;
            return 0;
        case REG_SPLIT_WR_TESTALL_WORK_PRESS_COOL:
            EngineBox.TestAll_TempBatt_SP_Cool = Value;
            return 0;
        case REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMAXFREDDO:
            EngineBox.TestAll_MaxOpValve_Cool = Value;
            return 0;

        case REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL_PGAIN:
            EngineBox.TestAll_SuperHeat_Cool_pGain = Value;
            return 0;

        case REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL_IGAIN:
            EngineBox.TestAll_SuperHeat_Cool_iGain = Value;
            return 0;

        case REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL_DGAIN:
            EngineBox.TestAll_SuperHeat_Cool_dGain = Value;
            return 0;

        case REG_SPLIT_WR_TESTALL_SUPERHEAT_COOL_TIME_EXEC_PID:
            EngineBox.TestAll_SuperHeat_Cool_Time_Exec_PID = Value;
            return 0;
            
        case REG_SPLIT_WR_MAINTENANCE_REMANING_TIME:
            Touch[0].Maintenance_Remaning_Time = Value;
            return 0;

        case REG_SPLIT_WR_FROZEN_ABIL:
            EngineBox.Frozen_Abil = Value;
            return 0;

        case REG_SPLIT_WR_FROZEN_ON:
            EngineBox.Frozen_On = Value;
            return 0;
            
            
        case REG_SPLIT_WR_TESTALL_FROZEN_ABIL:
            EngineBox.TestAll_Frozen_Abil = Value;
            return 0;
        case REG_SPLIT_WR_TESTALL_SUPERHEAT_SP_FRIGO:
            EngineBox.TestAll_SuperHeat_SP_Frigo = Value;
            return 0;
        case REG_SPLIT_WR_TESTALL_SUPERHEAT_SP_FREEZER:
            EngineBox.TestAll_SuperHeat_SP_Freezer = Value;
            return 0;
        case REG_SPLIT_WR_TESTALL_WORK_PRESS_FRIGO:
            EngineBox.TestAll_TempBatt_SP_Frigo = Value;
            return 0;
        case REG_SPLIT_WR_TESTALL_WORK_PRESS_FREEZER:
            EngineBox.TestAll_TempBatt_SP_Freezer = Value;
            return 0;
        case REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMAXFREDDO_FRIGO:
            EngineBox.TestAll_MaxOpValve_Frigo = Value;
            return 0;
        case REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMAXFREDDO_FREEZER:
            EngineBox.TestAll_MaxOpValve_Freezer = Value;
            return 0;

        case REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMINFREDDO:
            EngineBox.TestAll_MinOpValve_Cool = Value;
            return 0;
            
        case REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMINCALDO:
            EngineBox.TestAll_MinOpValve_Heat = Value;
            return 0;

        case REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMINFREDDO_FRIGO:
            EngineBox.TestAll_MinOpValve_Frigo = Value;
            return 0;

        case REG_SPLIT_WR_TESTALL_VALVELIM_APERTURAMINFREDDO_FREEZER:
            EngineBox.TestAll_MinOpValve_Freezer = Value;
            return 0;

        case REG_SPLIT_WR_ABIL_DEFROSTING:
            EngineBox.Abil_Defrosting = Value;
            return 0;
        case REG_SPLIT_WR_DEFROSTING_STEP_TIME:
            EngineBox.Defrosting_Step_Time = Value;
            return 0;
        case REG_SPLIT_WR_DEFROSTING_TIME:
            EngineBox.Defrosting_Time = Value;
            return 0;
        case REG_SPLIT_WR_DEFROSTING_TEMP_SET:
            EngineBox.Defrosting_Temp_Set = Value;
            return 0;
        case REG_SPLIT_WR_DRIPPING_TIME:
            EngineBox.Dripping_Time = Value;
            return 0;

        case REG_MASTER_COMERROR_CNTCOMERR:
            EngineBox.ComError.CntComErr = Value;
            return 0;

        case REG_MASTER_COMERROR_TIMEOUTCOMERR:
            EngineBox.ComError.TimeOutComErr = Value;
            return 0;
            
        case REG_MASTER_COMERROR_CRC_COMERR:
            EngineBox.ComError.CRC_ComErr = Value;
            return 0;
            
        case REG_MASTER_COMERROR_MODCNTCOMERR:
            EngineBox.ComError.ModCntComErr = Value;
            return 0;
            
        case REG_MASTER_COMERROR_MODTIMEOUTCOMERR:
            EngineBox.ComError.ModTimeOutComErr = Value;
            return 0;
            
        case REG_MASTER_COMERROR_MODCRC_COMERR:
            EngineBox.ComError.ModCRC_ComErr = Value;
            return 0;
            
        case REG_MASTER_COMERROR_MODCNTCOMERR_U2:
            EngineBox.ComError.ModCntComErr_U2 = Value;
            return 0;
            
        case REG_MASTER_COMERROR_MODTIMEOUTCOMERR_U2:
            EngineBox.ComError.ModTimeOutComErr_U2 = Value;
            return 0;
            
        case REG_MASTER_COMERROR_MODCRC_COMERR_U2:
            EngineBox.ComError.ModCRC_ComErr_U2 = Value;
            return 0;
            
        case REG_MASTER_COMERROR_CNTCOMERR_U3:
            EngineBox.ComError.CntComErr_U3 = Value;
            return 0;
            
        case REG_MASTER_COMERROR_TIMEOUTCOMERR_U3:
            EngineBox.ComError.TimeOutComErr_U3 = Value;
            return 0;
            
        case REG_MASTER_COMERROR_CRC_COMERR_U3:
            EngineBox.ComError.CRC_ComErr_U3 = Value;
            return 0;            
            
	// Ricezione dati per Doppio compressore			
        case REG_SPLIT_CURRENT_MODE_ENGINE_BOX2:	    
             EngineBox.FunctionMode_C2 = Value;
			return 0;	                
        case REG_SPLIT_ENGINE_BOX2_T_COMP_OUT:	    
             EngineBox.Temp_Compressor_Output_C2 = Value;
			return 0;	                
/*        case REG_SPLIT_ERRORE_1_BOX2_MOTORE:	    
             EngineBox.Errori1_C2 = Value;
			return 0;	                
        case REG_SPLIT_ERRORE_PERS1_BOX2_MOTORE:	    
             EngineBox.ErroriPers1_C2 = Value;
			return 0;	                
        case REG_SPLIT_ERRORE_PERS2_BOX2_MOTORE:	    
             EngineBox.ErroriPers2_C2 = Value;
			return 0; */	                
        case REG_SPLIT_CUMULATIVE_ERROR2:	    
             EngineBox.CumulativeError_C2 = Value;
			return 0;	                
        case REG_SPLIT_STATUS_BOX2_MOTORE:	    
             EngineBox.StatoCompressore_C2 = Value;
			return 0;	                
        case REG_SPLIT_DOUBLE_COMPRESSSOR_OK:	    
             EngineBox.DoubleCompressorOn = Value;
			return 0;	                
	
		case REG_SPLIT_FROM_INIT_SP_ROOM:        
            Reinit.FromEngBox.SetPoint = Value;
			return 0;			
        case REG_SPLIT_FROM_INIT_FAN_SPEED:        
            Reinit.FromEngBox.FanSpeed = Value;
			return 0;			
        case REG_SPLIT_FROM_INIT_POWER:        
            Reinit.FromEngBox.Power = Value;	             
			return 0;
		case REG_SPLIT_FROM_INIT_SP_ROOM_F:        
            Reinit.FromEngBox.SetPoint_F = Value;	             
			return 0;		
		case REG_SPLIT_FROM_INIT_DEFMODE:        
            Reinit.FromEngBox.DefMode = Value;	             
			return 0;					        
        case REG_SPLIT_FROM_TRIGGER_INIT:  
#if((K_UTAEnable==0) && (K_Disable_Trig_Init==0))
            if(Value >= K_LimLo_TOUCH_TRIGGER_INIT && Value <= K_LimHi_TOUCH_TRIGGER_INIT)
				Reinit.FromEngBox.TrigInit = Value;	             			
#else
				Reinit.FromEngBox.TrigInit = 0;
#endif
			return 0;

        case REG_SPLIT_FROM_INIT_UTAMODE:        
            Reinit.FromEngBox.UTA_Mode = Value;	             
			return 0;
            
        case REG_SPLIT_FROM_INIT_HEATER_PWR:        
            Reinit.FromEngBox.HeaterPwr = Value;	             
			return 0;            
			
        case REG_SPLIT_SYNC_WR_SETP:        
           if(Value >= K_LimLo_SPLIT_SP && Value <= K_LimHi_SPLIT_SP)
                Syncronize.SetPoint = Value;	             			
			return 0;
        case REG_SPLIT_SYNC_WR_FANMODE:        
            if(Value >= K_LimLo_SPLIT_FanMode && Value <= Me.MaxFanVentil_Mode)
                Syncronize.FanMode = Value;	             			
			return 0;			
        case REG_SPLIT_SYNC_WR_SETPF:        
            if(Value >= K_LimLo_SPLIT_SP_F && Value <= K_LimHi_SPLIT_SP_F)
                Syncronize.Script_SetPoint_F = Value;	             			
			return 0;			
        case REG_SPLIT_SYNC_WR_PWR:   
            if(Value >= K_LimLo_SPLIT_Pwr && Value <= K_LimHi_SPLIT_Pwr)
                Syncronize.Script_Split_Pwr = Value;	             			
			return 0;
        case REG_SPLIT_SYNC_WR_ECOMEN:        
           if(Value >= K_LimLo_SPLIT_EcoModeEn && Value <= K_LimHi_SPLIT_EcoModeEn)            
                Syncronize.EcoModeEnable = Value;	             			
			return 0;
        case REG_SPLIT_SYNC_WR_PWRECOM:        
           if(Value >= K_LimLo_SPLIT_PwrEcoMode && Value <= K_LimHi_SPLIT_PwrEcoMode)            
                Syncronize.Power_EcoMode = Value;	             			
			return 0;		
        case REG_SPLIT_WR_ECOMODE_STATUS:        
            EngineBox.Eco_Mode_Status = Value;	             			
			return 0;			
            
        case REG_SPLIT_SYNC_WR_UTA_MODE:        
            if(Value == CoreVentilazione || Value == CoreUtaAuto)
                Syncronize.Uta_Mode = Value;	             			
			return 0;		
            
        case REG_SPLIT_SYNC_WR_HEATER_PWR:        
            
            if(Value >= K_LimLo_SPLIT_HeaterPwr && Value <= K_LimHi_SPLIT_HeaterPwr)
            {
                if(Me.I_Am_SubSlave==0)
                    Syncronize.HeaterPwr = Value;	             			
                else
                    Touch[0].HeaterPwr = Value;
            }
			return 0;		
            
        case REG_SPLIT_WR_CHANNEL_GAS_REGULATION:        
            Me.Channel_Gas_Regulation = Value;	             			
			return 0;			

        case REG_SPLIT_WR_SUPERHEAT_HEAT_MIN_SETPOINT:        
            EngineBox.SuperHeat_Heat_Min_SetPoint = Value;	             			
			return 0;			

        case REG_SPLIT_WR_SUPERHEAT_HEAT_MAX_SETPOINT:        
            EngineBox.SuperHeat_Heat_Max_SetPoint = Value;	             			
			return 0;			

		case REG_SPLIT_ADD_MODULO:					// info generali
			My_Address = Value;
			return 0;
		case REG_SPLIT_SYSTEM_NUMBER_LO:
			SN1 = LoByte(Value);
			SN2 = HiByte(Value);
			return 0;
		case REG_SPLIT_SYSTEM_NUMBER_HI:
			SN3 = LoByte(Value);
			SN4 = HiByte(Value);
			return 0;
		case REG_SPLIT_FABRICATION_DAY:
			GiornoFabbricazione = Value;
			return 0;
		case REG_SPLIT_FABRICATION_MONTH:
			MeseFabbricazione = Value;
			return 0;
		case REG_SPLIT_FABRICATION_YEAR:
			AnnoFabbricazione = Value;
			return 0;
		case REG_SPLIT_CRC_16:
			CRC_Flash = Value;
			return 0;
		case REG_SPLIT_SAVE_2_E2:
			WriteE2Status = Value;
			return 0;
         
        case REG_SPLIT_ENGBOX_SUPERH_HEATCALC:    
            EngineBox.SuperH_HeatCalc = Value;
			return 0;
            
        case REG_SPLIT_ENGBOX_PRESS_LIQUID_COND:
            EngineBox.Pressure_LiquidCond = Value;            
			return 0;
            
        case REG_SPLIT_ENGBOX_SUPERH_PID_ERROR:    
            EngineBox.SuperH_Pid_Error = Value;
			return 0;

        case REG_SPLIT_ENGBOX_ALL_VALVEOPEN100P:
            EngineBox.AllValveOpen100p = Value;            
			return 0;            
            
		case REG_SPLIT_MAX_FAN_SPEED:
#if(SplitMode!=K_I_Am_UTA_and_InvFAN && SplitMode!=K_I_Am_StandardUTA && SplitMode!=K_I_Am_InverterFAN && SplitMode!=K_I_Am_ManualRegulation && SplitMode!=K_I_Am_StandardFAN)
            if(!Me.NetBSelect && !Me.SyxtSSelect)            
                Me.MaxFanVentil_Mode = Value;	       
#endif           
			return 0;                  
            
        case REG_DIAG_SPLIT_WR_MAX_VENTIL_MODE:
            DiagnosticSplit.MaxVentilMode = Value;
            //if(Syncronize.DiagWriteFanMode>DiagnosticSplit.MaxVentilMode) ELIMINATO PERCHè SE MAX VEL ERA MAGGIORE DEL MASSIMO, INIZIALIZZAVA AD UN VALORE CASUALE CARICATO ALL'AVVIO DELLA SCHEDA.
            //    Syncronize.DiagWriteFanMode=DiagnosticSplit.Ready_FanMode;
			return 0;    

        case REG_DIAG_SPLIT_WR_MIN_SET_POINT:
            DiagnosticSplit.MinSetPoint = Value;
			return 0;    

        case REG_DIAG_SPLIT_WR_MAX_SET_POINT:
            DiagnosticSplit.MaxSetPoint = Value;
			return 0;    
            
        case REG_DIAG_SPLIT_WR_HUMIDITY:
            DiagnosticSplit.Humidity = Value;
			return 0;    
            
        case REG_SPLIT_READY_OFFLINE:
            DiagnosticSplit.OffLine = Value;            
			return 0;            
            
        case REG_DIAG_SPLIT_MODDEV_TYPE:
            DiagnosticSplit.ModDevType = Value;            
			return 0;            
            
        case REG_DIAG_SPLIT_SUPERH_COOL_PID_ERROR:
            DiagnosticSplit.Superheat_Cool_PID_Error = Value;            
			return 0;

        case REG_DIAG_SPLIT_HEATER_ABIL:
            DiagnosticSplit.Heater_Abil = Value;          
			return 0;  
            
        case REG_DIAG_SPLIT_TEMPERATURE_PROBE_ERROR:
            DiagnosticSplit.Temperature_Probe_Error = Value;            
			return 0;            

        case REG_SPLIT_WR_BOX_PROBE_ERROR:
            EngineBox.Temperature_Probe_Error = Value;            
			return 0;            

        case REG_SPLIT_WR_DEHUMI_ABIL:
            EngineBox.Dehumi_Abil = Value;            
			return 0;            

        case REG_SPLIT_WR_SET_HUMI:
            EngineBox.Set_Humi = Value;            
			return 0;            
            
        case REG_SPLIT_WR_SET_DELTA_TEMP_MIN_DEHUMI:
            EngineBox.Set_Delta_Temp_Min_Dehumi = Value;            
			return 0;            

        case REG_SPLIT_WR_SET_FAN_SPEED_DEHUMI:
            EngineBox.Set_Fan_Speed_Dehumi = Value; 
			return 0;            
            
        case REG_SPLIT_WR_SET_PRESS_DEHUMI:
            EngineBox.Set_Press_Dehumi = Value; 
			return 0;            

        case REG_SPLIT_WR_SET_TEMP_AIR_OUT:
            EngineBox.Set_Temp_Air_Out = Value; 
			return 0;            
            
        case REG_SPLIT_WR_SET_TEMP_AIR_OUT_DEHUMI:
            EngineBox.Set_Temp_Air_Out_Dehumi = Value; 
			return 0;            
            
        case REG_DIAG_SPLIT_WR_GAS_LEAKS:
            EngineBox.AddressGasLeaks = Value;
			return 0;    
            
		case REG_BOARD_AUTORESET:
            Me.AutoresetMagicNumber = Value;
			if(Me.SubSlaveAddressAutoreset==0)
            {
                Me.AutoresetEnabled = 1;                    // Abilito Autoreset          
			}
            else if(Me.AutoresetMagicNumber==K_MagicNumber) // Se ricevuto comando con Value = "MagicNumber"
                Me.SetSubSlaveReset=1;                      // Setto flag che mi fa andare in modalità BUSMERGE e loopare in while nel main fino a timeout comunicazioni BUS1 o BUS2)
                                                            // e prima di loop invio comando a sottoslave di autoresettarsi
			return 0;			

		case REG_BOARD_SUBSLAVE_AUTORESET_ADDR:	
			Me.SubSlaveAddressAutoreset = Value;
			return 0;
                        
            
            
            
		default:
			return 1;
	}
}

//------------------------------------------------------------------------------
// Cambia il protocollo corrente con quello specificato: (compreso BaudRate)
// ProtocolIndex: 0=MyBus@125000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115200Baud
//------------------------------------------------------------------------------
void Change_ProtocolBusSlave(int ProtocolIndex, unsigned int N_Uart)
{
    static int LastProtocol = -1;
    
	oRTS_Slave = 0; //VERIFICARE SE SERVE VERAMENTE!!!!!!!!
	oRTS_UART3 = 0;
    
    if(Me.My_Address!=0)
        Me.ComToUart3 = N_Uart;
    else
        Me.ComToUart3 = 0;
    
	switch(ProtocolIndex)
	{
		case 0:
            if(LastProtocol != ProtocolIndex)
            {
                LastProtocol = ProtocolIndex;
                CurUsedProtocolId = FlagMyBusProt;		// Protocollo MyBUS

                if(Me.ComToUart3==1)                
                {                
                    set_baudrateU3_115200();			// @ 125000Bps
                    ClearUart3Err();            
                }
                else
                {
                    set_baudrateU2_115200();			// @ 125000Bps
                    ClearUart2Err();            
                }
                    
                DelaymSec(20); //2);
            }
		break;   
        
		case 1:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;
                CurUsedProtocolId = FlagMyBusProt;		// Protocollo MyBUS
                if(Me.ComToUart3==1)                
                {                
                    set_baudrateU3_57600(); //set_baudrateMyBUS();					// @ 57600Bps
                    ClearUart3Err();   
                }
                else
                {
                    set_baudrateU2_57600(); //set_baudrateMyBUS();					// @ 57600Bps
                    ClearUart2Err();            
                }
                    
                DelaymSec(20); //2);
            }
		break;

		case 2:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;
                CurUsedProtocolId = FlagModBusProt;		// Protocollo MODBUS

                if(Me.ComToUart3==1)                
                {                
                    set_baudrateU3_38400(); //set_baudrateModBUS();					// @ 38400Bps
                    ClearUart3Err();      
                }
                else
                {
                    set_baudrateU2_38400(); //set_baudrateModBUS();					// @ 38400Bps
                    ClearUart2Err();                                
                }
                DelaymSec(20); //2);
            }
		break;
		
		case 3:
            if(LastProtocol != ProtocolIndex)
            {       
                LastProtocol = ProtocolIndex;
                CurUsedProtocolId = FlagModBusProt;		// Protocollo MODBUS

                if(Me.ComToUart3==1)                
                {
                    set_baudrateU3_57600(); //set_baudrateMyBUS();					// @ 57600Bps
                    ClearUart3Err();
                }
                else
                {
                    set_baudrateU2_57600(); //set_baudrateMyBUS();					// @ 57600Bps
                    ClearUart2Err();                                
                }   
                DelaymSec(20); //2);
            }
		break;		
        
		case 4:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;
                CurUsedProtocolId = FlagModBusProt;		// Protocollo MODBUS

                if(Me.ComToUart3==1)                
                {                
                    set_baudrateU3_115200();           // @ 115000Bps
                    ClearUart3Err();
                }
                else
                {
                    set_baudrateU2_115200();           // @ 115000Bps
                    ClearUart2Err();                                
                }
                
                DelaymSec(20); //2);
            }
		break;	        
        
		case 5:
        //Mybys 250000 usato da master per com1 a quella velocità. non usato nello slave....
        break;	        

		case 6:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;			
                CurUsedProtocolId = FlagModBusProt;		// Protocollo MODBUS

                if(Me.ComToUart3==1)                
                {                
                    set_baudrateU3_9600();           // @ 9600Bps
                    ClearUart3Err();                         
                }
                else
                {
                    set_baudrateU2_9600();           // @ 9600Bps
                    ClearUart2Err();                             
                }
                
                DelaymSec(20); //2);
            }
		break;	                

		case 7:
            if(LastProtocol != ProtocolIndex)
            {            
                LastProtocol = ProtocolIndex;			
                CurUsedProtocolId = FlagModBusProt;		// Protocollo MODBUS
#if(K_Panel_On_Uart3==0)                
                set_baudrateU2_19200();           // @ 19200Bps
                ClearUart2Err();         
#else
                if(Me.ComToPanel==1)                
                {                
                    set_baudrateU3_19200();           // @ 19200Bps
                    ClearUart3Err();                         
                }
                else
                {
                    set_baudrateU2_19200();           // @ 19200Bps
                    ClearUart2Err();                             
                }
#endif                
                
                DelaymSec(20); //2);
            }
		break;	                        
		default:/*
                LastProtocol = ProtocolIndex;
                CurUsedProtocolId = FlagMyBusProt;		// Protocollo MyBUS
                set_baudrateU2_57600(); //set_baudrateMyBUS();					// @ 57600Bps
                ClearUart2Err();            
                DelaymSec(20); //2);
                 */
        break;
			
	}		

}