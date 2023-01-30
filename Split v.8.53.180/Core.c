//----------------------------------------------------------------------------------
//	Progect name:	Core.c
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			22/05/2011
//	Description:	Corpo delle funzioni della logica di funzionamento
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
#include "Driver_Comunicazione.h"
#include "Driver_ComunicazioneSec.h"
#include "Driver_ModBusSec.h"
#include "ProtocolloComunicazione.h"
#include "ProtocolloComunicazioneSec.h"
#include "ProtocolloModBusSec.h"
#include "Core.h"
#include "PID.h"
#include "delay.h"
#include "Valvola_PassoPasso.h"
#include "FWSelection.h"

extern unsigned ComunicationLost;				// flag di comunicazione persa
extern volatile TypTimer TimerCheckProbe;

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
/*  *****************************************************************************************************************************
	"Copia i dati dalla struct NetB[] a quella Touch[] per mantenere inalterata le funzioni in Core.C che usano Touch[]...
// Valori possibili provenienti dal Touch che finiscono in "Touch[0].FunctionMode":
//	#define	CoreOff						0		// Spento
//	#define CoreAuto					0x0002	// Automatico da touch
//	#define	CoreRiscaldamento			0x0004	// Riscaldmaneto automatico, si calcola tutto e aziona in mase allo stato del box motore
//	#define	CoreRaffrescamento			0x0008	// Raffreddamento automatico, si calcola tutto e aziona in mase allo stato del box motore
//	#define CoreVentilazione			0x0010	// ventilazione automatica, gestisce autonomamente ventilatore e valvola
//	#define CoreSlaveCaldo				0x0011	// segue la temperatura di evaporazione che gli viene impostata
//	#define CoreSlaveFreddo				0x0012	// segue la temp di evporazione che gli viene impostat
//	#define CoreManuale					0x0013	// viene gestito tutto in manuale
// Valori possibili provenienti dal Touch che finiscono in "Touch[0].FanMode":
//	case 0:		// Off
//	case 1:		// Night
//	case 2:		// Speed 1
//	case 3:		// Speed 2
//	case 4:		// Speed 3
//	case 5:		// Speed 4
//	case 6:		// Speed 5
//	case 7:		// Speed 6
//	case 8:		// Speed 7o
//	case 9:		// Auto
//
// Valori possibili provenienti dal NetB che finiscono in "NetB[0].FunctionMode":
// 0..1 (0=Raffrescamento, 1=Riscaldamento)
// Valori possibili provenienti dal NetB che finiscono in "NetB[0].FanMode":
// 0..4 (0=OFF, 1,2,3=VEL 4=Auto)
	***************************************************************************************************************************** */
void NetB2Touch(void)  	// <-- smistare dati Modalità/FanMode da NetB in registri FunctionMode e FanMode (vedi CoreOff,CoreVentilazione ...... 
{
    if(NetB[0].FanMode!=0)
        Touch[0].FanMode = NetB[0].FanMode;
            

    if(NetB[0].OnLine)
    {
        Me.PanelTempAmb = NetB[0].Temperature;
        Touch[0].SetPoint = NetB[0].SetPoint;
        if(Me.Error.Flood_Alarm)
            Touch[0].FunctionMode = CoreVentilazione * (NetB[0].FanMode!=0) * (EngineBox.SystemDisable==0);
        else
            Touch[0].FunctionMode = Me.DefMode * (NetB[0].FanMode!=0) * (EngineBox.SystemDisable==0);
    }

}



/*  *****************************************************************************************************************************
	Gestisco il compressore, start/stop e cambio di stagione
	***************************************************************************************************************************** */
void FunctionMode(void)
{
    static int DefMode = 0;
    int RealFanMode = 0;

    
#if(SplitMode!=K_I_Am_OnlyTouchComm) 
#if(K_UTAEnable==0)
#if(SplitMode==K_I_Am_InverterFAN || SplitMode==K_I_Am_StandardFAN || SplitMode==K_I_Am_Atex_Fan || SplitMode==K_I_Am_On_Off_Fan)  
if(Me.I_Am_SubSlave==0)    
    Me.DefMode = CoreVentilazione;
    
    DefMode = Me.DefMode;
#elif(SplitMode==K_I_Am_Frigo || SplitMode==K_I_Am_Freezer || SplitMode==K_I_Am_LoopWater)    
    Me.DefMode = CoreRaffrescamento;
    DefMode = Me.DefMode;
#else
    DefMode = Me.DefMode; 
#endif
#else
    if(Touch[0].Uta_Mode==CoreUtaAuto && Me.DefMode==CoreRiscaldamento)
        DefMode = CoreRiscaldamento;
    else if(Touch[0].Uta_Mode==CoreUtaAuto && Me.DefMode==CoreRaffrescamento)
        DefMode = CoreRaffrescamento;
    else
        DefMode = CoreVentilazione;
    
    if(Me.I_Am_SubSlave && Me.DefMode==CoreOff)
        DefMode = CoreOff;
#endif

    if(Me.Error.Flood_Alarm)
    {
        if(Me.DefMode!=0)
            DefMode = CoreVentilazione;
    }
    
    switch(Me.MaxFanVentil_Mode)
    {
        case 3:
            RealFanMode = 1 * (Touch[0].FanMode==1) + 8 * (Touch[0].FanMode==2) + 9 * (Touch[0].FanMode>=3);
            break;
        case 4:
            RealFanMode = 1 * (Touch[0].FanMode==1) + 4 * (Touch[0].FanMode==2) + 8 * (Touch[0].FanMode==3) + 9 * (Touch[0].FanMode>=4);
            break;
        case 5:
            RealFanMode = 1 * (Touch[0].FanMode==1) + 3 * (Touch[0].FanMode==2) + 6 * (Touch[0].FanMode==3) + 8 * (Touch[0].FanMode==4) + 9 * (Touch[0].FanMode>=5);
            break;
        case 6:
            RealFanMode = 1 * (Touch[0].FanMode==1) + 2 * (Touch[0].FanMode==2) + 4 * (Touch[0].FanMode==3) + 6 * (Touch[0].FanMode==4) + 8 * (Touch[0].FanMode==5) + 9 * (Touch[0].FanMode>=6);
            break;
        case 7:
            RealFanMode = 1 * (Touch[0].FanMode==1) + 2 * (Touch[0].FanMode==2) + 4 * (Touch[0].FanMode==3) + 5 * (Touch[0].FanMode==4) + 7 * (Touch[0].FanMode==5) + 8 * (Touch[0].FanMode==6) + 9 * (Touch[0].FanMode>=7);
            break;
        case 8:
            RealFanMode = 1 * (Touch[0].FanMode==1) + 2 * (Touch[0].FanMode==2) + 3 * (Touch[0].FanMode==3) + 4 * (Touch[0].FanMode==4) + 5 * (Touch[0].FanMode==5) + 7 * (Touch[0].FanMode==6) + 8 * (Touch[0].FanMode==7) + 9 * (Touch[0].FanMode>=8);
            break;
        case 9:
        default:
            RealFanMode = Touch[0].FanMode;
            break;
            
        
    }
    
    
#if(K_ManualSpeedEnable==0)	 
    
    if(Me.I_Am_SubSlave)
         Touch[0].FunctionMode = DefMode;
    else
    {
#if(K_Channel_Gas_Regulation==1)
        Touch[0].Script_Split_Pwr = Me.Channel_Gas_Regulation>0;

        Syncronize.Script_Split_Pwr = Touch[0].Script_Split_Pwr;
        Syncronize.Engine.Script_Split_Pwr = Touch[0].Script_Split_Pwr; 

        if (Me.Channel_Gas_Regulation<=PWMValue[2])
            RealFanMode = 1;
        else if (Me.Channel_Gas_Regulation<=PWMValue[3])
            RealFanMode = 2;
        else if (Me.Channel_Gas_Regulation<=PWMValue[4])
            RealFanMode = 3;
        else if (Me.Channel_Gas_Regulation<=PWMValue[5])
            RealFanMode = 4;
        else if (Me.Channel_Gas_Regulation<=PWMValue[6])
            RealFanMode = 5;
        else if (Me.Channel_Gas_Regulation<=PWMValue[7])
            RealFanMode = 6;
        else if (Me.Channel_Gas_Regulation<=PWMValue[8]-5)
            RealFanMode = 7;
        else
            RealFanMode = 8;

        Syncronize.FanMode = RealFanMode;
        Syncronize.Engine.FanMode = RealFanMode;
#elif(K_DiffPress_Auto_Fan_Control==1)
        int temp = ValueIn2ValueOut(Me.Temperature.Ambient, K_DiffPress_Auto_Min, K_DiffPress_Auto_Max, PWMValue[0], PWMValue[8]);                    

        Touch[0].Script_Split_Pwr = temp>0;

        Syncronize.Script_Split_Pwr = Touch[0].Script_Split_Pwr;
        Syncronize.Engine.Script_Split_Pwr = Touch[0].Script_Split_Pwr; 

        
        if (temp<=PWMValue[2])
            RealFanMode = 1;
        else if (temp<=PWMValue[3])
            RealFanMode = 2;
        else if (temp<=PWMValue[4])
            RealFanMode = 3;
        else if (temp<=PWMValue[5])
            RealFanMode = 4;
        else if (temp<=PWMValue[6])
            RealFanMode = 5;
        else if (temp<=PWMValue[7])
            RealFanMode = 6;
        else if (temp<=PWMValue[8]-5)
            RealFanMode = 7;
        else
            RealFanMode = 8;

        Syncronize.FanMode = RealFanMode;
        Syncronize.Engine.FanMode = RealFanMode;
#endif        
        
		//Me.Ventil_Mode = Touch[0].FanMode;		// aggiorno lo stato del ventilatore dai touch
        Me.Ventil_Mode = RealFanMode;		// aggiorno lo stato del ventilatore dai touch
		Me.SP = Touch[0].SetPoint;				// aggiorno il SP dai touch
        if(!NetB[0].OnLine)
            Touch[0].FunctionMode = (Touch[0].Script_Split_Pwr*DefMode*(EngineBox.SystemDisable==0));

#if(SplitMode==K_I_Am_InverterFAN || SplitMode==K_I_Am_StandardFAN)          
        if((Touch[0].FunctionMode!=0) && (RealFanMode==9) && (Me.Extractor.Abil==1))
        {
            Me.Ventil_Mode = Me.Extractor.AutoVel;
            if(Me.Extractor.AutoVel==0)
            Touch[0].FunctionMode = 0;
        }   
#endif
        
#if(K_Force_DigiIn1_Ventil==1)
        if(iDigiIn1)
        {
            if((Me.Ventil_Mode<K_Fan_DigiIn1_Vel) || (Touch[0].FunctionMode==CoreOff))
                Me.Ventil_Mode = K_Fan_DigiIn1_Vel;
            Touch[0].FunctionMode = (iDigiIn1*DefMode*(EngineBox.SystemDisable==0));
        }
#endif
        
#if(K_Force_Off_No_Proprity_Zone_Eco_Mode==1)
        if(EngineBox.Eco_Mode_Status==1)
            Touch[0].FunctionMode = 0;
#endif
   
#if(K_LoopPumpEnable==1)
        PumpLoopControl();
        if(Me.Error.Flow==1 && Me.Error.Flow2==1)
            Touch[0].FunctionMode = 0;
#endif        
#if(SplitMode==K_I_Am_Atex_Fan)
        AtexFanControl();
#endif    
#if(SplitMode==K_I_Am_On_Off_Fan)
        OnOffFanControl();
#endif    
        
    }
#if(K_Dampers_Selection>0)
        oDigitDampers1 = Touch[0].FunctionMode!=0;
#if(K_Dampers_Selection>1)
        oDigitDampers2 = Touch[0].FunctionMode!=0;
#endif        
        if(iDigiIn1==0)
            Touch[0].FunctionMode = 0;
#if(K_Dampers_Selection>1)
        if(iDigiIn2==0)
            Touch[0].FunctionMode = 0;
#endif
#endif     
    //if(Me.Function_Mode < CoreSlaveCaldo)		// se sono in modalità autonoma         
    {	
        //Me.Ventil_Mode = Touch[0].FanMode;		// aggiorno lo stato del ventilatore dai touch
        //Me.SP = Touch[0].SetPoint;				// aggiorno il SP dai touch
        switch(Touch[0].FunctionMode)			// aggiorno lo stato di funzionamento dai touch
        {
            case CoreRiscaldamento:
#if((SplitMode == K_I_Am_UTA_and_InvFAN) || (SplitMode == K_I_Am_StandardUTA))             
                if((Me.TempAmbRealSelection > (Me.SP + 200)))
#else
                if((Me.TempAmbRealSelection > (Me.SP + Caldo_Ist_SP)))
#endif                  
                    Me.Function_Mode = CoreOff;
                else if(Me.TempAmbRealSelection < (Me.SP - Caldo_Ist_SP))
                    Me.Function_Mode = CoreRiscaldamento;
                //Me.Old_Function_Mode = CoreRiscaldamento;
                break;

            case CoreRaffrescamento:
#if((SplitMode == K_I_Am_UTA_and_InvFAN) || (SplitMode == K_I_Am_StandardUTA))             
                if((Me.TempAmbRealSelection < (Me.SP - 200)))
#elif(SplitMode==K_I_Am_LoopWater)
                if(Me.TempAmbRealSelection < (Me.SP - 100))
#elif(K_Humidity_Correct==1)
                if(((Me.TempAmbRealSelection < (Me.SP - Freddo_Ist_SP_Off))) && (Me.Humidity_Correct==0)) //50)))

#else
                if((Me.TempAmbRealSelection < (Me.SP - Freddo_Ist_SP_Off))) //50)))
#endif                  
                    Me.Function_Mode = CoreOff;
#if(SplitMode==K_I_Am_LoopWater)
                else if((Me.TempAmbRealSelection > (Me.SP + 50)) && (Me.Temperature.AirOut>(K_Low_Temp_WaterOut+100)) && iDigiIn1)
#elif(K_Humidity_Correct==1)
                else if((Me.TempAmbRealSelection > (Me.SP - Freddo_Ist_SP_On)) || (Me.Humidity_Correct==1))//20))	//Riaccendo 2 decimi di grado prima di SP
#else
                else if(Me.TempAmbRealSelection > (Me.SP - Freddo_Ist_SP_On))//20))	//Riaccendo 2 decimi di grado prima di SP
#endif
                    Me.Function_Mode = CoreRaffrescamento;
                //Me.Old_Function_Mode = CoreRaffrescamento;
#if(SplitMode==K_I_Am_Freezer)
                if(Me.Error.Battery_Defrost || Me.Error.Battery_Drip)
                    Me.Function_Mode = CoreOff;
#endif
#if(SplitMode == K_I_Am_LoopWater)              
                TimerOutWaterStop.Enable = Me.Temperature.AirOut<K_Low_Temp_WaterOut;
                
                if(!TimerOutWaterStop.Enable)
                {
                    TimerOutWaterStop.Value = 0;
                    TimerOutWaterStop.TimeOut = 0;
                }
                    
                if(TimerOutWaterStop.TimeOut)
                {
                    Me.Function_Mode = CoreOff; //Non azzero timeout perchè deve essere azzerato quando la temperatura torna sopra i 5C.
                    TimerOutWaterStop.Value = 0;
                }
#endif                
                break;

            default:
                Me.Function_Mode = Touch[0].FunctionMode;
                break; 
        }
	}		
#else
    {
		Me.Function_Mode = Me.DefMode * (iDigiIn1 || iDigiIn2) * (EngineBox.SystemDisable==0);		// Se non ho touch collegato funziono con modalita' comunicata da Master solo se DigiIn1 o 2 sono attivi (Selettore 3 posizioni) 
	
		switch(Me.Function_Mode)			// aggiorno lo stato di funzionamento dai touch
		{
			case CoreRiscaldamento:
				if(iDigiIn1)
					Me.Ventil_Mode = Ventil_5;
				else if(iDigiIn2)
					Me.Ventil_Mode = Ventil_7;
				break;

			case CoreRaffrescamento:
				if(iDigiIn1)
					Me.Ventil_Mode = Ventil_4;
				else if(iDigiIn2)
					Me.Ventil_Mode = Ventil_7;			
				break;
				
			case CoreVentilazione:
				if(iDigiIn1)
					Me.Ventil_Mode = Ventil_4;
				else if(iDigiIn2)
					Me.Ventil_Mode = Ventil_7;			
				break;

			default:
				if(iDigiIn1)
					Me.Ventil_Mode = Ventil_5;
				else if(iDigiIn2)
					Me.Ventil_Mode = Ventil_7;
				break;
		}
		//Touch[0].FanMode = Me.Ventil_Mode;
        Syncronize.FanMode = Me.Ventil_Mode;
        //Syncronize.Old.FanMode = Me.Ventil_Mode;
        Syncronize.Engine.FanMode = Me.Ventil_Mode;
        
		//Touch[0].SetPoint = Me.SP;
        Syncronize.SetPoint = Me.SP;
        //Syncronize.Old.SetPoint = Me.SP;
        Syncronize.Engine.SetPoint = Me.SP;
        
        //Touch[0].Script_Split_Pwr = 1*(iDigiIn1 || iDigiIn2);      
        Syncronize.Script_Split_Pwr = 1*(iDigiIn1 || iDigiIn2);
        //Syncronize.Old.Script_Split_Pwr = 1*(iDigiIn1 || iDigiIn2); 
        Syncronize.Engine.Script_Split_Pwr = 1*(iDigiIn1 || iDigiIn2); 
    }
#endif 
    
#endif	
		
	// estraggo gli errori del box motor
    EngineBox.Error.CompressorHi =			((EngineBox.Errori1 & 0x0001) != 0);
	EngineBox.Error.CompressorLo =			((EngineBox.Errori1 & 0x0002) != 0);
	EngineBox.Error.CondensatoreHi = 		((EngineBox.Errori1 & 0x0004) != 0);
	EngineBox.Error.CondensatoreLo = 		((EngineBox.Errori1 & 0x0008) != 0);
    EngineBox.Error.Condenser_Fouled =		((EngineBox.Errori1 & 0x0010) != 0);    //EngineBox.Error.Gas_Recovery =			((EngineBox.Errori1 & 0x0010) != 0);
	EngineBox.Test.FluxReq = 				((EngineBox.Errori1 & 0x0020) != 0);
	EngineBox.Test.EEV_Req = 				((EngineBox.Errori1 & 0x0040) != 0);
	EngineBox.Error.SuperHeat =     		((EngineBox.Errori1 & 0x0080) != 0);    //EngineBox.Test.EEV_Go = 				((EngineBox.Errori1 & 0x0080) != 0);
	EngineBox.Error.Pressure_Lo = 			((EngineBox.Errori1 & 0x0100) != 0);
	EngineBox.Error.Pressure_Hi = 			((EngineBox.Errori1 & 0x0200) != 0);
	EngineBox.Error.Recharge_Oil =			((EngineBox.Errori1 & 0x0400) != 0);
	EngineBox.Error.CompressorHi_C2 =		((EngineBox.Errori1 & 0x0800) != 0);
    EngineBox.Error.Defrosting =        	((EngineBox.Errori1 & 0x4000) != 0);
    EngineBox.Error.MasterRestart =        	((EngineBox.Errori1 & 0x8000) != 0);
    
    EngineBox.Error.TemperatureProbe =		((EngineBox.Errori2 & 0x0001) != 0);
    

	EngineBox.PersErr.CompressorHi =		((EngineBox.ErroriPers1 & 0x0001) != 0);
	EngineBox.PersErr.CompressorLo =		((EngineBox.ErroriPers1 & 0x0002) != 0);
	EngineBox.PersErr.CondensatoreHi =		((EngineBox.ErroriPers1 & 0x0004) != 0);
	EngineBox.PersErr.CondensatoreLo =		((EngineBox.ErroriPers1 & 0x0008) != 0);
	//EngineBox.PersErr.Gas_Recovery =		((EngineBox.ErroriPers1 & 0x0010) != 0);
	EngineBox.PersErr.Allagamento =			((EngineBox.ErroriPers1 & 0x0020) != 0);
	EngineBox.PersErr.TermicaCompressore =	((EngineBox.ErroriPers1 & 0x0040) != 0);
	EngineBox.PersErr.SeaWaterLo =			((EngineBox.ErroriPers1 & 0x0080) != 0);
	EngineBox.PersErr.SeaWaterHi =			((EngineBox.ErroriPers1 & 0x0100) != 0);
	EngineBox.PersErr.CaricaRefrig =		((EngineBox.ErroriPers1 & 0x0200) != 0);
	EngineBox.PersErr.WaterSeaFlux =		((EngineBox.ErroriPers1 & 0x0400) != 0);
	EngineBox.PersErr.SuperHeat =   		((EngineBox.ErroriPers1 & 0x0800) != 0);    //EngineBox.PersErr.EEV_Open =			((EngineBox.ErroriPers1 & 0x0800) != 0);
	EngineBox.PersErr.FireAlarm =   		((EngineBox.ErroriPers1 & 0x1000) != 0);    //EngineBox.PersErr.EEV_Close =			((EngineBox.ErroriPers1 & 0x1000) != 0);
	//EngineBox.PersErr.BatteriaGhiacciata =	((EngineBox.ErroriPers1 & 0x2000) != 0);
	//EngineBox.PersErr.Fan_Speed =			((EngineBox.ErroriPers1 & 0x4000) != 0);
	//EngineBox.PersErr.Fan_OverLoad =		((EngineBox.ErroriPers1 & 0x8000) != 0);
	
	EngineBox.PersErr.Acceleration_Error =		((EngineBox.ErroriPers2 & 0x0001) != 0);
	EngineBox.PersErr.Acceleration_Error_C2 =	((EngineBox.ErroriPers2 & 0x0002) != 0);
	EngineBox.PersErr.CompressorHi_C2 =			((EngineBox.ErroriPers2 & 0x0004) != 0);
	EngineBox.PersErr.TermicaCompressore_C2 =	((EngineBox.ErroriPers2 & 0x0008) != 0);
	
	
    //Fisso open e close eev a zero perchè disabilitati per ora
    Me.Error.EEV_Close = 0;
    Me.Error.EEV_Open = 0;
    Me.Error.Fan_OverLoad = 0;
            

	// gestione errori
	if(Me.Error.BatteriaGhiacciata)
	{	
        //if(Me.Temperature.Gas > SogliVentNightBatteriaGhiacciata)
        if(Me.Error.Fan_Fault == 0) //Se ritorna a girare la ventola tolgo errore batteria ghiacciata
			Me.Error.BatteriaGhiacciata = 0;
	}
	else
	{	
        if((Me.Temperature.Gas < SogliTempBatteriaGhiacciata)/* && (Me.Error.Battery_Defrost == 1)*/ && (Me.Error.Fan_Fault == 1))
		{	
            Me.Error.BatteriaGhiacciata = 1;
			//Me.Error.EEV_Open = 1;
		}
	}
    
    //Allarme fuoco (se modalità UTA)
#if(K_Abil_Fire_Alarm==1)
    Me.Error.FireAlarm = !iDigiIn3; //In 3 fa allarme fuoco se aperto.
#endif
    
#if(K_Gas_Leaks==1)
    GasLeaksAlarmCycle();
#endif
    
#if(K_Abil_Flood_Alarm==1)
    FloodFanCoilAlarmCycle();
#endif
	
#if(K_Dampers_Selection>0)
    DampersAlarm();
#else
    Me.Error.Dampers = 0;
#endif
    
    //Ciclo Defrosting
#if (K_AbilSplitDefrostingCycle==1)					
        DefrostingCycle();        
#endif 
        
#if(K_Heater_Abil==1)
	if(/*Me.Error.EEV_Close || Me.Error.Fan_OverLoad || */Me.Error.Fan_Fault || Me.Error.FireAlarm || Me.Error.Battery_Defrost || 
      (EngineBox.RealFuncMode == EngineBox_CriticalFault && (Touch[0].FunctionMode != CoreVentilazione) && (Touch[0].HeaterPwr==0)) || EngineBox.Error.MasterRestart ||
      ((Me.Temperature.Error.Bit.Gas || Me.Temperature.Error.Bit.Liquid) && EngineBox.TestAllAbil.Split_Probe_Error && (Touch[0].FunctionMode != CoreVentilazione))/* || Me.Temperature.Error.Bit.Liquid*/ || (EngineBox.PersErr.FireAlarm && K_Abil_Off_FireAlarm==1) ||
#if(K_LoopDoublePumpEnable==1) 
       (Me.Error.Flow && Me.Error.Flow2))
#else
       Me.Error.Flow)
