//----------------------------------------------------------------------------------
//	Progect name:	Test 24FJ64A002
//	Device:			PIC24FJ64A002 @ 20Mhz
//	Auitor:			TDM Team
//	Date:			23/01/2011
//	Description:	Test del pic 24FJ64A002
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
#include "ConfigDevice.h"
#include "DefinePeriferiche.h"
#include "ADC.h"
#include "Timer.h"
#include "Usart.h"
#include "delay.h"
#include "I2C.h"
#include "EEPROM.h"
#include "PWM.h"
#include "Valvola_PassoPasso.h"
#include "Driver_Comunicazione.h"
#include "Driver_ComunicazioneSec.h"
#include "Driver_ModBusSec.h"
#include "ProtocolloComunicazione.h"
#include "ProtocolloComunicazioneSec.h"
#include "ProtocolloModBusSec.h"
#include "Driver_ModBus.h"
#include "ProtocolloModBus.h"
#include "Core.h"
#include "PID.h"
#include "FWSelection.h"
#include "i2c_LCD_Driver.h"
#include "LCD_ScreenInfo.h"
#include "Interrupt.h"

#ifdef UART3_ON  
#include "ServiceComunication.h"
#endif

//----------------------------------------------------------------------------------
//	DEFINE
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//	Variabili globali
//----------------------------------------------------------------------------------
VariabiliGlobaliADC();
VariabiliGlobaliDriverComunicazione();
VariabiliGlobaliDriverComunicazioneSec();
VariabiliGlobaliDriverModBusSec();
VariabiliGlobaliProtocolloComunicazione();
VariabiliGlobaliProtocolloComunicazioneSec();
VariabiliGlobaliProtocolloModBusSec();
VariabiliGlobaliEEPROM();
VariabiliGlobaliPWM();
VariabiliGlobaliValvolaExp();
VariabiliGlobaliCore();
VariabiliGlobaliPID();
VariabiliGlobaliDriverModBus();
VariabiliGlobaliProtocolloModBus();
#ifdef UART3_ON
GlobalVarServiceComunication();
#endif

// timer	
volatile TypTimer TmrLastRequest;
volatile TypTimer TimerStartUp;				// Timer StartUp
volatile TypTimer RefreshNetBRead;
volatile TypTimer TimerSystemCycle;			// Timer per il calcolo del tempo di ciclo programma
volatile TypTimer TimerComunicationError;   // Timer per gestione counter errori di comunicazione
volatile TypTimer TimerBusMergeMode;        // Timer per gestione Timeout funzione BUSMerge
volatile TypTimer TimerToggleComunication_Sec;        // Timer per gestione inibizione UART2
volatile TypTimer TimerCheckProbe;


//TypRTC	MyRTC;

unsigned RTC_Flag = 1;
//volatile int SecDivider = 0;                    // divisore per base tempo di 1 secondo
//volatile int Elapsed = 0;						// divisore per base tempo di 1 secondo
int Led_Divider = 0;                            // divisore per il lampeggio del led
unsigned char CntResumeTouch = 0;				// contatore per il resume dei touch
unsigned char CntResumeNetB = 0;				// contatore per il resume dei NetB
unsigned char CntResumeSyxtS = 0;				// contatore per il resume dei NetB
unsigned char CntResumeSplit_Sec = 0;			// contatore per il resume degli split sul sec bus
unsigned ComunicationLost;                      // flag di comunicazione persa
unsigned char CurUsedProtocolId = 0;            // Id del protocollo correntemente utilizzato
unsigned char ScanBoardCounter_Sec = 1;         // utilizzato per il check dei moduli eventualmente collegati a caldo
unsigned char ScanTouchCounter = 0;             // utilizzato per il check dei Touch eventualmente collegati a caldo
unsigned char ScanNetBCounter = 0;              // utilizzato per il check dei NetB eventualmente collegati a caldo
unsigned char ScanSyxtSCounter = 0;              // utilizzato per il check dei NetB eventualmente collegati a caldo



volatile int TimeOutInterCharComErr = 0;

volatile int ModCntComErr = 0;			
volatile int ModTimeOutComErr = 0;		
volatile int ModTimeOutInterCharComErr = 0;
volatile int ModCRC_ComErr = 0;

volatile int ModCntComErrSec = 0;			
volatile int ModTimeOutComErrSec = 0;		
volatile int ModTimeOutInterCharComErrSec = 0;
volatile int ModCRC_ComErrSec = 0;

volatile int CntComErrSec = 0;			
volatile int TimeOutComErrSec = 0;		
volatile int TimeOutInterCharComErrSec = 0;	
volatile int CRC_ComErrSec = 0;

unsigned ReplyResult;

unsigned int ForceU1MyBUS = 0;      // Flag per gestire la forzatura del solo protocollo MyBUS sul canale primario (UART1) alla prima ricezione MyBUS valida

int SchedSearchDevice=0;

int nCycle=0;

volatile int SaveNumber=0;

//----------------------------------------------------------------------------------
//	Prototipi 
//----------------------------------------------------------------------------------
void Init(void);
void Hw_Test(void);
void Hw_Test_PLX_DAQ(void);
void InitADConversion(void);
void BootLoaderAutoReset(void);
void GlobalIntReadBus(void);
void SendDataToTouch(void);
void CheckIO(void);
void ValveOpenCloseCheck(unsigned int TimeOpen, unsigned int TimeClosed);
void CheckFanSpeed(void);



void ErrorReport(void)
{				
    
//-----------------------------------------------------------------------
// gestione segnalazione errori di comunicazione
//-----------------------------------------------------------------------

    // Argo Errore Interchar temporaneamente disabilitato: DA SISTEMARE (vedi v8.31.127 Beta)
    //if(TimeOutInterCharComErr >= K_MaxCommErrorAlarm)
    //   Me.ErrorLedBlink.U1_MyBUS = 1;
    Me.ErrorLedBlink.U1_MyBUS = 0;
    
    if(TimeOutInterCharComErrSec >= K_MaxCommErrorAlarm || CntComErrSec >= K_MaxCommErrorAlarm || TimeOutComErrSec >= K_MaxCommErrorAlarm || CRC_ComErrSec >= K_MaxCommErrorAlarm)
       Me.ErrorLedBlink.U2_MyBUS = 1; 

    // Argo Errore Interchar temporaneamente disabilitato: DA SISTEMARE (vedi v8.31.127 Beta)
    //if(ModTimeOutInterCharComErr >= K_MaxCommErrorAlarm)
    //   Me.ErrorLedBlink.U1_ModBUS = 1; 
       Me.ErrorLedBlink.U1_ModBUS = 0; 
    
    if(ModTimeOutInterCharComErrSec >= K_MaxCommErrorAlarm || ModCntComErrSec >= K_MaxCommErrorAlarm || ModTimeOutComErrSec >= K_MaxCommErrorAlarm || ModCRC_ComErrSec >= K_MaxCommErrorAlarm)
       Me.ErrorLedBlink.U2_ModBUS = 1; 
    
    if(Me.CumulativeAlarm)
        Me.ErrorLedBlink.CumulativeAlarm = 1;
    
    
    
    if(TimerComunicationError.TimeOut)
    {	
        TimerComunicationError.Value = 0;
        TimerComunicationError.TimeOut = 0;		        
        
        TimeOutInterCharComErr = 0;     
        
        CntComErrSec = 0;
        TimeOutComErrSec = 0;
        TimeOutInterCharComErrSec = 0;        
        CRC_ComErrSec = 0;
        
        ModCntComErrSec = 0;
        ModTimeOutComErrSec = 0;
        ModTimeOutInterCharComErrSec = 0;             
        ModCRC_ComErrSec = 0;            
        
        ModCntComErr = 0;			
        ModTimeOutComErr = 0;		
        ModTimeOutInterCharComErr = 0;
        ModCRC_ComErr = 0;
    }            
    

}


float AD2Kpid(int Adc, float min, float max)
{
    float Kconv;
    
    Kconv = 700.0 / (max-min);
    
    return (float)Adc / Kconv;
}

