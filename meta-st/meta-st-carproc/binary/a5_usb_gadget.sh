#!/bin/bash
# Copyright (C) Julien COUVRAND <julien.couvrand@st.com>
# Copyright (C) 2016 Julien COUVRAND <julien.couvrand@st.com>
#
# Released under the terms of the GNU GPL
#
# It is used to create synopsis acm gadget.
# This script depends on USB CONFIGFS feature.
# More informations with http://lxr.free-electrons.com/source/Documentation/usb/gadget_configfs.txt

# Configuration inputs

configfs=/sys/kernel/config/usb_gadget
lang=0x409
conf=c.1

#-------------------------------------------------------------
# usage function
#-------------------------------------------------------------
usage() {
        echo "usage: $(basename $0) [-h] -d <udc name> <gadget class> <enable|disable>"
        echo ""
        echo "    -h                  print this help"

        echo "    -d  <udc name>      specify the USB gadget to be used and to enable"
        echo "                        where <udc name> is one of those found in /sys/class/udc/"
        echo "        <gadget class>  specify the gadget class such as:"
        echo "                        ncm, acm, ecm, uac2, mass_storage, rndis, multi, multiaudioecm"

        echo ""
        echo "Operation mode:"
        echo "    enable:             enable the gadget"
        echo "    disable:            disable the gadget"
        echo "    usage:              help about command"
}


#-------------------------------------------------------------
# Check UDC command function
#-------------------------------------------------------------
udc_cmd() {
        res=0
        mode=0
        usb_id=0
        #echo -e "Gadgets: setting up class "
        for res in $( ls /sys/class/udc)
        do
                echo -e "$res"
                if [ "$1" = "$res" ];
                then
                        serN=$1
                        return
                fi
        done
        echo "$1: unsupported value"
        usb=$(ls /sys/class/udc)
        echo -e "USB device gadget available: $usb"
        exit 0
}


#-------------------------------------------------------------
# Manage gadget configuration
#-------------------------------------------------------------
gadget_config() {
        idVendor="0x0525"
        firm="Linux 4.1.13 dwc_otg_hcd"
        name="DWC OTG Controller"
        bmAttributes="0xc0"
        MaxPower="2"
        bcdUSB="0x0200"
        bMaxPkt="0x40"
        bDevPro="0x00"
        bDevSubCl="0x00"
        bDevCl="0x02"

        # Probe and mount functions
        modprobe libcomposite

        mount -t configfs none /sys/kernel/config
        cd $configfs

        # Creating the gadgets
        mkdir -p $gadget
        cd $gadget
        echo $idProduct > idProduct
        echo $idVendor > idVendor
        echo $bcdUSB > bcdUSB
        echo $bMaxPkt > bMaxPacketSize0
        echo $bDevPro > bDeviceProtocol
        echo $bDevSubCl > bDeviceSubClass
        echo $bDevCl > bDeviceClass

        mkdir -p strings/$lang
        echo $serN > strings/$lang/serialnumber
        echo $firm > strings/$lang/manufacturer
        echo $name > strings/$lang/product

        # Creating the configurations and the functions
        mkdir -p configs/$conf
        echo $bmAttributes > configs/$conf/bmAttributes
        echo $MaxPower > configs/$conf/MaxPower
        mkdir -p configs/$conf/strings/$lang


        case "$class" in
                multi)
                  mkdir -p functions/acm.usb0
                  mkdir -p functions/ecm.usb0
                  mkdir -p functions/ncm.usb0
                  mkdir -p functions/mass_storage.usb0
                  echo "$class Gadget" > configs/$conf/strings/$lang/configuration
                  ln -s functions/acm.usb0 configs/$conf
                  ln -s functions/ecm.usb0 configs/$conf
                  ln -s functions/ncm.usb0 configs/$conf
                  ln -s functions/mass_storage.usb0 configs/$conf
                  echo $file > functions/mass_storage.usb0/lun.0/file
                  ;;
                multiaudioecm)
                  mkdir -p functions/ecm.usb0
                  mkdir -p functions/uac2.usb0
                  echo "$class Gadget" > configs/$conf/strings/$lang/configuration
                  ln -s functions/ecm.usb0 configs/$conf
                  ln -s functions/uac2.usb0 configs/$conf
                  ;;
                *)
                  mkdir -p functions/$class.usb0
                  echo "Basic $class" > configs/$conf/strings/$lang/configuration
                  ln -s functions/$class.usb0 configs/$conf
                  ;;
        esac


        if [ "$class" = "mass_storage" ] || [ "$class" = "multi" ] 
        then
                echo $file > functions/mass_storage.usb0/lun.0/file
        fi
}