#endif        
		Me.Function_Mode = CoreOff;
#else
	if(/*Me.Error.EEV_Close || Me.Error.Fan_OverLoad || */Me.Error.Fan_Fault || Me.Error.FireAlarm || Me.Error.Battery_Defrost || 
      (EngineBox.RealFuncMode == EngineBox_CriticalFault && (Touch[0].FunctionMode != CoreVentilazione)) || EngineBox.Error.MasterRestart ||
      ((Me.Temperature.Error.Bit.Gas || Me.Temperature.Error.Bit.Liquid) && EngineBox.TestAllAbil.Split_Probe_Error && (Touch[0].FunctionMode != CoreVentilazione))/* || Me.Temperature.Error.Bit.Liquid*/ || (EngineBox.PersErr.FireAlarm && K_Abil_Off_FireAlarm==1) ||
#if(K_LoopDoublePumpEnable==1) 
       (Me.Error.Flow && Me.Error.Flow2))
#else
       Me.Error.Flow)
#endif        
		Me.Function_Mode = CoreOff;
#endif

#if(SplitMode==K_I_Am_UTA_and_InvFAN || SplitMode==K_I_Am_StandardUTA || SplitMode==K_I_Am_InverterFAN || SplitMode==K_I_Am_StandardFAN || SplitMode==K_I_Am_Atex_Fan || SplitMode==K_I_Am_On_Off_Fan)
	if((EngineBox.RealFuncMode == EngineBox_CriticalFault) && (Touch[0].FunctionMode != CoreOff) && !Me.Error.FireAlarm && (K_Abil_Off_FireAlarm==0 || (!EngineBox.PersErr.FireAlarm && K_Abil_Off_FireAlarm==1)))
		Me.Function_Mode = CoreVentilazione;
#endif
        
	// compongo lo stato dello split
	Me.SplitStatus = 0;    
	Me.SplitStatus =	(Me.Error.ClearErrReq			<< 0) | 
						(Me.Error.EEV_Open				<< 1) | 
						(Me.Error.EEV_Close				<< 2) | 
						(Me.Error.BatteriaGhiacciata	<< 3) | 
						((Me.Error.Fan_Fault || Inverter.FanInverterFault)	<< 4) | 
						(Me.Error.FireAlarm 			<< 5) |
						(Me.Error.GasLeaks  			<< 6) | //(Me.Error.EEV_Test_End			<< 6) |
						(Me.Error.Battery_Defrost		<< 7) | 
						(Me.Error.ClearDiagnosticSplitErrReq		<< 8) | 
                        (((Me.Room.Split_Flags_Alarm_1_16>0) || (Me.Room.Split_Flags_EnableAndOffline_1_16>0)) <<9) |
                        (Me.Error.Dampers               << 10) |            //(Me.Error.Fan_OverLoad			<< 10) |
                        (((Me.ModDeviceEnableAndOffLine>0) || (Me.ModDeviceAlarm>0)) <<11) |
                        (Me.Error.SPTemperatureFault	<< 12) |
                        ((Me.Error.Flood_Alarm  || Me.Error.Flow || Me.Error.Flow2)  		<< 13) | 
                        (Me.Error.TemperatureProbe 		<< 14) | 
                        (Me.Error.ClearComErr      		<< 15);
    
    Me.CumulativeAlarm = Me.Error.EEV_Open || Me.Error.EEV_Close || Me.Error.BatteriaGhiacciata || Me.Error.Fan_Fault || Inverter.FanInverterFault || Me.Error.Fan_OverLoad || Me.Error.FireAlarm || 
                        (Me.Room.Split_Flags_Alarm_1_16>0) || (Me.Room.Split_Flags_EnableAndOffline_1_16>0) || (Me.ModDeviceEnableAndOffLine>0) || (Me.ModDeviceAlarm>0) ||
                         Me.Error.SPTemperatureFault || Me.Error.Flood_Alarm || (Me.Error.TemperatureProbe && EngineBox.TestAllAbil.Split_Probe_Error) || Me.Error.Dampers || Me.Error.Flow || Me.Error.Flow2 || Me.Error.GasLeaks;

	if(((Me.RoomMaster_Status & 0x0100) && (DiagnosticSplit.Reset_Req_Address == Me.My_Address) && (DiagnosticSplit.SubAddress == 0) && (DiagnosticSplit.Touch_Page!=62)) || 
	((Me.RoomMaster_Status & 0x0100) && (DiagnosticSplit.SubAddress == Me.My_Address) && Me.I_Am_SubSlave))
	{
		//Me.Error.Battery_Defrost = 0;
		Me.Error.EEV_Open = 0;
		Me.Error.EEV_Close = 0;
		Me.Error.BatteriaGhiacciata = 0;
		Me.Error.Fan_Fault = 0;
		Me.Error.Fan_OverLoad = 0;
		Me.Error.EEV_Test_End = 0;		
        Me.Error.FireAlarm = 0;
        Me.Error.SPTemperatureFault = 0;
        Me.Error.Flood_Alarm = 0;
        Me.Error.Dampers = 0;
        Me.Error.Flow = 0;
        Me.Error.Flow2 = 0;
   
        if(Me.Temperature.Error.Bit.Gas)
            Me.Temperature.CntError.Gas = 0;
        if(Me.Temperature.Error.Bit.Liquid)
            Me.Temperature.CntError.Liquid = 0;
        if(Me.Temperature.Error.Bit.AirOut)
            Me.Temperature.CntError.AirOut = 0;
        if(Me.Temperature.Error.Bit.Ambient)
            Me.Temperature.CntError.Ambient = 0;        
    }	
    
    if((Me.RoomMaster_Status & 0x0100) && (DiagnosticSplit.Reset_Req_Address == Me.My_Address) && (DiagnosticSplit.Touch_Page==62))
    {
        Me.ComError.CntComErrSec = 0;
        Me.ComError.TimeOutComErrSec = 0;                  
        Me.ComError.TimeOutInterCharComErrSec = 0;
        Me.ComError.CRC_ComErrSec = 0;
        Me.ComError.ModCntComErrSec = 0;
        Me.ComError.ModTimeOutComErrSec = 0;
        Me.ComError.ModTimeOutInterCharComErrSec = 0;
        Me.ComError.ModCRC_ComErrSec = 0;
    }
#if(SplitMode == K_I_Am_Frigo)    
    EngineBox.TestAllAbil.SuperHeat_SP_Frigo =   ((EngineBox.TestAll_Frozen_Abil & 0x0001) != 0);
	EngineBox.TestAllAbil.MaxOpValve_Frigo = 	((EngineBox.TestAll_Frozen_Abil & 0x0004) != 0);
	EngineBox.TestAllAbil.TempBatt_SP_Frigo =	((EngineBox.TestAll_Frozen_Abil & 0x0010) != 0);
    EngineBox.TestAllAbil.MinOpValve_Frigo     = 	((EngineBox.TestAll_Frozen_Abil & 0x0040) != 0);

    if(EngineBox.TestAllAbil.SuperHeat_SP_Frigo)
        Me.SuperHeat_SP = EngineBox.TestAll_SuperHeat_SP_Frigo;
    else
        Me.SuperHeat_SP = Me.SuperHeat_SP_Local;
			
    if(EngineBox.TestAllAbil.TempBatt_SP_Frigo)
        Me.Freddo_TempBatt_SP = EngineBox.TestAll_TempBatt_SP_Frigo;
    else
        Me.Freddo_TempBatt_SP = Me.Freddo_TempBatt_SP_Local;
	
#elif(SplitMode == K_I_Am_Freezer)    
	EngineBox.TestAllAbil.SuperHeat_SP_Freezer =	((EngineBox.TestAll_Frozen_Abil & 0x0002) != 0);
	EngineBox.TestAllAbil.MaxOpValve_Freezer   = 	((EngineBox.TestAll_Frozen_Abil & 0x0008) != 0);
	EngineBox.TestAllAbil.TempBatt_SP_Freezer  = 	((EngineBox.TestAll_Frozen_Abil & 0x0020) != 0);
    EngineBox.TestAllAbil.MinOpValve_Freezer   = 	((EngineBox.TestAll_Frozen_Abil & 0x0080) != 0);


    if(EngineBox.TestAllAbil.SuperHeat_SP_Freezer)
        Me.SuperHeat_SP = EngineBox.TestAll_SuperHeat_SP_Freezer;
    else
        Me.SuperHeat_SP = Me.SuperHeat_SP_Local;
			
    if(EngineBox.TestAllAbil.TempBatt_SP_Freezer)
        Me.Freddo_TempBatt_SP = EngineBox.TestAll_TempBatt_SP_Freezer;
    else
        Me.Freddo_TempBatt_SP = Me.Freddo_TempBatt_SP_Local;
#elif(SplitMode == K_I_Am_LoopWater)    
	EngineBox.TestAllAbil.SuperHeat_SP_Freezer =	((EngineBox.TestAll_Frozen_Abil & 0x0002) != 0);
	EngineBox.TestAllAbil.MaxOpValve_Freezer   = 	((EngineBox.TestAll_Frozen_Abil & 0x0008) != 0);
	EngineBox.TestAllAbil.TempBatt_SP_Freezer  = 	((EngineBox.TestAll_Frozen_Abil & 0x0020) != 0);
    EngineBox.TestAllAbil.MinOpValve_Freezer   = 	((EngineBox.TestAll_Frozen_Abil & 0x0080) != 0);


    if(EngineBox.TestAllAbil.SuperHeat_SP_Freezer)
        Me.SuperHeat_SP = EngineBox.TestAll_SuperHeat_SP_Freezer;
    else
        Me.SuperHeat_SP = Me.SuperHeat_SP_Local;
			
    if(EngineBox.TestAllAbil.TempBatt_SP_Freezer)
        Me.Freddo_TempBatt_SP = EngineBox.TestAll_TempBatt_SP_Freezer;
    else
        Me.Freddo_TempBatt_SP = Me.Freddo_TempBatt_SP_Local;
		
#else
    EngineBox.TestAllAbil.SuperHeat_SP_Cool =   ((EngineBox.TestAll_Abil & 0x0001) != 0);
	//EngineBox.TestAllAbil.SuperHeat_SP_Heat =	((EngineBox.TestAll_Abil & 0x0002) != 0);
	EngineBox.TestAllAbil.MaxOpValve_Cool = 	((EngineBox.TestAll_Abil & 0x0004) != 0);
	//EngineBox.TestAllAbil.MaxOpValve_Heat = 	((EngineBox.TestAll_Abil & 0x0008) != 0);
	EngineBox.TestAllAbil.TempBatt_SP_Cool =	((EngineBox.TestAll_Abil & 0x0010) != 0);
	EngineBox.TestAllAbil.TempBatt_SP_Heat = 	((EngineBox.TestAll_Abil & 0x0020) != 0);
    EngineBox.TestAllAbil.MinOpValve_Cool  = 	((EngineBox.TestAll_Abil & 0x0040) != 0);
    EngineBox.TestAllAbil.MinOpValve_Heat  = 	((EngineBox.TestAll_Abil & 0x0080) != 0);
    //0x0100 nel master per PID caldo
    EngineBox.TestAllAbil.SuperHeat_Custom_Par_Cool_PID  = 	((EngineBox.TestAll_Abil & 0x0200) != 0);
    EngineBox.TestAllAbil.SuperHeat_Cool_Correct = ((EngineBox.TestAll_Abil & 0x0400) != 0);
    EngineBox.TestAllAbil.SuperHeat_Heat_Correct = ((EngineBox.TestAll_Abil & 0x0800) != 0);
    EngineBox.TestAllAbil.Split_Probe_Error = ((EngineBox.TestAll_Abil & 0x1000) != 0);
    EngineBox.TestAllAbil.Master_Probe_Error = ((EngineBox.TestAll_Abil & 0x2000) != 0);
    //0x0400 nel master per errore compressione
    //0x0800 nel master per Programming Mode (Service Mode)
    
#if(K_DisAbil_TestAll_SuperHeat==1)
        Me.SuperHeat_SP = Me.SuperHeat_SP_Local;
#else
    if(EngineBox.TestAllAbil.SuperHeat_SP_Cool)
        Me.SuperHeat_SP = EngineBox.TestAll_SuperHeat_SP_Cool;
    else
        Me.SuperHeat_SP = Me.SuperHeat_SP_Local;
#endif

#if(K_DisAbil_TestAll_TempBatt==1)
        Me.Freddo_TempBatt_SP = Me.Freddo_TempBatt_SP_Local;
        Me.Caldo_TempBatt_SP = Me.Caldo_TempBatt_SP_Local;  
#else        
    if(EngineBox.TestAllAbil.TempBatt_SP_Cool)
        Me.Freddo_TempBatt_SP = EngineBox.TestAll_TempBatt_SP_Cool;
    else
        Me.Freddo_TempBatt_SP = Me.Freddo_TempBatt_SP_Local;
	
    if(EngineBox.TestAllAbil.TempBatt_SP_Heat)
        Me.Caldo_TempBatt_SP = EngineBox.TestAll_TempBatt_SP_Heat;
    else
        Me.Caldo_TempBatt_SP = Me.Caldo_TempBatt_SP_Local;  
#endif
#endif
}

/*  *****************************************************************************************************************************
	Recupero la pressione del gas idonea alla modalità corrente
	***************************************************************************************************************************** */
unsigned int PressureGas(int ReqTemp)
{
	if (ReqTemp < -1500)
		return 4000;			// sotto i -15, setto 4 Bar
	else if (ReqTemp <= -1000)	
		return 4900;			// sotto i -10, setto 6
	else if (ReqTemp <= -500)	
		return 6000;			// sotto i -5, setto 6,5
	else if (ReqTemp <= 0)
		return 7000;			// sotto i 0, setto 7,0
	else if (ReqTemp <= 300)
		return 7500;			// sotto i 3, setto 7,5
	else if (ReqTemp <= 500)
		return 8500;			// sotto i 5, setto 8,5
	else if (ReqTemp <= 750)
		return 9000;			// sotto i 7,5, setto 9
	else if (ReqTemp <= 1000)
		return 10000;			// sotto i 10, setto 11
	else if (ReqTemp <= 1500)
		return 11700;			// sotto i 15, setto 12
	else if (ReqTemp <= 2000)
		return 13600;			// sotto i 20, setto 14
	else if (ReqTemp < 2500)
		return 16500;			// sotto i 25, setto 16,5
	else if (ReqTemp < 3000)
		return 19000;			// sotto i 30, setto 19
	else if (ReqTemp < 3500)
		return 22000;			// sotto i 35, setto 22
	else if (ReqTemp < 4000)
		return 25500;			// sotto i 40, setto 25,5
	else if (ReqTemp < 4500)
		return 29000;			// sotto i 45, setto 29
	else if (ReqTemp < 5000)
		return 33000;			// sotto i 50, setto 33
	else if (ReqTemp < 5500)
		return 36000;			// sotto i 50, setto 36
	else
		return 40000;			// sotto i 50, setto 40
}


/*  *****************************************************************************************************************************
	Ricavo la Temperatura partendo dalla pressione del GAS idonea alla modalità corrente
	***************************************************************************************************************************** */

