//----------------------------------------------------------------------------------
//	Progect name:	Driver_ModBus.c
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			Emanuele
//	Date:			18/06/2011
//	Description:	Corpo delle funzioni pilota della comunicazione ModBus
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
#include "Driver_ModBus.h"
#include "ProtocolloModBus.h"

extern unsigned char  CurUsedProtocolId;      // Id del protocollo correntemente utilizzato su UART1 / UART2
extern volatile int ModCRC_ComErr; 
extern volatile int ModCRC_ComErr_U2;

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
unsigned int Mod_BaseCRC16(unsigned int Old_CRC, unsigned char NewChr)
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

unsigned int Mod_Generate_CRC(TypModBuffer * RtxBuff)
{
	unsigned int CRC_16=0xFFFF;
	unsigned int i = 0;
	
	for(i=0; i < (RtxBuff->nByte-2); i++)
	{
		CRC_16 = Mod_BaseCRC16(CRC_16, (unsigned char)RtxBuff->Buffer[i]);
	}
	return CRC_16;
}

void Mod_CopyBuffer(TypModBuffer * Destinazione, TypModBuffer * Originale)
{
	char i = 0;
	for(i=0; i<=Originale->nByte; i++)
	{
		Destinazione->Buffer[i] = Originale->Buffer[i];	// copio ogni attributo della struttura dalla struttura originale a quella di destinazione
	}
	Destinazione->nByte = Originale->nByte;
	Destinazione->Valid_Data = Originale->Valid_Data;
}


#if(K_Enable_TX_ModBus_Interrupt==1)
/* NEW */
char Mod_WriteBus(void)
{
	Mod_LastRx.Valid_Data = 0; 								// Azzero flag di RX dato valido!
	Mod_BufferTx.CntByte = 1;
    
    if(CurUsedProtocolId==FlagModBusProt_BUSS)
    {
        oRTS_Slave = 1;											// alzo il pin dell'rts x la 485
        _U2TXIF = 0;                                            // Clear interrupt flag TX Uart2 
        _U2TXIE = 1;                                            // Attivo l'interrupt di TX Uart2
    
        U2TXREG = Mod_BufferTx.Buffer[0];						// scrivo il primo byte nel registro di trasmissione x iniziare l'invio del bufferTX

        while (Mod_BufferTx.Valid_Data == 1)					// attendo che il buffer sia completamente inviato..... -> Mod_BufferTxSec.Valid_Data == 0 vedi interrupt "U2TX_VCTR"
            continue;    

        while(!U2STAbits.TRMT)                                  // attendo la fine della trasmissione
            continue;    

        _U2TXIE = 0;                                            // Disattivo l'interrupt di TX Uart2    
        //oRTS_Slave = 0;											// abbasso il pin dell'rts x la 485
    }
    else if(CurUsedProtocolId==FlagModBusProt_BUSM)
    {
        oRTS_Master = 1;										// alzo il pin dell'rts x la 485
        _U1TXIF = 0;                                            // Clear interrupt flag TX Uart1 
        _U1TXIE = 1;                                            // Attivo l'interrupt di TX Uart1
    
        U1TXREG = Mod_BufferTx.Buffer[0];						// scrivo il primo byte nel registro di trasmissione x iniziare l'invio del bufferTX

        while (Mod_BufferTx.Valid_Data == 1)					// attendo che il buffer sia completamente inviato..... -> Mod_BufferTxSec.Valid_Data == 0 vedi interrupt "U2TX_VCTR"
            continue;    

        while(!U1STAbits.TRMT)                                  // attendo la fine della trasmissione
            continue;    

        _U1TXIE = 0;                                            // Disattivo l'interrupt di TX Uart1    
        //oRTS_Master = 0;										// abbasso il pin dell'rts x la 485        
    }
    
    
	return 0;												// ritorno 0 in quanto l'operazione è andata a boun fine
}
#else
char Mod_WriteBus(void)
{
	char i = 0;
	
	// Modifica del 05/01/2014
	Mod_LastRx.Valid_Data = 0; 								// Azzero flag di RX dato valido!
	// Modifica del 02/11/2018  <- Argo MODBUS di Emanuense    
    iPckModRx = 0;
    

	if (Mod_BufferTx.Valid_Data == 1)						// se il buffer di trasmissione ha un pacchetto validato
	{
		for(i=0; i < Mod_BufferTx.nByte; i++)
		{
            if(CurUsedProtocolId==FlagModBusProt_BUSM)    
                putch1(Mod_BufferTx.Buffer[i]);					// invio sul bus la sequenza di byte
            if(CurUsedProtocolId==FlagModBusProt_BUSS)    
                putch2(Mod_BufferTx.Buffer[i]);					// invio sul bus la sequenza di byte
		}
		Mod_BufferTx.Valid_Data = 0;						// invalido il dato in quanto vecchio
		return 0;											// ritorno 0 in quanto l'operazione è andata a boun fine
	}
	return 1;												// ritorno 1 perche il pacchetto dati non è valido
}
#endif


