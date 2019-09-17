#!/usr/bin/env python

import sys, os, time
import re

INDEX_REFERENCE=0
INDEX_TEST_NAME= 1
INDEX_GST_SOURCE=2
INDEX_GST_ENCODE_PROP=3
INDEX_V4L2ENC_CAPS=4
INDEX_SINK_TYPE=5

DEFAULT_LAUNCH="gst-launch-1.0 "
DEBUG_MODE=" --gst-debug=3"
DEFAULT_ENCODE= " v4l2enc "

BLUE = '\033[94m'
RED = '\033[91m'
GREEN = '\033[92m'
WARNING = '\033[93m'
FAIL = '\033[91m'
ENDC = '\033[0m'

sys.path.append(os.getcwd)
from baseline_profile import *
from main_profile import *
from high_profile import *
from mvc_profile import *
from align import *
from robustness import *
from brc import *

module_name = ""

def run_one_test(script_data):

	delete_files_command = ''.join( [ "rm -f " , out_directory , "/" , script_data[INDEX_TEST_NAME] , ".*"] )
	os.popen(delete_files_command)
	delete_files_command = ''.join( [ "rm -f " , out_directory_log , "/" , script_data[INDEX_TEST_NAME] , ".*"] )
	os.popen(delete_files_command)

	#clean dmesg
	os.system("dmesg -C")

	#start test case
	gst_command_line=get_command_line(script_data)
	print "\nExecuting test case : " , script_data[INDEX_REFERENCE], ": ", script_data[INDEX_TEST_NAME]
	print BLUE + gst_command_line + ENDC
	log = os.popen(gst_command_line)

	#save the output log
	trace = log.readlines()
	log_file = ''.join ( [ out_directory_log, "/" , script_data[INDEX_REFERENCE], "_" , script_data[INDEX_TEST_NAME] , '.res'] )
	stat_file = ''.join ( [ out_directory_stat, "/" , script_data[INDEX_REFERENCE], "_" , script_data[INDEX_TEST_NAME] , '.stat'] )
	fichier = open(log_file, "w")
	fichier.write(''.join(map(str, trace)))
	fichier.close()

	#check result of test
	test="_".join ([script_data[INDEX_REFERENCE], script_data[INDEX_TEST_NAME] ] )
	file_sink=get_video_name(script_data)
	check_result(test, gst_command_line, file_sink, result, stat_file)



def list_one_test(script_data):
	print script_data[INDEX_REFERENCE],':',script_data[INDEX_TEST_NAME]


def get_command_line(script_data):

	gst_command=DEFAULT_LAUNCH
	gst_debug=DEBUG_MODE
	gst_source=script_data[INDEX_GST_SOURCE]
	gst_encode="".join ([DEFAULT_ENCODE, script_data[INDEX_GST_ENCODE_PROP]])
	gst_caps="".join ([ '"', script_data[INDEX_V4L2ENC_CAPS], '"'])
	if script_data[INDEX_SINK_TYPE] == ".mp4":
		gst_mux=" h264parse ! qtmux "
	else:
		gst_mux=" h264parse ! avimux "
	file_sink = get_video_name(script_data)
	gst_sink = "".join ([ 'filesink location=', file_sink ])

	gst_command_line =  "".join( [ gst_command , gst_source , " ! ", gst_encode, " ! ", gst_caps," ! ", gst_mux, " ! ", gst_sink , gst_debug] )

	return gst_command_line

def get_video_name(script_data):

	file_name="_".join ([script_data[INDEX_REFERENCE], script_data[INDEX_TEST_NAME] ] )
	file_sink="".join ([ out_directory , "/", file_name , script_data[INDEX_SINK_TYPE] ] )

	return file_sink

def print_usage(script_name):
	print "Usage : ",script_name,\
		" [list|run] [all|all_bp|all_mp|all_hp|all_mv|all_al|all_rb|all_br|test_ref] [out_dir] [dbg]\n"
	print "list all      : lists all tests available"
	print "list all_bp   : lists all tests of baseline profile category"
	print "list all_mp   : lists all tests of main profile category"
	print "list all_hp   : lists all tests of high profile category"
	print "list all_mv   : lists all tests of mvc profile category"
	print "list all_al   : lists all tests of align category"
	print "list all_rb   : lists all tests of robustness category"
	print "list all_br   : lists all tests of bitrate controller category"
	print "list test_ref : gives the gstreamer command line for this test\n"
	print "run all       : runs all tests"
	print "run all_bp    : runs all tests of baseline profile category"
	print "run all_mp    : runs all tests of main profile category"
	print "run all_hp    : runs all tests of high profile category"
	print "run all_mv    : runs all tests of mvc profile category"
	print "run all_al    : runs all tests of align category"
	print "run all_rb    : runs all tests of robustness category"
	print "run all_br    : runs all tests of bitrate controller category"
	print "run test_ref  : runs one test\n"
	print "out_dir       : output directory (optionnal)"
	print "dbg           : debug option (optionnal, activate traces...)"



