/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#include "nRF24L01.h"
#include "RF24.h"
#include <fruit.h>
#include <spimaster.h>

#define FAILURE_HANDLING
#define rf24_max(a,b) (a>b?a:b)
#define rf24_min(a,b) (a<b?a:b)
#define _BV(x) (1<<(x))

#ifndef true
#define true 1
#define false 0
#endif

#ifndef LOW
#define LOW 0
#define HIGH 1
#endif

#if RF24_SPI == 1
#define SPITRANSFER SPImaster1Transfer
#define SPIMASTERINIT SPImaster1Init
#elif RF24_SPI == 2
#define SPITRANSFER SPImaster2Transfer
#define SPIMASTERINIT SPImaster2Init
#else
#error RF24_SPI should be 1 or 2!
#endif

static bool p_variant; /* False for RF24L01 and true for RF24L01P */
static uint8_t payload_size; /**< Fixed size of payloads */
static bool dynamic_payloads_enabled; /**< Whether dynamic payloads are enabled. */
static uint8_t pipe0_reading_address[5]; /**< Last address set on pipe 0 for reading. */
static uint8_t addr_width; /**< The address width to use - 3,4 or 5 bytes. */

/**
  * 
  * The driver will delay for this duration when stopListening() is called
  * 
  * When responding to payloads, faster devices like ARM(RPi) are much faster than Arduino:
  * 1. Arduino sends data to RPi, switches to RX mode
  * 2. The RPi receives the data, switches to TX mode and sends before the Arduino radio is in RX mode
  * 3. If AutoACK is disabled, this can be set as low as 0. If AA/ESB enabled, set to 100uS minimum on RPi
  *
  * @warning If set to 0, ensure 130uS delay after stopListening() and before any sends
  */
  
  uint32_t txDelay;

  /**
  * 
  * On all devices but Linux and ATTiny, a small delay is added to the CSN toggling function
  * 
  * This is intended to minimise the speed of SPI polling due to radio commands
  *
  * If using interrupts or timed requests, this can be set to 0 Default:5
  */
  
  uint32_t csDelay;

/****************************************************************************/

static unsigned long int millis()
{
	return time()/1000;
}

/****************************************************************************/

static void delay(unsigned long millisecs)
{
	t_delay del;
	delayStart(del, millisecs * 1000);
	while(!delayFinished(del)){}
}

/****************************************************************************/

static void delayMicroseconds(unsigned long microsecs)
{
	t_delay del;
	delayStart(del, microsecs);
	while(!delayFinished(del)){}
}

/****************************************************************************/

static void csn(bool mode)
{
	digitalWrite(RF24_CSN,mode);
	delayMicroseconds(5);
}

/****************************************************************************/

static void ce(bool level)
{
	digitalWrite(RF24_CE,level);
}

/****************************************************************************/

static void beginTransaction() {
    csn(LOW);
}

/****************************************************************************/

static void endTransaction() {
    csn(HIGH);
}

/****************************************************************************/

uint8_t RF24_read_registers(uint8_t reg, uint8_t* buf, uint8_t len)
{
  uint8_t status;

  beginTransaction();
  status = SPITRANSFER( R_REGISTER | ( REGISTER_MASK & reg ) );
  while ( len-- ){
    *buf++ = SPITRANSFER(0xff);
  }
  endTransaction();

  return status;
}

/****************************************************************************/

static uint8_t read_register(uint8_t reg)
{
  uint8_t result;
  
  beginTransaction();
  SPITRANSFER( R_REGISTER | ( REGISTER_MASK & reg ) );
  result = SPITRANSFER(0xff);
  endTransaction();

  return result;
}

/****************************************************************************/

static uint8_t write_registers(uint8_t reg, const uint8_t* buf, uint8_t len)
{
  uint8_t status;

  beginTransaction();
  status = SPITRANSFER( W_REGISTER | ( REGISTER_MASK & reg ) );
  while ( len-- )
    SPITRANSFER(*buf++);
  endTransaction();

  return status;
}

/****************************************************************************/

static uint8_t write_register(uint8_t reg, uint8_t value)
{
  uint8_t status;

  beginTransaction();
  status = SPITRANSFER( W_REGISTER | ( REGISTER_MASK & reg ) );
  SPITRANSFER(value);
  endTransaction();

  return status;
}

/****************************************************************************/

