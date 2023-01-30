//----------------------------------------------------------------------------------
//	Progect name:	Driver_Comunicazione.c
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			Emanuele
//	Date:			04/01/2014
//	Description:	Corpo delle funzioni pilota della comunicazione su bus
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
#include "usart.h"
#include "Driver_Comunicazione.h"
#include "ProtocolloComunicazione.h"

extern volatile int CRC_ComErr; //ARGOCRCPUZZONE

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
int BaseCRC16(int Old_CRC, unsigned char NewChr)
{
	int i=0;
    int t_Old_CRC;
    unsigned char t_NewChr;
    
    t_Old_CRC = Old_CRC;
    t_NewChr = NewChr;
            
	for(i=0; i<8; i++)
	{
		t_Old_CRC = ((t_Old_CRC <<1)|(t_NewChr>>7));
		t_NewChr = (t_NewChr<<1);
		if((t_Old_CRC & 0x100) != 0)
			t_Old_CRC^=PolGenCRC16;
	}
	return t_Old_CRC;
/*
	int i=0;

	for(i=0; i<8; i++)
	{
		Old_CRC = ((Old_CRC <<1)|(NewChr>>7));
		NewChr = (NewChr<<1);
		if((Old_CRC & 0x100) != 0)
			Old_CRC^=PolGenCRC16;
	}
	return Old_CRC;
*/ 
}

int GenerateCRC(volatile TypBuffer * RtxBuff)
{
	int CRC_16=0;

	CRC_16 = BaseCRC16(CRC_16, RtxBuff->Start_Chr);     // N.B: RtxBuff->Start_Chr è l'equvalente di (*RtxBuff).Start_Chr che ritorna il valore della variabile del puntatore *RtxBuff
	CRC_16 = BaseCRC16(CRC_16, RtxBuff->Dest_Add);
	CRC_16 = BaseCRC16(CRC_16, RtxBuff->Mitt_Add);
	CRC_16 = BaseCRC16(CRC_16, RtxBuff->Comando);
	CRC_16 = BaseCRC16(CRC_16, (char)(RtxBuff->Registro & 0x00FF));
	CRC_16 = BaseCRC16(CRC_16, (char)(((RtxBuff->Registro & 0xFF00)>>8))&0x00FF);	// Modifica 25/05/2015 Aggiunta mascheratura byte shift
	CRC_16 = BaseCRC16(CRC_16, (char)(RtxBuff->Data & 0x00FF));
	CRC_16 = BaseCRC16(CRC_16, (char)(((RtxBuff->Data & 0xFF00)>>8))&0x00FF);		// Modifica 25/05/2015 Aggiunta mascheratura byte shift
    
	return CRC_16;
}

void CopyBuffer(volatile TypBuffer * Destinazione, volatile TypBuffer * Originale)
{
	Destinazione->Start_Chr = Originale->Start_Chr;			// copio ogni attributo della struttura dalla struttura originale a quella di destinazione
	Destinazione->Dest_Add = Originale->Dest_Add;
	Destinazione->Mitt_Add = Originale->Mitt_Add;
	Destinazione->Comando = Originale->Comando;
	Destinazione->Registro = Originale->Registro;
	Destinazione->Data = Originale->Data;
	Destinazione->CRC16 = Originale->CRC16;
	Destinazione->Valid_Data = Originale->Valid_Data;	
}