//----------------------------------------------------------------------------------
//	Main
//----------------------------------------------------------------------------------
int main(void)
{
	int media = 0;
	
#if (K_DEBUG_LCD==1)    
    LCD_Info_Init();                                // Inizializza eventuale display LCD seriale I2C 4x20
#endif        
    
	oPwrLed = LED_ON;
	Init();   
        
        
#if (K_AbilFanInverter==1)	 
    InitInverterFan(&Inverter, Add_Fan_Inverter);
#endif    
    
	TimerSystemCycle.Enable = 1;			// Parto con il calcolo del Cycle time  v8.4.16
    
#if K_DEVELOP==1 
    if(Me.My_Address==0)
#ifdef UART3_ON    
        Hw_Test_PLX_DAQ();        
#else
        Hw_Test();    
#endif
    
#endif         
        
	while (1)
	{        
        
		// leggo le sonde
		//AD_Read();    spostato in interrupt Timer 1

		// elaboro le letture
 		Me.Temperature.Gas = Steinhart(Adc_Temp[1], 1);   // si utilizza la curva di linearizzazione Steinhart
		Me.Temperature.Liquid = Steinhart(Adc_Temp[2], 1);	
		Me.Temperature.AirOut = Steinhart(Adc_Temp[3], 1);
#if(K_OnlyOne_Press_Sensor==1)
        if(Me.I_Am_SubSlave && !ComunicationLost)
            Me.Temperature.Ambient = Me.TempAmbRealSelection;
        else
            Me.Temperature.Ambient = DifferentialPressureConversion(Adc_Temp[5]);
#elif(K_DiffPress_Fan_Control==1 || K_DiffPress_Auto_Fan_Control==1)
        Me.Temperature.Ambient = DifferentialPressureConversion(Adc_Temp[5]);
#elif (K_ADAuxChannelEnable==0)
		Me.Temperature.Ambient = Steinhart(Adc_Temp[4], 2); 	   
#endif
#if(K_ADAuxChannelEnable==1)
		Me.Temperature.Ambient = (Steinhart(Adc_Temp[4], 2)+Steinhart(Adc_Temp[8], 1))/2;
#endif        
#if(K_ADAuxChannelEnable==2)
		Me.Temperature.Ambient = (Steinhart(Adc_Temp[4], 2)+Steinhart(Adc_Temp[8], 1)+Steinhart(Adc_Temp[9], 1))/3;
#endif               
#if(K_ADAuxChannelEnable==3)
		Me.Temperature.Ambient = (Steinhart(Adc_Temp[8], 1)+Steinhart(Adc_Temp[9], 1))/2;
#endif               
#if(K_ADAuxChannelEnable==4)
		Me.Temperature.Ambient = Steinhart(Adc_Temp[8], 1);
#endif 

#if(K_Channel_Gas_Regulation==1)
        if(!Me.I_Am_SubSlave)
            Me.Channel_Gas_Regulation = Pot_Read(Adc_Temp[5], PWMValue[1], PWMValue[8]);
#elif(K_Humidity_Read==1)
        Me.Humidity = HumidityConversion(Adc_Temp[5]);
        if(Me.Humidity==0)
            Me.Humidity=1;
#else
		
		Me.Pres_Big = Pressure_Read_45(Adc_Temp[5]);			//Adc_P_Grande;    
#endif
		//Me.Pres_Small = Pressure_Read_34(Adc_Temp[6]);		//Adc_P_Piccolo;
#if(K_Channel_Liquid_Light==1)
        Me.Channel_Liquid_Light = Pot_Read(Adc_Temp[6], 0, 255);
        oDigitOutLight = Me.Channel_Liquid_Light>128;
#else
		Me.Pres_Small = Pressure_Read_45(Adc_Temp[6]);		//Adc_P_Piccolo;	
#endif  		
        
		
	/* x taratura PID
        ValvePID.VKiPID = AD2Kpid(Adc_Temp[5], 0.0, 0.1);			//Adc_P_Grande;   Range Ki = 0.01 .. 1.0 
		ValvePID.VKdPID = AD2Kpid(Adc_Temp[6], 0.0, 2.5);		//Adc_P_Piccolo;
     */     
#ifdef SUPPLY24_ON            
        Me.Supply_Voltage = PowerSupplyConversion(Adc_Temp[10]);		// Tensione di alimentazione scheda	in centesimi di Volt (Es.: 2456 24,56V)          
#else        
        Me.Supply_Voltage = K_SupplyVoltage;
#endif        

        
#if(((SplitMode==K_I_Am_Freezer) || (SplitMode==K_I_Am_Frigo))&&(K_SuperHeatEngBox==1))
		Me.Superheat = EngineBox.Temp_Ritorno - PressureGasToTempR410A(EngineBox.Pressione_Gas_G);		// Superheat in °C        
#else
		Me.Superheat = Me.Temperature.Gas - PressureGasToTempR410A(EngineBox.Pressione_Gas_G);		// Superheat in °C
#endif
		//Me.Superheat = Me.Temperature.Gas - Me.Evap_Temp;
		/*
        media = Me.Temperature.Gas - PressureGasToTempR410A(EngineBox.Pressione_Gas_G);
		Me.Superheat = ((Me.Temperature.Gas - Me.Evap_Temp)+media)/2;
		*/
        
        Me.Superheat = Filter(&FilterSuperHeat, Me.Superheat, K_Campioni_Filter);
		if(Me.Superheat<0) 
			Me.Superheat = 0;
		
		Me.SuperheatRiscaldamento = EngineBox.Temp_Ritorno - (PressureGasToTempR410A(EngineBox.Pressione_Liq_P)-3000);//ATTENZIONE: Sottraggo 5 bar per ovviare a mancanza terza sonda pressione liquido
        Me.SuperheatRiscaldamento = Filter(&FilterSuperHeatRiscaldamento, Me.SuperheatRiscaldamento, K_Campioni_Filter);
		
		Me.EvapLiq = (PressureGasToTempR410A(EngineBox.Pressione_Gas_G)+300) - Me.Temperature.Liquid;
        //Calcolo valore che deve raggiungere Temp_Big
        //Me.T_Big_Request = PressureGasToTempR410A(EngineBox.Pressione_Gas_G) + ValvePID.SetP;	//DEBUG
        //Me.T_Big_Request = Filter(&FilterBig_Request, Me.T_Big_Request, K_Campioni_Filter);	//DEBUG
		//Me.T_Big_Request = Me.Evap_Temp + K_SetPoint;
		media = PressureGasToTempR410A(EngineBox.Pressione_Gas_G) + ValvePID.SetP;
		Me.T_Big_Request = ((Me.Evap_Temp + Me.SuperHeat_SP /*K_SetPoint*/)+media)/2;
		Me.T_Big_Request = Filter(&FilterBig_Request, Me.T_Big_Request, K_Campioni_Filter);	//DEBUG
		
		Me.Subcooling = PressureGasToTempR410A(EngineBox.Pressione_Liq_P) - Me.Temperature.Liquid;	// Subcooling in °C		
				
		Me.TempAmbRealSelection = TempAmbMidSel();	//Calcolo la temperatura fancoil mediata con sottoslave o singola per funzionamento
        
        RealFunctionMode();     // Calcolo la reale modalità di funzionamento
 
        OkDataProcess();//Faccio il dataprocess per attivare sincro con touch

//--------------------------------------------------------------------------------------------------------------------------------------------------------------        
// Zona comunicazioni Letture - Inizio
//--------------------------------------------------------------------------------------------------------------------------------------------------------------        
#if(k_Abil_Panel_Shared==1)        
if(Me.EnableCommunication_Sec==1)
#endif
{        
		// Leggo l'input delle schede split sul secondo bus
#if((SplitMode!=K_I_Am_Frigo && SplitMode!=K_I_Am_Freezer && K_UTAEnable==0) || K_Force_SubSlave_Abil==1)           
		RefreshAllInPut_Sec();					// Controllo se le eventuali schede Split sul BUS secondario sono abilitate e OnLine
#endif

#if(K_UTAEnable==0/* && SplitMode!=K_I_Am_Frigo && SplitMode!=K_I_Am_Freezer*/)        
		// Leggo l'imput dei Touch/NetB    
		if(Me.NetBSelect)
		{
			if(RefreshNetBRead.TimeOut)		// Leggo NetB ogni 100mS
			{
				RefreshNetBRead.TimeOut=0;				
				Reinit_DataSplitToNetB();
				RefreshAllNetBInPut();			// Leggo i dati provenienti dal NetB
				NetB2Touch();					// e li "travaso" in struct Touch[] per mantenere inalterata le funzioni in Core.C che usano Touch[]...
			}
            if(NetB[0].Enable && NetB[0].PermanentOffline)
                Me.ModDeviceEnableAndOffLine = 1;
            else if(NetB[0].OnLine)
                Me.ModDeviceEnableAndOffLine = 0;

            Me.ModDeviceStatus = 0;
            Me.ModDeviceStatus = (NetB[0].Enable || Me.ModDeviceEnableAndOffLine)<<3 | NetB[0].OffLine<<2 | NetB[0].OnLine<<1 | NetB[0].DeviceAlarm;      // Status: Bit3=ENABLE, Bit2=OFFLINE, Bit1=ONLINE, Bit0=Alarm
            
            if(!NetB[0].OnLine)
                Me.ModDeviceType = K_ModDeviceNone;
            
            Me.ModDeviceAlarm = 0;
		}
		else if(Me.SyxtSSelect)      // Leggo l'imput dei Touch/SyxtSense    
		{
            Reinit_DataSplitToSyxtS();
            RefreshAllSyxtSInPut();			// Leggo i dati provenienti dal SyxtSense
            SyxtS2Touch();					// e li "travaso" in struct Touch[] per mantenere inalterata le funzioni in Core.C che usano Touch[]...
            if(SyxtS[0].Enable && SyxtS[0].PermanentOffline)
                Me.ModDeviceEnableAndOffLine = 1;
            else if(SyxtS[0].OnLine)
                Me.ModDeviceEnableAndOffLine = 0;

            Me.ModDeviceStatus = 0;
            Me.ModDeviceStatus = (SyxtS[0].Enable || Me.ModDeviceEnableAndOffLine)<<3 | SyxtS[0].OffLine<<2 | SyxtS[0].OnLine<<1 | SyxtS[0].DeviceAlarm;      // Status: Bit3=ENABLE, Bit2=OFFLINE, Bit1=ONLINE, Bit0=Alarm
            
            if(!SyxtS[0].OnLine)
                Me.ModDeviceType = K_ModDeviceNone;
           
            Me.ModDeviceAlarm = 0;
		}
		else    
#endif				            
		{
			RefreshAllTouchInPut();				// Leggo i dati provenienti dal Touch       500mS
			Reinit_DataSplitToTouch();
            if(Touch[0].Enable && Touch[0].PermanentOffline)
                Me.ModDeviceEnableAndOffLine = 1;
            else if(Touch[0].OnLine)
                Me.ModDeviceEnableAndOffLine = 0;

            Me.ModDeviceStatus = 0;
            Me.ModDeviceStatus = (Touch[0].Enable || Me.ModDeviceEnableAndOffLine)<<3 | Touch[0].OffLine<<2 | Touch[0].OnLine<<1 | Touch[0].DeviceAlarm;      // Status: Bit3=ENABLE, Bit2=OFFLINE, Bit1=ONLINE, Bit0=Alarm
            
            if(!Touch[0].OnLine)
                Me.ModDeviceType = K_ModDeviceNone;
                
            Me.ModDeviceAlarm = 0;
		}

		CheckValueRange();	// Controllo limiti valori Var. ed eventualmente correggo se fuori range
} //if(K_EnableCommunication_Sec==1)
//--------------------------------------------------------------------------------------------------------------------------------------------------------------        
// Zona comunicazioni Letture - Fine
//--------------------------------------------------------------------------------------------------------------------------------------------------------------        
        
		// Elaborazione
		FunctionMode();			// aggiorno la modalità di funzionamento
#if(SplitMode!=K_I_Am_OnlyTouchComm) 
		TempEvaporazione();		// aggiorno la temperatura di evaporazione
		if(Me.InitValvolaInCorso == 0)
			RegolaValvola();		// aggiorno la regolazione della valvola
        
#if (K_TachoSpeedAlarmDisable==0)		
		ControlloVentil();		// aggiorno gli errori del ventilatore
#endif

        FanPowerCalculation();  // Calcola la potenza assorbita dal FAN
        FanM3hCalculation();    // Calcola i m3 dei FAN
		UpdateFanSpeed();       //AggiornaVentil();		// aggiorno i ventil        
        Thermal_Power_Output_Calculation ();
        
#if(K_Evaporator_FAN==1)
    FAN_Rele_Evaporator();
#endif
        
#if (K_AbilFanInverter==1)	
        PWM2Freq();
        RefreshInverterFan(&Inverter, Add_Fan_Inverter);	// aggiorno i registri dell'inverter compressore dei touch            
#endif    
        
        
#if(K_AbilFanInverter==0)                
        if(Me.Ventil_Selection==K_ZIEHL_ABEGG_ModBus)
        {
            RefreshZiehgAbbModbusFan(&Inverter, Add_Fan_ZihegAbb_ModBus);
        }
        else if(Me.Ventil_Selection==K_EBM_3200RPM_ModBus)
        {
            RefreshEBMModbusFan(&Inverter, Add_Fan_EBM_ModBus);
        }        
        else
        {
            Inverter.FanInverterFault = 0;
            Inverter.ErrorCode = 0;
        }
#endif
//--------------------------------------------------------------------------------------------------------------------------------------------------------------        
// Zona comunicazioni Scritture - Inizio
//--------------------------------------------------------------------------------------------------------------------------------------------------------------        
#if(k_Abil_Panel_Shared==1)        
if(Me.EnableCommunication_Sec==1)
{
#endif
#if((SplitMode!=K_I_Am_Frigo && SplitMode!=K_I_Am_Freezer && K_UTAEnable==0 && SplitMode!=K_I_Am_InverterFAN && SplitMode!=K_I_Am_StandardFAN && SplitMode!=K_I_Am_Atex_Fan && SplitMode!=K_I_Am_On_Off_Fan) || K_Force_SubSlave_Abil==1)             
		// Scrivo l'out delle schede split sul secondo bus
		RefreshAllOutPut_Sec();
#endif
#endif
		// Scrivo l'output dei touch

		if(Me.NetBSelect)
			RefreshAllNetBOutPut();		// Scrivo i dati verso il NetB
        else if(Me.SyxtSSelect)
			RefreshAllSyxtSOutPut();		// Scrivo i dati verso il SyxthSense		
		else
			RefreshAllTouchOutPut();		// Scrivo i dati verso il Touch

		
		// Funzioni di servizio
		//RefreshE2_Data();							// aggiorno i dati in E2
		//CheckValueRange();	// Controllo limiti valori Var. ed eventualmente correggo se fuori range
		//CurUsedProtocolId = FlagMyBusProt;
#if((K_UTAEnable==0 && SplitMode!=K_I_Am_InverterFAN && SplitMode!=K_I_Am_StandardFAN && SplitMode!=K_I_Am_Atex_Fan && SplitMode!=K_I_Am_On_Off_Fan) || K_Force_Panel_Abil==1)      
        if(Me.I_Am_SubSlave==0)
        {
//            if((Touch[0].OnLine==0 && Touch[0].PermanentOffline==1) &&  (SyxtS[0].OnLine==0 && SyxtS[0].PermanentOffline==1) && (NetB[0].OnLine==0 && NetB[0].PermanentOffline==1))//Se scollego i dispositivi, ricomincio a cercarli per vedere se ne ho collegato uno differente.
//                Me.ModBusDeviceDetected = 0;     
            
            if(Me.ModBusDeviceDetected == 0)        
            {
                SchedSearchDevice++;
                SearchModbusDeviceIndex(&SchedSearchDevice);       // Ricerca del device modbus collegato: 0= NoDevice connected, 1=Touch Exor/Carel connected, 2=Touch Syxthsense connected, 3=NetBuilding Panel connected        
            }            
            
            if(Me.NetBSelect)
            {
                Resume_NetBOffLine(&CntResumeNetB);	// tento di ripescare eventuali NetB off line
                CheckHotPlugNetB(&ScanNetBCounter);		// Controllo per eventuali Touch collegati a caldo....
            }
            else if(Me.SyxtSSelect)                
            {
                Resume_SyxtSOffLine(&CntResumeSyxtS);	// tento di ripescare eventuali SyxtS off line
                CheckHotPlugSyxtS(&ScanSyxtSCounter);		// Controllo per eventuali SyxtS collegati a caldo....
            }                
            else    
            {
                Resume_TouchOffLine(&CntResumeTouch);	// tento di ripescare eventuali touch off line	
                CheckHotPlugTouch(&ScanTouchCounter);		// Controllo per eventuali Touch collegati a caldo....	                      
            }
		}
#endif          
#if((SplitMode!=K_I_Am_OnlyTouchComm && SplitMode!=K_I_Am_Frigo && SplitMode!=K_I_Am_Freezer && SplitMode!=K_I_Am_InverterFAN && SplitMode!=K_I_Am_StandardFAN && SplitMode!=K_I_Am_Atex_Fan && SplitMode!=K_I_Am_On_Off_Fan && K_UTAEnable==0) || (K_Force_SubSlave_Abil==1))
        if(Me.I_Am_SubSlave==0)
        {
            ResumeOffLine_Sec(&CntResumeSplit_Sec);        
            CheckHotPlugSlave_Sec(&ScanBoardCounter_Sec);		// Controllo per eventuali schede collegate a caldo....	   
        }
#endif
#if(k_Abil_Panel_Shared==1)        
    Me.EnableCommunication_Sec=0;        
    TimerToggleComunication_Sec.Value = 0;
} //if(K_EnableCommunication_Sec==1)
#endif
//--------------------------------------------------------------------------------------------------------------------------------------------------------------        
// Zona comunicazioni Scritture - Fine
//--------------------------------------------------------------------------------------------------------------------------------------------------------------        

#if(K_Heater_Abil==1)
        HeaterControl();
#endif

#if(K_Gas_Valve_Control==1)
        GasValveControl();        
#endif

#if (SplitMode == K_I_Am_Frigo) || (SplitMode == K_I_Am_Freezer) || (K_ErrorSPEnable==1)       
        CheckErrorSPTempFault();                    //Controllo per capire se sono in temperatura entro un tempo stabilito(frigo e freezer))
#endif
 
#if (SplitMode != K_I_Am_InverterFAN) && (SplitMode != K_I_Am_ManualRegulation) && (SplitMode != K_I_Am_StandardFAN) && (SplitMode!=K_I_Am_Atex_Fan) && (SplitMode!=K_I_Am_On_Off_Fan) && (SplitMode != K_I_Am_OnlyTouchComm)
        CheckErrorTemperatureProbe();
#endif        
		ErrorReport();                              // Segnalo con LED rosso se presenti errori sullo Split
		
        RefreshE2_TouchValue_DoubleSector(K_UseVirtualE2);		// Aggiorna in EEPROM eventuali variazioni di parametri provenienti dal Touch (Velocità, SetPoint, Modalità....)
          
        
#ifdef UART3_ON            
        //ARGOPUZZONE  
        //PutDataForUART3(Me.Fan_Current_Limit, Me.Fan_Current_Fast, Me.Pwm_Req, Me.Pwm_Value, 0, 0, Me.CycleTime, 0);
#endif
       
        BootLoaderAutoReset();
 
        if(TimerSystemCycle.Value < K_MinTimeSystemCycle)               // Se il tempo di ciclo è minore di K_min... 
            DelaymSec(K_MinTimeSystemCycle - TimerSystemCycle.Value);     // Attendo a vuoto fino a raggiungere il tempo minimo
        
               
        
		Me.CycleTime = TimerSystemCycle.Value;		// Memorizzo valore timer tempo di ciclo per successivo invio sul Touch x diagnostica
        
        
#if (K_DEBUG_LCD==1)    
        Print_LCD_Info();                           // Stampa informazioni di sistema e debug su eventuale display LCD seriale I2C 4x20
#endif         
		TimerSystemCycle.TimeOut = 0;				// resetto il time out del timer Elapsed
		TimerSystemCycle.Value = 0;					// resetto il time value del timer Elapsed

	}
}

