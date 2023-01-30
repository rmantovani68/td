//----------------------------------------------------------------------------------
//	Progect name:	ProtocolloModBus.c
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			18/06/2011
//	Description:	Corpo delle funzioni che gestiscono la comunicazione ModBus con il sistema
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
//% #include "ADC.h"
//% #include "PWM.h"
#include "EEPROM.h"
//% #include "Valvola_PassoPasso.h"
#include "Driver_Comunicazione.h"
#include "Driver_ComunicazioneSec.h"
#include "Driver_ModBus.h"
#include "ProtocolloComunicazione.h"
#include "ProtocolloComunicazioneSec.h"
#include "ProtocolloModBus.h"
#include "ProtocolloModBusSec.h"
//% #include "Core.h"
//% #include "PID.h"
#include "FWSelection.h"
#include "delay.h"


//----------------------------------------------------------------------------------
//	Dichiarazioni di External
//----------------------------------------------------------------------------------
//GL extern volatile TypTimer TimerCommTimeout_SideA;	// Timer per la gestione del Timeout di comunicazione Side A
//GL extern volatile TypTimer TimerCommTimeout_SideB;	// Timer per la gestione del Timeout di comunicazione Side B

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------

unsigned char WaitModRx(int TimeOut_ms)
{
	TimeOutModPktRx.Value = 0;				// resetto il timer per il time out della risposta
	TimeOutModPktRx.TimeOut = 0;			//
	TimeOutModPktRx.Time = TimeOut_ms;		// lo configuro come richiesto
	TimeOutModPktRx.Enable = 1;				// lo faccio partire

	while((TimeOutModPktRx.TimeOut == 0) & (Mod_LastRx.Valid_Data == 0))	// attendo un evento
		continue;

	if(Mod_LastRx.Valid_Data != 0)		// se ho ricevuto un dato valido 
	{
		TimeOutModPktRx.Enable = 0;			// fermo il timer
		TimeOutModPktRx.TimeOut = 0;		// resetto il time out
		if( (Mod_LastRx.Buffer[0] == Mod_BufferTx.Buffer[0]) &	// verifico il pacchetto
			(Mod_LastRx.Buffer[1] == Mod_BufferTx.Buffer[1]))
			return 0;											// ritorno 0 se è ok
	}									// in caso contrario
	TimeOutModPktRx.Enable = 0;			// fermo il timer
	TimeOutModPktRx.TimeOut = 0;		// resetto il time out
	return 1;							// ritorno 1
}

/*
unsigned char Send_WaitModRx(char Address, char Comando, int Registro, int Data, int TimeOut_ms, char Retry)
{
	Result = 0;
	RetryCnt = 0;
	while (RetryCnt < Retry)
	{	
		RetryCnt +=1;
		Mod_Write_Cmd(Address, Comando, Registro -1, Data);
		Result = WaitModRx(TimeOut_ms);
		if (Result == 0)
			return 0;		// se ricevo un pacchetto valido 
	}
	return 1;				// se non ho nessuna risposta valida per dopo il numero di tentativi richiesti
}

unsigned char Send_WriteModReg(char Address, int Registro, int Data, int TimeOut_ms, char Retry, char Cmd)
{
	Result = 1;
	RetryCnt = 0;
	while (RetryCnt < Retry)
	{
		RetryCnt +=1;
		Mod_Write_Cmd(Address, Cmd, Registro -1, Data);
		Result = WaitModRx(TimeOut_ms);
		if ((Result == 0) &
			(Mod_LastRx.Buffer[2] == Mod_BufferTx.Buffer[2]) &
			(Mod_LastRx.Buffer[3] == Mod_BufferTx.Buffer[3]) &
			(Mod_LastRx.Buffer[4] == Mod_BufferTx.Buffer[4]) &
			(Mod_LastRx.Buffer[5] == Mod_BufferTx.Buffer[5])  )
		{
			Mod_LastRx.Valid_Data = 0;
			return 0;
		}
	}
	Mod_LastRx.Valid_Data = 0;
	return 1;
}
*/




int ExtracReg(char NumReg)
{
	// recupero un valore int dalla coda del buffer l'indice dei registri va da 1 al limite della richiesta
	//Indice = 0;
	//Data = 0;
	unsigned char Indice;
	int Data;    

	Indice = 1+(NumReg*2);
	/////Data = Mod_LastRx.Buffer[Indice]<<8;
	Data = ((Mod_LastRx.Buffer[Indice]<<8)&0xFF00);	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
	Data |= (Mod_LastRx.Buffer[Indice+1]&0x00FF);
	return Data;
}

