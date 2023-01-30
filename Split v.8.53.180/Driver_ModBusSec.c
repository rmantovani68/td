//----------------------------------------------------------------------------------
//	Progect name:	Driver_ModBus.c
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			18/06/2011
//	Description:	Corpo delle funzioni pilota della comunicazione ModBus
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
#include "Driver_ModBusSec.h"
#include "ProtocolloModBusSec.h"
#include "ProtocolloComunicazione.h"
#include "Driver_ComunicazioneSec.h"

//extern volatile int ModCntComErrSec;
extern volatile int ModTimeOutComErrSec;
extern volatile int ModTimeOutInterCharComErrSec;
extern volatile int ModCRC_ComErrSec;

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
unsigned int Mod_BaseCRC16Sec(unsigned int Old_CRC, unsigned char NewChr)
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
	Old_CRC ^= NewChr;
	for(i=0; i<8; i++)
	{
		LastBit = Old_CRC & 0x01;
		Old_CRC = (Old_CRC>>1) & 0x7FFF;
		if(LastBit == 1)
			Old_CRC^=ModPolGenCRC16;
	}
	return Old_CRC;
*/
}

unsigned int Mod_Generate_CRCSec(volatile TypModBufferSec * RtxBuff)
{
	unsigned int CRC_16=0xFFFF;
	unsigned int i = 0;
	
	for(i=0; i < (RtxBuff->nByte-2); i++)
	{
		CRC_16 = Mod_BaseCRC16Sec(CRC_16, (unsigned char)RtxBuff->Buffer[i]);
	}
	return CRC_16;
}

void Mod_CopyBufferSec(volatile TypModBufferSec * Destinazione, volatile TypModBufferSec * Originale)
{
	unsigned char i = 0;
    
	for(i=0; i<=Originale->nByte; i++)
	{
		Destinazione->Buffer[i] = Originale->Buffer[i];	// copio ogni attributo della struttura dalla struttura originale a quella di destinazione
	}
	Destinazione->nByte = Originale->nByte;
	Destinazione->Valid_Data = Originale->Valid_Data;
}

/* ORI 
char Mod_WriteBusSec(void)
{
	char i = 0;
	
	// Modifica del 05/01/2014
	Mod_LastRxSec.Valid_Data = 0; 								// Azzero flag di RX dato valido!

	if (Mod_BufferTxSec.Valid_Data == 1)						// se il buffer di trasmissione ha un pacchetto validato
	{
		for(i=0; i < Mod_BufferTxSec.nByte; i++)
		{
			putch2(Mod_BufferTxSec.Buffer[i]);					// invio sul bus la sequenza di byte
		}
		Mod_BufferTxSec.Valid_Data = 0;						// invalido il dato in quanto vecchio
		return 0;											// ritorno 0 in quanto l'operazione è andata a boun fine
	}
	return 1;												// ritorno 1 perche il pacchetto dati non è valido
}
*/ 

/* OLD
char Mod_WriteBusSec(void)
{

	// Modifica del 05/01/2014
	Mod_LastRxSec.Valid_Data = 0; 								// Azzero flag di RX dato valido!

	IEC1bits.U2TXIE = 1;									// Attivo l'interrupt di TX Uart2

	oRTS_Slave = 1;											// alzo il pin dell'rts x la 485
	U2TXREG = Mod_BufferTxSec.Buffer[0];						// scrivo il primo byte nel registro di trasmissione x iniziare l'invio del bufferTX
	Mod_BufferTxSec.CntByte = 1;
	while (Mod_BufferTxSec.Valid_Data == 1)					// attendo che il buffer sia completamente inviato..... -> Mod_BufferTxSec.Valid_Data == 0 vedi interrupt "U2TX_VCTR"
		__builtin_nop();					

	while(!U2STAbits.TRMT)                                  // attendo la fine della trasmissione
		continue;    
    
	oRTS_Slave = 0;											// abbasso il pin dell'rts x la 485
	IEC1bits.U2TXIE = 0;									// Disattivo l'interrupt di TX Uart2
	
	return 0;												// ritorno 0 in quanto l'operazione è andata a boun fine
}
 */