#-------------------------------------------------------------
# Fill configuration following USB gadget class
#-------------------------------------------------------------
class_config() {
        class=$1

        # To complete following tests
        case "$class" in
                acm)
                        idProduct="0xa4a7"
                        ;;
                ecm)
                        idProduct="0xa4a2"
                        ;;
                ncm)
                        idProduct="0xa4a1"
                        ;;
                mass_storage)
                        idProduct="0xa4a5"
                        file="/dev/mmcblk0p1"
                        ;;
                rndis)
                        idProduct="0xa4a2"
                        ;;
                multi)
                        idProduct="0xa4a6"
                        file="/dev/mmcblk0p1"
                        ;;
                uac2)
                        idProduct="0xa4a4"
                        ;;
                multiaudioecm)
                        idProduct="0xa4a7"
                        ;;
                        
                *)
                        echo -e "Wrong gadget class"
                        echo -e "Supported classes are:"
                        echo "ncm, acm, ecm, uac2, mass_storage, rndis, multi, multiaudioecm"
                        exit 0
                        ;;
        esac
}

#-------------------------------------------------------------
# clean function
#-------------------------------------------------------------
clean() {

        cd $configfs

        # Cleaning following classes
        case "$class" in
                acm | ecm | ncm | mass_storage | rndis | uac2)
                        rm $gadget/configs/$conf/$class.usb0
                        rmdir $gadget/configs/$conf/strings/$lang
                        rmdir $gadget/configs/$conf
                        rmdir $gadget/functions/$class.usb0
                        rmdir $gadget/strings/$lang
                        rmdir $gadget
                        ;;
                multi)
                        rm $gadget/configs/$conf/acm.usb0
                        rm $gadget/configs/$conf/ecm.usb0
                        rm $gadget/configs/$conf/ncm.usb0
                        rm $gadget/configs/$conf/mass_storage.usb0
                        rmdir $gadget/configs/$conf/strings/$lang
                        rmdir $gadget/configs/$conf
                        rmdir $gadget/functions/acm.usb0
                        rmdir $gadget/functions/ecm.usb0
                        rmdir $gadget/functions/ncm.usb0
                        rmdir $gadget/functions/mass_storage.usb0
                        rmdir $gadget/strings/$lang
                        rmdir $gadget
                        ;;
                multiaudioecm)
                        rm $gadget/configs/$conf/uac2.usb0
                        rm $gadget/configs/$conf/ecm.usb0
                        rmdir $gadget/configs/$conf/strings/$lang
                        rmdir $gadget/configs/$conf
                        rmdir $gadget/functions/uac2.usb0
                        rmdir $gadget/functions/ecm.usb0
                        rmdir $gadget/strings/$lang
                        rmdir $gadget
                        ;;
                        
                *)
                        echo "Invalid class"
                        ;;
        esac
}

#-------------------------------------------------------------
# disable function
#-------------------------------------------------------------
_disable() {

        # Disabling the gadget
        echo -e "Disable gadget"
        for file in "$configfs"/*; do
                echo -e "$file"
                if [ "$file" = "$configfs/$gadget" ];
                then
                        echo "" > $configfs/$gadget/UDC
                        # Cleaning up
                        clean
                fi
        done
}


#-------------------------------------------------------------
# enable function
#-------------------------------------------------------------
_enable() {

        # Configuring the gadget
        gadget_config

        # Enabling the gadget
        echo -e "Enable gadget"
        echo $serN > $configfs/$gadget/UDC
}


#----------------------------------
# main
#----------------------------------
trap _disable SIGHUP SIGINT SIGTERM

# Parse command line options.
cnt=$#

if [ $cnt -eq 0 ];
then
        usage
        exit 255
fi

option=""

# optional parameters check
while [ $# -gt 0 ];
do
        case "$1" in
                -h)
                        usage
                        exit 0
                        ;;
                -d)
                        shift
                        udc_cmd $1
                        shift
                        class_config $1
                        ;;
                *)
                        # being parsed in the next switch/case
                        option=$1
                        ;;
        esac
        shift
done

# mandatory parameters
case "$option" in
        enable)
                gadget=$serN
                _enable $serN
                ;;
        disable)
                gadget=$serN
                _disable
                ;;
        *)
                usage
                exit 255
esac
