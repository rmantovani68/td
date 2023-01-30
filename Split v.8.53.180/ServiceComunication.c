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
#include "Driver_Comunicazione.h"
#include "ProtocolloComunicazione.h"
#include "usart.h"
#include "FWSelection.h"
#include "delay.h"
#include "ServiceComunication.h"

extern int SaveNumber;

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------

#ifdef UART3_ON    
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
            DelaymSec(20); 
		break;   
        
		case 1:
			set_baudrateU3_57600(); 				// @ 57600Bps
            DelaymSec(20); 
		break;

		case 2:
			set_baudrateU3_115200(); 			// @ 115200Bps
            DelaymSec(20); 
		break;
	
		default:
			set_baudrateU3_57600();				// @ 57600Bps
            DelaymSec(20); 
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

	while (ServiceBusBufferTxU3.Valid_Data == 1)				// attendo che il buffer sia completamente inviato..... -> Mod_BufferTxSec.Valid_Data == 0 vedi interrupt "U2TX_VCTR"
		continue;    

	while(!U3STAbits.TRMT)                                  // attendo la fine della trasmissione
		continue;    
    
	_U3TXIE = 0;                                            // Disattivo l'interrupt di TX Uart3
	//oRTS_UART3 = 0;											// abbasso il pin dell'rts x la 485
}


void PutDataForUART3(int Val1, int Val2, int Val3, int Val4, unsigned int Val5, unsigned int Val6, unsigned int Val7, unsigned int Val8)
{
    static int cnt = 0;
    static int ReadSched = 0;
    
    
    cnt++;	
	if(cnt==1)	
	{	    
        ServiceBusBufferTxU3.nByte = 0;
        //ServiceBusBufferTxU3.nByte = sprintf(&ServiceBusBufferTxU3.Buffer[0], "CLEARDATA\r\n");  
        ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CLEARSHEET\r\n"); 
        ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "LABEL,Me.Fan_Current_Limit, Me.Fan_Current_Fast, Me.Pwm_Req, Me.Pwm_Value, 0, 0, Me.CycleTime, 0,,,,,,,,Kp,Ki,Kd\r\n");
        //ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,P2,100,200,300\n\r"); 
    }
    else
        ServiceBusBufferTxU3.nByte = 0;
    
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "DATA,");
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%i, ", Val1);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%i, ", Val2);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%i, ", Val3);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%i, ", Val4);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%u, ", Val5);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%u, ", Val6);   
    ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%u, ", Val7); 
    ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%u, ", Val8); 
    ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "\r\n"); 
        
    if(cnt==100)	
	{
		cnt=1;
        //ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "SAVEWORKBOOK\r\n");
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


void PutDataForUART3_NEW(int Val1, int Val2, int Val3, int Val4, int Val5)
{
    static int cnt = 0;
    static int ReadSched = 0;
    
    
    cnt++;	
	if(cnt==1)	
	{	    
        ServiceBusBufferTxU3.nByte = 0;
        //ServiceBusBufferTxU3.nByte = sprintf(&ServiceBusBufferTxU3.Buffer[0], "CLEARDATA\r\n");  
        ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CLEARSHEET\r\n"); 
        ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "LABEL,FAN Speed (RPM),FAN ADC_I,FAN Current (mA),FAN Power (decimi W),CycleTime\r\n");
        //ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,P2,100,200,300\n\r"); 
    }
    else
        ServiceBusBufferTxU3.nByte = 0;
    
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "DATA,");
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%i, ", Val1);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%i, ", Val2);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%i, ", Val3);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%i, ", Val4);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%i, ", Val5); 
    ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "\r\n"); 
        
    
    if(cnt==100)	
	{
		cnt=1;
        //ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "SAVEWORKBOOK\r\n");
        ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "ROW,SET,2\r\n");        
	}	
    
    /*
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
    */
    
    SendBufferToUART3();					// trasmetto la richiesta
}


