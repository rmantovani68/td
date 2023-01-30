//----------------------------------------------------------------------------------
//	Progect name:	Test 24FJ64A002
//	Device:			PIC24FJ64A002 @ 20Mhz
//	Auitor:			Emanuele
//	Date:			23/01/2011
//	Description:	Test del pic 24FJ64A002
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
#include <ctype.h>
#include "ConfigDevice.h"
#include "DefinePeriferiche.h"
#include "EEPROM.h"
#include "usart.h"
#include "Timer.h"
#include "ProtocolloComunicazione.h"
#include "Driver_Comunicazione.h"
#include "Driver_ModBus.h"
#include "ProtocolloModBus.h"
#if (K_AbilMODBCAN==1)               
 #include "Driver_ModBus_U4.h"
 #include "ProtocolloModBus_U4.h"
#endif //#if (K_AbilMODBCAN==1)   
#include "ADC.h"
#include "DAC.h"
#include "Core.h"
#include "HWSelection.h"
#include "FWSelection.h"
#include "delay.h"
#include "I2C.h"
#include "soft_i2c.h"
#include "i2c_LCD_Driver.h"
#include "LCD_ScreenInfo.h"
#include "SevenSegDisplay.h"
#include "ServiceComunication.h"
#include "PID.h"
#include "Interrupt.h"
#if (K_AbilMODBCAN==1)  
#include "ModBusCAN.h"
#endif


//----------------------------------------------------------------------------------
//	DEFINE
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//	Variabili globali
//----------------------------------------------------------------------------------
VariabiliGlobaliADC();
VariabiliGlobaliDAC();
VariabiliGlobaliDriverComunicazione();
VariabiliGlobaliDriverModBus();
VariabiliGlobaliProtocolloComunicazione();
VariabiliGlobaliProtocolloModBus();
VariabiliGlobaliEEPROM();
VariabiliGlobaliPID();
VariabiliGlobaliCore();
GlobalVarServiceComunication();
#if (K_AbilMODBCAN==1)  
VariabiliGlobaliDriverModBus_U4();
VariabiliGlobaliProtocolloModBus_U4();
VariabiliGlobaliModBusCAN();
#endif //#if (K_AbilMODBCAN==1)   

unsigned char           CurUsedProtocolId = 0;      // Id del protocollo correntemente utilizzato su UART1 / UART2
///unsigned char           CurUsedProtocolId_BUSS = 0; // Id del protocollo correntemente utilizzato su UART2

unsigned                ServiceSlaveMode = 0;		// Flag se indica il service mode, nel quale il master diventa uno slave
unsigned                InputProgrammingMode = 0;		// Flag con appoggiato il service mode da touch screen e da pulsante se abilitato
char                    CntProgCyclMy = 0;			// utilizzato per il resume dei moduli offline
char                    ScanBoardCounter = 1;		// utilizzato per il check dei moduli eventualmente collegati a caldo
//char                  CntProgCyclMod = 0;			// utilizzato per il resume dei moduli offline
volatile TypTimer		DividerSecondi;				// Timer per la divisione in secondi
volatile TypTimer		TimerElaborazioneC1;		// Timer Elaborazione1
volatile TypTimer		TimerElaborazioneC2;		// Timer Elaborazione2
volatile TypTimer		TimerPwrLed;				// Timer Led Power
volatile TypTimer 		TmrLastRequest;				// Timer per la verifica della comunicazione
volatile TypTimer		TimerSystemCycle;			// Timer per il calcolo del tempo di ciclo programma
//volatile TypTimer 		TimerRstComErr;
//volatile TypTimer 		TimerModRstComErr;
volatile TypTimer		TimerSwitchCompressor;		// Timer per time switch secondo compressore
volatile TypTimer		TimerIntegrateCompressor2;	// Timer per integrazione secondo compressore
volatile TypTimer		TimerDeintegrateCompressor2;// Timer per deintegrazione secondo compressore
volatile TypTimer 		TimerComunicationError;		// Timer per gestione counter errori di comunicazione
volatile TypTimer		TimerHwReset;				// Timer per gestire Teleruttore di sgancio potenza per ripristino allarme "MOFF" anomalo.
volatile TypTimer		TimerRestartAfterBlackOut;	// Timer per gestire la pausa alla riaccensione da blackout se i compressori erano in funzione 
volatile TypTimer       TimerSwitchPump;            // Timer per Switch pompe
volatile TypTimer       TimerTrigPump;				// Timer per freerun switch pompe
volatile TypTimer       TimerPostPump;				// Timer per freerun switch pompe
volatile TypTimer       TimerValve_On_Demand_Limit;	// Timer per calcolare il limite valvola
volatile TypTimer       TimerValve_On_Demand_Switch;// Timer per calcolare il tempo di switch della valvola
volatile TypTimer       TimerCoolingSuperHeatErr;
volatile TypTimer       TimerExecPID;
volatile TypTimer       TimerCoolWarmValveError;
volatile TypTimer       TimerInverterPersError_C1;
volatile TypTimer       TimerInverterPersError_C2;
volatile TypTimer       TimerInverterPersError_Pump;
volatile TypTimer       TimerSuperHeatError;
volatile TypTimer       TimerTemperatureProbeErrorBox1;
volatile TypTimer       TimerTemperatureProbeErrorBox2;
volatile TypTimer       TimerPressureProbeErrorBox;
volatile TypTimer       TimerCheckTemperatureProbe;
volatile TypTimer       TimerCheckPressureProbe;
volatile TypTimer       TimerExitProgrammingMode;
volatile TypTimer       TimerInVoltageInverterOk_C1;
volatile TypTimer       TimerInVoltageInverterOk_C2;

#if (K_AbilMODBCAN==1)
volatile TypTimer StableValueChangeHMI;         // Timer x gestione accettazione cambio valori da HMI Scheiber
#endif //#if (K_AbilMODBCAN==1)   


TypRTC      MyRTC;						// RTC di sistema
unsigned	RTC_Flag;					// RTc Flag di sistema
unsigned	ComunicationLost;			// comunicazione persa in modalità slave

volatile int CntComErr = 0;			
volatile int TimeOutComErr = 0;		
volatile int CRC_ComErr = 0;
volatile int CommandNotImplemented = 0;
volatile int ModCntComErr = 0;			
volatile int ModTimeOutComErr = 0;		
volatile int ModCRC_ComErr = 0;
volatile int ModCntComErr_U2 = 0;			
volatile int ModTimeOutComErr_U2 = 0;		
volatile int ModCRC_ComErr_U2 = 0;
#if (K_AbilMODBCAN==1)  
volatile int ModCntComErr_U4 = 0;			
volatile int ModTimeOutComErr_U4 = 0;		
volatile int ModCRC_ComErr_U4 = 0;
#endif //#if (K_AbilMODBCAN==1)   

#define K_MagicNumber 0x1234        // MagicNumber -> 4660
unsigned int StackOverflowWarning; // @ 0x0900; //0x069F; //0x0E20;


// timer	
volatile TypTimer TmrLastRequest;
volatile TypTimer TimerStartUp;					// Timer StartUp
//volatile TypTimer RefreshPanelRead;
//volatile TypTimer TimerSystemCycle;			// Timer per il calcolo del tempo di ciclo programma
//TypRTC	MyRTC;

//$ unsigned RTC_Flag = 1;
//volatile int SecDivider = 0;						// divisore per base tempo di 1 secondo
//volatile int HalfSecDivider = 0;						// divisore per base tempo di 1/2 secondo
//volatile int mSecDivider = 0;                       // divisore per base tempo di 100 mSec
//volatile int Elapsed = 0;						// divisore per base tempo di 1 secondo
//int Led_Divider = 0;					// divisore per il lampeggio del led
//char CntResumeTouch = 0;				// contatore per il resume dei touch
//char CntResumePanel = 0;				// contatore per il resume dei Panel
//char CntResumeSplit_Sec = 0;			// contatore per il resume degli split sul sec bus
//unsigned ComunicationLost;				// flag di comunicazione persa

//char ScanBoardCounter_Sec = 1;		// utilizzato per il check dei moduli eventualmente collegati a caldo


// Arghino
//int E2DataBuffer[0x400];
//int temp;

//----------------------------------------------------------------------------------
//	Prototipi 
//----------------------------------------------------------------------------------
void Init(void);
void Hw_Test(void);
void CheckEEPROM(void);
void UpdateRele(void);   
void UpdateDAC(void);
void Update7SegDisplay(void);
void ErrorReport(void);
void CheckWorkHours(void);
//void GlobalIntReadBus(void);


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
    //StackOverflowWarning = K_MagicNumber;
	
	Init();											// inizializzazione master
	
    PrintDigit('0', ON , OFF, OFF);
    
#if (K_DEBUG_LCD==1)    
    LCD_Info_Init();                                // Inizializza eventuale display LCD seriale I2C 4x20
#endif
    
	CntComErr = 0;
	TimeOutComErr =0;
	CRC_ComErr = 0;
	CommandNotImplemented = 0;
	ModCntComErr = 0;
	ModTimeOutComErr =0;
	ModCRC_ComErr = 0;    
	ModCntComErr_U2 = 0;
	ModTimeOutComErr_U2 =0;
	ModCRC_ComErr_U2 = 0;    
#if (K_AbilMODBCAN==1)      
	ModCntComErr_U4 = 0;
	ModTimeOutComErr_U4 =0;
	ModCRC_ComErr_U4 = 0;    
#endif //#if (K_AbilMODBCAN==1) 
    
	LoadAllE2_Data();								// recupero i settaggi dalla E^2	
	 
    
