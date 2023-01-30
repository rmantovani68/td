//----------------------------------------------------------------------------------
//	Progect name:	ADC.h
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			TDM Team
//	Date:			04/01/2014
//	Description:	Header delle funzioni pilota per l'acquisizione analogico digitale
//----------------------------------------------------------------------------------
#include "HWSelection.h"
#if (GB210==1) 
	#include <p24FJ256GB210.h>
#elif (DA210==1)
	#include <p24FJ256DA210.h>
#else
	#include <p24FJ256GB110.h>
#endif
#include "FWSelection.h"
#ifndef _ADC_H_
	#define _ADC_H_
	//---------------------------------------
	// Define
	//---------------------------------------
	//#define	Campioni_Acquisizione	64//32 //6 //10 // max 60

	#define	K_Campioni_Acquisizione1	64//8	// max 255	T GAS
	#define	K_Campioni_Acquisizione2	64//8	// max 255	T Liquid
	#define	K_Campioni_Acquisizione3	64//16  // max 255	T Out
	#define	K_Campioni_Acquisizione4	64//16 	// max 255	T Amb
#if(K_Channel_Gas_Regulation==1)
	#define	K_Campioni_Acquisizione5	2//8	// max 255	P GAS
#elif(K_DiffPress_Auto_Fan_Control==1)
	#define	K_Campioni_Acquisizione5	16	// max 255	P GAS
#else
	#define	K_Campioni_Acquisizione5	64//8	// max 255	P GAS
#endif
	#define	K_Campioni_Acquisizione6	64//8	// max 255	P Liquid
	#define	K_Campioni_Acquisizione7	64//10 	// max 255	I FAN
	#define	K_Campioni_Acquisizione8	64//32	// max 255	T Aux1 
	#define	K_Campioni_Acquisizione9	64//10	// max 255	T Aux2 
	#define	K_Campioni_Acquisizione10	64//10	// max 255	V Supply 

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

    #define K_Humidity_Max_Value            100          //Fondo scala umidita = 100%

	#define VariabiliGlobaliADC()                                       \
		volatile int Adc_Temp[11] = {0,0,0,0,0,0,0,0,0,0,0};            \
		volatile double Sum_Adc_Temp[11] = {0,0,0,0,0,0,0,0,0,0,0};     \
		volatile char N_Campioni[11] = {0,0,0,0,0,0,0,0,0,0,0};         \
 		volatile char Campioni_Acquisizione[11] = {0,K_Campioni_Acquisizione1,K_Campioni_Acquisizione2,K_Campioni_Acquisizione3,    \
                                                  K_Campioni_Acquisizione4,K_Campioni_Acquisizione5,K_Campioni_Acquisizione6,       \
                                                  K_Campioni_Acquisizione7,K_Campioni_Acquisizione8,K_Campioni_Acquisizione9,K_Campioni_Acquisizione10};
                                                                                              
	#define IncludeVariabiliGlobaliADC()                        \
		extern volatile int Adc_Temp[11];                       \
		extern volatile double Sum_Adc_Temp[11];                \
		extern volatile char N_Campioni[11];                    \
 		extern volatile char Campioni_Acquisizione[11]; 


//	#define VariabiliGlobaliADC()													\
//		volatile int Adc_T_Liquid = 0;	/* Valore ADC Temp bulbo piccolo */			\
//		volatile int Adc_T_Gas = 0;	/* Valore ADC Temp bulbo grande */              \
//		volatile int Adc_T_Out = 0;		/* Valore ADC Temp aria in uscita */		\
//		volatile int Adc_T_Amb = 0;		/* Valore ADC Temp ambiente */				\
//		volatile int Adc_P_Grande = 0;	/* Valore ADC pressione bulbo grande */		\
//		volatile int Adc_P_Piccolo = 0;	/* Valore ADC pressione bulbo piccolo */	\
//		volatile int Adc_I_Fan = 0;		/* Valore ADC Corrente ventilatore */		\
//		volatile int Adc_Aux1 = 0;		/* Valore ADC Canale AUX 1         */		\
//		volatile int Adc_Aux2 = 0;		/* Valore ADC Canale AUX 2         */		\
//		volatile int Adc_Supply = 0;    /* Valore ADC Canale Supply 24V    */		\
//		/* valore somma dei campioni della media mobile */							\
//		volatile double Sum_Adc_T_Liquid = 0;										\
//		volatile double Sum_Adc_T_Gas = 0;                                          \
//		volatile double Sum_Adc_T_Out = 0;											\
//		volatile double Sum_Adc_T_Amb = 0;											\
//		volatile double Sum_Adc_P_Grande = 0;										\
//		volatile double Sum_Adc_P_Piccolo = 0;										\
//		volatile double Sum_Adc_I_Fan = 0;  										\
//        volatile double Sum_Adc_Aux1 = 0;                               			\
//        volatile double Sum_Adc_Aux2 = 0;                   						\
//        volatile double Sum_Adc_Supply = 0;                                         \
//																					\
//		/* numero dei campioni della media mobile */								\
//		volatile char N_Campioni = 0;												
																					

