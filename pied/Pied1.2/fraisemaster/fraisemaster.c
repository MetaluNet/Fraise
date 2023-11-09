/*********************************************************************
 *
 *                Fraise master firmware v2.1
 *
 *********************************************************************

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Copyright (c) Antoine Rousseau   2009-2023
 ********************************************************************/

#include <pic18f2550.h>
#include <usart.h>
#include <stdio.h>
#include "prj_usb_config.h"
#include "fraisemaster.h"
#include "eeprom.h"

#define VERSION_STRING (STRING)"sV UsbFraise 2.1.6 (SpareTimeLabs/SDCC) A.Rousseau 2023\n"

#define ID_EE_ADDRESS 0 //Address of ID in EEPROM

typedef const char *STRING;
typedef unsigned char BYTE;

BYTE LineFromUsb[LINE_FROM_USB_MAXLEN];
BYTE LineFromUsbLen;
BYTE FrGotLineFromUsb;

extern void putchar(unsigned char c);

#define CDC_Flush_In_Now usbcdc_flush
extern void usbcdc_flush();

extern union USART USART_Status;

//---------- FrTXPacket : Master to Device(s) -------------------------------------------
unsigned char FrTXpacket[64]; 	//Fraise TX packet buffer
unsigned char FrTXpacket_i;
unsigned char FrTXpacket_len;
unsigned char FrTXchksum;
unsigned char FrTXtries; 		//number of tries to send the TX packet

//---------- FrRX : Device to Master -------------------------------------------
unsigned char FrRXbuf[64]; 		//Fraise RX buffer:32 bytes
unsigned char FrRXin;
unsigned char FrRXout;
unsigned char FrRXchksum;
unsigned char PollDelay;
volatile unsigned char PollCount;

//---------- FraiseStatus bits -------------------------------------------
union {
	unsigned char VAL;
	struct {
		unsigned RXFULL : 1; 		// a RX packet has been received
		unsigned RXCHAR	: 1; 		// the RX packet is char (do not convert to hexa string)
		unsigned TXCHAR	: 1; 		// the TX packet was char (was not converted from hexa string)
		unsigned TX_NEEDACK : 1; 	// the TX packet needs an acknowledge
		unsigned FBLDON : 1;		// booloader mode
		unsigned OERR : 1;
		unsigned FERR : 1;
	};
} FraiseStatus;

//---------- finite state machine FraiseState ----------------------------
typedef enum {
	fIDLE
	, fWAITACK
	, fOUT
	, fIN
	, fBLOUT
	, fBLIN
} tFraiseState;
tFraiseState FraiseState;

//---------- FraiseMessage from interrupt routine -------------------------
typedef enum {
	fmessNONE
	, fmessFOUND 	// polled device has been found
	, fmessLOST 	// polled device has been lost
	, fmessCHKSUM 	// checksum error on a packet from polled device
	, fmessNACK 	// destination device refused packet (packet error | buffer full)
	, fmessTOUT 	// destination device didn't acknowledge packet (timeout)
} tFraiseMessage;
tFraiseMessage FraiseMessage;


//---------- Devices State tables ----------------------------------------
unsigned char _PolledChild; 	// id of the polled child
unsigned char _bit_PolledChild; // 1 << PolledChild % 8
unsigned char MaxPolledChild; 	// maximum id of the polled child
unsigned char Children[16]; 	// 16 * 8 = 128 bits: bit(Children[i],j)=child[i*8+j] is polled
unsigned char ChildrenOK[16]; 	// 16 * 8 = 128 bits: bit(Children[i],j)=child[i*8+j] is present
unsigned char AckChild; 		//child which must send a ACK now

#define incPolledChild() { \
		_PolledChild++; \
		if(_PolledChild>MaxPolledChild) {_PolledChild = 1; _bit_PolledChild = 2;} \
		else _bit_PolledChild = ((_bit_PolledChild << 1) | (_bit_PolledChild >> 7)); \
	}
#define PolledChild() _PolledChild

#define bitset(var,bitno) ((var) |= (1 << (bitno)))
#define bitclr(var,bitno) ((var) &= ~(1 << (bitno)))
#define bittst(var,bitno) ((var) & (1 << (bitno)))