int PressureGasToTempR410A(unsigned int ReqPress)
{
// Valori ricavati da Tabella Pressione-Temperatura GAS R410A con range -25 +70 step 1°C
// Ingresso: Pressione 
// Uscita: Temperatura 


	// ritorno valore pressione corrispondente come da tabella GAS R410A

    if(ReqPress <= 	1480) return    -3200;
    if(ReqPress <= 	1580) return    -3100;
    if(ReqPress <= 	1690) return    -3000;
    if(ReqPress <= 	1800) return    -2900;
    if(ReqPress <= 	1920) return    -2800;
    if(ReqPress <= 	2040) return	-2700;
    if(ReqPress <= 	2160) return	-2600;
	if(ReqPress <= 	2500) return	-2500; 	
	if(ReqPress <= 	2580) return	-2400; 	
	if(ReqPress <= 	2660) return	-2300; 	
	if(ReqPress <= 	2740) return	-2200; 	
	if(ReqPress <= 	2820) return	-2100; 	
	if(ReqPress <= 	3000) return	-2000; 	
	if(ReqPress <= 	3200) return	-1900; 	
	if(ReqPress <= 	3400) return	-1800; 	
	if(ReqPress <= 	3600) return	-1700; 	
	if(ReqPress <= 	3800) return	-1600; 	
	if(ReqPress <= 	4000) return	-1500; 	
	if(ReqPress <= 	4200) return	-1400; 	
	if(ReqPress <= 	4400) return	-1300; 	
	if(ReqPress <= 	4600) return	-1200; 	
	if(ReqPress <= 	4800) return	-1100; 	
	if(ReqPress <= 	5000) return	-1000; 	
	if(ReqPress <= 	5200) return	-900; 	
	if(ReqPress <= 	5400) return	-800; 	
	if(ReqPress <= 	5600) return	-700; 	
	if(ReqPress <= 	5800) return	-600; 	
	if(ReqPress <= 	6000) return	-500; 	
	if(ReqPress <= 	6200) return	-400; 	
	if(ReqPress <= 	6400) return	-300; 	
	if(ReqPress <= 	6600) return	-200; 	
	if(ReqPress <= 	6800) return	-100; 	
	if(ReqPress <= 	7000) return	 000; 	
	if(ReqPress <= 	7300) return	 100; 	
	if(ReqPress <= 	7600) return	 200; 	
	if(ReqPress <= 	7900) return	 300; 	
	if(ReqPress <= 	8200) return	 400; 	
	if(ReqPress <= 	8500) return	 500; 	
	if(ReqPress <= 	8800) return	 600; 	
	if(ReqPress <=	9100) return	 700; 
	if(ReqPress <=	9400) return	 800; 
	if(ReqPress <=	9700) return	 900; 
	if(ReqPress <=	10000) return	1000; 
	if(ReqPress <=	10350) return	1100; 
	if(ReqPress <=	10700) return	1200; 
	if(ReqPress <=	10950) return	1300; 
	if(ReqPress <=	11300) return	1400; 
	if(ReqPress <=	11650) return	1500; 
	if(ReqPress <=	12000) return	1600; 
	if(ReqPress <=	12350) return	1700; 
	if(ReqPress <=	12700) return	1800; 
	if(ReqPress <=	13050) return	1900; 
	if(ReqPress <=	13500) return	2000; 
	if(ReqPress <=	13950) return	2100; 
	if(ReqPress <=	14400) return	2200; 
	if(ReqPress <=	14850) return	2300; 
	if(ReqPress <=	15300) return	2400; 
	if(ReqPress <=	15750) return	2500; 
	if(ReqPress <=	16200) return	2600; 
	if(ReqPress <=	16650) return	2700; 
	if(ReqPress <=	17100) return 	2800;	
	if(ReqPress <=	17550) return 	2900;	
	if(ReqPress <=	18000) return 	3000;	
	if(ReqPress <=	18500) return 	3100;	
	if(ReqPress <=	19000) return 	3200;	
	if(ReqPress <=	19500) return 	3300;	
	if(ReqPress <=	20000) return 	3400;	
	if(ReqPress <=	20500) return 	3500;	
	if(ReqPress <=	21000) return 	3600;	
	if(ReqPress <=	21500) return 	3700;	
	if(ReqPress <=	22000) return 	3800;	
	if(ReqPress <=	22500) return 	3900;	
	if(ReqPress <=	23000) return 	4000;	
	if(ReqPress <=	23700) return 	4100;	
	if(ReqPress <=	24400) return 	4200;	
	if(ReqPress <=	25100) return 	4300;	
	if(ReqPress <=	25800) return 	4400;	
	if(ReqPress <=	26500) return 	4500;	
	if(ReqPress <=	27200) return 	4600;	
	if(ReqPress <=	27900) return 	4700;	
	if(ReqPress <=	28600) return 	4800;	
	if(ReqPress <=	29300) return 	4900;	
	if(ReqPress <=	30000) return 	5000;	
	if(ReqPress <=	30830) return 	5100;	
	if(ReqPress <=	31660) return 	5200;	
	if(ReqPress <=	32490) return 	5300;	
	if(ReqPress <=	33320) return 	5400;	
	if(ReqPress <=	34150) return 	5500;	
	if(ReqPress <=	34980) return 	5600;	
	if(ReqPress <=	35810) return 	5700;	
	if(ReqPress <=	36640) return 	5800;	
	if(ReqPress <=	37470) return 	5900;	
	if(ReqPress <=	38300) return 	6000;	
	if(ReqPress <=	39130) return 	6100;	
	if(ReqPress <=	40000) return 	6200;	
	if(ReqPress <=	40870) return 	6300;	
	if(ReqPress <=	41740) return 	6400;	
	if(ReqPress <=	42610) return 	6500;	
	if(ReqPress <=	43480) return 	6600;	
	if(ReqPress <=	44350) return 	6700;	
	if(ReqPress <=	45220) return 	6800;	
	if(ReqPress <=	46090) return 	6900;	
	if(ReqPress <=	47000) return 	7000;	

	if(ReqPress >	47000) return 	7000;	

	return 0;
	
}

unsigned int TempToPressureGasR410A(int ReqTemp)
{
// Valori ricavati da Tabella Pressione-Temperatura GAS R407C con range -25 +70 step 1°C
// Ingresso: Temperatura 
// Uscita: Pressione 

	int TempConv;
	unsigned int RetVal=0;
	
	TempConv = ReqTemp/100;				// Converto Temperatura da centesimi in gradi
 

	// ritorno valore pressione corrispondente come da tabella GAS R410A

    if(TempConv <= -32) RetVal= 1480;
    if(TempConv == -31) RetVal= 1580;
    if(TempConv == -30) RetVal= 1690;
    if(TempConv == -29) RetVal= 1800;    
    if(TempConv == -28) RetVal= 1920;
    if(TempConv == -27) RetVal= 2040;
    if(TempConv == -26) RetVal= 2160;
	if(TempConv == -25) RetVal= 2500; 
	if(TempConv == -24) RetVal= 2580; 
	if(TempConv == -23) RetVal= 2660; 
	if(TempConv == -22) RetVal= 2740; 
	if(TempConv == -21) RetVal= 2820; 
	if(TempConv == -20) RetVal= 3000; 
	if(TempConv == -19) RetVal= 3200; 
	if(TempConv == -18) RetVal= 3400; 
	if(TempConv == -17) RetVal= 3600; 
	if(TempConv == -16) RetVal= 3800; 
	if(TempConv == -15) RetVal= 4000; 
	if(TempConv == -14) RetVal= 4200; 
	if(TempConv == -13) RetVal= 4400; 
	if(TempConv == -12) RetVal= 4600; 
	if(TempConv == -11) RetVal= 4800; 
	if(TempConv == -10) RetVal= 5000; 
	if(TempConv == -9)	RetVal= 5200; 
	if(TempConv == -8)	RetVal= 5400; 
	if(TempConv == -7)	RetVal= 5600; 
	if(TempConv == -6)	RetVal= 5800; 
	if(TempConv == -5)	RetVal= 6000; 
	if(TempConv == -4)	RetVal= 6200; 
	if(TempConv == -3)	RetVal= 6400; 
	if(TempConv == -2)	RetVal= 6600; 
	if(TempConv == -1)	RetVal= 6800; 
	if(TempConv ==  0)	RetVal= 7000; 
	if(TempConv ==  1)	RetVal= 7300; 
	if(TempConv ==  2)	RetVal= 7600; 
	if(TempConv ==  3)	RetVal= 7900; 
	if(TempConv ==  4)	RetVal= 8200; 
	if(TempConv ==  5)	RetVal= 8500; 
	if(TempConv ==  6)	RetVal= 8800; 
	if(TempConv ==  7)	RetVal= 9100; 
	if(TempConv ==  8)	RetVal= 9400; 
	if(TempConv ==  9)	RetVal= 9700; 
	if(TempConv == 10)	RetVal= 10000; 
	if(TempConv == 11)	RetVal= 10350; 
	if(TempConv == 12)	RetVal= 10700; 
	if(TempConv == 13)	RetVal= 10950; 
	if(TempConv == 14)	RetVal= 11300; 
	if(TempConv == 15)	RetVal= 11650; 
	if(TempConv == 16)	RetVal= 12000; 
	if(TempConv == 17)	RetVal= 12350; 
	if(TempConv == 18)	RetVal= 12700; 
	if(TempConv == 19)	RetVal= 13050; 
	if(TempConv == 20)	RetVal= 13500; 
	if(TempConv == 21)	RetVal= 13950; 
	if(TempConv == 22)	RetVal= 14400; 
	if(TempConv == 23)	RetVal= 14850; 
	if(TempConv == 24)	RetVal= 15300; 
	if(TempConv == 25)	RetVal= 15750; 
	if(TempConv == 26)	RetVal= 16200; 
	if(TempConv == 27)	RetVal= 16650; 
	if(TempConv == 28)	RetVal= 17100; 
	if(TempConv == 29)	RetVal= 17550; 
	if(TempConv == 30)	RetVal= 18000; 
	if(TempConv == 31)	RetVal= 18500; 
	if(TempConv == 32)	RetVal= 19000; 
	if(TempConv == 33)	RetVal= 19500;
	if(TempConv == 34)	RetVal= 20000;
	if(TempConv == 35)	RetVal= 20500;
	if(TempConv == 36)	RetVal= 21000;
	if(TempConv == 37)	RetVal= 21500;
	if(TempConv == 38)	RetVal= 22000;
	if(TempConv == 39)	RetVal= 22500;
	if(TempConv == 40)	RetVal= 23000;
	if(TempConv == 41)	RetVal= 23700;
	if(TempConv == 42)	RetVal= 24400;
	if(TempConv == 43)	RetVal= 25100;
	if(TempConv == 44)	RetVal= 25800;
	if(TempConv == 45)	RetVal= 26500;
	if(TempConv == 46)	RetVal= 27200;
	if(TempConv == 47)	RetVal= 27900;
	if(TempConv == 48)	RetVal= 28600;
	if(TempConv == 49)	RetVal= 29300;
	if(TempConv == 50)	RetVal= 30000;
	if(TempConv == 51)	RetVal= 30830;
	if(TempConv == 52)	RetVal= 31660;
	if(TempConv == 53)	RetVal= 32490;
	if(TempConv == 54)	RetVal= 33320;
	if(TempConv == 55)	RetVal= 34150;
	if(TempConv == 56)	RetVal= 34980;
	if(TempConv == 57)	RetVal= 35810;
	if(TempConv == 58)	RetVal= 36640;
	if(TempConv == 59)	RetVal= 37470;
	if(TempConv == 60)	RetVal= 38300;
	if(TempConv == 61)	RetVal= 39130;
	if(TempConv == 62)	RetVal= 40000;
	if(TempConv == 63)	RetVal= 40870;
	if(TempConv == 64)	RetVal= 41740;
	if(TempConv == 65)	RetVal= 42610;
	if(TempConv == 66)	RetVal= 43480;
	if(TempConv == 67)	RetVal= 44350;
	if(TempConv == 68)	RetVal= 45220;
	if(TempConv == 69)	RetVal= 46090;
	if(TempConv == 70)	RetVal= 47000;
	
	return RetVal;
	
}

unsigned int DinamicSuperHeatCold(void)
{ 
#if(K_UTAEnable==0)
	int DeltaTemp;
    //unsigned int RetVal;
	
	DeltaTemp = Me.Temperature.Ambient - Me.SP;			

	if(DeltaTemp <= 0)   return  1200; 
	if(DeltaTemp <= 25)  return  1100; 
	if(DeltaTemp <= 50)  return  1000; 
	if(DeltaTemp <= 75)  return  900; 
	if(DeltaTemp <= 100) return  800; 
	if(DeltaTemp <= 125) return  750; 
	if(DeltaTemp <= 150) return  700; 
	if(DeltaTemp <= 175) return  650; 
	if(DeltaTemp <= 200) return  600; 
	if(DeltaTemp <= 225) return  575; 
	if(DeltaTemp <= 250) return  525; 
	if(DeltaTemp <= 300) return  500; 
	if(DeltaTemp <= 400) return  400;
	if(DeltaTemp <= 500) return  450;
	if(DeltaTemp > 500)  return  400; 
#else

    static int CorrectValueSuperHeat=0;

    //if(Me.Temperature.AirOut<Me.CorrectSuperHeatAirOut)

    TimerCorrectSuperHeatAirOut.Enable = 1;

    if(Me.Temperature.AirOut>(Me.SP+100))
    {
        if(TimerCorrectSuperHeatAirOut.TimeOut)
        {
            TimerCorrectSuperHeatAirOut.TimeOut = 0;
            TimerCorrectSuperHeatAirOut.Value = 0;
            if(ValvePID.SetP >(Me.SuperHeat_SP-Me.Dinamic_SuperHeat))
               CorrectValueSuperHeat = CorrectValueSuperHeat-100; 
        }
    }

    if(Me.Temperature.AirOut<(Me.SP-100))
    {
        if(TimerCorrectSuperHeatAirOut.TimeOut)
        {
            TimerCorrectSuperHeatAirOut.TimeOut = 0;
            TimerCorrectSuperHeatAirOut.Value = 0;
            if(ValvePID.SetP <(Me.SuperHeat_SP+Me.Dinamic_SuperHeat))
               CorrectValueSuperHeat = CorrectValueSuperHeat+100; 
        }
    }
    
    return Me.SuperHeat_SP + (CorrectValueSuperHeat);

#endif
	//return RetVal;
}



/*  *****************************************************************************************************************************
	Aggiorno le variabili ElabValvola, in particolare l'abilitazione ed il fault.
	Successivamente se è tutto ok e deve lavorare mi calcolo la temperatura di evaporazione della batteria
	***************************************************************************************************************************** */
void TempEvaporazione(void)
{
	int ErrTemp;
	int DeltaEvap;
	int DeltaTemp;
	float kf;
	
    switch(Me.Function_Mode)
    {	
#if(K_UTAEnable==0)				
        case CoreRaffrescamento:		// Freddo Condizionamento
#if(K_ManualEvapTempEnable==0)                                   
            DeltaEvap = (Me.Freddo_TempBatt_SP - Me.Freddo_TempBatt_Min);
            DeltaTemp = (Me.SP + Freddo_Ist_Evap_SP_Max) - (Me.SP - Freddo_Ist_Evap_SP_Min);
            kf = (float)DeltaEvap/(float)DeltaTemp;

            //ErrTemp = (Me.SP + Freddo_Ist_Evap_SP_Max)-Me.Temperature.Ambient;
            ErrTemp = (Me.SP + Freddo_Ist_Evap_SP_Max)-Me.TempAmbRealSelection;

            if(ErrTemp < 0) ErrTemp=0; 
            if(ErrTemp > DeltaTemp)  ErrTemp=DeltaTemp; 


#if((SplitMode==K_I_Am_Freezer) || (SplitMode==K_I_Am_Frigo) || (SplitMode==K_I_Am_LoopWater))
            Me.Evap_Temp = Me.Freddo_TempBatt_SP;
#else
            Me.Evap_Temp = Me.Freddo_TempBatt_SP + (0-(int)((float)ErrTemp * kf));
#endif
            Me.EvapTempValve = Me.Evap_Temp;
#else             
            if(iDigiIn1)
                Me.Evap_Temp = Freddo_DigiIn1_EvapTemp;	//7 Bar
            else if(iDigiIn2)
                Me.Evap_Temp = Freddo_DigiIn2_EvapTemp;	//7 Bar		                
#endif                                
            break;

        case CoreRiscaldamento:			// Caldo Riscaldamento
#if(K_ManualEvapTempEnable==0)                                   
            DeltaEvap = (Me.Caldo_TempBatt_SP - Me.Caldo_TempBatt_Min);
            DeltaTemp = (Me.SP + 100) - (Me.SP - 100);
            kf = (float)DeltaEvap/(float)DeltaTemp;

            
            //ErrTemp = (Me.SP + 100)-Me.Temperature.Ambient;
            ErrTemp = (Me.SP + 100)-Me.TempAmbRealSelection;

            if(ErrTemp < 0) ErrTemp=0; 
            if(ErrTemp > DeltaTemp)  ErrTemp=DeltaTemp; 


            Me.Evap_Temp = Me.Caldo_TempBatt_Min+((int)((float)ErrTemp * kf));
#else
            if(iDigiIn1)
                Me.Evap_Temp = Caldo_DigiIn1_EvapTemp;	//18 Bar
            else if(iDigiIn2)
                Me.Evap_Temp = Caldo_DigiIn2_EvapTemp;	//22 Bar	                
#endif				                
            break;
#else
        case CoreRaffrescamento:		// Freddo Condizionamento
            Me.Evap_Temp = 200; //richiesta 7,5 bar fissi!
            Me.EvapTempValve = Me.Evap_Temp;
            break;

        case CoreRiscaldamento:			// Caldo Riscaldamento
            DeltaEvap = (Me.Caldo_TempBatt_SP - Me.Caldo_TempBatt_Min);
            DeltaTemp = (Me.SP + Caldo_Ist_SP) - (Me.SP - Caldo_Ist_SP);
            kf = (float)DeltaEvap/(float)DeltaTemp;

            ErrTemp = (Me.SP + Caldo_Ist_SP)-Me.Temperature.AirOut;

            if(ErrTemp < 0) ErrTemp=0; 
            if(ErrTemp > DeltaTemp)  ErrTemp=DeltaTemp; 


            Me.Evap_Temp = Me.Caldo_TempBatt_Min+((int)((float)ErrTemp * kf));
            break;                
#endif


        default:
                if(EngineBox.RealFuncMode==EngineBox_Freddo || EngineBox.RealFuncMode==EngineBox_GoFreddo)
                    Me.Evap_Temp=8000;// inizializzo a 80°C
                else
                    Me.Evap_Temp=0;// inizializzo a 0°C
            break;

    }
#if(K_Humidity_Correct==1)
    if(Me.Humidity_Correct==1)
    {
        Me.Evap_Press = EngineBox.Set_Press_Dehumi; //PressureGas(Me.Evap_Temp);
        Me.Evap_Temp = PressureGasToTempR410A(EngineBox.Set_Press_Dehumi); //PressureGas(Me.Evap_Temp);
    }
    else
        Me.Evap_Press = TempToPressureGasR410A(Me.Evap_Temp); //PressureGas(Me.Evap_Temp);
#else
	Me.Evap_Press = TempToPressureGasR410A(Me.Evap_Temp); //PressureGas(Me.Evap_Temp);
#endif
	//Me.Evap_Press = Filter(&FilterEvap_Press, Me.Evap_Press, K_Campioni_FilterEvapPress);	//TOLTO PERCHè ISTANTANEO LETTURA PRESSIONE DA RAGGIUNGERE!!! SI MEDIA SU MASTER!!!!
}


