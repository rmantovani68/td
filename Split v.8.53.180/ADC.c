//----------------------------------------------------------------------------------
//	Progect name:	ADC.c
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			TDM Team
//	Date:			04/01/2014
//	Description:	Corpo delle funzioni pilota per l'acquisizione analogico digitale
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
#include <math.h>
#include "Delay.h"
#include "DefinePeriferiche.h"
#include "ADC.h"
#include "Core.h"

void AD_Read_Sched(void)
{
    static int sched=0;
    
    if(sched==0)        // Lancia le conversioni (9))
    {
        _CSCNA = 1;		// Scan/input 0=> use the chanel selectet by CH0SA
                                    //            1=> Scan input selected in AD1CSSL reg
        _CH0NA = 0;		// selected negative input for mux A to AGnd
        _CH0SA = 0;		// selected input for mux A
        AD1CSSL = 0x0F3F; //0x0FFF;			// A/D old + New in v8.2	A/D input chanel scan selection 0=ignored; 1=selected (ch15-0) 
                                    // AN0..AN5 -> OLD   AN8 -> OLD   AN9,AN10 -> NEW 

        AD1CSSH = 0;
        _SMPI = 10;		// Interrupt request. Ogni quante conversione avviene l'interrupt (0-16)

        _AD1IF = 0;			// ripulisco il flag di interrupot dell'ADC
        _ASAM = 1;		// auto start sampling

        // then go to conversion
        while (!_AD1IF){};	// Conversdione finita?
        _ASAM = 0;		// Si, stoppo campinonamente e conversione

        _DONE = 0;
    }

    if(sched==2)    
    {
        Sum_Adc_Temp[1] += ADC1BUF0;		// sommo il valore letto dall'adc nel cumulativo     
        N_Campioni[1]++;						// incremento il cnt1 dei campioni        
        if(N_Campioni[1] > Campioni_Acquisizione[1])		// se ho oltrepassato la soglia dei campioni
        {	
            N_Campioni[1] = Campioni_Acquisizione[1];		// riscalo il numero dei campioni
            Sum_Adc_Temp[1] -= Adc_Temp[1];		// sottraggo il campione medio dalla somma cumulativa
        }                
        if(N_Campioni[1] != 0)
            Adc_Temp[1] = Sum_Adc_Temp[1] / N_Campioni[1];   
    }
	if(sched==4)    
    {
        Sum_Adc_Temp[2] += ADC1BUF1;		// sommo il valore letto dall'adc nel cumulativo     
        N_Campioni[2]++;						// incremento il cnt1 dei campioni        
        if(N_Campioni[2] > Campioni_Acquisizione[2])		// se ho oltrepassato la soglia dei campioni
        {	
            N_Campioni[2] = Campioni_Acquisizione[2];		// riscalo il numero dei campioni
            Sum_Adc_Temp[2] -= Adc_Temp[2];		// sottraggo il campione medio dalla somma cumulativa
        }                
        if(N_Campioni[2] != 0)
            Adc_Temp[2] = Sum_Adc_Temp[2] / N_Campioni[2];   
    }
	if(sched==6)    
    {
        Sum_Adc_Temp[3] += ADC1BUF2;		// sommo il valore letto dall'adc nel cumulativo     
        N_Campioni[3]++;						// incremento il cnt1 dei campioni        
        if(N_Campioni[3] > Campioni_Acquisizione[3])		// se ho oltrepassato la soglia dei campioni
        {	
            N_Campioni[3] = Campioni_Acquisizione[3];		// riscalo il numero dei campioni
            Sum_Adc_Temp[3] -= Adc_Temp[3];		// sottraggo il campione medio dalla somma cumulativa
        }                
        if(N_Campioni[3] != 0)
            Adc_Temp[3] = Sum_Adc_Temp[3] / N_Campioni[3];   
    }        
	if(sched==8)    
    {
        Sum_Adc_Temp[4] += ADC1BUF3;		// sommo il valore letto dall'adc nel cumulativo     
        N_Campioni[4]++;						// incremento il cnt1 dei campioni        
        if(N_Campioni[4] > Campioni_Acquisizione[4])		// se ho oltrepassato la soglia dei campioni
        {	
            N_Campioni[4] = Campioni_Acquisizione[4];		// riscalo il numero dei campioni
            Sum_Adc_Temp[4] -= Adc_Temp[4];		// sottraggo il campione medio dalla somma cumulativa
        }                
        if(N_Campioni[4] != 0)
            Adc_Temp[4] = Sum_Adc_Temp[4] / N_Campioni[4];   
    }        
	if(sched==10)   
    {
        Sum_Adc_Temp[5] += ADC1BUF4;		// sommo il valore letto dall'adc nel cumulativo     
        N_Campioni[5]++;						// incremento il cnt1 dei campioni        
        if(N_Campioni[5] > Campioni_Acquisizione[5])		// se ho oltrepassato la soglia dei campioni
        {	
            N_Campioni[5] = Campioni_Acquisizione[5];		// riscalo il numero dei campioni
            Sum_Adc_Temp[5] -= Adc_Temp[5];		// sottraggo il campione medio dalla somma cumulativa
        }                
        if(N_Campioni[5] != 0)
            Adc_Temp[5] = Sum_Adc_Temp[5] / N_Campioni[5];   
    }        
	if(sched==12)
    {
        Sum_Adc_Temp[6] += ADC1BUF5;		// sommo il valore letto dall'adc nel cumulativo     
        N_Campioni[6]++;						// incremento il cnt1 dei campioni        
        if(N_Campioni[6] > Campioni_Acquisizione[6])		// se ho oltrepassato la soglia dei campioni
        {	
            N_Campioni[6] = Campioni_Acquisizione[6];		// riscalo il numero dei campioni
            Sum_Adc_Temp[6] -= Adc_Temp[6];		// sottraggo il campione medio dalla somma cumulativa
        }                
        if(N_Campioni[6] != 0)
            Adc_Temp[6] = Sum_Adc_Temp[6] / N_Campioni[6];   
    }        
	if(sched==14)    
    {
        Sum_Adc_Temp[7] += ADC1BUF6;		// sommo il valore letto dall'adc nel cumulativo     
        N_Campioni[7]++;						// incremento il cnt1 dei campioni        
        if(N_Campioni[7] > Campioni_Acquisizione[7])		// se ho oltrepassato la soglia dei campioni
        {	
            N_Campioni[7] = Campioni_Acquisizione[7];		// riscalo il numero dei campioni
            Sum_Adc_Temp[7] -= Adc_Temp[7];		// sottraggo il campione medio dalla somma cumulativa
        }                
        if(N_Campioni[7] != 0)
            Adc_Temp[7] = Sum_Adc_Temp[7] / N_Campioni[7];   
    }        
	if(sched==16)    
    {
        Sum_Adc_Temp[8] += ADC1BUF7;		// sommo il valore letto dall'adc nel cumulativo     
        N_Campioni[8]++;						// incremento il cnt1 dei campioni        
        if(N_Campioni[8] > Campioni_Acquisizione[8])		// se ho oltrepassato la soglia dei campioni
        {	
            N_Campioni[8] = Campioni_Acquisizione[8];		// riscalo il numero dei campioni
            Sum_Adc_Temp[8] -= Adc_Temp[8];		// sottraggo il campione medio dalla somma cumulativa
        }                
        if(N_Campioni[8] != 0)
            Adc_Temp[8] = Sum_Adc_Temp[8] / N_Campioni[8];   
    }        
	if(sched==18)
    {
        Sum_Adc_Temp[9] += ADC1BUF8;		// sommo il valore letto dall'adc nel cumulativo     
        N_Campioni[9]++;						// incremento il cnt1 dei campioni        
        if(N_Campioni[9] > Campioni_Acquisizione[9])		// se ho oltrepassato la soglia dei campioni
        {	
            N_Campioni[9] = Campioni_Acquisizione[9];		// riscalo il numero dei campioni
            Sum_Adc_Temp[9] -= Adc_Temp[9];		// sottraggo il campione medio dalla somma cumulativa
        }                
        if(N_Campioni[9] != 0)
            Adc_Temp[9] = Sum_Adc_Temp[9] / N_Campioni[9];   
    }	

	if(sched==20)
    {
        Sum_Adc_Temp[10] += ADC1BUF9;		// sommo il valore letto dall'adc nel cumulativo     
        N_Campioni[10]++;						// incremento il cnt1 dei campioni        
        if(N_Campioni[10] > Campioni_Acquisizione[10])		// se ho oltrepassato la soglia dei campioni
        {	
            N_Campioni[10] = Campioni_Acquisizione[10];		// riscalo il numero dei campioni
            Sum_Adc_Temp[10] -= Adc_Temp[10];		// sottraggo il campione medio dalla somma cumulativa
        }                
        if(N_Campioni[10] != 0)
            Adc_Temp[10] = Sum_Adc_Temp[10] / N_Campioni[10];   
    }	
    
    
    sched++;
    if(sched>=22)
        sched=0;
};

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------
void AD_Read(void)
{
    int x;
    
	//AD1CON2bits.CSCNA = 1;		// Scan/input 0=> use the chanel selectet by CH0SA
								//            1=> Scan input selected in AD1CSSL reg
	//AD1CHSbits.CH0NA = 0;		// selected negative input for mux A to AGnd
	//AD1CHSbits.CH0SA = 0;		// selected input for mux A
/*
    AD1CSSL = 0x0FFF; //0x073F;			// A/D old + New in v8.2	A/D input chanel scan selection 0=ignored; 1=selected (ch15-0) 
*/	
	//AD1CSSH = 0;
	//AD1CON2bits.SMPI = 9;		// Interrupt request. Ogni quante conversione avviene l'interrupt (0-16)

	//IFS0bits.AD1IF = 0;			// ripulisco il flag di interrupot dell'ADC
	
    // Riabilitare se NON si usano le conversioni sotto interrupt  
    // AD1CON1bits.ASAM = 1;		// auto start sampling
    
	// then go to conversion
	// Riabilitare se NON si usano le conversioni sotto interrupt  
    //while (!IFS0bits.AD1IF){};	// Conversdione finita?
    
    
	_ASAM = 0;          // Stoppo campinonamento e conversione

	Sum_Adc_Temp[1] += ADC1BUF0;	// Sum_Adc_T_Gas	// sommo il valore letto dall'adc nel cumulativo
	Sum_Adc_Temp[2] += ADC1BUF1;    // Sum_Adc_T_Liquid
	Sum_Adc_Temp[3] += ADC1BUF2;    // Sum_Adc_T_Out
	Sum_Adc_Temp[4] += ADC1BUF3;    // Sum_Adc_T_Amb
	Sum_Adc_Temp[5] += ADC1BUF4;    // Sum_Adc_P_Grande
	Sum_Adc_Temp[6] += ADC1BUF5;    // Sum_Adc_P_Piccolo

	Sum_Adc_Temp[7] += ADC1BUF6;    // Sum_Adc_I_Fan
	Sum_Adc_Temp[8] += ADC1BUF7;    // Sum_Adc_Aux1
	Sum_Adc_Temp[9] += ADC1BUF8;    // Sum_Adc_Aux2
	Sum_Adc_Temp[10] += ADC1BUF9;   // Sum_Adc_Supply
	

    for(x=1; x<=10; x++)
    {
        N_Campioni[x]++;						// incremento il cnt1 dei campioni        
        
        if(N_Campioni[x] > Campioni_Acquisizione[x])		// se ho oltrepassato la soglia dei campioni
        {	
            N_Campioni[x] = Campioni_Acquisizione[x];		// riscalo il numero dei campioni
            Sum_Adc_Temp[x] -= Adc_Temp[x];		// sottraggo il campione medio dalla somma cumulativa
        }    
        
        if(N_Campioni[x] != 0)
            Adc_Temp[x] = Sum_Adc_Temp[x] / N_Campioni[x];          
    }    
    
    /*
	Sum_Adc_T_Gas += ADC1BUF0;		// sommo il valore letto dall'adc nel cumulativo
	Sum_Adc_T_Liquid += ADC1BUF1;
	Sum_Adc_T_Out += ADC1BUF2;
	Sum_Adc_T_Amb += ADC1BUF3;
	Sum_Adc_P_Grande += ADC1BUF4;
	Sum_Adc_P_Piccolo += ADC1BUF5;

    
	Sum_Adc_I_Fan += ADC1BUF6;
	Sum_Adc_Aux1 += ADC1BUF7;
	Sum_Adc_Aux2 += ADC1BUF8;

	Sum_Adc_Supply += ADC1BUF9;
	
	N_Campioni +=1;						// incremento il cnt dei campioni

	if(N_Campioni > Campioni_Acquisizione)		// se ho oltrepassato la soglia dei campioni
	{	N_Campioni = Campioni_Acquisizione;		// riscalo il numero dei campioni
		Sum_Adc_T_Gas -= Adc_T_Gas;		// sottraggo il campione medio dalla somma cumulativa
		Sum_Adc_T_Liquid -= Adc_T_Liquid;
		Sum_Adc_T_Out -= Adc_T_Out;
		Sum_Adc_T_Amb -= Adc_T_Amb;
		Sum_Adc_P_Grande -= Adc_P_Grande;
		Sum_Adc_P_Piccolo -= Adc_P_Piccolo;
        Sum_Adc_I_Fan -= Adc_I_Fan;
		Sum_Adc_Aux1 -= Adc_Aux1;
		Sum_Adc_Aux2 -= Adc_Aux2;		 
        Sum_Adc_Supply -= Adc_Supply;
	}

	if(N_Campioni != 0)
	{	Adc_T_Gas = Sum_Adc_T_Gas /N_Campioni;
		Adc_T_Liquid = Sum_Adc_T_Liquid/N_Campioni;
		Adc_T_Out = Sum_Adc_T_Out/N_Campioni;
		Adc_T_Amb = Sum_Adc_T_Amb/N_Campioni;
		Adc_P_Grande = Sum_Adc_P_Grande/N_Campioni;    
		Adc_P_Piccolo = Sum_Adc_P_Piccolo/N_Campioni; 
		Adc_I_Fan = Sum_Adc_I_Fan/N_Campioni;                 
		Adc_Aux1 = Sum_Adc_Aux1/N_Campioni;
		Adc_Aux2 = Sum_Adc_Aux2/N_Campioni;        
        Adc_Supply = Sum_Adc_Supply/N_Campioni;
	}		
    */
	_DONE = 0;
};

