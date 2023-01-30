//----------------------------------------------------------------------------------
//	Progect name:	Interrupt.c
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			TDM Team
//	Date:			27/02/2018
//	Description:	Routines di Interrupt
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
#include <libpic30.h>
#include "DefinePeriferiche.h"
#include "usart.h"
#include "Driver_Comunicazione.h"
#include "Driver_ModBus.h"
#include "ProtocolloComunicazione.h"
#include "ProtocolloModBus.h"

#if (K_AbilMODBCAN==1)  
 #include "Driver_ModBus_U4.h"
 #include "ProtocolloModBus_U4.h"
#endif //#if (K_AbilMODBCAN==1)  

#include "Core.h"
#include "SevenSegDisplay.h"
#include "FWSelection.h"
#include "Interrupt.h"
#include "ADC.h"

#ifdef UART3_ON  
#include "ServiceComunication.h"
#include "delay.h"
#endif

#if (K_AbilMODBCAN==1)  
#include "ModBusCAN.h"
#endif

volatile int SecDivider = 0;                        // divisore per base tempo di 1 secondo
volatile int HalfSecDivider = 0;					// divisore per base tempo di 1/2 secondo
volatile int mSecDivider = 0;                       // divisore per base tempo di 100 mSec
//int Led_Divider = 0;                                // divisore per il lampeggio del led

extern unsigned char CurUsedProtocolId;             // Id del protocollo correntemente utilizzato
// timer	

extern volatile TypTimer		DividerSecondi;				// Timer per la divisione in secondi
extern volatile TypTimer		TimerElaborazioneC1;		// Timer Elaborazione1
extern volatile TypTimer		TimerElaborazioneC2;		// Timer Elaborazione2
extern volatile TypTimer		TimerPwrLed;				// Timer Led Power
extern volatile TypTimer 		TmrLastRequest;				// Timer per la verifica della comunicazione
extern volatile TypTimer		TimerSystemCycle;			// Timer per il calcolo del tempo di ciclo programma
//extern volatile TypTimer 		TimerRstComErr;
//extern volatile TypTimer 		TimerModRstComErr;
extern volatile TypTimer		TimerSwitchCompressor;		// Timer per time switch secondo compressore
extern volatile TypTimer		TimerIntegrateCompressor2;	// Timer per integrazione secondo compressore
extern volatile TypTimer		TimerDeintegrateCompressor2;// Timer per deintegrazione secondo compressore
extern volatile TypTimer 		TimerComunicationError;		// Timer per gestione counter errori di comunicazione
extern volatile TypTimer		TimerHwReset;				// Timer per gestire Teleruttore di sgancio potenza per ripristino allarme "MOFF" anomalo.
extern volatile TypTimer		TimerRestartAfterBlackOut;	// Timer per gestire la pausa alla riaccensione da blackout se i compressori erano in funzione 
extern volatile TypTimer       TimerSwitchPump;            // Timer per Switch pompe
extern volatile TypTimer       TimerTrigPump;				// Timer per freerun switch pompe
extern volatile TypTimer       TimerPostPump;				// Timer per freerun switch pompe
extern volatile TypTimer       TimerValve_On_Demand_Limit;	// Timer per calcolare il limite valvola
extern volatile TypTimer       TimerValve_On_Demand_Switch;// Timer per calcolare il tempo di switch della valvola
extern volatile TypTimer       TimerCoolingSuperHeatErr;
extern volatile TypTimer       TimerExecPID;
extern volatile TypTimer       TimerCoolWarmValveError;
extern volatile TypTimer       TimerInverterPersError_C1;
extern volatile TypTimer       TimerInverterPersError_C2;
extern volatile TypTimer       TimerInverterPersError_Pump;
extern volatile TypTimer       TimerSuperHeatError;
extern volatile TypTimer       TimerTemperatureProbeErrorBox1;
extern volatile TypTimer       TimerTemperatureProbeErrorBox2;
extern volatile TypTimer       TimerPressureProbeErrorBox;
extern volatile TypTimer       TimerCheckTemperatureProbe;
extern volatile TypTimer       TimerCheckPressureProbe;
extern volatile TypTimer       TimerExitProgrammingMode;
extern volatile TypTimer       TimerInVoltageInverterOk_C1;
extern volatile TypTimer       TimerInVoltageInverterOk_C2;

#if (K_AbilMODBCAN==1)
extern volatile TypTimer StableValueChangeHMI;         // Timer x gestione accettazione cambio valori da HMI Scheiber
#endif

IncludeVariabiliGlobaliDriverModBus();


//extern volatile int SecDivider;               // divisore per base tempo di 1 secondo
extern unsigned ComunicationLost;               // flag di comunicazione persa
extern unsigned ServiceSlaveMode;               // Flag se indica il service mode, nel quale il master diventa uno slave
extern unsigned InputProgrammingMode;           // modalità di programmazione

unsigned int DspToggle=0;
extern TypRTC MyRTC;						// RTC di sistema

//extern int Led_Divider;                             // divisore per il lampeggio del led
//extern volatile unsigned int iPckModRx;

//extern unsigned int ForceU1MyBUS;                   // Flag per gestire la forzatura del solo protocollo MyBUS sul canale primario (UART1) alla prima ricezione MyBUS valida



