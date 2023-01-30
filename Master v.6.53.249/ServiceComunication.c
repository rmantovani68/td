//----------------------------------------------------------------------------------
//	Progect name:	ServiceComunication.c
//	Device:			PIC24FJ256GB210 @ 16Mhz (FRCPLL)
//	Autor:			TDM Team
//	Date:			31/12/2017
//	Description:	Gestione Protocolli Asiliari per Debug su UART3
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
#include "DefinePeriferiche.h"
#include "Timer.h"
#include "Driver_Comunicazione.h"
#include "ProtocolloComunicazione.h"
#include "usart.h"
#include "FWSelection.h"
#include "HWSelection.h"
#include "delay.h"
#include "ServiceComunication.h"



//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Cambia il protocollo corrente con quello specificato: (compreso BaudRate)
// ProtocolIndex: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
//------------------------------------------------------------------------------
void Change_BaudeUART3(int Baudindex)
{
	oRTS_UART3 = 0;

	switch(Baudindex)
	{
		case 0:
			set_baudrateU3_38400();				// @ 38400Bps
            DelaymSec(10); 
		break;   
        
		case 1:
			set_baudrateU3_57600(); 				// @ 57600Bps
            DelaymSec(10); 
		break;

		case 2:
			set_baudrateU3_115200(); 			// @ 115200Bps
            DelaymSec(10); 
		break;
	
		default:
			set_baudrateU3_57600();				// @ 57600Bps
            DelaymSec(10); 
        break;
			
	}		
	
	ClearUart3Err();
}



void PutStringToUART3(int Val1, int Val2, int Val3, int Val4, int Val5, int Val6, unsigned int Val7)
{
	static int cnt = 0;
	unsigned char txm[32];	
	int n,x;				
	
//	Change_BaudeUART3(0);			// Imposta Baud LowSpeed -> 38400Bps
//	Change_BaudeUART3(1);			// Imposta Baud HiSpeed -> 57600Bps
//	Change_BaudeUART3(2);			// Imposta Baud SHiSpeed -> 115200Bps
	
	cnt++;	
	if(cnt==1)	
	{	
		n = sprintf(txm, "CLEARDATA\r\n");   
		for(x=0; x<n; x++) putch3(txm[x]);           

   	
	}
	n = sprintf(txm, "DATA,");
	for(x=0; x<n; x++) putch3(txm[x]);   	
	n = sprintf(txm, "%i, ", Val1);
	for(x=0; x<n; x++) putch3(txm[x]);   
	n = sprintf(txm, "%i, ", Val2);
	for(x=0; x<n; x++) putch3(txm[x]);  
 	n = sprintf(txm, "%i, ", Val3);
	for(x=0; x<n; x++) putch3(txm[x]);    	
 	n = sprintf(txm, "%i, ", Val4);
	for(x=0; x<n; x++) putch3(txm[x]); 
 	n = sprintf(txm, "%i, ", Val5);
	for(x=0; x<n; x++) putch3(txm[x]);
 	n = sprintf(txm, "%i, ", Val6);
	for(x=0; x<n; x++) putch3(txm[x]);    
 	n = sprintf(txm, "%i, ", Val7);
	for(x=0; x<n; x++) putch3(txm[x]);      
 	n = sprintf(txm, "\r\n");
	for(x=0; x<n; x++) putch3(txm[x]); 	
	
	if(cnt==100)	
	{
		cnt=0;
		for(x=0; x<n; x++) putch3(txm[x]);      			 
		n = sprintf(txm, "ROW,SET,2\r\n");
	}	

	
	
}



void SendBufferToUART3(void)
{
	ServiceBusBufferTxU3.Valid_Data = 1; 						// Setto flag di TX buffer in corso...
	ServiceBusBufferTxU3.CntByte = 1;
    
	oRTS_UART3 = 1;											// alzo il pin dell'rts x la 485
    
    _U3TXIF = 0;                                            // Clear interrupt flag TX Uart3 
	_U3TXIE = 1;                                            // Attivo l'interrupt di TX Uart3
	U3TXREG = ServiceBusBufferTxU3.Buffer[0];						// scrivo il primo byte nel registro di trasmissione x iniziare l'invio del bufferTX

	while (ServiceBusBufferTxU3.Valid_Data == 1)				// attendo che il buffer sia completamente inviato..... -> Mod_BufferTx.Valid_Data == 0 vedi interrupt "U2TX_VCTR"
		continue;    

	while(!U3STAbits.TRMT)                                  // attendo la fine della trasmissione
		continue;    
    
	_U3TXIE = 0;                                            // Disattivo l'interrupt di TX Uart3
	//oRTS_UART3 = 0;											// abbasso il pin dell'rts x la 485
 
	return 0;												// ritorno 0 in quanto l'operazione è andata a buon fine
}


