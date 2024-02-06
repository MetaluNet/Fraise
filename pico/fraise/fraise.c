/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/pio.h"

bool claim_pio_sm_irq(const pio_program_t *program, PIO *pio_hw, uint *sm, uint *program_offset, uint *irq) {
    // Find a free pio
    *pio_hw = pio1;
    if (!pio_can_add_program(*pio_hw, program)) {
        *pio_hw = pio0;
        if (!pio_can_add_program(*pio_hw, program)) {
            *program_offset = -1;
            return false;
        }
    }
    *program_offset = pio_add_program(*pio_hw, program);

    // Find a state machine
    *sm = (int8_t)pio_claim_unused_sm(*pio_hw, false);
    if (*sm < 0) {
        return false;
    }

    if(irq != NULL) {
		// Find a free irq
		static_assert(PIO0_IRQ_1 == PIO0_IRQ_0 + 1 && PIO1_IRQ_1 == PIO1_IRQ_0 + 1, "");
		uint pio_irq = (*pio_hw == pio0) ? PIO0_IRQ_0 : PIO1_IRQ_0;
		if (irq_get_exclusive_handler(pio_irq)) {
		    pio_irq++;
		    if (irq_get_exclusive_handler(pio_irq)) {
		        return false;
		    }
		}
		*irq = pio_irq;
    }

    return true;
}

