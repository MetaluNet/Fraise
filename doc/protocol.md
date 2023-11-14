
Fraise Protocol v 2.1.2
=====================
*(Antoine Rousseau 2013-2023)*

**Fraise** is a half-duplex asynchronous serial communication protocol.

Up to 126 devices (called **fruits**) can be connected to the master controller (which is called **pied**), via a common bus.
The master controller is connected to a computer, usually via USB.

The bitrate of the Fraise bus is currently fixed to **250 kbits/sec**.

Communication is based on (1 start bit + 9 bits + 1 stop bit) words;
if bit 9 is set, the 8 lower bits are considered to be an address byte, or ID, which is used to select one of the microcontrollers.
ID are comprised between 1 and 126. ID 127 is reserved for future use.
Words with bit 9 cleared are data bytes, which are only read by the previously addressed **fruit**.

Transfers occur by packets of up to 31 effective data bytes. Packets can be of two types: raw bytes or characters string.

Each **fruit** stores an **ID** (8 bit) and a **NAME** (max 16 char string) in eeprom.
Devices connected to the same bus are not allowed to share the same ID or NAME.

In order to be renamed, a **fruit** should be connected alone to the bus.  
For any other operation, all **fruits** can be connected together to the bus.

A **fruit** is assigned to an ID using its NAME to address it.
The NAME of a **fruit** is also used in bootloader mode, for flashing a new application firmware.

Normal communications use ID to access the **fruit**.


Pied-to-fruit communication
------------------------------

### normal output

The **pied** initiates transmission by sending the **fruit** ID byte with bit 9 on, followed by
a length byte (with possibly bit 8 set to signal a string packet).
The message continues with the data bytes, then ends with a checksum byte.
The **fruit** acknowledges the packet by sending a ’0’ byte.

