/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/sync.h"

#define _FRAISE_INTERNAL_
#define _FRAISE_PRIVATE_DEFINE_VARS
#include "fraise.pio.h"
#include "fraise_master.h"
#include "fraise_master_private.h"
#include "fraise_master_buffers.h"

#include "boardconfig.h"

static bool is_bootloading = false;

static uint8_t polled_fruit = 1;
static uint8_t destination_fruit = 0;

typedef enum {
    FMS_POLL,
    FMS_RECEIVE,
    FMS_SEND,
    FMS_WAITACK,
} fraise_master_state_t;

static fraise_master_state_t state = FMS_POLL;

int64_t fraise_alarm_callback(alarm_id_t id, void *user_data) {
    switch(state) {
        case FMS_RECEIVE:              // the fruit didn't reply to the poll
            set_detected(polled_fruit, false);
            break;
        case FMS_WAITACK:           // the fruit didn't aknowledge the packet
            break;
    }
    state = FMS_POLL;
    fraise_program_enable_tx_interrupt();
    return 0;
}

int fraise_master_ferr_count;

static alarm_id_t alarm_id;
#define PIO_TXFULL_RETRY_TIME 1*1000
#define NONE_POLLED_TIME 100*1000
#define WAIT_ANSWER_TIME 1*1000
#define ONE_BYTE_TIME 44
#define fraise_add_alarm(t) do{ cancel_alarm(alarm_id); alarm_id = add_alarm_in_us(t, fraise_alarm_callback, NULL, true);} while(0)
#define fraise_cancel_alarm() cancel_alarm(alarm_id)

#ifndef nop
#define nop() __asm volatile ("nop")
#endif

