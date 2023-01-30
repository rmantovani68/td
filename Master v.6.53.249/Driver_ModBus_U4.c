//----------------------------------------------------------------------------------
//	Progect name:	Driver_ModBus_U4.c
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			TDM Team
//	Date:			11/04/2022
//	Description:	Corpo delle funzioni pilota della comunicazione ModBus su UART4
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//	Include
//----------------------------------------------------------------------------------
#include "FWSelection.h"

#if (K_AbilMODBCAN==1)  
#include "HWSelection.h"
#include <p24FJ256GB210.h>

#include <stdio.h>
#include "DefinePeriferiche.h"
#include "Timer.h"
#include "usart.h"
#include "Driver_ModBus.h"
#include "Driver_ModBus_U4.h"
#include "ProtocolloModBus.h"
#include "ProtocolloModBus_U4.h"

extern volatile int ModCRC_ComErr_U4;

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
unsigned int Mod_BaseCRC16_U4(unsigned int Old_CRC, unsigned char NewChr)
{
	unsigned int i=0;
	unsigned char LastBit = 0;
    unsigned int t_Old_CRC;
    unsigned char t_NewChr;
    
    t_Old_CRC = Old_CRC;
    t_NewChr = NewChr;    
    
	t_Old_CRC ^= t_NewChr;
	for(i=0; i<8; i++)
	{
		LastBit = t_Old_CRC & 0x01;
		t_Old_CRC = (t_Old_CRC>>1) & 0x7FFF;
		if(LastBit == 1)
			t_Old_CRC^=ModPolGenCRC16;
	}
	return t_Old_CRC;    
/*    
	unsigned int i=0;
	unsigned char LastBit = 0;
	(*Old_CRC) ^= NewChr;
	for(i=0; i<8; i++)
	{
		LastBit = (*Old_CRC) & 0x01;
		(*Old_CRC) = ((*Old_CRC)>>1) & 0x7FFF;
		if(LastBit == 1)
			(*Old_CRC)^=ModPolGenCRC16;
	}
	return (*Old_CRC);
*/
}

unsigned int Mod_Generate_CRC_U4(TypModBuffer * RtxBuff)
{
	unsigned int CRC_16=0xFFFF;
	unsigned int i = 0;
	
	for(i=0; i < (RtxBuff->nByte-2); i++)
	{
		CRC_16 = Mod_BaseCRC16_U4(CRC_16, (unsigned char)RtxBuff->Buffer[i]);
	}
	return CRC_16;
}

void Mod_CopyBuffer_U4(TypModBuffer * Destinazione, TypModBuffer * Originale)
{
	char i = 0;
	for(i=0; i<=Originale->nByte; i++)
	{
		Destinazione->Buffer[i] = Originale->Buffer[i];	// copio ogni attributo della struttura dalla struttura originale a quella di destinazione
	}
	Destinazione->nByte = Originale->nByte;
	Destinazione->Valid_Data = Originale->Valid_Data;
}