//----------------------------------------------------------------------------------
//	Interrupt
//----------------------------------------------------------------------------------
/*
	Interrupt vector list
#define INT0_VCTR	0x14	// External Interrupt 0
#define IC1_VCTR	0x16	// Input Capture 1
#define OC1_VCTR	0x18	// Output Compare 1
#define T1_VCTR		0x1a	// Timer 1

#define DMA0_VCTR	0x1c	// DMA Channel 0
#define IC2_VCTR	0x1e	// Input Capture 2
#define OC2_VCTR	0x20	// Output Compare 2
#define T2_VCTR		0x22	// Timer 2
#define T3_VCTR		0x24	// Timer 3
#define SPI1E_VCTR	0x26	// Serial Comms 1 Error
#define SPI1D_VCTR	0x28	// Serial Comms 1 Transfer Done
#define U1RX_VCTR	0x2a	// UART1 Receiver
#define U1TX_VCTR	0x2c	// UART1 Transmitter
#define ADC1_VCTR	0x2e	// A/D Converter 1
#define DMA1_VCTR	0x30	// DMA Channel 1
#define SI2C1_VCTR	0x34	// I2C1 Slave Interrupt
#define MI2C1_VCTR	0x36	// I2C1 Master Interrupt
#define CM_VCTR		0x38	// Comparator Event
#define INCH_VCTR	0x3A	// Input Change Interrupt
#define INT1_VCTR	0x3C	// External Interrupt 1
#define ADC2_VCTR	0x3e	// A/D Converter 2
#define IC7_VCTR	0x40	// Input Capture 7
#define DMA2_VCTR	0x44	// DMA Channel 2
#define IC8_VCTR	0x42	// Input Capture 8
#define OC3_VCTR	0x46	// Output Compare 3
#define OC4_VCTR	0x48	// Output Compare 4
#define T4_VCTR		0x4a	// Timer 4
#define T5_VCTR		0x4c	// Timer 5
#define INT2_VCTR	0x4e	// External Interrupt 2
#define U2RX_VCTR	0x50	// UART2 Receiver
#define U2TX_VCTR	0x52	// UART2 Transmitter
#define SPI2E_VCTR	0x54	// Serial Comms 2 Error
#define SPI2D_VCTR	0x56	// Serial Comms 2 Transfer Done
#define C1RX_VCTR	0x58	// ECAN1 Receive Data Ready
#define C1E_VCTR	0x58	// CAN1 Error on PS devices
#define C1_VCTR		0x5A	// CAN1 or ECAN1 Event
#define DMA3_VCTR	0x5c	// DMA Channel 3
#define IC3_VCTR	0x5e	// Input Capture 3
#define IC4_VCTR	0x60	// Input Capture 4
#define IC5_VCTR	0x62	// Input Capture 5
#define IC6_VCTR	0x64	// Input Capture 6
#define OC5_VCTR	0x66	// Output Compare 5
#define OC6_VCTR	0x68	// Output Compare 6
#define OC7_VCTR	0x6a	// Output Compare 7
#define OC8_VCTR	0x6c	// Output Compare 8
#define PMP_VCTR	0x6e	// Parallel Port Master
#define DMA4_VCTR	0x70	// DMA Channel 4
#define T6_VCTR		0x72	// Timer 6
#define T7_VCTR		0x74	// Timer 7
#define SI2C2_VCTR	0x76	// I2C2 Slave Interrupt
#define MI2C2_VCTR	0x78	// I2C2 Master Interrupt
#define T8_VCTR		0x7a	// Timer 8
#define T9_VCTR		0x7c	// Timer 9
#define INT3_VCTR	0x7e	// External Interrupt 3
#define INT4_VCTR	0x80	// External Interrupt 4
#define C2RX_VCTR	0x82	// ECAN2 Receive Data Ready
#define C2E_VCTR	0x82	// CAN2 Error on PS devices
#define C2_VCTR		0x84	// CAN2 or ECAN2 Event
#define PWM_VCTR	0x86	// PWM Period Match
#define QEI_VCTR	0x88	// QEI Interrupt
#define DCIE_VCTR	0x8A	// DCI Error
#define DCID_VCTR	0x8c	// DCI Transfer Done
#define DMA5_VCTR	0x8e	// DMA Channel 5
#define RTCC_VCTR	0x90	// Real-time Clock/Calendar
#define FLTA_VCTR	0x92	// PWM Fault A
#define FLTB_VCTR	0x94	// PWM Fault B
#define U1E_VCTR	0x96	// UART1 Error
#define U2E_VCTR	0x98	// UART2 Error
#define DMA6_VCTR	0x9c	// DMA Channel 6
#define DMA7_VCTR	0x9e	// DMA Channel 7
#define C1TX_VCTR	0xa0	// ECAN1 Transmit Data Request
#define C2TX_VCTR	0xa2	// ECAN2 Transmit Data Request
*/


