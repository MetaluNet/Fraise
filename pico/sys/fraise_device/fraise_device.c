/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/async_context_threadsafe_background.h"
#include "pico/async_context_poll.h"
#include "hardware/pio.h"
#include "pico/stdio/driver.h"
#include "RP2040.h"
#include "hardware/resets.h"
#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"

#include "fraise.pio.h"
#include "fraise_device.h"
#include "fraise_buffers.h"
#include "fraise_eeprom.h"

// This program
// - Uses a PIO state machine to receive 9-bit data (message starts with an address having bit 9 high)
// - Uses an interrupt to determine when the PIO FIFO has some data
// - Discards the message if the address doesn't match, or on framing error
// - Saves the message to the Fraise RX buffer (rxbuf, see fraise_buffers.c)
// - Uses an async context to pop the message from rxbuf, and dispatch it to the 4 possible fraise_receive* callbacks,
//   when notified by the irq or manually polled by the application calling fraise_poll().

// If the received message was a "poll" message (address with a 128 offset), the next message to send (if any)
// is fetched from the Fraise TX buffer (txbuf), and transmitted to the state machine which is switched to transmit mode.

static bool background_rx = false;
static PIO pio;
static uint sm;
static uint pgm_offset; // Offset of the program in the pio
static int8_t pio_irq;  // The irq used by the sm
static uint irq_index;  // The offset of pio_irq relatively to PIO0_IRQ_0 or PIO1_IRQ_0

static uint irq_count, irq_rx_count; // Debugging counters

static void async_worker_func(async_context_t *async_context, async_when_pending_worker_t *worker);

// Two async contexts (only one will be used) that will dispatch the incoming messages
// to the 4 fraise_receive* callbacks;
// one is notified by the irq, this other one must be manually polled by the application.
static async_context_threadsafe_background_t background_context;
static async_context_poll_t poll_context;
static async_context_t *context_core;
// The worker that will be assigned to the async context:
static async_when_pending_worker_t worker = { .do_work = async_worker_func };

// Function that will be called by stdout.
void fraise_out_chars(const char *buf, int len);
// Data structure for registering this function with the stdout plumbing.
stdio_driver_t stdio_fraise = {
    .out_chars = fraise_out_chars,
#ifdef PICO_STDIO_ENABLE_CRLF_SUPPORT
    .crlf_enabled = PICO_STDIO_DEFAULT_CRLF
#endif
};

static uint8_t FraiseID = 10; // default Fraise ID is 10

typedef enum {
    FS_LISTEN,
    FS_POLL,
    FS_RECEIVE,
    FS_SEND,
    FS_WAITACK,
} fraise_state_t;

static fraise_state_t state = FS_LISTEN;

