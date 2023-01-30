//----------------------------------------------------------------------------------
//	Progect name:	Delay.h
//	Device:			PIC18F4550 @ 20Mhz (PLL 96 Mhz)
//	Autor:			TDM Team
//	Date:			22/05/2011
//	Description:	Header delle funzioni Delay
//----------------------------------------------------------------------------------

#ifndef _DELAY_H_
	//----------------------------------------------------------------------------------
	//	Define
	//----------------------------------------------------------------------------------
	#define _DELAY_H_

	#define FCY 16000000UL/1.045 //32000000UL	//16Mhz

	#define __delay_ms(d) \
	  { __delay32( (unsigned long) (((unsigned long long) d)*(FCY)/1000ULL)); }
	#define __delay_us(d) \
	  { __delay32( (unsigned long) (((unsigned long long) d)*(FCY)/1000000ULL)); }

	#define DelaymSec(d) \
	  { __delay32( (unsigned long) (((unsigned long long) d)*(FCY)/1000ULL)); }
	#define DelayuSec(d) \
	  { __delay32( (unsigned long) (((unsigned long long) d)*(FCY)/1000000ULL)); }

#endif
