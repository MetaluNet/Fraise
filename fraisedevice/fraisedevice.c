/*********************************************************************
 *
 *                Fraise device firmware v2.1
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
 * Copyright (c) Antoine Rousseau   2009-2015   
 ********************************************************************/
 
//#include <config.h>
//#include <pic18fregs.h>
//#include <boardconfig.h>
//#include <boardio.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
//#include <typedefs.h>
//#include <ctype.h>
//#include <typedefs.h>

#include "fraisedevice.h"

//---------------  Serial macros :   -----------------
//serial drive:
#if 1
#define	InitSerDrv()	do { SERDRV_PIN = 0; SERDRV_TRI = 1; } while(0)
#define	SerDrv_On()		do { SERDRV_TRI = SERDRV_POL; } while(0)
#define	SerDrv_Off()	do { SERDRV_TRI =! SERDRV_POL; } while(0)
#define	SerDrv_isOn()	(SERDRV_TRI == SERDRV_POL)
#else
#define	InitSerDrv()	do { SERDRV_PIN = 1; SERDRV_TRI = 0; } while(0)
#define	SerDrv_On()		do { SERDRV_PIN = SERDRV_POL; } while(0)
#define	SerDrv_Off()	do { SERDRV_PIN =! SERDRV_POL; } while(0)
#define	SerDrv_isOn()	(SERDRV_PIN == SERDRV_POL)
#endif

//serial port:
#if UART_PORT==1
#define SPBRGx 			SPBRG1
#define SPBRGHx 		SPBRGH1
#define BAUDCONx 		BAUDCON1
#define BAUDCONxbits 	BAUDCON1bits
#define RCREGx 			RCREG1
#define RCSTAx 			RCSTA1
#define RCSTAxbits 		RCSTA1bits
#define TXREGx 			TXREG1
#define TXSTAx 			TXSTA1
#define TXSTAxbits 		TXSTA1bits
#define RCxIF	 		PIR1bits.RC1IF
#define TXxIF	 		PIR1bits.TX1IF
#define RCxIE	 		PIE1bits.RC1IE
#define TXxIE	 		PIE1bits.TX1IE
#define RCxIP	 		IPR1bits.RC1IP
#define TXxIP	 		IPR1bits.TX1IP
#else
#define SPBRGx 			SPBRG2
#define SPBRGHx 		SPBRGH2
#define BAUDCONx 		BAUDCON2
#define BAUDCONxbits 	BAUDCON2bits
#define RCREGx 			RCREG2
#define RCSTAx 			RCSTA2
#define RCSTAxbits 		RCSTA2bits
#define TXREGx 			TXREG2
#define TXSTAx 			TXSTA2
#define TXSTAxbits 		TXSTA2bits
#define RCxIF	 		PIR3bits.RC2IF
#define TXxIF	 		PIR3bits.TX2IF
#define RCxIE	 		PIE3bits.RC2IE
#define TXxIE	 		PIE3bits.TX2IE
#define RCxIP	 		IPR3bits.RC2IP
#define TXxIP	 		IPR3bits.TX2IP
#endif

//(1 port equ)
#ifndef BAUDCON1
#define SPBRG1 			SPBRG
#define SPBRGH1 		SPBRGH
#define BAUDCON1 		BAUDCON
#define BAUDCON1bits 	BAUDCONbits
#define RCREG1 			RCREG
#define RCSTA1 			RCSTA
#define RCSTA1bits 		RCSTAbits
#define TXREG1 			TXREG
#define TXSTA1 			TXSTA
#define TXSTA1bits 		TXSTAbits
#define RC1IF 			RCIF
#define TX1IF 			TXIF
#define RC1IE 			RCIE
#define TX1IE 			TXIE
#define RC1IP 			RCIP
#define TX1IP 			TXIP
#endif

static void Serial_Init_Receiver() {
	while(TXSTAxbits.TRMT == 0);
	SerDrv_Off();
	WREG=RCREGx;
	__asm nop __endasm ;	
	WREG=RCREGx;
	RCSTAxbits.CREN = 0;
	RCSTAxbits.CREN = 1;
	RCSTAxbits.ADDEN = 1;
	RCxIE = 1;
	TXxIE = 0;
}

