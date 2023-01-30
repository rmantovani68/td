//----------------------------------------------------------------------------------
//	Progect name:	Triac.c
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			22/05/2011
//	Description:	Corpo delle funzioni per il modulo di gestione del triac
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
#include "PWM.h"
#include "Timer.h"
#include "Core.h"
#include "PID.h"
#include "FWSelection.h"
#include "ProtocolloModBusSec.h"
#include "ADC.h"


extern unsigned ComunicationLost;				// flag di comunicazione persa


//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
// Inizializza i valori delle velocità ventialtore da costanti anziche da EEPROM
void Init_PWM_Value(void)
{
/*
	PWMValue[0] = K_FanSpeedOff;
	PWMValue[1] = K_FanSpeedNight;
	PWMValue[2] = K_FanSpeed1; 
	PWMValue[3] = K_FanSpeed2; 
	PWMValue[4] = K_FanSpeed3; 
	PWMValue[5] = K_FanSpeed4; 
	PWMValue[6] = K_FanSpeed5; 
	PWMValue[7] = K_FanSpeed6; 
	PWMValue[8] = K_FanSpeed7;	
*/

	PWMValue[0] = K_FanSpeedOff;
    
    if(Me.Ventil_Selection==K_EBM_24V)
    {    
        PWMValue[1] = K_EBM_FanSpeedNight;
        PWMValue[2] = K_EBM_FanSpeed1; 
        PWMValue[3] = K_EBM_FanSpeed2; 
        PWMValue[4] = K_EBM_FanSpeed3; 
        PWMValue[5] = K_EBM_FanSpeed4; 
        PWMValue[6] = K_EBM_FanSpeed5; 
        PWMValue[7] = K_EBM_FanSpeed6; 
        PWMValue[8] = K_EBM_FanSpeed7;	
    }
    else if (Me.Ventil_Selection==K_GR22_230V)
    {
        PWMValue[1] = K_GR22_FanSpeedNight;
        PWMValue[2] = K_GR22_FanSpeed1; 
        PWMValue[3] = K_GR22_FanSpeed2; 
        PWMValue[4] = K_GR22_FanSpeed3; 
        PWMValue[5] = K_GR22_FanSpeed4; 
        PWMValue[6] = K_GR22_FanSpeed5; 
        PWMValue[7] = K_GR22_FanSpeed6; 
        PWMValue[8] = K_GR22_FanSpeed7;	
    }    
    else if (Me.Ventil_Selection==K_GR25_230V)
    {
        PWMValue[1] = K_GR25_FanSpeedNight;
        PWMValue[2] = K_GR25_FanSpeed1; 
        PWMValue[3] = K_GR25_FanSpeed2; 
        PWMValue[4] = K_GR25_FanSpeed3; 
        PWMValue[5] = K_GR25_FanSpeed4; 
        PWMValue[6] = K_GR25_FanSpeed5; 
        PWMValue[7] = K_GR25_FanSpeed6; 
        PWMValue[8] = K_GR25_FanSpeed7;	
    }    
    else if (Me.Ventil_Selection==K_ZIEHL_ABEGG_ModBus)
    {
        PWMValue[1] = K_GRMod_FanSpeedNight;
        PWMValue[2] = K_GRMod_FanSpeed1; 
        PWMValue[3] = K_GRMod_FanSpeed2; 
        PWMValue[4] = K_GRMod_FanSpeed3; 
        PWMValue[5] = K_GRMod_FanSpeed4; 
        PWMValue[6] = K_GRMod_FanSpeed5; 
        PWMValue[7] = K_GRMod_FanSpeed6; 
        PWMValue[8] = K_GRMod_FanSpeed7;	
    }    
    else if (Me.Ventil_Selection==K_EBM_3200RPM_ModBus)
    {
        PWMValue[1] = K_GRMod_FanSpeedNight;
        PWMValue[2] = K_GRMod_FanSpeed1; 
        PWMValue[3] = K_GRMod_FanSpeed2; 
        PWMValue[4] = K_GRMod_FanSpeed3; 
        PWMValue[5] = K_GRMod_FanSpeed4; 
        PWMValue[6] = K_GRMod_FanSpeed5; 
        PWMValue[7] = K_GRMod_FanSpeed6; 
        PWMValue[8] = K_GRMod_FanSpeed7;	
    }    
}


// Trasforma il valore di PWMValue in valore freq. da passare all'inverter FAN
void PWM2Freq(void)
{
    if(Me.Pwm_Req==PWMValue[0])
    Inverter.Req_Freq = K_InverterFanSpeedOff;
    else if (Me.Pwm_Req==PWMValue[1])
    Inverter.Req_Freq = K_InverterFanSpeedNight;
    else if (Me.Pwm_Req==PWMValue[2])
    Inverter.Req_Freq = K_InverterFanSpeed1;
    else if (Me.Pwm_Req==PWMValue[3])
    Inverter.Req_Freq = K_InverterFanSpeed2;
    else if (Me.Pwm_Req==PWMValue[4])
    Inverter.Req_Freq = K_InverterFanSpeed3;
    else if (Me.Pwm_Req==PWMValue[5])
    Inverter.Req_Freq = K_InverterFanSpeed4;
    else if (Me.Pwm_Req==PWMValue[6])
    Inverter.Req_Freq = K_InverterFanSpeed5;
    else if (Me.Pwm_Req==PWMValue[7])
    Inverter.Req_Freq = K_InverterFanSpeed6;
    else if (Me.Pwm_Req==PWMValue[8])
    Inverter.Req_Freq = K_InverterFanSpeed7;
    
    
    Inverter.FreqFan = Inverter.Req_Freq;
    
}