int AD_ReadSingle(char Chanel)
{
	int Temp = 0;

	AD1CON2bits.CSCNA = 0;		// Scan/input 0=> use the chanel selectet by CH0SA
								//            1=> Scan input selected in AD1CSSL reg
	AD1CHSbits.CH0SA = Chanel;	// selected input for mux A
	AD1CON2bits.SMPI = 0;		// Interrupt request. Ogni quante conversione avviene l'interrupt (0-16)

	IFS0bits.AD1IF = 0;			// ripulisco il flag di interrupot dell'ADC
	AD1CON1bits.ASAM = 1;		// auto start sampling

	// then go to conversion
	while (!IFS0bits.AD1IF){};	// Conversdione finita?
	AD1CON1bits.ASAM = 0;		// Si, stoppo campinonamente e conversione
	Temp = ADC1BUF0;

	return Temp;				// restituisco la conversione
};

int Temp_Read(volatile int AD_Value, volatile int* AD_T, volatile int* Kt)
/*
	Ritorno la temperatura in centesimi di grado centigrado (signed int -32768 +32767):
	AD_Value è il valore in bit della lettura
	AD_T è un array contenente i valori notevoli delle conversioni (@ -20, -10, ecc..)
	Kt è un array contenente i coefficenti per la conversione delle temperature
	ex: -10,00	=> -1000
		-20,00	=> -2000
		+15,52	=> +1552
*/
{
	if(AD_Value < AD_T[1])								// < -20°C
		return ((AD_Value-AD_T[0]) *Kt[0]) TempOffSet_1;	

	if(AD_Value < AD_T[2])								// < -10°C
		return ((AD_Value-AD_T[1]) *Kt[1]) TempOffSet_2;

	if(AD_Value < AD_T[3])								// < 0°C
		return ((AD_Value-AD_T[2]) *Kt[2]) TempOffSet_3;

	if(AD_Value < AD_T[4])								// < 10°C
		return ((AD_Value-AD_T[3]) *Kt[3]) TempOffSet_4;

	if(AD_Value < AD_T[5])								// < 20°C
		return ((AD_Value-AD_T[4]) *Kt[4]) TempOffSet_5;

	if(AD_Value < AD_T[6])								// < 25°C
		return ((AD_Value-AD_T[5]) *Kt[5]) TempOffSet_6;

	if(AD_Value < AD_T[7])								// < 30°C
		return ((AD_Value-AD_T[6]) *Kt[6]) TempOffSet_7;

	if(AD_Value < AD_T[8])								// < 40°C
		return ((AD_Value-AD_T[7]) *Kt[7]) TempOffSet_8;

	if(AD_Value < AD_T[9])								// < 50°C
		return ((AD_Value-AD_T[8]) *Kt[8]) TempOffSet_9;

	if(AD_Value < AD_T[10])								// < 60 °C
		return ((AD_Value-AD_T[9]) *Kt[9]) TempOffSet_10;

	return ((AD_Value-AD_T[10]) *Kt[10]) TempOffSet_11;
}