void InsertReg(char NumReg, int Data)
{
	//Indice = 0;
    unsigned char Indice;    
	
	Indice = 5+(NumReg*2);
	/////Mod_BufferTx.Buffer[Indice] = (char)((Data&0xFF00)>>8);
	Mod_BufferTx.Buffer[Indice] = (char)(((Data&0xFF00)>>8)&0x00FF);	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
	Mod_BufferTx.Buffer[Indice+1] = (char)((Data&0x00FF));
}




void Mod_Process_Cmd(void)			// Routine aggiunta il 24/11/2015 per gestione "Bridge"
{
	int Register;
	int Data;
	char Command;
	unsigned char Mitt_Add;
	unsigned char nByte;
	
#define REG_WRITE_STATUS 1 // Temporaneo solo per non generare errori
	
	
	if(Mod_LastRx.Valid_Data == 1)
	{
		Mitt_Add = Mod_LastRx.Buffer[0];		// Address mittente	
		Command = Mod_LastRx.Buffer[1];		// Tipo di Comando
		//Register = ExtracReg(1); //((Mod_LastRx.Buffer[3]<<8)&0xFF00) | (Mod_LastRx.Buffer[4]&0x00FF); //(Mod_LastRx.Buffer[3] & 0x00FF) | ((Mod_LastRx.Buffer[2] & 0xFF00)>>8);// il registro di partenza (hi byte)
		//Data = ExtracReg(2);				// Dato ricevuto		
		Register = ((Mod_LastRx.Buffer[2]<<8)&0xFF00);	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
		Register |= (Mod_LastRx.Buffer[3]&0x00FF);		
		Data = ((Mod_LastRx.Buffer[4]<<8)&0xFF00);	// Modifica 05/01/2015 Aggiunta mascheratura byte shift
		Data |= (Mod_LastRx.Buffer[5]&0x00FF);
		nByte = (Mod_LastRx.Buffer[6]&0x00FF);
		
		char Esito = 0;
		switch (Command)	// interpreto il comando
		{


			case ReadModCmd:
				while(Mod_BufferTx.Valid_Data == 1)					// attendo 
					continue;
                //if(Mitt_Add!=k_Broadcast_Addr_Sec)
                //{
                    Mod_Reply_Cmd(Mitt_Add, ReadModCmd, Register, GetData(Register), 1);
				//GL Mod_GetDataMulti(Register, Data);
				//GL Mod_Reply_Cmd(Mitt_Add, ReadModCmd, Register, Register, Data);
                //}
				break;

			case WriteModCmd:
				while(Mod_BufferTx.Valid_Data == 1)					// attendo 
					continue;

                Esito = PutData(Register, Data);                    // Tento la scrittura del registro		
                if(Mitt_Add!=k_Broadcast_Addr_Sec)
                {
                    if(Esito == 0)			// in base all'esito rispondo
                        //Mod_Reply_Cmd(Mitt_Add, WriteModCmd, Register, GetData(Register), 1); N.B. !!!! <- Questo tipo di risposta (che rilegge da GetData) genera errore sulla risposta se il comando non è implementato anche come lettura oltre che scrittura
                        Mod_Reply_Cmd(Mitt_Add, WriteModCmd, Register, Data, 1);    // Quindi rispondo con il dato ricevuto perchè... (leggi soppra)
					else
                        Mod_Reply_Cmd(Mitt_Add, WriteModCmd, Register, Data+1, 1);  // N.B. in caso di errore di scrittura (Esito==1) rispondo con dato+1 per generare errore sulla risposta ed "avvisare" in questo modo il ricevente che c'e' stato un errore di scrittura del dato             
                }
                
				break;           
				
			case WriteModListCmd:				
				while(Mod_BufferTx.Valid_Data == 1)					// attendo 
					continue;
                
                // Esito = PutDataMulti(....., ....., ....); <- Implementare WriteModList...... !?
                if(Mitt_Add!=k_Broadcast_Addr_Sec)
                {                
                    Mod_Reply_Cmd(Mitt_Add, WriteModListCmd, Register, Data, 1);	// Rispondo con Addr e DataLen
                    // NOTA: Aggiungere risposta NAK in caso di scrittura fuori range (Esito == 1)
                }
				break;
		}
		
		Mod_LastRx.Valid_Data = 0;
		
		//GL TimerCommTimeout_SideA.TimeOut = 0;				// resetto il flag di timeout comm
		//GL TimerCommTimeout_SideA.Value = 0;				// e relativo timer
		
	}
}


