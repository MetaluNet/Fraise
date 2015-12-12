/*********************************************************************
 *
 *                i2c master library for Fraise pic18f  device
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  october 2013   adapted from Vangelis Rokas's sdcc i2c lib.
 ********************************************************************/

/*
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.
*/

#include <core.h>
#include <i2c_master.h>

#ifndef I2CMASTER_PORT
#define I2CMASTER_PORT 1
#endif

#if I2CMASTER_PORT==1
#define SSPxBUF 		SSPBUF
#define SSPxCON1 		SSPCON1 
#define SSPxCON1bits 	SSPCON1bits
#define SSPxCON2 		SSPCON2 
#define SSPxCON2bits 	SSPCON2bits
#define SSPxCON3 		SSPCON3 
#define SSPxCON3bits 	SSPCON3bits
#define SSPxSTAT 		SSPSTAT
#define SSPxSTATbits 	SSPSTATbits
#define SSPxMSK 		SSPMSK
#define SSPxMSKbits 	SSPMSKbits
#define SSPxADD 		SSPADD
#define SSPxADDbits 	SSPADDbits
#define SSPxIF 			PIR1bits.SSPIF
#define SSPxIE 			PIE1bits.SSPIE
#define SSPxIP 			PIR1bits.SSPIP
#define BCLxIF 			PIR2bits.BCLIF

/*#define SSPxBUF 		SSP1BUF
#define SSPxCON1 		SSP1CON1 
#define SSPxCON1bits 	SSP1CON1bits
#define SSPxCON2 		SSP1CON2 
#define SSPxCON2bits 	SSP1CON2bits
#define SSPxCON3 		SSP1CON3 
#define SSPxCON3bits 	SSP1CON3bits
#define SSPxSTAT 		SSP1STAT
#define SSPxSTATbits 	SSP1STATbits
#define SSPxMSK 		SSP1MSK
#define SSPxMSKbits 	SSP1MSKbits
#define SSPxADD 		SSP1ADD
#define SSPxADDbits 	SSP1ADDbits
#define SSPxIF 			PIR1bits.SSP1IF
#define SSPxIE 			PIE1bits.SSP1IE
#define SSPxIP 			PIR1bits.SSP1IP
#define BCLxIF 			PIR2bits.BCL1IF*/
#else
#define SSPxBUF 		SSP2BUF
#define SSPxCON1 		SSP2CON1 
#define SSPxCON1bits 	SSP2CON1bits
#define SSPxCON2 		SSP2CON2 
#define SSPxCON2bits 	SSP2CON2bits
#define SSPxCON3 		SSP2CON3 
#define SSPxCON3bits 	SSP2CON3bits
#define SSPxSTAT 		SSP2STAT
#define SSPxSTATbits 	SSP2STATbits
#define SSPxMSK 		SSP2MSK
#define SSPxMSKbits 	SSP2MSKbits
#define SSPxADD 		SSP2ADD
#define SSPxADDbits 	SSP2ADDbits
#define SSPxIF 			PIR3bits.SSP2IF
#define SSPxIE 			PIE3bits.SSP2IE
#define SSPxIP 			PIR3bits.SSP2IP
#define BCLxIF 			PIR3bits.BCL2IF
#endif

/*#ifndef SSP1CON1
#define SSPxBUF 		SSPBUF
#define SSPxCON1 		SSPCON1 
#define SSPxCON1bits 	SSPCON1bits
#define SSPxCON2 		SSPCON2 
#define SSPxCON2bits 	SSPCON2bits
#define SSPxCON3 		SSPCON3 
#define SSPxCON3bits 	SSPCON3bits
#define SSPxSTAT 		SSPSTAT
#define SSPxSTATbits 	SSPSTATbits
#define SSPxMSK 		SSPMSK
#define SSPxMSKbits 	SSPMSKbits
#define SSPxADD 		SSPADD
#define SSPxADDbits 	SSPADDbits
#define SSPxIF 			PIR1bits.SSPIF
#define SSPxIE 			PIE1bits.SSPIE
#define SSPxIP 			PIR1bits.SSPIP
#define BCLxIF 			PIR2bits.BCLIF
#endif*/


