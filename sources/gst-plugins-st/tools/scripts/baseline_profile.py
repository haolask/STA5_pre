#!/usr/bin/env python

DEFAULT_VIDEOTESTSRC= " videotestsrc num-buffers=100 "
#DEFAULT_VIDEOTESTSRC= "videotestsrc num-buffers=20 "
DEFAULT_FILESRC= "-v filesrc location= Jets_1280x720_sp.yuv num-buffers=100 "
#DEFAULT_FILESRC= "-v filesrc location= Jets_1280x720_sp.yuv num-buffers=20 "
DEFAULT_FILESRC_NV21= "-v filesrc location= Jets_nv21_1280x720.yuv num-buffers=100 "
#DEFAULT_FILESRC_NV21= "-v filesrc location= Jets_nv21_1280x720.yuv num-buffers=20 "
DEFAULT_FILESRC_VYUY= "-v filesrc location= Jets_vyuy_1280x720.yuv num-buffers=100 "
#DEFAULT_FILESRC_VYUY= "-v filesrc location= Jets_vyuy_1280x720.yuv num-buffers=20 "
VIDEOTESTSRC_FILE_VYUY= "-v filesrc location= videotestscr_vyuy_1280x720.yuv num-buffers=100 "


#this file contains the definition of tests for baseline profile
# in each table entry:
# [0]: reference/identifier of the test
# [1]: name of the test
# [2]: gstreamer source for the test
# [3]: v4l2enc (gstreamer plugins) properties
# [4]: v4l2enc (gstreamer plugins) capabilities
# [5]: file type of encoded video file ( .mp4 or .avi )

