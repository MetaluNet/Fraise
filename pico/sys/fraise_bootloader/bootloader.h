/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOOTLOADER_H
#define _BOOTLOADER_H

int processHexLine(const char *lineBuf, uint8_t lineLen); // return 1:finished 0:ok -1:checksum error -2:unsupported2 -3:unsupported3
void setVerbose(bool verbose);
uint8_t gethexbyte(const char *buf);
void run_app();
void reboot();

#endif // _BOOTLOADER_H