void PutDataForUART3_CheckBoard(int Number)
{
    static int cnt=1;
    static int cnt2=0;    
    static int cnt3=1;
    static int cnt4=0;
    static int cnt5=0;
    static int cnt6=0;
    static int cnt7=0;
    static int cnt8=0;
    static int CheckBox=0;
    int Split_Exp_Valve_Perc;
    unsigned int temp;
   
    ServiceBusBufferTxU3.nByte = 0;    
    
    switch(Number)
    {
        case -1:
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,FROMSHEET,Simple Data,U,1,%i \r\n");//(Number*3)+9);    
            ServiceBusBufferRxU3.PtData = &SaveNumber;
            break;  
            
        case 0:
            if(CheckBox==0)
            {
                
                ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Step Description,I,1,%i \r\n",((1*!Me.Error.EEPROM_Cell) + (2*Me.Error.EEPROM_Cell)));    
                
                ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,FROMSHEET,Step Description,E,%i \r\n", Number+1);//(Number*3)+9);    
                ServiceBusBufferRxU3.PtData = &ServiceBusBufferRxU3.CheckBox[Number];
                CheckBox=1;
            }
            else
            {
                ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,3,%i \r\n", Number+1);     

                ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,9,%f \r\n",(float)(Me.Temperature.Liquid)/100.0);     
                ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,10,%f \r\n",(float)(Me.Temperature.Gas)/100.0);     
                ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,11,%f \r\n",(float)(Me.Temperature.AirOut)/100.0);     
                ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,12,%f \r\n",(float)(Me.Temperature.Ambient)/100.0);     
                ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Step Description,I,1,%i \r\n",((1*!Me.Error.EEPROM_Cell) + (2*Me.Error.EEPROM_Cell)));    

                ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,FROMSHEET,Step Description,E,%i \r\n", Number+1);//(Number*3)+9);    
                ServiceBusBufferRxU3.PtData = &ServiceBusBufferRxU3.CheckBox[Number];
            }
               break;
        case 1:            
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,3,%i \r\n", Number+1);     

            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,17,%i \r\n",Me.Fan_Tacho); 

            if(cnt2++>=3)  // Ogni 3Sec. circa... invia dati al grafico FAN Speed
            {
                cnt2=0;
                if(cnt>=100) 
                    cnt=1;
                ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,FAN Speed Data,A,%i,%i \r\n",cnt++,Me.Fan_Tacho);     
            }
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,FROMSHEET,Step Description,E,%i \r\n", Number+1);//(Number*3)+9);    
            ServiceBusBufferRxU3.PtData = &ServiceBusBufferRxU3.CheckBox[Number];

            break;
        case 2:
            temp = K_MaxExcursionValve; //ValvolaAperta;
            Split_Exp_Valve_Perc = (unsigned int)( ((float)Me.ExpValve_Act / (float)temp) * 100.0);             
            
            
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,3,%i \r\n", Number+1);     

            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,22,%f \r\n",(float)(EngineBox.Pressione_Gas_G)/1000.0); 
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,24,%i \r\n",Split_Exp_Valve_Perc); 

            if(cnt4++>=2)  // Ogni 2Sec. circa... invia dati al grafico
            {
                cnt4=0;
                if(cnt3>=100) 
                    cnt3=1;
                ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Pressure Data,A,%i,%f \r\n",cnt3,(float)(EngineBox.Pressione_Gas_G)/1000.0);     
                ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Valve Position Data,A,%i,%i \r\n",cnt3++,Split_Exp_Valve_Perc);     
            }
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,FROMSHEET,Step Description,E,%i \r\n", Number+1);//(Number*3)+9);    
            ServiceBusBufferRxU3.PtData = &ServiceBusBufferRxU3.CheckBox[Number];

            break;        
        case 3:
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,3,%i \r\n", Number+1);     

            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,29,%f \r\n",(float)(Me.Temperature.Liquid)/100.0);        
            
            if(cnt5>=100) 
                cnt5=1;
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Probe Data,A,%i,%f \r\n",cnt5++,(float)(Me.Temperature.Liquid)/100.0);             
            
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,FROMSHEET,Step Description,E,%i \r\n", Number+1);//(Number*3)+9);    
            ServiceBusBufferRxU3.PtData = &ServiceBusBufferRxU3.CheckBox[Number];
            break;        
        case 4:
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,3,%i \r\n", Number+1);     

            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,31,%f \r\n",(float)(Me.Temperature.Gas)/100.0);   
            
            if(cnt6>=100) 
                cnt6=1;
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Probe Data,B,%i,%f \r\n",cnt6++,(float)(Me.Temperature.Gas)/100.0);              
            
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,FROMSHEET,Step Description,E,%i \r\n", Number+1);//(Number*3)+9);    
            ServiceBusBufferRxU3.PtData = &ServiceBusBufferRxU3.CheckBox[Number];

            break;        
        case 5:
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,3,%i \r\n", Number+1);     

            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,33,%f \r\n",(float)(Me.Temperature.AirOut)/100.0);     

            if(cnt7>=100) 
                cnt7=1;
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Probe Data,C,%i,%f \r\n",cnt7++,(float)(Me.Temperature.AirOut)/100.0);                
            
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,FROMSHEET,Step Description,E,%i \r\n", Number+1);//(Number*3)+9);    
            ServiceBusBufferRxU3.PtData = &ServiceBusBufferRxU3.CheckBox[Number];

            break;        
        case 6:
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,3,%i \r\n", Number+1);     

            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,35,%f \r\n",(float)(Me.Temperature.Ambient)/100.0);     
            
            if(cnt8>=100) 
                cnt8=1;
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Probe Data,D,%i,%f \r\n",cnt8++,(float)(Me.Temperature.Ambient)/100.0);            
            
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,FROMSHEET,Step Description,E,%i \r\n", Number+1);//(Number*3)+9);    
            ServiceBusBufferRxU3.PtData = &ServiceBusBufferRxU3.CheckBox[Number];

            break;        
        case 7:
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,3,%i \r\n", Number+1);     

            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,40,%i \r\n",Touch[0].OnLine);                 
            
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,FROMSHEET,Step Description,E,%i \r\n", Number+1);//(Number*3)+9);    
            ServiceBusBufferRxU3.PtData = &ServiceBusBufferRxU3.CheckBox[Number];
            break; 
        case 8:           
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,3,%i \r\n", Number+1);     

            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,45,%i \r\n",(~PORTA & 0x003F));     

            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,FROMSHEET,Step Description,E,%i \r\n", Number+1);//(Number*3)+9);    
            ServiceBusBufferRxU3.PtData = &ServiceBusBufferRxU3.CheckBox[Number];
            break;
        case 9:
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,3,%i \r\n", Number+1);     
 
            DelaymSec(1000); 
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Step Description,K,1,%i \r\n", 1);                 

            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,3,%i \r\n", 11);         //B3
            break;
        case 10:
            ServiceBusBufferTxU3.nByte = 0;                
            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,U,1,%i \r\n", SaveNumber+1);                 

            ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,ONSHEET,Simple Data,B,3,%i \r\n", 12);         //B3
            break; 
    }
    
    
    SendBufferToUART3();					// trasmetto la richiesta
    
    unsigned int cnt22=0;
    while(ServiceBusBufferRxU3.Valid_Data==0 && cnt22<1000)
    {
        DelaymSec(1);
        cnt22++;
    }
    
    if(ServiceBusBufferRxU3.Valid_Data)
        Process_ServiceU3Cmd(&ServiceBusBufferRxU3);    
}

