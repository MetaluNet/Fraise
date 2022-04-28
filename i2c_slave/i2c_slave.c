// from Sébastien Lorquet, see: https://sdcc-user.narkive.com/oPwSQV5x/example-or-tutorial-for-i2c-usage-with-pic
// or https://narkive.com/oPwSQV5x.19

#include "i2c_slave.h"

#ifndef I2CSLAVE_PORT
#define I2CSLAVE_PORT 1
#endif

#if I2CSLAVE_PORT==1
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
#define SSPxIP 			IPR1bits.SSPIP
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
#define SSPxIP 			IPR3bits.SSP2IP
#define BCLxIF 			PIR3bits.BCL2IF
#endif

// DA S RW BF
#define I2C_SLAVE_STATE_MASK 0x2D // - - X - X X - X
#define I2C_SLAVE_STATE_1 0x09 // - - 0 - 1 0 - 1 DA=0 S=1 RW=0 BF=1
#define I2C_SLAVE_STATE_2 0x29 // - - 1 - 1 0 - 1 DA=1 S=1 RW=0 BF=1
#define I2C_SLAVE_STATE_3_MASK 0x2C // - - X - X X - - da,s,rw
#define I2C_SLAVE_STATE_3 0x0C // - - 0 - 1 1 - - DA=0 S=1 RW=1
#define I2C_SLAVE_STATE_4 0x2C // - - 1 - 1 1 - 0 DA=1 S=1 RW=1 BF=0 CKP=0
#define I2C_SLAVE_STATE_5_MASK 0x28 // - - X - X - - - da,s
#define I2C_SLAVE_STATE_5 0x28 // - - 1 - 1 0 - 0 DA=1 S=1 CKP=1

unsigned char i2c_regs[I2C_REG_SIZE]; //actual memory storage
static unsigned char i2c_reg_addr; //current memory address, where the operation will take place
static unsigned char i2c_first_data; //true when we are about to get the first data byte, ie register address
static unsigned char i2c_status; //masked SSPxSTAT value
static unsigned char i2c_data; //temporary storage for data
#ifdef I2C_DEBUG
static unsigned char flag;
#endif


/*The initialization code defines the MSSP macrocell configuration. The slave
address is defined here.
the code from fabien lementec initializes the slave with 0x26 instead of
0x36, which I don't like. I prefer initializing CKP to 1 to be sure the
clock line is released. Apart from that, the initialization is very similar */

//-------------------------------------------------------------------------------------------
//Setup the pic peripheral
void i2c_slave_setup(unsigned char addr)
{
#if I2CSLAVE_PORT==1
	pinModeDigitalIn(I2C1SDA);
	pinModeDigitalIn(I2C1SCL);  
#else
	pinModeDigitalIn(I2C2SDA);
	pinModeDigitalIn(I2C2SCL);
#endif
	SSPxCON1 = 0x36; //7-bit I2C slave, no IRQ on start/stop
	//SSPxCON2bits.SEN = 1; //enable clock stretching
	SSPxADD = (addr<<1);
	SSPxIP = 0;
	SSPxIF = 0;
	SSPxIE = 1;
	//init vars
	i2c_reg_addr = 0;
	i2c_first_data = 0;
}

/* The slave is interrupt driven.

Algorithm:

There are 5 states, defined by the value in the SSPxSTAT register

State 1 is triggered when the slave receives the address byte with the RW
flag cleared, ie we start a write transaction.
State 2 is triggered when more bytes are sent by the master. Here, we want
to discriminate the first one which is the register address, but a PCF8574
is even simpler than that
State 3 is triggered when we just got an address byte with the RW flag set,
ie when we are READY to send the first byte of the read transaction. Now you
understand that we must first write the register address byte before reading
anything.
State 4 is triggered when we are READY to send the next bytes.
State 5 is triggered after the master sent a NACK in a read transaction.

There are two MSSP macrocells in PIC devices: PIC16 and devices listed in
AN734 page 17 have the OLD state machine, all other PIC18 have the NEW state
machine.
With the NEW machine, in state 3, we MUST read SSPxBUF or the read operation
will fail. This is what prevented me from posting this code earlier! There
is no need to read SSPxBUF in state 4.

So with new PICs, you have to */
#define I2C_SLAVE_NEW

/* This does not follow the state machine from fabien, I didn't try his code,
but I guess mine is more straightforward, with less if() statements.*/

/* Here is the code: */

