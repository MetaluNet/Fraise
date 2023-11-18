/*********************************************************************
 *
 *                Fraise Bootloader for pic18
 *
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  feb 18 2012     Original.
 ********************************************************************/

/******************************************************************************
 * -fbld.c-
 * Fraise bootloader.
 * This file has to be configured (by Makefile) for a specific 
 * processor/board target.
 *****************************************************************************/
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
#include <pic18fregs.h>

#include "eeprom.c"

#if CONFIG_SETUP
extern void Setup();
#endif

// bootloader exit time
#ifndef BOOT_TIME
#define BOOT_TIME 4000
#endif

//---------------  Serial macros :   -----------------
//serial drive:
#define	InitSerDrv()	{SERDRV_PIN = 0; SERDRV_TRI = 1;}
#define	SerDrv_On()		{SERDRV_TRI = SERDRV_POL;}
#define	SerDrv_Off()	{SERDRV_TRI =! SERDRV_POL;}
#define	SerDrv_isOn()	(SERDRV_TRI == SERDRV_POL)


//serial port:
#if UART_PORT==1
#define SPBRGx 			SPBRG1
#define SPBRGHx 		SPBRGH1
#define BAUDCONx 		BAUDCON1
#define BAUDCONxbits 	BAUDCON1bits
#define RCREGx 			RCREG1
#define _RCREGx 		_RCREG1
#define RCSTAx 			RCSTA1
#define RCSTAxbits 		RCSTA1bits
#define TXREGx 			TXREG1
#define TXSTAx 			TXSTA1
#define TXSTAxbits 		TXSTA1bits
#define RCxIF	 		PIR1bits.RC1IF
#define TXxIF	 		PIR1bits.TX1IF
#else
#define SPBRGx 			SPBRG2
#define SPBRGHx 		SPBRGH2
#define BAUDCONx 		BAUDCON2
#define BAUDCONxbits 	BAUDCON2bits
#define RCREGx 			RCREG2
#define _RCREGx 		_RCREG2
#define RCSTAx 			RCSTA2
#define RCSTAxbits 		RCSTA2bits
#define TXREGx 			TXREG2
#define TXSTAx 			TXSTA2
#define TXSTAxbits 		TXSTA2bits
#define RCxIF	 		PIR3bits.RC2IF
#define TXxIF	 		PIR3bits.TX2IF
#endif

//(1 port equ)
#ifndef BAUDCON1
#define SPBRG1 			SPBRG
#define SPBRGH1 		SPBRGH
#define BAUDCON1 		BAUDCON
#define BAUDCON1bits 	BAUDCONbits
#define RCREG1 			RCREG
#define _RCREG1 		_RCREG
#define RCSTA1 			RCSTA
#define RCSTA1bits 		RCSTAbits
#define TXREG1 			TXREG
#define TXSTA1 			TXSTA
#define TXSTA1bits 		TXSTAbits
#define RC1IF 			RCIF
#define TX1IF 			TXIF
#endif

//#define _RCREGx		_ ## RCREGx

//-------------  Timer0 macros :  ---------------------------------------- 
//prescaler=PS fTMR0=FOSC/(4*PS) nbCycles=0xffff-TMR0init T=nbCycles/fTMR0=(0xffff-TMR0init)*4PS/FOSC
//TMR0init=0xffff-(T*FOSC/4PS)
//ex: PS=256 : T=1s : TMR0init=0xffff-FOSC/4PS : if FOSC=48Mhz TMR0init=0xffff-46875 ; FOSC=8000 0xffff-7813 ; 64000000: 0xffff-62500
//Maximum 1s !!
#define	TMR0init(T) (0xffff-(T*FOSC/1024000)) //ms 
#define InitTMR0(T) { TMR0H=TMR0init(T)/256 ; TMR0L=TMR0init(T)%256; INTCONbits.TMR0IF=0; }
#define TMR0out() (INTCONbits.TMR0IF)