#define SET_CHILD(num) bitset(Children[(num)>>3],((num)&7))
#define CLR_CHILD(num) bitclr(Children[(num)>>3],((num)&7))
#define TST_CHILD(num) bittst(Children[(num)>>3],((num)&7))

#define SET_CHILDOK(num) bitset(ChildrenOK[(num)>>3],((num)&7))
#define CLR_CHILDOK(num) bitclr(ChildrenOK[(num)>>3],((num)&7))
#define TST_CHILDOK(num) bittst(ChildrenOK[(num)>>3],((num)&7))

#define SET_POLLEDCHILD() ( Children[_PolledChild>>3]|= _bit_PolledChild )
#define CLR_POLLEDCHILD() ( Children[_PolledChild>>3]&= ~_bit_PolledChild )
#define TST_POLLEDCHILD() ( Children[_PolledChild>>3]& _bit_PolledChild )

#define SET_POLLEDCHILDOK() ( ChildrenOK[_PolledChild>>3]|= _bit_PolledChild )
#define CLR_POLLEDCHILDOK() ( ChildrenOK[_PolledChild>>3]&= ~_bit_PolledChild )
#define TST_POLLEDCHILDOK() ( ChildrenOK[_PolledChild>>3]& _bit_PolledChild )

//-------------- byte to HEX string -----------------------------------
#define HI_CHAR(N) ( ((N)>>4)<10?((N)>>4)+'0':((N)>>4)-10+'A' )
#define LO_CHAR(N) ( ((N)&15)<10?((N)&15)+'0':((N)&15)-10+'A' )


//---------------serial macros ---------------------------------------
void Serial_Init_Receiver(void)
{
	while(TXSTAbits.TRMT == 0);
	WREG = RCREG;
	WREG = RCREG;
	RCSTAbits.CREN = 0;
	RCSTAbits.CREN = 1;
	PIE1bits.RCIE = 1;
	PIE1bits.TXIE = 0;
	mSerDrv_Off();
}

#define Serial_Init_Driver() {\
	RCSTAbits.CREN=0;	\
	mSerDrv_On();		\
	PIE1bits.RCIE=0;		\
}

#define Serial_Init_None() {\
	RCSTAbits.CREN=0;	\
	mSerDrv_Off();		\
	PIE1bits.RCIE=0;        \
	PIE1bits.TXIE=0;        \
}

#define Serial_Is_Receiver() (PIE1bits.RCIE)
//---------------------------------------------------------------------

void FraiseInit(void)
{
	FrGotLineFromUsb = 0;
	LineFromUsbLen = 0;

	FraiseStatus.VAL = 0;
	FraiseState = fIDLE;
	FraiseMessage = fmessNONE;
	FrRXin = 0;
	FrRXout = 0;
	PollDelay = 0;
	BAUDCON = 0x08;     // BRG16 = 1
	usart_open(USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_NINE_BIT & USART_CONT_RX & USART_BRGH_HIGH , 47);  // 48 MHz/4/250000 = 48

	USART_Status.TX_NINE = 1;

	// Initialize Timer2
	T2CON = 0;
	// The prescaler will be at 16
	T2CONbits.T2CKPS1 = 1;
	T2CONbits.T2CKPS0 = 1;
	PR2 = 255;
	T2CONbits.TMR2ON = 1;

	//Serial_Init_Receiver();
	Serial_Init_Driver();
	for(int i = 0; i < 16; i++) {
		Children[i] = 0;
		ChildrenOK[i] = 0;
	}
	MaxPolledChild = 4;
	_PolledChild = 1;
	_bit_PolledChild = 2;

	// Set interrupt priorities
	PIE1bits.TMR2IE = 0;

	IPR1bits.TMR2IP = 1;
	IPR1bits.TXIP = 1;
	IPR1bits.RCIP = 1;

	INTCONbits.GIEH = 1;
}

/*-----------------------------------------------------------------------------------*/
/*          Send received fraise packet to usb.                                      */
/*-----------------------------------------------------------------------------------*/