#define Serial_Init_Driver() do {\
	RCxIE = 0;			\
	RCSTAxbits.CREN = 0;\
	SerDrv_On();		\
} while(0)

#define Serial_Is_Driver() SerDrv_isOn()

//---------- FrTX : App to Device  -------------------------------------------
unsigned char FrTXbuf[256]; //Fraise TX ring buffer
unsigned char FrTXin = 0,FrTXout = 0,FrTXbufFree = 255; //Pointers to Fraise TX buffer
#define FrTXempty (FrTXin == FrTXout)

//---------- fraiseSend : Device to Master -------------------------------------------
unsigned char FrTXpacket[35]; //Fraise TX packet buffer
unsigned char FrTXpacket_i;
unsigned char FrTXpacket_len;
unsigned char FrTXchksum;
unsigned char FrTXtries; //number of tries to send the TX packet

//---------- FrRX : Master to device -------------------------------------------
unsigned char FrRXbuf[128]; //Fraise RX ring buffer
unsigned char FrRXchksum;
unsigned char FrRXin; //index following last validated packet
unsigned char FrRXin_tmp; //index following last received byte==next address to write to
unsigned char FrRXin_end; //index that will follow incoming packet
unsigned char FrRXout; //index of next byte to read from buffer
unsigned char FrRXout_end; //index following index of last byte of currently scanned packet
unsigned char FrRXout_len; //length of currently scanned packet
unsigned char FrRXout_first; //index of 1st byte of currently scanned packet

/*
	packets in FrRXbuf ring buffer: 
		1st byte=length+128*buffer_was_char, then _length_ bytes of data.
		when writing to buffer, if FrRXin_tmp==FrRXout then next write would overwrite some unread data.
		when reading from buffer, if FrRXin==FrRXout then there's nothing more to read.
*/


//---------- FraiseStatus bits -------------------------------------------
union {
	unsigned char VAL;
	struct {
		unsigned RX_OERR :1 ; // an OVERRUN ERROR occured on fraise RX side (firmware timing issue)
		unsigned RX_FERR :1 ; // an FRAME ERROR occured on fraise RX side (too much noise on physical transport, 
							  // e.g. bad connection, bug from another device...)
		unsigned RX_SERR :1 ; // an checksum error occured on fraise RX side (noise, bug...)
		
		//unsigned RX_BFUL :1 ; // couldn't get rx packet : buffer was full.
		
		unsigned RX_BRDCST :1 ; //current RX is broadcast
		
		unsigned RX_POLL :1 ; //current RX is poll signal
		/*unsigned TX_NACK :1 ; // master returned a NACK : checksum error on TX (noise, bug...)
		unsigned TX_NOACK :1 ; // master didn't acknowledge at all (noise, bug...)*/
		unsigned TX_ERR :1 ; //couldn't achieve to transmit data to master (noise, bug...)
	};
} FraiseStatus;

char FrInterruptEnabled = 0;
//---------- eeprom addresses ----------------------------
#define EE_ID 0
#define EE_PREFIX 1
#define EE_PREFIXMAXLEN 8
#define EE_NAME (EE_PREFIX + EE_PREFIXMAXLEN + 1)
#define EE_NAMEMAXLEN 16
#define EE_NAMEMAX (EE_NAME + EE_NAMEMAXLEN)

//---------- finite state machine FraiseState ----------------------------
typedef enum {
	fIDLE
	,fWAITACK
	,fOUT
	,fIN
} tFraiseState;
tFraiseState FraiseState;


//---------- Devices State tables ----------------------------------------	
unsigned char FrID=0; // device fraise id

/*#define bitset(var,bitno) ((var) |= (1 << (bitno)))
#define bitclr(var,bitno) ((var) &= ~(1 << (bitno)))
#define bittst(var,bitno) (var & (1 << (bitno)))*/

void fraiseSetID(unsigned char id)
{
	eeWriteByte(EE_ID,FrID = id);
}