void PutStringToBUS2(int Val1, int Val2, int Val3, int Val4, int Val5, int Val6)
{
	static int cnt = 0;
	unsigned char txm[32];	
	int n,x;				
	
	set_baudrateU2_38400();		// Imposta Baud LowSpeed
	
	cnt++;	
	if(cnt==1)	
	{	
		n = sprintf(txm, "CLEARDATA\r\n");   
		for(x=0; x<n; x++) putch2(txm[x]);           

   	
	}
	n = sprintf(txm, "DATA,");
	for(x=0; x<n; x++) putch2(txm[x]);   	
	n = sprintf(txm, "%i, ", Val1);
	for(x=0; x<n; x++) putch2(txm[x]);   
	n = sprintf(txm, "%i, ", Val2);
	for(x=0; x<n; x++) putch2(txm[x]);  
 	n = sprintf(txm, "%i, ", Val3);
	for(x=0; x<n; x++) putch2(txm[x]);    	
 	n = sprintf(txm, "%i, ", Val4);
	for(x=0; x<n; x++) putch2(txm[x]); 
 	n = sprintf(txm, "%i, ", Val5);
	for(x=0; x<n; x++) putch2(txm[x]);
 	n = sprintf(txm, "%i, ", Val6);
	for(x=0; x<n; x++) putch2(txm[x]);    
 	n = sprintf(txm, "\r\n");
	for(x=0; x<n; x++) putch2(txm[x]); 	
	
	if(cnt==100)	
	{
		cnt=0;
		for(x=0; x<n; x++) putch2(txm[x]);      			 
		n = sprintf(txm, "ROW,SET,2\r\n");
	}	

	set_baudrateU2_57600();		// Ripristina Baud HiSpeed	
	
}




/*  *****************************************************************************************************************************
	Regolo l'apertura della valvola in base alle informazioni di feedBeak, per mantenere la temperatura di evaporazione corretta
	***************************************************************************************************************************** */
