/* Fraise memory map */

MEMORY
{
    USB_BLD(r)      : ORIGIN = 0x10000000, LENGTH = 68k
    EEPROM(r)       : ORIGIN = 0x10000000 + 68k, LENGTH = 4k
    APP(rx)         : ORIGIN = 0x10000000 + 72k, LENGTH = 2048k - 72k
    RAM(rwx)        : ORIGIN = 0x20000000, LENGTH = 256k
    SCRATCH_X(rwx)  : ORIGIN = 0x20040000, LENGTH = 4k
    SCRATCH_Y(rwx)  : ORIGIN = 0x20041000, LENGTH = 4k
}

__usb_bootloader_start__ = ORIGIN(USB_BLD);
__eeprom_start__ = ORIGIN(EEPROM);
__fraise_app_start__ = ORIGIN(APP);
__fraise_app_length__ = LENGTH(APP);

