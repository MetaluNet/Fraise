/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/flash.h"
#include "pico/time.h"
#include "RP2040.h"
#include "hardware/flash.h"
#include "hardware/resets.h"
#include "hardware/sync.h"
#include "hardware/structs/watchdog.h"
#include "hardware/watchdog.h"

#include <stdio.h>
#include <string.h>

#include "fraise_bootdevice.h"
#include "bootloader.h"
#include "boardconfig.h"

#ifdef FRAISE_BLD_DEBUG

#define DEBUG printf
bool DONT_WRITE = true;

#else

#define DEBUG
bool DONT_WRITE = false;

#endif


extern int __fraise_app_start__, __fraise_app_length__;
#define FLASH_ADDR_MIN ((uint32_t)&__fraise_app_start__)
#define FLASH_ADDR_MAX (FLASH_ADDR_MIN + (uint32_t)&__fraise_app_length__)

uint8_t pageBuf[FLASH_PAGE_SIZE];
uint32_t pgmSize;
uint32_t address;
uint32_t lastAddress;
uint32_t startAddress;
uint16_t lastoffset;
bool verbose = false;

static void disable_interrupts(void)
{
    SysTick->CTRL &= ~1;

    NVIC->ICER[0] = 0xFFFFFFFF;
    NVIC->ICPR[0] = 0xFFFFFFFF;
}

static void reset_peripherals(void)
{
    reset_block(~(
                    RESETS_RESET_IO_QSPI_BITS |
                    RESETS_RESET_PADS_QSPI_BITS |
                    RESETS_RESET_SYSCFG_BITS |
                    RESETS_RESET_PLL_SYS_BITS
                ));
}


void run_app()
{
    // save pins in watchdog scratch registers
    watchdog_hw->scratch[6] = ('p' << 24) + ('i' << 16) + ('n' << 8) + 's';
    watchdog_hw->scratch[7] = (FRAISE_RX_PIN << 10) + (FRAISE_TX_PIN << 5) + FRAISE_DRV_PIN;

    fraise_unsetup();
    disable_interrupts();
    reset_peripherals();
    const uint32_t vtor = FLASH_ADDR_MIN;
    // Derived from the Leaf Labs Cortex-M3 bootloader.
    // Copyright (c) 2010 LeafLabs LLC.
    // Modified 2021 Brian Starkey <stark3y@gmail.com>
    // Originally under The MIT License
    uint32_t reset_vector = *(volatile uint32_t *)(vtor + 0x04);

    SCB->VTOR = (volatile uint32_t)(vtor);

    asm volatile("msr msp, %0"::"g"
                 (*(volatile uint32_t *)vtor));
    asm volatile("bx %0"::"r" (reset_vector));

    while(1);
}

uint8_t gethexbyte(const char *buf)
{
    uint8_t cl, ch;
    ch = buf[0] - '0';
    if(ch > 9) ch += '9' - 'A' + 1;
    cl = buf[1] - '0';
    if(cl > 9) cl += '9' - 'A' + 1;
    return (ch << 4) + cl;
}

static void programCurrentPage() {
    uint32_t addr = lastAddress & ~(FLASH_PAGE_SIZE - 1);
    if((addr < FLASH_ADDR_MIN) || (addr + FLASH_PAGE_SIZE >= FLASH_ADDR_MAX)) {
        DEBUG("e programCurrentPage address error! addr = %#10lx\n", addr);
        return;
    }
    if(DONT_WRITE) return;
    uint32_t status = save_and_disable_interrupts();
    flash_range_program((lastAddress & ~(FLASH_PAGE_SIZE - 1)) - XIP_BASE, pageBuf, FLASH_PAGE_SIZE);
    restore_interrupts(status);
}

static void eraseSector(uint32_t addr) {
    if((addr < FLASH_ADDR_MIN) || (addr + FLASH_SECTOR_SIZE >= FLASH_ADDR_MAX)) {
        DEBUG("e eraseSector address error! addr = %#10lx\n", addr);
        return;
    }
    if(DONT_WRITE) return;
    uint32_t status = save_and_disable_interrupts();
    flash_range_erase(addr - XIP_BASE, FLASH_SECTOR_SIZE);
    restore_interrupts(status);
}