def check_result(test, cmd, filepath, resultfile, statfile):

	global errors_str
	fichier = open(resultfile,'a')
	fichier.write(''.join( [cmd, ""]))

# test if file exit and size!=O
	if (os.path.isfile(filepath)):
		if (os.path.getsize(filepath) > 0):
			fichier.write(''.join( ["=> ", filepath, "...File exists, status seems to be OK!\n"]))

			fichier.write("\n=> Please check debugfs information :\n")
			#check HVA debugfs
			debugfs = os.popen("cat /sys/kernel/debug/%s/last" %module_name.replace("_", "-"))
			trace = debugfs.readlines()
			fichier.write(''.join(map(str, trace)))
			error = os.popen("grep error /sys/kernel/debug/%s/last" %module_name.replace("_", "-"))
			trace = error.readlines()
			if trace :
				errors_str +=  ''.join( [ test, " FAILED (HVA driver errors)\n"] )

			fichier.write("\n=> Please check media information :\n")
			#launch tool and save information
			mediainfo_cmd =  "".join( [ "gst-discoverer-1.0 -v " , filepath] )
			infos = os.popen(mediainfo_cmd)
			trace = infos.readlines()
			fichier.write(''.join(map(str, trace)))
			fichier.write("\n\n")

			# save statistics
			get_stats(test, statfile)

			# check bitrate
			res = check_bitrate(test, statfile)
			if res == 1:
				print ''.join( [ test, " Real bitrate is not around target bitrate!\n"] )
				errors_str +=  ''.join( [ test, " Real bitrate is not around target bitrate!\n"] )
			else:
				if res == 2:
					print ''.join( [ test, " Real bitrate exceeds target bitrate!\n"] )
					errors_str +=  ''.join( [ test, " Real bitrate exceeds target bitrate!\n"] )
				else:
					print ''.join( [ test, " Correct bitrate!\n"] )

		else:
			errors_str +=  ''.join( [ test, " FAILED (null output file size)\n"] )
			fichier.write(''.join( ["=> ", filepath, "...File exists but size is 0!\n"]))
			fichier.write("=> STATUS = KO \n")
	else:
		errors_str +=  ''.join( [ test, " FAILED (no output file)\n"] )
		fichier.write(''.join( ["=> ", filepath, "...No such file!\n"]))
		fichier.write("=> STATUS = KO.\n\n")


	fichier.close()


def get_stats(test, statfile):

	stat = open(statfile, "w")
	total = 0
	n = 0
	avg_duration = 0
	avg_size = 0

	grep = os.popen("dmesg | grep stream | grep us")
	lines = grep.readlines()
	for a in lines:
		n = n + 1
		m = re.search('([0-9]+) us', a)
		if m == None:
			print " No duration found!"
			stat.close()
			return 0
		else:
			duration = int(m.group(1))
			if duration == 0:
				print " invalid duration!"
				stat.close()
				return 0
			else:
				stat.write(''.join( [ "duration	", str(duration), "	us\n"]))
				total += duration
	if n:
		avg_duration = total / n
		print ''.join( [ "avg_duration ", str(avg_duration), " on ", str(n), " samples "])
		stat.write(''.join( ["avg_duration = ", str(avg_duration), " us\n\n"]))

	total = 0
	n = 0
	grep = os.popen("dmesg | grep stream | grep bytes")
	lines = grep.readlines()
	for a in lines:
		n = n + 1
		m = re.search('([0-9]+) bytes', a)
		if m == None:
			print " No size found!"
			stat.close()
			return 0
		else:
			size = int(m.group(1))
			if size == 0:
				print " invalid size!"
				stat.write(''.join( [ "au size 	", str(size), "	bytes\n"]))
			else:
				stat.write(''.join( [ "au size 	", str(size), "	bytes\n"]))
				total += size

	if n:
		avg_size = total / n
		print ''.join( [ "avg_size ", str(avg_size), " on ", str(n), " samples "])
		stat.write(''.join( ["avg_size = ", str(avg_size * 8), " bits\n\n"]))

	stat.close()
	return 1