test_list_baseline_profile= [ \
[ "3.1a", "enc_h264_bp_level_3_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.1b",\
"enc_h264_bp_level_3_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=30/1 ! videoscale ! video/x-raw, format=NV12, width=640, height=480",\
"bitrate=10000" ,\
"video/x-h264 , level=(string)3, profile=baseline, pixel-aspect-ratio=1/1",\
".mp4" ] ,\
\
[ "3.1c",\
"enc_h264_bp_level_3_1_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.1d",\
"enc_h264_bp_level_3_1_from_file" ,\
DEFAULT_FILESRC_NV21+"! videoparse format=nv21 width=1280 height=720 framerate=30/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"bitrate=14000" ,\
"video/x-h264, level=(string)3.1, profile=baseline",\
".mp4" ] ,\
\
[ "3.1e",\
"enc_h264_bp_level_3_2_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.1f",\
"enc_h264_bp_level_3_2_from_file" ,\
DEFAULT_FILESRC_NV21+"! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"bitrate=20000" ,\
"video/x-h264, level=(string)3.2, profile=baseline",\
".mp4" ] ,\
\
[ "3.1g",\
"enc_h264_bp_level_4_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.1h",\
"enc_h264_bp_level_4_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=30/1 ! videoscale ! video/x-raw, format=NV12, width=1920, height=1080",\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, profile=baseline",\
".mp4" ],\
\
[ "3.1i",\
"enc_h264_bp_level_4_1_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=baseline, framerate=30/1",\
".avi" ] ,\
\
[ "3.1j",\
"enc_h264_bp_level_4_1_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=30/1 ! videoscale ! video/x-raw, format=NV12, width=1920, height=1080",\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, profile=baseline",\
".mp4" ] ,\
\
[ "3.1k",\
"enc_h264_bp_level_4_2_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.1l",\
"enc_h264_bp_level_4_2_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoscale ! video/x-raw, format=NV12, width=1920, height=1080",\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, profile=baseline",\
".mp4" ] ,\
\
[ "3.1m",\
"enc_h264_bp_zoom_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! video/x-raw, width=1280, height=720, framerate=60/1 ! videocrop top=180 left=320 bottom=180 right=320 ! videoscale ! video/x-raw, format=NV12, width=1280, height=720 ",\
"bitrate=12000" ,\
"video/x-h264, profile=baseline",\
".avi" ] ,\
\
[ "3.1n",\
"enc_h264_bp_zoom_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videocrop top=180 left=320 bottom=180 right=320 ! videoscale ! video/x-raw, format=NV12, width=1280, height=720",\
"bitrate=12000" ,\
"video/x-h264, profile=baseline",\
".mp4" ] ,\
\
#[ "3.1o",\
#"enc_h264_bp_white_balance" ,\
#"" ,\
#"" ,\
#"" ,\
#"" ] ,\
\
[ "3.1p",\
"enc_h264_bp_short_from_videotestsrc" ,\
"videotestsrc num-buffers=100 ",\
"bitrate=12000" ,\
"video/x-h264, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.1q",\
"enc_h264_bp_short_from_file" ,\
" -v filesrc location= Jets_nv21_1280x720.yuv num-buffers=10 ! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"bitrate=12000" ,\
"video/x-h264, profile=baseline",\
".mp4" ] ,\
\
[ "3.1r",\
"enc_h264_bp_long_from_videotestsrc" ,\
"videotestsrc num-buffers=500 ",\
#tmp
#"videotestsrc num-buffers=50 ",\
"bitrate=12000" ,\
"video/x-h264, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.1s",\
"enc_h264_bp_long_from_file" ,\
" -v filesrc location= Jets_nv21_1280x720.yuv  num-buffers=500 ! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
#tmp
#" -v filesrc location= Jets_nv21_1280x720.yuv  num-buffers=50 ! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"bitrate=12000" ,\
"video/x-h264, profile=baseline",\
".mp4" ] ,\
\
[ "3.1t",\
"enc_h264_bp_rotation_from_videotestsrc" ,\
"videotestsrc num-buffers=100 ! \"video/x-raw, format=NV12, width=720, height=1280\" ",\
"bitrate=12000" ,\
"video/x-h264, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.1u",\
"enc_h264_bp_rotation_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoflip method=clockwise ",\
"bitrate=12000" ,\
"video/x-h264, profile=baseline",\
".mp4" ] ,\
\
[ "3.1v",\
"enc_h264_bp_until_full" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=12000" ,\
"video/x-h264, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.1w",\
"enc_h264_bp_nv12_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=NV12, width=1280, height=720\" ",\
"bitrate=12000" ,\
"video/x-h264, profile=baseline",\
".avi" ] ,\
\
[ "3.1x",\
"enc_h264_bp_nv12_from_file" ,\
DEFAULT_FILESRC_NV21+"! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=NV12\"",\
"bitrate=12000" ,\
"video/x-h264 , profile=baseline",\
".mp4" ] ,\
\
[ "3.1y",\
"enc_h264_bp_nv21_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=NV21, width=1280, height=720\" ",\
"bitrate=12000" ,\
"video/x-h264, profile=baseline",\
".avi" ] ,\
\
[ "3.1z",\
"enc_h264_bp_nv21_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=NV21\"",\
"bitrate=12000" ,\
"video/x-h264 , profile=baseline",\
".mp4" ] ,\
\
[ "3.1aa",\
"enc_h264_bp_uyvy_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=UYVY, width=1280, height=720\" ",\
"bitrate=12000" ,\
"video/x-h264, profile=baseline",\
".avi" ] ,\
\
[ "3.1ab",\
"enc_h264_bp_uyvy_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=UYVY\"",\
"bitrate=12000" ,\
"video/x-h264 , profile=baseline",\
".mp4" ] ,\
\
[ "3.1ac",\
"enc_h264_bp_vyuy_from_videotestsrc" ,\
VIDEOTESTSRC_FILE_VYUY+"! videoparse format=yuy2 width=1280 height=720 framerate=60/1 ! videoscale ! \"video/x-raw, width=1280, height=736, format=YUY2\"",\
"bitrate=12000" ,\
"video/x-h264, profile=baseline",\
".avi" ] ,\
\
[ "3.1ad",\
"enc_h264_bp_vyuy_from_file" ,\
DEFAULT_FILESRC_VYUY+"! videoparse format=yuy2 width=1280 height=720 framerate=60/1 ! videoscale ! \"video/x-raw, width=1280, height=736, format=YUY2\"",\
"bitrate=12000" ,\
"video/x-h264 , profile=baseline",\
".mp4" ] ,\
\
[ "3.1ae",\
"enc_h264_bp_rgb_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=RGB, width=1280, height=720\" ",\
"bitrate=12000" ,\
"video/x-h264, profile=baseline",\
".avi" ] ,\
\
[ "3.1af",\
"enc_h264_bp_rgb_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=RGB\"",\
"bitrate=12000" ,\
"video/x-h264 , profile=baseline",\
".mp4" ] ,\
\
[ "3.1ag",\
"enc_h264_bp_bgr_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=BGR, width=1280, height=720\" ",\
"bitrate=12000" ,\
"video/x-h264, profile=baseline",\
".avi" ] ,\
\
[ "3.1ah",\
"enc_h264_bp_bgr_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=BGR\"",\
"bitrate=12000" ,\
"video/x-h264 , profile=baseline",\
".mp4" ] ,\
\
[ "3.1ai",\
"enc_h264_bp_rgbx_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=RGBx, width=1280, height=720\" ",\
"bitrate=12000" ,\
"video/x-h264, profile=baseline",\
".avi" ] ,\
\
[ "3.1aj",\
"enc_h264_bp_rgbx_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=RGBx\"",\
"bitrate=12000" ,\
"video/x-h264 , profile=baseline",\
".mp4" ] ,\
\
[ "3.1ak",\
"enc_h264_bp_bgrx_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=BGRx, width=1280, height=720\" ",\
"bitrate=12000" ,\
"video/x-h264, profile=baseline",\
".avi" ] ,\
\
[ "3.1al",\
"enc_h264_bp_bgrx_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=BGRx\"",\
"bitrate=12000" ,\
"video/x-h264 , profile=baseline",\
".mp4" ] ,\
\
[ "3.1am",\
"enc_h264_bp_xrgb_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=xRGB, width=1280, height=720\" ",\
"bitrate=12000" ,\
"video/x-h264, profile=baseline",\
".avi" ] ,\
\
[ "3.1an",\
"enc_h264_bp_xrgb_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=xRGB\"",\
"bitrate=12000" ,\
"video/x-h264 , profile=baseline",\
".mp4" ] ,\
\
[ "3.1ao",\
"enc_h264_bp_xbgr_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=xBGR, width=1280, height=720\" ",\
"bitrate=12000" ,\
"video/x-h264, profile=baseline",\
".avi" ] ,\
\
[ "3.1ap",\
"enc_h264_bp_xbgr_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=xBGR\"",\
"bitrate=12000" ,\
"video/x-h264 , profile=baseline",\
".mp4" ] ,\
\
[ "3.1aq",\
"enc_h264_bp_level_4_720p_60fps_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.1ar",\
"enc_h264_bp_level_4_720p_60fps_from_file" ,\
DEFAULT_FILESRC_NV21+"! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"bitrate=20000" ,\
"video/x-h264, level=(string)4, profile=baseline",\
".mp4" ],\
\
[ "3.1as",\
"enc_h264_bp_level_4_1_720p_60fps_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.1at",\
"enc_h264_bp_level_4_1_720p_60fps_from_file" ,\
DEFAULT_FILESRC_NV21+"! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.1, profile=baseline",\
".mp4" ] ,\
\
[ "3.1au",\
"enc_h264_bp_level_4_2_720p_60fps_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=baseline, framerate=60/1",\
".avi" ] ,\
\
[ "3.1av",\
"enc_h264_bp_level_4_2_720p_60fps_from_file" ,\
DEFAULT_FILESRC_NV21+"! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"bitrate=50000" ,\
"video/x-h264, level=(string)4.2, profile=baseline",\
".mp4" ] ,\
\
\
] #end of baseline profile test list Table