//---------------------------------------------------------------------------------
//void interrupt _ADC1ConversionDone(void) @ ADC1_VCTR
//---------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _ADC1Interrupt()
{
	_AD1IF = 0;			// ripulisco il flag di interrupot dell'ADC
	_ASAM = 0;          // Stoppo campinonamento e conversione
    
	Sum_Adc_Temp[1] += ADC1BUF0;		// sommo il valore letto dall'adc nel cumulativo
	Sum_Adc_Temp[2] += ADC1BUF1;
	Sum_Adc_Temp[3] += ADC1BUF2;
	Sum_Adc_Temp[4] += ADC1BUF3;
	Sum_Adc_Temp[5] += ADC1BUF4;
	Sum_Adc_Temp[6] += ADC1BUF5;
    
	Sum_Adc_Temp[7] += ADC1BUF6;
	Sum_Adc_Temp[8] += ADC1BUF7;
	Sum_Adc_Temp[9] += ADC1BUF8;
    
	Sum_Adc_Temp[10] += ADC1BUF9;    
	
    unsigned int x;
    
    for(x=1; x<=10; x++)
    {
        N_Campioni[x]++;						// incremento il cnt1 dei campioni        
        
        if(N_Campioni[x] > Campioni_Acquisizione[x])		// se ho oltrepassato la soglia dei campioni
        {	
            N_Campioni[x] = Campioni_Acquisizione[x];		// riscalo il numero dei campioni
            Sum_Adc_Temp[x] -= Adc_Temp[x];		// sottraggo il campione medio dalla somma cumulativa
        }    
        
        if(N_Campioni[x] != 0)
            Adc_Temp[x] = Sum_Adc_Temp[x] / N_Campioni[x];          
    }
    
	_DONE = 0;
    
    
    _AD1IE = 0;             // disabilito interrupt AD    
}

//---------------------------------------------------------------------------------
//void interrupt UART1_Rx(void) @ U1RX_VCTR
//---------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt()
//void __attribute__((interrupt)) _U1RXInterrupt()
{
	_U1RXIF = 0;			// Clear interrupt flag Uart1 
	
    
    GlobalIntReadBus(); 
    

/*    
    switch (CurUsedProtocolId)                  // controlo il protocollo che sto utilizzando
	{
		case FlagMyBusProt_BUSM:				// utilizzo il mio protocollo
			iPckModRx = 0;
			IntReadBus(&BufferRx);
			break;

		case FlagModBusProt_BUSM:			// utilizzo il protocollo ModBus
			//@ Mod_IntReadBus_Master();    // <- Da creare come "Mod_IntReadBus" ma su UART1
            iPckRx = 0;	
            Mod_IntReadBus();
			break;
	}
*/	
	Recive_Cmd(&BufferRx, &LastRx);
	Mod_Recive_Cmd();
    
    
    if(ServiceSlaveMode)				// se sono in service mode
        Process_Cmd(&LastRx);			// gestisco le richieste     
    
/*  DA INDAGARE IL MOTIVO PER CUI SE IL PROTOCOLLO MODBUS O MYBUS RICEVONO UN DATO VALIDO E RESETTANO QUINDI I FLAG DELL'ALTRO
 * NON FUNZIONA PIU IL MYBUS   
    if(LastRx.Valid_Data==1)                           // Se ho ricevuto un dato valido su MyBUS
    {                                               // Resetto ricezione in corso per MODBUS
        //TimeOutModRx.Enable = 0;
        //TimeOutModRx.Value = 0;
        //iPckModRx = 0;                              // reinizializzo l'indice 
        //Mod_BufferRx.Valid_Data = 0;                // resetto la validità del buffer di rx 
        //Mod_LastRx.Valid_Data = 0;                  // resetto la validità del dato
        TmrLastRequest.Value = 0;                   // Comunication Lost...
        TmrLastRequest.TimeOut = 0;
    }
    
    if(Mod_LastRx.Valid_Data==1)                       // Se ho ricevuto un dato valido su MODBUS
    {                                               // Resetto ricezione in corso per MyBUS
        //TimeOutRx.Enable = 0;
        //TimeOutRx.Value = 0;        
        //iPckRx = 0;                                 // reinizializzo l'indice 
        //BufferRx.Valid_Data = 0;                    // resetto la validità del buffer di rx 
        //LastRx.Valid_Data = 0;                      // resetto la validità del dato
        TmrLastRequest.Value = 0;                   // Comunication Lost...
        TmrLastRequest.TimeOut = 0;
    }  
     */           
    
    // Per tutto il traffico ricevuto resetto il Timer Last Request x il Comunication Lost
    //if(LastRx.Valid_Data==1 || Mod_LastRx.Valid_Data==1)    // Se ho ricevuto un dato valido su MyBUS un dato valido su MODBUS
    {                                               
        TmrLastRequest.Value = 0;                   // Comunication Lost...
        TmrLastRequest.TimeOut = 0;
    }
    
    
	//return;
}

