#!/bin/sh

avr-gcc -Os -DF_CPU=16000000UL -mmcu=atmega328p -c -o ctrl.o src/main.c
avr-gcc -mmcu=atmega328p ctrl.o -o ctrl
avr-objcopy -O ihex -R .eeprom ctrl ctrl.hex
avrdude -F -V -c arduino -p ATMEGA328P -P /dev/tty.usbserial-00002014 -b 115200 -U flash:w:ctrl.hex