/* NEW */
char Mod_WriteBusSec(void)
{
	Mod_LastRxSec.Valid_Data = 0; 								// Azzero flag di RX dato valido!
	Mod_BufferTxSec.CntByte = 1;
    
	oRTS_Slave = 1;											// alzo il pin dell'rts x la 485
    
    IFS1bits.U2TXIF = 0;									// Clear interrupt flag TX Uart2 
	IEC1bits.U2TXIE = 1;									// Attivo l'interrupt di TX Uart2
	U2TXREG = Mod_BufferTxSec.Buffer[0];						// scrivo il primo byte nel registro di trasmissione x iniziare l'invio del bufferTX

	while (Mod_BufferTxSec.Valid_Data == 1)					// attendo che il buffer sia completamente inviato..... -> Mod_BufferTxSec.Valid_Data == 0 vedi interrupt "U2TX_VCTR"
		continue;    

	while(!U2STAbits.TRMT)                                  // attendo la fine della trasmissione
		continue;    
    
	IEC1bits.U2TXIE = 0;									// Disattivo l'interrupt di TX Uart2    
	//oRTS_Slave = 0;											// abbasso il pin dell'rts x la 485
	return 0;												// ritorno 0 in quanto l'operazione è andata a boun fine
}

char Mod_WriteBusU3(void)
{
	Mod_LastRxSec.Valid_Data = 0; 							// Azzero flag di RX dato valido!
	Mod_BufferTxSec.CntByte = 1;
    
	oRTS_UART3 = 1;											// alzo il pin dell'rts x la 485
    
    _U3TXIF = 0;                                            // Clear interrupt flag TX Uart3 
	_U3TXIE = 1;                                            // Attivo l'interrupt di TX Uart3
	U3TXREG = Mod_BufferTxSec.Buffer[0];					// scrivo il primo byte nel registro di trasmissione x iniziare l'invio del bufferTX

	while (Mod_BufferTxSec.Valid_Data == 1)					// attendo che il buffer sia completamente inviato..... -> Mod_BufferTxSec.Valid_Data == 0 vedi interrupt "U2TX_VCTR"
		continue;    

	while(!U3STAbits.TRMT)                                  // attendo la fine della trasmissione
		continue;    
    
	_U3TXIE = 0;                                            // Disattivo l'interrupt di TX Uart3    
	//oRTS_UART3 = 0;											// abbasso il pin dell'rts x la 485
	return 0;												// ritorno 0 in quanto l'operazione è andata a boun fine
}

