//----------------------------------------------------------------------------------
//	Progect name:	ADC.c
//	Device:			PIC24FJ256GB110 @ 32Mhz (FRCPLL)
//	Autor:			Emanuele
//	Date:			04/01/2014
//	Description:	Corpo delle funzioni pilota per l'acquisizione analogico digitale
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//	Include
//----------------------------------------------------------------------------------
#include "HWSelection.h"
#if (DA210==1)
	#include <p24FJ256DA210.h>
#else
	#include <p24FJ256GB210.h>
#endif

#include <stdio.h>
#include <math.h>
#include "DefinePeriferiche.h"
#include "ADC.h"

//----------------------------------------------------------------------------------
//	Funzioni
//----------------------------------------------------------------------------------

void AD_Read_Sched(void)
{
    static int sched=0;
    
    if(sched==0)        // Lancia le conversioni (9))
    {
        _CSCNA = 1;		// Scan/input 0=> use the chanel selectet by CH0SA
                                    //            1=> Scan input selected in AD1CSSL reg
        _CH0NA = 0;		// selected negative input for mux A to AGnd
        _CH0SA = 0;		// selected input for mux A
        AD1CSSL = 0x073F; //0x0FFF;			// A/D old + New in v8.2	A/D input chanel scan selection 0=ignored; 1=selected (ch15-0) 
                                    // AN0..AN5 -> OLD   AN8 -> OLD   AN9,AN10 -> NEW 

        AD1CSSH = 0;
        _SMPI = 9;		// Interrupt request. Ogni quante conversione avviene l'interrupt (0-16)

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

    sched++;
    if(sched>=20)
        sched=0;
};

void AD_Read(void)
{
    int x;
    
	_CSCNA = 1;		// Scan/input 0=> use the chanel selectet by CH0SA
								//            1=> Scan input selected in AD1CSSL reg
	_CH0NA = 0;		// selected negative input for mux A to AGnd
	_CH0SA = 0;		// selected input for mux A
    AD1CSSL = 0x073F; //0x0FFF;			// A/D old + New in v8.2	A/D input chanel scan selection 0=ignored; 1=selected (ch15-0) 
								// AN0..AN5 -> OLD   AN8 -> OLD   AN9,AN10 -> NEW 
	
	AD1CSSH = 0;
	_SMPI = 9;		// Interrupt request. Ogni quante conversione avviene l'interrupt (0-16)

	_AD1IF = 0;			// ripulisco il flag di interrupot dell'ADC
	_ASAM = 1;		// auto start sampling

	// then go to conversion
	while (!_AD1IF){};	// Conversdione finita?
	_ASAM = 0;		// Si, stoppo campinonamente e conversione

	Sum_Adc_Temp[1] += ADC1BUF0;		// sommo il valore letto dall'adc nel cumulativo
	Sum_Adc_Temp[2] += ADC1BUF1;
	Sum_Adc_Temp[3] += ADC1BUF2;
	Sum_Adc_Temp[4] += ADC1BUF3;
	Sum_Adc_Temp[5] += ADC1BUF4;
	Sum_Adc_Temp[6] += ADC1BUF5;

	Sum_Adc_Temp[7] += ADC1BUF6;
	Sum_Adc_Temp[8] += ADC1BUF7;
	Sum_Adc_Temp[9] += ADC1BUF8;
	

    for(x=1; x<=9; x++)
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
    
	_DONE = 0;
};

int AD_ReadSingle(char Chanel)
{
	int Temp = 0;

	_CSCNA = 0;		// Scan/input 0=> use the chanel selectet by CH0SA
								//            1=> Scan input selected in AD1CSSL reg
	_CH0SA = Chanel;	// selected input for mux A
	_SMPI = 0;		// Interrupt request. Ogni quante conversione avviene l'interrupt (0-16)

	_AD1IF = 0;			// ripulisco il flag di interrupot dell'ADC
	_ASAM = 1;		// auto start sampling

	// then go to conversion
	while (!_AD1IF){};	// Conversdione finita?
	_ASAM = 0;		// Si, stoppo campinonamente e conversione
	Temp = ADC1BUF0;

	return Temp;				// restituisco la conversione
};


unsigned int Pressure_Read(int AD_Value)
{
	// la sonda di pressione 0-34.5Bar o 0-45Bar ha un segnale d'uscita utile da 0.5 a 4.5.
	//	Bar         Vout	
	//	0           0.5v	
	//	34.5 o 45	4.5v	
    //
    // Con l'HW Rev.4.4 o sup. il condizionamento segnali sulle pressioni consente:
    // 0..5V -> 0..45Bar
    // oppure
    // 0..5V -> 0..34.5Bar
    //
    // Quindi K = 43.98827 (1023 x 43.98827 = 45000 mBar)
    // Quindi K = 33.72434 (1023 x 33.72434 = 34500 mBar)

#if(K_PressSensorSelect==K_34Bar) 
        return (unsigned int)((float)AD_Value*33.72434);
#endif      
        
#if(K_PressSensorSelect==K_45Bar) 
        return (unsigned int)((float)AD_Value*43.98827);
#endif              
        return 0;   // Se sbaglio valore di selezione ritorno zero per sicurezza.....
}

/*
unsigned int Pressure_Read(int AD_Value)
{
	// la sonda di pressione 0-45bar ha un segnale d'uscita utile da 0.5 a 4.5.
	//	Bar		Vout	Bit
	//	0		0.5v	102
	//	45		4.5v	921
	// da cui si ricava il fattore K di conversione lineare 55mBar/bit (originale 54.945, errore con il nuovo 45.045, inferiore ad 1 bit)
	//	K ==> 55mBar/bit
    //
    // Con l'HW Rev.4.4 o sup. il condizionamento segnali sulle pressioni consente:
    // 0..5V -> 0..45Bar
    // Quindi K = 44 (1023 x 44 = 45012mBar)
    //return AD_Value * 44; //CONTO SU MASTER
	return (unsigned int)((float)AD_Value*43.988);
}
*/

/*
unsigned int Pressure_Read_45(int AD_Value)
 * 
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
*/


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
	double Gain2 = 6.068612899; // Range 0..100°C con HW Maste v5.0                 //11.88865641; //11.85770308+0.200; //0.195;		//11.888656; //13.974521;			// Range -10 +40°C (Effettivo: -9 +41.2)
	
	double Offset;
	double Offset1 = 0.546322-0.0028;		// 0.550458716;	//0.586025-0.04;		//0.546712; // MAX=1.15781 -> Delta = 0.611098 -> G=4.0909968				// Offset per range -30 +70°C
	//double Offset2 = 0.793054+0.001;	//0.792301; // MAX=0.971198 -> Delta = 0.178897 -> G=13.974521				// Offset per range +10 +42°C
	double Offset2 = 0.673553719; //0.63155225; //0.635965-0.0004; //0.00025;		// 0.631552249;	// MAX=0.841836735 -> Delta = 0.210284486 -> G=11.888656				// Offset per range -10 +40°C
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
    
/*    
									// Coefficienti per modello Steinhart-Hart x range -10 .. +40 °C	
									// Nota: Usato range -10..40°C x calcolo parametri
	double A2 = 24.09262233e-3;	 	// x range -10 .. +40 °C		R:747 -> -10°C	
	double B2 = -37.87201711e-4;	// x range -10 .. +40 °C		R:961 -> 20°C	
	double C2 = 164.4978118e-7;		// x range -10 .. +40 °C		R:1122 -> 40°C		
*/    
    
                                    // ATTENZIONE QUESTI COEFFICIENTI (0..100°C) SONO STATI PRESI DAL MASTER 4.0 @ 5V
                                    // VERIFICARE CHE SIANO OK !!!!!!!
/**/    
									// Coefficienti per modello Steinhart-Hart x range 0 .. +100 °C	
									// Nota: valori ottenuti "taroccando" i parametri di R inseriti a xx°C 
									// con +1°C per compensare differenza di offset in HW per via dei valori di R
									// approssimati per valore commerciale al posto di quelli "ideali" da calcolo.
	
    // Originali da calcolo
	double A2 = 22.20565747e-3; //22.32236848e-3;	 	// x range 0 .. +100 °C		R:815 -> 0°C	(+7 Ohm = 822) 
	double B2 = -33.76720759e-4; //-33.95256028e-4;	// x range 0 .. +100 °C		R:1209 -> 50°C	(+9 Ohm = 1218)
	double C2 = 135.7978192e-7; //136.4889585e-7;		// x range 0 .. +100 °C		R:1696 -> 100°C	(+11 Ohm = 1707)    
/*
    // Ritoccati
	double A2 = 22.32236848e-3;	 	// x range 0 .. +100 °C		R:815 -> 0°C	(+7 Ohm = 822) 
	double B2 = -33.95256028e-4;	// x range 0 .. +100 °C		R:1209 -> 50°C	(+9 Ohm = 1218)
	double C2 = 136.4889585e-7;		// x range 0 .. +100 °C		R:1696 -> 100°C	(+11 Ohm = 1707)    
 */   

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