(in this chapter, # means next character has bit 9 set)

```
packet: #NLD(...)DS
- #N = fruit ID + bit9
- L = 128*c + l: bit8 = c = packet is character string, l = number of following data bytes (31 max).
- D(...)D = l data bytes.
- S = -sum (modulo 256) of all bytes before S (N+L+D+...+D).
```

**Fruit** number N must acknowledge the packet returning `0` in less than 1 millisecond, or returns `1` to signal a checksum error.


### broadcast output

If ID is 0, then all the **fruits** decode the packet; no **fruit** has to acknowledge.

Special broadcast commands are provided to assign a **fruit** to an ID, or to establish a 8 bits communication for bootloader purpose.
The first data byte is the broadcast command:

- `’I’` = all reInit
- `’F’ + NAME` = tell the **fruit** called NAME to switch to bootloader.
Note: the **fruit** always boots in bootloader mode, and jumps to application after 1 second or as soon as it receives a byte with bit 9 set.
- `’N’ + ID + NAME` = assign **fruit** NAME to an ID Number (1 to 127);
here ID is written in hexadecimal ascii e.g: "N10Fruit" -> **fruit** called "Fruit" is given ID = 16.
- `’B’` = broadcast string
- `’b’` = broadcast raw bytes

'B' and 'b' broadcast messages can be used by the user application (e.g to send a global clock).


Fruit-to-pied communication
------------------------------

**Fruits** are polled sequentially, between transmissions of pied-to-fruits packets.
The **pied** initiates transmission:
The first byte is the ID of tested **fruit**, with bit 9 set to signal a start byte, and bit 8 set to signal a poll message. Then ID + 128 is repeated with bit 9 cleared, in order to secure addressing.
If the **fruit** has nothing to transmit, it returns ’0’. Otherwise it sends a packet, which is acknowledged by the **pied**.


(# means next character has bit 9 set)

### polling for input

```
packet: #MM
- #M = fruit ID + bit8(128) + bit9 (M = #M - bit9)
```

**Fruit** number ID must answer before 1 ms.


### fruit answer

```
packet: LD(...)DS
- L = 128 * c + l: bit8 = c = packet is character string, l = number of following data bytes (31 max).
- D(...)D = l data bytes.
- S = -sum (modulo 256) of all bytes before S (L + D + ... + D).
```

The **pied** then returns ’0’ to acknowledge the packet, or ’1’ to signal a checksum error.


### fruit nop answer

```
packet: 0 (one null byte)
```
No checksum byte in this case, nor acknowledge from the **pied**.


Bootloder communication
-----------------------

Bit 9 is always cleared.
Bootloader packets start with the number of following bytes (length byte).
The last byte is the checksum (the sum of all bytes including length and checksum, modulo 256, must be zero).
The first byte after the length byte indicates the command:

- `’R’` = rename command: message must be "RENAME:" + NAME; **fruit** answers `’R’`.
- `’V’ + NAME` = verify that we are talking to the bootloader of the **fruit** called NAME; 
the **fruit** must answer `’V’` (additionnaly, the **fruit** now locks in bootloader mode).
- `’:’` = first character of an hexfile line; at the end of the line, the **fruit** answers:
  - `’X’` to acknowledge the line,
  - `’Y’` if the end of the hexfile has been successfully reached,
  - `’z’` for bad line checksum error,
  - `’u’` for unsupportded hex command, or
  - `’l’` for bad hex line format.
- `’A’` = goto Application (quit bootloader).

After 100ms without receiving anything from the **pied**, the **fruit** discards waiting received bytes, and next received byte will start a new line.

The **fruit** initially boots to bootloader mode; bootloader switches to application:
- when it receives a ’A’
- or when it receives an non-zero address byte (bit 9 on)
- or 1 second after power-up if it has not been verified.

-------------------------------------------------------------------------------------

Fraise USB Protocol
-------------------

The **pied** is connected to the host computer via USB.

(here * means byte has bit 9 set)

### output to fruit

`"0100\n" -> *0x01 0x01 0x00 0xFE`: send 0 to **fruit** ID 1.  
`"81Hi\n" -> *0x01 0x82 0x48 0x69 0xCC`: send "Hi" to **fruit** ID 1.

The **pied** answers to host `"sTnn\n"` if **fruit** nn didn’t answer (timeout),
or `"sann\n"` if it refused to acknowledge the packet (packet error or buffer full).


### broadcast output

`"!BI\n" -> *0x00 0x82 0x42 0x49 0xF3`: send "I" to all **fruits**.  
`"!b00\n" -> *0x00 0x01 0x00 0xFF`: send 0 to all **fruits**.


### polling for input

`"#S04\n"`: set polling for **fruit** ID 4  
`"#C04\n"`: clear polling for **fruit** ID 4

polling: with 0xmm = (0xnn | 0x80), periodically send *0xmm 0xmm, then wait for answer from **fruit** nn.

When **fruit** nn answers for the first time, print to USB: `"sCnn\n"` (**fruit** nn is connected).
When **fruit** nn stops answering, print to USB: `"scnn\n"`.

If the **pied** received a corrupted packet from **fruit** nn (checksum error), it signals this error by `"sxnn\n"`.

When the **pied** receives a packet from **fruit** nn, it transmits it to host prefixed with "NN", where NN equals to nn, plus 0x80 if message is a character string.


### control

`"!N04Fruit1\n"`: assign to fruit called "Fruit1" ID 04.  
`"#i\n"`: reinit the **pied** (clears polling for all **fruits**).  
`"#V\n"`: query the firmware version of the **pied**, which answers (currently) `"#V UsbFraise 2.1.6 (SpareTimeLabs/SDCC) A.Rousseau 2023\n"`  
`"#R\n"`: query the piedID (usually 1), the **pied** answers `"#ID01\n"`  
`"#UNLOCK\n"`: allow setting the piedID (which must be the next command)  
`"#W02\n"`: set the piedID to 2

The piedID allows to distinguish several pieds connected to several USB ports.


### bootloader mode

`"!FFruit1\n"`: ask "Fruit1" to jump to bootloader.

Additionnaly, the **pied** switches to bootloading mode:  
it transmits directly every line (’\n’ terminated, must not begin with "#" or "!") from host to the bus,
then listens to the bus for any answer from the **fruit**, and puts this answer back to host, adding the 'b' character at the beginning of each line.

`"RENAME:Fruit1\n"`: rename "Fruit1" ALL (!!!) connected **fruits** being in bootloader mode (that's why a single **fruit** should be connected before doing this).  
`"A\n"`: ask the **fruit** to run Application (quit bootloader mode)  
`"#F\n"`: ask the **pied** to quit bootloading mode


### logging

The **pied** can send informational messages to the USB host, for debugging purpose. Each log line if prefixed with the 'l' character.

------------------
LICENSE: CC-BY-ND
This document is placed under the terms of the Creative Commons Attribution-NoDerivatives 4.0 International Public License
http://creativecommons.org/licenses/by-nd/4.0/legalcode
