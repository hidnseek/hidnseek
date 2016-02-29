#!/bin/sh
avrdude -p atmega328p -c usbasp -P usb -e -Ulock:w:0x3F:m -Uefuse:w:0x05:m -Uhfuse:w:0xd8:m -Ulfuse:w:0xff:m
avrdude -p atmega328p -c usbasp -P usb -U flash:w:main.hex:i -Ulock:w:0x0F:m