//-------------  eeprom addresses : ----------------------------
#define EE_ID 0
#define EE_PREFIX 1
#define EE_PREFIXMAXLEN 8
#define EE_NAME (EE_PREFIX+EE_PREFIXMAXLEN+1)
#define EE_NAMEMAXLEN 16
#define EE_NAMEMAX (EE_NAME+EE_NAMEMAXLEN)

//reset prefix:
/*typedef unsigned char eeprom;
__code eeprom __at 0xF00000 __EEPROM[] ={ 0x00,0x00,0x00,0x00};*/
__code char __at( 0xF00000 ) __EEPROM[]= { 0x00,0x00,0x00,0x00};


//---------------- datas ----------------------------------------
//#pragma udata grp1 tmp1 tmp2 param1 param2 

//------------------------------------------------------------------

typedef union {
	unsigned char C[4];
	unsigned int I[2];
	long int L;
	unsigned long UL;
} union32_t;

union {
	unsigned char VAL;
	struct {
		unsigned VERIFIED :1; // Name of device has been verified
		unsigned MEM_USED :1; // Current memory block is used, has to be written
		unsigned WR_APP_BB :1; // Application boot block must be written at the end of app prog mem	
		unsigned VERBOSE1 :1; //
		unsigned VERBOSE2 :1; //
		/*unsigned DID_WRITE :1; 	// A write operation actually occured : hex differed from memory 
								// (for verify purpose) */
	};
} Flags;



unsigned char tmp1,tmp2,param1,param2;
unsigned char reclen,rectype,cs; 
union32_t	address,addtmp;
unsigned char buf[128];
unsigned char mem[64];
unsigned char app_bootblock[4]; // keep 4 first bytes of app fimware (inital goto)
								// to relocate them in the last 4 bytes of app prog memory space.
int Time;

//#define LED LATBbits.LATB4
//#define TLED TRISBbits.TRISB4

#ifndef	LED
char LED;
#endif

//#include <crt0i.c>
/*CAUTION ! NO CRT here : so don't use any initialized variable (e.g: int i = 10).*/

//where bootloader will replace application entry vector by bootloader entry
//#pragma code _app_start  (APP_START)
void _app_start (void) __naked
{
	__asm 
		ORG     APP_START
		GOTO    BLD_START
	__endasm ;
}

#define APP_STARTRELOC (BLD_START-4)
//where bootloader will relocate application initial goto
//#pragma code _app_startreloc  (BLD_START-4)
void _app_startreloc (void) __naked
{
	__asm 
		ORG     APP_STARTRELOC
		nop
		nop
	__endasm ;
}

void startapp(void) __naked
{
	__asm 
		GOTO     APP_STARTRELOC
	__endasm ;
}

extern int stack_end;

void main (void);
void _startup (void) __naked;

void _entry (void) __naked __interrupt 0
{
  __asm goto __startup __endasm;
}


void _startup (void) __naked
{
  __asm
    // Initialize the stack pointer
    lfsr 1, _stack_end
    lfsr 2, _stack_end
    clrf _TBLPTRU, 0	// 1st silicon doesn't do this on POR
    
    // initialize the flash memory access configuration. this is harmless
    // for non-flash devices, so we do it on all parts.
    bsf 0xa6, 7, 0
    bcf 0xa6, 6, 0

  __endasm ;
    
  /* Call the user's main routine */
  main();

loop:
  /* return from main will lock up */
  goto loop;
}


//---------------serial macros ---------------------------------------
/*#define Serial_Init_Receiver() {\
	while(TXSTAxbits.TRMT==0);\
	SerDrv_Off();		\
	WREG=RCREGx;	\
	__asm nop __endasm ;	 \
	WREG=RCREGx;		\
	RCSTAxbits.CREN=0;		\
	RCSTAxbits.CREN=1;		\
}*/

