//----------------------------------------------------------------------------------
//	Progect name:	Core.c
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			Emanuele
//	Date:			22/05/2011
//	Description:	Corpo delle funzioni della logica di funzionamento
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
#include <math.h>
#include "DefinePeriferiche.h"
#include "Timer.h"
#include "Driver_Comunicazione.h"
#include "ProtocolloComunicazione.h"
#include "Driver_ModBus.h"
#include "ProtocolloModBus.h"
#include "PID.h"
#include "Core.h"
#include "FWSelection.h"
#include "ServiceComunication.h"
#include "delay.h"
//#include "PWM.h"




//VariabiliGlobaliPID();
extern volatile TypTimer    TimerElaborazioneC1;			// Timer Elaborazione
extern volatile TypTimer    TimerElaborazioneC2;			// Timer Elaborazione x secondo compressore
extern volatile TypTimer	TimerSwitchCompressor;		// Timer per time switch secondo compressore
extern volatile TypTimer	TimerIntegrateCompressor2;	// Timer per integrazione secondo compressore
extern volatile TypTimer	TimerDeintegrateCompressor2;// Timer per deintegrazione secondo compressore
extern volatile TypTimer    TimerSwitchPump;            // Timer per time switch seconda pompa
extern volatile TypTimer    TimerTrigPump;			    // Timer per freerun switch seconda pompa
extern volatile TypTimer    TimerPostPump;			    // Timer per freerun switch seconda pompa
extern volatile TypTimer    TimerCoolingSuperHeatErr;
extern volatile TypTimer    TimerExecPID;
extern volatile TypTimer    TimerCoolWarmValveError;
extern volatile TypTimer    TimerSuperHeatError;
extern volatile TypTimer    TimerTemperatureProbeErrorBox1;
extern volatile TypTimer    TimerTemperatureProbeErrorBox2;
extern volatile TypTimer    TimerCheckTemperatureProbe;
extern volatile TypTimer    TimerCheckPressureProbe;
extern volatile TypTimer    TimerPressureProbeErrorBox;
extern unsigned RTC_Flag;					// RTc Flag di sistema

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
void EngineBoxAutoModeSelection(TypEngineBox * CompNumber)
{   
    EngineBox[0].CompressorReqOn = 0;
    EngineBox[0].SplitPowerOnCnt = 0;
    EngineBox[0].SplitFrigo_On = 0;
    EngineBox[0].SplitFreezer_On = 0;
    
	if(EngineBox[0].Test.FluxReq | EngineBox[0].Test.EEV_Req)
	{	CompNumber->InverterRaffrescamento = 0;
		CompNumber->InverterRiscaldamento = 0;
		CompNumber->GoRiscaldamento = 0;
		CompNumber->GoRaffrescamento = 0;
		CompNumber->GoShutDwComp = 0;
	}
	else
	{
		if(!RTC_Flag)
		{
			for(i=0;i<Max_Room;i++)
			{	
                if(Room[i].OnLine == 1)
				{	
					if((Room[i].Mode == CoreRiscaldamento) || (Room[i].Mode == CoreRaffrescamento))
						EngineBox[0].CompressorReqOn += 1;
					if(Room[i].PowerOn == 1)
						EngineBox[0].SplitPowerOnCnt +=1;           // Conteggia quanti Split hanno il Power = ON
                    if((Room[i].FrozenMode == 1) && (Room[i].Mode == CoreRaffrescamento))
                    {
                        EngineBox[0].SplitFrigo_On = 1;
                        if(EngineBox[0].DefMode == CoreVentilazione)
                            EngineBox[0].DefMode = CoreRaffrescamento;
                    }
                    if((Room[i].FrozenMode == 2) && (Room[i].Mode == CoreRaffrescamento))
                    {
                        EngineBox[0].SplitFreezer_On = 1;       
                        if(EngineBox[0].DefMode == CoreVentilazione)
                            EngineBox[0].DefMode = CoreRaffrescamento;
                    }
				}
			}
		}			
		if(EngineBox[0].CompressorReqOn!=0 && !CompNumber->Error.GlobalStatusFlag && !CompNumber->PersErr.GlobalStatusFlag && CompNumber->SelectCompressor)
		{	
#if (K_EnableBrakeStart==1)				
			if(!CompNumber->HeatingEngineStart)					// Se la fase di preriscaldamento non è conclusa non proseguo
#endif				
			{
				if(EngineBox[0].DefMode == CoreRiscaldamento)	// se il compressore richiede caldo, riscaldo
				{
					CompNumber->GoRiscaldamento = (CompNumber->InverterRaffrescamento || !CompNumber->InverterRiscaldamento);
					if(CompNumber->CoolingMode)			// controllo se il compressore sta girando in stagione opposta 
						CompNumber->GoShutDwComp = 1;										// setto il flag per lo spegnimento
				}

				if(EngineBox[0].DefMode == CoreRaffrescamento)// se il compressore richiede freddo, raffreddo
				{
					CompNumber->GoRaffrescamento = (CompNumber->InverterRiscaldamento || !CompNumber->InverterRaffrescamento);
					if(CompNumber->HeatingMode)			// controllo se il compressore sta girando in stagione opposta
						CompNumber->GoShutDwComp = 1;										// setto il flag per lo spegnimento
				}
			}
		}
		else if(EngineBox[0].CompressorReqOn!=0 && CompNumber->Error.GlobalStatusFlag)						// se nessuno richiede la presenza del compressore
		{	
			if(EngineBox[0].SplitPowerOnCnt==0)
			{
				if(CompNumber->EngineIsOn)
					CompNumber->GoShutDwComp = 1;
			}
		}	
		else if(CompNumber->EngineIsOn)			// controllo se il compressore sta girando 
				CompNumber->GoShutDwComp = 1;										// setto il flag per lo spegnimento
	}
    
	
	CompNumber->CoolingMode = CompNumber->InverterRaffrescamento || CompNumber->GoRaffrescamento;	// Flag x segnalare modalità in avvio Raffrescamento o raggiunta
	CompNumber->HeatingMode = CompNumber->InverterRiscaldamento || CompNumber->GoRiscaldamento;	// Flag x segnalare modalità in avvio Riscaldamento o raggiunta
	CompNumber->EngineIsOn = CompNumber->CoolingMode || CompNumber->HeatingMode;												// Flag x segnalare Engine = ON
	
		
	
	
}




//------------------------------------------------------------------------------------
// Gestione Macchina a stati finiti per generare lo stato di funzionamento del compressore
//------------------------------------------------------------------------------------
int WorkEngineBoxStateMachine(TypEngineBox * CompNumber, TypTimer * TimerWork)
{
	
	//------------------------------------------------------------------------------------
	// Gestione macchina a stati per start/stop e commutazione caldo/freddo
	//------------------------------------------------------------------------------------
	if(CompNumber->GoRiscaldamento || CompNumber->GoRaffrescamento || CompNumber->GoShutDwComp)
	{	
        if(CompNumber->GoShutDwComp)											// se ho attiva la procedura di spegnimento
		{	
			if(CompNumber->GoRiscaldamento || CompNumber->GoRaffrescamento)	// e una di avvio
			{	
                CompNumber->GoRiscaldamento = 0;								// prevale quella di spegnimento
				CompNumber->GoRaffrescamento = 0;
				if((CompNumber->StatoCompressore > CHECK_COMP_WORK) || (CompNumber->StatoCompressore == START_SM))
                {
					CompNumber->StatoCompressore = CHECK_COMP_SPEED;
                    ///EngineBox[0].HeatingEngineStart = 0;        // Fermo la fase di riscaldamento
                }
			}
			else if((CompNumber->InverterRiscaldamento || CompNumber->InverterRaffrescamento) && (CompNumber->StatoCompressore == START_SM))
			{
				CompNumber->StatoCompressore = CHECK_COMP_SPEED;
			}
		}
		else if(CompNumber->PersErr.GlobalStatusFlag)	
		{
			CompNumber->GoRiscaldamento = 0;								// prevale quella di spegnimento
			CompNumber->GoRaffrescamento = 0;
			CompNumber->StatoCompressore == START_SM;
		}

		switch(CompNumber->StatoCompressore)
		{
			case START_SM:    //0:
				if(!CompNumber->PersErr.GlobalStatusFlag)
					CompNumber->StatoCompressore = CHECK_COMP_SPEED;			// inizializzo la sequenza low RPM x ShutDw
				break;
		
			case CHECK_COMP_SPEED:  //1:									// Low RPM
				if(CompNumber->Out_Inverter != Off_Speed)                  // controllo se il compressore sta girando
					CompNumber->StatoCompressore = SET_COMP_SPEED_OFF;     // passo allo stadio successivo
				else                                                        // se il compressore non sta girando
					CompNumber->StatoCompressore = CHECK_COMP_WORK;		// passo allo stadio successivo
				break;

			case SET_COMP_SPEED_OFF:  //2:			
				CompNumber->Request_Out_Inverter = Min_Speed;		// imposto la richiesta del compressore per lo spegnimento
				if(CompNumber->Out_Inverter <= Min_Speed)			// se si è spento
				{	
					//if(EngineBox[0].Error.Pressure_Lo || EngineBox[0].Error.Pressure_Hi)	//Se arrivato al minimo e errore ancora presente
					//	CompNumber->Out_Inverter = Off_Speed;		// spengo il compressore
					if(EngineBox[0].PersErr.PumpInverterFault/* || CompNumber->PersErr.CompInverterFault*/)		
						TimerWork->Time = WaitErrorRestart;		// imposto il timer per bloccare il compressore in modo veloce
					else
						TimerWork->Time = WaitTimeRestart;		// imposto il timer per bloccare il compressore
						
					TimerWork->Value = 0;						// per xx minuti prima di potersi riattivare
					TimerWork->TimeOut =0;
					TimerWork->Enable =1;
					CompNumber->StatoCompressore = CHECK_COMP_TIMEOFF;	// e vado allo stadio successivo
				}
				break;
			
			case CHECK_COMP_TIMEOFF: //3:
				/*if((TimerWork->Value>(WaitTimeRestart-120)) || 									// Un minuto prima di fermare la pompa
					EngineBox[0].Error.Pressure_Lo || EngineBox[0].Error.Pressure_Hi || 
					CompNumber->PersErr.ThermicComp || EngineBox[0].PersErr.FloodSensor)*/		// Oppure se in ciclo di spegnimento e errori permanenti
                CompNumber->Request_Out_Inverter = Off_Speed;   //Dico che richiesta è zero                					
                CompNumber->Out_Inverter = Off_Speed;												// spengo il compressore				
				//CompNumber->Request_Out_Inverter = Min_Speed;	// imposto la richiesta del compressore per lo spegnimento
                
                //if(CompNumber->GoShutDwComp && (TimerWork->Value>(TimerWork->Time-10)))
                //   CompNumber->StopPump = 1;                                               tentativo di switch doppia pompa quando inverter spento
				if((TimerWork->TimeOut == 1) || EngineBox[0].PersErr.FloodSensor)					// se è passato il tempo di shutDown oppure ho un allarme di iFloodSensor
				{	//CompNumber->Out_Inverter = Off_Speed;		// spengo il compressore
                //    CompNumber->StopPump = 0; 
					TimerWork->TimeOut =0;					// resetto il timer
					TimerWork->Enable =0;
					CompNumber->StatoCompressore = CHECK_COMP_WORK;// vado allo stadio successivo					
				}
				break;
			
			case CHECK_COMP_WORK:  //4:
				if(CompNumber->GoShutDwComp)						// se la richiesta era di shutDown
				{
					CompNumber->GoShutDwComp = 0;					// ho terminato
					CompNumber->InverterRaffrescamento = 0;		// resetto tutti i flag
					CompNumber->InverterRiscaldamento = 0;
					//DEBUG_VV	EngineBox[0].Out.Valvola_Caldo = 0;				// disattivo la valvola
					CompNumber->StatoCompressore = START_SM;		// e lo stato del compressore
					CompNumber->LastModeCompressor = EngineBox_Off;
					CompNumber->HeatingMode = 0;
					CompNumber->CoolingMode = 0;
					CompNumber->EngineIsOn = 0;
                    EngineBox[0].IdMasterSplit = 0; //Azzero quando si è spento il motore

				}
				else
                {
					/*
                    if(!EngineBox[0].CompressorIsMoreHot)
                    { 
                        EngineBox[0].HeatingEngineStart = 1;            // Avvio la fase di preriscaldamento del motore
                    }
					 */
                    TimerWork->Time = 60;							// imposto il timer per attendere il flusso
                    TimerWork->Value = 0;							// per 60 secondi
                    TimerWork->TimeOut =0;
                    TimerWork->Enable =0;
					CompNumber->StatoCompressore = WAIT_COMP_START;	// altrimenti vado allo stadio successivo
				}
                break;
			
			case WAIT_COMP_START: //5:				
                    //DEBUG_PMP	EngineBox[0].Out.Pompa_Acqua = 1;
                    //EngineBox[0].Out_Pompa = Pump_Min_Speed;			// Attivo la pompa		La Pompa nei traghetti è gestita da Inverter mentre qui da contatto rele
                    //DEBUG_VV EngineBox[0].Out.Valvola_Caldo = 0; 				// disattivo la valvola
                    TimerWork->Enable = 1;
#if ((K_SIMULATION_WORK==1) || (K_DisableSeaFlowStart==1))
					CompNumber->StatoCompressore = WAIT_CHECK_FLUX;
#else			
#if (K_AbilCompressor2==1)					
					if(!EngineBox[0].In.Flow && !EngineBox[0].Out.Pompa_Acqua) //(EngineBox[0].Out_Inverter==0) && (EngineBox[1].Out_Inverter==0))			// NOTA: Rivedere logica per funzionamento con compressore parallelo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					{
						TimerWork->Time = 60;							// imposto il timer per attendere il flusso
						TimerWork->Value = 0;							// per 60 secondi
						TimerWork->TimeOut =0;
						CompNumber->StatoCompressore = WAIT_CHECK_FLUX;
					}
					else if(EngineBox[0].In.Flow && EngineBox[0].Out.Pompa_Acqua) // && ((EngineBox[0].Out_Inverter!=0) || (EngineBox[1].Out_Inverter!=0)))
					{
						TimerWork->Time = 60;							// imposto il timer per attendere il flusso
						TimerWork->Value = 0;							// per 60 secondi
						TimerWork->TimeOut =0;
						CompNumber->StatoCompressore = WAIT_CHECK_FLUX;
					}
					else if(TimerWork->TimeOut)
					{
						EngineBox[0].PersErr.WaterSeaFlux = 1;
						CompNumber->StatoCompressore = START_SM;
					}
#else
					if(!EngineBox[0].In.Flow)			// NOTA: Rivedere logica per funzionamento con compressore parallelo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					{
						TimerWork->Time = 60;							// imposto il timer per attendere il flusso
						TimerWork->Value = 0;							// per 60 secondi
						TimerWork->TimeOut =0;
						CompNumber->StatoCompressore = WAIT_CHECK_FLUX;
					}
					else if(TimerWork->TimeOut)
					{
						EngineBox[0].PersErr.WaterSeaFlux = 1;
						CompNumber->StatoCompressore = START_SM;
					}
						
#endif
#endif					
				break;

			case WAIT_CHECK_FLUX: //6:
				if(TimerWork->TimeOut)						// se sono passati 60 sec e non ho flusso
#if(K_External_SeaW_Supply)
                    EngineBox[0].PersErr.WaterSeaFlux = 1;
#else
					EngineBox[0].Test.FluxReq = 1;					// lancio il test di flusso
#endif
				if(EngineBox[0].In.Flow)							// se ho il flusso
				{	
					TimerWork->Value = 0;						// resetto il timer 
					TimerWork->TimeOut =0;
					TimerWork->Enable =0;
					CompNumber->StatoCompressore = SET_COMP_SPEED_START;   // e proseguo con l'avvio
				}
				break;					

			case SET_COMP_SPEED_START:  //7:													// IMPORTATO DA CORE.C TRAGHETTI V4.119
				CompNumber->Out_Inverter = Min_Speed;				// Attivo il compressore
				if(CompNumber->GoRaffrescamento)					// controllo se era una richiesta di raffrescamento
				{	EngineBox[0].Ric_Temp = TempStartCompFreddo;	// imposto la temperatura richiesta
					EngineBox[0].Ric_Pressione = TempToPressureGasR410A(TempStartCompFreddo);	// e la pressione
					CompNumber->StatoCompressore = START_COOL_MODE;// vado allo stadio successivo
					CompNumber->LastModeCompressor = EngineBox_Freddo;
				}
				else
				{	if(CompNumber->GoRiscaldamento)				// se invece devo riscaldare
					{	
						//DEBUG_VV	EngineBox[0].Out.Valvola_Caldo =1;			// attivo la valvola
						/*
						for(i=0; i<Max_Room; i++)					// imposto l'ultima camera come split master
						{	if(Room[i].Mode == CoreRiscaldamento)
								EngineBox[0].IdMasterSplit = i;
						}
						*/
						EngineBox[0].Ric_Temp = TempStartCompCaldo;	// imposto la temp di richiesta
						EngineBox[0].Ric_Pressione = TempToPressureGasR410A(TempStartCompCaldo);	// e la pressione
						CompNumber->StatoCompressore = START_HOT_MODE;	// vado allo stadio successivo			// IMPORTATO DA CORE.C TRAGHETTI V4.119
						CompNumber->LastModeCompressor = EngineBox_Caldo;
					}
					else                                                    // altrimenti
						CompNumber->StatoCompressore = CHECK_COMP_SPEED;   // torno all'inizio per lo spegnimento
				}
				break;

			case START_COOL_MODE:	//8															// START IN FREDDO		
				if(CompNumber->PersErr.GlobalStatusFlag ||	CompNumber->GoShutDwComp)
				{
                    CompNumber->StatoCompressore = START_SM;									// resetto lo stato del compressore				
                }
				else
				{
					EngineBox[0].Ric_Temp = TempStartCompFreddo;                                // imposto la temperatura richiesta
					EngineBox[0].Ric_Pressione = TempToPressureGasR410A(TempStartCompFreddo);	// e la pressione
#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)
					if((EngineBox[0].Pressure.Gas > EngineBox[0].Ric_Pressione) && (EngineBox[0].Pressure.LiquidCond < PressStartLiqFreddo) && (EngineBox[0].Out_Inverter==0 || EngineBox[1].Out_Inverter==0) && 
                      (EngineBox[0].TotalPowerBoxAbsorption <= Comp_Inverter[0].Max_Power_Inverter) && (CompNumber->BoxAbsorption<=CompNumber->MaxInverterAbsorption) && !CompNumber->Error.GlobalStatusFlag)                  // se ancora non è arrivato in temperatura e non ci sono errori attivi
#else
					if((EngineBox[0].Pressure.Gas > EngineBox[0].Ric_Pressione) && (EngineBox[0].Pressure.Liquid < PressStartLiqFreddo) && (EngineBox[0].Out_Inverter==0 || EngineBox[1].Out_Inverter==0) && 
                      (EngineBox[0].TotalPowerBoxAbsorption <= Comp_Inverter[0].Max_Power_Inverter) && (CompNumber->BoxAbsorption<=CompNumber->MaxInverterAbsorption) && !CompNumber->Error.GlobalStatusFlag)                  // se ancora non è arrivato in temperatura e non ci sono errori attivi
#endif
                        CompNumber->Request_Out_Inverter = EngineBox[0].Inverter_Max_Power_Out;
					else                                                                        // altrimenti
					{
                        if(!CompNumber->Error.GlobalStatusFlag)
                            CompNumber->Request_Out_Inverter = CompNumber->Out_Inverter;          // setto il valore dell'inverter
						CompNumber->GoRaffrescamento = 0;                                      // ho terminato
						CompNumber->InverterRaffrescamento = 1;                                // reimposto tutti i flag
						CompNumber->InverterRiscaldamento = 0;
						CompNumber->StatoCompressore = START_SM;                               // resetto lo stato del compressore
					}
				}
				break;
				
			case START_HOT_MODE:	//9															// START IN CALDO					
				if(CompNumber->PersErr.GlobalStatusFlag || CompNumber->GoShutDwComp)
                {
					CompNumber->StatoCompressore = START_SM;									// resetto lo stato del compressore	
                }
                else
				{
					EngineBox[0].Ric_Temp = TempStartCompCaldo;									// imposto la temp di richiesta
					EngineBox[0].Ric_Pressione = TempToPressureGasR410A(TempStartCompCaldo);	// e la pressione
					if((EngineBox[0].Pressure.Gas < EngineBox[0].Ric_Pressione) && (EngineBox[0].Out_Inverter==0 || EngineBox[1].Out_Inverter==0) &&
                      (EngineBox[0].TotalPowerBoxAbsorption <= Comp_Inverter[0].Max_Power_Inverter) && (CompNumber->BoxAbsorption<=CompNumber->MaxInverterAbsorption) && !CompNumber->Error.GlobalStatusFlag)					// se ancora non è arrivato in temperatura e non ci sono allarmi attivi

    					CompNumber->Request_Out_Inverter = EngineBox[0].Inverter_Max_Power_Out;
					else																		// altrimenti
					{	
                        if(!CompNumber->Error.GlobalStatusFlag)
                            CompNumber->Request_Out_Inverter = CompNumber->Out_Inverter;			// setto il valore dell'inverter
						CompNumber->GoRiscaldamento = 0;										// ho terminato
						CompNumber->InverterRaffrescamento = 0;								// reimposto tutti i flag
						CompNumber->InverterRiscaldamento = 1;
						CompNumber->StatoCompressore = START_SM;								// resetto lo stato del compressore
					}
				}
				break;
		}
        
        return 0;
	}
    else return 1;
}



