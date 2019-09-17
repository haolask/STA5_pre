#!/usr/bin/env python

DEFAULT_VIDEOTESTSRC= " videotestsrc num-buffers=500 "
LONG_VIDEOTESTSRC= " videotestsrc num-buffers=1000 "

#this file contains the definition of tests for bitrate controller
# in each table entry:
# [0]: reference/identifier of the test
# [1]: name of the test
# [2]: gstreamer source for the test
# [3]: v4l2enc (gstreamer plugins) properties
# [4]: v4l2enc (gstreamer plugins) capabilities
# [5]: file type of encoded video file ( .mp4 or .avi )

test_list_brc= [ \
[ "3.7a", "enc_h264_hp_cbr2000_fr30_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=1 bitrate=2000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.7b", "enc_h264_hp_cbr2000_fr60_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=1 bitrate=2000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.7c", "enc_h264_hp_cbr4000_fr30_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=1 bitrate=4000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.7d", "enc_h264_hp_cbr4000_fr60_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=1 bitrate=4000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.7e", "enc_h264_hp_cbr8000_fr30_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=1 bitrate=8000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.7f", "enc_h264_hp_cbr8000_fr60_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=1 bitrate=8000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.7g", "enc_h264_hp_cbr16000_fr30_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=1 bitrate=16000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.7h", "enc_h264_hp_cbr16000_fr60_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=1 bitrate=16000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.7i", "enc_h264_hp_vbr2000_fr30_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=0 bitrate=2000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.7j", "enc_h264_hp_vbr2000_fr60_from_videotestsrc" ,\
LONG_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=0 bitrate=2000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.7k", "enc_h264_hp_vbr4000_fr30_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=0 bitrate=4000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.7l", "enc_h264_hp_vbr4000_fr60_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=0 bitrate=4000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.7m", "enc_h264_hp_vbr8000_fr30_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=0 bitrate=8000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.7n", "enc_h264_hp_vbr8000_fr60_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=0 bitrate=8000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.7o", "enc_h264_hp_vbr16000_fr30_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=0 bitrate=16000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.7p", "enc_h264_hp_vbr16000_fr60_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=0 bitrate=16000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.7q", "enc_h264_hp_cbr2000_fr24000_1001_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=1 bitrate=2000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=24000/1001",\
".avi" ] ,\
\
[ "3.7r", "enc_h264_hp_cbr14000_fr24000_1001_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=1 bitrate=14000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=24000/1001",\
".avi" ] ,\
\
[ "3.7s", "enc_h264_hp_vbr2000_fr24000_1001_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=0 bitrate=2000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=24000/1001",\
".avi" ] ,\
\
[ "3.7t", "enc_h264_hp_vbr14000_fr24000_1001_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=0 bitrate=14000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=24000/1001",\
".avi" ] ,\
\
[ "3.7u", "enc_h264_hp_cbr2000_fr30000_1001_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=1 bitrate=2000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=30000/1001",\
".avi" ] ,\
\
[ "3.7v", "enc_h264_hp_cbr14000_fr30000_1001_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=1 bitrate=14000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=30000/1001",\
".avi" ] ,\
\
[ "3.7w", "enc_h264_hp_vbr2000_fr30000_1001_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=0 bitrate=2000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=30000/1001",\
".avi" ] ,\
\
[ "3.7x", "enc_h264_hp_vbr14000_fr30000_1001_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=0 bitrate=14000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=30000/1001",\
".avi" ] ,\
\
[ "3.7y", "enc_h264_hp_cbr2000_fr60000_1001_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=1 bitrate=2000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60000/1001",\
".avi" ] ,\
\
[ "3.7z", "enc_h264_hp_cbr14000_fr60000_1001_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=1 bitrate=14000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60000/1001",\
".avi" ] ,\
\
[ "3.7aa", "enc_h264_hp_vbr2000_fr60000_1001_from_videotestsrc" ,\
LONG_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=0 bitrate=2000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60000/1001",\
".avi" ] ,\
\
[ "3.7ab", "enc_h264_hp_vbr14000_fr60000_1001_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate-mode=0 bitrate=14000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60000/1001",\
".avi" ] ,\
\
[ "3.7ac", "enc_h264_hp_cbr_cpb_delay_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true cpb-size=75000 bitrate-mode=1 bitrate=2000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.7ad", "enc_h264_hp_vbr_cpb_from_videotestsrc" ,\
LONG_VIDEOTESTSRC,\
"cabac=true dct8x8=true cpb-size=75000 bitrate-mode=0 bitrate=2000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
\
] #end of brc test list Table

