#!/bin/bash
#
#
#

#-------------------------------------------------------------
# usage function
#-------------------------------------------------------------
usage() {
        echo "usage: usb_change_mode [-u usb port] [-d] [-h]"
        echo ""
        echo "    -u      <usbPort>   		specify the value of the port index - 0 or 1 "
        echo ""
        echo "    -d                        will pass the specified usb port on DEVICE mode"
        echo ""
        echo "    -h                        will pass the specified usb port on HOST mode"
        echo ""
}

#----------------------------------
# main
#----------------------------------
usbMode="NONE"
udc=""

while getopts ":dhu:" option
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
                                        echo " Change the USB MODE of the USB PORT : $usbPort"
						if [ $OPTARG -eq 1 ]
						then
							udc="48500000.usb"
						else
							udc="48400000.usb"
						fi
                                fi

                        else
                                echo " ERROR - No USB Port specified !! "
                                usage
                                exit 255
                        fi
                        ;;
                d)
                        if [ "$usbMode" != "NONE" ]
                        then
                                echo " ERROR - USB Mode already setted !!"
                                usage
                                exit 255
                        else
                                usbMode="DEVICE"
                                echo " Change the mode to $usbMode mode "
                        fi
                        ;;

                h)
                        if [ "$usbMode" != "NONE" ]
                        then
                                echo " ERROR - USB Mode already setted !!"
                                usage
                                exit 255
                        else
                                usbMode="HOST"
                                echo " Change the mode to $usbMode mode "
                        fi
                        ;;
                *)
            echo "! -$option n'est pas un argument valide"
            usage
            exit 255
            ;;
        esac
done

if [ "$usbMode" != "NONE" ]
then
	if [ "$usbMode" == "HOST" ]
	then
		echo disconnect > /sys/devices/platform/soc/$udc/udc/$udc/soft_connect
		sleep 1
	fi

        echo $usbPort > /sys/devices/platform/soc/48440000.usb-phy/usb_id
        echo $usbMode > /sys/devices/platform/soc/48440000.usb-phy/usb_mode

	if [ "$usbMode" == "DEVICE" ]
	then
		echo connect > /sys/devices/platform/soc/$udc/udc/$udc/soft_connect
		sleep 1
	fi

else
        echo " ERROR - USB MODE not specified !! "
        usage
        exit 255
fi
