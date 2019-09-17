#!/bin/bash
case $1 in
    suspend)
	echo 0 > /sys/devices/platform/soc/48440000.usb-phy/usb_id;
	echo OFF > /sys/devices/platform/soc/48440000.usb-phy/usb_drvvbus;
	echo 1 > /sys/devices/platform/soc/48440000.usb-phy/usb_id;
	echo OFF > /sys/devices/platform/soc/48440000.usb-phy/usb_drvvbus
    ;;
    resume)
    ;;
    *)
    ;;
esac