//---------------------------------------------------------------------------
// Calcola il valore della velocità della ventola (in valore PWM) rispetto al
// delta temperatura dell'aria in Ingresso e uscita della batteria
//---------------------------------------------------------------------------
// NEW
int DeltaTempInOut(void)
{
    int ErrTemp;
    int RetVal; 
    int MaxTemp=1000;
    int TempAmbFan;
    
    //TempAmbFan=TempAmbFanSel();
	TempAmbFan=Me.TempAmbRealSelection;
    
    if(Me.Function_Mode == CoreRaffrescamento || Me.Function_Mode == CoreSlaveFreddo)
    {
#if (K_FanBrake_DeltaTempInOut == 1)		
        ErrTemp = TempAmbFan - Me.Temperature.AirOut;
#else
		ErrTemp = MaxTemp;		//Tolto freno ventola su aria in uscita
#endif
    }
    else if(Me.Function_Mode == CoreRiscaldamento || Me.Function_Mode == CoreSlaveCaldo)  
    {

#if (K_FanBrake_DeltaTempInOut == 1)	
        ErrTemp = Me.Temperature.AirOut - TempAmbFan;
#else
		ErrTemp = MaxTemp;		//Tolto freno ventola su aria in uscita
#endif		
    }
    else if(Me.Function_Mode == CoreVentilazione)
    {
		//ErrTemp = TempAmbFan - Me.Temperature.AirOut;
		ErrTemp = MaxTemp;		//Tolto freno ventola su aria in uscita		//Da decidere cosa deve essere!!
    }
   
    if(ErrTemp < 0) ErrTemp=0; 
    if(ErrTemp > MaxTemp)  ErrTemp=MaxTemp; 
    
    RetVal = (int)((float)ErrTemp / ((float)MaxTemp/255.0));	// Range 0..255
    
    if(RetVal < PWMValue[1]) RetVal = PWMValue[1];
			
    return RetVal;
}


//---------------------------------------------------------------------------
// Calcola il valore della velocità della ventola (in valore PWM) rispetto al
// delta temperatura dell'aria in uscita dalla batteria e quella richiesta di evaporazione
//---------------------------------------------------------------------------
// NEW
int DeltaTempReqOut(void)
{
    int ErrTemp;
    int RetVal; 
    int MaxTemp=1500;
    //int TempOffs=500;
    int TempAmbFan;
    
    //TempAmbFan=TempAmbFanSel();
	TempAmbFan=Me.TempAmbRealSelection;
    
    if(Me.Function_Mode == CoreRaffrescamento || Me.Function_Mode == CoreSlaveFreddo)
    {
#if (K_FanBrake_DeltaTempReqOut == 1)		
        ErrTemp = Me.Temperature.AirOut - (Me.Evap_Temp + TempOffs);
#else
		ErrTemp = 0;	//Tolto freno ventola su aria in uscita		
#endif
    }
    else if(Me.Function_Mode == CoreRiscaldamento || Me.Function_Mode == CoreSlaveCaldo)  
    {
#if (K_FanBrake_DeltaTempReqOut == 1)				
        ErrTemp = (Me.Evap_Temp - TempOffs) - Me.Temperature.AirOut;
#else
		ErrTemp = 0;	//Tolto freno ventola su aria in uscita		
#endif
		
    }
    else if(Me.Function_Mode == CoreVentilazione)
    {
		//ErrTemp = Me.Temperature.AirOut - (Me.Evap_Temp + TempOffs);
		ErrTemp = 0;	//Tolto freno ventola su aria in uscita	//Da decidere cosa deve essere!!
    }
   
    if(ErrTemp < 0) ErrTemp=0; 
    if(ErrTemp > MaxTemp)  ErrTemp=MaxTemp; 
    
    RetVal = 255 - (int)((float)ErrTemp / ((float)MaxTemp/255.0));
    
    if(RetVal < PWMValue[1]) RetVal = PWMValue[1];
            
    return RetVal;
}




void Init_PWM_Module(void)		// timer TMR2
{
	// Reset PWM 
	OC1CON1 = 0x0000;
	OC1CON2 = 0x0000;

	OC1CON2bits.FLTMD = 0;			// fault mode is maintened until the new perio starts
	OC1CON2bits.FLTOUT = 0;			// output is driven high on a fault condition
	OC1CON2bits.FLTTRIEN = 0;		// out is forced to fault condition
	OC1CON2bits.OCINV = 1;			// bit out is inverted
	OC1CON2bits.OC32 = 0;			// cascade mode is disable
	OC1CON2bits.OCTRIG = 0;			// sync whit source
	OC1CON2bits.TRIGSTAT = 0;		// Timesource has not been triggered
	OC1CON2bits.OCTRIS = 0;			// Output compare connected to OCx pin
	OC1CON2bits.SYNCSEL = 0x1F;		// sync source this module

	OC1RS = 0x0FFF;					// valore di reset del cnt
	OC1R = 0x0000;					// valore di commutazione dell'output
	OC1CON1bits.OCSIDL = 0;			// sempre on anche in idle
	OC1CON1bits.OCTSEL = 7;			// clock source is sytem clock
	OC1CON1bits.ENFLT0 = 0;			// disable fault bit
	OC1CON1bits.TRIGMODE = 0;		// trig modo is auto reset
	OC1CON1bits.OCM = 6;			// Edge-alining PWM mode
}

void VariazionePWM(void)
{
	int req = 0;
    int CurrentPWMReduce = 0;
    int CurrentPWMNotIncrement = 0;
    int CurrentPWMSlowUp = 0;
#if(K_Channel_Gas_Regulation==1)
    int FastIncrement = 30;
#elif(K_DiffPress_Fan_Control==1 || K_DiffPress_Auto_Fan_Control==1)
    int FastIncrement = 1;
#else    
    int FastIncrement = 4;
#endif    
    if((Me.Fan_Current_Limit>0) && (Me.Pwm_Value>0))
    {
        CurrentPWMReduce = Me.Fan_Current_Fast > (Me.Fan_Current_Limit);
        CurrentPWMNotIncrement = Me.Fan_Current_Fast > (Me.Fan_Current_Limit-10);
        CurrentPWMSlowUp = Me.Fan_Current_Fast > (Me.Fan_Current_Limit-30);
    }
    
	if(RefreshVentil.TimeOut)	// controllo se devo aggiornare il ventil
	{	
        //if(Me.Pwm_Req != Me.Pwm_Value)
		{
			if((Me.Pwm_Value < (PWMValue[1]-20)) & (Me.Pwm_Req < Me.Pwm_Value))	// si sta spegnendo
				Me.Pwm_Value = PWMValue[0];
			if((Me.Pwm_Value < (PWMValue[1]-10)) & (Me.Pwm_Req > Me.Pwm_Value))	// si sta accendendo
				Me.Pwm_Value = PWMValue[1]-10;
			
            if((Me.Pwm_Req < Me.Pwm_Value) || CurrentPWMReduce)
			{
				req = Me.Pwm_Value - Me.Pwm_Req;
/*                
#if(K_Channel_Gas_Regulation==1)
                if(req >= 30)
                    req = 30;
                if(req >= 5)
                    Decrementa(&(Me.Pwm_Value), req, PWMValue[0]);
				else if(req >= 4)
#else
				if(req >= 4)
#endif
					Decrementa(&(Me.Pwm_Value), 4, PWMValue[0]);
				else if (req == 3)
					Decrementa(&(Me.Pwm_Value), 3, PWMValue[0]);
				else if (req == 2)
					Decrementa(&(Me.Pwm_Value), 2, PWMValue[0]);
				else if (req == 1)
					Decrementa(&(Me.Pwm_Value), 1, PWMValue[0]);				
                else if(CurrentPWMReduce)
                    Decrementa(&(Me.Pwm_Value), 1, PWMValue[0]);
 */

                if(req > FastIncrement)
                    req = FastIncrement;

                if((req <= 0) && CurrentPWMReduce)
                    req = 1;
                
                Decrementa(&(Me.Pwm_Value), req, PWMValue[0]);
			}
			else if((Me.Pwm_Req > Me.Pwm_Value) && !CurrentPWMNotIncrement)
			{
				req = Me.Pwm_Req - Me.Pwm_Value;
                if(CurrentPWMSlowUp)
                    Incrementa(&(Me.Pwm_Value), 1, PWMValue[8]);
                else
                {
                    if(req > FastIncrement)
                        req = FastIncrement;

                    Incrementa(&(Me.Pwm_Value), req, PWMValue[8]);

/*
                    if(req >= 30)
                        req = 30;
                    if(req >= 5)
                        Incrementa(&(Me.Pwm_Value), req, PWMValue[8]);
    				else if(req >= 4)
#else
                    if(req >= 4)
#endif
                        Incrementa(&(Me.Pwm_Value), 4, PWMValue[8]);
                    else if (req == 3)
                        Incrementa(&(Me.Pwm_Value), 3, PWMValue[8]);
                    else if (req == 2)
                        Incrementa(&(Me.Pwm_Value), 2, PWMValue[8]);
                    else if (req == 1)
                        Incrementa(&(Me.Pwm_Value), 1, PWMValue[8]);				
 */
                }
			}
		}
		RefreshVentil.Value = 0;
		RefreshVentil.TimeOut = 0;
	}

	OC1R = ((Me.Pwm_Value & 0x00FF) <<4);		// aggiorno il modulo OC
}