#define Serial_Init_Driver() {\
	SerDrv_On();		\
	RCSTAxbits.CREN=0;	\
}

#define Serial_Is_Driver() SerDrv_isOn()

//---------------------- Input funcs ----------------
void Serial_Init_Receiver() {
	while(TXSTAxbits.TRMT==0);
	SerDrv_Off();		
	//WREG=RCREGx;	
	//__asm nop __endasm ;	 
	//WREG=RCREGx;
	__asm 
		movf	_RCREGx,W
		movf	_RCREGx,W
	 __endasm ;
	RCSTAxbits.CREN=0;		
	RCSTAxbits.CREN=1;		
}

void InitTimer()
{
	InitTMR0(100);
}

unsigned char getchar() 
{
	while(!RCxIF) { 
		if(TMR0out()) {
			Time++;
			return -1;
		}
	}
	LED=0;
	if((RCSTAxbits.OERR)||(RCSTAxbits.FERR)) { 
		Serial_Init_Receiver(); 
		return -1; 
	}
	//LED=1;
	if(RCSTAxbits.RX9D) {
		if(RCREGx==0) {
			InitTimer();
			return getchar();
		}
		else startapp(); //if address!=0 exit bootloader
	}
	
	return 0;
	//return RCREGx;
}


void ReadPacket() 
{
/*	unsigned char i,cs;
	unsigned char *p;

	p=buf;*/
	
	InitTimer();
/*	*p++=cs=i=getchar();
	if(i==0) return;
	//if(--i==0) goto discard;
	if(TMR0out()) goto discard;
	do{
		InitTMR0(100);
		cs+=(*p++=getchar());
		if(TMR0out()) goto discard;
	} while(--i);
	
	if(cs!=0) goto discard;
	
	return ;
	
discard:	
	buf[0]=0;*/
	/*
	p=buf;
	cs=*buf++=i=getchar();
	do {
		cs+=(*buf++=getchar());		
	} while(--i);

	if(cs!=0) *buf=0;
	*/
	__asm
		banksel _tmp1
		lfsr 	0,_buf		; p=buf;
		call	_getchar
		tstfsz	WREG
		bra 	rpbad
		movf	_RCREGx,W
		movwf	_POSTINC0	
		movwf	_tmp1		
		movwf	_tmp2		; cs=*buf++=i=getchar();
		skpnz
		bra		rpbad		; if(i==0) 
		//btfsc	_INTCONbits,2 ;if TMR0IF
		//bra		rpbad		;if(timeout()||i=0 discard
		
rploop1:					; do
		call 	_InitTimer
		call	_getchar
		tstfsz	WREG
		bra 	rpbad
		movf	_RCREGx,W
		movwf	_POSTINC0	; 
		addwf	_tmp2,F		; cs+=*buf++=getchar();
		//btfsc	_INTCONbits,2 ;if TMR0IF
		//bra		rpbad		 ;if(timeout()) discard
		decfsz	_tmp1,F		; while(--tmp1)
		bra 	rploop1
		
		movf	_tmp2,F		;checksum
		skpnz
		return
rpbad:
		banksel _buf
		clrf	_buf			; bad packet sum
	 __endasm; 
}

//---------------------- Output funcs ----------------

void putchar(unsigned char c) __wparam
{
	while(!TXxIF);
	TXREGx=c;
}

void PrintHex(unsigned char n) __wparam
{
	//static 
	unsigned char c;
	c=n/16;
	c+='0';
	if(c>'9') c+=('A'-'9'-1);
	putchar(c);
	c=n&15;
	c+='0';
	if(c>'9') c+=('A'-'9'-1);
	putchar(c);
}	

void PrintLetterNL(unsigned char c) __wparam
{
	putchar(' ');
	putchar(c);
	putchar('\n');
}	

//---------------------- RENAME & PREFIX funcs ----------------

