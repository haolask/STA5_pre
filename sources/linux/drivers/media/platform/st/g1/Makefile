obj-$(CONFIG_VIDEO_ST_G1) := g1.o
g1-y := g1-v4l2.o g1-mem.o g1-debug.o g1-hw.o g1-hw-cfg.o g1-dwl.o

# stack common support
g1-y += stack/common/regdrv.o
g1-y += stack/common/refbuffer.o
g1-y += stack/common/tiledref.o
g1-y += stack/common/bqueue.o
g1-y += stack/common/commonconfig.o
g1-y += stack/common/workaround.o
g1-y += stack/common/errorhandling.o
g1-y += stack/common/fifo.o
ccflags-y += -Idrivers/media/platform/st/g1/stack/inc
ccflags-y += -Idrivers/media/platform/st/g1/stack/common
ccflags-y += -Idrivers/media/platform/st/g1/stack/config
ccflags-y += -Idrivers/rpmsg
ccflags-y += -DFIFO_DATATYPE=i32
ccflags-y += -D_ASSERT_USED
ccflags-y += -D"ASSERT(expr)=WARN_ON(!(expr))"
ccflags-y += -D_ERROR_PRINT
ccflags-y += -D"ERROR_PRINT(expr)=pr_err(\"g1: \" expr)"
ccflags-y += -D"EPRINT(expr)=pr_err(\"g1: \" expr)"
ccflags-y += -D_DEBUG_PRINT
ccflags-y += -D"DEBUG_PRINT(expr)=pr_debug expr"
ccflags-y += -D"DPRINT(expr)=pr_debug expr"

# stack VP8 support
g1-$(CONFIG_VIDEO_ST_G1_VP8) += stack/vp8/vp8decapi.o
g1-$(CONFIG_VIDEO_ST_G1_VP8) += stack/vp8/vp8hwd_pp_pipeline.o
g1-$(CONFIG_VIDEO_ST_G1_VP8) += stack/vp8/vp8hwd_asic.o
g1-$(CONFIG_VIDEO_ST_G1_VP8) += stack/vp8/vp8hwd_error.o
g1-$(CONFIG_VIDEO_ST_G1_VP8) += stack/vp8/vp8hwd_bool.o
g1-$(CONFIG_VIDEO_ST_G1_VP8) += stack/vp8/vp8hwd_probs.o
g1-$(CONFIG_VIDEO_ST_G1_VP8) += stack/vp8/vp8hwd_headers.o
g1-$(CONFIG_VIDEO_ST_G1_VP8) += stack/vp8/vp8hwd_decoder.o
g1-$(CONFIG_VIDEO_ST_G1_VP8) += stack/vp8/vp8hwd_buffer_queue.o
ccflags-$(CONFIG_VIDEO_ST_G1_VP8) += -Idrivers/media/platform/st/g1/stack/vp8
ccflags-$(CONFIG_VIDEO_ST_G1_VP8) += -DVP8DEC_TRACE
ifeq ($(CONFIG_VIDEO_ST_G1_PP),y)
ccflags-$(CONFIG_VIDEO_ST_G1_VP8) += -DPP_VP8DEC_PIPELINE_SUPPORT
endif

# VP8 support
g1-$(CONFIG_VIDEO_ST_G1_VP8) += g1-vp8-dec.o

