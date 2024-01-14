/*
 Copyright (C) 2019 Antoine Rousseau
 from 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * @file RF24.h
 *
 * RF24 Fraise module declarations
 */

/** @defgroup RF24 nRF24L01+ module
 *  2.4GHz wireless communication.
 */

#ifndef __RF24_H__
#define __RF24_H__

#include <fruit.h>

//#define RF24_SPI_CE K1
//#define RF24_SPI_CSN K2
//#define RF24_SPI 1/2

typedef unsigned char bool;

/**
 * Power Amplifier level.
 *
 * For use with setPALevel()
 */
typedef enum { RF24_PA_MIN = 0,RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX, RF24_PA_ERROR } rf24_pa_dbm_e ;

/**
 * Data rate.  How fast data moves through the air.
 *
 * For use with setDataRate()
 */
typedef enum { RF24_1MBPS = 0, RF24_2MBPS, RF24_250KBPS } rf24_datarate_e;

/**
 * CRC Length.  How big (if any) of a CRC is included.
 *
 * For use with setCRCLength()
 */
typedef enum { RF24_CRC_DISABLED = 0, RF24_CRC_8, RF24_CRC_16 } rf24_crclength_e;

/**
 * Driver for nRF24L01(+) 2.4GHz Wireless Transceiver
 */

bool RF24_init();

  /**
   * Checks if the chip is connected to the SPI bus
   */
