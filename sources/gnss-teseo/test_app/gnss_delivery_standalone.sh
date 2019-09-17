#!/bin/sh

gnssenv_test=$(echo $PWD | grep hbp)
if [ "${gnssenv_test}" != "" ]
then
#GNSS HBP environment detected
binpath=$(echo $PWD | awk '{number=split($0,text,/\/hbp\//); printf("%s", text[1]); exit}'  | cut -d\" -f1)/bin
else
#GNSS Standalone environment detected
binpath=../bin
fi

mkdir -p ${binpath}
rm ${binpath}/*

./gnss_test_app.sh 2
./gnss_test_app.sh 3 n
./gnss_test_app.sh 4
./gnss_test_app.sh 5
./gnss_test_app.sh 6
./gnss_test_app.sh 7
./gnss_test_app.sh 8