//-----------------------------------------------------------------------
// Gestione pausa alla riaccensione dopo un backout 
// (3 minuti se era acceso prima di backout)
//-----------------------------------------------------------------------	
	if(EngineBox[0].LastCompressorPowerStatus==1)		// Se il comp era acceso prima di blackout
	{
#if K_SIMULATION_WORK==1		
		TimerRestartAfterBlackOut.Time = 10;					// imposto il timer per attendere 10 sec.
#else
		TimerRestartAfterBlackOut.Time = K_Time_Restart_After_Blackout; //180; 				// imposto il timer per attendere 3 min.		
#endif
		TimerRestartAfterBlackOut.TimeOut = 0;
		TimerRestartAfterBlackOut.Value = 0;
		TimerRestartAfterBlackOut.Enable = 1;
		EngineBox[0].Error.MasterRestart = 1;
	}

	
		
//-----------------------------------------------------------------------
// Gestione pausa 5sec. alla partenza per dare tempo agli slave di essere 
// operativi con le comunicazioni
//-----------------------------------------------------------------------	
	
	PausaCompressore.Time = 5;					// altrimenti imposto il timer per attendere 5 sec	
	PausaCompressore.Enable = 1;

	while(PausaCompressore.TimeOut == 0)			// attendo 60 sec prima della scansione, per il risveglio dei touch
		continue;
	
	PausaCompressore.TimeOut =0;					// resetto il timer
	PausaCompressore.Value = 0;
	PausaCompressore.Enable = 0;	
	
	oErrLed = LED_OFF;									// Spengo LED Err
	

    CheckModuli();									// ricerca schede di espansione connesse	

	
	//------------------------------------------------------------
	// Comunicazioni con scheda Bridge per Init Data
	//------------------------------------------------------------
#if (K_AbilBridge1==1 || K_AbilBridge2==1)    
	//Room[1].Enable=1;
	//Room[1].OffLine=0;
	ReadDataFromSlaveMaster();						// Legge le impostazioni dallo Slave Master 
	
	DelaymSec(100);									// Pausa necessaria tra la fine delle comunicazioni in MyBus qui sopra
													// e il cambio a MODBUS @ 57600 per comunicare con il bridge altrimenti
#endif	
    
#if (K_AbilBridge1==1)    
    Bridge[0].PermanentOffLine = 1;
    Bridge[0].BridgeReady = 0;
	// DEBUG Init_Bridge(1);									// Controlla se presente scheda Bridge 1 e la inizializza
#else
    Bridge[0].PermanentOffLine = 0;
#endif	
#if (K_AbilBridge2==1)    
    Bridge[1].PermanentOffLine = 1;
    Bridge[1].BridgeReady = 0;
	// DEBUG Init_Bridge(2);		
#else
    Bridge[1].PermanentOffLine = 0;
#endif	 
    
#if (K_AbilMODBCAN==1)
    ModbCAN.PermanentOffLine = 1;
    ModbCAN.Ready = 0;
	// DEBUG Init_ModbCAN();		
#else
    //ModbCAN.PermanentOffLine = 0;
#endif //#if (K_AbilMODBCAN==1)  
    
	
    // Inizializzazioni Inverters Compressori/Pompa
    //----------------------------------
#if (K_AbilInverterInit==1)    
    InitInverterComp(&Comp_Inverter[0], K_MotorModel_Selection1, Add_Comp_Inverter1);
#if (K_AbilCompressor2==1)							// Se è presente il secondo inverter compressore....
#if(K_OnlyOneInverterComp2==0)
    InitInverterComp(&Comp_Inverter[1], K_MotorModel_Selection2, Add_Comp_Inverter2);    
#endif	
#endif	
#if (K_AbilPumpInverter==1)	
	InitInverterPump(&Comp_Inverter[2], K_PumpModel_Selection, Add_Pump_Inverter1);
#endif			

#endif    
    //----------------------------------	

	EngineBox[0].Inverter_Max_Power_Out = 255;		//DUALCOMP? Vedere come gestire potenza ripartita su due compressori!!!!!!!
	//EngineBox[1].Inverter_Max_Power_Out = 255;		//DUALCOMP? Vedere come gestire potenza ripartita su due compressori!!!!!!!

    Comp_Inverter[0].Max_Power_Inverter = (K_PowerMax_Available/10)/2;

    Room[k_Split_Master_Add].SuperHeat_Cool_Min_SetPoint = K_SetPoint_Cooling_SuperHeat;
    Room[k_Split_Master_Add].SuperHeat_Cool_Max_Value_Correct = K_Max_Correct_SuperHeat;

    EngineBox[0].TempCompressorHi = TempCompressorHI;
    EngineBox[0].TempRestartCompressorHi = TempRestarCompressorHI;

	TimerElaborazioneC1.Enable = 1;			// Parto con l'elaborazione v4.4.117
	TimerElaborazioneC2.Enable = 1;			
	TimerSystemCycle.Enable = 1;			// Parto con il calcolo del Cycle time 
    
    //Room[k_Split_Master_Add].Maintenance_Flag = 0;  
    //Room[k_Split_Master_Add].Maintenance_Set_Time = k_Max_HoursOfLife;

#if (K_EnableTesting==1)
	Hw_Test();