// IRQ called when the pio rx rx_fifo is not empty, or if the tx rx_fifo is not full.
static void fraise_irq(void) {
    static uint8_t rx_checksum;
    static uint8_t rx_bytes;
    static uint8_t rx_msg_length;
    static uint8_t tx_bytes_to_send;
    static bool    rx_is_broadcast;
    static bool    rx_is_char;

    irq_count++;

    while(!pio_sm_is_rx_fifo_empty(pio, sm)) {
        irq_rx_count++;
        uint16_t c = fraise_program_getc(pio, sm);
        if(pio_interrupt_get(pio, 4)) {                     // Framing error! Discard.
            fraise_program_discard_rx(pio, sm);
            state = FS_LISTEN;
            continue;
        }
        if(c > 255) state = FS_LISTEN;                      // bit 9 signals the start of a new message
        switch(state) {
            case FS_LISTEN:
                if((c == FraiseID + 0x100)                  // If we are the destination of the message,
                    || (c == 0x100)) {                      // or if the message is "broadcast" (destinated to everyone),
                    if(rxbuf_write_init()) {
                        state = FS_RECEIVE;                 // start receiving the message.
                        rx_bytes = 1;
                        rx_checksum = c;
                        rx_is_broadcast = (c == 0x100);
                    }
                } else if(c == FraiseID + 0x180) {          // The message is a "poll" signal.
                    state = FS_POLL;
                } else fraise_program_discard_rx(pio, sm);  // We're not interested in this message.
                break;
            case FS_POLL:
                if(c == FraiseID + 0x80) {                  // "poll" signal is confirmed.
                    tx_bytes_to_send = txbuf_read_init();   // Do we have a message to send?
                    if(tx_bytes_to_send == 0) {             // No: send 0.
                        fraise_program_start_tx(pio, sm, 1);
                        fraise_program_putc(pio, sm, 0);
                        state = FS_LISTEN;
                    } else {                                // Yes: start transmitting, enabling tx_fifo_not_full interrupt.
                        state = FS_SEND;
                        fraise_program_start_tx_with_interrupt(pio, sm, irq_index, tx_bytes_to_send);
                    }
                } else state = FS_LISTEN;
                break;
            case FS_RECEIVE:
                rx_checksum += c;
                rx_bytes++;
                if(rx_bytes == 2) {                         // The second byte is the length byte.
                    rx_msg_length = c & 63;
                    rx_is_char = (c & 128) != 0;
                }
                else if(rx_bytes == rx_msg_length + 3) {    // We have received the whole message.
                    state = FS_LISTEN;
                    if(rx_checksum == 0) {                  // If the checksum is good:
                        rxbuf_write_finish(rx_is_char, rx_is_broadcast);        // - validate the message,
                        async_context_set_work_pending(context_core, &worker);  // - notify the async worker,
                        if(!rx_is_broadcast) {                                  // - and if it isn't a broadcast message,
                            fraise_program_start_tx(pio, sm, 1);                //   send 0 (ack).
                            fraise_program_putc(pio, sm, 0);
                        }
                    } else {
                        if(!rx_is_broadcast) {                                  // If the checksum is wrong and if
                            fraise_program_start_tx(pio, sm, 1);                // the message isn't broadcasted,
                            fraise_program_putc(pio, sm, 1);                    // send 1 (nack).
                        }
                    }
                    break;
                }
                if(rx_bytes != 2) rxbuf_write_putc(c);      // Don't push the length_byte (it's already processed by rxbuf).
                break;
            case FS_WAITACK:
                if(c == 0) txbuf_read_finish();             // Remove the current message from the tx buffer if it has been acknowledged.
                state = FS_LISTEN;
                break;
            default:
                state = FS_LISTEN;
        }
    }

    while((state == FS_SEND) && !pio_sm_is_tx_fifo_full(pio, sm)) {
        fraise_program_putc(pio, sm, txbuf_read_getc());
        tx_bytes_to_send--;
        if(tx_bytes_to_send == 0) {                         // The message has been fully pushed to TX FIFO:
            fraise_program_disable_tx_interrupt(pio, sm, irq_index); // Stop triggering interrupt when tx rx_fifo not full
#define PIED_IS_BUGGED // pied doesn't reply ack...
#ifdef PIED_IS_BUGGED
            txbuf_read_finish();                            // Remove the current message from the tx buffer.
            state = FS_LISTEN;
#else
            state = FS_WAITACK;
#endif
        }
    }
}

static void disable_interrupts(void)
{
	SysTick->CTRL &= ~1;

	NVIC->ICER[0] = 0xFFFFFFFF;
	NVIC->ICPR[0] = 0xFFFFFFFF;
}

static void reset_peripherals(void)
{
    reset_block(~(
            RESETS_RESET_IO_QSPI_BITS |
            RESETS_RESET_PADS_QSPI_BITS |
            RESETS_RESET_SYSCFG_BITS |
            RESETS_RESET_PLL_SYS_BITS
    ));
}

void reboot() {
	hw_clear_bits(&watchdog_hw->ctrl, WATCHDOG_CTRL_ENABLE_BITS);
	watchdog_reboot(0, 0, 0);

	while (1) {
		tight_loop_contents();
	}
}

extern int __fraise_bootloader_start__;

void switch_to_bootloader()
{
    fraise_unsetup();
    disable_interrupts();
    reset_peripherals();
    const uint32_t vtor = (uint32_t)&__fraise_bootloader_start__ /*XIP_BASE + (64 * 1024)*/;
    // Derived from the Leaf Labs Cortex-M3 bootloader.
    // Copyright (c) 2010 LeafLabs LLC.
    // Modified 2021 Brian Starkey <stark3y@gmail.com>
    // Originally under The MIT License
    uint32_t reset_vector = *(volatile uint32_t *)(vtor + 0x04);

    SCB->VTOR = (volatile uint32_t)(vtor);

    asm volatile("msr msp, %0"::"g" (*(volatile uint32_t *)vtor));
    asm volatile("bx %0"::"r" (reset_vector));

    while(1);
}

