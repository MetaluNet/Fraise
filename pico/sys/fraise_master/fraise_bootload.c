/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

#define FRAISE_DONT_OVERWRITE_PRINTF
#include "fraise.h"
#include "fraise_master.h"
#include "fraise_master_buffers.h"

#define MAX_NB_RETRIES 10

bool usePicoBootloader = false;
bool waitAck = false;

uint8_t gethexbyte(const char *buf)
{
    uint8_t cl, ch;
    ch = buf[0] - '0';
    if(ch > 9) ch += '9' - 'A' + 1;
    cl = buf[1] - '0';
    if(cl > 9) cl += '9' - 'A' + 1;
    return (ch << 4) + cl;
}

bool fraise_master_bootload_push(const char *buf, int len) {
    if(!txbuf_write_init(len)) return false;
    while(len--) txbuf_write_putc(*buf++);
    txbuf_write_finish();
    return true;
}

bool fraise_master_bootload_pop_send() {
    uint8_t len = txbuf_read_init();
    char buf[256];
    int buflen = 0;
    if(len == 0) return false;

    txbuf_read_getc();  // discard length byte (will be redo by fraise_master_bootload_send())
    len -= 2;           // remove 2 from len, for length and checksum bytes
    while(len--) buf[buflen++] = txbuf_read_getc();
    txbuf_read_getc();  // remove checksum byte
    //printf("l bld_pop_snd len %d:", buflen); for(int i = 1; i < buflen; i++) printf("%02X", buf[i]); printf("\n");
    fraise_master_bootload_send(buf, buflen);

    return true;
}

void fraise_master_bootload_getline(const char *lineBuf, int lineLen) {
    if(lineBuf[0] == '!' && lineBuf[1] == 'F') {
        fraise_master_bootload_send_broadcast(lineBuf + 1, lineLen - 1);
        waitAck = false;
        sleep_ms(1);
    }
    else if(usePicoBootloader && lineBuf[0] == ':') { // hex line
        char buf[256] = {'%'};
        uint8_t count = 1;
        uint8_t bufcount = 1;
        while(count < lineLen - 1) {
            buf[bufcount++] = gethexbyte(lineBuf + count);
            count += 2;
        }
        if(!fraise_master_bootload_push(buf, bufcount)) printf("e bootloader buffer full!\n");
    } else fraise_master_bootload_send(lineBuf, strlen(lineBuf));
}

void fraise_bootloader_use_pico(bool useit) {
    usePicoBootloader = useit;
}

void fraise_master_bootload_service() {
    static absolute_time_t no_response_timeout;
    //static absolute_time_t data_request_timeout;
    static int nb_retries;
    char c;

    /*if(usePicoBootloader && txbuf_write_init(256) && time_reached(data_request_timeout)) {
        printf("bX\n"); // if enough space in tx buf, request new lines.
        data_request_timeout = make_timeout_time_ms(100);
    }*/

    while(fraise_master_get_raw_byte(&c)) {
        //printf("l frombld raw: %d %c\n", c, c);
        if(usePicoBootloader && c == 'X') {
            if(waitAck) {
                waitAck = false;
                nb_retries = 0;
                txbuf_read_finish(); // remove the message from the buffer
                //if(txbuf_write_init(256)) {
                int free = txbuf_get_freespace();
                //printf("l txbuf free %d\n", free);
                if(free >= 256) {
                    printf("bX\n"); // if enough space in tx buf, request new lines.
                    //data_request_timeout = make_timeout_time_ms(100);
                }
            }
        }
        else /*if(c != ' ' && c != '*')*/ printf("b%c\n", c);
    }

    if(usePicoBootloader) {
        if(!waitAck) {
            if(fraise_master_bootload_pop_send()) {
                waitAck = true;
                no_response_timeout = make_timeout_time_ms(100);
            } else no_response_timeout = at_the_end_of_time;
        } else if(time_reached(no_response_timeout)) {
            if(++nb_retries > MAX_NB_RETRIES) {
                printf("bE\n");
                fraise_master_buffers_reset();
            } else printf("l retry %d\n", nb_retries);
            waitAck = false;
            no_response_timeout = at_the_end_of_time;
        }
    }
}

