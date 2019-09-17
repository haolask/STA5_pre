#!/usr/bin/env python

DEFAULT_VIDEOTESTSRC= " videotestsrc num-buffers=600 "
#DEFAULT_VIDEOTESTSRC= "videotestsrc num-buffers=20 "
DEFAULT_FILESRC= "-v filesrc location= Jets_1280x720_sp.yuv num-buffers=500 "
#DEFAULT_FILESRC= "-v filesrc location= Jets_1280x720_sp.yuv num-buffers=20 "
DEFAULT_FILESRC_NV21= "-v filesrc location= Jets_nv21_1280x720.yuv num-buffers=100 "
#DEFAULT_FILESRC_NV21= "-v filesrc location= Jets_nv21_1280x720.yuv num-buffers=20 "
DEFAULT_FILESRC_VYUY= "-v filesrc location= Jets_vyuy_1280x720.yuv num-buffers=100 "
#DEFAULT_FILESRC_VYUY= "-v filesrc location= Jets_vyuy_1280x720.yuv num-buffers=20 "
VIDEOTESTSRC_FILE_VYUY= "-v filesrc location= videotestscr_vyuy_1280x720.yuv num-buffers=100 "

#this file contains the definition of tests for high profile
# in each table entry:
# [0]: reference/identifier of the test
# [1]: name of the test
# [2]: gstreamer source for the test
# [3]: v4l2enc (gstreamer plugins) properties
# [4]: v4l2enc (gstreamer plugins) capabilities
# [5]: file type of encoded video file ( .mp4 or .avi )