#endif	
	
    
    
    /*
    PrintDigit('2', ON , OFF, OFF);
    DelaymSec(2000);
    PrintDigit('A', ON , ON, OFF);
    DelaymSec(2000);
    PrintDigit('A', ON , OFF, OFF);
    DelaymSec(2000);
     */
   PrintDigit('h', ON , OFF, OFF);
    //DelaymSec(2000);


	while(1)
	{
        //dip da 1 a 3 = address
        //dip 4 non ancora utilizzato
        EngineBox[0].EraseAllEEPROM = ((~PORTA & 0x0010) != 0);             // Flag x attiuvare la funzione di Cancellazione della EEPROM (Dip Switch n.5 = ON)     
        EngineBox[0].ServiceMode = ((~PORTA & 0x0020) != 0);                // Service Mode (Dip Switch n.6 = ON)     
        EngineBox[0].ReverseDisplayDigit = ((~PORTA & 0x0040) != 0);		// Flag x rotazione display digit (Dip Switch n.7 = ON)     
        EngineBox[0].AllValveOpen100p = ((~PORTA & 0x0080) != 0);           // Flag x aprire tutte le valvole (utile quando si carica il GAS nell'impianto)
        
        if(EngineBox[0].EraseAllEEPROM==1)
        {
            PrintDigit('P', ON , OFF, ON);
            EraseEEPROM();
            LoadAllE2_Data();								// recupero i settaggi dalla E^2
            //LoadAllEEPROM_Debug(&E2DataBuffer[0]);
            while(EngineBox[0].EraseAllEEPROM==1)
            {
                oErrLed = !oErrLed;
                oPwrLed = oErrLed;                
                DelaymSec(200);
                EngineBox[0].EraseAllEEPROM = ((~PORTA & 0x0010) != 0);             // Flag x attiuvare la funzione di Cancellazione della EEPROM (Dip Switch n.1 = ON)                     
            }
        }
                
        //EngineBox[0].AbilSuperHeatBox = ((~PORTA & 0x0004) != 0);

        EngineBox[0].NoPayCastigationActivation = ((Room[k_Split_Master_Add].Maintenance_Flag & 0x0001) != 0); // Flag x attivare funzione di NoPay (Solo da touch screen)
        
		if(TimerRestartAfterBlackOut.TimeOut==1)		// Attesa alla ripartenza se compressori erano in funzione
		{
			TimerRestartAfterBlackOut.TimeOut = 0;
			TimerRestartAfterBlackOut.Enable = 0;
			EngineBox[0].LastCompressorPowerStatus = 0;
			EngineBox[0].Error.MasterRestart = 0;
		}		
		
		//GL Eliminato in v5.16.152!! x far lampeggiare correttamente il LED BLU in ServiceMode         TimerPwrLed.Enable = 0;
		// aggiorno gli ingressi digitali
		//@ NON SERVE DigitalInput = (PORTB & 0x3F)^0x3F;

        // leggo le sonde
		//AD_Read();    spostato sotto int timer1
        
		// aggiorno gli ingressi analogici
		
		EngineBox[0].Temperature.Compressor_Output = Steinhart(Adc_Temp[1], 2);             	
		EngineBox[0].Temperature.Compressor_Suction = Steinhart(Adc_Temp[2], 1); 
		EngineBox[0].Temperature.Condenser = Steinhart(Adc_Temp[3], 1);     
		EngineBox[0].Temperature.Sea_Water = Steinhart(Adc_Temp[4], 1);         
		EngineBox[0].Temperature.Liquid = Steinhart(Adc_Temp[5], 1);        
		
		EngineBox[1].Temperature.Compressor_Output = Steinhart(Adc_Temp[6], 2); 
	
        //EngineBox[0].Temperature.Gas = Steinhart(Adc_Temp_6, 1);     
        
        EngineBox[0].Temperature.Gas = EngineBox[0].Temperature.Compressor_Suction;            // Se sono in modalità Raffrescamento oppure OFF
        if(EngineBox[0].InverterRiscaldamento || EngineBox[0].GoRiscaldamento)  // Se sono in modalità Riscaldamento
            EngineBox[0].Temperature.Gas = EngineBox[0].Temperature.Compressor_Output;

		EngineBox[0].Pressure.Liquid = Pressure_Read(Adc_Temp[7]);                        
		EngineBox[0].Pressure.Gas = Pressure_Read(Adc_Temp[8]);             
#if(K_Condenser_Pressure_Simulated==1)
        EngineBox[0].Pressure.LiquidCond = EngineBox[0].Pressure.Liquid - K_Condenser_Pressure_Correct;
#else
        EngineBox[0].Pressure.LiquidCond = Pressure_Read(Adc_Temp[9]);
#endif   
           
/*
#if(K_AbilCompressor2==1)
        if((EngineBox[0].SuperHeat.ControlSuperHeat==0) && (EngineBox[0].SuperHeat.Value<(K_SetPointHeatingSuperHeat-100) || ((EngineBox[0].GoRiscaldamento==1)&&(EngineBox[1].EngineIsOn==0)) || ((EngineBox[1].GoRiscaldamento==1)&&(EngineBox[0].EngineIsOn==0))))
#else        
        if((EngineBox[0].SuperHeat.ControlSuperHeat==0) && (EngineBox[0].SuperHeat.Value<(K_SetPointHeatingSuperHeat-100) || (EngineBox[0].GoRiscaldamento==1)))
#endif
        {
            EngineBox[0].SuperHeat.ControlSuperHeat = 1;
            if(((EngineBox[0].GoRiscaldamento==1)&&(EngineBox[1].EngineIsOn==0)) || ((EngineBox[1].GoRiscaldamento==1)&&(EngineBox[0].EngineIsOn==0)))
                PID_SetOut(&SuperHeatPID, 0);
            else
                PID_SetOut(&SuperHeatPID, PressSeaWaterPID.iState);
        }
        else if (EngineBox[0].SuperHeat.Value>(K_SetPointHeatingSuperHeat+50))
        {
            EngineBox[0].SuperHeat.ControlSuperHeat = 0;
            PID_SetOut(&PressSeaWaterPID, SuperHeatPID.iState);          
        }
 */               
        //Argo Vaffa
        EngineBox[0].SuperHeat.ControlSuperHeat = 1;
        //Argo Vaffa
        PID_EngineBoxSuperHeat();        
        //PID_EngineBoxPressSeaWater();

#if(K_AbilCoolingSuperHeat==1)
        CoolingEngineBoxSuperHeat();
#endif        
        
		///TimerElaborazione.TimeOut = 1;			// Parto con l'elaborazione v4.4.117 -> ???????
		// v4.4.117 -> Aggiunta corretta gestione in Core.c CompressorCheck()
#if(K_Ing1_ModeSelection==1)
      InputProgrammingMode = EngineBox[0].ProgrammingModeEnable || iDigiIn1 || EngineBox[0].ServiceMode;
#else
      InputProgrammingMode = EngineBox[0].ProgrammingModeEnable || EngineBox[0].ServiceMode;
#endif
		TmrLastRequest.Enable = InputProgrammingMode;
		if(InputProgrammingMode && !ServiceSlaveMode)
		{            
#if (K_EnableHiSpeedMyBus==K_Baud115200)
            Change_Protocol_BUSM(K_MyBus_Baud115200);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#elif(K_EnableHiSpeedMyBus==K_Baud250000)     
            Change_Protocol_BUSM(K_MyBus_Baud250000);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#else
            Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
#endif	                                       
		}
		ServiceSlaveMode = InputProgrammingMode;
		TimerPwrLed.Enable = ServiceSlaveMode || EngineBox[0].Error.MasterRestart || RTC_Flag==1 || (EngineBox[0].NoPayCastigationActivation==1);
		if(ServiceSlaveMode)
		{
			if(ComunicationLost)
				TimerPwrLed.Time = 70;
			else
				TimerPwrLed.Time = 200;
			///RefreshE2_Data();	
            Change_Protocol_BUSM(K_MyBus_Baud57600);								// Imposto protocollo: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
            BootLoaderAutoReset();       //Importato da Slave. Inserire routine e controllare se presenti comandi MyBUS.            

            TimerExitProgrammingMode.Enable = ComunicationLost && EngineBox[0].ProgrammingModeEnable;
            if(!TimerExitProgrammingMode.Enable)
            {
                TimerExitProgrammingMode.Value = 0;
                TimerExitProgrammingMode.TimeOut = 0;
            }
            if(TimerExitProgrammingMode.TimeOut)
            {
                EngineBox[0].ProgrammingModeEnable = 0;
            }
		}
		else			
		{
			if(EngineBox[0].Error.MasterRestart)
				TimerPwrLed.Time = 40;
			else
            {
                if(EngineBox[0].NoPayCastigationActivation==0)
                    oPwrLed = LED_ON;
			}
            
			ResumeOffLine(&CntProgCyclMy);				// provo a riagganciare una scheda a giro
			CheckHotPlugSlave(&ScanBoardCounter);		// Controllo per eventuali schede collegate a caldo....						
			RefreshAllInPut();							// aggiorno gli ingressi
	
			// ********************* Elaborazione *********************
			//CompressorCheck(&TimerElaborazione);					// gestisco il compressore
			if(EngineBox[0].Error.MasterRestart==0)
				CompressorCheck();										// gestisco il/i compressore/i
			
			ErrorCheck();								// gestisco gli errori
			//CompressorAdjustment(&EngineBox[0]);		// gestico l'uscita del compressore
			PumpManagement();							// gestisco la pompa per il condensatore

			// ****** Gestione dati slave in diagnostica sul touch dello slave master *******	#v16		
			ReadDiagnosticSplitAddr(k_Split_Master_Add);// Leggo l'indirizzo dello slave da monitorare 
														// dall'unita slave Master (quella dotata di Touch)
														// che per convenzione ha indirizzo "1" (k_Split_Master_Add)
			ReadDiagnosticSubSplitAddr(k_Split_Master_Add);// Leggo l'indirizzo dello SubSlave da monitorare 
														// dall'unita slave Master (quella dotata di Touch)
														// che per convenzione ha indirizzo "1" (k_Split_Master_Add)			
			RefreshDiagnosticSplit();					// Leggo i dati dallo Split sotto disgnostica
			RefreshDiagnosticSubSplit();				// Leggo i dati dallo (Sub)Split sotto disgnostica
			
			ReadDiagnosticInverterAddr(k_Split_Master_Add);// Leggo l'indirizzo dello SubSlave da monitorare 
						
			SendDiagnosticSplitData(k_Split_Master_Add);	// Invio dati dello Split sullo slave master x monitoraggio su Touch
			//*******************************************************************************

			// ****** Gestione dati invertet in diagnostica sul touch dello slave master *******	#v16
			SendDiagnosticInverterData(k_Split_Master_Add, Comp_Inverter[0].DiagAddress);// Invio dati sullo slave master x monitoraggio su Touch
		
			MasSlaveSyncroData(k_Split_Master_Add);

			//SendWorkingSplitData();		// Invio dati di lavoro agli Split (SuperHeat Setpoint, Working Pressure Hot & Cool)
			
            SplitReadyCommand();

#if (K_AbilBridge1==1)  
    #if(K_HiSpeedU1ModBUS==1)            
            Change_Protocol_BUSM(K_ModBus_Baud115200);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
    #else
            Change_Protocol_BUSM(K_ModBus_Baud57600);						// Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)										
    #endif
            Bridge[0].OffLine = Send_WaitModRx(k_Bridge1_Addr, ReadModCmd, 0, 1, 20/*MaxModReadWait*/, 1/*MaxModReadRetries*/);
            Bridge[0].OnLine=!Bridge[0].OffLine;
            
			if(Bridge[0].OnLine==1)
			{
				// Scrivo "Room[1].PowerLimit" che arriva dal Touch (split) sul Bridge in posizione 1
				Send_DataToBridge(1);						// Invia dati al Bridge 1			
				DelaymSec(15);		
				// Leggo "EngineBox[0].PowerLimit" dal Bridge in posizione 1 che ivierò verso il Touch (Split)
				Get_DataFromBridge(&Bridge[0], 1);					// Legge dati dal Bridge 1			

				//if(Check_IfBridgeGoOnline(&Bridge[0], k_Bridge1_Addr, 1)==1)
                if(Check_IfBridgeGoOnline(&Bridge[0], k_Bridge1_Addr)==1)
				{
					Init_Bridge(1);									// Controlla se presente scheda Bridge 1 e la inizializza
				}				
			}
#endif	
#if (K_AbilBridge2==1)

            Bridge[1].Enable = 1;

            if(Check_IfBridgeGoOnline(&Bridge[1], k_Bridge2_Addr)==1)
                Init_Bridge(2);									// Controlla se presente scheda Bridge 1 e la inizializza
            
			if(Bridge[1].OnLine==1 && Bridge[1].BridgeReady==1)
			{

				// Scrivo "Room[1].PowerLimit" che arriva dal Touch (split) sul Bridge in posizione 1
				Send_DataToBridge(&Bridge[1], 2);                               // Invia dati al Bridge 1			
				DelaymSec(15);		
				// Leggo "EngineBox[1].PowerLimit" dal Bridge in posizione 1 che ivierò verso il Touch (Split)
				Get_DataFromBridge(&Bridge[1], 2);					// Legge dati dal Bridge 1			
			}
#endif			

#if (K_AbilMODBCAN==1)
            ModbCAN.Enable = 1;

//            if(Check_IfModbCANGoOnline(&ModbCAN)==1)
//                Init_ModbCAN();            						// Controlla se presente CAN Converter e lo inizializza
            
            //VIGATRAPPOLA
            //while(ModbCAN.ShotMaskFlag==1)
            //    ModbCAN.Enable = 1;
            //VIGATRAPPOLA
            
            
			if(ModbCAN.OnLine==1 && ModbCAN.Ready==1) // && ModbCAN.ShotMaskFlag==0)
			{

				//Send_DataToModbCAN();                           // Invia dati al CAN Converter
				//DelaymSec(15);		
				//Get_DataFromModbCAN();                          // Legge dati dal CAN Converter			
                Send_UpdateDataToRoom();
                ModbCAN.ChangedValueFromHMI=0;
			}
            
        Bridge[1].PermanentOffLine=ModbCAN.PermanentOffLine;
        Bridge[1].OffLine=ModbCAN.OffLine;
        Bridge[1].OnLine=ModbCAN.OnLine;
        Bridge[1].Enable=ModbCAN.Enable;
        Room[32].CumulativeAlarm = ModbCAN.CANBUS_Activity==0 && ModbCAN.OnLine==1;
        
            
#endif //#if (K_AbilMODBCAN==1)         
            
            
            
            			
			// NEW v5.14.147
			//CheckAllRangeValue();						// Controlla il valore delle variabili da inviare agli split e se fuori limiti corregge con "defVal"
			// NEW v5.14.147
			
	
			RefreshAllOutPut();							// aggiorno le uscite
			RefreshAllModeRoom();						// invio in broadcast la modalità letta dallo slave con addr.1 (Touch)

#if(K_AbilValveOnDemand==1)
            Valve_On_Demand();                          //Gestisco le valvole degli Slave con priorità          
#endif
#if(K_AbilScoreOnDemand==1)
            Score_On_Demand();                          //Gestisco le valvole degli Slave con priorità          
#endif
/*            
#if(K_AbilUTA ==1)           
            RefreshOutPutMasterRoom();                  // Invia dati solo allo Slave Master 
#endif
*/
/*
			Comp_Inverter[0].Req_Freq = DAC2FreqInverterComp(EngineBox[0].Out_Inverter);	// Converto il valore Out_inverter in scala DAC_1 (0..255 -> 0..10V x Inverter compressore)
																						// in valore frequenza da inviare via ModBus
			Comp_Inverter[0].Max_Req_Freq = DAC2FreqInverterComp(EngineBox[0].Inverter_Max_Power_Out);					// Converto il valore da scala DAC_1 (0..255 -> 0..10V x Inverter compressore)
																						// in valore frequenza da visualizzare su touch
#if (K_AbilCompressor2==1)		
			Comp_Inverter[1].Req_Freq = DAC2FreqInverterComp(EngineBox[1].Out_Inverter);	// Converto il valore Out_inverter in scala DAC_1 (0..255 -> 0..10V x Inverter compressore)
																						// in valore frequenza da inviare via ModBus
			Comp_Inverter[1].Max_Req_Freq = DAC2FreqInverterComp(EngineBox[0].Inverter_Max_Power_Out);					// Converto il valore da scala DAC_1 (0..255 -> 0..10V x Inverter compressore)
#endif
*/			
#if(K_OnlyOneInverterComp2==1)
            if(EngineBox[0].SelectCompressor==1)
#endif			
               RefreshInverterComp(0, &EngineBox[0], &Comp_Inverter[0], Add_Comp_Inverter1, &TimerInverterPersError_C1);	// aggiorno i registri dell'inverter compressore dei touch  
               InverterCheckVoltage(0, &Comp_Inverter[0], &TimerInVoltageInverterOk_C1);
#if (K_AbilCompressor2==1)					
#if(K_OnlyOneInverterComp2==0)
            RefreshInverterComp(1, &EngineBox[1], &Comp_Inverter[1], Add_Comp_Inverter2, &TimerInverterPersError_C2);	// aggiorno i registri dell'inverter compressore dei touch            
            InverterCheckVoltage(1, &Comp_Inverter[1], &TimerInVoltageInverterOk_C2);
#else
            else
                RefreshInverterComp(1, &EngineBox[1], &Comp_Inverter[0], Add_Comp_Inverter1, &TimerInverterPersError_C2);	// aggiorno i registri dell'inverter compressore dei touch
#endif

#endif            
			
#if (K_AbilPumpInverter==1)	
            RefreshInverterPump(&EngineBox[0], &Comp_Inverter[2], Add_Pump_Inverter1, &TimerInverterPersError_Pump);	// aggiorno i registri dell'inverter compressore dei touch            
#endif          			
			
			//-------------------------------------------------------------------------------------------
			// Gestione flag di stato compressore (viene memorizzato in EEPROM) per gestione 
			// della pausa alla riaccensione dopo un backout (3 minuti se era acceso prima di backout)
			//-------------------------------------------------------------------------------------------
			if(EngineBox[0].Error.MasterRestart==0)
			{
				if(EngineBox[0].Out_Inverter>0 || EngineBox[1].Out_Inverter>0 )
					EngineBox[0].LastCompressorPowerStatus = 1;
				else
					EngineBox[0].LastCompressorPowerStatus = 0;
			}
			else
			{
				TimerPwrLed.Time = 20;
			}
			
			//@
            RefreshE2_CompressorStatus();			// Memorizzo eventuale variazione in EEPROM
	
		}

		
        UpdateRele();
        
        UpdateDAC();
        
        Update7SegDisplay();
        
		ErrorReport();                              // Segnalo con LED rosso se presenti errori 
  
 // Blocco in caso di ore di lavoro superate!!!!!!!!!
        if(!InputProgrammingMode)
    		CheckWorkHours();


#ifdef K_ABIL_UART_SERVICE_DEBUG            
        //PutStringToUART3(Me.Temp_Big, Me.Temp_Small, Me.Temp_AirOut, Me.Temp_Amb, Me.Pres_Big, Me.Pres_Small, Me.CycleTime);
        // Per trasmetter esotto int TX UART3 si puo usare anche:
        //PutDataForUART3(EngineBox[0].Temperature.Compressor_Output, EngineBox[0].Temperature.Compressor_Suction, EngineBox[0].Temperature.Condenser, EngineBox[0].Temperature.Sea_Water, EngineBox[0].Temperature.Liquid, EngineBox[1].Temperature.Compressor_Output, EngineBox[0].SystemCycleTime);
#if (K_AbilMODBCAN==1)      
#else        
        PutDataForUART4(EngineBox[0].Temperature.Compressor_Output, EngineBox[0].Temperature.Compressor_Suction, EngineBox[0].Temperature.Condenser, EngineBox[0].Pressure.Gas, EngineBox[0].Pressure.Liquid, EngineBox[0].Pressure.LiquidCond, EngineBox[0].SystemCycleTime);
        //PutDataForUART4(EngineBox[0].Temperature.Compressor_Suction, PressureGasToTempR410A(EngineBox[0].Pressure.LiquidCond), EngineBox[0].SuperHeat.Value, (int)SuperHeatPID.Error, EngineBox[0].SuperHeat.HeatingCalc, EngineBox[0].Pressure.LiquidCond, EngineBox[0].SystemCycleTime);
#endif //#if (K_AbilMODBCAN==1)         

        //Argo
        //PutDataForUART4(Comp_Inverter[0].Out_Current, PressureGasToTempR410A(EngineBox[0].Pressure.LiquidCond), EngineBox[0].SuperHeat.Value, (int)SuperHeatPID.Error, EngineBox[0].SuperHeat.HeatingCalc, EngineBox[0].Pressure.LiquidCond, EngineBox[0].SystemCycleTime);      
        //PutDataForUART4(EngineBox[0].Temperature.Compressor_Suction, PressureGasToTempR410A(EngineBox[0].Pressure.LiquidCond), EngineBox[0].SuperHeat.Value, (int)SuperHeatPID.Error, EngineBox[0].SuperHeat.HeatingCalc, EngineBox[0].Pressure.LiquidCond, SuperHeatPID.SetP); //EngineBox[0].SystemCycleTime);
                 
#endif
        
        
		EngineBox[0].SystemCycleTime = TimerSystemCycle.Value;		// Memorizzo valore timer tempo di ciclo per successivo invio sul Touch x diagnostica
		TimerSystemCycle.TimeOut = 0;				// resetto il time out del timer Elapsed
		TimerSystemCycle.Value = 0;					// resetto il time value del timer Elapsed
        
#if (K_DEBUG_LCD==1)    
        Print_LCD_Info();                           // Stampa informazioni di sistema e debug su eventuale display LCD seriale I2C 4x20
#endif        

#if (K_AbilMODBCAN==1)  
	Start_TMR3();
#endif //#if (K_AbilMODBCAN==1)
                        
// Per il momento commentato per future prove        CheckStackOverflow();                       // Controlla x eventuali "sforamenti dello Stack sulla RAM utilizzata
        
        
	}    
  
 
}


