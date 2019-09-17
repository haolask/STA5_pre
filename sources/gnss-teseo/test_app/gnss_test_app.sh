#!/bin/sh

#$1 : set value [1-7]. Leave blank for selection.
APP_CHOICE=$1
#$2 should be [y] or [n]. Leave blank for selection.
DR_CHOICE=$2

echo "Choose application to compile:"
echo "1 - gnss_app_client (w/wo DR)    : Acquire GNSS position from NMEA flow on TCP/IP"
echo "2 - gnss_standalone_read         : Read flow from UART in GNSS standalone mode"
echo "3 - gnss_socket_client (w/wo DR) : Read flow from socket and save into log file in File System"
echo "4 - gnss_socket_redirect         : Redirect flow from UART to socket"
echo "5 - gnss_redirect_to_usb         : Redirect flow from Teseo UART to USB Serial"
echo "6 - gnss_uart_download           : Flash Teseo binary image with external Loader via Host UART"
echo "7 - gnss_teseo3_flasher          : Flash Teseo3 binary image internally"
echo "8 - gnss_teseo5_flasher          : Flash Teseo5 binary image internally"


while true; do
    if [ "$APP_CHOICE" = "" ]
    then
    read -p "Choice? " APP_CHOICE
    else
    echo "Selected $APP_CHOICE"
    fi

    case $APP_CHOICE in
      [1] ) AppliName="gnss_app_client" ; break;;
      [2] ) AppliName="gnss_standalone_read"; break;;
      [3] ) AppliName="gnss_socket_client"  ; break;;
      [4] ) AppliName="gnss_socket_redirect"; break;;
      [5] ) AppliName="gnss_redirect_to_usb"; break;;
      [6] ) AppliName="gnss_uart_download"; break;;
      [7] ) AppliName="gnss_teseo3_flasher"; break;;
      [8] ) AppliName="gnss_teseo5_flasher"; break;;
      * ) echo "Wrong choice, try again."; APP_CHOICE="";;
    esac
done


if [ "${AppliName}" = "gnss_app_client" ] || [ "${AppliName}" = "gnss_socket_client" ]
then
echo ""
echo "DR enabled:[y/n]"
while true; do
    if [ "$DR_CHOICE" = "" ]
    then
    read -p "Choice? " DR_CHOICE
    else
    echo "Selected $DR_CHOICE"
    fi

    case $DR_CHOICE in
      [y] ) DR_OPTIONS="-DDR_CODE_LINKED" ; break;;
      [n] ) DR_OPTIONS=""  ; break;;
      * ) echo "DR enabled:[y/n]"; DR_CHOICE="";;
    esac
done
fi

if [ "${AppliName}" = "gnss_redirect_to_usb" ]
then
  LANGUAGE="c++"
else
  LANGUAGE="c"
fi

echo ""
echo "START COMPILATION"
echo "Generate application \"${AppliName}.bin\""

if [ "${ST_OE_BUILD_DIR}" != "" ]
then
echo "YOCTO environment detected"
GNSS_BB_PATH=$ST_OE_ROOT_DIR/meta-st/meta-st-carproc/recipes-bsp/gnss-teseo
GNSS_BB_FILE=gnss-teseo_*-st.bb
GNSS_VERSION=`cd $GNSS_BB_PATH; ls $GNSS_BB_FILE | cut -d"_" -f2 | cut -d"-" -f1`
release=`grep "PR =" $GNSS_BB_PATH/$GNSS_BB_FILE | cut -d" " -f3 | sed -e 's#"##g'`
linuxpath=`echo ${ST_OE_BUILD_DIR}/tmp/work/*-poky-linux-gnueabi/gnss-teseo/${GNSS_VERSION}-st-${release}/recipe-sysroot`
GCC_DIR=${linuxpath}-native/usr/bin/arm-poky-linux-gnueabi
GCC_OPTIONS="-march=armv7ve -marm -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a7"
LDFLAGS="-Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed"
CFLAGS="-O2 -pipe -feliminate-unused-debug-types"
CppFLAGS=""

