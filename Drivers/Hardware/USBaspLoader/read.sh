#!/bin/sh
avrdude -p atmega328p -c usbasp -P usb -U flash:r:flash.hex:i