int T_Amb_Read(volatile int AD_Value, volatile int* AD_T, volatile int* Kt)
/*
	Ritorno la temperatura in centesimi di grado centigrado (signed int -32768 +32767):
	AD_Value è il valore in bit della lettura
	AD_T è un array contenente i valori notevoli delle conversioni (@ 10, 13, ecc..)
	Kt è un array contenente i coefficenti per la conversione delle temperature
	ex: 10,00	=> 1000
		20,00	=> 2000
		25,52	=> 2552
*/
{
	if(AD_Value < AD_T[1])								// < 13°C
		return ((AD_Value-AD_T[0]) *Kt[0]) TempA_OffSet_1;	

	if(AD_Value < AD_T[2])								// < 16°C
		return ((AD_Value-AD_T[1]) *Kt[1]) TempA_OffSet_2;

	if(AD_Value < AD_T[3])								// < 19°C
		return ((AD_Value-AD_T[2]) *Kt[2]) TempA_OffSet_3;

	if(AD_Value < AD_T[4])								// < 21°C
		return ((AD_Value-AD_T[3]) *Kt[3]) TempA_OffSet_4;

	if(AD_Value < AD_T[5])								// < 24°C
		return ((AD_Value-AD_T[4]) *Kt[4]) TempA_OffSet_5;

	if(AD_Value < AD_T[6])								// < 27°C
		return ((AD_Value-AD_T[5]) *Kt[5]) TempA_OffSet_6;

	if(AD_Value < AD_T[7])								// < 30°C
		return ((AD_Value-AD_T[6]) *Kt[6]) TempA_OffSet_7;

	if(AD_Value < AD_T[8])								// < 33°C
		return ((AD_Value-AD_T[7]) *Kt[7]) TempA_OffSet_8;

	if(AD_Value < AD_T[9])								// < 36°C
		return ((AD_Value-AD_T[8]) *Kt[8]) TempA_OffSet_9;

	if(AD_Value < AD_T[10])								// < 40 °C
		return ((AD_Value-AD_T[9]) *Kt[9]) TempA_OffSet_10;

	return ((AD_Value-AD_T[10]) *Kt[10]) TempA_OffSet_11;
}