void Mod_IntReadBus(int RxData)
{
//	while(U2STAbits.URXDA)				// se ho un interrupt di ricezione dalla seriale
//	{
		TimeOutModRx.Value = 0;			// resetto il timer comunque perchè ho appena ricevuto un carattere
		TimeOutModRx.Time = K_TimeModInterChar; 	// setto il time out @ ms
		if(TimeOutModRx.Enable == 0)	// verifico se il timer per il time out è già attivo (rx in corso)
			TimeOutModRx.Enable = 1;	// se non lo attivo
		
		if(iPckModRx <ByteLenModBuff)
		{
			Mod_BufferRx.Buffer[iPckModRx] = RxData; //GL U1RXREG;	// salvo nel buffer i dati in arrivo            
			if(((iPckModRx == 0) & (Mod_BufferRx.Buffer[iPckModRx] == Mod_BufferTx.Buffer[0])) | (iPckModRx != 0))
			{
				iPckModRx+=1;							// incremento l'indice
				if(iPckModRx > 3)						// se ho completato la sequenza
				{
					if(  ((Mod_BufferRx.Buffer[1] == ReadModCoil) & (iPckModRx >(4+(Mod_BufferRx.Buffer[2])))) |	// Modifica del 05/01/2014 introdotto READ COIL register
						 ((Mod_BufferRx.Buffer[1] == ReadModCmd) & (iPckModRx >(4+(Mod_BufferRx.Buffer[2])))) |
						 ((Mod_BufferRx.Buffer[1] == ReadModInput) & (iPckModRx >(4+(Mod_BufferRx.Buffer[2])))) |
						 ((Mod_BufferRx.Buffer[1] == WriteModCoil) & (iPckModRx >7)) |							
						 ((Mod_BufferRx.Buffer[1] == WriteModCmd) & (iPckModRx >7)) |
						 ((Mod_BufferRx.Buffer[1] == WriteModListCmd) & (iPckModRx >7)))
					{
						Mod_BufferRx.nByte = iPckModRx;
						iPckModRx = 0;					// azzero l'indice di ricezione in quanto il pacchetto è terminato
						TimeOutModRx.Enable = 0;		// fermo il timer del time out
						TimeOutModRx.Value = 0;			// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
						TimeOutModRx.TimeOut = 0;		// e resetto il flag di time out
						Mod_BufferRx.Valid_Data = 1;	// dico che ho ricevuto un dato valido
					}
					else
					{
						if(	(Mod_BufferRx.Buffer[1] != ReadModCoil) & (Mod_BufferRx.Buffer[1] != ReadModCmd) & (Mod_BufferRx.Buffer[1] != ReadModInput) & 
							(Mod_BufferRx.Buffer[1] != WriteModCoil) &(Mod_BufferRx.Buffer[1] != WriteModCmd) & (Mod_BufferRx.Buffer[1] != WriteModListCmd))
						{
							iPckModRx = 0;				// azzero l'indice di ricezione in quanto il pacchetto sfora il buffer di rx
							TimeOutModRx.Enable = 0;	// fermo il timer del time out
							TimeOutModRx.Value = 0;		// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
							TimeOutModRx.TimeOut = 0;	// e resetto il flag di time out
						}
					}
				}
			}
		}
		else
		{
			iPckModRx = 0;				// azzero l'indice di ricezione in quanto il pacchetto sfora il buffer di rx
			TimeOutModRx.Enable = 0;	// fermo il timer del time out
			TimeOutModRx.Value = 0;		// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
			TimeOutModRx.TimeOut = 0;	// e resetto il flag di time out
		}
	//IFS1bits.U2RXIF = 0;
	//}
}

