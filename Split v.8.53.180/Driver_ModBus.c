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
#include "Driver_ModBus.h"
#include "ProtocolloModBus.h"
#include "ProtocolloModBusSec.h"
#include "ProtocolloComunicazione.h"

extern volatile int ModCntComErr;			
extern volatile int ModTimeOutComErr;		
extern volatile int ModTimeOutInterCharComErr;
extern volatile int ModCRC_ComErr;

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
unsigned int Mod_BaseCRC16(unsigned int * Old_CRC, unsigned char NewChr)
{
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
}

unsigned int Mod_Generate_CRC(volatile TypModBuffer * RtxBuff)
{
	unsigned int CRC_16=0xFFFF;
	unsigned int i = 0;
	
	for(i=0; i < (RtxBuff->nByte-2); i++)
	{
		CRC_16 = Mod_BaseCRC16(&CRC_16, (unsigned char)RtxBuff->Buffer[i]);
	}
	return CRC_16;
}

void Mod_CopyBuffer(volatile TypModBuffer * Destinazione, volatile TypModBuffer * Originale)
{
	//char i = 0;
	unsigned int i = 0;
	
	for(i=0; i<=Originale->nByte; i++)
	{
		Destinazione->Buffer[i] = Originale->Buffer[i];	// copio ogni attributo della struttura dalla struttura originale a quella di destinazione
	}
	Destinazione->nByte = Originale->nByte;
	//Destinazione->Valid_Data = Originale->Valid_Data;
}



char Mod_WriteBus(void)
{
	//char i = 0;
	unsigned int i = 0;
	
	// Modifica del 05/01/2014
	Mod_LastRx.Valid_Data = 0; 								// Azzero flag di RX dato valido!

	if (Mod_BufferTx.Valid_Data == 1)						// se il buffer di trasmissione ha un pacchetto validato
	{
		for(i=0; i < Mod_BufferTx.nByte; i++)
		{
			putch1(Mod_BufferTx.Buffer[i]);					// invio sul bus la sequenza di byte
		}
		Mod_BufferTx.Valid_Data = 0;						// invalido il dato in quanto vecchio
		return 0;											// ritorno 0 in quanto l'operazione è andata a boun fine
	}
	return 1;												// ritorno 1 perche il pacchetto dati non è valido
}

void Mod_IntReadBus(int RxData)
{	
//GL 	while(U1STAbits.URXDA)				// se ho un interrupt di ricezione dalla seriale
//GL	{	
		
		TimeOutModRx.Value = 0;                 // resetto il timer comunque perchè ho appena ricevuto un carattere
		TimeOutModRx.Time = K_TimeModInterChar;	// setto il time out intercarattere (dovrebbe essere impostato ad almeno 3.5 volte il tempo di un byte @ BaudRate corrente)
		if(TimeOutModRx.Enable == 0)            // verifico se il timer per il time out è già attivo (rx in corso)
			TimeOutModRx.Enable = 1;            // se non lo attivo

		if(iPckModRx <ByteLenModBuff)
		{
			Mod_BufferRx.Buffer[iPckModRx] = RxData; //GL U1RXREG;	// salvo nel buffer i dati in arrivo					

//			if(1); //SISTEMARE!??! //((iPckModRx == 0) && (Mod_BufferRx.Buffer[iPckModRx] == Mod_BufferTx.Buffer[0])) || (iPckModRx != 0))
//			{
				iPckModRx+=1;							// incremento l'indice
				if(iPckModRx > 3)						// se ho completato la sequenza
				{
/*					if(  ((Mod_BufferRx.Buffer[1] == 1) && (iPckModRx >(4+(Mod_BufferRx.Buffer[2])))) ||	// Modifica del 05/01/2014 introdotto READ COIL register
						 ((Mod_BufferRx.Buffer[1] == 3) && (iPckModRx >(4+(Mod_BufferRx.Buffer[2])))) ||
						 ((Mod_BufferRx.Buffer[1] == 4) && (iPckModRx >(4+(Mod_BufferRx.Buffer[2])))) ||
						 ((Mod_BufferRx.Buffer[1] == 6) && (iPckModRx >7)) ||
						 ((Mod_BufferRx.Buffer[1] == 16) && (iPckModRx >7)))
*/
					if(  //((Mod_BufferRx.Buffer[1] == 3) && (iPckModRx >(4+(Mod_BufferRx.Buffer[2])))) ||
						 ((Mod_BufferRx.Buffer[1] == 3) && (iPckModRx >7)) ||	
						 ((Mod_BufferRx.Buffer[1] == 4) && (iPckModRx >(4+(Mod_BufferRx.Buffer[2])))) ||
						 ((Mod_BufferRx.Buffer[1] == 6) && (iPckModRx >7)) ||
						 ((Mod_BufferRx.Buffer[1] == 16) && (iPckModRx > (8+(Mod_BufferRx.Buffer[6])))) )
 
/*					if(  ((Mod_BufferRx.Buffer[1] == ReadModCoil) && (iPckModRx >(4+(Mod_BufferRx.Buffer[2])))) ||	// Modifica del 05/01/2014 introdotto READ COIL register
						 ((Mod_BufferRx.Buffer[1] == ReadModCmd) && (iPckModRx >(4+(Mod_BufferRx.Buffer[2])))) ||
						 ((Mod_BufferRx.Buffer[1] == ReadModInput) && (iPckModRx >(4+(Mod_BufferRx.Buffer[2])))) ||
						 ((Mod_BufferRx.Buffer[1] == WriteModForceSingleCoil) && (iPckModRx >7)) ||							
						 ((Mod_BufferRx.Buffer[1] == WriteModCmd) && (iPckModRx >7)) ||
						 ((Mod_BufferRx.Buffer[1] == WriteModListCmd) && (iPckModRx >7)))      
 */               
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
						if(	(Mod_BufferRx.Buffer[1] != 1) && (Mod_BufferRx.Buffer[1] != 3) && (Mod_BufferRx.Buffer[1] != 4) && 
							(Mod_BufferRx.Buffer[1] != 6) && (Mod_BufferRx.Buffer[1] != 16))
						{
							iPckModRx = 0;				// azzero l'indice di ricezione in quanto il pacchetto sfora il buffer di rx
							TimeOutModRx.Enable = 0;	// fermo il timer del time out
							TimeOutModRx.Value = 0;		// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
							TimeOutModRx.TimeOut = 0;	// e resetto il flag di time out
						}
					}
				}
//			}
		}
		else
		{
			iPckModRx = 0;				// azzero l'indice di ricezione in quanto il pacchetto sfora il buffer di rx
			TimeOutModRx.Enable = 0;	// fermo il timer del time out
			TimeOutModRx.Value = 0;		// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
			TimeOutModRx.TimeOut = 0;	// e resetto il flag di time out
		}

		//GL IFS0bits.U1RXIF = 0;		
		
        //GL
        //TimeOutModRx.Enable = 0;	// fermo il timer del time out
        //TimeOutModRx.Value = 0;		// e ne azzero il conteggio, cosi da essere pronto alla ricezione di un nuovo pacchetto
        //TimeOutModRx.TimeOut = 0;	// e resetto il flag di time out
        //GL
        