#if(K_Enable_TX_MyBus_Interrupt==1)        
char WriteBus(void)
{
	LastRx.Valid_Data = 0;                                  // Azzero flag di RX dato valido!
	MyBUS_BufferTx.CntByte = 1;
    MyBUS_BufferTx.nByte = 10;    
    
	oRTS_Master = 1;											// alzo il pin dell'rts x la 485
    
    _U1TXIF = 0;        									// Clear interrupt flag TX Uart1 
	_U1TXIE = 1;                							// Attivo l'interrupt di TX Uart1
	U1TXREG = MyBUS_BufferTx.Buffer[0];						// scrivo il primo byte nel registro di trasmissione x iniziare l'invio del bufferTX

	while (MyBUS_BufferTx.Valid_Data == 1)					// attendo che il buffer sia completamente inviato..... -> Mod_BufferTxSec.Valid_Data == 0 vedi interrupt "U2TX_VCTR"
		continue;    

	while(!U1STAbits.TRMT)                                  // attendo la fine della trasmissione
		continue;    
    
	_U1TXIE = 0;                                            // Disattivo l'interrupt di TX Uart2    
	//oRTS_Master = 0;                                        // abbasso il pin dell'rts x la 485

	return 0;												// ritorno 0 in quanto l'operazione è andata a boun fine   
}
#else    
char WriteBus(volatile TypBuffer * Tx_Buff)
{
	if (Tx_Buff->Valid_Data == 1)							// se il buffer di trasmissione ha un pacchetto validato
	{	
		oRTS_Master = 1;			// alzo il pin dell'rts x la 485
		putch1(Tx_Buff->Start_Chr);							// invio sul bus la sequenza di byte
		putch1(Tx_Buff->Dest_Add);
		putch1(Tx_Buff->Mitt_Add);
		putch1(Tx_Buff->Comando);
		//putch1((char)((Tx_Buff->Registro & 0xFF00)>>8));
		putch1((char)(((Tx_Buff->Registro & 0xFF00)>>8))&0x00FF);	// Modifica 25/05/2015 Aggiunta mascheratura byte shift
		putch1((char)(Tx_Buff->Registro & 0x00FF));
		//putch1((char)((Tx_Buff->Data & 0xFF00)>>8));
		putch1((char)(((Tx_Buff->Data & 0xFF00)>>8))&0x00FF);		// Modifica 25/05/2015 Aggiunta mascheratura byte shift
		putch1((char)(Tx_Buff->Data & 0x00FF));
		//putch1((char)((Tx_Buff->CRC16 & 0xFF00)>>8));
		putch1((char)(((Tx_Buff->CRC16 & 0xFF00)>>8))&0x00FF);		// Modifica 25/05/2015 Aggiunta mascheratura byte shift
		putch1((char)(Tx_Buff->CRC16 & 0x00FF));

		oRTS_Master = 0;			// abbasso il pin dell'rts x la 485

		Tx_Buff->Valid_Data = 0;							// invalido il dato in quanto vecchio
		return 0;											// ritorno 0 in quanto l'operazione è andata a boun fine
	}
	return 1;												// ritorno 1 perche il pacchetto dati non è valido
}
#endif 



void IntReadBus(volatile TypBuffer * Rx_Buff, int RxData)
{
//	int RxData = 0;
//	while(U1STAbits.URXDA)
	{
//		RxData = U1RXREG;
		if((((iPckRx == 0) && (RxData == StartPack)) || (iPckRx != 0)) && !oRTS_Master)
		{
			if(TimeOutRx.Enable == 0)				// verifico se il timer per il time out è già attivo (rx in corso)
				TimeOutRx.Enable = 1;				// se non lo attivo
			TimeOutRx.Value = 0;					// resetto il timer comunque perchè ho appena ricevuto un carattere
			TimeOutRx.Time = K_TimeInterChar;		// setto il time out @ ms
	
			switch(iPckRx)							// in base all'indice ripongo il carattere ricevuto nella giusta posizione
			{
				case 0:
					Rx_Buff->Start_Chr = RxData;	//StartPack; //U1RXREG;
					break;
	
				case 1:
					Rx_Buff->Dest_Add = RxData;
					break;
	
				case 2:
					Rx_Buff->Mitt_Add = RxData;
					break;
	
				case 3:
					Rx_Buff->Comando = RxData;
					break;
	
				case 4:
					Rx_Buff->Registro = (int)RxData;
					break;
	
				case 5:
					//Rx_Buff->Registro = ((Rx_Buff->Registro<<8)|(int)RxData);
					Rx_Buff->Registro = (((Rx_Buff->Registro<<8)&0xFF00) | (int)RxData);	// Modifica 25/05/2015 Aggiunta mascheratura byte shift
					break;
	
				case 6:
					Rx_Buff->Data = (int)RxData;
					break;
	
				case 7:
					//Rx_Buff->Data = ((Rx_Buff->Data <<8)|(int)RxData);
					Rx_Buff->Data = (((Rx_Buff->Data <<8)&0xFF00) | (int)RxData);			// Modifica 25/05/2015 Aggiunta mascheratura byte shift
					break;
	
				case 8:
					Rx_Buff->CRC16 = (int)RxData;
					break;
	
				case 9:
					//Rx_Buff->CRC16 = ((Rx_Buff->CRC16 <<8)|(int)RxData);
					Rx_Buff->CRC16 = (((Rx_Buff->CRC16 <<8)&0xFF00) | (int)RxData);	// Modifica 25/05/2015 Aggiunta mascheratura byte shift
					break;
			}
	
			iPckRx+=1;						// incremento l'indice
			if(iPckRx > 9)					// se ho completato la sequenza
			{
				iPckRx = 0;					// azzero l'indice di ricezione in quanto presumo che si sia inetrrotta la comunicazione
				TimeOutRx.Enable = 0;		// fermo il timer del time out
				TimeOutRx.Value = 0;		// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
				TimeOutRx.TimeOut = 0;		// e resetto il flag di time out
				Rx_Buff->Valid_Data = 1;	// dico che ho ricevuto un dato valido
			}
		}

	}
}