void PutDataForUART3_GetFileNumber(void)
{
    static int SaveNumber=0;
    
    ServiceBusBufferTxU3.nByte = 0;
    
    ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,FROMSHEET,Simple Data,U,1 \r\n");//(Number*3)+9);    
    ServiceBusBufferRxU3.PtData = &SaveNumber;
    
    SendBufferToUART3();					// trasmetto la richiesta
}





/*
void PutDataForUART3_CheckBoard(int Val1, int Val2, int Val3, int Val4, unsigned int Val5, unsigned int Val6, unsigned int Val7, unsigned int Val8)
{
    static int cnt = 0;
    static int ReadSched = 0;
    
    
    cnt++;	
	if(cnt==1)	
	{	    
        ServiceBusBufferTxU3.nByte = 0;
        //ServiceBusBufferTxU3.nByte = sprintf(&ServiceBusBufferTxU3.Buffer[0], "CLEARDATA\r\n");  
        //ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "DONE\r\n");        // Flush EXCEL Buffer side              //ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "MSG, Son tutti Argo con il ... \r\n");               
        ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CLEARSHEET\r\n"); 
        ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "LABEL,Temp_Small,Temp_Big,Temp_AirOut,Temp_Amb,Pres_Big,Pres_Small,CycleTime,ExpValve_Act,,,,,,,,Kp,Ki,Kd\r\n");
        //ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,SET,P2,100,200,300\n\r"); 
    }
    else
        ServiceBusBufferTxU3.nByte = 0;
    
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "DATA,");
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%i, ", Val1);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%i, ", Val2);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%i, ", Val3);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%i, ", Val4);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%u, ", Val5);        
	ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%u, ", Val6);   
    ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%u, ", Val7); 
    ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "%u, ", Val8); 
    ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "\r\n"); 
        
    if(cnt==100)	
	{
		cnt=1;
        //ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "SAVEWORKBOOK\r\n");
        ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "BEEP\r\n");       
        ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "ROW,SET,2\r\n");     
        //ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CUSTOMBOX1,LABEL,Measure humidity as well?\r\n");     
        //ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CUSTOMBOX1,GET\r\n");     

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
*/


void PutDataForUART3_CheckBox(int Number)
{    
    ServiceBusBufferTxU3.nByte = 0;


    //ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,Foglio1!A%i \r\n", Number+1);//(Number*3)+9);    
    ServiceBusBufferTxU3.nByte += sprintf(&ServiceBusBufferTxU3.Buffer[ServiceBusBufferTxU3.nByte], "CELL,GET,FROMSHEET,Foglio1,A,%i \r\n", Number+1);//(Number*3)+9);    
    ServiceBusBufferRxU3.PtData = &ServiceBusBufferRxU3.CheckBox[Number];
    
    SendBufferToUART3();					// trasmetto la richiesta
    DelaymSec(20);
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


int StringToInt(volatile char * Buffer)
{
    int result=0;
    
    if(*Buffer=='V' && *(Buffer+1)=='e' && *(Buffer+2)=='r' && *(Buffer+3)=='o')
        return 1;
    if(*Buffer=='F' && *(Buffer+1)=='a' && *(Buffer+2)=='l' && *(Buffer+3)=='s' && *(Buffer+4)=='o')
        return 0;
        
    while(*Buffer!=10)
    {
        result *= 10;
        result += (int)(*Buffer-48);
        Buffer++;
    }
    return result;
}



#endif