void WriteEeprom() //load start_eeprom in EEADR and max_len in param1
{
	/*
		uchar *p=buf+8;
		uchar i=buf[0]-8;
		if((i!=0)||(i<=maxlen)) do
		{
			eeprom[eeadr++]=*p++;
		} while(--i);
		eeprom[eeadr]=0;
		putchar('E');
	*/
	__asm
		lfsr 	0,(_buf+8)
		banksel _buf
		movf	_buf,W
		addlw	-8
		banksel _tmp1
		movwf	_tmp1
		bz		weend
		subwf	_param1,W ; w=param1-tmp1=maxlen-len
		bnc		weend	; //too long
weloop:
		movf	_POSTINC0,W
		movwf	_EEDATA
		call	_ee_write_byte
		incf	_EEADR,F
		decfsz	_tmp1,F
		bra		weloop
weend:
		clrf	_EEDATA
		call	_ee_write_byte
	__endasm;
}

unsigned char Sum7()
{
	unsigned char c,i;

	i=6;
	c=0;
	__asm lfsr 0,_buf+2 __endasm;
	do{
		c+=POSTINC0;
	} while(--i);
	return c;
}

void Rename()
{
	if(Sum7()!=(('E'+'N'+'A'+'M'+'E'+':')%256)) return;
	
	EEADR=EE_NAME;
	param1=EE_NAMEMAXLEN;
	WriteEeprom();
	PrintLetterNL('R');
}		
		
void Prefix()
{
	if(Sum7()!=(('R'+'E'+'F'+'I'+'X'+':')%256)) return;
	
	EEADR=EE_PREFIX;
	param1=EE_PREFIXMAXLEN;
	WriteEeprom();
	PrintLetterNL('P');
}		
		
void VerifyName()
{
	/*unsigned char c,c2,i;
	unsigned char *p,*pend;
	
	//0 	1 	2 	3 	4 	5 	6
	//[6] 	V 	T 	r 	u 	c 	[cs]
	
	pend=buf+buf[0]; //last char address+1	
	p=buf+2;
	i=EE_PREFIX;
	do{
		c2=ee_read_byte(i);
		if(!c2) {//end of string
			if(i>EE_NAME) {putchar('E');return ;}//prematured end of name; abort.
			i=EE_NAME; //end of PREFIX, get first NAME char
			c2=ee_read_byte(i);
		}
		if(*p++!=c2) {putchar('M');return ;} //char mismatch; abort.
		if(i>EE_NAMEMAX) {putchar('>');return ;}
		i++;
	} while(p<pend);
	
	if(i<EE_NAME) {putchar('P');return ;} //we havn't reached the begin of NAME ; abort.
	if(ee_read_byte(i)) {putchar('F');return ;} //we havn't reached the end of name packet; abort.

	putchar('V');*/
	
	__asm
		lfsr 	0,(_buf+2)
		banksel _buf
		decf	_buf,W
		banksel _tmp1
		movwf	_tmp2
		decf	_tmp2,F
		
		movlw	EE_PREFIX	
		movwf 	_tmp1
verloop:
		movf	_tmp1,W
		call	_ee_read_byte
		bnz		verloop_2
		movlw	EE_NAME
		movwf 	_tmp1
		call	_ee_read_byte
verloop_2: 	; W=ee_read_byte(tmp1)
		subwf	_POSTINC0,W	; (*p++)-W
		skpz	
		return
		incf	_tmp1,F
		decfsz	_tmp2,F
		bra		verloop
		
		movf	_tmp1,W
		call	_ee_read_byte
		skpz	
		return
	__endasm;
	
	PrintLetterNL('V');
	Flags.VERIFIED=1;
	Flags.WR_APP_BB=0;
}		

//---------------------- Program memory write ----------------

void DoWrite(unsigned char eecon1) __wparam
{
	EECON1=eecon1;
	EECON2=0x55;
	EECON2=0xAA;
	EECON1bits.WR=1;
}

