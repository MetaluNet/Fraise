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

#define FRAISE_DONT_OVERWRITE_PRINTF
#include "fraise.pio.h"
#include "fraise_master.h"
#include "fraise_master_buffers.h"

#include "boardconfig.h"

#define MAX_FRUITS 128

typedef struct {
    bool detected: 1;
} fruit_vstate_t;

typedef struct {
    bool polled: 1;
    bool detected_sent: 1;
} fruit_state_t;

static volatile fruit_vstate_t fruit_vstates[MAX_FRUITS];
static fruit_state_t fruit_states[MAX_FRUITS];

static inline void set_polled(uint8_t id, bool v) {
    fruit_states[id].polled = v;
}
static inline bool is_polled(uint8_t id) {
    return fruit_states[id].polled;
}

static inline void set_detected(uint8_t id, bool v) {
    fruit_vstates[id].detected = v;
}
static inline bool is_detected(uint8_t id) {
    return fruit_vstates[id].detected;
}

static inline void set_detected_sent(uint8_t id, bool v) {
    fruit_states[id].detected_sent = v;
}
static inline bool is_detected_sent(uint8_t id) {
    return fruit_states[id].detected_sent;
}

static bool is_initialized = false;
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
    default:
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
    static bool    rx_is_char;
    static int last_txbuf_head;
    static uint8_t tx_number_tries;

    switch(state) {
    case FMS_POLL:
        fraise_cancel_alarm();
        while(!pio_sm_is_rx_fifo_empty(pio, sm)) fraise_program_getc(); // Flush RXFIFO
        if(pio_sm_is_tx_fifo_full(pio, sm)) {               // this shouldn't happend: the irq was triggered by tx_fifo_not_full
            fraise_program_disable_tx_interrupt();
            fraise_add_alarm(PIO_TXFULL_RETRY_TIME);
            return;
        }
        tx_bytes_to_send = txbuf_read_init();               // if message available on tx_buffer, send it:
        if(tx_bytes_to_send) {
            txbuf_read_getc();                              // discard the first byte in the buffer (the length of the data).
            destination_fruit = txbuf_read_getc();          // the second byte in the buffer is the ID of the destination fruit.
            if(last_txbuf_head != txbuf_read_get_head()) tx_number_tries = 0;
            else if(tx_number_tries++ > 3) {
                txbuf_read_finish();
                printf("sT%02X\n", destination_fruit & 127); // signal timeout and discard the message.
                return;
            }
            last_txbuf_head = txbuf_read_get_head();
            state = FMS_SEND;
            fraise_program_start_tx_with_interrupt(tx_bytes_to_send - 1);
            fraise_program_putc_blocking(destination_fruit | 256);   // set bit9
            tx_bytes_to_send -= 2;
            break;
        }

        // if rxbuf isn't full, poll next fruit if any
        if(is_bootloading || !rxbuf_write_init()) {         // if rxbuf is full or is_bootloading, don't poll and retry later.
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
    default:
        break;
    }

    while((state == FMS_SEND) && !pio_sm_is_tx_fifo_full(pio, sm)) {
        fraise_cancel_alarm();
        while(!pio_sm_is_rx_fifo_empty(pio, sm)) fraise_program_getc(); // Flush RXFIFO
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
                fraise_add_alarm(ONE_BYTE_TIME *                // after all bytes are sent; we need to add one to the fifo_level
                                 (pio_sm_get_tx_fifo_level(pio, sm) + 1));   // to take account of the byte being transferred currently.
            }
        }
    }
}

