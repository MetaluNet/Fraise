/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FRAISE_DEVICE_H
#define _FRAISE_DEVICE_H

#include "pico/stdlib.h"
#include <stdio.h>
#include "fraise.h"

// Process incoming messages.
// This function is called automatically every loop()
void fraise_poll_rx();

void fraise_setID(uint8_t id);

void fraise_debug_print_next_txmessage(); // print next pending message to send, and remove it from the list
uint fraise_debug_get_irq_count();
uint fraise_debug_get_irq_rx_count();

#endif // _FRAISE_DEVICE_H