static void addDataByte(uint32_t writeAddress, uint8_t b) {
    if(
        ((writeAddress & ~(FLASH_PAGE_SIZE - 1)) != (lastAddress & ~(FLASH_PAGE_SIZE - 1)))
        && (lastAddress != 0)
    )
    {
        if(verbose) DEBUG("l program page change: writing page = %#08lx (currently writing to %#08lx)\n", (lastAddress & ~(FLASH_PAGE_SIZE - 1)) - XIP_BASE, writeAddress - XIP_BASE);
        programCurrentPage();
    }

    if((writeAddress & ~(FLASH_SECTOR_SIZE - 1)) != (lastAddress & ~(FLASH_SECTOR_SIZE - 1)))
    {
        if(verbose) DEBUG("l program block change: erasing block = %#08lx (currently writing to %#08lx)\n", (writeAddress & ~(FLASH_SECTOR_SIZE - 1)) - XIP_BASE, writeAddress - XIP_BASE);
        eraseSector(writeAddress & ~(FLASH_SECTOR_SIZE - 1));
    }

    lastAddress = writeAddress;
    pageBuf[(writeAddress & (FLASH_PAGE_SIZE - 1))] = b;
}

int processHexLine(const char *lineBuf, uint8_t lineLen) {
    uint8_t bytes[32];
    uint8_t nbytes = 0;
    uint8_t checksum = 0;

    if(lineBuf[0] == ':') {
        for(uint8_t i = 1; i < lineLen; i += 2, nbytes++) {
            checksum += bytes[nbytes] = gethexbyte(lineBuf + i);
        }
    } else if(lineBuf[0] == '%') {
        for(uint8_t i = 1; i < lineLen; i += 1, nbytes++) {
            checksum += bytes[nbytes] = lineBuf[i];
        }
    }

    if(checksum != 0) {
        DEBUG("e checksum error: %d\n", checksum);
        return -1;
    }

    uint8_t datalen = bytes[0];
    uint16_t offset = ((uint16_t)bytes[1] << 8) + bytes[2];
    uint8_t rectype = bytes[3];

    //if(verbose) DEBUG("l rectype: %d\n", rectype);
    switch(rectype) {
    case 0: /*data*/
        for(uint8_t i = 0; i < datalen; i++) {
            addDataByte(address + offset + i, bytes[4 + i]);
        }
        pgmSize += datalen;
        return 0;
    case 1: /* end of file */
        if((lastAddress & (FLASH_PAGE_SIZE - 1)) != 0) {
            if(verbose) DEBUG("l Need to write last page = %lx\n", (lastAddress & ~(FLASH_PAGE_SIZE - 1)) - XIP_BASE);
            programCurrentPage();
        }
        DEBUG("l programming finished. pgmSize=%ld\n", pgmSize);
        return 1;
    case 2: /* Extended Segment Address */
        /* rp2040 hex files don't seem to use this. */
        DEBUG("l unsupported record type 2 'Extended Segment Address' in hexfile!\n");
        return -2;
    case 3: /* Start Segment Address */
        /* rp2040 hex files don't use this. */
        DEBUG("l unsupported record type 3 'Start Segment Address' in hexfile!\n");
        return -3;
    case 4: /* Extended Linear Address */
        address = ((bytes[4] << 8) + (bytes[5])) << 16;
        if(verbose) DEBUG("l program address=%lx\n", address);

        if(address == FLASH_ADDR_MIN) { // start of file
            pgmSize = 0;
            lastAddress = 0;
        }
        return 0;
    case 5: /* Start Linear Address */
        startAddress = (bytes[4] << 24) + (bytes[5] << 16) + (bytes[6] << 8) + bytes[7];
        if(verbose) DEBUG("l start address=%lx (currently unused)\n", startAddress);
        return 0;
    }
    return 0;
}

void setVerbose(bool _verbose) {
    verbose = _verbose;
}

void reboot() {
    hw_clear_bits(&watchdog_hw->ctrl, WATCHDOG_CTRL_ENABLE_BITS);
    watchdog_reboot(0, 0, 0);

    while (1) {
        tight_loop_contents();
    }
}