void Mod_IntTimeOutRx(void)
{
	if(TimeOutModRx.Enable ==1)				// controllo se il timer è attivo
	{
		TimeOutModRx.Value +=1;				// gestisco il conteggio del timer
		if(TimeOutModRx.Value >= TimeOutModRx.Time)		// gestisco il time out
		{
			TimeOutModRx.Enable = 0;		// fermo il timer del time out
			TimeOutModRx.Value = 0;			// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
			while(U2STAbits.URXDA)			// svuoto il buffer di ricezione
				iPckModRx = U2RXREG;
			iPckModRx = 0;					// azzero l'indice di ricezione in quanto presumo che si sia inetrrotta la comunicazione
		}
	}
}

void Mod_Recive_Cmd(void)
{
	unsigned int Temp = 0;								// temporaneo per il calcolo del crc e l'isolamento dei dati dal buffer
	unsigned int CRC16_calcolato = 0;


	if(Mod_BufferRx.Valid_Data == 1)			// controllo se ho finito di ricevere un pacchetto
	{
		Mod_CopyBuffer(&Mod_LastRx, &Mod_BufferRx);	// copio quello che ho ricevuto così da liberare il buffer di ricezione
		iPckModRx = 0;							// reinizializzo l'indice 
		Mod_BufferRx.Valid_Data = 0;			// resetto la validità del buffer di rx in quanto lo già copiato per analizzarlo
		Mod_LastRx.Valid_Data = 0;				// resetto la validità del dato, sarà il controllo a giudicarne la validità

		CRC16_calcolato = Mod_Generate_CRC(&Mod_LastRx);				// mi calcolo il CRC del pacchetto

		Temp = ((Mod_LastRx.Buffer[(Mod_LastRx.nByte)-1])<<8)&0xFF00; //(*0x100);		// recupero la parte alta del CRC dal bufer dei dati ricevuti  // Modifica 05/01/2015 Aggiunta mascheratura byte shift
		Temp = Temp | ((Mod_LastRx.Buffer[(Mod_LastRx.nByte)-2])&0x00FF);	// recupero la parte bassa del CRC dal bufer dei dati ricevuti
		
		if(CRC16_calcolato == Temp)									// se è uguale a quello ricevuto il pacchetto è ok
		{
			Mod_LastRx.Data = ((Mod_LastRx.Buffer[3]<<8)&0xFF00) | (Mod_LastRx.Buffer[4]&0x00FF);
			/*
			Data1 = Mod_LastRx.Buffer[3] & 0x00FF; 
			Data2 = (char)Mod_LastRx.Buffer[4] & 0x00FF;
			Data3 = (Data1<<8) | Data2;
			Mod_LastRx.Data = Data3;
			*/
			Mod_LastRx.Valid_Data = 1;								// a questo punto devo analizzare il pacchetto
		}
        else 
        {
            if(CurUsedProtocolId==FlagModBusProt_BUSM)    
            {
                ModCRC_ComErr++;
                EngineBox[0].ComError.ModCRC_ComErr++;
            }
            if(CurUsedProtocolId==FlagModBusProt_BUSS)      
            {
                ModCRC_ComErr_U2++;
                EngineBox[0].ComError.ModCRC_ComErr_U2++;
            }
        }
	}
}