//---------------------------------------------------------------------------------
//void interrupt UART1_Tx(void) @ U1TX_VCTR         Interrupt buffer TX UART1 empty:
//---------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt()	
{
	//Lasciare sempre all'inizio della funzione interrupt!!!    
	_U1TXIF = 0;                                            // Clear interrupt flag TX Uart1 
    
    while(U1STAbits.UTXBF)                                  // attendo che il buffer di scrittura sia libero
		continue;

	//oRTS_Master = 0;											// abbasso il pin dell'rts x la 485                 
    
    switch (CurUsedProtocolId)
    {
        case FlagMyBusProt_BUSM:                                 // utilizzo il protocollo MyBUS 
            if(MyBUS_BufferTx.CntByte < MyBUS_BufferTx.nByte)			// se il buffer TX contiene ancora byte da inviare....
            {
                MyBUS_BufferTx.CntByte++;								// incremento n. byte inviati
                //oRTS_Master = 1;                                             // alzo il pin dell'rts x la 485                                                
                U1TXREG = MyBUS_BufferTx.Buffer[MyBUS_BufferTx.CntByte-1];  // scrivo il carattere nel registro di trasmissione
            }
            else
            {
                MyBUS_BufferTx.Valid_Data = 0;
                BufferTx.Valid_Data = 0;
                //_U1TXIE = 0;                                    // Disattivo l'interrupt di TX Uart1
                oRTS_Master = 0;                                // abbasso il pin dell'rts x la 485
            }
		break;        
        
        case FlagModBusProt_BUSM:                                            // utilizzo il protocollo ModBus
            if(Mod_BufferTx.CntByte < Mod_BufferTx.nByte)               // se il buffer TX contiene ancora byte da inviare....
            {
                Mod_BufferTx.CntByte++;                                 // incremento n. byte inviati
                //oRTS_Master = 1;                                         // alzo il pin dell'rts x la 485                                                
                U1TXREG = Mod_BufferTx.Buffer[Mod_BufferTx.CntByte-1];  // scrivo il carattere nel registro di trasmissione
            }
            else
            {
                Mod_BufferTx.Valid_Data = 0;
                //_U1TXIE = 0;                                            // Disattivo l'interrupt di TX Uart1    
                oRTS_Master = 0;										// abbasso il pin dell'rts x la 485                        
            }
        break;
    }    
    
    //_U1TXIF = 0;                                            // Clear interrupt flag TX Uart1 
}



//---------------------------------------------------------------------------------
//void interrupt UART1 Error @ U1E_VCTR
//---------------------------------------------------------------------------------
// void _ISR __attribute__((no_auto_psv)) _U1ErrInterrupt()
 void __attribute__((interrupt, no_auto_psv)) _U1ErrInterrupt()
 {
	unsigned char errframData;
	//static unsigned int erroverrun1 = 0;	// solo x DEBUG
	//static unsigned int errframing1 = 0;	// solo x DEBUG
    
	//Lasciare sempre all'inizio della funzione interrupt!!! 
	_U1ERIF = 0;
    
	if(U1STAbits.OERR)
	{
	    U1STAbits.OERR = 0; //CLEAR THE OVERRUN ERROR BIT
		//erroverrun1++;	// solo x DEBUG
	    
	}else if(U1STAbits.FERR)
	{
	        U1STAbits.FERR = 0;
	        errframData=U1RXREG ; // flush buffer
	        //errframing1++;		// solo x DEBUG
	}  
    
   //IFS4bits.U1ERIF = 0;
 }


//---------------------------------------------------------------------------------
//void interrupt UART2_Rx(void) @ U2RX_VCTR
//---------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt()	
//void __attribute__((interrupt)) _U2RXInterrupt()
{
    int RxChar = 0;  
    // Da sistemare per gestire entrambe i protocolli MyBUS e MODBUS
    // Ora fa solo MODBUS
	_U2RXIF = 0;				// Clear interrupt flag Uart2
	//switch (CurUsedProtocolId)			// controlo il protocollo che sto utilizzando
	//{
		//case FlagMyBusProt_BUSS:				// utilizzo il mio protocollo
		//	iPckModRx = 0;
		//	IntReadBusSec(&BufferRxSec);
		//	break;

		//case FlagModBusProt_BUSS:			// utilizzo il protocollo ModBus

    
	while(U2STAbits.URXDA)                          // Receive Buffer Data Available bit (read-only)
	{       
        RxChar = U2RXREG;      
        Mod_IntReadBus(RxChar);
    }
	
	//Recive_Cmd_Sec(&BufferRxSec, &LastRxSec);
	Mod_Recive_Cmd();

	//return;
    
}



//---------------------------------------------------------------------------------
//void interrupt UART2_Tx(void) @ U2TX_VCTR         Interrupt buffer TX UART2 empty:
//---------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _U2TXInterrupt()	
{
	//Lasciare sempre all'inizio della funzione interrupt!!!    
	_U2TXIF = 0;									// Clear interrupt flag TX Uart2

    while(U2STAbits.UTXBF)                                  // attendo che il buffer di scrittura sia libero
		continue;

	//oRTS_Slave = 0;											//!!! // abbasso il pin dell'rts x la 485                 
    
    if(Mod_BufferTx.CntByte < Mod_BufferTx.nByte)               // se il buffer TX contiene ancora byte da inviare....
    {
        Mod_BufferTx.CntByte++;                                           // incremento n. byte inviati
        //oRTS_Slave = 1;                                                             //!!! // alzo il pin dell'rts x la 485                                                
        U2TXREG = Mod_BufferTx.Buffer[Mod_BufferTx.CntByte-1];  // scrivo il carattere nel registro di trasmissione
    }
    else
    {
        Mod_BufferTx.Valid_Data = 0;                            // Segnalo fine TX Buffer
        //_U2TXIE = 0;                                            // Disattivo l'interrupt di TX Uart3
        oRTS_Slave = 0;											// abbasso il pin dell'rts x la 485        
    }
       
	//_U2TXIF = 0;                                                // Clear interrupt flag TX Uart2
}



