#!/bin/bash

# The CMOST firmware file .boot.h contains the CMOST firmware as C language array
# ETAL uses it to download the firmware (or patches) to the CMOST
# But in the .boot.h file delivered by the CMOST group the firmware array is
# always named 'CMOST_Firmware' so it is not possible to include multiple
# .boot.h files in TUNER_DRIVER which is a requirement to support different CMOST
# flavours at the same time.
#
# This script renames the 'CMOST_Firmware' string to something more specific
# so that multiple .boot.h files for different CMOST flavours can be included
# in TUNER_DRIVER.
#
# The script is intended to be run on the .boot.h file delivered by the CMOST
# group before committing it to the ETAL software repository
# It has to be run on the .boot.h file every time a new one is released, before
# committing it to the ETAL repository

# variables

# we need a way to identify for which CMOST flavour the file is;
# the file contains comments which refer to the CMOST flavour
# so we use those
star_t=TDA7707
star_s=TDA7708
dot_t=STA710
dot_s=STA709

# the name of the array in the original .boot.h file
array_name=CMOST_Firmware

# how we want to translate $array_name for the various CMOST flavours
star_t_array=CMOST_Firmware_STAR_T
star_s_array=CMOST_Firmware_STAR_S
dot_t_array=CMOST_Firmware_DOT_T
dot_s_array=CMOST_Firmware_DOT_S

# internal function, to be invoked with
#  change_name <CMOST flavour> <new array name> <filename>
#  $1 = <CMOST flavour>
#  $2 = <new array name>
#  $3 = <filaname>
change_name ()
{
	if grep -q $2 $3
	then
		echo ERROR: The file $3 already contains the $2 string, aborting
		exit 1
	fi

	if [ -f $3.bak ]
	then
		echo ERROR: This script would rename $3 to $3.bak before processing but
		echo $3.bak is already existing, aborting to avoid overwriting
		exit 1
	fi

	mv $3 $3.bak
	if sed -e "s/\b$array_name\b/$2/" $3.bak > $3
	then
		echo Success, substitution made for $1
		exit 0
	else
		echo ERROR: Substitution failed for $1, aborting
		exit 1
	fi
}

# here starts the 'main'

# parameter validation
if [ -z $1 ]
then
	echo Usage: 
	echo changeArrayName.sh "<filename"
	echo
	echo where "<filename>" is the name of the CMOST firmware file in .h format \(.boot.h\)
	echo
	echo The script searches one of the strings $star_t, $star_s, $dot_t or $dot_s in the file
	echo and if it finds it changes the references to the generic \'$array_name\' string to a more
	echo specific name $star_t_array, $star_s_array, $dot_t_array or $dot_s_array so the
	echo file can be included in the ETAL build
	exit 1
fi
if [ ! -f $1 ]
then
	echo ERROR: Cannot open $1
	exit 1
fi

# processing
if grep -q $star_t $1
then
	change_name $star_t $star_t_array $1
	exit
fi

if grep -q $star_s $1
then
	change_name $star_s $star_s_array $1
	exit
fi

if grep -q $dot_t $1
then
	change_name $dot_t $dot_t_array $1
	exit
fi

if grep -q $dot_s $1
then
	change_name $dot_s $dot_s_array $1
	exit
fi

# error condition
echo ERROR: Did not find string $star_t, $star_s, $dot_t or $dot_s in file $1, aborting
exit 1