void PutDataForUART3(int Val1, int Val2, int Val3, unsigned int Val4, unsigned int Val5, unsigned int Val6, unsigned int Val7)
{
    static int cnt = 0;
    static int ReadSched = 0;
    
    
    cnt++;	
	if(cnt==1)	
	{	    
        ServiceBusBufferTxU3.nByte = 0;
        //ServiceBusBufferTxU3.nByte = sprintf(&ServiceBusBufferTxU3.Buffer[0], "CLEARDATA\r\n");  
        ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CLEARSHEET\r\n"); 
        ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "LABEL,Temp_Small,Temp_Big,Temp_AirOut,Pressure.Gas,Pressure.Liquid,Pressure.LiquidCond,CycleTime,,,,,,,,,Kp,Ki,Kd\r\n");
    }
    else
        ServiceBusBufferTxU3.nByte = 0;
    
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "DATA,");
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%i, ", Val1);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%i, ", Val2);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%i, ", Val3);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%u, ", Val4);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%u, ", Val5);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%u, ", Val6);   
    ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%u, ", Val7); 
    ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "\r\n"); 
        
    if(cnt==100)	
	{
		cnt=1;
        ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "ROW,SET,2\r\n");        
	}	

    switch(ReadSched)
    {
        case 0:
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,P2\r\n");    
            ServiceBusBufferRxU3.PtData = &ServiceBusBufferRxU3.Cella1;
        break;
        case 1:
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,Q2\r\n");    
            ServiceBusBufferRxU3.PtData = &ServiceBusBufferRxU3.Cella2;
        break;
        case 2:
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,R2\r\n");    
            ServiceBusBufferRxU3.PtData = &ServiceBusBufferRxU3.Cella3;
        break;        
    }
    if(++ReadSched>=3) 
        ReadSched=0;
    
    
    SendBufferToUART3();					// trasmetto la richiesta
}




//------------------------------------------------------------------------------
// Cambia il protocollo corrente con quello specificato: (compreso BaudRate)
// ProtocolIndex: 0=MyBus@115000Baud, 1=MyBus@57600Baud, 2=ModBus@38400Baud, 3=ModBus@57600Baud, 4=ModBus@115000Baud
//------------------------------------------------------------------------------
void Change_BaudeUART4(int Baudindex)
{
	oRTS_UART4 = 0;

	switch(Baudindex)
	{
		case 0:
			set_baudrateU4_38400();				// @ 38400Bps
            DelaymSec(10); 
		break;   
        
		case 1:
			set_baudrateU4_57600(); 				// @ 57600Bps
            DelaymSec(10); 
		break;

		case 2:
			set_baudrateU4_115200(); 			// @ 115200Bps
            DelaymSec(10); 
		break;
	
		default:
			set_baudrateU4_57600();				// @ 57600Bps
            DelaymSec(10); 
        break;
			
	}		
	
	ClearUart4Err();
}



void PutStringToUART4(int Val1, int Val2, int Val3, int Val4, int Val5, int Val6, unsigned int Val7)
{
	static int cnt = 0;
	unsigned char txm[32];	
	int n,x;				
	
//	Change_BaudeUART3(0);			// Imposta Baud LowSpeed -> 38400Bps
//	Change_BaudeUART3(1);			// Imposta Baud HiSpeed -> 57600Bps
//	Change_BaudeUART3(2);			// Imposta Baud SHiSpeed -> 115200Bps
	
	cnt++;	
	if(cnt==1)	
	{	
		n = sprintf(txm, "CLEARDATA\r\n");   
		for(x=0; x<n; x++) putch4(txm[x]);           

   	
	}
	n = sprintf(txm, "DATA,");
	for(x=0; x<n; x++) putch4(txm[x]);   	
	n = sprintf(txm, "%i, ", Val1);
	for(x=0; x<n; x++) putch4(txm[x]);   
	n = sprintf(txm, "%i, ", Val2);
	for(x=0; x<n; x++) putch4(txm[x]);  
 	n = sprintf(txm, "%i, ", Val3);
	for(x=0; x<n; x++) putch4(txm[x]);    	
 	n = sprintf(txm, "%i, ", Val4);
	for(x=0; x<n; x++) putch4(txm[x]); 
 	n = sprintf(txm, "%i, ", Val5);
	for(x=0; x<n; x++) putch4(txm[x]);
 	n = sprintf(txm, "%i, ", Val6);
	for(x=0; x<n; x++) putch4(txm[x]);    
 	n = sprintf(txm, "%i, ", Val7);
	for(x=0; x<n; x++) putch4(txm[x]);      
 	n = sprintf(txm, "\r\n");
	for(x=0; x<n; x++) putch4(txm[x]); 	
	
	if(cnt==100)	
	{
		cnt=0;
		for(x=0; x<n; x++) putch4(txm[x]);      			 
		n = sprintf(txm, "ROW,SET,2\r\n");
	}	

	
	
}



