/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FRAISE_DONT_OVERWRITE_PRINTF
#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"
#include "fraise_eeprom.h"
#include "fraise_master.h"
#include "pico/multicore.h"

#if PICO_RP2040
#include "RP2040.h"
#else
#include "RP2350.h"
#endif

char lineBuf[1024];
uint8_t lineLen;
uint8_t piedID = 1;

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

void reboot() {
    sleep_ms(50); // wait for the host to disconnect the USB device
    fraise_disable_interrupts();
    multicore_reset_core1();
    reset_peripherals();
    watchdog_reboot(0, 0, 0);
    while (1) {
        tight_loop_contents();
    }
}

void processSysLine() {
    static bool unlocked = false;
    switch(lineBuf[1]) {
    case 'R':
        printf("sID%02X\n", piedID);
        fraise_master_bootload_stop();
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
    case 'S':
        fraise_master_set_poll(gethexbyte(lineBuf + 2), true);
        break;
    case 'C':
        fraise_master_set_poll(gethexbyte(lineBuf + 2), false);
        break;
    case 'i':
        fraise_master_reset_polls();
        break;
    case 'F':
        fraise_master_bootload_stop();
        break;
    case 'P':
        fraise_bootloader_use_pico(lineBuf[2] != '0');
    }
    unlocked = false;
}

void processBroadcast() {
    switch(lineBuf[1]) {
    case 'N':
        fraise_master_assign(lineBuf + 4, gethexbyte(lineBuf + 2));
        break;
    case 'b': {
        char buf[64];
        uint8_t count = 2;
        uint8_t bufcount = 0;
        while(count < lineLen - 1) {
            buf[bufcount++] = gethexbyte(lineBuf + count);
            count += 2;
        }
        fraise_master_sendbytes_broadcast(buf, bufcount);
    }
    break;
    case 'B':
        fraise_master_sendchars_broadcast(lineBuf + 2);
        break;
    case 'F':
        fraise_master_bootload_start(lineBuf + 2);
        break;
    }
}

#define startsWith(str, prefix) (!(strncmp((const char *)(str), (const char *)(prefix), strlen(prefix))))
#define ishex(x) ((x >= '0'&& x <='9') || (x >= 'A' && x <= 'F'))
void processLine() {
    if(lineBuf[0] == '#') processSysLine();
    else if(startsWith(lineBuf, "waitack")) printf("ack\n");
    else if(startsWith(lineBuf, "reboot")) {
        sleep_ms(100); // wait for the host to disconnect the USB device
        reboot();
    }
    else if(startsWith(lineBuf, "whoami")) {
        printf("swhoami fraise_master\n");
    }
    else if(fraise_master_is_bootloading()) fraise_master_bootload_getline(lineBuf, strlen(lineBuf));
    else if(lineBuf[0] == '!') processBroadcast();
    else if(ishex(lineBuf[0]) && ishex(lineBuf[1])) { // normal output to fruit
        uint8_t id = gethexbyte(lineBuf);
        if(id < 128) {
            char buf[64];
            uint8_t count = 2;
            uint8_t bufcount = 0;
            while(count < lineLen - 1) {
                buf[bufcount++] = gethexbyte(lineBuf + count);
                count += 2;
            }
            fraise_master_sendbytes(id, buf, bufcount);
        }
        else fraise_master_sendchars(id & 127, lineBuf + 2);
    }
}

void stdioTask()
{
    int c;
    while((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
        if(c == '\n') {
            lineBuf[lineLen] = 0;
            processLine();
            lineLen = 0;
        }
        else if(c == '&') { // inner line break
            char tmp = lineBuf[lineLen];
            lineBuf[lineLen] = 0;
            processLine();
            lineLen = 0;
            lineBuf[lineLen] = tmp;
        }
        else lineBuf[lineLen++] = c;
    }
}

int main() {
    stdio_init_all();
#ifdef PICO_DEFAULT_LED_PIN
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
#endif
    fraise_setup();
    eeprom_setup();
    setup();
    while(true) {
        stdioTask();
        if(fraise_master_is_bootloading()) fraise_master_bootload_service();
        else fraise_master_service();
        loop();
    }
}

__attribute__((weak)) void setup() {}
__attribute__((weak)) void loop() {}
#define STRINGIFY(x) #x
#ifdef FRAISE_MASTER_DEBUG
#define dummy_callback(f) __attribute__((weak)) void f(const char *data, uint8_t len){ printf("dummy " STRINGIFY(f) "()\n");}
#else
#define dummy_callback(f) __attribute__((weak)) void f(const char *data, uint8_t len){}
#endif

dummy_callback(fraise_receivebytes);
dummy_callback(fraise_receivechars);
dummy_callback(fraise_receivebytes_broadcast);
dummy_callback(fraise_receivechars_broadcast);

__attribute__((weak)) void fraise_master_receivebytes(uint8_t fruit_id, const char *data, uint8_t len) {}
__attribute__((weak)) void fraise_master_receivechars(uint8_t fruit_id, const char *data, uint8_t len) {}
__attribute__((weak)) void fraise_master_fruit_detected(uint8_t fruit_id, bool detected) {}