bool RF24_isChipConnected();

  /**
   * Start listening on the pipes opened for reading.
   *
   * 1. Be sure to call openReadingPipe() first.  
   * 2. Do not call write() while in this mode, without first calling stopListening().
   * 3. Call available() to check for incoming traffic, and read() to get it. 
   *  
   * @code
   * Open reading pipe 1 using address CCCECCCECC
   *  
   * byte address[] = { 0xCC,0xCE,0xCC,0xCE,0xCC };
   * radio.openReadingPipe(1,address);
   * radio.startListening();
   * @endcode
   */
  void RF24_startListening(void);
  void RF24_startListeningFast(void);
  /**
   * Stop listening for incoming messages, and switch to transmit mode.
   *
   * Do this before calling write().
   * @code
   * radio.stopListening();
   * radio.write(&data,sizeof(data));
   * @endcode
   */
  void RF24_stopListening(void);
  void RF24_stopListeningFast(void);
  /**
   * Check whether there are bytes available to be read
   * @code
   * if(radio.available()){
   *   radio.read(&data,sizeof(data));
   * }
   * @endcode
   * @return True if there is a payload available, false if none is
   */
  bool RF24_available(void);

  /**
   * Read the available payload
   *
   * The size of data read is the fixed payload size, see getPayloadSize()
   *
   * @note I specifically chose 'void*' as a data type to make it easier
   * for beginners to use.  No casting needed.
   *
   * @note No longer boolean. Use available to determine if packets are
   * available. Interrupt flags are now cleared during reads instead of
   * when calling available().
   *
   * @param buf Pointer to a buffer where the data should be written
   * @param len Maximum number of bytes to read into the buffer
   *
   * @code
   * if(radio.available()){
   *   radio.read(&data,sizeof(data));
   * }
   * @endcode
   * @return No return value. Use available().
   */
  void RF24_read(void* buf, unsigned char len);

  /**
   * Be sure to call openWritingPipe() first to set the destination
   * of where to write to.
   *
   * This blocks until the message is successfully acknowledged by
   * the receiver or the timeout/retransmit maxima are reached.  In
   * the current configuration, the max delay here is 60-70ms.
   *
   * The maximum size of data written is the fixed payload size, see
   * getPayloadSize().  However, you can write less, and the remainder
   * will just be filled with zeroes.
   *
   * TX/RX/RT interrupt flags will be cleared every time write is called
   *
   * @param buf Pointer to the data to be sent
   * @param len Number of bytes to be sent
   *
   * @code
   * radio.stopListening();
   * radio.write(&data,sizeof(data));
   * @endcode
   * @return True if the payload was delivered successfully and an ACK was received, or upon successfull transmission if auto-ack is disabled.
   */
  //bool RF24_write( const void* buf, uint8_t len );

  /**
   * New: Open a pipe for writing via byte array. Old addressing format retained
   * for compatibility.
   *
   * Only one writing pipe can be open at once, but you can change the address
   * you'll write to. Call stopListening() first.
   *
   * Addresses are assigned via a byte array, default is 5 byte address length
   *
   * @code
   *   uint8_t addresses[][6] = {"1Node","2Node"};
   *   radio.openWritingPipe(addresses[0]);
   * @endcode
   * @code
   *  uint8_t address[] = { 0xCC,0xCE,0xCC,0xCE,0xCC };
   *  radio.openWritingPipe(address);
   *  address[0] = 0x33;
   *  radio.openReadingPipe(1,address);
   * @endcode
   * @see setAddressWidth
   *
   * @param address The address of the pipe to open. Coordinate these pipe
   * addresses amongst nodes on the network.
   */

  void RF24_openWritingPipe(const uint8_t * address);

  /**
   * Open a pipe for reading
   *
   * Up to 6 pipes can be open for reading at once.  Open all the required
   * reading pipes, and then call startListening().
   *
   * @see openWritingPipe
   * @see setAddressWidth
   *
   * @note Pipes 0 and 1 will store a full 5-byte address. Pipes 2-5 will technically 
   * only store a single byte, borrowing up to 4 additional bytes from pipe #1 per the
   * assigned address width.
   * @warning Pipes 1-5 should share the same address, except the first byte.
   * Only the first byte in the array should be unique, e.g.
   * @code
   *   uint8_t addresses[][6] = {"1Node","2Node"};
   *   openReadingPipe(1,addresses[0]);
   *   openReadingPipe(2,addresses[1]);
   * @endcode
   *
   * @warning Pipe 0 is also used by the writing pipe.  So if you open
   * pipe 0 for reading, and then startListening(), it will overwrite the
   * writing pipe.  Ergo, do an openWritingPipe() again before write().
   *
   * @param number Which pipe# to open, 0-5.
   * @param address The 24, 32 or 40 bit address of the pipe to open.
   */

  void RF24_openReadingPipe(uint8_t number, const uint8_t *address);

   /**@}*/
  /**
   * @name Advanced Operation
   *
   *  Methods you can use to drive the chip in more advanced ways
   */
  /**@{*/

  /**
   * Test whether there are bytes available to be read in the
   * FIFO buffers. 
   *
   * @param[out] pipe_num Which pipe has the payload available
   *  
   * @code
   * uint8_t pipeNum;
   * if(radio.available(&pipeNum)){
   *   radio.read(&data,sizeof(data));
   *   Serial.print("Got data on pipe");
   *   Serial.println(pipeNum);
   * }
   * @endcode
   * @return True if there is a payload available, false if none is
   */
  bool RF24_pipeAvailable(uint8_t* pipe_num);

  /**
   * Check if the radio needs to be read. Can be used to prevent data loss
   * @return True if all three 32-byte radio buffers are full
   */
  bool RF24_rxFifoFull();

  /**
   * Enter low-power mode
   *
   * To return to normal power mode, call powerUp().
   *
   * @note After calling startListening(), a basic radio will consume about 13.5mA
   * at max PA level.
   * During active transmission, the radio will consume about 11.5mA, but this will
   * be reduced to 26uA (.026mA) between sending.
   * In full powerDown mode, the radio will consume approximately 900nA (.0009mA)   
   *
   * @code
   * radio.powerDown();
   * avr_enter_sleep_mode(); // Custom function to sleep the device
   * radio.powerUp();
   * @endcode
   */
  void RF24_powerDown(void);

  /**
   * Leave low-power mode - required for normal radio operation after calling powerDown()
   * 
   * To return to low power mode, call powerDown().
   * @note This will take up to 5ms for maximum compatibility 
   */
  void RF24_powerUp(void) ;

  /**
  * Write for single NOACK writes. Optionally disables acknowledgements/autoretries for a single write.
  *
  * @note enableDynamicAck() must be called to enable this feature
  *
  * Can be used with enableAckPayload() to request a response
  * @see enableDynamicAck()
  * @see setAutoAck()
  * @see write()
  *
  * @param buf Pointer to the data to be sent
  * @param len Number of bytes to be sent
  * @param multicast Request ACK (0), NOACK (1)
  */
  bool RF24_write( const void* buf, uint8_t len, const bool multicast );

