#!/bin/bash

# copies the current target_config.{h,mak} to testbuild/ adding a
# numerical suffix
#
# run from the testbuild directory

targetcount=0
currentcount=0
if [ -f target_config.h.1 ] ;
then
	for i in target_config.h.* 
	do 
		let "currentcount = `echo $i | sed -e"s/target_config.h.//" - `"
		echo Skipping version $currentcount
		# the shell normally orders the target_config as 1, 10, 2, 3, ...
		# so we need this additional test to cope with suffixes greater than 10
		if [ $currentcount -gt $targetcount ] ;
		then
			let "targetcount = currentcount"
		fi
	done
fi

let "targetcount += 1"
cp ../target_config.h   ./target_config.h.$targetcount
cp ../target_config.mak ./target_config.mak.$targetcount

echo Created target_config version $targetcount

