/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FRAISE_BOOT_DEVICE_H
#define _FRAISE_BOOT_DEVICE_H

void fraise_setup(uint rxpin, uint txpin, uint drvpin, bool drvlevel);
void fraise_unsetup();

bool fraise_getword(uint16_t *res); // if true, res is the next word received by Fraise.
void fraise_puts(const char *msg); // send null terminated string

#endif // _FRAISE_BOOT_DEVICE_H