void FrSendtoUsb(void)
{
	unsigned char i = 0, c, c2;

	if(!FraiseStatus.RXFULL) return;

	c = FrRXbuf[i++]; //discard len byte

	// send hex string of PolledChild+128*packet_is_char

	c2 = (PolledChild() >> 4);
	if(FraiseStatus.RXCHAR) c2 |= 8;
	c2 += '0';
	if(c2 > '9') c2 += 'A' - '9' - 1;
	putchar(c2);

	c2 = (PolledChild() & 15) + '0';
	if(c2 > '9') c2 += 'A' - '9' - 1;
	putchar(c2);

	if(FraiseStatus.RXCHAR)
		while(i < FrRXin) putchar(FrRXbuf[i++]);
	else
		while(i < FrRXin) {
			c = FrRXbuf[i++];
			c2 = (c >> 4) + '0';
			if(c2 > '9') c2 += 'A' - '9' - 1;
			putchar(c2);
			c2 = (c & 15) + '0';
			if(c2 > '9') c2 += 'A' - '9' - 1;
			putchar(c2);
		}

	putchar('\n');
	FrRXout = FrRXin = 0;
	FraiseStatus.RXFULL = 0;
}
/*-----------------------------------------------------------------------------------*/
/*          Send fraise ISR message to usb.                                          */
/*-----------------------------------------------------------------------------------*/
void FrSendMessagetoUsb(void)
{
	/*	 fmessNONE
		,fmessFOUND  // polled device has been found
		,fmessLOST   // polled device has been lost
		,fmessCHKSUM // checksum error on a packet from polled device
		,fmessNACK   // destination device didn't acknowledge packet (packet error | buffer full)
		,fmessTOUT   // destination device didn't acknowledge packet (timeout)
	*/
	if(FraiseStatus.OERR) {
		FraiseStatus.OERR = 0;
		printf((STRING)"OERR !!\n");
	}
	if(FraiseStatus.FERR) {
		FraiseStatus.FERR = 0;
		printf((STRING)"FERR !!\n");
	}

	if(FraiseMessage == fmessNONE) return;
	switch(FraiseMessage) {
	case fmessFOUND : // print "sCdd"
		if(!TST_CHILDOK(AckChild)) {
			SET_CHILDOK(AckChild);
			putchar('\n');
			putchar('s');
			putchar('C');
			putchar(HI_CHAR(AckChild));
			putchar(LO_CHAR(AckChild));
			putchar('\n');
		}
		break;
	case fmessLOST : // print "scdd"
		if(TST_CHILDOK(AckChild)) {
			CLR_CHILDOK(AckChild);
			putchar('\n');
			putchar('s');
			putchar('c');
			putchar(HI_CHAR(AckChild));
			putchar(LO_CHAR(AckChild));
			putchar('\n');
		}
		break;
	case fmessCHKSUM : // print "sxdd"
		putchar('\n');
		putchar('s');
		putchar('x');
		putchar(HI_CHAR(AckChild));
		putchar(LO_CHAR(AckChild));
		putchar('\n');
		break;
	case fmessNACK :// print "sadd"
		putchar('\n');
		putchar('s');
		putchar('a');
		putchar(HI_CHAR(AckChild));
		putchar(LO_CHAR(AckChild));
		putchar('\n');
		break;
	case fmessTOUT :// print "sTdd"
		putchar('\n');
		putchar('s');
		putchar('T');
		putchar(HI_CHAR(AckChild));
		putchar(LO_CHAR(AckChild));
		putchar('\n');
		break;
	}
	FraiseMessage = fmessNONE;
}

/*-----------------------------------------------------------------------------------*/
/*          Analyse packet from usb, build a packet to send to fraise if applicable. */
/*-----------------------------------------------------------------------------------*/

#define FrTXPacketInit(b) { FrTXpacket_i=1 ; FrTXchksum=FrTXpacket[0]=(b);}
#define FrTXPacketData(b) { FrTXchksum+=FrTXpacket[FrTXpacket_i]=(b);FrTXpacket_i++;}
#define FrTXPacketClose() { FrTXpacket[FrTXpacket_i]=-(char)FrTXchksum; FrTXpacket_len=FrTXpacket_i+1; }

