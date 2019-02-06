![fraise-logo](http://metalu.net/wp-content/uploads/2018/09/logo-Fraise-baseline-red-e1537650333235.png)

--------------------------------

[**Fraise**](https://github.com/MetaluNet/Fraise) is:


-	a protocol for communication between multiple microcontrollers and a host computer
-	some conventions about the physical layers and associated connectors
-	a C library implementing Fraise protocol plus various hardware-related modules at firmware level 
-	a multi-platform integrated environment hosted by [Pure Data](http://www.puredata.info) for interacting with Fraise boards

Fraise boards are currently powered by Microchip's PIC18F26K22 8-bit microcontrollers, which have similar capabilities to Arduino's ATmega328.  

Compilation of the firmware is done with [SDCC](http://sdcc.sourceforge.net) and [gputils](http://gputils.sourceforge.net); Fraise ships with the compiler's binaries for Linux-x86, Linux-arm, Windows and OSX, and automates from Pd all the compilation and flashing processes.

![fruit-utils](http://metalu.net/local/cache-vignettes/L321xH101/fruit_utils-30b1e.png)

--------------------------------

## dependencies:

PureData + externals:

-	zexy
-	moonlib 
-	hcs 
-	ggee 
-	comport

(NOTE : every needed external is now shipped with latest Fraise version for most platforms)


On 64 bit linux, the compiler requires 32 bit compatibility libraries.
On Debian-like do:

`sudo apt-get install libc6-i386`

`sudo apt-get install lib32stdc++6`

## installation:

Install latest release of PureData (0.47-1 at the time of writing).

Download latest Fraise version from Github: <https://github.com/MetaluNet/Fraise/archive/master.zip>.

Extract the archive somewhere, declare this path into Pd preferences.

Alternatively, extract the archive as "Fraise" folder (remove "-master" if needed) in your user-specific Pd folder (see: http://puredata.info/docs/faq/how-do-i-install-externals-and-help-files) so Fraise can be found by [declare -stdpath Fraise]: 

GNU/Linux: ~/.local/lib/pd/extra (since Pd-0.47-1, preferred) or ~/pd-externals (deprecated/older Pd-versions; still usable) 

Mac OS X: ~/Library/Pd 

Windows: "%AppData%\Pd" (since Pd-0.47) <br>
"%AppData%\Pd" will be something like "C:\Users\myusername\AppData\Roaming\pd"<br>
prior to Win7 (e.g. Vista, XP, Win2000) this was synonymous with "%UserProfile%\Application Data\Pd" (which was used before Pd-0.47) 


On Linux, you may need to add your user to the dialout group, 
to have read/write permissions on the USB device /dev/ttyACM0. Just do:<br>
`sudo adduser [your_username] dialout`

then logout (close your session) and re-login.


## examples:

Open `fruit/example/exampleFruit.pd` with Pd.

## documentation:

Fraise library is generated by [doxygen](http://www.stack.nl/~dimitri/doxygen/), and is published there: http://metalunet.github.io/Fraise-doc

Also, have a look at <http://metalu.net/en/outils/fraise-overview/.

--------------------------------
	Antoine Rousseau 2007-2019
license : GNU GPL (see [LICENSE.txt](LICENSE.txt) )