// IRQ called when the pio rx rx_fifo is not empty, or if the tx rx_fifo is not full.
static void fraise_master_irq_handler(void) {
    static uint8_t rx_checksum;
    static uint8_t rx_bytes;
    static uint8_t rx_msg_length;
    static uint8_t tx_bytes_to_send;
    static bool    rx_is_broadcast;
    static bool    rx_is_char;
    switch(state) {
        case FMS_POLL:
            fraise_cancel_alarm();
            if(pio_sm_is_tx_fifo_full(pio, sm)) {               // this shouldn't happend: the irq was triggered by tx_fifo_not_full
                fraise_program_disable_tx_interrupt();
                fraise_add_alarm(PIO_TXFULL_RETRY_TIME);
                return;
            }
            tx_bytes_to_send = txbuf_read_init();               // if message available on tx_buffer, send it:
            if(tx_bytes_to_send) {
                state = FMS_SEND;
                fraise_program_start_tx_with_interrupt(tx_bytes_to_send - 1);
                txbuf_read_getc();          					// discard the first byte in the buffer (the length of the data).
                destination_fruit = txbuf_read_getc();          // the second byte in the buffer is the ID of the destination fruit.
                fraise_program_putc_blocking(destination_fruit | 256);   // set bit9
                tx_bytes_to_send -= 2;
                break;
            }

            // if rxbuf isn't full, poll next fruit if any
            if(!rxbuf_write_init()) {                           // if rxbuf is full, don't poll and retry later.
                fraise_program_disable_tx_interrupt();
                fraise_add_alarm(PIO_TXFULL_RETRY_TIME);
                return;
            }

            if(++polled_fruit >= MAX_FRUITS) polled_fruit = 1;
            int count = 0;
            while(!is_polled(polled_fruit)) {
                if(++polled_fruit >= MAX_FRUITS) polled_fruit = 1;
                if(count++ >= MAX_FRUITS) {                // if no fruit to poll, abort and retry later
                    fraise_program_disable_tx_interrupt();
                    fraise_add_alarm(NONE_POLLED_TIME);
                    return;
                }
            }
            fraise_program_disable_tx_interrupt();
            fraise_program_start_tx(2);
            fraise_program_putc_blocking(polled_fruit | 128 | 256); // poll signal (bit9 and bit8 on)
            fraise_program_putc_blocking(polled_fruit | 128);       // poll confirm (bit9 off, bit8 on)
            state = FMS_RECEIVE;
            rx_bytes = 0;
            rxbuf_write_putc(polled_fruit);                         // Start buffer with the polled fruit ID.
            fraise_add_alarm(WAIT_ANSWER_TIME);                      // Arm timeout alarm
            return;
        case FMS_WAITACK:
            if(!pio_sm_is_rx_fifo_empty(pio, sm)) {
                fraise_cancel_alarm();
                uint16_t c = fraise_program_getc();
                if(pio_interrupt_get(pio, 4) || (c > 255)) {        // Framing error or bit9=1: discard.
                	pio_interrupt_clear(pio, 4);
                	fraise_master_ferr_count++;
                }
                else if(c == 0) txbuf_read_finish(); // the packet has been acknowledged: remove it from the buffer.
                else {
                    // TODO: signal checksum error
                }
                state = FMS_POLL;
                fraise_program_enable_tx_interrupt();
            }
            return;
        case FMS_RECEIVE:
            while(!pio_sm_is_rx_fifo_empty(pio, sm)) {
                fraise_cancel_alarm();
                uint16_t c = fraise_program_getc();
                if(pio_interrupt_get(pio, 4) || (c > 255)) {        // Framing error or bit9=1: discard.
                    pio_interrupt_clear(pio, 4);
                    pio_sm_clear_fifos(pio, sm);
                    state = FMS_POLL;
                    fraise_program_enable_tx_interrupt();
                    fraise_master_ferr_count++;
                    return;
                }
                rx_bytes++;
                if(rx_bytes == 1) {                       // First byte = packet length
                    if(c == 0) {                            // the fruit has nothing to send
                        state = FMS_POLL;
                        fraise_program_enable_tx_interrupt();
                        if(is_polled(polled_fruit)) set_detected(polled_fruit, true);
                        return;
                    }
                    rx_is_char = (c >= 128);
                    rx_msg_length = c & 127;
                    rx_checksum = c;
                } else {
                    rx_checksum += c;
                    rxbuf_write_putc(c);
                    if(rx_bytes == rx_msg_length + 2) {     // "+ 2" is for accounting the 'length' and the 'checksum' bytes
                        if(rx_checksum == 0) {
                            rxbuf_write_finish(rx_is_char);
                            if(is_polled(polled_fruit)) set_detected(polled_fruit, true);
                        }
                        fraise_program_start_tx(1);
                        fraise_program_putc_blocking(rx_checksum != 0); // Acknowledge the packet, or not...
                        state = FMS_POLL;
                        fraise_add_alarm(ONE_BYTE_TIME);     // Wait for the ACK byte to be sent.
                        return;
                   }
                }
                fraise_add_alarm(WAIT_ANSWER_TIME);
            }
            return;
    }

    while((state == FMS_SEND) && !pio_sm_is_tx_fifo_full(pio, sm)) {
        fraise_cancel_alarm();
        fraise_program_putc(txbuf_read_getc());
        tx_bytes_to_send--;
        if(tx_bytes_to_send == 0) {                         // The message has been fully pushed to TX FIFO:
            fraise_program_disable_tx_interrupt();          // Stop triggering interrupt when tx rx_fifo not full;
            if(destination_fruit != 0) {                    // If the massage isn't broadcast:
		        fraise_add_alarm(WAIT_ANSWER_TIME);             // Arm the timeout alarm
		        state = FMS_WAITACK;                            // Set the state to wait for the Ack.
	        } else {                                        // If the message is broadcast, remove it from the buffer.
	            txbuf_read_finish();                            // remove it from the buffer,
	            state = FMS_POLL;                               // and restart the state machine 
	            fraise_add_alarm(ONE_BYTE_TIME * pio_sm_get_tx_fifo_level (pio, sm)); // after all bytes are sent.
            }
        }
    }
}

// Find a free pio and state machine and load the program into it.
// Returns false if this fails.
static bool init_pio(const pio_program_t *program, PIO *pio_hw, uint *sm, uint *offset) {
    // Find a free pio
    *pio_hw = pio1;
    if (!pio_can_add_program(*pio_hw, program)) {
        *pio_hw = pio0;
        if (!pio_can_add_program(*pio_hw, program)) {
            *offset = -1;
            return false;
        }
    }
    *offset = pio_add_program(*pio_hw, program);
    // Find a state machine
    *sm = (int8_t)pio_claim_unused_sm(*pio_hw, false);
    if (*sm < 0) {
        return false;
    }
    return true;
}

