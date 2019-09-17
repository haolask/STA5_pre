/*
 * Copyright (C) STMicroelectronics SA 2015
 * Author: Hugues Fruchet <hugues.fruchet@st.com> for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#ifndef G1_CFG_H
#define G1_CFG_H

/* guard value for pixel budget */
#define G1_MAX_RESO		(1920 * 1088)

#define G1_MIN_WIDTH	48
#define G1_MAX_WIDTH	1920
#define G1_MIN_HEIGHT	48
#define G1_MAX_HEIGHT	1920

/* guard value for a single AU size (width x height):
 * for a stream encoded in YUV420 pixel format, assuming a compression
 * ratio of 1/4, the maximum size of an AU is (width x height) x 3 / 4
 */
#define G1_MAX_AU_SIZE	(G1_MAX_RESO * 3 / 4)

/* guard value for number of AUs */
#define G1_MAX_AUS		10

/* By default the decoder works using DTS timestamp
 * Kernel flag  in Kconfig CONFIG_VIDEO_ST_G1_PTS_BASED is set to 'n'
 * Choice fully depends on multimedia framework using V4L2 API.
 */
#ifdef CONFIG_VIDEO_ST_G1_PTS_BASED
/* In this configuration, AU buffer timestamp indicates
 * the presentation time
 */
#define G1_DEFAULT_TIMESTAMP G1_PRESENTATION_TIMESTAMP
#else
/* In this configuration, AU buffer timestamp indicates
 * the decoding time
 */
#define G1_DEFAULT_TIMESTAMP G1_DECODING_TIMESTAMP
#endif

/* guard output frame count */
#define G1_MIN_FRAME_USER	3
#define G1_MAX_FRAME_USER	16  /* platform/use-case dependent */
#define G1_PEAK_FRAME_SMOOTHING	0  /* IP perf dependent, can be tuned */
#define G1_MAX_FRAMES (G1_PEAK_FRAME_SMOOTHING + G1_MAX_FRAME_USER)

#ifndef VIDEO_MAX_FRAME
#error "VIDEO_MAX_FRAME not defined, please include videodev2.h"
#endif
#if G1_MAX_FRAMES > VIDEO_MAX_FRAME
#undef G1_MAX_FRAMES
#define G1_MAX_FRAMES (VIDEO_MAX_FRAME)
#endif

#define G1_MAX_CTRL_NUM      7

/* extra space to be allocated to store codec specific data per frame */
#define G1_MAX_FRAME_PRIV_SIZE 100

/* PM runtime auto power-off after 5ms of inactivity */
#define G1_HW_AUTOSUSPEND_DELAY_MS	5

/* In case of misalignement on 16 pixels for the output frame,
 * we have two choices : either we update width or height to
 * the first aligned value above or the first aligned value below
 * This flag when defined set to the first aligned value below.
 * Comment following line to force alignment on the first aligned value above
 * (refer to try_fmt_frame )
 */
#define G1_FRAME_FIRST_ALIGNED_VALUE_BELOW  1

/* useDisplaySmooothing : (H264 only)
 * -------------------------------------
 * flag to enable extra buffering in DPB output so that driver may read
 * output pictures one by one.
 * If set to false : it means that driver MUST get every available frames
 * from the G1 while H264DecNextPicture indicates a frame is ready.
 * It may be up to DPB number of frames to get before starting to decode a
 * new IDR Access unit.
 * Consequence :
 * in case of 'real time' streaming (AU are sent without any
 * buffering in real time), read several frames in loop may introduce
 * delay higher than the frame duration => if the number of preallocated
 * output (frame) buffers is lower than the total number of frames (DPB)
 * to get before starting to decode an IDR AU, then driver needs to wait
 * for buffer availability before continuing to get frames from G1.
 */
#define G1_H264_USE_DISPLAY_SMOOTHING   1

#ifdef CONFIG_VIDEO_ST_G1_VP8
extern const struct g1_dec vp8dec;
#endif

#ifdef CONFIG_VIDEO_ST_G1_H264
extern const struct g1_dec h264dec;
#endif

#ifdef CONFIG_VIDEO_ST_G1_VC1
extern const struct g1_dec vc1dec;
#endif

#ifdef CONFIG_VIDEO_ST_G1_MPEG4
extern const struct g1_dec mp4dec;
#endif

#ifdef CONFIG_VIDEO_ST_G1_MPEG2
extern const struct g1_dec mpeg2dec;
#endif

#ifdef CONFIG_VIDEO_ST_G1_AVS
extern const struct g1_dec avsdec;
#endif

#ifdef CONFIG_VIDEO_ST_G1_RAW
extern const struct g1_dec rawdec;
#endif

#ifdef CONFIG_VIDEO_ST_G1_JPEG
extern const struct g1_dec jpegdec;
#endif

#ifdef CONFIG_VIDEO_ST_G1_RV
extern const struct g1_dec rvdec;
#endif

#endif /* G1_CFG_H */