static uint8_t write_payload(const void* buf, uint8_t data_len, const uint8_t writeType)
{
  uint8_t status;
  const uint8_t* current = (const uint8_t*)(buf);
  uint8_t blank_len;

  data_len = rf24_min(data_len, payload_size);
  
  blank_len = dynamic_payloads_enabled ? 0 : payload_size - data_len;
  
  beginTransaction();
  status = SPITRANSFER( writeType );
  while ( data_len-- ) {
    SPITRANSFER(*current++);
  }
  while ( blank_len-- ) {
    SPITRANSFER(0);
  }  
  endTransaction();

  return status;
}

/****************************************************************************/

static uint8_t read_payload(void* buf, uint8_t data_len)
{
  uint8_t status;
  uint8_t* current = (uint8_t*)(buf);
  uint8_t blank_len;

  if(data_len > payload_size) data_len = payload_size;
  blank_len = dynamic_payloads_enabled ? 0 : payload_size - data_len;
  
  beginTransaction();
  status = SPITRANSFER( R_RX_PAYLOAD );
  while ( data_len-- ) {
    *current++ = SPITRANSFER(0xFF);
  }
  while ( blank_len-- ) {
    SPITRANSFER(0xff);
  }
  endTransaction();

  return status;
}

/****************************************************************************/

static uint8_t spiTrans(uint8_t cmd){

  uint8_t status;
  
  beginTransaction();
  status = SPITRANSFER( cmd );
  endTransaction();
  
  return status;
}

/****************************************************************************/

uint8_t RF24_flush_rx(void)
{
  return spiTrans( FLUSH_RX );
}

/****************************************************************************/

uint8_t RF24_flush_tx(void)
{
  return spiTrans( FLUSH_TX );
}

/****************************************************************************/

uint8_t RF24_get_status(void)
{
  return spiTrans(RF24_NOP);
}

/****************************************************************************/

void RF24_setChannel(uint8_t channel)
{
  const uint8_t max_channel = 125;
  write_register(RF_CH,rf24_min(channel,max_channel));
}

uint8_t RF24_getChannel()
{
  
  return read_register(RF_CH);
}
/****************************************************************************/

void RF24_setPayloadSize(uint8_t size)
{
  payload_size = rf24_min(size,32);
}

/****************************************************************************/

uint8_t RF24_getPayloadSize(void)
{
  return payload_size;
}

/****************************************************************************/

void RF24_toggle_features(void)
{
    beginTransaction();
	SPITRANSFER( ACTIVATE );
    SPITRANSFER( 0x73 );
	endTransaction();
}

/****************************************************************************/

bool RF24_init(void)
{

  uint8_t setup=0;

  p_variant = false;
  payload_size = 32;
  dynamic_payloads_enabled = false;
  addr_width = 5;
  csDelay = 5;
  pipe0_reading_address[0]=0;

  /*SPIMASTERINIT();
  pinModeDigitalOut(RF24_CE);
  pinModeDigitalOut(RF24_CSN);

  delay(5);
  ce(LOW);
  csn(HIGH);
  delay(5);*/

  ce(LOW);
  csn(HIGH);
  pinModeDigitalOut(RF24_CE);
  pinModeDigitalOut(RF24_CSN);

//  ce(HIGH);
//  csn(LOW);
  ce(LOW);
  csn(HIGH);

  SPIMASTERINIT();

  /*csn(LOW);
  Nop(); Nop();Nop(); Nop();Nop(); Nop();Nop(); Nop();
  csn(HIGH);*/

  // Must allow the radio time to settle else configuration bits will not necessarily stick.
  // This is actually only required following power up but some settling time also appears to
  // be required after resets too. For full coverage, we'll always assume the worst.
  // Enabling 16b CRC is by far the most obvious case if the wrong timing is used - or skipped.
  // Technically we require 4.5ms + 14us as a worst case. We'll just call it 5ms for good measure.
  // WARNING: Delay is based on P-variant whereby non-P *may* require different timing.
  delay( 5 ) ;
  //delay( 20 ) ;

  // Reset NRF_CONFIG and enable 16-bit CRC.
  write_register( NRF_CONFIG, 0x0C ) ;

  write_register(EN_RXADDR,0); // disable all pipes
  
  // Set 1500uS (minimum for 32B payload in ESB@250KBPS) timeouts, to make testing a little easier
  // WARNING: If this is ever lowered, either 250KBS mode with AA is broken or maximum packet
  // sizes must never be used. See documentation for a more complete explanation.
  RF24_setRetries(5,15);

  // Reset value is MAX
  //setPALevel( RF24_PA_MAX ) ;

  write_register(RF_SETUP, 0b00001110 );
  // check for connected module and if this is a p nRF24l01 variant
  //
  //return;
  if( RF24_setDataRate( RF24_250KBPS ) )
  {
    p_variant = true ;
  }
  //setup = read_register(RF_SETUP);
  /*if( setup == 0b00001110 )     // register default for nRF24L01P
  {
    p_variant = true ;
  }*/
  
  // Then set the data rate to the slowest (and most reliable) speed supported by all
  // hardware.
  RF24_setDataRate( RF24_1MBPS ) ;

  // Initialize CRC and request 2-byte (16bit) CRC
  //setCRCLength( RF24_CRC_16 ) ;

  // Disable dynamic payloads, to match dynamic_payloads_enabled setting - Reset value is 0
  //RF24_toggle_features(); // no effect on 24L01+
  write_register(FEATURE,0 );
  write_register(DYNPD,0);
  dynamic_payloads_enabled = false;

  // Reset current status
  // Notice reset and flush is the last thing we do
  write_register(NRF_STATUS,_BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT) );

  // Set up default configuration.  Callers can always change it later.
  // This channel should be universally safe and not bleed over into adjacent
  // spectrum.
  RF24_setChannel(76);

  // Flush buffers
  RF24_flush_rx();
  RF24_flush_tx();

  RF24_powerUp(); //Power up by default when begin() is called

  // Enable PTX, do not write CE high so radio will remain in standby I mode ( 130us max to transition to RX or TX instead of 1500us from powerUp )
  // PTX should use only 22uA of power
  write_register(NRF_CONFIG, ( read_register(NRF_CONFIG) ) & ~_BV(PRIM_RX) );

  // if setup is 0 or ff then there was no response from module
  return ( setup != 0 && setup != 0xff );
}

