#!/bin/bash

if [ $# -lt 1 ];then
	echo -e "Usage:"
	echo -e "\t$(basename $0) <deploy_folder> [text_appened_to_output_file]"
	echo -e "\nExpected deploy folder is sta1295-evb-mmc, ..."
	exit
fi

if [ ! -d $1 ]; then
	echo "$1 doesn't exists !"
	exit
fi
folder=$(basename $1)

if [ $# -eq 2 ];then
	append="_"$2"_"
else
	append="_"
fi

suffix_list=$(find . -type l -name "*.manifest" | sed 's#core-image-##' | sed 's#-sta1.*##' |  cut -d/ -f3-)
images_count=$(echo $suffix_list | wc -w)

echo -e $images_count "image(s) available:"
for suffix in $suffix_list; do
	echo -e "\t$suffix"
done

if [ -e $folder$append"extraction.tar.gz" ]; then
	echo $folder$append"extraction.tar.gz" already exists
	exit
fi

cp -rL 	$folder $folder$append"extraction"

find $folder$append"extraction" | grep '[0-9]\{14\}' | xargs rm

rm -f $folder$append"extraction.tar.gz"
tar czf $folder$append"extraction.tar.gz" $folder$append"extraction"
rm -rf $folder$append"extraction"

echo "Images available in "$folder$append"extraction.tar.gz"