void RegolaValvola(void)
{	
	int Errore = 0;
	static int BloccaRegolaValveFreddo = 0;
	static int BloccaRegolaValveCaldo = 1;
	static int TrigInitValvola = 0;
	static int RicEvapCalcCaldo = 0;
	static int Valve_Pid = 0;
	static int Valve_ErrPress = 0;
	static int ValveCounter = 0;
    static int LastFunctMode = 0;   //1=OFF-RIscaldamento; 2=RIscaldamento; 3=Off-Raffrescamento; 3=Raffrescamento
    static int BloccoTimer = 0; 
    static int TrigYesValveOnFrozen = 0;
           int Lim_On_Demand_Freddo_Value = 0;
    
   
    int Valve_VoLimMin = 0; 
    int Valve_VoLimMax = 0;
    int Valve_VoLimOpenCold = 0;
    int TempOutput = 0;
    int ValveValueErrTemp = 0;
    
    static int ValveValueErrTemp_HTemp;
    static int CorrectValueSuperHeat=0;
    
    
    if(EngineBox.TestAllAbil.SuperHeat_Custom_Par_Cool_PID)
    {
        ValvePID.pGain = ((float)EngineBox.TestAll_SuperHeat_Cool_pGain)/100.0;    		// proportional gain
        ValvePID.iGain = ((float)EngineBox.TestAll_SuperHeat_Cool_iGain)/100.0;			// integral gain
        ValvePID.dGain = ((float)EngineBox.TestAll_SuperHeat_Cool_dGain)/100.0;     		// derivative gain
        TimerExecPID.Time = EngineBox.TestAll_SuperHeat_Cool_Time_Exec_PID;
    }
    else
    {
        ValvePID.pGain = K_p_gain;    		// proportional gain
        ValvePID.iGain = K_i_gain;			// integral gain
        ValvePID.dGain = K_d_gain;     		// derivative gain
        TimerExecPID.Time = K_TimeExecPID;
    }
    
    CalculateValveRangeValue();
    
    EngineBox.Lim_Max_Press_Correct_Valve = EngineBox.PressureHiLiqValue-2000;
    
    Me.ExpValve_Err_Code = K_CodeErrBox_NoErr;
    Me.ExpValve_Err = 0;
    
	if(ComunicationLost)
	{
		if(Me.DefMode == CoreRaffrescamento || Me.DefMode == CoreSlaveFreddo)
			Me.ExpValve_Req = Me.ValveLim.ValveLostCommCool;
		else
			Me.ExpValve_Req = Me.ValveLim.ValvoLostCommHot;
	}
	else if(EngineBox.Test.FluxReq)					// in caso di test di flusso del condensatore
		Me.ExpValve_Req = Me.ValveLim.EEV_FluxTest;					// imposto la valvola al valore del test
	else 
    {
        //Controllo valvola per Errore Alta Pressione Liquido (PressureHi)
        if((Me.Function_Mode == CoreRiscaldamento) || (Me.Function_Mode == CoreRaffrescamento) || (Touch[0].FunctionMode == CoreOff))
        {
            Valve_VoLimMin = Me.ValveLim.ValvolaChiusa; 
            Valve_VoLimMax = (int)((float)K_Freddo_Valve_Perc_Liq_PressHi_Work * (float)Me.ValveLim.ValvolaAperta / 100.0);
        }
        else
        {
            Valve_VoLimMin = Me.ValveLim.ValvolaChiusa; 
            Valve_VoLimMax = (int)((float)K_Freddo_Valve_Perc_Liq_PressHi_StandBy * (float)Me.ValveLim.ValvolaAperta / 100.0);
        }
        if(EngineBox.Condenser_Press_Abil)
            ValveValueErrTemp = (unsigned int)ValueIn2ValueOut((float)EngineBox.Pressure_LiquidCond, (float)(EngineBox.PressureHiLiqValue-2000), (float)EngineBox.PressureHiLiqValue, (float)Valve_VoLimMin, (float)Valve_VoLimMax);
        else        
            ValveValueErrTemp = (unsigned int)ValueIn2ValueOut((float)EngineBox.Pressione_Liq_P, (float)(EngineBox.PressureHiLiqValue-2000), (float)EngineBox.PressureHiLiqValue, (float)Valve_VoLimMin, (float)Valve_VoLimMax);

        if(ValveValueErrTemp>Me.ExpValve_Err)
        {
            Me.ExpValve_Err = ValveValueErrTemp;
            Me.ExpValve_Err_Code = K_CodeErrBox_PressureHi;            
        }
        
        //Controllo valvola per Errore Alta Pressione Gas (PressureHi)
        if((Me.Function_Mode == CoreRiscaldamento) || (Me.Function_Mode == CoreRaffrescamento) || (Touch[0].FunctionMode == CoreOff))
        {
            Valve_VoLimMin = Me.ValveLim.ValvolaChiusa; 
            Valve_VoLimMax = (int)((float)K_Freddo_Valve_Perc_Gas_PressHi_Work * (float)Me.ValveLim.ValvolaAperta / 100.0);
        }
        else
        {
            Valve_VoLimMin = Me.ValveLim.ValvolaChiusa; 
            Valve_VoLimMax = (int)((float)K_Freddo_Valve_Perc_Gas_PressHi_StandBy * (float)Me.ValveLim.ValvolaAperta / 100.0);
        }
        ValveValueErrTemp = (unsigned int)ValueIn2ValueOut((float)EngineBox.Pressione_Gas_G, (float)(EngineBox.PressureHiGasValue-2000), (float)EngineBox.PressureHiGasValue, (float)Valve_VoLimMin, (float)Valve_VoLimMax);
        if(EngineBox.Error.Pressure_Hi)
            ValveValueErrTemp = Valve_VoLimMax;

        if(ValveValueErrTemp>Me.ExpValve_Err)
        {
            Me.ExpValve_Err = ValveValueErrTemp;
            Me.ExpValve_Err_Code = K_CodeErrBox_PressureHi;            
        }

        //Controllo valvola per Errore Bassa Pressione Liquido (PressureLo)
        if((Me.Function_Mode == CoreRiscaldamento) || (Me.Function_Mode == CoreRaffrescamento) || (Touch[0].FunctionMode == CoreOff))
        {
            Valve_VoLimMin = Me.ValveLim.ValvolaChiusa; 
            Valve_VoLimMax = (int)((float)K_Freddo_Valve_Perc_Liq_PressLo_Work * (float)Me.ValveLim.ValvolaAperta / 100.0);
        }
        else
        {
            Valve_VoLimMin = Me.ValveLim.ValvolaChiusa; 
            Valve_VoLimMax = (int)((float)K_Freddo_Valve_Perc_Liq_PressLo_StandBy * (float)Me.ValveLim.ValvolaAperta / 100.0);
        }
        if(Me.DefMode==CoreRiscaldamento)
        {
            if((Me.Function_Mode == CoreRiscaldamento) || (Me.Function_Mode == CoreRaffrescamento))
            {
                if(EngineBox.Condenser_Press_Abil)
                    ValveValueErrTemp = (unsigned int)ValueIn2ValueOut((float)EngineBox.Pressure_LiquidCond, (float)(EngineBox.PressureLoLiqValue+2000), (float)EngineBox.PressureLoLiqValue, (float)Valve_VoLimMin, (float)Valve_VoLimMax);
                else
                    ValveValueErrTemp = (unsigned int)ValueIn2ValueOut((float)EngineBox.Pressione_Liq_P, (float)(EngineBox.PressureLoLiqValue+2000), (float)EngineBox.PressureLoLiqValue, (float)Valve_VoLimMin, (float)Valve_VoLimMax);
            }
            else
            {
                if(EngineBox.Condenser_Press_Abil)
                    ValveValueErrTemp = (unsigned int)ValueIn2ValueOut((float)EngineBox.Pressure_LiquidCond, (float)(EngineBox.PressureLoLiqValue+2500), (float)EngineBox.PressureLoLiqValue+1500, (float)Valve_VoLimMin, (float)Valve_VoLimMax);
                else
                    ValveValueErrTemp = (unsigned int)ValueIn2ValueOut((float)EngineBox.Pressione_Liq_P, (float)(EngineBox.PressureLoLiqValue+2500), (float)EngineBox.PressureLoLiqValue+1500, (float)Valve_VoLimMin, (float)Valve_VoLimMax);
            }                
        }
        else
        {
            if(EngineBox.Condenser_Press_Abil)
                ValveValueErrTemp = (unsigned int)ValueIn2ValueOut((float)EngineBox.Pressure_LiquidCond, (float)(EngineBox.PressureLoLiqValue+1000), (float)EngineBox.PressureLoLiqValue, (float)Valve_VoLimMin, (float)Valve_VoLimMax);
            else
                ValveValueErrTemp = (unsigned int)ValueIn2ValueOut((float)EngineBox.Pressione_Liq_P, (float)(EngineBox.PressureLoLiqValue+1000), (float)EngineBox.PressureLoLiqValue, (float)Valve_VoLimMin, (float)Valve_VoLimMax);            
        }
        
        if(ValveValueErrTemp>Me.ExpValve_Err)
        {
            Me.ExpValve_Err = ValveValueErrTemp;
            Me.ExpValve_Err_Code = K_CodeErrBox_PressureLo;        
        }
        //Controllo valvola per Errore Bassa Pressione Gas (PressureLo)
        if((Me.Function_Mode == CoreRiscaldamento) || (Me.Function_Mode == CoreRaffrescamento) || (Touch[0].FunctionMode == CoreOff))
        {
            Valve_VoLimMin = Me.ValveLim.ValvolaChiusa; 
            Valve_VoLimMax = (int)((float)K_Freddo_Valve_Perc_Gas_PressLo_Work * (float)Me.ValveLim.ValvolaAperta / 100.0);
        }
        else
        {
            Valve_VoLimMin = Me.ValveLim.ValvolaChiusa; 
            Valve_VoLimMax = (int)((float)K_Freddo_Valve_Perc_Gas_PressLo_StandBy * (float)Me.ValveLim.ValvolaAperta / 100.0);
        }
        ValveValueErrTemp = (unsigned int)ValueIn2ValueOut((float)EngineBox.Pressione_Gas_G, (float)(EngineBox.PressureLoGasValue+1000), (float)EngineBox.PressureLoGasValue, (float)Valve_VoLimMin, (float)Valve_VoLimMax);
        if(EngineBox.Error.Pressure_Lo)
            ValveValueErrTemp = Valve_VoLimMax;

        if(ValveValueErrTemp>Me.ExpValve_Err)
        {
            Me.ExpValve_Err = ValveValueErrTemp;
            Me.ExpValve_Err_Code = K_CodeErrBox_PressureLo;        
        }
        
        //Controllo valvola per Errore Alta Temperatura Compressore (CompressorHi)
        if(Me.DefMode==CoreRiscaldamento)
        {
            if((Me.Function_Mode == CoreRiscaldamento) || (Touch[0].FunctionMode == CoreOff))
            {
                Valve_VoLimMin = Me.ValveLim.ValvolaChiusa; 
                Valve_VoLimMax = (int)((float)K_Freddo_Valve_Perc_TempHi_Work * (float)Me.ValveLim.ValvolaAperta / 100.0);
            }
            else
            {
                Valve_VoLimMin = Me.ValveLim.ValvolaChiusa; 
                Valve_VoLimMax = (int)((float)K_Freddo_Valve_Perc_TempHi_StandBy * (float)Me.ValveLim.ValvolaAperta / 100.0);
            }
            if(EngineBox.Compressor_Speed>0)
                TempOutput = EngineBox.Temp_Mandata;
            if(EngineBox.Compressor_Speed_C2>0)
            {
                if(EngineBox.Temp_Compressor_Output_C2>TempOutput)
                   TempOutput = EngineBox.Temp_Compressor_Output_C2;
            }
            ValveValueErrTemp = (unsigned int)ValueIn2ValueOut((float)TempOutput, (float)EngineBox.Threshold_Compressor_Hi-500, (float)EngineBox.Threshold_Compressor_Hi, (float)Valve_VoLimMin, (float)Valve_VoLimMax);


            if(ValveValueErrTemp>Me.ExpValve_Err)
            {
                Me.ExpValve_Err = ValveValueErrTemp;
                Me.ExpValve_Err_Code = K_CodeErrBox_CompressorHi;        
            }
        }
        if(Me.DefMode==CoreRaffrescamento)
        {
            Valve_VoLimMax = (int)((float)K_Freddo_Valve_Perc_TempHi_Cold * (float)Me.ValveLim.ValvolaAperta / 100.0);
            Valve_VoLimOpenCold = (int)((float)K_Freddo_Valve_Lim_Perc_TempHi_Cold * (float)Me.ValveLim.ValvolaAperta / 100.0);

            if(EngineBox.Error.CompressorHi || EngineBox.Error.CompressorHi_C2)
            {
                if(Me.Function_Mode == CoreRaffrescamento)
                {
                    if(Me.ExpValve_Err_Code==0)
                    Me.ExpValve_Err_Code = K_CodeErrBox_CompressorHi;
                }
                else
                {
                    TimerExpValv_Err.Enable = 1;

                    if(TimerExpValv_Err.TimeOut)
                    {
                        TimerExpValv_Err.Time = K_Time_Go_Correct_Valve_Err;
                        TimerExpValv_Err.TimeOut = 0;
                        if(ValveValueErrTemp_HTemp<Valve_VoLimOpenCold)
                        {
                            ValveValueErrTemp_HTemp = ValveValueErrTemp_HTemp+Valve_VoLimMax;                    
                        }
                    }
                    if(ValveValueErrTemp_HTemp>Me.ExpValve_Err)
                    {
                        Me.ExpValve_Err = ValveValueErrTemp_HTemp;
                        Me.ExpValve_Err_Code = K_CodeErrBox_CompressorHi;
                    }
                }
            }
            else
            {
                ValveValueErrTemp_HTemp = 0;
                TimerExpValv_Err.Enable = 0;
                TimerExpValv_Err.Value = 0;
                TimerExpValv_Err.Time = K_Time_Wait_Correct_Valve_Err;
            }
        }

#if (SplitMode == K_I_Am_Frigo)
        if(Me.Function_Mode != CoreRaffrescamento)
        {
            Me.ExpValve_Err = 0;
            Me.ExpValve_Err_Code = K_CodeErrBox_NoErr;        
        }
#elif (SplitMode == K_I_Am_Freezer)
        if(Me.Error.Battery_Defrost || Me.Error.Battery_Drip || ((Me.Function_Mode != CoreRaffrescamento)))
        {
            Me.ExpValve_Err = 0;
            Me.ExpValve_Err_Code = K_CodeErrBox_NoErr;        
        }  
#elif (SplitMode == K_I_Am_StandardUTA || SplitMode == K_I_Am_UTA_and_InvFAN)
        if(Me.Function_Mode == CoreVentilazione)
        {
            Me.ExpValve_Err = 0;
            Me.ExpValve_Err_Code = K_CodeErrBox_NoErr;        
        }                
#endif
#if (SplitMode == K_I_Am_LoopWater)
        if((Me.Function_Mode != CoreRaffrescamento) || (Me.Temperature.AirOut<K_Low_Temp_WaterOut))
        {
            Me.ExpValve_Err = 0;
            Me.ExpValve_Err_Code = K_CodeErrBox_NoErr;        
        }
#endif

        /*
        //Controllo valvola per Errore Bassa Temperatura(CompressorLo)
        if(Me.Function_Mode == CoreRiscaldamento)
        {
            Valve_VoLimMin = (int)((float)K_Freddo_Valve_Perc_TempLo_Hot_Work * (float)Me.ValveLim.ValvolaAperta / 100.0);
            Valve_VoLimMax = Me.ValveLim.ValvolaChiusa; 
          
            ValveValueErrTemp = (int)ValueIn2ValueOut((float)EngineBox.Temp_Ritorno, (float)K_Freddo_Valve_ViLimMin_TempLo_Hot, (float)K_Freddo_Valve_ViLimMax_TempLo_Hot, (float)Valve_VoLimMin, (float)Valve_VoLimMax);
 
            if(ValveValueErrTemp<0 && Me.ExpValve_Err==0)
            {
                Me.ExpValve_Err = ValveValueErrTemp;
                Me.ExpValve_Err_Code = K_CodeErrBox_CompressorLo;            
            }
        }

        if(Me.Function_Mode == CoreRaffrescamento)
        {
            Valve_VoLimMin = (int)((float)K_Freddo_Valve_Perc_TempLo_Cold_Work * (float)Me.ValveLim.ValvolaAperta / 100.0);
            Valve_VoLimMax = Me.ValveLim.ValvolaChiusa; 
          
            ValveValueErrTemp = (int)ValueIn2ValueOut((float)EngineBox.Temp_Ritorno, (float)K_Freddo_Valve_ViLimMin_TempLo_Cold, (float)K_Freddo_Valve_ViLimMax_TempLo_Cold, (float)Valve_VoLimMin, (float)Valve_VoLimMax);
 
            if(ValveValueErrTemp<0 && Me.ExpValve_Err==0 && Me.ExpValve_Err_Code != K_CodeErrBox_CompressorHi)
            {
                Me.ExpValve_Err = ValveValueErrTemp;
                Me.ExpValve_Err_Code = K_CodeErrBox_CompressorLo;            
            }
        }
        */
        /*        
        if(EngineBox.Error.Pressure_Lo)
        {
            ValveValueErrTemp = 0;//SISTEMARE CON FORMULA
            
            if(ValveValueErrTemp > Me.ExpValve_Err)
            {
                Me.ExpValve_Err = ValveValueErrTemp;
                Me.ExpValve_Err_Code = K_CodeErrBox_PressureLo;
            }
        }
        if(EngineBox.Error.CompressorHi || EngineBox.Error.CompressorHi_C2)
        {
            ValveValueErrTemp = 0;//SISTEMARE CON FORMULA
            
            if(ValveValueErrTemp > Me.ExpValve_Err)
            {
                Me.ExpValve_Err = ValveValueErrTemp;
                Me.ExpValve_Err_Code = K_CodeErrBox_CompressorHi;
            }
        }
        if(EngineBox.Error.CompressorLo)
        {
            if(ValveValueErrTemp == 0)
            {
                ValveValueErrTemp = 0;//SISTEMARE CON FORMULA

                Me.ExpValve_Err = ValveValueErrTemp;
                Me.ExpValve_Err_Code = K_CodeErrBox_CompressorLo;
            }
        }
*/
        //Se i motori sono fermi, chiudo subito tutta la valvola, se non è un errore di pressione
        if((EngineBox.Compressor_Speed==0) && (EngineBox.Compressor_Speed_C2==0) && 
            !EngineBox.Error.Pressure_Hi && !EngineBox.Error.Pressure_Lo)        
        {
            Me.ExpValve_Err = 0;
            Me.ExpValve_Err_Code = K_CodeErrBox_NoErr;
        }

#if (SplitMode != K_I_Am_Frigo && SplitMode != K_I_Am_Freezer)
        //Se la ventola del FanCoil è ferma e ho le temperature liq-gas sotto i 5C, chiudo tutta la valvola per evitare ghiaccio
        if(((Me.Function_Mode == CoreOff) || Me.Error.Fan_Fault) && ((Me.Temperature.Liquid<500) || (Me.Temperature.Gas<500)) && ((EngineBox.Compressor_Speed>0) || (EngineBox.Compressor_Speed_C2>0)))
        {
            Me.ExpValve_Err = 0;
            Me.ExpValve_Err_Code = K_CodeErrBox_NoErr;        
        }                
#endif
        
        //Inizio con la regolazione della valvola
        Me.Setp_PressLiqP_Caldo = TempToPressureGasR410A(EngineBox.Temp_Acqua_Mare); //PressureGas(EngineBox.Temp_Acqua_Mare);	// Calcolo pressione di lavoro della valvola in base a temperatura acqua di mare secondo tbella conversione temperature / presisoni.
		if(Me.Setp_PressLiqP_Caldo>K_Lim_Max_Press_Liq_Caldo) 				// Se la pressione del liquido calcolata supera il limite massimo consentito
			Me.Setp_PressLiqP_Caldo=K_Lim_Max_Press_Liq_Caldo;				// correggo al limite							

        if(Me.InitValvolaInCorso == 0)
        {    
            TimerResyncValvola.Enable = ((Me.ExpValve_Req==0) && (Me.ExpValve_Act==0) && (Me.ExpValve_Delta_Req==0) && (Me.ExpValve_Delta_Act==0)) || 
                                        ((Me.ExpValve_Req==0) && (Me.ExpValve_Act==0) && (Me.ExpValve_Delta_Act>0));

            if((Me.ExpValve_Req==0) && (Me.ExpValve_Act==0) && (Me.ExpValve_Delta_Act>0))
            {
                if((EngineBox.RealFuncMode==EngineBox_GoOff)&&(EngineBox.Compressor_Speed==0) && (EngineBox.Compressor_Speed_C2==0))
                    TimerResyncValvola.Time = 1;
                else
                    TimerResyncValvola.Time = K_WaitDischOffsValvola;
            }
            else
                TimerResyncValvola.Time = K_WaitResyncValvola;

            if(TimerResyncValvola.TimeOut)
            {
                TimerResyncValvola.Enable = 0;
                TimerResyncValvola.TimeOut = 0;
                TimerResyncValvola.Value = 0;                     
                if((Me.ExpValve_Req==0) && (Me.ExpValve_Act==0) && (Me.ExpValve_Delta_Act>0))
                    DischOffsValvola();    
                else
                    ReSyncValvola();                    //Reinizializzo la valvola per essere sicuro che è allineata con lo zero.
            }
        }
  	
        switch(EngineBox.RealFuncMode)
        {
            case EngineBox_Caldo:								// se il box motore sta riscaldando

                TrigInitValvola = 0;

                if((Me.Function_Mode != CoreRiscaldamento) &&
                    (Me.Function_Mode != CoreSlaveCaldo))
                {
                    BloccaRegolaValveCaldo = 1;
                    TimerRestartValve.TimeOut=0;
                    TimerRestartValve.Value=0;	
                }
                
                if((Me.Function_Mode == CoreRiscaldamento) |
                    (Me.Function_Mode == CoreSlaveCaldo))		// e la camera chiede riscaldamento
                {
                    LastFunctMode = 2;  //Riscaldamento                            

                    TimerRestartValve.Enable = BloccaRegolaValveCaldo;

                    if(!TimerRestartValve.Enable)
                    {
                        TimerRestartValve.TimeOut=0;
                        TimerRestartValve.Value=0;	
                    }

                    if(TimerRestartValve.TimeOut)	
                        BloccaRegolaValveCaldo = 0;								

#if(K_UTAEnable==0)
                    RicEvapCalcCaldo = PressureGasToTempR410A(EngineBox.Pressione_Gas_G);
                    if(RicEvapCalcCaldo > Me.Evap_Temp)	//Se il limite supera la richiesta
                        RicEvapCalcCaldo = Me.Evap_Temp;		//raso al massimo a richiesta in quel momento.

                    //Me.EvapTempValve = RicEvapCalcCaldo-K_PerformanceLossBattery;   //Sottraggo PerformanceLossBattery gradi (o successivamente parametro) per resa batteria
                    Me.EvapTempValve = RicEvapCalcCaldo-Me.PerformanceLossBattery_SP;   //Sottraggo PerformanceLossBattery gradi (o successivamente parametro) per resa batteria 
#else
                    Me.EvapTempValve = Me.SP;
#endif

                    TimerExpValvola.Time = WaitExpValvolaCaldo;	// inizializzo il timer di regolazione alla velocità caldo
                    if(TimerExpValvola.TimeOut == 1)	// Esegue regolazione ogni xx sec??                                                  	
                    {	                   
#if(K_ValveNoPressCondenser==1)
                        LimiteMinCaldo	= Me.ValveLim.AperturaMinCaldo;	
                        if(BloccaRegolaValveCaldo)
                        {
                            Incrementa(&Me.ExpValve_Ctrl, K_IncOutValvole*2, LimiteMinCaldo);	// aumento graduale della valvola se entro in routine regolazione da split acceso
                            ValveCounter = Me.ExpValve_Ctrl;
                            if(Me.ExpValve_Ctrl>=LimiteMinCaldo)
                                BloccaRegolaValveCaldo = 0;
                        }
                        else
                        {		
                            DeltaValv = ((float)Me.ValveLim.AperturaMaxCaldo * 80.0)/100.0;   //80% //768.0;
                            DeltaPres = 10000.0;
                            k = DeltaValv/DeltaPres;

                            ErrPress = (float)((int)(EngineBox.Ric_Pressione-EngineBox.Pressione_Gas_G));

                            if(ErrPress < 0.0) ErrPress=0.0; 
                            if(ErrPress > DeltaPres)  ErrPress=DeltaPres; 

                            ErrorePressCaldo = (ErrPress * k);
                            LimiteMaxCaldo = Me.ValveLim.AperturaMaxCaldo-((int)ErrorePressCaldo);	

                            Errore = Me.EvapTempValve - Me.Temperature.AirOut;		//Sottraggo PerditaResaBatteria gradi (o successivamente parametro) per resa batteria
                            if(Errore>0)
                            {	
                                if(Errore>50)
                                Incrementa(&ValveCounter, K_IncOutValvole, LimiteMaxCaldo);		// Apro valvola fino a MAX per alzare la temperatura
                            }
                            else
                            {
                                Errore = Me.Temperature.AirOut - Me.EvapTempValve;	//Sottraggo PerditaResaBatteria gradi (o successivamente parametro) per resa batteria
                                if(Errore>0)
                                {	
                                    if(Errore>50)
                                    Decrementa(&ValveCounter, K_IncOutValvole, LimiteMinCaldo);		// Chiudo valvola fino a MIN per abbassare la temperatura
                                }
                            }
                           if(ValveCounter < Me.ValveLim.AperturaMinCaldo)
                                ValveCounter = Me.ValveLim.AperturaMinCaldo;
                           if(ValveCounter > Me.ValveLim.AperturaMaxCaldo)
                                ValveCounter = Me.ValveLim.AperturaMaxCaldo;

                            Me.ExpValve_Ctrl = ValveCounter; //COpio la richiesta di posizione della valvola da trattare poi con errori


                        }                                
#else                  
                        //ValveCounter = (int)ValueIn2ValueOut((float)EngineBox.SuperH_HeatCalc, (float)Me.ValveLim.ValvolaChiusa, (float)Me.ValveLim.ValvolaAperta, (float)Me.ValveLim.AperturaMinCaldo, (float)Me.ValveLim.AperturaMaxCaldo);//EngineBox.SuperH_HeatCalc;
                        if(BloccaRegolaValveCaldo)
                        {
                            Incrementa(&Me.ExpValve_Ctrl, K_IncOutValvole, EngineBox.SuperH_HeatCalc);
                            TimerExpValvola.TimeOut = 0;
                            ValveCounter = Me.ExpValve_Ctrl; //La richiesta in avvio è quella reale per non mandare subito in limitazione

                            if(Me.ExpValve_Ctrl >= EngineBox.SuperH_HeatCalc)
                                BloccaRegolaValveCaldo = 0;
                        }
                        else
                        {
                            ValveCounter = EngineBox.SuperH_HeatCalc;
                            if(ValveCounter < Me.ValveLim.AperturaMinCaldo)
                                ValveCounter = Me.ValveLim.AperturaMinCaldo;
                            if(ValveCounter > Me.ValveLim.AperturaMaxCaldo)
                                ValveCounter = Me.ValveLim.AperturaMaxCaldo;

                            Me.ExpValve_Ctrl = ValveCounter;
                        }
#endif
                        TimerExpValvola.TimeOut = 0;
                    }							
                }
                else											// se le modalità non sono concorde
                {
                    if(!BloccoTimer&&(LastFunctMode == 2))
                    {    
                        BloccoTimer = 1;
                        TimerExpValvola.Time = 10000;
                        TimerExpValvola.Value = 0;
                        TimerExpValvola.TimeOut = 0;
                    }
                    if(!BloccoTimer)                            
                        TimerExpValvola.Time = WaitExpValvolaCaldo;	// inizializzo il timer di regolazione alla velocità caldo
                    if(TimerExpValvola.TimeOut == 1)	// Esegue regolazione ogni xx sec??                                                  	
                    {	
                        BloccoTimer = 0;
                        LastFunctMode = 1; //OFF-Riscaldamento                                

                        Decrementa(&Me.ExpValve_Ctrl, K_IncOutValvole*4, Me.ValveLim.ValvolaChiusa);
                        TimerExpValvola.TimeOut = 0;

                        ValveCounter = Me.ExpValve_Ctrl;
                    }
                }
                break;

            case EngineBox_Freddo:
                //---------------------------------------------
                // Blocco ingresso regolatore PID (per riavvio iniziale valvola)
                //---------------------------------------------						
                if((Me.Function_Mode != CoreRaffrescamento) &&
                    (Me.Function_Mode != CoreSlaveFreddo))
                {
                    BloccaRegolaValveFreddo = 1;
                    TimerRestartValve.TimeOut=0;
                    TimerRestartValve.Value=0;	
                }

                if((Me.Function_Mode == CoreRaffrescamento) |
                    (Me.Function_Mode == CoreSlaveFreddo))		// Devo raffrescare
                {
                LastFunctMode = 4; //Raffrescamento	

                //---------------------------------------------
                // Blocco ingresso regolatore PID (per riavvio iniziale valvola)
                //---------------------------------------------						
                TimerRestartValve.Enable = BloccaRegolaValveFreddo;

                if(!TimerRestartValve.Enable)
                {
                    TimerRestartValve.TimeOut=0;
                    TimerRestartValve.Value=0;	
                }

                if(TimerRestartValve.TimeOut)	
                    BloccaRegolaValveFreddo = 0;								
                //---------------------------------------------
                // Nuova gestione valvola con regolatore PID
                //---------------------------------------------
#if(K_UTAEnable==0)
                    if(Me.Dinamic_SuperHeat==1)
                    {
                        ValvePID.SetP = DinamicSuperHeatCold() + EngineBox.SuperHeat_Correct_Cooling;
                    }
                    else
                    {
                        ValvePID.SetP = Me.SuperHeat_SP + EngineBox.SuperHeat_Correct_Cooling /*K_SetPoint*/;					// Setpoint! (Valore desiderato di SuperHeat) 
                    }											// Per taratura PID utilizzazre un canale analogico 
                                
                    if(Me.Humidity_Correct==1)
                        Me.CorrectSuperHeatAirOut=EngineBox.Set_Temp_Air_Out_Dehumi;            
                    else
                        Me.CorrectSuperHeatAirOut = Me.CorrectSuperHeatAirOut_Local;
                
                    TimerCorrectSuperHeatAirOut.Enable = (Me.Temperature.AirOut<Me.CorrectSuperHeatAirOut) || (CorrectValueSuperHeat>0);
                    if(TimerCorrectSuperHeatAirOut.Enable)
                    {                        
                        if(TimerCorrectSuperHeatAirOut.TimeOut)
                        {
                            if(Me.Temperature.AirOut<Me.CorrectSuperHeatAirOut)
                            {
                                if((ValvePID.SetP+CorrectValueSuperHeat)<K_Lim_CorrectSuperHeatAirOut)
                                   CorrectValueSuperHeat = CorrectValueSuperHeat+100; 
                                else if((ValvePID.SetP+CorrectValueSuperHeat)>K_Lim_CorrectSuperHeatAirOut)
                                {
                                    if(CorrectValueSuperHeat>0)
                                        CorrectValueSuperHeat = CorrectValueSuperHeat-100;
                                }
                            }
                            else
                            {
                                if(CorrectValueSuperHeat>0)
                                    CorrectValueSuperHeat = CorrectValueSuperHeat-100;
                            }                                
                            TimerCorrectSuperHeatAirOut.TimeOut = 0;
                            TimerCorrectSuperHeatAirOut.Value = 0;
                        }
                    }
                    else
                    {
                        CorrectValueSuperHeat = 0;
                        TimerCorrectSuperHeatAirOut.TimeOut = 0;
                        TimerCorrectSuperHeatAirOut.Value = 0;
                    }
                 
                
                    ValvePID.SetP = ValvePID.SetP + CorrectValueSuperHeat;
#if(SplitMode!=K_I_Am_LoopWater)                
                    if(Me.ExpValve_Err_Code == K_CodeErrBox_CompressorHi && ValvePID.SetP > K_Correct_SuperHeat_Valve_Err)
                        ValvePID.SetP = K_Correct_SuperHeat_Valve_Err;
#endif
#else
                    if(Me.Dinamic_SuperHeat!=0)
                        ValvePID.SetP = DinamicSuperHeatCold() + EngineBox.SuperHeat_Correct_Cooling;
                    else
                        ValvePID.SetP = Me.SP;
#endif
                    // (es. Me.Temperature.Gas) per simulare risposta a gradino
                    ValvePID.RetVal = (int)(((float)Me.Superheat * K_gainSuperHeat));//-((float)Me.EvapLiq * 1.0));										// Retroaction Value (settare a "0" per taratura PID ad anello aperto!)

                    if(TimerExecPID.TimeOut)
                    {
                        TimerExecPID.TimeOut=0;
                        TimerExecPID.Value=0;

                        UpdatePID(&ValvePID);
                        Valve_Pid = PIDOut2Valve(ValvePID.PVal, K_PoMin, K_PoMax, Me.ValveLim.AperturaMinFreddo, Me.ValveLim.AperturaMaxFreddo);
                                                
                        if(BloccaRegolaValveFreddo)
                        {
                            TimerExpValvola.Time = WaitExpValvolaFreddo;	// inizializzo il timer di regolazione alla velocità caldo

                            if(TimerExpValvola.TimeOut == 1)
                            {
                                Incrementa(&Me.ExpValve_Ctrl, K_IncOutValvole, Valve_Pid);
                                PID_SetOut(&ValvePID, Me.ExpValve_Ctrl);
                                TimerExpValvola.TimeOut = 0;
                                Me.ExpValve_Pid_Cooling = Me.ExpValve_Ctrl; //La richiesta in avvio è quella reale per non mandare subito in limitazione
                                
                                if(Me.ExpValve_Ctrl >= Valve_Pid || (Me.ExpValve_Cur_Number_Priority!=0))
                                    BloccaRegolaValveFreddo = 0;
                            }
                        }
                        else
                        {
                            //Correzione valvola se ho errore di alta pressione liquido
                            if(EngineBox.Condenser_Press_Abil)
                                Errore = EngineBox.Pressure_LiquidCond - EngineBox.Lim_Max_Press_Correct_Valve;
                            else                            
                                Errore = EngineBox.Pressione_Liq_P - EngineBox.Lim_Max_Press_Correct_Valve;	//Calcolo errore
                            if(Errore>0)	//Se errore >0
                            {
                                if(Errore>50)	//Se Errore >500 mBar
                                    Incrementa(&Valve_ErrPress, K_IncOutValvole*2, Me.ValveLim.ValveNoWorkMaxFreddo);		// Apro valvola fino a MAX per abbassare la pressione													
                            }
                            else
                            {
                                if(EngineBox.Condenser_Press_Abil)
                                   Errore = EngineBox.Lim_Max_Press_Correct_Valve - EngineBox.Pressure_LiquidCond;	//Calcolo errore
                                else                                                
                                    Errore = EngineBox.Lim_Max_Press_Correct_Valve - EngineBox.Pressione_Liq_P;	//Calcolo errore
                                if(Errore>50)	//Se Errore >500 mBar
                                    Decrementa(&Valve_ErrPress, K_IncOutValvole*2, Me.ValveLim.ValveNoWorkMinFreddo);		// Chiudo valvola fino a MIN per alzare la pressione
                            }
                            
                            Me.ExpValve_Pid_Cooling = Valve_Pid;
                            
                            if((Me.ExpValve_Cur_Number_Priority!=0)&&(Me.ExpValve_On_Demand_Priority!=0))
                            {
                                if(Me.ExpValve_On_Demand_Priority==Me.ExpValve_Cur_Number_Priority)
                                {
#if(K_ValveOnDemand_Max_Escursion_100P==1)
                                    Lim_On_Demand_Freddo_Value = (int)ValueIn2ValueOut((float)K_Max_Escurs_Open_Valve_OnDemand, 0.0, 100.0, (float)K_ValvolaChiusa, (float)K_MaxExcursionValve);
                                    Valve_Pid = PIDOut2Valve(ValvePID.PVal, K_PoMin, K_PoMax, Me.ValveLim.AperturaMinFreddo, Lim_On_Demand_Freddo_Value);
#endif
                                    Lim_On_Demand_Freddo_Value = (int)ValueIn2ValueOut((float)Me.ExpValve_On_Demand_Request, 0.0, 100.0, (float)K_ValvolaChiusa, (float)K_MaxExcursionValve);
                                    if(Valve_Pid>Lim_On_Demand_Freddo_Value)
                                        Me.ExpValve_Ctrl = Lim_On_Demand_Freddo_Value + Valve_ErrPress;
                                    else
                                        Me.ExpValve_Ctrl = Valve_Pid + Valve_ErrPress;
                                }
                                else
                                {
#if(K_ValveOnDemand_Min_Percent_On==0)
                                    Me.ExpValve_Ctrl = Valve_ErrPress;
#else
                                    Lim_On_Demand_Freddo_Value = (int)ValueIn2ValueOut((float)Me.ValveOnDemand_Min_Percent_Val, 0.0, 100.0, (float)K_ValvolaChiusa, (float)K_MaxExcursionValve);
                                    if(Valve_Pid>Lim_On_Demand_Freddo_Value)
                                        Me.ExpValve_Ctrl = Lim_On_Demand_Freddo_Value + Valve_ErrPress;
                                    else
                                        Me.ExpValve_Ctrl = Valve_Pid + Valve_ErrPress;
#endif
                                }
                            }
                            else
                            {
                                Me.ExpValve_Ctrl = Valve_Pid + Valve_ErrPress;
                            }
                            
                            if(Me.ExpValve_Ctrl<Me.ValveLim.AperturaMinFreddo)
                                Me.ExpValve_Ctrl=Me.ValveLim.AperturaMinFreddo;
                            if(Me.ExpValve_Ctrl>Me.ValveLim.AperturaMaxFreddo)
                                Me.ExpValve_Ctrl=Me.ValveLim.AperturaMaxFreddo;
                        }
                    }
                }
                else										// se le modalità non sono concorde
                {
                    if(!BloccoTimer&&(LastFunctMode == 4))
                    {    
                        BloccoTimer = 1;
                        TimerExpValvola.Time = 10000;
                        TimerExpValvola.Value = 0;
                        TimerExpValvola.TimeOut = 0;
                    }
                    if(!BloccoTimer)
                        TimerExpValvola.Time = WaitExpValvolaFreddo;	// inizializzo il timer di regolazione alla velocità caldo

                    if(TimerExpValvola.TimeOut == 1)
                    {
                        BloccoTimer = 0;
                        LastFunctMode = 3; //OFF-Raffrescamento

                        Decrementa(&Me.ExpValve_Ctrl, K_IncOutValvole*10/*2*/, Me.ValveLim.ValvolaChiusa);
                        PID_SetOut(&ValvePID, Me.ExpValve_Ctrl);
                        Me.ExpValve_Pid_Cooling = Me.ExpValve_Ctrl;

                        TimerExpValvola.TimeOut = 0;
                    }                            
                }
                
                break;

            case EngineBox_Off:							// se il box motore è fermo 
                    Me.ExpValve_Ctrl = Me.ValveLim.ValvolaOff;
                    PID_SetOut(&ValvePID, Me.ValveLim.ValvolaChiusa);		// Inizializzo uscita PID per avere "InitValvolaFreddo"								
                    Me.ExpValve_Pid_Cooling = Me.ExpValve_Ctrl;
                break;

        case EngineBox_GoOff:								// il box motore si sta spegnendo
            BloccoTimer = 0;
            if(LastFunctMode == 1)  //Ero in OFF-Riscaldamento
            {
                TimerExpValvola.Time = WaitExpValvolaCaldo;	// inizializzo il timer di regolazione alla velocità caldo
                if(TimerExpValvola.TimeOut == 1)	// Esegue regolazione ogni xx sec??                                                  	
                {	
                    Decrementa(&Me.ExpValve_Ctrl, K_IncOutValvole*4, Me.ValveLim.ValvolaChiusa);	//Per ora non agisce superheat
                    TimerExpValvola.TimeOut = 0;
                }                    
                //if(EngineBox.Compressor_Speed==0 && EngineBox.Compressor_Speed_C2==0)
                if(EngineBox.Inverter_Upper_Out_Freq==0)
                    Me.ExpValve_Ctrl = Me.ValveLim.ValvolaChiusa;		// la chiudo completamente
            }
            if(LastFunctMode == 2)  //ero in Riscaldamento
            {
#if(K_UTAEnable==0)
                RicEvapCalcCaldo = PressureGasToTempR410A(EngineBox.Pressione_Gas_G);
                if(RicEvapCalcCaldo > Me.Evap_Temp)	//Se il limite supera la richiesta
                    RicEvapCalcCaldo = Me.Evap_Temp;		//raso al massimo a richiesta in quel momento.

                Me.EvapTempValve = RicEvapCalcCaldo - Me.PerformanceLossBattery_SP;   //Sottraggo PerformanceLossBattery gradi (o successivamente parametro) per resa batteria
#else
                Me.EvapTempValve = Me.SP;
#endif
                TimerExpValvola.Time = WaitExpValvolaCaldo;	// inizializzo il timer di regolazione alla velocità caldo
                if(TimerExpValvola.TimeOut == 1)	// Esegue regolazione ogni xx sec??                                                  	
                {	                   
#if(K_ValveNoPressCondenser==1)
                    LimiteMinCaldo	= Me.ValveLim.AperturaMinCaldo;	

                    DeltaValv = ((float)Me.ValveLim.AperturaMaxCaldo * 80.0)/100.0;   //80%
                    DeltaPres = 10000.0;
                    k = DeltaValv/DeltaPres;

                    ErrPress = (float)((int)(EngineBox.Ric_Pressione-EngineBox.Pressione_Gas_G));

                    if(ErrPress < 0.0) ErrPress=0.0; 
                    if(ErrPress > DeltaPres)  ErrPress=DeltaPres; 

                    ErrorePressCaldo = (ErrPress * k);
                    LimiteMaxCaldo = Me.ValveLim.AperturaMaxCaldo-((int)ErrorePressCaldo);	

                    Errore = Me.EvapTempValve - Me.Temperature.AirOut;		//Sottraggo PerditaResaBatteria gradi (o successivamente parametro) per resa batteria
                    if(Errore>0)
                    {	
                        if(Errore>50)
                        Incrementa(&ValveCounter, K_IncOutValvole, LimiteMaxCaldo);		// Apro valvola fino a MAX per alzare la temperatura
                    }
                    else
                    {
                        Errore = Me.Temperature.AirOut - Me.EvapTempValve;	//Sottraggo PerditaResaBatteria gradi (o successivamente parametro) per resa batteria
                        if(Errore>0)
                        {	
                            if(Errore>50)
                            Decrementa(&ValveCounter, K_IncOutValvole, Me.ValveLim.Valve_Min_Go_Off);		// Chiudo valvola fino a MIN per abbassare la temperatura
                        }
                    }

                    if(ValveCounter < Me.ValveLim.AperturaMinCaldo)
                         ValveCounter = Me.ValveLim.AperturaMinCaldo;
                    if(ValveCounter > Me.ValveLim.AperturaMaxCaldo)
                         ValveCounter = Me.ValveLim.AperturaMaxCaldo;

                     Me.ExpValve_Ctrl = ValveCounter; //COpio la richiesta di posizione della valvola da trattare poi con errori
#else
                    
                     //ValveCounter = (int)ValueIn2ValueOut((float)EngineBox.SuperH_HeatCalc, (float)Me.ValveLim.ValvolaChiusa, (float)Me.ValveLim.ValvolaAperta, (float)Me.ValveLim.AperturaMinCaldo, (float)Me.ValveLim.AperturaMaxCaldo);//EngineBox.SuperH_HeatCalc;
                     ValveCounter = EngineBox.SuperH_HeatCalc;
                    if(ValveCounter < Me.ValveLim.AperturaMinCaldo)
                        ValveCounter = Me.ValveLim.AperturaMinCaldo;
                    if(ValveCounter > Me.ValveLim.AperturaMaxCaldo)
                        ValveCounter = Me.ValveLim.AperturaMaxCaldo;

                    Me.ExpValve_Ctrl = ValveCounter;
#endif
                    TimerExpValvola.TimeOut = 0;
                }
                //if(EngineBox.Compressor_Speed==0 && EngineBox.Compressor_Speed_C2==0)
                if(EngineBox.Inverter_Upper_Out_Freq==0)
                    Me.ExpValve_Ctrl = Me.ValveLim.ValvolaChiusa;		// la chiudo completamente
            }

            if(LastFunctMode == 3)  //ero in OFF-Raffrescamento
            {
                Me.ExpValve_Ctrl = Me.ValveLim.ValvolaChiusa;		// la chiudo completamente
                PID_SetOut(&ValvePID, Me.ValveLim.ValvolaChiusa);		// Inizializzo uscita PID per avere "InitValvolaFreddo"	                    
                Me.ExpValve_Pid_Cooling = Me.ExpValve_Ctrl;
            }

            if(LastFunctMode == 4)  //ero in Raffrescamento
            {
                if(TimerExecPID.TimeOut)
                {
                    TimerExecPID.TimeOut=0;
                    TimerExecPID.Value=0;
                    UpdatePID(&ValvePID);
                    Valve_Pid = PIDOut2Valve(ValvePID.PVal, K_PoMin, K_PoMax, Me.ValveLim.AperturaMinFreddo, Me.ValveLim.AperturaMaxFreddo);

                    //Correzione valvola se ho errore di alta pressione liquido
                    if(EngineBox.Condenser_Press_Abil)
                       Errore = EngineBox.Pressure_LiquidCond - EngineBox.Lim_Max_Press_Correct_Valve;	//Calcolo errore
                    else                                     
                        Errore = EngineBox.Pressione_Liq_P - EngineBox.Lim_Max_Press_Correct_Valve;	//Calcolo errore
                    if(Errore>0)	//Se errore >0
                    {
                        if(Errore>50)	//Se Errore >500 mBar
                            //Incrementa(&Valve_ErrPress, 4, 42);//20%		// Apro valvola fino a MAX per abbassare la pressione													
                            Incrementa(&Valve_ErrPress, K_IncOutValvole*2, Me.ValveLim.ValveNoWorkMaxFreddo);	
                    }
                    else
                    {
                        if(EngineBox.Condenser_Press_Abil)
                            Errore = EngineBox.Lim_Max_Press_Correct_Valve - EngineBox.Pressure_LiquidCond;
                        else                                                            
                            Errore = EngineBox.Lim_Max_Press_Correct_Valve - EngineBox.Pressione_Liq_P;	//Calcolo errore
                        if(Errore>50)	//Se Errore >500 mBar
                            Decrementa(&Valve_ErrPress, K_IncOutValvole*2, Me.ValveLim.ValveNoWorkMinFreddo);		// Chiudo valvola fino a MIN per alzare la pressione
                    }

                    Me.ExpValve_Pid_Cooling = Valve_Pid + Valve_ErrPress;
                    Me.ExpValve_Ctrl = Valve_Pid + Valve_ErrPress;

                    if(Me.ExpValve_Ctrl<Me.ValveLim.AperturaMinFreddo)
                        Me.ExpValve_Ctrl=Me.ValveLim.AperturaMinFreddo;
                    if(Me.ExpValve_Ctrl>Me.ValveLim.AperturaMaxFreddo)
                        Me.ExpValve_Ctrl=Me.ValveLim.AperturaMaxFreddo;
                }
                //if(EngineBox.Compressor_Speed==0 && EngineBox.Compressor_Speed_C2==0)
                if(EngineBox.Inverter_Upper_Out_Freq==0)
                {
                    Me.ExpValve_Ctrl = Me.ValveLim.ValvolaChiusa;		// la chiudo completamente                                
                    PID_SetOut(&ValvePID, Me.ValveLim.ValvolaChiusa);		// Inizializzo uscita PID per avere "InitValvolaFreddo"	                    
                }
            }				
            break;

        case EngineBox_GoFreddo:							// il box motore si sta acccendendo in freddo			
            if((Me.Function_Mode == CoreRaffrescamento) |
                (Me.Function_Mode == CoreSlaveFreddo))		// e la camera chiede riscaldamento
                LastFunctMode = 4;
            else
                LastFunctMode = 3;

            Me.ExpValve_Ctrl = Me.ValveLim.InitValvolaFreddo;
            PID_SetOut(&ValvePID, Me.ValveLim.InitValvolaFreddo);		// Inizializzo uscita PID per avere "InitValvolaFreddo"
            Me.ExpValve_Pid_Cooling = Me.ExpValve_Ctrl;
            BloccaRegolaValveFreddo = 1;					//Se sono in fase di avvio faccio reinit lo stesso
            break;

        case EngineBox_GoCaldo:							// il box motore si sta accendendo in caldo
            if((Me.Function_Mode == CoreRiscaldamento) |
                (Me.Function_Mode == CoreSlaveCaldo))		// e la camera chiede riscaldamento
            {
                LastFunctMode = 2;
    
                BloccaRegolaValveCaldo = 0;
                if(!TrigInitValvola)
                {
                    Me.ExpValve_Ctrl = 0;	//Me.ValveLim.AperturaMinCaldo;
                    TrigInitValvola = 1;
                }

                TimerExpValvola.Time = WaitExpValvolaCaldo;	// inizializzo il timer di regolazione alla velocità caldo
                if((TimerExpValvola.TimeOut == 1))	// Esegue regolazione ogni xx sec??                                                  	
                {	
#if(K_ValveNoPressCondenser==1)
                    Errore = Me.SuperheatRiscaldamento;//PressureGasToTempR410A(EngineBox.Pressione_Liq_P) - EngineBox.Temp_Ritorno;   
                    //DeltaMaxEsc = ((int)((((float)(Me.AperturaMaxCaldo - Me.ValveLim.AperturaMinCaldo))/100.0) *20.0)) + Me.ValveLim.AperturaMinCaldo;
                    DeltaMaxEsc = ((int)((((float)(Me.ValveLim.AperturaMaxCaldo))/100.0) *20.0));

                    if(Errore>(K_SetPoint_SuperHeatRiscaldamento+100))	//200 perchè Me.SuperheatRiscaldamento-100 è setpoint, chiudo un grado prima
                        Incrementa(&Me.ExpValve_Ctrl, K_IncOutValvole, DeltaMaxEsc);		// 20% Apro valvola fino a MAX per alzare la pressione													

                    if(Errore<(K_SetPoint_SuperHeatRiscaldamento+100))                        
                        Decrementa(&Me.ExpValve_Ctrl, K_IncOutValvole, Me.ValveLim.ValvolaChiusa);	//48	// Chiudo valvola fino a 0 per abbassare la pressione
#else
                    //ValveCounter = (int)ValueIn2ValueOut((float)EngineBox.SuperH_HeatCalc, (float)Me.ValveLim.ValvolaChiusa, (float)Me.ValveLim.ValvolaAperta, (float)Me.ValveLim.AperturaMinCaldo, (float)Me.ValveLim.AperturaMaxCaldo);//EngineBox.SuperH_HeatCalc;
                    ValveCounter = EngineBox.SuperH_HeatCalc;
                    if(ValveCounter < Me.ValveLim.AperturaMinCaldo)
                        ValveCounter = Me.ValveLim.AperturaMinCaldo;
                    if(ValveCounter > Me.ValveLim.AperturaMaxCaldo)
                        ValveCounter = Me.ValveLim.AperturaMaxCaldo;

                    Me.ExpValve_Ctrl = ValveCounter;
#endif
                    TimerExpValvola.TimeOut = 0;
                    //ValveCounter = Me.ExpValve_Ctrl;
                }
            }
            else    //Altrimenti la chiudo
            {
                LastFunctMode = 1;

                TimerExpValvola.Time = WaitExpValvolaCaldo;	// inizializzo il timer di regolazione alla velocità caldo
                if(TimerExpValvola.TimeOut == 1)	// Esegue regolazione ogni xx sec??                                                  	
                {	
                    BloccoTimer = 0;
                    LastFunctMode = 1; //OFF-Riscaldamento                                

                    Decrementa(&Me.ExpValve_Ctrl, K_IncOutValvole*2, Me.ValveLim.ValvolaChiusa);
                    ValveCounter = Me.ExpValve_Ctrl;

                    TimerExpValvola.TimeOut = 0;
                }
            }
            break;

        default:	// se nessuna delle precedenti
            Me.ExpValve_Ctrl = Me.ValveLim.ValvolaChiusa;
            PID_SetOut(&ValvePID, Me.ValveLim.ValvolaChiusa);		// Inizializzo uscita PID per avere "InitValvolaFreddo"
            Me.ExpValve_Pid_Cooling = Valve_Pid;
            //PID_SetOut(&ValvePID2, Me.ValveLim.ValvolaChiusa);
            break;
		}
            
        Me.ExpValve_Req = Me.ExpValve_Ctrl + (Me.ExpValve_Err);  //Sommo posizione richiesta di funzionamento + posizione richiesta per errore attivo
        
        if((EngineBox.Compressor_Speed!=0) || (EngineBox.Compressor_Speed_C2!=0))   //Se i motori non sono spenti, correggo limiti minimi e massimi
        {
            if((Me.Function_Mode == CoreRiscaldamento) && (Me.ExpValve_Req<(int)Me.ValveLim.AperturaMinCaldo))  //Se sono in caldo correggo valore a Min caldo
                Me.ExpValve_Req = Me.ValveLim.AperturaMinCaldo;
            
            if((Me.Function_Mode == CoreRaffrescamento) && (Me.ExpValve_Req<(int)Me.ValveLim.AperturaMinFreddo))  //Se sono in caldo correggo valore a Min caldo
                Me.ExpValve_Req = Me.ValveLim.AperturaMinFreddo;
        }
#if (SplitMode != K_I_Am_Frigo) && (SplitMode != K_I_Am_Freezer)
                if((EngineBox.Frozen_On & 0x0002)!=0)
                {
                    TimerOnValveFrozen.Time = 5*60;//EngineBox.TimeOnValveFrozen;
                    TimerOnValveFrozen.Enable = 1;
                    if(TimerOnValveFrozen.TimeOut)
                    {
                        TrigYesValveOnFrozen = !TrigYesValveOnFrozen;
                        TimerOnValveFrozen.Value = 0;
                        TimerOnValveFrozen.TimeOut = 0;
                    }
                    if(TrigYesValveOnFrozen==1 || (((EngineBox.Abil_Defrosting & 0x0008)==0)))
                    {
                        if(Me.ExpValve_Req > Me.ValveLim.ValvolaFrozen)
                            Me.ExpValve_Req = Me.ValveLim.ValvolaFrozen;
                        PID_SetOut(&ValvePID, Me.ExpValve_Req);
                        Me.ExpValve_Pid_Cooling = Me.ExpValve_Req;
                    }
                    else
                    {
                        Me.ExpValve_Req = Me.ValveLim.ValvolaChiusa;
                        PID_SetOut(&ValvePID, Me.ValveLim.ValvolaChiusa);		// Inizializzo uscita PID per avere "InitValvolaFreddo"								
                        Me.ExpValve_Pid_Cooling = Me.ExpValve_Req;                        
                    }
                }
                else
                {
                    TimerOnValveFrozen.Enable = 0;
                    TimerOnValveFrozen.Value = 0;
                    TimerOnValveFrozen.TimeOut = 0;
                    TrigYesValveOnFrozen = 0;
                }
#endif  
        
#if(K_Gas_Valve_Control)
        if(!iDigitOutGas)
        {
            Me.ExpValve_Req = Me.ValveLim.ValvolaChiusa;
            PID_SetOut(&ValvePID, Me.ValveLim.ValvolaChiusa);		// Inizializzo uscita PID per avere "InitValvolaFreddo"								
            Me.ExpValve_Pid_Cooling = Me.ExpValve_Req;                        
        }        
#endif        
        if(Me.ExpValve_Req<(int)Me.ValveLim.ValvolaChiusa)  //Raso valore apertura minima valvola
            Me.ExpValve_Req = Me.ValveLim.ValvolaChiusa;
        if(Me.ExpValve_Req>(int)Me.ValveLim.ValvolaAperta)  //Raso valore apertura massima valvola
            Me.ExpValve_Req = Me.ValveLim.ValvolaAperta;
        
        if((EngineBox.AllValveOpen100p==1)&&(EngineBox.Compressor_Speed==0) && (EngineBox.Compressor_Speed_C2==0))
            Me.ExpValve_Req = Me.ValveLim.ValvolaAperta;
	}
}



