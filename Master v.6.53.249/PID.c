#include "FWSelection.h"
#include "PID.h"
#include "ProtocolloComunicazione.h"


/*********************************************************************************************************************

		PPPP	IIIII	DDDD
		P   P	  I		D   D
		PPPP	  I		D   D
		P		  I 	D   D
		P		IIIII	DDDD						 

**********************************************************************************************************************
										 ___  
										|	| 
									.-->|Pp |---------. 
									|	|___|		  |	
									|				+ |	
							 ___	|	 ___		 _|_	
						    /   \	|	|	|	    /   \	
		----SetPoint------>| SUM |--|-->|Pi |----->| SUM |------PVal----->Process (Valve step)
						+	\___/	|	|___|	+	\___/					 |
							  ^		|				  ^						 |
							- | 	|	 ___  		+ | 					 |
							  |		|	|	|		  |						 |
							  |		'-->|Pd |---------'					 	 |
							  |			|___|							.---\_/---.
							  |											| Process |
							  '-------RetVal----------------------------|  Effect |	
																		'---------'


PID => controllo Proporzionale-Integrale-Derivativo:

k = campione ingresso attuale
k-1 = campione ingresso precedente
Pp = parte proporzionale
Pi = parte integrale
Pd = parte derivativa
Kp = Guadagno parte proporzionale
Ki = Guadagno parte integrale
Kd = Guadagno parte derivativa
SetP = SetPoint							(Calcolo superheat??? = Temperatura aria uscita - temperatura convertita da pressione)
RetVal = Valore Retroazione di Processo (??? Cosa utilizzare)
Err = errore 
PVal = uscita PID						(Pilotaggio Valvola espansione)


Err = SetP - RetVal;

Pp(k) = Kp * Err(k)						=> Uscita Parte proporzionale = Guadagno proporzionale * Errore attuale
Pi(k) = Ki * Err(k) + Pi(k-1)			=> Uscita Parte integrale = Guadagno integrale * Errore attuale + Parte integrale precedente
Pd(k) = Kd * [Err(k) - Err(k-1)]		=> Uscita Parte derivativa = Guadagno derivativo * (Errore attuale - Errore precedente)

PVal(k) = Pp(k) + Pi(k) + Pd(k)			=> Uscita PID = Parte proporzionale + Parte integrale + Parte derivativa


Metodo Ziegler?Nichols per la determinazione dei parametri PID:

Iniziare la taratura impostando Ki e Kd = 0
Aumentare gradualmente Kp (partendo da un valore relativamente basso es. 0.1).
Simulare un transitorio a gradino in ingresso (relativamente sostenuto es. MinIn..MaxIn).
Aumentare il valore di Kp fino ad ottenere un'oscillazione dell'uscita sostenuta ma non instabile (non devono sparire dopo un transitorio).
Una volta deterninato il valore di Kp, ricavare i valori di Ki e Kd dalla seguente tabella (Ziegler?Nichols).

Tipo		Kp				Ki					Kd
P			0.50 * Ku		-					-
PI			0.45 * Ku		1.2 * Kp / Pu		-
PID			0.60 * Ku		2 * Kp / Pu			Kp * Pu / 8

Nota:
Ku = è il valore del guadagno per cui la variabile controllata presenta oscillazioni sostenute.
Pu = è il periodo critico delle oscillazioni sostenute;

                | Pu  |
        _       |     |
       | |      _     _     _
      |   |    | |   | |   | |   _   ______________
     |     |__|   |_|   |_|   |_| |_|
    |
   |		
  |	
 |
----------------------------------------------------------

**********************************************************************************************************************/
//---------------------------------------------------------------------------------------------
// Trasforma l'uscita del PID nella scala e con la direzione corretta per la gestione della 
// valvola di espansione
//---------------------------------------------------------------------------------------------
int PIDOut2Valve(int OutPID, int ValMinIn, int ValMaxIn, int ValMinOut, int ValMaxOut)
{
	float div;
	
	div = (float)(ValMaxIn-ValMinIn) / (float)(ValMaxOut-ValMinOut);
  
	return ValMaxOut-((int)((float)OutPID / div));

}

//---------------------------------------------------------------------------------------------
// Calcola l'uscita del PID partendo dal valore di regoalzione della valvola
//---------------------------------------------------------------------------------------------
int Valve2PIDOut(int OutValve, int ValMinIn, int ValMaxIn, int ValMinOut, int ValMaxOut)
{
	float mul;
	
	mul = (float)(ValMaxIn-ValMinIn) / (float)(ValMaxOut-ValMinOut);
  
	return ValMaxIn-((int)((float)OutValve * mul));

}

//---------------------------------------------------------------------------------------------
// Imposta il valore di uscita iniziale del PID per avere un pilotaggio valvola di partenza
// Valore in ingresso: valore posizione valvola
//--------------------------------------------------------------------------------------------- 
void PID_SetOut(volatile TypPID * pid, int SetValveOut) 
{ 
	pid->iState = SetValveOut;	// Inizializza la memoria integrale PID con il valore di partenza 
}

