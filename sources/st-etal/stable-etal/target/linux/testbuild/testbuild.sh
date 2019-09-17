#!/bin/bash

# Synopsis:
# --------
#
# $ testbuild.sh <type> [<target_config_name>]
#   <type> is 'all' or 'splint'
#   <target_config_name> is the name of one of the existing target_config.h.* files

# Executes a 'make clean' followed by 'make all' or 'make splint' using each one of the
# configuration files contained in the ETAL/target/linux/testbuild directory;
# the target_config files originally contained in ETAL/target/linux are saved and restored
# at the end of the process.
#
# Optional:
# If invoked with a specific target_config.h.x name the script will only execute the procedure
# for that file. But see below for script invocation.

# Script invocation:
# -----------------
#
# This script is normally invoked by ETAL's main Makefile using the 'testbuild' or 'testsplint'
# targets.
#
# Thus:
# $ cd ETAL/target/linux
# $ make testbuild
#
# But, ETAL's global Makefile does not support invoking this script with the optional
# parameter, so to use this feature the script must be invoked directly:
# 
# $ cd ETAL/target/linux/testbuild
# $ ./testbuild.sh all target_config.h.3
#  or
# $ ./testbuild.sh splint target_config.h.3

# References:
# ----------
# Advanced Bash-Scripting Guide (http://www.tldp.org/LDP/abs/html/)

# temporary file for build warning count; will be used only if mktemp command fails
LOGFALLBACK=buildlog.txt
failed=0
total=0
warning=0

#
# local functions
#
process_target ()
{
	# $1 is the parameter of the testbuild.sh invocation ('all' or 'splint')
	# $2 is the target_config.h.? filename
	echo
	echo Building set $2
	echo

	failed_current=0
	warning_current=0

	# copy the selected target_config to the make directory
	cp -f $2 ../target_config.h
	# change the file timestamp for make
    touch ../target_config.h
	# copy the .mak file: we derive the filename from the .h file by
	# substituting the .h suffix with the .mak one with a sed regular expression
	cp -f `echo $2 | sed -e"s/\.h/.mak/" -` ../target_config.mak
	# change the file timestamp for make
    touch ../target_config.mak
	pushd ..
	# We want to count the warning messages produced by gcc; since gcc outputs warnings on stderr
	# we duplicate the standard error and redirect it to to the standard output (2>&1)
	# then we pipe the standard output to 'tee' so we can save it to a file but also view it on the
	# terminal. Later we process the file to count warnings

	# To support parallel make execution use two make invocations.
	# This avoids parallel execution of 'clean' and 'testbuild'
	# which would result in incomplete builds
	make clean 
	echo "log ... " $LOG
	make $1 2>&1 | tee $LOG

	# Here we count the number of lines in the gcc output created with tee, containing
	# the string 'warning'
    if [ "$1" == "all" ]; then
        let "warning_current += `perl -ne 'BEGIN{$cnt=0;}if(/: warning:\s*Clock skew detected.\s*Your build may be incomplete./i){$_=\"\";}if(/has modification time .* in the future/i){$_=\"\";}if(/warning.*forced in submake/i){$_=\"\";}if(/warning.*jobserver unavailable/i){$_=\"\";}if(/: warning:/i){$cnt++;}END{print\"$cnt\";}' $LOG`"
    fi
    if [ "$1" == "splint" ]; then
        let "warning_current += `perl -ne 'BEGIN{$cnt=0;}if(/: warning:\s*Clock skew detected.\s*Your build may be incomplete./i){$_=\"\";}if(/has modification time .* in the future/i){$_=\"\";}if(/warning.*forced in submake/i){$_=\"\";}if(/warning.*jobserver unavailable/i){$_=\"\";}if(/(\d+) code warnings/i){$cnt+=$1;}END{print\"$cnt\";}' $LOG`"
    fi

 	# tee overwrites the make command return value, so we use grep the log for errors instead
	# grep returns 0 if a match is found (or an error occurred, given the -q option)
	# one file in the build contains the word 'error' so it would always match the grep test,
	# we need to exclude it from the logfile
	grep -E -v "oserror|Werror" $LOG | grep -v Werror | grep -q -i error -
	if [ $? -eq 0 ] ; then
		echo
		echo "****************************"
		echo BUILD FAILED for $2
		echo "****************************"
		let "failed += 1"
		failed_current=1
	else
	    if  [ "$warning_current" != "0" ] ; then
		echo
		echo "****************************"
		echo  BUILD WITH WARNING for $2
		echo "****************************"	    
	    else
		echo
		echo "****************************"
		echo BUILD OK for $2
		echo "****************************"	    
	    fi
	fi
	let "total += 1"

   echo "failed_current " $failed_current
   echo "warning_current " $warning_current
   let warning+=$warning_current

    # let save the log in case error
    #
    if [ "$warning_current" == "0" ] && [ "$failed_current" == "0" ]; then
         rm -f $LOG
    else
          echo "LOG SAVED in : " /tmp/tmp.$2
          cp $LOG /tmp/tmp.$2
          rm -f $LOG	  
    fi
	popd
	echo
	echo
	echo
}