void RealFunctionMode(void)
{
    unsigned int RealFuncMode=0;
    
    if(EngineBox.DoubleCompressorOn!=0)
    {
        if((EngineBox.FunctionMode == EngineBox_Caldo) || (EngineBox.FunctionMode_C2 == EngineBox_Caldo))
            RealFuncMode = EngineBox_Caldo;
        else if ((EngineBox.FunctionMode == EngineBox_Freddo) || (EngineBox.FunctionMode_C2 == EngineBox_Freddo))
            RealFuncMode = EngineBox_Freddo;
        else if ((EngineBox.FunctionMode == EngineBox_GoCaldo) || (EngineBox.FunctionMode_C2 == EngineBox_GoCaldo))
            RealFuncMode = EngineBox_GoCaldo;
        else if ((EngineBox.FunctionMode == EngineBox_GoFreddo) || (EngineBox.FunctionMode_C2 == EngineBox_GoFreddo))
            RealFuncMode = EngineBox_GoFreddo;
        else if ((EngineBox.FunctionMode == EngineBox_GoOff) || (EngineBox.FunctionMode_C2 == EngineBox_GoOff))
            RealFuncMode = EngineBox_GoOff;
        else if ((EngineBox.FunctionMode == EngineBox_CriticalFault) && (EngineBox.FunctionMode_C2 == EngineBox_CriticalFault))
            RealFuncMode = EngineBox_CriticalFault;
        else if ((EngineBox.FunctionMode == EngineBox_Off) || (EngineBox.FunctionMode_C2 == EngineBox_Off))
            RealFuncMode = EngineBox_Off;
    }
    else	
        RealFuncMode = 	EngineBox.FunctionMode;
    
    
    EngineBox.RealFuncMode = RealFuncMode;
	    
}


