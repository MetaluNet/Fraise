/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FRAISE_MASTER_H
#define _FRAISE_MASTER_H

#include "pico/stdlib.h"
#include <stdio.h>
#include "fraise.h"

void fraise_master_set_poll(uint8_t, bool poll);
void fraise_master_reset_polls();
void fraise_master_assign(const char* fruitname, uint8_t id);

void fraise_master_sendbytes(uint8_t id, const char *data, uint8_t len);
void fraise_master_sendchars(uint8_t id, const char *data);
void fraise_master_sendbytes_broadcast(const char *data, uint8_t len);
void fraise_master_sendchars_broadcast(const char *data);

void fraise_master_start_bootload(const char *buf);
void fraise_master_stop_bootload();
void fraise_master_send_bootload(const char *buf);
bool fraise_master_is_bootloading();

void fraise_unsetup();

#endif // _FRAISE_MASTER_H