// NEW sostituisce -> FineRegVentil(int SpeedLimit)
int FanSpeedLimit(int SpeedReq)
{
	/**/
	int TempErr;
	float DeltaV;
	float DeltaT;
	float KDiv;
	int RetVal;
    int TempAmbFan;
    
    //TempAmbFan=TempAmbFanSel();
	TempAmbFan=Me.TempAmbRealSelection;
	
	Me.Fan_MaxSpeed = Filter(&FilterFan_MaxSpeed, ((DeltaTempReqOut() + DeltaTempInOut()) / 2), K_Campioni_FilterFanSpeed);
	if(SpeedReq==K_FanSpeedAuto)
	{
#if(K_DiffPress_Fan_Control==1)
        RetVal = DeltaSP2VelDiffPress((Me.SP/100), Me.TempAmbRealSelection);
#else
		if((Me.Function_Mode == CoreRiscaldamento) | (Me.Function_Mode == CoreSlaveCaldo))	// Se deve riscaldare
			TempErr = (Me.SP - TempAmbFan);
		else if((Me.Function_Mode == CoreRaffrescamento) | (Me.Function_Mode == CoreSlaveFreddo))	// Se deve raffrescare										// Se deve raffrescare
			TempErr = (TempAmbFan - Me.SP);		
		else if(Me.Function_Mode == CoreVentilazione)	// Se deve ventilare
#if(K_AutoTempFanControl==1)
			TempErr = (TempAmbFan - Me.SP);		
#elif(K_Heater_Abil)
			TempErr = 60;		
#else
			TempErr = 25;
#endif
        RetVal = DeltaSP2Vel(TempErr);
#endif
	}
	else
	{
		RetVal = SpeedReq;
	}

#if(K_Heater_Abil==1)
#if(K_Humidity_Correct==1)
    if((Touch[0].HeaterPwr==1) && (Me.Error.Fan_Fault==0) && (Me.Fan_Tacho>0) && (Me.Humidity_Correct==1) && (RetVal<PWMValue[EngineBox.Set_Fan_Speed_Dehumi]) || Me.HeaterFresh)
        RetVal=PWMValue[EngineBox.Set_Fan_Speed_Dehumi];
#else
    if(((Touch[0].FunctionMode==CoreVentilazione) || (Touch[0].FunctionMode==CoreRiscaldamento)) && (Touch[0].HeaterPwr==1) && 
        (Me.Error.Fan_Fault==0) && (Me.Fan_Tacho>0) && (Me.TempAmbRealSelection < (Me.SP - Caldo_Ist_SP) && (RetVal<PWMValue[3])) || Me.HeaterFresh)
        RetVal=PWMValue[3];    
#endif
#endif
	if(RetVal < PWMValue[1])		// Se velocità ventil calcolata è inferiore a Night .... limito
		RetVal = PWMValue[1];
	
	if(RetVal>Me.Fan_MaxSpeed)			// Se velocità ventil calcolata è superiore a massimo consentito da calcoli precedenti... limito
		RetVal=Me.Fan_MaxSpeed;
	
	
	
	return RetVal;
}