test_list_high_profile= [ \
[ "3.3a", "enc_h264_hp_level_3_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.3b",\
"enc_h264_hp_level_3_from_file" ,\
DEFAULT_FILESRC+" ! videoparse format=nv12 width=1280 height=720 framerate=30/1 ! videoscale ! video/x-raw, format=NV12, width=640, height=480",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264 , level=(string)3, profile=high, pixel-aspect-ratio=1/1",\
".mp4" ] ,\
\
[ "3.3c",\
"enc_h264_hp_level_3_1_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=16800" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.3d",\
"enc_h264_hp_level_3_1_from_file" ,\
DEFAULT_FILESRC_NV21+" ! videoparse format=nv21 width=1280 height=720 framerate=30/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"cabac=true dct8x8=true bitrate=16800" ,\
"video/x-h264, level=(string)3.1, profile=high",\
".mp4" ] ,\
\
[ "3.3e",\
"enc_h264_hp_level_3_2_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.3f",\
"enc_h264_hp_level_3_2_from_file" ,\
DEFAULT_FILESRC_NV21+" ! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)3.2, profile=high",\
".mp4" ] ,\
\
[ "3.3g",\
"enc_h264_hp_level_4_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.3h",\
"enc_h264_hp_level_4_from_file" ,\
DEFAULT_FILESRC+" ! videoparse format=nv12 width=1280 height=720 framerate=30/1 ! videoscale ! video/x-raw, format=NV12, width=1920, height=1080",\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, profile=high",\
".mp4" ],\
\
[ "3.3i",\
"enc_h264_hp_level_4_1_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 cpb-size=60000 qp_min=20" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=high, framerate=30/1",\
".avi" ] ,\
\
[ "3.3j",\
"enc_h264_hp_level_4_1_from_file" ,\
DEFAULT_FILESRC+" ! videoparse format=nv12 width=1280 height=720 framerate=30/1 ! videoscale ! video/x-raw, format=NV12, width=1920, height=1080",\
"cabac=true dct8x8=true bitrate=60000 cpb-size=60000 qp_min=20" ,\
"video/x-h264, level=(string)4.1, profile=high",\
".mp4" ] ,\
\
[ "3.3k",\
"enc_h264_hp_level_4_2_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 qp_min=20" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.3l",\
"enc_h264_hp_level_4_2_from_file" ,\
DEFAULT_FILESRC+" ! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoscale ! video/x-raw, format=NV12, width=1920, height=1080",\
"cabac=true dct8x8=true bitrate=60000 qp_min=20" ,\
"video/x-h264, level=(string)4.2, profile=high",\
".mp4" ] ,\
\
[ "3.3m",\
"enc_h264_hp_zoom_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+" ! video/x-raw, width=1280, height=720, framerate=60/1 ! videocrop top=180 left=320 bottom=180 right=320 ! videoscale ! video/x-raw, format=NV12, width=1280, height=720 ",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, profile=high",\
".avi" ] ,\
\
[ "3.3n",\
"enc_h264_hp_zoom_from_file" ,\
DEFAULT_FILESRC+" ! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videocrop top=180 left=320 bottom=180 right=320 ! videoscale ! video/x-raw, format=NV12, width=1280, height=720",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, profile=high",\
".mp4" ] ,\
\
#[ "3.3o",\
#"enc_h264_hp_white_balance" ,\
#"" ,\
#"" ,\
#"" ,\
#"" ] ,\
\
[ "3.3p",\
"enc_h264_hp_short_from_videotestsrc" ,\
"videotestsrc num-buffers=100 ",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.3q",\
"enc_h264_hp_short_from_file" ,\
" -v filesrc location= Jets_nv21_1280x720.yuv num-buffers=100 ! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, profile=high",\
".mp4" ] ,\
\
[ "3.3r",\
"enc_h264_hp_long_from_videotestsrc" ,\
"videotestsrc num-buffers=500 ",\
#tmp
#"videotestsrc num-buffers=50 ",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.3s",\
"enc_h264_hp_long_from_file" ,\
" -v filesrc location= Jets_nv21_1280x720.yuv  num-buffers=500 ! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
#tmp
#" -v filesrc location= Jets_nv21_1280x720.yuv  num-buffers=50 ! videoparse format=nv21 width=1280 height=720 #framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, profile=high",\
".mp4" ] ,\
\
[ "3.3t",\
"enc_h264_hp_rotation_from_videotestsrc" ,\
"videotestsrc num-buffers=100 ! \"video/x-raw, format=NV12, width=720, height=1280\" ",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.3u",\
"enc_h264_hp_rotation_from_file" ,\
DEFAULT_FILESRC+" ! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoflip method=clockwise ",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, profile=high",\
".mp4" ] ,\
\
[ "3.3v",\
"enc_h264_hp_until_full" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.3w",\
"enc_h264_hp_nv12_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+" ! \"video/x-raw, format=NV12, width=1280, height=720\" ",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, profile=high",\
".avi" ] ,\
\
[ "3.3x",\
"enc_h264_hp_nv12_from_file" ,\
DEFAULT_FILESRC_NV21+" ! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=NV12\"",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264 , profile=high",\
".mp4" ] ,\
\
[ "3.3y",\
"enc_h264_hp_nv21_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+" ! \"video/x-raw, format=NV21, width=1280, height=720\" ",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, profile=high",\
".avi" ] ,\
\
[ "3.3z",\
"enc_h264_hp_nv21_from_file" ,\
DEFAULT_FILESRC+" ! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=NV21\"",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264 , profile=high",\
".mp4" ] ,\
\
[ "3.3aa",\
"enc_h264_hp_uyvy_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=UYVY, width=1280, height=720\" ",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, profile=high",\
".avi" ] ,\
\
[ "3.3ab",\
"enc_h264_hp_uyvy_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=UYVY\"",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264 , profile=high",\
".mp4" ] ,\
\
[ "3.3ac",\
"enc_h264_hp_vyuy_from_videotestsrc" ,\
VIDEOTESTSRC_FILE_VYUY+"! videoparse format=yuy2 width=1280 height=720 framerate=60/1 ! videoscale ! \"video/x-raw, width=1280, height=736, format=YUY2\"",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, profile=high",\
".avi" ] ,\
\
[ "3.3ad",\
"enc_h264_hp_vyuy_from_file" ,\
DEFAULT_FILESRC_VYUY+"! videoparse format=yuy2 width=1280 height=720 framerate=60/1 ! videoscale ! \"video/x-raw, width=1280, height=736, format=YUY2\"",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264 , profile=high",\
".mp4" ] ,\
\
[ "3.3ae",\
"enc_h264_hp_rgb_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=RGB, width=1280, height=720\" ",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, profile=high",\
".avi" ] ,\
\
[ "3.3af",\
"enc_h264_hp_rgb_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=RGB\"",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264 , profile=high",\
".mp4" ] ,\
\
[ "3.3ag",\
"enc_h264_hp_bgr_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=BGR, width=1280, height=720\" ",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, profile=high",\
".avi" ] ,\
\
[ "3.3ah",\
"enc_h264_hp_bgr_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=BGR\"",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264 , profile=high",\
".mp4" ] ,\
\
[ "3.3ai",\
"enc_h264_hp_rgbx_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=RGBx, width=1280, height=720\" ",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, profile=high",\
".avi" ] ,\
\
[ "3.3aj",\
"enc_h264_hp_rgbx_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=RGBx\"",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264 , profile=high",\
".mp4" ] ,\
\
[ "3.3ak",\
"enc_h264_hp_bgrx_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=BGRx, width=1280, height=720\" ",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, profile=high",\
".avi" ] ,\
\
[ "3.3al",\
"enc_h264_hp_bgrx_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=BGRx\"",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264 , profile=high",\
".mp4" ] ,\
\
[ "3.3am",\
"enc_h264_hp_xrgb_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=xRGB, width=1280, height=720\" ",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, profile=high",\
".avi" ] ,\
\
[ "3.3an",\
"enc_h264_hp_xrgb_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=xRGB\"",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264 , profile=high",\
".mp4" ] ,\
\
[ "3.3ao",\
"enc_h264_hp_xbgr_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=xBGR, width=1280, height=720\" ",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264, profile=high",\
".avi" ] ,\
\
[ "3.3ap",\
"enc_h264_hp_xbgr_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=xBGR\"",\
"cabac=true dct8x8=true bitrate=12000" ,\
"video/x-h264 , profile=high",\
".mp4" ] ,\
\
[ "3.3aq",\
"enc_h264_hp_level_4_720p_60fps_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.3ar",\
"enc_h264_hp_level_4_720p_60fps_from_file" ,\
DEFAULT_FILESRC_NV21+"! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"cabac=true dct8x8=true bitrate=24000" ,\
"video/x-h264, level=(string)4, profile=high",\
".mp4" ],\
\
[ "3.3as",\
"enc_h264_hp_level_4_1_720p_60fps_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 qp_min=20" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.3at",\
"enc_h264_hp_level_4_1_720p_60fps_from_file" ,\
DEFAULT_FILESRC_NV21+"! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"cabac=true dct8x8=true bitrate=60000 qp_min=20" ,\
"video/x-h264, level=(string)4.1, profile=high",\
".mp4" ] ,\
\
[ "3.3au",\
"enc_h264_hp_level_4_2_720p_60fps_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true dct8x8=true bitrate=60000 qp_min=20" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=high, framerate=60/1",\
".avi" ] ,\
\
[ "3.3av",\
"enc_h264_hp_level_4_2_720p_60fps_from_file" ,\
DEFAULT_FILESRC_NV21+"! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"cabac=true dct8x8=true bitrate=60000 qp_min=20" ,\
"video/x-h264, level=(string)4.2, profile=high",\
".mp4" ] ,\
\
\
] #end of high profile test list Table