void UpdateRele(void)
{        
#if (K_DEVELOP==1)			
/*// !!!!!!!!!!! DEBUG !!!!!!!!!!!!
		//aggiorno le uscite a rele
		oInverterMotore = !oFreeOut5;
		oCaldoFreddo = !oInverterMotore;
		oPompaAcqua = !oCaldoFreddo;
		oFreeOut4 = !oPompaAcqua;
		oFreeOut5 = !oFreeOut4;
// !!!!!!!!!!! DEBUG !!!!!!!!!!!! */        
		//DEBUG & Test
        /*
        if(iFlowSwitch)
        {
            LATDbits.LATD0	= 1;
            LATDbits.LATD1	= 1;
            LATDbits.LATD2	= 1;
            LATDbits.LATD3	= 1;
            LATDbits.LATD4	= 1;        
        }
        else
        {
            LATDbits.LATD0	= !PORTBbits.RB0;
            LATDbits.LATD1	= !PORTBbits.RB1;
            LATDbits.LATD2	= !PORTBbits.RB2;
            LATDbits.LATD3	= !PORTBbits.RB3;
            LATDbits.LATD4	= !PORTBbits.RB4;
        }
        */
		//DEBUG & Test            
#else	
	#if (K_AbilCompressor2==0)		// K_AbilCompressor2 = 1 -> Compressore doppio		
		//aggiorno le uscite a rele con 1 compressore	
		oPersErr = ((StatoRele & 0x01) != 0);
		oCaldoFreddo = ((StatoRele & 0x02) != 0);
		oPompaAcqua = ((StatoRele & 0x04) != 0);
		oPompaAcqua2 = ((StatoRele & 0x08) != 0);	
#if (K_AbilRechargeOil==1)
		oOil1 = ((StatoRele & 0x10) != 0);		
#endif
	#else
		//aggiorno le uscite a rele con 2 compressori
    #if(K_OnlyOneInverterComp2==1)
        oCompressor2 = ((StatoRele & 0x01) != 0);
	#elif (K_Abil_Double_Pump==1)			
		oPompaAcqua2 = ((StatoRele & 0x01) != 0);
    #else    
		oPersErr = ((StatoRele & 0x01) != 0);	
	#endif		
		oCaldoFreddo = ((StatoRele & 0x02) != 0);
		oPompaAcqua = ((StatoRele & 0x04) != 0);
		oOil1 = ((StatoRele & 0x08) != 0);		
		oOil2 = ((StatoRele & 0x10) != 0);		
	#endif		
#endif
		
}