//GL	}
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
			while(U1STAbits.URXDA)			// svuoto il buffer di ricezione
				iPckModRx = U1RXREG;
			iPckModRx = 0;					// azzero l'indice di ricezione in quanto presumo che si sia inetrrotta la comunicazione
            
            if(Mod_BufferRx.Buffer[0] == My_Address)    //GL 05/03/2018 - Aggiunto per evitare conteggi errati della pausa intercarattere 
                ModTimeOutInterCharComErr +=1;          //sulle risposte in lettura (ReadModCmd) delle altre periferiche (Touch e altri subslave)           
		}                                               //perchè di lunghezza diversa rispetto ad una domanda dello Slave (vedi "Mod_IntReadBus" qui sopra....)
	}                                                   //quindi incremento contatore solo se il pacchetto era destinato a me
}




void Mod_Recive_Cmd(void)       // ModBus Slave: Riceve Domande e deve controllare che Mod_LastRx.Buffer[0]  == My_Address
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
			if((Mod_LastRx.Buffer[0] == My_Address /*GetMyAddress()*/) || (Mod_LastRx.Buffer[0] == k_Broadcast_Addr_Sec))		// Modifica 24/11/2015: accetto il pacchetto solo se destinato a me o in broadcast!
			{					
				//???????? Errato!NON 3e4 ma 4e5  Mod_LastRx.Data = ((Mod_LastRx.Buffer[3]<<8)&0xFF00) | (Mod_LastRx.Buffer[4]&0x00FF);
				Mod_LastRx.Data = ((Mod_LastRx.Buffer[4]<<8)&0xFF00) | (Mod_LastRx.Buffer[5]&0x00FF);
				/*
				Data1 = Mod_LastRx.Buffer[3] & 0x00FF; 
				Data2 = (char)Mod_LastRx.Buffer[4] & 0x00FF;
				Data3 = (Data1<<8) | Data2;
				Mod_LastRx.Data = Data3;
				*/
				Mod_LastRx.Valid_Data = 1;								// a questo punto devo analizzare il pacchetto
			}
		}	
		else
		{
            if(Mod_LastRx.Buffer[0] == My_Address)
                ModCRC_ComErr +=1;		
        }		
	}
}


/*	Address => Indirizzo del dispositivo a cui è rivolto il comando 
	Comando => codice del comando ModBus
	Registro => Registro di partenza o unico del comando
	Data =>		Func 3/4-> Numero di registri da leggere
				Func 6	-> Valore del registro da settare
				Func 16	-> Numero di registri da scrivere
*/
/*
void Mod_Write_Cmd(char Address, char Comando, int Registro, int Data)
{
	volatile int CRC = 0;
	volatile char ReturnValue = 0;
	//volatile char Indice = 0;
	switch(Comando)
	{
		case 1:							// Read Coil register
		case 3:							// Read Output register
		case 4:							// Read Input register
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

		case 6:							// Prese Single register
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
	
			Mod_BufferTx.Valid_Data = 1;			// setto il flag per la richiesta della trasmissione
			break;

		case 16:						//Preset multiple register
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
*/


