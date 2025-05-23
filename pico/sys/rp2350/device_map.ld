/* Fraise device memory map */

MEMORY
{
    FRAISE_BLD(r)   : ORIGIN = 0x10000000 , LENGTH = 28k
    EEPROM(r)       : ORIGIN = 0x10000000 + 28k, LENGTH = 4k
    APP(rx)         : ORIGIN = 0x10000000 + 32k, LENGTH = 4096k - 32k
    RAM(rwx)        : ORIGIN = 0x20000000, LENGTH = 512k
    SCRATCH_X(rwx)  : ORIGIN = 0x20040000, LENGTH = 4k
    SCRATCH_Y(rwx)  : ORIGIN = 0x20041000, LENGTH = 4k
}

__fraise_bootloader_start__ = ORIGIN(FRAISE_BLD);
__eeprom_start__ = ORIGIN(EEPROM);
__fraise_app_start__ = ORIGIN(APP);
__fraise_app_length__ = LENGTH(APP);

