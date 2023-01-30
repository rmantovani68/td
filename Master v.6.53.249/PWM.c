//----------------------------------------------------------------------------------
//	Progect name:	Triac.c
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			Emanuele
//	Date:			22/05/2011
//	Description:	Corpo delle funzioni per il modulo di gestione del triac
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//	Include
//----------------------------------------------------------------------------------
#include "HWSelection.h"
#include <p24FJ256GB210.h>

#include <stdio.h>
#include "DefinePeriferiche.h"
#include "PWM.h"
#include "Timer.h"
#include "Core.h"
#include "PID.h"
#include "FWSelection.h"
#include "ProtocolloModBus.h"


extern unsigned ComunicationLost;				// flag di comunicazione persa


//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
// Inizializza i valori delle velocità ventialtore da costanti anziche da EEPROM
void Init_PWM_Value(void)
{
	PWMValue[0] = K_FanSpeedOff;
	PWMValue[1] = K_FanSpeedNight;
	PWMValue[2] = K_FanSpeed1; 
	PWMValue[3] = K_FanSpeed2; 
	PWMValue[4] = K_FanSpeed3; 
	PWMValue[5] = K_FanSpeed4; 
	PWMValue[6] = K_FanSpeed5; 
	PWMValue[7] = K_FanSpeed6; 
	PWMValue[8] = K_FanSpeed7;	
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
    
    TempAmbFan=TempAmbFanSel();
    
    if(Me.Function_Mode == CoreRaffrescamento || Me.Function_Mode == CoreSlaveFreddo)
    {
#if (K_FanBrake_DeltaTempInOut == 1)		
        ErrTemp = TempAmbFan - Me.Temp_AirOut;
#else
		ErrTemp = MaxTemp;		//Tolto freno ventola su aria in uscita
#endif
    }
    else if(Me.Function_Mode == CoreRiscaldamento || Me.Function_Mode == CoreSlaveCaldo)  
    {

#if (K_FanBrake_DeltaTempInOut == 1)	
        ErrTemp = Me.Temp_AirOut - TempAmbFan;
#else
		ErrTemp = MaxTemp;		//Tolto freno ventola su aria in uscita
#endif		
    }
    else if(Me.Function_Mode == CoreVentilazione)
    {
		//ErrTemp = TempAmbFan - Me.Temp_AirOut;
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
    int TempOffs=500;
    int TempAmbFan;
    
    TempAmbFan=TempAmbFanSel();
    
    if(Me.Function_Mode == CoreRaffrescamento || Me.Function_Mode == CoreSlaveFreddo)
    {
#if (K_FanBrake_DeltaTempReqOut == 1)		
        ErrTemp = Me.Temp_AirOut - (Me.Evap_Temp + TempOffs);
#else
		ErrTemp = 0;	//Tolto freno ventola su aria in uscita		
#endif
    }
    else if(Me.Function_Mode == CoreRiscaldamento || Me.Function_Mode == CoreSlaveCaldo)  
    {
#if (K_FanBrake_DeltaTempReqOut == 1)				
        ErrTemp = (Me.Evap_Temp - TempOffs) - Me.Temp_AirOut;
#else
		ErrTemp = 0;	//Tolto freno ventola su aria in uscita		
#endif
		
    }
    else if(Me.Function_Mode == CoreVentilazione)
    {
		//ErrTemp = Me.Temp_AirOut - (Me.Evap_Temp + TempOffs);
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
	if(RefreshVentil.TimeOut)	// controllo se devo aggiornare il ventil
	{	if(Me.Pwm_Req != Me.Pwm_Value)
		{
			if((Me.Pwm_Value < (PWMValue[1]-20)) & (Me.Pwm_Req < Me.Pwm_Value))	// si sta spegnendo
				Me.Pwm_Value = PWMValue[0];
			if((Me.Pwm_Value < (PWMValue[1]-10)) & (Me.Pwm_Req > Me.Pwm_Value))	// si sta accendendo
				Me.Pwm_Value = PWMValue[1]-10;
			if(Me.Pwm_Req > Me.Pwm_Value)
				Incrementa(&(Me.Pwm_Value), 4, PWMValue[8]);
			else
				Decrementa(&(Me.Pwm_Value), 4, PWMValue[0]);
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
    
    TempAmbFan=TempAmbFanSel();
	
	Me.Fan_MaxSpeed = Filter(&FilterFan_MaxSpeed, ((DeltaTempReqOut() + DeltaTempInOut()) / 2), K_Campioni_FilterFanSpeed);
	if(SpeedReq==K_FanSpeedAuto)
	{

		if((Me.Function_Mode == CoreRiscaldamento) | (Me.Function_Mode == CoreSlaveCaldo))	// Se deve riscaldare
			TempErr = (Me.SP - TempAmbFan);
		else if((Me.Function_Mode == CoreRaffrescamento) | (Me.Function_Mode == CoreSlaveFreddo))	// Se deve raffrescare										// Se deve raffrescare
			TempErr = (TempAmbFan - Me.SP);		
		else if(Me.Function_Mode == CoreVentilazione)	// Se deve ventilare										// Se deve raffrescare
			TempErr = 25;	//(TempAmbFan - Me.SP);		


		if(TempErr < K_Lim_Temp_Auto_FanSpeed_Min) TempErr = K_Lim_Temp_Auto_FanSpeed_Min;
		if(TempErr > K_Lim_Temp_Auto_FanSpeed_Max) TempErr = K_Lim_Temp_Auto_FanSpeed_Max;


		DeltaT = (float)K_Lim_Temp_Auto_FanSpeed_Max-(float)K_Lim_Temp_Auto_FanSpeed_Min;		
		DeltaV = (float)K_Lim_VelPwm_Auto_FanSpeed_Max - (float)K_Lim_VelPwm_Auto_FanSpeed_Min;
		KDiv = DeltaT / DeltaV;			


		RetVal = (int)(((float)(TempErr-K_Lim_Temp_Auto_FanSpeed_Min) / KDiv) + (float)K_Lim_VelPwm_Auto_FanSpeed_Min);	
	}
	else
	{
		RetVal = SpeedReq;
	}
	
	if(RetVal < K_FanSpeedNight)		// Se velocità ventil calcolata è inferiore a Night .... limito
		RetVal = K_FanSpeedNight;
	
	if(RetVal>Me.Fan_MaxSpeed)			// Se velocità ventil calcolata è superiore a massimo consentito da calcoli precedenti... limito
		RetVal=Me.Fan_MaxSpeed;
	
	
	
	return RetVal;
}

// NEW sostituisce -> AggiornaVentil(void)
void UpdateFanSpeed(void)
{
	static int SpeedBoost = 0;	
    static int LastVelPWM = 0;
    static int LastOn = 0;
	
	Me.Error.Battery_Defrost = 0;
	if(Me.Error.BatteriaGhiacciata)
	{	Me.Pwm_Req= PWMValue[4];
		Me.Error.Battery_Defrost = 1;
	}
	else if((Me.Temp_Big < SogliVent2_BatteriaGhiacciata) & (Me.Function_Mode == CoreOff))
	{	Me.Pwm_Req= PWMValue[3];
		Me.Error.Battery_Defrost = 1;
	}
	else if((Me.Temp_Big < SogliVent1_BatteriaGhiacciata) & (Me.Function_Mode == CoreOff))
	{	Me.Pwm_Req= PWMValue[2];
		Me.Error.Battery_Defrost = 1;
	}
	else if((Me.Temp_Big < SogliVentNightBatteriaGhiacciata) & (Me.Function_Mode == CoreOff))
	{	Me.Pwm_Req= PWMValue[1];
		Me.Error.Battery_Defrost = 1;
	}
	else if((Me.Function_Mode != 0) & ((EngineBox.Error.CompressorLo & EngineBox.InverterRiscaldamento) | 
		(EngineBox.Error.CondensatoreLo) | (EngineBox.Error.Gas_Recovery)))
		Me.Pwm_Req= PWMValue[2];														// faccio girare a 2 il ventilatore
	else if(EngineBox.Test.FluxReq | EngineBox.Test.EEV_Req | EngineBox.Test.EEV_Go)	// se ho un test
		Me.Pwm_Req= PWMValue[0];														// fermo il ventilatore
	else
	{	
		switch(Me.Function_Mode)
		{	case CoreRaffrescamento:
			case CoreRiscaldamento:
			case CoreVentilazione:
			case CoreSlaveCaldo:
			case CoreSlaveFreddo:			 
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
                    
                    LastVelPWM = Me.Pwm_Req;
				break;

			case CoreOff:					// lo split richiede lo spegnimento
					if(((Touch[0].FunctionMode == CoreRiscaldamento) ||
						(Touch[0].FunctionMode == CoreRaffrescamento) ||
						(Touch[0].FunctionMode == CoreAuto)) && 
						((((EngineBox.CumulativeError & 0x0002)==0) || ((EngineBox.CumulativeError_C2 & 0x0002)==0) && EngineBox.DoubleCompressorOn) ||
						(((EngineBox.CumulativeError & 0x0002)==0) && !EngineBox.DoubleCompressorOn)))		// se comunque è acceso e non ci sono PersErr delle 2 unità
					{	
						
						if(Me.Ventil_Mode > 8)
						{
							Me.Pwm_Req = PWMValue[2];				// speed 1
						}
						else
							Me.Pwm_Req = PWMValue[Me.Ventil_Mode];	// actual speed

	                    LastVelPWM = Me.Pwm_Req;
					}
                    else if(EngineBox.RealFuncMode == EngineBox_GoOff)					
                    {       
                        Me.Pwm_Req = LastVelPWM;
                    }
					/*
					else if(((EngineBox.IdMasterSplit == Me.My_Address)&&!Me.I_Am_SubSlave) || 
							((EngineBox.IdMasterSplit==Me.My_SplitMaster_Address) && Me.I_Am_SubSlave))					
					{
					

						if(EngineBox.RealFuncMode == EngineBox_GoOff)
							Me.Pwm_Req = PWMValue[2];	// speed 1
						else
						{	
							Me.Old_Function_Mode = CoreOff;
							Me.Pwm_Req = PWMValue[0];
						}
					}
					*/
                    else if(Me.Error.Fan_Fault)
                    {
                        Me.Pwm_Req = PWMValue[1];
                    }
					else
					{	
						//Me.Old_Function_Mode = CoreOff;
						Me.Pwm_Req = PWMValue[0];
                        if(Me.Pwm_Value==K_FanSpeedOff)
							LastVelPWM = K_FanSpeedOff;
					}
				break;
	
			default:
				Me.Pwm_Req = PWMValue[0];
				break;
		}
	}
	
	if(EngineBox.RealFuncMode == EngineBox_CriticalFault && (Touch[0].FunctionMode!=CoreVentilazione))		
		Me.Pwm_Req = PWMValue[0];
    
    //DEBUG    
    //if (!iDigiIn2)
    //    Me.Pwm_Req = 255;
    //DEBUG
	
    VariazionePWM();	
	
}

/*
void AggiornaVentil(void)
{	
	Me.Error.Battery_Defrost = 0;
	if(Me.Error.BatteriaGhiacciata)
	{	Me.Pwm_Req= PWMValue[4];
		Me.Error.Battery_Defrost = 1;
	}
	else if((Me.Temp_Big < SogliVent2_BatteriaGhiacciata) & (Me.Function_Mode == CoreOff))
	{	Me.Pwm_Req= PWMValue[3];
		Me.Error.Battery_Defrost = 1;
	}
	else if((Me.Temp_Big < SogliVent1_BatteriaGhiacciata) & (Me.Function_Mode == CoreOff))
	{	Me.Pwm_Req= PWMValue[2];
		Me.Error.Battery_Defrost = 1;
	}
	else if((Me.Temp_Big < SogliVentNightBatteriaGhiacciata) & (Me.Function_Mode == CoreOff))
	{	Me.Pwm_Req= PWMValue[1];
		Me.Error.Battery_Defrost = 1;
	}
	else if((Me.Function_Mode != 0) & ((EngineBox.Error.CompressorLo & EngineBox.InverterRiscaldamento) | 
		(EngineBox.Error.CondensatoreLo) | (EngineBox.Error.Gas_Recovery)))
		Me.Pwm_Req= PWMValue[2];														// faccio girare a 2 il ventilatore
	else if(EngineBox.Test.FluxReq | EngineBox.Test.EEV_Req | EngineBox.Test.EEV_Go)	// se ho un test
		Me.Pwm_Req= PWMValue[0];														// fermo il ventilatore
	else
	{	switch(Me.Function_Mode)
		{	case CoreRaffrescamento:
			case CoreRiscaldamento:
			case CoreVentilazione:
			case CoreSlaveCaldo:
			case CoreSlaveFreddo:
				switch(Me.Ventil_Mode)	
				{
					case 0:		// Off
						Me.Pwm_Req= PWMValue[0];
						break;
					case 1:		// Night
						FineRegVentil(PWMValue[1]);
						break;
					case 2:		// Speed 1
						FineRegVentil(PWMValue[2]);
						break;
					case 3:		// Speed 2
						FineRegVentil(PWMValue[3]);
						break;
					case 4:		// Speed 3
						FineRegVentil(PWMValue[4]);
						break;
					case 5:		// Speed 4
						FineRegVentil(PWMValue[5]);
						break;
					case 6:		// Speed 5
						FineRegVentil(PWMValue[6]);
						break;
					case 7:		// Speed 6
						FineRegVentil(PWMValue[7]);
						break;
					case 8:		// Speed 7
						FineRegVentil(PWMValue[8]);
						break;
					case 9:		// Auto
						//Me.Pwm_Req = AutoSpeed();
						FineRegVentil(AutoSpeed());
						break;
				}
				break;

			case CoreOff:					// lo split richiede lo spegnimento
					if((Touch[0].FunctionMode == CoreRiscaldamento) |
						(Touch[0].FunctionMode == CoreRaffrescamento) |
						(Touch[0].FunctionMode == CoreAuto))		// se comunque è acceso
					{	if(Me.Ventil_Mode > 8)
							Me.Pwm_Req = PWMValue[2];				// speed 1
						else
							Me.Pwm_Req = PWMValue[Me.Ventil_Mode];	// actual speed
					}
					else if(EngineBox.IdMasterSplit == Me.My_Address)
					{	if(EngineBox.FunctionMode == EngineBox_GoOff)
							Me.Pwm_Req = PWMValue[2];	// speed 1
						else
						{	Me.Old_Function_Mode = CoreOff;
							Me.Pwm_Req = PWMValue[0];
						}
					}
					else
					{	Me.Old_Function_Mode = CoreOff;
						Me.Pwm_Req = PWMValue[0];
					}
				break;
	
			default:
				Me.Pwm_Req = PWMValue[0];
				break;
		}
	}
	if(EngineBox.FunctionMode == EngineBox_CriticalFault)
		Me.Pwm_Req = PWMValue[0];
	VariazionePWM();
}
*/
  
int	 AutoSpeed(void)
{	int TempErr = 0;
	char i = 0;
    int TempAmbFan;
    
#if(K_Mid_SlaveSec_Temp_Amb_Fan==1)
    if(!Me.I_Am_SubSlave)
    {
        if(EnableMySecBus)
            TempAmbFan=Me.TempAmbFan;
        else
            TempAmbFan=Me.Temp_Amb;   
    }
    else
    {
        if(!ComunicationLost)
            TempAmbFan=Me.TempAmbFan;
        else
            TempAmbFan=Me.Temp_Amb;      
    }
#else
        TempAmbFan=Me.Temp_Amb;
#endif

	if((Me.Function_Mode == CoreRiscaldamento) | (Me.Function_Mode == CoreSlaveCaldo))	// Se deve riscaldare
		TempErr = (Me.SP - TempAmbFan);
	else										// Se deve raffrescare
		TempErr = (TempAmbFan - Me.SP);

	if(TempErr < (TempLimitSpeed[0]-100))
		return PWMValue[0];						// Spento

	for(i=0; i<8; i++)
	{	if(TempErr < TempLimitSpeed[i])
			return PWMValue[i+1];
	}
	return PWMValue[8];
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
			EvapErr = (Me.Evap_Temp - Me.Temp_AirOut);
			AbilCambioVentola = 1;
			}
		else if((Me.Function_Mode == CoreRaffrescamento) | (Me.Function_Mode == CoreSlaveFreddo))	// Se deve raffrescare
			{
			EvapErr = (Me.Temp_AirOut - Me.Evap_Temp);//(Me.Evap_Temp + 500));
            /*
			if(Me.Temp_Big > (Me.Temp_Small+1000))
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
    
#if (K_AbilFanInverter==1)	
    Me.Error.Fan_Fault = Inverter.FanInverterFault; // Inserire gestione allarme FAN speed
		if(Me.Error.Fan_Fault)
			Me.Pwm_Value = 0;
#else    
	if(Me.Pwm_Value >= PWMValue[1])
	{
		Me.Error.Fan_Fault = (Me.Fan_Tacho == 0);
		if(Me.Error.Fan_Fault)
			Me.Pwm_Value = 0;
	}
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
	TimerTacho +=1;	// incremento il contatore
	if(TimerTacho > 999)
	{
		TachoTMR = TMR3;
		TMR3 = 0;
		TimerTacho = 0;
		FlagTachoUpdateRequest = 1;
	}
}

void RefreshTacho(void)
{
#if (K_AbilFanInverter==1)
    Me.Fan_Tacho = (unsigned int)( ((float)Me.Pwm_Value / (float)255) * 100.0); //7200; // Da inserire formula x ricavare velocità RPM da freq. Inverter!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 	
#else    
	if(FlagTachoUpdateRequest)
        //Me.Fan_Tacho = TachoTMR *30;
		Me.Fan_Tacho = TachoTMR/3 * 60;			// TachoTMR = 3 impulsi x giro contati in 1000mS / 3 * 60 sec. -> giri al min.
#endif    
}

/*  *****************************************************************************************************************************
	Gestico l'incremento di una variabile
	***************************************************************************************************************************** */
unsigned char Incrementa(int * Variabile, int Value, int Max)
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
			return 1;
		}
		else
			return 0;					// altrimenti sono esattamente a minimo e non modifico nulla
	}
}

/*  *****************************************************************************************************************************
	Gestico il decremento di una variabile
	***************************************************************************************************************************** */
unsigned char Decrementa(int * Variabile, int Value, int Min)
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
            TempAmbFan=Me.Temp_Amb;
    }
    else
    {
        if(!ComunicationLost)
            TempAmbFan=Me.TempAmbFan;
        else
            TempAmbFan=Me.Temp_Amb;
    }
#else
        TempAmbFan=Me.Temp_Amb;
#endif
        return TempAmbFan;
}