void ResetMem()
{
	Flags.MEM_USED=0;
	__asm
		banksel _tmp1
		movlw	.64
		movwf	_tmp1
		lfsr 0,_mem
resetmemloop:
		setf	_POSTINC0
		decfsz	_tmp1,F
		bra 	resetmemloop
	__endasm;
}
	
void WriteBlock()
{
	//unsigned char i;
	
	if(address.C[0]&63) {
		putchar('!'); //address error : not a 64 boundary
		return;
	}

	//if(address.UL&0xffe00000) return; 
	if(address.C[3]||(address.C[2]&0xe0)) return; //error: not in program memory space; cfg/eemprom ?

	if(address.I[0]>=(BLD_START&0xffff)) return; //error: in bootloader space !

	if(address.UL==APP_START) {
		__asm
			movff _mem+0,_app_bootblock+0
			movff _mem+1,_app_bootblock+1
			movff _mem+2,_app_bootblock+2
			movff _mem+3,_app_bootblock+3
		__endasm;
		//overwrite app initial goto by goto BLD_START
		mem[0]=(BLD_START/2)&0xff;
		mem[1]=0xef;
		mem[2]=((BLD_START/2)/0xff)&0xff;
		mem[3]=0xf0+(((BLD_START/2)/0xffff)&0x0f);
		Flags.WR_APP_BB=1;
	}
	else if(address.UL==(BLD_START-64)) { //at the end of the last app 64 bytes block, append app initial goto:
		__asm
			movff _app_bootblock+0,_mem+60
			movff _app_bootblock+1,_mem+61
			movff _app_bootblock+2,_mem+62
			movff _app_bootblock+3,_mem+63
		__endasm;
		Flags.WR_APP_BB=0;
	}
	
	if(Flags.VERBOSE1){
		putchar(' ');PrintHex(address.C[1]);PrintHex(address.C[0]);
	}
	
	TBLPTRU=address.C[2];
	TBLPTRH=address.C[1];
	TBLPTRL=address.C[0];
	
	DoWrite(0b10010100); //ERASE BLOCK

	__asm 
		tblrd*-					; point to address-1
		lfsr 0,_mem 
		
		banksel _tmp1
		movlw	8
		movwf	_tmp1
wbloop1:
		movlw	8
		movwf	_tmp2
wbloop2:
		movf	_POSTINC0,W		
		movwf 	_TABLAT
		tblwt+*
		decfsz	_tmp2,F
		bra		wbloop2
		
		movlw	b'10000100'		; Setup writes
		rcall 	_DoWrite
		decfsz _tmp1,F
		bra 	wbloop1

	__endasm;
	
	EECON1bits.WREN=0;			//disable writes
	PrintLetterNL('W');
}

unsigned char GetHex(void)
{
	/*
		tmp1=*p++-'0';
		if(tmp1>9) tmp1+='9'-'A'+1;
		tmp2=*p++-'0';
		if(tmp2>9) tmp2+='9'-'A'+1;
		return(tmp1<<4+tmp2);
	*/
	__asm
		banksel _tmp1;
		movff	_POSTINC0,_tmp1;
		movff	_POSTINC0,_tmp2;
		movlw	-'0'
		addwf	_tmp1,F
		addwf	_tmp2,F
		
		movlw	-10
		addwf	_tmp1,W
		movlw	'9'-'A'+1
		skpnc	
		addwf	_tmp1,F
		
		movlw	-10
		addwf	_tmp2,W
		movlw	'9'-'A'+1
		skpnc	
		addwf	_tmp2,F
		
		swapf	_tmp1,W
		addwf	_tmp2,W
	__endasm;

		return WREG;
}

#define SeekBuf(lit) __asm lfsr 0,_buf+lit __endasm

