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

#this file contains the definition of tests for main profile
# in each table entry:
# [0]: reference/identifier of the test
# [1]: name of the test
# [2]: gstreamer source for the test
# [3]: v4l2enc (gstreamer plugins) properties
# [4]: v4l2enc (gstreamer plugins) capabilities
# [5]: file type of encoded video file ( .mp4 or .avi )

test_list_main_profile= [ \
[ "3.2a", "enc_h264_mp_level_3_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=10000" ,\
"video/x-h264, level=(string)3, width=640, height=480, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.2b",\
"enc_h264_mp_level_3_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=30/1 ! videoscale ! video/x-raw, format=NV12, width=640, height=480",\
"cabac=true bitrate=10000" ,\
"video/x-h264 , level=(string)3, profile=main, pixel-aspect-ratio=1/1",\
".mp4" ] ,\
\
[ "3.2c",\
"enc_h264_mp_level_3_1_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=14000" ,\
"video/x-h264, level=(string)3.1, width=1280, height=720, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.2d",\
"enc_h264_mp_level_3_1_from_file" ,\
DEFAULT_FILESRC_NV21+"! videoparse format=nv21 width=1280 height=720 framerate=30/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"cabac=true bitrate=14000" ,\
"video/x-h264, level=(string)3.1, profile=main",\
".mp4" ] ,\
\
[ "3.2e",\
"enc_h264_mp_level_3_2_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)3.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.2f",\
"enc_h264_mp_level_3_2_from_file" ,\
DEFAULT_FILESRC_NV21+"! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)3.2, profile=main",\
".mp4" ] ,\
\
[ "3.2g",\
"enc_h264_mp_level_4_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.2h",\
"enc_h264_mp_level_4_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=30/1 ! videoscale ! video/x-raw, format=NV12, width=1920, height=1080",\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, profile=main",\
".mp4" ],\
\
[ "3.2i",\
"enc_h264_mp_level_4_1_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000 qp_min=20" ,\
"video/x-h264, level=(string)4.1, width=1920, height=1080, profile=main, framerate=30/1",\
".avi" ] ,\
\
[ "3.2j",\
"enc_h264_mp_level_4_1_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=30/1 ! videoscale ! video/x-raw, format=NV12, width=1920, height=1080",\
"cabac=true bitrate=50000 qp_min=20" ,\
"video/x-h264, level=(string)4.1, profile=main",\
".mp4" ] ,\
\
[ "3.2k",\
"enc_h264_mp_level_4_2_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000 qp_min=20" ,\
"video/x-h264, level=(string)4.2, width=1920, height=1080, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.2l",\
"enc_h264_mp_level_4_2_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoscale ! video/x-raw, format=NV12, width=1920, height=1080",\
"cabac=true bitrate=50000 qp_min=20" ,\
"video/x-h264, level=(string)4.2, profile=main",\
".mp4" ] ,\
\
[ "3.2m",\
"enc_h264_mp_zoom_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+" ! video/x-raw, width=1280, height=720, framerate=60/1 ! videocrop top=180 left=320 bottom=180 right=320 ! videoscale ! video/x-raw, format=NV12, width=1280, height=720 ",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".avi" ] ,\
\
[ "3.2n",\
"enc_h264_mp_zoom_from_file" ,\
DEFAULT_FILESRC+" ! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videocrop top=180 left=320 bottom=180 right=320 ! videoscale ! video/x-raw, format=NV12, width=1280, height=720",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".mp4" ] ,\
\
#[ "3.2o",\
#"enc_h264_mp_white_balance" ,\
#"" ,\
#"" ,\
#"" ,\
#"" ] ,\
\
[ "3.2p",\
"enc_h264_mp_short_from_videotestsrc" ,\
"videotestsrc num-buffers=100 ",\
"cabac=true bitrate=12000" ,\
"video/x-h264, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.2q",\
"enc_h264_mp_short_from_file" ,\
" -v filesrc location= Jets_nv21_1280x720.yuv num-buffers=100 ! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720 ",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".mp4" ] ,\
\
[ "3.2r",\
"enc_h264_mp_long_from_videotestsrc" ,\
"videotestsrc num-buffers=500 ",\
#tmp
#"videotestsrc num-buffers=50 ",\
"cabac=true bitrate=12000" ,\
"video/x-h264, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.2s",\
"enc_h264_mp_long_from_file" ,\
"-v filesrc location= Jets_nv21_1280x720.yuv  num-buffers=500 ! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
#tmp
#"-v filesrc location= Jets_nv21_1280x720.yuv  num-buffers=50 ! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".mp4" ] ,\
\
[ "3.2t",\
"enc_h264_mp_rotation_from_videotestsrc" ,\
"videotestsrc num-buffers=100 ! \"video/x-raw, format=NV12, width=720, height=1280\" ",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.2u",\
"enc_h264_mp_rotation_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoflip method=clockwise ",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".mp4" ] ,\
\
[ "3.2v",\
"enc_h264_mp_until_full" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=12000" ,\
"video/x-h264, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.2w",\
"enc_h264_mp_nv12_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+" ! \"video/x-raw, format=NV12, width=1280, height=720\" ",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".avi" ] ,\
\
[ "3.2x",\
"enc_h264_mp_nv12_from_file" ,\
DEFAULT_FILESRC_NV21+"! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=NV12\"",\
"cabac=true bitrate=12000" ,\
"video/x-h264 , profile=main",\
".mp4" ] ,\
\
[ "3.2y",\
"enc_h264_mp_nv21_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+" ! \"video/x-raw, format=NV21, width=1280, height=720\" ",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".avi" ] ,\
\
[ "3.2z",\
"enc_h264_mp_nv21_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=NV21\"",\
"cabac=true bitrate=12000" ,\
"video/x-h264 , profile=main",\
".mp4" ] ,\
\
[ "3.2aa",\
"enc_h264_mp_uyvy_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+" ! \"video/x-raw, format=UYVY, width=1280, height=720\" ",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".avi" ] ,\
\
[ "3.2ab",\
"enc_h264_mp_uyvy_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=UYVY\"",\
"cabac=true bitrate=12000" ,\
"video/x-h264 , profile=main",\
".mp4" ] ,\
\
[ "3.2ac",\
"enc_h264_mp_vyuy_from_videotestsrc" ,\
VIDEOTESTSRC_FILE_VYUY+"! videoparse format=yuy2 width=1280 height=720 framerate=60/1 ! videoscale ! \"video/x-raw, width=1280, height=736, format=YUY2\"",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".avi" ] ,\
\
[ "3.2ad",\
"enc_h264_mp_vyuy_from_file" ,\
DEFAULT_FILESRC_VYUY+"! videoparse format=yuy2 width=1280 height=720 framerate=60/1 ! videoscale ! \"video/x-raw, width=1280, height=736, format=YUY2\"",\
"cabac=true bitrate=12000" ,\
"video/x-h264 , profile=main",\
".mp4" ] ,\
\
[ "3.2ae",\
"enc_h264_mp_rgb_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=RGB, width=1280, height=720\" ",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".avi" ] ,\
\
[ "3.2af",\
"enc_h264_mp_rgb_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=RGB\"",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".mp4" ] ,\
\
[ "3.2ag",\
"enc_h264_mp_bgr_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=BGR, width=1280, height=720\" ",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".avi" ] ,\
\
[ "3.2ah",\
"enc_h264_mp_bgr_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=BGR\"",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".mp4" ] ,\
\
[ "3.2ai",\
"enc_h264_mp_rgbx_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=RGBx, width=1280, height=720\" ",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".avi" ] ,\
\
[ "3.2aj",\
"enc_h264_mp_rgbx_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=RGBx\"",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".mp4" ] ,\
\
[ "3.2ak",\
"enc_h264_mp_bgrx_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=BGRx, width=1280, height=720\" ",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".avi" ] ,\
\
[ "3.2al",\
"enc_h264_mp_bgrx_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=BGRx\"",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".mp4" ] ,\
\
[ "3.2am",\
"enc_h264_mp_xrgb_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=xRGB, width=1280, height=720\" ",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".avi" ] ,\
\
[ "3.2an",\
"enc_h264_mp_xrgb_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=xRGB\"",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".mp4" ] ,\
\
[ "3.2ao",\
"enc_h264_mp_xbgr_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC+"! \"video/x-raw, format=xBGR, width=1280, height=720\" ",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".avi" ] ,\
\
[ "3.2ap",\
"enc_h264_mp_xbgr_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=nv12 width=1280 height=720 framerate=60/1 ! videoconvert ! \"video/x-raw, format=xBGR\"",\
"cabac=true bitrate=12000" ,\
"video/x-h264, profile=main",\
".mp4" ] ,\
\
[ "3.2aq",\
"enc_h264_mp_level_4_720p_60fps_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.2ar",\
"enc_h264_mp_level_4_720p_60fps_from_file" ,\
DEFAULT_FILESRC_NV21+"! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"cabac=true bitrate=20000" ,\
"video/x-h264, level=(string)4, profile=main",\
".mp4" ],\
\
[ "3.2as",\
"enc_h264_mp_level_4_1_720p_60fps_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000 qp_min=20" ,\
"video/x-h264, level=(string)4.1, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.2at",\
"enc_h264_mp_level_4_1_720p_60fps_from_file" ,\
DEFAULT_FILESRC_NV21+"! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"cabac=true bitrate=50000 qp_min=20" ,\
"video/x-h264, level=(string)4.1, profile=main",\
".mp4" ] ,\
\
[ "3.2au",\
"enc_h264_mp_level_4_2_720p_60fps_from_videotestsrc" ,\
DEFAULT_VIDEOTESTSRC,\
"cabac=true bitrate=50000 qp_min=20" ,\
"video/x-h264, level=(string)4.2, width=1280, height=720, profile=main, framerate=60/1",\
".avi" ] ,\
\
[ "3.2av",\
"enc_h264_mp_level_4_2_720p_60fps_from_file" ,\
DEFAULT_FILESRC_NV21+"! videoparse format=nv21 width=1280 height=720 framerate=60/1 ! videoconvert ! video/x-raw, format=NV12, width=1280, height=720",\
"cabac=true bitrate=50000 qp_min=20" ,\
"video/x-h264, level=(string)4.2, profile=main",\
".mp4" ] ,\
\
\
] #end of main profile test list Table