//------------------------------------------------------------------------------------
// Gestione Variazione velocità compressore => potenza erogata
//------------------------------------------------------------------------------------
void SpeedPower_CompressorRegulation(int index, TypEngineBox * NumComp, volatile TypTimer * TimerElaborazione)
{
	int ErroreP = 0;
    int CompTempHi = 0;
	static int PowLimflag[2]={0,0};
	static int vNumRegola[2]={0,0}; 
	static char ValueIsInRange=0;
    static int Index0Check=0;
    
    if(PowLimflag[index]==1)       
        TimerElaborazione->Time = TimeIncOutInverter_PowerReduce;        
    else if(!ValueIsInRange)
        TimerElaborazione->Time = TimeIncOutInverter_ValueInRange;
    else if(EngineBox[0].DefMode==CoreRiscaldamento)
        TimerElaborazione->Time = TimeIncOutInverter_Heating;
    else
        TimerElaborazione->Time = TimeIncOutInverter_Cooling;
                
    if(EngineBox[0].StatoCompressore!=START_SM || ((EngineBox[0].Out_Inverter==0)&&(EngineBox[0].Out_Inverter==0)))
        Index0Check=0;
    else if(index==0&&EngineBox[0].Out_Inverter>0)
        Index0Check=1;    
    
    // -----------------------------------
	// Limitazione di potenza
    // -----------------------------------
    
    if(Comp_Inverter[0].PermanentOffLine==0)
        Comp_Inverter[0].Filtered_Out_Current = Filter(&FilterInverterOut_Current1, Comp_Inverter[0].Out_Current, K_Campioni_FilterInverterOut_Current);
    else
        Comp_Inverter[0].Filtered_Out_Current = 0;
        
#if(K_AbilCompressor2==1)
    if(Comp_Inverter[1].PermanentOffLine==0)
        Comp_Inverter[1].Filtered_Out_Current = Filter(&FilterInverterOut_Current2, Comp_Inverter[1].Out_Current, K_Campioni_FilterInverterOut_Current);
    else
        Comp_Inverter[1].Filtered_Out_Current = 0;
#endif

    if(Comp_Inverter[0].Filtered_Out_Current > Comp_Inverter[1].Filtered_Out_Current)
        Comp_Inverter[0].Highest_Filtered_Out_Current = Comp_Inverter[0].Filtered_Out_Current;
    else
        Comp_Inverter[0].Highest_Filtered_Out_Current = Comp_Inverter[1].Filtered_Out_Current;
    
    if((EngineBox[0].TotalPowerBoxAbsorption > Comp_Inverter[0].Max_Power_Inverter) || (Comp_Inverter[0].Highest_Filtered_Out_Current > K_Inv_PercLimit_Out_Current) || 
       (EngineBox[0].BoxAbsorption > EngineBox[0].MaxInverterAbsorption) || (EngineBox[1].BoxAbsorption > EngineBox[1].MaxInverterAbsorption))		
    {																	
        if(!PowLimflag[index])
        {
            PowLimflag[index]=1;
            //EngineBox[0].Inverter_Max_Power_Out = Room[1].PowerLimit;
            EngineBox[0].Inverter_Max_Power_Out = NumComp->Request_Out_Inverter;	
        }
        else
        {
            //PowLimflag[index]=0;
            Decrement(&(EngineBox[0].Inverter_Max_Power_Out), 5, Min_Speed);	// Comincio a "frenare"	
        }
    }
    else
    {
        PowLimflag[index]=0;
        if(EngineBox[0].DefMode == CoreRiscaldamento)
            Increment(&(EngineBox[0].Inverter_Max_Power_Out), 5, Max_Speed_Heating);	// Reimposto limiti massimi
        else
            Increment(&(EngineBox[0].Inverter_Max_Power_Out), 5, Max_Speed_Cooling);	// Reimposto limiti massimi
    }

	//Allineamento dei due compressori in aggancio con partenza del secondo.
	if((EngineBox[0].Request_Out_Inverter != EngineBox[1].Request_Out_Inverter) && (index==0))	//Controllo che non siano allineati prima del movimento del primo compressore
       ValueIsInRange = 0;														//e in caso affermativo dico che non sono in range.
//	if((EngineBox[0].Request_Out_Inverter == EngineBox[1].Request_Out_Inverter) && (index==1))	//Controllo che siano allineati prima del movimento del primo compressore
//       ValueIsInRange = 1;														//e in caso affermativo dico che sono in range
	if(!EngineBox[0].SelectCompressor || !EngineBox[1].SelectCompressor || (EngineBox[0].StatoCompressore != START_SM) || (EngineBox[1].StatoCompressore != START_SM) || EngineBox[0].Error.GlobalStatusFlag || EngineBox[1].Error.GlobalStatusFlag || EngineBox[0].PersErr.GlobalStatusFlag || EngineBox[1].PersErr.GlobalStatusFlag)	//Controllo che non siano tutti e due accesi e che non ci siano errori attivi su uno dei due compressori
		ValueIsInRange = 1;														//e in caso affermativo faccio finta di essere in range per svincolarli uno dall'altro
		
	if(EngineBox[0].SelectCompressor && EngineBox[1].SelectCompressor && !EngineBox[0].Error.GlobalStatusFlag &&
	   !EngineBox[0].PersErr.GlobalStatusFlag && !EngineBox[1].PersErr.GlobalStatusFlag && !ValueIsInRange)
	{
		if(EngineBox[0].Inverter_Max_Power_Out>((EngineBox[0].Out_Inverter+EngineBox[1].Out_Inverter)/2))
		{
			EngineBox[0].Inverter_Max_Power_Out = (EngineBox[0].Out_Inverter+EngineBox[1].Out_Inverter)/2;
			if(EngineBox[0].Inverter_Max_Power_Out<Min_Speed)
				EngineBox[0].Inverter_Max_Power_Out=Min_Speed;
		}	
	}

#if(K_AbilCompressor2==1)
    if((EngineBox[0].Out_Inverter==0) || (EngineBox[1].Out_Inverter==0) || EngineBox[0].Error.CompressorHi || EngineBox[1].Error.CompressorHi)
        CompTempHi = EngineBox[index].Temperature.Compressor_Output;
    else
        CompTempHi = EngineBox[0].Temperature.Max_Compressor_Output;
#else
    CompTempHi = EngineBox[0].Temperature.Compressor_Output;
#endif
	
	if(!(NumComp->PersErr.GlobalStatusFlag || NumComp->Error.GlobalStatusFlag))
	{	
	//----------------------------------------------------------------------------------------------------
	// v4.4.117 
	// Gestione limitazione potenza compressore da dato di potenza erogata letta via ModBus dall'inverter
	// Se supero il limite di potenza imposto da slider Touch (calcolato a partire da Room[1].PowerLimit... vedi routine in 
	// ProtocolloComunicazione.c -> Comp_Inverter[0].Max_Power_Inverter = (unsigned int)((float)Room[k_Split_Master_Add].PowerLimit * (float)k_taglia_inverter)/255
	//----------------------------------------------------------------------------------------------------			
        if((index==0) || ((index==1) && (!ValueIsInRange || (EngineBox[0].Out_Inverter==Off_Speed) || (EngineBox[0].StatoCompressore!=START_SM) || EngineBox[0].Error.GlobalStatusFlag || EngineBox[0].PersErr.GlobalStatusFlag)))		
        {
            if((TimerElaborazione->TimeOut == 1) && (NumComp->Request_Out_Inverter == NumComp->Out_Inverter) && !NumComp->Error.GlobalStatusFlag && !NumComp->PersErr.GlobalStatusFlag)
            {	
                TimerElaborazione->TimeOut = 0;				// v4.4.117
                TimerElaborazione->Value = 0;				// v4.4.117

                if(EngineBox[index].Out_Inverter>EngineBox[0].Inverter_Max_Power_Out)
                {
                    Decrement(&(EngineBox[index].Request_Out_Inverter), 1, EngineBox[0].Inverter_Max_Power_Out);
                }
                else if(!ValueIsInRange)	//Sati i compressori, indipendentemente dalla modalità di lavoro li allineo
                {
                    if(EngineBox[0].Request_Out_Inverter>EngineBox[1].Request_Out_Inverter && (index==0))
                    {
                        Decrement(&(EngineBox[index].Request_Out_Inverter), 1, EngineBox[0].Inverter_Max_Power_Out);
                    }
                    if(EngineBox[0].Request_Out_Inverter>EngineBox[1].Request_Out_Inverter && (index==1))
                    {
                        Increment(&(EngineBox[index].Request_Out_Inverter), 1, EngineBox[0].Inverter_Max_Power_Out);
                    }
                    if(EngineBox[0].Request_Out_Inverter<EngineBox[1].Request_Out_Inverter && (index==0))
                    {
                        Increment(&(EngineBox[index].Request_Out_Inverter), 1, EngineBox[0].Inverter_Max_Power_Out);
                    }
                    if(EngineBox[0].Request_Out_Inverter<EngineBox[1].Request_Out_Inverter && (index==1))
                    {
                        Decrement(&(EngineBox[index].Request_Out_Inverter), 1, EngineBox[0].Inverter_Max_Power_Out);
                    }
                    if((EngineBox[0].Request_Out_Inverter == EngineBox[1].Request_Out_Inverter) && (index==1))	//Controllo che siano allineati prima del movimento del primo compressore
                       ValueIsInRange = 1;														//e in caso affermativo dico che sono in range
                }
                else	//e ricomincio a lavorare
                {	
                    if(NumComp->InverterRiscaldamento)		// Riscaldamento
                    {	
                        EngineBox[0].Ric_Temp = CompressorTemp();
                        EngineBox[0].Ric_Pressione = TempToPressureGasR410A(EngineBox[0].Ric_Temp);
                        EngineBox[0].Ric_Pressione = Filter(&FilterEvap_PressCaldo, EngineBox[0].Ric_Pressione, K_Campioni_FilterEvapPressCaldo); //10);

#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)
                        if((EngineBox[0].Pressure.LiquidCond > (PressioneLo_Liq_P+2000)) && (CompTempHi < EngineBox[0].TempRestartCompressorHi))
#else
                        if((EngineBox[0].Pressure.Liquid > (PressioneLo_Liq_P+2000)) && (CompTempHi < EngineBox[0].TempRestartCompressorHi))
#endif
                        {					
                            ErroreP = EngineBox[0].Ric_Pressione - EngineBox[0].Pressure.Gas;
                            if(ErroreP > Soglia_Inverter_Caldo_LO)
                            {	
                                Increment(&(EngineBox[index].Request_Out_Inverter), 1, EngineBox[0].Inverter_Max_Power_Out);
                            }	
                            else
                            {	
                                ErroreP = EngineBox[0].Pressure.Gas - EngineBox[0].Ric_Pressione;
                                if(ErroreP > Soglia_Inverter_Caldo_HI)
                                {	
                                    Decrement(&(EngineBox[index].Request_Out_Inverter), 1, Min_Speed);
                                }
                            }	
                        }
#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)
                        else if(((EngineBox[0].Pressure.LiquidCond <= (PressioneLo_Liq_P+2000)) && (EngineBox[0].Pressure.LiquidCond >= (PressioneLo_Liq_P+1000))) || (CompTempHi >= EngineBox[0].TempRestartCompressorHi))
#else
                        else if(((EngineBox[0].Pressure.Liquid <= (PressioneLo_Liq_P+2000)) && (EngineBox[0].Pressure.Liquid >= (PressioneLo_Liq_P+1000))) || (CompTempHi >= EngineBox[0].TempRestartCompressorHi))    
#endif                            
                        {
                            ErroreP = EngineBox[0].Pressure.Gas - EngineBox[0].Ric_Pressione;
                            if(ErroreP > Soglia_Inverter_Caldo_HI)
                            {	
                                Decrement(&(EngineBox[index].Request_Out_Inverter), 1, Min_Speed);
                            }
                        }
#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)
                        else if(EngineBox[0].Pressure.LiquidCond < (PressioneLo_Liq_P+1000))
#else
                        else if(EngineBox[0].Pressure.Liquid < (PressioneLo_Liq_P+1000))
#endif
                        {	
                            vNumRegola[index] = vNumRegola[index]+1;

                            if(vNumRegola[index]>=2)
                            {
                                Decrement(&(EngineBox[index].Request_Out_Inverter), 1, Min_Speed);
                                vNumRegola[index]=0;
                            }
                        }				
                    }

                    if(NumComp->InverterRaffrescamento)		//raffrescamento
                    {	
                        EngineBox[0].Ric_Temp = CompressorTemp();
                        EngineBox[0].Ric_Pressione = TempToPressureGasR410A(EngineBox[0].Ric_Temp);
                        EngineBox[0].Ric_Pressione = Filter(&FilterEvap_PressFreddo, EngineBox[0].Ric_Pressione, K_Campioni_FilterEvapPressFreddo);				
                        //EngineBox[0].HoldErr = EngineBox[0].Pressure.Gas;

#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)
                        if((EngineBox[0].Pressure.LiquidCond < (PressioneHi_Liq_P-1000)) && (CompTempHi < EngineBox[0].TempRestartCompressorHi))
#else
                        if((EngineBox[0].Pressure.Liquid < (PressioneHi_Liq_P-1000)) && (CompTempHi < EngineBox[0].TempRestartCompressorHi))                            
#endif
                        {
                            ErroreP = EngineBox[0].Pressure.Gas - EngineBox[0].Ric_Pressione;
                            if(ErroreP > Soglia_Inverter_Freddo_HI)
                            {	
                                Increment(&(EngineBox[index].Request_Out_Inverter), 1, EngineBox[0].Inverter_Max_Power_Out);
                            }
                            else
                            {	
                                ErroreP = EngineBox[0].Ric_Pressione - EngineBox[0].Pressure.Gas;
                                if(ErroreP > Soglia_Inverter_Freddo_LO)
                                {	
                                    Decrement(&(EngineBox[index].Request_Out_Inverter), 1, Min_Speed);
                                }
                            }
                        }
#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)                    
                        else if(((EngineBox[0].Pressure.LiquidCond >= (PressioneHi_Liq_P-1000)) && (EngineBox[0].Pressure.LiquidCond <= (PressioneHi_Liq_P-500))) || (CompTempHi >= EngineBox[0].TempRestartCompressorHi))
#else
                        else if(((EngineBox[0].Pressure.Liquid >= (PressioneHi_Liq_P-1000)) && (EngineBox[0].Pressure.Liquid <= (PressioneHi_Liq_P-500))) || (CompTempHi >= EngineBox[0].TempRestartCompressorHi))
#endif
                        {
                            ErroreP = EngineBox[0].Ric_Pressione - EngineBox[0].Pressure.Gas;
                            if(ErroreP > Soglia_Inverter_Freddo_LO)
                            {	
                                    Decrement(&(EngineBox[index].Request_Out_Inverter), 1, Min_Speed);
                            }
                        }
#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)                       
                        else if(EngineBox[0].Pressure.LiquidCond > (PressioneHi_Liq_P-500))
#else
                        else if(EngineBox[0].Pressure.Liquid > (PressioneHi_Liq_P-500))                            
#endif
                        {	
                            vNumRegola[index] = vNumRegola[index]+1;

                            if(vNumRegola[index]>=2)
                            {
                                Decrement(&(EngineBox[index].Request_Out_Inverter), 1, Min_Speed);
                                vNumRegola[index]=0;
                            }
                        }
                    }
                }
            }
        } 
        else if((index==1)&&ValueIsInRange&&(Index0Check==1)&&(EngineBox[1].Out_Inverter>0))
            EngineBox[1].Request_Out_Inverter = EngineBox[0].Request_Out_Inverter;
	}        
}


void EngineBox_ModeSelection(TypEngineBox * CompNumber)
{
	// aggiorno le variabili con i cumulativi
	CompNumber->Mode = EngineBox_Off;
	
	if(CompNumber->InverterRaffrescamento)
		CompNumber->Mode = EngineBox_Freddo;
	
	if(CompNumber->GoRaffrescamento)
		CompNumber->Mode = EngineBox_GoFreddo;
	
	if(CompNumber->InverterRiscaldamento)
		CompNumber->Mode = EngineBox_Caldo;
	
	if(CompNumber->GoRiscaldamento)
		CompNumber->Mode = EngineBox_GoCaldo;
	
	if(CompNumber->GoShutDwComp || 
		((CompNumber->StatoCompressore < WAIT_COMP_START) && (CompNumber->StatoCompressore > START_SM)))
		CompNumber->Mode = EngineBox_GoOff;
	
	//if(CompNumber->PersErr.CaricaRefrig | CompNumber->PersErr.WaterSeaFlux |/*CompNumber->PersErr.EEV_Open|
	//	CompNumber->PersErr.EEV_Close|*/ CompNumber->PersErr.FloodSensor | CompNumber->PersErr.ThermicComp|
	//	CompNumber->PersErr.SeaWaterLo | CompNumber->PersErr.SeaWaterHi)
	if(CompNumber->PersErr.GlobalStatusFlag && !CompNumber->GoShutDwComp)	
		CompNumber->Mode = EngineBox_CriticalFault;   
}



/*  *****************************************************************************************************************************
	Gestico il compressore, start/stop e cambio di stagione
	***************************************************************************************************************************** */
void CompressorCheck(void)
{
    
#if (K_AbilCompressor2==1)
    if(((EngineBox[1].Temperature.Compressor_Output > EngineBox[0].Temperature.Compressor_Output) && (EngineBox[1].Out_Inverter>0)) ||
        (EngineBox[0].Out_Inverter==0))
        EngineBox[0].Temperature.Max_Compressor_Output = EngineBox[1].Temperature.Compressor_Output;
    else
        EngineBox[0].Temperature.Max_Compressor_Output = EngineBox[0].Temperature.Compressor_Output;                
#else
    EngineBox[0].Temperature.Max_Compressor_Output = EngineBox[0].Temperature.Compressor_Output;
#endif
	
	SelectWorkCompressor();						// Gestisce la partenza arresto e la selezione del compressore di lavoro
    AbsorptionManagement();                     //Controllo i dati di assorbimento del compressore
    EngineBoxAutoModeSelection(&EngineBox[0]);
    if(WorkEngineBoxStateMachine(&EngineBox[0], &TimerSmBox1))                                 // Se sono in una di queste fasi: GoRiscaldamento || GoRaffrescamento GoShutDwComp gestisto FSM
		SpeedPower_CompressorRegulation(0, &EngineBox[0], &TimerElaborazioneC1);        // Altrimenti gestisco Variazione velocità compressore => potenza erogata
	EngineBox_ModeSelection(&EngineBox[0]);
	///CompressorHeatingEngine(&TimerHeatingEngine1, 0, Add_Comp_Inverter1);			// Gestione preriscaldamento motore
	CompressorAdjustment(&EngineBox[0], &TimerCompressoreC1);		// gestico l'uscita del compressore
	
#if (K_AbilCompressor2==1)
	EngineBoxAutoModeSelection(&EngineBox[1]);
    if(WorkEngineBoxStateMachine(&EngineBox[1], &TimerSmBox2))                                 // Se sono in una di queste fasi: GoRiscaldamento || GoRaffrescamento GoShutDwComp gestisto FSM
        SpeedPower_CompressorRegulation(1, &EngineBox[1], &TimerElaborazioneC2);        // Altrimenti gestisco Variazione velocità compressore => potenza erogata
	EngineBox_ModeSelection(&EngineBox[1]);
	///CompressorHeatingEngine(&TimerHeatingEngine2, 1, Add_Comp_Inverter2);			// Gestione preriscaldamento motore
	CompressorAdjustment(&EngineBox[1], &TimerCompressoreC2);		// gestico l'uscita del compressore
#endif
	
	WarmColdValveManagement();	
	
    
}




/*  *****************************************************************************************************************************
	Gestisco le variazioni di potenza del compressore per avere una variazione continua
	***************************************************************************************************************************** */
void CompressorAdjustment(TypEngineBox * CompNumber, TypTimer * TimerCompressore)
{	
	int Increase = 0;
	//static int CompNumber->ultimoerrore=0;		// Flag che segnala rientro da errore:usato per ora per ripristinare in "Request_Out_Inverter" l'ultimo valore di "Out_Inverter"

	if(EngineBox[0].PersErr.PumpInverterFault || CompNumber->PersErr.CompInverterFault)
	{
		TimerCompressore->Time = TimeEmergencySlewRate;
		Increase = EmergencyStepVariation;
		CompNumber->Request_Out_Inverter = Min_Speed;
		if(CompNumber->Out_Inverter<=Min_Speed)
			CompNumber->Out_Inverter=Off_Speed;
	}
	else if(EngineBox[0].Error.Pressure_Lo || EngineBox[0].Error.Pressure_Hi)	// ** 4.4.122 - Separata gestione da Allagamento&TermicaCompressore&CondensatoreLo
	{	
		TimerCompressore->Time = TimeEmergencyPressureSlewRate;
		Increase = EmergencyPressureStepVariation;
		CompNumber->ultimoerrore=1;
	}		
	else if(EngineBox[0].PersErr.FloodSensor || CompNumber->PersErr.ThermicComp || EngineBox[0].Error.CondensatoreLo /*|| EngineBox[0].Error.Gas_Recovery*/ || EngineBox[0].Test.FluxReq)
	{
		TimerCompressore->Time = TimeEmergencySlewRate;
		Increase = EmergencyStepVariation;
		CompNumber->ultimoerrore=1;
	}	
	else if(CompNumber->Error.CompressorHi || EngineBox[0].Error.CompressorLo || EngineBox[0].Error.CondensatoreHi)
	{
		TimerCompressore->Time = TimeErrLowPriorSlewRate;
		Increase = IncOutInverter_Low_Critical;
		CompNumber->ultimoerrore=1;

		if(((EngineBox[0].Pressure.Gas > EngineBox[0].Ric_Pressione) && CompNumber->InverterRiscaldamento) || 
			((EngineBox[0].Pressure.Gas < EngineBox[0].Ric_Pressione) && CompNumber->InverterRaffrescamento))
		{
			TimerCompressore->Time = TimeSlowSlewRate;
			Increase = IncOutInverter;
		}
		else  if(CompNumber->GoShutDwComp)
		{
			TimerCompressore->Time = TimeShutDown;
			//Increase = 15;
			Increase = 4;
			if(EngineBox[0].PersErr.PumpInverterFault || CompNumber->PersErr.CompInverterFault)
			{
				TimerCompressore->Time = TimeEmergencySlewRate;
				Increase = EmergencyStepVariation;
			}	
		}
	}	
	else if( (CompNumber->Error.Recharge_Oil==1) && (((EngineBox[0].Pressure.Gas<=K_RecOil_GasG_Press) && CompNumber->InverterRiscaldamento) || 
			((EngineBox[0].Pressure.Liquid<=K_RecOil_LiqP_Press) && CompNumber->InverterRaffrescamento)))	
	{
		TimerCompressore->Time = TimeRecOilUpSlewRate;
		Increase = RecOilUpStepVariation;
		CompNumber->Request_Out_Inverter = EngineBox[0].Inverter_Max_Power_Out;
		CompNumber->ultimoerrore=1;
	}
	else if((CompNumber->Error.Recharge_Oil==1) && (((EngineBox[0].Pressure.Gas>K_RecOil_GasG_Press) && CompNumber->InverterRiscaldamento) ||
			((EngineBox[0].Pressure.Liquid>K_RecOil_LiqP_Press) && CompNumber->InverterRaffrescamento)))
	{
		TimerCompressore->Time = TimeRecOilDwSlewRate;
		Increase = RecOilDwStepVariation;
		CompNumber->Request_Out_Inverter = Min_Speed;
		CompNumber->ultimoerrore=1;
	}
	else
	{
		if(CompNumber->GoRiscaldamento)
		{
			TimerCompressore->Time = TimeStartHotSlewRate;
			Increase = IncOutInverter;
		}	
        else if(CompNumber->GoRaffrescamento)
        {
			TimerCompressore->Time = TimeStartColdSlewRate;
			Increase = IncOutInverter;            
        }
		else if(CompNumber->GoShutDwComp)
		{
			TimerCompressore->Time = TimeShutDown;
			Increase = IncOutInverter; //15;
			/*
			if(EngineBox[0].PersErr.PumpInverterFault || CompNumber->PersErr.CompInverterFault)
			{
				TimerCompressore->Time = TimeEmergencySlewRate;
				Increase = EmergencyStepVariation;
				CompNumber->Request_Out_Inverter = Min_Speed;
				if(CompNumber->Out_Inverter<=Min_Speed)
					CompNumber->Out_Inverter=Off_Speed;
			}
			*/	
		}
		else if((CompNumber->InverterRiscaldamento == 1) || (CompNumber->InverterRaffrescamento == 1))
		{
			TimerCompressore->Time = TimeSlowSlewRate;
			Increase = 0;
		}
	}

	if(!TimerCompressore->Enable)
		TimerCompressore->Value = 0;
	TimerCompressore->Enable = (CompNumber->Out_Inverter > Off_Speed);

	if(!TimerCompressore->Enable)
		TimerCompressore->TimeOut = 0;

	if(TimerCompressore->TimeOut)
	{
		if(Increase == 0)
		{
			if(CompNumber->ultimoerrore) 
			{
				CompNumber->Request_Out_Inverter=CompNumber->Out_Inverter;
				CompNumber->ultimoerrore=0;
			}
			
			if(CompNumber->Out_Inverter != CompNumber->Request_Out_Inverter)
				CompNumber->Out_Inverter = CompNumber->Request_Out_Inverter;

		}
		else
		{
			if(CompNumber->Request_Out_Inverter != CompNumber->Out_Inverter)
			{
                if((EngineBox[0].DefMode == CoreRiscaldamento) && (CompNumber->Request_Out_Inverter>Max_Speed_Heating))    //Se il limite supera la richiesta massima in caldo
                    CompNumber->Request_Out_Inverter=Max_Speed_Heating;                                             //raso al limite
                if((EngineBox[0].DefMode == CoreRaffrescamento) && (CompNumber->Request_Out_Inverter>Max_Speed_Cooling))   //Se il limite supera la richiesta massima in freddo
                    CompNumber->Request_Out_Inverter=Max_Speed_Cooling;                                             //raso al limite

				if(CompNumber->Request_Out_Inverter < CompNumber->Out_Inverter)
					Decrement(&(CompNumber->Out_Inverter), Increase, Min_Speed);
				else
					Increment(&(CompNumber->Out_Inverter), Increase, EngineBox[0].Inverter_Max_Power_Out);
			}
		}
		TimerCompressore->TimeOut = 0;
	}
}

/*  *****************************************************************************************************************************
	Recupero la temperatura di evaporazione idonea alla modalità corrente
	***************************************************************************************************************************** */
int CompressorTemp (void)
{
	int FindTemp = 0;
	char i=0;
    int  TempScore = 0;
    int  SumDeltaScore = 0;
    int  SumOutScore = 0;
    int  SumTotalScore = 0;    
    int  NumberSplitConnect = 0;
    int  ScoreMaxSplitConnect = 0;
    int  MaxLoadPercent = 0;
    
    
#if (K_AbilCompressor2==1)
	if(EngineBox[0].InverterRaffrescamento || EngineBox[1].InverterRaffrescamento)
#else
	if(EngineBox[0].InverterRaffrescamento)
#endif
	{								// raffrescamento, cerco la temperatura più bassa
#if(K_New_ReqPressModeCooling==1)
        FindTemp = 8000;		// inizializzo a 80°C
        for(i=0;i<Max_Room;i++)
        {
            if(Room[i].Enable == 1)
            {
                NumberSplitConnect += 1;    //Numero di split connessi al sistema (anche se non OnLine)
                ScoreMaxSplitConnect += 20;  //Ogni split pesa 20 punti per il totale di carico assorbimento
            }
            if(Room[i].Mode == CoreRaffrescamento)
            {
                if(FindTemp > (Room[i].Temp_Evaporazione))  //serve per indicare quale split sta chiedendo
				{
					FindTemp = Room[i].Temp_Evaporazione;
					EngineBox[0].IdMasterSplit = i;
				}
                
                TempScore = ((Room[i].AirTemp - Room[i].SetPoint)/100)*2;   //il setpoint vale dopppio rispetto la temperatura di uscita
                if(TempScore>0)
                {
                    if(TempScore>10)
                        TempScore=10;
                    SumDeltaScore = SumDeltaScore + TempScore;
                }
                
                TempScore = (Room[i].OutTemp - K_MaxAirOut_PressWork)/100;
                if(TempScore>0)
                {
                    if(TempScore>10)
                        TempScore=10;                    
                    SumOutScore = SumOutScore + TempScore;
                }
            }
        }
        
        SumTotalScore = SumDeltaScore + SumOutScore;
        MaxLoadPercent = (int)((round((float)SumTotalScore/(float)ScoreMaxSplitConnect)) * 100.0);
                
       //MaxLoadPercent >=100 press 5.8
       //MaxLoadPercent < 100 e > 50  press da 5.8 a 6.3
       //MaxLoadPercent < 50 e > 20  press da 6.3 a 7
       //MaxLoadPercent <20 press 7 
     
        FindTemp = Delta2ReqPress(MaxLoadPercent);        
#else
		FindTemp = 8000;		// inizializzo a 80°C
		for(i=0;i<Max_Room;i++)
		{
			if(Room[i].Mode == CoreRaffrescamento)
			{
				if(FindTemp > (Room[i].Temp_Evaporazione))
				{
					FindTemp = Room[i].Temp_Evaporazione;
					EngineBox[0].IdMasterSplit = i;
				}
			}
		}
#endif
	}
#if (K_AbilCompressor2==1)
	if(EngineBox[0].InverterRiscaldamento || EngineBox[1].InverterRiscaldamento)
#else
	if(EngineBox[0].InverterRiscaldamento)

#endif
	{								// riscaldamento, cerco la temperatura più alta
		for(i=0;i<Max_Room;i++)
		{
			if(Room[i].Mode == CoreRiscaldamento)
			{
				if(FindTemp < (Room[i].Temp_Evaporazione))
				{
					FindTemp = Room[i].Temp_Evaporazione;
					EngineBox[0].IdMasterSplit = i;
				}
			}
		}
	}
	return FindTemp;
}

#if(K_New_ReqPressModeCooling==1)
int Delta2ReqPress(int LoadPerc)
{
    int Min_Temp;
    int Mid_Temp;
    int Max_Temp;
    
    Min_Temp = PressureGasToTempR410A(K_Min_Press);
    Mid_Temp = PressureGasToTempR410A(K_Mid_Press);
    Max_Temp = PressureGasToTempR410A(K_Max_Press);
    
    if(LoadPerc < K_Min_Load)   return Min_Temp;
	if(LoadPerc <= K_Mid_Load)   return (int)(ValueIn2ValueOut(LoadPerc, K_Min_Load, K_Mid_Load, Min_Temp, Mid_Temp)); 
	if(LoadPerc <= K_Max_Load)  return (int)(ValueIn2ValueOut(LoadPerc, K_Mid_Load, K_Max_Load, Mid_Temp, Max_Temp)); 
    if(LoadPerc > K_Max_Load)  return Max_Temp; 
}
#endif

/*
void CompressorHeatingEngine(TypTimer * Timer, int index, int InvAddr)
{

    if(EngineBox[0].HeatingEngineStart)
    {
        //-------------------------------------------------------------
        // Preriscaldamento motore allo start sistema
        if(!Timer->Enable)
        {
            Set_Braking_Current(&Comp_Inverter[index], InvAddr, K_HeatingEngineStart);  // Setto il nuovo valore di corrente di frenatura...(Valore di Default = 50%)
            
			DC_Braking(&EngineBox[index], &Comp_Inverter[index], InvAddr, 1);                              // Abilito DC Braking        

            //Set_Braking_Current(&Comp_Inverter[1], Add_Comp_Inverter2, K_HeatingEngineStart);  // Setto il nuovo valore di corrente di frenatura...(Valore di Default = 50%)
            //DC_Braking(&Comp_Inverter[1], Add_Comp_Inverter2, 1);                              // Abilito DC Braking        
        }
        Timer->Enable = 1;              // Attivo il timer per il riscaldamento motore.     

        if(Timer->TimeOut == 1)         // if timeout
        {    
            Timer->TimeOut = 0;
            EngineBox[0].HeatingEngineStart = 0;    // termino la fase di preriscaldamento
            DC_Braking(&EngineBox[0], &Comp_Inverter[0], InvAddr, 0);                              // Disabilito DC Braking    
			DC_Braking(&EngineBox[1], &Comp_Inverter[1], InvAddr, 0);                              // Disabilito DC Braking    
            Set_Braking_Current(&Comp_Inverter[0], InvAddr, K_HeatingEngineWork);   // Setto il nuovo valore di corrente di frenatura...(Valore di Default = 50%)            
			Set_Braking_Current(&Comp_Inverter[1], InvAddr, K_HeatingEngineWork);   // Setto il nuovo valore di corrente di frenatura...(Valore di Default = 50%)            

            //DC_Braking(&Comp_Inverter[1], Add_Comp_Inverter2, 0);                              // Disabilito DC Braking    
            //Set_Braking_Current(&Comp_Inverter[1], Add_Comp_Inverter2, K_HeatingEngineWork);   // Setto il nuovo valore di corrente di frenatura...(Valore di Default = 50%)            
        }
        //-------------------------------------------------------------
    
    }
    else
    {
        Timer->Enable = 0;              // Disattivo il timer per il riscaldamento motore.             
        Timer->TimeOut = 0;             // resetto il timer
        Timer->Value = 0;
        Timer->Time = K_TimeHeatingEngine;         // 600Sec. = 10min.    
        
#if (K_AbilCompressor2==1)
		if(EngineBox[0].Out_Inverter>0 || EngineBox[1].Out_Inverter>0) 
#else
		if(EngineBox[0].Out_Inverter>0)
#endif
        {
            EngineBox[0].CompressorIsMoreHot = 1;   // Alzo flag per segnalare che il compressore è partito e quindi è caldo
            TimerPostHeatingEngine.Value = 0;
        }
        
#if (K_AbilCompressor2==1)
		TimerPostHeatingEngine.Enable = ((EngineBox[0].Out_Inverter==0) && (EngineBox[1].Out_Inverter==0) && (EngineBox[0].CompressorIsMoreHot==1) && (EngineBox[0].SplitPowerOnCnt==0)) || (EngineBox[0].PersErr.GlobalStatusFlag && EngineBox[1].PersErr.GlobalStatusFlag);
#else
        TimerPostHeatingEngine.Enable = ((EngineBox[0].Out_Inverter==0) && (EngineBox[0].CompressorIsMoreHot==1) && (EngineBox[0].SplitPowerOnCnt==0)) || EngineBox[0].PersErr.GlobalStatusFlag;
#endif

        if(TimerPostHeatingEngine.TimeOut)
        {
            TimerPostHeatingEngine.TimeOut = 0;
            EngineBox[0].CompressorIsMoreHot = 0;
            TimerPostHeatingEngine.Value = 0;
        }
        
    }
    
    
}
 */