//---------------------------------------------------------------------------------
//void interrupt UART2 Error @ U2E_VCTR
//---------------------------------------------------------------------------------
// void _ISR __attribute__((no_auto_psv)) _U2ErrInterrupt()
 void __attribute__((interrupt, no_auto_psv)) _U2ErrInterrupt()
 {
	unsigned char errframData;
//	static unsigned int erroverrun2 = 0;	// solo x DEBUG
//	static unsigned int errframing2 = 0;	// solo x DEBUG
    
	//Lasciare sempre all'inizio della funzione interrupt!!!
	_U2ERIF = 0;
    
	if(U2STAbits.OERR)
	{
	    U2STAbits.OERR = 0; //CLEAR THE OVERRUN ERROR BIT
		//erroverrun2++;	// solo x DEBUG
	    
	}else if(U2STAbits.FERR)
	{
	        U2STAbits.FERR = 0;
	        errframData=U2RXREG ; // flush buffer
	        //errframing2++;	// solo x DEBUG
	}  
   //IFS4bits.U2ERIF = 0;
 }



//---------------------------------------------------------------------------------
//void interrupt UART3_Rx(void) @ U3RX_VCTR
//---------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _U3RXInterrupt()
//void __attribute__((interrupt)) _U1RXInterrupt()
{
	_U3RXIF = 0;			// Clear interrupt flag Uart3

    IntReadServiceBusU3(&ServiceBusBufferRxU3);
    if(ServiceBusBufferRxU3.Valid_Data)
        Process_ServiceU3Cmd(&ServiceBusBufferRxU3);
	    
	//return;
}

 
//---------------------------------------------------------------------------------
//void interrupt UART3_Tx(void) @ U3TX_VCTR         Interrupt buffer TX UART3 empty:
//---------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _U3TXInterrupt()	
{
	_U3TXIF = 0;									// Clear interrupt flag TX Uart3

    while(U3STAbits.UTXBF)                                  // attendo che il buffer di scrittura sia libero
		continue;

	oRTS_UART3 = 0;											//!!! // abbasso il pin dell'rts x la 485                 
 
    if(ServiceBusBufferTxU3.CntByte < ServiceBusBufferTxU3.nByte)               // se il buffer TX contiene ancora byte da inviare....
    {
        ServiceBusBufferTxU3.CntByte++;                                           // incremento n. byte inviati
        oRTS_UART3 = 1;                                                             //!!! // alzo il pin dell'rts x la 485                                                
        U3TXREG = ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.CntByte-1];  // scrivo il carattere nel registro di trasmissione
    }
    else
    {
        ServiceBusBufferTxU3.Valid_Data = 0;                      // Segnalo fine TX Buffer
        //_U3TXIE = 0;                                            // Disattivo l'interrupt di TX Uart3
        oRTS_UART3 = 0;											// abbasso il pin dell'rts x la 485        
    }
  
    
    //IFS5bits.U3TXIF = 0;									// Clear interrupt flag TX Uart3
	//return;
}


//---------------------------------------------------------------------------------
//void interrupt UART3 Error @ U3E_VCTR
//---------------------------------------------------------------------------------
 void __attribute__((interrupt, no_auto_psv)) _U3ErrInterrupt()
 {
	unsigned char errframData;
 
	_U3ERIF = 0;
    
	if(U3STAbits.OERR)
	{
	    U3STAbits.OERR = 0; //CLEAR THE OVERRUN ERROR BIT    
	}
    else if(U3STAbits.FERR)
	{
	        U3STAbits.FERR = 0;
	        errframData=U3RXREG ; // flush buffer
	}  
   
 }


#if (K_AbilMODBCAN==1)  
 //---------------------------------------------------------------------------------
//void interrupt UART4_Rx(void) @ U4RX_VCTR
//---------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _U4RXInterrupt()
//void __attribute__((interrupt)) _U1RXInterrupt()
{
	_U4RXIF = 0;			// Clear interrupt flag Uart4
    
    int RxChar = 0;  
    
	while(U4STAbits.URXDA)                          // Receive Buffer Data Available bit (read-only)
	{       
        RxChar = U4RXREG;      
        Mod_IntReadBus_U4(RxChar);
    }
	
	//Recive_Cmd_Sec(&BufferRxSec, &LastRxSec);
	Mod_Recive_Cmd_U4();
}
#else   
 //---------------------------------------------------------------------------------
//void interrupt UART4_Rx(void) @ U4RX_VCTR
//---------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _U4RXInterrupt()
//void __attribute__((interrupt)) _U1RXInterrupt()
{
	_U4RXIF = 0;			// Clear interrupt flag Uart3

    IntReadServiceBusU4(&ServiceBusBufferRxU4);
    if(ServiceBusBufferRxU4.Valid_Data)
        Process_ServiceU4Cmd(&ServiceBusBufferRxU4);
	
    
	//return;
}
#endif  //#if (K_AbilMODBCAN==1)  


