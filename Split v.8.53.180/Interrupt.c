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
#if (GB210==1) 
	#include <p24FJ256GB210.h>
#elif (DA210==1)
	#include <p24FJ256DA210.h>
#else
	#include <p24FJ256GB110.h>
#endif
#include <stdio.h>
#include <libpic30.h>
//#include "ConfigDevice.h"
#include "DefinePeriferiche.h"
//#include "ADC.h"
//#include "Timer.h"
#include "Usart.h"
//#include "delay.h"
//#include "I2C.h"
//#include "EEPROM.h"
//#include "PWM.h"
#include "Valvola_PassoPasso.h"
#include "Driver_Comunicazione.h"
#include "Driver_ComunicazioneSec.h"
#include "Driver_ModBus.h"
#include "Driver_ModBusSec.h"
#include "ProtocolloComunicazione.h"
#include "ProtocolloComunicazioneSec.h"
#include "ProtocolloModBus.h"
#include "ProtocolloModBusSec.h"
//#include "Core.h"
//#include "PID.h"
#include "FWSelection.h"
//#include "i2c_LCD_Driver.h"
//#include "LCD_ScreenInfo.h"
#include "Interrupt.h"
#include "ADC.h"

#ifdef UART3_ON  
#include "ServiceComunication.h"
#endif

volatile int SecDivider = 0;                        // divisore per base tempo di 1 secondo
volatile int mSecDivider = 0;                       // divisore per base tempo di 100 mSec

extern unsigned char CurUsedProtocolId;             // Id del protocollo correntemente utilizzato
// timer	
extern volatile TypTimer TmrLastRequest;
extern volatile TypTimer TimerStartUp;				// Timer StartUp
extern volatile TypTimer RefreshNetBRead;
extern volatile TypTimer TimerSystemCycle;			// Timer per il calcolo del tempo di ciclo programma
extern volatile TypTimer TimerComunicationError;    // Timer per gestione counter errori di comunicazione
extern volatile TypTimer TimerBusMergeMode;         // Timer per gestione Timeout funzione BUSMerge

extern volatile TypTimer TimerToggleComunication_Sec;
extern volatile TypTimer TimerCheckProbe;

IncludeVariabiliGlobaliDriverModBus();
IncludeVariabiliGlobaliDriverModBusSec();

//extern volatile int SecDivider;                     // divisore per base tempo di 1 secondo
extern unsigned ComunicationLost;                   // flag di comunicazione persa
extern int Led_Divider;                             // divisore per il lampeggio del led
extern volatile unsigned int iPckModRx;

extern unsigned int ForceU1MyBUS;                   // Flag per gestire la forzatura del solo protocollo MyBUS sul canale primario (UART1) alla prima ricezione MyBUS valida


void ErrorLedBlink(void);

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
	_AD1IF = 0;             // ripulisco il flag di interrupot dell'ADC

    AD_Read();
    
    _AD1IE = 0;             // disabilito interrupt AD    
}


//---------------------------------------------------------------------------------
//void interrupt UART2_Tx(void) @ U2TX_VCTR         Interrupt buffer TX UART2 empty:
//---------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _U2TXInterrupt()	
{
	IFS1bits.U2TXIF = 0;									// Clear interrupt flag TX Uart2 

    while(U2STAbits.UTXBF)                                  // attendo che il buffer di scrittura sia libero
		continue;

	//oRTS_Slave = 0;											// abbasso il pin dell'rts x la 485                 
    
    switch (CurUsedProtocolId)
    {
        case FlagMyBusProt:                                 // utilizzo il protocollo MyBUS 
            if(MyBUS_BufferTxSec.CntByte < MyBUS_BufferTxSec.nByte)			// se il buffer TX contiene ancora byte da inviare....
            {
                MyBUS_BufferTxSec.CntByte++;								// incremento n. byte inviati
                //oRTS_Slave = 1;                                             // alzo il pin dell'rts x la 485                                                
                U2TXREG = MyBUS_BufferTxSec.Buffer[MyBUS_BufferTxSec.CntByte-1];  // scrivo il carattere nel registro di trasmissione
            }
            else
            {
                MyBUS_BufferTxSec.Valid_Data = 0;
                oRTS_Slave = 0;											// abbasso il pin dell'rts x la 485                                 
            }
		break;
        
        case FlagModBusProt:                                            // utilizzo il protocollo ModBus
            if(Mod_BufferTxSec.CntByte < Mod_BufferTxSec.nByte)               // se il buffer TX contiene ancora byte da inviare....
            {
                Mod_BufferTxSec.CntByte++;                                 // incremento n. byte inviati
                //oRTS_Slave = 1;                                         // alzo il pin dell'rts x la 485                                                
                U2TXREG = Mod_BufferTxSec.Buffer[Mod_BufferTxSec.CntByte-1];  // scrivo il carattere nel registro di trasmissione
            }
            else
            {
                Mod_BufferTxSec.Valid_Data = 0;
                oRTS_Slave = 0;											// abbasso il pin dell'rts x la 485                                                 
            }
        break;
    }
    
    
    //IFS1bits.U2TXIF = 0;									// Clear interrupt flag TX Uart2 
	//????? return;
}