unsigned int Pressure_Read_45(int AD_Value)
{
	// la sonda di pressione 0-45bar ha un segnale d'uscita utile da 0.5 a 4.5.
	// Con il nuovo HW v8.2 il range di uscita al micro è:
	//	Bar		Vout	Bit
	//	0		0.5v	0
	//	45		4.5v	1023
	// da cui si ricava il fattore K di conversione lineare 43.988 mBar/bit 
	if(AD_Value > 50)		// Soglia anti "sfarfallio" a vuoto senza sonda collegata
		return (unsigned int)((float)AD_Value*43.988);
	else
		return 0;	
}

unsigned int Pressure_Read_34(int AD_Value)						// New HW v8.2
{
	// la sonda di pressione 0-45bar ha un segnale d'uscita utile da 0.5 a 4.5.
	// Con il nuovo HW v8.2 il range di uscita al micro è:
	//	Bar		Vout	Bit
	//	0		0.5v	0
	//	34		4.5v	1023
	// da cui si ricava il fattore K di conversione lineare 33.236 mBar/bit 
	if(AD_Value > 50)		// Soglia anti "sfarfallio" a vuoto senza sonda collegata
		return (unsigned int)((float)AD_Value*33.236);
	else
		return 0;		
}


//----------------------------------------------------------------------------------
// Steinhart-Hart
// La formula di linearizzazione è: 
// Temperatura (in Kelvin) = 1 / [ a + (b * lnR) + (c * (lnR³)) ] - dComp 
// Temperatura (in Celsius) = 1 / [ a + (b * lnR) + (c * (lnR³)) ] - dComp - 273.15
//
// I parametri A,B,C sono stati calcolati con il "Thermistor calculator" a questo link:
// http://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm
// I valori usati per il calcolo sono stati prelevati dal datasheet del produttore
// del sensore PTC "AMWEI COP5X25LPTC1000F1500" considerando il range utilizzato -30..70°C
// "AMWEI Nickel Plating Copper Tube Linear PTC Thermistor Temperature Sensor Probe COP5X25LPTC1000F1500 Data Sheet.pdf"
// che è conosciuta sul mercato con la sigla "KTY81-110"
// Per i valori di Gain e Offset riferirsi al DOC "Curva PTC KTY81-110.xls"
// Valore R @ 100°C ->	1696 Ohm (MAX)
// Valore R @ 70°C ->	1392 Ohm (MAX)
// Valore R @ 20°C -> 	961 Ohm (MIDDLE)
// Valore R @ -30°C -> 	624 Ohm (MIN)
//----------------------------------------------------------------------------------
	double R1 = 2210.0; 						// resistenza in serie alla PTC
	double Vref = 2.5;							// Vref di alimentazione PTC
	double VoMAX = 2.5;							// Tensione di uscita MAX OP
	double ADres = 1023.0;						// Risoluzione A/D 10bit

	double Gain;
	double Gain1 = 6.014368873-0.1;		//4.0931935; //4.113547;			// Range -30 +70°C (Effettivo: -31 +70.5)
	//double Gain2 = 14.0; //13.974521;				// Range +10 +42°C (+10 +41.5)
	double Gain2 = 11.88865641; //11.85770308+0.200; //0.195;		//11.888656; //13.974521;			// Range -10 +40°C (Effettivo: -9 +41.2)
	
	double Offset;
	double Offset1 = 0.546322-0.0028;		// 0.550458716;	//0.586025-0.04;		//0.546712; // MAX=1.15781 -> Delta = 0.611098 -> G=4.0909968				// Offset per range -30 +70°C
	//double Offset2 = 0.793054+0.001;	//0.792301; // MAX=0.971198 -> Delta = 0.178897 -> G=13.974521				// Offset per range +10 +42°C
	double Offset2 = 0.63155225; //0.635965-0.0004; //0.00025;		// 0.631552249;	// MAX=0.841836735 -> Delta = 0.210284486 -> G=11.888656				// Offset per range -10 +40°C
	double Vout;
	double Vptc;
	double Rth;
	double logR;
	double logR3;
	double kelvin;
	double celsius;
	double dComp;
	double A;
	double B;
	double C;
									// Coefficienti per modello Steinhart-Hart x range -30 .. +70 °C
	double A1 = 24.66335875e-3;	 	// x range -30 .. +70 °C	R:624 -> -30°C
	double B1 = -39.04130450e-4;		// x range -30 .. +70 °C	R:961 -> 20°C
	double C1 = 171.6693777e-7;		// x range -30 .. +70 °C	R:1392 -> 70°C