void Mod_Write_Cmd(char Address, char Comando, int Registro, int Data)
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
			Mod_BufferTx.nByte = 8;									// la richiesta è formata da 8 byte
			Mod_BufferTx.Buffer[0] = Address;						// il primo è l'indirizzo del destinatario
			Mod_BufferTx.Buffer[1] = Comando;						// il comando 
			Mod_BufferTx.Buffer[2] = (char)(((Registro & 0xFF00)>>8)&0x00FF);// il registro di partenza (hi byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTx.Buffer[3] = (char)(Registro & 0x00FF);		// il registro di partenza (lo byte)
			Mod_BufferTx.Buffer[4] = (char)(((Data & 0xFF00)>>8)&0x00FF);	// il numero di registri da leggere (hi byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTx.Buffer[5] = (char)(Data & 0x00FF);			// il numero di registri da leggere (lo byte)
	
			CRC = Mod_Generate_CRC(&Mod_BufferTx);					// calcolo il CRC
			Mod_BufferTx.Buffer[6] = (char)(CRC & 0x00FF);			// CRC (Hi Byte)
			Mod_BufferTx.Buffer[7] = (char)(((CRC & 0xFF00)>>8)&0x00FF);		// CRC (Lo BHyte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift

			Mod_BufferTx.Valid_Data = 1;			// setto il flag per la richiesta della trasmissione
			break;
			
		case WriteModCoil:							// Write Single Coil		
			
			if(Data>0) 
				TxDataCoil = 0xFF00;
			
			Mod_BufferTx.nByte = 8;									// la richiesta è formata da 8 byte
			Mod_BufferTx.Buffer[0] = Address;						// il primo è l'indirizzo del destinatario
			Mod_BufferTx.Buffer[1] = Comando;						// il comando 
			Mod_BufferTx.Buffer[2] = (char)(((Registro & 0xFF00)>>8)&0x00FF);// il registro di partenza (hi byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTx.Buffer[3] = (char)(Registro & 0x00FF);		// il registro di partenza (lo byte)
			Mod_BufferTx.Buffer[4] = (char)(((TxDataCoil & 0xFF00)>>8)&0x00FF);	// il numero di registri da leggere (hi byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTx.Buffer[5] = (char)(TxDataCoil & 0x00FF);			// il numero di registri da leggere (lo byte)
	
			CRC = Mod_Generate_CRC(&Mod_BufferTx);					// calcolo il CRC
			Mod_BufferTx.Buffer[6] = (char)(CRC & 0x00FF);			// CRC (Hi Byte)
			Mod_BufferTx.Buffer[7] = (char)(((CRC & 0xFF00)>>8)&0x00FF);		// CRC (Lo BHyte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift

			Mod_BufferTx.Valid_Data = 1;							// setto il flag per la richiesta della trasmissione
			break;

		case WriteModCmd:											// Prese Single register
			Mod_BufferTx.nByte = 8;									// la richiesta è formata da 8 byte
			Mod_BufferTx.Buffer[0] = Address;						// il primo è l'indirizzo del destinatario
			Mod_BufferTx.Buffer[1] = Comando;						// il comando 
			Mod_BufferTx.Buffer[2] = (char)(((Registro & 0xFF00)>>8)&0x00FF);// il registro da settare (Hi Byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTx.Buffer[3] = (char)(Registro & 0x00FF);		// il registro da settare (Lo Byte)
			Mod_BufferTx.Buffer[4] = (char)(((Data & 0xFF00)>>8)&0x00FF);	// valore da settare (Hi Byte)		// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTx.Buffer[5] = (char)(Data & 0x00FF);			// valore da settare (Lo Byte)
	
			CRC = Mod_Generate_CRC(&Mod_BufferTx);					// calcolo il CRC
			Mod_BufferTx.Buffer[6] = (char)(CRC & 0x00FF);			// CRC (Hi Byte)
			Mod_BufferTx.Buffer[7] = (char)(((CRC & 0xFF00)>>8)&0x00FF);		// CRC (Lo Byte)					// Modifica 05/01/2015 Aggiunta mascheratura byte shift
	
			Mod_BufferTx.Valid_Data = 1;							// setto il flag per la richiesta della trasmissione
			break;

		case WriteModListCmd:										//Preset multiple register
			Mod_BufferTx.nByte = 9+(2*Data);						// la richiesta è formata da 8 byte
			Mod_BufferTx.Buffer[0] = Address;						// il primo è l'indirizzo del destinatario
			Mod_BufferTx.Buffer[1] = Comando;						// il comando 
			Mod_BufferTx.Buffer[2] = (char)(((Registro & 0xFF00)>>8)&0x00FF);// il registro da settare (Hi Byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTx.Buffer[3] = (char)(Registro & 0x00FF);		// il registro da settare (Lo Byte)
			Mod_BufferTx.Buffer[4] = (char)(((Data & 0xFF00)>>8)&0x00FF);	// Numero di registri da settare (Hi Byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTx.Buffer[5] = (char)(Data & 0x00FF);			// Numero di registri da settare (Lo Byte)
			Mod_BufferTx.Buffer[6] = Data*2;						// Numero di byte dati che seguono
	
			CRC = Mod_Generate_CRC(&Mod_BufferTx);					// calcolo il CRC
			Mod_BufferTx.Buffer[Mod_BufferTx.nByte-2] = (char)(CRC & 0x00FF);		// CRC (Hi Byte)
			Mod_BufferTx.Buffer[Mod_BufferTx.nByte-1] = (char)(((CRC & 0xFF00)>>8)&0x00FF);	// CRC (Lo Byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
	
			Mod_BufferTx.Valid_Data = 1;			// setto il flag per la richiesta della trasmissione
			break;
	}
	ReturnValue = Mod_WriteBus();					// trasmetto la richiesta
}