//---------------------------------------------------------------------------------
//void interrupt UART1_Rx(void) @ U1RX_VCTR
//---------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt()
//void __attribute__((interrupt)) _U1RXInterrupt()
{
    int rxchar;
    
	IFS0bits.U1RXIF = 0;			// Clear interrupt flag Uart1 
    
    if(Me.GoBusMergeMode)                       // Se sono in modalità "BUS Merge"
    {
        IFS0bits.U1RXIF = 0;                    // Clear interrupt flag Uart1                 
        rxchar = U1RXREG;        
#if(K_MyBus==0)
            putch2(rxchar);                         // Invio sul BUS2 il carattere ricevuto sul BUS1        
#else
            putch3(rxchar);                         // Invio sul BUS3 il carattere ricevuto sul BUS1        
#endif
        TimerBusMergeMode.Value = 0;
        return;        
    } 
    
    GlobalIntReadBus();    
    
    
	Recive_Cmd(&BufferRx, &LastRx);                 // MyBUS
    if(ForceU1MyBUS==0)
        Mod_Recive_Cmd();                           // MODBUS
/*    
    if(LastRx.Valid_Data)                           // Se ho ricevuto un dato valido su MyBUS
    {                                               // Resetto ricezione in corso per MODBUS
        TmrLastRequest.Value = 0;                   // Comunication Lost...
        TmrLastRequest.TimeOut = 0;
    }
    
    if(ForceU1MyBUS==0)                                 // Se non ho il flag di forzatura del solo protocollo MyBUS....
    {
        if(Mod_LastRx.Valid_Data)                       // Se ho ricevuto un dato valido su MODBUS
        {                                               // Resetto ricezione in corso per MyBUS
            TmrLastRequest.Value = 0;                   // Comunication Lost...
            TmrLastRequest.TimeOut = 0;
        }  
    }
*/    
#if(K_EnableAutoSwithU1MyBUS==1)                    // Se ho il flag di forzatura protocollo abilitato...
    if(LastRx.Valid_Data == 1)                      // Se ho ricevuto un pacchetto MyBUS valido ed indirizzato a me
        ForceU1MyBUS=1;                             // Alzo il flag di forzatura del solo protocollo MyBUS (escludendo così il MODBUS fino al prossimo ComunicationLost)
#endif
    
    
	Process_Cmd(&LastRx);                           // MyBUS
    if(ForceU1MyBUS==0)                             // Se non ho il flag di forzatura del solo protocollo MyBUS....
        Mod_Process_Cmd();                          // MODBUS
    
    //IFS0bits.U1RXIF = 0;			// Clear interrupt flag Uart1 

    //????? return;

}