/*
									// Coefficienti per modello Steinhart-Hart x range 10 .. +42 °C	
									// Nota: Usato range 0..50°C x calcolo parametri
	double A2 = 24.01859610e-3;	 	// x range 10 .. +42 °C		R:815 -> 0°C	
	double B2 = -37.71246222e-4;	// x range 10 .. +42 °C		R:1000 -> 25°C	
	double C2 = 163.4093844e-7;		// x range 10 .. +42 °C		R:1209 -> 50°C	
*/	
									// Coefficienti per modello Steinhart-Hart x range -10 .. +40 °C	
									// Nota: Usato range -10..40°C x calcolo parametri
	double A2 = 24.09262233e-3;	 	// x range -10 .. +40 °C		R:747 -> -10°C	
	double B2 = -37.87201711e-4;	// x range -10 .. +40 °C		R:961 -> 20°C	
	double C2 = 164.4978118e-7;		// x range -10 .. +40 °C		R:1122 -> 40°C		

	double K = 3.75; 				// dissipation factor (AMWEY -> 2.5..5 mW/C)

int Steinhart(int AD_Value, int Range)
{


	switch(Range)
	{
		case 1:
				Gain = Gain1;
				Offset = Offset1;
				A = A1;
				B = B1;
				C = C1;
				break;
		case 2:
				Gain = Gain2;
				Offset = Offset2;
				A = A2;
				B = B2;
				C = C2;				
				break;
	}

	Vout =  (double)(AD_Value) / ADres * VoMAX;
	Vptc = (Vout / Gain) + Offset;
	Rth = (R1 * Vptc ) / (Vref - Vptc); //Vptc / (Vref - Vptc / R1);    //(R1 * Vptc ) / (Vin - Vptc);
	logR  = log(Rth);
	logR3 = logR * logR * logR;

	dComp = (Vptc*Vptc)/(K * Rth);				// Compensazione dissipazione
	kelvin = (1.0 / (A + B * logR + C * logR3 )) - dComp;
	celsius = kelvin - 273.15;

	return (int)(celsius * 100);
}