/*
   ACCENSIONE
   Valutare indice di carico sistema (numero FC accesi e DeltaT)
   Se carico superiore al 50% accendere entrambi contemporaneamente
   Carico inferiore al 50% accenderne solo 1 e farlo lavorare fino al 50% capacità massima, 
   se al 50% pressione lavoro non è soddisfatta attendere 2 minuti e se non è soddisfatta ancora 
   accendere anche il 2°. Tenere i motori agganciati allo stesso regime fino a quando il regime si 
   assesta dal 25 al 30% per 3 minuti, poi spegnerne uno. 
   Nel caso subentra il ciclo di spegnimento macchina questo ha la precedenza e si possono spegnere insieme. 
   Se sono agganciati possono arrivare a lavorare fino al 100%
   Valore di carico al 50% deve poter essere settabile

   MANTENIMENTO
   In richiesta a salire oltre il 50% per 2 minuti accendere il secondo e quando è arrivato anche lui al 50% alzarli insieme. 
   Nel caso mentre il secondo accelera la pressione va soddisfatta si ferma l?accelerazione del secondo e cala il primo. 
   Studiare come fare punto di aggancio tra i due.
   In discesa quando i due arrivano al 30% per più di 3 minuti ne spegni uno
 */
void SelectWorkCompressor(void)
{
	// n.1 Timer x gestire quale compressore è quello di lavoro (se la potenza richiesta è inferiore al 1/2 max di 1 compressore)
	// 
	//static bit Compressor2Master = 0;
	static unsigned WorkAllCompressor = 0;
    
	if(EngineBox[0].Compressor2Master>1) //Se legge dalla E2 un valore fuori range
		EngineBox[0].Compressor2Master=0;//Inizializzo a zero    
	
#if (K_AbilCompressor2==1)
#if(K_OnlyOneInverterComp2==0)
	TimerSwitchCompressor.Enable =	((EngineBox[0].Out_Inverter > 0) || (EngineBox[1].Out_Inverter > 0));
	
	if(TimerSwitchCompressor.TimeOut || ((TimerSwitchCompressor.Value>(K_TimeSwitchCompressor-240)) && (EngineBox[0].Out_Inverter == 0) && (EngineBox[1].Out_Inverter == 0)))
	{
		TimerSwitchCompressor.TimeOut = 0;
		TimerSwitchCompressor.Value = 0;
		if(EngineBox[0].Compressor2Master==0)
			EngineBox[0].Compressor2Master=1;
		else
			EngineBox[0].Compressor2Master=0;
	} 
	
	TimerIntegrateCompressor2.Enable = ((EngineBox[0].Out_Inverter > K_Perc_Ok_Compressor2) || (EngineBox[1].Out_Inverter > K_Perc_Ok_Compressor2))&&!Comp_Inverter[0].EcoModeEnable;
	
	if(!TimerIntegrateCompressor2.Enable)
	{
		TimerIntegrateCompressor2.Value = 0;
		TimerIntegrateCompressor2.TimeOut = 0;
	}		
	
	if(TimerIntegrateCompressor2.TimeOut)
	{
		TimerIntegrateCompressor2.Value = 0;
		TimerIntegrateCompressor2.TimeOut = 0;
		WorkAllCompressor = 1;
	}
	
	TimerDeintegrateCompressor2.Enable = (EngineBox[0].Out_Inverter <= K_Perc_No_Compressor2) && (EngineBox[1].Out_Inverter <= K_Perc_No_Compressor2);
	
	if(!TimerDeintegrateCompressor2.Enable)
	{
		TimerDeintegrateCompressor2.Value = 0;
		TimerDeintegrateCompressor2.TimeOut = 0;
	}

	if(TimerDeintegrateCompressor2.TimeOut)
	{
		TimerDeintegrateCompressor2.TimeOut = 0;
		WorkAllCompressor = 0;
	}
    	
	if((EngineBox[0].StatoCompressore == CHECK_COMP_TIMEOFF) && (EngineBox[1].StatoCompressore == CHECK_COMP_TIMEOFF))
	{
		TimerDeintegrateCompressor2.Value = 0;
		TimerDeintegrateCompressor2.TimeOut = 0;
		WorkAllCompressor = 0;
	}	
	
#if (k_ForceWorkAllCompressorHot==1)
    if(EngineBox[0].DefMode==CoreRiscaldamento)
        WorkAllCompressor = 1;
#endif
    
    if(Comp_Inverter[0].EcoModeEnable && WorkAllCompressor==1)
        WorkAllCompressor = 0;
    
	EngineBox[0].SelectCompressor = (EngineBox[0].Compressor2Master==0) || WorkAllCompressor || EngineBox[1].PersErr.GlobalStatusFlag;			// Abilito il 2 comp		
	EngineBox[1].SelectCompressor = (EngineBox[0].Compressor2Master==1) || WorkAllCompressor || EngineBox[0].PersErr.GlobalStatusFlag;			// Abilito il 2 comp		
#else
    static int OldChangeModeWork = PumpSelection_Auto;  //ATTENZIONE: USO I PARAMETRI DOPPIA POMPA PERCHè SUL MASTER SONO GIA PRESENTI.
    static int CompressorLastOn = 0;
    
    if(EngineBox[0].SwitchPumpSelection != OldChangeModeWork)
    {
        if(EngineBox[0].SwitchPumpSelection == PumpSelection_1)
            if(EngineBox[1].EngineIsOn==1)
                EngineBox[1].GoShutDwComp=1;

        if(EngineBox[0].SwitchPumpSelection == PumpSelection_2)
            if(EngineBox[0].EngineIsOn==1)
                EngineBox[0].GoShutDwComp=1;
    }
    OldChangeModeWork = EngineBox[0].SwitchPumpSelection;
    
    TimerSwitchCompressor.Time = EngineBox[0].SwitchPumpTime*60*60;//Moltiplico *60 x 2 volte per portare in ore (uso il tempo della pompa per risarmiare spazio sul touch)
	TimerSwitchCompressor.Enable =	((EngineBox[0].Out_Inverter > 0) || (EngineBox[1].Out_Inverter > 0));
	
	if(TimerSwitchCompressor.TimeOut || ((TimerSwitchCompressor.Value>(K_TimeSwitchCompressor-240)) && (EngineBox[0].Out_Inverter == 0) && (EngineBox[1].Out_Inverter == 0)))
	{
		TimerSwitchCompressor.TimeOut = 0;
		TimerSwitchCompressor.Value = 0;
		if(EngineBox[0].Compressor2Master==0)
        {
            if(EngineBox[0].EngineIsOn==1 && !EngineBox[1].PersErr.GlobalStatusFlag)
                EngineBox[0].GoShutDwComp=1;
			EngineBox[0].Compressor2Master=1;
        }
		else
        {
            if(EngineBox[1].EngineIsOn==1 && !EngineBox[0].PersErr.GlobalStatusFlag)
                EngineBox[1].GoShutDwComp=1;
			EngineBox[0].Compressor2Master=0;
        }
	} 

    TimerDeintegrateCompressor2.Time = 10;
	TimerDeintegrateCompressor2.Enable = EngineBox[0].Out_Inverter == 0 && EngineBox[1].Out_Inverter == 0 && CompressorLastOn == 1;
	
	if(!TimerDeintegrateCompressor2.Enable)
	{
		TimerDeintegrateCompressor2.Value = 0;
		TimerDeintegrateCompressor2.TimeOut = 0;
	}

	if(TimerDeintegrateCompressor2.TimeOut)
	{
		TimerDeintegrateCompressor2.TimeOut = 0;
		CompressorLastOn = 0;
	}

	if(CompressorLastOn == 0)    
    {
        EngineBox[1].SelectCompressor = (((EngineBox[0].Compressor2Master==1) || EngineBox[0].PersErr.GlobalStatusFlag) && !EngineBox[1].PersErr.GlobalStatusFlag && (EngineBox[0].SwitchPumpSelection==PumpSelection_Auto)) || (EngineBox[0].SwitchPumpSelection==PumpSelection_2);			// Abilito il 2 comp    
        EngineBox[0].SelectCompressor = (!EngineBox[1].SelectCompressor && (EngineBox[0].SwitchPumpSelection==PumpSelection_Auto)) || (EngineBox[0].SwitchPumpSelection==PumpSelection_1);
    }
    
	if(EngineBox[0].Out_Inverter != 0 || EngineBox[1].Out_Inverter != 0)    
        CompressorLastOn = 1;
#endif
#else
		EngineBox[0].SelectCompressor = 1;
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
		return 7000;			// sotto i 0, setto 7,5
	else if (ReqTemp <= 300)
		return 7500;			// sotto i 0, setto 7,5
	else if (ReqTemp <= 500)
		return 8500;			// sotto i 5, setto 8,5
	else if (ReqTemp <= 750)
		return 9000;			// sotto i 7,5, setto 9,5
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
	//unsigned int RetVal;
	
	TempConv = ReqTemp/100;				// Converto Temperatura da centesimi in gradi
 

	// ritorno valore pressione corrispondente come da tabella GAS R410A

    if(TempConv <= -32) return 1480;
    if(TempConv == -31) return 1580;
    if(TempConv == -30) return 1690;
    if(TempConv == -29) return 1800;    
    if(TempConv == -28) return 1920;
    if(TempConv == -27) return 2040;
    if(TempConv == -26) return 2160;
	if(TempConv == -25) return 2500; 
	if(TempConv == -24) return 2580; 
	if(TempConv == -23) return 2660; 
	if(TempConv == -22) return 2740; 
	if(TempConv == -21) return 2820; 
	if(TempConv == -20) return 3000; 
	if(TempConv == -19) return 3200; 
	if(TempConv == -18) return 3400; 
	if(TempConv == -17) return 3600; 
	if(TempConv == -16) return 3800; 
	if(TempConv == -15) return 4000; 
	if(TempConv == -14) return 4200; 
	if(TempConv == -13) return 4400; 
	if(TempConv == -12) return 4600; 
	if(TempConv == -11) return 4800; 
	if(TempConv == -10) return 5000; 
	if(TempConv == -9)	return 5200; 
	if(TempConv == -8)	return 5400; 
	if(TempConv == -7)	return 5600; 
	if(TempConv == -6)	return 5800; 
	if(TempConv == -5)	return 6000; 
	if(TempConv == -4)	return 6200; 
	if(TempConv == -3)	return 6400; 
	if(TempConv == -2)	return 6600; 
	if(TempConv == -1)	return 6800; 
	if(TempConv ==  0)	return 7000; 
	if(TempConv ==  1)	return 7300; 
	if(TempConv ==  2)	return 7600; 
	if(TempConv ==  3)	return 7900; 
	if(TempConv ==  4)	return 8200; 
	if(TempConv ==  5)	return 8500; 
	if(TempConv ==  6)	return 8800; 
	if(TempConv ==  7)	return 9100; 
	if(TempConv ==  8)	return 9400; 
	if(TempConv ==  9)	return 9700; 
	if(TempConv == 10)	return 10000; 
	if(TempConv == 11)	return 10350; 
	if(TempConv == 12)	return 10700; 
	if(TempConv == 13)	return 10950; 
	if(TempConv == 14)	return 11300; 
	if(TempConv == 15)	return 11650; 
	if(TempConv == 16)	return 12000; 
	if(TempConv == 17)	return 12350; 
	if(TempConv == 18)	return 12700; 
	if(TempConv == 19)	return 13050; 
	if(TempConv == 20)	return 13500; 
	if(TempConv == 21)	return 13950; 
	if(TempConv == 22)	return 14400; 
	if(TempConv == 23)	return 14850; 
	if(TempConv == 24)	return 15300; 
	if(TempConv == 25)	return 15750; 
	if(TempConv == 26)	return 16200; 
	if(TempConv == 27)	return 16650; 
	if(TempConv == 28)	return 17100; 
	if(TempConv == 29)	return 17550; 
	if(TempConv == 30)	return 18000; 
	if(TempConv == 31)	return 18500; 
	if(TempConv == 32)	return 19000; 
	if(TempConv == 33)	return 19500;
	if(TempConv == 34)	return 20000;
	if(TempConv == 35)	return 20500;
	if(TempConv == 36)	return 21000;
	if(TempConv == 37)	return 21500;
	if(TempConv == 38)	return 22000;
	if(TempConv == 39)	return 22500;
	if(TempConv == 40)	return 23000;
	if(TempConv == 41)	return 23700;
	if(TempConv == 42)	return 24400;
	if(TempConv == 43)	return 25100;
	if(TempConv == 44)	return 25800;
	if(TempConv == 45)	return 26500;
	if(TempConv == 46)	return 27200;
	if(TempConv == 47)	return 27900;
	if(TempConv == 48)	return 28600;
	if(TempConv == 49)	return 29300;
	if(TempConv == 50)	return 30000;
	if(TempConv == 51)	return 30830;
	if(TempConv == 52)	return 31660;
	if(TempConv == 53)	return 32490;
	if(TempConv == 54)	return 33320;
	if(TempConv == 55)	return 34150;
	if(TempConv == 56)	return 34980;
	if(TempConv == 57)	return 35810;
	if(TempConv == 58)	return 36640;
	if(TempConv == 59)	return 37470;
	if(TempConv == 60)	return 38300;
	if(TempConv == 61)	return 39130;
	if(TempConv == 62)	return 40000;
	if(TempConv == 63)	return 40870;
	if(TempConv == 64)	return 41740;
	if(TempConv == 65)	return 42610;
	if(TempConv == 66)	return 43480;
	if(TempConv == 67)	return 44350;
	if(TempConv == 68)	return 45220;
	if(TempConv == 69)	return 46090;
	if(TempConv == 70)	return 47000;
	
	return 0;
	
}
/*  *****************************************************************************************************************************
	Gestico la pompa del condensatore
	***************************************************************************************************************************** */
void PumpManagement(void)		
{
	int Error = 0;
	int Work = 0;
	int TestFlux = 0;
	int TestEEV = 0;
	int Lock = 0;
	int ErroreP = 0;
    int TestPump = 0;
    int PumpMaxPower=0;
    static int Freq_Req_PompaAcqua;
    static int PumpAbsorption=0;

    
#if(K_AbilCompressor2==1)	
	double ReqFreqMotors = 0;
#else
	unsigned int ReqFreqMotors = 0;
#endif	
	static int ReinitPump=0;
	static int Lim_Min_Pump=0;
	//static bit PumpWorkOn = 0;
    
    
	if(EngineBox[0].SelectWorkingPump>1) //Se legge dalla E2 un valore fuori range
		EngineBox[0].SelectWorkingPump=0;//Inizializzo a zero  
    
    
#if(K_Abil_Double_Pump==1)
    TimerSwitchPump.Time = EngineBox[0].SwitchPumpTime*60*60;//Moltiplico *60 x 2 volte per portare in ore
	TimerSwitchPump.Enable = (EngineBox[0].Out.Pompa_Acqua || EngineBox[0].Out.Pompa_Acqua2) && (EngineBox[0].SwitchPumpSelection==PumpSelection_Auto) && !EngineBox[0].Test.FluxReq;
	
	if(TimerSwitchPump.TimeOut)
	{
		TimerSwitchPump.TimeOut = 0;
		TimerSwitchPump.Value = 0;
		if(EngineBox[0].SelectWorkingPump==0)
			EngineBox[0].SelectWorkingPump=1;
		else
			EngineBox[0].SelectWorkingPump=0;                 
	}

	if((EngineBox[0].PersErr.Pump1Fault && (EngineBox[0].SwitchPumpSelection==PumpSelection_Auto)) || (EngineBox[0].SwitchPumpSelection==PumpSelection_2))
		EngineBox[0].SelectWorkingPump=1;
	else if((EngineBox[0].PersErr.Pump2Fault && (EngineBox[0].SwitchPumpSelection==PumpSelection_Auto)) || (EngineBox[0].SwitchPumpSelection==PumpSelection_1))
		EngineBox[0].SelectWorkingPump=0;
#endif
#if(K_External_SeaW_Supply==1)    
	Lock = EngineBox[0].Test.FluxReq || EngineBox[0].Test.EEV_Req || EngineBox[0].Test.EEV_Go || EngineBox[0].PersErr.FloodSensor || EngineBox[0].PersErr.PumpInverterFault;
#else
	Lock = EngineBox[0].Test.FluxReq || EngineBox[0].Test.EEV_Req || EngineBox[0].Test.EEV_Go || EngineBox[0].PersErr.FloodSensor || EngineBox[0].PersErr.PumpInverterFault || EngineBox[0].PersErr.WaterSeaFlux;
#endif
	TestFlux = EngineBox[0].Test.FluxReq && EngineBox[0].PumpON_FluxTestFlag;
	TestEEV = (EngineBox[0].Test.EEV_Req || EngineBox[0].Test.EEV_Go) && EngineBox[0].PumpON_EEVTestFlag;
#if(K_External_SeaW_Supply==1)    
	Error = (EngineBox[0].PersErr.SeaWaterLo || EngineBox[0].PersErr.SeaWaterHi || EngineBox[0].Error.CondensatoreHi || EngineBox[0].Error.CondensatoreLo || EngineBox[0].PersErr.WaterSeaFlux) && !Lock;
#else
	Error = (EngineBox[0].PersErr.SeaWaterLo || EngineBox[0].PersErr.SeaWaterHi || EngineBox[0].Error.CondensatoreHi || EngineBox[0].Error.CondensatoreLo) && !Lock;    
#endif
    TestPump =  ((EngineBox[0].Manual_Pump_Abil==1) && (Room[1].SystemDisable==1)) && !Lock;
    
#if (K_AbilCompressor2==1)
	Work = (EngineBox[0].InverterRaffrescamento || EngineBox[0].InverterRiscaldamento ||
		EngineBox[1].InverterRaffrescamento || EngineBox[1].InverterRiscaldamento ||
		((EngineBox[0].GoRiscaldamento || EngineBox[0].GoRaffrescamento) && (EngineBox[0].StatoCompressore >= WAIT_CHECK_FLUX)) || 
		((EngineBox[1].GoRiscaldamento || EngineBox[1].GoRaffrescamento) && (EngineBox[1].StatoCompressore >= WAIT_CHECK_FLUX)) || 
		EngineBox[0].GoShutDwComp || EngineBox[1].GoShutDwComp) && !Lock;
#else
	Work = (EngineBox[0].InverterRaffrescamento || EngineBox[0].InverterRiscaldamento || 
		((EngineBox[0].GoRiscaldamento || EngineBox[0].GoRaffrescamento) && (EngineBox[0].StatoCompressore >= WAIT_CHECK_FLUX)) ||
		EngineBox[0].GoShutDwComp) && !Lock;
#endif
	
#if(K_Abil_Double_Pump==1)    
    EngineBox[0].Out.Pompa_Acqua = (Work || Error || TestFlux || TestEEV || TestPump)&&!EngineBox[0].SelectWorkingPump;
	EngineBox[0].Out.Pompa_Acqua2 = (Work || Error || TestFlux || TestEEV || TestPump)&&EngineBox[0].SelectWorkingPump;
	//EngineBox[0].VirtualOut.Pompa_Acqua = (Work || Error || TestFlux || TestEEV)&&!EngineBox[0].SelectWorkingPump;
	//EngineBox[0].VirtualOut.Pompa_Acqua2 = (Work || Error || TestFlux || TestEEV)&&EngineBox[0].SelectWorkingPump;
#else
    EngineBox[0].Out.Pompa_Acqua = Work || Error || TestFlux || TestEEV || TestPump;
#endif
	
	//CheckSwitchPump();
	
#if(K_AbilPumpInverter==1 || K_AbilPumpDAC==1)	
#if(K_AbilPumpInverter==1)
    PumpMaxPower = Comp_Inverter[2].Power_Inverter/10;
    PumpAbsorption = Filter(&FilterPumpAbsorption, Comp_Inverter[2].Out_Power, K_Campioni_FilterPumpAbsorption);
    
    Comp_Inverter[2].Filtered_Out_Current = Filter(&FilterInverterPumpOut_Current, Comp_Inverter[2].Out_Current, K_Campioni_FilterInverterOut_Current);
#else
    PumpMaxPower = 0;
    PumpAbsorption = 0;
    
    Comp_Inverter[2].Filtered_Out_Current = 0;
#endif
    
	if(TestPump)
    {
        Freq_Req_PompaAcqua =  EngineBox[0].Manual_Pump_Freq;
        EngineBox[0].FreqPompa_Acqua = EngineBox[0].Manual_Pump_Freq;
		ReinitPump = 1;
    }
    else if(Error && !Work)
	{
		Freq_Req_PompaAcqua = K_LIM_ERR_INV_PMP;
		ReinitPump = 1;

        if(EngineBox[0].FreqPompa_Acqua<K_LIM_MIN_FREQ_INV_PUMP)
        {
            EngineBox[0].FreqPompa_Acqua = K_LIM_MIN_FREQ_INV_PUMP;
        }			
	}
	else if(EngineBox[0].Test.FluxReq)
	{
		if(TestFlux)
        {
            Freq_Req_PompaAcqua = K_LIM_ERR_INV_PMP;
			EngineBox[0].FreqPompa_Acqua = K_LIM_ERR_INV_PMP;
        }
		else
        {
            Freq_Req_PompaAcqua = 0;
			EngineBox[0].FreqPompa_Acqua = 0;
        }
        
        ReinitPump = 1;
	}
	#if(K_AbilCompressor2==1)
	else if((EngineBox[0].HeatingMode || EngineBox[1].HeatingMode) && Work)
	#else		
    else if((EngineBox[0].HeatingMode) && Work)
	#endif
	{
        if(ReinitPump == 1)
        {
            Freq_Req_PompaAcqua = K_LIM_MIN_FREQ_INV_PUMP;
            EngineBox[0].FreqPompa_Acqua = K_LIM_MIN_FREQ_INV_PUMP;
            ReinitPump = 0;
        }		
        #if(K_Force_Freq_Inv_Pump_Hot==1)
        Freq_Req_PompaAcqua = K_LIM_FORCED_FREQ_INV_PUMP_HOT;
        #else
        Freq_Req_PompaAcqua = FreqInv2FreqPump((long)(Comp_Inverter[0].Req_Freq) + (long)(Comp_Inverter[1].Req_Freq), K_LIM_MIN_FREQ_INV_COMP2PUMP, K_LIM_MAX_FREQ_INV_COMP2PUMP, K_LIM_MIN_FREQ_INV_PUMP, K_LIM_MAX_FREQ_INV_PUMP);
        if(EngineBox[0].Temperature.Sea_Water<K_SeaWTemp_Correction_Pump)
            Freq_Req_PompaAcqua = Freq_Req_PompaAcqua + (int)((float)Freq_Req_PompaAcqua * 30.0 / 100.0);

        if(Freq_Req_PompaAcqua<K_LIM_MIN_FREQ_INV_PUMP)
            Freq_Req_PompaAcqua=K_LIM_MIN_FREQ_INV_PUMP;
        if(Freq_Req_PompaAcqua>K_LIM_MAX_FREQ_INV_PUMP)
            Freq_Req_PompaAcqua=K_LIM_MAX_FREQ_INV_PUMP;      
        #endif    
    }	
	
	#if(K_AbilCompressor2==1)
	else if((EngineBox[0].CoolingMode || EngineBox[1].CoolingMode) && Work)
	#else		
	else if((EngineBox[0].CoolingMode) && Work)
	#endif
	{
#if(K_Force_Freq_Inv_Pump_Cold==1)
        if(ReinitPump == 1)
        {
            Freq_Req_PompaAcqua = K_LIM_MIN_FREQ_INV_PUMP;
            EngineBox[0].FreqPompa_Acqua = K_LIM_MIN_FREQ_INV_PUMP;
            ReinitPump = 0;
        }		
        Freq_Req_PompaAcqua = K_LIM_FORCED_FREQ_INV_PUMP_COLD;
#else	
	#if(K_AbilCompressor2==1)
        ReqFreqMotors = (double)Comp_Inverter[0].Req_Freq + (double)Comp_Inverter[1].Req_Freq;
        if(ReqFreqMotors<=((double)kPump_Freq_Threshold_1*(Comp_Inverter[0].Req_Freq!=0)+((double)kPump_Freq_Threshold_1*(Comp_Inverter[1].Req_Freq!=0))))
            Lim_Min_Pump = K_LIM_MIN_1_FREQ_INV_PUMP;
        else if(ReqFreqMotors<=((double)kPump_Freq_Threshold_2*(Comp_Inverter[0].Req_Freq!=0)+((double)kPump_Freq_Threshold_2*(Comp_Inverter[1].Req_Freq!=0))))
            Lim_Min_Pump = K_LIM_MIN_2_FREQ_INV_PUMP;
        else if(ReqFreqMotors<=((double)kPump_Freq_Threshold_3*(Comp_Inverter[0].Req_Freq!=0)+((double)kPump_Freq_Threshold_3*(Comp_Inverter[1].Req_Freq!=0))))
            Lim_Min_Pump = K_LIM_MIN_3_FREQ_INV_PUMP;
        else if(ReqFreqMotors<=((double)kPump_Freq_Threshold_4*(Comp_Inverter[0].Req_Freq!=0)+((double)kPump_Freq_Threshold_4*(Comp_Inverter[1].Req_Freq!=0))))
            Lim_Min_Pump = K_LIM_MIN_4_FREQ_INV_PUMP;
        else if(ReqFreqMotors<=((double)kPump_Freq_Threshold_5*(Comp_Inverter[0].Req_Freq!=0)+((double)kPump_Freq_Threshold_5*(Comp_Inverter[1].Req_Freq!=0))))
            Lim_Min_Pump = K_LIM_MIN_5_FREQ_INV_PUMP;
        else
            Lim_Min_Pump = K_LIM_MIN_5_FREQ_INV_PUMP;
    #else		
        ReqFreqMotors = Comp_Inverter[0].Req_Freq;
        if(ReqFreqMotors<=kPump_Freq_Threshold_1)
            Lim_Min_Pump = K_LIM_MIN_1_FREQ_INV_PUMP;
        else if(ReqFreqMotors<=kPump_Freq_Threshold_2)
            Lim_Min_Pump = K_LIM_MIN_2_FREQ_INV_PUMP;
        else if(ReqFreqMotors<=kPump_Freq_Threshold_3)
            Lim_Min_Pump = K_LIM_MIN_3_FREQ_INV_PUMP;
        else if(ReqFreqMotors<=kPump_Freq_Threshold_4)
            Lim_Min_Pump = K_LIM_MIN_4_FREQ_INV_PUMP;
        else if(ReqFreqMotors<=kPump_Freq_Threshold_5)
            Lim_Min_Pump = K_LIM_MIN_5_FREQ_INV_PUMP;
        else
            Lim_Min_Pump = K_LIM_MIN_5_FREQ_INV_PUMP;
#endif	
			
        if(Lim_Min_Pump>K_LIM_MAX_FREQ_INV_PUMP)
            Lim_Min_Pump=K_LIM_MAX_FREQ_INV_PUMP;

        if(ReinitPump == 1)
        {
            Freq_Req_PompaAcqua = Lim_Min_Pump;
            EngineBox[0].FreqPompa_Acqua = Lim_Min_Pump;
            ReinitPump = 0;
        }			
              
#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)   
        ErroreP = EngineBox[0].Pressure.LiquidCond - K_Lim_Press_Liq_Pump;
        if(ErroreP > K_Ist_Work_Pump)
        {	
            Freq_Req_PompaAcqua= K_LIM_MAX_FREQ_INV_PUMP;
        }	
        else
        {	
            ErroreP = K_Lim_Press_Liq_Pump - EngineBox[0].Pressure.LiquidCond;
            if(ErroreP > K_Ist_Work_Pump)
            {	
                Freq_Req_PompaAcqua = Lim_Min_Pump;
            }
        }	
#else
        ErroreP = EngineBox[0].Pressure.Liquid - K_Lim_Press_Liq_Pump;
        if(ErroreP > K_Ist_Work_Pump)
        {	
            Freq_Req_PompaAcqua= K_LIM_MAX_FREQ_INV_PUMP;
        }	
        else
        {	
            ErroreP = K_Lim_Press_Liq_Pump - EngineBox[0].Pressure.Liquid;
            if(ErroreP > K_Ist_Work_Pump)
            {	
                Freq_Req_PompaAcqua = Lim_Min_Pump;
            }
        }	
#endif
#endif
    }
#if(K_AbilCompressor2==1)
		else if((EngineBox[0].GoShutDwComp==1 || EngineBox[1].GoShutDwComp==1) && Work)
#else		
		else if((EngineBox[0].GoShutDwComp==1) && Work)
#endif
			Freq_Req_PompaAcqua = K_LIM_SHUTDWN_INV_PMP;


#if(K_AbilCompressor2==1)
		else if(EngineBox[0].LastModeCompressor==0 && EngineBox[1].LastModeCompressor==0)
#else		
		else if(EngineBox[0].LastModeCompressor==0)
#endif
		{	
            Freq_Req_PompaAcqua = 0;
			EngineBox[0].FreqPompa_Acqua = 0;
			ReinitPump = 1;
		}
    
        if(Comp_Inverter[0].EcoModeEnable && Freq_Req_PompaAcqua > K_LIM_FREQ_INV_PUMP_ECO_MODE)
            Freq_Req_PompaAcqua = K_LIM_FREQ_INV_PUMP_ECO_MODE;
                
		if(EngineBox[0].PersErr.WaterSeaFlux==1 || EngineBox[0].PersErr.FloodSensor==1 || EngineBox[0].PersErr.PumpInverterFault==1)
		{
            Freq_Req_PompaAcqua = 0;
			EngineBox[0].FreqPompa_Acqua = 0;
			ReinitPump = 1;
		}
        if(TimerWorkPump.TimeOut)
        {
            TimerWorkPump.Value = 0;
            TimerWorkPump.TimeOut = 0;

            if((PumpAbsorption>PumpMaxPower) || (Comp_Inverter[2].Filtered_Out_Current > K_Inv_PercLimit_Out_Current))
                Decrement(&EngineBox[0].FreqPompa_Acqua, K_IncHzWorkPump, K_LIM_MIN_FREQ_INV_PUMP); 
            else if((EngineBox[0].FreqPompa_Acqua < Freq_Req_PompaAcqua) && (PumpAbsorption<(PumpMaxPower-2)) && (Comp_Inverter[2].Filtered_Out_Current < (K_Inv_PercLimit_Out_Current-150)))
                Increment(&EngineBox[0].FreqPompa_Acqua, K_IncHzWorkPump, Freq_Req_PompaAcqua);
            else if(EngineBox[0].FreqPompa_Acqua > Freq_Req_PompaAcqua)
                Decrement(&EngineBox[0].FreqPompa_Acqua, K_IncHzWorkPump, Freq_Req_PompaAcqua); 
        }
#else
	EngineBox[0].FreqPompa_Acqua = 4500*(EngineBox[0].Out.Pompa_Acqua || EngineBox[0].Out.Pompa_Acqua2);	//passato a valore max su touch. da convertire in percentuale su touch per non mettere valore fisso.
#endif
}