#define FrTXPacketLaunch()			\
{									\
	Serial_Init_Driver();			\
	TXSTAbits.TX9D=1;				\
	TXREG=AckChild=FrTXpacket[0];	\
	TXSTAbits.TX9D=0;				\
	FrTXpacket_i=1;					\
	FraiseState=fOUT;				\
	PIE1bits.TXIE=1;				\
}

#define FrTXPacketLaunchBl()	\
{								\
	Serial_Init_Driver();		\
	TXREG=FrTXpacket[0];		\
	FrTXpacket_i=1;				\
	FraiseState=fBLOUT;			\
	PIE1bits.TXIE=1;			\
}

#define GETNEXTCHAR() LineFromUsb[i++]
#define PEEKNEXTCHAR() LineFromUsb[i]
#define SKIPNEXTCHAR() i++
#define LINE_HAS_CHAR() (i<LineFromUsbLen)

void FrGetLineFromUsb(void)
{
	unsigned char len, c, c2, n;
	unsigned char i;

	if(!FrGotLineFromUsb) goto discard;

	FraiseStatus.TX_NEEDACK = 0;
	FraiseStatus.TXCHAR = 0;

	len = LineFromUsbLen;
	i = 0;

	c = GETNEXTCHAR(); // 1st byte = command (or hi nibble of address)

	if(c == '#') {
	// ****************** system command , begining by '#':   **********************
		if(len < 2) goto discard;
		c = GETNEXTCHAR(); // what is the command ?
		if(c == 'S') { // start device pulling
			if(len != 4) goto discard; //incorrect  packet...
			c = GETNEXTCHAR();
			c -= '0';
			if (c > 9) c -= 'A' - '9' - 1;
			c2 = GETNEXTCHAR();
			c2 -= '0';
			if (c2 > 9) c2 -= 'A' - '9' - 1;
			if((c > 15) || (c2 > 15)) goto discard; // invalid packet
			n = c2 + (c << 4);
			if((n > 0) && (n < 128)) {
				SET_CHILD(n);
				CLR_CHILDOK(n);
				if(MaxPolledChild < n) MaxPolledChild = n;
			}
			goto discard;
		}
		else if(c == 'C') { // stop device pulling
			if(len != 4) goto discard; // incorrect  packet...
			c = GETNEXTCHAR();
			c -= '0';
			if (c > 9) c -= 'A' - '9' - 1;
			c2 = GETNEXTCHAR();
			c2 -= '0';
			if (c2 > 9) c2 -= 'A' - '9' - 1;
			if((c > 15) || (c2 > 15)) goto discard; // invalid packet
			n = c2 + (c << 4);
			if((n > 0) && (n < 128)) {
				CLR_CHILDOK(n);
				CLR_CHILD(n);
			}
			goto discard;
		}
		else if(c == 'i') {
			printf((STRING)"s fraise init...\n");
			FraiseInit();
			goto discard;
		}
		else if(c == 'L') { // get log
			printf((STRING)"\ns fraise log :\n");
			c2 = 0;
			printf((STRING)"FraiseState : %d ; FraiseStatus: %d\n", (int)FraiseState, (int)FraiseStatus);
			for(n = 1; n <= MaxPolledChild; n++) {
				if (TST_CHILD(n)) {
					printf((STRING)"%d:%d ", n, TST_CHILDOK(n) != 0);
					if((++c2) % 16 == 0) putchar('\n');
				}
			}
			putchar('\n');
			goto discard; // return;
		}
		else if(c == 'F') { // quit bootloader mode
			FraiseStatus.FBLDON = 0;
			FraiseState = fIDLE;
			Serial_Init_None();
			printf((STRING)"Quit bootloader mode.\n");
			goto discard; // return;
		}
		else if(c == 'V') { // get version string
			printf(VERSION_STRING);
			goto discard; // return;
		}
		else if(c == 'R') { // read id
			n = ee_read_byte(ID_EE_ADDRESS);
			printf ((STRING)"sID%c%c\n", HI_CHAR(n), LO_CHAR(n));
		}
		else if(c == 'W') { // write id
			c = GETNEXTCHAR();
			c -= '0';
			if (c > 9) c -= 'A' - '9' - 1;
			c2 = GETNEXTCHAR();
			c2 -= '0';
			if (c2 > 9) c2 -= 'A' - '9' - 1;
			if((c > 15) || (c2 > 15)) goto discard; // invalid packet
			n = c2 + (c << 4);
			ee_write_byte(ID_EE_ADDRESS, n);
		}
		else if(c == 'E') { // echo
			while(LINE_HAS_CHAR()) {
				c = GETNEXTCHAR();
				putchar(c);
			}
			putchar('\n');
		}
		else if(c == 'D') { // delay between each device polling (ms)
			if(len != 4) goto discard; // incorrect  packet...
			c = GETNEXTCHAR();
			c -= '0';
			if (c > 9) c -= 'A' - '9' - 1;
			c2 = GETNEXTCHAR();
			c2 -= '0';
			if (c2 > 9) c2 -= 'A' - '9' - 1;
			if((c > 15) || (c2 > 15)) goto discard; // invalid packet
			n = c2 + (c << 4);
			PollDelay = n;
		}
		goto discard; // unknown system command ; discard packet.
	}
	else if(c == '!') {
	// ****************** broadcast tx , begining by '!':  *****************
		len -= 1; // discard '!' byte
		if(len < 1) goto discard;

		FrTXPacketInit(0);

		c = PEEKNEXTCHAR(); //what is the command ?

		if(c == 'B' || c == 'I' || c == 'N' || c == 'R' || c == 'P' || c == 'F') {
			FraiseStatus.TXCHAR = 1;

			if(c == 'F') {
				FraiseStatus.FBLDON = 1;
				printf((STRING)"Enter bootloader mode.\n");
			}
			else FraiseStatus.FBLDON = 0;
			goto fill_packet;
		}
		if(c == 'b') {
			SKIPNEXTCHAR();
			len -= 1; // discard 'b' byte
			goto fill_packet;
		}

		goto discard;
	}

	if(FraiseStatus.FBLDON) {
	// ****************** message is to be sent to fruit's bootloader *****************
		FrTXPacketInit(len + 1);
		FrTXPacketData(c);

		while(LINE_HAS_CHAR()) FrTXPacketData(GETNEXTCHAR());
		FrTXPacketClose();
		FrTXtries = 0;
		FrTXPacketLaunchBl();
		goto discard;
	}

	// else
	// ****************** normal master to device tx *****************
	// the 2 first bytes must be device id + 128 if it is a char packet:
	if(len < 3) goto discard;

	c -= '0';
	if (c > 9) c -= 'A' - '9' - 1;

	c2 = GETNEXTCHAR();

	c2 -= '0';
	if (c2 > 9) c2 -= 'A' - '9' - 1;
	if((c > 15) || (c2 > 15)) goto discard; // invalid id
	n = c2 + (c << 4);

	if(n & 128) FraiseStatus.TXCHAR = 1;
	else FraiseStatus.TXCHAR = 0;

	n &= 127;
	if (n == 0) goto discard;
	FrTXPacketInit(n);

	len -= 2; // remove the two id bytes
	FraiseStatus.TX_NEEDACK = 1;


fill_packet:
	if(FraiseStatus.TXCHAR) len |= 128;
	else len >>= 1; // if not TXCHAR, data len in serial stream will be half than at text input (e.g two text bytes "00" -> one null byte )
	FrTXPacketData(len);

	if(FraiseStatus.TXCHAR) {
		while(LINE_HAS_CHAR()) FrTXPacketData(GETNEXTCHAR());
	}
	else while(LINE_HAS_CHAR()) {
			c = GETNEXTCHAR();
			c -= '0';
			if (c > 9) c -= 'A' - '9' - 1;
			if(!LINE_HAS_CHAR()) goto discard; // incomplete...
			c2 = GETNEXTCHAR();
			c2 -= '0';
			if (c2 > 9) c2 -= 'A' - '9' - 1;
			if((c > 15) || (c2 > 15)) goto discard; // invalid hex string
			c2 += (c << 4);
			FrTXPacketData(c2);
		}
	FrTXPacketClose();
	FrTXtries = 0;
	FrTXPacketLaunch();

discard:
	LineFromUsbLen = 0;
	FrGotLineFromUsb = 0;
	return;
}