/****************************************************************************/

bool RF24_isChipConnected()
{
  uint8_t setup = read_register(SETUP_AW);
  if(setup >= 1 && setup <= 3)
  {
    return true;
  }

  return false;
}

/****************************************************************************/

void RF24_startListening(void)
{
  RF24_powerUp();
  write_register(NRF_CONFIG, read_register(NRF_CONFIG) | _BV(PRIM_RX));
  write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT) );
  ce(HIGH);
  // Restore the pipe0 adddress, if exists
  if (pipe0_reading_address[0] > 0){
    write_registers(RX_ADDR_P0, pipe0_reading_address, addr_width);	
  }else{
	RF24_closeReadingPipe(0);
  }

  // Flush buffers
  //flush_rx();
  if(read_register(FEATURE) & _BV(EN_ACK_PAY)){
	RF24_flush_tx();
  }

  // Go!
  //delayMicroseconds(100);
}

/****************************************************************************/
static const uint8_t child_pipe_enable[] =
{
  ERX_P0, ERX_P1, ERX_P2, ERX_P3, ERX_P4, ERX_P5
};

void RF24_stopListening(void)
{  
  ce(LOW);

  delayMicroseconds(txDelay);
  
  if(read_register(FEATURE) & _BV(EN_ACK_PAY)){
    delayMicroseconds(txDelay); //200
	RF24_flush_tx();
  }
  //flush_rx();
  write_register(NRF_CONFIG, ( read_register(NRF_CONFIG) ) & ~_BV(PRIM_RX) );
 
  write_register(EN_RXADDR,read_register(EN_RXADDR) | _BV(child_pipe_enable[0])); // Enable RX on pipe0
  
  //delayMicroseconds(100);

}

/****************************************************************************/

void RF24_powerDown(void)
{
  ce(LOW); // Guarantee CE is low on powerDown
  write_register(NRF_CONFIG,read_register(NRF_CONFIG) & ~_BV(PWR_UP));
}

/****************************************************************************/

//Power up now. Radio will not power down unless instructed by MCU for config changes etc.
void RF24_powerUp(void)
{
   uint8_t cfg = read_register(NRF_CONFIG);

   // if not powered up then power up and wait for the radio to initialize
   if (!(cfg & _BV(PWR_UP))){
      write_register(NRF_CONFIG, cfg | _BV(PWR_UP));

      // For nRF24L01+ to go from power down mode to TX or RX mode it must first pass through stand-by mode.
	  // There must be a delay of Tpd2stby (see Table 16.) after the nRF24L01+ leaves power down mode before
	  // the CEis set high. - Tpd2stby can be up to 5ms per the 1.0 datasheet
      delay(5);
   }
}