void switch_to_bootloader_if_name_matches(char *data, uint8_t len)
{
    if(strncmp(data, eeprom_get_name(), len)) return; // continue only if the name matches

    //switch_to_bootloader();
    reboot();
}

void assign(char *data, uint8_t len)
{
    unsigned char c, c2, tmpid;
    c = data[0];
    c2 = data[1];
    c -= '0';
    if (c > 9) c -= 'A' - '9' - 1;
    c2 -= '0';
    if (c2 > 9) c2 -= 'A' - '9' - 1;
    if((c > 7) || (c2 > 15)) { // bad id... return
        return;
    }
    tmpid = c2 + (c << 4);

    if(strncmp(data + 2, eeprom_get_name(), len - 2)) return; // continue only if the name matches.
    if(FraiseID == tmpid) return; // no need to rewrite the eeprom if the id is the same.

    fraise_setID(tmpid);
    eeprom_set_id(tmpid);
    fraise_unsetup();
    eeprom_commit();
    fraise_setup();
}

// Process incoming messages from Fraise RX buffer, dispatch them to the 4 Fraise receive callbacks.
static void async_worker_func(async_context_t *async_context, async_when_pending_worker_t *worker) {
    static bool message_is_chars;
    static bool message_is_broadcast;
    static char message[64];
    static int message_length;
    static uint8_t message_count;

    while((message_length = rxbuf_read_init()) != 0) {      // There is a message.
        message_count = 0;
        message_is_chars = (message_length & 128) != 0;
        message_is_broadcast = (message_length & 64) != 0;
        message_length = message_length & 63;
        while(message_count < message_length) {             // Copy the message.
            message[message_count++] = rxbuf_read_getc();
        }
        rxbuf_read_finish();                                // Remove the message from rxbuf.
        if(message_is_chars) message[message_count] = 0;    // Terminate the string.
        // Dispatch the message to the callbacks:
        if(message_is_broadcast) {
            if(message_is_chars) {
                char c = message[0];
                if(c == 'B') fraise_receivechars_broadcast(message + 1, message_count - 1);
                else if(c == 'F') switch_to_bootloader_if_name_matches(message + 1, message_count - 1);
                else if(c == 'N') assign(message + 1, message_count - 1);
            }
            else fraise_receivebytes_broadcast(message, message_count);
        } else {
            if(message_is_chars) fraise_receivechars(message, message_count);
            else fraise_receivebytes(message, message_count);
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

void fraise_setup(/*bool background_rx*/) {
    // Get the pin numbers
    int rxpin, txpin, drvpin;
    fraise_get_pins(&rxpin, &txpin, &drvpin);

    // Setup an async context and worker to perform work when needed
    //background_rx = _background_rx;
    if(background_rx) {
        if (!async_context_threadsafe_background_init_with_defaults(&background_context)) {
            panic("failed to setup context");
        } else context_core = &background_context.core;
    } else {
        if (!async_context_poll_init_with_defaults(&poll_context)) {
            panic("failed to setup context");
        } else context_core = &poll_context.core;
    }
    async_context_add_when_pending_worker(context_core, &worker);

    // Set up the state machine we're going to use
    if (!init_pio(&fraise_program, &pio, &sm, &pgm_offset)) {
        panic("failed to setup pio");
    }
    fraise_program_init(pio, sm, pgm_offset, rxpin, txpin, drvpin);

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
    irq_add_shared_handler(pio_irq, fraise_irq, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY); // Add a shared IRQ handler
    irq_set_enabled(pio_irq, true); // Enable the IRQ
    irq_index = pio_irq - ((pio == pio0) ? PIO0_IRQ_0 : PIO1_IRQ_0); // Get index of the IRQ
    pio_set_irqn_source_enabled(pio, irq_index, pis_sm0_rx_fifo_not_empty + sm, true); // Set pio to tell us when the FIFO is NOT empty

    // Register the print function with stdio_fraise
    stdio_set_driver_enabled(&stdio_fraise, true);
}

void fraise_setID(uint8_t id) {
    FraiseID = id;
}

void fraise_get_pins(int *rxpin, int *txpin, int *drvpin)
{
	*rxpin = (watchdog_hw->scratch[7] >> 10) & 31;
	*txpin = (watchdog_hw->scratch[7] >> 5) & 31;
	*drvpin = (watchdog_hw->scratch[7] >> 0) & 31;
}

void fraise_unsetup() {
    // Disable interrupt
    fraise_program_disable_tx_interrupt(pio, sm, irq_index);
    pio_set_irqn_source_enabled(pio, irq_index, pis_sm0_rx_fifo_not_empty + sm, false);
    irq_set_enabled(pio_irq, false);
    irq_remove_handler(pio_irq, fraise_irq);

    // Cleanup pio
    pio_sm_set_enabled(pio, sm, false);
    pio_remove_program(pio, &fraise_program, pgm_offset);
    pio_sm_unclaim(pio, sm);

    async_context_remove_when_pending_worker(context_core, &worker);
    async_context_deinit(context_core);
}

void fraise_poll_rx(){
    async_context_poll(context_core);
}

// Put a string, which must be null-terminated
bool fraise_puts(char* msg){
    if (!txbuf_write_init()) {
        //printf("tx buffer full!\n");
        return false;
    }
    char c;
    int i = 0;
    while((c = *msg++)) {
        if(i < 31) txbuf_write_putc(c);
        else break;
        i++;
    }
    txbuf_write_finish(true);
    return true;
}

bool fraise_putbytes(char* data, uint8_t len){
    if (!txbuf_write_init()) {
        //printf("tx buffer full!\n");
        return false;
    }
    int i = 0;
    while(len--) {
        if(i < 31) txbuf_write_putc(*data++);
        else break;
        i++;
    }
    txbuf_write_finish(false);
    return true;
}

// For stdout:
void fraise_out_chars(const char *buf, int len) {
    static char line[64];
    static int count;
    for (int i = 0; i < len; i++) {
        char c = buf[i];
        if(c == '\n') {                             // On end-of-line:
            if (txbuf_write_init()) {               // If there is room in txbuf,
                for (int j = 0; j < count; j++) {   // copy the message to txbuf.
                    if(j < 31) txbuf_write_putc(line[j]);
                    else break;
                }
                txbuf_write_finish(true);           // Validate the message.
            }
            count = 0;                              // Clear line.
        } else line[count++] = c;
    }
}

void fraise_debug_print_next_txmessage(){
#ifdef FRAISE_DEVICE_DEBUG
    int len = txbuf_read_init();
    if(!len) {
        printf("no pending message\n");
        return;
    }
    while(len--) printf("%02x ", txbuf_read_getc());
    putchar('\n');
    txbuf_read_finish();
#endif
}

uint fraise_debug_get_irq_count() {
    uint c = irq_count;
    irq_count = 0;
    return c;
}
uint fraise_debug_get_irq_rx_count() {
    uint c = irq_rx_count;
    irq_rx_count = 0;
    return c;
}

int main() {
	stdio_init_all();
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
	fraise_setup();
	eeprom_setup();
	fraise_setID(eeprom_get_id());
	setup();
	if(background_rx) {
		while(true) {
			loop();
		}
	} else {
		while(true) {
			fraise_poll_rx();
			loop();
		}
	}
}

__attribute__((weak)) void setup(){}
__attribute__((weak)) void loop(){}
#define STRINGIFY(x) #x
#ifdef FRAISE_DEVICE_DEBUG
#define dummy_callback(f) __attribute__((weak)) void f(char *data, uint8_t len){ printf("dummy " STRINGIFY(f) "()\n");}
#else
#define dummy_callback(f) __attribute__((weak)) void f(char *data, uint8_t len){}
#endif

dummy_callback(fraise_receivebytes);
dummy_callback(fraise_receivechars);
dummy_callback(fraise_receivebytes_broadcast);
dummy_callback(fraise_receivechars_broadcast);

