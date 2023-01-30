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
#include "DefinePeriferiche.h"
#include "Timer.h"
#include "SevenSegDisplay.h"
#include "ProtocolloComunicazione.h"

 
extern unsigned int DspToggle;

void PrintDigit(unsigned char Digit, unsigned char charon , unsigned char decpointon, unsigned char blinkon)
{
    
    EngineBox[0].DisplayDigit = Digit;
    EngineBox[0].DisplayDigitDP = decpointon;
    
    if(!(EngineBox[0].DisplayDigitBlink==1 && DspToggle==1))
        PutSegment(EngineBox[0].DisplayDigit, EngineBox[0].DisplayDigitDP);

    if(blinkon) EngineBox[0].DisplayDigitBlink = 1;
        else EngineBox[0].DisplayDigitBlink = 0;        

}

void PutSegment(unsigned char Digit, unsigned char decpointon)
{
//           AAA                         
//         F     B   
//         F     B   
//           GGG     
//         E     C   
//         E     C   	
//           DDD     
	switch(Digit)
    {
        case 0:
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentG = LED_OFF;}				
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentG = LED_OFF;}
            break;															
        case 1:            
			if(EngineBox[0].ReverseDisplayDigit==1)
				{oSegmentA = LED_ON; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentG = LED_OFF;}				
			else
				{oSegmentD = LED_ON; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentG = LED_OFF;}
            break;					
        case 2:            
			if(EngineBox[0].ReverseDisplayDigit==1)
				{oSegmentA = LED_OFF; oSegmentB = LED_ON; oSegmentC = LED_OFF; oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentG = LED_OFF;}				
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_ON; oSegmentF = LED_OFF; oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentG = LED_OFF;}
            break;					
        case 3:            
			if(EngineBox[0].ReverseDisplayDigit==1)
				{oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_ON; oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentG = LED_OFF;}				
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_ON; oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentG = LED_OFF;}
            break;					
        case 4:            
			if(EngineBox[0].ReverseDisplayDigit==1)
				{oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentD = LED_ON; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentG = LED_OFF;}				
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentA = LED_ON; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentG = LED_OFF;}
            break;					
        case 5:            
			if(EngineBox[0].ReverseDisplayDigit==1)
				{oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentD = LED_OFF; oSegmentE = LED_ON; oSegmentF = LED_OFF; oSegmentG = LED_OFF;}				
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentA = LED_OFF; oSegmentB = LED_ON; oSegmentC = LED_OFF; oSegmentG = LED_OFF;}
            break;					
        case 6:            
			if(EngineBox[0].ReverseDisplayDigit==1)
				{oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_ON; oSegmentG = LED_OFF;}				
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_ON; oSegmentG = LED_OFF;}
            break;					
        case 7:            
			if(EngineBox[0].ReverseDisplayDigit==1)
				{oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentG = LED_ON;}				
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentG = LED_ON;}
            break;					
                        
        case 'A':																																								
			if(EngineBox[0].ReverseDisplayDigit==0)
	            {oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentD = LED_OFF; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentG = LED_ON;}		
			else          			          
	            {oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentA = LED_OFF; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentG = LED_ON;}						
            break;																																								
        case 'b':																																								
 			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_ON; oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentG = LED_ON;}						
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_ON; oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentG = LED_ON;}						
            break;                                                                                                                  
        case 'c':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_OFF; oSegmentG = LED_ON;} 
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_OFF; oSegmentG = LED_ON;} 
            break;
        case 'C':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_ON; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentG = LED_OFF;} 
			else
				{oSegmentD = LED_ON; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentG = LED_OFF;} 
            break;
        case 'd':
 			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_OFF; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_OFF; oSegmentG = LED_ON;} 
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_OFF; oSegmentG = LED_ON;} 
            break;
        case 'E':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_ON; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentG = LED_ON;}        
			else
				{oSegmentD = LED_ON; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentG = LED_ON;}        
            break;
        case 'F':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_ON; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentD = LED_OFF; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentG = LED_ON;} 
			else
				{oSegmentD = LED_ON; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentA = LED_OFF; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentG = LED_ON;} 
            break;
        case 'g':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentD = LED_ON; oSegmentE = LED_OFF; oSegmentF = LED_ON; oSegmentG = LED_ON;} 
			else
				{oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentA = LED_ON; oSegmentB = LED_OFF; oSegmentC = LED_ON; oSegmentG = LED_ON;} 
            break;
        case 'H':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_OFF; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentD = LED_OFF; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentG = LED_ON;} 
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentA = LED_OFF; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentG = LED_ON;} 
            break;
        case 'h':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_ON; oSegmentD = LED_OFF; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentG = LED_ON;} 
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_ON; oSegmentA = LED_OFF; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentG = LED_ON;} 
            break;            
        case 'I':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentD = LED_OFF; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentG = LED_OFF;} 
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentA = LED_OFF; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentG = LED_OFF;} 
            break;
        case 'L':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentG = LED_OFF;} 
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentG = LED_OFF;} 
            break;
        case 'n':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_ON; oSegmentD = LED_OFF; oSegmentE = LED_ON; oSegmentF = LED_OFF; oSegmentG = LED_ON;} 
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_ON; oSegmentA = LED_OFF; oSegmentB = LED_ON; oSegmentC = LED_OFF; oSegmentG = LED_ON;} 
            break;
        case 'o':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_ON; oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_OFF; oSegmentG = LED_ON;} 
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_ON; oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_OFF; oSegmentG = LED_ON;} 
            break;            
        case 'O':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentG = LED_OFF;} 
			else
				{oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentG = LED_OFF;} 
            break;
        case 'P':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_OFF; oSegmentD = LED_OFF; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentG = LED_ON;} 
			else
				{oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_OFF; oSegmentA = LED_OFF; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentG = LED_ON;} 
            break;
        case 'q':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_ON; oSegmentG = LED_ON;} 
			else
				{oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_ON; oSegmentG = LED_ON;} 
            break;
        case 'r':
			if(EngineBox[0].ReverseDisplayDigit==0)
	            {oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentD = LED_OFF; oSegmentE = LED_ON; oSegmentF = LED_OFF; oSegmentG = LED_ON;} 
			else
	            {oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentA = LED_OFF; oSegmentB = LED_ON; oSegmentC = LED_OFF; oSegmentG = LED_ON;} 
            break;           
        case 'S':
			if(EngineBox[0].ReverseDisplayDigit==0)
	            {oSegmentA = LED_ON; oSegmentB = LED_OFF; oSegmentC = LED_ON; oSegmentD = LED_ON; oSegmentE = LED_OFF; oSegmentF = LED_ON; oSegmentG = LED_ON;} 
			else
	            {oSegmentD = LED_ON; oSegmentE = LED_OFF; oSegmentF = LED_ON; oSegmentA = LED_ON; oSegmentB = LED_OFF; oSegmentC = LED_ON; oSegmentG = LED_ON;} 
            break;
       case 't':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentD = LED_OFF; oSegmentE = LED_ON; oSegmentF = LED_OFF; oSegmentG = LED_ON;} 
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentA = LED_OFF; oSegmentB = LED_ON; oSegmentC = LED_OFF; oSegmentG = LED_ON;} 
            break;           
       case 'u':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_ON; oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_OFF; oSegmentG = LED_OFF;} 
			else
				{oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_ON; oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_OFF; oSegmentG = LED_OFF;} 
            break;           
       case 'U':
			if(EngineBox[0].ReverseDisplayDigit==0)
	            {oSegmentA = LED_OFF; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentG = LED_OFF;} 
			else
	            {oSegmentD = LED_OFF; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentG = LED_OFF;} 
            break;            
       case '0':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentG = LED_OFF;} 
			else
				{oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentG = LED_OFF;} 
            break;
       case '1':
			if(EngineBox[0].ReverseDisplayDigit==0)
	            {oSegmentA = LED_OFF; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentG = LED_OFF;} 
			else
	            {oSegmentD = LED_OFF; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentG = LED_OFF;} 
            break;
       case '2':
			if(EngineBox[0].ReverseDisplayDigit==0)
	            {oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_OFF; oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_OFF; oSegmentG = LED_ON;} 
			else
	            {oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_OFF; oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_OFF; oSegmentG = LED_ON;} 
            break;
       case '3':
			if(EngineBox[0].ReverseDisplayDigit==0)
	            {oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentD = LED_ON; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentG = LED_ON;} 
			else
	            {oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentA = LED_ON; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentG = LED_ON;} 
            break;
       case '4':
			if(EngineBox[0].ReverseDisplayDigit==0)
	            {oSegmentA = LED_OFF; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_ON; oSegmentG = LED_ON;} 
			else
	            {oSegmentD = LED_OFF; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_ON; oSegmentG = LED_ON;} 
            break;
       case '5':
			if(EngineBox[0].ReverseDisplayDigit==0)
	            {oSegmentA = LED_ON; oSegmentB = LED_OFF; oSegmentC = LED_ON; oSegmentD = LED_ON; oSegmentE = LED_OFF; oSegmentF = LED_ON; oSegmentG = LED_ON;} 
			else
	            {oSegmentD = LED_ON; oSegmentE = LED_OFF; oSegmentF = LED_ON; oSegmentA = LED_ON; oSegmentB = LED_OFF; oSegmentC = LED_ON; oSegmentG = LED_ON;} 
            break;
       case '6':
			if(EngineBox[0].ReverseDisplayDigit==0)
	            {oSegmentA = LED_ON; oSegmentB = LED_OFF; oSegmentC = LED_ON; oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentG = LED_ON;} 
			else
	            {oSegmentD = LED_ON; oSegmentE = LED_OFF; oSegmentF = LED_ON; oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentG = LED_ON;} 
            break;
       case '7':
			if(EngineBox[0].ReverseDisplayDigit==0)
				{oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentD = LED_OFF; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentG = LED_OFF;} 
			else
				{oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentA = LED_OFF; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentG = LED_OFF;} 
            break;
       case '8':
			if(EngineBox[0].ReverseDisplayDigit==0)
	            {oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentG = LED_ON;} 
			else
	            {oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentG = LED_ON;} 
            break;
       case '9':
			if(EngineBox[0].ReverseDisplayDigit==0)
	            {oSegmentA = LED_ON; oSegmentB = LED_ON; oSegmentC = LED_ON; oSegmentD = LED_ON; oSegmentE = LED_OFF; oSegmentF = LED_ON; oSegmentG = LED_ON;} 
			else
	            {oSegmentD = LED_ON; oSegmentE = LED_ON; oSegmentF = LED_ON; oSegmentA = LED_ON; oSegmentB = LED_OFF; oSegmentC = LED_ON; oSegmentG = LED_ON;} 
            break;
            
        default:
            oSegmentA = LED_ON; oSegmentB = LED_OFF; oSegmentC = LED_OFF; oSegmentD = LED_ON; oSegmentE = LED_OFF; oSegmentF = LED_OFF; oSegmentG = LED_ON; 
            break;
    }
    
    if(decpointon) oSegmentDP = LED_ON;
        else oSegmentDP = LED_OFF;    
    
    
}