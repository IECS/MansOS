#!/bin/sh

USBPORT=/dev/ttyUSB1

echo Listing all motes...
echo ls | ./shell $USBPORT
echo " "

echo Getting red LED status...
echo get 1.3.0 | ./shell $USBPORT
echo " "

echo Turning red LED on...
echo set 1.3.0 o 1 | ./shell $USBPORT
echo " "

echo Getting red LED status again...
echo get 1.3.0 | ./shell $USBPORT
echo " "

echo Turning red LED off...
echo set 1.3.0 o 0 | ./shell $USBPORT
echo " "