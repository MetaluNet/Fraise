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

#include <fruit.h>
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

char i2cm_errno = 0;

void i2cm_init(unsigned char mode, unsigned char slew, unsigned char addr_brd)
{
  SSPxSTAT &= 0x3f;
  SSPxCON1 = 0;
  SSPxCON2 = 0;
  SSPxCON1 |= mode;
  SSPxSTAT |= slew;

#if I2CMASTER_PORT==1
  pinModeDigitalIn(I2C1SDA);
  pinModeDigitalIn(I2C1SCL);  
#else
  pinModeDigitalIn(I2C2SDA);
  pinModeDigitalIn(I2C2SCL);
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
#define MAXLOOP 1000
#define SERVICE() 

unsigned char i2cm_drdy(void)
{
  if(SSPxSTATbits.BF) return (+1);
  else return (0);
}

void i2cm_idle(void)
{
    unsigned int i = MAXLOOP;

    while((--i > 0) && ( (SSPxCON2 & 0x1f) | (SSPxSTATbits.R_W))) SERVICE();
    i2cm_errno = (i == 0);
}

void i2cm_start(void)
{
  unsigned int i = MAXLOOP;
  i2cm_idle();
  SSPxCON2bits.SEN = 1;
  while ((--i > 0) && SSPxCON2bits.SEN) SERVICE();
  i2cm_errno = (i == 0);
}

void i2cm_restart(void)
{
  i2cm_idle();
  SSPxCON2bits.RSEN = 1;
  while (SSPxCON2bits.RSEN);
}

void i2cm_stop(void)
{
  unsigned int i = MAXLOOP;
  i2cm_idle();
  SSPxCON2bits.PEN = 1;
  //i2cm_idle();
  /*while ((--i > 0) && SSPxCON2bits.PEN) SERVICE();
  i2cm_errno = (i == 0);*/
}

void i2cm_ack(void)
{
  unsigned int i = MAXLOOP;
  i2cm_idle();
  SSPxIF = 0;
  SSPxCON2bits.ACKDT = 0;
  SSPxCON2bits.ACKEN = 1;
  while ((--i > 0) && !SSPxIF) SERVICE();
  i2cm_errno = (i == 0);
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
  unsigned int i = MAXLOOP;
  i2cm_idle();
  SSPxCON2bits.RCEN = 1;
  while((--i > 0) &&  !i2cm_drdy()) SERVICE();
  i2cm_errno = (i == 0);
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
    //while( !SSPxIF );
    if( SSPxCON2bits.ACKSTAT || (i2cm_errno != 0)) return -1;
    else return 0;
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

char i2cm_begin(unsigned char address, unsigned char doread)
{
	i2cm_start();
	if(i2cm_errno != 0) return -1;
	return i2cm_writechar( (address<<1) | (doread != 0) );
}

