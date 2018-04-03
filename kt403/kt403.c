/*********************************************************************
 *
 *                KT403 MP3 player for Fraise
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
#include <kt403.h>

// adapted from:
/*
 * MP3Player_KT403A.h
 * A library for Grove-Serial MP3 Player V2.0
 *
 * Copyright (c) 2015 seeed technology inc.
 * Website    : www.seeed.cc
 * Author     : Wuruibin
 * Created Time: Dec 2015
 * Modified Time:
 * 
 * The MIT License (MIT)
 */

#ifndef KT403_UART_PORT
#define KT403_UART_PORT AUXSERIAL_NUM
#define KT403_UART_RX AUXSERIAL_RX
#define KT403_UART_TX AUXSERIAL_TX
#endif

//serial port:
#if KT403_UART_PORT==1
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

static void kt403_write(byte b)
{
	while(!TXxIF) {};
	TXREGx = b;
}

void kt403_Init()
{
	digitalSet(KT403_UART_TX);
	pinModeDigitalOut(KT403_UART_TX);
	pinModeDigitalIn(KT403_UART_RX);

	TXSTAxbits.TXEN = 1;
	TXSTAxbits.BRGH = 1;
	TXSTAxbits.TX9 = 0;
	TXxIE = 0;

	RCSTAxbits.RX9 = 0;
	RCSTAxbits.CREN = 0;
	RCSTAxbits.CREN = 1;
	RCSTAxbits.ADDEN = 0;
	RCxIE = 1;
	RCxIP = 0; // low interrupt priority
	RCSTAxbits.SPEN = 1;
	
	BAUDCONxbits.BRG16 = 1;
	SET_BAUDRATE(9600);
	
	kt403_SelectPlayerDevice(2); // SD card
}

void kt403_lowISR()
{
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
}



/**************************************************************** 
 * Function Name: SelectPlayerDevice
 * Description: Select the player device, U DISK or SD card.
 * Parameters: 0x01:U DISK;  0x02:SD card
 * Return: none
****************************************************************/ 
void kt403_SelectPlayerDevice(byte device)
{
    kt403_write(0x7E);
    kt403_write(0xFF);
    kt403_write(0x06);
    kt403_write(0x09);
    kt403_write(0x00);
    kt403_write(0x00);
    kt403_write(device);
    kt403_write(0xEF);
    delay(200);
}

/**************************************************************** 
 * Function Name: SpecifyMusicPlay
 * Description: Specify the music index to play, the index is decided by the input sequence of the music.
 * Parameters: index: the music index: 0-65535.
 * Return: none
****************************************************************/ 
void kt403_SpecifyMusicPlay(word index)
{
    isPlaying = 1;

    kt403_write(0x7E);
    kt403_write(0xFF);
    kt403_write(0x06);
    kt403_write(0x03);
    kt403_write(0x01); // feedback ON
    kt403_write(index / 256);
    kt403_write(index % 256);
    kt403_write(0xEF);
    delay(10);
}

/**************************************************************** 
 * Function Name: SpecifyfolderPlay
 * Description: Specify the music index in the folder to play, the index is decided by the input sequence of the music.
 * Parameters: folder: folder name, must be number;  index: the music index.
 * Return: none
****************************************************************/ 
void kt403_SpecifyfolderPlay(byte folder, byte index)
{
	isPlaying = 1;

    kt403_write(0x7E);
    kt403_write(0xFF);
    kt403_write(0x06);
    kt403_write(0x0F);
    kt403_write(0x01); // feedback ON
    kt403_write(folder);
    kt403_write(index);
    kt403_write(0xEF);
    delay(10);
}

/**************************************************************** 
 * Function Name: PlayPause
 * Description: Pause the MP3 player.
 * Parameters: none
 * Return: none
****************************************************************/ 
void kt403_PlayPause(void)
{
    kt403_write(0x7E);
    kt403_write(0xFF);
    kt403_write(0x06);
    kt403_write(0x0E);
    kt403_write(0x01); // feedback ON
    kt403_write(0x00);
    kt403_write(0x00);
    kt403_write(0xEF);

	isPlaying = 0;
}

