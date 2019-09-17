#!/bin/sh
#
# Usage: readtracelog.sh [executable_file] [trace.out]
# Description: read an -finstrument_functions trace log and decode function names
# Author: David Pastor

do_help() {
	echo "Usage: readtracelog.sh [executable_file] [trace.out] > [trace_dec.txt]"
	echo "Description: read an -finstrument_functions trace log and decode function names"
}

if test ! -f "$1"
then
	echo "Error: executable $1 does not exist."
	do_help
	exit 1
fi
if test ! -f "$2"
then
	echo "Error: trace log $2 does not exist."
	do_help
	exit 1
fi
EXECUTABLE="$1"
TRACELOG="$2"
OFFSET=0
if test ! -z "$3"
then
	OFFSET="$3"
fi
while read LINETYPE FADDR CADDR CTIME CTIME2; do
	if test "${LINETYPE}" = "r"
	then
		OFFSET=${FADDR}
		CLOCKSPERSEC=${CTIME}
		echo "Relocation from ${FADDR} to ${CADDR}"
	else
	    FADDROFFSET=$(( ${FADDR} - ${OFFSET} + 0x7ffc ))
	    CADDROFFSET=$(( ${CADDR} - ${OFFSET} + 0x7ffc ))
	    FADDROFFSETHEX=$( printf "%x" ${FADDROFFSET} )
	    CADDROFFSETHEX=$( printf "%x" ${CADDROFFSET} )
		FNAME="$(arm-v7-linux-uclibceabi-addr2line -f -e ${EXECUTABLE} ${FADDROFFSETHEX}|head -1)"
		CDATE=$( echo "scale=9; ${CTIME} * 0.000000001 + ${CTIME2}" | bc )
	fi
	if test "${LINETYPE}" = "e"
	then
		CNAME="$(arm-v7-linux-uclibceabi-addr2line -f -e ${EXECUTABLE} ${CADDROFFSETHEX}|head -1)"
		CLINE="$(arm-v7-linux-uclibceabi-addr2line -s -e ${EXECUTABLE} ${CADDROFFSETHEX})"
		echo "Enter ${FNAME} at ${CDATE} s, called from ${CNAME} (${CLINE})"
	fi
	if test "${LINETYPE}" = "x"
	then
		echo "Exit  ${FNAME} at ${CDATE} s"
	fi
done < "${TRACELOG}"