def check_bitrate(test, statfile):

	num = 0
	den = 1
	avg_size = 0
	reqbr = 0
	effbr = 0
	stat = open(statfile, "a")

	grep = os.popen("grep \"framerate=\" /sys/kernel/debug/%s/last" %module_name.replace("_", "-"))
	lines = grep.readlines()
	for a in lines:
		m = re.search('([0-9]+)/([0-9]+)', a)
		if m == None:
			print " No framerate found!"
		else:
			num = int(m.group(1))
			if num == 0:
				print " No framerate numerator found!"
				stat.close()
				return 0

			den = int(m.group(2))
			if den == 0:
				print " No framerate denominator found!"
				stat.close()
				return 0
			stat.write(''.join(["framerate = ",
				str(m.group(1)),"/",str(m.group(2)),"\n"]))

	grep = os.popen(''.join(["grep avg_size ", statfile]))
	lines = grep.readlines()
	for a in lines:
		m = re.search('[0-9]+', a)
		if m == None:
			print " No avg size found!"
		else:
			avg_size = int(m.group())
			if avg_size == 0:
				print " No avg size found!"
				stat.close()
				return 0

	effbr = avg_size * num / den
	stat.write(''.join(["real bitrate = ", str(effbr),"\n"]))

	grep = os.popen("grep \"bitrate=\" /sys/kernel/debug/%s/last" %module_name.replace("_", "-"))
	lines = grep.readlines()
	for a in lines:
		m = re.search('[0-9]+', a)
		if m == None:
			print " No bitrate found!"
		else:
			reqbr = int(m.group())*1000
			stat.write(''.join(["target bitrate = ", str(reqbr),"\n"]))

		if reqbr == 0:
			print " No bitrate value found!"
			stat.close()
			return 0

	print "check that bitrate is around requested one"
	if (float((max(reqbr,effbr) - min (reqbr,effbr)))/float(reqbr) <= 0.05 ):
		stat.close()
		return 0
	else:
		print str(effbr)
		stat.close()
		return 1




def getdevice():

	dev="/dev/videoN"
	for i in range(0,10) :
		if os.path.exists("/sys/class/video4linux/video"+str(i)+"/name"):
			file=os.popen("cat /sys/class/video4linux/video"+str(i)+"/name")
			devi=file.readlines()

			if "hva" in devi.pop():
				dev="/dev/video"+str(i)
				break
		else:
			break

	return dev


#main
# syntax
#  argument 1 - list, run
#  argument 2 - reference itd   ou  all
#  argument 3 - output directory for video files and traces
#  argument 4 - debug option (traces, others tools activation for debug)


if ((len(sys.argv)!=3) and (len(sys.argv)!=4) and (len(sys.argv)!=5)) :
	print_usage(sys.argv[0])
	sys.exit(0)

if (sys.argv[1] != "run") and (sys.argv[1] != "list") and (sys.argv[1] != "play"):
	print_usage(sys.argv[0])