void fraiseInit(void)
{
	FrTXin = 0;
	FrTXout = 0;
	FrTXbufFree = 255;
	
	//SERIAL:
	SerDrv_Off();
	InitSerDrv();

//baud rate : br = FOSC/[4 (n+1)] : n = FOSC / (4 * br) - 1 : br = 250kHz, n = FOSC/1000000 - 1
#define BRGHL (FOSC/1000000 - 1)
	SPBRGHx = BRGHL/256;
	SPBRGx = BRGHL%256;

	BAUDCONxbits.BRG16 = 1;

	TXSTAxbits.TXEN = 1;
	TXSTAxbits.BRGH = 1;
	TXSTAxbits.TX9 = 1;
	TXSTAxbits.TX9D = 0;

	RCSTAxbits.RX9 = 1;
	RCSTAxbits.SPEN = 1;

	FraiseStatus.VAL = 0;
	FraiseState = fIDLE;
	FrRXin = 0;
	FrRXout = 0;
	FrTXpacket_len = 0;
	
	FrID = eeReadByte(EE_ID);

    	// set serial interrupts to low priority
	TXxIP = 0;
	RCxIP = 0;
	FrInterruptEnabled = 1;
	
	// Use our own special output function for STDOUT
	stdout = STREAM_USER;
	
	Serial_Init_Receiver();
}

void fraiseSetInterruptEnable(char enable)
{
	FrInterruptEnabled = enable;
	if(enable == 0) {
		Serial_Init_Receiver();
		RCxIE = 0;
		TXxIE = 0;
	}
}

char fraiseGetInterruptEnable(void)
{
	return FrInterruptEnabled;
}

//----------------- RX buffer utils : --------------------------------------------


static void fraisePutChar(unsigned char c)
{
	if((FrTXin + 1) == FrTXout) {
		//printf("!50 Err: undetected FrTX full !!!\r\n");
		return; //overflow check done by fraiseSend, but...
	}
	FrTXbuf[FrTXin] = c;
	FrTXin++;
}

static unsigned char fraiseGetTXChar()
{
	unsigned char c;
	
	if(FrTXin == FrTXout) return 0; //underflow check
	c = FrTXbuf[FrTXout];
	FrTXout++;
	return c;
}

char fraiseSend(const unsigned char *buf,unsigned char len)
{
	unsigned char i,c;
	
	FrTXbufFree = FrTXout;
	FrTXbufFree -= FrTXin;
	FrTXbufFree -= 1;
	
	if ((len + 3) > FrTXbufFree) {
		//printf("!10 Err: Fraise TXbuf full\r\n");
		return -1;
	}
	if (len < 2) {
		//printf("!11 Err: null Fraise TXpacket\r\n");
		return 0;
	}
	len -= 1; //don't keep '\n' terminator
	fraisePutChar(len); //prefix tx packet buffer by its length
	i = 0;
	while(i < len) { //fill tx packet buffer
		c = buf[i];
		i++;
		fraisePutChar(c);
	}

	//FrTXbufFree = ((int)FrTXout - (int)FrTXin + (sizeof(FrTXbuf) - 1)) %sizeof(FrTXbuf);
	FrTXbufFree = FrTXout;
	FrTXbufFree -= FrTXin;
	FrTXbufFree -= 1;
	return 0;
}

#define fraiseSendInit(b) do { FrTXpacket_i = 0 ; FrTXchksum = (b); FrTXpacket[FrTXpacket_i] = (b); FrTXpacket_i++; } while(0)
#define fraiseSendData(b) do { FrTXchksum += (b); FrTXpacket[FrTXpacket_i] = (b); FrTXpacket_i++; } while(0)
#define fraiseSendClose() do { FrTXpacket[FrTXpacket_i] = -FrTXchksum; FrTXpacket_i++; FrTXpacket_len = FrTXpacket_i; } while(0)

#define fraiseSendLaunch() \
{		\
	TXREGx = FrTXpacket[0]; \
	FrTXpacket_i = 1;		\
	TXxIE = 1;				\
}