//	#define IncludeVariabiliGlobaliADC()			\
//		extern volatile int Adc_T_Liquid;			\
//		extern volatile int Adc_T_Gas;              \
//		extern volatile int Adc_T_Out;				\
//		extern volatile int Adc_T_Amb;				\
//		extern volatile int Adc_P_Grande;			\
//		extern volatile int Adc_P_Piccolo;			\
//		extern volatile int Adc_I_Fan;				\
//        extern volatile int Adc_Aux1;				\
//		extern volatile int Adc_Aux2;				\
//		extern volatile int Adc_Supply;				\
//		extern volatile double Sum_Adc_T_Liquid;	\
//		extern volatile double Sum_Adc_T_Gas;       \
//		extern volatile double Sum_Adc_T_Out;		\
//		extern volatile double Sum_Adc_T_Amb;		\
//		extern volatile double Sum_Adc_P_Grande;	\
//		extern volatile double Sum_Adc_P_Piccolo;	\
//		extern volatile double Sum_Adc_I_Fan;       \
//        extern volatile double Sum_Adc_Aux1;        \
//        extern volatile double Sum_Adc_Aux2;        \
//        extern volatile double Sum_Adc_Supply;      \
//		extern volatile char N_Campioni;

                                                                      
                                                                      
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
#if (GB210==1 || DA210==1)   
    #define K_AD1CSSL        0x0F3F                   
#else
    #define K_AD1CSSL        0x073F     // A/D input chanel scan selection x NEWHW 8.2  0=ignored; 1=selected
#endif
                                                        
                                                        
                                                        
	//---------------------------------------
	// Configurazione
	//---------------------------------------
#if (GB210==1 || DA210==1)   
	#define Init_Adc()\
		AD1CON1bits.FORM = DataOutFormat;	/* formato uscita */						\
		AD1CON1bits.SSRC = 7;				/* sorgente conversion trigger (7-auto) */	\
		AD1CON1bits.ADSIDL = 0;				/* stop in idle mode */						\
		AD1CON1bits.ASAM = 1;				/* Sample auto start */						\
		AD1CON2bits.VCFG = SouceRef;		/* voltage reference configuration */		\
		AD1CON2bits.CSCNA = 1;				/* Scan/input 0=> use the chanel selectet by CH0SA */	\
											/*            1=> Scan input selected in AD1CSSL reg */	\
		AD1CON2bits.SMPI = 10;/*9;				/* Interrupt request. Ogni quante conversione avviene l'interrupt (0-16) */	\
		AD1CON2bits.BUFM = 0;				/* Buffer Mode select bit */				\
		AD1CON2bits.ALTS = 0;				/* Alternate input sample mode 1=> alternate btw mux A & B; 0=> sempre mux A*/	\
		AD1CON3bits.ADRC = 0;				/* A/D conversion clock source 1=>internal RC; 0=>sys clock */	\
		AD1CON3bits.SAMC = AcquisitionTime;	/* Auto sample time bits */					\
		AD1CON3bits.ADCS = ConversionClock;	/* A/D conversion clock period */			\
		AD1CHSbits.CH0NB = 0;				/* S/H amplifier negative input select for mux B 1=Neg in is AN1; 0=Neg in is VR- */\
		AD1CHSbits.CH0SB = 17;				/* S/H Amplifier positive input select for mux B */	\
		AD1CHSbits.CH0NA = 0;				/* S/H amplifier negative input select for mux A 1=Neg in is AN1; 0=Neg in is VR- */\
		AD1CHSbits.CH0SA = 0; /*17;				/* S/H Amplifier positive input select for mux A */	\
		ANCFG = 0;							/* skip internal Vbg, Vbg/2 and Vbg/6    NEW in GB210 */	\
        ANSAbits.ANSA6 = 0;	/* RA6 = DIP n.7 *//* Analog Selection Register for Port A 1=Analog 0=Digital*/\
        ANSAbits.ANSA7 = 0;	/* RA7 = DIP n.8 *//* Analog Selection Register for Port A 1=Analog 0=Digital*/\
        ANSAbits.ANSA9 = 0;	/* RA9 = N.C.    *//* Analog Selection Register for Port A 1=Analog 0=Digital*/\
        ANSAbits.ANSA10 = 0;/* RA10 = +Vref *//* Analog Selection Register for Port A 1=Analog 0=Digital*/\
        ANSB = K_AD1CSSL;      /* A/D old + Ne< in v8.2    *//* Analog Selection Register for Port B 1=Analog 0=Digital*/ \
                            /* AN0..AN5 -> OLD   AN8 -> OLD   AN9,AN10 -> NEW */                        \
        ANSC = 0;                           /* Analog Selection Register for Port C 1=Analog 0=Digital*/ \
        ANSD = 0;                           /* Analog Selection Register for Port D 1=Analog 0=Digital*/ \
        ANSE = 0;                           /* Analog Selection Register for Port E 1=Analog 0=Digital*/ \
        ANSF = 0;                           /* Analog Selection Register for Port F 1=Analog 0=Digital*/ \
        ANSG = 0;                           /* Analog Selection Register for Port G 1=Analog 0=Digital*/ \
        AD1CSSL = K_AD1CSSL;   /* A/D old + New in v8.2*//* A/D input chanel scan selection 0=ignored; 1=selected (ch15-0) */\
                            /* AN0..AN5 -> OLD   AN8 -> OLD   AN9,AN10 -> NEW */   \
		AD1CSSH = 0;						/* skip internal Vbg and Vbg/2 */\
		AD1CON1bits.ADON = 1				/* A/D module On */
