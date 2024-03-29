![fraise-logo](doc/img/Fraise.png)

--------------------------------

[**Fraise**](https://github.com/MetaluNet/Fraise) is an easy yet powerful way to build programs for microcontroller boards, that can communicate together and with [Pure Data](http://www.puredata.info) (Pd).

Fraise is at the same time:

-	an Pd-hosted easy-to-install integrated environment, for programming and flashing the Fraise boards, then interacting with them.
-	a protocol establishing a communication between multiple boards and the host computer
-	a C/C++ firmware-level API, implementing the Fraise protocol plus various hardware-specific modules


Two different microcontrollers board are currently supported by Fraise:

- RaspberryPi Pico which is the more powerful of the two, and is readily available from many retailers at low cost;
- legacy Fraise boards powered by Microchip's PIC18F26K22, which are currently only manufactured at home for personal use. Their main advantage is the number of analog inputs; unfortunately they require the use of SDCC compiler, which isn't maintained anymore for PIC targets.

--------------------------------

## single or multiple boards

Fraise allows multiple boards to be connected to each other and to communicate with the host computer, via the USB connection of the first board.

The first board is fully programmable; it can be the only one in simple cases.  
Additionally, it automatically implements the *Fraise protocol*, which can connect to up to 127 other boards (in the simplest case, the bus is formed by shorting Pico's pins 20 and 21; the buses on the various boards simply need to be connected together).

The Fraise bus is based on 9-bit asynchronous serial communication. Thanks to a carefully selected, moderate bitrate (250 kbs), the microcontroller boards can be located hundreds of meter away from the computer when connected through RS485 transceivers (called Fraiseivers).  

This unique feature allows to build relatively complex installations, involving multiple distant sensors/actuators sets, that communicate reliably via wire with the central application, with minimal latency.

--------------------------------

## installation

Fraise requires Pure Data version 0.54 or later.

Fraise can be installed through the built-in Pd externals installer, aka Deken (menu Help/Find externals), search for the **"Fraise"** library.  

In order to develop Fraise firmwares by yourself, you will need the compilation toolchain (gcc, sdcc, cmake...) which is also available from Deken, install the **"Fraise-toolchain"** library for your system.

On **Linux**: you may need to authorize your user to use the USB serial devices; just type in a terminal:  
`sudo adduser [your_username] dialout`  
then logout (close your session) and re-login.

--------------------------------

## example

1. connect the Pico board in BOOTSEL mode via USB (you may need to hold down the BOOTSEL button if the boards has been previously programmed with another firmware)
2. copy the file `boards/pico/usb_bootloader.uf2` to the Pico's mass-storage directory, which should have been created
3. open `pico/fraise/example/blink.pd` with Pd.

## documentation

The documentation for the Fraise firmware API is available there: [Fraise API doc ](http://metalunet.github.io/Fraise-doc).

The low-level Fraise protocol is documented here: [protocol.md](doc/protocol.md).  

Also, you can have a look at <http://metalu.net/en/outils/fraise-overview/>.



--------------------------------
Fraise code is hosted at <https://github.com/MetaluNet/Fraise>,  
the toolchain builder at <https://github.com/MetaluNet/Fraise-toolchain>.

	Antoine Rousseau 2007-2024  
license : GNU GPL (see [LICENSE.txt](LICENSE.txt) )
