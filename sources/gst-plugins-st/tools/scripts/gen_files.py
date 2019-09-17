#!/usr/bin/env python

import sys, os, time

BLUE = '\033[94m'
RED = '\033[94m'
GREEN = '\033[92m'
WARNING = '\033[93m'
FAIL = '\033[91m'
ENDC = '\033[0m'

def Gen_File_Convert(name, pixformat):
	if (sys.argv[1] == "VYUY"):
		cmd = "".join ([ "gst-launch-1.0 filesrc location= Jets_1280x720_sp.yuv num-buffers=100 ! videoparse format=nv21 width=1280 height=720\
 ! videoconvert ! \"video/x-raw, format=","UYVY",", width=1280, height=720\" ! filesink location=",name])
	else:
		cmd = "".join ([ "gst-launch-1.0 filesrc location= Jets_1280x720_sp.yuv num-buffers=100 ! videoparse format=nv12 width=1280 height=720\
 ! videoconvert ! \"video/x-raw, format=",pixformat,", width=1280, height=720\" ! filesink location=",name])

	print BLUE + cmd + ENDC
	os.popen(cmd)
	if os.path.isfile(name) and os.access(name, os.R_OK):
		print name + " generated.\n"
	else:
		print FAIL + name + " not generated.\n"+ ENDC


def Gen_File_From_VideoTestScr(name, pixformat):
	if (sys.argv[1] == "VYUY"):
		cmd = "".join ([ "gst-launch-1.0 videotestsrc num-buffers=100 !\
		 \"video/x-raw, format=NV12, width=1280, height=720\" !\
		 videoparse format=nv21 width=1280 height=720 !\
		 videoconvert ! \"video/x-raw, format=UYVY, width=1280, height=720\" ! filesink location=",name])
	else:
		cmd = "".join ([ "gst-launch-1.0 videotestsrc num-buffers=100 !\
		 videoparse format=nv12 width=1280 height=720\
		 ! videoconvert ! \"video/x-raw, format=",pixformat,", width=1280, height=720\" !\
		 filesink location=",name])

	print BLUE + cmd + ENDC
	os.popen(cmd)
	if os.path.isfile(name) and os.access(name, os.R_OK):
		print name + " generated.\n"
	else:
		print FAIL + name + " not generated.\n"+ ENDC


def Gen_File_Scale(name, width, height):
	cmd = "".join ([ "gst-launch-1.0 filesrc location= Jets_1280x720_sp.yuv ! videoparse format=nv12 width=1280 height=720\
 ! videoscale ! \"video/x-raw,width=",width,",height=",height,"\" ! filesink location=",name])
	print BLUE + cmd + ENDC
	os.popen(cmd)
	if os.path.isfile(name) and os.access(name, os.R_OK):
		print name + " generated.\n"
	else:
		print FAIL + name + " not generated.\n"+ ENDC

#main
# syntax
#  argument none
if (sys.argv[1] == "UYVY"):
	name = "".join ([ "Jets_uyvy_1280x720.yuv"])
if (sys.argv[1] == "VYUY"):
	name = "".join ([ "Jets_vyuy_1280x720.yuv"])
if (sys.argv[1] == "NV21"):
	name = "".join ([ "Jets_nv21_1280x720.yuv"])

Gen_File_Convert(name , sys.argv[1])

if (sys.argv[1] == "VYUY"):
#generate an additionnal file from videotestscr in VYUY pixel format
	Gen_File_From_VideoTestScr("videotestscr_vyuy_1280x720.yuv" , sys.argv[1])