void fraise_setup() {
    if(is_initialized) return;

    // Get the pin numbers
    int rxpin, txpin, drvpin, drvlevel;
    fraise_get_pins(&rxpin, &txpin, &drvpin, &drvlevel);

    // Reset the buffers
    fraise_master_buffers_reset();

    // Set up the state machine and irq we're going to use
    if (!claim_pio_sm_irq(&fraise_program, &pio, &sm, &pgm_offset, &pio_irq)) {
        //panic("failed to setup pio");
        fraise_printf("fraise_master failed to setup pio!\n");
        return;
    }

    fraise_program_init(rxpin, txpin, drvpin, drvlevel);

    // Enable interrupt
    irq_add_shared_handler(pio_irq, fraise_master_irq_handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY); // Add a shared IRQ handler
    irq_set_enabled(pio_irq, true); // Enable the IRQ
    irq_index = pio_irq - ((pio == pio0) ? PIO0_IRQ_0 : PIO1_IRQ_0); // Get index of the IRQ

    // Cleanup pio
    pio_sm_clear_fifos(pio, sm);
    fraise_master_buffers_reset();

    fraise_program_enable_rx_interrupt(); // Set pio to tell us when the FIFO is NOT empty
    state = FMS_POLL;
    fraise_program_enable_tx_interrupt(); // This should instantly run the IRQ handler.

    is_initialized = true;
}

void fraise_get_pins(int *rxpin, int *txpin, int *drvpin, int *drvlevel)
{
    *rxpin = FRAISE_RX_PIN;
    *txpin = FRAISE_TX_PIN;
    *drvpin = FRAISE_DRV_PIN;
    *drvlevel = FRAISE_DRV_LEVEL;
}

void fraise_unsetup() {
    if(!is_initialized) return;

    fraise_master_reset_polls();
    // Disable interrupt
    fraise_program_disable_tx_interrupt();
    fraise_program_disable_rx_interrupt();
    fraise_cancel_alarm();
    irq_set_enabled(pio_irq, false);
    irq_remove_handler(pio_irq, fraise_master_irq_handler);

    // Cleanup pio
    pio_sm_clear_fifos(pio, sm);
    pio_sm_set_enabled(pio, sm, false);
    pio_remove_program(pio, &fraise_program, pgm_offset);
    pio_sm_unclaim(pio, sm);

    is_initialized = false;
}