void i2cm_init(unsigned char mode, unsigned char slew, unsigned char addr_brd)
{
  SSPxSTAT &= 0x3f;
  SSPxCON1 = 0;
  SSPxCON2 = 0;
  SSPxCON1 |= mode;
  SSPxSTAT |= slew;

#if I2CMASTER_PORT==1
  SetPinDigiIn(I2C1SDA);
  SetPinDigiIn(I2C1SCL);  
  SetPinAnsel(I2C1SDA,0);
  SetPinAnsel(I2C1SCL,0);
#else
  SetPinDigiIn(I2C2SDA);
  SetPinDigiIn(I2C2SCL);
  SetPinAnsel(I2C2SDA,0);
  SetPinAnsel(I2C2SCL,0);
#endif

  SSPxADD = addr_brd;

  SSPxCON1 |= 0x20; // enable SSP
  
  SSPxIF = 0; // MSSP Interrupt Flag
  BCLxIF = 0; // Bus Collision Interrupt Flag

}

void i2cm_close(void)
{
  SSPxCON1 &= 0xdf;
}

/*--------------------------------------*/

unsigned char i2cm_drdy(void)
{
  if(SSPxSTATbits.BF) return (+1);
  else return (0);
}

void i2cm_idle(void)
{
  while((SSPxCON2 & 0x1f) | (SSPxSTATbits.R_W));
}

void i2cm_start(void)
{
  i2cm_idle();
  SSPxCON2bits.SEN = 1;
  while (SSPxCON2bits.SEN); 
}

void i2cm_restart(void)
{
  i2cm_idle();
  SSPxCON2bits.RSEN = 1;
  while (SSPxCON2bits.RSEN);
}

void i2cm_stop(void)
{
  i2cm_idle();
  SSPxCON2bits.PEN = 1;
  while (SSPxCON2bits.PEN);
}

void i2cm_ack(void)
{
  i2cm_idle();
  SSPxIF = 0;
  SSPxCON2bits.ACKDT = 0;
  SSPxCON2bits.ACKEN = 1;
  while (!SSPxIF); 
}

void i2cm_nack(void)
{
  i2cm_idle();
  SSPxIF = 0;
  SSPxCON2bits.ACKDT = 1;
  SSPxCON2bits.ACKEN = 1;  
  while (!SSPxIF); 
}

/*--------------------------------------*/

unsigned char i2cm_readchar(void)
{
  i2cm_idle();
  SSPxCON2bits.RCEN = 1;
  while( !i2cm_drdy() );
  return ( SSPxBUF );
}

char i2cm_readstr(_I2CPARAM_SPEC unsigned char *ptr, unsigned char len)
{
  unsigned char count=0;
  
  while( len-- ) {
    *ptr++ = i2cm_readchar();
    
    while(SSPxCON2bits.RCEN) {
      if(BCLxIF)return (-1);
      count++;
    
      if(len) {
        i2cm_ack();
        while(SSPxCON2bits.ACKEN);
      }
    }
  }
  
  return count;
}

char i2cm_writechar(unsigned char dat)
{
  SSPxCON1bits.WCOL = 0;
  SSPxBUF = dat;
  if( SSPxCON1bits.WCOL ) {
    return -1;
  } else {
    i2cm_idle();
    return 0;
  }
}

char i2cm_writestr(unsigned char *ptr)
{
  while( *ptr ) {
    if( SSPxCON1bits.SSPM3 ) {
      if(i2cm_writechar( *ptr )) {
        return (-3);
      }
      i2cm_idle();
      if( SSPxCON2bits.ACKSTAT ) {
        return (-2);
      }
    } else {
      SSPxIF = 0;
      SSPxBUF = *ptr;
      SSPxCON1bits.CKP = 1;
      while( !SSPxIF );
      
      if((!SSPxSTATbits.R_W) && ( !SSPxSTATbits.BF )) {
        return (-2);
      }
    }
    
    ptr++;
  }

  return 0;
}

/*--------------------------------------*/

void i2cm_begin(unsigned char address, unsigned char doread)
{
	i2cm_start();
	i2cm_writechar( (address<<1) | (doread != 0) );
}