#if(K_Enable_TX_ModBus_U4_Interrupt==1)
/* NEW */
char Mod_WriteBus_U4(void)
{
	Mod_LastRx_U4.Valid_Data = 0; 								// Azzero flag di RX dato valido!
	Mod_BufferTx_U4.CntByte = 1;
    
	oRTS_UART4 = 1;											// alzo il pin dell'rts x la 485
	_U4TXIF = 0;                                            // Clear interrupt flag TX Uart1 
	_U4TXIE = 1;                                            // Attivo l'interrupt di TX Uart1

	U4TXREG = Mod_BufferTx_U4.Buffer[0];						// scrivo il primo byte nel registro di trasmissione x iniziare l'invio del bufferTX

	while (Mod_BufferTx_U4.Valid_Data == 1)					// attendo che il buffer sia completamente inviato..... -> Mod_BufferTxSec.Valid_Data == 0 vedi interrupt "U2TX_VCTR"
		continue;    

	while(!U4STAbits.TRMT)                                  // attendo la fine della trasmissione
		continue;    

	_U4TXIE = 0;                                            // Disattivo l'interrupt di TX Uart1    
    
    
	return 0;												// ritorno 0 in quanto l'operazione è andata a boun fine
}
#else
char Mod_WriteBus_U4(void)
{
	char i = 0;
	
	// Modifica del 05/01/2014
	Mod_LastRx_U4.Valid_Data = 0; 								// Azzero flag di RX dato valido!
	// Modifica del 02/11/2018  <- Argo MODBUS di Emanuense    
    iPckModRx_U4 = 0;
    

	if (Mod_BufferTx_U4.Valid_Data == 1)						// se il buffer di trasmissione ha un pacchetto validato
	{
		for(i=0; i < Mod_BufferTx_U4.nByte; i++)
		{
            putch4(Mod_BufferTx_U4.Buffer[i]);					// invio sul bus la sequenza di byte
		}
		Mod_BufferTx_U4.Valid_Data = 0;						// invalido il dato in quanto vecchio
		return 0;											// ritorno 0 in quanto l'operazione è andata a boun fine
	}
	return 1;												// ritorno 1 perche il pacchetto dati non è valido
}
#endif


void Mod_IntReadBus_U4(int RxData)
{
//	while(U2STAbits.URXDA)				// se ho un interrupt di ricezione dalla seriale
//	{
		TimeOutModRx_U4.Value = 0;			// resetto il timer comunque perchè ho appena ricevuto un carattere
		TimeOutModRx_U4.Time = K_TimeModInterChar; 	// setto il time out @ ms
		if(TimeOutModRx_U4.Enable == 0)	// verifico se il timer per il time out è già attivo (rx in corso)
			TimeOutModRx_U4.Enable = 1;	// se non lo attivo
		
		if(iPckModRx_U4 < ByteLenModBuff)
		{
			Mod_BufferRx_U4.Buffer[iPckModRx_U4] = RxData; //GL U1RXREG;	// salvo nel buffer i dati in arrivo            
			if(((iPckModRx_U4 == 0) & (Mod_BufferRx_U4.Buffer[iPckModRx_U4] == Mod_BufferTx_U4.Buffer[0])) | (iPckModRx_U4 != 0))
			{
				iPckModRx_U4+=1;							// incremento l'indice
				if(iPckModRx_U4 > 3)						// se ho completato la sequenza
				{
					if(  ((Mod_BufferRx_U4.Buffer[1] == ReadModCoil) & (iPckModRx_U4 >(4+(Mod_BufferRx_U4.Buffer[2])))) |	// Modifica del 05/01/2014 introdotto READ COIL register
						 ((Mod_BufferRx_U4.Buffer[1] == ReadModCmd) & (iPckModRx_U4 >(4+(Mod_BufferRx_U4.Buffer[2])))) |
						 ((Mod_BufferRx_U4.Buffer[1] == ReadModInput) & (iPckModRx_U4 >(4+(Mod_BufferRx_U4.Buffer[2])))) |
						 ((Mod_BufferRx_U4.Buffer[1] == WriteModCoil) & (iPckModRx_U4 >7)) |							
						 ((Mod_BufferRx_U4.Buffer[1] == WriteModCmd) & (iPckModRx_U4 >7)) |
						 ((Mod_BufferRx_U4.Buffer[1] == WriteModListCmd) & (iPckModRx_U4 >7)))
					{
						Mod_BufferRx_U4.nByte = iPckModRx_U4;
						iPckModRx_U4 = 0;					// azzero l'indice di ricezione in quanto il pacchetto è terminato
						TimeOutModRx_U4.Enable = 0;		// fermo il timer del time out
						TimeOutModRx_U4.Value = 0;			// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
						TimeOutModRx_U4.TimeOut = 0;		// e resetto il flag di time out
						Mod_BufferRx_U4.Valid_Data = 1;	// dico che ho ricevuto un dato valido
					}
					else
					{
						if(	(Mod_BufferRx_U4.Buffer[1] != ReadModCoil) & (Mod_BufferRx_U4.Buffer[1] != ReadModCmd) & (Mod_BufferRx_U4.Buffer[1] != ReadModInput) & 
							(Mod_BufferRx_U4.Buffer[1] != WriteModCoil) &(Mod_BufferRx_U4.Buffer[1] != WriteModCmd) & (Mod_BufferRx_U4.Buffer[1] != WriteModListCmd))
						{
							iPckModRx_U4 = 0;				// azzero l'indice di ricezione in quanto il pacchetto sfora il buffer di rx
							TimeOutModRx_U4.Enable = 0;	// fermo il timer del time out
							TimeOutModRx_U4.Value = 0;		// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
							TimeOutModRx_U4.TimeOut = 0;	// e resetto il flag di time out
						}
					}
				}
			}
		}
		else
		{
			iPckModRx_U4 = 0;				// azzero l'indice di ricezione in quanto il pacchetto sfora il buffer di rx
			TimeOutModRx_U4.Enable = 0;	// fermo il timer del time out
			TimeOutModRx_U4.Value = 0;		// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
			TimeOutModRx_U4.TimeOut = 0;	// e resetto il flag di time out
		}
	//IFS1bits.U2RXIF = 0;
	//}
}