//----------------------------------------------------------------------------------
//	function and procedure
//----------------------------------------------------------------------------------
void Init(void)
{
int temp;    
    
	RCON = 0;				// Reset control

	LATA = 0;				// resetto il portA
	TRISA = 0x07FF;			// setto gli i/o del PortA
	LATB = 0;				// resetto il portB
	TRISB = 0x0FFF;         // setto gli i/o del PortB (Aggiunti A/D Ch5 e Ch6)
	LATC = 0;				// resetto il portC
	TRISC = 0x0010;			// setto gli i/o del PortC
	LATD = 0;				// resetto il portD
	TRISD = 0x9C03;			// setto gli i/o del PortD	// NEW HW n.1 DigitOut (RD13)  e n.1 DigiIn (RD12))
	LATE = 0;				// resetto il portE
    ODCE = 0x0010;          // RE4 = Open Drain
    oCurrDriver = 1;        //   
    //TRISE = 0x00C0;			// setto gli i/o del PortE
    TRISE = 0x0080;			// setto gli i/o del PortE      <- Soluzione contro l'impallamento della EEPROM (reset del port SCL)
    
	LATF = 0;				// resetto il portF
	TRISF = 0x0114;			// setto gli i/o del PortF
	LATG = 0;				// resetto il portG
#ifdef UART3_ON        
	TRISG = 0x0200;			// setto gli i/o del PortG RG9=Input -> RxUART3
#else
	TRISG = 0x0000;			// setto gli i/o del PortG 0040
#endif
	oPWM_Fan1 = 1;
	oPWM_Fan2 = 1;

	Remap_RP();				// rimappo le periferiche

	// abilito i pull up
	CNPU3bits.CN33PUE = 1;	// Pull up A0
	CNPU3bits.CN34PUE = 1;	// Pull up A1
	CNPU3bits.CN35PUE = 1;	// Pull up A2
	CNPU3bits.CN36PUE = 1;	// Pull up A3
	CNPU3bits.CN37PUE = 1;	// Pull up A4
	CNPU3bits.CN38PUE = 1;	// Pull up A5
	CNPU3bits.CN39PUE = 1;	// Pull up A6
	CNPU3bits.CN40PUE = 1;	// Pull up A7

	CNPU5bits.CN64PUE = 1;	// pull up SCL
	CNPU5bits.CN65PUE = 1;	// pull up SDA
	CNPU4bits.CN48PUE = 0;	// pull up iTacho OFF (Vedi nuovo HW v8.2)
	CNPU4bits.CN49PUE = 1;	// pull up iFault
	CNPU4bits.CN50PUE = 1;	// pull up iHome
	CNPU4bits.CN55PUE = 1;	// pull up Idigi1
	CNPU4bits.CN56PUE = 1;	// pull up Idigi2

    DelaymSec(200);         // Pausa prima di leggere il DIP switch
	
	// Init ADC
	_AD1IE = 0;         // abilito interrupt AD
	_AD1IF = 0;         // resetto il flag d'interrupt del convertitore
	_AD1IP = 1;//4;		// imposto il livello di interupt (7 più alto)  
    
	Init_Adc();	

	My_Address = (~PORTA & 0x003F); //0x007F);                     // configuro l'indirizzo del modulo
    
	Me.My_Address = My_Address;
	//Me.NetBSelect = ((~PORTA & 0x0080) != 0);		// Flag attivazione gestione Pannelli NetBuilding (Dip Switch n.8 = ON)
    //if(!Me.NetBSelect)
    //    Me.SyxtSSelect = ((~PORTA & 0x0040) != 0);		// Flag attivazione gestione Pannelli NetBuilding (Dip Switch n.8 = ON)


	//oErrLed=LED_ON;
    
    DelaymSec(200);         
	I2C_Init();             // inizializzo l'I2C1
    DelaymSec(200);         
    
    if(Me.My_Address!=0)    // Se Address==0 vado al Test HW che eseguirà i test sulla EPPROM
    {
        CheckEEPROM_ReadWrite();
        CheckEEPROM_Retention();
        //LoadAllE2_Data();		// carico i valori dala E2
        LoadE2_TouchValue_DoubleSector();	// carica i valori di funzionamento da EEPROM
        Me.EEPROM_SectorPage = Store_Offset_SectorPage(K_EEPROM_Primary_Sector); //Salva l'offset per risalire alla pagina della EEProm utilizzata
    }

    
	//oErrLed=LED_OFF;	

	
    Me.HW_Version = HW_VERSION;                         // Carico Versione HW di Compilazione      
    Me.FW_Version_HW = Versione_Hw;                     // Carico Versione HW del FW
    Me.FW_Version_FW = Versione_Fw;                     // Carico Versione FW del FW
    Me.FW_Revision_FW = Revisione_Fw;                   // Carico Revisone FW
    
    Me.DefMode = K_Def_SPLIT_Mode;
	Me.SuperHeat_SP_Local = K_SetPoint;	
	Me.Freddo_TempBatt_SP_Local = Freddo_TempBatt_MaxPwr;
    Me.Freddo_TempBatt_Min = Freddo_OffSetBatt_MinPwr;
	Me.Caldo_TempBatt_SP_Local = Caldo_TempBatt_MaxPwr;	
    Me.Caldo_TempBatt_Min = Caldo_OffSetBatt_MinPwr;	
	Me.PerformanceLossBattery_SP = K_PerformanceLossBattery;
    Me.Dinamic_SuperHeat = K_Dinamic_SuperHeat_Default;
    Me.OtherCabin.Number = 0;   //Carico a zero di default!
    Me.Inverter_Supply = K_Supply_Inverter;
	Me.TempAmb_Middle_Heating = K_Mid_SlaveSec_Temp_Amb_Function_Heating;
	Me.TempAmb_Middle_Cooling = K_Mid_SlaveSec_Temp_Amb_Function_Cooling;
    Me.DoublePump_Abil = K_DoublePump_Abil;
	Me.SplitMode_Abil = K_Mode_Abil;
    Me.FanNewStyleTimeOn = K_FanNewStyleHeatTimeON;
    Me.FanNewStyleTimeOff = K_FanNewStyleHeatTimeOFF;
    Me.NetB_LCD_Intens = K_NetB_LCD_Intensity;

	Me.Extractor.Number = 0;    //Carico a zero di default!
	Me.Extractor.DeltaVel = 0;  //Carico a zero di default!
    
    Me.Error.SPTemperatureFault = 0;
    
    EngineBox.PressureLoLiqValue = K_Freddo_Valve_ViLimMax_Liq_PressLo;				                          
    EngineBox.PressureLoGasValue = K_Freddo_Valve_ViLimMax_Gas_PressLo;				                          
    EngineBox.PressureHiLiqValue = K_Freddo_Valve_ViLimMax_Liq_PressHi;				                          
    EngineBox.PressureHiGasValue = K_Freddo_Valve_ViLimMax_Gas_PressHi;

    Me.M3h_Min = K_M3h_Min;
    Me.M3h_Max = K_M3h_Max;

    Me.Min_Current_Fan = K_Min_Current_Custom;
	Me.Max_Current_Fan = K_Max_Current_Custom;
	Me.Efficiency_Fan = K_Efficiency_Custom;
    Me.Supply_Fan = K_Supply_Custom;

    EngineBox.Threshold_Compressor_Hi = K_Freddo_Valve_ViLimMax_TempHi;
    
    Touch[0].PermanentOffline = 1;
    SyxtS[0].PermanentOffline = 1;
    NetB[0].PermanentOffline = 1;

    
    /*        
    if(Me.NetBSelect)       //DA RIVEDERE ModDeviceType
    {
        Me.MaxFanVentil_Mode = K_DefaultFanSpeed_NetB;
        Me.ModDeviceType = 2;   // NetBuilding      // TYPE: 0=Nessun Device, 1=Touch, 2=NetBuilding, 3=SyxthSense
    }
    else if(Me.SyxtSSelect)
    {
        Me.MaxFanVentil_Mode = K_DefaultFanSpeed_SyxtS;
        Me.ModDeviceType = 3;   // SyxthSense
    }
    else
    {
        Me.MaxFanVentil_Mode = K_DefaultFanSpeed;
        Me.ModDeviceType = 1;   // Touch    
    }
*/    

    Me.CorrectSuperHeatAirOut_Local = K_Default_Temp_CorrectSuperHeatAirOut;
    Me.CorrectSuperHeatAirOut = K_Default_Temp_CorrectSuperHeatAirOut;
    
    DiagnosticSplit.OldValueCmdAddress = 0;
    
    DiagnosticSplit.Humidity = 0;
    
    Me.Ventil_Selection = K_VentilSelection;
    
    Inverter.FanInverterFault = 0;
	    
    //InitADConversion();     // Esegue un primo giro di conversioni per averle a posto prima di comunicare con il Master
    
    INTCON1bits.NSTDIS = 0; // Interrupt nesting is enabled (Default)

//-----------------------------------------------------------------------------------    
// Legenda Priorità assegnate ai vari Interrupt (Livello 7 = MAX):
//
// Livello      Risorsa
//--------------------------
//  7           UART2  (Slave BUS)
//  6           UART1  (Master BUS)
//  5           Timer1
//  4           A/D  
//  3           UART3  (Service BUS)
//-----------------------------------------------------------------------------------    

	// Init UART 1			// Bus serial @57K6, 8,n,1
	init_comms1();
    _U1RXIE = 1;            // abilito interrupt su rx usart 1
    _U1TXIE = 0;			// Disattivo l'interrupt di TX Uart1
    _U1ERIE = 1;            // abilito interrupt Error usart 1
	_U1RXIP = 6;            // imposto il livello di interupt (7 più alto)
    _U1ERIP = 6;            // imposto il livello di interupt (7 più alto)
    _U1TXIP = 7;            // imposto il livello di interupt per La tramissione carattere UART1 (7 più alto)    
    ClearUart1Err();

	// Init UART 2			// Touch Serial @57K6, 8,n,1
	init_comms2();
    _U2RXIE = 1;            // abilito interrupt su rx usart 2
    _U2TXIE = 0;			// Disattivo l'interrupt di TX Uart2
    _U2ERIE	= 1;            // abilito interrupt Error usart 2
    _U2RXIP = 7;    	
    _U2ERIP = 7;
    _U2TXIP = 7;            // imposto il livello di interupt per La tramissione carattere UART2 (7 più alto)        
	ClearUart2Err();
    
    
#ifdef UART3_ON     
	// Init UART 3			
	init_comms3();
    _U3RXIE = 1;            // abilito interrupt su rx usart 3
    _U3TXIE = 0;            // Disattivo l'interrupt di TX Uart3
    _U3ERIE = 1;            // abilito interrupt Error usart 3    
//#if(K_Panel_On_Uart3==0)    
#if(K_MyBus==0 && K_Panel_On_Uart3==0 && K_ModBus_Generic_On_Uart3==0)    
    _U3RXIP = 3;            // imposto il livello di interupt per la ricezione carattere UART3    
    _U3ERIP = 3;            // imposto il livello di interupt x ERROR
    _U3TXIP = 3;            // imposto il livello di interupt per la tramissione carattere UART3    
#else
    _U3RXIP = 7;            // imposto il livello di interupt per la ricezione carattere UART3    
    _U3ERIP = 7;            // imposto il livello di interupt x ERROR
    _U3TXIP = 7;            // imposto il livello di interupt per la tramissione carattere UART3        
#endif    
	ClearUart3Err();    
#endif
    

	Init_Timer();			// Init TMR1
	Init_PWM_Module();		// Init PWM
	InitValvola();			// Init Valvola
	Tacho_Init();
	PID_init(&ValvePID, K_p_gain, K_i_gain, K_d_gain, K_iMin, K_iMax, K_PoMin, K_PoMax, K_dead_zone);
	PID_init(&ValvePID2, K_p_gain, K_i_gain, K_d_gain, K_iMin, K_iMax, K_PoMin, K_PoMax, K_dead_zone);
	Init_PWM_Value();		// Inizializza i valori delle velocità ventialtore da costanti anziche da EEPROM
	
	// inizializzo il timer per il test delle EEV
	TimerTestEEV.Enable = 0;
	TimerTestEEV.TimeOut = 0;
	TimerTestEEV.Time = WaitCloseTime;
	TimerTestEEV.Value = 0;

	// inizializzo il timer per il controllo della valvola espansione
	TimerExpValvola.TimeOut = 0;
	TimerExpValvola.Time = WaitExpValvolaCaldo;
	TimerExpValvola.Value = 0;
	TimerExpValvola.Enable = 1;
	
	TimerRestartValve.Time = K_TimeRestartValve;			// Tempo in mS per esecuzione regolatore PID per controllo Valvola espansione
	TimerRestartValve.TimeOut = 0;					// resetto il timer
	TimerRestartValve.Value = 0;
	TimerRestartValve.Enable = 1;
	

	// configuro il timer per il controllo della comunicazione
	TmrLastRequest.Value = 0;
	TmrLastRequest.Time = 30;
	TmrLastRequest.TimeOut = 0;
	TmrLastRequest.Enable = 1;
    
	// inizializzo il timer per il resync della valvola espansione
	TimerResyncValvola.TimeOut = 0;
	TimerResyncValvola.Time = K_WaitResyncValvola;
	TimerResyncValvola.Value = 0;
	TimerResyncValvola.Enable = 0;    
    
    // inizializzo il timer per l'errore valvola di espansione in freddo di alta temperatura
    TimerExpValv_Err.TimeOut = 0;
	TimerExpValv_Err.Time = K_Time_Wait_Correct_Valve_Err;
	TimerExpValv_Err.Value = 0;
	TimerExpValv_Err.Enable = 0; 
    
    
    TimerCheckFanTacho.TimeOut = 0;
	TimerCheckFanTacho.Time = 30;   //10;
	TimerCheckFanTacho.Value = 0;
	TimerCheckFanTacho.Enable = 0;  
    
    TimerCheckBatteryDefrost.TimeOut = 0;
	TimerCheckBatteryDefrost.Time = 60;
	TimerCheckBatteryDefrost.Value = 0;
	TimerCheckBatteryDefrost.Enable = 0;    
    
    TimerFanVelSwitch.TimeOut = 0;
	TimerFanVelSwitch.Time = K_FanNewStyleHeatTimeON;
	TimerFanVelSwitch.Value = 0;
	TimerFanVelSwitch.Enable = 1;   
    
    TimerDefrostingCycleWork.Time = K_DefrostingCycleWork;
    TimerDefrostingCycleWork.Enable = 0;
    TimerDefrostingCycleWork.TimeOut = 0;
    //TimerDefrostingCycleWork.Value = 0;
    
    TimerDefrostingCycleStop.Time = K_DefrostingCycleStop;
    TimerDefrostingCycleStop.Enable = 0;
    TimerDefrostingCycleStop.TimeOut = 0;
    TimerDefrostingCycleStop.Value = 0;
    
    TimerDefrostingCycleCheck.Time = K_DefrostingCycleCheck;
    TimerDefrostingCycleCheck.Enable = 0;
    TimerDefrostingCycleCheck.TimeOut = 0;
    TimerDefrostingCycleCheck.Value = 0;
    
    TimerCorrectSuperHeatAirOut.Time = K_CorrectSuperHeatAirOut;
    TimerCorrectSuperHeatAirOut.Enable = 0;
    TimerCorrectSuperHeatAirOut.TimeOut = 0;
    TimerCorrectSuperHeatAirOut.Value = 0;   
    
    TimerFloodFanCoil.Time = K_TimerFloodFanCoil;
    TimerFloodFanCoil.Enable = 0;
    TimerFloodFanCoil.TimeOut = 0;
    TimerFloodFanCoil.Value = 0;    
    
    TimerOutWaterStop.Time = 30;
    TimerOutWaterStop.Enable = 0;
    TimerOutWaterStop.TimeOut = 0;
    TimerOutWaterStop.Value = 0;    

    TimerGasLeaks.Time = K_TimerGasLeaks;
    TimerGasLeaks.Enable = 0;
    TimerGasLeaks.TimeOut = 0;
    TimerGasLeaks.Value = 0;    

            
    TimerSelectPumpWork.Time = 12*60*60;
    TimerSelectPumpWork.Enable = 0;
    TimerSelectPumpWork.TimeOut = 0;
    TimerSelectPumpWork.Value = 0;    
    

#if (SplitMode == K_I_Am_Frigo) || (SplitMode == K_I_Am_Freezer) || (K_ErrorSPEnable==1)
    TimerTemperatureFault.Time = K_TimeTemperatureFault;
    TimerTemperatureFault.Enable = 0;
    TimerTemperatureFault.TimeOut = 0;
    TimerTemperatureFault.Value = 0;   
#endif    
    
    TimerTemperatureProbeError.Time = K_TimeTemperatureProbe_Error;
    TimerTemperatureProbeError.Enable = 0;
    TimerTemperatureProbeError.TimeOut = 0;
    TimerTemperatureProbeError.Value = 0;   
    
    TimerCheckProbe.Time = K_TimerCheckProbe_Error;
    TimerCheckProbe.Enable = 1;    
    TimerCheckProbe.TimeOut = 0;    
    TimerCheckProbe.Value = 0; 
    
    TimerDampersAlarm.Time = K_TimerDampers_Alarm;
    TimerDampersAlarm.Enable = 0;    
    TimerDampersAlarm.TimeOut = 0;    
    TimerDampersAlarm.Value = 0; 
    
    TimerErrFlow.Time = K_TimerFlow_Error;
    TimerErrFlow.Enable = 0;    
    TimerErrFlow.TimeOut = 0;    
    TimerErrFlow.Value = 0;     
    
    TimerHeaterFresh.Time = 1*60;   //faccio andare ventola ancora 1 minuto
    TimerHeaterFresh.Enable = 0;    
    TimerHeaterFresh.TimeOut = 0;    
    TimerHeaterFresh.Value = 0;     
    
/*    
    TimerMaxTimeCmdAddress.Time = K_TimeMaxCmdAddress;
    TimerMaxTimeCmdAddress.Enable = 0;
    TimerMaxTimeCmdAddress.TimeOut = 0;
    TimerMaxTimeCmdAddress.Value = 0;     
*/    
    if(SplitMode==K_I_Am_Frigo)
            Me.Frozen.Mode = 1;         // Setto bit0 per segnalare modalità Frigo
    else if(SplitMode==K_I_Am_Freezer)
            Me.Frozen.Mode = 2;         // Setto bit1 per segnalare modalità Freezer
    else if(SplitMode==K_I_Am_LoopWater)
            Me.Frozen.Mode = 3;         // Setto bit1 per segnalare modalità LoopWater
    else        
            Me.Frozen.Mode = 0;         // Resetto tutti i bit per annullare modalità Frigo/Freezer
    
	// setto i valori predefiniti del power manager
	/*
	Me.PowerLimit = InitPowerLimit;
	Me.OldPowerLimit = InitPowerLimit;
	EngineBox.PowerLimit = InitPowerLimit;
	EngineBox.OldPowerLimit = InitPowerLimit;7
	 */

    
	TimerStartUp.Time = 6;    
/*    
    //DA MODIFICARE
	// timer di startup dei touch/NetB  
	if(Me.NetBSelect)
		TimerStartUp.Time = 6; //20;
	else if (Me.SyxtSSelect)
		TimerStartUp.Time = 6; //20;
	else
	{
		if(Me.My_Address == k_Split_Master_Add)
			TimerStartUp.Time = 4;//40;  
		else
			TimerStartUp.Time = 4;//60;				// Se sono split slave attendo più  a lungo per essere sicuro di "svegliarmi" quando lo Slave Master è già pronto per poter passare i dati "DefMode" corretti.
	}
*/

#if K_DEVELOP==1	
    if(Me.My_Address==0)        
        TimerStartUp.Time = 4;
#endif			
		
#if (SplitMode==K_I_Am_InverterFAN || SplitMode==K_I_Am_StandardFAN || SplitMode==K_I_Am_Atex_Fan || SplitMode==K_I_Am_On_Off_Fan)
        TimerStartUp.Time = 5;
#endif			
	

#if K_SIMULATION_WORK==1
        TimerStartUp.Time = 1;
#endif			
		
	TimerStartUp.Enable = 1;
	//oErrLed = LED_ON;

	while(TimerStartUp.TimeOut == 0)
		continue;
	TimerStartUp.TimeOut = 0;
	TimerStartUp.Value = 0;
	TimerStartUp.Enable = 0;
	//oErrLed = LED_OFF;

#if(K_UTAEnable==0)
    if(Me.I_Am_SubSlave==0)
    {    
        // cerco gli split sul secondo bus    

        #if K_DEVELOP==1
        if(Me.My_Address!=0 && (Me.I_Am_SubSlave==0))
        #endif        
            CheckModuli_Sec();

        Me.EnableHiSpeedModBusTouch_Sec = K_EnableHiSpeedModBusTouch_Sec;
        Me.EnableHiSpeedMyBus_Sec = K_EnableHiSpeedMyBus_Sec;

        Me.ModBusDeviceDetected = 0; //Inizializzo x sicurezza

        for(temp=0; temp<=K_DEVICE_MODBUS_NUMBER; temp++)
            SearchModbusDeviceIndex(&temp);       // Ricerca del device modbus collegato: 0= NoDevice connected, 1=Touch Exor/Carel connected, 2=Touch Syxthsense connected, 3=NetBuilding Panel connected
        
        if(Me.NetBSelect)
        {
             Me.MaxFanVentil_Mode = K_DefaultFanSpeed_NetB;
             Me.ModDeviceType = K_ModDeviceNetB;
        }
        else if (Me.SyxtSSelect)        
        {
             Me.MaxFanVentil_Mode = K_DefaultFanSpeed_SyxtS;
             Me.ModDeviceType = K_ModDeviceSyxtS;
        }
        else
        {
             Me.MaxFanVentil_Mode = K_DefaultFanSpeed;
             if(Touch[0].OnLine)
                Me.ModDeviceType = K_ModDeviceExor;
             else
                Me.ModDeviceType = K_ModDeviceNone;
        }
        
        
        
        /*
        // cerco i touch/NetB
        if(Me.NetBSelect)
        {
            Search_NetB();
            InitNetB();
        }
        else if (Me.SyxtSSelect)        
        {
            Search_SyxtS();
            InitSyxtS();
        }            
        else
        {	
            Search_Touch();
            InitTouch();
        }
        */
    }
#else
#if(K_Force_SubSlave_Abil==1)
    #if K_DEVELOP==1
    if(Me.My_Address!=0 && (Me.I_Am_SubSlave==0))
    #endif        
        CheckModuli_Sec();
#endif
    Me.MaxFanVentil_Mode = K_DefaultFanSpeed;
    Me.ModDeviceType = K_ModDeviceNone;    
#endif
    
//#if(K_DiffPress_Fan_Control==1)
//	RefreshVentil.Time = 1500;
//#else
	RefreshVentil.Time = 500;
//#endif
	RefreshVentil.TimeOut = 0;
	RefreshVentil.Value = 0;
	RefreshVentil.Enable = 1;

	RefreshFineVentil.Time = 5;
	RefreshFineVentil.TimeOut = 0;
	RefreshFineVentil.Value = 0;
	RefreshFineVentil.Enable = 1;

	RefreshNetBRead.Time = 100;	// Lettura pannelli NetBuilding ogni 100mS
	RefreshNetBRead.TimeOut = 0;
	RefreshNetBRead.Value = 0;
	RefreshNetBRead.Enable = 1;

	TimerSystemCycle.Time = 15000;					// Tempo di calcolo tempo di ciclo programma (MAX = 15000 mSec.)
	TimerSystemCycle.Enable = 0;
	TimerSystemCycle.TimeOut = 0;					// resetto il timer
	TimerSystemCycle.Value = 0;

	TimerExecPID.Time = K_TimeExecPID;			// Tempo in mS per esecuzione regolatore PID per controllo Valvola espansione
	TimerExecPID.TimeOut = 0;					// resetto il timer
	TimerExecPID.Value = 0;
	TimerExecPID.Enable = 1;
    
	TimerComunicationError.Value =0;
	TimerComunicationError.Time = 60;					// Secondi
	TimerComunicationError.TimeOut = 0;
	TimerComunicationError.Enable =1;    

    TimerBusMergeMode.Value = 0;
	TimerBusMergeMode.Time = 10;                        // Secondi
	TimerBusMergeMode.TimeOut = 0;
	TimerBusMergeMode.Enable = 0;   

    TimerToggleComunication_Sec.Value = 0;
	TimerToggleComunication_Sec.Time = 1000;    //mSec
	TimerToggleComunication_Sec.TimeOut = 0;
	TimerToggleComunication_Sec.Enable = 1;    
        
}

 
 void Hw_Test(void)
 {           
	unsigned int Split_Exp_Valve_Perc;     
    char TouchAddres = Add_TouchRoom;
    int OutCnt=0;
    int StopCnt=0;
	unsigned int temp;    
	static int TrigTimer=0;
    
    unsigned int TimeOpen; 
    unsigned int TimeClosed;
    unsigned int TimeStart=20;   
            
    CheckEEPROM_ReadWrite();
    //if(Me.Error.EEPROM_ReadWrite == 1)
      // Aggiungere segnalazione EEPROM guasta con lampeggio LED (da rivedere secondo nuovo schema di lampeggi)
    
    // ------------------------------------------------------------------------------------
    // Testo tutta la EEPROM con scritture e letture (8 banchi da 256 byte = 2048byte = 1024 Word -> 0x800)
    // ------------------------------------------------------------------------------------
    CheckEEPROMCell();
    
    InitEEPROM_Retention();      

    
    if(Me.NetBSelect)       // Se address==128
    {
        TimeOpen = 5; 
        TimeClosed = 5;        
        TimerDisableValve.Time = 1;     // Tempo di rilascio corrente driver valvola
    }
    else                        // Se address==0
    {
        TimeOpen = 5; 
        TimeClosed = 40;        
        TimerDisableValve.Time = 1;     // Tempo di rilascio corrente driver valvola
    }
    
    TimerStartUp.Time = TimeStart; 
    TimerStartUp.TimeOut=0;
    TimerStartUp.Value=0;
    TimerStartUp.Enable = 0;

    
    InitValveRangeValue();
    CalculateValveRangeValue();
    
    
    while(1)
    {
		// leggo le sonde
		//AD_Read();    spostato in interrupt Timer 1

		//WorkValvola();			        
        
		// elaboro le letture
		Me.Temperature.Liquid = Steinhart(Adc_Temp[2], 1);	// si utilizza la curva di linearizzazione Steinhart
 		Me.Temperature.Gas = Steinhart(Adc_Temp[1], 1);
		Me.Temperature.AirOut = Steinhart(Adc_Temp[3], 1);
		Me.Temperature.Ambient = Steinhart(Adc_Temp[4], 2); 	
        
		Me.Evap_Press = Steinhart(Adc_Temp[8], 1);				// travaso valori ADC n.6
		Me.Evap_Temp = Steinhart(Adc_Temp[9], 1);				// e n.7 in Press_Big & Small per visualizzazione su Touch (solo x test HW)

		Me.Pres_Big = Pressure_Read_45(Adc_Temp[5]);			//Adc_P_Grande;    
		Me.Pres_Small = Pressure_Read_45(Adc_Temp[6]);		//Adc_P_Piccolo;	
       		      

        // invio dati su touch per split
        temp = Me.ValveLim.ValvolaAperta;
        Split_Exp_Valve_Perc = (unsigned int)( ((float)Me.ExpValve_Act / (float)temp) * 100.0); 
        
		WriteModRegSec (TouchAddres, REG_TOUCH_PICCOLO_SPLIT_1, Me.Temperature.Liquid);		
		WriteModRegSec (TouchAddres, REG_TOUCH_GRANDE_SPLIT_1, Me.Temperature.Gas);		
		WriteModRegSec (TouchAddres, REG_TOUCH_ARIA_INGRESSO_SPLIT_1, Me.Temperature.Ambient);		
		WriteModRegSec (TouchAddres, REG_TOUCH_ARIA_USCITA_SPLIT_1, Me.Temperature.AirOut);		
		WriteModRegSec (TouchAddres, REG_TOUCH_AD5_AUX1, Me.Evap_Press);		
		WriteModRegSec (TouchAddres, REG_TOUCH_AD6_AUX2, Me.Evap_Temp);	
        
		WriteModRegSec (TouchAddres, REG_TOUCH_PRESSIONE_G_SPLIT_1, Me.Pres_Big);		
		WriteModRegSec (TouchAddres, REG_TOUCH_PRESSIONE_P_SPLIT_1, Me.Pres_Small);		
        
		WriteModRegSec (TouchAddres, REG_TOUCH_STATO_VALVOLA_SPLIT_1, Split_Exp_Valve_Perc);	        
		WriteModRegSec (TouchAddres, REG_TOUCH_VENTILATORE_SPLIT_1, Me.Fan_Tacho);	          
        
		//invio dati su touch per motore
		InsertRegSec(1, EngineBox.HoursOfLife);				// REG_TOUCH_HOURS_OF_LIFE v8.4.16 -> invio ore di vita unita master x diagnostica
		InsertRegSec(2, EngineBox.Temp_Mandata);				// REG_TOUCH_MANDATA_COMPRESSORE
		//InsertRegSec(2, EngineBox.Temp_Compressor_Output_C2);				// REG_TOUCH_MANDATA_COMPRESSORE
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


        // Check IO Opto
        if(StopCnt==0) OutCnt++;
        if(OutCnt>2) OutCnt=0;     
        
        switch(OutCnt)
        {
            case 0:
                LATDbits.LATD8 = 1;     // Dig OUT n.1
                LATDbits.LATD9  = 0;    // Dig OUT n.2
                LATDbits.LATD13 = 0;    // Dig OUT n.3 
                if(!iDigiIn1) StopCnt=1;
                else  StopCnt=0;
                break;
            case 1:
                LATDbits.LATD8 = 0;     // Dig OUT n.1
                LATDbits.LATD9  = 1;    // Dig OUT n.2
                LATDbits.LATD13 = 0;    // Dig OUT n.3              
                if(!iDigiIn2) StopCnt=1;
                else  StopCnt=0;                
                break;                
            case 2:
                LATDbits.LATD8 = 0;     // Dig OUT n.1
                LATDbits.LATD9  = 0;    // Dig OUT n.2
                LATDbits.LATD13 = 1;    // Dig OUT n.3 
                if(!iDigiIn3) StopCnt=1;
                else  StopCnt=0;                
                break;                
        }
  		
    
                
        // Gestione valvola Aperta/Chiusa
/* */        
        
        
        TrigTimer = (Me.ExpValve_Act==Me.ValveLim.ValvolaAperta && Me.ExpValve_Req==Me.ValveLim.ValvolaAperta) ||    
                    (Me.ExpValve_Act==Me.ValveLim.ValvolaChiusa && Me.ExpValve_Req==Me.ValveLim.ValvolaChiusa);
        
        if(TrigTimer)
            TimerStartUp.Enable = 1;
        
        
        if(Me.ExpValve_Act==Me.ValveLim.ValvolaAperta && TimerStartUp.TimeOut==1) //20)
        {
            Me.ExpValve_Req=Me.ValveLim.ValvolaChiusa; //20;
			TimerStartUp.Time = TimeClosed; //5;				// 1Sec. OLD
			TimerStartUp.TimeOut=0;
			TimerStartUp.Value=0;
            TimerStartUp.Enable = 0;
        }
        if(Me.ExpValve_Act==Me.ValveLim.ValvolaChiusa && TimerStartUp.TimeOut==1)
        {
            Me.ExpValve_Req=Me.ValveLim.ValvolaAperta;
			TimerStartUp.Time = TimeOpen; //5;				// 1Sec. OLD
			TimerStartUp.TimeOut=0;
			TimerStartUp.Value=0;            
            TimerStartUp.Enable = 0;
        }
 

        // Gestione Ventola a velocità 3 e check Tacho
        Me.Pwm_Req = PWMValue[2]; //K_FanSpeed1
        VariazionePWM();        
		//RefreshTacho();			// elaboro il segnalo tacho

        if(Me.Ventil_Selection==K_EBM_24V)    
            Me.Fan_Tacho = TachoTMR/3 * 60;			// TachoTMR = 3 impulsi x giro contati in 1000mS / 3 * 60 sec. -> giri al min.
        else if(Me.Ventil_Selection==K_GR22_230V || Me.Ventil_Selection==K_GR25_230V)    
            Me.Fan_Tacho = TachoTMR * 60;			// TachoTMR = 1 impulso x giro contati in 1000mS * 60 sec. -> giri al min.
        
        //Me.Fan_Tacho = (TachoTMR * 60)/3;			// TachoTMR = 3 impulsi x giro contati in 1000mS / 3 * 60 sec. -> giri al min.

        
        if(Me.Fan_Tacho==0)
        {
            LATDbits.LATD8 = 1;     // Dig OUT n.1
            LATDbits.LATD9 = 1;     // Dig OUT n.2
            LATDbits.LATD13 = 1;    // Dig OUT n.3   
        }


        ErrorReport();
    }
       
 
 }
 
 
