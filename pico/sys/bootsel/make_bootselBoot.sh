#!/binsh

cc bootselBoot.c -o bootselBoot -I/usr/include/libusb-1.0 -L/usr/lib/arm-linux-gnueabihf -lusb-1.0