//-----------------------------------------------------------------------------------------
// Scala ingresso:     0..32V -> 0..2.5v	->  0..1023
// Scala partitore resistivo su 24V:  24V / (2K87+330R) * 330R = 2.475 -> 9.696969697
// Lettura minima: < 15V (fino a che sopravvive il micro...)  
// Lettura massima: 32V MAX (saturazione canale di conversione) -> 1023 - 0.031280547V per bit
// Fattore di correzione per la caduta del diodo antiinversione: 0.967
// 
// Scala partitore resistivo su 32V:  32V / (3K94+330R) * 330R = 2.473V
// Scala partitore resistivo su 32V:  32V / (2K87+243R) * 243R = 2.498V
// Correzione valore R101 (R53 su Slave) = 330R -> 243R     330R//      ->243R
//-----------------------------------------------------------------------------------------
unsigned int PowerSupplyConversion(int AD_Value)
{
    #define K_PowerSupplyConversion 2.43        // Costante di conversione in centesimi di Volt
                                                // Pratico 2.43(con R=330R MAX 24.9Vin) //Teorico 2.36973844(con R=330R MAX 24.9Vin) 
                                                // Pratico 3.16(con R=243R MAX 32.7Vin) //Teorico 3.1280547(con R=243R MAX 32.7Vin) 
                                                // Pratico 3.071(con R=330R//1K=248R MAX 31.42Vin) //Teorico 2.36973844(con R=330R//1K=248R MAX 31.42Vin) 

    #define K_DiodeCompensation     70.0        // 0.7V x 100

    return (unsigned int)(((float)AD_Value * K_PowerSupplyConversion)+K_DiodeCompensation);
}