int TempAmbMidSel(void)
{
    int TempAmb;
    if(Me.DefMode==CoreRiscaldamento)
    {
        if(Me.TempAmb_Middle_Heating==K_Temp_Middle_SubSlave)
        {
            if(!Me.I_Am_SubSlave)
            {
                if(EnableMySecBus)
                    TempAmb=Me.TempAmbMiddle;
                else
                    TempAmb=Me.Temperature.Ambient;
            }
            else
            {
                if(!ComunicationLost)
                    TempAmb=Me.TempAmbMiddle;
                else
                    TempAmb=Me.Temperature.Ambient;
            }
        }
        else if (Me.TempAmb_Middle_Heating==K_Temp_Middle_SubSlave_And_Panel)
        {
            if(NetB[0].OnLine || SyxtS[0].OnLine)
            {
                if(EnableMySecBus)
                    TempAmb=(Me.TempAmbMiddle + Me.PanelTempAmb)/2;
                else
                    TempAmb=(Me.Temperature.Ambient + Me.PanelTempAmb)/2;
            }
            else
            {
                if(!Me.I_Am_SubSlave)
                {
                    if(EnableMySecBus)
                        TempAmb=Me.TempAmbMiddle;
                    else
                        TempAmb=Me.Temperature.Ambient;
                }
                else
                {
                    if(!ComunicationLost)
                        TempAmb=Me.TempAmbMiddle;
                    else
                        TempAmb=Me.Temperature.Ambient;
                }
            }
        }
        else if (Me.TempAmb_Middle_Heating==K_Temp_Only_Panel)
        {
            if(NetB[0].OnLine || SyxtS[0].OnLine)
            {
                TempAmb=Me.PanelTempAmb;
            }
            else
            {
                if(!Me.I_Am_SubSlave)
                {
                    if(EnableMySecBus)
                        TempAmb=Me.TempAmbMiddle;
                    else
                        TempAmb=Me.Temperature.Ambient;
                }
                else
                {
                    if(!ComunicationLost)
                        TempAmb=Me.TempAmbMiddle;
                    else
                        TempAmb=Me.Temperature.Ambient;
                }
            }
        }
        else    //K_Temp_Only_Slave
            TempAmb=Me.Temperature.Ambient;
    }
#if(K_AutoTempFanControl==1)
    else if((Me.DefMode==CoreRaffrescamento) || Me.DefMode==CoreVentilazione)
#else
    else if(Me.DefMode==CoreRaffrescamento)
#endif        
   {
        if(Me.TempAmb_Middle_Cooling==K_Temp_Middle_SubSlave)
        {
            if(!Me.I_Am_SubSlave)
            {
                if(EnableMySecBus)
                    TempAmb=Me.TempAmbMiddle;
                else
                    TempAmb=Me.Temperature.Ambient;
            }
            else
            {
                if(!ComunicationLost)
                    TempAmb=Me.TempAmbMiddle;
                else
                    TempAmb=Me.Temperature.Ambient;
            }
        }
        else if (Me.TempAmb_Middle_Cooling==K_Temp_Middle_SubSlave_And_Panel)
        {
            if(NetB[0].OnLine || SyxtS[0].OnLine)
            {
                if(EnableMySecBus)
                    TempAmb=(Me.TempAmbMiddle + Me.PanelTempAmb)/2;
                else
                    TempAmb=(Me.Temperature.Ambient + Me.PanelTempAmb)/2;
            }
            else
            {
                if(!Me.I_Am_SubSlave)
                {
                    if(EnableMySecBus)
                        TempAmb=Me.TempAmbMiddle;
                    else
                        TempAmb=Me.Temperature.Ambient;
                }
                else
                {
                    if(!ComunicationLost)
                        TempAmb=Me.TempAmbMiddle;
                    else
                        TempAmb=Me.Temperature.Ambient;
                }
            }
        }
        else if (Me.TempAmb_Middle_Cooling==K_Temp_Only_Panel)
        {
            if(NetB[0].OnLine || SyxtS[0].OnLine)
            {
                TempAmb=Me.PanelTempAmb;
            }
            else
            {
                if(!Me.I_Am_SubSlave)
                {
                    if(EnableMySecBus)
                        TempAmb=Me.TempAmbMiddle;
                    else
                        TempAmb=Me.Temperature.Ambient;
                }
                else
                {
                    if(!ComunicationLost)
                        TempAmb=Me.TempAmbMiddle;
                    else
                        TempAmb=Me.Temperature.Ambient;
                }
            }
        }
        else    //K_Temp_Only_Slave
            TempAmb=Me.Temperature.Ambient;        
    }
    else
    {        
        //TempAmb=Me.Temperature.Ambient;   
#if(K_OnlyOne_Press_Sensor==1)
        if(!Me.I_Am_SubSlave)
        {
            TempAmb=Me.Temperature.Ambient;
        }
        else
        {
            if(!ComunicationLost)
                TempAmb=Me.TempAmbMiddle;
            else
                TempAmb=0;
        }       
#else
        if(!Me.I_Am_SubSlave)
        {
            if(EnableMySecBus)
                TempAmb=Me.TempAmbMiddle;
            else
                TempAmb=Me.Temperature.Ambient;
        }
        else
        {
            if(!ComunicationLost)
                TempAmb=Me.TempAmbMiddle;
            else
                TempAmb=Me.Temperature.Ambient;
        }
#endif
    }

    if(((Me.DefMode==CoreRiscaldamento && Me.TempAmb_Middle_Heating==K_Temp_Only_Slave) || (Me.DefMode==CoreRaffrescamento && Me.TempAmb_Middle_Cooling==K_Temp_Only_Slave) || (Me.DefMode==CoreVentilazione))&&!Me.I_Am_SubSlave)
    {
        if(EnableMySecBus)
            Me.TempAmbTouchVisual=Me.TempAmbMiddle;
        else
            Me.TempAmbTouchVisual=Me.Temperature.Ambient;
        
    }
    else
    {
            Me.TempAmbTouchVisual = TempAmb;
    }
    return TempAmb;
}

#if(K_Heater_Abil==1)
void HeaterControl(void)
{
    static int OldOutHeater = 0;
    static int BloccoOutHeater = 0;
    
    static int AirOutHot = 0;
    
    if(Me.Temperature.AirOut>6000)
        AirOutHot = 1;
    else if (Me.Temperature.AirOut<5000)
        AirOutHot = 0;

#if(K_Humidity_Correct==1)
    HumidityControl();
    if(((Touch[0].HeaterPwr==1) && (Me.Error.Fan_Fault==0) && (Me.Fan_Tacho>50) && (Me.Pwm_Value>=PWMValue[EngineBox.Set_Fan_Speed_Dehumi])) || ((Me.Fan_Tacho>50) && (Me.Humidity_Correct==1)))
#else
    if((Touch[0].HeaterPwr==1) && (Me.Error.Fan_Fault==0) && (Me.Fan_Tacho>50) && (Me.Pwm_Value>=PWMValue[3]))
#endif
    {
        if(Touch[0].FunctionMode==CoreRiscaldamento)
        {
            if(((Me.TempAmbRealSelection > (Me.SP + Caldo_Ist_SP)) && BloccoOutHeater==0) || AirOutHot==1)
                oDigitOutHeater = 0;
            else if((Me.TempAmbRealSelection < (Me.SP - Caldo_Ist_SP)) && BloccoOutHeater==0)
                oDigitOutHeater = 1;      

        }
#if(K_Humidity_Correct==1)
        if(Touch[0].FunctionMode==CoreRaffrescamento)
        {
            //HumidityControl();
            if(((Me.Humidity_Correct && ((EngineBox.Compressor_Speed>0) || (EngineBox.Compressor_Speed_C2>0))) ||
               (Me.TempAmbRealSelection < (Me.SP - 20))) && BloccoOutHeater==0 && AirOutHot==0)
            {
                oDigitOutHeater = 1;
            }
            else if(BloccoOutHeater==0 || AirOutHot==1)
            {
                oDigitOutHeater = 0;                
            }
        }    
        else
        {
            Me.Humidity_Correct = 0;            
        }
#else
        Me.Humidity_Correct = 0;            
        if(Touch[0].FunctionMode==CoreRaffrescamento)
        {
            if((Me.TempAmbRealSelection < (Me.SP - 100)) && BloccoOutHeater==0 && AirOutHot==0)
                oDigitOutHeater = 1;
            else if(((Me.TempAmbRealSelection > (Me.SP-70)) && BloccoOutHeater==0) || AirOutHot==1)	//Riaccendo 2 decimi di grado prima di SP
                oDigitOutHeater = 0;

        }
#endif
        if(Touch[0].FunctionMode==CoreVentilazione)
        {
            if(((Me.TempAmbRealSelection > (Me.SP + Caldo_Ist_SP)) && BloccoOutHeater==0) || AirOutHot==1)
                oDigitOutHeater = 0;
            else if((Me.TempAmbRealSelection < (Me.SP - Caldo_Ist_SP)) && BloccoOutHeater==0)
                oDigitOutHeater = 1;      
        }
        if(Touch[0].FunctionMode==CoreOff)
        {
            oDigitOutHeater = 0;
        }            
    }
    else
    {
        oDigitOutHeater = 0;
        Me.Humidity_Correct = 0;
    }
    
    TimerHeaterFresh.Enable = iDigitOutHeater==0 && Me.HeaterFresh==1;
    
    if(iDigitOutHeater==1)
        Me.HeaterFresh = 1;
    
    if(!TimerHeaterFresh.Enable)
    {
        TimerHeaterFresh.Value = 0;
        TimerHeaterFresh.TimeOut = 0;
    }
    if(TimerHeaterFresh.TimeOut)
    {
        TimerHeaterFresh.Value = 0;
        TimerHeaterFresh.TimeOut = 0;
        Me.HeaterFresh = 0;        
    }

    if(iDigitOutHeater!=OldOutHeater)
    {
        OldOutHeater = iDigitOutHeater;
        BloccoOutHeater = 1;
    }
    if(BloccoOutHeater>0)
    {
        BloccoOutHeater++;
        if(BloccoOutHeater>120)//Tempo di ciclo (500ms) x4 = 60 sec.
            BloccoOutHeater = 0;
    } 

#if(K_Double_Heater_Abil==1)    
    if(iDigitOutHeater && (Me.TempAmbRealSelection < (Me.SP-300)))
       oDigitOutHeater2 = 1;
    else if (!iDigitOutHeater || (Me.TempAmbRealSelection > (Me.SP-100)))
       oDigitOutHeater2 = 0;
#endif           
}
#endif
//Ciclo di defrosting unità
#if((SplitMode==K_I_Am_Frigo) || (SplitMode==K_I_Am_Freezer))
void DefrostingCycle(void)
{
    TimerDefrostingCycleWork.Time = (long)(EngineBox.Defrosting_Step_Time*3600L);
    TimerDefrostingCycleCheck.Time = EngineBox.Defrosting_Time*60;
    TimerDefrostingCycleStop.Time = EngineBox.Dripping_Time*60;
#if(SplitMode==K_I_Am_Freezer)
    TimerDefrostingCycleWork.Enable = ((EngineBox.Abil_Defrosting & 0x0001)!=0) && (Me.Function_Mode == CoreRaffrescamento) && !Me.Error.Battery_Defrost;
#elif(SplitMode==K_I_Am_Frigo)     
    TimerDefrostingCycleWork.Enable = ((EngineBox.Abil_Defrosting & 0x0002)!=0) && (Me.Function_Mode == CoreRaffrescamento) && !Me.Error.Battery_Defrost;
#endif
#if(SplitMode==K_I_Am_Freezer)
    if(((EngineBox.Abil_Defrosting & 0x0001) == 0) || (Touch[0].FunctionMode != CoreRaffrescamento))
#elif(SplitMode==K_I_Am_Frigo)     
    if(((EngineBox.Abil_Defrosting & 0x0002) == 0) || (Touch[0].FunctionMode != CoreRaffrescamento))
#endif
    {
        TimerDefrostingCycleWork.TimeOut = 0;
        TimerDefrostingCycleWork.Value = 0;
    }

    if(TimerDefrostingCycleWork.TimeOut || (((EngineBox.Abil_Defrosting & 0x0004)!=0) && (Touch[0].Script_Split_Pwr==1) && !Me.Error.Battery_Defrost))
    {
        Me.Error.Battery_Defrost = 1;
        TimerDefrostingCycleWork.Value = 0;
        TimerDefrostingCycleWork.TimeOut = 0;
    }
       
    TimerDefrostingCycleCheck.Enable = Me.Error.Battery_Defrost && !Me.Error.Battery_Drip;
    
    if(!TimerDefrostingCycleCheck.Enable)
    {
        TimerDefrostingCycleCheck.TimeOut=0;
        TimerDefrostingCycleCheck.Value=0;
    }
    
    if(TimerDefrostingCycleCheck.TimeOut || (((Me.Temperature.Liquid>EngineBox.Defrosting_Temp_Set) || (Me.Temperature.Gas>EngineBox.Defrosting_Temp_Set))&&Me.Error.Battery_Defrost&&!Me.Error.Battery_Drip))
    {
        Me.Error.Battery_Drip = 1;
        TimerDefrostingCycleCheck.TimeOut=0;
        TimerDefrostingCycleCheck.Value=0;
    }

    TimerDefrostingCycleStop.Enable = Me.Error.Battery_Drip;
    
    if(!TimerDefrostingCycleStop.Enable)
    {
        TimerDefrostingCycleStop.TimeOut=0;
        TimerDefrostingCycleStop.Value=0;
    }
    
    if(TimerDefrostingCycleStop.TimeOut)
    {
        Me.Error.Battery_Defrost = 0;
        Me.Error.Battery_Drip = 0;

        TimerDefrostingCycleStop.Enable = 0;        
        TimerDefrostingCycleStop.TimeOut=0;
        TimerDefrostingCycleStop.Value=0;

        TimerDefrostingCycleWork.Enable = 0;        
        TimerDefrostingCycleWork.TimeOut = 0;
        TimerDefrostingCycleWork.Value = 0;        

        TimerDefrostingCycleCheck.Enable = 0;
        TimerDefrostingCycleCheck.TimeOut=0;
        TimerDefrostingCycleCheck.Value=0;
    }

    oDigitOutDefros = Me.Error.Battery_Defrost && !Me.Error.Battery_Drip && (Me.ExpValve_Act==K_ValvolaChiusa);
    
    Me.OutDefrosPower = K_Defr_Res_Power*(iDigitOutDefros==1);
    Me.OutDefrosCurrent = (unsigned int)((float)K_Defr_Res_Power/(float)K_Defr_Res_Voltage);
}
#else
void DefrostingCycle(void)
{
    if((((Me.Temperature.Liquid<K_DefrostingTempLiqTrig) && !Me.Temperature.Error.Bit.Liquid) || ((Me.Temperature.Gas<K_DefrostingTempGasTrig) && !Me.Temperature.Error.Bit.Gas)) && Me.Error.Battery_Defrost==0 && Me.Function_Mode==CoreRaffrescamento)
        TimerDefrostingCycleWork.Enable = 1;
    if(EngineBox.DoubleCompressorOn!=0)
    {
        if((EngineBox.Compressor_Speed==0) && (EngineBox.Compressor_Speed_C2==0))
            TimerDefrostingCycleWork.Enable = 0;
    }
    else
    {
        if(EngineBox.Compressor_Speed==0)
            TimerDefrostingCycleWork.Enable = 0;    
    }
    
    TimerDefrostingCycleCheck.Enable = TimerDefrostingCycleWork.Enable && Me.Temperature.Liquid>K_DefrostingTempLiqOk && Me.Temperature.Gas>K_DefrostingTempGasOk;
    
    if(!TimerDefrostingCycleWork.Enable)
    {
        TimerDefrostingCycleWork.TimeOut=0;
        TimerDefrostingCycleWork.Value=0;
    }

    if(TimerDefrostingCycleCheck.TimeOut)
    {
        TimerDefrostingCycleCheck.TimeOut=0;
        TimerDefrostingCycleCheck.Value=0;
        TimerDefrostingCycleWork.Enable = 0;
    }
    
    if(!TimerDefrostingCycleCheck.Enable)
    {
        TimerDefrostingCycleCheck.TimeOut=0;
        TimerDefrostingCycleCheck.Value=0;
    }
    
    if(TimerDefrostingCycleWork.TimeOut)
    {
        TimerDefrostingCycleWork.Enable = 0;        
        TimerDefrostingCycleWork.TimeOut = 0;
        TimerDefrostingCycleWork.Value = 0;        
        
        Me.Error.Battery_Defrost = 1;
    }
    
    TimerDefrostingCycleStop.Enable = Me.Error.Battery_Defrost == 1 && Me.Temperature.Liquid>K_DefrostingTempLiqOk && Me.Temperature.Gas>K_DefrostingTempGasOk;
    
    if(!TimerDefrostingCycleStop.Enable)
    {
        TimerDefrostingCycleStop.TimeOut=0;
        TimerDefrostingCycleStop.Value=0;
    }
    
    if(TimerDefrostingCycleStop.TimeOut)
    {
        Me.Error.Battery_Defrost = 0;

        TimerDefrostingCycleStop.Enable = 0;        
        TimerDefrostingCycleStop.TimeOut=0;
        TimerDefrostingCycleStop.Value=0;

        TimerDefrostingCycleWork.Enable = 0;        
        TimerDefrostingCycleWork.TimeOut = 0;
        TimerDefrostingCycleWork.Value = 0;        

        TimerDefrostingCycleCheck.Enable = 0;
        TimerDefrostingCycleCheck.TimeOut=0;
        TimerDefrostingCycleCheck.Value=0;
    }    
}
#endif

//Ciclo di allarme presenza acqua vaschetta unità
void FloodFanCoilAlarmCycle(void)
{
    TimerFloodFanCoil.Enable = !iDigiIn1 && (Me.Error.Flood_Alarm == 0);
    
    if(!TimerFloodFanCoil.Enable)
    {
        TimerFloodFanCoil.TimeOut = 0;
        TimerFloodFanCoil.Value = 0;        
    }    
    
    if(TimerFloodFanCoil.TimeOut)
	{
		Me.Error.Flood_Alarm = 1;
        TimerFloodFanCoil.TimeOut = 0;
        TimerFloodFanCoil.Value = 0;        
	}
	if(iDigiIn1 && (Me.Error.Flood_Alarm == 1))
    {
		Me.Error.Flood_Alarm = 0;    
	}
}

//Ciclo di allarme perdita gas
void GasLeaksAlarmCycle(void)
{
    TimerGasLeaks.Enable = !iDigiIn3 && (Me.Error.GasLeaks == 0);
    
    if(!TimerGasLeaks.Enable)
    {
        TimerGasLeaks.TimeOut = 0;
        TimerGasLeaks.Value = 0;        
    }    
    
    if(TimerGasLeaks.TimeOut)
	{
		Me.Error.GasLeaks = 1;
        TimerGasLeaks.TimeOut = 0;
        TimerGasLeaks.Value = 0;        
	}
	if(iDigiIn3 && (Me.Error.GasLeaks == 1))
    {
		Me.Error.GasLeaks = 0;    
	}
}