#if (K_AbilMODBCAN==1)  
//---------------------------------------------------------------------------------
//void interrupt UART4_Tx(void) @ U4TX_VCTR         Interrupt buffer TX UART4 empty:
//---------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _U4TXInterrupt()	
{
	//Lasciare sempre all'inizio della funzione interrupt!!!        
	_U4TXIF = 0;									// Clear interrupt flag TX Uart4

    while(U4STAbits.UTXBF)                                  // attendo che il buffer di scrittura sia libero
		continue;

	//oRTS_UART4 = 0;											//!!! // abbasso il pin dell'rts x la 485                 
 
    if(Mod_BufferTx_U4.CntByte < Mod_BufferTx_U4.nByte)               // se il buffer TX contiene ancora byte da inviare....
    {
        Mod_BufferTx_U4.CntByte++;                                           // incremento n. byte inviati
        //oRTS_UART4 = 1;                                                             //!!! // alzo il pin dell'rts x la 485                                                
        U4TXREG = Mod_BufferTx_U4.Buffer[Mod_BufferTx_U4.CntByte-1];  // scrivo il carattere nel registro di trasmissione
    }
    else
    {
        Mod_BufferTx_U4.Valid_Data = 0;                            // Segnalo fine TX Buffer
        //_U4TXIE = 0;                                            // Disattivo l'interrupt di TX Uart3
        oRTS_UART4 = 0;											// abbasso il pin dell'rts x la 485        
    }
       
	//_U4TXIF = 0;                                                // Clear interrupt flag TX Uart2    
    
    
}
#else
//---------------------------------------------------------------------------------
//void interrupt UART4_Tx(void) @ U4TX_VCTR         Interrupt buffer TX UART4 empty:
//---------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _U4TXInterrupt()	
{
	_U4TXIF = 0;									// Clear interrupt flag TX Uart4

    while(U4STAbits.UTXBF)                                  // attendo che il buffer di scrittura sia libero
		continue;

	oRTS_UART4 = 0;											//!!! // abbasso il pin dell'rts x la 485                 
 
    if(ServiceBusBufferTxU4.CntByte < ServiceBusBufferTxU4.nByte)               // se il buffer TX contiene ancora byte da inviare....
    {
        ServiceBusBufferTxU4.CntByte++;                                           // incremento n. byte inviati
        oRTS_UART4 = 1;                                                             //!!! // alzo il pin dell'rts x la 485                                                
        U4TXREG = ServiceBusBufferTxU4.Buffer[ServiceBusBufferTxU4.CntByte-1];  // scrivo il carattere nel registro di trasmissione
    }
    else
    {
        ServiceBusBufferTxU4.Valid_Data = 0;                      // Segnalo fine TX Buffer
        //_U4TXIE = 0;                                            // Disattivo l'interrupt di TX Uart4
        oRTS_UART4 = 0;											// abbasso il pin dell'rts x la 485        
    }
  
    
    //IFS5bits.U4TXIF = 0;									// Clear interrupt flag TX Uart4
	//return;
}
#endif  //#if (K_AbilMODBCAN==1) 

//---------------------------------------------------------------------------------
//void interrupt UART4 Error @ U4E_VCTR
//---------------------------------------------------------------------------------
 void __attribute__((interrupt, no_auto_psv)) _U4ErrInterrupt()
 {
	unsigned char errframData;
 
	_U4ERIF = 0;
    
	if(U4STAbits.OERR)
	{
	    U4STAbits.OERR = 0; //CLEAR THE OVERRUN ERROR BIT    
	}
    else if(U4STAbits.FERR)
	{
	        U4STAbits.FERR = 0;
	        errframData=U4RXREG ; // flush buffer
	}  
   
 }
 
 
 
 
 

