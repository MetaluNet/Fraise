comport - Pd external for unix/windows to use the serial ports

 (c) 1998-2005  Winfried Ritsch (see LICENCE.txt)
 Institute for Electronic Music - Graz

on Windows the COM0, COM1, ... are used and 
under Unix devices /dev/ttyS0, /dev/ttyS1, ...
and new on unix /dev/USB0, ... and can be accessed via a Index.

Please see testcomport.pd for more help.

USE: There should be a external comport.dll for windows, comport.pd_linux for linux and so on.

just copy it to the extra folder of your pd Installation or working directory. 
Please see testcomport.pd for more help.

if you have improvements or questions feel free to contact me under
ritsch _at_ iem.at

if you want to compile the newest (bleeding edge, and possibly unstable) source
code yourself, you can get a copy from the public git repository:
   https://git.iem.at/pd/comport/

there's also an issue tracker for reporting bugs and requesting new features
available at:
   https://git.iem.at/pd/comport/-/issues

