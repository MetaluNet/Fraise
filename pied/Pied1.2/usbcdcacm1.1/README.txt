30.3.2013/Kustaa Nyholm

README.TXT for USB CDC ACM v 1.1 for PIC18F4550

This is a self contained example code that implements
a USB CDC ACM aka Virtual Serial Port.

Brought up to date and improved thanks to Ron Musgrave's
pestering, thank's Ron, I enjoyed the whole process.

As delivered the main program reads characters sent
to the virtual serial port (COMn on Windows, /dev/ttyACM0
on Linux and /dev/tty.tty.usbmodem1411 on Mac OS X, exact names 
may vary) and sends back the decimal value of the ASCII
character sent to the terminal. In other words if you do
for example on Mac OS X Terminal:

screen /dev/tty.tty.usbmodem1411

and type 'hello' you should see on the screen:

Wellcome!
104
101
108
108
111

(The 'Wellcome' text will only appear the first time
you try this after pluging in the device).

This version (1.1) compiles with SDCC 3.2.0 and supports
the optional GET_LINE_CODING CDC ACM message without
which some overzealous Windows software things the 
COM port is not working.

This version has been tested on Mac OS X 10.8.2 and 
Ubuntun 9.04 with 'screen' and on Windows XP (SP2) 
and Windows 7 Ultimate 32 bit  with 'putty' 0.62, 
Tera Term Pro 2.3 and HyperTerminal Private Edition 7.0.

The code works with PIC18F4550 with 4 MHz Xtal.

The Makefile has some hardcoded paths that are specific
to my installation because I need to maintain several
parallel SDCC installations. You need to modify those
to suit your installation. 

For standard SDCC installation it probably is enough to 
change SDCC setting to:

SDCC = /usr/local/bin/sdcc

and remove from the SDCCFLAGS setting this:

-L /Users/nyholku/sdcc-3.2.0/share/sdcc/non-free/lib/pic16

have not tested that though.

The Makefile calls script 'load' which expects to find
'pk2cmd' on the PATH (augmented in the Makefile) and
which of course assumes that a PICKit2 is available.

The code was developed on Mac OS X so may need some
tweaking to get the compiling to work on other operating
systems.

All Linux and Windows testing was performed under 
Parallels Desktop 7.0.15107.


Thats all folks, Kusti