static void fraiseDecodeNextTXPacket()
{
	unsigned char len, c, txout_end, ischar;
	
	if(TXSTAxbits.TRMT == 0) return; //return if a serial transmission is in progress

	len = fraiseGetTXChar(); //1st byte = len
	if(!len) {
		//printf("!12 Err: TXbuffer inconsistency !\n");		
		FrTXin = FrTXout=0;
		return; //?
	}
	
	txout_end = FrTXout;
	txout_end += len;
	
	c=fraiseGetTXChar(); //2nd byte = command (or hi nibble of address)
	len -= 1;
	
	if(c == '#') {						
		//****************** system command , begining by '#':   **********************
		if(len < 1) goto discard;
		c = fraiseGetTXChar(); //what is the command ?
		if(c == 'i'){
			//printf("s fraise init...\n");
			fraiseInit();
			goto discard;
		}
		/*else if(c=='l'){
			printf("flog:stat=%d,rxout=%d,rxin=%d,adden=%d,cren=%d,rcie=%d,rcif=%d\n",
				FraiseStatus.VAL,FrRXout,FrRXin,RCSTAbits.ADDEN,RCSTAbits.CREN,PIE1bits.RCIE,PIR1bits.RCIF);
			goto discard;
		}*/
		else if(c == 'r'){
			Serial_Init_Receiver();
			goto discard;
		}
		else if(c == 's'){
			//FraiseStatus.VAL=0;
			SerDrv_Off();
			RCSTAxbits.ADDEN = 1; 
			RCxIE = 1;		
			TXxIE = 0;
			c = RCREGx;	
			c = RCREGx;	
			RCSTAxbits.CREN = 0;		
			RCSTAxbits.CREN = 1;		

			goto discard;
		}
		
		goto discard; //unknown system command ; discard packet.
	}	

	if(c == 'C') { //"char" packet
		ischar = 1;
		goto fill_packet;
	}
	
	if(c == 'B') { //"bytes" packet
		ischar = 0;
		goto fill_packet;
	}
	
	goto discard;
	
fill_packet:
		if(ischar) len |= 128;

		fraiseSendInit(len);
		
		while(FrTXout != txout_end) {
			c = fraiseGetTXChar();
			fraiseSendData(c); 
		}
		fraiseSendClose();	
		FrTXtries = 0;
		
discard:
		FrTXout = txout_end;
		return;
}

void fraiseSendBroadcast(const unsigned char *buf, unsigned char len)
{
	unsigned char i = len;
	unsigned char ischar = 0;
	unsigned char chksum = 0;
	
	if(!i) return;
	if(*buf == 'C') {
		ischar = 1;
		len |= 128;
	}
	
	i--;
	buf++;
	Serial_Init_Driver();
	TXxIE = 0;
	
	while(TXxIF == 0);
	
	TXSTAxbits.TX9D = 1; 	// address byte
	chksum += (TXREGx = 0);	// adress is null = broadcast
	
	Nop(); while(TXxIF == 0);
	TXSTAxbits.TX9D = 0;

	chksum += (TXREGx = len);

	Nop(); while(TXxIF == 0);
	if(ischar) chksum += (TXREGx = 'B');
	else chksum += (TXREGx = 'b');

	while(i) {
		Nop(); while(TXxIF == 0);
		chksum += (TXREGx = *buf);
		i--;
		buf++;
	}
	
	Nop(); while(TXxIF == 0);
	TXREGx = -chksum;
	Nop(); while(TXxIF == 0);
	
	Serial_Init_Receiver();
}

//--------------------- Interrupt routine : -------------------------------

