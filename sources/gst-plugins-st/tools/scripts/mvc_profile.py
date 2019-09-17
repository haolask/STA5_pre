#!/usr/bin/env python

DEFAULT_FILESRC= "-v filesrc location= bbb_sunflower_1280_1440_YUY2.yuv num-buffers=600 "
DEFAULT_ENCPARAM = "cabac=true bitrate=12000"

#this file contains the definition of tests for mvc
# in each table entry:
# [0]: reference/identifier of the test
# [1]: name of the test
# [2]: gstreamer source for the test
# [3]: v4l2enc (gstreamer plugins) properties
# [4]: v4l2enc (gstreamer plugins) capabilities
# [5]: file type of encoded video file ( .mp4 or .avi )

test_list_mvc_profile= [ \
[ "3.4a",\
"enc_h264_mvc_nv12_from_file" ,\
DEFAULT_FILESRC+" ! videoparse format=yuy2 width=1280 height=1440 framerate=30/1 ! videoconvert ! \"video/x-raw, format=NV12\"",\
DEFAULT_ENCPARAM ,\
"video/x-h264 , profile=stereo",\
".mp4" ] ,\
\
[ "3.4b",\
"enc_h264_mvc_nv21_from_file" ,\
DEFAULT_FILESRC+" ! videoparse format=yuy2 width=1280 height=1440 framerate=30/1 ! videoconvert ! \"video/x-raw, format=NV21\"",\
DEFAULT_ENCPARAM ,\
"video/x-h264 , profile=stereo",\
".mp4" ] ,\
\
[ "3.4c",\
"enc_h264_mvc_uyvy_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=yuy2 width=1280 height=1440 framerate=30/1 ! videoconvert ! \"video/x-raw, format=UYVY\"",\
DEFAULT_ENCPARAM ,\
"video/x-h264 , profile=stereo",\
".mp4" ] ,\
\
[ "3.4d",\
"enc_h264_mvc_rgb_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=yuy2 width=1280 height=1440 framerate=30/1 ! videoconvert ! \"video/x-raw, format=RGB\"",\
DEFAULT_ENCPARAM ,\
"video/x-h264 , profile=stereo",\
".mp4" ] ,\
\
[ "3.4e",\
"enc_h264_mvc_bgr_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=yuy2 width=1280 height=1440 framerate=30/1 ! videoconvert ! \"video/x-raw, format=BGR\"",\
DEFAULT_ENCPARAM ,\
"video/x-h264 , profile=stereo",\
".mp4" ] ,\
\
[ "3.4f",\
"enc_h264_mvc_rgbx_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=yuy2 width=1280 height=1440 framerate=30/1 ! videoconvert ! \"video/x-raw, format=RGBx\"",\
DEFAULT_ENCPARAM ,\
"video/x-h264 , profile=stereo",\
".mp4" ] ,\
\
[ "3.4g",\
"enc_h264_mvc_bgrx_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=yuy2 width=1280 height=1440 framerate=30/1 ! videoconvert ! \"video/x-raw, format=BGRx\"",\
DEFAULT_ENCPARAM ,\
"video/x-h264 , profile=stereo",\
".mp4" ] ,\
\
[ "3.4h",\
"enc_h264_mvc_xrgb_from_file" ,\
DEFAULT_FILESRC+"! videoparse format=yuy2 width=1280 height=1440 framerate=30/1 ! videoconvert ! \"video/x-raw, format=xRGB\"",\
DEFAULT_ENCPARAM ,\
"video/x-h264 , profile=stereo",\
".mp4" ] ,\
\
\
] #end of brc test list Table