void Mod_IntTimeOutRx_U4(void)
{
	if(TimeOutModRx_U4.Enable ==1)				// controllo se il timer è attivo
	{
		TimeOutModRx_U4.Value +=1;				// gestisco il conteggio del timer
		if(TimeOutModRx_U4.Value >= TimeOutModRx_U4.Time)		// gestisco il time out
		{
			TimeOutModRx_U4.Enable = 0;		// fermo il timer del time out
			TimeOutModRx_U4.Value = 0;			// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
			while(U4STAbits.URXDA)			// svuoto il buffer di ricezione
				iPckModRx_U4 = U4RXREG;
			iPckModRx_U4 = 0;					// azzero l'indice di ricezione in quanto presumo che si sia inetrrotta la comunicazione
		}
	}
}

void Mod_Recive_Cmd_U4(void)
{
	unsigned int Temp = 0;								// temporaneo per il calcolo del crc e l'isolamento dei dati dal buffer
	unsigned int CRC16_calcolato = 0;


	if(Mod_BufferRx_U4.Valid_Data == 1)			// controllo se ho finito di ricevere un pacchetto
	{
		Mod_CopyBuffer_U4(&Mod_LastRx_U4, &Mod_BufferRx_U4);	// copio quello che ho ricevuto così da liberare il buffer di ricezione
		iPckModRx_U4 = 0;							// reinizializzo l'indice 
		Mod_BufferRx_U4.Valid_Data = 0;			// resetto la validità del buffer di rx in quanto lo già copiato per analizzarlo
		Mod_LastRx_U4.Valid_Data = 0;				// resetto la validità del dato, sarà il controllo a giudicarne la validità

		CRC16_calcolato = Mod_Generate_CRC_U4(&Mod_LastRx_U4);				// mi calcolo il CRC del pacchetto

		Temp = ((Mod_LastRx_U4.Buffer[(Mod_LastRx_U4.nByte)-1])<<8)&0xFF00; //(*0x100);		// recupero la parte alta del CRC dal bufer dei dati ricevuti  // Modifica 05/01/2015 Aggiunta mascheratura byte shift
		Temp = Temp | ((Mod_LastRx_U4.Buffer[(Mod_LastRx_U4.nByte)-2])&0x00FF);	// recupero la parte bassa del CRC dal bufer dei dati ricevuti
		
		if(CRC16_calcolato == Temp)									// se è uguale a quello ricevuto il pacchetto è ok
		{
			Mod_LastRx_U4.Data = ((Mod_LastRx_U4.Buffer[3]<<8)&0xFF00) | (Mod_LastRx_U4.Buffer[4]&0x00FF);
			/*
			Data1 = Mod_LastRx.Buffer[3] & 0x00FF; 
			Data2 = (char)Mod_LastRx.Buffer[4] & 0x00FF;
			Data3 = (Data1<<8) | Data2;
			Mod_LastRx.Data = Data3;
			*/
			Mod_LastRx_U4.Valid_Data = 1;								// a questo punto devo analizzare il pacchetto
		}
        else 
        {
			ModCRC_ComErr_U4++;
			EngineBox[0].ComError.ModCRC_ComErr_U4++;
        }
	}
}