void WarmColdValveManagement(void)
{
/*
#if (K_AbilCompressor2==1)
	EngineBox[0].Out.Valvola_Caldo = (EngineBox[0].LastModeCompressor==EngineBox_Caldo) || (EngineBox[1].LastModeCompressor==EngineBox_Caldo);
#else
	EngineBox[0].Out.Valvola_Caldo = EngineBox[0].LastModeCompressor==EngineBox_Caldo;
#endif
*/
#if (K_AbilCompressor2==1)
    if((EngineBox[0].LastModeCompressor==EngineBox_Off) && (EngineBox[1].LastModeCompressor==EngineBox_Off))    
        EngineBox[0].Out.Valvola_Caldo = EngineBox[0].DefMode == EngineBox_Caldo;
#else
    if(EngineBox[0].LastModeCompressor==EngineBox_Off)    
    	EngineBox[0].Out.Valvola_Caldo = EngineBox[0].DefMode == EngineBox_Caldo;
#endif
    
}

void AbsorptionManagement(void)
{
    int i;
    
    //Sommo gli assorbimenti delle ventole degli slave
    EngineBox[0].Split_Total_Fan_Absorption = 0;
    EngineBox[0].Split_Total_AirPowerOutput = 0;
    EngineBox[0].Split_Total_AirPowerBTU = 0;
  
    for(i=1; i<Max_Room; i++)
    {
        if(Room[i].OnLine == 1)		// se abilitato e OnLine
        {
            EngineBox[0].Split_Total_Fan_Absorption = EngineBox[0].Split_Total_Fan_Absorption + (unsigned int)(round((float)Room[i].Total_Fan_Power/10.0));
            EngineBox[0].Split_Total_AirPowerOutput = EngineBox[0].Split_Total_AirPowerOutput + (unsigned int)(round((float)Room[i].TotalAirPowerOutput));
            EngineBox[0].Split_Total_AirPowerBTU = EngineBox[0].Split_Total_AirPowerBTU + (unsigned int)(round((float)Room[i].TotalAirPowerBTU));
        }

    }

#if(K_AbilPumpInverter==1)	
	EngineBox[0].TotalPowerBoxAbsorption = Comp_Inverter[0].Out_Power + Comp_Inverter[1].Out_Power + Comp_Inverter[2].Out_Power + EngineBox[0].Split_Total_Fan_Absorption;
#else
	EngineBox[0].TotalPowerBoxAbsorption = Comp_Inverter[0].Out_Power + Comp_Inverter[1].Out_Power + (K_Absorption_Pump_OnOff*(EngineBox[0].Out.Pompa_Acqua==1)) + EngineBox[0].Split_Total_Fan_Absorption;
#endif			
    
    EngineBox[0].BoxAbsorption = Filter(&FilterPowerBox1Absorption, Comp_Inverter[0].Out_Power, K_Campioni_FilterPowerBoxAbsorption);
    EngineBox[1].BoxAbsorption = Filter(&FilterPowerBox2Absorption, Comp_Inverter[1].Out_Power, K_Campioni_FilterPowerBoxAbsorption);
    EngineBox[0].TotalPowerBoxAbsorption = Filter(&FilterTotalPowerBoxAbsorption, EngineBox[0].TotalPowerBoxAbsorption, K_Campioni_FilterPowerBoxAbsorption);

// NOTA: La seguente divisione con castizzazione in Float è stata fatta per evitare un "RESET" della scheda in caso di dividendo == 0 !!!!!!!!!!!!!!!!!!!!!!!
// Sembra che se la divisione viene eseguita tra unsigned int (o int) la divisione provochi un reset della scheda! se il dividendo è zero.
    EngineBox[0].System_Efficency_EER = (unsigned int)((float)EngineBox[0].Split_Total_AirPowerOutput / (float)EngineBox[0].TotalPowerBoxAbsorption);
}



/*  *****************************************************************************************************************************
	Gestico l'incremento di una variabile
	***************************************************************************************************************************** */
unsigned char Increment(int * Variabile, int Value, int Max)
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
			return 1;					// e ritorno la modifica del valore
		}
		else
			return 0;					// altrimenti sono esattamente a minimo e non modifico nulla
	}
}

/*  *****************************************************************************************************************************
	Gestico il decremento di una variabile
	***************************************************************************************************************************** */
unsigned char Decrement(int * Variabile, int Value, int Min)
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
			return 1;					// e ritorno la modifica del valore
		}
		else
			return 0;					// altrimenti sono esattamente a minimo e non modifico nulla
	}
}

/*  *****************************************************************************************************************************
	Gestico gli errori
	***************************************************************************************************************************** */
void ErrorCheck(void)
{	
//------------------------------------------------------------
// aggiornamento ingressi
//------------------------------------------------------------
/*
Cablaggio ingressi logica DL (Master):
----------------------------------------
1 = Service Mode		-
2 = Termica compressore 1	(Hi Pressure)
3 = Termica compressore 2	(Lo Pressure)
4 = Mancanza olio comp1		(Termica comp.)
5 = Mancanza olio comp2		(Allagamento)
6 = Sensore di fusso		-
*/	
#if (K_AbilCompressor2==1)		// K_AbilCompressor2 = 1 -> Compressore doppio	
#if(K_Ing2_ModeSelection==0)
	EngineBox[0].In.ThermicComp = !iDigiIn2;                    // sonda termica del compressore
#endif
	EngineBox[1].In.ThermicComp = !iThermicComp2;				// sonda termica del compressore
	EngineBox[0].In.LowOilLevel = !iLowOilLevel1;				// sonda di allagamento 1
	EngineBox[1].In.LowOilLevel = !iLowOilLevel2;				// sonda di allagamento 2
	EngineBox[0].In.Flow = iFlowSwitch;							// ingresso flussostato	
#else	
#if(K_Ing2_ModeSelection==0)
	EngineBox[0].In.CompressorHi = !iDigiIn2;                   // sonda di alta pressione in mandata
#endif
	EngineBox[0].In.CompressorLo = !iLowPressure;				// sonda di bassa pressione in mandata
	EngineBox[0].In.ThermicComp = !iThermicComp1;    			// sonda termica del compressore
#if (K_AbilRechargeOil==1)
	EngineBox[0].In.LowOilLevel = !iLowOilLevel1;
#else	
	EngineBox[0].In.FloodSensor = iFloodSensor;				// sonda di allagamento
#endif
	EngineBox[0].In.Flow = iFlowSwitch;							// ingresso flussostato
#endif
	
    Error_Thermic_Comp();

    Error_Fire_Alarm();

#if (K_AbilCompressor2==0&&K_AbilRechargeOil==0)	
    Error_Flood();
#endif		

    Error_Sea_Water_Lo();

    Error_Sea_Water_Hi();

    Error_Refrigerant_Charge();

    Error_Compressor_Hi();

    Error_Compressor_Lo();

    Error_Condenser_Hi();

    Error_Condenser_Lo();
    
    Error_Gas_Recovery();
    
    Error_Pressure_Lo();

    Error_Pressure_Hi();

    Error_Condenser_Fouled();

#if(K_External_SeaW_Supply)
    Error_Flow_Req();
#else
    Error_Flow_Test();  
#endif
    
    //Error_EEV_Test();
    
#if (K_AbilErrorCompression==1)   
    CompressionError();
#endif      
    
	//------------------------------------------------------------
	// ciclo di recupero dell'olio
	//------------------------------------------------------------	
#if (K_AbilRechargeOil==1)					
        RechargeOilCycle(&EngineBox[0], &TimerWorkRechargeOil1, &TimerPauseRechargeOil1, &TimerAlarmRechargeOil1, &TimerLowCriticalAlarmRechargeOil1);	// ciclo di recupero dell'olio
#if (K_AbilCompressor2==1)
        RechargeOilCycle(&EngineBox[1], &TimerWorkRechargeOil2, &TimerPauseRechargeOil2, &TimerAlarmRechargeOil2, &TimerLowCriticalAlarmRechargeOil2);	// ciclo di recupero dell'olio			
#endif
#endif	

#if (K_AbilDefrostingCycle==1)					
        DefrostingCycle();        
#endif            

    Error_CoolWarm_Valve();
        
    Error_SuperHeat();
    
    Error_TemperatureProbe();
    
    Error_PressureProbe();

    Error_Reset();
    Error_Compact();
}
    
void Error_Thermic_Comp(void)
{
	//------------------------------------------------------------
	// gestione flag di allarme termica compressore 1
	//------------------------------------------------------------
	if(EngineBox[0].In.ThermicComp && !EngineBox[0].PersErr.ThermicComp)								// Gestisco l'allarme della termica compressore
	{ 
		EngineBox[0].PersErr.ThermicComp =1;
		if(EngineBox[0].EngineIsOn)
        {
            EngineBox[0].Out_Inverter = Min_Speed;
			EngineBox[0].GoShutDwComp =1;
        }
	}

#if (K_AbilCompressor2==1)			
	//------------------------------------------------------------
	// gestione flag di allarme termica compressore 2
	//------------------------------------------------------------
	if(EngineBox[1].In.ThermicComp && !EngineBox[1].PersErr.ThermicComp)								// Gestisco l'allarme della termica compressore
	{ 
		EngineBox[1].PersErr.ThermicComp =1;
		if(EngineBox[1].EngineIsOn)
        {
            EngineBox[1].Out_Inverter = Min_Speed;
			EngineBox[1].GoShutDwComp =1;
        }
	}
#endif
}

void Error_Fire_Alarm(void)
{
	//------------------------------------------------------------
	// gestione flag di allarme fuoco
	//------------------------------------------------------------
	if(EngineBox[0].PersErr.FireAlarm)								// Gestisco l'allarme del fuoco
	{ 
		if(EngineBox[0].EngineIsOn)
        {
            if(EngineBox[0].Out_Inverter > Min_Speed)
                EngineBox[0].Out_Inverter = Min_Speed;
			EngineBox[0].GoShutDwComp =1;
        }
#if (K_AbilCompressor2==1)			
		if(EngineBox[1].EngineIsOn)
        {
            if(EngineBox[1].Out_Inverter > Min_Speed)
                EngineBox[1].Out_Inverter = Min_Speed;
			EngineBox[1].GoShutDwComp =1;
        }
#endif
	}	
}

void Error_Flood(void)
{
	//------------------------------------------------------------
	// gestione flag di allarme allagamento
	//------------------------------------------------------------
	if(EngineBox[0].In.FloodSensor && !EngineBox[0].PersErr.FloodSensor)								// Gestisco l'allarme di allagamento del box motore
	{ 
		EngineBox[0].PersErr.FloodSensor =1;
		if(EngineBox[0].EngineIsOn)
			EngineBox[0].GoShutDwComp =1;
	}		
}

void Error_Sea_Water_Lo(void)
{
	//------------------------------------------------------------
	// controllo per la temperatura dell'acqua troppo bassa
	//------------------------------------------------------------
	if(EngineBox[0].PersErr.SeaWaterLo)
	{	
		if(EngineBox[0].Temperature.Sea_Water > RestartLow_Temp_Sea_Water)
			EngineBox[0].PersErr.SeaWaterLo = 0;
	}
	else
	{	
		if(EngineBox[0].Temperature.Sea_Water < Low_Temp_Sea_Water)
		{
			EngineBox[0].PersErr.SeaWaterLo = 1;
			if(EngineBox[0].EngineIsOn)
				EngineBox[0].GoShutDwComp =1;											// spengo
#if (K_AbilCompressor2==1)
			if(EngineBox[1].EngineIsOn)
				EngineBox[1].GoShutDwComp =1;
#endif		
		}
	}
}

void Error_Sea_Water_Hi(void)
{
   	//------------------------------------------------------------
	// controllo per la temperatura dell'acqua troppo alta
	//------------------------------------------------------------
	if(EngineBox[0].PersErr.SeaWaterHi)
	{	
		if(EngineBox[0].Temperature.Sea_Water < Restart_High_Temp_Sea_Water)
			EngineBox[0].PersErr.SeaWaterHi = 0;
	}
	else
	{	
		if(EngineBox[0].Temperature.Sea_Water > High_Temp_Sea_Water)
		{
			EngineBox[0].PersErr.SeaWaterHi = 1;
			if(EngineBox[0].EngineIsOn)
				EngineBox[0].GoShutDwComp =1;											// spengo
#if (K_AbilCompressor2==1)
			if(EngineBox[1].EngineIsOn)
				EngineBox[1].GoShutDwComp =1;
#endif		
		}
	}
}

void Error_Refrigerant_Charge(void)
{
    static int ErrRefrigChargeNoPress;    
   
    //------------------------------------------------------------
	// controllo per la carica refrigerante
	//------------------------------------------------------------
	if(EngineBox[0].PersErr.CaricaRefrig)
	{
#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)
		if((EngineBox[0].Pressure.Gas > K_RefrigCharge_Press_Min) && (EngineBox[0].Pressure.Liquid > K_RefrigCharge_Press_Min) && (EngineBox[0].Pressure.LiquidCond > K_RefrigCharge_Press_Min) && (ErrRefrigChargeNoPress == 1))
#else
		if((EngineBox[0].Pressure.Gas > K_RefrigCharge_Press_Min) && (EngineBox[0].Pressure.Liquid > K_RefrigCharge_Press_Min) && (ErrRefrigChargeNoPress == 1))
#endif            
        {
			EngineBox[0].PersErr.CaricaRefrig = 0;
            ErrRefrigChargeNoPress = 0;
        }
	}
	else
	{	
#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)
		if((EngineBox[0].Pressure.Gas < K_RefrigCharge_Press_Min) && ((EngineBox[0].Pressure.Liquid < K_RefrigCharge_Press_Min) || (EngineBox[0].Pressure.LiquidCond < K_RefrigCharge_Press_Min)))
#else
		if((EngineBox[0].Pressure.Gas < K_RefrigCharge_Press_Min) && (EngineBox[0].Pressure.Liquid < K_RefrigCharge_Press_Min))
#endif
		{
			EngineBox[0].PersErr.CaricaRefrig = 1;
			ErrRefrigChargeNoPress = 1;
			if(EngineBox[0].EngineIsOn)
				EngineBox[0].GoShutDwComp =1;											// spengo
#if (K_AbilCompressor2==1)
			if(EngineBox[1].EngineIsOn)
				EngineBox[1].GoShutDwComp =1;
#endif		
		}
	}    
}

void Error_Compressor_Hi(void)
{
    //------------------------------------------------------------
	// Contollo limite di alta temperatura mandata compressore 1
	//------------------------------------------------------------
    EngineBox[0].TempRestartCompressorHi = EngineBox[0].TempCompressorHi - 200;

	if(EngineBox[0].Error.CompressorHi)											// se ho il flag settato
	{	if(EngineBox[0].Temperature.Compressor_Output < EngineBox[0].TempRestartCompressorHi) 	// controllo se sono uscito dalla condizione di anomalia
			EngineBox[0].Error.CompressorHi =0;									// e resetto il flag di errore
	}
	else
	{	
        if(EngineBox[0].Temperature.Compressor_Output >= EngineBox[0].TempCompressorHi)			// controllo se la temperatura di mandata è oltre il limite di guardia
		{	
			EngineBox[0].Error.CompressorHi =1;									// setto il flag di errore
			if(EngineBox[0].CntError.CompressorHi < 250)						// gestisco l'incremento del contatore degli errori
				EngineBox[0].CntError.CompressorHi +=1;
			//TimerResetErrori.Value = 0;											// resetto il conteggio del timer di reset degli errori
			if(EngineBox[0].EngineIsOn)
				EngineBox[0].Request_Out_Inverter = SpeedCompressorHI;				// imposto la richiesta del compressore
		}
	}
	TimerErrCompressorHiC1.Enable = (EngineBox[0].Error.CompressorHi != 0)&&(EngineBox[0].Out_Inverter == SpeedCompressorHI);	// gestisco l'abilitazione del timer se allarme attivo e se raggiunto limite compressore
	if(!TimerErrCompressorHiC1.Enable)											// se non ho l'errore
	{	
        TimerErrCompressorHiC1.Value =0;											// resetto il timer
		TimerErrCompressorHiC1.TimeOut =0;
	}
	if(TimerErrCompressorHiC1.TimeOut)											// in caso di time out
	{	
		if(EngineBox[0].EngineIsOn)
			EngineBox[0].GoShutDwComp =1;											// spengo 
		TimerErrCompressorHiC1.TimeOut = 0;										// e resetto il time out  // IMPORTATO DA CORE.C TRAGHETTI V4.119
	}
	if(EngineBox[0].Temperature.Compressor_Output >= TempCriticalCompressorHi)			// se supero una soglia limite di temepratura
	{	
		if(EngineBox[0].EngineIsOn)												
		{
			EngineBox[0].GoShutDwComp =1;											// spengo immediatamente 		
			if(EngineBox[0].Temperature.Compressor_Output >= TempCriticalCompressorHi && EngineBox[0].Out_Inverter > Min_Speed)
				EngineBox[0].Out_Inverter = Min_Speed;
		}
	}    
	if(EngineBox[0].CntError.CompressorHi >= CicliCompressorHi)					// se ho passato il numero di errori limite
	{
		EngineBox[0].PersErr.CompressorHi =1;									// setto l'errore persistente
		EngineBox[0].PersErr.CaricaRefrig = 1;
		if(EngineBox[0].EngineIsOn)
			EngineBox[0].GoShutDwComp =1;											// spengo
	}
    
	TimerResErrCompressorHiC1.Enable = EngineBox[0].CntError.CompressorHi>0;	//!EngineBox[0].Error.CompressorHi;

	if(!TimerResErrCompressorHiC1.Enable)
	{	
		TimerResErrCompressorHiC1.TimeOut = 0;
		TimerResErrCompressorHiC1.Value = 0;
	}

	if(TimerResErrCompressorHiC1.TimeOut)
	{	
		TimerResErrCompressorHiC1.TimeOut = 0;
		TimerResErrCompressorHiC1.Value = 0;
		EngineBox[0].CntError.CompressorHi = 0;
	}

#if (K_AbilCompressor2==1)	
	//------------------------------------------------------------
	// Contollo limite di alta temperatura mandata compressore 2
	//------------------------------------------------------------
	if(EngineBox[1].Error.CompressorHi)											// se ho il flag settato
	{	if(EngineBox[1].Temperature.Compressor_Output < EngineBox[0].TempRestartCompressorHi) 	// controllo se sono uscito dalla condizione di anomalia
			EngineBox[1].Error.CompressorHi =0;									// e resetto il flag di errore
	}
	else
	{	if(EngineBox[1].Temperature.Compressor_Output >= EngineBox[0].TempCompressorHi)			// controllo se la temperatura di mandata è oltre il limite di guardia
		{	
			EngineBox[1].Error.CompressorHi =1;									// setto il flag di errore
			if(EngineBox[1].CntError.CompressorHi < 250)						// gestisco l'incremento del contatore degli errori
				EngineBox[1].CntError.CompressorHi +=1;
			//TimerResetErrori.Value = 0;											// resetto il conteggio del timer di reset degli errori
			if(EngineBox[1].EngineIsOn)
				EngineBox[1].Request_Out_Inverter = SpeedCompressorHI;				// imposto la richiesta del compressore
		}
	}
	TimerErrCompressorHiC2.Enable = (EngineBox[1].Error.CompressorHi != 0)&&(EngineBox[1].Out_Inverter == SpeedCompressorHI);	// gestisco l'abilitazione del timer se allarme attivo e se raggiunto limite compressore
	if(!TimerErrCompressorHiC2.Enable)											// se non ho l'errore
	{	
		TimerErrCompressorHiC2.Value =0;											// resetto il timer
		TimerErrCompressorHiC2.TimeOut =0;
	}
	if(TimerErrCompressorHiC2.TimeOut)											// in caso di time out
	{	
		if(EngineBox[1].EngineIsOn)
			EngineBox[1].GoShutDwComp =1;											// spengo 
		TimerErrCompressorHiC2.TimeOut = 0;										// e resetto il time out  // IMPORTATO DA CORE.C TRAGHETTI V4.119
	}
	if(EngineBox[1].Temperature.Compressor_Output >= TempCriticalCompressorHi)			// se supero una soglia limite di temepratura
	{	
		if(EngineBox[1].EngineIsOn)
		{
			EngineBox[1].GoShutDwComp =1;										// spengo immediatamente
			if(EngineBox[1].Temperature.Compressor_Output >= TempCriticalCompressorHi && EngineBox[1].Out_Inverter > Min_Speed)
				EngineBox[1].Out_Inverter = Min_Speed;
		}
	}	    
	if(EngineBox[1].CntError.CompressorHi >= CicliCompressorHi)					// se ho passato il numero di errori limite
	{
		EngineBox[1].PersErr.CompressorHi =1;									// setto l'errore persistente
		EngineBox[0].PersErr.CaricaRefrig = 1;
		if(EngineBox[1].EngineIsOn)
			EngineBox[1].GoShutDwComp =1;											// spengo
	}
    
	TimerResErrCompressorHiC2.Enable = EngineBox[1].CntError.CompressorHi>0;	//!EngineBox[0].Error.CompressorHi;

	if(!TimerResErrCompressorHiC2.Enable)
	{	
		TimerResErrCompressorHiC2.TimeOut = 0;
		TimerResErrCompressorHiC2.Value = 0;
	}

	if(TimerResErrCompressorHiC2.TimeOut)
	{	
		TimerResErrCompressorHiC2.TimeOut = 0;
		TimerResErrCompressorHiC2.Value = 0;
		EngineBox[1].CntError.CompressorHi = 0;
	}
    
#endif
}

void Error_Compressor_Lo(void)
{
/*
	//------------------------------------------------------------
	// controllo limite di bassa temperatura compressore
	//------------------------------------------------------------
	if(EngineBox[0].Error.CompressorLo)											// se ho il flag di errore settato
	{	
		if(EngineBox[0].Temperature.Compressor_Suction > TempRestartCompressorLo)		// controllo se sono uscito dalla condizione di anomalia 
			EngineBox[0].Error.CompressorLo =0;									// e resetto il flag di errore
	}
	else
	{	
		if(((EngineBox[0].Temperature.Compressor_Suction < TempCompressorLoRaffr) && EngineBox[0].InverterRaffrescamento) ||
			((EngineBox[0].Temperature.Compressor_Suction < TempCompressorLoRisc) && EngineBox[0].InverterRiscaldamento))	// controllo se la temp di ritorno è al di sotto della soglia
		{	
			EngineBox[0].Error.CompressorLo =1;									// e setto il flag di errore
			//if(EngineBox[0].CntError.CompressorLo < 250)						// gestisco l'incremento del contatore degli errori
			//	EngineBox[0].CntError.CompressorLo +=1;
			//TimerResetErrori.Value = 0;											// resetto il conteggio del timer di reset degli errori
			if(EngineBox[0].EngineIsOn)
			{
				if(EngineBox[0].CoolingMode)								// imposto la richiesta del compressore
					EngineBox[0].Request_Out_Inverter = SpeedCompressorLoRaff;
				else
					EngineBox[0].Request_Out_Inverter = SpeedCompressorLoRisc;
			}
	#if (K_AbilCompressor2==1)
			if(EngineBox[1].EngineIsOn)
			{
				if(EngineBox[1].CoolingMode)								// imposto la richiesta del compressore
					EngineBox[1].Request_Out_Inverter = SpeedCompressorLoRaff;
				else
					EngineBox[1].Request_Out_Inverter = SpeedCompressorLoRisc;
			}				
	#endif		
		}
	}

#if (K_AbilCompressor2==1)
	TimerErrCompressorLo.Enable = (EngineBox[0].Error.CompressorLo != 0)&&
	((((EngineBox[0].Out_Inverter == SpeedCompressorLoRaff) && EngineBox[0].CoolingMode) || ((EngineBox[1].Out_Inverter == SpeedCompressorLoRaff) && EngineBox[1].CoolingMode)) ||
	(((EngineBox[0].Out_Inverter == SpeedCompressorLoRisc) && EngineBox[0].HeatingMode) || ((EngineBox[1].Out_Inverter == SpeedCompressorLoRisc)  && EngineBox[1].HeatingMode)));	// gestisco l'abilitazione del timer se allarme attivo e se raggiunto limite compressore
#else
	TimerErrCompressorLo.Enable = (EngineBox[0].Error.CompressorLo != 0)&&
	(((EngineBox[0].Out_Inverter == SpeedCompressorLoRaff) && EngineBox[0].CoolingMode) || 
	((EngineBox[0].Out_Inverter == SpeedCompressorLoRisc) && EngineBox[0].HeatingMode));	// gestisco l'abilitazione del timer se allarme attivo e se raggiunto limite compressore		
#endif						
	if(!TimerErrCompressorLo.Enable)											// se non è attivo
	{	
		TimerErrCompressorLo.Value =0;											// resetto il timer
		TimerErrCompressorLo.TimeOut =0;										
	}
	if(TimerErrCompressorLo.TimeOut)											// in caso di time out
	{	
		if(EngineBox[0].EngineIsOn)
			EngineBox[0].GoShutDwComp =1;											// spengo 
#if (K_AbilCompressor2==1)
		if(EngineBox[1].EngineIsOn)
			EngineBox[1].GoShutDwComp =1;
#endif		
		TimerErrCompressorLo.TimeOut = 0;										// e resetto il timer	// IMPORTATO DA CORE.C TRAGHETTI V4.119
	}

	if(EngineBox[0].CntError.CompressorLo >= CicliCompressorLo)					// se ho passato il numero di errori limite
	{
		EngineBox[0].PersErr.CompressorLo =1;									// setto l'errore persistente
		EngineBox[0].PersErr.CaricaRefrig = 1;
		if(EngineBox[0].EngineIsOn)
			EngineBox[0].GoShutDwComp =1;											// spengo
#if (K_AbilCompressor2==1)
		if(EngineBox[1].EngineIsOn)
			EngineBox[1].GoShutDwComp =1;
#endif				
	}

	TimerResErrCompressorLo.Enable = EngineBox[0].CntError.CompressorLo>0;	//!EngineBox[0].Error.CompressorLo;

	if(!TimerResErrCompressorLo.Enable)
	{	
		TimerResErrCompressorLo.TimeOut = 0;
		TimerResErrCompressorLo.Value = 0;
	}

	if(TimerResErrCompressorLo.TimeOut)
	{	
		TimerResErrCompressorLo.TimeOut = 0;
		TimerResErrCompressorLo.Value = 0;
		EngineBox[0].CntError.CompressorLo = 0;
	}
*/
    EngineBox[0].Error.CompressorLo = 0;    //Per ora bassa temperatura non agisce
}