//---------------------------------------------------------------------------------
//void interrupt TMR_1(void) @ T1_VCTR (interrupt ogni 1mS)
//---------------------------------------------------------------------------------
//void __attribute__((interrupt, shadow)) _T1Interrupt()
//void __attribute__((interrupt)) _T1Interrupt()
 void __attribute__((interrupt, no_auto_psv)) _T1Interrupt()
{
    _T1IF = 0;				// resetto il flag di interrupt
    
	IntTimeOutRx();
	Mod_IntTimeOutRx();
#if (K_AbilMODBCAN==1)      
	Mod_IntTimeOutRx_U4();
#endif  //#if (K_AbilMODBCAN==1)              
     
	// Gestisco i timer in ms     
    ChkTimer(&TimeOutRx);
    ChkTimer(&TimeOutPktRx);
    ChkTimer(&TimeOutModRx);
    ChkTimer(&TimeOutModPktRx);
#if (K_AbilMODBCAN==1)          
    ChkTimer(&TimeOutModRx_U4);
    ChkTimer(&TimeOutModPktRx_U4);
#endif  //#if (K_AbilMODBCAN==1)         
    ChkTimer(&DividerSecondi);
    ChkTimer(&TimerCompressoreC1);
    ChkTimer(&TimerCompressoreC2);
    ChkTimer(&TimerWorkPump);
    ChkTimer(&TimerPwrLed);
    ChkTimer(&TimerSystemCycle);
    //ChkTimer(&TimerRstComErr);			// timer per il reset dei contatori legati alla segnalazione di errori di comunicazione
    //ChkTimer(&TimerModRstComErr);			// timer per il reset dei contatori legati alla segnalazione di errori di comunicazione
    ChkTimer(&TimerComunicationError);
    ChkTimer(&TimerTrigPump);
    ChkTimer(&TimerPostPump);
    ChkTimer(&TimerExecPID);
    
/*    
#if (K_AbilMODBCAN==1)  
    ChkTimer(&StableValueChangeHMI);
#endif    
*/            
    if(TimerPwrLed.TimeOut)
    {
        TimerPwrLed.TimeOut = 0;
        oPwrLed = !oPwrLed;
        //if(iDigiIn1)				
        if(InputProgrammingMode)				
            oErrLed = !oPwrLed;
    }
     

    AD_Read_Sched();      // Era nel Main
      
/*    
    if(++mSecDivider>=20) // Ogni 20mS
    {
		mSecDivider = 0;        
        //AD_Read();      // Era nel Main
        //AD_ReadSingle(1);
        _AD1IE = 1;             // abilito interrupt AD
        _ASAM = 1;		        // Start conversioni sotto interrupt
    }   
*/
    
#if (K_AbilMODBCAN==1)              
    //VIGASHOT Shot(ModbCAN.ChangedValueFromHMI, &ModbCAN.ShotMaskFlag, &ModbCAN.ShotMaskCnt, 3000);   
#endif  //#if (K_AbilMODBCAN==1)       
        
    HalfSecDivider +=1;					// gestisco la base tempo di 1 secondo
	if(HalfSecDivider >=500)            // timer 1/2 sec
	{
		HalfSecDivider = 0;
        if(EngineBox[0].DisplayDigitBlink)
        {
            if(DspToggle)
                PutSegment(EngineBox[0].DisplayDigit, EngineBox[0].DisplayDigitDP);         
            else
                PutSegment(0, 0);         
            DspToggle = !DspToggle;
        }        
    }
     
	SecDivider +=1;                     // gestisco la base tempo di 1 secondo
	if(SecDivider >=1000)				// timer 1 sec
	{
		SecDivider = 0;
		// Gestisco i timer in sec     
		DividerSecondi.TimeOut =0;
        ChkTimer(&LCD_ScreeScheduler);
		ChkTimer(&PausaCompressore);
		ChkTimer(&TimerElaborazioneC1);
		ChkTimer(&TimerElaborazioneC2);
		//ChkTimer(&TimerResetErrori);
		ChkTimer(&TmrLastRequest);
		ChkTimer(&TimerErrCompressorHiC1);
		ChkTimer(&TimerErrCompressorHiC2);
		ChkTimer(&TimerErrCompressorLo);
		ChkTimer(&TimerErrCondensatoreHi);
		ChkTimer(&TimerErrCondensatoreLo);
		ChkTimer(&TimerErrFlussostato);
		ChkTimer(&TimerErrGasRecovery);
		ChkTimer(&TimerResErrCompressorHiC1);
		ChkTimer(&TimerResErrCompressorHiC2);		
		ChkTimer(&TimerResErrCompressorLo);
		ChkTimer(&TimerResErrCondensatoreHi);
		ChkTimer(&TimerResErrCondensatoreLo);
		ChkTimer(&TimerErrPressureLo);
		ChkTimer(&TimerErrPressureHi);
		ChkTimer(&TimerResTime_Gas_Rec);
#if (K_AbilRechargeOil==1)		
		ChkTimer(&TimerWorkRechargeOil1);
		ChkTimer(&TimerPauseRechargeOil1);
		ChkTimer(&TimerAlarmRechargeOil1);	
        ChkTimer(&TimerLowCriticalAlarmRechargeOil1);			
	#if (K_AbilCompressor2==1)		
		ChkTimer(&TimerWorkRechargeOil2);	
		ChkTimer(&TimerPauseRechargeOil2);
		ChkTimer(&TimerAlarmRechargeOil2);		
        ChkTimer(&TimerLowCriticalAlarmRechargeOil2);			
	#endif	
#endif		
        ChkTimer(&TimerHeatingEngine1);
        ChkTimer(&TimerHeatingEngine2);
        ChkTimer(&TimerPostHeatingEngine);
		ChkTimer(&TimerSmBox1);
		ChkTimer(&TimerSmBox2);
		
		ChkTimer(&TimerSwitchCompressor);      	
		ChkTimer(&TimerIntegrateCompressor2);
		ChkTimer(&TimerDeintegrateCompressor2);
		ChkTimer(&TimerHwReset);
		ChkTimer(&TimerRestartAfterBlackOut);
        ChkTimer(&TimerSwitchPump);
        
        ChkTimer(&TimerDefrostingCycleWork);
        ChkTimer(&TimerDefrostingCycleStop);
        ChkTimer(&TimerDefrostingCycleCheck);
   
        ChkTimer(&TimerValve_On_Demand_Limit);
        ChkTimer(&TimerValve_On_Demand_Switch);
        ChkTimer(&TimerCoolingSuperHeatErr);
		ChkTimer(&TimerCondenserFouled); 
        
        ChkTimer(&TimerCoolWarmValveError); 

        ChkTimer(&TimerInverterPersError_C1); 
        ChkTimer(&TimerInverterPersError_C2); 
        ChkTimer(&TimerInverterPersError_Pump); 
        
        ChkTimer(&TimerSuperHeatError);

        ChkTimer(&TimerTemperatureProbeErrorBox1);
        ChkTimer(&TimerTemperatureProbeErrorBox2);
        
        ChkTimer(&TimerPressureProbeErrorBox);

        ChkTimer(&TimerCheckTemperatureProbe);
        ChkTimer(&TimerCheckPressureProbe);
        
        ChkTimer(&TimerAcceleration);     

        ChkTimer(&TimerExitProgrammingMode);

        ChkTimer(&TimerInVoltageInverterOk_C1);
        ChkTimer(&TimerInVoltageInverterOk_C2);
        
		
		RefreshRTC(&MyRTC);
		ComunicationLost = TmrLastRequest.TimeOut;     
	}

    /*
	ComunicationLost = TmrLastRequest.TimeOut;
	Led_Divider +=1;
	if(ComunicationLost)
	{	if(Led_Divider > 50)
		{	
            oPwrLed = !oPwrLed;
			Led_Divider = 0;
        }
	}
	else
	{	if(Led_Divider > 200)
		{	oPwrLed = !oPwrLed;
			Led_Divider = 0;
		}
	}
    */
    
	//IFS0bits.T1IF = 0;				// resetto il flag di interrupt
	return;        
}

