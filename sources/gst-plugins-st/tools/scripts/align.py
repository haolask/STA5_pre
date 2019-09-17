#!/usr/bin/env python

DEFAULT_VIDEOTESTSRC= " videotestsrc num-buffers=100 "
#DEFAULT_VIDEOTESTSRC= "videotestsrc num-buffers=20 "

#this file contains the definition of tests for baseline profile
# in each table entry:
# [0]: reference/identifier of the test
# [1]: name of the test
# [2]: gstreamer source for the test
# [3]: v4l2enc (gstreamer plugins) properties
# [4]: v4l2enc (capabilities) properties
# [5]: file type of encoded video file ( .mp4 or .avi )

test_list_align= [ \
[ "3.5a", "enc_h264_align_1250_706_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=1250, height=706, framerate=30/1",\
".avi" ] ,\
\
[ "3.5b", "enc_h264_align_1252_708_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=1252, height=708, framerate=30/1",\
".avi" ] ,\
\
[ "3.5c", "enc_h264_align_1254_710_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=1254, height=710, framerate=30/1",\
".avi" ] ,\
\
[ "3.5d", "enc_h264_align_1256_712_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=1256, height=712, framerate=30/1",\
".avi" ] ,\
\
[ "3.5e", "enc_h264_align_1258_714_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=1258, height=714, framerate=30/1",\
".avi" ] ,\
\
[ "3.5f", "enc_h264_align_1260_716_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=1260, height=716, framerate=30/1",\
".avi" ] ,\
\
[ "3.5g", "enc_h264_align_1262_718_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=1262, height=718, framerate=30/1",\
".avi" ] ,\
\
[ "3.5h", "enc_h264_align_1264_720_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=1264, height=720, framerate=30/1",\
".avi" ] ,\
\
[ "3.5i", "enc_h264_align_1266_722_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=1266, height=722, framerate=30/1",\
".avi" ] ,\
\
[ "3.5j", "enc_h264_align_1268_724_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=1268, height=724, framerate=30/1",\
".avi" ] ,\
\
[ "3.5k", "enc_h264_align_1270_726_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=1270, height=726, framerate=30/1",\
".avi" ] ,\
\
[ "3.5l", "enc_h264_align_1272_728_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=1272, height=728, framerate=30/1",\
".avi" ] ,\
\
[ "3.5m", "enc_h264_align_1274_730_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=1274, height=730, framerate=30/1",\
".avi" ] ,\
\
[ "3.5n", "enc_h264_align_1276_732_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=1276, height=732, framerate=30/1",\
".avi" ] ,\
\
[ "3.5o", "enc_h264_align_1278_734_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=1278, height=734, framerate=30/1",\
".avi" ] ,\
\
\
] #end of align test list Table