#if 0
  /**
   * This will not block until the 3 FIFO buffers are filled with data.
   * Once the FIFOs are full, writeFast will simply wait for success or
   * timeout, and return 1 or 0 respectively. From a user perspective, just
   * keep trying to send the same data. The library will keep auto retrying
   * the current payload using the built in functionality.
   * @warning It is important to never keep the nRF24L01 in TX mode and FIFO full for more than 4ms at a time. If the auto
   * retransmit is enabled, the nRF24L01 is never in TX mode long enough to disobey this rule. Allow the FIFO
   * to clear by issuing txStandBy() or ensure appropriate time between transmissions.
   *
   * @code
   * Example (Partial blocking):
   *
   *			radio.writeFast(&buf,32);  // Writes 1 payload to the buffers
   *			txStandBy();     		   // Returns 0 if failed. 1 if success. Blocks only until MAX_RT timeout or success. Data flushed on fail.
   *
   *			radio.writeFast(&buf,32);  // Writes 1 payload to the buffers
   *			txStandBy(1000);		   // Using extended timeouts, returns 1 if success. Retries failed payloads for 1 seconds before returning 0.
   * @endcode
   *
   * @see txStandBy()
   * @see write()
   * @see writeBlocking()
   *
   * @param buf Pointer to the data to be sent
   * @param len Number of bytes to be sent
   * @return True if the payload was delivered successfully false if not
   */
  bool writeFast( const void* buf, uint8_t len );

  /**
  * WriteFast for single NOACK writes. Disables acknowledgements/autoretries for a single write.
  *
  * @note enableDynamicAck() must be called to enable this feature
  * @see enableDynamicAck()
  * @see setAutoAck()
  *
  * @param buf Pointer to the data to be sent
  * @param len Number of bytes to be sent
  * @param multicast Request ACK (0) or NOACK (1)
  */
  bool writeFast( const void* buf, uint8_t len, const bool multicast );

  /**
   * This function extends the auto-retry mechanism to any specified duration.
   * It will not block until the 3 FIFO buffers are filled with data.
   * If so the library will auto retry until a new payload is written
   * or the user specified timeout period is reached.
   * @warning It is important to never keep the nRF24L01 in TX mode and FIFO full for more than 4ms at a time. If the auto
   * retransmit is enabled, the nRF24L01 is never in TX mode long enough to disobey this rule. Allow the FIFO
   * to clear by issuing txStandBy() or ensure appropriate time between transmissions.
   *
   * @code
   * Example (Full blocking):
   *
   *			radio.writeBlocking(&buf,32,1000); //Wait up to 1 second to write 1 payload to the buffers
   *			txStandBy(1000);     			   //Wait up to 1 second for the payload to send. Return 1 if ok, 0 if failed.
   *					  				   		   //Blocks only until user timeout or success. Data flushed on fail.
   * @endcode
   * @note If used from within an interrupt, the interrupt should be disabled until completion, and sei(); called to enable millis().
   * @see txStandBy()
   * @see write()
   * @see writeFast()
   *
   * @param buf Pointer to the data to be sent
   * @param len Number of bytes to be sent
   * @param timeout User defined timeout in milliseconds.
   * @return True if the payload was loaded into the buffer successfully false if not
   */
  bool writeBlocking( const void* buf, uint8_t len, uint32_t timeout );

#endif

  /**
   * This function should be called as soon as transmission is finished to
   * drop the radio back to STANDBY-I mode. If not issued, the radio will
   * remain in STANDBY-II mode which, per the data sheet, is not a recommended
   * operating mode.
   *
   * @note When transmitting data in rapid succession, it is still recommended by
   * the manufacturer to drop the radio out of TX or STANDBY-II mode if there is
   * time enough between sends for the FIFOs to empty. This is not required if auto-ack
   * is enabled.
   *
   * Relies on built-in auto retry functionality.
   *
   * @code
   * Example (Partial blocking):
   *
   *			radio.writeFast(&buf,32);
   *			radio.writeFast(&buf,32);
   *			radio.writeFast(&buf,32);  //Fills the FIFO buffers up
   *			bool ok = txStandBy();     //Returns 0 if failed. 1 if success.
   *					  				   //Blocks only until MAX_RT timeout or success. Data flushed on fail.
   * @endcode
   * @see txStandBy(unsigned long timeout)
   * @return True if transmission is successful
   *
   */
   bool RF24_txStandBy();