//Ciclo di allarme serrande (se presenti)
#if(K_Dampers_Selection>0)
void DampersAlarm(void)
{
    TimerDampersAlarm.Enable = (oDigitDampers1 && !iDigiIn1) || (!oDigitDampers1 && iDigiIn1);
#if(K_Dampers_Selection>1)
    TimerDampersAlarm.Enable = TimerDampersAlarm.Enable || (oDigitDampers2 && !iDigiIn2) || (!oDigitDampers2 && iDigiIn2);
#endif
    if(!TimerDampersAlarm.Enable)
    {
        TimerDampersAlarm.Value = 0;
        TimerDampersAlarm.TimeOut = 0;
        Me.Error.Dampers = 0;
    }
    if(TimerDampersAlarm.TimeOut)
    {
        Me.Error.Dampers = 1;
        TimerDampersAlarm.Value = 0;
        TimerDampersAlarm.TimeOut = 0;

    }        
        
}
#endif
//-----------------------------------------------------------------------------------------------
// Converte il valore di ingresso "Vin" con i range min e max nel valore di uscita Vout con i relativi
// range min e max
//-----------------------------------------------------------------------------------------------
float ValueIn2ValueOut(float Vin, float VinMin, float VinMax, float VoutMin, float VoutMax)
{
	float fDVin;
	float fDVout;
	float fK;
	float Vout;
    float VinCalc;
	
    VinCalc = Vin;
    if(VinMax-VinMin>0)
    {
        if(VinCalc < VinMin)
            VinCalc = VinMin;
        if(VinCalc > VinMax)
            VinCalc = VinMax;        
    }    
    else
    {
        if(VinCalc < VinMax)
            VinCalc = VinMax;
        if(VinCalc > VinMin)
            VinCalc = VinMin;        
    }        
    
	fDVin = VinMax - VinMin;
	fDVout = VoutMax - VoutMin;	
	
	fK = fDVin / fDVout;
	
	Vout = ((VinCalc - VinMin) / fK) + VoutMin;

	return Vout;
}

/*
unsigned int ValueIn2ValueOut(unsigned int Vin, unsigned int VinMin, unsigned int VinMax, unsigned int VoutMin, unsigned int VoutMax)
{
	float fDVin;
	float fDVout;
	float fK;
	int Vout;
	
	fDVin = (float)VinMax - (float)VinMin;
	fDVout = (float)VoutMax - (float)VoutMin;	
	
	fK = fDVin / fDVout;
	
	Vout = (int)(((float)Vin - (float)VinMin) / fK) + (float)VoutMin;

	if(Vout > VoutMax)
		Vout = VoutMax;
	if(Vout < VoutMin)
		Vout = VoutMin;	
	
	return Vout;
}

int ValueIn2ValueOut(int Vin, int VinMin, int VinMax, int VoutMin, int VoutMax)
{
	float fDVin;
	float fDVout;
	float fK;
	int Vout;
	
	fDVin = (float)VinMax - (float)VinMin;
	fDVout = (float)VoutMax - (float)VoutMin;	
	
	fK = fDVin / fDVout;
	
	Vout = (int)(((float)Vin - (float)VinMin) / fK) + (float)VoutMin;

	if(Vout > VoutMax)
		Vout = VoutMax;
	if(Vout < VoutMin)
		Vout = VoutMin;	
	
	return Vout;
}
 */

int ValueToPercent(int Value, int MaxValue)
{
	int retval;

	retval = (unsigned int)( ((float)Value / (float)MaxValue) * 100.0); 
	return retval; 
}

#if(K_Gas_Valve_Control==1)
void GasValveControl(void)
{
    if((EngineBox.Pressione_Gas_G > K_Press_Close_Valve_Control) || ((Me.Function_Mode!=CoreRaffrescamento) && (Me.ExpValve_Act==K_ValvolaChiusa)))
        oDigitOutGas = 0;
    else if((EngineBox.Pressione_Gas_G <= Me.Evap_Press) && (Me.Function_Mode==CoreRaffrescamento))
        oDigitOutGas = 1;      
}
#endif

#if (SplitMode == K_I_Am_Frigo) || (SplitMode == K_I_Am_Freezer) || (K_ErrorSPEnable==1)
void CheckErrorSPTempFault(void)
{
    TimerTemperatureFault.Enable = (Me.Temperature.Ambient>(Me.SP+K_DeltaTemperatureFault))&&(Touch[0].Script_Split_Pwr==1)&&(EngineBox.SystemDisable==0)&&(Me.Error.SPTemperatureFault==0);
	
	if(TimerTemperatureFault.Enable==0)
	{
		TimerTemperatureFault.Value = 0;
		TimerTemperatureFault.TimeOut = 0;
	}
	if(TimerTemperatureFault.TimeOut)
	{
		Me.Error.SPTemperatureFault = 1;
		TimerTemperatureFault.Value = 0;
		TimerTemperatureFault.TimeOut = 0;
	}
	
	if(Me.Temperature.Ambient<(Me.SP+K_DeltaRestartTemperatureFault))
		Me.Error.SPTemperatureFault = 0;
}
#endif

void CheckErrorTemperatureProbe(void)
{
    int Time_Ambient_1;
    //int Time_Ambient_2;
    if(Me.Function_Mode == CoreRaffrescamento)
    {
        if(Me.Temperature.Ambient>(Me.SP+200))
            Time_Ambient_1 = K_MaxCntTempElapsedTimeError_Ambient_HI;
        else
            Time_Ambient_1 = K_MaxCntTempElapsedTimeError_Ambient_LO;
    }
    else
    {
        if(Me.Temperature.Ambient<(Me.SP-200))
            Time_Ambient_1 = K_MaxCntTempElapsedTimeError_Ambient_HI;
        else
            Time_Ambient_1 = K_MaxCntTempElapsedTimeError_Ambient_LO;        
    }
    if(TimerCheckProbe.TimeOut)
    {   
        Me.Temperature.Freeze.Gas = CheckProbeFreeze(Me.Temperature.Gas, &Me.Temperature.PreviusVal.Gas, &Me.Temperature.CntError.Gas, K_MaxCntTempElapsedTimeError_Gas);
        Me.Temperature.Freeze.Liquid = CheckProbeFreeze(Me.Temperature.Liquid, &Me.Temperature.PreviusVal.Liquid, &Me.Temperature.CntError.Liquid, K_MaxCntTempElapsedTimeError_Liquid);
        Me.Temperature.Freeze.AirOut = CheckProbeFreeze(Me.Temperature.AirOut, &Me.Temperature.PreviusVal.AirOut, &Me.Temperature.CntError.AirOut, K_MaxCntTempElapsedTimeError_AirOut);
        Me.Temperature.Freeze.Ambient = CheckProbeFreeze(Me.Temperature.Ambient, &Me.Temperature.PreviusVal.Ambient, &Me.Temperature.CntError.Ambient, Time_Ambient_1);
        TimerCheckProbe.Value = 0;
        TimerCheckProbe.TimeOut = 0;        
    }
    
    Me.Temperature.Error.Bit.Gas = (Me.Temperature.Gas<=K_Gas_Probe_Lo) || (Me.Temperature.Gas>=K_Gas_Probe_Hi) || (Me.Temperature.Freeze.Gas && EngineBox.TestAllAbil.Split_Probe_Error);
    Me.Temperature.Error.Bit.Liquid = (Me.Temperature.Liquid<=K_Liquid_Probe_Lo) || (Me.Temperature.Liquid>=K_Liquid_Probe_Hi) || (Me.Temperature.Freeze.Liquid && EngineBox.TestAllAbil.Split_Probe_Error);
    Me.Temperature.Error.Bit.AirOut = (Me.Temperature.AirOut<=K_AirOut_Probe_Lo) || (Me.Temperature.AirOut>=K_AirOut_Probe_Hi) || (Me.Temperature.Freeze.AirOut && EngineBox.TestAllAbil.Split_Probe_Error);
#if (K_ADAuxChannelEnable==0)
    Me.Temperature.Error.Bit.Ambient = (Me.Temperature.Ambient<=K_Ambient_Probe_Lo) || /*(Me.Temperature.Ambient>=K_Ambient_Probe_Hi) || */(Me.Temperature.Freeze.Ambient && EngineBox.TestAllAbil.Split_Probe_Error);
#endif
                
    TimerTemperatureProbeError.Enable = (Me.Temperature.Error.Bit.Gas || Me.Temperature.Error.Bit.Liquid || Me.Temperature.Error.Bit.AirOut || Me.Temperature.Error.Bit.Ambient) && !Me.Error.TemperatureProbe;

    if(!TimerTemperatureProbeError.Enable)
    {
        TimerTemperatureProbeError.Value = 0;
        TimerTemperatureProbeError.TimeOut = 0;
    }
    
    if(TimerTemperatureProbeError.TimeOut)
    {
        TimerTemperatureProbeError.Value = 0;
        TimerTemperatureProbeError.TimeOut = 0;
        Me.Error.TemperatureProbe = 1;
    }
    
    if(!Me.Temperature.Error.Bit.Gas && !Me.Temperature.Error.Bit.Liquid && !Me.Temperature.Error.Bit.AirOut && !Me.Temperature.Error.Bit.Ambient)
    {
        Me.Error.TemperatureProbe = 0;        
    }
}

int CheckProbeFreeze(int Value, int * PrevVal, int * CntErr, unsigned int MaxCnt)
{
    if(Value > ((*PrevVal) + K_DeltaTempElapsedTimeError) ||
      (Value < ((*PrevVal) - K_DeltaTempElapsedTimeError)))
    {
        (*PrevVal) = Value;
        (*CntErr)=0;
    }
    else
    {
        if(((*CntErr)<MaxCnt) && ((Me.Function_Mode == CoreRiscaldamento) || (Me.Function_Mode == CoreRaffrescamento)) && ((EngineBox.Compressor_Speed>0) || (EngineBox.Compressor_Speed_C2>0)))
            (*CntErr)++;       
    } 
    if((*CntErr)>=MaxCnt)
        return 1;
    else
        return 0;
}

void Thermal_Power_Output_Calculation (void)
{
    float AirDensity=0;
    float Density_0C=1.2930;
    float Density_10C=1.2470;
    float Density_15C=1.2250;
    float Density_20C=1.2050;
    float Density_30C=1.1650;
    float Density_40C=1.1270;
    float Density_60C=1.0600;
    float Density_80C=1.0000;

    float AirCalSpec=0;
    float CalSpec_0C=1.0037;
    float CalSpec_10C=1.0041;
    float CalSpec_15C=1.0043;
    float CalSpec_20C=1.0045;
    float CalSpec_30C=1.0050;
    float CalSpec_40C=1.0055;
    float CalSpec_60C=1.0068;
    float CalSpec_80C=1.0084;

    float AirMass=0;
    
    float DeltaT=0;
    
    float PowerJ=0;
    float PowerOutput=0;

    if(Me.Temperature.Ambient<0)
    {
        AirDensity = Density_0C;
        AirCalSpec = CalSpec_0C;
    }
    else if(Me.Temperature.Ambient<=1000)
    {
        AirDensity = ValueIn2ValueOut((float)Me.Temperature.Ambient,0.0,1000.0,Density_0C,Density_10C);
        AirCalSpec = ValueIn2ValueOut((float)Me.Temperature.Ambient,0.0,1000.0,CalSpec_0C,CalSpec_10C);
    }
    else if(Me.Temperature.Ambient<=1500)
    {
        AirDensity = ValueIn2ValueOut((float)Me.Temperature.Ambient,1000.0,1500.0,Density_10C,Density_15C);
        AirCalSpec = ValueIn2ValueOut((float)Me.Temperature.Ambient,1000.0,1500.0,CalSpec_10C,CalSpec_15C);
    }
    else if(Me.Temperature.Ambient<=2000)
    {
        AirDensity = ValueIn2ValueOut((float)Me.Temperature.Ambient,1500.0,2000.0,Density_15C,Density_20C);
        AirCalSpec = ValueIn2ValueOut((float)Me.Temperature.Ambient,1500.0,2000.0,CalSpec_15C,CalSpec_20C);
    }
    else if(Me.Temperature.Ambient<=3000)
    {
        AirDensity = ValueIn2ValueOut((float)Me.Temperature.Ambient,2000.0,3000.0,Density_20C,Density_30C);
        AirCalSpec = ValueIn2ValueOut((float)Me.Temperature.Ambient,2000.0,3000.0,CalSpec_20C,CalSpec_30C);
    }
    else if(Me.Temperature.Ambient<=4000)
    {
        AirDensity = ValueIn2ValueOut((float)Me.Temperature.Ambient,3000.0,4000.0,Density_30C,Density_40C);
        AirCalSpec = ValueIn2ValueOut((float)Me.Temperature.Ambient,3000.0,4000.0,CalSpec_30C,CalSpec_40C);
    }
    else if(Me.Temperature.Ambient<=6000)
    {
        AirDensity = ValueIn2ValueOut((float)Me.Temperature.Ambient,4000.0,6000.0,Density_40C,Density_60C);
        AirCalSpec = ValueIn2ValueOut((float)Me.Temperature.Ambient,4000.0,6000.0,CalSpec_40C,CalSpec_60C);
    }
    else if(Me.Temperature.Ambient<=8000)
    {
        AirDensity = ValueIn2ValueOut((float)Me.Temperature.Ambient,6000.0,8000.0,Density_60C,Density_80C);
        AirCalSpec = ValueIn2ValueOut((float)Me.Temperature.Ambient,6000.0,8000.0,CalSpec_60C,CalSpec_80C);
    }
    else
    {
        AirDensity = Density_80C;
        AirCalSpec = CalSpec_80C;
    }
    
    AirMass = AirDensity * (float)Me.Fan_M3h;
    if(Me.Function_Mode==CoreRiscaldamento)    
        DeltaT = (float)Me.Temperature.AirOut - (float)Me.Temperature.Ambient;
    else if (Me.Function_Mode==CoreRaffrescamento)
        DeltaT = (float)Me.Temperature.Ambient - (float)Me.Temperature.AirOut; //Al posto di fare il modulo, giro il deltaper avere risultato positivo

    PowerJ = (AirCalSpec*AirMass*DeltaT)/100.0;
    
    PowerOutput = PowerJ/3600.0;
    
    if(PowerOutput<0.0)
       PowerOutput=0.0;
    
    if(Me.Function_Mode==CoreRiscaldamento || Me.Function_Mode==CoreRaffrescamento)
    {
        Me.AirPowerOutput = (unsigned int)(PowerOutput*100.0);
        Me.AirPowerBTU = (unsigned int)(PowerOutput*341.2142);    
    }
    else
    {
        Me.AirPowerOutput = 0;
        Me.AirPowerBTU = 0;    
    }
    
    Filter(&FilterAirPowerOutput, Me.AirPowerOutput, K_Campioni_FilterAirPower);
    Filter(&FilterAirPowerBTU, Me.AirPowerBTU, K_Campioni_FilterAirPower);
    
}

#if(K_LoopPumpEnable==1)
#if(K_LoopDoublePumpEnable==1)    
void PumpLoopControl(void)
{
    static int BloccoPumpLoop = 0;
    static int OldPumpLoop = 0;
    static int PumpLoop = 0;
    

    TimerSelectPumpWork.Time = 12L*60L*60L;//Moltiplico *60 x 2 volte per portare in ore (12 ore)
    TimerSelectPumpWork.Enable = iDigitPumpLoop1 || iDigitPumpLoop2;//(Me.Function_Mode || Room[1].Function_Mode) && ((EngineBox.Abil_Defrosting & 0x0001)!=0);

    if(TimerSelectPumpWork.TimeOut)
    {
        TimerSelectPumpWork.TimeOut = 0;
        TimerSelectPumpWork.Value = 0;
        if(Me.SelectWorkingPump==0)
            Me.SelectWorkingPump=1;
        else
            Me.SelectWorkingPump=0;                 
    }

    if(Me.Error.Flow)
        Me.SelectWorkingPump=1;
    else if(Me.Error.Flow2)
        Me.SelectWorkingPump=0;  
    

    Me.Pump_Work = iDigitPumpLoop1 || iDigitPumpLoop2;    

        
    if((BloccoPumpLoop==0 && Touch[0].FunctionMode==0) || (Me.Error.Flow==1 && Me.Error.Flow2==1))
        PumpLoop = 0;
    else if(BloccoPumpLoop==0 && Touch[0].FunctionMode!=0)
        PumpLoop = 1;

    if(PumpLoop!=OldPumpLoop)
    {
        OldPumpLoop = PumpLoop;
        BloccoPumpLoop = 1;
    }
    if(BloccoPumpLoop>0)
    {
        BloccoPumpLoop++;
        if(BloccoPumpLoop>60)//Tempo di ciclo (500ms) x2 = 30 sec.
            BloccoPumpLoop = 0;
    } 
    
    oDigitPumpLoop1 = PumpLoop && Me.SelectWorkingPump==0 && !Me.Error.Flow;
    oDigitPumpLoop2 = PumpLoop && Me.SelectWorkingPump==1 && !Me.Error.Flow2;
    
    FlowLoopControl();
}

void FlowLoopControl(void)
{
	TimerErrFlow.Enable = !iDigiIn1 && (iDigitPumpLoop1 || iDigitPumpLoop2);

    if(!TimerErrFlow.Enable)
	{	
		TimerErrFlow.Value = 0;
		TimerErrFlow.TimeOut = 0;
	} 	
    if(TimerErrFlow.TimeOut)
    {
        if(iDigitPumpLoop1==1)
            Me.Error.Flow = 1;
        if(iDigitPumpLoop2==1)
            Me.Error.Flow2 = 1;
        
		TimerErrFlow.Value = 0;
		TimerErrFlow.TimeOut = 0;        
	}  
}
#else
void PumpLoopControl(void)
{
    static int BloccoPumpLoop = 0;
    static int OldPumpLoop = 0;

    Me.Error.Flow2 = 0;

    Me.Pump_Work = iDigitPumpLoop1;    

        
    if((BloccoPumpLoop==0 && Touch[0].FunctionMode==0) || Me.Error.Flow==1)
        oDigitPumpLoop1 = 0;
    else if(BloccoPumpLoop==0 && Touch[0].FunctionMode!=0)
        oDigitPumpLoop1 = 1;

    if(oDigitPumpLoop1!=OldPumpLoop)
    {
        OldPumpLoop = oDigitPumpLoop1;
        BloccoPumpLoop = 1;
    }
    if(BloccoPumpLoop>0)
    {
        BloccoPumpLoop++;
        if(BloccoPumpLoop>60)//Tempo di ciclo (500ms) x2 = 30 sec.
            BloccoPumpLoop = 0;
    } 

    FlowLoopControl();
}

void FlowLoopControl(void)
{
	TimerErrFlow.Enable = !iDigiIn1 && iDigitPumpLoop1;

    if(!TimerErrFlow.Enable)
	{	
		TimerErrFlow.Value = 0;
		TimerErrFlow.TimeOut = 0;
	} 	
    if(TimerErrFlow.TimeOut)
    {
		Me.Error.Flow = 1;
		TimerErrFlow.Value = 0;
		TimerErrFlow.TimeOut = 0;        
	}
    
}
#endif
#endif

#if(K_Humidity_Correct==1)
void HumidityControl(void)
{
/*
Temperatura pari a 18°C, l?umidità ideale dovrebbe essere compresa tra il 60 ed il 70%;
Temperatura pari a 19°C, l?umidità ideale dovrebbe essere compresa tra il 50 ed il 70%;
Temperatura pari a 20°C, l?umidità ideale dovrebbe essere compresa tra il 40 ed il 70%;
Temperatura pari a 22°C, l?umidità ideale dovrebbe essere compresa tra il 40 ed il 60%;
Temperatura oltre  24°C, è accettabile un?umidità compresa tra il 30 ed il 40%.
*/
    static int Humi_Hist_On = 8;
    static int Humi_Hist_Off = 2;
    static int TempOff = 0;
    static int TempOn = 0;
    TempOff = (Me.SP+(EngineBox.Set_Delta_Temp_Min_Dehumi+50));
    TempOn = (Me.SP+EngineBox.Set_Delta_Temp_Min_Dehumi);
    
    if((Me.TempAmbRealSelection>TempOff) || (Me.Humidity<=(EngineBox.Set_Humi+Humi_Hist_Off)))
        Me.Humidity_Correct = 0;
    else if((Me.TempAmbRealSelection<=TempOn)&&(Me.Humidity>=(EngineBox.Set_Humi+Humi_Hist_On)))
        Me.Humidity_Correct = 1;   
}
#endif

