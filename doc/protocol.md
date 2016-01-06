
Fraise Protocol v 2.1
=====================
*(Antoine Rousseau 2013-2015)*

**Fraise** is a half-duplex asynchronous serial communication protocol.

Up to **126 devices** (called "fruits") can be connected to the master controller (which is called "pied"), via a common bus (see Wiring).
The master controller can be connected to a master computer, for example trough USB link.

Fraise rate is currently fixed to **250 kbits/sec**.

Communication is based on (1 start bit + 9 bits + 1 stop bit) words ; if bit 9 is set, the 8 lower bits are considered to be an address byte, or ID, which is used to select one of the microcontrollers. ID are comprised between 1 and 126. ID 127 is reserved for future use. Words with bit 9 cleared are data bytes, which are only read by the previously addressed device.

Transfers occur by packets of 31 effective data bytes maximum. Packets can be of two types : raw bytes or characters string.

Each device stores an **ID** (8 bit) and a **NAME** (max 16 char string) in eeprom. Devices connected to the same bus are not allowed to share the same ID or NAME.

In order to be renamed, a device has to be connected alone to the bus.  
For any other operation, all devices can be connected together to the bus.

A device is assigned to an ID by using its NAME to address it.
The NAME of a device is also used in bootloader mode, for flashing a new application firmware.

Normal communications use ID to access the device.


Master-to-device communication :
------------------------------

### normal output :

Master starts sending device ID byte with bit 9 on. Then a length byte (with possibly bit 8 set to signal a string packet) is appended. Then the data bytes, then a checksum byte. Device acknowledges the packet by sending a ’0’ byte.

(# means next character has bit 9 set)

```
packet : #NLD(...)DS
- #N = device ID + bit9
- L = 128*c + l : bit8 = c = packet is character string, l = number of following data bytes (31 max).
- D(...)D = l data bytes.
- S = -sum (modulo 256) of all bytes before S (N+L+D+...+D).
```

Device N then has 1 ms max to return ’0’ to acknowledge the packet, or ’1’ to signal a checksum error.


### broadcast output :

If ID is 0, then all the devices decode the packet ; no device has to acknowledge.

Special broadcast commands are provided to assign a device to an ID, or to establish a 8 bits communication for bootloader purpose. First data byte is the broadcast command :

- `’I’` = all reInit
- `’F’ + NAME` = device NAME jump to Fraise bootloader ; alternatively, device powers on in bootloader mode, and jumps to application after 4 seconds or as soon as it receives a byte with bit 9 set.
- `’N’ + ID + NAME` = assign device NAME to an ID Number (1 to 127) ; here ID is written in hexadecimal ascii e.g : "N10Fruit" -> device called "Fruit" is assigned to ID = 16
- `’B’` = broadcast char string
- `’b’` = broadcast raw

'B' and 'b' broadcast messages can be used by applications (e.g global clock).


Device-to-master communication :
------------------------------

Devices are polled sequentially, between transmissions of master-to-device packets.
Master initiates transmission :
The first byte is the ID of tested device , with bit 9 set to signal a start byte, and bit 8 set to signal a poll message. Then ID + 128 is repeated with bit 9 cleared, in order to secure addressing.
If the device has nothing to say it returns quickly, answering a ’0’. Either it sends a packet which is acknowledged by master.


(# means next character has bit 9 set)

### polling for input :

```
packet : #MM
- #M = device ID + bit8(128) + bit9 (M = #M - bit9)
```

device ID must answer before 1 ms.


### device answer :

```
packet : LD(...)DS
- L = 128*c + l : bit8 = c = packet is character string, l = number of following data bytes (31 max).
- D(...)D = l data bytes.
- S = -sum (modulo 256) of all bytes before S (L+D+...+D).
```

Master then returns ’0’ to acknowledge the packet, or ’1’ to signal a checksum error.


### device nop answer :

```
packet : 0 (one null byte)
```
No checksum byte in this case, nor master acknowledge.


Bootloder communication :
-----------------------

Bit 9 is always cleared.
Bootloader packets start with the number of following bytes (length byte). The last byte is the checksum (the sum of all bytes including length and checksum, modulo 256, must be zero).
The first byte after the length byte indicates the command :

- `’R’` = rename command : message must be "RENAME:" + NAME ; device answers ’R’.
- `’V’ + NAME` = verify that we are talking to the bootloader of device NAME ; 
                 device must answer ’V’.
- `’:’` = first character of an hexfile line ; at the end of the line, device answers :
  - `’X’` to acknowledge the line,
  - `’Y’` if the end of the hexfile has been successfully reached,
  - `’z’` for bad line checksum error,
  - `’u’` for unsupportded hex command, or
  - `’l’` for bad hex line format.
- `’A’` = goto Application (quit bootloader).

After 100ms without receiving anything from master, device discards waiting received bytes, and next received byte will start a new line.

Device initially boots to bootloader mode ; bootloader jumps to application :
- if it receives a ’A’
- if it receives an non-zero address byte (bit 9 on)
- 4 seconds after power-on without having been verified.


-------------------------------------------------------------------------------------

Fraise USB Protocol :
-------------------

Master is USB connected to the host computer.

(here * means byte has bit 9 set)

### output to device :

`"0100\n" -> *0x01 0x01 0x00 0xFE` : send 0 to device ID 1.  
`"81Hi\n" -> *0x01 0x82 0x48 0x69 0xCC` : send "Hi" to device ID 1.

Master answers to host `"sTnn\n"` if device nn didn’t answer (timeout), or `"sann\n"` if it refused to acknowledge the packet (packet error or device buffer full).


### broadcast output :

`"!BI\n" -> *0x00 0x82 0x42 0x49 0xF3` : send "I" to all devices.  
`"!b00\n" -> *0x00 0x01 0x00 0xFF` : send 0 to all devices.


### polling for input :

`"#S04\n"` : set polling for device ID 4  
`"#C04\n"` : clear polling for device ID 4

polling: with 0xmm = (0xnn | 0x80), periodically send *0xmm 0xmm, then wait for answer from device nn.

When device nn answers for the first time, print to usb : `"sCnn\n"` (device nn is connected).
When device nn stops answering, print to usb : `"scnn\n"`.

If master received a corrupted packet from device nn (checksum error), it signals this error by `"sxnn\n"`.

When master receives a packet from device nn, it transmits it to host prefixed with "NN", where NN equals to nn plus 0x80 if message is a character string.


### control :

`"!N04Fruit1\n"` : assign to fruit called "Fruit1" ID 04.  
`"#i\n"` : reinit Fraise master (clears polling for all devices).


### bootloader mode :

`"!FFruit1\n"` : ask "Fruit1" to jump to bootloader.

Additionnaly, master switchs to bootloder mode : then it transmits directly every line (’\n’ terminated, must not begin with "#" or "!") from host to the bus, then listens to the bus for any answer and puts it back to host.

`"RENAME:Fruit1\n"` : rename ALL (!!!) connected devices being in bootloader mode to "Fruit1" (so only one device should be connected before doing this).  
`"A\n"` : run Application (==quit bootloader)  
`"#F\n"` : ask master to quit bootloder mode


------------------
LICENSE : CC-BY-ND
This document is placed under the terms of the Creative Commons Attribution-NoDerivatives 4.0 International Public License
http://creativecommons.org/licenses/by-nd/4.0/legalcode