void fraiseISR(void)
{	
	static unsigned char c, c2;
	
	if(FrInterruptEnabled == 0) return;

	if(TXxIE && TXxIF) {
		if(FrTXpacket_i == FrTXpacket_len) { //if end of the packet:
			Serial_Init_Receiver();
			FrTXpacket_len = 0;
			return;
		}
		c = FrTXpacket[FrTXpacket_i];
		FrTXpacket_i++;
		TXREGx = c; // send next byte
		
		return;
	}
	
	if(RCxIE && RCxIF) {
		if(RCSTAxbits.OERR){
			FraiseStatus.RX_OERR = 1;
			Serial_Init_Receiver();
			return;
		}
		if(RCSTAxbits.FERR){
			FraiseStatus.RX_FERR = 1;
			Serial_Init_Receiver();
			return;
		}

		if (RCSTAxbits.RX9D) { // -------  address : 
			c = RCREGx; // get byte
			if(c == (FrID | 128)) { // poll signal
				FraiseStatus.RX_POLL = 1;
				RCSTAxbits.ADDEN = 0;
				return;
			}	
			if ((c == 0) || (c == FrID)) {
				FraiseStatus.RX_POLL = 0;
				RCSTAxbits.ADDEN = 0;
				if(c) FraiseStatus.RX_BRDCST = 0;
				else FraiseStatus.RX_BRDCST = 1;

				FrRXchksum = c;
				FrRXin_tmp = FrRXin;
				FrRXin_end = FrRXin - 1; // protect FrRXin_end
				FrRXbuf[FrRXin_tmp] = c;
				FrRXin_tmp++;
				if(FrRXin_tmp == sizeof(FrRXbuf)) FrRXin_tmp = 0;
				return;
			}
			FraiseStatus.RX_POLL= 0;
			RCSTAxbits.ADDEN = 1;
			return;
		}
							// -----------  data :
		c = RCREGx; // get byte
		if(FraiseStatus.RX_POLL) { // had a poll signal
			if(c == (FrID | 128)){ 	// confirmation byte of poll signal:
				// answer poll signal :
				Serial_Init_Driver();
				if(FrTXpacket_len == 0) {
					TXREGx = 0;
					Serial_Init_Receiver();
				} else {
					fraiseSendLaunch();
					return;
				}
			}
			RCSTAxbits.ADDEN = 1;
			return;
		}	

		FrRXchksum += c;

		if(FrRXin_tmp == FrRXin_end) {// RX packet complete
			if(!FrRXchksum) { // RX packet checksum ok 
				FrRXin = FrRXin_tmp; //validation of packet
				if(!FraiseStatus.RX_BRDCST){
					Serial_Init_Driver();
					TXREGx = 0; //Acknowldge
					Serial_Init_Receiver();
				}
			} 
			else {			// RX packet checksum error
				FraiseStatus.RX_SERR = 1;
				if(!FraiseStatus.RX_BRDCST){
					Serial_Init_Driver();
					TXREGx = 1; //Checksum nack
					Serial_Init_Receiver();
				}
			}
			return;
		}// endof RX packet complete

		if((FrRXin_tmp == FrRXout)&&(FrRXout != FrRXin)) { //collision case : RX buffer full !
			Serial_Init_Receiver(); // discard current packet... TODO:send an buffer full nack (2)
			return;
		}

		c2 = (FrRXin + 1) % sizeof(FrRXbuf);
		if(FrRXin_tmp == c2) { //FrRXin_tmp == (FrRXin + 1): first byte following address = data length...
			FrRXin_end = (c & 31);
			FrRXin_end += 2;
			FrRXin_end += FrRXin;
			FrRXin_end %= sizeof(FrRXbuf);
		}

		FrRXbuf[FrRXin_tmp++] = c;
		if(FrRXin_tmp == sizeof(FrRXbuf)) FrRXin_tmp = 0; //ring buffer index
		return;
	}
}

//----------------- RX buffer utils : --------------------------------------------

unsigned char fraiseGetChar()
{
	unsigned char c;
	
	if((FrRXout == FrRXout_end) || (FrRXout == FrRXin)) return 0; //underflow error
	c = FrRXbuf[FrRXout++];
	if(FrRXout == sizeof(FrRXbuf)) FrRXout = 0;
	return c;
}

unsigned char fraisePeekChar()
{
	return FrRXbuf[FrRXout];
}

unsigned char fraiseGetLen()
{
	return FrRXout_len;
}

unsigned char fraiseGetIndex()
{
	return FrRXout;
}

unsigned char fraiseGetAt(unsigned char i)
{
	return FrRXbuf[(i + FrRXout_first)%sizeof(FrRXbuf)];
}

void fraiseSendCopy()
{
	char l;
	unsigned char i;
	l = (FrRXout - FrRXout_first)%sizeof(FrRXbuf);
	l--;
	i = FrRXout_first;
	putchar('C'); putchar(' '); 
	
	while(l > 0) {
		printf("%d ", FrRXbuf[i]);
		i++;
		if(i == sizeof(FrRXbuf)) i = 0;
		l--;
	}
}
//----------------- System : --------------------------------------------