void Error_Condenser_Hi(void)
{
	//------------------------------------------------------------
	// controllo limite alta temperatura condensatore
	//------------------------------------------------------------
	if(EngineBox[0].Error.CondensatoreHi)										// se ho il flag di errore attivo
	{	if(EngineBox[0].Temperature.Condenser < TempRestartCondensatoreHi)			// controllo se sono uscito dalla condizione di anomanlia
			EngineBox[0].Error.CondensatoreHi =0;								// e resetto il flag di errore
		//if(EngineBox[0].Out_Inverter == 0)									// controllo se il motore si è spento
		//	EngineBox[0].PersErr.CondensatoreHi =1;								// setto l'errore persistente
	}
	else
	{	
		if(EngineBox[0].Temperature.Condenser > TempCondensatoreHi)					// se la temperatura el condensatore è superariore al limite
		{	
			EngineBox[0].Error.CondensatoreHi =1;								// setto il flag di errore
/*
			if((EngineBox[0].CntError.CondensatoreHi < 250)&(EngineBox[0].InverterRaffrescamento|EngineBox[0].InverterRiscaldamento|EngineBox[0].GoRiscaldamento|EngineBox[0].GoRaffrescamento))						// gestisco l'incremento del contatore degli errori
			{		
				//EngineBox[0].CntError.CondensatoreHi +=1;
				//TimerResetErrori.Value = 0;											// resetto il conteggio del timer di reset degli errori
				EngineBox[0].Request_Out_Inverter = SpeedCondensatoreHi;			// imposto la richiesta del compressore
#if (K_AbilCompressor2==1)
				EngineBox[1].Request_Out_Inverter = SpeedCondensatoreHi;			// imposto la richiesta del compressore
#endif
			}
*/
			if((EngineBox[0].CntError.CondensatoreHi < 250)&&(EngineBox[0].EngineIsOn || EngineBox[1].EngineIsOn))						// gestisco l'incremento del contatore degli errori
			{		
				//EngineBox[0].CntError.CondensatoreHi +=1;
				//TimerResetErrori.Value = 0;											// resetto il conteggio del timer di reset degli errori
				if(EngineBox[0].EngineIsOn)
					EngineBox[0].Request_Out_Inverter = SpeedCondensatoreHi;			// imposto la richiesta del compressore
#if (K_AbilCompressor2==1)
				if(EngineBox[1].EngineIsOn)						// gestisco l'incremento del contatore degli errori
					EngineBox[1].Request_Out_Inverter = SpeedCondensatoreHi;			// imposto la richiesta del compressore
#endif				
			}
		}
	}

	TimerErrCondensatoreHi.Enable = ((EngineBox[0].Error.CondensatoreHi != 0)&&(EngineBox[0].EngineIsOn || EngineBox[1].EngineIsOn));	// gestisco l'abilitazione del timer
	if(!TimerErrCondensatoreHi.Enable)											// se il timer non è attivo
	{	
		TimerErrCondensatoreHi.Value =0;										// resetto il timer
		TimerErrCondensatoreHi.TimeOut =0;
	}
	if(TimerErrCondensatoreHi.TimeOut)											// in caso di time out
	{	
		if(EngineBox[0].EngineIsOn)
			EngineBox[0].GoShutDwComp =1;											// spengo 
#if (K_AbilCompressor2==1)
		if(EngineBox[1].EngineIsOn)
			EngineBox[1].GoShutDwComp =1;
#endif		
		TimerErrCondensatoreHi.TimeOut = 0;										// e resetto il timer
	}
/*
	if(EngineBox[0].CntError.CondensatoreHi >= CicliCondensatoreHi)				// se ho passato il numero di errori limite
	{	
		EngineBox[0].PersErr.CondensatoreHi =1;									// setto l'errore persistente
		if(EngineBox[0].EngineIsOn)
			EngineBox[0].GoShutDwComp =1;											// spengo
#if (K_AbilCompressor2==1)
		if(EngineBox[1].EngineIsOn)
			EngineBox[1].GoShutDwComp =1;
#endif		
 
	}
*/
	TimerResErrCondensatoreHi.Enable = EngineBox[0].CntError.CondensatoreHi>0;	//!EngineBox[0].Error.CondensatoreHi;

	if(!TimerResErrCondensatoreHi.Enable)
	{	
		TimerResErrCondensatoreHi.TimeOut = 0;
		TimerResErrCondensatoreHi.Value = 0;
	}

	if(TimerResErrCondensatoreHi.TimeOut)
	{	
		TimerResErrCondensatoreHi.TimeOut = 0;
		TimerResErrCondensatoreHi.Value = 0;
		EngineBox[0].CntError.CondensatoreHi = 0;
	}
}

void Error_Condenser_Lo(void)
{
	int Err = 0;

	//------------------------------------------------------------
	// controllo limite bassa temperatura condensatore
	//------------------------------------------------------------
	Err = EngineBox[0].Temperature.Sea_Water - EngineBox[0].Temperature.Condenser;
	
    if(EngineBox[0].Error.CondensatoreLo)										// se ho il flag di errore attivo
	{	        
        if(((Err<TempRestartErrCondLo)||(EngineBox[0].Temperature.Sea_Water>600)) && (EngineBox[0].Temperature.Condenser>TempRestartCondensatoreLo))		// controllo se sono uscito dalla condizione di anomalia
            EngineBox[0].Error.CondensatoreLo =0;								// e retto il flag di errore
	}
	else
	{	
		if((Err > TempErrCondLo)&&(EngineBox[0].Temperature.Sea_Water<600)&&!EngineBox[0].PersErr.CondensatoreLo)//700))			// se errore >4° e temp acqua mare minore di 7°														// controllo se la temperatura del condensatore è inferiore alla temperatura di guardia
		{	
			EngineBox[0].Error.CondensatoreLo =1;								// è imposto il flag di allarme
/*
			if((EngineBox[0].CntError.CondensatoreLo < 250)&(EngineBox[0].EngineIsOn))						// gestisco l'incremento del contatore degli errori
				EngineBox[0].CntError.CondensatoreLo +=1;
			{			//TimerResetErrori.Value = 0;											// resetto il conteggio del timer di reset degli errori
			EngineBox[0].Request_Out_Inverter = SpeedCondensatoreLo;			// imposto la richiesta del compressore
#if (K_AbilCompressor2==1)
			EngineBox[1].Request_Out_Inverter = SpeedCondensatoreLo;			// imposto la richiesta del compressore
#endif
			}
*/
			if((EngineBox[0].CntError.CondensatoreLo < 250)&&(EngineBox[0].EngineIsOn || EngineBox[1].EngineIsOn))						// gestisco l'incremento del contatore degli errori
			{		
				EngineBox[0].CntError.CondensatoreLo +=1;
				//TimerResetErrori.Value = 0;											// resetto il conteggio del timer di reset degli errori
				if(EngineBox[0].EngineIsOn)
					EngineBox[0].Request_Out_Inverter = SpeedCondensatoreLo;			// imposto la richiesta del compressore
#if (K_AbilCompressor2==1)
				if(EngineBox[1].EngineIsOn)						// gestisco l'incremento del contatore degli errori
					EngineBox[1].Request_Out_Inverter = SpeedCondensatoreLo;			// imposto la richiesta del compressore
#endif				
			}

		}
	}

	TimerErrCondensatoreLo.Enable = ((EngineBox[0].Error.CondensatoreLo != 0)&&(EngineBox[0].EngineIsOn || EngineBox[1].EngineIsOn));	// gestisco l'abilitazione del timer
	if(!TimerErrCondensatoreLo.Enable)											// se il timer non è attivo
	{	
		TimerErrCondensatoreLo.Value =0;										// resetto il timer
		TimerErrCondensatoreLo.TimeOut =0;
	}
	if(TimerErrCondensatoreLo.TimeOut)											// in caso di time out
	{	
		if(EngineBox[0].EngineIsOn)
			EngineBox[0].GoShutDwComp =1;											// spengo 
#if (K_AbilCompressor2==1)
		if(EngineBox[1].EngineIsOn)
			EngineBox[1].GoShutDwComp =1;
#endif		
		TimerErrCondensatoreLo.TimeOut = 0;										// e resetto il timer	// IMPORTATO DA CORE.C TRAGHETTI V4.119
	}
	if(EngineBox[0].CntError.CondensatoreLo >= CicliCondensatoreLo)				// se ho passato il numero di errori limite
	{
		EngineBox[0].PersErr.CondensatoreLo =1;									// setto l'errore persistente
        EngineBox[0].Error.CondensatoreLo =0;
		if(EngineBox[0].EngineIsOn)
			EngineBox[0].GoShutDwComp =1;											// spengo
#if (K_AbilCompressor2==1)
		if(EngineBox[1].EngineIsOn)
			EngineBox[1].GoShutDwComp =1;
#endif		
	}
    
	TimerResErrCondensatoreLo.Enable = EngineBox[0].CntError.CondensatoreLo>0;	//!EngineBox[0].Error.CondensatoreLo;

	if(!TimerResErrCondensatoreLo.Enable)
	{	
		TimerResErrCondensatoreLo.TimeOut = 0;
		TimerResErrCondensatoreLo.Value = 0;
	}

	if(TimerResErrCondensatoreLo.TimeOut)
	{	
		TimerResErrCondensatoreLo.TimeOut = 0;
		TimerResErrCondensatoreLo.Value = 0;
		EngineBox[0].CntError.CondensatoreLo = 0;
	}
}

void Error_Gas_Recovery(void)
{
	/*
	//------------------------------------------------------------
	// controllo per il ciclo di recupero del gas
	//------------------------------------------------------------
	if(EngineBox[0].HeatingMode)		// se sono in riscaldamento o sta andando in riscaldmaneto
	{	if(EngineBox[0].Error.Gas_Recovery)										// se ho il flag di errore attivo
		{	
			if(EngineBox[0].Pressure.Liquid > Gas_Recovery_PressureRestore)  	// controllo se sono uscito dalla condizione di anomalia		
				EngineBox[0].Error.Gas_Recovery = 0;							// e resetto il flag di recovery
		}
		else
		{	
#if (K_AbilCompressor2==1)
			if(((EngineBox[0].Out_Inverter > Gas_Recovery_SpeedLimit) ||
				(EngineBox[1].Out_Inverter > Gas_Recovery_SpeedLimit))	&& 			// se il compressore gira al massimo
				(EngineBox[0].Pressure.Gas < Gas_Recovery_PressureLimit) && 	// e non raggiunge la pressione minima
				(EngineBox[0].Pressure.Liquid < Liq_Recovery_PressureLimit))
#else
			if((EngineBox[0].Out_Inverter > Gas_Recovery_SpeedLimit) && 			// se il compressore gira al massimo
				(EngineBox[0].Pressure.Gas < Gas_Recovery_PressureLimit) && 	// e non raggiunge la pressione minima
				(EngineBox[0].Pressure.Liquid < Liq_Recovery_PressureLimit))
#endif			
			{	
				EngineBox[0].Error.Gas_Recovery = 1;							// segnalo l'anomalia
				if(EngineBox[0].CntError.GasRecovery < 250)						// gestisco l'incremento del contatore degli errori
				{		
					EngineBox[0].CntError.GasRecovery +=1;
				//TimerResetErrori.Value = 0;										// resetto il conteggio del timer di reset degli errori
					if(EngineBox[0].EngineIsOn)
						EngineBox[0].Request_Out_Inverter = SpeedGasRecovery;	// imposto la richiesta del compressore
#if (K_AbilCompressor2==1)
					if(EngineBox[1].EngineIsOn)
						EngineBox[1].Request_Out_Inverter = SpeedGasRecovery;	// imposto la richiesta del compressore
#endif
				}
			}
		}
	}
	else
		EngineBox[0].Error.Gas_Recovery = 0;
	
	TimerErrGasRecovery.Enable = (EngineBox[0].Error.Gas_Recovery !=0) && (EngineBox[0].HeatingMode || EngineBox[1].HeatingMode);	// gestisco l'abilitazione del timer
	if(!TimerErrGasRecovery.Enable)											// se il timer non è attivo
	{	
		TimerErrGasRecovery.Value =0;										// resetto il timer
		TimerErrGasRecovery.TimeOut =0;
	}

	if((EngineBox[0].CntError.GasRecovery >= CicliGas_Recovery) || TimerErrGasRecovery.TimeOut)					// se ho passato il numero di errori limite
	{
		EngineBox[0].PersErr.Gas_Recovery =1;									// setto l'errore persistente
		EngineBox[0].PersErr.CaricaRefrig = 1;
		if(EngineBox[0].EngineIsOn)
			EngineBox[0].GoShutDwComp =1;											// spengo
#if (K_AbilCompressor2==1)
		if(EngineBox[1].EngineIsOn)
			EngineBox[1].GoShutDwComp =1;
#endif		
	}

	TimerResTime_Gas_Rec.Enable = EngineBox[0].CntError.GasRecovery>0;	//!EngineBox[0].Error.Gas_Recovery;

	if(!TimerResTime_Gas_Rec.Enable)
	{	
		TimerResTime_Gas_Rec.TimeOut = 0;
		TimerResTime_Gas_Rec.Value = 0;
	}

	if(TimerResTime_Gas_Rec.TimeOut)
	{	
		TimerResTime_Gas_Rec.TimeOut = 0;
		TimerResTime_Gas_Rec.Value = 0;
		EngineBox[0].CntError.GasRecovery = 0;
	}

	*/
    EngineBox[0].Error.Gas_Recovery = 0;    
    EngineBox[0].PersErr.Gas_Recovery = 0;            
}

void Error_Pressure_Lo(void)
{
    //------------------------------------------------------------
	// controllo per la bassa pressione
	//------------------------------------------------------------
	if(EngineBox[0].Error.Pressure_Lo)											// se ho il flag di errore attivo
	{
#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)
		if(!EngineBox[0].In.CompressorLo && (EngineBox[0].Pressure.Liquid > PressioneLo_Liq_P) && (EngineBox[0].Pressure.LiquidCond > PressRestartLo_Liq_P) && (EngineBox[0].Pressure.Gas > PressRestartLo_Gas_G))
#else
		if(!EngineBox[0].In.CompressorLo && (EngineBox[0].Pressure.Liquid > PressRestartLo_Liq_P) && (EngineBox[0].Pressure.Gas > PressRestartLo_Gas_G))            
#endif
			EngineBox[0].Error.Pressure_Lo =0;								// e resetto il flag di errore
	}
	else
	{	
#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)
		if(EngineBox[0].In.CompressorLo || (EngineBox[0].Pressure.Liquid < PressioneLo_Liq_P) || (EngineBox[0].Pressure.LiquidCond < PressioneLo_Liq_P) || (EngineBox[0].Pressure.Gas < PressioneLo_Gas_G))
#else
		if(EngineBox[0].In.CompressorLo || (EngineBox[0].Pressure.Liquid < PressioneLo_Liq_P) || (EngineBox[0].Pressure.Gas < PressioneLo_Gas_G))
#endif
		{	
			EngineBox[0].Error.Pressure_Lo =1;									// setto il flag di errore
			if(EngineBox[0].EngineIsOn)
				EngineBox[0].Request_Out_Inverter = SpeedPressioneLo;				// imposto la richiesta del compressore
#if (K_AbilCompressor2==1)
			if(EngineBox[1].EngineIsOn)
				EngineBox[1].Request_Out_Inverter = SpeedPressioneLo;				// imposto la richiesta del compressore
#endif
		}		
	}

	TimerErrPressureLo.Enable = (EngineBox[0].Error.Pressure_Lo !=0) && (EngineBox[0].EngineIsOn || EngineBox[1].EngineIsOn);	// gestisco l'abilitazione del timer
	if(!TimerErrPressureLo.Enable)											// se il timer non è attivo
	{	
		TimerErrPressureLo.Value =0;										// resetto il timer
		TimerErrPressureLo.TimeOut =0;
	}
	if(TimerErrPressureLo.TimeOut)											// in caso di time out
	{
		if(EngineBox[0].EngineIsOn)
			EngineBox[0].GoShutDwComp =1;											// spengo 
#if (K_AbilCompressor2==1)
		if(EngineBox[1].EngineIsOn)
			EngineBox[1].GoShutDwComp =1;
#endif		
	}
}

void Error_Pressure_Hi(void)
{
	//------------------------------------------------------------
	// controllo per la alta pressione
	//------------------------------------------------------------
	if(EngineBox[0].Error.Pressure_Hi)											// se ho il flag di errore attivo
	{
#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)
		if(!EngineBox[0].In.CompressorHi && (EngineBox[0].Pressure.Liquid < PressRestartHi_Liq_P) && (EngineBox[0].Pressure.LiquidCond < PressRestartHi_Liq_P) && (EngineBox[0].Pressure.Gas < PressRestartHi_Gas_G))
#else
		if(!EngineBox[0].In.CompressorHi && (EngineBox[0].Pressure.Liquid < PressRestartHi_Liq_P) && (EngineBox[0].Pressure.Gas < PressRestartHi_Gas_G))
#endif

			EngineBox[0].Error.Pressure_Hi =0;								// e resetto il flag di errore
	}
	else
	{	
#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)
		if(EngineBox[0].In.CompressorHi || (EngineBox[0].Pressure.Liquid > PressioneHi_Liq_P) || (EngineBox[0].Pressure.LiquidCond > PressioneHi_Liq_P) || (EngineBox[0].Pressure.Gas > PressioneHi_Gas_G))
#else
        if(EngineBox[0].In.CompressorHi || (EngineBox[0].Pressure.Liquid > PressioneHi_Liq_P) || (EngineBox[0].Pressure.Gas > PressioneHi_Gas_G))    
#endif
		{	
			EngineBox[0].Error.Pressure_Hi =1;									// setto il flag di errore
			if(EngineBox[0].EngineIsOn)
				EngineBox[0].Request_Out_Inverter = SpeedPressioneHi;				// imposto la richiesta del compressore
#if (K_AbilCompressor2==1)
			if(EngineBox[1].EngineIsOn)
				EngineBox[1].Request_Out_Inverter = SpeedPressioneHi;				// imposto la richiesta del compressore
#endif
		}		
	}

	TimerErrPressureHi.Enable = (EngineBox[0].Error.Pressure_Hi !=0) &&
								(((EngineBox[0].Out_Inverter == SpeedPressioneHi)&&(EngineBox[0].EngineIsOn)) ||
								((EngineBox[1].Out_Inverter == SpeedPressioneHi)&&(EngineBox[1].EngineIsOn)));	// gestisco l'abilitazione del timer
	
	if(!TimerErrPressureHi.Enable)											// se il timer non è attivo
	{	
		TimerErrPressureHi.Value =0;										// resetto il timer
		TimerErrPressureHi.TimeOut =0;
	}
	if(TimerErrPressureHi.TimeOut)											// in caso di time out
	{
		if(EngineBox[0].EngineIsOn)
			EngineBox[0].GoShutDwComp =1;											// spengo 
#if (K_AbilCompressor2==1)
		if(EngineBox[1].EngineIsOn)
			EngineBox[1].GoShutDwComp =1;
#endif		
		TimerErrPressureHi.TimeOut =0;
	}
#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)
	if(EngineBox[0].In.CompressorHi || (EngineBox[0].Pressure.Liquid > PressioneCriticalHi_Liq_P) || (EngineBox[0].Pressure.LiquidCond > PressioneCriticalHi_Liq_P) || (EngineBox[0].Pressure.Gas > PressioneCriticalHi_Gas_G))			// se supero una soglia limite di temepratura
#else
    if(EngineBox[0].In.CompressorHi || (EngineBox[0].Pressure.Liquid > PressioneCriticalHi_Liq_P) || (EngineBox[0].Pressure.Gas > PressioneCriticalHi_Gas_G))			// se supero una soglia limite di temepratura
#endif
	{	
		if(EngineBox[0].EngineIsOn)												
		{
			EngineBox[0].GoShutDwComp =1;											// spengo immediatamente 		
			if(EngineBox[0].Out_Inverter > Min_Speed)
				EngineBox[0].Out_Inverter = Min_Speed;
		}

#if (K_AbilCompressor2==1)
        if(EngineBox[1].EngineIsOn)
        {
            EngineBox[1].GoShutDwComp =1;										// spengo immediatamente
            if(EngineBox[1].Out_Inverter > Min_Speed)
                EngineBox[1].Out_Inverter = Min_Speed;
        }
#endif		
	}     
}   

void Error_Condenser_Fouled(void)
{
    int Err;

	//------------------------------------------------------------
	// Contollo del condensatore sporco
	//------------------------------------------------------------
    Err = EngineBox[0].Temperature.Liquid - EngineBox[0].Temperature.Sea_Water;
    
	if(EngineBox[0].Error.CondenserFouled)											// se ho il flag settato
	{	
		if(Err < K_TempRestartCondenserFouled) 	// controllo se sono uscito dalla condizione di anomalia
			EngineBox[0].Error.CondenserFouled =0;									// e resetto il flag di errore
        
        TimerCondenserFouled.Value = 0;
        TimerCondenserFouled.TimeOut = 0;     
	}
	else
	{
#if (K_AbilCompressor2==1)        
        TimerCondenserFouled.Enable = (Err > K_TempCondenserFouled) && ((EngineBox[0].Out_Inverter>Off_Speed) || (EngineBox[1].Out_Inverter>Off_Speed)) && EngineBox[0].InverterRaffrescamento;
#else
        TimerCondenserFouled.Enable = (Err > K_TempCondenserFouled) && (EngineBox[0].Out_Inverter>Off_Speed) && EngineBox[0].InverterRaffrescamento;
#endif
        if(!TimerCondenserFouled.Enable)
        {
            TimerCondenserFouled.Value = 0;
            TimerCondenserFouled.TimeOut = 0;
        }
        
        if(TimerCondenserFouled.TimeOut)			// controllo se la temperatura di mandata è oltre il limite di guardia
		{	
            TimerCondenserFouled.Value = 0;
            TimerCondenserFouled.TimeOut = 0;
			EngineBox[0].Error.CondenserFouled =1;									// setto il flag di errore
		}
	}
}

void Error_Flow_Req(void)
{
    if(EngineBox[0].EngineIsOn || EngineBox[1].EngineIsOn)
        TimerErrFlussostato.Time = TimeFlussostato_Work;
    else
        TimerErrFlussostato.Time = TimeFlussostato_StandBy;

    TimerErrFlussostato.Enable = !EngineBox[0].In.Flow && EngineBox[0].Out.Pompa_Acqua && (EngineBox[0].StatoCompressore != WAIT_CHECK_FLUX) && (EngineBox[1].StatoCompressore != WAIT_CHECK_FLUX);    

    if(!TimerErrFlussostato.Enable)
	{	
		TimerErrFlussostato.Value = 0;
		TimerErrFlussostato.TimeOut = 0;
	} 	
    if(TimerErrFlussostato.TimeOut)
    {
        EngineBox[0].PersErr.WaterSeaFlux =1;       

        if(EngineBox[0].EngineIsOn && EngineBox[0].GoShutDwComp==0)
        {
            EngineBox[0].Request_Out_Inverter = Min_Speed;				// imposto la richiesta del compressore
            if(EngineBox[0].Out_Inverter>Min_Speed)
                 EngineBox[0].Out_Inverter = Min_Speed;
             EngineBox[0].GoShutDwComp=1;
        }
#if (K_AbilCompressor2==1)        
        if(EngineBox[1].EngineIsOn && EngineBox[1].GoShutDwComp==0)
        {
            EngineBox[1].Request_Out_Inverter = Min_Speed;				// imposto la richiesta del compressore            
            if(EngineBox[1].Out_Inverter>Min_Speed)
                 EngineBox[1].Out_Inverter = Min_Speed;        
             EngineBox[1].GoShutDwComp=1;
        }
#endif
		TimerErrFlussostato.Value = 0;
		TimerErrFlussostato.TimeOut = 0;        
	}
    if(EngineBox[0].PersErr.WaterSeaFlux && (EngineBox[0].In.Flow || !EngineBox[0].Out.Pompa_Acqua || (Room[1].SystemDisable==1)))
        EngineBox[0].PersErr.WaterSeaFlux = 0;
}