// NEW sostituisce -> AggiornaVentil(void)
void UpdateFanSpeed(void)
{
    static int LastVelPWM = 0;
    static int FanVelSwitch = 0;
    static int PWM_AutoPress_On = 0;
	
	//Me.Error.Battery_Defrost = 0;
    /*
	if((Me.Temperature.Gas < SogliVentNightBatteriaGhiacciata) && (Me.Function_Mode == CoreOff))
	{
		TimerCheckBatteryDefrost.Enable = 1;
		
		if(VentilBatteryDefrost==0)
			TimerCheckBatteryDefrost.Time = 300;
		else
			TimerCheckBatteryDefrost.Time = 60;
			
		if(TimerCheckBatteryDefrost.TimeOut)
		{
			Me.Error.Battery_Defrost = 1;
			TimerCheckBatteryDefrost.TimeOut = 0;
			TimerCheckBatteryDefrost.Value = 0;
			
			if(VentilBatteryDefrost==0)
				VentilBatteryDefrost = 1;
			else if(VentilBatteryDefrost==1)
				VentilBatteryDefrost=0;
		}			
	}
	else
	{
		Me.Error.Battery_Defrost = 0;
		TimerCheckBatteryDefrost.Enable = 0;
		TimerCheckBatteryDefrost.TimeOut = 0;
		TimerCheckBatteryDefrost.Value = 0;
		VentilBatteryDefrost = 0;
	}
	*/
	if(Me.Error.BatteriaGhiacciata)
	{	
		Me.Pwm_Req= PWMValue[1];
	}
	else if(Me.Error.Battery_Defrost)
	{
        /*
		if(VentilBatteryDefrost==1)
			Me.Pwm_Req= PWMValue[1];
		else
			Me.Pwm_Req= PWMValue[0];
        */
        Me.Pwm_Req= LastVelPWM;														// lascio il ventilatore all'ultimo valore

	}
	//else if((Me.Function_Mode != 0) & ((EngineBox.Error.CompressorLo & EngineBox.InverterRiscaldamento) | 
	//	(EngineBox.Error.CondensatoreLo) | (EngineBox.Error.Gas_Recovery)))
	//	Me.Pwm_Req= PWMValue[2];														// faccio girare a 2 il ventilatore
#if(SplitMode!=K_I_Am_UTA_and_InvFAN && SplitMode!=K_I_Am_StandardUTA && SplitMode!=K_I_Am_InverterFAN && SplitMode!=K_I_Am_StandardFAN && SplitMode!=K_I_Am_Atex_Fan && SplitMode!=K_I_Am_On_Off_Fan)
	else if(EngineBox.Test.FluxReq)// | EngineBox.Test.EEV_Req | EngineBox.Test.EEV_Go)	// se ho un test
		Me.Pwm_Req= PWMValue[0];														// fermo il ventilatore
#endif
    else if(EngineBox.Error.Defrosting)
        Me.Pwm_Req= LastVelPWM;														// lascio il ventilatore all'ultimo valore
    else if(Me.Humidity_Correct==1)
        Me.Pwm_Req= PWMValue[EngineBox.Set_Fan_Speed_Dehumi];
	else
	{	
		switch(Me.Function_Mode)
		{	
            case CoreRaffrescamento:
			case CoreRiscaldamento:
			case CoreVentilazione:
			case CoreSlaveCaldo:
			case CoreSlaveFreddo:	
#if(K_Channel_Gas_Regulation==1)
					Me.Pwm_Req = Me.Channel_Gas_Regulation;
#elif(K_DiffPress_Auto_Fan_Control==1)
                    if(Me.Temperature.Ambient<K_DiffPress_Auto_Min)
                        PWM_AutoPress_On = 0;
                    else if(Me.Temperature.Ambient>K_DiffPress_Auto_Min)
                        PWM_AutoPress_On = 1;
					Me.Pwm_Req = ValueIn2ValueOut(Me.Temperature.Ambient, K_DiffPress_Auto_Min, K_DiffPress_Auto_Max, PWMValue[1], PWMValue[8])*(PWM_AutoPress_On==1);   
#else
					if(Me.Ventil_Mode==Ventil_Auto)
					{
						Me.Pwm_Req = FanSpeedLimit(K_FanSpeedAuto);
					}		
					else if(Me.Ventil_Mode==Ventil_Off)		Me.Pwm_Req = FanSpeedLimit(PWMValue[0]);
					else if(Me.Ventil_Mode==Ventil_Night)	Me.Pwm_Req = FanSpeedLimit(PWMValue[1]);
					else if(Me.Ventil_Mode==Ventil_1) Me.Pwm_Req = FanSpeedLimit(PWMValue[2]);
					else if(Me.Ventil_Mode==Ventil_2) Me.Pwm_Req = FanSpeedLimit(PWMValue[3]);
					else if(Me.Ventil_Mode==Ventil_3) Me.Pwm_Req = FanSpeedLimit(PWMValue[4]);
					else if(Me.Ventil_Mode==Ventil_4) Me.Pwm_Req = FanSpeedLimit(PWMValue[5]);
					else if(Me.Ventil_Mode==Ventil_5) Me.Pwm_Req = FanSpeedLimit(PWMValue[6]);
					else if(Me.Ventil_Mode==Ventil_6) Me.Pwm_Req = FanSpeedLimit(PWMValue[7]);
					else if(Me.Ventil_Mode==Ventil_7) Me.Pwm_Req = FanSpeedLimit(PWMValue[8]);
#endif
                    LastVelPWM = Me.Pwm_Req;
				break;

			case CoreOff:					// lo split richiede lo spegnimento
                if(Me.Error.Fan_Fault)
                {
                    //Me.Pwm_Req = PWMValue[2];
                    if(TimerFanVelSwitch.TimeOut)
                    {

                        TimerFanVelSwitch.TimeOut = 0;
                        TimerFanVelSwitch.Value = 0;       
                        if(FanVelSwitch)
                        {
                            Me.Pwm_Req = PWMValue[2];				// Vel. 1
                            TimerFanVelSwitch.Time = 30;//15;
                        }
                        else
                        {
                            Me.Pwm_Req = PWMValue[0];				// Vel. 0 -> OFF                      
                            TimerFanVelSwitch.Time = 180;
                        }

                        FanVelSwitch = !FanVelSwitch;
                    }                        
                }
                else if(Me.Error.Fan_OverLoad || Me.Error.FireAlarm || (EngineBox.PersErr.FireAlarm && K_Abil_Off_FireAlarm==1))
                {
                    Me.Pwm_Req = PWMValue[0];
                }
#if(SplitMode!=K_I_Am_StandardFAN && SplitMode!=K_I_Am_InverterFAN && SplitMode!=K_I_Am_Atex_Fan && SplitMode!=K_I_Am_On_Off_Fan)                
                else if(EngineBox.Error.MasterRestart)
                {
                    Me.Pwm_Req = PWMValue[0];
                }
                else if(((EngineBox.RealFuncMode == EngineBox_GoOff)&&(EngineBox.Compressor_Speed>0 || EngineBox.Compressor_Speed_C2>0)) ||
                        ((Me.ExpValve_Ctrl > Me.ValveLim.ValvolaChiusa)&&EngineBox.RealFuncMode == EngineBox_Freddo) ||
                        ((Me.ExpValve_Ctrl > Me.ValveLim.ValvolaChiusa)&&EngineBox.RealFuncMode == EngineBox_Caldo))
                        /*
                        ((Me.ExpValve_Ctrl > Me.ValveLim.AperturaMinCaldo)&&(Touch[0].FunctionMode == CoreRiscaldamento)&&EngineBox.RealFuncMode == EngineBox_Caldo) || 
                        ((Me.ExpValve_Ctrl > Me.ValveLim.ValvolaChiusa)&&(Touch[0].FunctionMode == CoreOff)&&EngineBox.RealFuncMode == EngineBox_Caldo))*/
                {       
                    Me.Pwm_Req = LastVelPWM;
                }                
#if(K_Heater_Abil==1)
#if(K_Humidity_Correct==1)                
                else if(Touch[0].FunctionMode==CoreRaffrescamento && (Me.TempAmbRealSelection < (Me.SP - 20)) && (Touch[0].HeaterPwr==1))
                {
                    Me.Pwm_Req = PWMValue[EngineBox.Set_Fan_Speed_Dehumi];	// speed 6
                    LastVelPWM = Me.Pwm_Req;
                }
#else
                else if(Touch[0].FunctionMode==CoreRaffrescamento && (Me.TempAmbRealSelection < (Me.SP - 70)) && (Touch[0].HeaterPwr==1))
                {
                    Me.Pwm_Req = PWMValue[3];	// speed 6
                    LastVelPWM = Me.Pwm_Req;
                }
#endif
                else if(iDigitOutHeater==1 || Me.HeaterFresh==1)
                {
                    if(Me.Ventil_Mode > 8)
                    {
                        Me.Pwm_Req = PWMValue[5];	// speed 6
                    }
                    else
#if(K_Humidity_Correct==1)
                        Me.Pwm_Req = PWMValue[EngineBox.Set_Fan_Speed_Dehumi];	// actual speed                    
#else
                        Me.Pwm_Req = PWMValue[3];	// actual speed                    
#endif
                    LastVelPWM = Me.Pwm_Req;
                }                    
#endif
/*
                else if(Me.Temperature.Error.Bit.Gas || Me.Temperature.Error.Bit.Liquid)    //Azzera dopo aver spento unità
                {
                    Me.Pwm_Req = PWMValue[0];
                    LastVelPWM = Me.Pwm_Req;
                }                                
*/
            #if(FanOldStyleHeatCoreOff || (SplitMode == K_I_Am_UTA_and_InvFAN) || (SplitMode == K_I_Am_StandardUTA))
                else if(((Touch[0].FunctionMode == CoreRiscaldamento) ||
                    (Touch[0].FunctionMode == CoreRaffrescamento)) && 
                    (((((EngineBox.CumulativeError & 0x0002)==0) || ((EngineBox.CumulativeError_C2 & 0x0002)==0)) && EngineBox.DoubleCompressorOn!=0) ||
                    (((EngineBox.CumulativeError & 0x0002)==0) && EngineBox.DoubleCompressorOn==0)))		// se comunque è acceso e non ci sono PersErr delle 2 unità
                {	

                    if(Me.Ventil_Mode > 8)
                    {
                        Me.Pwm_Req = PWMValue[1];				// speed 1
                    }
                    else
                        Me.Pwm_Req = PWMValue[Me.Ventil_Mode];	// actual speed

                    LastVelPWM = Me.Pwm_Req;
                }
            #else
                else if((Touch[0].FunctionMode == CoreRaffrescamento) && 
                    (((((EngineBox.CumulativeError & 0x0002)==0) || ((EngineBox.CumulativeError_C2 & 0x0002)==0)) && EngineBox.DoubleCompressorOn!=0) ||
                    (((EngineBox.CumulativeError & 0x0002)==0) && EngineBox.DoubleCompressorOn==0)))		// se comunque è acceso e non ci sono PersErr delle 2 unità
                {	

                    if(Me.Ventil_Mode > 8)
                    {
                        Me.Pwm_Req = PWMValue[1];				// speed night //PWMValue[2];				// speed 1
                    }
                    else
                        Me.Pwm_Req = PWMValue[Me.Ventil_Mode];	// actual speed

                    LastVelPWM = Me.Pwm_Req;
                }
                else if((Touch[0].FunctionMode == CoreRiscaldamento) && 
                    (((((EngineBox.CumulativeError & 0x0002)==0) || ((EngineBox.CumulativeError_C2 & 0x0002)==0)) && EngineBox.DoubleCompressorOn!=0) ||
                    (((EngineBox.CumulativeError & 0x0002)==0) && EngineBox.DoubleCompressorOn==0)))		// se comunque è acceso e non ci sono PersErr delle 2 unità
                {	
                    if(Me.FanNewStyleTimeOn>0 && Me.FanNewStyleTimeOff>0)
                    {
                        if(TimerFanVelSwitch.TimeOut)
                        {

                            TimerFanVelSwitch.TimeOut = 0;
                            TimerFanVelSwitch.Value = 0;       
                            if(FanVelSwitch)
                            {
                                Me.Pwm_Req = PWMValue[1];				// Vel. Night
                                TimerFanVelSwitch.Time = Me.FanNewStyleTimeOn;//K_FanNewStyleHeatTimeON;
                            }
                            else
                            {
                                Me.Pwm_Req = PWMValue[0];				// Vel. 0 -> OFF                      
                                TimerFanVelSwitch.Time = Me.FanNewStyleTimeOff;//K_FanNewStyleHeatTimeOFF;
                            }

                            FanVelSwitch = !FanVelSwitch;
                        }                        
                    }
                    else if(Me.FanNewStyleTimeOn==0)
                    {
                        Me.Pwm_Req = PWMValue[0];				// Vel. 0 -> OFF
                    }
                    else if(Me.FanNewStyleTimeOff==0)
                    {
                        Me.Pwm_Req = PWMValue[1];				// Vel. Night
                    }                    

                    LastVelPWM = Me.Pwm_Req;
                }
            #endif
#endif
                else
                {	
                    //Me.Old_Function_Mode = CoreOff;
                    Me.Pwm_Req = PWMValue[0];
                    if(Me.Pwm_Value==PWMValue[0])
                        LastVelPWM = PWMValue[0];
                }
            break;

        default:
            Me.Pwm_Req = PWMValue[0];
            if(Me.Pwm_Value==PWMValue[0])
                LastVelPWM = PWMValue[0];
            break;
		}
	}
    
#if(SplitMode!=K_I_Am_UTA_and_InvFAN && SplitMode!=K_I_Am_StandardUTA && SplitMode!=K_I_Am_InverterFAN && SplitMode!=K_I_Am_StandardFAN && SplitMode!=K_I_Am_Atex_Fan && SplitMode!=K_I_Am_On_Off_Fan)
	if(EngineBox.Eco_Mode_Status==1)
	{
		if(Me.Pwm_Req > PWMValue[4])				// Vel. 3
		{
			Me.Pwm_Req = PWMValue[4];
			LastVelPWM = Me.Pwm_Req;
		}
	}
#if(K_Heater_Abil==1)
	if(EngineBox.RealFuncMode == EngineBox_CriticalFault && (Touch[0].FunctionMode!=CoreVentilazione) && ((Touch[0].HeaterPwr==0) || (Me.Pwm_Req<PWMValue[3])) && !Me.Error.Fan_Fault)
#else	
	if(EngineBox.RealFuncMode == EngineBox_CriticalFault && (Touch[0].FunctionMode!=CoreVentilazione) && !Me.Error.Fan_Fault)		
#endif
		Me.Pwm_Req = PWMValue[0];
#endif
    
    //DEBUG    
    //if (!iDigiIn2)
    //    Me.Pwm_Req = 255;
    //DEBUG
	
    VariazionePWM();	
	
}