void UpdateDAC()
{
    // aggiorno le uscite analogiche
    DA_Write(DA_CH_01, DAC_1);				
    DA_Write(DA_CH_02, DAC_2);			
    DA_Write(DA_CH_03, DAC_3);			
    DA_Write(DA_CH_04, DAC_4);			    
}

void Update7SegDisplay()
{
    if(EngineBox[0].Mode == EngineBox_CriticalFault)
       PrintDigit('E', ON , OFF, ON);
#if (K_AbilCompressor2==1)
    else if(EngineBox[0].Mode == EngineBox_GoFreddo || EngineBox[0].Mode == EngineBox_Freddo || EngineBox[1].Mode == EngineBox_GoFreddo || EngineBox[1].Mode == EngineBox_Freddo)
#else
    else if(EngineBox[0].Mode == EngineBox_GoFreddo || EngineBox[0].Mode == EngineBox_Freddo)
#endif
       PrintDigit('C', ON , OFF, OFF);        
#if (K_AbilCompressor2==1)
    else if(EngineBox[0].Mode == EngineBox_GoCaldo || EngineBox[0].Mode == EngineBox_Caldo || EngineBox[1].Mode == EngineBox_GoCaldo || EngineBox[1].Mode == EngineBox_Caldo)
#else
    else if(EngineBox[0].Mode == EngineBox_GoCaldo || EngineBox[0].Mode == EngineBox_Caldo)
#endif
       PrintDigit('H', ON , OFF, OFF);        
    else if(EngineBox[0].Mode == EngineBox_GoOff)
       PrintDigit('S', ON , OFF, OFF);                
    else
       PrintDigit('h', ON , OFF, OFF);    
}

//-----------------------------------------------------------------------
// gestione segnalazione errori di comunicazione
//-----------------------------------------------------------------------
void ErrorReport(void)                              // Segnalo con LED rosso se presenti errori 
{
    unsigned char rotseg=1;    
//-----------------------------------------------------------------------
// gestione segnalazione errori di comunicazione
//-----------------------------------------------------------------------
    if(!ServiceSlaveMode)
    {
        if(CntComErr >= K_MaxCommErrorAlarm || TimeOutComErr >= K_MaxCommErrorAlarm || CRC_ComErr >= K_MaxCommErrorAlarm || 
           ModCntComErr >= K_MaxCommErrorAlarm || ModTimeOutComErr >= K_MaxCommErrorAlarm || ModCRC_ComErr >= K_MaxCommErrorAlarm || 
           ModCntComErr_U2 >= K_MaxCommErrorAlarm || ModTimeOutComErr_U2 >= K_MaxCommErrorAlarm || ModCRC_ComErr_U2 >= K_MaxCommErrorAlarm ||
#if (K_AbilMODBCAN==1)                
           ModCntComErr_U4 >= K_MaxCommErrorAlarm || ModTimeOutComErr_U4 >= K_MaxCommErrorAlarm || ModCRC_ComErr_U4 >= K_MaxCommErrorAlarm ||
           Bridge[0].PermanentOffLine==1 || Bridge[1].PermanentOffLine==1 || ModbCAN.PermanentOffLine==1) // || CommandNotImplemented==1)
#else
           Bridge[0].PermanentOffLine==1 || Bridge[1].PermanentOffLine==1) // || CommandNotImplemented==1)            
#endif //#if (K_AbilMODBCAN==1)             
            oErrLed = LED_ON;
        else
            oErrLed = LED_OFF;
    }

    if(TimerComunicationError.TimeOut)
    {	
        CntComErr = 0;
        TimeOutComErr = 0;
        CRC_ComErr = 0;
        ModCntComErr = 0;
        ModTimeOutComErr = 0;
        ModCRC_ComErr = 0;            
        ModCntComErr_U2 = 0;
        ModTimeOutComErr_U2 = 0;
        ModCRC_ComErr_U2 = 0;            
#if (K_AbilMODBCAN==1)          
        ModCntComErr_U4 = 0;
        ModTimeOutComErr_U4 = 0;
        ModCRC_ComErr_U4 = 0;   
#endif //#if (K_AbilMODBCAN==1)                  
        TimerComunicationError.Value = 0;
        TimerComunicationError.TimeOut = 0;		
    }
    
/*    
    //ARGOCRCPUZZONEROTANTE
    if(CntComErr >= K_MaxCommErrorAlarm || TimeOutComErr >= K_MaxCommErrorAlarm || CRC_ComErr >= K_MaxCommErrorAlarm || ModCntComErr >= K_MaxCommErrorAlarm || ModTimeOutComErr >= K_MaxCommErrorAlarm || ModCRC_ComErr >= K_MaxCommErrorAlarm)
    {
       //PrintDigit('F', ON , OFF, ON);    
        while(1)
        {
            if(++rotseg>6) rotseg=1;
            PrintDigit(rotseg, ON , OFF, OFF);      // Rotazione segmento display
            DelaymSec(100);
        }
    }
    //ARGOCRCPUZZONEROTANTE
*/    
    
}


void CheckWorkHours(void)
{
    if(MyRTC.HoursOfLife >= Room[k_Split_Master_Add].Maintenance_Set_Time) 
        RTC_Flag=1;
    
    if(((RTC_Flag==1) && (EngineBox[0].NoPayCastigationActivation==0)) || ((Room[k_Split_Master_Add].Maintenance_Flag & 0x0002) != 0))
    {
        RTC_Flag=0;
        MyRTC.HoursOfLife=0;
        StoreInt(MyRTC.HoursOfLife, E2_Addr_Hours);	// Azzero ore conteggiate in E2         
    }	

    Room[k_Split_Master_Add].Maintenance_Remaning_Time = (Room[k_Split_Master_Add].Maintenance_Set_Time-MyRTC.HoursOfLife)/24;
            
    //TimerPwrLed.Enable = RTC_Flag==1 || (EngineBox[0].NoPayCastigationActivation==1);		

    if(TimerPwrLed.Enable)
    {
        if(RTC_Flag==1)
            TimerPwrLed.Time = 50;
        else
            TimerPwrLed.Time = 300;
    }
    else 
        oPwrLed = LED_ON;    
}