void Mod_IntReadBusSec(void)
{
    unsigned int flag;
    
    if(Me.ComToUart3==1)                    
    	flag=U3STAbits.URXDA;			// se ho un interrupt di ricezione dalla seriale
                                        // Serve veramente verificare il flag di interrupt????? 
                                        // Questa routine viene chiamata all'interno della routine di interrupt RX!!!
    else
    	flag=U2STAbits.URXDA;			// se ho un interrupt di ricezione dalla seriale    
                                        // Serve veramente verificare il flag di interrupt????? 
                                        // Questa routine viene chiamata all'interno della routine di interrupt RX!!!
        
  	while(flag)				// se ho un interrupt di ricezione dalla seriale                
	{
		TimeOutModRxSec.Value = 0;      // resetto il timer comunque perchè ho appena ricevuto un carattere
		TimeOutModRxSec.Time = K_TimeModInterCharSec;	// setto il time out intercarattere (dovrebbe essere impostato ad almeno 3.5 volte il tempo di un byte @ BaudRate corrente)
		if(TimeOutModRxSec.Enable == 0)	// verifico se il timer per il time out è già attivo (rx in corso)
			TimeOutModRxSec.Enable = 1;	// se non lo attivo
		
		if(iPckModRxSec <ByteLenModBuffSec)
		{
            if(Me.ComToUart3==1)                        
        		Mod_BufferRxSec.Buffer[iPckModRxSec] = U3RXREG;	// salvo nel buffer i dati in arrivo		
            else
            	Mod_BufferRxSec.Buffer[iPckModRxSec] = U2RXREG;	// salvo nel buffer i dati in arrivo		                

			if(((iPckModRxSec == 0) & (Mod_BufferRxSec.Buffer[iPckModRxSec] == Mod_BufferTxSec.Buffer[0])) | (iPckModRxSec != 0))
			{
				iPckModRxSec+=1;							// incremento l'indice
				if(iPckModRxSec > 3)						// se ho completato la sequenza
				{
					if(  ((Mod_BufferRxSec.Buffer[1] == ReadModCoil) && (iPckModRxSec >(4+(Mod_BufferRxSec.Buffer[2])))) ||	// Modifica del 05/01/2014 introdotto READ COIL register
                         ((Mod_BufferRxSec.Buffer[1] == ReadModInputStatus) && (iPckModRxSec >(4+(Mod_BufferRxSec.Buffer[2])))) ||                          
						 ((Mod_BufferRxSec.Buffer[1] == ReadModCmd) && (iPckModRxSec >(4+(Mod_BufferRxSec.Buffer[2])))) ||
						 ((Mod_BufferRxSec.Buffer[1] == ReadModInput) && (iPckModRxSec >(4+(Mod_BufferRxSec.Buffer[2])))) ||
						 ((Mod_BufferRxSec.Buffer[1] == WriteModForceSingleCoil) && (iPckModRxSec >7)) ||							
						 ((Mod_BufferRxSec.Buffer[1] == WriteModCmd) && (iPckModRxSec >7)) ||
						 ((Mod_BufferRxSec.Buffer[1] == WriteModListCmd) && (iPckModRxSec >7)))
					{
						Mod_BufferRxSec.nByte = iPckModRxSec;
						iPckModRxSec = 0;					// azzero l'indice di ricezione in quanto il pacchetto è terminato
						TimeOutModRxSec.Enable = 0;		// fermo il timer del time out
						TimeOutModRxSec.Value = 0;			// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
						TimeOutModRxSec.TimeOut = 0;		// e resetto il flag di time out
#if(k_Abil_Panel_Shared==1)                        
                        if(WaitingReplaySec == 1)
#endif                            
                            Mod_BufferRxSec.Valid_Data = 1;	// dico che ho ricevuto un dato valido
					}
					else
					{
                        if(	(Mod_BufferRxSec.Buffer[1] != ReadModCoil) & (Mod_BufferRxSec.Buffer[1] != ReadModInputStatus) & (Mod_BufferRxSec.Buffer[1] != ReadModCmd) & (Mod_BufferRxSec.Buffer[1] != ReadModInput) & 
										(Mod_BufferRxSec.Buffer[1] != WriteModForceSingleCoil) &(Mod_BufferRxSec.Buffer[1] != WriteModCmd) & (Mod_BufferRxSec.Buffer[1] != WriteModListCmd))
						{
							iPckModRxSec = 0;				// azzero l'indice di ricezione in quanto il pacchetto sfora il buffer di rx
							TimeOutModRxSec.Enable = 0;	// fermo il timer del time out
							TimeOutModRxSec.Value = 0;		// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
							TimeOutModRxSec.TimeOut = 0;	// e resetto il flag di time out
						}
					}
				}
			}
		}
		else
		{
			iPckModRxSec = 0;				// azzero l'indice di ricezione in quanto il pacchetto sfora il buffer di rx
			TimeOutModRxSec.Enable = 0;	// fermo il timer del time out
			TimeOutModRxSec.Value = 0;		// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
			TimeOutModRxSec.TimeOut = 0;	// e resetto il flag di time out
		}
        
        if(Me.ComToUart3==1)                        
        {
            flag=U3STAbits.URXDA;			// se ho un interrupt di ricezione dalla seriale        
            _U3RXIF = 0;
        }
        else
        {
            flag=U2STAbits.URXDA;				// se ho un interrupt di ricezione dalla seriale           
            _U2RXIF = 0;        
        }
	}
}

