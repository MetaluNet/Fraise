/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "pico/stdio/driver.h"

#include "fraise_stdout.h"
#include "fraise.h"

// Function that will be called by stdout.
void fraise_stdout_chars(const char *buf, int len) {
	while(len--) fraise_putchar(*buf++);
}

// Data structure for registering this function with the stdout plumbing.
stdio_driver_t stdio_fraise_stdout = {
    .out_chars = fraise_stdout_chars,
#ifdef PICO_STDIO_ENABLE_CRLF_SUPPORT
    .crlf_enabled = PICO_STDIO_DEFAULT_CRLF
#endif
};

void fraise_stdout_set_enabled(bool enable) {
	stdio_set_driver_enabled(&stdio_fraise_stdout, enable);
}