void fraise_master_reset() {
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

void fraise_master_assign(const char* fruitname, uint8_t id) {
    if(strlen(fruitname) > 16) {
        printf("e fruit name too long! (16 chars max)");
        return;
    }
    char buffer[32];
    sprintf(buffer, "N%02X%s", id, fruitname);
    fraise_master_sendchars_broadcast(buffer);
}

// -----------------------------

extern bool waitAck; // from fraise_bootload.c
void fraise_master_bootload_start(const char *fruitname) {
    if(strlen(fruitname) > 16) {
        printf("e fruit name too long! (16 chars max)");
        return;
    }
    is_bootloading = true;
    char buffer[32];
    sprintf(buffer, "F%s", fruitname);
    fraise_master_sendchars_broadcast(buffer);
    sleep_ms(10);
    waitAck = false;
    fraise_program_disable_rx_interrupt();
    fraise_program_disable_tx_interrupt();
    fraise_cancel_alarm();
    fraise_master_buffers_reset();
}

void fraise_master_bootload_stop() {
    is_bootloading = false;
    fraise_master_buffers_reset();
    state = FMS_POLL;
    fraise_program_enable_rx_interrupt();
    fraise_program_enable_tx_interrupt();
}

bool fraise_master_is_bootloading() {
    return is_bootloading;
}

void fraise_master_bootload_send(const char *buf, int len) {
    if(!is_bootloading) return;
    uint8_t checksum = len + 1;
    const char *p = buf;
    fraise_program_start_tx(len + 2);
    fraise_program_putc_blocking((uint8_t)(len + 1));
    while(len--) {
        checksum += *p;
        fraise_program_putc_blocking((uint8_t)*p);
        p++;
    }
    fraise_program_putc_blocking((uint8_t)(256 - checksum));
    while(pio_sm_get_tx_fifo_level(pio, sm) != 0);  // wait the fifo is empty, then wait for
    sleep_us(44);                                   // the current byte to be fully transferred.
}

void fraise_master_bootload_send_broadcast(const char *buf, int len) { // e.g "Fmyfruit"
    if(!is_bootloading) return;

    uint8_t checksum = len | 128;
    const char *p = buf;
    fraise_program_start_tx(len + 3);
    fraise_program_putc_blocking(256);
    fraise_program_putc_blocking(len | 128);
    while(len--) {
        checksum += *p;
        fraise_program_putc_blocking((uint8_t)*p);
        p++;
    }
    fraise_program_putc_blocking((uint8_t)(256 - checksum));
    while(pio_sm_get_tx_fifo_level(pio, sm) != 0);  // wait the fifo is empty, then wait for
    sleep_us(44);                                   // the current byte to be fully transferred.
}

bool fraise_master_get_raw_byte(char *w) {
    *w = 0;
    if(pio_interrupt_get(pio, 4)) { // Framing error! Discard.
        pio_interrupt_clear(pio, 4);
        pio_sm_clear_fifos(pio, sm); // flush the rx fifo
        printf("l FERR\n");
        return false;
    }
    if(pio_sm_is_rx_fifo_empty(pio, sm)) return false;
    //*w = fraise_program_getc();
    *w = pio_sm_get_blocking(pio, sm) >> 23;
    return true;
}
// -----------------------------

void fraise_master_set_poll(uint8_t id, bool poll) {
    if(id == 0) {
        if(poll) printf("sC00\n");
        else printf("sc00\n");
    } else if(id < MAX_FRUITS) {
        uint32_t status = save_and_disable_interrupts();
        set_polled(id, poll);
        set_detected(id, false);
        restore_interrupts(status);
        //printf("l polled(%d):%d\n", id, is_polled(id));
    }
}

void fraise_master_reset_polls() {
    uint32_t status = save_and_disable_interrupts();
    for(int id = 0; id < MAX_FRUITS ; id++) {
        set_polled(id, false);
        set_detected(id, false);
        set_detected_sent(id, false);
    }
    restore_interrupts(status);
}

// -----------------------------

void fraise_master_sendbytes_raw(uint8_t id, const char *data, uint8_t len, bool isChar) {
    if(id >= MAX_FRUITS) return;
    if(!txbuf_write_init(len)) {
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
        fraise_init_get_buffer(data, len);
        fraise_receivebytes(data, len);
        return;
    }
    fraise_master_sendbytes_raw(id, data, len, false);
}

void fraise_master_sendchars(uint8_t id, const char *data) {
    int len = strlen(data);
    if(id == 0) {
        fraise_receivechars(data, len);
        return;
    }
    fraise_master_sendbytes_raw(id, data, len, true);
}

void fraise_master_sendbytes_broadcast(const char *data, uint8_t len) {
    fraise_init_get_buffer(data, len);
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
    // Process incoming message
    char buf[64];
    int len;
    while((len = rxbuf_read_init())) {
        bool isChar = len > 127;
        len = (len & 63) - 2;
        uint8_t id = rxbuf_read_getc();
        for(int i = 0; i < len; i++) buf[i] = rxbuf_read_getc();
        if(isChar) {
            printf("%02X", id | 128);
            for(int i = 0; i < len; i++) putchar(buf[i]);
            putchar('\n');
            buf[len] = 0; // end string
            fraise_master_receivechars(id, buf, len);
        }
        else {
            printf("%02X", id);
            for(int i = 0; i < len; i++) printf("%02X", buf[i]);
            putchar('\n');
            fraise_master_receivebytes(id, buf, len);
        }
        rxbuf_read_finish();
    }
    // TODO: pace 'detected changes' updates
    for(int i = 1; i < MAX_FRUITS; i++) {
        if(is_detected_sent(i) != is_detected(i)) {
            bool d = is_detected(i);
            if(d) {
                set_detected_sent(i, true);
                fraise_master_fruit_detected(i, true);
                printf("sC%02X\n", i);
            }
            else {
                set_detected_sent(i, false);
                fraise_master_fruit_detected(i, false);
                printf("sc%02X\n", i);
            }
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

void fraise_print_status() {
    printf("l fr psol %d %d %d %d\n", PIO_NUM(pio), sm, pgm_offset, fraise_program.length); // pio sm offset length
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
    return true;
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

void fraise_printf(const char* fmt, ...) {
    va_list args;
    char buf[64];
    char *p = buf;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    while(*p) fraise_putchar(*p++);
}

