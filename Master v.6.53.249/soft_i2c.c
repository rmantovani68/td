//----------------------------------------------------------------------------------
//	File Name:		soft_i2c.c
//	Device:			PIC18F4620 @ 10Mhz (PLLx4)
//	Autor:			G.L.
//	Date:			09/03/2016
//	Description:	Software I2C Routines 
//----------------------------------------------------------------------------------

#include "soft_i2c.h"
#include "delay.h"



void i2cInit(void)
{
    SDA_OPIN = 1;        // output a logic one   
    SCL_OPIN = 1;        // output a logic one    
    SDA_DIR = 0;         // bring SDA to Output 
    SCL_DIR = 0;         // bring SCL to Output    
    DelayuSec(50);
}

unsigned char i2cReadByte(void)
{
   unsigned char inByte, n;
   i2cHighSda();
   for (n=0; n<8; n++)
   {
      i2cHighScl();

      if (SDA_IPIN)
         inByte = (inByte << 1) | 0x01; // msbit first
      else
         inByte = inByte << 1;
      i2cLowScl();
   }
   return(inByte);
}

void i2cWriteByte(unsigned char outByte)
{
   unsigned char n;
   for(n=0; n<8; n++)
   {
      if(outByte&0x80)
         i2cHighSda();
      else
         i2cLowSda();
      i2cHighScl();
      i2cLowScl();
      outByte = outByte << 1;
   }
   i2cAck();
   i2cHighSda();
}

void i2cNack(void)
{
   i2cHighScl();
   i2cLowScl();		// bring data high and clock
}

void i2cAck(void)
{
   SDA_DIR = 1;         // bring SDA to high impedance
   SDA_OPIN = 1;        // output a logic one    
   i2cLowScl();
   i2cHighScl();
   i2cLowScl();
   i2cHighSda();
 //  i2cLowSda();	
 //  i2cHighScl();
 //  i2cLowScl();
 //  i2cHighSda();		// bring data low and clock
}


void i2cStart(void)
{
    i2cHighSda();
    i2cHighScl();	
    i2cLowSda();
    i2cLowScl();
 //  i2cLowScl();
 //  i2cHighSda();
 //  i2cHighScl();	
 //  i2cLowSda();
 //  i2cLowScl();		// bring SDA low while SCL is high
}

void i2cStop(void)
{
   i2cLowScl();
   i2cLowSda();
   i2cHighScl();
   i2cHighSda();		// bring SDA high while SCL is high
}
/*
void Soft_I2C_Start(void)
{
_Sda=HIGH;
_Scl=HIGH;
_Sda=LOW;
_Scl=HIGH;
}
void Soft_I2C_Stop(void)
{
_Sda=LOW;
_Scl=HIGH;
_Sda=HIGH;
_Scl=HIGH;
}
*/




void i2cHighSda(void)
{
   //SDA_DIR = 1;		// bring SDA to high impedance
   SDA_OPIN = 1;        // output a logic one   
   SDA_DIR = 0;         // bring SDA to Output 
   DelayuSec(2);
}

void i2cLowSda(void)
{
   SDA_OPIN = 0;    // output a logic zero 
   SDA_DIR = 0;		// bring SDA to Output 
   DelayuSec(2);
}

void i2cHighScl(void)
{
   //SCL_DIR = 1;		// bring SCL to high impedance
   SCL_OPIN = 1;        // output a logic one      
   SCL_DIR = 0;         // bring SCL to Output       
   DelayuSec(2);
}

void i2cLowScl(void)
{
   SCL_OPIN = 0;		
   SCL_DIR = 0;
   DelayuSec(2);
}


/*
void i2cHighSda(void)
{
   SDA_DIR = 1;		// bring SDA to high impedance
   DelayUs(15);
}

void i2cLowSda(void)
{
   SDA_OPIN = 0;
   SDA_DIR = 0;		// output a logic zero
   DelayUs(15);
}

void i2cHighScl(void)
{
   SCL_DIR = 1;		// bring SCL to high impedance
   DelayUs(15);
}

void i2cLowScl(void)
{
   SCL_OPIN = 0;		
   SCL_DIR = 0;
   DelayUs(15);
}
 */