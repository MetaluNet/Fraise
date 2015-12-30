![fraise-logo](http://metalu.net/IMG/png/siteon0.png)

**FRAISE** : **FRA**mework for **I**nterfacing **S**oftware and **E**lectronics

--------------------------------

**Fraise** is :


-	a protocol for communication between microcontrollers
-	some conventions about the communication physical layers and associated connectors 
-	a multi-platform integrated development and runtime environment hosted by Pure Data (www.puredata.info) for microcontroller boards

Fraise boards are currently powered by Microchip's PIC18F26K22 8-bit microcontrollers, which have similar capabilities to Arduino's ATmega328.  
Compilation of the firmware is done with [SDCC](http://sdcc.sourceforge.net); Fraise ships binaries of SDCC for Linux-x86, Windows and OSX (Linux-arm to come), and automates from Pd all the compilation and upload processes.

![fruit-utils](http://metalu.net/local/cache-vignettes/L321xH101/fruit_utils-30b1e.png)

--------------------------------

##dependencies :

PureData + externals :

-	zexy
-	moonlib 
-	hcs 
-	ggee 
-	comport


##installation :

Install Pd + externals (or pd-extended).

Download latest Fraise version from Github : <https://github.com/MetaluNet/Fraise/archive/master.zip>.

Extract the archive somewhere, declare this path into Pd preferences.


##try :

Open `fruit/example/exampleFruit.pd` with Pd.

##doc :

...will come progressively.

For now, look at <http://metalu.net/ressources-techniques/fraise>.

--------------------------------
	Antoine Rousseau 2007-2015
license : GNU GPL (see [LICENSE.txt](LICENSE.txt) )