void FineRegVentil(int SpeedLimit)
{	int EvapErr = 0;
	static int AbilCambioVentola =0;


	if(Me.Pwm_Req == PWMValue[0])
		Me.Pwm_Req = PWMValue[1];

	if(RefreshFineVentil.TimeOut)
	{
		RefreshFineVentil.TimeOut =0;
		RefreshFineVentil.Value = 0;

		if((Me.Function_Mode == CoreRiscaldamento) | (Me.Function_Mode == CoreSlaveCaldo))		// Se deve riscaldare
			{
			EvapErr = (Me.Evap_Temp - Me.Temperature.AirOut);
			AbilCambioVentola = 1;
			}
		else if((Me.Function_Mode == CoreRaffrescamento) | (Me.Function_Mode == CoreSlaveFreddo))	// Se deve raffrescare
			{
			EvapErr = (Me.Temperature.AirOut - Me.Evap_Temp);//(Me.Evap_Temp + 500));
            /*
			if(Me.Temperature.Gas > (Me.Temperature.Liquid+1000))
				AbilCambioVentola = 0;
			else
				AbilCambioVentola = 1;
            */
            AbilCambioVentola = 1;
			}
		else											// se sono in altre condizioni (ventilazione)
			{
			EvapErr = -300;
			AbilCambioVentola = 1;
			}
			
			if(EvapErr > 1600)				// 1600
				Me.Pwm_Req = PWMValue[1];
			else
			if(AbilCambioVentola == 1)
			{
				if(EvapErr > 2000)               	// 1200
					Me.Pwm_Req = PWMValue[2];
				else if(EvapErr > 1800)          	// 1200
					Me.Pwm_Req = PWMValue[3];
				else if(EvapErr > 1600)          	// 1000
					Me.Pwm_Req = PWMValue[4];
				else if(EvapErr > 1400)          	// 800
					Me.Pwm_Req = PWMValue[5];
				else if(EvapErr > 1200)              // 600
					Me.Pwm_Req = PWMValue[6];
				else if(EvapErr > 1000)              // 400
					Me.Pwm_Req = PWMValue[7];
				else
					Me.Pwm_Req = PWMValue[8];
			}
/*		if(EvapErr > 200)
			Decrementa(&(Me.Pwm_Req), 5, PWMValue[1]);
		else if(EvapErr < -200)
			Incrementa(&(Me.Pwm_Req), 5, SpeedLimit);	*/
	

		if(Me.Pwm_Req > SpeedLimit)
			Me.Pwm_Req = SpeedLimit;
	}
}