void Error_Flow_Test(void)
{
#if (K_AbilCompressor2==1)
    if(EngineBox[0].EngineIsOn || EngineBox[1].EngineIsOn)
        TimerErrFlussostato.Time = TimeFlussostato_Work;
    else if((EngineBox[0].Manual_Pump_Abil==1) && (Room[1].SystemDisable==1))
        TimerErrFlussostato.Time = TimeFlussostato_TestPump;        
    else
        TimerErrFlussostato.Time = TimeFlussostato_StandBy;

#if(K_Abil_Double_Pump==1)
    TimerErrFlussostato.Enable = !EngineBox[0].In.Flow && (EngineBox[0].Out.Pompa_Acqua || EngineBox[0].Out.Pompa_Acqua2) && !EngineBox[0].PersErr.PumpInverterFault && !EngineBox[0].GoShutDwComp && !EngineBox[1].GoShutDwComp && !EngineBox[0].Test.FluxReq && (EngineBox[0].StatoCompressore != WAIT_CHECK_FLUX) && (EngineBox[1].StatoCompressore != WAIT_CHECK_FLUX);
#else
	TimerErrFlussostato.Enable = !EngineBox[0].In.Flow && EngineBox[0].Out.Pompa_Acqua && !EngineBox[0].PersErr.PumpInverterFault && !EngineBox[0].GoShutDwComp && !EngineBox[1].GoShutDwComp && !EngineBox[0].Test.FluxReq && (EngineBox[0].StatoCompressore != WAIT_CHECK_FLUX) && (EngineBox[1].StatoCompressore != WAIT_CHECK_FLUX);
#endif
#else	
    if(EngineBox[0].EngineIsOn)
        TimerErrFlussostato.Time = TimeFlussostato_Work;
    else if((EngineBox[0].Manual_Pump_Abil==1) && (Room[1].SystemDisable==1))
        TimerErrFlussostato.Time = TimeFlussostato_TestPump;        
    else
        TimerErrFlussostato.Time = TimeFlussostato_StandBy;
    
#if(K_Abil_Double_Pump==1)
    TimerErrFlussostato.Enable = !EngineBox[0].In.Flow && (EngineBox[0].Out.Pompa_Acqua || EngineBox[0].Out.Pompa_Acqua2) && !EngineBox[0].PersErr.PumpInverterFault && !EngineBox[0].GoShutDwComp && !EngineBox[0].Test.FluxReq && (EngineBox[0].StatoCompressore != WAIT_CHECK_FLUX);
#else
	TimerErrFlussostato.Enable = !EngineBox[0].In.Flow && EngineBox[0].Out.Pompa_Acqua && !EngineBox[0].PersErr.PumpInverterFault && !EngineBox[0].GoShutDwComp && !EngineBox[0].Test.FluxReq && (EngineBox[0].StatoCompressore != WAIT_CHECK_FLUX);
#endif
#endif

    if(!TimerErrFlussostato.Enable)
	{	
		TimerErrFlussostato.Value = 0;
		TimerErrFlussostato.TimeOut = 0;
	} 	
    if(TimerErrFlussostato.TimeOut)
    {
		EngineBox[0].Test.FluxReq = 1;
       if(EngineBox[0].EngineIsOn)
            EngineBox[0].Out_Inverter = Min_Speed;
#if (K_AbilCompressor2==1)        
        if(EngineBox[1].EngineIsOn)
            EngineBox[1].Out_Inverter = Min_Speed;        
#endif        
		TimerErrFlussostato.Value = 0;
		TimerErrFlussostato.TimeOut = 0;        
	}
	if(EngineBox[0].PersErr.PumpInverterFault)	//Se scatta allarme flusso permanente ma ho errore inverter, lo resetto.
		EngineBox[0].PersErr.WaterSeaFlux = 0;

    
	if(EngineBox[0].PersErr.CondensatoreHi/* || EngineBox[0].PersErr.CondensatoreLo*/ &&
		!(EngineBox[0].PersErr.CaricaRefrig || EngineBox[0].PersErr.WaterSeaFlux || EngineBox[0].PersErr.EEV_Open ||
			EngineBox[0].PersErr.EEV_Close || EngineBox[0].PersErr.FloodSensor || EngineBox[0].PersErr.ThermicComp))
		EngineBox[0].Test.FluxReq = 1;
    
	//------------------------------------------------------------
	//	gestione test flusso acqua condensatore
	//------------------------------------------------------------
	if(EngineBox[0].Test.FluxReq && (!EngineBox[0].Test.EEV_Req || 				// se il test delle EEV
		(EngineBox[0].Test.EEV_Req && EngineBox[0].StatoCompressore < START_EEV_TEST)))	// non è iniziato
	{
		if(EngineBox[0].StatoCompressore < START_FLUX_TEST)
			EngineBox[0].StatoCompressore = START_FLUX_TEST;
#if (K_AbilCompressor2==1)
			EngineBox[1].StatoCompressore = START_SM;
#endif
            
		if(((EngineBox[0].StatoCompressore > START_FLUX_TEST) && (EngineBox[0].StatoCompressore < START_EEV_TEST)) &&
			(EngineBox[0].PersErr.FloodSensor || EngineBox[0].PersErr.WaterSeaFlux))
		{	
            EngineBox[0].StatoCompressore = START_SM;
			EngineBox[0].Test.FluxReq = 0;
			//DEBUG_PMP EngineBox[0].Out.Pompa_Acqua = 0;
			EngineBox[0].PumpON_FluxTestFlag = 0;
		}

		switch(EngineBox[0].StatoCompressore)
		{
			case START_FLUX_TEST: //20:
				//DEBUG_PMP EngineBox[0].Out.Pompa_Acqua = 0;					// spengo subito la pompa acqua
				EngineBox[0].PumpON_FluxTestFlag = 1;
#if (K_AbilCompressor2==1)
				EngineBox[0].Request_Out_Inverter = Min_Speed;		// imposto la richiesta del compressore al minimo
				EngineBox[1].Request_Out_Inverter = Min_Speed;		// imposto la richiesta del compressore al minimo
				if(EngineBox[0].Out_Inverter <= Min_Speed && EngineBox[1].Out_Inverter <= Min_Speed)			// nel caso in cui sono già al minimo
				{	
					EngineBox[0].Out_Inverter = Off_Speed;				// spengo il compressore 1
					EngineBox[1].Out_Inverter = Off_Speed;				// spengo il compressore 2
					EngineBox[0].PumpON_FluxTestFlag = 0;
#if(K_AbilPumpInverter==0 && K_AbilPumpDAC==0)
                    if(EngineBox[0].CntError.FluxTest==0)
						PausaCompressore.Time = 20;	//20				// imposto il timer per capire se si chiude il flussostato entro 20 secondi
                    else
                        PausaCompressore.Time = 3;	//20	
#else
                    if(EngineBox[0].CntError.FluxTest==0)
						PausaCompressore.Time = 20;	//20				// imposto il timer per capire se si chiude il flussostato entro 20 secondi
                    else
                        PausaCompressore.Time = 15;	//20	
#endif
					PausaCompressore.Value = 0;
					PausaCompressore.TimeOut =0;
					PausaCompressore.Enable =1;
					EngineBox[0].StatoCompressore = CHECK_TIME_NOFLUX;	// vado allo step successivo
				}
#else
				EngineBox[0].Request_Out_Inverter = Min_Speed;		// imposto la richiesta del compressore al minimo
				if(EngineBox[0].Out_Inverter <= Min_Speed)			// nel caso in cui sono già al minimo
				{	
					EngineBox[0].Out_Inverter = Off_Speed;				// spengo il compressore
					EngineBox[0].PumpON_FluxTestFlag = 0;
#if(K_AbilPumpInverter==0 && K_AbilPumpDAC==0)
                    if(EngineBox[0].CntError.FluxTest==0)
						PausaCompressore.Time = 20;	//20				// imposto il timer per capire se si chiude il flussostato entro 20 secondi
                    else
                        PausaCompressore.Time = 3;	//20	
#else
                    if(EngineBox[0].CntError.FluxTest==0)
						PausaCompressore.Time = 20;	//20				// imposto il timer per capire se si chiude il flussostato entro 20 secondi
                    else
                        PausaCompressore.Time = 15;	//20	
#endif
					PausaCompressore.Value = 0;
					PausaCompressore.TimeOut =0;
					PausaCompressore.Enable =1;
					EngineBox[0].StatoCompressore = CHECK_TIME_NOFLUX;	// vado allo step successivo
				}
#endif
				break;

				case CHECK_TIME_NOFLUX:    //21:
					if(!EngineBox[0].In.Flow && (PausaCompressore.TimeOut==1))	// controllo di non avere flusso
					//if(!EngineBox[0].In.Flow)										// controllo di non avere flusso e che il timer sia scaduto
					{	
                        //DEBUG_PMP EngineBox[0].Out.Pompa_Acqua = 1;						// faccio partire la pompa acqua
						EngineBox[0].PumpON_FluxTestFlag = 1;
						//EngineBox[0].Out_Pompa = Pump_Max_Speed;				// al massimo
#if(K_AbilPumpInverter==0 && K_AbilPumpDAC==0)
						PausaCompressore.Time = 7;	//15				// imposto il timer per capire se si chiude il flussostato entro 20 secondi
#else
						PausaCompressore.Time = 15;	//20				// imposto il timer per capire se si chiude il flussostato entro 20 secondi
#endif                        
						PausaCompressore.Value = 0;
						PausaCompressore.TimeOut =0;
						PausaCompressore.Enable =1;
						EngineBox[0].StatoCompressore = CHECK_TIME_YESFLUX;		// vado allo step successivo
					}
					else if(EngineBox[0].In.Flow && (PausaCompressore.TimeOut==1))	// se scade il timeout e ho ancora flusso
					{
						EngineBox[0].PersErr.WaterSeaFlux =1;					// attivo allarme flussostato
                        EngineBox[0].CntError.FluxTest =0;						//Resetto test di flusso
#if(K_AbilPumpInverter==0 && K_AbilPumpDAC==0)
						PausaCompressore.Time = 20;	//20				// imposto il timer per capire se si chiude il flussostato entro 20 secondi
#else
						PausaCompressore.Time = 20;	//20				// imposto il timer per capire se si chiude il flussostato entro 20 secondi
#endif                        
						PausaCompressore.Value = 0;
						PausaCompressore.Enable =1;
						EngineBox[0].StatoCompressore = EXIT_FLUX_TEST;			// Esco dal test di flusso
					}
				break;

			case CHECK_TIME_YESFLUX: //22:
				//if(EngineBox[0].In.Flow && (PausaCompressore.TimeOut==1))			// se scade timeout e ho flusso
				//if(EngineBox[0].In.Flow)											// se ho flusso
                if(PausaCompressore.TimeOut==1)			// se scade timeout
				{	
                    //DEBUG_PMP EngineBox[0].Out.Pompa_Acqua = 0;				// spengo la pompa acqua
					//EngineBox[0].Out_Pompa = Pump_Off_Speed;		// e la porto al minimo
					PausaCompressore.Time =0;						// resetto il timer 
					PausaCompressore.Value = 0;
					PausaCompressore.TimeOut =0;
					PausaCompressore.Enable =0;
					EngineBox[0].PumpON_FluxTestFlag = 0;
					EngineBox[0].CntError.FluxTest +=1;				// e incremento di uno il contatore cicli
					//if(EngineBox[0].CntError.FluxTest >= CicliTestFlussoAcqua)	//Se ho superato i cicli
                    if(EngineBox[0].In.Flow)	//Se ho ho flusso
					{	
                        EngineBox[0].CntError.FluxTest =0;						//Resetto test di flusso
						if(EngineBox[0].PersErr.CondensatoreHi)					//Resetto gli errori
						{	EngineBox[0].CntError.CondensatoreHi =0;
							EngineBox[0].Error.CondensatoreHi = 0;
							EngineBox[0].PersErr.CondensatoreHi = 0;
						}
                        /*
						if(EngineBox[0].PersErr.CondensatoreLo)
						{	EngineBox[0].CntError.CondensatoreLo =0;
							EngineBox[0].Error.CondensatoreLo = 0;
							EngineBox[0].PersErr.CondensatoreLo = 0;
						}
                        */
#if(K_AbilPumpInverter==0 && K_AbilPumpDAC==0)
						PausaCompressore.Time = WaitTimeRestart;//15;	//20				// imposto il timer per capire se si chiude il flussostato entro 20 secondi
#else
						PausaCompressore.Time = WaitTimeRestart;	//20				// Argo DaModificare????   WaitTimeRestart imposto il timer per capire se si chiude il flussostato entro 20 secondi
#endif                        
						PausaCompressore.Value = 0;
						PausaCompressore.Enable =1;
						EngineBox[0].StatoCompressore = EXIT_FLUX_TEST;			// Esco dal test di flusso
					}
					else
						EngineBox[0].StatoCompressore = START_FLUX_TEST;		// altrimenti riparto con il ciclo
				}
				//else if(!EngineBox[0].In.Flow && (PausaCompressore.TimeOut==1))	// se scade il timeout e non ho ancora flusso
                if(!EngineBox[0].In.Flow && (EngineBox[0].CntError.FluxTest >= CicliTestFlussoAcqua))	// se scade il timeout e non ho ancora flusso
				{    
#if(K_Abil_Double_Pump==1)
					if((EngineBox[0].PersErr.Pump1Fault==0) && EngineBox[0].Out.Pompa_Acqua)
					{
						EngineBox[0].PersErr.Pump1Fault=1;
						if(((EngineBox[0].PersErr.Pump2Fault==1) && (EngineBox[0].SwitchPumpSelection==PumpSelection_Auto)) || (EngineBox[0].SwitchPumpSelection==PumpSelection_1))
							EngineBox[0].PersErr.WaterSeaFlux =1;
#if(K_AbilPumpInverter==0 && K_AbilPumpDAC==0)
						PausaCompressore.Time = 15;	//20				// imposto il timer per capire se si chiude il flussostato entro 20 secondi
#else
						PausaCompressore.Time = 15;	//20				// imposto il timer per capire se si chiude il flussostato entro 20 secondi
#endif                        
						PausaCompressore.Value = 0;
						PausaCompressore.Enable =1;
                        EngineBox[0].CntError.FluxTest =0;						//Resetto test di flusso
						EngineBox[0].StatoCompressore = EXIT_FLUX_TEST;					// Esco dal test di flusso
					}
					else if(EngineBox[0].PersErr.Pump2Fault==0&&EngineBox[0].Out.Pompa_Acqua2)
					{
						EngineBox[0].PersErr.Pump2Fault=1;
						if(((EngineBox[0].PersErr.Pump1Fault==1) && (EngineBox[0].SwitchPumpSelection==PumpSelection_Auto)) || (EngineBox[0].SwitchPumpSelection==PumpSelection_2))
							EngineBox[0].PersErr.WaterSeaFlux =1;							// attivo allarme flussostato				
#if(K_AbilPumpInverter==0 && K_AbilPumpDAC==0)
						PausaCompressore.Time = 15;	//20				// imposto il timer per capire se si chiude il flussostato entro 20 secondi
#else
						PausaCompressore.Time = 15;	//20				// imposto il timer per capire se si chiude il flussostato entro 20 secondi
#endif                        
						PausaCompressore.Value = 0;
						PausaCompressore.Enable =1;
                        EngineBox[0].CntError.FluxTest =0;						//Resetto test di flusso
						EngineBox[0].StatoCompressore = EXIT_FLUX_TEST;					// Esco dal test di flusso
					}
#else
					EngineBox[0].PersErr.Pump1Fault=1;
					EngineBox[0].PersErr.WaterSeaFlux =1;
#if(K_AbilPumpInverter==0 && K_AbilPumpDAC==0)
						PausaCompressore.Time = 15;	//20				// imposto il timer per capire se si chiude il flussostato entro 20 secondi
#else
						PausaCompressore.Time = 15;	//20				// imposto il timer per capire se si chiude il flussostato entro 20 secondi
#endif                        
					PausaCompressore.Value = 0;
					PausaCompressore.Enable =1;				
                    EngineBox[0].CntError.FluxTest =0;						//Resetto test di flusso
					EngineBox[0].StatoCompressore = EXIT_FLUX_TEST;					// Esco dal test di flusso
#endif               
//					EngineBox[0].PersErr.WaterSeaFlux =1;							// attivo allarme flussostato				
//					PausaCompressore.Time = 15;	//20				// imposto il timer per capire se si chiude il flussostato entro 20 secondi
//					PausaCompressore.Value = 0;
//					PausaCompressore.Enable =1;					
//					EngineBox[0].StatoCompressore = EXIT_FLUX_TEST;					// Esco dal test di flusso
				}
				break;

			case EXIT_FLUX_TEST:    //23:
				//EngineBox[0].Request_Out_Inverter = Min_Speed;
				//if(EngineBox[0].Out_Inverter <= Min_Speed)			// nel caso in cui sono già al minimo
					EngineBox[0].Out_Inverter = Off_Speed;			// spengo il compressore
#if (K_AbilCompressor2==1)
					EngineBox[1].Out_Inverter = Off_Speed;			// spengo il compressore
#endif
					//DEBUG_PMP EngineBox[0].Out.Pompa_Acqua = 0;				// spengo la pompa acqua
					EngineBox[0].PumpON_FluxTestFlag = 0;
					//EngineBox[0].Out_Pompa = Pump_Off_Speed;		// e la porto al minimo
				
					EngineBox[0].CntError.FluxTest =0;				// resetto il contatore dei cicli test di flusso
					if(PausaCompressore.TimeOut==1)
					{
						PausaCompressore.Time =0;						// resetto il timer 
						PausaCompressore.Value = 0;
						PausaCompressore.TimeOut =0;
						PausaCompressore.Enable =0;
						EngineBox[0].Test.FluxReq =0;					// finisco con il test di flusso
						EngineBox[0].StatoCompressore = START_SM;
						
					}
				
				break;
		}
	}
}

void Error_EEV_Test(void)
{
    //------------------------------------------------------------
	//	gestione test EEV
	//------------------------------------------------------------
	if(EngineBox[0].Test.EEV_Req &(!EngineBox[0].Test.FluxReq | 				// se il test di flusso non è iniziato
		(EngineBox[0].Test.FluxReq & ((EngineBox[0].StatoCompressore < START_FLUX_TEST)|(EngineBox[0].StatoCompressore > START_EEV_TEST)))))
	{
		if(EngineBox[0].StatoCompressore < START_EEV_TEST)
			EngineBox[0].StatoCompressore = START_EEV_TEST;
	
		switch(EngineBox[0].StatoCompressore)
		{
			case START_EEV_TEST:    //30:
				EngineBox[0].Request_Out_Inverter = Min_Speed;		// imposto la richiesta del compressore al minimo
				if(EngineBox[0].Out_Inverter <= Min_Speed)			// nel caso in cui sono già al minimo
				{	
                    EngineBox[0].Out_Inverter = Off_Speed;			// spengo il compressore
					//DEBUG_PMP EngineBox[0].Out.Pompa_Acqua = 1;
					EngineBox[0].PumpON_EEVTestFlag = 1;
					PausaCompressore.Time = 180;					// imposto il timer per bloccare il compressore
					PausaCompressore.Value = 0;						// per 10 secondi prima di potersi riattivare
					PausaCompressore.TimeOut =0;
					PausaCompressore.Enable =1;
					EngineBox[0].StatoCompressore = MAKE_EEV_TEST;
				}
				break;

			case MAKE_EEV_TEST:      //31:
				if(PausaCompressore.TimeOut)
				{	
                    PausaCompressore.Value = 0;
					PausaCompressore.TimeOut =0;
					PausaCompressore.Enable =0;
					//DEBUG_VV	EngineBox[0].Out.Valvola_Caldo = 0;
					//DEBUG_PMP EngineBox[0].Out.Pompa_Acqua = 1;
					EngineBox[0].PumpON_EEVTestFlag = 1;
					EngineBox[0].Out_Inverter = Min_Speed;
					EngineBox[0].Request_Out_Inverter = CompressorSpeedEEV_Test;
					EngineBox[0].StatoCompressore = CHECK_SPEED_READY_EEV_TEST;
				}
				break;
		
			case CHECK_SPEED_READY_EEV_TEST: //32:
				if(EngineBox[0].Out_Inverter >= CompressorSpeedEEV_Test)
				{	
                    EngineBox[0].Test.EEV_Go = 1;
					PausaCompressore.Time = TimeEEV_Test;
					PausaCompressore.Value = 0;
					PausaCompressore.TimeOut =0;
					PausaCompressore.Enable =1;
					EngineBox[0].StatoCompressore = CHECK_END_EEV_TEST;
				}
				break;
	
			case CHECK_END_EEV_TEST:    // 33:
				if(PausaCompressore.TimeOut || EngineBox[0].Test.EEV_End)
				{	
                    PausaCompressore.Value = 0;
					PausaCompressore.TimeOut =0;
					PausaCompressore.Enable =0;
					EngineBox[0].StatoCompressore = MAKE_END_EEV_TEST;
				}
				break;

			case MAKE_END_EEV_TEST: //34:
				EngineBox[0].Request_Out_Inverter = Min_Speed;		// imposto la richiesta del compressore al minimo
				if(EngineBox[0].Out_Inverter <= Min_Speed)			// nel caso in cui sono già al minimo
				{	
                    EngineBox[0].Out_Inverter = Off_Speed;			// spengo il compressore
					//DEBUG_PMP EngineBox[0].Out.Pompa_Acqua = 1;
					EngineBox[0].PumpON_EEVTestFlag = 1;
					PausaCompressore.Time = 10;						// imposto il timer per bloccare il compressore
					PausaCompressore.Value = 0;						// per 10 secondi prima di potersi riattivare
					PausaCompressore.TimeOut =0;
					PausaCompressore.Enable =1;
					EngineBox[0].StatoCompressore = CHECK_TIMEOUT_EEV_TEST;
				}
				break;
		
			case CHECK_TIMEOUT_EEV_TEST:    //35:
				if(PausaCompressore.TimeOut)
				{
					if(!EngineBox[0].PersErr.EEV_Open |EngineBox[0].PersErr.EEV_Close)
					{	
                        if((EngineBox[0].Pressure.Liquid < SogliaP_Small) | 
							(EngineBox[0].Temperature.Compressor_Output < SogliaT_MandataComp))
							EngineBox[0].PersErr.CaricaRefrig = 1;
					}
					PausaCompressore.Time = 180;					// imposto il timer per bloccare il compressore
					PausaCompressore.Value = 0;						// per 180 secondi prima di potersi riattivare
					PausaCompressore.TimeOut =0;
					PausaCompressore.Enable =1;
					EngineBox[0].StatoCompressore = EXIT_EEV_TEST;
				}
				break;

			case EXIT_EEV_TEST: //36:
				if(PausaCompressore.TimeOut)
				{	
                    EngineBox[0].Test.EEV_Req= 0;
					EngineBox[0].Test.EEV_Go = 0;
					EngineBox[0].StatoCompressore = START_SM;
				}
				break;
		}
	}
}

void Error_CoolWarm_Valve(void)
{
    int ErrorCooling;
    int ErrorHeating;
    
#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)
    ErrorCooling = (EngineBox[0].CoolingMode || EngineBox[1].CoolingMode) && ((EngineBox[0].Pressure.Gas>(EngineBox[0].Pressure.Liquid+1500)) || (EngineBox[0].Pressure.Gas>(EngineBox[0].Pressure.LiquidCond+1500)));
    ErrorHeating = (EngineBox[0].HeatingMode || EngineBox[1].HeatingMode) && ((EngineBox[0].Pressure.Gas<(EngineBox[0].Pressure.Liquid-1500)) || (EngineBox[0].Pressure.Gas<(EngineBox[0].Pressure.LiquidCond-1500)));
#else
    ErrorCooling = (EngineBox[0].CoolingMode || EngineBox[1].CoolingMode) && (EngineBox[0].Pressure.Gas>EngineBox[0].Pressure.Liquid+1500);
    ErrorHeating = (EngineBox[0].HeatingMode || EngineBox[1].HeatingMode) && (EngineBox[0].Pressure.Gas<EngineBox[0].Pressure.Liquid-1500);
#endif
    
    //if(ErrorCooling || ErrorHeating)
    if((ErrorCooling || ErrorHeating) && ((EngineBox[0].Out_Inverter>100) || (EngineBox[1].Out_Inverter>100)))    
        TimerCoolWarmValveError.Enable = 1;
     
    if((EngineBox[0].Out_Inverter==0) && (EngineBox[1].Out_Inverter==0))
        TimerCoolWarmValveError.Enable = 0;
       
    if(TimerCoolWarmValveError.Enable)
    {
#if(K_Condenser_Pressure_Abil==1 && K_Forced_PressioneHi_Liq_P_Liq_Pres==0)
        if(EngineBox[0].CoolingMode || EngineBox[1].CoolingMode)
        {            
            if((EngineBox[0].Pressure.Gas<EngineBox[0].Pressure.Liquid-2000) && (EngineBox[0].Pressure.Gas<EngineBox[0].Pressure.LiquidCond-2000))
                TimerCoolWarmValveError.Enable = 0;
        }
        if(EngineBox[0].HeatingMode || EngineBox[1].HeatingMode)
        {
            if((EngineBox[0].Pressure.Gas>EngineBox[0].Pressure.Liquid+2000) && (EngineBox[0].Pressure.Gas>EngineBox[0].Pressure.LiquidCond+2000))
                TimerCoolWarmValveError.Enable = 0;
        }
    }
#else
        if(EngineBox[0].CoolingMode || EngineBox[1].CoolingMode)
        {            
            if(EngineBox[0].Pressure.Gas<EngineBox[0].Pressure.Liquid-2000)
                TimerCoolWarmValveError.Enable = 0;
        }
        if(EngineBox[0].HeatingMode || EngineBox[1].HeatingMode)
        {
            if(EngineBox[0].Pressure.Gas>EngineBox[0].Pressure.Liquid+2000)
                TimerCoolWarmValveError.Enable = 0;
        }
    }    
#endif
    if(!TimerCoolWarmValveError.Enable)
    {
        TimerCoolWarmValveError.Value = 0;
        TimerCoolWarmValveError.TimeOut = 0;
    }
    
            
    if(TimerCoolWarmValveError.TimeOut)
    {
        EngineBox[0].PersErr.CoolWarm_Valve = 1;
        TimerCoolWarmValveError.Value = 0;
        TimerCoolWarmValveError.TimeOut = 0;        
		if(EngineBox[0].EngineIsOn)
			EngineBox[0].GoShutDwComp =1;											// spengo 
#if (K_AbilCompressor2==1)
		if(EngineBox[1].EngineIsOn)
			EngineBox[1].GoShutDwComp =1;
#endif	        
    }
}

void Error_SuperHeat(void)
{
#if (K_AbilCompressor2==1)
    TimerSuperHeatError.Enable = (EngineBox[0].SuperHeat.Value < Room[k_Split_Master_Add].SuperHeat_Heat_Min_SetPoint) && (EngineBox[0].InverterRiscaldamento || EngineBox[1].InverterRiscaldamento) && !EngineBox[0].PersErr.SuperHeat && EngineBox[0].TestAllAbil.SuperHeat_Heat_Correct;
#else
    TimerSuperHeatError.Enable = (EngineBox[0].SuperHeat.Value < Room[k_Split_Master_Add].SuperHeat_Heat_Min_SetPoint) && EngineBox[0].InverterRiscaldamento && !EngineBox[0].PersErr.SuperHeat && EngineBox[0].TestAllAbil.SuperHeat_Heat_Correct;
#endif
    
    if(!TimerSuperHeatError.Enable)
    {
        TimerSuperHeatError.Value = 0;
        TimerSuperHeatError.TimeOut = 0;
    }
    
    
    if((EngineBox[0].InverterRiscaldamento || EngineBox[1].InverterRiscaldamento) && EngineBox[0].TestAllAbil.SuperHeat_Heat_Correct)
        EngineBox[0].Error.SuperHeat = (EngineBox[0].SuperHeat.Value < Room[k_Split_Master_Add].SuperHeat_Heat_Max_SetPoint);
    else if((EngineBox[0].InverterRaffrescamento || EngineBox[1].InverterRaffrescamento) && EngineBox[0].TestAllAbil.SuperHeat_Cool_Correct)
        EngineBox[0].Error.SuperHeat = (EngineBox[0].SuperHeat.CoolingCalc < Room[k_Split_Master_Add].SuperHeat_Cool_Min_SetPoint);    
    else 
        EngineBox[0].Error.SuperHeat = 0;
                
    if(TimerSuperHeatError.TimeOut)
    {
        EngineBox[0].PersErr.SuperHeat = 1;
        TimerSuperHeatError.Value = 0;
        TimerSuperHeatError.TimeOut = 0;        
		if(EngineBox[0].EngineIsOn)
			EngineBox[0].GoShutDwComp =1;											// spengo 
#if (K_AbilCompressor2==1)
		if(EngineBox[1].EngineIsOn)
			EngineBox[1].GoShutDwComp =1;
#endif	        
    }    
}

