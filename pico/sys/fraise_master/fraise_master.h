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

#ifdef __cplusplus
extern "C" {
#endif

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

void fraise_master_bootload_getline(const char *buf, int len);
void fraise_master_bootload_service();
void fraise_bootloader_use_pico(bool useit);

void fraise_master_service();
void fraise_master_reset();

// User define callbacks receiving messages from fruits:
void fraise_master_receivebytes(uint8_t fruit_id, const char *data, uint8_t len);
void fraise_master_receivechars(uint8_t fruit_id, const char *data, uint8_t len);
void fraise_master_fruit_detected(uint8_t fruit_id, bool detected);

// Take the first 2 bytes and convert them from hexadecimal string representation to unsigned char.
// Example: if buf starts with "0A", gethexbyte will return 10
uint8_t gethexbyte(const char *buf);

#ifdef __cplusplus
}
#endif


#endif // _FRAISE_MASTER_H

