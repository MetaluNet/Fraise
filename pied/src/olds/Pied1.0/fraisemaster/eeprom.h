// read and write eeprom of PIC18F2550 (and 18F2455, 18F4455, 18F4550)
// EEPROM size is 256 bytes
// (c) Raphael Wimmer. Licensed under GNU GPL v2 or higher
// modified by Antoine Rousseau 17 jan 2010

//#include <pic18fregs.h>


void ee_write_byte(unsigned char address, unsigned char _data){

    EEDATA = _data;
    EEADR = address;
    // start write sequence as described in datasheet, page 91
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.WREN = 1; // enable writes to data EEPROM
    INTCONbits.GIEH = 0;  // disable interrupts
    //INTCONbits.GIEL = 0;  // disable interrupts
    EECON2 = 0x55;
    EECON2 = 0x0AA;
    EECON1bits.WR = 1;   // start writing
    while(EECON1bits.WR){
        __asm nop __endasm;}
    if(EECON1bits.WRERR){
        printf("ERROR: writing to EEPROM failed!\n");
    }
    EECON1bits.WREN = 0;
    INTCONbits.GIEH = 1;  // enable interrupts
    //INTCONbits.GIEL = 1;  // enable interrupts
}

unsigned char ee_read_byte(unsigned char address){
    EEADR = address;
    EECON1bits.CFGS = 0;
    EECON1bits.EEPGD = 0;
    EECON1bits.RD = 1;
    return EEDATA;
}