#ifdef UART3_ON 
 void Hw_Test_PLX_DAQ(void)
 {           
    unsigned int TimeOpen; 
    unsigned int TimeClosed;
    unsigned int TimeStart=20;  
    
    int TestNumber=-1;
    int openvalvola=0;    
    
    CheckEEPROM_ReadWrite();
    
    // ------------------------------------------------------------------------------------
    // Testo tutta la EEPROM con scritture e letture (8 banchi da 256 byte = 2048byte = 1024 Word -> 0x800)
    // ------------------------------------------------------------------------------------
    //CheckEEPROMCell();  //Commentato. ora il test della EEPROM è solo sui settori realmente utilizzati. (vedi routine sotto)
    // ------------------------------------------------------------------------------------
    // Testo solo le pagine della EEPROM con scritture e letture (2 banchi da 256 byte = 512byte = 256 Word)
    // ------------------------------------------------------------------------------------
    CheckEEPROMCellSector(); //Check dei settori della EEprom utilizzati.
    
    InitEEPROM_Retention();      
    
    
    
    My_Address = 1;
    Me.My_Address = 0;      // Se DIPSW==0 mi attribuisco l'indirizzo 1 per il collaudo del BUS
    
    if(Me.NetBSelect)       // Se address==128
    {
        TimeOpen = 5; 
        TimeClosed = 5;        
        TimerDisableValve.Time = 1;     // Tempo di rilascio corrente driver valvola
    }
    else                        // Se address==0
    {                                   // 7,8Sec = 60Hz      15,6Sec = 30Hz
        TimeOpen = 5;         // 18,2sec.PAUSA + 7,8sec.MOVIMENTO = 26 sec (8sec -> 30% duty cycle)
        TimeClosed = 40;       // 36,4sec.PAUSA + 15,6sec.MOVIMENTO = 52 sec (15,6sec -> 30% duty cycle) 
        TimerDisableValve.Time = 1;    // Tempo di rilascio corrente driver valvola
    }
    
    TimerStartUp.Time = TimeStart; 
    TimerStartUp.TimeOut=0;
    TimerStartUp.Value=0;
    TimerStartUp.Enable = 0;

    
    InitValveRangeValue();
    CalculateValveRangeValue();
    
    Touch[0].OnLine = 0;
    Touch[0].Enable = 0;
    Touch[0].OffLine = 0;
            
    while(1)
    {
		// leggo le sonde
		//AD_Read();    spostato in interrupt Timer 1

		//WorkValvola();			// Cari
        
		// elaboro le letture
		Me.Temperature.Liquid = Steinhart(Adc_Temp[2], 1);	// si utilizza la curva di linearizzazione Steinhart
 		Me.Temperature.Gas = Steinhart(Adc_Temp[1], 1);
		Me.Temperature.AirOut = Steinhart(Adc_Temp[3], 1);
		Me.Temperature.Ambient = Steinhart(Adc_Temp[4], 2); 	
        
		Me.Evap_Press = Steinhart(Adc_Temp[8], 1);				// travaso valori ADC n.6
		Me.Evap_Temp = Steinhart(Adc_Temp[9], 1);				// e n.7 in Press_Big & Small per visualizzazione su Touch (solo x test HW)

		Me.Pres_Big = Pressure_Read_45(Adc_Temp[5]);			//Adc_P_Grande;    
		Me.Pres_Small = Pressure_Read_45(Adc_Temp[6]);		//Adc_P_Piccolo;	
        
         
        
        //SendDataToTouch();

        CheckIO();    
                

        //PutDataForUART3_CheckBoard(10);         // Leggo il numero base per costruire il nome del file per il salvataggio del file a fine procedura.....
      
                
        switch(TestNumber)
        {
            case -1:                                     // Temperature Value Check
                PutDataForUART3_CheckBoard(TestNumber);         // Leggo il numero base per costruire il nome del file per il salvataggio del file a fine procedura.....
                if(SaveNumber>0)
                    TestNumber++;
                break;
            case 0:                                     // Temperature Value Check
                //....
                //....
                //PutDataForUART3_CheckBox(TestNumber);
                PutDataForUART3_CheckBoard(TestNumber);
                if(ServiceBusBufferRxU3.CheckBox[TestNumber]==1)
                    TestNumber++;
                break;
            case 1:                                     // Fan Speed Check
                //....
                //....
                if(openvalvola==0)
                {
                    openvalvola=1;
                    OpenAllValvola();        
                }                
                CheckFanSpeed();
                PutDataForUART3_CheckBoard(TestNumber);
                if(ServiceBusBufferRxU3.CheckBox[TestNumber]==1)
                    TestNumber++;
                break;
            case 2:                                     // Valve Open/Close Check (Pressure GAS)
                //....
                //....
                openvalvola = 0;
                OC1R = 0;          // Spengo FAN e aggiorno il modulo OC
                // Gestione valvola Aperta/Chiusa        
                ValveOpenCloseCheck(TimeOpen, TimeClosed);      
                PutDataForUART3_CheckBoard(TestNumber);
                if(ServiceBusBufferRxU3.CheckBox[TestNumber]==1)
                    TestNumber++;
                break;
            case 3:                                     // Probe Temp Position Check (Gas)
                //....
                //....
                if(openvalvola==0)
                {
                    openvalvola=1;
                    OpenAllValvola();        
                }
                PutDataForUART3_CheckBoard(TestNumber);
                if(ServiceBusBufferRxU3.CheckBox[TestNumber]==1)
                    TestNumber++;
                break;
            case 4:                                     // Probe Temp Position Check (Liquid)
                //....
                //....
                PutDataForUART3_CheckBoard(TestNumber);
                if(ServiceBusBufferRxU3.CheckBox[TestNumber]==1)
                    TestNumber++;
                break;
            case 5:                                     // Probe Temp Position Check (Outlet))
                //....
                //....
                PutDataForUART3_CheckBoard(TestNumber);
                if(ServiceBusBufferRxU3.CheckBox[TestNumber]==1)
                    TestNumber++;
                break;
            case 6:                                     // Probe Temp Position Check (inlet))
                //....
                //....
                PutDataForUART3_CheckBoard(TestNumber);
                if(ServiceBusBufferRxU3.CheckBox[TestNumber]==1)
                    TestNumber++;
                break;
            case 7:                                         // Touch Comunication Check
                //....
                //....
                Resume_TouchOffLine(&CntResumeTouch);	// tento di ripescare eventuali touch off line	
                CheckHotPlugTouch(&ScanTouchCounter);		// Controllo per eventuali Touch collegati a caldo....	                             
                SendDataToTouch();
                PutDataForUART3_CheckBoard(TestNumber);
                if(ServiceBusBufferRxU3.CheckBox[TestNumber]==1)
                    TestNumber++;
                break;
            case 8:                                         // Assign an address to the slave board
                //....
                //....
                PutDataForUART3_CheckBoard(TestNumber);
                if(ServiceBusBufferRxU3.CheckBox[TestNumber]==1)
                    TestNumber++;
                break;                
            case 9:
                //....
                //....
                // Salva il fglio di Excel con il nome xxxxx                
                PutDataForUART3_CheckBoard(TestNumber);
                //if(ServiceBusBufferRxU3.CheckBox[TestNumber]==1)
                TestNumber++;
                DelaymSec(4000);
                break;
            case 10:
                //....
                //....
                // Azzera tutti campi numerici e i grafici           
                PutDataForUART3_CheckBoard(TestNumber);
                //if(ServiceBusBufferRxU3.CheckBox[TestNumber]==1)
                TestNumber++;
                break;                
            case 11:
                //....
                //....
                // Salva il fglio di Excel con il nome xxxxx                
                //PutDataForUART3_CheckBoard(TestNumber);
                //if(ServiceBusBufferRxU3.CheckBox[TestNumber]==1)
                //TestNumber++;
                break;                      
        }        
        
        ErrorReport();
        
        
        //PutDataForUART3_CheckBoard(Me.Temperature.Liquid, Me.Temperature.Gas, Me.Temperature.AirOut, Me.Temperature.Ambient, EngineBox.Pressione_Gas_G, EngineBox.Pressione_Liq_P, TestNumber/*Me.CycleTime*/, Me.ExpValve_Act);

        if(TimerSystemCycle.Value < (K_MinTimeSystemCycle*2))               // Se il tempo di ciclo è minore di K_min... 
            DelaymSec((K_MinTimeSystemCycle*2) - TimerSystemCycle.Value);     // Attendo a vuoto fino a raggiungere il tempo minimo
        
		Me.CycleTime = TimerSystemCycle.Value;		// Memorizzo valore timer tempo di ciclo per successivo invio sul Touch x diagnostica
        
		TimerSystemCycle.TimeOut = 0;				// resetto il time out del timer Elapsed
		TimerSystemCycle.Value = 0;					// resetto il time value del timer Elapsed
        

    } //while(1)
       
 
 }
