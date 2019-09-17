#!/bin/sh
#
BOOT_FILE_TMP=./boot.h.tmp

echo "starting bin transform " $# $1

#get parameters
if [ $# -eq 0 ]
then
    echo "usage : bin_process.sh <boot.h>  \n" 
    exit 1
else
    BOOT_FILE_INPUT=$1
    BOOT_FILE_ROOTS=$(echo "$1" | cut -f 1 -d '.')
    BOOT_FILE_HEX_OUTPUT="$BOOT_FILE_ROOTS"".boot.hex"
    BOOT_FILE_BIN_OUTPUT="$BOOT_FILE_ROOTS"".boot.bin"
fi

echo "input = " $BOOT_FILE_INPUT "; output_hex =  " $BOOT_FILE_HEX_OUTPUT "; output_bin = "$BOOT_FILE_BIN_OUTPUT

#remove the C comments from the file
gcc -E $BOOT_FILE_INPUT > $BOOT_FILE_TMP

#parse file at proper format
#/^ *0x/ => get only lines starting by <space> and 0x...
# gsub(/0x/... => remove 0x
# gsub(/([^0-9A-F])/ ==> remove any characters others then 0-F

   awk '\
/^ *0x/ {gsub(/0x/,"");gsub(/([^0-9A-F])/,""); printf "%s", $0}\
' $BOOT_FILE_TMP > $BOOT_FILE_HEX_OUTPUT

xxd -r -p $BOOT_FILE_HEX_OUTPUT $BOOT_FILE_BIN_OUTPUT

#remove tmps
/bin/rm $BOOT_FILE_TMP
/bin/rm $BOOT_FILE_HEX_OUTPUT

