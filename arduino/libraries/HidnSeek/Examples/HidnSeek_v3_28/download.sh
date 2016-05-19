#!/bin/sh
avrdude -p atmega328p -c usbasp -P usb -U flash:w:HidnSeek_v3_28.hex:i
while [ $? != 0 ]
do
  sleep 5
  avrdude -p atmega328p -c usbasp -P usb -U flash:w:HidnSeek_v3_28.hex:i
done