# stack H264 support
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/h264decapi.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/h264hwd_asic.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/h264hwd_cabac.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/h264hwd_cavlc.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/h264hwd_conceal.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/h264hwd_decoder.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/h264hwd_dpb.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/h264hwd_dpb_lock.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/h264hwd_inter_prediction.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/h264hwd_intra_prediction.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/h264hwd_macroblock_layer.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/h264hwd_slice_data.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/h264hwd_storage.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/h264_pp_pipeline.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/legacy/h264hwd_byte_stream.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/legacy/h264hwd_nal_unit.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/legacy/h264hwd_neighbour.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/legacy/h264hwd_pic_order_cnt.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/legacy/h264hwd_pic_param_set.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/legacy/h264hwd_seq_param_set.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/legacy/h264hwd_slice_group_map.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/legacy/h264hwd_slice_header.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/legacy/h264hwd_stream.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/legacy/h264hwd_util.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/legacy/h264hwd_vlc.o
g1-$(CONFIG_VIDEO_ST_G1_H264) += stack/h264high/legacy/h264hwd_vui.o
ccflags-$(CONFIG_VIDEO_ST_G1_H264) += -Idrivers/media/platform/st/g1/stack/h264high
ccflags-$(CONFIG_VIDEO_ST_G1_H264) += -Idrivers/media/platform/st/g1/stack/h264high/legacy
ccflags-$(CONFIG_VIDEO_ST_G1_H264) += -DH264DEC_TRACE
ifeq ($(CONFIG_VIDEO_ST_G1_PP),y)
ccflags-$(CONFIG_VIDEO_ST_G1_H264) += -DPP_H264DEC_PIPELINE_SUPPORT
endif

# H264 support
g1-$(CONFIG_VIDEO_ST_G1_H264) += g1-h264-dec.o

# stack VC1 support
g1-$(CONFIG_VIDEO_ST_G1_VC1) += stack/vc1/vc1decapi.o
g1-$(CONFIG_VIDEO_ST_G1_VC1) += stack/vc1/vc1hwd_asic.o
g1-$(CONFIG_VIDEO_ST_G1_VC1) += stack/vc1/vc1hwd_bitplane.o
g1-$(CONFIG_VIDEO_ST_G1_VC1) += stack/vc1/vc1hwd_decoder.o
g1-$(CONFIG_VIDEO_ST_G1_VC1) += stack/vc1/vc1hwd_headers.o
g1-$(CONFIG_VIDEO_ST_G1_VC1) += stack/vc1/vc1hwd_picture_layer.o
g1-$(CONFIG_VIDEO_ST_G1_VC1) += stack/vc1/vc1hwd_stream.o
g1-$(CONFIG_VIDEO_ST_G1_VC1) += stack/vc1/vc1hwd_vlc.o
ccflags-$(CONFIG_VIDEO_ST_G1_VC1) += -Idrivers/media/platform/st/g1/stack/vc1
ccflags-$(CONFIG_VIDEO_ST_G1_VC1) += -D_VC1DEC_TRACE
ifeq ($(CONFIG_VIDEO_ST_G1_PP),y)
ccflags-$(CONFIG_VIDEO_ST_G1_VC1) += -DPP_VC1DEC_PIPELINE_SUPPORT
endif
# VC1 support
g1-$(CONFIG_VIDEO_ST_G1_VC1) += g1-vc1-dec.o

# stack MPEG4/H263/Sorenson Spark/DivX support
g1-$(CONFIG_VIDEO_ST_G1_MPEG4) += stack/mpeg4/mp4decapi.o
g1-$(CONFIG_VIDEO_ST_G1_MPEG4) += stack/mpeg4/mp4decapi_internal.o
g1-$(CONFIG_VIDEO_ST_G1_MPEG4) += stack/mpeg4/mp4dechwd_error_conceal.o
g1-$(CONFIG_VIDEO_ST_G1_MPEG4) += stack/mpeg4/mp4dechwd_custom.o
g1-$(CONFIG_VIDEO_ST_G1_MPEG4) += stack/mpeg4/mp4dechwd_headers.o
g1-$(CONFIG_VIDEO_ST_G1_MPEG4) += stack/mpeg4/mp4dechwd_motiontexture.o
g1-$(CONFIG_VIDEO_ST_G1_MPEG4) += stack/mpeg4/mp4dechwd_rvlc.o
g1-$(CONFIG_VIDEO_ST_G1_MPEG4) += stack/mpeg4/mp4dechwd_shortvideo.o
g1-$(CONFIG_VIDEO_ST_G1_MPEG4) += stack/mpeg4/mp4dechwd_strmdec.o
g1-$(CONFIG_VIDEO_ST_G1_MPEG4) += stack/mpeg4/mp4dechwd_utils.o
g1-$(CONFIG_VIDEO_ST_G1_MPEG4) += stack/mpeg4/mp4dechwd_videopacket.o
g1-$(CONFIG_VIDEO_ST_G1_MPEG4) += stack/mpeg4/mp4dechwd_vlc.o
g1-$(CONFIG_VIDEO_ST_G1_MPEG4) += stack/mpeg4/mp4dechwd_vop.o
ccflags-$(CONFIG_VIDEO_ST_G1_MPEG4) += -Idrivers/media/platform/st/g1/stack/mpeg4
ccflags-$(CONFIG_VIDEO_ST_G1_MPEG4) += -DMP4DEC_TRACE
ifeq ($(CONFIG_VIDEO_ST_G1_PP),y)
ccflags-$(CONFIG_VIDEO_ST_G1_MPEG4) += -DPP_MPEG4DEC_PIPELINE_SUPPORT
endif
# MPEG4 support
g1-$(CONFIG_VIDEO_ST_G1_MPEG4) += g1-mpeg4-dec.o

