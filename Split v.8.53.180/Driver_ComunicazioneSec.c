//----------------------------------------------------------------------------------
//	Progect name:	Driver_Comunicazione.c
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			22/05/2011
//	Description:	Corpo delle funzioni pilota della comunicazione su bus
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
#include "ProtocolloComunicazione.h"
#include "ProtocolloComunicazioneSec.h"
#include "Driver_ModBusSec.h"

extern volatile int TimeOutComErrSec;
extern volatile int TimeOutInterCharComErrSec;
extern volatile int CRC_ComErrSec;

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
/* OLD
char WriteBusSec(volatile TypBuffer * Tx_Buff)
{
	if (Tx_Buff->Valid_Data == 1)							// se il buffer di trasmissione ha un pacchetto validato
	{
		putch2(Tx_Buff->Start_Chr);							// invio sul bus la sequenza di byte
		putch2(Tx_Buff->Dest_Add);
		putch2(Tx_Buff->Mitt_Add);
		putch2(Tx_Buff->Comando);
		putch2((char)(((Tx_Buff->Registro & 0xFF00)>>8))&0x00FF);	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
		putch2((char)(Tx_Buff->Registro & 0x00FF));
		putch2((char)(((Tx_Buff->Data & 0xFF00)>>8))&0x00FF);		// Modifica 05/01/2015 Aggiunta mascheratura byte shift
		putch2((char)(Tx_Buff->Data & 0x00FF));
		putch2((char)(((Tx_Buff->CRC16 & 0xFF00)>>8))&0x00FF);		// Modifica 05/01/2015 Aggiunta mascheratura byte shift
		putch2((char)(Tx_Buff->CRC16 & 0x00FF));

		Tx_Buff->Valid_Data = 0;							// invalido il dato in quanto vecchio
		return 0;											// ritorno 0 in quanto l'operazione è andata a boun fine
	}
	return 1;												// ritorno 1 perche il pacchetto dati non è valido
}
*/

char WriteBusSec(void)
{
	LastRxSec.Valid_Data = 0;                                  // Azzero flag di RX dato valido!
	MyBUS_BufferTxSec.CntByte = 1;
    MyBUS_BufferTxSec.nByte = 10;    
    
	oRTS_Slave = 1;											// alzo il pin dell'rts x la 485
    
    IFS1bits.U2TXIF = 0;									// Clear interrupt flag TX Uart2 
	IEC1bits.U2TXIE = 1;									// Attivo l'interrupt di TX Uart2
	U2TXREG = MyBUS_BufferTxSec.Buffer[0];						// scrivo il primo byte nel registro di trasmissione x iniziare l'invio del bufferTX

	while (MyBUS_BufferTxSec.Valid_Data == 1)					// attendo che il buffer sia completamente inviato..... -> Mod_BufferTxSec.Valid_Data == 0 vedi interrupt "U2TX_VCTR"
		continue;    

	while(!U2STAbits.TRMT)                                  // attendo la fine della trasmissione
		continue;    
    
	IEC1bits.U2TXIE = 0;									// Disattivo l'interrupt di TX Uart2    
	//oRTS_Slave = 0;											// abbasso il pin dell'rts x la 485

	return 0;												// ritorno 0 in quanto l'operazione è andata a boun fine   
}

char WriteBusU3(void)
{
	LastRxSec.Valid_Data = 0;                                  // Azzero flag di RX dato valido!
	MyBUS_BufferTxSec.CntByte = 1;
    MyBUS_BufferTxSec.nByte = 10;    
    
	oRTS_UART3 = 1;											// alzo il pin dell'rts x la 485
    
    _U3TXIF = 0;									// Clear interrupt flag TX Uart3 
	_U3TXIE = 1;									// Attivo l'interrupt di TX Uart3
	U3TXREG = MyBUS_BufferTxSec.Buffer[0];						// scrivo il primo byte nel registro di trasmissione x iniziare l'invio del bufferTX

	while (MyBUS_BufferTxSec.Valid_Data == 1)					// attendo che il buffer sia completamente inviato..... -> Mod_BufferTxSec.Valid_Data == 0 vedi interrupt "U3TX_VCTR"
		continue;    

	while(!U3STAbits.TRMT)                                  // attendo la fine della trasmissione
		continue;    
    
	_U3TXIE = 0;									// Disattivo l'interrupt di TX Uart3    

	return 0;												// ritorno 0 in quanto l'operazione è andata a boun fine   
}