//-----------------------------------------------------------------------------------------
// Scala ingresso:      0,5V..4,5v    ->  0..1023
//                      K = 1023/255 -> 4.011765
// Scala uscita:        0..255
//-----------------------------------------------------------------------------------------
unsigned int Pot_Read(int AD_Value, int ValMin, int ValMax)
{
    //#define K_PotConversion         4.011765    // Costante di conversione in centesimi di Volt
    if(AD_Value>5)
        return (int)(ValueIn2ValueOut((float)AD_Value, 5.0, 1023.0, (float)ValMin, (float)ValMax)); 
    else
        return 0;
    //return (unsigned int)((float)AD_Value / K_PotConversion);    
}

//-----------------------------------------------------------------------------------------
//
//  Scala 0..500Pa -> 4..20mA su 125Ohm -> 0.5V - 2.5V
//
//  0.5-4.5V -> 0..1023 
//  0.5-2.5V -> 0..512 
//
//  
//
//-----------------------------------------------------------------------------------------
int DifferentialPressureConversion(int AD_Value)
{    
    
#if(K_DiffPress_SensorSelection==0)    //Premasgard 2121SD
#ifndef K_Pressure_Max_Value
    #define K_Pressure_Max_Value    500          //Fondo scala pressione differenziale    
#endif
    #define K_MAX_PressureRes       512
    #define K_PressureConversion    (float)K_Pressure_Max_Value/(float)K_MAX_PressureRes  // -> 0.9765625   

    return (int)((float)AD_Value * K_PressureConversion);     //K_PressureConversion);
    
#elif(K_DiffPress_SensorSelection==1)    //E+E ELEKTRONIK Mod. EE610-HV51A7
    //E+E ELEKTRONIK Mod. EE610-HV51A7
    // Configurazione del DIP-Switch 8 posizioni:   S1=ON  ,S2=ON  ,    0-100 Pa        measurement range
    //                                              S1=OFF ,S2=OFF ,    -/+100 Pa       measurement range    
    //                                              S3=OFF ,S4=OFF ,    50 mS           response time
    //                                              S5=OFF ,S6=OFF ,    Pa              display unit
    //                                              S7= OFF ,           backlight OFF   backlight
    //                                              S8= OFF ,           4-20mA          output signal -> morsetto 3(GND) e 5(IOUT)
    //
    #define K_Pressure_Pol_Value    0    //Polarità segnmale uscita                 0= 0..100Pa  1= +/-100Pa    
    #define K_Pressure_Max_Value    100  //Fondo scala pressione differenziale      0..100Pa  o  +/-100Pa (vedi "K_Pressure_Pol_Value)
    #define K_MAX_PressureRes       512
#if(K_Pressure_Pol_Value==0)
    #define K_PressureOffs          0.0
    #define K_PressureConversion    (float)K_Pressure_Max_Value/(float)K_MAX_PressureRes  // -> 0.1953125 @ 100Pa
#else
    #define K_PressureOffs          (float)K_Pressure_Max_Value
    #define K_PressureConversion    (float)K_Pressure_Max_Value*2.0/(float)K_MAX_PressureRes  // -> 0.390625 @ +/-100Pa
#endif
    return (int)( ((float)AD_Value * K_PressureConversion)-K_PressureOffs);     //K_PressureConversion);
#endif
    
}

//-----------------------------------------------------------------------------------------
//
//  Scala 0..100% ->  1.0V - 3.6V
//
//  0.5-4.5V -> 0..1023 
//  1.0-3.6V -> 128..793 
//
//  
//
//-----------------------------------------------------------------------------------------
unsigned int HumidityConversion(int AD_Value)
{    
    unsigned int RetVal;
    #define K_MIN_Humidity          128.0    
    #define K_MAX_Humidity          793.0
    #define K_Offs_Humidity         19.0
    #define K_HumidityConversion    (float)K_Humidity_Max_Value/(K_MAX_Humidity-K_MIN_Humidity)  
    
    RetVal = (unsigned int)(((float)AD_Value * K_HumidityConversion)-K_Offs_Humidity);    
    if(RetVal>K_Humidity_Max_Value)
        RetVal=K_Humidity_Max_Value;
    return RetVal;
}