void Mod_IntTimeOutRxSec(void)
{
	if(TimeOutModRxSec.Enable ==1)				// controllo se il timer è attivo
	{
		TimeOutModRxSec.Value +=1;				// gestisco il conteggio del timer
		if(TimeOutModRxSec.Value >= TimeOutModRxSec.Time)		// gestisco il time out
		{
			TimeOutModRxSec.Enable = 0;		// fermo il timer del time out
			TimeOutModRxSec.Value = 0;			// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto

            if(Me.ComToUart3==1)                        
            {
                while(U3STAbits.URXDA)			// svuoto il buffer di ricezione
                    iPckModRxSec = U3RXREG;
            }
            else
            {
                while(U2STAbits.URXDA)			// svuoto il buffer di ricezione
                    iPckModRxSec = U2RXREG;                
            }
          
			iPckModRxSec = 0;					// azzero l'indice di ricezione in quanto presumo che si sia inetrrotta la comunicazione
#if(k_Abil_Panel_Shared==1)                        
            if(WaitingReplaySec == 1)
#endif                        
            {
                ModTimeOutInterCharComErrSec +=1;      
                Me.ComError.ModTimeOutInterCharComErrSec +=1;      
            }
		}
	}
}

void Mod_Recive_CmdSec(void)       // ModBus Master: Riceve Risposte e deve controllare che Mod_LastRxSec.Buffer[0] == Mod_BufferTxSec.Buffer[0] 
{
	unsigned int Temp = 0;								// temporaneo per il calcolo del crc e l'isolamento dei dati dal buffer
	unsigned int CRC16_calcolato = 0;


	if(Mod_BufferRxSec.Valid_Data == 1)			// controllo se ho finito di ricevere un pacchetto
	{
		Mod_CopyBufferSec(&Mod_LastRxSec, &Mod_BufferRxSec);	// copio quello che ho ricevuto così da liberare il buffer di ricezione
		iPckModRxSec = 0;							// reinizializzo l'indice 
		Mod_BufferRxSec.Valid_Data = 0;			// resetto la validità del buffer di rx in quanto lo già copiato per analizzarlo
		Mod_LastRxSec.Valid_Data = 0;				// resetto la validità del dato, sarà il controllo a giudicarne la validità

		CRC16_calcolato = Mod_Generate_CRCSec(&Mod_LastRxSec);				// mi calcolo il CRC del pacchetto

		Temp = ((Mod_LastRxSec.Buffer[(Mod_LastRxSec.nByte)-1])<<8)&0xFF00; //(*0x100);		// recupero la parte alta del CRC dal bufer dei dati ricevuti  // Modifica 05/01/2015 Aggiunta mascheratura byte shift
		Temp = Temp | ((Mod_LastRxSec.Buffer[(Mod_LastRxSec.nByte)-2])&0x00FF);	// recupero la parte bassa del CRC dal bufer dei dati ricevuti
		
		if(CRC16_calcolato == Temp)									// se è uguale a quello ricevuto il pacchetto è ok
		{
//Argo		if((Mod_LastRxSec.Buffer[0] == Mod_BufferTxSec.Buffer[0]  /*GetMyAddress()*/) || (Mod_LastRxSec.Buffer[0] == 0))		// Modifica 24/11/2015: accetto il pacchetto solo se destinato a me o in broadcast!
//			{					            
            if(Mod_LastRxSec.Buffer[1]==ReadModCoil || Mod_LastRxSec.Buffer[1]==ReadModInputStatus)
                Mod_LastRxSec.Data =(Mod_LastRxSec.Buffer[3]&0x00FF);
            else    
                Mod_LastRxSec.Data = ((Mod_LastRxSec.Buffer[3]<<8)&0xFF00) | (Mod_LastRxSec.Buffer[4]&0x00FF);
			/*
			Data1 = Mod_LastRxSec.Buffer[3] & 0x00FF; 
			Data2 = (char)Mod_LastRxSec.Buffer[4] & 0x00FF;
			Data3 = (Data1<<8) | Data2;
			Mod_LastRxSec.Data = Data3;
			*/
			Mod_LastRxSec.Valid_Data = 1;								// a questo punto devo analizzare il pacchetto
//            }
        }
		else
		{	
//Argo            if(Mod_LastRxSec.Buffer[0] == Mod_BufferTxSec.Buffer[0])
            ModCRC_ComErrSec +=1;
            Me.ComError.ModCRC_ComErrSec +=1;
		}           
	}
}

