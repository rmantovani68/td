#ifndef _PID_H_
	#define _PID_H_

    //#define     DEBUGPID        1	// inserire questo define per escludere altre comunicazioni 
                                    // su BUS secondario e lasciare solo protocollo PLX-DAQ per
                                    // debug e taratura PID

	#define		K_p_gain		3.50//10.0         Range: 0..300       0...65535   0.03
    #define		K_i_gain		0.01//0.8           Range: 0..300
	#define		K_d_gain		0.0//30.0//0       //0.02  //1.5
	#define		K_dead_zone		2
	#define		K_PoMax			7680 //250 //AperturaMaxFreddo //210
    #define		K_PoMin			0 //0 //AperturaMinFreddo //50
	#define		Kf_iMax			7680.0 //300 //300
	#define		Kf_iMin			0.0 //-50 //-50
	//#define		Kf_PoMax        1920.0/*25%*///2150.0/*28%*///1382.0/*20%*///3072.0/*40%*///7680.0/*100%*///ATTENZIONE: DA SISTEMARE //250 //AperturaMaxFreddo //210    !!!!  LIMITE PID IMPOSTATO ALLA MASSIMO STEP DELLA VALVOLA (480 X KMW = 480 X 16 = 7680)
    #define		Kf_PoMin		0.0 //0 //AperturaMinFreddo //50

	//#define		K_ValveMax		AperturaMaxFreddo //210
    //#define		K_ValveMin		AperturaMinFreddo //50

	#define		K_SetPoint		600     // Setpoint PID (SuperHeat = 6°C)
    #define     K_gainSuperHeat 1.0     // Guadagno valore retroazione PID (SuperHeat))
    
    #define     K_TimeExecPID   600//1      // Tempo in mS per esecuzione regolatore PID per controllo Valvola espansione
    
  	#define     K_Campioni_FilterEvapPressFreddo	50//500 //2000  //Numero campioni filtri funzione FILTER x calcolo media evap press
  	#define     K_Campioni_FilterEvapPressCaldo 	10    //Numero campioni filtri funzione FILTER x calcolo media evap press

 	#define     K_Campioni_FilterAccInverter		100//500    //Numero campioni filtri funzione CompressionError x calcolo media Acc. Inverter
  	#define     K_Campioni_FilterAccGas				100//500    //Numero campioni filtri funzione CompressionError x calcolo media Acc. Pressione Gas

    #define     K_Campioni_FilterHeatingSuperHeat   15  //20
    #define     K_Campioni_FilterPressSeaWater      20
    #define     K_Campioni_FilterSuperHeatHeatingCalc   7//10//4//25

    #define     K_Campioni_FilterSuperHeatErrorValue   3

    #define     K_Campioni_FilterPowerBoxAbsorption     5
    #define     K_Campioni_FilterCoolingSuperHeat       7
    #define     K_Campioni_FilterPumpAbsorption         5


    #define     K_Campioni_FilterInverterOut_Current    9//10


	#define VariabiliGlobaliPID()                   \
		volatile TypPID ValvePID;                   \
		volatile TypPID SuperHeatPID;               \
		volatile TypPID PressSeaWaterPID;           \
        volatile TypPID FilterSuperHeatHeatingCalc; \
        volatile TypPID FilterSuperHeatErrorValue;  \
        volatile TypFilter FilterHeatingSuperHeat;  \
        volatile TypFilter FilterPressSeaWater;     \
		volatile TypFilter FilterEvap_PressFreddo;	\
		volatile TypFilter FilterEvap_PressCaldo;	\
		volatile TypFilter FilterAccGas;			\
		volatile TypFilter FilterAccInverter;       \
        volatile TypFilter FilterTotalPowerBoxAbsorption;\
        volatile TypFilter FilterPowerBox1Absorption;\
        volatile TypFilter FilterPowerBox2Absorption;\
        volatile TypFilter FilterCoolingSuperHeat;  \
        volatile TypFilter FilterInverterOut_Current1; \
        volatile TypFilter FilterInverterOut_Current2; \
        volatile TypFilter FilterPumpAbsorption;    \
        volatile TypFilter FilterInverterPumpOut_Current;
        
        

	#define IncludeVariabiliGlobaliPID()                    \
        extern volatile TypPID ValvePID;                    \
		extern volatile TypPID SuperHeatPID;                \
		extern volatile TypPID PressSeaWaterPID;            \
        extern volatile TypPID FilterSuperHeatHeatingCalc;  \
        extern volatile TypPID FilterSuperHeatErrorValue;   \
        extern volatile TypFilter FilterHeatingSuperHeat;   \
        extern volatile TypFilter FilterPressSeaWater;      \
		extern volatile TypFilter FilterEvap_PressFreddo;	\
		extern volatile TypFilter FilterEvap_PressCaldo;	\
		extern volatile TypFilter FilterAccGas;				\
		extern volatile TypFilter FilterAccInverter;        \
        extern volatile TypFilter FilterTotalPowerBoxAbsorption; \
        extern volatile TypFilter FilterPowerBox1Absorption; \
        extern volatile TypFilter FilterPowerBox2Absorption; \
        extern volatile TypFilter FilterCoolingSuperHeat;   \
        extern volatile TypFilter FilterInverterOut_Current1; \
        extern volatile TypFilter FilterInverterOut_Current2; \
        extern volatile TypFilter FilterPumpAbsorption;     \
        extern volatile TypFilter FilterInverterPumpOut_Current;

typedef struct
{
  int	 SetP;			// Set Point
  int	 RetVal;		// Retroaction Value
  int	 PVal;			// PID Output limited
  int	 PValNoLim;		// PID Output No limited
  int	 DeadZone;		// Dead zone
  
  int   saturation_flag_Up;            // Flag per gestione "anti-wind-up"
  int   saturation_flag_Dw;            // Flag per gestione "anti-wind-up"
  float OldiState;  

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
    void UpdatePID(volatile TypPID * pid, int Direction_Positive);
    int PIDOut2Valve(int OutPID, int ValMinIn, int ValMaxIn, int ValMinOut, int ValMaxOut);
    int Valve2PIDOut(int OutValve, int ValMinIn, int ValMaxIn, int ValMinOut, int ValMaxOut);
    void PID_SetOut(volatile TypPID * pid, int SetValveOut);
    int Filter(volatile TypFilter * fil, int Var2fil, int nsamples);     
#endif