# stack MPEG2/MPEG1 support
g1-$(CONFIG_VIDEO_ST_G1_MPEG2) += stack/mpeg2/mpeg2decapi.o
g1-$(CONFIG_VIDEO_ST_G1_MPEG2) += stack/mpeg2/mpeg2decapi_internal.o
g1-$(CONFIG_VIDEO_ST_G1_MPEG2) += stack/mpeg2/mpeg2hwd_headers.o
g1-$(CONFIG_VIDEO_ST_G1_MPEG2) += stack/mpeg2/mpeg2hwd_strm.o
g1-$(CONFIG_VIDEO_ST_G1_MPEG2) += stack/mpeg2/mpeg2hwd_utils.o
ccflags-$(CONFIG_VIDEO_ST_G1_MPEG2) += -Idrivers/media/platform/st/g1/stack/mpeg2
ccflags-$(CONFIG_VIDEO_ST_G1_MPEG2) += -DMPEG2DEC_TRACE
ifeq ($(CONFIG_VIDEO_ST_G1_PP),y)
ccflags-$(CONFIG_VIDEO_ST_G1_MPEG2) += -DPP_MPEG2DEC_PIPELINE_SUPPORT
endif
# MPEG2 support
g1-$(CONFIG_VIDEO_ST_G1_MPEG2) += g1-mpeg2-dec.o

# stack AVS support
g1-$(CONFIG_VIDEO_ST_G1_AVS) += stack/avs/avs_vlc.o
g1-$(CONFIG_VIDEO_ST_G1_AVS) += stack/avs/avs_headers.o
g1-$(CONFIG_VIDEO_ST_G1_AVS) += stack/avs/avsdecapi_internal.o
g1-$(CONFIG_VIDEO_ST_G1_AVS) += stack/avs/avs_utils.o
g1-$(CONFIG_VIDEO_ST_G1_AVS) += stack/avs/avs_strm.o
g1-$(CONFIG_VIDEO_ST_G1_AVS) += stack/avs/avsdecapi.o
ccflags-$(CONFIG_VIDEO_ST_G1_AVS) += -Idrivers/media/platform/st/g1/stack/avs
ccflags-$(CONFIG_VIDEO_ST_G1_AVS) += -DAVSDEC_TRACE
ifeq ($(CONFIG_VIDEO_ST_G1_PP),y)
ccflags-$(CONFIG_VIDEO_ST_G1_AVS) += -DPP_AVSDEC_PIPELINE_SUPPORT
endif
# AVS support
g1-$(CONFIG_VIDEO_ST_G1_AVS) += g1-avs-dec.o

# PP is also used in standalone to provide scaler services
g1-$(CONFIG_VIDEO_ST_G1_RAW) += g1-raw-dec.o