void Mod_Reply_Cmd(char Address, char Comando, int Registro, int Value, int nData)
/*	Address => Indirizzo del dispositivo a cui è rivolta la risposta 
	Comando => codice del comando ModBus
	Registro => indirizzo Registro di partenza o unico del comando
	Value =>	Valore del registro letto o scritto
*/
{
	volatile int CRC = 0;
	volatile char ReturnValue = 0;
	//volatile char Indice = 0;
	unsigned int i;
	unsigned char nByte;
	
	nByte = nData*2;
	
	switch(Comando)
	{
		case ReadModCmd:							//3 Read Output resgister
		case ReadModInput:							//4 Read Input register
			Mod_BufferTx.nByte = 5+nByte; //7;									// la risposta è formata da 7 byte (5+2CRC)
			Mod_BufferTx.Buffer[0] = Address;							// il primo è l'indirizzo del destinatario
			Mod_BufferTx.Buffer[1] = Comando;							// il comando 
			Mod_BufferTx.Buffer[2] = nByte; //2;								// il n. di byte di dato spediti
			for(i=3; i<3+nByte; i+=2)
			{
				Mod_BufferTx.Buffer[i] = (char)((Value & 0xFF00)>>8);	// il valore del dato letto (hi byte) 3
				Mod_BufferTx.Buffer[i+1] = (char)(Value & 0x00FF);		// il valore del dato letto (lo byte) 4
			}
			CRC = Mod_Generate_CRC(&Mod_BufferTx);						// calcolo il CRC
			Mod_BufferTx.Buffer[i] = (char)(CRC & 0x00FF);				// CRC (Hi Byte)	5
			Mod_BufferTx.Buffer[i+1] = (char)((CRC & 0xFF00)>>8);		// CRC (Lo BHyte)	6

			Mod_BufferTx.Valid_Data = 1;			// setto il flag per la richiesta della trasmissione
			break;

		case WriteModCmd:							//6 Preset Single register
		case WriteModListCmd:						//16 Preset multiple register			
			Mod_BufferTx.nByte = 8;									// la risposta è formata da 8 byte
			Mod_BufferTx.Buffer[0] = Address;						// il primo è l'indirizzo del destinatario
			Mod_BufferTx.Buffer[1] = Comando;						// il comando 
			Mod_BufferTx.Buffer[2] = (char)((Registro & 0xFF00)>>8);// il registro settato (Hi Byte)
			Mod_BufferTx.Buffer[3] = (char)(Registro & 0x00FF);		// il registro settato (Lo Byte)
			Mod_BufferTx.Buffer[4] = (char)((Value & 0xFF00)>>8);	// valore settato (Hi Byte)
			Mod_BufferTx.Buffer[5] = (char)(Value & 0x00FF);		// valore settato (Lo Byte)
	
			CRC = Mod_Generate_CRC(&Mod_BufferTx);					// calcolo il CRC
			Mod_BufferTx.Buffer[6] = (char)(CRC & 0x00FF);			// CRC (Hi Byte)
			Mod_BufferTx.Buffer[7] = (char)((CRC & 0xFF00)>>8);		// CRC (Lo Byte)
	
			Mod_BufferTx.Valid_Data = 1;			// setto il flag per la richiesta della trasmissione
			break;
		/*
		case 16:						//Preset multiple register
		
			Mod_BufferTx.nByte = 9+(2*Data);						// la richiesta è formata da 8 byte
			Mod_BufferTx.Buffer[0] = Address;						// il primo è l'indirizzo del destinatario
			Mod_BufferTx.Buffer[1] = Comando;						// il comando 
			Mod_BufferTx.Buffer[2] = (char)((Registro & 0xFF00)>>8);// il registro da settare (Hi Byte)
			Mod_BufferTx.Buffer[3] = (char)(Registro & 0x00FF);		// il registro da settare (Lo Byte)
			Mod_BufferTx.Buffer[4] = (char)((Data & 0xFF00)>>8);	// Numero di registri da settare (Hi Byte)
			Mod_BufferTx.Buffer[5] = (char)(Data & 0x00FF);			// Numero di registri da settare (Lo Byte)
			Mod_BufferTx.Buffer[6] = Data*2;						// Numero di byte dati che seguono
	
			CRC = Mod_Generate_CRC(&Mod_BufferTx);					// calcolo il CRC
			Mod_BufferTx.Buffer[Mod_BufferTx.nByte-2] = (char)(CRC & 0x00FF);		// CRC (Hi Byte)
			Mod_BufferTx.Buffer[Mod_BufferTx.nByte-1] = (char)((CRC & 0xFF00)>>8);	// CRC (Lo Byte)
	
			Mod_BufferTx.Valid_Data = 1;			// setto il flag per la richiesta della trasmissione
			break;
		*/
	}

		ReturnValue = Mod_WriteBus();					// trasmetto la richiesta
}