void ControlloVentil(void)
{	
    static int ControlFanFault = 0;
    
#if (K_AbilFanInverter==1)	
    Me.Error.Fan_Fault = Inverter.FanInverterFault; // Inserire gestione allarme FAN speed
		if(Me.Error.Fan_Fault)
			Me.Pwm_Value = 0;
#else    
    
    TimerCheckFanTacho.Enable = (Me.Pwm_Value >= PWMValue[1]) && (Me.Pwm_Req >= PWMValue[1]) && (ControlFanFault == 0);  //PWMValue[1])
    
    if(!TimerCheckFanTacho.Enable)
    {
        TimerCheckFanTacho.TimeOut = 0;
        TimerCheckFanTacho.Value = 0;        
    }    
    
    if(TimerCheckFanTacho.TimeOut)
	{
		ControlFanFault = 1;
        TimerCheckFanTacho.TimeOut = 0;
        TimerCheckFanTacho.Value = 0;        
	}
	if((Me.Fan_Tacho == 0) && (ControlFanFault == 1) && (Me.Error.Fan_Fault == 0))
    {
		Me.Error.Fan_Fault = 1;
		Me.Pwm_Value = 0;	        
	}
       
	if((Me.Pwm_Req < PWMValue[1]) || (Me.Error.Fan_Fault == 1))
		ControlFanFault = 0;
    
   if(Me.Fan_Tacho != 0)
        Me.Error.Fan_Fault = 0;    
#endif    
}

void Tacho_Init(void)
{
	T3CON = 0;				// Resetto il registro di controllo		
	T3CONbits.TSIDL = 0;	// Setto il modulo per lavorare anche in idle mode
	T3CONbits.TCKPS = 0;	// Setto il prescaler 0=>1; 1=>8; 2=>64; 3=>256
	T3CONbits.TCS = 1;		// seto la sorgente del clock esterna
	T3CONbits.TON = 1;		// Setto a modalita di lettura e scrittura @ 16bit

	IFS0bits.T3IF = 0;		// resetto il flag del TMR2
	IPC2bits.T3IP = 2;		// imposto la priorità al massimo
	IEC0bits.T3IE = 0;		// siabilito l'interrupt
}

void TachoInterrupt(void)
{
    int Timer = TMR3;
    if(Timer>2)
        TachoTMR = Timer;
    else
        TachoTMR = 0;

    TMR3 = 0;
    RefreshTacho(); 

    /*
		TachoTMR = TMR3;
		TMR3 = 0;
        RefreshTacho();
    */
}

void RefreshTacho(void)
{

#if (K_AbilFanInverter==1)
    Me.Fan_Tacho = (unsigned int)( ((float)Me.Pwm_Value / (float)255) * 100.0); //7200; // Da inserire formula x ricavare velocità RPM da freq. Inverter!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 	
#elif(K_Evaporator_FAN==1)
    Me.Fan_Tacho = 7200*iDigitEvapFan;
#elif(SplitMode==K_I_Am_Atex_Fan)
    Me.Fan_Tacho = 2200*iDigiIn1;
#elif(SplitMode==K_I_Am_On_Off_Fan)
#if (K_AlarmEnable==1)
    Me.Fan_Tacho = 2200*iDigiIn1;
#else    
    Me.Fan_Tacho = 2200*iDigitFan;
#endif
#else
if(Me.Ventil_Selection==K_EBM_24V)    
	Me.Fan_Tacho = TachoTMR/3 * 60;			// TachoTMR = 3 impulsi x giro contati in 1000mS / 3 * 60 sec. -> giri al min.
else if(Me.Ventil_Selection==K_GR22_230V || Me.Ventil_Selection==K_GR25_230V)    
	Me.Fan_Tacho = TachoTMR * 60;			// TachoTMR = 1 impulso x giro contati in 1000mS * 60 sec. -> giri al min.

    Me.Fan_Tacho = Filter(&FilterFan_Tacho, Me.Fan_Tacho, K_Campioni_Filter_FanTacho);
#endif    
}