/****************************************************************************/

//Per the documentation, we want to set PTX Mode when not listening. Then all we do is write data and set CE high
//In this mode, if we can keep the FIFO buffers loaded, packets will transmit immediately (no 130us delay)
//Otherwise we enter Standby-II mode, which is still faster than standby mode
//Also, we remove the need to keep writing the config register over and over and delaying for 150 us each time if sending a stream of data

void RF24_startFastWrite( const void* buf, uint8_t len, const bool multicast) {//, bool startTx){ //TMRh20

	//write_payload( buf,len);
	write_payload( buf, len,multicast ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD ) ;
	//if(startTx){
		ce(HIGH);
	//}

}

/****************************************************************************/

//Added the original startWrite back in so users can still use interrupts, ack payloads, etc
//Allows the library to pass all tests
void RF24_startWrite( const void* buf, uint8_t len, const bool multicast ){

  // Send the payload

  //write_payload( buf, len );
  write_payload( buf, len,multicast? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD ) ;
  ce(HIGH);
  #if !defined(F_CPU) || F_CPU > 20000000
	delayMicroseconds(10);
  #endif
  ce(LOW);
}

/******************************************************************/
#define errNotify()

//Similar to the previous write, clears the interrupt flags
bool RF24_write( const void* buf, uint8_t len, const bool multicast )
{
	uint8_t status;
#if defined (FAILURE_HANDLING) || defined (RF24_LINUX)
	uint32_t timer;
#endif 

	//Start Writing
	RF24_startFastWrite(buf,len,multicast);

	//Wait until complete or failed
#if defined (FAILURE_HANDLING)
	timer = millis();
#endif 
	
	while( ! ( RF24_get_status()  & ( _BV(TX_DS) | _BV(MAX_RT) ))) { 
		#if defined (FAILURE_HANDLING) || defined (RF24_LINUX)
			if(millis() - timer > 95){			
				errNotify();
				#if defined (FAILURE_HANDLING)
				  return 0;		
				#else
				  delay(100);
				#endif
			}
		#endif
	}
    
	ce(LOW);

	status = write_register(NRF_STATUS,_BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT) );

  //Max retries exceeded
  if( status & _BV(MAX_RT)){
  	RF24_flush_tx(); //Only going to be 1 packet int the FIFO at a time using this method, so just flush
  	return 0;
  }
	//TX OK 1 or 0
  return 1;
}

/*bool RF24_write( const void* buf, uint8_t len ){
	return write(buf,len,0);
}*/

/****************************************************************************/

//For general use, the interrupt flags are not important to clear
bool RF24_writeBlocking( const void* buf, uint8_t len, uint32_t timeout )
{
	//Block until the FIFO is NOT full.
	//Keep track of the MAX retries and set auto-retry if seeing failures
	//This way the FIFO will fill up and allow blocking until packets go through
	//The radio will auto-clear everything in the FIFO as long as CE remains high

	uint32_t timer = millis();							  //Get the time that the payload transmission started

	while( ( RF24_get_status()  & ( _BV(TX_FULL) ))) {		  //Blocking only if FIFO is full. This will loop and block until TX is successful or timeout

		if( RF24_get_status() & _BV(MAX_RT)){					  //If MAX Retries have been reached
			RF24_reUseTX();										  //Set re-transmit and clear the MAX_RT interrupt flag
			if(millis() - timer > timeout){ return 0; }		  //If this payload has exceeded the user-defined timeout, exit and return 0
		}
		#if defined (FAILURE_HANDLING) || defined (RF24_LINUX)
			if(millis() - timer > (timeout+95) ){			
				errNotify();
				#if defined (FAILURE_HANDLING)
				return 0;			
                #endif				
			}
		#endif

  	}

  	//Start Writing
	RF24_startFastWrite(buf,len,0);								  //Write the payload if a buffer is clear

	return 1;												  //Return 1 to indicate successful transmission
}

/****************************************************************************/

void RF24_reUseTX(){
		write_register(NRF_STATUS,_BV(MAX_RT) );			  //Clear max retry flag
		spiTrans( REUSE_TX_PL );
		ce(LOW);										  //Re-Transfer packet
		ce(HIGH);
}

/****************************************************************************/

