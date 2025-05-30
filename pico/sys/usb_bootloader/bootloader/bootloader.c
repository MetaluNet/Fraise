/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/flash.h"

#if PICO_RP2040
#include "RP2040.h"
#else
#include "RP2350.h"
#endif

#include "pico/time.h"
#include "hardware/flash.h"
#include "hardware/resets.h"
#include "hardware/sync.h"
#include "fraise_eeprom.h"
#include <stdio.h>
#include <string.h>

extern int __fraise_app_start__, __fraise_app_length__;
#define FLASH_ADDR_MIN ((uint32_t)&__fraise_app_start__)
#define FLASH_ADDR_MAX (FLASH_ADDR_MIN + (uint32_t)&__fraise_app_length__)

uint8_t lineBuf[1024];
uint8_t pageBuf[FLASH_PAGE_SIZE];
uint32_t pgmSize;
uint32_t address;
uint32_t lastAddress;
uint32_t startAddress;
uint16_t lastoffset;
int lineLen;
bool verbose = false;
uint8_t piedID = 1;

static void jump_to_vtor(uint32_t vtor)
{
    // Derived from the Leaf Labs Cortex-M3 bootloader.
    // Copyright (c) 2010 LeafLabs LLC.
    // Modified 2021 Brian Starkey <stark3y@gmail.com>
    // Originally under The MIT License
    uint32_t reset_vector = *(volatile uint32_t *)(vtor + 0x04);

    SCB->VTOR = (volatile uint32_t)(vtor);

    asm volatile("msr msp, %0"::"g"
                 (*(volatile uint32_t *)vtor));
    asm volatile("bx %0"::"r" (reset_vector));
}

static void fraise_disable_interrupts(void)
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

uint8_t gethexbyte(const uint8_t *buf)
{
    uint8_t cl, ch;
    ch = buf[0] - '0';
    if(ch > 9) ch += '9' - 'A' + 1;
    cl = buf[1] - '0';
    if(cl > 9) cl += '9' - 'A' + 1;
    return (ch << 4) + cl;
}

//#define DONT_WRITE

void programCurrentPage() {
    uint32_t addr = lastAddress & ~(FLASH_PAGE_SIZE - 1);
    if((addr < FLASH_ADDR_MIN) || (addr + FLASH_PAGE_SIZE >= FLASH_ADDR_MAX)) {
        printf("e programCurrentPage address error! addr = %#10x\n", addr);
        return;
    }
#ifndef DONT_WRITE
    uint32_t status = save_and_disable_interrupts();
    flash_range_program((lastAddress & ~(FLASH_PAGE_SIZE - 1)) - XIP_BASE, pageBuf, FLASH_PAGE_SIZE);
    restore_interrupts(status);
#endif
}

void eraseSector(uint32_t addr) {
    if((addr < FLASH_ADDR_MIN) || (addr + FLASH_SECTOR_SIZE >= FLASH_ADDR_MAX)) {
        printf("e eraseSector address error! addr = %#10x\n", addr);
        return;
    }
#ifndef DONT_WRITE
    uint32_t status = save_and_disable_interrupts();
    flash_range_erase(addr - XIP_BASE, FLASH_SECTOR_SIZE);
    restore_interrupts(status);
#endif
}

void addDataByte(uint32_t writeAddress, uint8_t b) {
    if(
        ((writeAddress & ~(FLASH_PAGE_SIZE - 1)) != (lastAddress & ~(FLASH_PAGE_SIZE - 1)))
        && (lastAddress != 0)
    )
    {
        if(verbose) printf("l program page change: writing page = %#08lx (currently writing to %#08lx)\n", (lastAddress & ~(FLASH_PAGE_SIZE - 1)) - XIP_BASE, writeAddress - XIP_BASE);
        programCurrentPage();
    }

    if((writeAddress & ~(FLASH_SECTOR_SIZE - 1)) != (lastAddress & ~(FLASH_SECTOR_SIZE - 1)))
    {
        if(verbose) printf("l program block change: erasing block = %#08lx (currently writing to %#08lx)\n", (writeAddress & ~(FLASH_SECTOR_SIZE - 1)) - XIP_BASE, writeAddress - XIP_BASE);
        eraseSector(writeAddress & ~(FLASH_SECTOR_SIZE - 1));
    }

    lastAddress = writeAddress;
    pageBuf[(writeAddress & (FLASH_PAGE_SIZE - 1))] = b;
}