#else
	#define Init_Adc()\
		AD1CON1bits.FORM = DataOutFormat;	/* formato uscita */						\
		AD1CON1bits.SSRC = 7;				/* sorgente conversion trigger (7-auto) */	\
		AD1CON1bits.ADSIDL = 0;				/* stop in idle mode */						\
		AD1CON1bits.ASAM = 1;				/* Sample auto start */						\
		AD1CON2bits.VCFG = SouceRef;		/* voltage reference configuration */		\
		AD1CON2bits.CSCNA = 1;				/* Scan/input 0=> use the chanel selectet by CH0SA */	\
											/*            1=> Scan input selected in AD1CSSL reg */	\
		AD1CON2bits.SMPI = 9;				/* Interrupt request. Ogni quante conversione avviene l'interrupt (0-16) */	\
		AD1CON2bits.BUFM = 0;				/* Buffer Mode select bit */				\
		AD1CON2bits.ALTS = 0;				/* Alternate input sample mode 1=> alternate btw mux A & B; 0=> sempre mux A*/	\
		AD1CON3bits.ADRC = 0;				/* A/D conversion clock source 1=>internal RC; 0=>sys clock */	\
		AD1CON3bits.SAMC = AcquisitionTime;	/* Auto sample time bits */					\
		AD1CON3bits.ADCS = ConversionClock;	/* A/D conversion clock period */			\
		AD1CHSbits.CH0NB = 0;				/* S/H amplifier negative input select for mux B 1=Neg in is AN1; 0=Neg in is VR- */\
		AD1CHSbits.CH0SB = 17;				/* S/H Amplifier positive input select for mux B */	\
		AD1CHSbits.CH0NA = 0;				/* S/H amplifier negative input select for mux A 1=Neg in is AN1; 0=Neg in is VR- */\
		AD1CHSbits.CH0SA = 17;				/* S/H Amplifier positive input select for mux A */	\
		AD1PCFGL = K_AD1PCFGL;              /* Analog input configuration 0=Analog; 1=Digital (ch15-0)*/        \
		AD1PCFGH = 0x0000;                  /* Analog input configuration #bit0 VBG/2 (0=enable; 1=disable)*/ 	\
                                            /* Analog input configuration #bit1 VBG (0=enable; 1=disable)*/ 	\
        AD1CSSL = K_AD1CSSL;            	/* A/D input chanel scan selection 0=ignored; 1=selected (ch15-0) */\
		AD1CSSH = 0;						/* skip internal Vbg and Vbg/2 */\
		AD1CON1bits.ADON = 1				/* A/D module On */
#endif                
                
                
                
             
                
	//---------------------------------------
	// Prototipi
	//---------------------------------------
	void AD_Read(void);
	int AD_ReadSingle(char Chanel);
	int Temp_Read(volatile int AD_Value, volatile int* AD_T, volatile int* Kt);
	int T_Amb_Read(volatile int AD_Value, volatile int* AD_T, volatile int* Kt);
	unsigned int Pressure_Read_45(int AD_Value);
	unsigned int Pressure_Read_34(int AD_Value);
	int Steinhart(int AD_Value, int Range);
    int DifferentialPressureConversion(int AD_Value);
    void AD_Read_Sched(void);        
#endif