#if (K_AbilMODBCAN==1)               
//---------------------------------------------------------------------------------
//void interrupt TMR_2(void) @ T2_VCTR (interrupt ogni 1mS)
//---------------------------------------------------------------------------------
 void __attribute__((interrupt, no_auto_psv)) _T2Interrupt()
{
    // DEBUG
    //oRTS_UART3 = !iRTS_UART3;
    _T2IF = 0;                      // resetto il flag di interrupt    
} 
 
//---------------------------------------------------------------------------------
//void interrupt TMR_3(void) @ T3_VCTR (interrupt ogni 1Sec. -> vedi "Init_Timer3")
//---------------------------------------------------------------------------------
 void __attribute__((interrupt, no_auto_psv)) _T3Interrupt()
{
    //_T3IF = 0;				// resetto il flag di interrupt
    
    static int SecCnt=0;
    //static int SecCnt2=0;
    int Reply;
    
    
//    while(oRTS_Master || oRTS_Slave || MyBUS_BufferTx.Valid_Data==1 || Mod_BufferTx.Valid_Data==1 || 
//          iPckModRx!=0 || iPckRx!=0)
//        continue;
    
#if (K_AbilMODBCAN==1)  
    ChkTimer(&StableValueChangeHMI);
#endif    
    
    
    
    if(Check_IfModbCANGoOnline(&ModbCAN)==1)
        Init_ModbCAN();            						// Controlla se presente CAN Converter e lo inizializza  
    
    if(ModbCAN.OnLine && ModbCAN.Ready)				// timer 1 sec
	{    
        Get_DataFromModbCAN();
        Send_DataToModbCAN();
        Send_AliveCANFrame_ToModbCAN();
    }
            
    
    
    /*
    if(ModbCAN.Enable)                            // timer 1 sec
	{    
        
        Reply = Check_IfModbCANIsPresent();
        if(Reply!=0)
        {            
            if(ModbCAN.Ready==0)
            {
                Init_ModbCAN();            						// Controlla se presente CAN Converter e lo inizializza  
                ModbCAN.Ready=1;
            }                
            
            //if(Reply==1)
            Get_DataFromModbCAN();
            Send_DataToModbCAN();
            Send_AliveCANFrame_ToModbCAN();        
        }                 
        else
        {
            ModbCAN.Ready=0;
        }
        
            
        //SecCnt=0;
        //VIGATRAPPOLA
        //if(SecCnt==1)
        //    Stop_TMR3();
        //VIGATRAPPOLA
    }    
    */
    
    
    // DEBUG
    //oRTS_UART3 = !iRTS_UART3;
    _T3IF = 0;				// resetto il flag di interrupt    
} 

//---------------------------------------------------------------------------------
//void interrupt TMR_4(void) @ T4_VCTR (interrupt ogni 20mS)
//---------------------------------------------------------------------------------
 void __attribute__((interrupt, no_auto_psv)) _T4Interrupt()
{
     ////AD_Read();      // Era nel Main    

////    AD_Read_Sched();      // Era nel Main              

    // DEBUG
    //oRTS_UART3 = !iRTS_UART3;
    _T4IF = 0;				// resetto il flag di interrupt
} 
 
//---------------------------------------------------------------------------------
//void interrupt TMR_5(void) @ T5_VCTR (interrupt ogni 250mS)
//---------------------------------------------------------------------------------
 void __attribute__((interrupt, no_auto_psv)) _T5Interrupt()
{
    // DEBUG
    //oRTS_UART3 = !iRTS_UART3;
    _T5IF = 0;				// resetto il flag di interrupt        
}  
#endif //#if (K_AbilMODBCAN==1)   
 
 
void GlobalIntReadBus(void)
{
	int RxChar = 0; 
    
	while(U1STAbits.URXDA)                          // Receive Buffer Data Available bit (read-only)
	{       
        RxChar = U1RXREG;
        if(CurUsedProtocolId == FlagMyBusProt_BUSM)
            IntReadBus(&BufferRx, RxChar);
        else if (CurUsedProtocolId == FlagModBusProt_BUSM)
            Mod_IntReadBus(RxChar);
    }
}