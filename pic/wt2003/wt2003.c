/*********************************************************************
 *
 *                WT2003 MP3 player for Fraise
 *
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  mar 2018     Original.
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
#include <wt2003.h>

#ifndef WT2003_UART_PORT
#define WT2003_UART_PORT AUXSERIAL_NUM
#define WT2003_UART_RX AUXSERIAL_RX
#define WT2003_UART_TX AUXSERIAL_TX
#endif

//serial port:
#if WT2003_UART_PORT==1
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

static byte isPlaying;
static byte RXstate = 0;
static byte buffer[6];
static byte wt2003_txbuffer[16];

static void delay(word millisecs)
{
	t_delay del;
	delayStart(del, millisecs * 1000);
	while(!delayFinished(del)){}
}

static void setBaudRate(unsigned long int br)
{
//baud rate : br=FOSC/[4 (n+1)] : n=FOSC/(4*br)-1 : br=250kHz, n=FOSC/1000000 - 1
/* 
br(n+1)*4 = fosc  
4.br.n + 4br = f         
n = (f - 4br)/ 4br = f/4br - 1
*/
//#define BRGHL (FOSC/1000000 - 1)
	unsigned long int brg = FOSC/(4 * br) - 1;
	SPBRGHx=brg/256;
	SPBRGx=brg%256;
}
#define BRGHL(br) ((FOSC / (4UL * br)) - 1)
#define SET_BAUDRATE(br) SPBRGHx = BRGHL(br) / 256UL; SPBRGx = BRGHL(br) % 256UL;

static void wt2003_write(byte b)
{
	while(!TXxIF) {};
	TXREGx = b;
}


static void wt2003_send(byte len)
{
	byte chksum;
	byte i, b;
	
	wt2003_write(0x7E);
	wt2003_write(chksum = len + 2); // add "length" and "chksum" bytes
	for(i = 0 ; i < len ; i++) {
		b = wt2003_txbuffer[i];
		chksum += b;
		wt2003_write(b);
	}
	wt2003_write(chksum);
	wt2003_write(0xEF);
}

void wt2003_Init()
{
	digitalSet(WT2003_UART_TX);
	pinModeDigitalOut(WT2003_UART_TX);
	pinModeDigitalIn(WT2003_UART_RX);

	TXSTAxbits.TXEN = 1;
	TXSTAxbits.BRGH = 1;
	TXSTAxbits.TX9 = 0;
	TXxIE = 0;

	RCSTAxbits.RX9 = 0;
	RCSTAxbits.CREN = 0;
	RCSTAxbits.ADDEN = 0;
	//RCSTAxbits.CREN = 1;
	//RCxIE = 1;
	RCxIP = 0; // low interrupt priority
	RCSTAxbits.SPEN = 1;
	
	BAUDCONxbits.BRG16 = 1;
	SET_BAUDRATE(9600);
	
}

void wt2003_lowISR()
{
#if 0 // wt2003 return message isn't implemented yet
	byte rxbyte;
	
	if(!RCxIF) return;
			
	if(RCSTAxbits.OERR){ // overrun error
		WREG=RCREGx;
		__asm nop __endasm ;	
		WREG=RCREGx;
		RCSTAxbits.CREN=0;
		RCSTAxbits.CREN=1;
		return;
	}
	if(RCSTAxbits.FERR){ // framing error
		
		return;
	}

	rxbyte = RCREGx;

	switch(RXstate) {
		case 1:
			if(rxbyte == 255) RXstate = 2; break;
		case 2:
			//if(rxbyte == 6)
			buffer[0] = rxbyte;
			RXstate = 3; 
			break;
		case 3:
			/*if(rxbyte == 0x3C || rxbyte == 0x3D || rxbyte == 0x3E)
			{
				isPlaying = 0;
			}
			else isPlaying = 1;*/
			//isPlaying = rxbyte;
			if( (rxbyte == 64) // file not found
				||(rxbyte == 61)) // end of track
				isPlaying = 0;
			buffer[1] = rxbyte;
			RXstate = 4;
			break;
		case 4:
		case 5:
		case 6:
			buffer[RXstate - 2] = rxbyte;
			RXstate++;
		default:
			if(rxbyte == 126) RXstate = 1;
			else RXstate = 0;
	}
	
	/*if(Status == 0x3C || Status == 0x3D || Status == 0x3E)
    {
        isPlaying = 0;
    }
    else
    {
        isPlaying = 1;
    }*/
#endif
}

