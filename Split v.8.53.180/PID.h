#ifndef _PID_H_
	#define _PID_H_

#include "FWSelection.h"
#include "Valvola_PassoPasso.h"

    //#define     DEBUGPID        1	// inserire questo define per escludere altre comunicazioni 
                                    // su BUS secondario e lasciare solo protocollo PLX-DAQ per
                                    // debug e taratura PID

	#define		K_p_gain		2.275//2.235 13_07_15 OK   //0.1 //2.0
	#define		K_i_gain		0.0107//0.0078 13_07_15 OK  //0.07//0.05 //0.7 
	#define		K_d_gain		0       //0.02  //1.5
	#define		K_dead_zone		2
	#define		K_PoMax			9500//250 //AperturaMaxFreddo //210
    #define		K_PoMin			0 //0 //AperturaMinFreddo //50
	#define		K_iMax			9500//300 //300
	#define		K_iMin			0 //-50 //-50

//	#define		K_ValveMax		AperturaMaxFreddo //210
//    #define		K_ValveMin		AperturaMinFreddo //50


	#define		K_SetPoint		600     // Setpoint PID (SuperHeat = 6°C)

    #define     K_gainSuperHeat 1.0     // Guadagno valore retroazione PID (SuperHeat))
    
    #define     K_TimeExecPID   500     // Tempo in mS per esecuzione regolatore PID per controllo Valvola espansione
    
    #define     K_Campioni_Filter               20    //Numero campioni filtri funzione FILTER
    #define     K_Campioni_FilterFanSpeed       20//50    //Numero campioni filtri funzione FILTER x calcolo media delta vel. ventola
	#define     K_Campioni_FilterEvapPress      500    //Numero campioni filtri funzione FILTER x calcolo media evap press
    #define     K_Campioni_Filter_FanTacho      2
    #define     K_Campioni_FilterFan_Power      8  //Numero campioni filtri funzione FILTER FAN Power
    #define     K_Campioni_FilterFan_Current	8  //50    //Numero campioni filtri funzione FILTER FAN Current
    #define     K_Campioni_FilterFan_M3h        8  //Numero campioni filtri funzione FILTER FAN M3h
    #define     K_Campioni_FilterAirPower       2


	#define VariabiliGlobaliPID()				\
		volatile TypPID ValvePID;               \
		volatile TypPID ValvePID2;				\
        volatile TypFilter FilterSuperHeat;		\
		volatile TypFilter FilterSuperHeatRiscaldamento;\
        volatile TypFilter FilterBig_Request;	\
        volatile TypFilter FilterExpValve;		\
		volatile TypFilter FilterFan_MaxSpeed;	\
		volatile TypFilter FilterFan_Tacho;     \
		volatile TypFilter FilterEvap_Press;    \
		volatile TypFilter FilterFan_Power;     \
		volatile TypFilter FilterFan_Current;   \
		volatile TypFilter FilterFan_FastCurrent;   \
        volatile TypFilter FilterFan_M3h;       \
        volatile TypFilter FilterAirPowerOutput;\
        volatile TypFilter FilterAirPowerBTU;

	#define IncludeVariabiliGlobaliPID()			\
        extern volatile TypPID ValvePID;			\
        extern volatile TypPID ValvePID2;			\
        extern volatile TypFilter FilterSuperHeat;	\
        extern volatile TypFilter FilterSuperHeatRiscaldamento;	\
        extern volatile TypFilter FilterBig_Request;   \
        extern volatile TypFilter FilterExpValve;	   \
		extern volatile TypFilter FilterFan_MaxSpeed;  \
        extern volatile TypFilter FilterFan_Tacho;     \
		extern volatile TypFilter FilterEvap_Press;    \
		extern volatile TypFilter FilterFan_Power;     \
		extern volatile TypFilter FilterFan_Current;   \
		extern volatile TypFilter FilterFan_FastCurrent;  \
        extern volatile TypFilter FilterFan_M3h;       \
        extern volatile TypFilter FilterAirPowerOutput;\
        extern volatile TypFilter FilterAirPowerBTU;

typedef struct
{
  int	 SetP;			// Set Point
  int	 RetVal;		// Retroaction Value
  int	 PVal;			// PID Output limited
  int	 PValNoLim;		// PID Output No limited
  int	 DeadZone;		// Dead zone

  float	fPVal;			// PID Output (float))
  float pState;      	// Proportional state
  float iState;      	// Integrator state
  float dState;      	// Derivator state
  float iMax;			// Maximum and minimum allowable integrator state
  float iMin;  		
  int   PoMax;			// Maximum and minimum allowable PID output
  int   PoMin;  		

  float iGain;			// integral gain
  float pGain;    		// proportional gain
  float dGain;          // derivative gain
  
  float Error;
  float LastError;
  
  //DEBUG
  float VKiPID;
  float VKdPID;
  //DEBUG

} TypPID;



typedef struct
{
  int       SampleCounter;			// Contatore campioni
  double    SumVar;                 // Var per somma campioni
  int       VarFiltered;            // Variabile per calcolo media campioni
} TypFilter;

	//---------------------------------------
	// Variabili 
	//---------------------------------------
	IncludeVariabiliGlobaliPID();

	//---------------------------------------
	// Prototipi delle funzioni
	//---------------------------------------
    void PID_init(volatile TypPID * pid, float p_gain, float i_gain, float d_gain, float i_Min, float i_Max, int Po_Min, int Po_Max, int dead_zone);
	void UpdatePID(volatile TypPID * pid);
    int PIDOut2Valve(int OutPID, int ValMinIn, int ValMaxIn, int ValMinOut, int ValMaxOut);
    int Valve2PIDOut(int OutValve, int ValMinIn, int ValMaxIn, int ValMinOut, int ValMaxOut);
    void PID_SetOut(volatile TypPID * pid, int SetValveOut);
    int Filter(volatile TypFilter * fil, int Var2fil, int nsamples);     
#endif