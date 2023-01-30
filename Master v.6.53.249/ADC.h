//----------------------------------------------------------------------------------
//	Progect name:	ADC.h
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			Emanuele
//	Date:			04/01/2014
//	Description:	Header delle funzioni pilota per l'acquisizione analogico digitale
//----------------------------------------------------------------------------------
#ifndef _ADC_H_
	#define _ADC_H_
	//---------------------------------------
	// Define
	//---------------------------------------
/*
	Circuito condizionamento sonda PTC
	Vcc - termistore - Rserie (4K7)
	Amplificatore invertente -8,27 (Ri = 10K, Rf = 82K7)
	OffSet amplificatore 4,47V (R1 Vcc = 1K18, R2 GND = 10K)
	
	+-----------+-----------+------------+
	| Temp (°C)	| Res (hom)	| Ad_Value	 |
	+-----------+-----------+------------+
	| -40		| 567		| Over Range |
	+-----------+-----------+------------+
	| -30		| 624		| 0			 | K1 = 0,13 °C /bit
	+-----------+-----------+------------+
	| -20		| 684		| 80		 | K2 = 0,12 °C /bit
	+-----------+-----------+------------+
	| -10		| 747		| 164		 | K3 = 0,11 °C /bit
	+-----------+-----------+------------+
	| 0			| 815		| 254		 | K4 = 0,11 °C /bit
	+-----------+-----------+------------+
	| 10		| 886		| 349		 | K5 = 0,10 °C /bit
	+-----------+-----------+------------+
	| 20		| 961		| 449		 | K6 = 0,10 °C /bit
	+-----------+-----------+------------+
	| 25		| 1000		| 501		 | K7 = 0,9 °C /bit
	+-----------+-----------+------------+
	| 30		| 1040		| 554		 | K8 = 0,9 °C /bit
	+-----------+-----------+------------+
	| 40		| 1122		| 663		 | K9 = 0,9 °C /bit
	+-----------+-----------+------------+
	| 50		| 1209		| 779		 | K10 = 0,8 °C /bit
	+-----------+-----------+------------+
	| 60		| 1299		| 899		 | K11 = 0,8 °C /bit
	+-----------+-----------+------------+
	| 70		| 1392		| 1023		 |
	+-----------+-----------+------------+
	| 80		| 1490		| Over Range |
	+-----------+-----------+------------+
	| 90		| 1591		| Over Range |
	+-----------+-----------+------------+
	| 100		| 1696		| Over Range |
	+-----------+-----------+------------+
	| 110		| 1805		| Over Range |
	+-----------+-----------+------------+
	| 120		| 1915		| Over Range |
	+-----------+-----------+------------+
	| 130		| 2023		| Over Range |
	+-----------+-----------+------------+
	| 140		| 2124		| Over Range |
	+-----------+-----------+------------+
	| 150		| 2211		| Over Range |
	+-----------+-----------+------------+

	Temp = [Upper Limit Section Temp] + ([AD_Value] - [AD_Lower limit])* [K Section]

	#define	AD_T1			50	//0
	#define	AD_T2			138	//80
	#define	AD_T3			230	//164
	#define	AD_T4			330	//254
	#define	AD_T5			422	//349
	#define	AD_T6			537	//449
	#define	AD_T7			596	//501
	#define	AD_T8			653	//554
	#define	AD_T9			761	//663
	#define	AD_T10			863	//779
	#define	AD_T11			956	//899

	#define	Kt_1			11	//13
	#define	Kt_2			11	//12
	#define	Kt_3			10	//11
	#define	Kt_4			11	//11
	#define	Kt_5			9	//10
	#define	Kt_6			8	//10
	#define	Kt_7			9	//9
	#define	Kt_8			9	//9
	#define	Kt_9			10	//9
	#define	Kt_10			11	//8
	#define	Kt_11			17	//8
*/

	#define	K_Campioni_Acquisizione1	64//8	// max 255	T1 Mandata_Compressore
	#define	K_Campioni_Acquisizione2	64//8	// max 255	T2 Ritorno_Compressore
	#define	K_Campioni_Acquisizione3	64//16 //64	// max 255	T3 Condensatore
	#define	K_Campioni_Acquisizione4	64//16 //64	// max 255	T4 Acqua_Mare
	#define	K_Campioni_Acquisizione5	64//8	// max 255	T5 Collettore_Piccolo
	#define	K_Campioni_Acquisizione6	64//8	// max 255	T6 Collettore_Grande
	#define	K_Campioni_Acquisizione7	64//10//16//16	// max 255	Pressione LIQ_P
	#define	K_Campioni_Acquisizione8	64//32//16//16	// max 255	Pressione GAS_G
	#define	K_Campioni_Acquisizione9	64//10//16//16	// max 255	Pressione AUX

	// Offset per il calcolo delle temperature con range -30; +70°C
	#define	TempOffSet_1	-3000
	#define	TempOffSet_2	-2000
	#define	TempOffSet_3	-1000
	#define	TempOffSet_4	+0
	#define	TempOffSet_5	+1000
	#define	TempOffSet_6	+2000
	#define	TempOffSet_7	+2500
	#define	TempOffSet_8	+3000
	#define	TempOffSet_9	+4000
	#define	TempOffSet_10	+5000
	#define	TempOffSet_11	+6000

	// Offset per il calcolo della temperatura ambiente range +10; +40 °C
	#define	TempA_OffSet_1	+1000
	#define	TempA_OffSet_2	+1300
	#define	TempA_OffSet_3	+1600
	#define	TempA_OffSet_4	+1900
	#define	TempA_OffSet_5	+2100
	#define	TempA_OffSet_6	+2400
	#define	TempA_OffSet_7	+2700
	#define	TempA_OffSet_8	+3000
	#define	TempA_OffSet_9	+3300
	#define	TempA_OffSet_10	+3600
	#define	TempA_OffSet_11	+3900

	//---------------------------------------
	// Variabili
	//---------------------------------------
	/*
	int	Temperatura_1 = 0;		// Temp mandata compressorw
	int	Temperatura_2 = 0;		// Temp ritorno compressore
	int	Temperatura_3 = 0;		// Temp condensatore
	int	Temperatura_4 = 0;		// Temp acqua mare
	int	Temperatura_5 = 0;		// Temp Aux 1
	int	Temperatura_6 = 0;		// Temp Aux 2
	int Gas_Pressure = 0;		// Pressione del gas
*/

	#define VariabiliGlobaliADC()													\
		volatile int Adc_Temp[10] = {0,0,0,0,0,0,0,0,0,0};                             \
		volatile double Sum_Adc_Temp[10] = {0,0,0,0,0,0,0,0,0,0};						\
		volatile char N_Campioni[10] = {0,0,0,0,0,0,0,0,0,0};                          \
 		volatile char Campioni_Acquisizione[10] = {0,K_Campioni_Acquisizione1,K_Campioni_Acquisizione2,K_Campioni_Acquisizione3,     \
                                                  K_Campioni_Acquisizione4,K_Campioni_Acquisizione5,K_Campioni_Acquisizione6,     \
                                                  K_Campioni_Acquisizione7,K_Campioni_Acquisizione8,K_Campioni_Acquisizione9};
                                                                                              
	#define IncludeVariabiliGlobaliADC()                                            \
		extern volatile int Adc_Temp[10];                                            \
		extern volatile double Sum_Adc_Temp[10];										\
		extern volatile char N_Campioni[10];                                         \
 		extern volatile char Campioni_Acquisizione[10]; 

	//---------------------------------------
	// Variabili
	//---------------------------------------
	IncludeVariabiliGlobaliADC();

	//---------------------------------------
	// Impostazioni ADC
	//---------------------------------------
	#define	SouceRef		1	// 0=> Vr+=AVdd;		Vr-=AVss
								// 1=> Vr+=ExtVref+;	Vr-=AVss
								// 2=> Vr+=AVdd;		Vr-=ExtVref-
								// 3=> Vr+=ExtVref+;	Vr-=ExtVref-
								// 4-7=> Vr+=AVdd;		Vr-=AVss
	#define	AD_ResultFormat	1	// 0=> Left; 1=> Right
	#define	AcquisitionTime	7	// 0-31 => 0-31 T_AD
	#define	ConversionClock	2//6	// 0-63 => 1-64 Tcy
	#define DataOutFormat	0	// 0=> Unsigned (0000 00dd dddd dddd);
								// 1=> Signd integer (ssss sssd dddd dddd);
								// 2=> Fractional (dddd dddd dd00 0000);
								// 3=> Signed Fractional (sddd dddd dd00 0000);

    #define K_AD1PCFGL       0xF8C0     // Configurazione Analog input per NEWHW (>= v8.2) 
    #define K_AD1CSSL        0x073F     // A/D input chanel scan selection x NEWHW 8.2  0=ignored; 1=selected

                                                                                                                
                                                        
                                                        
	//---------------------------------------
	// Configurazione
	//---------------------------------------  
	#define Init_Adc()\
		_FORM = DataOutFormat;	/* formato uscita */						\
		_SSRC = 7;				/* sorgente conversion trigger (7-auto) */	\
		_ADSIDL = 0;				/* stop in idle mode */						\
		_ASAM = 1;				/* Sample auto start */						\
		_VCFG = SouceRef;		/* voltage reference configuration */		\
		_CSCNA = 1;				/* Scan/input 0=> use the chanel selectet by CH0SA */	\
											/*            1=> Scan input selected in AD1CSSL reg */	\
		_SMPI = 9;				/* Interrupt request. Ogni quante conversione avviene l'interrupt (0-16) */	\
		_BUFM = 0;				/* Buffer Mode select bit */				\
		_ALTS = 0;				/* Alternate input sample mode 1=> alternate btw mux A & B; 0=> sempre mux A*/	\
		_ADRC = 0;				/* A/D conversion clock source 1=>internal RC; 0=>sys clock */	\
		_SAMC = AcquisitionTime;	/* Auto sample time bits */					\
		_ADCS = ConversionClock;	/* A/D conversion clock period */			\
		_CH0NB = 0;				/* S/H amplifier negative input select for mux B 1=Neg in is AN1; 0=Neg in is VR- */\
		_CH0SB = 17;				/* S/H Amplifier positive input select for mux B */	\
		_CH0NA = 0;				/* S/H amplifier negative input select for mux A 1=Neg in is AN1; 0=Neg in is VR- */\
		_CH0SA = 17;				/* S/H Amplifier positive input select for mux A */	\
		ANCFG = 0;							/* skip internal Vbg, Vbg/2 and Vbg/6    NEW in GB210 */	\
        _ANSA6 = 0;	/* RA6 = DIP n.7 /* Analog Selection Register for Port A 1=Analog 0=Digital*/\
        _ANSA7 = 0;	/* RA7 = DIP n.8 /* Analog Selection Register for Port A 1=Analog 0=Digital*/\
        _ANSA9 = 0;	/* RA9 = N.C.    /* Analog Selection Register for Port A 1=Analog 0=Digital*/\
        _ANSA10 = 0;/* RA10 = +Vref /* Analog Selection Register for Port A 1=Analog 0=Digital*/\
        ANSB = 0x073F;      /* A/D old + Ne< in v8.2    /* Analog Selection Register for Port B 1=Analog 0=Digital*/ \
                            /* AN0..AN5 -> OLD   AN8 -> OLD   AN9,AN10 -> NEW */                        \
        ANSC = 0;                           /* Analog Selection Register for Port C 1=Analog 0=Digital*/ \
        ANSD = 0;                           /* Analog Selection Register for Port D 1=Analog 0=Digital*/ \
        ANSE = 0;                           /* Analog Selection Register for Port E 1=Analog 0=Digital*/ \
        ANSF = 0;                           /* Analog Selection Register for Port F 1=Analog 0=Digital*/ \
        ANSG = 0;                           /* Analog Selection Register for Port G 1=Analog 0=Digital*/ \
        AD1CSSL = 0x073F;   /* A/D old + New in v8.2/* A/D input chanel scan selection 0=ignored; 1=selected (ch15-0) */\
                            /* AN0..AN5 -> OLD   AN8 -> OLD   AN9,AN10 -> NEW */   \
		AD1CSSH = 0;						/* skip internal Vbg and Vbg/2 */\
		_ADON = 1				/* A/D module On */
               
                

	//---------------------------------------
	// Prototipi
	//---------------------------------------
    void AD_Read_Sched(void);
	void AD_Read(void);
	int AD_ReadSingle(char Chanel);
	int Temp_Read(volatile int AD_Value, volatile int* AD_T, volatile int* Kt);
	int Steinhart(int AD_Value, int Range);
	int T_Amb_Read(volatile int AD_Value, volatile int* AD_T, volatile int* Kt);
	unsigned int Pressure_Read(int AD_Value);
#endif