bool RF24_writeFast( const void* buf, uint8_t len, const bool multicast )
{
	//Block until the FIFO is NOT full.
	//Keep track of the MAX retries and set auto-retry if seeing failures
	//Return 0 so the user can control the retrys and set a timer or failure counter if required
	//The radio will auto-clear everything in the FIFO as long as CE remains high

	#if defined (FAILURE_HANDLING) || defined (RF24_LINUX)
		uint32_t timer = millis();
	#endif
	
	while( ( RF24_get_status()  & ( _BV(TX_FULL) ))) {			  //Blocking only if FIFO is full. This will loop and block until TX is successful or fail

		if( RF24_get_status() & _BV(MAX_RT)){
			//reUseTX();										  //Set re-transmit
			write_register(NRF_STATUS,_BV(MAX_RT) );			  //Clear max retry flag
			return 0;										  //Return 0. The previous payload has been retransmitted
															  //From the user perspective, if you get a 0, just keep trying to send the same payload
		}
		#if defined (FAILURE_HANDLING) || defined (RF24_LINUX)
			if(millis() - timer > 95 ){			
				errNotify();
				#if defined (FAILURE_HANDLING)
				return 0;							
				#endif
			}
		#endif
  	}
		     //Start Writing
	RF24_startFastWrite(buf,len,multicast);

	return 1;
}

/*bool RF24_writeFast( const void* buf, uint8_t len ){
	return RF24_writeFast(buf,len,0);
}*/

/****************************************************************************/

bool RF24_rxFifoFull(){
	return read_register(FIFO_STATUS) & _BV(RX_FULL);
}
/****************************************************************************/

bool RF24_txStandBy(){

    #if defined (FAILURE_HANDLING) || defined (RF24_LINUX)
		uint32_t timeout = millis();
	#endif
	while( ! (read_register(FIFO_STATUS) & _BV(TX_EMPTY)) ){
		if( RF24_get_status() & _BV(MAX_RT)){
			write_register(NRF_STATUS,_BV(MAX_RT) );
			ce(LOW);
			RF24_flush_tx();    //Non blocking, flush the data
			return 0;
		}
		#if defined (FAILURE_HANDLING) || defined (RF24_LINUX)
			if( millis() - timeout > 95){
				errNotify();
				#if defined (FAILURE_HANDLING)
				return 0;	
				#endif
			}
		#endif
	}

	ce(LOW);			   //Set STANDBY-I mode
	return 1;
}

/****************************************************************************/

bool RF24_txStandByTimeout(uint32_t timeout, bool startTx){

	uint32_t start;
	
    if(startTx){
	  RF24_stopListening();
	  ce(HIGH);
	}
	start = millis();

	while( ! (read_register(FIFO_STATUS) & _BV(TX_EMPTY)) ){
		if( RF24_get_status() & _BV(MAX_RT)){
			write_register(NRF_STATUS,_BV(MAX_RT) );
				ce(LOW);										  //Set re-transmit
				ce(HIGH);
				if(millis() - start >= timeout){
					ce(LOW); RF24_flush_tx(); return 0;
				}
		}
		#if defined (FAILURE_HANDLING) || defined (RF24_LINUX)
			if( millis() - start > (timeout+95)){
				errNotify();
				#if defined (FAILURE_HANDLING)
				return 0;	
				#endif
			}
		#endif
	}

	
	ce(LOW);				   //Set STANDBY-I mode
	return 1;

}

/****************************************************************************/

void RF24_maskIRQ(bool tx, bool fail, bool rx){

	uint8_t config = read_register(NRF_CONFIG);
	/* clear the interrupt flags */
	config &= ~(1 << MASK_MAX_RT | 1 << MASK_TX_DS | 1 << MASK_RX_DR);
	/* set the specified interrupt flags */
	config |= fail << MASK_MAX_RT | tx << MASK_TX_DS | rx << MASK_RX_DR;
	write_register(NRF_CONFIG, config);
}

/****************************************************************************/

uint8_t RF24_getDynamicPayloadSize(void)
{
  uint8_t result = 0;

  beginTransaction();
  SPITRANSFER( R_RX_PL_WID );
  result = SPITRANSFER(0xff);
  endTransaction();

  if(result > 32) { RF24_flush_rx(); delay(2); return 0; }
  return result;
}

/****************************************************************************/

bool RF24_available(void)
{
  return RF24_pipeAvailable(NULL);
}

/****************************************************************************/

