#ifndef _INTERRUPT_H_
	#define _INTERRUPT_H_
	

	//------------------------------------------------------------------
	// Prototipi delle funzioni
	//------------------------------------------------------------------
/*
void __attribute__((interrupt, no_auto_psv)) _U2TXInterrupt();
void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt();
void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt();
void __attribute__((interrupt, no_auto_psv)) _U1ErrInterrupt();
void __attribute__((interrupt, no_auto_psv)) _U2ErrInterrupt();
void __attribute__((interrupt, no_auto_psv)) _U3RXInterrupt();
void __attribute__((interrupt, no_auto_psv)) _U3TXInterrupt();
void __attribute__((interrupt, no_auto_psv)) _U3ErrInterrupt();
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt();
*/

void GlobalIntReadBus(void);
#endif
    
    
