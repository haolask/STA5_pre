rem pipe etalCtrlApp.pl output to putty plink configured with sessionEtal
perl etalCtrlApp.pl | plink -load sessionEtal
rem perl etalCtrlApp.pl | plink -load sessionEtal | c:\cygwin\bin\tee etal_log.txt