void IntReadBusSec(volatile TypBuffer * Rx_Buff)
{
    unsigned char rxchar;
    _Bool flag;
    
    
////////////////////////////
//	U2STAbits.OERR = 0; //CLEAR THE OVERRUN ERROR BIT    
//	U2STAbits.FERR = 0;        
////////////////////////////    
    
    if(Me.ComToUart3==1)       
        flag = U3STAbits.URXDA;
    else
        flag = U2STAbits.URXDA;
        
	while(flag)
	{
        
////////////////////////////
//unsigned char errframData;        
    if(Me.ComToUart3==1)    
    {
        if(U3STAbits.OERR)
            U3STAbits.OERR = 0; //CLEAR THE OVERRUN ERROR BIT
        if((U3STAbits.FERR == 1)||(U3STAbits.PERR == 1))
            rxchar = (U3RXREG); //clear register by read        
    }
    else
    {
        if(U2STAbits.OERR)
            U2STAbits.OERR = 0; //CLEAR THE OVERRUN ERROR BIT
        if((U2STAbits.FERR == 1)||(U2STAbits.PERR == 1))
            rxchar = (U2RXREG); //clear register by read
    }
//		//erroverrun2++;	// solo x DEBUG
//	    
//	}
  //  if(U2STAbits.FERR)
//	{
//	        U2STAbits.FERR = 0;
//	        errframData=U2RXREG ; // flush buffer
//	        //errframing2++;	// solo x DEBUG
//	}          
////////////////////////////        
        
        
        
        if(Me.ComToUart3==1)       
            rxchar = U3RXREG;  
        else
            rxchar = U2RXREG;  
        
		if(((iPckRxSec == 0) && (rxchar == StartPack)) || (iPckRxSec != 0))
		{
			if(TimeOutRxSec.Enable == 0)			// verifico se il timer per il time out è già attivo (rx in corso)
				TimeOutRxSec.Enable = 1;			// se non lo attivo
			TimeOutRxSec.Value = 0;					// resetto il timer comunque perchè ho appena ricevuto un carattere
			TimeOutRxSec.Time = K_TimeInterCharSec;	// setto il time out @ 10 ms
	
			switch(iPckRxSec)						// in base all'indice ripongo il carattere ricevuto nella giusta posizione
			{
				case 0:
					Rx_Buff->Start_Chr = rxchar; //U2RXREG;
					break;
	
				case 1:
					Rx_Buff->Dest_Add = rxchar;
					break;
	
				case 2:
					Rx_Buff->Mitt_Add = rxchar;
					break;
	
				case 3:
					Rx_Buff->Comando = rxchar;
					break;
	
				case 4:
					Rx_Buff->Registro = (int)rxchar;
					break;
	
				case 5:
					Rx_Buff->Registro = (((Rx_Buff->Registro<<8)&0xFF00) | (int)rxchar);	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
					break;
	
				case 6:
					Rx_Buff->Data = (int)rxchar;
					break;
	
				case 7:
					Rx_Buff->Data = (((Rx_Buff->Data <<8)&0xFF00) | (int)rxchar);	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
					break;
	
				case 8:
					Rx_Buff->CRC16 = (int)rxchar;
					break;
	
				case 9:
					Rx_Buff->CRC16 = (((Rx_Buff->CRC16 <<8)&0xFF00) | (int)rxchar);	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
					break;
			}
	
			iPckRxSec+=1;					// incremento l'indice
			if(iPckRxSec > 9)				// se ho completato la sequenza
			{
				iPckRxSec = 0;				// azzero l'indice di ricezione in quanto presumo che si sia inetrrotta la comunicazione
				TimeOutRxSec.Enable = 0;	// fermo il timer del time out
				TimeOutRxSec.Value = 0;		// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
				TimeOutRxSec.TimeOut = 0;	// e resetto il flag di time out
				Rx_Buff->Valid_Data = 1;	// dico che ho ricevuto un dato valido
			}
		}
        if(Me.ComToUart3==1)                        
        {
            flag = U3STAbits.URXDA;
    		_U3RXIF = 0;
        }
        else
        {
            flag = U2STAbits.URXDA;
    		_U2RXIF = 0;
        }
	}
}

void IntTimeOutRxSec(void)
{
	if(TimeOutRxSec.Enable == 1)		// controllo se il timer è attivo
	{
		TimeOutRxSec.Value += 1;		// gestisco il conteggio del timer
		if (TimeOutRxSec.Value >= TimeOutRxSec.Time)	// gestisco il time out
		{
			TimeOutRxSec.Enable = 0;	// fermo il timer del time out
			TimeOutRxSec.Value = 0;		// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
			iPckRxSec = 0;				// azzero l'indice di ricezione in quanto presumo che si sia inetrrotta la comunicazione
            TimeOutInterCharComErrSec +=1;  
            Me.ComError.TimeOutInterCharComErrSec +=1;
		}
	}
}