void HexPacket()
{
	unsigned char c;
	
	//Echo();
	
	SeekBuf(2);
	cs=reclen= GetHex();
	cs+=addtmp.C[1]= GetHex();
	cs+=addtmp.C[0]= GetHex();
	cs+=rectype= GetHex();
	if(rectype==4){
		if((addtmp.C[1]==0)&&(addtmp.C[0]==0)&&(reclen==2)) {
			cs+=addtmp.C[3]= GetHex();
			cs+=addtmp.C[2]= GetHex();
			cs+=GetHex();
			if(cs) goto cs_error;
		} else goto line_error;
	}
	if(rectype==1)
	{
		cs+=GetHex();
		if(cs) goto cs_error;
		if(Flags.MEM_USED) WriteBlock();
		ResetMem();		
		if(Flags.WR_APP_BB) {
			address.UL=(BLD_START-64);
			WriteBlock();
			ResetMem();
		}
		PrintLetterNL('Y');
		return;
	}
	//if((addtmp.UL&0xffc0)!=(address.UL&0xffc0)){
	//if((addtmp.UL&0xffffffc0)!=(address.UL&0xffffffc0)){
	if((addtmp.C[3]!=address.C[3])||(addtmp.C[2]!=address.C[2])||
	(addtmp.C[1]!=address.C[1])||((addtmp.C[0]&0xc0)!=(address.C[0]&0xc0))){
		if(Flags.MEM_USED) WriteBlock();
		ResetMem();

		//address.UL=addtmp.UL;
		__asm
			movff	_addtmp+0,_address+0
			movff	_addtmp+1,_address+1
			movff	_addtmp+2,_address+2
			movff	_addtmp+3,_address+3
		__endasm;
	}

	if(rectype==4) goto ack;
	if(rectype) goto unsupport_error;
	if(!reclen) goto ack;
	
	//putchar('T');
	
	
	if(addtmp.C[3]||(addtmp.C[2]&0xe0)) {
		goto unsupport_error; //maybe do config/eeprom stuff later...		
	}

	if(Flags.VERBOSE2) {
		PrintHex(reclen);putchar('@');PrintHex(addtmp.C[1]);PrintHex(addtmp.C[0]);putchar(' ');
	}
	
		TBLPTRU=addtmp.C[2];
		TBLPTRH=addtmp.C[1];
		TBLPTRL=addtmp.C[0];
		
	__asm
		banksel _addtmp
		lfsr 	0,_buf+10
		lfsr 	2,_mem
		movf	_addtmp,W
		andlw	63
		addwf	_FSR2L,F
		skpnc	
		incf	_FSR2H,F
		movf	_POSTDEC2,F ; dummy rewind 1
hexpackfillmem:
		call 	_GetHex
		banksel _cs
		addwf 	_cs,F
		movwf	_PREINC2	;cs+=mem[add&63+i]=buf[10+2*i];
		
		tblrd*+
		subwf	_TABLAT,W
		bz		hpfm_memid	
		__endasm; Flags.MEM_USED=1; putchar('*');__asm
hpfm_memid:	
		//movf	INDF0
		__endasm; if(Flags.VERBOSE2) { PrintHex(INDF2); putchar(' ');} __asm
		banksel _reclen
		decfsz	_reclen,F
		bra		hexpackfillmem
	__endasm;
	
	c=GetHex();
	cs+=c;
	if(cs) {
		address.UL=0xffffffff;
		Flags.MEM_USED=0;
		goto cs_error;
	}
	
ack:
	PrintLetterNL('X'); //OK
	return;

cs_error:	//bad checksum...
	PrintLetterNL('z');
	return;

unsupport_error:	//unsupported...
	PrintLetterNL('u');
	return;

line_error:	//bad format,...
	PrintLetterNL('l');		
}

//---------------------- Program memory read ----------------

