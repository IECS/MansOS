#!/bin/sh

USBPORT=/dev/ttyUSB0

echo Listing all motes...
echo ls | ./shell $USBPORT
echo " "