void Recive_Cmd_Sec(volatile TypBuffer * Rx_Buff, volatile TypBuffer * LastCmd)
{
	if(Rx_Buff->Valid_Data == 1)		// controllo se ho finito di ricevere un pacchetto
	{
		int CRC16_calcolato = 0;

		CopyBuffer(LastCmd, Rx_Buff);	// copio quello che ho ricevuto così da liberare il buffer di ricezione
		iPckRxSec = 0;					// reinizializzo l'indice 
		Rx_Buff->Valid_Data = 0;		// resetto la validità del buffer di rx in quanto lo già copiato per analizzarlo
		LastCmd->Valid_Data = 0;		// resetto la validità del dato, sarà il controllo a giudicarne la validità

		CRC16_calcolato = GenerateCRC(LastCmd);	// mi calcolo il CRC del pacchetto

		if(CRC16_calcolato == LastCmd->CRC16)		// se è uguale a quello ricevuto il pacchetto è ok
		{
			//if(LastCmd->Dest_Add == My_Address)		// cotrollo se è indirizzato a me
			if(LastCmd->Dest_Add == k_My_Address_Sec)		// cotrollo se è indirizzato a me            
			{
				LastCmd->Valid_Data = 1;			// a questo punto devo analizzare il pacchetto
			}
		}
		else
		{	
            //TimerModRstComErr.Value = 0;
            if(LastCmd->Dest_Add == k_My_Address_Sec)  
            {
                CRC_ComErrSec +=1;
                Me.ComError.CRC_ComErrSec +=1;
            } 
		}                   
	}
}

void Write_Cmd_Sec(volatile TypBuffer * Tx_Buff, char Address, char Comando, int Registro, int Data)
{
	volatile int CRC = 0;
	volatile char ReturnValue = 0;

	Tx_Buff->Start_Chr = StartPack;
	Tx_Buff->Dest_Add = Address;
	Tx_Buff->Mitt_Add = k_My_Address_Sec; //My_Address;
	Tx_Buff->Comando = Comando;
	Tx_Buff->Registro = Registro;
	Tx_Buff->Data = Data;
	CRC = GenerateCRC(Tx_Buff);
	Tx_Buff->CRC16 = CRC;
	Tx_Buff->Valid_Data = 1;
    
    MyBUS_BufferTxSec.Buffer[0] = Tx_Buff->Start_Chr;    
    MyBUS_BufferTxSec.Buffer[1] = Tx_Buff->Dest_Add;    
    MyBUS_BufferTxSec.Buffer[2] = Tx_Buff->Mitt_Add;    
    MyBUS_BufferTxSec.Buffer[3] = Tx_Buff->Comando;    
    MyBUS_BufferTxSec.Buffer[4] = (char)(((Tx_Buff->Registro & 0xFF00)>>8)&0x00FF);    
    MyBUS_BufferTxSec.Buffer[5] = (char)(Tx_Buff->Registro & 0x00FF);    
    MyBUS_BufferTxSec.Buffer[6] = (char)(((Tx_Buff->Data & 0xFF00)>>8)&0x00FF);
    MyBUS_BufferTxSec.Buffer[7] = (char)(Tx_Buff->Data & 0x00FF);    
    MyBUS_BufferTxSec.Buffer[8] = (char)(((Tx_Buff->CRC16 & 0xFF00)>>8)&0x00FF);    
    MyBUS_BufferTxSec.Buffer[9] = (char)(Tx_Buff->CRC16 & 0x00FF);    

    if(Me.ComToUart3==1)
        ReturnValue = WriteBusU3();					// trasmetto la richiesta    
    else
	ReturnValue = WriteBusSec();
}

/*
void Write_brc_Cmd_Sec(volatile TypBuffer * Tx_Buff, char Comando, int Registro, int Data)
{
	volatile int CRC = 0;
	volatile char ReturnValue = 0;

	Tx_Buff->Start_Chr = StartPack;
	Tx_Buff->Dest_Add = k_Broadcast_Addr;		// Modifica del 02/12/2015
	Tx_Buff->Mitt_Add = k_My_Address_Sec; //My_Address;
	Tx_Buff->Comando = Comando;
	Tx_Buff->Registro = Registro;
	Tx_Buff->Data = Data;
	CRC = GenerateCRC(Tx_Buff);
	Tx_Buff->CRC16 = CRC;
	Tx_Buff->Valid_Data = 1;
    
    MyBUS_BufferTxSec.Buffer[0] = Tx_Buff->Start_Chr;    
    MyBUS_BufferTxSec.Buffer[1] = Tx_Buff->Dest_Add;    
    MyBUS_BufferTxSec.Buffer[2] = Tx_Buff->Mitt_Add;    
    MyBUS_BufferTxSec.Buffer[3] = Tx_Buff->Comando;    
    MyBUS_BufferTxSec.Buffer[4] = (char)(((Tx_Buff->Registro & 0xFF00)>>8)&0x00FF);    
    MyBUS_BufferTxSec.Buffer[5] = (char)(Tx_Buff->Registro & 0x00FF);    
    MyBUS_BufferTxSec.Buffer[6] = (char)(((Tx_Buff->Data & 0xFF00)>>8)&0x00FF);
    MyBUS_BufferTxSec.Buffer[7] = (char)(Tx_Buff->Data & 0x00FF);    
    MyBUS_BufferTxSec.Buffer[8] = (char)(((Tx_Buff->CRC16 & 0xFF00)>>8)&0x00FF);    
    MyBUS_BufferTxSec.Buffer[9] = (char)(Tx_Buff->CRC16 & 0x00FF);       

	ReturnValue = WriteBusSec();
}
 */