void SendBufferToUART4(void)
{
	ServiceBusBufferTxU4.Valid_Data = 1; 						// Setto flag di TX buffer in corso...
	ServiceBusBufferTxU4.CntByte = 1;
    
	oRTS_UART4 = 1;											// alzo il pin dell'rts x la 485
    
    _U4TXIF = 0;                                            // Clear interrupt flag TX Uart3 
	_U4TXIE = 1;                                            // Attivo l'interrupt di TX Uart3
	U4TXREG = ServiceBusBufferTxU4.Buffer[0];						// scrivo il primo byte nel registro di trasmissione x iniziare l'invio del bufferTX

	while (ServiceBusBufferTxU4.Valid_Data == 1)				// attendo che il buffer sia completamente inviato..... -> Mod_BufferTx.Valid_Data == 0 vedi interrupt "U2TX_VCTR"
		continue;    

	while(!U4STAbits.TRMT)                                  // attendo la fine della trasmissione
		continue;    
    
	_U4TXIE = 0;                                            // Disattivo l'interrupt di TX Uart3
	//oRTS_UART4 = 0;											// abbasso il pin dell'rts x la 485
    
	return 0;												// ritorno 0 in quanto l'operazione è andata a buon fine
}


void PutDataForUART4(int Val1, int Val2, int Val3, int Val4, int Val5, unsigned int Val6, unsigned int Val7)
{
    static int cnt = 0;
    static int ReadSched = 0;
    
    cnt++;	
	if(cnt==1)	
	{	    
        ServiceBusBufferTxU4.nByte = 0;
        //ServiceBusBufferTxU4.nByte = sprintf(&ServiceBusBufferTxU4.Buffer[0], "CLEARDATA\r\n");  
        ServiceBusBufferTxU4.nByte += sprintf(&ServiceBusBufferTxU4.Buffer[ServiceBusBufferTxU4.nByte], "CLEARSHEET\r\n"); 
                                                                                                        //EngineBox[0].Temperature.Compressor_Output, EngineBox[0].Temperature.Compressor_Suction, EngineBox[0].Temperature.Condenser, EngineBox[0].Pressure.Gas, EngineBox[0].Pressure.Liquid, EngineBox[0].Pressure.LiquidCond, EngineBox[0].SystemCycleTime);
        ServiceBusBufferTxU4.nByte += sprintf(&ServiceBusBufferTxU4.Buffer[ServiceBusBufferTxU4.nByte], "LABEL,Temperature.Compressor_Output,Temperature.Compressor_Suction,Temperature.Condenser,Pressure.Gas,Pressure.Liquid,Pressure.LiquidCond,SystemCycleTime,,,,,,,,,Kp,Ki,Kd\r\n");
    }
    else
        ServiceBusBufferTxU4.nByte = 0;

	ServiceBusBufferTxU4.nByte += sprintf(&ServiceBusBufferTxU4.Buffer[ServiceBusBufferTxU4.nByte], "DATA,");
	ServiceBusBufferTxU4.nByte += sprintf(&ServiceBusBufferTxU4.Buffer[ServiceBusBufferTxU4.nByte], "%i, ", Val1);        
	ServiceBusBufferTxU4.nByte += sprintf(&ServiceBusBufferTxU4.Buffer[ServiceBusBufferTxU4.nByte], "%i, ", Val2);        
	ServiceBusBufferTxU4.nByte += sprintf(&ServiceBusBufferTxU4.Buffer[ServiceBusBufferTxU4.nByte], "%i, ", Val3);        
	ServiceBusBufferTxU4.nByte += sprintf(&ServiceBusBufferTxU4.Buffer[ServiceBusBufferTxU4.nByte], "%i, ", Val4);        
	ServiceBusBufferTxU4.nByte += sprintf(&ServiceBusBufferTxU4.Buffer[ServiceBusBufferTxU4.nByte], "%i, ", Val5);        
	ServiceBusBufferTxU4.nByte += sprintf(&ServiceBusBufferTxU4.Buffer[ServiceBusBufferTxU4.nByte], "%u, ", Val6);   
    ServiceBusBufferTxU4.nByte += sprintf(&ServiceBusBufferTxU4.Buffer[ServiceBusBufferTxU4.nByte], "%u, ", Val7); 
    ServiceBusBufferTxU4.nByte += sprintf(&ServiceBusBufferTxU4.Buffer[ServiceBusBufferTxU4.nByte], "\r\n"); 

    if(cnt==100)	
	{
		cnt=1;
        ServiceBusBufferTxU4.nByte += sprintf(&ServiceBusBufferTxU4.Buffer[ServiceBusBufferTxU4.nByte], "ROW,SET,2\r\n");        
	}	

    /* Argo Vaffa
    switch(ReadSched)
    {
        case 0:
            ServiceBusBufferTxU4.nByte += sprintf(&ServiceBusBufferTxU4.Buffer[ServiceBusBufferTxU4.nByte], "CELL,GET,P2\r\n");    
            ServiceBusBufferRxU4.PtData = &ServiceBusBufferRxU4.Cella1;
        break;
        case 1:
            ServiceBusBufferTxU4.nByte += sprintf(&ServiceBusBufferTxU4.Buffer[ServiceBusBufferTxU4.nByte], "CELL,GET,Q2\r\n");    
            ServiceBusBufferRxU4.PtData = &ServiceBusBufferRxU4.Cella2;
        break;
        case 2:
            ServiceBusBufferTxU4.nByte += sprintf(&ServiceBusBufferTxU4.Buffer[ServiceBusBufferTxU4.nByte], "CELL,GET,R2\r\n");    
            ServiceBusBufferRxU4.PtData = &ServiceBusBufferRxU4.Cella3;
        break;        
    }
    if(++ReadSched>=3) 
        ReadSched=0;
    */
    
    SendBufferToUART4();					// trasmetto la richiesta
}