case "$LANGUAGE" in
  "c")
    echo ${GCC_DIR}/arm-poky-linux-gnueabi-gcc ${LDFLAGS} ${CFLAGS} ${GCC_OPTIONS} ${DR_OPTIONS} --sysroot=${linuxpath} -I${linuxpath}/usr/include ${AppliName}.c -o ${AppliName}.bin -lpthread
    ${GCC_DIR}/arm-poky-linux-gnueabi-gcc ${LDFLAGS} ${CFLAGS} ${GCC_OPTIONS} ${DR_OPTIONS} --sysroot=${linuxpath} -I${linuxpath}/usr/include ${AppliName}.c -o ${AppliName}.bin -lpthread
    RET_VAL=$?
    if [ $RET_VAL != 0 ]; then echo "!! Compilation error $RET_VAL !!"; exit $RET_VAL; fi
    ;;
  "c++")
    echo ${GCC_DIR}/arm-poky-linux-gnueabi-g++ ${LDFLAGS} ${CppFLAGS} ${GCC_OPTIONS} ${DR_OPTIONS} --sysroot=${linuxpath} -I${linuxpath}/usr/include ${AppliName}.cpp -o ${AppliName}.bin -lpthread
    ${GCC_DIR}/arm-poky-linux-gnueabi-g++ ${LDFLAGS} ${CppFLAGS} ${GCC_OPTIONS} ${DR_OPTIONS} --sysroot=${linuxpath} -I${linuxpath}/usr/include ${AppliName}.cpp -o ${AppliName}.bin -lpthread
    RET_VAL=$?
    if [ $RET_VAL != 0 ]; then echo "!! Compilation error $RET_VAL !!"; exit $RET_VAL; fi
    ;;
  *)
    echo "ERROR: Unsupported language $LANGUAGE"
    exit 100
    ;;
esac

else

if [ "${A2_ROOT_VIEW}" != "" ]
then
echo "Buildroot environment detected"

case "$LANGUAGE" in
  "c")
    ${A2_ROOT_VIEW}/buildroot/output/host/usr/bin/arm-v7-linux-uclibceabi-gcc -mlittle-endian -mfloat-abi=soft ${DR_OPTIONS} ${AppliName}.c -o ${AppliName}.bin -lpthread
    RET_VAL=$?
    if [ $RET_VAL != 0 ]; then echo "!! Compilation error $RET_VAL !!"; exit $RET_VAL; fi
    ;;

  "c++")
    ${A2_ROOT_VIEW}/buildroot/output/host/usr/bin/arm-v7-linux-uclibceabi-g++ -std=c++14 -mlittle-endian -mfloat-abi=soft ${DR_OPTIONS} ${AppliName}.cpp -o ${AppliName}.bin -lpthread
    RET_VAL=$?
    if [ $RET_VAL != 0 ]; then echo "!! Compilation error $RET_VAL !!"; exit $RET_VAL; fi
    ;;

  *)
    echo "ERROR: Unsupported language $LANGUAGE"
    exit 100
    ;;
esac

else

echo
echo "Error !!"
echo "Your environment is not set properly."
echo "Please use :"
echo "   command \"source a2_env_setup.sh\" for Buildroot environment"
echo "   command \"source envsetup.sh\" for Yocto environment"
echo
exit
fi

fi

gnssenv_test=$(echo $PWD | grep hbp)
if [ "${gnssenv_test}" != "" ]
then
#GNSS HBP environment detected
binpath=$(echo $PWD | awk '{number=split($0,text,/\/hbp\//); printf("%s", text[1]); exit}'  | cut -d\" -f1)/bin
else
#GNSS Standalone environment detected
binpath=../bin
fi

mv ${AppliName}.bin ${binpath}/${AppliName}.bin

echo " -- New application ${binpath}/${AppliName}.bin is available --"
echo "END"