void ReadProg()
{
	unsigned char i;
	
	SeekBuf(2);
	TBLPTRH=GetHex();
	TBLPTRL=GetHex();

	i=16;
	putchar('r');putchar(' ');
	do{
		__asm tblrd*+ __endasm;
		PrintHex(TABLAT);
		putchar(' ');
	} while(--i);
	putchar('\n');

}

//---------------------- Echo ----------------

void Echo()
{
	/*unsigned char *p;
	unsigned char i;

	p=buf+1;
	i=buf[0];
	if(i>1)
	while(i--) putchar(*p++);
	putchar('K');*/
	
	__asm
		lfsr 	0,(_buf+1)
		banksel _buf
		movf	_buf,W
		banksel _tmp1
		movwf	_tmp1
		bz		echoend
		decf	_tmp1,F
		bz		echoend
echoloop:
		movf	_POSTINC0,W
		call	_putchar
		decfsz	_tmp1,F
		bra		echoloop
echoend:
	__endasm;
		putchar('\n');
}

//-------------------- Main ---------------------

void main(void)
{
	unsigned char c;
	
	__asm
		MOVLW	0xff
ifdef _TRISA
		MOVWF	_TRISA
endif
		MOVWF	_TRISB
		MOVWF	_TRISC
ifdef _TRISD
		MOVWF	_TRISD
endif
		MOVWF	_TRISE
	__endasm;	

#if CONFIG_SETUP
	Setup();
#endif

	/*T0CONbits.TMR0ON = 1;
	T0CONbits.T08BIT = 0;	// 16 bit timer
	T0CONbits.T0CS = 0;		// Use internal clock
	T0CONbits.T0SE = 1;		// Hi to low
	T0CONbits.PSA = 0;		// Use the prescaler
	T0CONbits.T0PS2 = 1;	// 
	T0CONbits.T0PS1 = 1;	// 1/256 prescaler: 
	T0CONbits.T0PS0 = 1;	// */
	T0CON=0b10010111;
	
	//SERIAL:
	SerDrv_Off();
	InitSerDrv();
//baud rate : br=FOSC/[4 (n+1)] : n=FOSC/(4*br)-1 : br=250kHz, n=FOSC/1000000 - 1
#define BRGHL (FOSC/1000000 - 1)
	SPBRGHx=BRGHL/256;
	SPBRGx=BRGHL%256;

	BAUDCONxbits.BRG16=1;

	TXSTAxbits.TXEN=1;
	TXSTAxbits.BRGH=1;
	TXSTAxbits.TX9=1;
	TXSTAxbits.TX9D=0;

	RCSTAxbits.RX9=1;
	RCSTAxbits.SPEN=1;
	
	address.UL=0;
	addtmp.UL=0;
	ResetMem();
	Flags.VAL=0;

#ifdef TLED
	TLED=0;
#endif
	LED=1;

	Serial_Init_Driver();
	putchar('K');
	Serial_Init_Receiver();

	Time=0;
	//InitTMR0(1000);
	/*while(1){
		if(TMR0out()){
			if(LED) LED=0;
			else LED=1;
			InitTMR0(1000);
		}
	}*/			
	//c=0;

	while(1){
		
		if(Serial_Is_Driver()) Serial_Init_Receiver();

		buf[0]=0;
		ReadPacket();

		if(buf[0]) {

			Serial_Init_Driver();

			if(LED) LED=0;
			else LED=1;
			
			c=buf[1];
			
			if(c=='R') Rename();
			else if(c=='P') Prefix();
			else if(c=='V') VerifyName();
			if(Flags.VERIFIED){
				if(c=='E') Echo();
				else if(c==':') HexPacket();
				else if(c=='r') ReadProg();
				else if (c=='A') startapp();
				else if (c=='v') {
					Flags.VERBOSE1=(buf[2]!='0');
					Flags.VERBOSE2=(buf[2]>'1');
				} 
			}
		}
		
		if((!Flags.VERIFIED)&&(Time>(BOOT_TIME / 25))) startapp(); //default 4s timeout.
	}
}


