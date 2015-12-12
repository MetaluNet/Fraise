#ifndef SPIMASTER_H
#define SPIMASTER_H

void SPImaster1Init();
void SPImaster2Init();

byte SPImaster1Transfer(byte b);
byte SPImaster2Transfer(byte b);

#endif