# stack JPEG support
g1-$(CONFIG_VIDEO_ST_G1_JPEG) += stack/jpeg/jpegdecapi.o
g1-$(CONFIG_VIDEO_ST_G1_JPEG) += stack/jpeg/jpegdecutils.o
g1-$(CONFIG_VIDEO_ST_G1_JPEG) += stack/jpeg/jpegdecscan.o
g1-$(CONFIG_VIDEO_ST_G1_JPEG) += stack/jpeg/jpegdechdrs.o
g1-$(CONFIG_VIDEO_ST_G1_JPEG) += stack/jpeg/jpegdecinternal.o
ccflags-$(CONFIG_VIDEO_ST_G1_JPEG) += -Idrivers/media/platform/st/g1/stack/jpeg
ccflags-$(CONFIG_VIDEO_ST_G1_JPEG) += -DJPEGDEC_TRACE
ifeq ($(CONFIG_VIDEO_ST_G1_PP),y)
ccflags-$(CONFIG_VIDEO_ST_G1_JPEG) += -DPP_JPEGDEC_PIPELINE_SUPPORT
endif
# JPEG support
g1-$(CONFIG_VIDEO_ST_G1_JPEG) += g1-jpeg-dec.o

#stack RV support
g1-$(CONFIG_VIDEO_ST_G1_RV) +=  stack/rv/on2rvdecapi.o
g1-$(CONFIG_VIDEO_ST_G1_RV) +=  stack/rv/rvdecapi.o
g1-$(CONFIG_VIDEO_ST_G1_RV) +=  stack/rv/rvdecapi_internal.o
g1-$(CONFIG_VIDEO_ST_G1_RV) +=  stack/rv/rv_headers.o
g1-$(CONFIG_VIDEO_ST_G1_RV) +=  stack/rv/rv_rpr.o
g1-$(CONFIG_VIDEO_ST_G1_RV) +=  stack/rv/rv_strm.o
g1-$(CONFIG_VIDEO_ST_G1_RV) +=  stack/rv/rv_utils.o
ccflags-$(CONFIG_VIDEO_ST_G1_RV) += -Idrivers/media/platform/st/g1/stack/rv
ccflags-$(CONFIG_VIDEO_ST_G1_RV) += -DRVDEC_TRACE
#ccflags-$(CONFIG_VIDEO_ST_G1_RV) += -DRV_RAW_STREAM_SUPPORT
ifeq ($(CONFIG_VIDEO_ST_G1_PP),y)
ccflags-$(CONFIG_VIDEO_ST_G1_RV) += -DPP_RVDEC_PIPELINE_SUPPORT
endif
# RealVideo (RV30/RV40) support
g1-$(CONFIG_VIDEO_ST_G1_RV) += g1-rv-dec.o

# stack PP support
g1-$(CONFIG_VIDEO_ST_G1_PP) += stack/pp/ppapi.o
g1-$(CONFIG_VIDEO_ST_G1_PP) += stack/pp/ppinternal.o
ccflags-$(CONFIG_VIDEO_ST_G1_PP) += -Idrivers/media/platform/st/g1/stack/pp
ccflags-$(CONFIG_VIDEO_ST_G1_PP) += -DPP_TRACE
ccflags-$(CONFIG_VIDEO_ST_G1_PP) += -D_PPDEBUG_PRINT
ccflags-$(CONFIG_VIDEO_ST_G1_PP) += -D"PPDEBUG_PRINT(expr)=pr_debug expr"
ccflags-$(CONFIG_VIDEO_ST_G1_PP) += -D_TRACE_PP_CTRL
ccflags-$(CONFIG_VIDEO_ST_G1_PP) += -D"TRACE_PP_CTRL(...)=pr_debug(__VA_ARGS__)"

# PP support
g1-$(CONFIG_VIDEO_ST_G1_PP) += g1-pp.o
# FIXME shadow registers need to be shared between dec & pp.
# This point is not understood and need investigations
ccflags-$(CONFIG_VIDEO_ST_G1_PP) += -DDEC_PP_COMMON_SHADOW_REGS