int Mod_GetDataMulti(int Reg, int DataLen)
{
	int i, nByte;	
	
	if(Reg >= K_MaxReg) return 1;							// Se supero dimensione Buffer dati ritorno con dato ZERO

	nByte = DataLen * 2;
	

	for(i=0; i<DataLen; i++)
	{
		if(BridgeBufferB.cFlag[Reg+i]==1)
		{
			Mod_PreBufferTx[i*2] = (BridgeBufferB.Data[Reg+i]>>8) & 0x00FF;		// Read Side B in PreTxBuff (Hi)
			Mod_PreBufferTx[i*2+1] = BridgeBufferB.Data[Reg+i] & 0x00FF;		// Read Side B in PreTxBuff (Lo)
			BridgeBufferB.cFlag[Reg+i]=0;		
			
			if(BridgeBufferA.Data[Reg+i] != BridgeBufferB.Data[Reg+i])
			{
				BridgeBufferA.Data[Reg+i] = BridgeBufferB.Data[Reg+i];
				BridgeBufferA.OldData[Reg+i] = BridgeBufferA.Data[Reg+i]; //BridgeBufferA.Data[buffpt];						
				//BridgeBufferA.cFlag[Reg+i]=1;
			}
		}
		else
		{
			Mod_PreBufferTx[i*2] = (BridgeBufferA.Data[Reg+i]>>8) & 0x00FF;		// Read Side A in PreTxBuff (Hi)
			Mod_PreBufferTx[i*2+1] = BridgeBufferA.Data[Reg+i] & 0x00FF;		// Read Side A in PreTxBuff (Lo)		
		}
	}	
	
	
	
	/*
	for(i=0; i<DataLen; i++)
	{
		if(BridgeBufferB.cFlag[Reg+i]==0)
		{
			Mod_PreBufferTx[i*2] = (BridgeBufferA.Data[Reg+i]>>8) & 0x00FF;		// Read Side A in PreTxBuff (Hi)
			Mod_PreBufferTx[i*2+1] = BridgeBufferA.Data[Reg+i] & 0x00FF;		// Read Side A in PreTxBuff (Lo)
		}
		else if(BridgeBufferB.cFlag[Reg+i]==1 && BridgeBufferA.cFlag[Reg+i]==0)
		{
			//BridgeBufferA.Data[Reg+i] = BridgeBufferB.Data[Reg+i];				// Update Side A
			BridgeBufferB.cFlag[Reg+i]=0;				
			Mod_PreBufferTx[i*2] = (BridgeBufferB.Data[Reg+i]>>8) & 0x00FF;		// Read Side A in PreTxBuff (Hi)
			Mod_PreBufferTx[i*2+1] = BridgeBufferB.Data[Reg+i] & 0x00FF;		// Read Side A in PreTxBuff (Lo)		

		}
		else if(BridgeBufferB.cFlag[Reg+i]==1 && BridgeBufferA.cFlag[Reg+i]==1)
		{
			BridgeBufferB.Data[Reg+i] = BridgeBufferA.Data[Reg+i];				// Update Side B
			BridgeBufferB.cFlag[Reg+i]=0;	
			Mod_PreBufferTx[i*2] = (BridgeBufferA.Data[Reg+i]>>8) & 0x00FF;		// Read Side A in PreTxBuff (Hi)
			Mod_PreBufferTx[i*2+1] = BridgeBufferA.Data[Reg+i] & 0x00FF;		// Read Side A in PreTxBuff (Lo)				

		}
	}
	*/
	return 0;
	

}

//GL
/*
char Mod_PutDataMulti(int Reg, int StartBuff, unsigned char ByteLen)
{
	int i, buffpt, new;
	
	if(Reg >= K_MaxReg) return 1;							// Se supero dimensione Buffer dati ritorno con errore (1)
		
	buffpt = Reg;


	for(i=StartBuff; i<StartBuff+ByteLen; i+=2)
	{
		new = ((Mod_LastRx.Buffer[i]<<8)&0xFF00);
		new |=  (Mod_LastRx.Buffer[i+1]&0x00FF);		
		
		//PROVA in v8.13.07 per cercare di eliminare saltuario incastramento di aggiornamento buffer
		//if(BridgeBufferA.OldData[buffpt] != new)
		{
			BridgeBufferA.Data[buffpt] = new;	// Aggiorno il Buffer in posizione "buffpt" con il dato arrivato
			//BridgeBufferA.cFlag[buffpt] = 1;										// Segnalo dato cambiato (pronto per la lettura dall'altro lato)
			BridgeBufferA.OldData[buffpt] = new; //BridgeBufferA.Data[buffpt];			
			BridgeBufferA.LastWrite[buffpt]=1;			// Segnalo ultima scrittura registro Side A
			BridgeBufferB.LastWrite[buffpt]=0;			// Resetto scrittura registro Side B
		}
		BridgeBufferA.cFlag[buffpt] = 1;										// Segnalo dato cambiato (pronto per la lettura dall'altro lato)
		buffpt++;
	}
	return 0;												// Ritorno con valore NO errore (0)		
	

}
*/