//------------- time constants and macros --------------------------------------
#define T_2SERBYTES				(256UL - ((65UL * 12UL) / 16UL))		// For 80 us TMR2 tick , 20 * 4 us, 15 us before
#define T_1ms					(256UL - ((600UL * 12UL / 3UL) / 16UL))	// postscaler = 3
#define InitTimer1ms() {PIE1bits.TMR2IE = 0; T2CON = 31; TMR2 = T_1ms; PIR1bits.TMR2IF = 0;}
#define ResetTimer1ms() {TMR2 = T_1ms; PIR1bits.TMR2IF = 0;}
#define InitTimer(time) {PIE1bits.TMR2IE = 0; T2CON = 7; /*no post*/ TMR2 = time; PIR1bits.TMR2IF = 0; PIE1bits.TMR2IE = 1;}
#define TimerOut() (PIR1bits.TMR2IF)
#define StopTimer() {T2CON = 3; PIE1bits.TMR2IE = 0; /*stop tmr2*/ PIR1bits.TMR2IF = 0;}

void FraiseISR()
{
	unsigned char c;

	if(RCSTAbits.OERR) FraiseStatus.OERR = 1;
	if(RCSTAbits.FERR) FraiseStatus.FERR = 1;

	if(FraiseState == fWAITACK)
	{
		if (!Serial_Is_Receiver()) {
			if (TimerOut()) {
				Serial_Init_Receiver();
				InitTimer1ms();
			}
			return;
		}
		if(PIR1bits.RCIF) {
			StopTimer();
			c = RCREG;
			Serial_Init_None();
			if(c != 0) { // nACK
				if(++FrTXtries < 3) {
					FrTXPacketLaunch(); // resend packet maximum 3 times
					return;
				}
				else {
					FraiseMessage = fmessNACK;
				}
			}
			FraiseState = fIDLE;
			return;
		}
		return;
	}
	else if(FraiseState == fIN)
	{
		if (!Serial_Is_Receiver()) {
			if (TimerOut()) {
				Serial_Init_Receiver();
				InitTimer1ms();
			}
			return;
		}
		if(PIR1bits.RCIF) {
			c = RCREG;
			FrRXbuf[FrRXin] = c;
			FrRXin++;
			ResetTimer1ms();
			if(FrRXin == 1) { // first byte
				FrRXchksum = 0;
				FrRXout = (c & 63); // get length
				FrRXout++;
				if(c == 0) { // device answered : nothing to say
					StopTimer();
					Serial_Init_None();
					FraiseState = fIDLE;
					FraiseMessage = fmessFOUND;
				} else {
					if(c & 128) FraiseStatus.RXCHAR = 1;
					else FraiseStatus.RXCHAR = 0;
				}
			}
			FrRXchksum += c;

			if(FrRXin > FrRXout) { // end of packet
				StopTimer();
				Serial_Init_None();
				FraiseState = fIDLE;
				if(!FrRXchksum) { // checksum ok
					FrRXin--; // discard checksum
					FraiseStatus.RXFULL = 1;
					FraiseMessage = fmessFOUND;
					TXREG = 0; // ACK
				} else { // checksum error
					FraiseMessage = fmessCHKSUM;
					TXREG = 1; //NACK
				}
			} // endif 'end of packet'
			return;
		} // endif(PIR1bits.RCIF)

		return;
	} // endif (FraiseState == fIn)
	else if(FraiseState == fOUT) // if we are sending a packet to a child:
	{
		if (TimerOut() && (FrTXpacket_i == FrTXpacket_len)) { // end of transmission
			StopTimer();
			FraiseState = fIDLE; // return to IDLE state.
			Serial_Init_Receiver();
			Serial_Init_None();
			return;
		}
		if(PIR1bits.TXIF == 0) return;
		StopTimer();
		TXREG = FrTXpacket[FrTXpacket_i++]; // send next byte
		if(FrTXpacket_i == FrTXpacket_len) { // if end of the packet:
			PIE1bits.TXIE = 0;
			if(FraiseStatus.TX_NEEDACK) {
				InitTimer(T_2SERBYTES); // 2 bytes wait inside of hardware serial tx buffer
				FraiseState = fWAITACK; 	// goto to WAITACK state.
			}
			else {
				if(FraiseStatus.FBLDON) {
					Serial_Init_Receiver();
					FraiseState = fBLIN;
				}
				else {
					InitTimer(T_2SERBYTES);
				}
			}
		}
		return;
	}
	else if(FraiseState == fBLOUT) // if we are sending a bootloader packet :
	{
		StopTimer();
		if(PIR1bits.TXIF == 0) return;
		TXREG = FrTXpacket[FrTXpacket_i++]; // send next byte
		if(FrTXpacket_i == FrTXpacket_len) { // if end of the packet:
			PIE1bits.TXIE = 0;
			InitTimer(T_2SERBYTES); // 2 bytes wait inside of hardware serial tx buffer
			FraiseState = fBLIN; // goto to BLIN state.
			FrRXin = FrRXout = 0;
		}
		return;
	}	// endif(FraiseState == fBLOUT)
	else if(FraiseState == fBLIN)
	{
		if (!Serial_Is_Receiver()) {
			if (TimerOut()) {
				Serial_Init_Receiver();
				StopTimer();
			}
			return;
		}
		if(PIR1bits.RCIF) {
			c = RCREG;
			FrRXbuf[FrRXin] = c;
			FrRXin++;
			return;
		}
		return;
	}	// endif(FraiseState == fBLIN)
	else if(FraiseState == fIDLE)	// shouldn't happen... clear interrupt flags:
	{
		StopTimer();
		PIE1bits.TXIE = 0;
		PIE1bits.RCIE = 0;
		return;
	}

} // end of LowISR