bool RF24_pipeAvailable(uint8_t* pipe_num)
{
  if (!( read_register(FIFO_STATUS) & _BV(RX_EMPTY) )){

    // If the caller wants the pipe number, include that
    if ( pipe_num ){
	  uint8_t status = RF24_get_status();
      *pipe_num = ( status >> RX_P_NO ) & 0x07;
  	}
  	return 1;
  }


  return 0;


}

/****************************************************************************/

void RF24_read( void* buf, uint8_t len ){

  // Fetch the payload
  read_payload( buf, len );

  //Clear the two possible interrupt flags with one command
  write_register(NRF_STATUS,_BV(RX_DR) | _BV(MAX_RT) | _BV(TX_DS) );

}

/****************************************************************************/

void RF24_whatHappened(bool* tx_ok,bool* tx_fail,bool* rx_ready)
{
  // Read the status & reset the status in one easy call
  // Or is that such a good idea?
  uint8_t status = write_register(NRF_STATUS,_BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT) );

  // Report to the user what happened
  *tx_ok = status & _BV(TX_DS);
  *tx_fail = status & _BV(MAX_RT);
  *rx_ready = status & _BV(RX_DR);
}

#if 0
/****************************************************************************/

void RF24_openWritingPipe(uint64_t value)
{
  // Note that AVR 8-bit uC's store this LSB first, and the NRF24L01(+)
  // expects it LSB first too, so we're good.

  write_registers(RX_ADDR_P0,(uint8_t*)(&value), addr_width);
  write_registers(TX_ADDR, (uint8_t*)(&value), addr_width);
  
  
  //const uint8_t max_payload_size = 32;
  //write_register(RX_PW_P0,rf24_min(payload_size,max_payload_size));
  write_register(RX_PW_P0,payload_size);
}
#endif

/****************************************************************************/
void RF24_openWritingPipe(const uint8_t *address)
{
  // Note that AVR 8-bit uC's store this LSB first, and the NRF24L01(+)
  // expects it LSB first too, so we're good.

  write_registers(RX_ADDR_P0,address, addr_width);
  write_registers(TX_ADDR, address, addr_width);

  //const uint8_t max_payload_size = 32;
  //write_register(RX_PW_P0,rf24_min(payload_size,max_payload_size));
  write_register(RX_PW_P0,payload_size);
}

/****************************************************************************/
static const uint8_t child_pipe[] =
{
  RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2, RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5
};
static const uint8_t child_payload_size[] =
{
  RX_PW_P0, RX_PW_P1, RX_PW_P2, RX_PW_P3, RX_PW_P4, RX_PW_P5
};


#if 0
void RF24_openReadingPipe(uint8_t child, uint64_t address)
{
  // If this is pipe 0, cache the address.  This is needed because
  // openWritingPipe() will overwrite the pipe 0 address, so
  // startListening() will have to restore it.
  if (child == 0){
    memcpy(pipe0_reading_address,&address,addr_width);
  }

  if (child <= 6)
  {
    // For pipes 2-5, only write the LSB
    if ( child < 2 )
      write_registers(child_pipe[child], (const uint8_t*)(&address), addr_width);
    else
      write_registers(child_pipe[child], (const uint8_t*)(&address), 1);

    write_registers(child_payload_size[child],payload_size);

    // Note it would be more efficient to set all of the bits for all open
    // pipes at once.  However, I thought it would make the calling code
    // more simple to do it this way.
    write_register(EN_RXADDR,read_register(EN_RXADDR) | _BV(child_pipe_enable[child]));
  }
}
#endif

/****************************************************************************/
void RF24_setAddressWidth(uint8_t a_width){

	if(a_width -= 2){
		write_register(SETUP_AW,a_width%4);
		addr_width = (a_width%4) + 2;
	}else{
        write_register(SETUP_AW,0);
        addr_width = 2;
    }

}

/****************************************************************************/

void RF24_openReadingPipe(uint8_t child, const uint8_t *address)
{
  // If this is pipe 0, cache the address.  This is needed because
  // openWritingPipe() will overwrite the pipe 0 address, so
  // startListening() will have to restore it.
  if (child == 0){
    memcpy(pipe0_reading_address,address,addr_width);
  }
  if (child <= 6)
  {
    // For pipes 2-5, only write the LSB
    if ( child < 2 ){
      write_registers(child_pipe[child], address, addr_width);
    }else{
      write_registers(child_pipe[child], address, 1);
	}
    write_register(child_payload_size[child],payload_size);

    // Note it would be more efficient to set all of the bits for all open
    // pipes at once.  However, I thought it would make the calling code
    // more simple to do it this way.
    write_register(EN_RXADDR,read_register(EN_RXADDR) | _BV(child_pipe_enable[child]));

  }
}

