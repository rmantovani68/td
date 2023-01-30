//----------------------------------------------------------------------------------
//	File Name:		LCD_ScreenInfo.c
//	Device:			PIC18F4620 @ 10Mhz (PLLx4)
//	Autor:			G.L.
//	Date:			09/03/2016
//	Description:	LCD Routines for print data info
//----------------------------------------------------------------------------------
#include "HWSelection.h"
#if (DA210==1)
	#include <p24FJ256DA210.h>
#else
	#include <p24FJ256GB210.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include "delay.h"
#include "i2c_LCD_Driver.h"
#include "soft_i2c.h"
#include "Timer.h"
#include "Core.h"
#include "ProtocolloComunicazione.h"
#include "ProtocolloModBus.h"

#define K_MAX_SCREEN	2 //4
// Lcd constants
//              00000000011111111112
//              12345678901234567890 
char txt1[] =  "   TERMODINAMICA    ";
char txt2[] =  "Serial LCD for Debug";
char txt3[] =  "   and data info    ";
char txt4[] =  "  Master v.5.7.132  ";
char LCDstr[32];


void LCD_Info_Init(void)
{
    I2C_LCD_Init();
    I2C_LCD_Cmd(_LCD_CURSOR_OFF);
    I2C_LCD_Cmd(_LCD_CLEAR);    
    
    LCD_ScreeScheduler.Time = 5;		
	LCD_ScreeScheduler.TimeOut = 0;				
	LCD_ScreeScheduler.Value = 0;    
	LCD_ScreeScheduler.Enable = 1; 
}

void Print_LCD_Info(void)
{
    static int screen_scheduler=0;
    static int ii=0;
    static int xx=3;
    static int yy=5;    
    
    ii++;
    xx+=2;
    yy+=3;
    
    switch(screen_scheduler)
    {
        case 0:
//            sprintf(LCDstr, "DefMode= %i BRC=%i          ", Me.DefMode, Me.BroadcastRxCounter);
            sprintf(LCDstr, "DefMode= %i           ", EngineBox[0].Acceleration_GasPressure);			
            I2C_LCD_Out(1,1,LCDstr);     
            sprintf(LCDstr, "FuncMode= %i           ", EngineBox[0].Acceleration_GasPressure);
            I2C_LCD_Out(2,1,LCDstr);     
            sprintf(LCDstr, "FanSpeed= %i           ", EngineBox[0].Acceleration_GasPressure);
            I2C_LCD_Out(3,1,LCDstr);     			
            sprintf(LCDstr, "SP= %i T=%i               ", EngineBox[0].Acceleration_GasPressure, EngineBox[0].Acceleration_GasPressure);
            I2C_LCD_Out(4,1,LCDstr);                  // Write text in fourth row	
/*			
            I2C_LCD_Out(1,1,txt1);                  // Write text in first row
            I2C_LCD_Out(2,1,txt2);                  // Write text in second row
            I2C_LCD_Out(3,1,txt3);                  // Write text in third row
            sprintf(LCDstr, "  Master v.%i.%i.%i  ", Versione_Hw, Versione_Fw, Revisione_Fw);
            I2C_LCD_Out(4,1,LCDstr);                  // Write text in fourth row
*/
             break;
        case 1:          
//                           00000000011111111112
//                           12345678901234567890       
 //           sprintf(LCDstr, "TDefMode= %i BRC=%i         ", Touch[0].DefMode, Me.BroadcastRxCounter);
            sprintf(LCDstr, "TDefMode= %i          ", EngineBox[0].Acceleration_GasPressure);			
            I2C_LCD_Out(1,1,LCDstr);     
            sprintf(LCDstr, "TFuncMode= %i           ", EngineBox[0].Acceleration_GasPressure);
            I2C_LCD_Out(2,1,LCDstr);     
            sprintf(LCDstr, "TFanSpeed= %i           ", EngineBox[0].Acceleration_GasPressure);
            I2C_LCD_Out(3,1,LCDstr);     			
            sprintf(LCDstr, "TSP= %i   PWR=%i        ", EngineBox[0].Acceleration_GasPressure, EngineBox[0].Acceleration_GasPressure);
            I2C_LCD_Out(4,1,LCDstr);                  // Write text in fourth row				
			/*
            I2C_LCD_Out(1,1,"    System ready    ");    // Write text in first row
            I2C_LCD_Out(2,1,"  Engine Alarm = 0  ");    // Write text in second row
            I2C_LCD_Out(3,1,"  Handler Alarm = 0 ");    // Write text in third row
            //I2C_LCD_Out(4,1,txt4);                 // Write text in fourth row
            sprintf(LCDstr, "Program Cycle n:%i     ", ii);
            I2C_LCD_Out(4,1,LCDstr);                // Write text in fourth row      
			 */          
            break;
        case 2:                
//                           00000000011111111112
//                           12345678901234567890                 
            I2C_LCD_Out(1,1,"T_Comp_Out= 16.11 'C");    // Write text in first row
            I2C_LCD_Out(2,1,"T_Comp_Suc= 36.42 'C");    // Write text in second row
            I2C_LCD_Out(3,1,"T_Condens.= 12.81 'C");    // Write text in third row
            I2C_LCD_Out(4,1,"T_SeaWater= 6.55 'C ");    // Write text in fourth row     
            break;
        case 3:     
//                           00000000011111111112
//                           12345678901234567890                
            sprintf(LCDstr, "Temp_Amb= %.2f 'C     ", (float)EngineBox[0].Acceleration_GasPressure/100.0);
            I2C_LCD_Out(1,1,LCDstr);                    // Write text in first row
            I2C_LCD_Out(2,1,"Temp_Liq= 11.32 'C  ");    // Write text in second row
            I2C_LCD_Out(3,1,"Pres_Gas= 7.81 Bar  ");    // Write text in third row
            I2C_LCD_Out(4,1,"Pres_Liq= 26.55 Bar ");    // Write text in fourth row                   
            break;
        case 4:       
//                           00000000011111111112
//                           12345678901234567890                 
            I2C_LCD_Out(1,1,"Comunication Status:");    // Write text in first row
            sprintf(LCDstr, "CRC Err:%i  Tmt:%i  ", xx, yy);
            I2C_LCD_Out(2,1,LCDstr);                    // Write text in second row
            I2C_LCD_Out(3,1,"Frame Err:0  Baud:2 ");    // Write text in third row
            I2C_LCD_Out(4,1,"ModBus:RTU Device:3 ");    // Write text in fourth row                  
            break;                
        
    }
    
    if(LCD_ScreeScheduler.TimeOut)		
    {
        LCD_ScreeScheduler.TimeOut = 0;
        screen_scheduler++;
        if(screen_scheduler>=K_MAX_SCREEN) //4)
            screen_scheduler=0;
    }
	
}
