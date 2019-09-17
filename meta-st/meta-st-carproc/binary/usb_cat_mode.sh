#!/bin/bash
#
#
#

#-------------------------------------------------------------
# usage function
#-------------------------------------------------------------
usage() {
        echo "usage: usb_cat_mode [-u usb port]"
        echo ""
        echo "    -u      <usbPort>             specify the value of the port index - 0 or 1 "
        echo ""
        echo ""
}

#----------------------------------
# main
#----------------------------------
while getopts ":u:" option
do
        case $option in
                u)
                        if [ -n $OPTARG ]
                        then
                                if [ $OPTARG -gt '1' ]
                                then
                                        echo " ERROR - The specified usb port doesn't existe : $usbPort !! "
                                        usage
                                        exit 255
                                else
                                        usbPort="$OPTARG"
                                fi

                        else
                                echo " ERROR - No USB Port specified !! "
                                usage
                                exit 255
                        fi
                        ;;

                *)
            echo "! -$option n'est pas un argument valide"
            usage
            exit 255
            ;;
        esac
done

echo $usbPort > /sys/devices/platform/soc/48440000.usb-phy/usb_id
echo  USB:$usbPort 
cat /sys/devices/platform/soc/48440000.usb-phy/usb_mode
