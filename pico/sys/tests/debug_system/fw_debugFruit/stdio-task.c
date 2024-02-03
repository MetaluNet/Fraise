#include <pico/stdio.h>
#include <stdio.h>
#include <string.h>
#include "pico/sync.h"

uint8_t lineBuf[256];
uint8_t lineLen;

#define startsWith(str, prefix) (!(strncmp((const char *)(str), (const char *)(prefix), strlen(prefix))))

void processLine() {
	if(startsWith(lineBuf, "#R")) printf("sID02\n");
	else if(startsWith(lineBuf, "#E")) puts((const char*)(lineBuf + 2));
	else if(startsWith(lineBuf, "#V")) printf("sV UsbFraise PicoPied v0.1\n");
	else if(startsWith(lineBuf, "waitack")) printf("ack\n");
	else if(startsWith(lineBuf, "reboot")) {
		sleep_ms(50); // wait for the host to disconnect the USB device
		reboot();
	}
	else if(startsWith(lineBuf, "whoami")) {
		printf("swhoami debugFruit\n");
	}

}

void stdioTask()
{
	int c;
	while((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT){
		if(c == '\n') {
			lineBuf[lineLen] = 0;
			processLine();
			lineLen = 0;
		}
		else lineBuf[lineLen++] = c;
	}
}