#endif 
 
 
void SendDataToTouch(void)
{
	unsigned int Split_Exp_Valve_Perc;     
    char TouchAddres = Add_TouchRoom;
	unsigned int temp;    

    // invio dati su touch per split
    temp = Me.ValveLim.ValvolaAperta;
    Split_Exp_Valve_Perc = (unsigned int)( ((float)Me.ExpValve_Act / (float)temp) * 100.0); 

    ReadModRegSec (TouchAddres, REG_TOUCH_PAGE, &(Touch[0].Current_Page));

    if(Touch[0].Current_Page==0)
        //WriteModRegSec (TouchAddres, REG_TOUCH_PAGE, 10);
        InitTouch();
    
    
    WriteModRegSec (TouchAddres, REG_TOUCH_PICCOLO_SPLIT_1, Me.Temperature.Liquid);		
    WriteModRegSec (TouchAddres, REG_TOUCH_GRANDE_SPLIT_1, Me.Temperature.Gas);		
    WriteModRegSec (TouchAddres, REG_TOUCH_ARIA_INGRESSO_SPLIT_1, Me.Temperature.Ambient);		
    WriteModRegSec (TouchAddres, REG_TOUCH_ARIA_USCITA_SPLIT_1, Me.Temperature.AirOut);		
    WriteModRegSec (TouchAddres, REG_TOUCH_AD5_AUX1, Me.Evap_Press);		
    WriteModRegSec (TouchAddres, REG_TOUCH_AD6_AUX2, Me.Evap_Temp);	

    WriteModRegSec (TouchAddres, REG_TOUCH_PRESSIONE_G_SPLIT_1, Me.Pres_Big);		
    WriteModRegSec (TouchAddres, REG_TOUCH_PRESSIONE_P_SPLIT_1, Me.Pres_Small);		

    WriteModRegSec (TouchAddres, REG_TOUCH_STATO_VALVOLA_SPLIT_1, Split_Exp_Valve_Perc);	        
    WriteModRegSec (TouchAddres, REG_TOUCH_VENTILATORE_SPLIT_1, Me.Fan_Tacho);	
    
    WriteModRegSec (Add_TouchRoom, REG_TOUCH_COUNTER_COMUNICATION,8); //3);
 

    //invio dati su touch per motore
    InsertRegSec(1, EngineBox.HoursOfLife);				// REG_TOUCH_HOURS_OF_LIFE v8.4.16 -> invio ore di vita unita master x diagnostica
    InsertRegSec(2, EngineBox.Temp_Mandata);				// REG_TOUCH_MANDATA_COMPRESSORE
    //InsertRegSec(2, EngineBox.Temp_Compressor_Output_C2);				// REG_TOUCH_MANDATA_COMPRESSORE
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

} 
 
 
void CheckIO(void)
{
    static int OutCnt=0;
    static int StopCnt=0;

    // Check IO Opto
    if(StopCnt==0) OutCnt++;
    if(OutCnt>2) OutCnt=0;

    
    switch(OutCnt)
    {
        case 0:
            LATDbits.LATD8 = 1;     // Dig OUT n.1
            LATDbits.LATD9 = 0;     // Dig OUT n.2
            LATDbits.LATD13 = 0;    // Dig OUT n.3   
            if(!iDigiIn1) StopCnt=1;
            else  StopCnt=0;
            break;
        case 1:
            LATDbits.LATD8 = 0;     // Dig OUT n.1
            LATDbits.LATD9 = 1;     // Dig OUT n.2
            LATDbits.LATD13 = 0;    // Dig OUT n.3               
            if(!iDigiIn2) StopCnt=1;
            else  StopCnt=0;                
            break;                
        case 2:
            LATDbits.LATD8 = 0;     // Dig OUT n.1
            LATDbits.LATD9 = 0;     // Dig OUT n.2
            LATDbits.LATD13 = 1;    // Dig OUT n.3   
            if(!iDigiIn3) StopCnt=1;
            else  StopCnt=0;                
            break;                
    }
}
  
 
void ValveOpenCloseCheck(unsigned int TimeOpen, unsigned int TimeClosed)
{        
	static int TrigTimer=0;    

    TrigTimer = (Me.ExpValve_Act==Me.ValveLim.ValvolaAperta && Me.ExpValve_Req==Me.ValveLim.ValvolaAperta) ||    
                (Me.ExpValve_Act==Me.ValveLim.ValvolaChiusa && Me.ExpValve_Req==Me.ValveLim.ValvolaChiusa);

    if(TrigTimer)
        TimerStartUp.Enable = 1;


    if(Me.ExpValve_Act==Me.ValveLim.ValvolaAperta && TimerStartUp.TimeOut==1) //20)
    {
        Me.ExpValve_Req=Me.ValveLim.ValvolaChiusa; //20;
        TimerStartUp.Time = TimeClosed; //5;				// 1Sec. OLD
        TimerStartUp.TimeOut=0;
        TimerStartUp.Value=0;
        TimerStartUp.Enable = 0;
    }
    if(Me.ExpValve_Act==Me.ValveLim.ValvolaChiusa && TimerStartUp.TimeOut==1)
    {
        Me.ExpValve_Req=Me.ValveLim.ValvolaAperta;
        TimerStartUp.Time = TimeOpen; //5;				// 1Sec. OLD
        TimerStartUp.TimeOut=0;
        TimerStartUp.Value=0;            
        TimerStartUp.Enable = 0;
    }
 
        
}
 
 
void CheckFanSpeed(void)
{
    static int SpeedSet=0;
    static int Cnt=0;
    
    
    // Gestione Ventola a velocità 3 e check Tacho
    //Me.Pwm_Req = PWMValue[SpeedSet]; //K_FanSpeed1
    //VariazionePWM();

    Me.Pwm_Value = PWMValue[SpeedSet];
    
    if(Me.Ventil_Selection==K_ZIEHL_ABEGG_ModBus)
    {
        RefreshZiehgAbbModbusFan(&Inverter, Add_Fan_ZihegAbb_ModBus);
    }
    else if(Me.Ventil_Selection==K_EBM_3200RPM_ModBus)
    {
        RefreshEBMModbusFan(&Inverter, Add_Fan_EBM_ModBus);
    }        
    else
    {
        OC1R = ((Me.Pwm_Value & 0x00FF) <<4);		// aggiorno il modulo OC
    }
    //RefreshTacho();			// elaboro il segnalo tacho

    if(Me.Ventil_Selection==K_EBM_24V)    
        Me.Fan_Tacho = TachoTMR/3 * 60;			// TachoTMR = 3 impulsi x giro contati in 1000mS / 3 * 60 sec. -> giri al min.
    else if(Me.Ventil_Selection==K_GR22_230V || Me.Ventil_Selection==K_GR25_230V)    
        Me.Fan_Tacho = TachoTMR * 60;			// TachoTMR = 1 impulso x giro contati in 1000mS * 60 sec. -> giri al min.

    //Me.Fan_Tacho = (TachoTMR * 60)/3;			// TachoTMR = 3 impulsi x giro contati in 1000mS / 3 * 60 sec. -> giri al min.

    if(Me.Fan_Tacho==0)
    {
            LATDbits.LATD8 = 1;     // Dig OUT n.1
            LATDbits.LATD9 = 1;     // Dig OUT n.2
            LATDbits.LATD13 = 1;    // Dig OUT n.3
    }
    
    
    if(++Cnt>=8)
    {
        if((SpeedSet==0&&Cnt>=20) || (SpeedSet>0 && SpeedSet<8))
        {
            SpeedSet++;
            Cnt=0;        
        }
        if(SpeedSet==8&&Cnt>=20)
        {
            SpeedSet=0;
            Cnt=0;
        }
    }
}  
 



  
void InitADConversion(void)
{
		AD_Read();

		// elaboro le letture
		Me.Temperature.Liquid = Steinhart(Adc_Temp[2], 1);	// si utilizza la curva di linearizzazione Steinhart
 		Me.Temperature.Gas = Steinhart(Adc_Temp[1], 1);
		Me.Temperature.AirOut = Steinhart(Adc_Temp[3], 1);
		Me.Temperature.Ambient = Steinhart(Adc_Temp[4], 2); 	
		
		Me.Pres_Big = Pressure_Read_45(Adc_Temp[5]);			//Adc_P_Grande;    
		//Me.Pres_Small = Pressure_Read_34(Adc_Temp[6]);		//Adc_P_Piccolo;	
		Me.Pres_Small = Pressure_Read_45(Adc_Temp[6]);		//Adc_P_Piccolo;	  
        
		Me.Fan_Power = Adc_Temp[7];	
        
		Me.Supply_Voltage = Adc_Temp[10];	
			    
    
}


