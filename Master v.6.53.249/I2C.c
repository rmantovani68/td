/*
-----------------------------------------------------------------------------
	Nome: New_I2C.c
	Autore: TDM Team
	Revisione\Data:	Rev.2 - 09/05/2010
	Descrizione: Corpo delle funzioni per la comunicazione su bus I2C
	Dipendenze: I2C.h - Header di questo file	
-----------------------------------------------------------------------------
*/
#include "HWSelection.h"
#if (DA210==1)
	#include <p24FJ256DA210.h>
#else
	#include <p24FJ256GB210.h>
#endif

#include <stdio.h>		
#include <libpic30.h>
#include "delay.h"
#include "I2C.h"			// Header del file .c

/*
-----------------------------------------------------------------------------
	Tipo: procedura	
	Nome: I2C_Init()
	Descrizione: Inizializzazione del modulo SSP per il Bus I2C sui pin SDA1-RB9 e SCL1-RB8
-----------------------------------------------------------------------------
*/
void I2C_Init(void)
{
	int temp;
	// I2CBRG = 194 for 10Mhz OSCI with PPL with 100kHz I2C clock
	I2C3BRG = DIVIDER_I2C;	// Imposto il divider per generare il clock
	I2C3CON = 0;
	I2C3CONbits.I2CEN = 0;	// Disabilito il modulo I2C
	I2C3CONbits.DISSLW = 1;	// Disabilito lo slew rate control
	IFS5bits.MI2C3IF = 0;	// Resetto Interrupt
	I2C3CONbits.I2CEN = 1;	// Abilito il modulo I2C
	temp = I2C3RCV;			// Leggo il buffer per cancellarlo
	Reset_I2C_bus();		// Setto il bus in idle
}

/*
-----------------------------------------------------------------------------
	Tipo: funzione	
	Nome: I2C_Is_Idle()
	Descrizione: Testa se il Bus è libero
	Exit Value:
				  0 -> Il Bus risulta occupato
				255 -> Il bus risulta libero
-----------------------------------------------------------------------------
*/
unsigned char I2C_Is_Idle(void)					// restituisce 1 se il bus I2C è libero
{
	if(I2C3STATbits.R_W || 
		I2C3CONbits.ACKEN || 
		I2C3CONbits.RCEN || 
		I2C3CONbits.PEN || 
		I2C3CONbits.RSEN || 
		I2C3CONbits.SEN)		// testo se uno dei flag di attivata del bus I2C è settato
		return 0;				// nel caso segnalo che il bus è in uso
	else
		return 255;				// o segnalo che è libero
}

/*
-----------------------------------------------------------------------------
	Tipo: funzione	
	Nome: I2C_Start()
	Descrizione: Genera lo start e attende che venga geerato correttamente.
	Exit Value:
				0 -> Eseguita con successo
				1 -> Time Out durante la generazione dello start
-----------------------------------------------------------------------------
*/
//function iniates a start condition on bus
unsigned char I2C_Start(void)
{
	int x = 0;

	I2C3CONbits.ACKDT = 0;	// Resetto ogni ACK precedente
	DelayuSec(20);
	I2C3CONbits.SEN = 1;	// Inizializzo la condizione di start
	__builtin_nop();
	while (I2C3CONbits.SEN)	// Attendo la fine dello start
	{	DelayuSec(2);
		x++;
		if (x > 20)
			return 1;		// Se lo start dura più di 20 uSec
	}
	DelayuSec(4);
	return 0;
}

/*
-----------------------------------------------------------------------------
	Tipo: funzione
	Nome: I2C_Repeated_Start()
	Descrizione: Genera il segnale di Repeted Start sul bus I2C e attende 
				 che venga generato correttamente.
	Exit value:
				0 -> Eseguita con successo
				1 -> Time Out durante la generazione del repeted start
-----------------------------------------------------------------------------
*/
unsigned char I2C_Repeated_Start(void)
{
	int x = 0;

	I2C3CONbits.RSEN = 1;		// Inizializzo la condizione di repeted start
	__builtin_nop();
	while (I2C3CONbits.RSEN)	// Attendo la fine del repeted start
	{	DelayuSec(2);
		x++;
		if (x > 20) 
			return 1;
	}
	DelayuSec(4);
	return 0;
}