#if 0
  /**
   * This function allows extended blocking and auto-retries per a user defined timeout
   * @code
   *	Fully Blocking Example:
   *
   *			radio.writeFast(&buf,32);
   *			radio.writeFast(&buf,32);
   *			radio.writeFast(&buf,32);   //Fills the FIFO buffers up
   *			bool ok = txStandBy(1000);  //Returns 0 if failed after 1 second of retries. 1 if success.
   *					  				    //Blocks only until user defined timeout or success. Data flushed on fail.
   * @endcode
   * @note If used from within an interrupt, the interrupt should be disabled until completion, and sei(); called to enable millis().
   * @param timeout Number of milliseconds to retry failed payloads
   * @return True if transmission is successful
   *
   */
   bool txStandBy(uint32_t timeout, bool startTx = 0);
#endif

  /**
   * Write an ack payload for the specified pipe
   *
   * The next time a message is received on @p pipe, the data in @p buf will
   * be sent back in the acknowledgement.
   * @see enableAckPayload()
   * @see enableDynamicPayloads()
   * @warning Only three of these can be pending at any time as there are only 3 FIFO buffers.<br> Dynamic payloads must be enabled.
   * @note Ack payloads are handled automatically by the radio chip when a payload is received. Users should generally
   * write an ack payload as soon as startListening() is called, so one is available when a regular payload is received.
   * @note Ack payloads are dynamic payloads. This only works on pipes 0&1 by default. Call 
   * enableDynamicPayloads() to enable on all pipes.
   *
   * @param pipe Which pipe# (typically 1-5) will get this response.
   * @param buf Pointer to data that is sent
   * @param len Length of the data to send, up to 32 bytes max.  Not affected
   * by the static payload set by setPayloadSize().
   */
  void RF24_writeAckPayload(uint8_t pipe, const void* buf, uint8_t len);

  /**
   * Determine if an ack payload was received in the most recent call to
   * write(). The regular available() can also be used.
   *
   * Call read() to retrieve the ack payload.
   *
   * @return True if an ack payload is available.
   */
  bool RF24_isAckPayloadAvailable(void);

  /**
   * Call this when you get an interrupt to find out why
   *
   * Tells you what caused the interrupt, and clears the state of
   * interrupts.
   *
   * @param[out] tx_ok The send was successful (TX_DS)
   * @param[out] tx_fail The send failed, too many retries (MAX_RT)
   * @param[out] rx_ready There is a message waiting to be read (RX_DS)
   */
  void RF24_whatHappened(bool* tx_ok,bool* tx_fail,bool* rx_ready);
  /**
   * Non-blocking write to the open writing pipe used for buffered writes
   *
   * @note Optimization: This function now leaves the CE pin high, so the radio
   * will remain in TX or STANDBY-II Mode until a txStandBy() command is issued. Can be used as an alternative to startWrite()
   * if writing multiple payloads at once.
   * @warning It is important to never keep the nRF24L01 in TX mode with FIFO full for more than 4ms at a time. If the auto
   * retransmit/autoAck is enabled, the nRF24L01 is never in TX mode long enough to disobey this rule. Allow the FIFO
   * to clear by issuing txStandBy() or ensure appropriate time between transmissions.
   *
   * @see write()
   * @see writeFast()
   * @see startWrite()
   * @see writeBlocking()
   *
   * For single noAck writes see:
   * @see enableDynamicAck()
   * @see setAutoAck()
   *
   * @param buf Pointer to the data to be sent
   * @param len Number of bytes to be sent
   * @param multicast Request ACK (0) or NOACK (1)
   * @return True if the payload was delivered successfully false if not
   */
  //void startFastWrite( const void* buf, uint8_t len, const bool multicast, bool startTx = 1 );

  /**
   * Non-blocking write to the open writing pipe
   *
   * Just like write(), but it returns immediately. To find out what happened
   * to the send, catch the IRQ and then call whatHappened().
   *
   * @see write()
   * @see writeFast()
   * @see startFastWrite()
   * @see whatHappened()
   *
   * For single noAck writes see:
   * @see enableDynamicAck()
   * @see setAutoAck()
   *
   * @param buf Pointer to the data to be sent
   * @param len Number of bytes to be sent
   * @param multicast Request ACK (0) or NOACK (1)
   *
   */
  //void startWrite( const void* buf, uint8_t len, const bool multicast );
  
  /**
   * This function is mainly used internally to take advantage of the auto payload
   * re-use functionality of the chip, but can be beneficial to users as well.
   *
   * The function will instruct the radio to re-use the data in the FIFO buffers,
   * and instructs the radio to re-send once the timeout limit has been reached.
   * Used by writeFast and writeBlocking to initiate retries when a TX failure
   * occurs. Retries are automatically initiated except with the standard write().
   * This way, data is not flushed from the buffer until switching between modes.
   *
   * @note This is to be used AFTER auto-retry fails if wanting to resend
   * using the built-in payload reuse features.
   * After issuing reUseTX(), it will keep reending the same payload forever or until
   * a payload is written to the FIFO, or a flush_tx command is given.
   */
   void RF24_reUseTX();

  /**
   * Empty the transmit buffer. This is generally not required in standard operation.
   * May be required in specific cases after stopListening() , if operating at 250KBPS data rate.
   *
   * @return Current value of status register
   */
  //uint8_t flush_tx(void);

  /**
   * Test whether there was a carrier on the line for the
   * previous listening period.
   *
   * Useful to check for interference on the current channel.
   *
   * @return true if was carrier, false if not
   */
  bool RF24_testCarrier(void);

  /**
   * Test whether a signal (carrier or otherwise) greater than
   * or equal to -64dBm is present on the channel. Valid only
   * on nRF24L01P (+) hardware. On nRF24L01, use testCarrier().
   *
   * Useful to check for interference on the current channel and
   * channel hopping strategies.
   *
   * @code
   * bool goodSignal = radio.testRPD();
   * if(radio.available()){
   *    Serial.println(goodSignal ? "Strong signal > 64dBm" : "Weak signal < 64dBm" );
   *    radio.read(0,0);
   * }
   * @endcode
   * @return true if signal => -64dBm, false if not
   */
  bool RF24_testRPD(void) ;

  /**
   * Test whether this is a real radio, or a mock shim for
   * debugging.  Setting either pin to 0xff is the way to
   * indicate that this is not a real radio.
   *
   * @return true if this is a legitimate radio
   */
  //bool isValid() { return ce_pin != 0xff && csn_pin != 0xff; }
  
   /**
   * Close a pipe after it has been previously opened.
   * Can be safely called without having previously opened a pipe.
   * @param pipe Which pipe # to close, 0-5.
   */
  void RF24_closeReadingPipe( uint8_t pipe ) ;

   /**
   * 
   * If a failure has been detected, it usually indicates a hardware issue. By default the library
   * will cease operation when a failure is detected.  
   * This should allow advanced users to detect and resolve intermittent hardware issues.  
   *   
   * In most cases, the radio must be re-enabled via radio.begin(); and the appropriate settings
   * applied after a failure occurs, if wanting to re-enable the device immediately.
   *
   * The three main failure modes of the radio include:
   *
   * Writing to radio: Radio unresponsive - Fixed internally by adding a timeout to the internal write functions in RF24 (failure handling)
   *
   * Reading from radio: Available returns true always - Fixed by adding a timeout to available functions by the user. This is implemented internally in  RF24Network.
   *
   * Radio configuration settings are lost - Fixed by monitoring a value that is different from the default, and re-configuring the radio if this setting reverts to the default.
   * 
   * See the included example, GettingStarted_HandlingFailures
   *    
   *  @code
   *  if(radio.failureDetected){ 
   *    radio.begin();                       // Attempt to re-configure the radio with defaults
   *    radio.failureDetected = 0;           // Reset the detection value
   *	radio.openWritingPipe(addresses[1]); // Re-configure pipe addresses
   *    radio.openReadingPipe(1,addresses[0]);
   *    report_failure();                    // Blink leds, send a message, etc. to indicate failure
   *  }
   * @endcode
  */
  //#if defined (FAILURE_HANDLING)
    //bool failureDetected; 
  //#endif
    
  /**@}*/

  /**@}*/
  /**
   * @name Optional Configurators
   *
   *  Methods you can use to get or set the configuration of the chip.
   *  None are required.  Calling begin() sets up a reasonable set of
   *  defaults.
   */
  /**@{*/

  /**
  * Set the address width from 3 to 5 bytes (24, 32 or 40 bit)
  *
  * @param a_width The address width to use: 3,4 or 5
  */

  void RF24_setAddressWidth(uint8_t a_width);
  
  /**
   * Set the number and delay of retries upon failed submit
   *
   * @param delay How long to wait between each retry, in multiples of 250us,
   * max is 15.  0 means 250us, 15 means 4000us.
   * @param count How many retries before giving up, max 15
   */
  void RF24_setRetries(uint8_t delay, uint8_t count);

  /**
   * Set RF communication channel
   *
   * @param channel Which RF channel to communicate on, 0-125
   */
  void RF24_setChannel(uint8_t channel);
  
    /**
   * Get RF communication channel
   *
   * @return The currently configured RF Channel
   */
  uint8_t RF24_getChannel(void);

  /**
   * Set Static Payload Size
   *
   * This implementation uses a pre-stablished fixed payload size for all
   * transmissions.  If this method is never called, the driver will always
   * transmit the maximum payload size (32 bytes), no matter how much
   * was sent to write().
   *
   * @todo Implement variable-sized payloads feature
   *
   * @param size The number of bytes in the payload
   */
  void RF24_setPayloadSize(uint8_t size);

  /**
   * Get Static Payload Size
   *
   * @see setPayloadSize()
   *
   * @return The number of bytes in the payload
   */
  uint8_t RF24_getPayloadSize(void);

  /**
   * Get Dynamic Payload Size
   *
   * For dynamic payloads, this pulls the size of the payload off
   * the chip
   *
   * @note Corrupt packets are now detected and flushed per the
   * manufacturer.
   * @code
   * if(radio.available()){
   *   if(radio.getDynamicPayloadSize() < 1){
   *     // Corrupt payload has been flushed
   *     return; 
   *   }
   *   radio.read(&data,sizeof(data));
   * }
   * @endcode
   *
   * @return Payload length of last-received dynamic payload
   */
  uint8_t RF24_getDynamicPayloadSize(void);

  /**
   * Enable custom payloads on the acknowledge packets
   *
   * Ack payloads are a handy way to return data back to senders without
   * manually changing the radio modes on both units.
   *
   * @note Ack payloads are dynamic payloads. This only works on pipes 0&1 by default. Call 
   * enableDynamicPayloads() to enable on all pipes.
   */
  void RF24_enableAckPayload(void);

  /**
   * Enable dynamically-sized payloads
   *
   * This way you don't always have to send large packets just to send them
   * once in a while.  This enables dynamic payloads on ALL pipes.
   *
   */
  void RF24_enableDynamicPayloads(void);
  
  /**
   * Disable dynamically-sized payloads
   *
   * This disables dynamic payloads on ALL pipes. Since Ack Payloads
   * requires Dynamic Payloads, Ack Payloads are also disabled.
   * If dynamic payloads are later re-enabled and ack payloads are desired
   * then enableAckPayload() must be called again as well.
   *
   */
  void RF24_disableDynamicPayloads(void);
  
  /**
   * Enable dynamic ACKs (single write multicast or unicast) for chosen messages
   *
   * @note To enable full multicast or per-pipe multicast, use setAutoAck()
   *
   * @warning This MUST be called prior to attempting single write NOACK calls
   * @code
   * radio.enableDynamicAck();
   * radio.write(&data,32,1);  // Sends a payload with no acknowledgement requested
   * radio.write(&data,32,0);  // Sends a payload using auto-retry/autoACK
   * @endcode
   */
  void RF24_enableDynamicAck();
  
  /**
   * Determine whether the hardware is an nRF24L01+ or not.
   *
   * @return true if the hardware is nRF24L01+ (or compatible) and false
   * if its not.
   */
  bool RF24_isPVariant(void) ;

  /**
   * Enable or disable auto-acknowlede packets
   *
   * This is enabled by default, so it's only needed if you want to turn
   * it off for some reason.
   *
   * @param enable Whether to enable (true) or disable (false) auto-acks
   */
  void RF24_setAutoAck(bool enable);

  /**
   * Enable or disable auto-acknowlede packets on a per pipeline basis.
   *
   * AA is enabled by default, so it's only needed if you want to turn
   * it off/on for some reason on a per pipeline basis.
   *
   * @param pipe Which pipeline to modify
   * @param enable Whether to enable (true) or disable (false) auto-acks
   */
  void RF24_setAutoAckPipe( uint8_t pipe, bool enable ) ;

  /**
   * Set Power Amplifier (PA) level to one of four levels:
   * RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
   *
   * The power levels correspond to the following output levels respectively:
   * NRF24L01: -18dBm, -12dBm,-6dBM, and 0dBm
   *
   * SI24R1: -6dBm, 0dBm, 3dBM, and 7dBm.
   *
   * @param level Desired PA level.
   */
  void RF24_setPALevel ( uint8_t level );

  /**
   * Fetches the current PA level.
   *
   * NRF24L01: -18dBm, -12dBm, -6dBm and 0dBm
   * SI24R1:   -6dBm, 0dBm, 3dBm, 7dBm
   *
   * @return Returns values 0 to 3 representing the PA Level.
   */
   uint8_t RF24_getPALevel( void );

  /**
   * Set the transmission data rate
   *
   * @warning setting RF24_250KBPS will fail for non-plus units
   *
   * @param speed RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
   * @return true if the change was successful
   */
  bool RF24_setDataRate(rf24_datarate_e speed);

  /**
   * Fetches the transmission data rate
   *
   * @return Returns the hardware's currently configured datarate. The value
   * is one of 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS, as defined in the
   * rf24_datarate_e enum.
   */
  rf24_datarate_e RF24_getDataRate( void ) ;

  /**
   * Set the CRC length
   * <br>CRC checking cannot be disabled if auto-ack is enabled
   * @param length RF24_CRC_8 for 8-bit or RF24_CRC_16 for 16-bit
   */
  void RF24_setCRCLength(rf24_crclength_e length);

  /**
   * Get the CRC length
   * <br>CRC checking cannot be disabled if auto-ack is enabled
   * @return RF24_CRC_DISABLED if disabled or RF24_CRC_8 for 8-bit or RF24_CRC_16 for 16-bit
   */
  rf24_crclength_e RF24_getCRCLength(void);

  /**
   * Disable CRC validation
   * 
   * @warning CRC cannot be disabled if auto-ack/ESB is enabled.
   */
  void RF24_disableCRC( void ) ;

  /**
  * The radio will generate interrupt signals when a transmission is complete,
  * a transmission fails, or a payload is received. This allows users to mask
  * those interrupts to prevent them from generating a signal on the interrupt
  * pin. Interrupts are enabled on the radio chip by default.
  *
  * @code
  * 	Mask all interrupts except the receive interrupt:
  *
  *		radio.maskIRQ(1,1,0);
  * @endcode
  *
  * @param tx_ok  Mask transmission complete interrupts
  * @param tx_fail  Mask transmit failure interrupts
  * @param rx_ready Mask payload received interrupts
  */
  void RF24_maskIRQ(bool tx_ok,bool tx_fail,bool rx_ready);
  
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
  
  extern uint32_t RF24_txDelay;

  /**
  * 
  * On all devices but Linux and ATTiny, a small delay is added to the CSN toggling function
  * 
  * This is intended to minimise the speed of SPI polling due to radio commands
  *
  * If using interrupts or timed requests, this can be set to 0 Default:5
  */
  
  extern uint32_t RF24_csDelay;
  
  void RF24_toggle_features(void);
  
  uint8_t RF24_read_registers(uint8_t reg, uint8_t* buf, uint8_t len);
  uint8_t RF24_read_register(uint8_t reg);
  void RF24_read_pipes_addresses(uint8_t *buf); // buf length must be at least 5+5+4=14  
  uint8_t RF24_flush_rx(void);
  uint8_t RF24_flush_tx(void);
  void RF24_reset(void); // set every register to its reset value



  /**@}*/


#endif // __RF24_H__
