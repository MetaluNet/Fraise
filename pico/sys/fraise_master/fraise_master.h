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

void fraise_master_set_poll(uint8_t id, bool poll);
void fraise_master_reset_polls();
void fraise_master_assign(const char* fruitname, uint8_t id);

void fraise_master_sendbytes(uint8_t id, const char *data, uint8_t len);
void fraise_master_sendchars(uint8_t id, const char *data);
void fraise_master_sendbytes_broadcast(const char *data, uint8_t len);
void fraise_master_sendchars_broadcast(const char *data);

void fraise_master_bootload_start(const char *fruitname);
void fraise_master_bootload_stop();
void fraise_master_bootload_send(const char *buf, int len);
void fraise_master_bootload_send_broadcast(const char *buf, int len);
bool fraise_master_is_bootloading();
bool fraise_master_get_raw_byte(char *w);

bool fraise_master_bootload_getline(const char *buf, int len);
void fraise_master_bootload_service();
void fraise_bootloader_use_pico(bool useit);
//bool fraise_master_bootload_push(const char *buf, int len);
//bool fraise_master_bootload_pop_send();

void fraise_master_service();
void fraise_master_reset();

void fraise_unsetup();

uint8_t gethexbyte(const uint8_t *buf);

#endif // _FRAISE_MASTER_H