void Error_TemperatureProbe(void)
{
    //ARGOPROBE
    /*
    static int check1=0;
    static int check2=0;
    static int check3=0;
    static int check4=0;
    static int check5=0;
    static int check6=0;

    static int Trig1=0;
    static int Trig2=0;
    static int Trig3=0;
    static int Trig4=0;
    static int Trig5=0;
    static int Trig6=0;
     */
    //ARGOPROBE
     
    if(TimerCheckTemperatureProbe.TimeOut)
    {
        EngineBox[0].Temperature.Freeze.Compressor_Output = CheckTempProbeFreeze(EngineBox[0].Temperature.Compressor_Output, &EngineBox[0].Temperature.PreviusVal.Compressor_Output, &EngineBox[0].Temperature.CntError.Compressor_Output, K_MaxCntTempElapsedTimeError_Compressor_Output);  
        EngineBox[0].Temperature.Freeze.Compressor_Suction = CheckTempProbeFreeze(EngineBox[0].Temperature.Compressor_Suction, &EngineBox[0].Temperature.PreviusVal.Compressor_Suction, &EngineBox[0].Temperature.CntError.Compressor_Suction, K_MaxCntTempElapsedTimeError_Compressor_Suction);
        EngineBox[0].Temperature.Freeze.Condenser = CheckTempProbeFreeze(EngineBox[0].Temperature.Condenser, &EngineBox[0].Temperature.PreviusVal.Condenser, &EngineBox[0].Temperature.CntError.Condenser, K_MaxCntTempElapsedTimeError_Condenser);
        EngineBox[0].Temperature.Freeze.Sea_Water = CheckTempProbeFreeze(EngineBox[0].Temperature.Sea_Water, &EngineBox[0].Temperature.PreviusVal.Sea_Water, &EngineBox[0].Temperature.CntError.Sea_Water, K_MaxCntTempElapsedTimeError_Sea_Water);
        EngineBox[0].Temperature.Freeze.Liquid = CheckTempProbeFreeze(EngineBox[0].Temperature.Liquid, &EngineBox[0].Temperature.PreviusVal.Liquid, &EngineBox[0].Temperature.CntError.Liquid, K_MaxCntTempElapsedTimeError_Liquid);
        EngineBox[1].Temperature.Freeze.Compressor_Output = CheckTempProbeFreeze(EngineBox[1].Temperature.Compressor_Output, &EngineBox[1].Temperature.PreviusVal.Compressor_Output, &EngineBox[1].Temperature.CntError.Compressor_Output, K_MaxCntTempElapsedTimeError_Compressor2_Output);
        TimerCheckTemperatureProbe.Value = 0;
        TimerCheckTemperatureProbe.TimeOut = 0;
    }
    
    EngineBox[0].Temperature.Error.Bit.Compressor_Output = (EngineBox[0].Temperature.Compressor_Output <= K_Compressor_Output_Probe_Lo) || ((EngineBox[0].Temperature.Compressor_Output >= K_Compressor_Output_Probe_Hi) && (EngineBox[0].Out_Inverter==Off_Speed)) || (EngineBox[0].Temperature.Freeze.Compressor_Output && EngineBox[0].TestAllAbil.Master_Probe_Error);
    EngineBox[0].Temperature.Error.Bit.Compressor_Suction = ((EngineBox[0].Temperature.Compressor_Suction<=K_Compressor_Suction_Probe_Lo)*(!EngineBox[0].SplitFrigo_On && !EngineBox[0].SplitFreezer_On)) || (EngineBox[0].Temperature.Compressor_Suction>=K_Compressor_Suction_Probe_Hi) || (EngineBox[0].Temperature.Freeze.Compressor_Suction && EngineBox[0].TestAllAbil.Master_Probe_Error);
    EngineBox[0].Temperature.Error.Bit.Condenser= (EngineBox[0].Temperature.Condenser<=K_Condenser_Probe_Lo) || (EngineBox[0].Temperature.Condenser>=K_Condenser_Probe_Hi) || (EngineBox[0].Temperature.Freeze.Condenser && EngineBox[0].TestAllAbil.Master_Probe_Error);
    EngineBox[0].Temperature.Error.Bit.Sea_Water = (EngineBox[0].Temperature.Sea_Water<=K_Sea_Water_Probe_Lo) || (EngineBox[0].Temperature.Sea_Water>=K_Sea_Water_Probe_Hi) || (EngineBox[0].Temperature.Freeze.Sea_Water && EngineBox[0].TestAllAbil.Master_Probe_Error);
    EngineBox[0].Temperature.Error.Bit.Liquid = ((EngineBox[0].Temperature.Liquid<=K_Liquid_Probe_Lo)*(!EngineBox[0].SplitFrigo_On && !EngineBox[0].SplitFreezer_On)) || (EngineBox[0].Temperature.Liquid>=K_Liquid_Probe_Hi) || (EngineBox[0].Temperature.Freeze.Liquid && EngineBox[0].TestAllAbil.Master_Probe_Error);
                
    TimerTemperatureProbeErrorBox1.Enable = (EngineBox[0].Temperature.Error.Bit.Compressor_Output || EngineBox[0].Temperature.Error.Bit.Compressor_Suction || EngineBox[0].Temperature.Error.Bit.Condenser || EngineBox[0].Temperature.Error.Bit.Sea_Water || EngineBox[0].Temperature.Error.Bit.Liquid) && !EngineBox[0].Error.TemperatureProbe;

    if(!TimerTemperatureProbeErrorBox1.Enable)
    {
        TimerTemperatureProbeErrorBox1.Value = 0;
        TimerTemperatureProbeErrorBox1.TimeOut = 0;
    }
    
    if(TimerTemperatureProbeErrorBox1.TimeOut)
    {
        TimerTemperatureProbeErrorBox1.Value = 0;
        TimerTemperatureProbeErrorBox1.TimeOut = 0;
        EngineBox[0].Error.TemperatureProbe = 1;
    }

    if((EngineBox[0].Temperature.Error.Bit.Compressor_Output || EngineBox[0].Temperature.Error.Bit.Compressor_Suction) && EngineBox[0].EngineIsOn && EngineBox[0].TestAllAbil.Master_Probe_Error)
        EngineBox[0].GoShutDwComp =1;											// spengo 
    
    if(!EngineBox[0].Temperature.Error.Bit.Compressor_Output && !EngineBox[0].Temperature.Error.Bit.Compressor_Suction && !EngineBox[0].Temperature.Error.Bit.Condenser && !EngineBox[0].Temperature.Error.Bit.Sea_Water && !EngineBox[0].Temperature.Error.Bit.Liquid)
    {
        EngineBox[0].Error.TemperatureProbe = 0;        
    }
#if (K_AbilCompressor2==1)
    
    EngineBox[1].Temperature.Error.Bit.Compressor_Output = (EngineBox[1].Temperature.Compressor_Output <= K_Compressor_Output_Probe_Lo) || ((EngineBox[1].Temperature.Compressor_Output >= K_Compressor_Output_Probe_Hi) && (EngineBox[1].Out_Inverter==Off_Speed)) || (EngineBox[1].Temperature.Freeze.Compressor_Output && EngineBox[0].TestAllAbil.Master_Probe_Error);
    EngineBox[0].Temperature.Error.Bit.Compressor_Output2 = EngineBox[1].Temperature.Error.Bit.Compressor_Output; //Travaso per compattazione errori
    
    TimerTemperatureProbeErrorBox2.Enable = (EngineBox[1].Temperature.Error.Bit.Compressor_Output || EngineBox[0].Temperature.Error.Bit.Compressor_Suction || EngineBox[0].Temperature.Error.Bit.Condenser || EngineBox[0].Temperature.Error.Bit.Sea_Water || EngineBox[0].Temperature.Error.Bit.Liquid) && !EngineBox[1].Error.TemperatureProbe;

    if(!TimerTemperatureProbeErrorBox2.Enable)
    {
        TimerTemperatureProbeErrorBox2.Value = 0;
        TimerTemperatureProbeErrorBox2.TimeOut = 0;
    }

    if(TimerTemperatureProbeErrorBox2.TimeOut)
    {
        TimerTemperatureProbeErrorBox2.Value = 0;
        TimerTemperatureProbeErrorBox2.TimeOut = 0;
        EngineBox[1].Error.TemperatureProbe = 1;
    }

    if((EngineBox[1].Temperature.Error.Bit.Compressor_Output || EngineBox[0].Temperature.Error.Bit.Compressor_Suction) && EngineBox[1].EngineIsOn && EngineBox[0].TestAllAbil.Master_Probe_Error)
        EngineBox[1].GoShutDwComp =1;
 
    if(!EngineBox[1].Temperature.Error.Bit.Compressor_Output && !EngineBox[0].Temperature.Error.Bit.Compressor_Suction && !EngineBox[0].Temperature.Error.Bit.Condenser && !EngineBox[0].Temperature.Error.Bit.Sea_Water && !EngineBox[0].Temperature.Error.Bit.Liquid)
    {
        EngineBox[1].Error.TemperatureProbe = 0;        
    }
#endif	                
}

void Error_PressureProbe(void)
{     
    if(TimerCheckPressureProbe.TimeOut)
    {
        EngineBox[0].Pressure.Freeze.Gas = CheckPressProbeFreeze(EngineBox[0].Pressure.Gas, &EngineBox[0].Pressure.PreviusVal.Gas, &EngineBox[0].Pressure.CntError.Gas, K_MaxCntTempElapsedTimeError_Pressure_Gas);  
        EngineBox[0].Pressure.Freeze.Liquid = CheckPressProbeFreeze(EngineBox[0].Pressure.Liquid, &EngineBox[0].Pressure.PreviusVal.Liquid, &EngineBox[0].Pressure.CntError.Liquid, K_MaxCntTempElapsedTimeError_Pressure_Liquid);
        EngineBox[0].Pressure.Freeze.LiquidCond = CheckPressProbeFreeze(EngineBox[0].Pressure.LiquidCond, &EngineBox[0].Pressure.PreviusVal.LiquidCond, &EngineBox[0].Pressure.CntError.LiquidCond, K_MaxCntTempElapsedTimeError_Pressure_LiquidCond);
        TimerCheckPressureProbe.Value = 0;
        TimerCheckPressureProbe.TimeOut = 0;
    }
    
    EngineBox[0].Pressure.Error.Bit.Gas = ((EngineBox[0].Pressure.Gas <= K_Pressure_Gas_Probe_Lo)&&((EngineBox[0].Pressure.Liquid>1000) || (EngineBox[0].Pressure.LiquidCond>1000))) || ((EngineBox[0].Pressure.Gas >= K_Pressure_Gas_Probe_Hi) && ((EngineBox[0].Out_Inverter==Off_Speed) || (EngineBox[0].DefMode != EngineBox_Caldo))) || (EngineBox[0].Pressure.Freeze.Gas && EngineBox[0].TestAllAbil.Master_Probe_Error);
    EngineBox[0].Pressure.Error.Bit.Liquid = ((EngineBox[0].Pressure.Liquid <= K_Pressure_Liquid_Probe_Lo)&&((EngineBox[0].Pressure.Gas>1000) || (EngineBox[0].Pressure.LiquidCond>1000))) || ((EngineBox[0].Pressure.Liquid>=K_Pressure_Liquid_Probe_Hi) && ((EngineBox[0].Out_Inverter==Off_Speed) || (EngineBox[0].DefMode != EngineBox_Freddo))) || (EngineBox[0].Pressure.Freeze.Liquid && EngineBox[0].TestAllAbil.Master_Probe_Error);
#if(K_Condenser_Pressure_Abil==1)
    EngineBox[0].Pressure.Error.Bit.LiquidCond = ((EngineBox[0].Pressure.LiquidCond<=K_Pressure_LiquidCond_Probe_Lo)&&((EngineBox[0].Pressure.Gas>1000) || (EngineBox[0].Pressure.Liquid>1000))) || ((EngineBox[0].Pressure.LiquidCond>=K_Pressure_LiquidCond_Probe_Hi) && ((EngineBox[0].Out_Inverter==Off_Speed) || (EngineBox[0].DefMode != EngineBox_Freddo))) || (EngineBox[0].Pressure.Freeze.LiquidCond && EngineBox[0].TestAllAbil.Master_Probe_Error);
#else
    EngineBox[0].Pressure.Error.Bit.LiquidCond = 0;
#endif
    
    TimerPressureProbeErrorBox.Enable = (EngineBox[0].Pressure.Error.Bit.Gas || EngineBox[0].Pressure.Error.Bit.Liquid || EngineBox[0].Pressure.Error.Bit.LiquidCond) && !EngineBox[0].Error.Pressure_Probe;

    if(!TimerPressureProbeErrorBox.Enable)
    {
        TimerPressureProbeErrorBox.Value = 0;
        TimerPressureProbeErrorBox.TimeOut = 0;
    }
    
    if(TimerPressureProbeErrorBox.TimeOut)
    {
        TimerPressureProbeErrorBox.Value = 0;
        TimerPressureProbeErrorBox.TimeOut = 0;
        EngineBox[0].Error.Pressure_Probe = 1;
    }

    if((EngineBox[0].Pressure.Error.Bit.Gas || EngineBox[0].Pressure.Error.Bit.Liquid || EngineBox[0].Pressure.Error.Bit.LiquidCond) && EngineBox[0].EngineIsOn && EngineBox[0].TestAllAbil.Master_Probe_Error)
        EngineBox[0].GoShutDwComp =1;											// spengo 

#if (K_AbilCompressor2==1)
    if((EngineBox[0].Pressure.Error.Bit.Gas || EngineBox[0].Pressure.Error.Bit.Liquid || EngineBox[0].Pressure.Error.Bit.LiquidCond) && EngineBox[1].EngineIsOn && EngineBox[0].TestAllAbil.Master_Probe_Error)
        EngineBox[1].GoShutDwComp =1;											// spengo 
#endif    
    if(!EngineBox[0].Pressure.Error.Bit.Gas && !EngineBox[0].Pressure.Error.Bit.Liquid && !EngineBox[0].Pressure.Error.Bit.LiquidCond)
    {
        EngineBox[0].Error.Pressure_Probe = 0;        
    }
}

void Error_Reset(void)
{
	//Reset da touch
    
    if(EngineBox[0].In.ClearComErrorReq==1)
    {
        EngineBox[0].ComError.CntComErr=0;			
        EngineBox[0].ComError.TimeOutComErr=0;
        EngineBox[0].ComError.CRC_ComErr=0;
        EngineBox[0].ComError.ModCntComErr=0;			
        EngineBox[0].ComError.ModTimeOutComErr=0;		
        EngineBox[0].ComError.ModCRC_ComErr=0;
        EngineBox[0].ComError.ModCntComErr_U2=0;			
        EngineBox[0].ComError.ModTimeOutComErr_U2=0;		
        EngineBox[0].ComError.ModCRC_ComErr_U2=0;
        EngineBox[0].ComError.CntComErr_U3=0;
        EngineBox[0].ComError.TimeOutComErr_U3=0;
        EngineBox[0].ComError.CRC_ComErr_U3=0;                              
    }    
    
	if(EngineBox[0].In.ClearErrorReq)
	{	
		if(EngineBox[0].PersErr.CompressorHi)
			EngineBox[0].CntError.CompressorHi = 0;
		if(EngineBox[0].PersErr.CompressorLo)
			EngineBox[0].CntError.CompressorLo = 0;
		if(EngineBox[0].PersErr.CondensatoreHi)
			EngineBox[0].CntError.CondensatoreHi = 0;
		if(EngineBox[0].PersErr.CondensatoreLo)
			EngineBox[0].CntError.CondensatoreLo = 0;
		//EngineBox[0].Error.CondensatoreLo = 0;
		//if(EngineBox[0].PersErr.Gas_Recovery)
		//	EngineBox[0].CntError.GasRecovery = 0;
		//EngineBox[0].Error.CompressorHi = 0;
		//EngineBox[0].Error.CompressorLo = 0;
		//EngineBox[0].Error.CondensatoreHi = 0;
		//EngineBox[0].Error.Gas_Recovery = 0;
		EngineBox[0].PersErr.CompressorHi = 0;
		EngineBox[0].PersErr.CompressorLo = 0;
		EngineBox[0].PersErr.CondensatoreHi = 0;
		EngineBox[0].PersErr.CondensatoreLo = 0;
		//EngineBox[0].PersErr.Gas_Recovery = 0;
		EngineBox[0].PersErr.FloodSensor = 0;
		EngineBox[0].PersErr.ThermicComp = 0;
		EngineBox[0].PersErr.SeaWaterLo = 0;
		EngineBox[0].PersErr.SeaWaterHi = 0;
		EngineBox[0].PersErr.CaricaRefrig = 0;
		EngineBox[0].PersErr.WaterSeaFlux = 0;
		EngineBox[0].PersErr.EEV_Open = 0;
		EngineBox[0].PersErr.EEV_Close = 0;
		EngineBox[0].PersErr.BatteriaGhiacciata = 0;
		EngineBox[0].PersErr.Fan_Speed = 0;
		EngineBox[0].PersErr.Fan_OverLoad = 0;
        EngineBox[0].PersErr.Acceleration_Error = 0;
		EngineBox[0].PersErr.LowOilLevel = 0;
		EngineBox[0].PersErr.Pump1Fault = 0;
		EngineBox[0].PersErr.Pump2Fault = 0;  
        EngineBox[0].Error.CondenserFouled = 0;
        EngineBox[0].PersErr.CoolWarm_Valve = 0;
        EngineBox[0].PersErr.SuperHeat = 0;
        EngineBox[0].MaxErrorPersComp = 0;
        EngineBox[0].MaxErrorPersPump = 0;

#if (K_AbilCompressor2==1)
		if(EngineBox[1].PersErr.CompressorHi)
			EngineBox[1].CntError.CompressorHi = 0;
        EngineBox[1].PersErr.Acceleration_Error = 0;
		EngineBox[1].PersErr.CompressorHi;
		EngineBox[1].PersErr.ThermicComp = 0;
		EngineBox[1].PersErr.LowOilLevel = 0;
        EngineBox[1].MaxErrorPersComp = 0;
#if(K_OnlyOneInverterComp2==1)        
        if(EngineBox[0].PersErr.CompInverterFault && EngineBox[1].SelectCompressor==1)
            EngineBox[0].PersErr.CompInverterFault = 0;
        if(EngineBox[1].PersErr.CompInverterFault && EngineBox[0].SelectCompressor==1)
            EngineBox[1].PersErr.CompInverterFault = 0;
#endif
#endif

    if(EngineBox[0].Temperature.Error.Bit.Compressor_Output)
        EngineBox[0].Temperature.CntError.Compressor_Output = 0;
    if(EngineBox[0].Temperature.Error.Bit.Compressor_Suction)
        EngineBox[0].Temperature.CntError.Compressor_Suction = 0;
    if(EngineBox[0].Temperature.Error.Bit.Condenser)
        EngineBox[0].Temperature.CntError.Condenser = 0;
    if(EngineBox[0].Temperature.Error.Bit.Sea_Water)
        EngineBox[0].Temperature.CntError.Sea_Water = 0;
    if(EngineBox[0].Temperature.Error.Bit.Liquid)
        EngineBox[0].Temperature.CntError.Liquid = 0;
    if(EngineBox[1].Temperature.Error.Bit.Compressor_Output)
        EngineBox[1].Temperature.CntError.Compressor_Output = 0;

    if(EngineBox[0].Pressure.Error.Bit.Gas)
        EngineBox[0].Pressure.CntError.Gas = 0;
    if(EngineBox[0].Pressure.Error.Bit.Liquid)
        EngineBox[0].Pressure.CntError.Liquid = 0;
    if(EngineBox[0].Pressure.Error.Bit.LiquidCond)
        EngineBox[0].Pressure.CntError.LiquidCond = 0;
	}
}

void Error_Compact(void)
{
	//------------------------------------------------------------
	// Compattazione errori
	//------------------------------------------------------------
	EngineBox[0].Errori1 = 0;
	EngineBox[0].Errori1 = 
		(EngineBox[0].Error.CompressorHi	<< 0) |
		(EngineBox[0].Error.CompressorLo	<< 1) |
		(EngineBox[0].Error.CondensatoreHi	<< 2) |
		(EngineBox[0].Error.CondensatoreLo	<< 3) |
        (EngineBox[0].Error.CondenserFouled	<< 4) | //(EngineBox[0].Error.Gas_Recovery	<< 4) |
		(EngineBox[0].Test.FluxReq			<< 5) |
		(EngineBox[0].Test.EEV_Req			<< 6) |
		(EngineBox[0].Error.SuperHeat		<< 7) | //(EngineBox[0].Test.EEV_Go			<< 7) |
		(EngineBox[0].Error.Pressure_Lo		<< 8) |
		(EngineBox[0].Error.Pressure_Hi		<< 9) |
		(EngineBox[0].Error.Recharge_Oil	<< 10)|
		(EngineBox[1].Error.CompressorHi	<< 11)|
#if(K_AbilRechargeOil==1)
		(EngineBox[0].In.LowOilLevel		<< 12)|
	#if(K_AbilCompressor2==1)
		(EngineBox[1].In.LowOilLevel		<< 13)|
	#endif
#endif
		(EngineBox[0].Error.Defrosting      << 14) |
		(EngineBox[0].Error.MasterRestart	<< 15);

	EngineBox[0].Errori2 = 0;
	EngineBox[0].Errori2 = 
		((EngineBox[0].Error.TemperatureProbe || EngineBox[1].Error.TemperatureProbe) << 0) |
         (EngineBox[0].Error.Pressure_Probe                                           << 1) | 
        ((Comp_Inverter[0].Ok_Voltage==0)                                             << 2) |
        ((Comp_Inverter[1].Ok_Voltage==0)                                             << 3) | 
        ((EngineBox[0].PersErr.Fan_Speed==1)                                          << 4);
	
	EngineBox[0].ErroriPers1 = 0;
	EngineBox[0].ErroriPers1 = 
		(EngineBox[0].PersErr.CompressorHi		<<  0) |
		(EngineBox[0].PersErr.CompressorLo		<<  1) |
		(EngineBox[0].PersErr.CondensatoreHi	<<  2) |
		(EngineBox[0].PersErr.CondensatoreLo	<<  3) |
		//(EngineBox[0].PersErr.Gas_Recovery		<<  4) |
		(EngineBox[0].PersErr.FloodSensor		<<  5) |
		(EngineBox[0].PersErr.ThermicComp		<<  6) |
		(EngineBox[0].PersErr.SeaWaterLo		<<  7) |
		(EngineBox[0].PersErr.SeaWaterHi		<<  8) |
		(EngineBox[0].PersErr.CaricaRefrig		<<  9) |
		(EngineBox[0].PersErr.WaterSeaFlux		<< 10) |
		(EngineBox[0].PersErr.SuperHeat			<< 11) | //(EngineBox[0].PersErr.EEV_Open			<< 11) |
		(EngineBox[0].PersErr.FireAlarm			<< 12) ;//(EngineBox[0].PersErr.EEV_Close			<< 12) |
		//(EngineBox[0].PersErr.BatteriaGhiacciata<< 13) |
		//(EngineBox[0].PersErr.Fan_Speed			<< 14) |
		//(EngineBox[0].PersErr.Fan_OverLoad		<< 15);

	EngineBox[0].ErroriPers2 = 0;
	EngineBox[0].ErroriPers2 = 
		(EngineBox[0].PersErr.Acceleration_Error <<  0) |
		(EngineBox[1].PersErr.Acceleration_Error <<  1) |
		(EngineBox[1].PersErr.CompressorHi		 <<  2) |
		(EngineBox[1].PersErr.ThermicComp		 <<  3)	| 
		(EngineBox[0].PersErr.LowOilLevel		 <<  4)	|
		(EngineBox[1].PersErr.LowOilLevel		 <<  5) |
		(EngineBox[0].PersErr.CompInverterFault	 <<  6)	|
		(EngineBox[1].PersErr.CompInverterFault	 <<  7) |
		(EngineBox[0].PersErr.PumpInverterFault	 <<  8) | 
		(EngineBox[0].PersErr.CompMOff			 <<  9) |	// Allarmo "MOFF" Anomalo su Inverter 1
		(EngineBox[1].PersErr.CompMOff			 <<  10)|	// Allarmo "MOFF" Anomalo su Inverter 2
		(EngineBox[0].PersErr.PumpMOff			 <<  11)|	// Allarmo "MOFF" Anomalo su Inverter Pompa 1
		(EngineBox[0].PersErr.Pump1Fault         <<  12)|
		(EngineBox[0].PersErr.Pump2Fault         <<  13)|
        (EngineBox[0].PersErr.CoolWarm_Valve     <<  14)|
        (RTC_Flag                                <<  15);

	//EngineBox[0].CumErr = (EngineBox[0].Errori1 != 0) | ((EngineBox[0].Errori1 & 0x02FF) != 0);
    /*
	EngineBox[0].CumErr = 0;
	if(EngineBox[0].Errori1 != 0)
		EngineBox[0].CumErr = 1;
	else if((EngineBox[0].ErroriPers1 & 0x02FF) != 0)
		EngineBox[0].CumErr = 1;
	else if(EngineBox[0].ErroriPers2 != 0)
		EngineBox[0].CumErr = 1;
    */
	// gestione rele di segnalazione
	//EngineBox[0].Out.PersErr = ((EngineBox[0].ErroriPers1 & 0x02FF) != 0) || (EngineBox[0].ErroriPers2 != 0);	// IMPORTATO DA CORE.C TRAGHETTI V4.119 - DA CHIEDERE A MAURI SE INSERIRLO

    // Flag x Cumulativo Errori COMPRESSORE 1
    EngineBox[0].Error.GlobalStatusFlag =       EngineBox[0].Error.CompressorHi || EngineBox[0].Error.CompressorLo || EngineBox[0].Error.CondensatoreHi || 
                                                EngineBox[0].Error.CondensatoreLo /*|| EngineBox[0].Error.Gas_Recovery*/ || EngineBox[0].Error.Pressure_Lo ||
                                                EngineBox[0].Error.Pressure_Hi || EngineBox[0].Error.Defrosting || (Comp_Inverter[0].Ok_Voltage==0) /*|| EngineBox[0].Error.Recharge_Oil*/;
    
    EngineBox[0].Error.TouchStatusFlag =        EngineBox[0].Error.CondensatoreHi || EngineBox[0].Error.CondensatoreLo || EngineBox[0].PersErr.Pump1Fault || EngineBox[0].PersErr.Pump2Fault ||
                                                EngineBox[0].Split_Flags_Alarm_1_16 || EngineBox[0].Split_Flags_Alarm_17_32 || EngineBox[0].Split_Flags_EnableAndOffline_1_16 || EngineBox[0].Split_Flags_EnableAndOffline_17_32 ||
                                                EngineBox[0].Split_ModDev_Flags_Alarm_1_16 || EngineBox[0].Split_ModDev_Flags_Alarm_17_32 || EngineBox[0].Split_ModDev_Flags_EnableAndOffline_1_16 || EngineBox[0].Split_ModDev_Flags_EnableAndOffline_17_32 ||
                                                EngineBox[0].Error.CondenserFouled || (EngineBox[0].Error.TemperatureProbe && EngineBox[0].TestAllAbil.Master_Probe_Error) || (Comp_Inverter[0].Ok_Voltage==0);
	

    // Flag x Cumulativo Errori Persistenti
    EngineBox[0].PersErr.GlobalStatusFlag =     EngineBox[0].PersErr.CompressorHi || EngineBox[0].PersErr.CompressorLo || EngineBox[0].PersErr.CondensatoreHi || 
                                                EngineBox[0].PersErr.CondensatoreLo /*|| EngineBox[0].PersErr.Gas_Recovery*/ || EngineBox[0].PersErr.FloodSensor ||
                                                EngineBox[0].PersErr.ThermicComp || EngineBox[0].PersErr.SeaWaterLo || EngineBox[0].PersErr.SeaWaterHi ||
                                                EngineBox[0].PersErr.CaricaRefrig || EngineBox[0].PersErr.WaterSeaFlux | EngineBox[0].PersErr.LowOilLevel || EngineBox[0].PersErr.CompInverterFault || EngineBox[0].PersErr.PumpInverterFault || EngineBox[0].PersErr.FireAlarm || 
                                                EngineBox[0].PersErr.CoolWarm_Valve || EngineBox[0].PersErr.Acceleration_Error || (RTC_Flag==1) || EngineBox[0].PersErr.SuperHeat || ((EngineBox[0].Temperature.Error.Bit.Compressor_Output || EngineBox[0].Temperature.Error.Bit.Compressor_Suction || EngineBox[0].Error.Pressure_Probe) && EngineBox[0].TestAllAbil.Master_Probe_Error);
    
    // Flag x Cumulativo Errori COMPRESSORE 2
    EngineBox[1].Error.GlobalStatusFlag =       EngineBox[1].Error.CompressorHi || EngineBox[0].Error.CompressorLo || EngineBox[0].Error.CondensatoreHi || 
                                                EngineBox[0].Error.CondensatoreLo /*|| EngineBox[0].Error.Gas_Recovery*/ || EngineBox[0].Error.Pressure_Lo ||
                                                EngineBox[0].Error.Pressure_Hi || (Comp_Inverter[1].Ok_Voltage==0) /*|| EngineBox[0].Error.Recharge_Oil*/;

    EngineBox[1].Error.TouchStatusFlag =        EngineBox[0].Error.CondensatoreHi || EngineBox[0].Error.CondensatoreLo || EngineBox[0].PersErr.Pump1Fault || EngineBox[0].PersErr.Pump2Fault || 
                                                EngineBox[0].Split_Flags_Alarm_1_16  || EngineBox[0].Split_Flags_Alarm_17_32 || EngineBox[0].Split_Flags_EnableAndOffline_1_16 || EngineBox[0].Split_Flags_EnableAndOffline_17_32 ||
                                                EngineBox[0].Split_ModDev_Flags_Alarm_1_16 || EngineBox[0].Split_ModDev_Flags_Alarm_17_32 || EngineBox[0].Split_ModDev_Flags_EnableAndOffline_1_16 || EngineBox[0].Split_ModDev_Flags_EnableAndOffline_17_32 ||
                                                EngineBox[0].Error.CondenserFouled || (EngineBox[1].Error.TemperatureProbe && EngineBox[0].TestAllAbil.Master_Probe_Error) || (Comp_Inverter[1].Ok_Voltage==0);
    

    // Flag x Cumulativo Errori Persistenti
    EngineBox[1].PersErr.GlobalStatusFlag =     EngineBox[1].PersErr.CompressorHi || EngineBox[0].PersErr.CompressorLo || EngineBox[0].PersErr.CondensatoreHi || 
                                                EngineBox[0].PersErr.CondensatoreLo /*|| EngineBox[0].PersErr.Gas_Recovery*/ ||
                                                EngineBox[1].PersErr.ThermicComp || EngineBox[0].PersErr.SeaWaterLo || EngineBox[0].PersErr.SeaWaterHi ||
                                                EngineBox[0].PersErr.CaricaRefrig || EngineBox[0].PersErr.WaterSeaFlux | EngineBox[1].PersErr.LowOilLevel || EngineBox[1].PersErr.CompInverterFault || EngineBox[0].PersErr.PumpInverterFault || EngineBox[0].PersErr.FireAlarm || 
												EngineBox[0].PersErr.CoolWarm_Valve || EngineBox[1].PersErr.Acceleration_Error || (RTC_Flag==1) || EngineBox[0].PersErr.SuperHeat || ((EngineBox[1].Temperature.Error.Bit.Compressor_Output || EngineBox[0].Temperature.Error.Bit.Compressor_Suction || EngineBox[0].Error.Pressure_Probe) && EngineBox[0].TestAllAbil.Master_Probe_Error);
    

	int MasterError = EngineBox[0].PersErr.GlobalStatusFlag || EngineBox[1].PersErr.GlobalStatusFlag;
    int SplitError = (EngineBox[0].Split_Flags_EnableAndOffline_1_16!=0) || (EngineBox[0].Split_Flags_EnableAndOffline_17_32!=0) || 
                     (EngineBox[0].Split_Flags_Alarm_1_16!=0) || (EngineBox[0].Split_Flags_Alarm_17_32!=0) || 
                     (EngineBox[0].Split_ModDev_Flags_EnableAndOffline_1_16!=0) || (EngineBox[0].Split_ModDev_Flags_EnableAndOffline_17_32!=0) || 
                     (EngineBox[0].Split_ModDev_Flags_Alarm_1_16!=0) || (EngineBox[0].Split_ModDev_Flags_Alarm_17_32!=0);
    
	EngineBox[0].Error.Active_Error = 0;
	EngineBox[0].Error.Active_Error = (MasterError	<<  0) |	
                                      (SplitError   <<  1) |
                                      ((MasterError || SplitError) << 15);	
		
#if (K_AbilCompressor2==0)		// K_AbilCompressor2 = 0 -> Compressore singolo	
    EngineBox[0].Out.PersErr = EngineBox[0].PersErr.GlobalStatusFlag || EngineBox[0].Error.SPTemperatureFault;    
#else                           // K_AbilCompressor2 = 1 -> Compressore doppio	
    EngineBox[0].Out.PersErr = EngineBox[0].PersErr.GlobalStatusFlag || EngineBox[1].PersErr.GlobalStatusFlag || EngineBox[0].Error.SPTemperatureFault || EngineBox[0].PersErr.Fan_Speed;    
#endif    
}

