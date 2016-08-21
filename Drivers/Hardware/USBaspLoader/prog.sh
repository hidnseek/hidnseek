#!/bin/sh
FILE=$1
if [ ! -f $FILE ] || [ $# -eq 0 ]; then
  FILE="main.hex"
  avrdude -p atmega328p -c usbasp -P usb -e -Ulock:w:0x3F:m -Uefuse:w:0x05:m -Uhfuse:w:0xd8:m -Ulfuse:w:0xff:m
  avrdude -p atmega328p -c usbasp -P usb -U flash:w:$FILE:i -Ulock:w:0x0F:m
else
  avrdude -p atmega328p -c usbasp -P usb -U flash:w:$FILE:i
fi