void PID_init(volatile TypPID * pid, float p_gain, float i_gain, float d_gain, float i_Min, float i_Max, int Po_Min, int Po_Max, int dead_zone) 
{ 
	pid->iMax = i_Max;				// Maximum and minimum allowable integrator state
	pid->iMin = i_Min;  		
	pid->PoMax = Po_Max;			// Maximum and minimum allowable integrator state
	pid->PoMin = Po_Min;  

	pid->pGain = p_gain;    		// proportional gain
	pid->iGain = i_gain;			// integral gain
	pid->dGain = d_gain;     		// derivative gain
	pid->iState = 0;				// reset integral memory

	pid->Error = 0;
	pid->LastError = 0;
    pid->saturation_flag_Up = 0;            // Flag per gestione "anti-wind-up"
    pid->saturation_flag_Dw = 0;            // Flag per gestione "anti-wind-up"
    pid->OldiState = 0; 
  
	pid->DeadZone = dead_zone;		// Dead zone
}      


void UpdatePID(volatile TypPID * pid, int Direction_Positive)
{
    float RealKf_PoMax;

    if(EngineBox[0].TestAllAbil.MaxOpValve_Heat)
        RealKf_PoMax = (float)Room[1].TestAll_MaxOpValve_Heat * 7680.0 / 100.0;
    else
        RealKf_PoMax = Kf_PoMax;

    pid->iMax = RealKf_PoMax;				// Maximum and minimum allowable integrator state    

    if(Direction_Positive==1) //Mettere 1 per direzione positiva
        pid->Error = pid->SetP - pid->RetVal;
    else                     //Mettere 0 per direzione negativa
        pid->Error = pid->RetVal - pid->SetP;
    
    pid->pState = pid->pGain * pid->Error;					// Calcolo la parte proporzionale						->	Pp(k) = Kp * Err(k)

    if(pid->iGain==0.0) pid->iState = 0.0;    

    if (pid->iState > pid->iMax) 
        pid->iState = pid->iMax;
    else if (pid->iState < pid->iMin) 
        pid->iState = pid->iMin;         

    pid->iState = pid->iState + (pid->iGain * pid->Error);					// Calcolo la parte integrale e controllo limiti		->	Pi(k) = Ki * Err(k) + Pi(k-1)						
    if((pid->saturation_flag_Up==1) && (pid->iState>pid->OldiState))
       pid->iState=pid->OldiState;    
    if((pid->saturation_flag_Dw==1) && (pid->iState<pid->OldiState))
       pid->iState=pid->OldiState;
    pid->OldiState= pid->iState;
    /* Argo
    if (pid->iState > pid->iMax) 
        pid->iState = pid->iMax;
    else if (pid->iState < pid->iMin) 
        pid->iState = pid->iMin;
    */
    
    pid->dState = pid->dGain * (pid->Error - pid->LastError);	// Calcolo la parte derivativa							->	Pd(k) = Kd * [Err(k) - Err(k-1)]

    pid->LastError = pid->Error;

    // Controllo limiti per passaggio da float a int
    pid->fPVal = pid->pState + pid->iState + pid->dState;

    if(pid->fPVal > RealKf_PoMax)
    {
        pid->PVal = (int)round(RealKf_PoMax);
        pid->saturation_flag_Up = 1;
    }
    else if(pid->fPVal < Kf_PoMin)
    {
        pid->PVal = (int)round(Kf_PoMin);
        pid->saturation_flag_Dw = 1;
    }
    else      
    {
        pid->PVal = (int)round(pid->fPVal);          // Calcolo uscita PID e controllo limiti				->	 PVal(k) = Pp(k) + Pi(k) + Pd(k)
        pid->saturation_flag_Up = 0;
        pid->saturation_flag_Dw = 0;
    }
    
    pid->PValNoLim = pid->PVal;

    /* Argo
    if (pid->PVal > pid->PoMax) 
        pid->PVal = pid->PoMax;
    else if (pid->PVal < pid->PoMin) 
        pid->PVal = pid->PoMin;
     */
}


int Filter(volatile TypFilter * fil, int Var2fil, int nsamples) 
{ 
	if(fil->SampleCounter==0) fil->SumVar=0;	// Se è il primo "giro" inizializzo la memoria dei campioni
	
	fil->SumVar += (double)Var2fil;						// Sommo il campione attuale ai precedenti
	
	fil->SampleCounter +=1;						// incremento il cnt dei campioni

	if(fil->SampleCounter > nsamples)			// se ho oltrepassato la soglia dei campioni
	{	
		fil->SampleCounter = nsamples;			// riscalo il numero dei campioni
		fil->SumVar -= (double)fil->VarFiltered;		// sottraggo il campione medio dalla somma cumulativa
	}

	if(fil->SampleCounter != 0)
	{	
		fil->VarFiltered = (int)(fil->SumVar / (double)fil->SampleCounter); 
	}		

	return fil->VarFiltered;
}