/****************************************************************************/

void RF24_closeReadingPipe( uint8_t pipe )
{
  write_register(EN_RXADDR,read_register(EN_RXADDR) & ~_BV(child_pipe_enable[pipe]));
}

/****************************************************************************/

void RF24_enableDynamicPayloads(void)
{
  // Enable dynamic payload throughout the system

    //toggle_features();
    write_register(FEATURE,read_register(FEATURE) | _BV(EN_DPL) );


  // Enable dynamic payload on all pipes
  //
  // Not sure the use case of only having dynamic payload on certain
  // pipes, so the library does not support it.
  write_register(DYNPD,read_register(DYNPD) | _BV(DPL_P5) | _BV(DPL_P4) | _BV(DPL_P3) | _BV(DPL_P2) | _BV(DPL_P1) | _BV(DPL_P0));

  dynamic_payloads_enabled = true;
}

/****************************************************************************/
void RF24_disableDynamicPayloads(void)
{
  // Disables dynamic payload throughout the system.  Also disables Ack Payloads

  //toggle_features();
  write_register(FEATURE, 0);


  // Disable dynamic payload on all pipes
  //
  // Not sure the use case of only having dynamic payload on certain
  // pipes, so the library does not support it.
  write_register(DYNPD, 0);

  dynamic_payloads_enabled = false;
}

/****************************************************************************/

void RF24_enableAckPayload(void)
{
  //
  // enable ack payload and dynamic payload features
  //

    //toggle_features();
    write_register(FEATURE,read_register(FEATURE) | _BV(EN_ACK_PAY) | _BV(EN_DPL) );

  //
  // Enable dynamic payload on pipes 0 & 1
  //

  write_register(DYNPD,read_register(DYNPD) | _BV(DPL_P1) | _BV(DPL_P0));
  dynamic_payloads_enabled = true;
}

/****************************************************************************/

void RF24_enableDynamicAck(void){
  //
  // enable dynamic ack features
  //
    //toggle_features();
    write_register(FEATURE,read_register(FEATURE) | _BV(EN_DYN_ACK) );
}

/****************************************************************************/

void RF24_writeAckPayload(uint8_t pipe, const void* buf, uint8_t len)
{
  const uint8_t* current = (const uint8_t*)(buf);

  uint8_t data_len = rf24_min(len,32);

  beginTransaction();
  SPITRANSFER(W_ACK_PAYLOAD | ( pipe & 0x07 ) );

  while ( data_len-- )
    SPITRANSFER(*current++);
  endTransaction();
}

/****************************************************************************/

bool RF24_isAckPayloadAvailable(void)
{
  return ! (read_register(FIFO_STATUS) & _BV(RX_EMPTY));
}

/****************************************************************************/

bool RF24_isPVariant(void)
{
  return p_variant ;
}

/****************************************************************************/

void RF24_setAutoAck(bool enable)
{
  if ( enable )
    write_register(EN_AA, 0x3F);
  else
    write_register(EN_AA, 0);
}

/****************************************************************************/

void RF24_setAutoAckPipe( uint8_t pipe, bool enable )
{
  if ( pipe <= 6 )
  {
    uint8_t en_aa = read_register( EN_AA ) ;
    if( enable )
    {
      en_aa |= _BV(pipe) ;
    }
    else
    {
      en_aa &= ~_BV(pipe) ;
    }
    write_register( EN_AA, en_aa ) ;
  }
}

/****************************************************************************/

bool RF24_testCarrier(void)
{
  return ( read_register(CD) & 1 );
}

/****************************************************************************/

bool RF24_testRPD(void)
{
  return ( read_register(RPD) & 1 ) ;
}

/****************************************************************************/

void RF24_setPALevel(uint8_t level)
{

  uint8_t setup = read_register(RF_SETUP) & 0xF8;

  if(level > 3){  						// If invalid level, go to max PA
	  level = (RF24_PA_MAX << 1) + 1;		// +1 to support the SI24R1 chip extra bit
  }else{
	  level = (level << 1) + 1;	 		// Else set level as requested
  }


  write_register( RF_SETUP, setup |= level ) ;	// Write it to the chip
}

/****************************************************************************/

