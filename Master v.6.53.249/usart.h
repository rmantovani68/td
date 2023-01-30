#ifndef _SERIAL_H_
	#define _SERIAL_H_
#include "FWSelection.h"
	
	//------------------------------------------------------------------
	// costanti per la modalità di funzionamento
	//------------------------------------------------------------------
	// Costanti EnBits
	#define En_TxRxBclk		3
	#define	En_TxRxCtsRts	2
	#define	En_TxRxRts		1
	#define	En_TxRx			0
	
	// Costanti per ParityData
	#define	None_9bit		3
	#define	Odd_8bit		2
	#define	Even_8bit		1
	#define	None_8bit		0
	
	// Costanti per StopBits
	#define	One				0
	#define	Two				1
	
	// Costanti per TxInterruptMode
	#define	NotUse			3	// Combinazione illegale, errore
	#define	EndTsrBuffEmpt	2	// Interrupt quando un carattere è appena stato inviato al tsr (transfer shift register) e il buffer si è svuotato
	#define	BuffEmpyt		1	// Interrupt quando il buffer di tx si svuota
	#define	Tsr				0	// Interrupt quando un carattere viene inviato al TSR
	
	// Costanti per RxInterruptMode
	#define	OnBuffFull		3	// Interrupt quando il buffer di ricezione e pieno
	#define	OnBuff4_3		2	// Interrupt quando il buffer di ricezione è a 3/4
	#define	OnCharRx		0	// Interrupt per ogni carattere ricevuto
	
	//------------------------------------------------------------------
	//		SETTAGGI GENERICI
	//------------------------------------------------------------------
	#define FOSC			16000000L	// Frequenza del core in Hz
	
	//------------------------------------------------------------------
	//		SETTAGGI UART 1
	//------------------------------------------------------------------
	#define U1_Baud38400    38400		// Baudrate desiderato in bps LowSpeed
	#define U1_Baud57600	57600		// Baudrate desiderato in bps HiSpeed  (33-> 34)
	#define U1_Baud115200	115200      // Baudrate desiderato in bps HiSpeed  (33-> 34)
	#define U1_Baud250000	250000      // Baudrate desiderato in bps HiSpeed  (33-> 34)
    #define	U1_IrEncode		0			// Abilita Ir Encoder per la seriale (1=Attivo; 0=Disattivato)
	#define	U1_RtsMode		1			// scelgo la modalità di controllo del bit RTS (1=Simplex Mode; 0=Flow Control Mode)
	#define	U1_EnBits		En_TxRx		// Scelgo i bit che voglio abilitare
	#define	U1_WakeUpRx		0			// Abilita il wake-up in caso di ricezione (1=Attivo; 0=Disattivato)
	#define	U1_RxInv		0			// Abilita l'inversione di polarità del pin rx (1=Attivo; 0=Disattivato)
	#define U1_HighSpeed	1			// Imposta il divisore per alte velocità (1=Attivo; 0=Disattivato)
	#define	U1_ParityData	None_8bit	// Imposta la parità ed il numero di bit
	#define	U1_StopBit		One			// Imposto il numero di bit di stop
	#define	U1_TxIntMode	BuffEmpyt	// imposto l'interrupt di tx
	#define	U1_TxInv		0			// Abilita l'inversione di polarità del pin Tx (1=Attivo; 0=Disattivato)
	#define	U1_RxIntMode	OnCharRx	// imposto l'interrupt di rx
	
	//------------------------------------------------------------------
	//		SETTAGGI UART 2
	//------------------------------------------------------------------
	#define U2_Baud38400	38400 		// Baudrate desiderato in bps
	#define U2_Baud57600	57600 		// Baudrate desiderato in bps
	#define U2_Baud115200	115200      // Baudrate desiderato in bps
	#define	U2_IrEncode		0			// Abilita Ir Encoder per la seriale (1=Attivo; 0=Disattivato)
	#define	U2_RtsMode		1			// scelgo la modalità di controllo del bit RTS (1=Simplex Mode; 0=Flow Control Mode)
	#define	U2_EnBits		En_TxRx		// Scelgo i bit che voglio abilitare
	#define	U2_WakeUpRx		0			// Abilita il wake-up in caso di ricezione (1=Attivo; 0=Disattivato)
	#define	U2_RxInv		0			// Abilita l'inversione di polarità del pin rx (1=Attivo; 0=Disattivato)
	#define U2_HighSpeed	1			// Imposta il divisore per alte velocità (1=Attivo; 0=Disattivato)
	#define	U2_ParityData	None_8bit	// Imposta la parità ed il numero di bit
	#define	U2_StopBit		One			// Imposto il numero di bit di stop
	#define	U2_TxIntMode	BuffEmpyt	// imposto l'interrupt di tx
	#define	U2_TxInv		0			// Abilita l'inversione di polarità del pin Tx (1=Attivo; 0=Disattivato)
	#define	U2_RxIntMode	OnCharRx	// imposto l'interrupt di rx
	
	//------------------------------------------------------------------
	//		SETTAGGI UART 3
	//------------------------------------------------------------------
	#define U3_Baud9600		9600        // Baudrate desiderato in bps
    #define U3_Baud38400	38400       // Baudrate desiderato in bps
	#define U3_Baud57600	57600       // Baudrate desiderato in bps
    #define U3_Baud115200	115200      // Baudrate desiderato in bps
	#define	U3_IrEncode		0			// Abilita Ir Encoder per la seriale (1=Attivo; 0=Disattivato)
	#define	U3_RtsMode		1			// scelgo la modalità di controllo del bit RTS (1=Simplex Mode; 0=Flow Control Mode)
	#define	U3_EnBits		En_TxRx		// Scelgo i bit che voglio abilitare
	#define	U3_WakeUpRx		0			// Abilita il wake-up in caso di ricezione (1=Attivo; 0=Disattivato)
	#define	U3_RxInv		0			// Abilita l'inversione di polarità del pin rx (1=Attivo; 0=Disattivato)
	#define U3_HighSpeed	1			// Imposta il divisore per alte velocità (1=Attivo; 0=Disattivato)
	#define	U3_ParityData	None_8bit	// Imposta la parità ed il numero di bit
	#define	U3_StopBit		One			// Imposto il numero di bit di stop
	#define	U3_TxIntMode	BuffEmpyt	// imposto l'interrupt di tx
	#define	U3_TxInv		0			// Abilita l'inversione di polarità del pin Tx (1=Attivo; 0=Disattivato)
	#define	U3_RxIntMode	OnCharRx	// imposto l'interrupt di rx  

	//------------------------------------------------------------------
	//		SETTAGGI UART 4
	//------------------------------------------------------------------
	#define U4_Baud9600		9600        // Baudrate desiderato in bps
    #define U4_Baud38400	38400       // Baudrate desiderato in bps
	#define U4_Baud57600	57600       // Baudrate desiderato in bps
    #define U4_Baud115200	115200      // Baudrate desiderato in bps
	#define	U4_IrEncode		0			// Abilita Ir Encoder per la seriale (1=Attivo; 0=Disattivato)
	#define	U4_RtsMode		1			// scelgo la modalità di controllo del bit RTS (1=Simplex Mode; 0=Flow Control Mode)
	#define	U4_EnBits		En_TxRx		// Scelgo i bit che voglio abilitare
	#define	U4_WakeUpRx		0			// Abilita il wake-up in caso di ricezione (1=Attivo; 0=Disattivato)
	#define	U4_RxInv		0			// Abilita l'inversione di polarità del pin rx (1=Attivo; 0=Disattivato)
	#define U4_HighSpeed	1			// Imposta il divisore per alte velocità (1=Attivo; 0=Disattivato)
	#define	U4_ParityData	None_8bit	// Imposta la parità ed il numero di bit
	#define	U4_StopBit		One			// Imposto il numero di bit di stop
	#define	U4_TxIntMode	BuffEmpyt	// imposto l'interrupt di tx
	#define	U4_TxInv		0			// Abilita l'inversione di polarità del pin Tx (1=Attivo; 0=Disattivato)
	#define	U4_RxIntMode	OnCharRx	// imposto l'interrupt di rx  


	//------------------------------------------------------------------
	// CODICE PER IL SETTAGGIO DELLA PORTA 1
	//------------------------------------------------------------------
	#if U1_HighSpeed == 0
		#define U1_Divider38400 ((int)(FOSC/(16UL * U1_Baud38400) -1))    // LowSpeed
		#define U1_Divider57600 ((int)(FOSC/(16UL * U1_Baud57600) -1)   
		#define U1_Divider115200 ((int)(FOSC/(16UL * U1_Baud115200) -1))
		#define U1_Divider250000 ((int)(FOSC/(16UL * U1_Baud250000) -1))
	#else
		#define U1_Divider38400 ((int)(FOSC/(4UL * U1_Baud38400) -1))     // LowSpeed
		#define U1_Divider57600 ((int)(FOSC/(4UL * U1_Baud57600) -1))   
		#define U1_Divider115200 ((int)(FOSC/(4UL * U1_Baud115200) -1))
		#define U1_Divider250000 ((int)(FOSC/(4UL * U1_Baud250000) -1))
	#endif
	
	#if U1_EnBits > 1
		#define	Tmp_U1_UEN1 1
	#else
		#define	Tmp_U1_UEN1 0
	#endif
	
	#if (U1_EnBits == 1)||(U1_EnBits == 3)
		#define	Tmp_U1_UEN0 1
	#else
		#define	Tmp_U1_UEN0 0
	#endif
	
	#if U1_TxIntMode > 1
		#define	Tmp_U1_IntMd1 1
	#else
		#define	Tmp_U1_IntMd1 0
	#endif
	
	#if (U1_TxIntMode == 1)||(U1_TxIntMode == 3)
		#define	Tmp_U1_IntMd0 1
	#else
		#define	Tmp_U1_IntMd0 0
	#endif
	
	//------------------------------------------------------------------
	// CODICE PER IL SETTAGGIO DELLA PORTA 2
	//------------------------------------------------------------------
	#if U2_HighSpeed == 0
		#define U2_Divider38400 ((int)(FOSC/(16UL * U2_Baud38400) -1))
		#define U2_Divider57600 ((int)(FOSC/(16UL * U2_Baud57600) -1))
		#define U2_Divider115200 ((int)(FOSC/(16UL * U2_Baud115200) -1))
	#else
		#define U2_Divider38400 ((int)(FOSC/(4UL * U2_Baud38400) -1))
		#define U2_Divider57600 ((int)(FOSC/(4UL * U2_Baud57600) -1))
		#define U2_Divider115200 ((int)(FOSC/(4UL * U2_Baud115200) -1))
	#endif
	
	#if U2_EnBits > 1
		#define	Tmp_U2_UEN1 1
	#else
		#define	Tmp_U2_UEN1 0
	#endif
	
	#if (U2_EnBits == 1)||(U2_EnBits == 3)
		#define	Tmp_U2_UEN0 1
	#else
		#define	Tmp_U2_UEN0 0
	#endif
	
	#if U2_TxIntMode > 1
		#define	Tmp_U2_IntMd1 1
	#else
		#define	Tmp_U2_IntMd1 0
	#endif
	
	#if (U2_TxIntMode == 1)||(U2_TxIntMode == 3)
		#define	Tmp_U2_IntMd0 1
	#else
		#define	Tmp_U2_IntMd0 0
	#endif
	


	//------------------------------------------------------------------
	// CODICE PER IL SETTAGGIO DELLA PORTA 3
	//------------------------------------------------------------------
	#if U3_HighSpeed == 0
		#define U3_Divider9600 ((int)(FOSC/(16UL * U3_Baud9600) -1))
		#define U3_Divider38400 ((int)(FOSC/(16UL * U3_Baud38400) -1))
		#define U3_Divider57600 ((int)(FOSC/(16UL * U3_Baud57600) -1))
		#define U3_Divider115200 ((int)(FOSC/(16UL * U3_Baud115200) -1))
	#else
		#define U3_Divider9600 ((int)(FOSC/(4UL * U3_Baud9600) -1))
		#define U3_Divider38400 ((int)(FOSC/(4UL * U3_Baud38400) -1))
		#define U3_Divider57600 ((int)(FOSC/(4UL * U3_Baud57600) -1))
		#define U3_Divider115200 ((int)(FOSC/(4UL * U3_Baud115200) -1))
	#endif
	
	#if U3_EnBits > 1
		#define	Tmp_U3_UEN1 1
	#else
		#define	Tmp_U3_UEN1 0
	#endif
	
	#if (U3_EnBits == 1)||(U3_EnBits == 3)
		#define	Tmp_U3_UEN0 1
	#else
		#define	Tmp_U3_UEN0 0
	#endif
	
	#if U3_TxIntMode > 1
		#define	Tmp_U3_IntMd1 1
	#else
		#define	Tmp_U3_IntMd1 0
	#endif
	
	#if (U3_TxIntMode == 1)||(U3_TxIntMode == 3)
		#define	Tmp_U3_IntMd0 1
	#else
		#define	Tmp_U3_IntMd0 0
	#endif


	//------------------------------------------------------------------
	// CODICE PER IL SETTAGGIO DELLA PORTA 4
	//------------------------------------------------------------------
	#if U4_HighSpeed == 0
		#define U4_Divider9600 ((int)(FOSC/(16UL * U4_Baud9600) -1))
		#define U4_Divider38400 ((int)(FOSC/(16UL * U4_Baud38400) -1))
		#define U4_Divider57600 ((int)(FOSC/(16UL * U4_Baud57600) -1))
		#define U4_Divider115200 ((int)(FOSC/(16UL * U4_Baud115200) -1))
	#else
		#define U4_Divider9600 ((int)(FOSC/(4UL * U4_Baud9600) -1))
		#define U4_Divider38400 ((int)(FOSC/(4UL * U4_Baud38400) -1))
		#define U4_Divider57600 ((int)(FOSC/(4UL * U4_Baud57600) -1))
		#define U4_Divider115200 ((int)(FOSC/(4UL * U4_Baud115200) -1))
	#endif
	
	#if U4_EnBits > 1
		#define	Tmp_U4_UEN1 1
	#else
		#define	Tmp_U4_UEN1 0
	#endif
	
	#if (U4_EnBits == 1)||(U4_EnBits == 3)
		#define	Tmp_U4_UEN0 1
	#else
		#define	Tmp_U4_UEN0 0
	#endif
	
	#if U4_TxIntMode > 1
		#define	Tmp_U4_IntMd1 1
	#else
		#define	Tmp_U4_IntMd1 0
	#endif
	
	#if (U4_TxIntMode == 1)||(U4_TxIntMode == 3)
		#define	Tmp_U4_IntMd0 1
	#else
		#define	Tmp_U4_IntMd0 0
	#endif


	//------------------------------------------------------------------
	// Serial initialization UART1
	//------------------------------------------------------------------

	#define init_comms1()					\
		oRTS_Master = 0;					\
		U1BRG = U1_Divider57600; 			\
		U1MODEbits.UARTEN = 1;				\
		U1MODEbits.USIDL = 0;				\
		U1MODEbits.IREN = U1_IrEncode;		\
		U1MODEbits.RTSMD = U1_RtsMode;		\
		U1MODEbits.UEN0 = Tmp_U1_UEN0;		\
		U1MODEbits.UEN1 = Tmp_U1_UEN1;		\
		U1MODEbits.WAKE = U1_WakeUpRx;		\
		U1MODEbits.LPBACK = 0;				\
		U1MODEbits.ABAUD = 0;				\
		U1MODEbits.RXINV = U1_RxInv;		\
		U1MODEbits.BRGH = U1_HighSpeed;		\
		U1MODEbits.PDSEL = U1_ParityData;	\
		U1MODEbits.STSEL = U1_StopBit;		\
		U1STAbits.UTXISEL1 = Tmp_U1_IntMd1;	\
		U1STAbits.UTXISEL0 = Tmp_U1_IntMd0;	\
		U1STAbits.UTXINV = U1_TxInv;		\
		U1STAbits.UTXBRK = 0;				\
		U1STAbits.UTXEN	= 1;				\
		U1STAbits.URXISEL = U1_RxIntMode;	\
		U1STAbits.ADDEN = 0;
		
	
	//------------------------------------------------------------------
	// Serial initialization UART2
	//------------------------------------------------------------------
	#define init_comms2()					\
		oRTS_Slave = 0;						\
		U2BRG = U2_Divider57600;    		\
		U2MODEbits.UARTEN = 1;				\
		U2MODEbits.USIDL = 0;				\
		U2MODEbits.IREN = U2_IrEncode;		\
		U2MODEbits.RTSMD = U2_RtsMode;		\
		U2MODEbits.UEN0 = Tmp_U2_UEN0;		\
		U2MODEbits.UEN1 = Tmp_U2_UEN1;		\
		U2MODEbits.WAKE = U2_WakeUpRx;		\
		U2MODEbits.LPBACK = 0;				\
		U2MODEbits.ABAUD = 0;				\
		U2MODEbits.RXINV = U2_RxInv;		\
		U2MODEbits.BRGH = U2_HighSpeed;		\
		U2MODEbits.PDSEL = U2_ParityData;	\
		U2MODEbits.STSEL = U2_StopBit;		\
		U2STAbits.UTXISEL1 = Tmp_U2_IntMd1;	\
		U2STAbits.UTXISEL0 = Tmp_U2_IntMd0;	\
		U2STAbits.UTXINV = U2_TxInv;		\
		U2STAbits.UTXBRK = 0;				\
		U2STAbits.UTXEN	= 1;				\
		U2STAbits.URXISEL = U2_RxIntMode;	\
		U2STAbits.ADDEN = 0;


	//------------------------------------------------------------------
	// Serial initialization UART3
	//------------------------------------------------------------------
	#define init_comms3()					\
		oRTS_UART3 = 0;						\
		U3BRG = U3_Divider115200;           \
		U3MODEbits.UARTEN = 1;				\
		U3MODEbits.USIDL = 0;				\
		U3MODEbits.IREN = U3_IrEncode;		\
		U3MODEbits.RTSMD = U3_RtsMode;		\
		U3MODEbits.UEN0 = Tmp_U3_UEN0;		\
		U3MODEbits.UEN1 = Tmp_U3_UEN1;		\
		U3MODEbits.WAKE = U3_WakeUpRx;		\
		U3MODEbits.LPBACK = 0;				\
		U3MODEbits.ABAUD = 0;				\
		U3MODEbits.RXINV = U3_RxInv;		\
		U3MODEbits.BRGH = U3_HighSpeed;		\
		U3MODEbits.PDSEL = U3_ParityData;	\
		U3MODEbits.STSEL = U3_StopBit;		\
		U3STAbits.UTXISEL1 = Tmp_U3_IntMd1;	\
		U3STAbits.UTXISEL0 = Tmp_U3_IntMd0;	\
		U3STAbits.UTXINV = U3_TxInv;		\
		U3STAbits.UTXBRK = 0;				\
		U3STAbits.UTXEN	= 1;				\
		U3STAbits.URXISEL = U3_RxIntMode;	\
		U3STAbits.ADDEN = 0;


	//------------------------------------------------------------------
	// Serial initialization UART4
	//------------------------------------------------------------------
	#define init_comms4()					\
		oRTS_UART4 = 0;						\
		U4BRG = U4_Divider115200;           \
		U4MODEbits.UARTEN = 1;				\
		U4MODEbits.USIDL = 0;				\
		U4MODEbits.IREN = U4_IrEncode;		\
		U4MODEbits.RTSMD = U4_RtsMode;		\
		U4MODEbits.UEN0 = Tmp_U4_UEN0;		\
		U4MODEbits.UEN1 = Tmp_U4_UEN1;		\
		U4MODEbits.WAKE = U4_WakeUpRx;		\
		U4MODEbits.LPBACK = 0;				\
		U4MODEbits.ABAUD = 0;				\
		U4MODEbits.RXINV = U4_RxInv;		\
		U4MODEbits.BRGH = U4_HighSpeed;		\
		U4MODEbits.PDSEL = U4_ParityData;	\
		U4MODEbits.STSEL = U4_StopBit;		\
		U4STAbits.UTXISEL1 = Tmp_U4_IntMd1;	\
		U4STAbits.UTXISEL0 = Tmp_U4_IntMd0;	\
		U4STAbits.UTXINV = U4_TxInv;		\
		U4STAbits.UTXBRK = 0;				\
		U4STAbits.UTXEN	= 1;				\
		U4STAbits.URXISEL = U4_RxIntMode;	\
		U4STAbits.ADDEN = 0;



	#define set_baudrateU1_57600()              \
		U1BRG = U1_Divider57600;					

	#define set_baudrateU1_38400()              \
		U1BRG = U1_Divider38400;					

    #define set_baudrateU1_250000()             \
		U1BRG = U1_Divider250000;	

    #define set_baudrateU1_115200()             \
		U1BRG = U1_Divider115200;	

	#define set_baudrateU2_57600()				\
		U2BRG = U2_Divider57600;					

	#define set_baudrateU2_38400()              \
		U2BRG = U2_Divider38400;					

    #define set_baudrateU2_115200()             \
		U2BRG = U2_Divider115200;					

    #define set_baudrateU3_9600()               \
		U3BRG = U3_Divider9600;					

    #define set_baudrateU3_38400()              \
		U3BRG = U3_Divider38400;					

	#define set_baudrateU3_57600()				\
		U3BRG = U3_Divider57600;					

    #define set_baudrateU3_115200()             \
		U3BRG = U3_Divider115200;					

    #define set_baudrateU4_9600()               \
		U4BRG = U4_Divider9600;					

    #define set_baudrateU4_38400()              \
		U4BRG = U4_Divider38400;					

	#define set_baudrateU4_57600()				\
		U4BRG = U4_Divider57600;					

    #define set_baudrateU4_115200()             \
		U4BRG = U4_Divider115200;		

	//------------------------------------------------------------------
	// Prototipi delle funzioni
	//------------------------------------------------------------------
	void putch1(char byte);
	char getch1(void);
	void putch2(char byte);
	char getch2(void);
	void putch3(char byte);
	char getch3(void);
	void putch4(char byte);
	char getch4(void);    
	void ClearUart1Err(void);
	void ClearUart2Err(void);
	void ClearUart3Err(void);
	void ClearUart4Err(void);
#endif