void Mod_Write_Cmd_U4(char Address, char Comando, int Registro, int Data)
/*	Address => Indirizzo del dispositivo a cui è rivolto il comando 
	Comando => codice del comando ModBus
	Registro => Registro di partenza o unico del comando
	Data =>		Func 3/4-> Numero di registri da leggere
				Func 6	-> Valore del registro da settare
				Func 16	-> Numero di registri da scrivere
*/
{
	volatile int CRC = 0;
	volatile char ReturnValue = 0;
	//volatile char Indice = 0;
	
	int TxDataCoil;
	
	switch(Comando)
	{												// Attenzione non togliere i case per "Read" servono!!! (Sara' da sistemare)
		case ReadModCoil:							// Read Coil register
		case ReadModCmd:							// Read Output register
		case ReadModInput:							// Read Input register
			Mod_BufferTx_U4.nByte = 8;									// la richiesta è formata da 8 byte
			Mod_BufferTx_U4.Buffer[0] = Address;						// il primo è l'indirizzo del destinatario
			Mod_BufferTx_U4.Buffer[1] = Comando;						// il comando 
			Mod_BufferTx_U4.Buffer[2] = (char)(((Registro & 0xFF00)>>8)&0x00FF);// il registro di partenza (hi byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTx_U4.Buffer[3] = (char)(Registro & 0x00FF);		// il registro di partenza (lo byte)
			Mod_BufferTx_U4.Buffer[4] = (char)(((Data & 0xFF00)>>8)&0x00FF);	// il numero di registri da leggere (hi byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTx_U4.Buffer[5] = (char)(Data & 0x00FF);			// il numero di registri da leggere (lo byte)
	
			CRC = Mod_Generate_CRC_U4(&Mod_BufferTx_U4);					// calcolo il CRC
			Mod_BufferTx_U4.Buffer[6] = (char)(CRC & 0x00FF);			// CRC (Hi Byte)
			Mod_BufferTx_U4.Buffer[7] = (char)(((CRC & 0xFF00)>>8)&0x00FF);		// CRC (Lo BHyte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift

			Mod_BufferTx_U4.Valid_Data = 1;			// setto il flag per la richiesta della trasmissione
			break;
			
		case WriteModCoil:							// Write Single Coil		
			
			if(Data>0) 
				TxDataCoil = 0xFF00;
			
			Mod_BufferTx_U4.nByte = 8;									// la richiesta è formata da 8 byte
			Mod_BufferTx_U4.Buffer[0] = Address;						// il primo è l'indirizzo del destinatario
			Mod_BufferTx_U4.Buffer[1] = Comando;						// il comando 
			Mod_BufferTx_U4.Buffer[2] = (char)(((Registro & 0xFF00)>>8)&0x00FF);// il registro di partenza (hi byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTx_U4.Buffer[3] = (char)(Registro & 0x00FF);		// il registro di partenza (lo byte)
			Mod_BufferTx_U4.Buffer[4] = (char)(((TxDataCoil & 0xFF00)>>8)&0x00FF);	// il numero di registri da leggere (hi byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTx_U4.Buffer[5] = (char)(TxDataCoil & 0x00FF);			// il numero di registri da leggere (lo byte)
	
			CRC = Mod_Generate_CRC_U4(&Mod_BufferTx_U4);					// calcolo il CRC
			Mod_BufferTx_U4.Buffer[6] = (char)(CRC & 0x00FF);			// CRC (Hi Byte)
			Mod_BufferTx_U4.Buffer[7] = (char)(((CRC & 0xFF00)>>8)&0x00FF);		// CRC (Lo BHyte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift

			Mod_BufferTx_U4.Valid_Data = 1;							// setto il flag per la richiesta della trasmissione
			break;

		case WriteModCmd:											// Prese Single register
			Mod_BufferTx_U4.nByte = 8;									// la richiesta è formata da 8 byte
			Mod_BufferTx_U4.Buffer[0] = Address;						// il primo è l'indirizzo del destinatario
			Mod_BufferTx_U4.Buffer[1] = Comando;						// il comando 
			Mod_BufferTx_U4.Buffer[2] = (char)(((Registro & 0xFF00)>>8)&0x00FF);// il registro da settare (Hi Byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTx_U4.Buffer[3] = (char)(Registro & 0x00FF);		// il registro da settare (Lo Byte)
			Mod_BufferTx_U4.Buffer[4] = (char)(((Data & 0xFF00)>>8)&0x00FF);	// valore da settare (Hi Byte)		// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTx_U4.Buffer[5] = (char)(Data & 0x00FF);			// valore da settare (Lo Byte)
	
			CRC = Mod_Generate_CRC_U4(&Mod_BufferTx_U4);					// calcolo il CRC
			Mod_BufferTx_U4.Buffer[6] = (char)(CRC & 0x00FF);			// CRC (Hi Byte)
			Mod_BufferTx_U4.Buffer[7] = (char)(((CRC & 0xFF00)>>8)&0x00FF);		// CRC (Lo Byte)					// Modifica 05/01/2015 Aggiunta mascheratura byte shift
	
			Mod_BufferTx_U4.Valid_Data = 1;							// setto il flag per la richiesta della trasmissione
			break;

		case WriteModListCmd:										//Preset multiple register
			Mod_BufferTx_U4.nByte = 9+(2*Data);						// la richiesta è formata da 8 byte
			Mod_BufferTx_U4.Buffer[0] = Address;						// il primo è l'indirizzo del destinatario
			Mod_BufferTx_U4.Buffer[1] = Comando;						// il comando 
			Mod_BufferTx_U4.Buffer[2] = (char)(((Registro & 0xFF00)>>8)&0x00FF);// il registro da settare (Hi Byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTx_U4.Buffer[3] = (char)(Registro & 0x00FF);		// il registro da settare (Lo Byte)
			Mod_BufferTx_U4.Buffer[4] = (char)(((Data & 0xFF00)>>8)&0x00FF);	// Numero di registri da settare (Hi Byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTx_U4.Buffer[5] = (char)(Data & 0x00FF);			// Numero di registri da settare (Lo Byte)
			Mod_BufferTx_U4.Buffer[6] = Data*2;						// Numero di byte dati che seguono
	
			CRC = Mod_Generate_CRC_U4(&Mod_BufferTx_U4);					// calcolo il CRC
			Mod_BufferTx_U4.Buffer[Mod_BufferTx_U4.nByte-2] = (char)(CRC & 0x00FF);		// CRC (Hi Byte)
			Mod_BufferTx_U4.Buffer[Mod_BufferTx_U4.nByte-1] = (char)(((CRC & 0xFF00)>>8)&0x00FF);	// CRC (Lo Byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
	
			Mod_BufferTx_U4.Valid_Data = 1;			// setto il flag per la richiesta della trasmissione
			break;
	}
	ReturnValue = Mod_WriteBus_U4();					// trasmetto la richiesta
}

#endif  //#if (K_AbilMODBCAN==1)  
