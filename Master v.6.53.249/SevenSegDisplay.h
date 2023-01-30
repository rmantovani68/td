//----------------------------------------------------------------------------------
//	File Name:		SevenSegDisplay.h
//	Device:			p24FJ256GB210
//	Autor:			G.L.
//	Date:			20/06/2017
//	Description:	Routines for print caracter on 7 Segment Display
//----------------------------------------------------------------------------------

#ifndef SEVENSEGDISPLAY_H
    #define SEVENSEGDISPLAY_H

#define ON          1
#define OFF         0





void PrintDigit(unsigned char Digit, unsigned char charon , unsigned char decpointon, unsigned char blinkon);
void PutSegment(unsigned char Digit, unsigned char decpointon);


#endif