/*  *****************************************************************************************************************************
	Gestico l'incremento di una variabile
	***************************************************************************************************************************** */
unsigned char Incrementa(volatile int * Variabile, int Value, int Max)
{
	if(((*Variabile) + Value) <  Max)	// verifico se posso incrementare senza sfondare il limite massimo
	{	
		(*Variabile) += Value;			// incremento
		return 1;						// ritorno che ho modificato il valore
	}
	else								// altrimenti se non sono nelle condizioni ideali
	{
		if((*Variabile) < Max)			// verifico se la variabile è m inore del massimo
		{
			(*Variabile) = Max;			// quindi la setto al massimo
			return 1;					// e ritorno la modifica del valore
		}
		else if((*Variabile) > Max)		// altrimenti controllo se è già al di sopra del valore massimo
		{
			(*Variabile) -= Value;		// e gli sottraggo il valore di variazione
			if((*Variabile)<Max)
				(*Variabile)=Max;
			return 1;
		}
		else
			return 0;					// altrimenti sono esattamente a minimo e non modifico nulla
	}
}

/*  *****************************************************************************************************************************
	Gestico il decremento di una variabile
	***************************************************************************************************************************** */
unsigned char Decrementa(volatile int * Variabile, int Value, int Min)
{
	if(((*Variabile) - Value) > Min)	// verifico se posso decrementare senza sfondare il limite minimo
	{	
		(*Variabile) -= Value;			// decremento 
		return 1;						// ritorno che ho modificato il valore
	}
	else								// altrimenti se non sono nelle condizioni ideali
	{
		if((*Variabile) > Min)			// verifico se la variabile è maggiore del minimo
		{
			(*Variabile) = Min;			// quindi la setto al minimo
			return 1;					// e ritorno la modifica del valore
		}
		else if((*Variabile) < Min)		// altrimenti controllo se è già al di sotto del valore minimo
		{
			(*Variabile) += Value;		// e gli sommo il valore di variazione
			if((*Variabile)>Min)
				(*Variabile)=Min;
			return 1;
		}
		else
			return 0;					// altrimenti sono esattamente a minimo e non modifico nulla
	}
}



int TempAmbFanSel(void)
{
    int TempAmbFan;
#if(K_Mid_SlaveSec_Temp_Amb_Fan==1)
    if(!Me.I_Am_SubSlave)
    {
        if(EnableMySecBus)
            TempAmbFan=Me.TempAmbFan;
        else
            TempAmbFan=Me.Temperature.Ambient;
    }
    else
    {
        if(!ComunicationLost)
            TempAmbFan=Me.TempAmbFan;
        else
            TempAmbFan=Me.Temperature.Ambient;
    }
#else
        TempAmbFan=Me.Temperature.Ambient;
#endif
        return TempAmbFan;
}

int DeltaSP2Vel(int TempErr)
{
    unsigned int RetVal;	

	//if(TempErr < -200) TempErr = -200;
	//if(TempErr > 1900) TempErr = 1900;
	
    if(TempErr <-20)   return PWMValue[1];
	if(TempErr <= 0)    return (int)(ValueIn2ValueOut(TempErr, -20, 0, PWMValue[1], PWMValue[2])); 
	if(TempErr <= 30)  return (int)(ValueIn2ValueOut(TempErr, 0, 30, PWMValue[2], PWMValue[3])); 
	if(TempErr <= 80)  return (int)(ValueIn2ValueOut(TempErr, 30, 80, PWMValue[3], PWMValue[4])); 
	if(TempErr <= 120) return (int)(ValueIn2ValueOut(TempErr, 80, 120, PWMValue[4], PWMValue[5])); 
	if(TempErr <= 140) return (int)(ValueIn2ValueOut(TempErr, 120, 140, PWMValue[5], PWMValue[6])); 
	if(TempErr <= 160) return (int)(ValueIn2ValueOut(TempErr, 140, 160, PWMValue[6], PWMValue[7])); 
	if(TempErr <= 190) return (int)(ValueIn2ValueOut(TempErr, 160, 190, PWMValue[7], PWMValue[8])); 
    if(TempErr > 190)  return PWMValue[8]; 
	
	//return RetVal;    
}

int DeltaSP2VelDiffPress(unsigned int SetpAuto, int DiffPress)
{
    int RetVal = Me.Pwm_Req;
	int Errore = 0;
    //static int RetVal = 0;
    Errore = DiffPress - SetpAuto;
    if(Errore>0)
    {	
        if(Errore>=1)
            Incrementa(&RetVal, 1, PWMValue[8]);		// Apro valvola fino a MAX per alzare la temperatura
    }
    else
    {
        Errore = SetpAuto - DiffPress;		
        if(Errore>0)
        {	
            if(Errore>=1)
                Decrementa(&RetVal, 1, PWMValue[1]);	// Chiudo valvola fino a MIN per abbassare la temperatura
        }
    }
    return RetVal;
}