//---------------------------------------------------------------------------------
//void interrupt UART2_Rx(void) @ U2RX_VCTR
//---------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt()	
//void __attribute__((interrupt)) _U2RXInterrupt()
{
    char rxchar;
    
	IFS1bits.U2RXIF = 0;				// Clear interrupt flag Uart2 

#if(k_Abil_Panel_Shared==1&&K_Panel_On_Uart3==0)  
    if(WaitingReplaySec==0)    
    {
        Me.EnableCommunication_Sec=0;
        TimerToggleComunication_Sec.Value = 0;
        TimerToggleComunication_Sec.Time = Me.CycleTime+100; //650//1000;
    }
#endif

#if(K_MyBus==0)
    if(Me.GoBusMergeMode)                       // Se sono in modalità "BUS Merge"
    {
        IFS1bits.U2RXIF = 0;				// Clear interrupt flag Uart2          
        rxchar = U2RXREG;
        putch1(rxchar);                         // Invio sul BUS1 il carattere ricevuto sul BUS2                                                
        TimerBusMergeMode.Value = 0;       
        return;        
    } 
#endif
    
	switch (CurUsedProtocolId)			// controlo il protocollo che sto utilizzando
	{
		case FlagMyBusProt:				// utilizzo il mio protocollo
			iPckModRxSec = 0;
			IntReadBusSec(&BufferRxSec);
			break;
		case FlagModBusProt:			// utilizzo il protocollo ModBus
			Mod_IntReadBusSec();
			break;
	}
	
	//Recive_Cmd_Sec(&BufferRxSec, &LastRxSec);
	//Mod_Recive_CmdSec();

    //IFS1bits.U2RXIF = 0;				// Clear interrupt flag Uart2 
    
	//????? return;
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
 
	IFS4bits.U1ERIF = 0;
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
//void interrupt UART2 Error @ U2E_VCTR
//---------------------------------------------------------------------------------
// void _ISR __attribute__((no_auto_psv)) _U2ErrInterrupt()
 void __attribute__((interrupt, no_auto_psv)) _U2ErrInterrupt()
 {
	unsigned char errframData;
//	static unsigned int erroverrun2 = 0;	// solo x DEBUG
//	static unsigned int errframing2 = 0;	// solo x DEBUG
 
	IFS4bits.U2ERIF = 0;
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


#ifdef UART3_ON
//---------------------------------------------------------------------------------
//void interrupt UART3_Rx(void) @ U3RX_VCTR
//---------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _U3RXInterrupt()
//void __attribute__((interrupt)) _U1RXInterrupt()
{
    char rxchar;
    
	IFS5bits.U3RXIF = 0;			// Clear interrupt flag Uart3

#if(k_Abil_Panel_Shared==1&&K_Panel_On_Uart3==1)  
    if(WaitingReplaySec==0)    
    {
        Me.EnableCommunication_Sec=0;
        TimerToggleComunication_Sec.Value = 0;
        TimerToggleComunication_Sec.Time = Me.CycleTime+100; //650//1000;
    }
#endif

#if(K_MyBus==1)
    if(Me.GoBusMergeMode)               // Se sono in modalità "BUS Merge"
    {
        _U3RXIF = 0;                    // Clear interrupt flag Uart3          
        rxchar = U3RXREG;
        putch1(rxchar);                 // Invio sul BUS1 il carattere ricevuto sul BUS3                                               
        TimerBusMergeMode.Value = 0;       
        return;        
    } 
#endif
    
    if(Me.My_Address==0)
        IntReadServiceBusU3(&ServiceBusBufferRxU3);
    else    
    {
        switch (CurUsedProtocolId)			// controlo il protocollo che sto utilizzando
        {
            case FlagMyBusProt:				// utilizzo il mio protocollo
                iPckModRxSec = 0;
                IntReadBusSec(&BufferRxSec);
                break;
            case FlagModBusProt:			// utilizzo il protocollo ModBus
                Mod_IntReadBusSec();
                break;
        }
    }
    /*
    if(Me.My_Address==0)
        IntReadServiceBusU3(&ServiceBusBufferRxU3);
    else    
        Mod_IntReadBusSec();
    */
}

 
//---------------------------------------------------------------------------------
//void interrupt UART3_Tx(void) @ U3TX_VCTR         Interrupt buffer TX UART3 empty:
//---------------------------------------------------------------------------------
void __attribute__((interrupt, no_auto_psv)) _U3TXInterrupt()	
{
    unsigned int chtx;
    
	IFS5bits.U3TXIF = 0;									// Clear interrupt flag TX Uart3

    while(U3STAbits.UTXBF)                                  // attendo che il buffer di scrittura sia libero
		continue;

	//oRTS_UART3 = 0;											//!!! // abbasso il pin dell'rts x la 485                 

    if(Me.ComToUart3==0 || Me.My_Address==0)
    {    
        if(ServiceBusBufferTxU3.CntByte < ServiceBusBufferTxU3.nByte)               // se il buffer TX contiene ancora byte da inviare....
        {
            ServiceBusBufferTxU3.CntByte++;                                           // incremento n. byte inviati
            chtx = ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.CntByte-1];

            U3TXREG = (int)chtx;        
        }
        else
        {
            ServiceBusBufferTxU3.Valid_Data = 0;                      // Segnalo fine TX Buffer
            oRTS_UART3 = 0;											// abbasso il pin dell'rts x la 485        
        }
    }
    else   // utilizzo il protocollo ModBus
    {   

        switch (CurUsedProtocolId)
        {
            case FlagMyBusProt:                                 // utilizzo il protocollo MyBUS 
                if(MyBUS_BufferTxSec.CntByte < MyBUS_BufferTxSec.nByte)			// se il buffer TX contiene ancora byte da inviare....
                {
                    MyBUS_BufferTxSec.CntByte++;								// incremento n. byte inviati
                    //oRTS_Slave = 1;                                             // alzo il pin dell'rts x la 485                                                
                    U3TXREG = MyBUS_BufferTxSec.Buffer[MyBUS_BufferTxSec.CntByte-1];  // scrivo il carattere nel registro di trasmissione
                }
                else
                {
                    MyBUS_BufferTxSec.Valid_Data = 0;
                    oRTS_UART3 = 0;											// abbasso il pin dell'rts x la 485                                 
                }
            break;

            case FlagModBusProt:                                            // utilizzo il protocollo ModBus
                if(Mod_BufferTxSec.CntByte < Mod_BufferTxSec.nByte)               // se il buffer TX contiene ancora byte da inviare....
                {
                    Mod_BufferTxSec.CntByte++;                                 // incremento n. byte inviati
                    //oRTS_UART3 = 1;                                         // alzo il pin dell'rts x la 485                                                
                    U3TXREG = Mod_BufferTxSec.Buffer[Mod_BufferTxSec.CntByte-1];  // scrivo il carattere nel registro di trasmissione
                }
                else
                {
                   Mod_BufferTxSec.Valid_Data = 0;
                   oRTS_UART3 = 0;											// abbasso il pin dell'rts x la 485        
                }
            break;
        }
    }    
}


//---------------------------------------------------------------------------------
//void interrupt UART3 Error @ U3E_VCTR
//---------------------------------------------------------------------------------
 void __attribute__((interrupt, no_auto_psv)) _U3ErrInterrupt()
 {
	unsigned char errframData;
 
	IFS5bits.U3ERIF = 0;
	if(U3STAbits.OERR)
	{
	    U3STAbits.OERR = 0; //CLEAR THE OVERRUN ERROR BIT    
	}
    else if(U3STAbits.FERR)
	{
	        U3STAbits.FERR = 0;
	        errframData=U3RXREG ; // flush buffer
	}  
   	//IFS5bits.U3ERIF = 0;
 }

#endif
 
 


//---------------------------------------------------------------------------------
//void interrupt TMR_1(void) @ T1_VCTR (interrupt ogni 1mS)
//---------------------------------------------------------------------------------
//void __attribute__((interrupt, shadow)) _T1Interrupt()
//void __attribute__((interrupt)) _T1Interrupt()
 void __attribute__((interrupt, no_auto_psv)) _T1Interrupt()
{
     
    IFS0bits.T1IF = 0;				// resetto il flag di interrupt     
    
    // ARGOVIGA
    // oRTS_UART3 = 1;      // per Debug
    
     
	// Gestisco i timer in ms
	IntTimeOutRx();					// gestisco il time out del protocollo MyBUS UART1
	IntTimeOutRxSec();				// gestisco il time out del protocollo MyBUS UART2
	Mod_IntTimeOutRx();             // gestisco il time out del protocollo MODBUS UART1
	Mod_IntTimeOutRxSec();			// gestisco il time out del protocollo MODBUS UART2
    
    /*
	Process_Cmd(&LastRx);                           // MyBUS
    if(ForceU1MyBUS==0)                             // Se non ho il flag di forzatura del solo protocollo MyBUS....
        Mod_Process_Cmd();                          // MODBUS    
    */
    
	Recive_Cmd_Sec(&BufferRxSec, &LastRxSec);
	Mod_Recive_CmdSec();    
    
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
    
	//TachoInterrupt();				// gestisco il conteggio per la velocità del ventilatore
	ChkTimer(&TimeOutPktRxSec);
	ChkTimer(&TimeOutModPktRxSec);
	ChkTimer(&TimerValvola);
	ChkTimer(&RefreshVentil);
	ChkTimer(&RefreshNetBRead);	// Timer per la lettura temporizzata dei valori da NetB NetBuilding
	ChkTimer(&TimerExpValvola);		// Gestione Expansione valvola
	ChkTimer(&TimerSystemCycle);
	ChkTimer(&TimerExecPID);

#if(k_Abil_Panel_Shared==1)
    ChkTimer(&TimerToggleComunication_Sec);
    
    if(TimerToggleComunication_Sec.TimeOut)
    {
        TimerToggleComunication_Sec.TimeOut = 0;
        Me.EnableCommunication_Sec=1;
        TimerToggleComunication_Sec.Value = 0;
        TimerToggleComunication_Sec.Time = Me.CycleTime+1500; //2000 //5000;
    }
#endif
    
    
    static int ExecValveCnt=0;
    static int MaxCnt;

    if(((EngineBox.RealFuncMode==EngineBox_GoOff)&&(EngineBox.Compressor_Speed==0) && (EngineBox.Compressor_Speed_C2==0)) || (EngineBox.Error.Pressure_Hi==1) || (Me.My_Address==0))    
    {
        Me.ExpValve_Speed_Mode = K_Valve_Hi_Speed;
        MaxCnt = K_ExecValve_Cnt_HiSpeed;   // 0 Full Speed
    }
    else
    {
        Me.ExpValve_Speed_Mode = K_Valve_Low_Speed;
        MaxCnt = K_ExecValve_Cnt_LoSpeed;   // 2
    }
    
    if(ExecValveCnt++ >= MaxCnt )
    {
        RefreshValue();					// aggiorna la valvola
        ExecValveCnt=0;
    }
    
    //ARGOPUZZONE
    /*
    static int tmp=0;
    
    if(tmp++>=1)
    {    
        oRTS_UART3 = 1;
        tmp=0;
    }
    else
        oRTS_UART3 = 0;
*/    

	SecDivider +=1;					// gestisco la base tempo di 1 secondo
	if(SecDivider > 999)			// timer 1 sec
	{
		SecDivider = 0;
		// Gestisco i timer in sec
        if(Me.Ventil_Selection!=K_ZIEHL_ABEGG_ModBus && Me.Ventil_Selection!=K_EBM_3200RPM_ModBus)
            TachoInterrupt();			// gestisco il conteggio per la velocità del ventilatore        
        ChkTimer(&LCD_ScreeScheduler);
		ChkTimer(&TmrLastRequest);	// gestisco il timer per la segnalazione del flag comunication lost
		ChkTimer(&TimerStartUp);	// timer per l'attesa inaziale.
		ChkTimer(&RefreshFineVentil);
		ChkTimer(&TimerTestEEV);
		ChkTimer(&TimerSpeedChangeCoreOff);
		ChkTimer(&TimerRestartValve);		
		ChkTimer(&TimerDisableValve);
        ChkTimer(&TimerResyncValvola);		
        ChkTimer(&TimerCheckFanTacho);
		ChkTimer(&TimerCheckBatteryDefrost);
        ChkTimer(&TimerFanVelSwitch);        
		//RefreshRTC(&MyRTC);
        ChkTimer(&TimerComunicationError);     
        ChkTimer(&TimerBusMergeMode);
        ChkTimer(&TimerExpValv_Err);
        
        ChkTimerLong(&TimerDefrostingCycleWork);
        ChkTimer(&TimerDefrostingCycleStop);
        ChkTimer(&TimerDefrostingCycleCheck);
        ChkTimer(&TimerOnValveFrozen);
        ChkTimer(&TimerCorrectSuperHeatAirOut);    
        
        ChkTimer(&TimerTemperatureFault);
        
        ChkTimer(&TimerFloodFanCoil);
        
        ChkTimer(&TimerGasLeaks);
        
        ChkTimer(&TimerTemperatureProbeError);
                
        ChkTimer(&TimerCheckProbe);
        
        ChkTimer(&TimerDampersAlarm);
        
        //ChkTimer(&TimerMaxTimeCmdAddress);
        
        ChkTimer(&TimerErrFlow);        
        
        ChkTimerLong(&TimerSelectPumpWork);
        
        ChkTimer(&TimerOutWaterStop);

        ChkTimer(&TimerHeaterFresh);
	}

	ComunicationLost = TmrLastRequest.TimeOut;
	Led_Divider +=1;
	if(ComunicationLost)
	{	if(Led_Divider > 50)
		{	
            oPwrLed = !oPwrLed;
			Led_Divider = 0;
        }
        set_baudrateU1_57600();         
        ForceU1MyBUS=0;                 // Resetto il flag di forzatura del solo protocollo MyBUS (riabilitando così anche il protocollo MODBUS su UART1)
	}
	else
	{	if(Led_Divider > 200)
		{	oPwrLed = !oPwrLed;
			Led_Divider = 0;
		}
	}

    
    ErrorLedBlink();
    
    
    // ARGOVIGA
    // oRTS_UART3 = 0;   // per Debug

    
    //IFS0bits.T1IF = 0;				// resetto il flag di interrupt     
    
	//????? return;
}


 
void ErrorLedBlink(void) 
{    
    static int mSecCounter=0;
    static int ErrorSched=0;
    static int LEDTimeToggle=0;
    static int ErrLampCnt=0;
    static int ErrLampFlash=0;
    static int ErrTimePause=2000;
    
    
    //ErrLampFlash = 0;
    

    if(mSecCounter >= ErrTimePause)
    {    
        switch(ErrorSched)
        {
            case 0: 
                    if(Me.ErrorLedBlink.U1_MyBUS != 0)
                    {
                        ErrLampFlash=1;
                        Me.ErrorLedBlink.U1_MyBUS = 0;
                        break;   
                    }
                    else
                    {
                        ErrorSched++;
                    }

            case 1: 
                    if(Me.ErrorLedBlink.U2_MyBUS != 0)
                    {
                        ErrLampFlash=2;
                        Me.ErrorLedBlink.U2_MyBUS = 0;
                        break;   
                    }
                    else
                    {
                        ErrorSched++;
                    }

            case 2: 
                    if(Me.ErrorLedBlink.U1_ModBUS != 0)
                    {
                        ErrLampFlash=3;
                        Me.ErrorLedBlink.U1_ModBUS = 0;
                        break;   
                    }
                    else
                    {
                        ErrorSched++;
                    }

            case 3: 
                    if(Me.ErrorLedBlink.U2_ModBUS != 0)
                    {
                        ErrLampFlash=4;
                        Me.ErrorLedBlink.U2_ModBUS = 0;
                        break;   
                    }
                    else
                    {
                        ErrorSched++;
                    }

            case 4: 
                    if(Me.ErrorLedBlink.CumulativeAlarm != 0)
                    {
                        ErrLampFlash=5;
                        Me.ErrorLedBlink.CumulativeAlarm = 0;
                        break;   
                    }
                    else
                    {
                        ErrorSched++;
                    }  

            case 5: 
                    if(Me.Error.EEPROM_Retention != 0 || Me.Error.EEPROM_ReadWrite != 0 || Me.Error.EEPROM_Cell != 0)
                    {
                        ErrLampFlash=6;
                        break;   
                    }
                    else
                    {
                        ErrorSched++;
                    }                
            //break;   

        }
 
        ErrorSched++;
        if(ErrorSched>5) 
            ErrorSched=0;

        
        if(ErrLampFlash==0)
            mSecCounter=ErrTimePause;
        else
            mSecCounter = 0;
        
        ErrLampCnt=0;
    }
    
    if(++LEDTimeToggle > 250)
    {
        if(++ErrLampCnt < ErrLampFlash*2)
            oErrLed=!oErrLed;
        else
        {
            oErrLed=LED_OFF;
            ErrLampFlash=0;
        }
        LEDTimeToggle=0;
    }

    if(ErrLampFlash==0)
        mSecCounter++;        

}