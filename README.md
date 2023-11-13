![fraise-logo](http://metalu.net/wp-content/uploads/2018/09/logo-Fraise-baseline-red-e1537650333235.png)

--------------------------------

[**Fraise**](https://github.com/MetaluNet/Fraise) is:


-	a protocol establishing a communication between multiple microcontroller boards and a host computer
-	a C firmware library implementing the Fraise protocol plus various hardware-specific modules
-	a multi-platform integrated environment nested in [Pure Data](http://www.puredata.info) for interacting with Fraise boards

Fraise boards are currently powered by Microchip's PIC18F26K22 8-bit microcontrollers, which have similar capabilities to Arduino's ATmega328.  

The firmware is compiled with [SDCC](http://sdcc.sourceforge.net) and [gputils](http://gputils.sourceforge.net); Fraise ships with the compiler's binaries, and automates from Pd all the compilation and flashing processes.

![fruit-utils](http://metalu.net/local/cache-vignettes/L321xH101/fruit_utils-30b1e.png)

--------------------------------

## installation:

Fraise requires Pure Data version 0.54 or later.

Fraise can be installed through the built-in Pd externals installer (aka Deken).


On Linux, you may need to allow your user to use the USB serial devices; just type in a terminal:<br>
`sudo adduser [your_username] dialout`

then logout (close your session) and re-login.

## dependencies:

Fraise relies on two externals, which bring functionalities that are not (yet) supported by Pd natively:

-	ggee/shell, or motex/system on Windows, to allow execution of a batch file
-	comport, to access USB serial devices

These externals are included in the Fraise distribution for most platforms, so it shouldn't be necessary to install anything.


## examples:

Open `fruit/example/exampleFruit.pd` with Pd.

## documentation:

Fraise library is generated by [doxygen](http://www.stack.nl/~dimitri/doxygen/), and is published there: http://metalunet.github.io/Fraise-doc

Also, have a look at <http://metalu.net/en/outils/fraise-overview/.

The latest Fraise version can be downloaded from: <https://github.com/MetaluNet/Fraise>.


--------------------------------
	Antoine Rousseau 2007-2023
license : GNU GPL (see [LICENSE.txt](LICENSE.txt) )