/*
-----------------------------------------------------------------------------
	Tipo: funzione	
	Nome: I2C_Stop()
	Descrizione: Genera il segnale di Stop sul bus I2C e attende che venga 
				 generato correttamente
	Exit value:
				0 -> Eseguita con successo
				1 -> Time Out durante l'operazione
-----------------------------------------------------------------------------
*/
void I2C_Stop(void)
{
	int x = 0;
	
	I2C3CONbits.PEN = 1;	// Mando un bit di stop
	__builtin_nop();
	while (I2C3CONbits.PEN)	// Attendo la conclusione
	{	DelayuSec(2);
		x ++;
		if (x > 20) 
			break;
	}
	DelayuSec(20);
}

/*
-----------------------------------------------------------------------------
	Tipo: procedura
	Nome: Reset_I2C_bus()
	Descrizione: Resetta il bus
-----------------------------------------------------------------------------
*/
void Reset_I2C_bus(void)
{
	int x = 0;
	
	I2C3CONbits.PEN = 1;	// Mando un bit di stop
	__builtin_nop();
	while (I2C3CONbits.PEN)	// Attendo la conclusione
	{	DelayuSec(2);
		x ++;
		if (x > 20) break;
	}
	I2C3CONbits.RCEN = 0;
	IFS5bits.MI2C3IF = 0;	// Reseto l'interrupt
	I2C3STATbits.IWCOL = 0;
	I2C3STATbits.BCL = 0;
	DelayuSec(20);
}

/*
-----------------------------------------------------------------------------
	Tipo: funzione	
	Nome: I2C_Write(Data)
	Descrizione: Scrive il Byte Data sul Bus I2C
	Exit Value:
				0 -> Eseguita con successo
				1 -> Time Out durante la trasmissione
				2 -> Non ho ricevuto l'ACK dallo Slave
-----------------------------------------------------------------------------
*/
unsigned char I2C_Write(unsigned char Data)	// Scrivo un byte
{
	int i;

	DelayuSec(20);
	while (I2C3STATbits.TBF)
		continue;
	IFS5bits.MI2C3IF = 0;		// Resetto l'interrupt
	I2C3TRN = Data;				// Carico i dati da trasmettere
	__builtin_nop();
	for (i=0; i<500; i++)		// Attendo la fine della trasmissione del byte
	{	if (!I2C3STATbits.TRSTAT) 
			break;
		DelayuSec(2);
	}
	if(i>=499)
		return 1;				// Se non ho completato il send
	
	if(I2C3STATbits.ACKSTAT)	// Controllo se ho ricevuto Nack
		return 2;				// Se non ho ricevuto l'ACK
	DelayuSec(4);    
	return 0;					// Esco segnalando la corretta conclusione della funzione
}

/*
-----------------------------------------------------------------------------
	Tipo: funzione	
	Nome: I2C_Read(Ack)
	Descrizione: Riceve un Byte dal Bus I2C
	Exit Value: Data Read
	Ack ->	0 ==> MACK	- flag inviato per proseguire la lettura
			1 ==> NMACK	- flag inviato per terminare la lettura
-----------------------------------------------------------------------------
*/

unsigned char I2C_Read(unsigned char vAck)	// leggo un byte e rispondo con MACK o NMACK
{
	int LoopCnt = 0;
	unsigned char ReciveDt;

	I2C3CONbits.RCEN = 1;		// abilita la ricezione dati
	__builtin_nop();
	__builtin_nop();
	while(!I2C3STATbits.RBF)	// attendi l'evento ricezione dati
	{	LoopCnt ++;
		if(LoopCnt > 2000)
			break;
	}

	ReciveDt = I2C3RCV;			// copio i dati ricevuti

	if(!vAck)
	{	I2C3CONbits.ACKEN = 1;	// avvio la trasmissione del segnale di Ack
		DelayuSec(20);
	}
	return ReciveDt;			// restituisco il byte letto
}