script_usage ()
{
	echo USAGE:
	echo $(basename $0) '{all|splint} [target_config.h.x]'
	echo - Argument in curly brackets '({})' is mandatory, specify either \'all\' or \'splint\'
	echo - Argument in square brackets '([])' is optional, if specified it must be the filename
	echo   of one of the 'target_config.h.*' contained in the `pwd` directory
	echo If '[target_config.h.x]' is specified the script must be run from the testbuild/ directory
}

script_warning ()
{
	echo "*******************************************************************"
	echo "* WARNING:"
	echo "* This script temporarily overwrites your target_config.h,mak files"
	echo "* If you interrupt the script before its natural end you will find a"
	echo "* copy of the original files in target_config.*.testbuild"
	echo "*******************************************************************"
}

script_warning2 ()
{
	echo "*******************************************************************"
	echo "* WARNING:"
	echo "* This script overwrites your target_config.h,mak files"
	echo "*******************************************************************"
}
#
# main entry
#

# parameter check
if [ -z $1 ] || [ "$1" != "all" ] && [ "$1" != "splint" ] ; then
	echo
	echo ERROR: missing required parameter
	echo
	script_usage
	exit 1
fi

# create a temporary file to collect the build log for warning count
# --tmpdir with no parameter requests the creation in the default
# tmp directory (normally /tmp)
LOG=`mktemp --tmpdir`
if [ ! -e $LOG ] ; then
	# fallback to a predefined name if mkdir fails
	# in this case the file will be located in the current directory (target/linux/)
	LOG=$LOGFALLBACK
fi

# target_config specified on the command line, use only that one and exit
if [ -n "$2" ] ; then
	if [ -e $2 ] ; then
		script_warning2
		read -p 'CTRL-C to abort, <return> to continue'
		process_target $1 $2
		exit $?
	fi
	echo
	echo ERROR: The file \'$2\' does not exist, exiting
	echo
	script_usage
	exit 1
fi

# regular processing, loop over all the target_config.h.*

script_warning

#clean up tmp files
/bin/rm tmp.target_config.* > /dev/null 2>&1

# save current target_config
pwd
cp -f target_config.h   target_config.h.testbuild
cp -f target_config.mak target_config.mak.testbuild
# save libs and exe
/bin/rm ../../etalcore/exports/etal.a.testbuild > /dev/null 2>&1
cp -f ../../etalcore/exports/etal.a ../../etalcore/exports/etal.a.testbuild > /dev/null 2>&1
/bin/rm ../../etalcore/exports/etalcore.a.testbuild > /dev/null 2>&1
cp -f ../../etalcore/exports/etalcore.a ../../etalcore/exports/etalcore.a.testbuild > /dev/null 2>&1
/bin/rm ../../tuner_driver/exports/tuner_driver.a.testbuild > /dev/null 2>&1
cp -f ../../tuner_driver/exports/tuner_driver.a ../../tuner_driver/exports/tuner_driver.a.testbuild > /dev/null 2>&1
/bin/rm o/etaltest.testbuild > /dev/null 2>&1
cp -f o/etaltest o/etaltest.testbuild > /dev/null 2>&1
/bin/rm ../../etalcore/exports/etal.a.testbuild > /dev/null 2>&1
cp -f ../../etalcore/exports/etal.a ../../etalcore/exports/etal.a.testbuild > /dev/null 2>&1

pushd testbuild

# overwrite with the predefined ones and build
for i in target_config.h.*
do
	process_target $1 $i
done

# restore target_config
popd
mv -f target_config.h.testbuild   target_config.h
mv -f target_config.mak.testbuild target_config.mak
mv -f ../../etalcore/exports/etal.a.testbuild ../../etalcore/exports/etal.a > /dev/null 2>&1
mv -f ../../etalcore/exports/etalcore.a.testbuild ../../etalcore/exports/etalcore.a > /dev/null 2>&1
mv -f ../../tuner_driver/exports/tuner_driver.a.testbuild ../../tuner_driver/exports/tuner_driver.a > /dev/null 2>&1
mv -f o/etaltest.testbuild o/etaltest > /dev/null 2>&1

echo "*******************************************************************"
echo "****************************"
echo "**** TEST BUILD RESULTS : "
echo "**** " $failed "builds FAILED / " $total "total"
echo "**** total " $warning " warnings"
echo "*******************************************************************"