void IntTimeOutRx(void)
{
	if(TimeOutRx.Enable == 1)			// controllo se il timer è attivo
	{
		TimeOutRx.Value += 1;			// gestisco il conteggio del timer
		if (TimeOutRx.Value >= TimeOutRx.Time)	// gestisco il time out
		{
			TimeOutRx.Enable = 0;		// fermo il timer del time out
			TimeOutRx.Value = 0;		// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
			iPckRx = 0;					// azzero l'indice di ricezione in quanto presumo che si sia inetrrotta la comunicazione
		}
	}
}

void Recive_Cmd(volatile TypBuffer * Rx_Buff, volatile TypBuffer * LastCmd)
{
	if(Rx_Buff->Valid_Data == 1)		// controllo se ho finito di ricevere un pacchetto
	{
		int CRC16_calcolato = 0;

		CopyBuffer(LastCmd, Rx_Buff);	// copio quello che ho ricevuto così da liberare il buffer di ricezione
		iPckRx = 0;						// reinizializzo l'indice 
		Rx_Buff->Valid_Data = 0;		// resetto la validità del buffer di rx in quanto lo già copiato per analizzarlo
		LastCmd->Valid_Data = 0;		// resetto la validità del dato, sarà il controllo a giudicarne la validità

		CRC16_calcolato = GenerateCRC(LastCmd);	// mi calcolo il CRC del pacchetto

		if(CRC16_calcolato == LastCmd->CRC16)		// se è uguale a quello ricevuto il pacchetto è ok
		{
			//if((LastCmd->Dest_Add == Master_Default_Address) | (LastCmd->Dest_Add == LastCmd->Mitt_Add))	// cotrollo se è indirizzato a me o è in broadCast
			//if((LastCmd->Dest_Add == Master_Default_Address) || (LastCmd->Dest_Add == LastCmd->Mitt_Add))	// Modifica 10/06/2015
			#if(AbilRX_All == 1)
			if(((LastCmd->Dest_Add >= MinRX_All) && (LastCmd->Dest_Add <= MaxRX_All)) || (LastCmd->Dest_Add == k_Broadcast_Addr))	// cotrollo se è indirizzato a me o è in broadCast
			#else
            if((LastCmd->Dest_Add == EngineBox[0].BoardAddress) || (LastCmd->Dest_Add == Master_Default_Address) || (LastCmd->Dest_Add == k_Broadcast_Addr))	// Modifica 02/12/2015
			#endif                
			{
			#if(AbilRX_All == 1)
                Master_Default_Address = LastCmd->Dest_Add;
			#endif                                
				LastCmd->Valid_Data = 1;			// a questo punto devo analizzare il pacchetto
			}
		}
        else
        {
            CRC_ComErr++;   
            EngineBox[0].ComError.CRC_ComErr++;
        }

	}
}

void Write_Cmd(volatile TypBuffer * Tx_Buff, char Address, char Comando, int Registro, int Data)
{
	volatile int CRC = 0;
	volatile char ReturnValue = 0;

	Tx_Buff->Start_Chr = StartPack;
	Tx_Buff->Dest_Add = Address;
	Tx_Buff->Mitt_Add = EngineBox[0].BoardAddress;
	Tx_Buff->Comando = Comando;
	Tx_Buff->Registro = Registro;
	Tx_Buff->Data = Data;
	CRC = GenerateCRC(Tx_Buff);
	Tx_Buff->CRC16 = CRC;
	Tx_Buff->Valid_Data = 1;
/*
    BufferTx.Buffer[0] = Tx_Buff->Start_Chr;    
    BufferTx.Buffer[1] = Tx_Buff->Dest_Add;    
    BufferTx.Buffer[2] = Tx_Buff->Mitt_Add;    
    BufferTx.Buffer[3] = Tx_Buff->Comando;    
    BufferTx.Buffer[4] = (char)(((Tx_Buff->Registro & 0xFF00)>>8)&0x00FF);    
    BufferTx.Buffer[5] = (char)(Tx_Buff->Registro & 0x00FF);    
    BufferTx.Buffer[6] = (char)(((Tx_Buff->Data & 0xFF00)>>8)&0x00FF);
    BufferTx.Buffer[7] = (char)(Tx_Buff->Data & 0x00FF);    
    BufferTx.Buffer[8] = (char)(((Tx_Buff->CRC16 & 0xFF00)>>8)&0x00FF);    
    BufferTx.Buffer[9] = (char)(Tx_Buff->CRC16 & 0x00FF);      
*/
    MyBUS_BufferTx.Buffer[0] = Tx_Buff->Start_Chr;    
    MyBUS_BufferTx.Buffer[1] = Tx_Buff->Dest_Add;    
    MyBUS_BufferTx.Buffer[2] = Tx_Buff->Mitt_Add;    
    MyBUS_BufferTx.Buffer[3] = Tx_Buff->Comando;    
    MyBUS_BufferTx.Buffer[4] = (char)(((Tx_Buff->Registro & 0xFF00)>>8)&0x00FF);    
    MyBUS_BufferTx.Buffer[5] = (char)(Tx_Buff->Registro & 0x00FF);    
    MyBUS_BufferTx.Buffer[6] = (char)(((Tx_Buff->Data & 0xFF00)>>8)&0x00FF);
    MyBUS_BufferTx.Buffer[7] = (char)(Tx_Buff->Data & 0x00FF);    
    MyBUS_BufferTx.Buffer[8] = (char)(((Tx_Buff->CRC16 & 0xFF00)>>8)&0x00FF);    
    MyBUS_BufferTx.Buffer[9] = (char)(Tx_Buff->CRC16 & 0x00FF);     
    
#if(K_Enable_TX_MyBus_Interrupt==1)        
	ReturnValue = WriteBus();
#else    
    ReturnValue = WriteBus(Tx_Buff);
#endif 
}