void FraiseService(void)
{
	FrSendMessagetoUsb();

	if(FraiseState == fBLIN) {
		if(FrRXin != FrRXout) {
			while(FrRXin != FrRXout) putchar(FrRXbuf[FrRXout++]);
			CDC_Flush_In_Now();
		}

		if(FrGotLineFromUsb) FrGetLineFromUsb();
		goto _fin;
	}

	INTCONbits.GIEH = 0;

	if(FraiseState == fIDLE) {
		StopTimer();

		if(FrGotLineFromUsb) {
			FrGetLineFromUsb();
			goto _fin;
		}
		// goto _fin;
		if(!FraiseStatus.RXFULL) {
			if(PollCount >= PollDelay) {
				PollCount = 0;
				incPolledChild();
				if(TST_POLLEDCHILD())
				{
					FrRXchksum = FrRXin = FrRXout = 0;
					TXSTAbits.TX9D = 1;

					Serial_Init_Driver();
					FrSendMessagetoUsb();

					INTCONbits.GIEL = 0;
					AckChild = PolledChild();
					TXREG = (PolledChild() | 128);
					TXSTAbits.TX9D = 0;
					TXREG = (PolledChild() | 128);
					InitTimer(T_2SERBYTES);
					FraiseState = fIN;
					INTCONbits.GIEL = 1;
				}
			}
		}
	} else if(FraiseState == fWAITACK) {
		if((Serial_Is_Receiver()) && TimerOut() ) { // didn't rcved ACK before Timeout
			StopTimer();
			Serial_Init_None();
			if(++FrTXtries < 3) {
				FrTXPacketLaunch();    // resend packet maximum 3 times
			}
			else
			{
				FraiseMessage = fmessTOUT;
				FraiseState = fIDLE;
			}
		}
	} else if(FraiseState == fIN) {
		if( (Serial_Is_Receiver()) && TimerOut() ) { // timeout
			StopTimer();
			FraiseMessage = fmessLOST;
			Serial_Init_None();
			FraiseState = fIDLE;
		}
	}

_fin :
	INTCONbits.GIEH = 1;

	FrSendtoUsb();
}

void FraiseSOF(void)
{
	static BYTE il;

	if((il++) == 200) {
		il = 0;
		mLED_2_Toggle();
	}

	if(++PollCount == 0) PollCount = 255;
}