void wt2003_deInit()
{
	//RCSTAxbits.CREN = 0;
	//RCxIE = 0;
}


/**************************************************************** 
 * Function Name: SpecifyMusicPlay
 * Description: Specify the music index to play, the index is decided by the input sequence of the music.
 * Parameters: index: the music index: 0-65535.
 * Return: none
****************************************************************/ 
void wt2003_SpecifyMusicPlay(word index)
{
    isPlaying = 1;

    wt2003_txbuffer[0] = 0xA2;
    wt2003_txbuffer[1] = index / 256;
    wt2003_txbuffer[2] = index % 256;
    wt2003_send(3);
    delay(10);
}


/**************************************************************** 
 * Function Name: PlayPause
 * Description: Pause the MP3 player.
 * Parameters: none
 * Return: none
****************************************************************/ 
void wt2003_PlayPause(void)
{
    wt2003_txbuffer[0] = 0xAA;
    wt2003_send(1);
    isPlaying = !isPlaying;
    delay(10);
}

/**************************************************************** 
 * Function Name: Stop
 * Description: Stop playing any track.
 * Parameters: none
 * Return: none
****************************************************************/ 
void wt2003_Stop(void)
{
    wt2003_txbuffer[0] = 0xAB;
    wt2003_send(1);
    isPlaying = 0;
    delay(10);
}

/**************************************************************** 
 * Function Name: PlayNext
 * Description: Play the next song.
 * Parameters: none
 * Return: none
****************************************************************/ 
void wt2003_PlayNext(void)
{
    wt2003_txbuffer[0] = 0xAC;
    wt2003_send(1);
    isPlaying = 1;
    delay(10);
}

/**************************************************************** 
 * Function Name: PlayPrevious
 * Description: Play the previous song.
 * Parameters: none
 * Return: none
****************************************************************/ 
void wt2003_PlayPrevious(void)
{
    wt2003_txbuffer[0] = 0xAD;
    wt2003_send(1);
    isPlaying = 1;
    delay(10);
}

/**************************************************************** 
 * Function Name: PlayLoop
 * Description: Play loop for all the songs.
 * Parameters: none
 * Return: none
****************************************************************/ 
void wt2003_PlayLoop(byte loop)
{
    wt2003_txbuffer[0] = 0xAF;
    wt2003_txbuffer[1] = loop;
    wt2003_send(2);
    delay(10);
}

/**************************************************************** 
 * Function Name: SetVolume
 * Description: Set the volume, the range is 0x00 to 0x1E.
 * Parameters: volume: the range is 0x00 to 0x1E.
 * Return: none
****************************************************************/ 
void wt2003_SetVolume(byte volume)
{
    wt2003_txbuffer[0] = 0xAE;
    wt2003_txbuffer[1] = volume;
    wt2003_send(2);
    delay(10);
}

/**************************************************************** 
 * Function Name: QueryPlayStatus
 * Description: Query play status.
 * Parameters: none
 * Return: 0: played out; 1: other.
 * Usage: while(QueryPlayStatus() != 0);  // Waiting to play out.
****************************************************************/ 
byte wt2003_IsPlaying(void)
{
    return (isPlaying /*== 65*/);
}

void wt2003_printStatus()
{
	printf("C wt2003 %d %d %d %d %d\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
}