/**************************************************************** 
 * Function Name: PlayResume
 * Description: Resume the MP3 player.
 * Parameters: none
 * Return: none
****************************************************************/ 
void kt403_PlayResume(void)
{
	isPlaying = 1;

    kt403_write(0x7E);
    kt403_write(0xFF);
    kt403_write(0x06);
    kt403_write(0x0D);
    kt403_write(0x01); // feedback ON
    kt403_write(0x00);
    kt403_write(0x00);
//  kt403_write(0xFE);
//  kt403_write(0xEE);
    kt403_write(0xEF);
}

/**************************************************************** 
 * Function Name: PlayNext
 * Description: Play the next song.
 * Parameters: none
 * Return: none
****************************************************************/ 
void kt403_PlayNext(void)
{
    isPlaying = 1;
    kt403_write(0x7E);
    kt403_write(0xFF);
    kt403_write(0x06);
    kt403_write(0x01);
    kt403_write(0x01); // feedback ON
    kt403_write(0x00);
    kt403_write(0x00);
    kt403_write(0xEF);
}

/**************************************************************** 
 * Function Name: PlayPrevious
 * Description: Play the previous song.
 * Parameters: none
 * Return: none
****************************************************************/ 
void kt403_PlayPrevious(void)
{
    isPlaying = 1;
    kt403_write(0x7E);
    kt403_write(0xFF);
    kt403_write(0x06);
    kt403_write(0x02);
    kt403_write(0x01); // feedback ON
    kt403_write(0x00);
    kt403_write(0x00);
    kt403_write(0xEF);
}

/**************************************************************** 
 * Function Name: PlayLoop
 * Description: Play loop for all the songs.
 * Parameters: none
 * Return: none
****************************************************************/ 
void kt403_PlayLoop(byte loop)
{
    kt403_write(0x7E);
    kt403_write(0xFF);
    kt403_write(0x06);
    kt403_write(0x11);
    kt403_write(0x00); // feedback OFF
    kt403_write(0x00);
    kt403_write(loop != 0);
    kt403_write(0xEF);
}

/**************************************************************** 
 * Function Name: Stop
 * Description: Stop playing any track.
 * Parameters: none
 * Return: none
****************************************************************/ 
void kt403_Stop(void)
{
    kt403_write(0x7E);
    kt403_write(0xFF);
    kt403_write(0x06);
    kt403_write(0x16);
    kt403_write(0x00); // feedback OFF
    kt403_write(0x00);
    kt403_write(0x00);
    kt403_write(0xEF);
    isPlaying = 0;
    delay(10);
}

/**************************************************************** 
 * Function Name: SetVolume
 * Description: Set the volume, the range is 0x00 to 0x1E.
 * Parameters: volume: the range is 0x00 to 0x1E.
 * Return: none
****************************************************************/ 
void kt403_SetVolume(byte volume)
{
    kt403_write(0x7E);
    kt403_write(0xFF);
    kt403_write(0x06);
    kt403_write(0x06);
    kt403_write(0x00);
    kt403_write(0x00);
    kt403_write(volume);
    kt403_write(0xEF);
    delay(10);
}

/**************************************************************** 
 * Function Name: IncreaseVolume
 * Description: Increase the volume.
 * Parameters: none
 * Return: none
****************************************************************/ 
void kt403_IncreaseVolume(void)
{
    kt403_write(0x7E);
    kt403_write(0xFF);
    kt403_write(0x06);
    kt403_write(0x04);
    kt403_write(0x00);
    kt403_write(0x00);
    kt403_write(0x00);
    kt403_write(0xEF);
}

/**************************************************************** 
 * Function Name: DecreaseVolume
 * Description: Decrease the volume.
 * Parameters: none
 * Return: none
****************************************************************/ 
void kt403_DecreaseVolume(void)
{
    kt403_write(0x7E);
    kt403_write(0xFF);
    kt403_write(0x06);
    kt403_write(0x05);
    kt403_write(0x00);
    kt403_write(0x00);
    kt403_write(0x00);
    kt403_write(0xEF);
}

/**************************************************************** 
 * Function Name: QueryPlayStatus
 * Description: Query play status.
 * Parameters: none
 * Return: 0: played out; 1: other.
 * Usage: while(QueryPlayStatus() != 0);  // Waiting to play out.
****************************************************************/ 
byte kt403_IsPlaying(void)
{
    return (isPlaying /*== 65*/);
}

void kt403_printStatus()
{
	printf("C kt403 %d %d %d %d %d\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
}