void processHexLine() {
    uint8_t bytes[32];
    uint8_t nbytes = 0;
    uint8_t checksum = 0;

    for(uint8_t i = 1; i < lineLen; i += 2, nbytes++) {
        checksum += bytes[nbytes] = gethexbyte(lineBuf + i);
    }

    if(checksum != 0) {
        printf("e checksum error: %d\n", checksum);
        return;
    }

    uint8_t datalen = bytes[0];
    uint16_t offset = ((uint16_t)bytes[1] << 8) + bytes[2];
    uint8_t rectype = bytes[3];

    switch(rectype) {
    case 0: /*data*/
        for(uint8_t i = 0; i < datalen; i++) {
            addDataByte(address + offset + i, bytes[4 + i]);
        }
        pgmSize += datalen;
        break;
    case 1: /* end of file */
        if((lastAddress & (FLASH_PAGE_SIZE - 1)) != 0) {
            if(verbose) printf("l Need to write last page = %x\n", (lastAddress & ~(FLASH_PAGE_SIZE - 1)) - XIP_BASE);
            programCurrentPage();
        }
        printf("l programming finished. pgmSize=%d\n", pgmSize);
        break;
    case 2: /* Extended Segment Address */
        /* rp2040 hex files don't seem to use this. */
        printf("l unsupported record type 2 'Extended Segment Address' in hexfile!\n");
        break;
    case 3: /* Start Segment Address */
        /* rp2040 hex files don't use this. */
        printf("l unsupported record type 3 'Start Segment Address' in hexfile!\n");
        break;
    case 4: /* Extended Linear Address */
        address = ((bytes[4] << 8) + (bytes[5])) << 16;
        if(verbose) printf("l program address=%x\n", address);

        if(address == FLASH_ADDR_MIN) { // start of file
            pgmSize = 0;
            lastAddress = 0;
        }
        break;
    case 5: /* Start Linear Address */
        startAddress = (bytes[4] << 24) + (bytes[5] << 16) + (bytes[6] << 8) + bytes[7];
        if(verbose) printf("l start address=%x (currently unused)\n", startAddress);
        break;
    }
}

void runapp() {
    fraise_disable_interrupts();
    reset_peripherals();
    jump_to_vtor(FLASH_ADDR_MIN);
    while(1);
}

void processSysLine() {
    static bool unlocked = false;
    switch(lineBuf[1]) {
    case 'R':
        printf("sID%02X\n", piedID);
        break;
    case 'V':
        printf("sV UsbFraise PicoPied v0.1\n");
        break;
    case 'E':
        puts((const char*)(lineBuf + 2));
        break;
    case 'U':
        if(!strncmp(lineBuf, "#UNLOCK", 7)) unlocked = true;
        return;
    case 'W':
        if(unlocked) {
            piedID = gethexbyte(lineBuf + 2);
            eeprom_set_id(piedID);
            eeprom_commit();
        }
        break;
    }
    unlocked = false;
}

#define startsWith(str, prefix) (!(strncmp(str, prefix, strlen(prefix))))
void processLine() {
    if(lineBuf[0] == '#') processSysLine();
    else if(lineBuf[0] == ':') processHexLine();
    else if(startsWith(lineBuf, "waitack")) printf("ack\n");
    else if(startsWith(lineBuf, "runapp")) {
        sleep_ms(50); // wait for the host to disconnect the USB device
        runapp();
    }
    else if(startsWith(lineBuf, "whoami")) {
        printf("swhoami usb_bootloader\n");
    }
    /*else if(startsWith(lineBuf, "readflash")) {
        uint32_t addr;
        sscanf(lineBuf, "readflash %d", &addr); // scanf adds 50k to the final bin...
        printf("l readflash at %#08x:", addr);
        for(int i = 0; i < 16; i++) {
            printf("%02X", *(const uint8_t *) (XIP_BASE + addr + i));
        }
        printf("\n");
    }*/
    else if(startsWith(lineBuf, "verbose")) {
        verbose = true;
        printf("l verbose on\n");
    }
    else if(startsWith(lineBuf, "noverbose")) {
        verbose = false;
        printf("l verbose off\n");
    }
    else if(startsWith(lineBuf, "getms")) {
        printf("l boot to connected %ldms\n", to_ms_since_boot(get_absolute_time()));
    }
}

int main() {
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    bool had_usb = false;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    stdio_init_all();
    eeprom_setup();
    piedID = eeprom_get_id();

    /*if(piedID == 255) { // piedID isn't initialized, default to ID 1
        eeprom_set_id(piedID = 1);
        eeprom_commit();
    }*/

    gpio_put(LED_PIN, 1);

    while(true) {
        int c = getchar_timeout_us(100000);
        if(!had_usb && stdio_usb_connected()) {
            had_usb = true;
            gpio_put(LED_PIN, 0);
        }
        if(c == PICO_ERROR_TIMEOUT) {
            if((!had_usb) && to_ms_since_boot(get_absolute_time()) > 3000) runapp();
        } else {
            if(c == '\n') {
                lineBuf[lineLen] = 0;
                processLine();
                lineLen = 0;
            }
            else if(c == '&') {
                char tmp = lineBuf[lineLen];
                lineBuf[lineLen] = 0;
                processLine();
                lineLen = 0;
                lineBuf[lineLen] = tmp;
            }
            else lineBuf[lineLen++] = (uint8_t) c;
        }
    }
}

