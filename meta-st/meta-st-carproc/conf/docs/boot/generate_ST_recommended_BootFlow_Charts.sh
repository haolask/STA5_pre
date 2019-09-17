#!/bin/bash - 
#===============================================================================
#
#          FILE: generate_ST_recommended_BootFlow_Charts.sh
# 
#         USAGE: ./generate_ST_recommended_BootFlow_Charts.sh 
# 
#   DESCRIPTION: 
# 
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: YOUR NAME (), 
#  ORGANIZATION: 
#       CREATED: 02/21/2019 14:55
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error


#-------------------------------------------------------------
# usage function
#-------------------------------------------------------------
usage() {
        echo "usage: $(basename $0) [-j <path platuml.jar>]"
        echo ""
        echo "    -h                  print this help"

        echo "    -j  <path platuml.jar>  specify the plantUML jar file instead of fetch it from network"
}


#----------------------------------
# main
#----------------------------------

# Parse command line options.
cnt=$#
erase=0

if [ $cnt -eq 0 ];
then
wget http://sourceforge.net/projects/plantuml/files/plantuml.jar

fichier="./plantuml.jar"
erase=1
else
	# optional parameters check
	while [ $# -gt 0 ];
	do
		case "$1" in
		        -j)
		                shift
		                fichier=$1
		                ;;
		        -h)
				usage
				exit 0
				;;
			*)
		                usage
				shift
		                ;;
		esac
		shift
	done
fi


if [ -f $fichier ]; then
	echo "$fichier found in your folder - execute PNG file generation"
	java -jar $fichier *.txt
	if [ $erase -eq 1 ]; then
		rm  plantuml.jar
	fi
	echo "List of Generated files :"
	ls *.png
else
   echo "$fichier not found - you must obtain it - http://sourceforge.net/projects/plantuml/files/plantuml.jar"
fi