void fraise_setup() {
    // Get the pin numbers
    int rxpin, txpin, drvpin;
    fraise_get_pins(&rxpin, &txpin, &drvpin);

    // Reset the buffers
    fraise_master_buffers_reset();

    // Set up the state machine we're going to use
    if (!init_pio(&fraise_program, &pio, &sm, &pgm_offset)) {
        panic("failed to setup pio");
    }
    fraise_program_init(rxpin, txpin, drvpin);

    // Find a free irq
    static_assert(PIO0_IRQ_1 == PIO0_IRQ_0 + 1 && PIO1_IRQ_1 == PIO1_IRQ_0 + 1, "");
    pio_irq = (pio == pio0) ? PIO0_IRQ_0 : PIO1_IRQ_0;
    if (irq_get_exclusive_handler(pio_irq)) {
        pio_irq++;
        if (irq_get_exclusive_handler(pio_irq)) {
            panic("All IRQs are in use");
        }
    }

    // Enable interrupt
    irq_add_shared_handler(pio_irq, fraise_master_irq_handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY); // Add a shared IRQ handler
    irq_set_enabled(pio_irq, true); // Enable the IRQ
    irq_index = pio_irq - ((pio == pio0) ? PIO0_IRQ_0 : PIO1_IRQ_0); // Get index of the IRQ
    
    fraise_program_enable_rx_interrupt(); // Set pio to tell us when the FIFO is NOT empty

    fraise_program_enable_tx_interrupt(); // This should instantly run the IRQ handler.
}

void fraise_get_pins(int *rxpin, int *txpin, int *drvpin)
{
	*rxpin = FRAISE_RX_PIN;
	*txpin = FRAISE_TX_PIN;
	*drvpin = FRAISE_DRV_PIN;
}

void fraise_unsetup() {
    // Disable interrupt
    fraise_program_disable_tx_interrupt();
    fraise_program_disable_rx_interrupt();
    fraise_cancel_alarm();
    irq_set_enabled(pio_irq, false);
    irq_remove_handler(pio_irq, fraise_master_irq_handler);

    // Cleanup pio
    pio_sm_set_enabled(pio, sm, false);
    pio_remove_program(pio, &fraise_program, pgm_offset);
    pio_sm_unclaim(pio, sm);
}

void fraise_master_reset(){
	//fraise_master_reset_polls();
	fraise_program_disable_tx_interrupt();
	fraise_cancel_alarm();
	uint32_t status = save_and_disable_interrupts();
	pio_sm_clear_fifos(pio, sm);
	pio_sm_exec_wait_blocking (pio, sm, jmp_rx);
	fraise_master_buffers_reset();
	state = FMS_POLL;
	restore_interrupts(status);
	fraise_program_enable_tx_interrupt();
}

// -----------------------------

void fraise_master_assign(const char* fruitname, uint8_t id){
	// TODO send 'assign' command
	/*uint32_t status = save_and_disable_interrupts();
	sleep_ms(1);
	fraise_program_start_tx(strlen(fruitname) + 4);*/
	if(strlen(fruitname) > 16) {
	    printf("e fruit name too long! (16 chars max)");
	    return;
    }
	char buffer[32];
	sprintf(buffer, "N%02X%s", id, fruitname);
	fraise_master_sendchars_broadcast(buffer);
}

// -----------------------------

void fraise_master_start_bootload(const char *fruitname){
	// TODO send 'enter bootloader' command
	if(strlen(fruitname) > 16) {
	    printf("e fruit name too long! (16 chars max)");
	    return;
    }
	is_bootloading = true;
	char buffer[32];
	sprintf(buffer, "F%s", fruitname);
	fraise_master_sendchars_broadcast(buffer);
	sleep_ms(1);

	fraise_program_disable_rx_interrupt();
	fraise_program_disable_tx_interrupt();
	fraise_cancel_alarm();
}

void fraise_master_stop_bootload(){
	// TODO send 'quit bootloader' command
	is_bootloading = false;
	state = FMS_POLL;
	fraise_program_enable_rx_interrupt();
	fraise_program_enable_tx_interrupt();
}

bool fraise_master_is_bootloading() {
	return is_bootloading;
}

void fraise_master_send_bootload(const char *buf){
	// TODO bootloader communication
	if(!is_bootloading) return;
	int len = strlen(buf);
	uint8_t checksum = len + 1;
	const char *p = buf;
	fraise_program_start_tx(len + 2);
	fraise_program_putc_blocking(len + 1);
	while(*p) {
	    checksum += *p;
	    fraise_program_putc_blocking(*p);
	    //printf("l bld send: %c\n", *p);
	    p++;
    }
    fraise_program_putc_blocking(256 - checksum);
    //while(pio_sm_get_tx_fifo_level(pio, sm) != 0);
}