void Write_brc_Cmd(volatile TypBuffer * Tx_Buff, char Comando, int Registro, int Data)
{
	volatile int CRC = 0;
	volatile char ReturnValue = 0;

	Tx_Buff->Start_Chr = StartPack;
	Tx_Buff->Dest_Add = k_Broadcast_Addr;		// Modifica del 02/12/2015
	Tx_Buff->Mitt_Add = EngineBox[0].BoardAddress;
	Tx_Buff->Comando = Comando;
	Tx_Buff->Registro = Registro;
	Tx_Buff->Data = Data;
	CRC = GenerateCRC(Tx_Buff);
	Tx_Buff->CRC16 = CRC;
	Tx_Buff->Valid_Data = 1;
/*
    BufferTx.Buffer[0] = Tx_Buff->Start_Chr;    
    BufferTx.Buffer[1] = Tx_Buff->Dest_Add;    
    BufferTx.Buffer[2] = Tx_Buff->Mitt_Add;    
    BufferTx.Buffer[3] = Tx_Buff->Comando;    
    BufferTx.Buffer[4] = (char)(((Tx_Buff->Registro & 0xFF00)>>8)&0x00FF);    
    BufferTx.Buffer[5] = (char)(Tx_Buff->Registro & 0x00FF);    
    BufferTx.Buffer[6] = (char)(((Tx_Buff->Data & 0xFF00)>>8)&0x00FF);
    BufferTx.Buffer[7] = (char)(Tx_Buff->Data & 0x00FF);    
    BufferTx.Buffer[8] = (char)(((Tx_Buff->CRC16 & 0xFF00)>>8)&0x00FF);    
    BufferTx.Buffer[9] = (char)(Tx_Buff->CRC16 & 0x00FF);        
*/    
    MyBUS_BufferTx.Buffer[0] = Tx_Buff->Start_Chr;    
    MyBUS_BufferTx.Buffer[1] = Tx_Buff->Dest_Add;    
    MyBUS_BufferTx.Buffer[2] = Tx_Buff->Mitt_Add;    
    MyBUS_BufferTx.Buffer[3] = Tx_Buff->Comando;    
    MyBUS_BufferTx.Buffer[4] = (char)(((Tx_Buff->Registro & 0xFF00)>>8)&0x00FF);    
    MyBUS_BufferTx.Buffer[5] = (char)(Tx_Buff->Registro & 0x00FF);    
    MyBUS_BufferTx.Buffer[6] = (char)(((Tx_Buff->Data & 0xFF00)>>8)&0x00FF);
    MyBUS_BufferTx.Buffer[7] = (char)(Tx_Buff->Data & 0x00FF);    
    MyBUS_BufferTx.Buffer[8] = (char)(((Tx_Buff->CRC16 & 0xFF00)>>8)&0x00FF);    
    MyBUS_BufferTx.Buffer[9] = (char)(Tx_Buff->CRC16 & 0x00FF); 
    
#if(K_Enable_TX_MyBus_Interrupt==1)        
	ReturnValue = WriteBus();
#else    
    ReturnValue = WriteBus(Tx_Buff);
#endif    
}
