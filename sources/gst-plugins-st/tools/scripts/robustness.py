#!/usr/bin/env python

DEFAULT_VIDEOTESTSRC= " videotestsrc num-buffers=600 "

#this file contains the definition of robustness tests
# in each table entry:
# [0]: reference/identifier of the test
# [1]: name of the test
# [2]: gstreamer source for the test
# [3]: v4l2enc (gstreamer plugins) properties
# [4]: v4l2enc (gstreamer plugins) capabilities
# [5]: file type of encoded video file ( .mp4 or .avi )

test_list_robustness= [ \
# SNOW
[ "3.6a",\
"enc_h264_bp_level_3_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC ,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6b",\
"enc_h264_bp_level_3_1_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6c",\
"enc_h264_bp_level_3_2_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000 cpb-size=40000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6d",\
"enc_h264_bp_level_4_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000 cpb-size=25000 qp-min=35" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6e",\
"enc_h264_bp_level_4_1_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000 cpb-size=62500 qp-min=35" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6f",\
"enc_h264_bp_level_4_2_from_videotestsrc_snow" ,\
"videotestsrc num-buffers=1000 pattern=snow",\
"bitrate=50000 cpb-size=62500 qp-min=35" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6g",\
"enc_h264_bp_level_4_720p_60fps_from_videotestsrc_snow" ,\
"videotestsrc num-buffers=600 pattern=snow",\
"bitrate=20000 cpb-size=25000 qp-min=35" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6h",\
"enc_h264_bp_level_4_1_720p_60fps_from_videotestsrc_snow" ,\
"videotestsrc num-buffers=1000 pattern=snow",\
"bitrate=50000 cpb-size=62500 qp-min=35" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6i",\
"enc_h264_bp_level_4_2_720p_60fps_from_videotestsrc_snow" ,\
"videotestsrc num-buffers=1000 pattern=snow",\
"bitrate=50000 cpb-size=62500 qp-min=35" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6j",\
"enc_h264_mp_level_3_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC ,\
"cabac=true bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6k",\
"enc_h264_mp_level_3_1_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=14000 cpb-size=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6l",\
"enc_h264_mp_level_3_2_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000 cpb-size=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6m",\
"enc_h264_mp_level_4_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000 cpb-size=25000 qp-min=35" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6n",\
"enc_h264_mp_level_4_1_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6o",\
"enc_h264_mp_level_4_2_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000 cpb-size=62500 qp-min=30" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6p",\
"enc_h264_mp_level_4_720p_60fps_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000 cpb-size=25000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6q",\
"enc_h264_mp_level_4_1_720p_60fps_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000 cpb-size=62500" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6r",\
"enc_h264_mp_level_4_2_720p_60fps_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000 cpb-size=62500" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6s",\
"enc_h264_hp_level_3_1_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=16800 cpb-size=14000 qp-min=30" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6t",\
"enc_h264_hp_level_3_2_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000 cpb-size=20000 qp-min=30" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6u",\
"enc_h264_hp_level_4_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000 cpb-size=25000 qp-min=35" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6v",\
"enc_h264_hp_level_4_1_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=35" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6w",\
"enc_h264_hp_level_4_2_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=35" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6x",\
"enc_h264_hp_level_4_720p_60fps_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000 cpb-size=25000 qp-min=30" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6y",\
"enc_h264_hp_level_4_1_720p_60fps_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=35" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6z",\
"enc_h264_hp_level_4_2_720p_60fps_from_videotestsrc_snow" ,\
"%s pattern=snow" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=35" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
\
# BLUE
[ "3.6aa",\
"enc_h264_bp_level_3_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC ,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ab",\
"enc_h264_bp_level_3_1_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ac",\
"enc_h264_bp_level_3_2_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ad",\
"enc_h264_bp_level_4_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ae",\
"enc_h264_bp_level_4_1_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6af",\
"enc_h264_bp_level_4_2_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ag",\
"enc_h264_bp_level_4_720p_60fps_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ah",\
"enc_h264_bp_level_4_1_720p_60fps_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ai",\
"enc_h264_bp_level_4_2_720p_60fps_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6aj",\
"enc_h264_mp_level_3_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC ,\
"cabac=true bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ak",\
"enc_h264_mp_level_3_1_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6al",\
"enc_h264_mp_level_3_2_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6am",\
"enc_h264_mp_level_4_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6an",\
"enc_h264_mp_level_4_1_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ao",\
"enc_h264_mp_level_4_2_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ap",\
"enc_h264_mp_level_4_720p_60fps_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6aq",\
"enc_h264_mp_level_4_1_720p_60fps_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ar",\
"enc_h264_mp_level_4_2_720p_60fps_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6as",\
"enc_h264_hp_level_3_1_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=16800" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6at",\
"enc_h264_hp_level_3_2_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6au",\
"enc_h264_hp_level_4_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6av",\
"enc_h264_hp_level_4_1_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6aw",\
"enc_h264_hp_level_4_2_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ax",\
"enc_h264_hp_level_4_720p_60fps_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ay",\
"enc_h264_hp_level_4_1_720p_60fps_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6az",\
"enc_h264_hp_level_4_2_720p_60fps_from_videotestsrc_blue" ,\
"%s pattern=blue" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
\
# CHECKERS-2
[ "3.6ba",\
"enc_h264_bp_level_3_from_videotestsrc_checkers-2" ,\
"%s pattern=checkers-2" %DEFAULT_VIDEOTESTSRC ,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6bb",\
"enc_h264_bp_level_3_1_from_videotestsrc_checkers-2" ,\
"%s pattern=checkers-2" %DEFAULT_VIDEOTESTSRC,\
"bitrate=14000 cpb-size=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6bc",\
"enc_h264_bp_level_3_2_from_videotestsrc_checkers-2" ,\
"%s pattern=checkers-2" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000 cpb-size=20000 qp-min=30" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6bd",\
"enc_h264_bp_level_4_from_videotestsrc_checkers-2" ,\
"%s pattern=checkers-2" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000 cpb-size=25000 qp-min=35" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6be",\
"enc_h264_bp_level_4_1_from_videotestsrc_checkers-2" ,\
"%s pattern=checkers-2" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000 cpb-size=62500 qp-min=35" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6bf",\
"enc_h264_bp_level_4_2_from_videotestsrc_checkers-2" ,\
"%s pattern=checkers-2" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000 cpb-size=50000 qp-min=25" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6bg",\
"enc_h264_bp_level_4_720p_60fps_from_videotestsrc_checkers-2" ,\
"%s pattern=checkers-2" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000 cpb-size=25000 qp-min=25" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6bh",\
"enc_h264_bp_level_4_1_720p_60fps_from_videotestsrc_checkers-2" ,\
"videotestsrc num-buffers=1000 pattern=checkers-2",\
"bitrate=50000 cpb-size=62500 qp-min=20" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6bi",\
"enc_h264_bp_level_4_2_720p_60fps_from_videotestsrc_checkers-2" ,\
"videotestsrc num-buffers=1000 pattern=checkers-2",\
"bitrate=50000 cpb-size=62500" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6bj",\
"enc_h264_mp_level_3_from_videotestsrc_checkers-2" ,\
"%s pattern=checkers-2" %DEFAULT_VIDEOTESTSRC ,\
"cabac=true bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6bk",\
"enc_h264_mp_level_3_1_from_videotestsrc_checkers-2" ,\
"%s pattern=checkers-2" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=14000 cpb-size=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6bl",\
"enc_h264_mp_level_3_2_from_videotestsrc_checkers-2" ,\
"videotestsrc num-buffers=1000 pattern=checkers-2",\
"cabac=true bitrate=20000 cpb-size=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6bm",\
"enc_h264_mp_level_4_from_videotestsrc_checkers-2" ,\
"%s pattern=checkers-2" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000 cpb-size=25000 qp-min=25" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6bn",\
"enc_h264_mp_level_4_1_from_videotestsrc_checkers-2" ,\
"%s pattern=checkers-2" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6bo",\
"enc_h264_mp_level_4_2_from_videotestsrc_checkers-2" ,\
"videotestsrc num-buffers=1000 pattern=checkers-2",\
"cabac=true bitrate=50000 cpb-size=25000 qp-min=25" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6bp",\
"enc_h264_mp_level_4_720p_60fps_from_videotestsrc_checkers-2" ,\
"videotestsrc num-buffers=1000 pattern=checkers-2",\
"cabac=true bitrate=20000 cpb-size=25000 qp-min=25" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6bq",\
"enc_h264_mp_level_4_1_720p_60fps_from_videotestsrc_checkers-2" ,\
"videotestsrc num-buffers=1000 pattern=checkers-2",\
"cabac=true bitrate=50000 cpb-size=62500 qp-min=20" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6br",\
"enc_h264_mp_level_4_2_720p_60fps_from_videotestsrc_checkers-2" ,\
"videotestsrc num-buffers=1000 pattern=checkers-2",\
"cabac=true bitrate=50000 cpb-size=62500 qp-min=20" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6bs",\
"enc_h264_hp_level_3_1_from_videotestsrc_checkers-2" ,\
"%s pattern=checkers-2" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=16800 cpb-size=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6bt",\
"enc_h264_hp_level_3_2_from_videotestsrc_checkers-2" ,\
"videotestsrc num-buffers=1000 pattern=checkers-2",\
"cabac=true dct8x8=true bitrate=24000 cpb-size=20000 qp-min=25" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6bu",\
"enc_h264_hp_level_4_from_videotestsrc_checkers-2" ,\
"%s pattern=checkers-2" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000 cpb-size=25000 qp-min=25" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6bv",\
"enc_h264_hp_level_4_1_from_videotestsrc_checkers-2" ,\
"%s pattern=checkers-2" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6bw",\
"enc_h264_hp_level_4_2_from_videotestsrc_checkers-2" ,\
"videotestsrc num-buffers=1000 pattern=checkers-2",\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6bx",\
"enc_h264_hp_level_4_720p_60fps_from_videotestsrc_checkers-2" ,\
"videotestsrc num-buffers=1000 pattern=checkers-2",\
"cabac=true dct8x8=true bitrate=24000 cpb-size=25000 qp-min=25" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6by",\
"enc_h264_hp_level_4_1_720p_60fps_from_videotestsrc_checkers-2" ,\
"videotestsrc num-buffers=1000 pattern=checkers-2",\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6bz",\
"enc_h264_hp_level_4_2_720p_60fps_from_videotestsrc_checkers-2" ,\
"videotestsrc num-buffers=1000 pattern=checkers-2",\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
\
# CIRCULAR
[ "3.6ca",\
"enc_h264_bp_level_3_from_videotestsrc_circular" ,\
"%s pattern=circular" %DEFAULT_VIDEOTESTSRC ,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6cb",\
"enc_h264_bp_level_3_1_from_videotestsrc_circular" ,\
"%s pattern=circular" %DEFAULT_VIDEOTESTSRC,\
"bitrate=14000 cpb-size=14000 qp-min=25" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6cc",\
"enc_h264_bp_level_3_2_from_videotestsrc_circular" ,\
"videotestsrc num-buffers=1000 pattern=circular",\
"bitrate=20000 cpb-size=20000 qp-min=25" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6cd",\
"enc_h264_bp_level_4_from_videotestsrc_circular" ,\
"%s pattern=circular" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000 cpb-size=25000 qp-min=25" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ce",\
"enc_h264_bp_level_4_1_from_videotestsrc_circular" ,\
"%s pattern=circular" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000 cpb-size=625000 qp-min=25" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6cf",\
"enc_h264_bp_level_4_2_from_videotestsrc_circular" ,\
"videotestsrc num-buffers=1000 pattern=circular",\
"bitrate=50000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6cg",\
"enc_h264_bp_level_4_720p_60fps_from_videotestsrc_circular" ,\
"videotestsrc num-buffers=1000 pattern=circular",\
"bitrate=20000 cpb-size=25000 qp-min=25" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ch",\
"enc_h264_bp_level_4_1_720p_60fps_from_videotestsrc_circular" ,\
"videotestsrc num-buffers=1000 pattern=circular",\
"bitrate=50000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ci",\
"enc_h264_bp_level_4_2_720p_60fps_from_videotestsrc_circular" ,\
"videotestsrc num-buffers=1000 pattern=circular",\
"bitrate=50000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6cj",\
"enc_h264_mp_level_3_from_videotestsrc_circular" ,\
"%s pattern=circular" %DEFAULT_VIDEOTESTSRC ,\
"cabac=true bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ck",\
"enc_h264_mp_level_3_1_from_videotestsrc_circular" ,\
"%s pattern=circular" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=14000 cpb-size=14000 qp-min=25" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6cl",\
"enc_h264_mp_level_3_2_from_videotestsrc_circular" ,\
"videotestsrc num-buffers=1000 pattern=circular",\
"cabac=true bitrate=20000 cpb-size=20000 qp-min=25" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6cm",\
"enc_h264_mp_level_4_from_videotestsrc_circular" ,\
"%s pattern=circular" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000 cpb-size=25000 qp-min=25" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6cn",\
"enc_h264_mp_level_4_1_from_videotestsrc_circular" ,\
"%s pattern=circular" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6co",\
"enc_h264_mp_level_4_2_from_videotestsrc_circular" ,\
"videotestsrc num-buffers=1000 pattern=circular",\
"cabac=true bitrate=50000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6cp",\
"enc_h264_mp_level_4_720p_60fps_from_videotestsrc_circular" ,\
"videotestsrc num-buffers=1000 pattern=circular",\
"cabac=true bitrate=20000 cpb-size=25000 qp-min=25" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6cq",\
"enc_h264_mp_level_4_1_720p_60fps_from_videotestsrc_circular" ,\
"videotestsrc num-buffers=1000 pattern=circular",\
"cabac=true bitrate=50000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6cr",\
"enc_h264_mp_level_4_2_720p_60fps_from_videotestsrc_circular" ,\
"videotestsrc num-buffers=1000 pattern=circular",\
"cabac=true bitrate=50000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6cs",\
"enc_h264_hp_level_3_1_from_videotestsrc_circular" ,\
"%s pattern=circular" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=16800 cpb-size=14000 qp-min=25" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ct",\
"enc_h264_hp_level_3_2_from_videotestsrc_circular" ,\
"videotestsrc num-buffers=1000 pattern=circular",\
"cabac=true dct8x8=true bitrate=24000 cpb-size=20000 qp-min=25" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6cu",\
"enc_h264_hp_level_4_from_videotestsrc_circular" ,\
"%s pattern=circular" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000 cpb-size=25000 qp-min=25" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6cv",\
"enc_h264_hp_level_4_1_from_videotestsrc_circular" ,\
"%s pattern=circular" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6cw",\
"enc_h264_hp_level_4_2_from_videotestsrc_circular" ,\
"videotestsrc num-buffers=1000 pattern=circular",\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6cx",\
"enc_h264_hp_level_4_720p_60fps_from_videotestsrc_circular" ,\
"videotestsrc num-buffers=1000 pattern=circular",\
"cabac=true dct8x8=true bitrate=24000 cpb-size=25000 qp-min=25" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6cy",\
"enc_h264_hp_level_4_1_720p_60fps_from_videotestsrc_circular" ,\
"videotestsrc num-buffers=1000 pattern=circular",\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6cz",\
"enc_h264_hp_level_4_2_720p_60fps_from_videotestsrc_circular" ,\
"videotestsrc num-buffers=1000 pattern=circular",\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
\
# BLINK
[ "3.6da",\
"enc_h264_bp_level_3_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC ,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6db",\
"enc_h264_bp_level_3_1_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6dc",\
"enc_h264_bp_level_3_2_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6dd",\
"enc_h264_bp_level_4_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6de",\
"enc_h264_bp_level_4_1_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6df",\
"enc_h264_bp_level_4_2_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6dg",\
"enc_h264_bp_level_4_720p_60fps_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6dh",\
"enc_h264_bp_level_4_1_720p_60fps_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6di",\
"enc_h264_bp_level_4_2_720p_60fps_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6dj",\
"enc_h264_mp_level_3_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC ,\
"cabac=true bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6dk",\
"enc_h264_mp_level_3_1_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6dl",\
"enc_h264_mp_level_3_2_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6dm",\
"enc_h264_mp_level_4_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6dn",\
"enc_h264_mp_level_4_1_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6do",\
"enc_h264_mp_level_4_2_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6dp",\
"enc_h264_mp_level_4_720p_60fps_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6dq",\
"enc_h264_mp_level_4_1_720p_60fps_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6dr",\
"enc_h264_mp_level_4_2_720p_60fps_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6ds",\
"enc_h264_hp_level_3_1_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=16800" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6dt",\
"enc_h264_hp_level_3_2_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6du",\
"enc_h264_hp_level_4_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6dv",\
"enc_h264_hp_level_4_1_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6dw",\
"enc_h264_hp_level_4_2_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6dx",\
"enc_h264_hp_level_4_720p_60fps_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6dy",\
"enc_h264_hp_level_4_1_720p_60fps_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6dz",\
"enc_h264_hp_level_4_2_720p_60fps_from_videotestsrc_blink" ,\
"%s pattern=blink" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
\
# ZONE-PLATE
[ "3.6ea",\
"enc_h264_bp_level_3_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC ,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6eb",\
"enc_h264_bp_level_3_1_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ec",\
"enc_h264_bp_level_3_2_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ed",\
"enc_h264_bp_level_4_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ee",\
"enc_h264_bp_level_4_1_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ef",\
"enc_h264_bp_level_4_2_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6eg",\
"enc_h264_bp_level_4_720p_60fps_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6eh",\
"enc_h264_bp_level_4_1_720p_60fps_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ei",\
"enc_h264_bp_level_4_2_720p_60fps_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6ej",\
"enc_h264_mp_level_3_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC ,\
"cabac=true bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ek",\
"enc_h264_mp_level_3_1_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6el",\
"enc_h264_mp_level_3_2_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6em",\
"enc_h264_mp_level_4_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6en",\
"enc_h264_mp_level_4_1_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6eo",\
"enc_h264_mp_level_4_2_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ep",\
"enc_h264_mp_level_4_720p_60fps_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6eq",\
"enc_h264_mp_level_4_1_720p_60fps_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6er",\
"enc_h264_mp_level_4_2_720p_60fps_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6es",\
"enc_h264_hp_level_3_1_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=16800" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6et",\
"enc_h264_hp_level_3_2_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6eu",\
"enc_h264_hp_level_4_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ev",\
"enc_h264_hp_level_4_1_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ew",\
"enc_h264_hp_level_4_2_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ex",\
"enc_h264_hp_level_4_720p_60fps_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ey",\
"enc_h264_hp_level_4_1_720p_60fps_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ez",\
"enc_h264_hp_level_4_2_720p_60fps_from_videotestsrc_zone-plate" ,\
"%s pattern=zone-plate" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
\
# GAMUT
[ "3.6fa",\
"enc_h264_bp_level_3_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC ,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6fb",\
"enc_h264_bp_level_3_1_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6fc",\
"enc_h264_bp_level_3_2_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6fd",\
"enc_h264_bp_level_4_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6fe",\
"enc_h264_bp_level_4_1_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ff",\
"enc_h264_bp_level_4_2_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6fg",\
"enc_h264_bp_level_4_720p_60fps_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6fh",\
"enc_h264_bp_level_4_1_720p_60fps_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6fi",\
"enc_h264_bp_level_4_2_720p_60fps_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6fj",\
"enc_h264_mp_level_3_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC ,\
"cabac=true bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6fk",\
"enc_h264_mp_level_3_1_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6fl",\
"enc_h264_mp_level_3_2_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6fm",\
"enc_h264_mp_level_4_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6fn",\
"enc_h264_mp_level_4_1_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6fo",\
"enc_h264_mp_level_4_2_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6fp",\
"enc_h264_mp_level_4_720p_60fps_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6fq",\
"enc_h264_mp_level_4_1_720p_60fps_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6fr",\
"enc_h264_mp_level_4_2_720p_60fps_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6fs",\
"enc_h264_hp_level_3_1_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=16800" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ft",\
"enc_h264_hp_level_3_2_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6fu",\
"enc_h264_hp_level_4_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6fv",\
"enc_h264_hp_level_4_1_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6fw",\
"enc_h264_hp_level_4_2_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6fx",\
"enc_h264_hp_level_4_720p_60fps_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6fy",\
"enc_h264_hp_level_4_1_720p_60fps_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6fz",\
"enc_h264_hp_level_4_2_720p_60fps_from_videotestsrc_gamut" ,\
"%s pattern=gamut" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
\
# BALL
[ "3.6ga",\
"enc_h264_bp_level_3_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC ,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6gb",\
"enc_h264_bp_level_3_1_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6gc",\
"enc_h264_bp_level_3_2_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6gd",\
"enc_h264_bp_level_4_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ge",\
"enc_h264_bp_level_4_1_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6gf",\
"enc_h264_bp_level_4_2_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6gg",\
"enc_h264_bp_level_4_720p_60fps_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6gh",\
"enc_h264_bp_level_4_1_720p_60fps_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6gi",\
"enc_h264_bp_level_4_2_720p_60fps_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6gj",\
"enc_h264_mp_level_3_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC ,\
"cabac=true bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6gk",\
"enc_h264_mp_level_3_1_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6gl",\
"enc_h264_mp_level_3_2_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6gm",\
"enc_h264_mp_level_4_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6gn",\
"enc_h264_mp_level_4_1_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6go",\
"enc_h264_mp_level_4_2_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6gp",\
"enc_h264_mp_level_4_720p_60fps_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6gq",\
"enc_h264_mp_level_4_1_720p_60fps_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6gr",\
"enc_h264_mp_level_4_2_720p_60fps_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6gs",\
"enc_h264_hp_level_3_1_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=16800" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6gt",\
"enc_h264_hp_level_3_2_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6gu",\
"enc_h264_hp_level_4_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6gv",\
"enc_h264_hp_level_4_1_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6gw",\
"enc_h264_hp_level_4_2_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6gx",\
"enc_h264_hp_level_4_720p_60fps_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6gy",\
"enc_h264_hp_level_4_1_720p_60fps_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6gz",\
"enc_h264_hp_level_4_2_720p_60fps_from_videotestsrc_ball" ,\
"%s pattern=ball" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
\
# PINWHEEL
[ "3.6ha",\
"enc_h264_bp_level_3_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC ,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6hb",\
"enc_h264_bp_level_3_1_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6hc",\
"enc_h264_bp_level_3_2_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6hd",\
"enc_h264_bp_level_4_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6he",\
"enc_h264_bp_level_4_1_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6hf",\
"enc_h264_bp_level_4_2_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6hg",\
"enc_h264_bp_level_4_720p_60fps_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6hh",\
"enc_h264_bp_level_4_1_720p_60fps_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6hi",\
"enc_h264_bp_level_4_2_720p_60fps_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6hj",\
"enc_h264_mp_level_3_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC ,\
"cabac=true bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6hk",\
"enc_h264_mp_level_3_1_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6hl",\
"enc_h264_mp_level_3_2_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6hm",\
"enc_h264_mp_level_4_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6hn",\
"enc_h264_mp_level_4_1_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ho",\
"enc_h264_mp_level_4_2_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6hp",\
"enc_h264_mp_level_4_720p_60fps_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6hq",\
"enc_h264_mp_level_4_1_720p_60fps_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6hr",\
"enc_h264_mp_level_4_2_720p_60fps_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6hs",\
"enc_h264_hp_level_3_1_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=16800" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ht",\
"enc_h264_hp_level_3_2_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6hu",\
"enc_h264_hp_level_4_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6hv",\
"enc_h264_hp_level_4_1_from_videotestsrc_pinwheel" ,\
"%s pattern=pinwheel" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6hw",\
"enc_h264_hp_level_4_2_from_videotestsrc_pinwheel" ,\
"videotestsrc num-buffers=100 pattern=pinwheel",\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6hx",\
"enc_h264_hp_level_4_720p_60fps_from_videotestsrc_pinwheel" ,\
"videotestsrc num-buffers=100 pattern=pinwheel",\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6hy",\
"enc_h264_hp_level_4_1_720p_60fps_from_videotestsrc_pinwheel" ,\
"videotestsrc num-buffers=100 pattern=pinwheel",\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6hz",\
"enc_h264_hp_level_4_2_720p_60fps_from_videotestsrc_pinwheel" ,\
"videotestsrc num-buffers=100 pattern=pinwheel",\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
\
# SPOKES
[ "3.6ia",\
"enc_h264_bp_level_3_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes" ,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ib",\
"enc_h264_bp_level_3_1_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ic",\
"enc_h264_bp_level_3_2_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"bitrate=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6id",\
"enc_h264_bp_level_4_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ie",\
"enc_h264_bp_level_4_1_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.6if",\
"enc_h264_bp_level_4_2_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ig",\
"enc_h264_bp_level_4_720p_60fps_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ih",\
"enc_h264_bp_level_4_1_720p_60fps_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ii",\
"enc_h264_bp_level_4_2_720p_60fps_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6ij",\
"enc_h264_mp_level_3_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes" ,\
"cabac=true bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6ik",\
"enc_h264_mp_level_3_1_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"cabac=true bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6il",\
"enc_h264_mp_level_3_2_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6im",\
"enc_h264_mp_level_4_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6in",\
"enc_h264_mp_level_4_1_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.6io",\
"enc_h264_mp_level_4_2_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ip",\
"enc_h264_mp_level_4_720p_60fps_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6iq",\
"enc_h264_mp_level_4_1_720p_60fps_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ir",\
"enc_h264_mp_level_4_2_720p_60fps_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"cabac=true bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
\
[ "3.6is",\
"enc_h264_hp_level_3_1_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"cabac=true dct8x8=true bitrate=16800" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6it",\
"enc_h264_hp_level_3_2_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6iu",\
"enc_h264_hp_level_4_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6iv",\
"enc_h264_hp_level_4_1_from_videotestsrc_spokes" ,\
"%s pattern=spokes" %DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 cpb-size=62500 qp-min=25" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.6iw",\
"enc_h264_hp_level_4_2_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6ix",\
"enc_h264_hp_level_4_720p_60fps_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes" ,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6iy",\
"enc_h264_hp_level_4_1_720p_60fps_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.6iz",\
"enc_h264_hp_level_4_2_720p_60fps_from_videotestsrc_spokes" ,\
"videotestsrc num-buffers=100 pattern=spokes",\
"cabac=true dct8x8=true bitrate=60000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
\
] #end of test list Table

