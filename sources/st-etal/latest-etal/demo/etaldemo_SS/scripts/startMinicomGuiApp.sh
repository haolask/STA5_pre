#!/bin/bash

if [ -f ~/tmp/etalCtrlAppFifo ]; then
	rm ~/tmp/etalCtrlAppFifo
fi
mkfifo ~/tmp/etalCtrlAppFifo
perl etalCtrlApp.pl > ~/tmp/etalCtrlAppFifo &
minicom -D /dev/ttyUSB0 -b 115200 -C etallogttyusb.txt -w < ~/tmp/etalCtrlAppFifo