void IntReadServiceBusU3(volatile TypServiceBusBuffer * Rx_Buff)
{
	char RxData = 0;
    static int RxCharCnt = 0;
    
    if(Rx_Buff->Valid_Data==0)
    {
        while(U3STAbits.URXDA)
        {
            RxData = U3RXREG;

            if(RxData != 10)    // Se non ho ancora ricevuto il carattere di fine stringa....
            {
                Rx_Buff->Buffer[RxCharCnt++] = RxData;

                if(RxCharCnt>=ByteLenServiceBusBuff) 
                    RxCharCnt--;

            }
            else        // ho ricevuto il carattere di fine stringa (10)
            {
                Rx_Buff->Buffer[RxCharCnt++] = RxData;
                if(Rx_Buff->Buffer[0]!=10 && Rx_Buff->Buffer[0]!=32)                  // Se ho ricevuto almeno un carattere al di fuori di 10 ("\n") o di 32 (" " spazio))
                    Rx_Buff->Valid_Data = 1;                // dico che ho ricevuto un dato valido
                RxCharCnt=0;
            }
        }
    }
}


void Process_ServiceU3Cmd(volatile TypServiceBusBuffer * Rx_Buff)
{
 
	if(Rx_Buff->Valid_Data == 1)
	{
        Rx_Buff->Data = StringToInt(Rx_Buff->Buffer);
        *Rx_Buff->PtData = Rx_Buff->Data; 
		Rx_Buff->Valid_Data = 0;
	}
   
}


void IntReadServiceBusU4(volatile TypServiceBusBuffer * Rx_Buff)
{
	char RxData = 0;
    static int RxCharCnt = 0;
    
    if(Rx_Buff->Valid_Data==0)
    {
        while(U4STAbits.URXDA)
        {
            RxData = U4RXREG;

            if(RxData != 10)    // Se non ho ancora ricevuto il carattere di fine stringa....
            {
                Rx_Buff->Buffer[RxCharCnt++] = RxData;

                if(RxCharCnt>=ByteLenServiceBusBuff) 
                    RxCharCnt--;

            }
            else        // ho ricevuto il carattere di fine stringa (10)
            {
                Rx_Buff->Buffer[RxCharCnt++] = RxData;
                if(Rx_Buff->Buffer[0]!=10 && Rx_Buff->Buffer[0]!=32)                  // Se ho ricevuto almeno un carattere al di fuori di 10 ("\n") o di 32 (" " spazio))
                    Rx_Buff->Valid_Data = 1;                // dico che ho ricevuto un dato valido
                RxCharCnt=0;
            }
        }
    }
}


void Process_ServiceU4Cmd(volatile TypServiceBusBuffer * Rx_Buff)
{
 
	if(Rx_Buff->Valid_Data == 1)
	{
        Rx_Buff->Data = StringToInt(Rx_Buff->Buffer);
        *Rx_Buff->PtData = Rx_Buff->Data; 
		Rx_Buff->Valid_Data = 0;
	}
   
}


int StringToInt(char * Buffer)
{
    int result=0;
    
    while(*Buffer!=10)
    {
        result *= 10;
        result += (int)(*Buffer-48);
        Buffer++;
    }
    return result;
}