// -----------------------------

void fraise_master_set_poll(uint8_t id, bool poll){
	if(id == 0) {
		if(poll) printf("sC00\n");
		else printf("sc00\n");
	} else if(id < MAX_FRUITS) {
		uint32_t status = save_and_disable_interrupts();
		set_polled(id, poll);
		set_detected(id, false);
		restore_interrupts(status);
		printf("l polled(%d):%d\n", id, is_polled(id));
	}
}

void fraise_master_reset_polls() {
	uint32_t status = save_and_disable_interrupts();
	for(int id = 0; id < MAX_FRUITS ; id++) {
		set_polled(id, false);
		set_detected(id, false);
	}
	restore_interrupts(status);
}

// -----------------------------

void fraise_master_sendbytes_raw(uint8_t id, const char *data, uint8_t len, bool isChar) {
	if(id >= MAX_FRUITS) return;
	if(!txbuf_write_init()) {
		printf("e TXBUF full!\n");
		return;
	}
	txbuf_write_putc(id);
	txbuf_write_putc(len | (128 * isChar));
	for(int i = 0; i < len; i++) txbuf_write_putc(data[i]);
	txbuf_write_finish();
}

void fraise_master_sendbytes(uint8_t id, const char *data, uint8_t len) {
	if(id == 0) {
		fraise_receivebytes(data, len);
		return;
	}
	fraise_master_sendbytes_raw(id, data, len, false);
}

void fraise_master_sendchars(uint8_t id, const char *data){
	int len = strlen(data);
	if(id == 0) {
		fraise_receivechars(data, len);
		return;
	}
	fraise_master_sendbytes_raw(id, data, len, true);
}

void fraise_master_sendbytes_broadcast(const char *data, uint8_t len) {
	fraise_receivebytes_broadcast(data, len);
	fraise_master_sendbytes_raw(0, data, len, false);
}

void fraise_master_sendchars_broadcast(const char *data) {
	int len = strlen(data);
	fraise_receivechars_broadcast(data, len);
	fraise_master_sendbytes_raw(0, data, len, true);
}

// -----------------------------

void fraise_master_service() {
	int l;
	bool isChar;
	uint8_t id;
	// Process incoming message
	while((l = rxbuf_read_init())) {
		isChar = l > 127;
		l = (l & 63) - 2;
		id = rxbuf_read_getc();
		printf("%02X", id | (isChar?128:0));
		if(isChar) while(l--) putchar(rxbuf_read_getc());
		else while(l--) printf("%02X", rxbuf_read_getc());
		putchar('\n');
		rxbuf_read_finish();
	}
	// TODO: pace 'detected changes' updates
	for(int i = 1; i < MAX_FRUITS; i++) {
		if(is_detected_sent(i) != is_detected(i)) {
			bool d = is_detected(i);
			if(d) {
				set_detected_sent(i, true);
				printf("sC%02X\n", i);
			}
			else {
				set_detected_sent(i, false);
				printf("sc%02X\n", i);
			}
		}
	}
	if(is_bootloading) {
	    while(!pio_sm_is_rx_fifo_empty(pio, sm)) {
	        printf("b%c\n", fraise_program_getc());
	    }
	}
}

void fraise_master_print_stats() {
	printf("l state:%d ferr_count:%d\n", state, fraise_master_ferr_count);
	for(int i = 1; i < 3; i++) {
		printf("l fruit_%d polled=%d detected=%d detected_sent=%d\n",
			i, is_polled(i), is_detected(i), is_detected_sent(i));
	}
}

// ------------------------------
// virtual fruit fraise_put* emulation

// Send a text message (must be a null-terminated string)
bool fraise_puts(const char* msg) {
	printf("80%s\n", msg);
	return true;
}

// Send a raw bytes message
bool fraise_putbytes(const char* data, uint8_t len) {
	printf("00");
	for(int i = 0; i < len ; i++) printf("%02X", data[i]);
	putchar('\n');
}

// ------------------------------
// virtual fruit stdout emulation

void fraise_putchar(char c) {
	static char line[64];
	static int count = 0;
	if(c == '\n') {
		line[count] = 0;
		printf("80%s\n", line);
		count = 0;
		return;
	}
	if(count < 64) line[count++] = c;
}

void fraise_printf(char* fmt, ...) {
	va_list args;
	char buf[64];
	char *p = buf;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	while(*p) fraise_putchar(*p++);
}