uint8_t RF24_getPALevel(void)
{

  return (read_register(RF_SETUP) & (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH))) >> 1 ;
}

/****************************************************************************/

bool RF24_setDataRate(rf24_datarate_e speed)
{
  bool result = false;
  uint8_t setup = read_register(RF_SETUP) ;

  // HIGH and LOW '00' is 1Mbs - our default
  setup &= ~(_BV(RF_DR_LOW) | _BV(RF_DR_HIGH)) ;
  
  #if !defined(F_CPU) || F_CPU > 20000000
    txDelay=250;
  #else //16Mhz Arduino
    txDelay=85;
  #endif
  if( speed == RF24_250KBPS )
  {
    // Must set the RF_DR_LOW to 1; RF_DR_HIGH (used to be RF_DR) is already 0
    // Making it '10'.
    setup |= _BV( RF_DR_LOW ) ;
  #if !defined(F_CPU) || F_CPU > 20000000
    txDelay=450;
  #else //16Mhz Arduino
	txDelay=155;
  #endif
  }
  else
  {
    // Set 2Mbs, RF_DR (RF_DR_HIGH) is set 1
    // Making it '01'
    if ( speed == RF24_2MBPS )
    {
      setup |= _BV(RF_DR_HIGH);
      #if !defined(F_CPU) || F_CPU > 20000000
      txDelay=190;
      #else //16Mhz Arduino	  
	  txDelay=65;
	  #endif
    }
  }
  write_register(RF_SETUP,setup);

  // Verify our result
  if ( read_register(RF_SETUP) == setup )
  {
    result = true;
  }
  return result;
}

/****************************************************************************/

rf24_datarate_e RF24_getDataRate( void )
{
  rf24_datarate_e result ;
  uint8_t dr = read_register(RF_SETUP) & (_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));

  // switch uses RAM (evil!)
  // Order matters in our case below
  if ( dr == _BV(RF_DR_LOW) )
  {
    // '10' = 250KBPS
    result = RF24_250KBPS ;
  }
  else if ( dr == _BV(RF_DR_HIGH) )
  {
    // '01' = 2MBPS
    result = RF24_2MBPS ;
  }
  else
  {
    // '00' = 1MBPS
    result = RF24_1MBPS ;
  }
  return result ;
}

/****************************************************************************/

void RF24_setCRCLength(rf24_crclength_e length)
{
  uint8_t config = read_register(NRF_CONFIG) & ~( _BV(CRCO) | _BV(EN_CRC)) ;

  // switch uses RAM (evil!)
  if ( length == RF24_CRC_DISABLED )
  {
    // Do nothing, we turned it off above.
  }
  else if ( length == RF24_CRC_8 )
  {
    config |= _BV(EN_CRC);
  }
  else
  {
    config |= _BV(EN_CRC);
    config |= _BV( CRCO );
  }
  write_register( NRF_CONFIG, config ) ;
}

/****************************************************************************/

rf24_crclength_e RF24_getCRCLength(void)
{
  rf24_crclength_e result = RF24_CRC_DISABLED;
  
  uint8_t config = read_register(NRF_CONFIG) & ( _BV(CRCO) | _BV(EN_CRC)) ;
  uint8_t AA = read_register(EN_AA);
  
  if ( config & _BV(EN_CRC ) || AA)
  {
    if ( config & _BV(CRCO) )
      result = RF24_CRC_16;
    else
      result = RF24_CRC_8;
  }

  return result;
}

/****************************************************************************/

void RF24_disableCRC( void )
{
  uint8_t disable = read_register(NRF_CONFIG) & ~_BV(EN_CRC) ;
  write_register( NRF_CONFIG, disable ) ;
}

/****************************************************************************/
void RF24_setRetries(uint8_t delay, uint8_t count)
{
  write_register(SETUP_RETR,(delay&0xf)<<ARD | (count&0xf)<<ARC);
}

uint8_t RF24_read_register(uint8_t reg)
{
	return read_register(reg);
}

void RF24_read_pipes_addresses(uint8_t *buf) // buf length must be at least 5+5+4=14
{
	RF24_read_registers(child_pipe[0], buf, 5);
	RF24_read_registers(child_pipe[1], buf+5, 5);
	RF24_read_registers(child_pipe[2], buf+10, 1);
	RF24_read_registers(child_pipe[3], buf+11, 1);
	RF24_read_registers(child_pipe[4], buf+12, 1);
	RF24_read_registers(child_pipe[5], buf+13, 1);
}