else:
	#check list
	warning=0
	if (getdevice()=="/dev/videoN"):
		print  WARNING + "ERROR: HVA is not probed!" + ENDC
		warning=1
	if (os.path.isfile("Jets_1280x720_sp.yuv")==0):
		print  WARNING + "WARNING: Jets_1280x720_sp.yuv: no such file!" + ENDC
		warning=1
	if (os.path.isfile("Jets_nv21_1280x720.yuv")==0):
		print  WARNING + "WARNING: Jets_nv21_1280x720.yuv: no such file! Execute 'gen_files.py NV21' to get it." + ENDC
		warning=1
	if (os.path.isfile("Jets_vyuy_1280x720.yuv")==0):
		print  WARNING + "WARNING: Jets_vyuy_1280x720.yuv: no such file! Execute 'gen_files.py VYUY' to get it." + ENDC
		warning=1
	if (os.path.isfile("videotestscr_vyuy_1280x720.yuv")==0):
		print  WARNING + "WARNING: videotestscr_vyuy_1280x720.yuv: no such file! Execute 'gen_files.py VYUY' to get it." + ENDC
		warning=1

	if (warning==1):
		raw_input("Press [Enter] key to continue...")

	#get module name
	module_name =  os.popen("ls /sys/module/ | grep hva").read()[:-1]
        if (module_name==""):
		print  WARNING + "ERROR: module name not found (use 'hva' as default name)!" + ENDC
		module_name = "hva"

	debug=0
	root_for_tests = "./tests/"
	if (len(sys.argv)==4) and (sys.argv[3] =="dbg") :
		debug=1;
	else:
		if (len(sys.argv)==4) and (sys.argv[3] != "dbg") :
			root_for_tests = ''.join([sys.argv[3] , "/tests/" ])
		else:
			if (len(sys.argv)==5) and (sys.argv[4] == "dbg"):
				debug=1;
				root_for_tests = ''.join([sys.argv[3] , "/tests/" ])
			else:
				if (len(sys.argv)==5) and (sys.argv[4] != "dbg"):
					root_for_tests = ''.join([sys.argv[3] , "/tests/" ])

	area_name ="multimedia"
	gst_command_line =""
	errors_str =""

	out_directory = ''.join([ root_for_tests , area_name ])
	out_directory_log = ''.join([ out_directory , "/log" ])
	out_directory_stat = ''.join([ out_directory , "/stat" ])
	result=''.join ( [ out_directory, "/" , 'result.txt'] )
	cmdlist=''.join ( [ out_directory, "/" , 'cmd.txt'] )

	#output directory setup ( log/sink/history/stat)
	path = os.getcwd()
	mkdir_command = ''.join( ["mkdir -p " , out_directory ] )
	os.popen(mkdir_command)
	mkdir_command = ''.join( ["mkdir -p " , out_directory_log ] )
	os.popen(mkdir_command)
	mkdir_command = ''.join( ["mkdir -p " , out_directory_stat ] )
	os.popen(mkdir_command)

	# add marker for new run in result file
	f = open(result, 'a')
	f.write("\n\n**** NEW RUN ****\n\n")
	f.close()

	#deactivate all dynamic debug traces
	os.popen("echo -n '-p' > /sys/kernel/debug/dynamic_debug/control")

	#activate traces if requested
	if (debug==1):
		os.popen('echo "module %s +p" > /sys/kernel/debug/dynamic_debug/control' %module_name)
		os.popen("echo 8 > /proc/sys/kernel/printk")
	else:
		os.popen('echo "module %s -p" > /sys/kernel/debug/dynamic_debug/control' %module_name)
		#traces for performance
		os.popen('echo "format perf +p" > /sys/kernel/debug/dynamic_debug/control')

	#set test list
	test_list=test_list_baseline_profile+\
		test_list_main_profile+\
		test_list_high_profile+\
		test_list_mvc_profile+\
		test_list_align+\
		test_list_robustness+\
		test_list_brc

	if sys.argv[2] == "all_bp":
		test_list=test_list_baseline_profile
	if sys.argv[2] == "all_mp":
		test_list=test_list_main_profile
	if sys.argv[2] == "all_hp":
		test_list=test_list_high_profile
	if sys.argv[2] == "all_mv":
		test_list=test_list_mvc_profile
	if sys.argv[2] == "all_al":
		test_list=test_list_align
	if sys.argv[2] == "all_rb":
		test_list=test_list_robustness
	if sys.argv[2] == "all_br":
		test_list=test_list_brc

	#start execution
	if sys.argv[1] == "list":
		if (sys.argv[2] == "all"):
			# clear cmd list file if exist
			fichier = open(cmdlist,'w')
			fichier.close()
			print "Commands list saved in ", cmdlist
		for test_case in test_list :
			if (sys.argv[2] == test_case[INDEX_REFERENCE]):
				list_one_test(test_case)
				print get_command_line(test_case)
			if (sys.argv[2] == "all") or (sys.argv[2] == "all_bp") or (sys.argv[2] == "all_mp")\
				or (sys.argv[2] == "all_hp") or (sys.argv[2] == "all_mv") or (sys.argv[2] == "all_al")\
				or (sys.argv[2] == "all_rb") or (sys.argv[2] == "all_br"):
				list_one_test(test_case)
				gst_command_line = get_command_line(test_case)
				fichier = open(cmdlist,'a')
				fichier.write(''.join( [test_case[INDEX_REFERENCE], "\t",
					test_case[INDEX_TEST_NAME], "\t", gst_command_line, "\n"]))
				fichier.close()

	if sys.argv[1] == "run":
		for test_case in test_list :
			if (sys.argv[2] == test_case[INDEX_REFERENCE]):
				run_one_test(test_case)
			if (sys.argv[2] == "all") or (sys.argv[2] == "all_bp") or (sys.argv[2] == "all_mp")\
				or (sys.argv[2] == "all_hp") or (sys.argv[2] == "all_mv") or (sys.argv[2] == "all_al")\
				or (sys.argv[2] == "all_rb") or (sys.argv[2] == "all_br"):
				run_one_test(test_case)
		# print errors if any
		print  RED + errors_str + ENDC
		print "Tests report saved in ", result
		print "Please check this file and run gst-play-1.0 on ",out_directory, "video files"


	if sys.argv[1] == "play":
		for test_case in test_list :
			if (sys.argv[2] == test_case[INDEX_REFERENCE]):
				list_one_test(test_case)
				name = get_video_name(test_case)
				cmd = "".join( [ "gst-play-1.0 " , name] )
				os.popen(cmd)
