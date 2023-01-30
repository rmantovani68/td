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
#include "usart.h"

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------

//----------------------------------------
// USART 1 - RS485 Master
//----------------------------------------
void putch1 (char byte)
{
	while(U1STAbits.UTXBF)		// attendo che il buffer di scrittura sia libero
		continue;
	oRTS_Master = 1;			// alzo il pin dell'rts x la 485
	U1TXREG = byte;				// pongo il carattere nel buffer di trasmissione
	__builtin_nop();
	while(!U1STAbits.TRMT)		// attendo la fine della trasmissione
		continue;
	oRTS_Master = 0;			// abbasso il pin dell'rts x la 485
}

char getch1(void)
{
	while(!U1STAbits.URXDA)		// attendo che ci sia un dato disponibile
		continue;	
	return U1RXREG;				// lo leggo, riportandolo come output della funzione
}

//----------------------------------------
// USART 2 - RS485 Slave
//----------------------------------------
void putch2 (char byte)
{
	while(U2STAbits.UTXBF)		// attendo che il buffer di scrittura sia libero
		continue;
	oRTS_Slave = 1;				// alzo il pin dell'rts x la 485
	U2TXREG = byte;				// pongo il carattere nel buffer di trasmissione
	__builtin_nop();
	while(!U2STAbits.TRMT)		// attendo la fine della trasmissione
		continue;
	oRTS_Slave = 0;				// abbasso il pin dell'rts x la 485
}

char getch2(void)
{
	while(!U2STAbits.URXDA)		// attendo che ci sia un dato disponibile
		continue;	
	return U2RXREG;				// lo leggo, riportandolo come output della funzione
}

#ifdef UART3_ON 
//----------------------------------------
// USART 3 - RS485 Service Com Protocol
//----------------------------------------
void putch3 (char byte)
{
	while(U3STAbits.UTXBF)		// attendo che il buffer di scrittura sia libero
		continue;
	oRTS_UART3 = 1;				// alzo il pin dell'rts x la 485
	U3TXREG = byte;				// pongo il carattere nel buffer di trasmissione
	__builtin_nop();
	while(!U3STAbits.TRMT)		// attendo la fine della trasmissione
		continue;
	oRTS_UART3 = 0;				// abbasso il pin dell'rts x la 485
}

char getch3(void)
{
	while(!U3STAbits.URXDA)		// attendo che ci sia un dato disponibile
		continue;	
	return U3RXREG;				// lo leggo, riportandolo come output della funzione
}
#endif


//----------------------------------------
// USART 4 x debug rtx RS232TTL
//----------------------------------------
void putch (char byte)
{
	while(U4STAbits.UTXBF)		// attendo che il buffer di scrittura sia libero
		continue;
	U4RXREG = byte;				// pongo il carattere nel buffer di trasmissione
	while(!U4STAbits.TRMT)		// attendo la fine della trasmissione
		continue;
}

char getch(void)
{
	while(!U4STAbits.URXDA)		// attendo che ci sia un dato disponibile
		continue;	
	return U4RXREG;				// lo leggo, riportandolo come output della funzione
}

char getche(void)
{
	while(!U4STAbits.URXDA)		// attendo che ci sia un dato disponibile
		continue;	
	return U4RXREG;				// lo leggo, riportandolo come output della funzione
}

void ClearUart1Err()
{
	if(U1STAbits.OERR)
	{
		U1STAbits.OERR = 0;
		IFS0bits.U1RXIF = 0;
	}
}

void ClearUart2Err()
{
	if(U2STAbits.OERR)
	{
		U2STAbits.OERR = 0;
		IFS1bits.U2RXIF = 0;
	}
}

#ifdef UART3_ON 
void ClearUart3Err()
{
	if(U3STAbits.OERR)
	{
		U3STAbits.OERR = 0;
		IFS5bits.U3RXIF = 0;
	}
}
#endif