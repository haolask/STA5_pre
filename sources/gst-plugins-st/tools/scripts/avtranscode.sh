#!/bin/sh -
#===============================================================================
#
#          FILE:  avtranscode.sh
#
#         USAGE:  ./avtranscode.sh
#
#   DESCRIPTION:
#
#        AUTHOR: Hugues Fruchet (), hugues.fruchet@st.com
#                Christophe Roullier (), christophe.roullier@st.com
#       COMPANY: STMicroelectronics
#       CREATED: 02/20/2014 05:37:57 PM CET
#      MODIFIED: 03/05/2015
#===============================================================================
echo "usage: ./avtranscode <input file> <width> <height> <profile> <level> <prefix>"
echo "     ==> By default Video Codec=H264, Audio codec=AAC, Container=AVI"
echo "usage: ./avtranscode <input file> <width> <height> <profile> <level> <vformat> <aformat> <container> <prefix>"
echo "examples:"
echo "#avtranscode Irma-I_know.mp4  1280 720 baseline 3.2 0"
echo "#avtranscode Irma-I_know.mp4  1920 1080 baseline 4 \"video/x-h264\" \"audio/mpeg,mpegversion=1,layer=3\" \"video/quicktime\" 0"

if [ -z "$7" ]
then
# i.e : avtranscode.sh Irma-I_know.mp4  1280 720 baseline 3.2 0
videocodec="video/x-h264"
audiocodec="audio/mpeg, mpegversion=4"
container="video/x-msvideo"
extension="avi"
if [ -z "$6" ]
then
prefix=0
else
prefix="$6"
fi
else
# i.e : avtranscode.sh Irma-I_know.mp4  1920 1080 baseline 4 "video/x-h264" "audio/mpeg,mpegversion=1,layer=3" "video/quicktime" 0"
videocodec="$6"
audiocodec="$7"
if [ -z "$8" ]
then
echo "ERROR : please precise container format"
exit
else
container="$8"
fi
case $container in
"application/ogg") extension="ogg";;
"video/x-matroska") extension="mkv";;
"video/quicktime") extension="mp4";;
"video/webm") extension="webm";;
"video/x-msvideo") extension="avi";;
"*") extension=".avi";;
esac
if [ -z "$9" ]
then
prefix=0
else
prefix="$9"
fi
fi

GST_DEBUG=*:2,v4l2enc*:3,v4l2trans*:3,GST_PADS:2 GST_DEBUG_LEVEL=*:1 encoding file:///$1 --vformat="$videocodec, width=$2, height=$3, profile=(string)$4, level=(string)$5" --aformat="$audiocodec" --format="$container" --outputuri=file:///$1_transcoded_$2x$3_$4_$5_$prefix.$extension