#if (K_AbilErrorCompression==1)   
void CompressionError(void)
{
    int Errore = 0;
    static int CompressionFail = 0;    
#if(K_Condenser_Pressure_Abil==1)    
    if(EngineBox[0].DefMode == CoreRiscaldamento)
        Errore = EngineBox[0].Pressure.Gas - EngineBox[0].Pressure.LiquidCond;
    else if(EngineBox[0].DefMode == CoreRaffrescamento)
        Errore = EngineBox[0].Pressure.LiquidCond - EngineBox[0].Pressure.Gas;
#else
    if(EngineBox[0].DefMode == CoreRiscaldamento)
        Errore = EngineBox[0].Pressure.Gas - EngineBox[0].Pressure.Liquid;
    else if(EngineBox[0].DefMode == CoreRaffrescamento)
        Errore = EngineBox[0].Pressure.Liquid - EngineBox[0].Pressure.Gas;
#endif    
    else
        Errore = 0;
    
    if(Errore > (K_Delta_Error_Compressor+1000))
        CompressionFail = 0;
    if(Errore < K_Delta_Error_Compressor)
        CompressionFail = 1;
    
#if (K_AbilCompressor2==1)
    TimerAcceleration.Enable = ((((EngineBox[0].Out_Inverter>K_Soglia_Error_Comp_Motore) && !EngineBox[0].PersErr.Acceleration_Error) || ((EngineBox[1].Out_Inverter>K_Soglia_Error_Comp_Motore) && !EngineBox[1].PersErr.Acceleration_Error)) && (CompressionFail==1)) && EngineBox[0].TestAllAbil.Compression_Error;
#else
    TimerAcceleration.Enable = ((EngineBox[0].Out_Inverter>K_Soglia_Error_Comp_Motore) && (CompressionFail==1)) && EngineBox[0].TestAllAbil.Compression_Error;
#endif    
    if(!TimerAcceleration.Enable)
    {
        TimerAcceleration.TimeOut=0;
        TimerAcceleration.Value=0;        
    }
    if(TimerAcceleration.TimeOut)
    {
        TimerAcceleration.TimeOut=0;
        TimerAcceleration.Value=0;
		if(EngineBox[0].EngineIsOn)
        {
			EngineBox[0].GoShutDwComp =1;											// spengo 
            EngineBox[0].PersErr.Acceleration_Error = 1;
        }
#if (K_AbilCompressor2==1)
		if(EngineBox[1].EngineIsOn)
        {
			EngineBox[1].GoShutDwComp =1;
            EngineBox[1].PersErr.Acceleration_Error = 1;
        }
#endif	        
    }
/*
    static float Old_Out_Inverter = 0.0;
    static float Old_Gas_Pressure = 0.0;    
    static int Acceleration_Inverter = 0;
    static int Acceleration_Gas = 0;
    int Acceleration_Ratio;
    
    TimerAcceleration.Enable = EngineBox[0].Out_Inverter>0;
    
    if(!TimerAcceleration.Enable)
    {
        TimerAcceleration.TimeOut = 0;    
        TimerAcceleration.Value = 0;                
    }
    
    if(TimerAcceleration.TimeOut)
    {
        TimerAcceleration.TimeOut = 0;   

        Acceleration_Inverter = (int)(((float)EngineBox[0].Out_Inverter - Old_Out_Inverter) * 128.0);     // Moltplico per 128 per portare l'accelerazione Inverter sulla stessa scala dell'accellerazione del GAS 
        Acceleration_Gas = (int)((float)EngineBox[0].Pressure.Gas - Old_Gas_Pressure);         // Predisposto divisione per 1 
        
        Old_Out_Inverter = (float)EngineBox[0].Out_Inverter;
        Old_Gas_Pressure = (float)EngineBox[0].Pressure.Gas;             
        
        Acceleration_Inverter = Filter(&FilterAccInverter, Acceleration_Inverter, K_Campioni_FilterAccInverter);
        Acceleration_Gas = Filter(&FilterAccGas, Acceleration_Gas, K_Campioni_FilterAccGas);
        
        Acceleration_Ratio = Acceleration_Inverter / Acceleration_Gas;
       
        if(Acceleration_Ratio < K_InverterAcceleration)
        {
           if(EngineBox[0].Out_Inverter > K_xxx && EngineBox[0].Pressure.Gas < K_yyyy) 
            EngineBox[0].PersErr.Acceleration_Error = 1;
        }

        
        // Solo per DEBUG
        EngineBox[0].Acceleration_Inverter = Acceleration_Inverter;
        EngineBox[0].Acceleration_GasPressure = Acceleration_Gas;    
#if (K_DEBUG==1)  
        //DEBUG$$
        if(EngineBox[0].Acceleration_Inverter+32>255)
            EngineBox[0].Acceleration_Inverter = 255-32;                
        DAC_3 = 32 + EngineBox[0].Acceleration_Inverter;
		DAC_4 = 32 + EngineBox[0].Acceleration_GasPressure;
#endif        
        
    }
*/    
}
#endif



/*  *****************************************************************************************************************************
	Gestico il ciclo di recupero dell'olio
	***************************************************************************************************************************** */
void RechargeOilCycle(TypEngineBox * NumComp, TypTimer * TimerWork, TypTimer * TimerPause, TypTimer * TimerAlarm, TypTimer * TimerLowCriticalAlarm)
{
	if(NumComp->In.LowOilLevel && !NumComp->PersErr.LowOilLevel)
	{
		TimerAlarm->Enable = (NumComp->Out_Inverter > Off_Speed) && (NumComp->Out_Inverter >= k_Recharge_Oil_Speed_Thersold); 		// Se la velocità del compressore > 0 e sono in soglia di ricarica olio
        TimerLowCriticalAlarm->Enable = NumComp->Out_Inverter > Off_Speed; // Se la velocità del compressore > 0 (errore bassa criticità)
		
		if(TimerAlarm->Enable && !TimerPause->Enable && (NumComp->Out_Inverter >= k_Recharge_Oil_Speed_Thersold))
			TimerWork->Enable = 1;
		
		if(TimerWork->Enable && !NumComp->Out.RechargeOil && (NumComp->Out_Inverter >= k_Recharge_Oil_Speed_Thersold))
			NumComp->Out.RechargeOil = 1;

		if(TimerWork->TimeOut || NumComp->Out_Inverter < k_Recharge_Oil_Speed_Thersold)	
		{
			NumComp->Out.RechargeOil = 0;
			TimerWork->Enable = 0;
			TimerPause->Enable = 1;
			TimerWork->TimeOut = 0;
		}

		if(TimerPause->TimeOut || NumComp->Out_Inverter < k_Recharge_Oil_Speed_Thersold)
		{
			TimerPause->Enable = 0;
			TimerWork->Value = 0;
			TimerPause->TimeOut = 0;
		}
		
		if(TimerAlarm->TimeOut || TimerLowCriticalAlarm->TimeOut)
		{
			NumComp->PersErr.LowOilLevel = 1;
			if(NumComp->EngineIsOn)
				NumComp->GoShutDwComp = 1;
			TimerAlarm->TimeOut = 0;
		}
	}
	else
	{
		TimerWork->Enable = 0;
		TimerWork->TimeOut = 0;
		TimerWork->Value = 0;
		TimerPause->Enable = 0;
		TimerPause->Value = 0;
		TimerPause->TimeOut = 0;
		TimerAlarm->Enable = 0;
		TimerAlarm->TimeOut = 0;
		TimerAlarm->Value = 0;	
        TimerLowCriticalAlarm->Enable = 0;
		TimerLowCriticalAlarm->TimeOut = 0;
		TimerLowCriticalAlarm->Value = 0;	
		NumComp->Out.RechargeOil = 0;	
	} 
}


#if (K_AbilDefrostingCycle==1)					   
void DefrostingCycle(void)
{
    if(EngineBox[0].Pressure.Gas<K_DefrostingPressTrig)
        TimerDefrostingCycleWork.Enable = 1;
#if(K_AbilCompressor2==1)
    if(((EngineBox[0].Out_Inverter==Off_Speed) && (EngineBox[1].Out_Inverter==Off_Speed)) || (EngineBox[0].SplitFrigo_On==1) || (EngineBox[0].SplitFreezer_On==1))
        TimerDefrostingCycleWork.Enable = 0;
#else
    if((EngineBox[0].Out_Inverter==Off_Speed) || (EngineBox[0].SplitFrigo_On==1) || (EngineBox[0].SplitFreezer_On==1))
        TimerDefrostingCycleWork.Enable = 0;    
#endif
    TimerDefrostingCycleCheck.Enable = TimerDefrostingCycleWork.Enable && EngineBox[0].Pressure.Gas>K_DefrostingPressOk;
    
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
        
        EngineBox[0].Error.Defrosting = 1;
        
		if(EngineBox[0].EngineIsOn)
			EngineBox[0].GoShutDwComp =1;											// spengo
#if (K_AbilCompressor2==1)
		if(EngineBox[1].EngineIsOn)
			EngineBox[1].GoShutDwComp =1;
#endif		
    }
    
    TimerDefrostingCycleStop.Enable = EngineBox[0].Error.Defrosting == 1;
    
    if(!TimerDefrostingCycleStop.Enable)
    {
        TimerDefrostingCycleStop.TimeOut=0;
        TimerDefrostingCycleStop.Value=0;
    }
    
    if(TimerDefrostingCycleStop.TimeOut)
    {
        EngineBox[0].Error.Defrosting = 0;
    }
    
}
#endif

//-----------------------------------------------------------------------------------------------
// Converte il valore Out_inverter in scala DAC_1 da scala 0..10V x Inverter compressore (0..255)
// in un valore con scala "frequenza" (x 100) da inviare via ModBus all'Inverter
// 0..255 (bit) -> min_freq..max_freq 
//-----------------------------------------------------------------------------------------------
int DAC2FreqInverterComp(int outDAC)
{
	double out;

	out = (double)k_LIM_MAX_FREQ_INV_COMP - (double)k_LIM_MIN_FREQ_INV_COMP;
	out = (out / (double)255.0 * (double)outDAC) + (double)k_LIM_MIN_FREQ_INV_COMP;
	return (int)out; 		// (delta / 255 * DAC) + OFFS
}


//-----------------------------------------------------------------------------------------------
// Converte il valore di frequenza dell'inverter compressore nel valore di pilotaggio di frequenza
// dell'inverter pompa
//-----------------------------------------------------------------------------------------------
int FreqInv2FreqPump(long FcIn, int FcMin, long FcMax, int FpMin, long FpMax)
{
	float fDc;
	float fDp;
	float fK;
	int Fpout;
	
	fDc = (float)FcMax - (float)FcMin;
	fDp = (float)FpMax - (float)FpMin;	
	
	fK = fDc / fDp;
	
	Fpout = (int)(((float)FcIn - (float)FcMin) / fK) + (float)FpMin;

	if(Fpout > FpMax)
		Fpout = FpMax;
	if(Fpout < FpMin)
		Fpout = FpMin;	
	
	return Fpout;
}

int ValueToPercent(int Value, int MaxValue)
{
	int retval;

	retval = (unsigned int)( ((float)Value / (float)MaxValue) * 100.0); 
	return retval; 
}

int ValueToPercentDecimal(int Value, int MaxValue)
{
	int retval;

	retval = (unsigned int)( ((float)Value / (float)MaxValue) * 1000.0); 
	return retval; 
}

#if(K_AbilPumpInverter==1 || K_AbilPumpDAC==1)
unsigned int FreqInverterPump2DAC(unsigned int freq)
{
	double out;
	double k; 
	/*
	out = (double)k_LIM_MAX_FREQ_INV_COMP - (double)k_LIM_MIN_FREQ_INV_COMP;
	out = (out / (double)255.0 * (double)outDAC) + (double)k_LIM_MIN_FREQ_INV_COMP;
	return (int)out; 		// (delta / 255 * DAC) + OFFS
	*/	
	k = (double)5000 / 255.0;   //Fisso 50 hz come massimo, quindi abbassando K_LIM_MAX_FREQ_INV_PUMP proporzionalmente, abbasso uscita DAC per limitarlo.
	out = (double)freq / k;
	return (unsigned int)out;
}
#endif

// Controlla se fare lo switch delle pompe ed eventualmente effettua la corretta manovra "Arresto Libero" dell'inverter
void CheckSwitchPump(void)
{
	static int TempTrigPump1;
	static int TempTrigPump2;
	
	if((EngineBox[0].VirtualOut.Pompa_Acqua != EngineBox[0].Out.Pompa_Acqua) && EngineBox[0].Out.Pompa_Acqua2)
	{
		EngineBox[0].VirtualOut.TrigPump1 = 1;
		TempTrigPump1 = 1;
	}
	
	if((EngineBox[0].VirtualOut.Pompa_Acqua2 != EngineBox[0].Out.Pompa_Acqua2) && EngineBox[0].Out.Pompa_Acqua)
	{
		EngineBox[0].VirtualOut.TrigPump2 = 1;	
		TempTrigPump2 = 1;	
	}
	TimerTrigPump.Enable = (EngineBox[0].VirtualOut.TrigPump1 && TempTrigPump1) || (EngineBox[0].VirtualOut.TrigPump2 && TempTrigPump2);
	
	if(TimerTrigPump.TimeOut)
	{
		TimerTrigPump.TimeOut = 0;
		TimerTrigPump.Value = 0;
		TempTrigPump1 = 0;
		TempTrigPump2 = 0;
	}	
	
		
	if(!TempTrigPump1)
		EngineBox[0].Out.Pompa_Acqua = EngineBox[0].VirtualOut.Pompa_Acqua;
		
	if(!TempTrigPump2)
		EngineBox[0].Out.Pompa_Acqua2 = EngineBox[0].VirtualOut.Pompa_Acqua2;

	TimerPostPump.Enable = (EngineBox[0].VirtualOut.TrigPump1 && !TempTrigPump1) || (EngineBox[0].VirtualOut.TrigPump2 && !TempTrigPump2);
	
	if(TimerPostPump.TimeOut)
	{
		TimerPostPump.TimeOut = 0;
		TimerPostPump.Value = 0;
		EngineBox[0].VirtualOut.TrigPump1 = 0;
		EngineBox[0].VirtualOut.TrigPump2 = 0;		
	}
}

void PID_EngineBoxSuperHeat(void)
{
    int RetVal;    
        
    //int Temp;
    
    int RealSetPSuperHeat;
    int MinSPSuperHeat;
    
    //int PID_RetVal;
    /* 
    SuperHeatPID.pGain = ((float)ServiceBusBufferRxU4.Cella1)/100.0;    		// proportional gain
	SuperHeatPID.iGain = ((float)ServiceBusBufferRxU4.Cella2)/100.0;			// integral gain
	SuperHeatPID.dGain = 0.0; //((float)ServiceBusBufferRxU4.Cella3)/100.0;     		// derivative gain
    */
    RetVal = PressureGasToTempR410A(EngineBox[0].Pressure.LiquidCond);
    EngineBox[0].SuperHeat.Value = EngineBox[0].Temperature.Compressor_Suction - RetVal/*PressureGasToTempR410A(EngineBox[0].Pressure.LiquidCond)*/;

    EngineBox[0].SuperHeat.Value = Filter(&FilterHeatingSuperHeat, EngineBox[0].SuperHeat.Value, K_Campioni_FilterHeatingSuperHeat);
    //PID_RetVal = EngineBox[0].SuperHeat.Value;
    //if(EngineBox[0].SuperHeat.Value<0) 
    //    EngineBox[0].SuperHeat.Value = 0;    
    
    if(EngineBox[0].SuperHeat.ControlSuperHeat == 1)
    {
        //SuperHeatPID.SetP = K_SetPointHeatingSuperHeat;                      // Setpoint! (Valore desiderato di SuperHeat) 
        if(EngineBox[0].TestAllAbil.SuperHeat_SP_Heat)
            RealSetPSuperHeat = Room[k_Split_Master_Add].TestAll_SuperHeat_SP_Heat;
        else
            RealSetPSuperHeat = K_SetPointHeatingSuperHeat;
        
        if(EngineBox[0].TestAllAbil.SuperHeat_Custom_Par_Heat_PID)
        {
            SuperHeatPID.pGain = ((float)Room[k_Split_Master_Add].TestAll_SuperHeat_Heat_pGain)/100.0;    		// proportional gain
            SuperHeatPID.iGain = ((float)Room[k_Split_Master_Add].TestAll_SuperHeat_Heat_iGain)/100.0;			// integral gain
            SuperHeatPID.dGain = ((float)Room[k_Split_Master_Add].TestAll_SuperHeat_Heat_dGain)/100.0;     		// derivative gain
            TimerExecPID.Time = Room[k_Split_Master_Add].TestAll_SuperHeat_Heat_Time_Exec_PID;                   // Tempo esecuzione PID (ms)
        }
        else
        {
            SuperHeatPID.pGain = K_p_gain;    		// proportional gain
            SuperHeatPID.iGain = K_i_gain;			// integral gain
            SuperHeatPID.dGain = K_d_gain;     		// derivative gain
            SuperHeatPID.DeadZone = K_dead_zone;		// Dead zone
            TimerExecPID.Time = K_TimeExecPID;
        }
        
        TimerExecPID.Enable = 1;

        if(EngineBox[0].SuperHeat.Value<RealSetPSuperHeat)
        {
            MinSPSuperHeat = RealSetPSuperHeat/2;
            if(MinSPSuperHeat<600)
                MinSPSuperHeat = 600;   //Se il superheat è minore di 6K, comuque raso il minimo superheat a 6K
            SuperHeatPID.SetP = (int)round(ValueIn2ValueOut((float)EngineBox[0].Temperature.Max_Compressor_Output, (float)EngineBox[0].TempRestartCompressorHi, (float)EngineBox[0].TempCompressorHi, (float)RealSetPSuperHeat, (float)(MinSPSuperHeat)));
        }
        else
        {
            SuperHeatPID.SetP = RealSetPSuperHeat;
        }
        
        SuperHeatPID.SetP  = Filter(&FilterSuperHeatErrorValue, SuperHeatPID.SetP, K_Campioni_FilterSuperHeatErrorValue);
        
        SuperHeatPID.RetVal = (int)(((float)EngineBox[0].SuperHeat.Value * K_GainHeatingSuperHeat));                   // Retroaction Value (settare a "0" per taratura PID ad anello aperto!)
        
        if(TimerExecPID.TimeOut)
        {        
            UpdatePID(&SuperHeatPID, 0);      

            EngineBox[0].SuperHeat.HeatingCalc = SuperHeatPID.PVal;

            EngineBox[0].SuperHeat.HeatingCalc  = Filter(&FilterSuperHeatHeatingCalc, EngineBox[0].SuperHeat.HeatingCalc , K_Campioni_FilterSuperHeatHeatingCalc);

#if(K_AbilCompressor2==1)        
            if((Comp_Inverter[0].Out_Freq == Off_Speed) && (Comp_Inverter[1].Out_Freq == Off_Speed))
#else
            if(Comp_Inverter[0].Out_Freq == Off_Speed)            
#endif
            {
                SuperHeatPID.iState = 0;                    //Resetto valore PID se motori sono spenti
                EngineBox[0].SuperHeat.HeatingCalc  = 0;    //Resetto valore PID se motori sono spenti
            }
            TimerExecPID.TimeOut = 0;
            TimerExecPID.Value = 0;
        }
    }
}


void PID_EngineBoxPressSeaWater(void)
{
    //int PID_RetVal;
    /* 
    SuperHeatPID.pGain = ((float)ServiceBusBufferRxU4.Cella1)/100.0;    		// proportional gain
	SuperHeatPID.iGain = ((float)ServiceBusBufferRxU4.Cella2)/100.0;			// integral gain
	SuperHeatPID.dGain = 0.0; //((float)ServiceBusBufferRxU4.Cella3)/100.0;     		// derivative gain
    */
    EngineBox[0].PressSeaWater.Value =  EngineBox[0].Temperature.Sea_Water - PressureGasToTempR410A(EngineBox[0].Pressure.LiquidCond);

    EngineBox[0].PressSeaWater.Value = Filter(&FilterPressSeaWater, EngineBox[0].PressSeaWater.Value, K_Campioni_FilterHeatingSuperHeat);
    //PID_RetVal = EngineBox[0].SuperHeat.Value;
    if(EngineBox[0].PressSeaWater.Value<0) 
        EngineBox[0].PressSeaWater.Value = 0;    
    
    if(EngineBox[0].SuperHeat.ControlSuperHeat == 0)
    {    
        PressSeaWaterPID.SetP = K_SetPressSeaWaterSP;                      // Setpoint! (Valore desiderato di SuperHeat) 
        PressSeaWaterPID.RetVal = (int)(((float)EngineBox[0].PressSeaWater.Value * K_GainHeatingSuperHeat));                   // Retroaction Value (settare a "0" per taratura PID ad anello aperto!)
        UpdatePID(&PressSeaWaterPID, 0);        
        //EngineBox[0].SuperHeat.HeatingCalc = PressSeaWaterPID.PVal;
        if(PressSeaWaterPID.PVal>EngineBox[0].SuperHeat.HeatingCalc)
            Increment(&EngineBox[0].SuperHeat.HeatingCalc, 5, PressSeaWaterPID.PVal);
        else
            Decrement(&EngineBox[0].SuperHeat.HeatingCalc, 5, PressSeaWaterPID.PVal);
    }
}      

#if(K_AbilCoolingSuperHeat==1)
void CoolingEngineBoxSuperHeat(void)
{
    EngineBox[0].SuperHeat.CoolingCalc = EngineBox[0].Temperature.Compressor_Suction - PressureGasToTempR410A(EngineBox[0].Pressure.Gas);  
    EngineBox[0].SuperHeat.CoolingCalc = Filter(&FilterCoolingSuperHeat, EngineBox[0].SuperHeat.CoolingCalc, K_Campioni_FilterCoolingSuperHeat);

#if(K_AbilCompressor2==1)
    if((EngineBox[0].CoolingMode || EngineBox[1].CoolingMode) && EngineBox[0].TestAllAbil.SuperHeat_Cool_Correct)
#else
    //if(EngineBox[0].CoolingMode && (EngineBox[0].AbilSuperHeatBox==1))
    if(EngineBox[0].CoolingMode && EngineBox[0].TestAllAbil.SuperHeat_Cool_Correct)
#endif
    {
        TimerCoolingSuperHeatErr.Enable = (EngineBox[0].SuperHeat.CoolingCalc < Room[k_Split_Master_Add].SuperHeat_Cool_Min_SetPoint) || (EngineBox[0].SuperHeat.Correct_Cooling > 0);

        if(!TimerCoolingSuperHeatErr.Enable)
        {
            TimerCoolingSuperHeatErr.Value = 0;
            TimerCoolingSuperHeatErr.TimeOut = 0;
        }

        if(TimerCoolingSuperHeatErr.TimeOut)
        {
            TimerCoolingSuperHeatErr.Value = 0;
            TimerCoolingSuperHeatErr.TimeOut = 0;

            if((EngineBox[0].SuperHeat.CoolingCalc < Room[k_Split_Master_Add].SuperHeat_Cool_Min_SetPoint) && (EngineBox[0].SuperHeat.Correct_Cooling < Room[k_Split_Master_Add].SuperHeat_Cool_Max_Value_Correct))
                EngineBox[0].SuperHeat.Correct_Cooling+=100;

            if((EngineBox[0].SuperHeat.CoolingCalc > Room[k_Split_Master_Add].SuperHeat_Cool_Min_SetPoint) && (EngineBox[0].SuperHeat.Correct_Cooling > 0))
                EngineBox[0].SuperHeat.Correct_Cooling-=100;
        }
    }
    else
    {
        TimerCoolingSuperHeatErr.Enable = 0;
        TimerCoolingSuperHeatErr.Value = 0;
        TimerCoolingSuperHeatErr.TimeOut = 0;
        EngineBox[0].SuperHeat.Correct_Cooling = 0;        
    }
    
}
#endif


//(conver > (floor(conver)+0.5f)) ? ceil(conver) : floor(conver);
float round(float fvalue)
{
    float ftemp;
    
    ftemp = floor(fvalue)+0.5;
    
    if(fvalue > ftemp)
        ftemp = ceil(fvalue);
    else
        ftemp = floor(fvalue);
    
    return  ftemp;
}

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
	
    float VinTemp;
    
    
    VinTemp = Vin;
    
    if(VinMax >= VinMin)
    {
        if(VinTemp > VinMax)
            VinTemp = VinMax;
        if(VinTemp < VinMin)
            VinTemp = VinMin;
    }
    else    
    {
        if(VinTemp < VinMax)
            VinTemp = VinMax;
        if(VinTemp > VinMin)
            VinTemp = VinMin;
    }
    
	fDVin = VinMax - VinMin;
	fDVout = VoutMax - VoutMin;	
	
	fK = fDVin / fDVout;
	
	Vout = ((VinTemp - VinMin) / fK) + VoutMin;
    
    
    
    
/*
	if(Vout > VoutMax)
		Vout = VoutMax;
	if(Vout < VoutMin)
		Vout = VoutMin;	
*/	
	return Vout;
}



int CheckTempProbeFreeze(int Value, int * PrevVal, int * CntErr, unsigned int MaxCnt)
{
    if(Value > ((*PrevVal) + K_DeltaTempElapsedTimeError) ||
      (Value < ((*PrevVal) - K_DeltaTempElapsedTimeError)))
    {
        (*PrevVal) = Value;
        (*CntErr)=0;
    }
    else
    {
#if (K_AbilCompressor2==1)
        if(((*CntErr)<MaxCnt) && (EngineBox[0].CompressorReqOn>0) && ((EngineBox[0].Out_Inverter>Off_Speed) || (EngineBox[1].Out_Inverter>Off_Speed)))
#else
        if(((*CntErr)<MaxCnt) && (EngineBox[0].CompressorReqOn>0) && (EngineBox[0].Out_Inverter>Off_Speed))
#endif
            (*CntErr)++;       
    } 
    if((*CntErr)>=MaxCnt)
    {
        return 1;
    }
    else
        return 0;
}

int CheckPressProbeFreeze(int Value, int * PrevVal, int * CntErr, unsigned int MaxCnt)
{
    if(Value > ((*PrevVal) + K_DeltaPressElapsedTimeError) ||
      (Value < ((*PrevVal) - K_DeltaPressElapsedTimeError)))
    {
        (*PrevVal) = Value;
        (*CntErr)=0;
    }
    else
    {
#if (K_AbilCompressor2==1)
        if(((*CntErr)<MaxCnt) && (EngineBox[0].CompressorReqOn>0) && ((EngineBox[0].Out_Inverter>Off_Speed) || (EngineBox[1].Out_Inverter>Off_Speed)))
#else
        if(((*CntErr)<MaxCnt) && (EngineBox[0].CompressorReqOn>0) && (EngineBox[0].Out_Inverter>Off_Speed))
#endif
            (*CntErr)++;       
    } 
    if((*CntErr)>=MaxCnt)
    {
        return 1;
    }
    else
        return 0;
}