void FanPowerCalculation(void)
{
    int Mem_Adc_Temp;
    int FanCurrentTemp;    
#define     K_CurrentZero   5
#if (K_AbilFanInverter==1)	 
//    da sistemare vlori e scala
    Me.Fan_Current = 0;//Inverter.Out_Current;
    Me.Fan_Power = 0;//Inverter.Out_Power*100;
    //Me.Fan_Current = Inverter.Out_Current;
    //Me.Fan_Power = Inverter.Out_Power*100;
#elif (SplitMode==K_I_Am_LoopWater)
#if(K_LoopDoublePumpEnable==1)
    if(iDigitPumpLoop1 || iDigitPumpLoop2)
#else
    if(iDigitPumpLoop1)
#endif
    {
         FanCurrentTemp = ((349.0)/10.0);
         Me.Fan_Current = (unsigned int) ((348.0)/10.0);
         //Me.Fan_Current = Filter(&FilterFan_Power, Me.Fan_Current, K_Campioni_FilterFan_Power);    
         Me.Fan_Power = (unsigned int)(FanCurrentTemp * 230.0);   
    }
#else
    if(Me.Ventil_Selection==K_EBM_24V)
    {
        Mem_Adc_Temp = Adc_Temp[7];
        
        if(Mem_Adc_Temp<=K_CurrentZero)
            Mem_Adc_Temp=0;

        Me.Fan_Current_Fast = (int)round((float)Mem_Adc_Temp * 0.9775171);
        Me.Fan_Current_Fast = Filter(&FilterFan_FastCurrent, Me.Fan_Current_Fast, 5); // Riscalo per avere centesimi di "A"



        Me.Fan_Current = (unsigned int)((float)Filter(&FilterFan_Current, Mem_Adc_Temp, K_Campioni_FilterFan_Current) * (float)0.9775171); // Riscalo per avere centesimi di "A"
                                                                             // Fondoscala misura corrnte FAN: 10A
                                                                             // Risoluzione: 9.775 mA x bit
                                                                             // Calcolo Potenza in decimi di Watt @ 24.0V
                                                                             // Circa 9.775 mA x bit -> 9.775 * 1023 = 9999.8 mA -> 10A
        Me.Fan_Power = (unsigned int)(((float)Me.Fan_Current * (float)Me.Supply_Voltage)/100.0);     // Riscalo per avere scala in centesimi di Watt
        Me.Fan_Power = Filter(&FilterFan_Power, Me.Fan_Power, K_Campioni_FilterFan_Power);    
    }
    else// if(Me.Ventil_Selection==K_GR22_230V || Me.Ventil_Selection==K_GR25_230V)
    {
        Me.Fan_Current_Fast = 0;
        
        if(Me.Pwm_Value>=PWMValue[1] && !Me.Error.Fan_Fault)
        {
            Me.Fan_Current = (unsigned int) (ValueIn2ValueOut((float)Me.Pwm_Value,(float)35,(float)255,(float)Me.Min_Current_Fan,(float)Me.Max_Current_Fan)/10.0);
            Me.Fan_Current = Filter(&FilterFan_Power, Me.Fan_Current, K_Campioni_FilterFan_Power);    
            Me.Fan_Power = (unsigned int)(((float)Me.Fan_Current * (float)Me.Supply_Fan) * ((float)Me.Efficiency_Fan/100.0));
        }
        else
        {
            Me.Fan_Current = 0;
            Me.Fan_Power = 0;
        }
    }
#endif
}

#if(K_Evaporator_FAN==1)
void FAN_Rele_Evaporator(void)
{
    oDigitEvapFan = ((Touch[0].Script_Split_Pwr==1)&&(EngineBox.SystemDisable==0)) || (Me.Pwm_Req>0) || Me.Error.Battery_Defrost || Me.Error.Battery_Drip;
}
#endif
    

void FanM3hCalculation(void)
{
    if(Me.Pwm_Value>=PWMValue[1] && !Me.Error.Fan_Fault)
    {
        Me.Fan_M3h = (unsigned int) ValueIn2ValueOut((float)Me.Pwm_Value,(float)35,(float)255,(float)Me.M3h_Min,(float)Me.M3h_Max);
        Me.Fan_M3h = Filter(&FilterFan_M3h, Me.Fan_M3h, K_Campioni_FilterFan_M3h);
    }
    else
    {
        Me.Fan_M3h = 0;
    }
}

#if(SplitMode==K_I_Am_Atex_Fan)
void AtexFanControl(void)
{
    static int BloccoPumpLoop = 0;
    static int OldPumpLoop = 0;

    Me.Pump_Work = iDigitFan;    

        
    if((BloccoPumpLoop==0 && Me.Error.Flow==1) || Touch[0].FunctionMode==0)
        oDigitFan = 0;
    else if(BloccoPumpLoop==0 && Touch[0].FunctionMode!=0)
        oDigitFan = 1;

    if(oDigitFan!=OldPumpLoop)
    {
        OldPumpLoop = oDigitFan;
        BloccoPumpLoop = 1;
    }
    if(BloccoPumpLoop>0)
    {
        BloccoPumpLoop++;
        if(BloccoPumpLoop>60)//Tempo di ciclo (500ms) x2 = 30 sec.
            BloccoPumpLoop = 0;
    } 

    AtexTachoControl();
}

void AtexTachoControl(void)
{
	TimerErrFlow.Enable = !iDigiIn1 && iDigitFan;

    if(!TimerErrFlow.Enable)
	{	
		TimerErrFlow.Value = 0;
		TimerErrFlow.TimeOut = 0;
	} 	
    if(TimerErrFlow.TimeOut)
    {
		Me.Error.Fan_Fault = 1;
		TimerErrFlow.Value = 0;
		TimerErrFlow.TimeOut = 0;        
	}
    
    if(iDigiIn1 && Me.Error.Fan_Fault == 1)
        Me.Error.Fan_Fault = 0;
    
}
#endif

#if(SplitMode==K_I_Am_On_Off_Fan)
void OnOffFanControl(void)
{
    static int BloccoPumpLoop = 0;
    static int OldPumpLoop = 0;

    Me.Pump_Work = iDigitFan;    

        
    if((BloccoPumpLoop==0 && Me.Error.Flow==1) || Touch[0].FunctionMode==0)
        oDigitFan = 0;
    else if(BloccoPumpLoop==0 && Touch[0].FunctionMode!=0)
        oDigitFan = 1;

    if(oDigitFan!=OldPumpLoop)
    {
        OldPumpLoop = oDigitFan;
        BloccoPumpLoop = 1;
    }
    if(BloccoPumpLoop>0)
    {
        BloccoPumpLoop++;
        if(BloccoPumpLoop>60)//Tempo di ciclo (500ms) x2 = 30 sec.
            BloccoPumpLoop = 0;
    } 
#if(K_AlarmEnable==1)
    OnOffTachoControl();
#else
    Me.Error.Fan_Fault = 0;
#endif
}

void OnOffTachoControl(void)
{
	TimerErrFlow.Enable = !iDigiIn1 && iDigitFan;

    if(!TimerErrFlow.Enable)
	{	
		TimerErrFlow.Value = 0;
		TimerErrFlow.TimeOut = 0;
	} 	
    if(TimerErrFlow.TimeOut)
    {
		Me.Error.Fan_Fault = 1;
		TimerErrFlow.Value = 0;
		TimerErrFlow.TimeOut = 0;        
	}
    
    if(iDigiIn1 && Me.Error.Fan_Fault == 1)
        Me.Error.Fan_Fault = 0;
    
}
#endif
