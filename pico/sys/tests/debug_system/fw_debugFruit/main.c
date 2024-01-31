/**
 * 12-channel PWM driver
 */

#define BOARD pico

#include <stdio.h>
#include <string.h>
#include "fraise.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
int ledPeriod = 250;

void setup() {
	gpio_init(0);
	gpio_set_dir(0, GPIO_OUT);
	gpio_put(0, 1);

	gpio_init(12);
	gpio_set_dir(12, GPIO_OUT);
	gpio_put(12, 0);
}

int testval = 10;
void loop(){
	static absolute_time_t nextLed;// = make_timeout_time_ms(100);
	static bool led = false;

	if(absolute_time_min(nextLed, get_absolute_time()) == nextLed) {
		gpio_put(LED_PIN, led = !led);
		nextLed = make_timeout_time_ms(ledPeriod);
		//printf("tv %d\n", testval);
	}
}

void print_next_txmessage(){
    int len = txbuf_read_init();
    if(!len) {
        printf("no pending message\n");
        return;
    }
    while(len--) printf("%02x ", txbuf_read_getc());
    putchar('\n');
    txbuf_read_finish();
}

extern void fraise_master_print_stats();
extern void fraise_master_reset();

void fraise_receivebytes(const char *data, uint8_t len){
	if(data[0] == 1) ledPeriod = (int)data[1] * 10;	
	else {
		printf("rcvd ");
		for(int i = 0; i < len; i++) printf("%d ", (uint8_t)data[i]);
		putchar('\n');
	}
}

void fraise_receivechars(const char *data, uint8_t len){
	if(data[0] == 'E') { // Echo
		printf("E%s\n", data + 1);
	}
}