void BootLoaderAutoReset(void)
{

    if(Me.SetSubSlaveReset==1)                       // Se sono in modalità "BUS Merge"
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
  
        
        //ReplyResult = Send_WriteReg_Sec(Me.SubSlaveAddressAutoreset, Check, 0, 0, MaxChkWait, MaxChkRetries);   // Controllo se SubSlave esiste....
        ReplyResult = FuncCheckReg_Sec(Me.SubSlaveAddressAutoreset);                                            // Controllo se SubSlave esiste....

        if(ReplyResult==0)         // Se il SubSlave esiste e quindi mi ha risposto.....
        {
            ReplyResult = FuncWriteReg_Sec(Me.SubSlaveAddressAutoreset, REG_BOARD_AUTORESET, 0x1234, 0);         // Invio comando "Autoreset" a SubSLave selezionato

            Change_ProtocolBusSlave(K_MyBus_Baud57600, K_MyBus);                             // Imposto protocollo 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
            
            Me.GoBusMergeMode = 1;
            TimerBusMergeMode.Enable = 1;
            while(TimerBusMergeMode.TimeOut==0)                                                             // e rimango in loop fino a timeout comunicazioni BUS1<->BUS2
            {
                oErrLed = !oErrLed;
                oPwrLed = !oPwrLed;
                DelaymSec(100);
            }
            TimerBusMergeMode.TimeOut=0;
            TimerBusMergeMode.Enable = 0;
            TimerBusMergeMode.Value = 0;
            Me.GoBusMergeMode = 0;                  // Resetto la modalità "BusMergedMode"
        }

        Me.SetSubSlaveReset = 0;                // Resetto il flag di reset SubSlave           
        Me.SubSlaveAddressAutoreset = 0;                    
    } 

    if(Me.AutoresetEnabled && (Me.AutoresetMagicNumber==K_MagicNumber))
    {
        asm ("RESET");                          // Software Reset Instruction          
    }

}


void GlobalIntReadBus(void)
{
	int RxChar = 0; 
    
	while(U1STAbits.URXDA)                          // Receive Buffer Data Available bit (read-only)
	{       
        RxChar = U1RXREG;
        
        IntReadBus(&BufferRx, RxChar);
        if(ForceU1MyBUS==0)
            Mod_IntReadBus(RxChar);
    }
    
}

void OkDataProcess(void)
{
 	static int cnt=0;
	static int blocco=0;

    if((DiagnosticSplit.Ready_CMD_Address==My_Address) && (blocco==1))
    {		
        DiagnosticSplit.DataProcess = 1;
    }
    if(DiagnosticSplit.Ready_CMD_Address==My_Address&&blocco==0)
        cnt+=1;
    if(cnt>=4)//15)
    {
        blocco=1;
        cnt=0;
    }
    if(DiagnosticSplit.Ready_CMD_Address!=My_Address)
    {
        DiagnosticSplit.DataProcess = 0;
        blocco=0;
    }
}