//----------------------------------------------------------------------------------
//	function and procedure
//----------------------------------------------------------------------------------
void Init(void)
{
	RCON = 0;				// Reset control
    DelaymSec(200);         // Serve per stabilizzare tensioni EEPROM!!
    
    // N.B.: TRISx = 1 -> Input, TRISx = 0 -> Output
    // >>>>> Configurare la direzione dei pin N.C. e "NOT EXIST" come Output (0) <<<<<
    
	LATA = 0;				// resetto il portA         
	TRISA = 0x06FF; //0000 0110 1111 1111  //0x07FF;			// setto gli i/o del PortA: RA0..RA7 = DIPSW (in), RA8 = NOT EXIST (out), RA9..RA10 = Vref (in), RA11..R13 = NOT EXIST (out), RA14..RA15 = N.C. (out)
	LATB = 0;				// resetto il portB
	TRISB = 0x0FFF;  //0000 1111 1111 1111  //0x01FF;			// setto gli i/o del PortB: RB0..RB5 = AN0..AN5 (in), RB6,RB7 = PGC,PGD (in), RB8..RB11 = AN6..AN9 (in), RB12..RB15 = N.C. (out)
	LATC = 0;				// resetto il portC 
	TRISC = 0x4000;  //0100 0000 0000 0000  //0x0010;			// setto gli i/o del PortC: RC0 = NOT EXIST (out), RC1,RC2 = N.C. (out), RC3 = RTS_4 (out), RC4 = RTS_3 (out), RC5..RC11 = NOT EXIST (out), RC12,RC13 = N.C. (out), RC14 = KEY (in), RC15 = N.C. (out)
	LATD = 0;				// resetto il portD
	TRISD = 0xBF00;  //1011 1111 0000 0000  //0x9C03;			// setto gli i/o del PortD: RD0..RD7 = 7seg Digit (out), RD8..RD13 = Digital IN (in), RD14 = N.C. (out), RD15 = RX_M (in)                           	// NEW HW n.1 DigitOut (RD13)  e n.1 DigiIn (RD12))
	LATE = 0;				// resetto il portE

    //$ ODCE = 0x0010;          // RE4 = Open Drain
    //$ oCurrDriver = 1;        //   
    
    TRISE = 0x0080;  //0000 0000 1000 0000  //0x00C0;           // setto gli i/o del PortE: RE0..4 = REL1..5 (out), RE5 = N.C. (out), RE6 = SCL (out), RE7 = SDA (in), RE8,RE9 = N.C. (out), RE10..RE15 = NOT EXIST (out)
    
	LATF = 0;				// resetto il portF
	TRISF = 0x0100;  //0000 0001 0000 0000  //0x0114;			// setto gli i/o del PortF: RF0,RF1 = N.C. (out), RF2 = TX_S (out), RF3 = RTS_S (out), RF4 = TX_M (out), RF5 = RTS_M (out), RF6 = NOT EXIST (out), RF7 = N.C. (out), RF8 = RX_S (in), RF9..RF11 = NOT EXIST (out), RF12,RF13 = = N.C. (out), RF14,RF15 = NOT EXIST (out)
	LATG = 0;				// resetto il portG
	TRISG = 0x0280;  //0000 0010 1000 0000  //0x0000;			// setto gli i/o del PortG: RG0 = PWRLED (out), RG1 = ERRLED (out), RG2,RG3 = N.C. (out), RG4,RG5 = NOT EXIST (out), RG6 = TX_3 (out), RG7 = RX_3 (in), RG8 = TX_4 (out), RG9 = RX_4 (in), RG10,RG11 = NOT EXIST (out), RG12 = SCK (out), RG13 = SDO (out), RG14 = L_DAC (out), RG15 = N.C. (out)

//------------------------------------------------------
// Settaggio direzione PORTs:  1= Input 0= Output
//------------------------------------------------------
    
    TRISGbits.TRISG15 = 0;  // CN82	RG15
    TRISEbits.TRISE5 = 0;   // CN63	RE5
    TRISCbits.TRISC1 = 0;   // CN45	RC1
    TRISCbits.TRISC2 = 0;   // CN46	RC2
    TRISEbits.TRISE8 = 0;   // CN66	RE8
    TRISEbits.TRISE9 = 0;   // CN67	RE9
    TRISFbits.TRISF13 = 0;  // CN76	RF13
    TRISFbits.TRISF12 = 0;  // CN75	RF12
    TRISBbits.TRISB12 = 0;  // CN30	RB12
    TRISBbits.TRISB13 = 0;  // CN31	RB13
    TRISBbits.TRISB14 = 0;  // CN32	RB14
    TRISBbits.TRISB15 = 0;  // CN12	RB15
    TRISDbits.TRISD14 = 0;  // CN20	RD14
    TRISFbits.TRISF7 = 0;   // CN73	RF7
    TRISGbits.TRISG3 = 0;   // CN84	RG3
    TRISGbits.TRISG2 = 0;   // CN83	RG2
    TRISCbits.TRISC12 = 0;  // CN23	RC12
    TRISCbits.TRISC15 = 0;  // CN22	RC15
    TRISAbits.TRISA14 = 0;  // CN43	RA14
    TRISAbits.TRISA15 = 0;  // CN44	RA15
    TRISCbits.TRISC13 = 0;  // CN1	RC13
    TRISFbits.TRISF0 = 0;   // CN68	RF0
    TRISFbits.TRISF1 = 0;   // CN69	RF1    
    
    // abilito i pull up
    _CN21PUE = 1;	// pull up RX_M   
    _CN74PUE = 1;	// pull up RX_S
    _CN9PUE = 1;	// pull up RX_3   
    _CN11PUE = 1;	// pull up RX_4   

    _CN24PUE = 1;	// pull up PGC   
    _CN25PUE = 1;	// pull up PGD
//------------------------------------------------------      
        
    
    
	//$ oPWM_Fan1 = 1;
	//$ oPWM_Fan2 = 1;
    
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
    
	CNPU4bits.CN53PUE = 1;	// pull up DIN1   
	CNPU4bits.CN54PUE = 1;	// pull up DIN2
	CNPU4bits.CN55PUE = 1;	// pull up DIN3
	CNPU4bits.CN56PUE = 1;	// pull up DIN4
	CNPU4bits.CN57PUE = 1;	// pull up DIN5
	CNPU2bits.CN19PUE = 1;	// pull up DIN6

    
    
	oErrLed=LED_ON;
    DelaymSec(200);         //DEBUG ArgoEEPROM !    
	I2C_Init();             // inizializzo l'I2C1
    DelaymSec(200);         //DEBUG ArgoEEPROM !
    CheckEEPROM();          //DEBUG ArgoEEPROM !
    
   
	oErrLed=LED_OFF;	
	
	// Init ADC
	_AD1IE = 0;		// abilito interrupt AD
	_AD1IF = 0;		// reseto il flag d'interrupt del convertitore
	_AD1IP = 1;//4;		// imposto il livello di interupt (7 più alto) 
  
    
    /// >>>>>>>>>>>>>>>>>>> DA RIVEDERE I NOMI DELLE VARIABILI A/D <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<       
	Init_Adc();     
   

   // EngineBox[0].BoardAddress = (~PORTA & 0x00FF);

    EngineBox[0].BoardAddress = Master_Default_Address-(~PORTA & 0x0007);   //ATTENZIONE: l'indirizzo è base (254) meno indirzzo da dip. Ex. Dip 1 = 0N -> 254-1 = 253. Max address: 7 quindi 247-254.
    
    
    
    /* DA CONTROLLARE SE SERVONO
	Me.SuperHeat_SP = K_SetPoint;	
	Me.Freddo_TempBatt_SP = Freddo_TempBatt_MaxPwr;
    Me.Freddo_TempBatt_Min = Freddo_OffSetBatt_MinPwr;
	Me.Caldo_TempBatt_SP = Caldo_TempBatt_MaxPwr;	
    Me.Caldo_TempBatt_Min = Caldo_OffSetBatt_MinPwr;	
	Me.PerformanceLossBattery_SP = K_PerformanceLossBattery;
    Me.Dinamic_SuperHeat = K_Dinamic_SuperHeat_Default;
    Me.OtherCabin.Number = 0;   //Carico a zero di default!
	*/
    
    
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
    
    
	// Init UART 3			
	init_comms3();
    _U3RXIE = 1;            // abilito interrupt su rx usart 3
    _U3TXIE = 0;            // Disattivo l'interrupt di TX Uart3
    _U3ERIE = 1;            // abilito interrupt Error usart 3    
    _U3RXIP = 3;            // imposto il livello di interupt per la ricezione carattere UART3    
    _U3ERIP = 3;            // imposto il livello di interupt x ERROR
    _U3TXIP = 3;            // imposto il livello di interupt per la tramissione carattere UART3        
	ClearUart3Err();    
    
    
	// Init UART 4		
	init_comms4();
    _U4RXIE = 1;            // abilito interrupt su rx usart 4
    _U4TXIE = 0;            // Disattivo l'interrupt di TX Uart4
    _U4ERIE = 1;            // abilito interrupt Error usart 4    
    _U4RXIP = 3;            // imposto il livello di interupt per la ricezione carattere UART4    
    _U4ERIP = 3;            // imposto il livello di interupt x ERROR
    _U4TXIP = 3;            // imposto il livello di interupt per la tramissione carattere UART4        
	ClearUart4Err();    
    
    
#if (K_AbilMODBCAN==1)  
	Init_Timer1();			// Init TMR1
	//Init_Timer2();			// Init TMR2
	Init_Timer3();			// Init TMR3    
	//Init_Timer4();			// Init TMR4
	//Init_Timer5();			// Init TMR5        
#else    
	Init_Timer();			// Init TMR1
#endif //#if (K_AbilMODBCAN==1)     

	PID_init(&SuperHeatPID, K_p_gain, K_i_gain, K_d_gain, Kf_iMin, Kf_iMax, K_PoMin, K_PoMax, K_dead_zone);
	PID_init(&PressSeaWaterPID, K_p_gain, K_i_gain, K_d_gain, Kf_iMin, Kf_iMax, K_PoMin, K_PoMax, K_dead_zone);    
     

    EngineBox[0].HW_Version = HW_VERSION;					// REG_TOUCH_MASTER_HW_VERSION
	EngineBox[0].FW_Version_HW = Versione_Hw;				// REG_TOUCH_MASTER_FW_VERSION_HW                
	EngineBox[0].FW_Version_FW = Versione_Fw;				// REG_TOUCH_MASTER_FW_VERSION_FW
	EngineBox[0].FW_Revision_FW = Revisione_Fw;             // REG_TOUCH_MASTER_FW_REVISION_FW
    
	//TimerResetErrori.Time = ResetTimeErrorCounter;	// inizializzo i timer per gli errori
	TimerErrCompressorHiC1.Time = TimeCompressorHi;
	TimerErrCompressorHiC2.Time = TimeCompressorHi;

	TimerErrCompressorLo.Time = TimeCompressorLo;
	TimerErrCondensatoreHi.Time = TimeCondensatoreHi;
	TimerErrCondensatoreLo.Time = TimeCondensatoreLo;
	TimerErrFlussostato.Time = TimeFlussostato_Work;
	TimerErrGasRecovery.Time = Time_Gas_Recovery;
	TimerResErrCompressorHiC1.Time = TimeResCompressorHi;
	TimerResErrCompressorHiC2.Time = TimeResCompressorHi;
	TimerResErrCompressorLo.Time = TimeResCompressorLo;
	TimerResErrCondensatoreHi.Time = TimeResCondensatoreHi;
	TimerResErrCondensatoreLo.Time = TimeResCondensatoreLo;
	TimerErrPressureHi.Time = K_TimeErrPressureHi;
	TimerErrPressureLo.Time = K_TimeErrPressureLo;
#if (K_AbilRechargeOil==1)	
	TimerWorkRechargeOil1.Time = TimeWorkRechargeOil;
	TimerPauseRechargeOil1.Time = TimePauseRechargeOil;
	TimerAlarmRechargeOil1.Time = TimeAlarmRechargeOil;
    TimerLowCriticalAlarmRechargeOil1.Time = TimeLowCriticalAlarmRechargeOil;
#if (K_AbilCompressor2==1)
	TimerWorkRechargeOil2.Time = TimeWorkRechargeOil;	
	TimerPauseRechargeOil2.Time = TimePauseRechargeOil;
	TimerAlarmRechargeOil2.Time = TimeAlarmRechargeOil;
    TimerLowCriticalAlarmRechargeOil2.Time = TimeLowCriticalAlarmRechargeOil;
#endif
#endif
	TimerResTime_Gas_Rec.Time = TimeRes_Gas_Recovery;

	DividerSecondi.Time = 1000;		// Timer per generare una base tempo di 1 secondo
	DividerSecondi.Value = 0;		// resetto il conteggio del timer
	DividerSecondi.TimeOut = 0;		// resetto il time out
	DividerSecondi.Enable =1;		// abilito il timer

	TimerPwrLed.Time = 250;
	TimerPwrLed.Value = 0;
	TimerPwrLed.TimeOut = 0;
	TimerPwrLed.Enable = 0;

	TmrLastRequest.Value = 0;
	TmrLastRequest.Time = 30;
	TmrLastRequest.TimeOut = 0;
	TmrLastRequest.Enable = 0;
/*
	TimerRstComErr.Value =0;
	TimerRstComErr.Time = 20000;					// Millisecondi
	TimerRstComErr.TimeOut = 0;
	TimerRstComErr.Enable =1;
    
	TimerModRstComErr.Value =0;
	TimerModRstComErr.Time = 20000;					// Millisecondi
	TimerModRstComErr.TimeOut = 0;
	TimerModRstComErr.Enable =1;    
*/
	TimerComunicationError.Value =0;
	TimerComunicationError.Time = 20000;					// Millisecondi
	TimerComunicationError.TimeOut = 0;
	TimerComunicationError.Enable =1;
	
	TimerElaborazioneC1.Time = TimeIncOutInverter_Cooling;						// Tempo di gestione variazione compressore 2Sec.
	TimerElaborazioneC1.Enable = 0;
	TimerElaborazioneC1.TimeOut = 0;					// resetto il timer
	TimerElaborazioneC1.Value = 0;
	
	TimerElaborazioneC2.Time = 2;					// Tempo di gestione variazione compressore2 2Sec.
	TimerElaborazioneC2.Enable = 0;
	TimerElaborazioneC2.TimeOut = 0;					// resetto il timer
	TimerElaborazioneC2.Value = 0;	

	TimerSystemCycle.Time = 15000;					// Tempo di calcolo tempo di ciclo programma (MAX = 5000 mSec.)
	TimerSystemCycle.Enable = 0;
	TimerSystemCycle.TimeOut = 0;					// resetto il timer
	TimerSystemCycle.Value = 0;
    
	TimerExecPID.Time = K_TimeExecPID;
	TimerExecPID.Enable = 1;
	TimerExecPID.TimeOut = 0;					// resetto il timer
	TimerExecPID.Value = 0;
        
    TimerPostHeatingEngine.Time = K_TimePostHeatingEngine;    
    TimerPostHeatingEngine.Enable = 0;    
    TimerPostHeatingEngine.TimeOut = 0;    
    TimerPostHeatingEngine.Value = 0;    
    
    TimerAcceleration.Time = K_TimeAccelerationErrorCheck;    
    TimerAcceleration.Enable = 0;    
    TimerAcceleration.TimeOut = 0;    
    TimerAcceleration.Value = 0;      

    TimerWorkPump.Time = K_TimeWorkPump;    
    TimerWorkPump.Enable = 1;    
    TimerWorkPump.TimeOut = 0;    
    TimerWorkPump.Value = 0;      	
	
    
	TimerSwitchCompressor.Time = K_TimeSwitchCompressor;		// Timer per time switch secondo compressore
    TimerSwitchCompressor.Enable = 0;    
    TimerSwitchCompressor.TimeOut = 0;    
    TimerSwitchCompressor.Value = 0;    	
	TimerIntegrateCompressor2.Time = K_TimeIntegrateCompressor2;	// Timer per integrazione secondo compressore
    TimerIntegrateCompressor2.Enable = 0;    
    TimerIntegrateCompressor2.TimeOut = 0;    
    TimerIntegrateCompressor2.Value = 0;    	
	TimerDeintegrateCompressor2.Time = K_TimeDeintegrateCompressor2;	
    TimerDeintegrateCompressor2.Enable = 0;    
    TimerDeintegrateCompressor2.TimeOut = 0;    
    TimerDeintegrateCompressor2.Value = 0;    	
	
	TimerHwReset.Time = K_TimeHwReset;
	TimerHwReset.Enable = 0;
	TimerHwReset.TimeOut = 0;
	TimerHwReset.Value = 0;
	
    TimerSwitchPump.Time = 3600;
    TimerSwitchPump.Enable = 0;
    TimerSwitchPump.TimeOut = 0;
    TimerSwitchPump.Value = 0;
	
	TimerTrigPump.Time = K_TimeTrigPump;	
    TimerTrigPump.Enable = 0;    
    TimerTrigPump.TimeOut = 0;    
    TimerTrigPump.Value = 0;    	
	
	TimerPostPump.Time = K_TimeTrigPump;	
    TimerPostPump.Enable = 0;    
    TimerPostPump.TimeOut = 0;    
    TimerPostPump.Value = 0;   	
    
    TimerCoolingSuperHeatErr.Time = K_TimeCoolingSuperHeatErr;	
    TimerCoolingSuperHeatErr.Enable = 0;    
    TimerCoolingSuperHeatErr.TimeOut = 0;    
    TimerCoolingSuperHeatErr.Value = 0;   	

    TimerCondenserFouled.Time = K_CondenserFouled;    
    TimerCondenserFouled.Enable = 0;    
    TimerCondenserFouled.TimeOut = 0;    
    TimerCondenserFouled.Value = 0;  
    
    TimerCoolWarmValveError.Time = K_CoolWarm_Valve_Error;
    TimerCoolWarmValveError.Enable = 0;    
    TimerCoolWarmValveError.TimeOut = 0;    
    TimerCoolWarmValveError.Value = 0;  

    TimerInverterPersError_C1.Time = 45*60;
    TimerInverterPersError_C1.Enable = 0;    
    TimerInverterPersError_C1.TimeOut = 0;    
    TimerInverterPersError_C1.Value = 0;  

    TimerInverterPersError_C2.Time = 45*60;
    TimerInverterPersError_C2.Enable = 0;    
    TimerInverterPersError_C2.TimeOut = 0;    
    TimerInverterPersError_C2.Value = 0;  
    
    TimerInverterPersError_Pump.Time = 45*60;
    TimerInverterPersError_Pump.Enable = 0;    
    TimerInverterPersError_Pump.TimeOut = 0;    
    TimerInverterPersError_Pump.Value = 0;  
    
    TimerSuperHeatError.Time = K_TimeSuperHeat_Error;
    TimerSuperHeatError.Enable = 0;    
    TimerSuperHeatError.TimeOut = 0;    
    TimerSuperHeatError.Value = 0; 
    
    TimerTemperatureProbeErrorBox1.Time = K_TimeTemperatureProbe_Error;
    TimerTemperatureProbeErrorBox1.Enable = 0;    
    TimerTemperatureProbeErrorBox1.TimeOut = 0;    
    TimerTemperatureProbeErrorBox1.Value = 0; 
    
    TimerTemperatureProbeErrorBox2.Time = K_TimeTemperatureProbe_Error;
    TimerTemperatureProbeErrorBox2.Enable = 0;    
    TimerTemperatureProbeErrorBox2.TimeOut = 0;    
    TimerTemperatureProbeErrorBox2.Value = 0; 
    
    TimerCheckTemperatureProbe.Time = K_TimerCheckTemperatureProbe_Error;
    TimerCheckTemperatureProbe.Enable = 1;    
    TimerCheckTemperatureProbe.TimeOut = 0;    
    TimerCheckTemperatureProbe.Value = 0; 
    
    TimerCheckPressureProbe.Time = K_TimerCheckPressureProbe_Error;
    TimerCheckPressureProbe.Enable = 1;    
    TimerCheckPressureProbe.TimeOut = 0;    
    TimerCheckPressureProbe.Value = 0; 

    TimerPressureProbeErrorBox.Time = K_TimePressureProbe_Error;
    TimerPressureProbeErrorBox.Enable = 1;    
    TimerPressureProbeErrorBox.TimeOut = 0;    
    TimerPressureProbeErrorBox.Value = 0; 
    
    TimerExitProgrammingMode.Time = 5*60;
    TimerExitProgrammingMode.Enable = 1;    
    TimerExitProgrammingMode.TimeOut = 0;    
    TimerExitProgrammingMode.Value = 0; 
    
    TimerInVoltageInverterOk_C1.Time = K_Time_Error_Ok_Voltage;
    TimerInVoltageInverterOk_C1.Enable = 0;    
    TimerInVoltageInverterOk_C1.TimeOut = 0;    
    TimerInVoltageInverterOk_C1.Value = 0; 
    
    TimerInVoltageInverterOk_C2.Time = K_Time_Error_Ok_Voltage;
    TimerInVoltageInverterOk_C2.Enable = 0;    
    TimerInVoltageInverterOk_C2.TimeOut = 0;    
    TimerInVoltageInverterOk_C2.Value = 0; 

#if (K_AbilMODBCAN==1)
    LoadTimer(&StableValueChangeHMI, K_StableValueChangeHMI, 0, 1);
#endif //#if (K_AbilMODBCAN==1)        
    
#if (K_AbilDefrostingCycle==1)		    
    TimerDefrostingCycleWork.Time = K_DefrostingCycleWork;
    TimerDefrostingCycleWork.Enable = 0;
    TimerDefrostingCycleWork.TimeOut = 0;
    TimerDefrostingCycleWork.Value = 0;
    
    TimerDefrostingCycleStop.Time = K_DefrostingCycleStop;
    TimerDefrostingCycleStop.Enable = 0;
    TimerDefrostingCycleStop.TimeOut = 0;
    TimerDefrostingCycleStop.Value = 0;
    
    TimerDefrostingCycleCheck.Time = K_DefrostingCycleCheck;
    TimerDefrostingCycleCheck.Enable = 0;
    TimerDefrostingCycleCheck.TimeOut = 0;
    TimerDefrostingCycleCheck.Value = 0;
#endif
    
#if(K_AbilValveOnDemand==1 || K_AbilScoreOnDemand==1)
/*    
    TimerValve_On_Demand_Limit.Time = K_TimeOnDemandValveLimitCheck;
    TimerValve_On_Demand_Limit.Enable = 0;
    TimerValve_On_Demand_Limit.TimeOut = 0;
    TimerValve_On_Demand_Limit.Value = 0;
*/    
    TimerValve_On_Demand_Switch.Time = K_TimeOnDemandValveSwitch;
    TimerValve_On_Demand_Switch.Enable = 1;
    TimerValve_On_Demand_Switch.TimeOut = 0;
    TimerValve_On_Demand_Switch.Value = 0;
#endif   
    
	Reset_All_DA();
    
	EngineBox[0].DoubleCompressorOn = K_AbilCompressor2;		// Abilito/Disabilito la gestione del secondo compressore
	
#if(K_Supply_Inverter==283 || K_Supply_Inverter==566)
    Comp_Inverter[0].Ok_Voltage = 0;
#if(K_AbilCompressor2==1)
    Comp_Inverter[1].Ok_Voltage = 0;
#else
    Comp_Inverter[1].Ok_Voltage = 1;
#endif
#else
    Comp_Inverter[0].Ok_Voltage = 1;
    Comp_Inverter[1].Ok_Voltage = 1;
#endif
}

 
 
 
void Hw_Test(void)
{           
	unsigned char DipAddress;
    char TouchAddres=1;
    
	oPwrLed = LED_ON;
	oErrLed = LED_ON;
    
    CheckModuli();									// ricerca schede di espansione connesse	
    
    Change_Protocol_BUSS(K_ModBus_Baud115200);                             // Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)
	InitTouch();										// e atterndo che il touch ridiventi OnLine per la 
    
    
    while(1)
    {
        // leggo le sonde
		//AD_Read();    spostato sotto int timer1

		// aggiorno gli ingressi analogici
		
		EngineBox[0].Temperature.Compressor_Output = Steinhart(Adc_Temp[1], 2);             	
		EngineBox[0].Temperature.Compressor_Suction = Steinhart(Adc_Temp[2], 1); 
		EngineBox[0].Temperature.Condenser = Steinhart(Adc_Temp[3], 1);     
		EngineBox[0].Temperature.Sea_Water = Steinhart(Adc_Temp[4], 1);         
		EngineBox[0].Temperature.Liquid = Steinhart(Adc_Temp[5], 1);        	
		EngineBox[1].Temperature.Compressor_Output = Steinhart(Adc_Temp[6], 2); 
	    
        
        EngineBox[0].Temperature.Gas = EngineBox[0].Temperature.Compressor_Suction;            // Se sono in modalità Raffrescamento oppure OFF
        if(EngineBox[0].InverterRiscaldamento || EngineBox[0].GoRiscaldamento)  // Se sono in modalità Riscaldamento
            EngineBox[0].Temperature.Gas = EngineBox[0].Temperature.Compressor_Output;
         
        
		EngineBox[0].Pressure.Liquid = Pressure_Read(Adc_Temp[7]);                
		EngineBox[0].Pressure.Gas = Pressure_Read(Adc_Temp[8]);             
        EngineBox[0].Pressure.LiquidCond = Pressure_Read(Adc_Temp[9]);
       		      
        RefreshAllInPut();							// aggiorno gli ingressi
        
		
/*	*/	
        Change_Protocol_BUSS(K_ModBus_Baud115200);                             // Imposto protocollo (0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud)
        
		if(Touch.PermanentOffline)								// se Touch diventa OffLine Permanente
		{				
			InitTouch();										// e atterndo che il touch ridiventi OnLine per la 
																// corretta reinizializzazione dei valori di funzionamento
		}
		
		Resume_TouchOffLine();
		
		//TEST
        Touch.OffLine = 0;
        Touch.Enable = 1;
        Touch.PermanentOffline = 0;
        
		WriteModReg (TouchAddres, REG_TOUCH_SP_ROOM, 2350);		
		
		ReadModReg (TouchAddres, REG_TOUCH_PAGE, &(Touch.Page));	
		ReadModReg (TouchAddres, REG_TOUCH_SP_ROOM, &(Touch.SetPoint));	
		ReadModReg (TouchAddres, REG_TOUCH_FUNCTION_MODE, &(Touch.FunctionMode));	
		ReadModReg (TouchAddres, REG_TOUCH_FAN_SPEED, &(Touch.FanMode));	
		ReadModReg (TouchAddres, REG_TOUCH_PWR, &(Touch.Split_Pwr));	
		ReadModReg (TouchAddres, REG_TOUCH_MODE, &(Touch.Split_Mode));		
		
		//WriteModReg (TouchAddres, REG_TOUCH_PAGE, 2);	

		WriteModReg (TouchAddres, REG_TOUCH_MANDATA_COMPRESSORE, EngineBox[0].Temperature.Compressor_Output);					//80 = REG_TOUCH_MANDATA_COMPRESSORE
		WriteModReg (TouchAddres, REG_TOUCH_RITORNO_COMPRESSORE, EngineBox[0].Temperature.Compressor_Suction);				//81 = REG_TOUCH_RITORNO_COMPRESSORE
		WriteModReg (TouchAddres, REG_TOUCH_TEMP_CONDENSATORE, EngineBox[0].Temperature.Condenser);							//82 = REG_TOUCH_TEMP_CONDENSATORE		
		WriteModReg (TouchAddres, REG_TOUCH_SEA_WATER_IN_TEMP, EngineBox[0].Temperature.Sea_Water);							//83 = REG_TOUCH_SEA_WATER_IN_TEMP		
		WriteModReg (TouchAddres, REG_TOUCH_TEMP_P, EngineBox[0].Temperature.Liquid);							//93 = REG_TOUCH_TEMP_P
		WriteModReg (TouchAddres, REG_TOUCH_TEMP_G, EngineBox[1].Temperature.Compressor_Output);					//94 = REG_TOUCH_TEMP_G 
        
		WriteModReg (TouchAddres, REG_TOUCH_PRESSIONE_P, EngineBox[0].Pressure.Liquid);						//84 = REG_TOUCH_PRESSIONE_P
		WriteModReg (TouchAddres, REG_TOUCH_PRESSIONE_G, EngineBox[0].Pressure.Gas);							//85 = REG_TOUCH_PRESSIONE_G
		WriteModReg (TouchAddres, REG_TOUCH_ENGBOX_PRESS_LIQUID_COND, EngineBox[0].Pressure.LiquidCond);							//85 = REG_TOUCH_PRESSIONE_G

		WriteModReg (TouchAddres, 226, 8); //3);        
        

		LATEbits.LATE0	= !PORTDbits.RD8;		// Rel1 = In1
		LATEbits.LATE1	= !PORTDbits.RD9;		// Rel2 = In2
		LATEbits.LATE2	= !PORTDbits.RD10;		// Rel3 = In3
		LATEbits.LATE3	= !PORTDbits.RD11;		// Rel4 = In4
		LATEbits.LATE4	= !PORTDbits.RD12;		// Rel5 = In5	


		
		if(PORTDbits.RD13)	// In6 = Off -> Out DAC = 0V
		{
			DAC_1 = 0;
			DAC_2 = 0;
			DAC_3 = 0;
			DAC_4 = 0;
		}
		else				// In6 = On -> Out DAC = 10V
		{
			DAC_1 = 255;
			DAC_2 = 255;
			DAC_3 = 255;
			DAC_4 = 255;			
		}
		
		
		// aggiorno le uscite analogiche
		DA_Write(DA_CH_01, DAC_1);				
		DA_Write(DA_CH_02, DAC_2);			
		DA_Write(DA_CH_03, DAC_3);			
		DA_Write(DA_CH_04, DAC_4);				
        
        DipAddress = ~PORTA; 

        switch(DipAddress)
        {
            case 0:
                PrintDigit('0', ON , ON, OFF);        
                break;
            case 1:
                PrintDigit('1', ON , ON, OFF);        
                break;
            case 2:
                PrintDigit('2', ON , ON, OFF);             
                break;
            case 4:
                PrintDigit('3', ON , ON, OFF);            
                break;
            case 8:
                PrintDigit('4', ON , ON, OFF);             
                break;
            case 16:
                PrintDigit('5', ON , ON, OFF);               
                break;
            case 32:
                PrintDigit('6', ON , ON, OFF);          
                break;
            case 64:
                PrintDigit('7', ON , ON, OFF);               
                break;
            case 128:
                PrintDigit('8', ON , ON, OFF);               
                break;
                
        }
        
        
    }
       

    
    
 }
  
  
  
  /*
 void TestEEPROM(void)
 {
    int MagicNumber = 0x1234;
    int Test=0;
    
    ///////////StoreInt(MagicNumber, CRC_Hi_Flash);
    Test = LoadInt(CRC_Hi_Flash);
    
    if(Test != MagicNumber)
    {
        oDigitOut2 = 1;
        oDigitOut3 = 1;   
        DelaymSec(50);
        I2C_Init();
        StoreInt(MagicNumber, CRC_Hi_Flash);
    }
    else //if(iDigiIn3)
    {
        oDigitOut2 = 0;
        oDigitOut3 = 0;           
    }
    
    //DelaymSec(10);
 }
   */
 
 void CheckEEPROM(void)
 {
    int MagicNumber = 0x1234;
    int Test=0;
    int Passed=0;
    int Retry=10;
    
    while(!Passed && Retry>0)
    {
        StoreInt(MagicNumber, CRC_Hi_Flash);
        Test = LoadInt(CRC_Hi_Flash);

        if(Test != MagicNumber)
        {
            //oEvaporatorFan = 1;       ????? Come segnalo anomalia EEPROM ???????
            //oDigitOut2 = 1;
            //oDigitOut3 = 1;   
            DelaymSec(50);
            I2C_Init();
            Retry--;
        }
        else //if(iDigiIn3)
        {
            //oEvaporatorFan = 0;
            //oDigitOut2 = 0;
            //oDigitOut3 = 0;    
            Passed=1;
        }
    }
    
    if(Passed==0) EngineBox[0].Error.GlobalStatusFlag = 1;           // Segnalo Errore EEPROM (Uso Flag Globale????)

    //DelaymSec(10);
 }  
 
 


void BootLoaderAutoReset(void)
{
    if(EngineBox[0].AutoresetMagicNumber==K_MagicNumber)
    {
        asm ("RESET");                          // Software Reset Instruction          
    }
}