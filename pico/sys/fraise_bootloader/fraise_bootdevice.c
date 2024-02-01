/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include "hardware/pio.h"
#include "fraise.pio.h"
#include "fraise_bootdevice.h"

// This program
// - Uses a PIO state machine to receive data
// - Use an interrupt to determine when the PIO FIFO has some data
// - Saves characters in a queue

#define FIFO_SIZE 256

static PIO pio;
static uint sm;
static int8_t pio_irq;
static queue_t fifo;
static uint offset;
static uint irq_index;

// IRQ called when the pio rx fifo is not empty, i.e. there are some characters on the uart
static void pio_irq_func(void) {
    while(!pio_sm_is_rx_fifo_empty(pio, sm)) {
        if(pio_interrupt_get(pio, 4)) { // Framing error! Discard.
            pio_interrupt_clear(pio, 4);
            continue;
        }
        uint16_t c = fraise_program_getc(pio, sm);
        if (!queue_try_add(&fifo, &c)) panic("fifo full");
    }
}

// Find a free pio and state machine and load the program into it.
// Returns false if this fails
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

void fraise_setup(uint rxpin, uint txpin, uint drvpin) {
    // create a queue so the irq can save the data somewhere
    queue_init(&fifo, 2, FIFO_SIZE);

    if (!init_pio(&fraise_program, &pio, &sm, &offset)) {
        panic("failed to setup pio");
    }
    fraise_program_init(pio, sm, offset, rxpin, txpin, drvpin);

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
    irq_add_shared_handler(pio_irq, pio_irq_func, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY); // Add a shared IRQ handler
    irq_set_enabled(pio_irq, true); // Enable the IRQ
    irq_index = pio_irq - ((pio == pio0) ? PIO0_IRQ_0 : PIO1_IRQ_0); // Get index of the IRQ
    pio_set_irqn_source_enabled(pio, irq_index, pis_sm0_rx_fifo_not_empty + sm, true); // Set pio to tell us when the FIFO is NOT empty
}

void fraise_unsetup() {
    // Disable interrupt
    pio_set_irqn_source_enabled(pio, irq_index, pis_sm0_rx_fifo_not_empty + sm, false);
    irq_set_enabled(pio_irq, false);
    irq_remove_handler(pio_irq, pio_irq_func);

    // Cleanup pio
    pio_sm_set_enabled(pio, sm, false);
    pio_remove_program(pio, &fraise_program, offset);
    pio_sm_unclaim(pio, sm);
}

bool fraise_getword(uint16_t *res) {
	if (queue_is_empty(&fifo)) return false;
	uint16_t c;
	if (!queue_try_remove(&fifo, &c)) {
		panic("fifo empty");
	}
	*res = c;
	return true;
}

void fraise_puts(char *msg){
    fraise_program_start_tx(pio, sm, strlen(msg));
    char c;
    while((c = *msg++)) fraise_program_putc(pio, sm, c);
}