//-------------------------------------------------------------------------------------------
//Manage I2C slave events
void i2c_slave_ISR()
{
	if(!SSPxIF) return;
	//SSPxIF = 0; return;
	//no local variables here for faster response times
	i2c_status = SSPxSTAT & I2C_SLAVE_STATE_MASK; //Mask useless bits
	#ifdef I2C_DEBUG
	ser_putchar('[');
	ser_puthb(i2c_status);
	ser_putchar(']');
	#endif

	//--------------------
	// state 1: just got an address byte for a write transaction.
	//--------------------
	if(i2c_status == I2C_SLAVE_STATE_1) {
		i2c_first_data = 1; //next byte will be the register address
		//dummy read the buffer to clear BF
		i2c_data = SSPxBUF;
		#ifdef I2C_DEBUG
		ser_putchar('1');
		#endif
		goto i2c_slave_exit;
	}
	//--------------------
	// state 2: just got a data byte in a write transaction.
	//--------------------
	if(i2c_status == I2C_SLAVE_STATE_2) {
		i2c_data = SSPxBUF; //read data to clear BF
		if(i2c_first_data) { //this is the first data byte, store into the register address
			i2c_first_data = 0;
			i2c_reg_addr = i2c_data;
			#ifdef I2C_DEBUG
			flag=0;
			#endif
		} else { //next bytes are really data
			if(i2c_reg_addr<I2C_REG_SIZE) {
				i2c_regs[i2c_reg_addr] = i2c_data;
				i2c_reg_addr++; //next write will go to next register
				#ifdef I2C_DEBUG
				flag=1;
				#endif
			}
		}
		#ifdef I2C_DEBUG
		ser_putchar('2');
		ser_putchar('<');
		if(flag==0) {
			ser_putchar('@');
			ser_puthb(i2c_reg_addr);
		}else {
			ser_puthb(i2c_reg_addr-1);
			ser_putchar(':');
			ser_puthb(i2c_data);
		}
		ser_putchar('>');
		#endif
		goto i2c_slave_exit;
	}
	//--------------------
	// state 3: just got an address byte for a read transaction. ready to fill buffer for next read.
	//--------------------
	if( (i2c_status & I2C_SLAVE_STATE_3_MASK) == I2C_SLAVE_STATE_3) {
		#ifdef I2C_SLAVE_NEW
		//SSPxBUF must be read
		i2c_data = SSPxBUF;
		#endif
		if(i2c_reg_addr<I2C_REG_SIZE) {
			i2c_data = i2c_regs[i2c_reg_addr];
			i2c_reg_addr++;
		} else {
			i2c_data = 0xFF;
		}
		while(SSPxSTATbits.BF==1); //wait for previous byte to be read
		SSPxCON1bits.WCOL=0;
redowrite3:
		SSPxBUF = i2c_data;
		if(SSPxCON1bits.WCOL) goto redowrite3;
		//Release clock
		SSPxCON1bits.CKP = 1;

		#ifdef I2C_DEBUG
		ser_putchar('3');
		ser_putchar('<');
		ser_puthb(i2c_reg_addr-1);
		ser_putchar(':');
		ser_puthb(i2c_data);
		ser_putchar('>');
		#endif
		goto i2c_slave_exit;
	}
	//--------------------
	//state 4: a byte has just been read by the master. ready to fill buffer for next read.
	//--------------------
	if(i2c_status == I2C_SLAVE_STATE_4) {
		if(SSPxCON1bits.CKP) {
			//CKP=1 means state 5!
			#ifdef I2C_DEBUG
			ser_putchar('!');
			#endif
			goto i2c_slave_state_5;
		}
		if(i2c_reg_addr<I2C_REG_SIZE) {
			i2c_data = i2c_regs[i2c_reg_addr];
			i2c_reg_addr++;
		} else {
			i2c_data = 0xFF;
		}
		while(SSPxSTATbits.BF==1); //wait for previous byte to be read
		SSPxCON1bits.WCOL=0;
redowrite4:
		SSPxBUF = i2c_data;
		if(SSPxCON1bits.WCOL) goto redowrite4;

		//Release clock
		SSPxCON1bits.CKP = 1;

		#ifdef I2C_DEBUG
		ser_putchar('4');
		ser_putchar('<');
		ser_puthb(i2c_reg_addr-1);
		ser_putchar(':');
		ser_puthb(i2c_data);
		ser_putchar('>');
		#endif
		goto i2c_slave_exit;
	}
	//--------------------
	//state 5: just got a NACK from the master, this is the end of a read transaction
	//--------------------
i2c_slave_state_5:
	if( (i2c_status & I2C_SLAVE_STATE_5_MASK) == I2C_SLAVE_STATE_5) {
		#ifdef I2C_DEBUG
		ser_putchar('5');
		#endif
		goto i2c_slave_exit;
	}

	//error... just reset!
	#ifdef I2C_DEBUG
	ser_putchar('X');
	#endif
	//Reset();
	SSPxCON1bits.WCOL = 0;
	SSPxCON1bits.SSPOV = 0;
	SSPxCON1bits.SSPEN = 0;
	SSPxCON1bits.SSPEN = 1;
i2c_slave_exit:
	SSPxIF = 0; //ACK interrupt or the next one will to be triggered!
}