// Verifica tutte le posizioni del buffer alla ricerca di eventuali dati non sincronizzati
// FlagA==0 && FlagB==0 && DataA!=DataB
// if(LastWriteA)
//	DataB = DataA;
// if(LastWriteB)
//	DataA = DataB;
//GL
/*
void CheckBufferSync(void)		
{
	int x;
	
	for(x=0; x<K_MaxReg; x++)
	{
		if(BridgeBufferA.cFlag[x]==0 && BridgeBufferB.cFlag[x]==0 && BridgeBufferA.Data[x]!=BridgeBufferB.Data[x])
		{
			if(BridgeBufferA.LastWrite[x]==1)
			{
				BridgeBufferB.Data[x] = BridgeBufferA.Data[x];
			}
			if(BridgeBufferB.LastWrite[x]==1)
			{
				BridgeBufferA.Data[x] = BridgeBufferB.Data[x];
			}			
		}
	}
	
}
*/

//GL
/*
int SendInitBufferToSideB(void)
{
	// Invio con check dei dati del buffer verso il SideB
	static unsigned int datapointer=1;
	
	oLedEEV = 0;		// Accendo LED Giallo
	//BridgeBufferB.Data[datapointer] = BridgeBufferA.Data[datapointer];
	if(BridgeBufferA.Data[datapointer] != BridgeBufferB.Data[datapointer])
	{
		BridgeBufferB.Data[datapointer] = BridgeBufferA.Data[datapointer];		
		BridgeBufferA.cFlag[datapointer] = 1;
		BridgeBufferB.cFlag[datapointer] = 0;
	}
	//DelaymSec(5);
	
	if((BridgeBufferA.cFlag[datapointer] == 0) && (BridgeBufferA.Data[datapointer] == BridgeBufferB.Data[datapointer])) 
	{
		oErrLed = 0;		// Led acceso	// DEBUG
		datapointer++;
		DelaymSec(2);						// DEBUG
		oErrLed = 1;		// Led spento	// DEBUG
	}
	if (BridgeBufferA.Data[datapointer] == BridgeBufferB.Data[datapointer])
	{
		datapointer++;
	}
	if(datapointer>=K_MaxReg)
	{
		BridgeBufferA.Data[0] &= 0xFFFC;	// Azzero i bit0 e bit1
		datapointer=1;
		oLedEEV = 1;		// Spengo LED Giallo
	}
	
}

int SendInitBufferToSideBdue(void)
{
	// Invio con check dei dati del buffer verso il SideB
	unsigned int x;
	
	for(x=0; x<K_MaxReg; x++)
	{
		//BridgeBufferB.Data[datapointer] = BridgeBufferA.Data[datapointer];
		if(BridgeBufferA.Data[x] != BridgeBufferB.Data[x])
			BridgeBufferA.cFlag[x] = 1;

		//DelaymSec(5);

	}
	BridgeBufferB.Data[0] &= 0xFFFC;	// Azzero i bit0 e bit1


}


// Bridge Status Flags
// bit 0:  Master Data Init Start (R/W): Viene settato a 1 dal Master quando inizia 
//		   la fase di scrittura dei dati di inizializzazione da inviare al SideB. 
//		   Durante questa fase le eventuali scritture dal SideB sono bloccate dal Bridge.
// bit 1:  Bridge SideB Data Init Start (R/W): Viene settato a 1 dal Master quando ha finito
//		   la fase 1 (scrittura dati di init bit0==1) avvisando il Bridge di iniziare l'invio con check
//		   di avvenuto recepimento del dato verso la logica SideB (DL).
//		   Verrà resettato insieme al bit0 una volta terminato l'invio con verifica di tutti i dati del 
//		   Buffer Bridge verso la logica SideB.

// Check bit0 Word0 del buffer -> Master Data Init Start
int CheckMasterInitFLag(void)
{
	if((BridgeBufferA.Data[0] & 0x0001)) 
		return 1;
	else 
		return 0;
	
}

// Check bit1 Word0 del buffer -> Bridge SideB Data Init Start
int CheckMasterSyncFLag(void)
{
	if((BridgeBufferA.Data[0] & 0x0002)) 
		return 1;
	else 
		return 0;
	
}
*/

//GL 
/*
void CheckCommTimeoutFlag_SideA(void)
{
    
	if(TimerCommTimeout_SideA.TimeOut)
		BridgeBufferB.Data[0] |= 0x4000;
	else
		BridgeBufferB.Data[0] &= 0xBFFF;
     
}

void CheckCommTimeoutFlag_SideB(void)
{
	if(TimerCommTimeout_SideB.TimeOut)
		BridgeBufferA.Data[0] |= 0x8000;
	else
		BridgeBufferA.Data[0] &= 0x7FFF;
}
*/

