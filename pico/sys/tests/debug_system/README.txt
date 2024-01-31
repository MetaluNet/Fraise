This is a set of firmware for debug purpose.

The aim is to ease debugging and development of the usb and fraise bootloaders, 
and the fraise_device and fraise_master libraries.

-------

In 0_debug_system.pd, the pied1 is the main pied; it is a pico based pied (flashed with the normal usb_bootloader)
, but can also be replaced by the previous pic18f based version, to test compatibility with the pico based fruit1.

The "debugPied" fruit is actually the virtual fruit of pied1. Rebuilding its firmware is the way to recompile
the whole pied firmware, and allows building simple debugging communication.

-------

The fruit1 is a pico fruit, but with a USB connexion with the computer. Its fraise_bootloader is also modified
to setup an USB connexion, and is relocated in flash after a slightly modified version of the pied's usb_bootloader.
This modified fraise_bootloader hence acts as a third-level bootloader.

The pied2 is a "fake pied" hosting fruit1. It's mainly used to ease the opening of the debug-usb-port of fruit1.

-------

The fruit2 is a (pic18f based) Versa2 serving to test compatibility.
