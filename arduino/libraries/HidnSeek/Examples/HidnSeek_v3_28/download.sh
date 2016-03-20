#!/bin/sh
avrdude -p atmega328p -c usbasp -P usb -U flash:w:HidnSeek_v3_28.hex:i