void Mod_Write_CmdSec(char Address, char Comando, int Registro, int Data)
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
	
	int TxDataCoil=0;
	
	switch(Comando)
	{												// Attenzione non togliere i case per "Read" servono!!! (Sara' da sistemare)
		case ReadModCoil:							// Read Coil register
        case ReadModInputStatus:    
		case ReadModCmd:							// Read Output register
		case ReadModInput:							// Read Input register
			Mod_BufferTxSec.nByte = 8;									// la richiesta è formata da 8 byte
			Mod_BufferTxSec.Buffer[0] = Address;						// il primo è l'indirizzo del destinatario
			Mod_BufferTxSec.Buffer[1] = Comando;						// il comando 
			Mod_BufferTxSec.Buffer[2] = (char)(((Registro & 0xFF00)>>8)&0x00FF);// il registro di partenza (hi byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTxSec.Buffer[3] = (char)(Registro & 0x00FF);		// il registro di partenza (lo byte)
			Mod_BufferTxSec.Buffer[4] = (char)(((Data & 0xFF00)>>8)&0x00FF);	// il numero di registri da leggere (hi byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTxSec.Buffer[5] = (char)(Data & 0x00FF);			// il numero di registri da leggere (lo byte)
	
			CRC = Mod_Generate_CRCSec(&Mod_BufferTxSec);					// calcolo il CRC
			Mod_BufferTxSec.Buffer[6] = (char)(CRC & 0x00FF);			// CRC (Hi Byte)
			Mod_BufferTxSec.Buffer[7] = (char)(((CRC & 0xFF00)>>8)&0x00FF);		// CRC (Lo BHyte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift

			Mod_BufferTxSec.Valid_Data = 1;			// setto il flag per la richiesta della trasmissione
			break;
			
		case WriteModForceSingleCoil:							// Write Single Coil		
			
			if(Data>0) 
				TxDataCoil = 0xFF00;
            else
                TxDataCoil = 0;
			
			Mod_BufferTxSec.nByte = 8;									// la richiesta è formata da 8 byte
			Mod_BufferTxSec.Buffer[0] = Address;						// il primo è l'indirizzo del destinatario
			Mod_BufferTxSec.Buffer[1] = Comando;						// il comando 
			Mod_BufferTxSec.Buffer[2] = (char)(((Registro & 0xFF00)>>8)&0x00FF);// il registro di partenza (hi byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTxSec.Buffer[3] = (char)(Registro & 0x00FF);		// il registro di partenza (lo byte)
			Mod_BufferTxSec.Buffer[4] = (char)(((TxDataCoil & 0xFF00)>>8)&0x00FF);	// il numero di registri da leggere (hi byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTxSec.Buffer[5] = (char)(TxDataCoil & 0x00FF);			// il numero di registri da leggere (lo byte)
	
			CRC = Mod_Generate_CRCSec(&Mod_BufferTxSec);					// calcolo il CRC
			Mod_BufferTxSec.Buffer[6] = (char)(CRC & 0x00FF);			// CRC (Hi Byte)
			Mod_BufferTxSec.Buffer[7] = (char)(((CRC & 0xFF00)>>8)&0x00FF);		// CRC (Lo BHyte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift

			Mod_BufferTxSec.Valid_Data = 1;							// setto il flag per la richiesta della trasmissione
			break;

		case WriteModCmd:											// Prese Single register
			Mod_BufferTxSec.nByte = 8;									// la richiesta è formata da 8 byte
			Mod_BufferTxSec.Buffer[0] = Address;						// il primo è l'indirizzo del destinatario
			Mod_BufferTxSec.Buffer[1] = Comando;						// il comando 
			Mod_BufferTxSec.Buffer[2] = (char)(((Registro & 0xFF00)>>8)&0x00FF);// il registro da settare (Hi Byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTxSec.Buffer[3] = (char)(Registro & 0x00FF);		// il registro da settare (Lo Byte)
			Mod_BufferTxSec.Buffer[4] = (char)(((Data & 0xFF00)>>8)&0x00FF);	// valore da settare (Hi Byte)		// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTxSec.Buffer[5] = (char)(Data & 0x00FF);			// valore da settare (Lo Byte)
	
			CRC = Mod_Generate_CRCSec(&Mod_BufferTxSec);					// calcolo il CRC
			Mod_BufferTxSec.Buffer[6] = (char)(CRC & 0x00FF);			// CRC (Hi Byte)
			Mod_BufferTxSec.Buffer[7] = (char)(((CRC & 0xFF00)>>8)&0x00FF);		// CRC (Lo Byte)					// Modifica 05/01/2015 Aggiunta mascheratura byte shift
	
			Mod_BufferTxSec.Valid_Data = 1;							// setto il flag per la richiesta della trasmissione
			break;

		case WriteModListCmd:										//Preset multiple register
			Mod_BufferTxSec.nByte = 9+(2*Data);						// la richiesta è formata da 8 byte
			Mod_BufferTxSec.Buffer[0] = Address;						// il primo è l'indirizzo del destinatario
			Mod_BufferTxSec.Buffer[1] = Comando;						// il comando 
			Mod_BufferTxSec.Buffer[2] = (char)(((Registro & 0xFF00)>>8)&0x00FF);// il registro da settare (Hi Byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTxSec.Buffer[3] = (char)(Registro & 0x00FF);		// il registro da settare (Lo Byte)
			Mod_BufferTxSec.Buffer[4] = (char)(((Data & 0xFF00)>>8)&0x00FF);	// Numero di registri da settare (Hi Byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
			Mod_BufferTxSec.Buffer[5] = (char)(Data & 0x00FF);			// Numero di registri da settare (Lo Byte)
			Mod_BufferTxSec.Buffer[6] = Data*2;						// Numero di byte dati che seguono
	
			CRC = Mod_Generate_CRCSec(&Mod_BufferTxSec);					// calcolo il CRC
			Mod_BufferTxSec.Buffer[Mod_BufferTxSec.nByte-2] = (char)(CRC & 0x00FF);		// CRC (Hi Byte)
			Mod_BufferTxSec.Buffer[Mod_BufferTxSec.nByte-1] = (char)(((CRC & 0xFF00)>>8)&0x00FF);	// CRC (Lo Byte)	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
	
			Mod_BufferTxSec.Valid_Data = 1;			// setto il flag per la richiesta della trasmissione
			break;
	}
    
    if(Me.ComToUart3==1)
        ReturnValue = Mod_WriteBusU3();					// trasmetto la richiesta    
    else
        ReturnValue = Mod_WriteBusSec();					// trasmetto la richiesta
}