static char CompareName() 
{
	unsigned char c, c2, eei;

	eei = EE_PREFIX;
	while(c = fraiseGetChar()){
		c2 = eeReadByte(eei);
		if(c2 == 0) { //end of string
			if(eei < EE_NAME) { //end of prefix
				eei = EE_NAME; //goto to name first char
				c2 = eeReadByte(eei);
				if(c2 == 0) { //no name?!
					return -1; 
				}
			} 
			else { //end of name before end of spelled name ; discard.
				return -1; 
			}
		}
		if(c2 != c) { //spelled name differs from device name; discard.
			return -1; 
		}
		eei++;
		if(eei > EE_NAMEMAX) { //spelled name too long; discard.
			return -1;
		}
	}
	
	return 0;
}


static void Assign() //"N" command
{
	unsigned char c, c2, tmpid;
	c = fraiseGetChar();
	c2 = fraiseGetChar();
	c -= '0'; if (c > 9) c -= 'A' - '9' - 1;
	c2 -= '0'; if (c2 > 9) c2 -= 'A' - '9' - 1;
	if((c > 7) || (c2 > 15)) { // bad id... discard
		return; 
	}
	tmpid = c2 + (c << 4);

	if(CompareName()) return;

	fraiseSetID(tmpid);
	return;
}

static void ResetToBld() //"F" command
{
	//INTCONbits.GIEH = 0; // disable high priority interrupts
	if(CompareName()) {
		//INTCONbits.GIEH = 1; // re-enable high priority interrupts
		return;
	}
	Reset();
}



//----------------- Main Service : -----------------------------------------

void fraiseService(void)
{
	unsigned char c, ischar, isbroadcast;

	if(FrRXout != FrRXin) //FraiseStatus.RX_FULL
	{
		c = FrRXout;
		c += 1;
		c %= sizeof(FrRXbuf);
		FrRXout_len = FrRXbuf[c];
		FrRXout_len &= 31;
		
		FrRXout_end = FrRXout;
		FrRXout_end += FrRXout_len;
		FrRXout_end += 2;
		FrRXout_end %= sizeof(FrRXbuf);
		
		isbroadcast = (fraiseGetChar() == 0);
		FrRXout_len = fraiseGetChar();
		ischar = FrRXout_len & 128;

		if(!(FrRXout_len &= 31)){ //packet error
			FrRXout = FrRXin;
			return;
		}
		
		if(isbroadcast) //Broadcast packet
		{
			if(ischar){
				c = fraiseGetChar();
				FrRXout_len -= 1;
				if     (c == 'B') {
#ifdef UD_RCVCB
					fraiseReceiveCharBroadcast();
#endif
					}
				else if(c == 'N') Assign();		/* assign to id if name matchs */
				else if(c == 'F') ResetToBld();	/* goto Fraisebootloader if name matchs */
				else if(c == 'I') __asm reset __endasm; //init
			}
#ifdef UD_RCVB
			else {
				c = fraiseGetChar();
				FrRXout_len -= 1;
				if     (c == 'b') fraiseReceiveBroadcast();
			}
#endif
		}
		else 			//Normal device packet
		{
			FrRXout_first = FrRXout;
			if(ischar) {
#ifdef UD_RCVC
				fraiseReceiveChar();
#endif
			}
#ifdef UD_RCV
			else fraiseReceive();
#endif
		}
		
		goto discard;

	discard:
		FrRXout = FrRXout_end;
	}
		
	if((!FrTXempty) && (!FrTXpacket_len)) { // if there is sth to send to master and nothing in TXpacket :
		fraiseDecodeNextTXPacket();
	}
	else if(Serial_Is_Driver() && (!FrTXpacket_len) && TXSTAxbits.TRMT)// security:
		Serial_Init_Receiver();

	return;	
}
	
//-------------------------------------------------------------------
// user putchar :
unsigned char txbuf[35];
unsigned char txlen = 0;
#ifndef PUTCHAR
#define PUTCHAR(c) void putchar(char c) __wparam
#endif
//PUTCHAR (c){c;} //dummy putchar
PUTCHAR (c) {
	txbuf[txlen] = c;
	if(txlen < (sizeof(txbuf) - 1)) txlen++;
	if(c == '\n') {
		fraiseSend(txbuf, txlen);
		txlen = 0;
	}
}