/*
-----------------------------------------------------------------------------
	Tipo: funzione
	Nome: SetRegI2C(Address, Reg, Data)
	Descrizione: Scrive il byte Data nel registro Reg del dispositivo con indirizzo Addres
	Exit Value: 
				0 -> Eseguita con successo
				1 -> Bus is Busy
				2 -> Errore durante la scrittura dell'indirizzo
				3 -> Errore durante la scrittura del registro da impostare
				4 -> Errore durante la scrittura dei dati
	Parametri:
				Address	->	Indirizzo del dispositivo
				Reg		->	Registro da scrivere
				Data	->	Valore da scrivere nel registro
-----------------------------------------------------------------------------
*/
unsigned char SetRegI2C(unsigned char Address, unsigned int Reg, unsigned char Data)
{
	unsigned char TmpSt = 0;
	int i = 0;
    
    unsigned char RealReg;
    unsigned char RealAddr;
    
    RealReg = (unsigned char)(Reg & 0x00FF);
    RealAddr = Address | (unsigned char)(Reg>>7 & 0x00FE);
    
	TmpSt = I2C_Start();			// Start
	if(TmpSt != 0)
		return 1;
	TmpSt = I2C_Write(RealAddr);    // Invio Indirizzo + W
	if(TmpSt != 0)
		return 2;
	TmpSt = I2C_Write(RealReg);		// Invio registro
	if(TmpSt != 0)
		return 3;
	TmpSt = I2C_Write(Data);		// Invio Dati
	if(TmpSt != 0)
		return 4;
	I2C_Stop();						// Invio Stop
	for(i=0; i<100; i++)			// attendo max 10 mSec
	{
		TmpSt = I2C_Start();			// Start
		TmpSt = I2C_Write(RealAddr);		// Invio Indirizzo + W
		if(TmpSt == 0)
			return 0;	
	}
	return 5;
    
/*    
	unsigned char TmpSt = 0;
	int i = 0;
	TmpSt = I2C_Start();			// Start
	if(TmpSt != 0)
		return 1;
	TmpSt = I2C_Write(Address);		// Invio Indirizzo + W
	if(TmpSt != 0)
		return 2;
	TmpSt = I2C_Write(Reg);			// Invio registro
	if(TmpSt != 0)
		return 3;
	TmpSt = I2C_Write(Data);		// Invio Dati
	if(TmpSt != 0)
		return 4;
	I2C_Stop();						// Invio Stop
	for(i=0; i<100; i++)			// attendo max 10 mSec
	{
		TmpSt = I2C_Start();			// Start
		TmpSt = I2C_Write(Address);		// Invio Indirizzo + W
		if(TmpSt == 0)
			return 0;	
	}
	return 5;
 */
}

/*
-----------------------------------------------------------------------------
	Tipo: funzione	
	Nome: ReadRegI2C(Address, Re)
	Descrizione: Riceve un Byte dal Bus I2C
	Exit Value: 
				Data	->	Valore letto dal bus I2C
	Parametri:
				Addres	->	Indirizzo del dispositivo
				Reg		->	Registro da leggere
-----------------------------------------------------------------------------
*/
unsigned char ReadRegI2C(unsigned char Address, unsigned int Reg)
{
	unsigned char RxBuff = 0;
    
    unsigned char RealReg;
    unsigned char RealAddr;
    
    RealReg = (unsigned char)(Reg & 0x00FF);
    RealAddr = Address | (unsigned char)(Reg>>7 & 0x00FE);
    
	I2C_Start();				// invio Start
	I2C_Write(RealAddr);		// invio indirizzo + W
	I2C_Write(RealReg);			// invio Registro da leggere
	DelayuSec(20);
	I2C_Repeated_Start();		// invio RepeatedStart
	I2C_Write(RealAddr+1);		// invio Indirizzo + R
	RxBuff = I2C_Read(NMACK);	// Leggo i Dati
	I2C_Stop();

	return RxBuff;
    
/*    
	unsigned char RxBuff = 0;

	I2C_Start();				// invio Start
	I2C_Write(Address);			// invio indirizzo + W
	I2C_Write(Reg);				// invio Registro da leggere
	DelayuSec(20);
	I2C_Repeated_Start();		// invio RepeatedStart
	I2C_Write(Address+1);		// invio Indirizzo + R
	RxBuff = I2C_Read(NMACK);	// Leggo i Dati
	I2C_Stop();

	return RxBuff;
 */
}
