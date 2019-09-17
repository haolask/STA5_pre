/**
 * @file sta_splash_animation.c
 * @brief Play animation at boot from M-JPEG
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "trace.h"

#include "utils.h"

#include "FreeRTOS.h"
#include "sta_type.h"
#include "sta_mtu.h"
#include "jpegdecapi.h"
#include "g1-res.h"
#include "dwl.h"
#include "sta_ltdc.h"
#include "sta_lcd.h"
#include "task.h"
#include "sta_rpmsg_mm.h"
#include "sta_mm.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define READ_U32_BE(ptr) ((ptr[0] << 24) | (ptr[1] << 16) | \
			  (ptr[2] << 8)  | (ptr[3]))

#define CHECK_CONDITION(cond, str, label)	\
do {						\
	if (!(cond)) {				\
		TRACE_ERR(str "\n");		\
		goto label;			\
	}					\
} while (0)

#define LTDC_DISPLAY_OVERLAY_SPLASH 0
static SemaphoreHandle_t g_splash_unfreeze_task_sem;
static SemaphoreHandle_t g_splash_task_ended_sem;
static SemaphoreHandle_t g_splash_decode_ended_sem;
static bool init_done;
static bool end_of_animation;
static bool end_of_anim_task;
static void *g_splash_g1_ctx;

static inline uint32_t WAIT_FOR_COMPLETION(SemaphoreHandle_t sem)
{
	return xSemaphoreTake(sem, portMAX_DELAY);
}

static inline uint32_t WAIT_FOR_COMPLETION_TIMEOUT(SemaphoreHandle_t sem,
						   uint32_t delay_ms)
{
	return xSemaphoreTake(sem, pdMS_TO_TICKS(delay_ms));
}

static inline void COMPLETE(SemaphoreHandle_t sem)
{
	xSemaphoreGive(sem);
}

int splash_screen_free_resource(uint8_t services, void *priv, int len,
				void *data)
{
	if ((services | RPMSG_MM_SERV_STOP_ANIM) != RPMSG_MM_SERV_STOP_ANIM)
		return 0;

	return ltdc_layer_release(LTDC_DISPLAY_OVERLAY_SPLASH);
}

int splash_screen_init(void)
{
	rpmsg_mm_register_service(RPMSG_MM_SERV_STOP_ANIM,
				  splash_screen_free_resource,
				  NULL);
	return ltdc_layer_book(LTDC_DISPLAY_OVERLAY_SPLASH);
}

/**
 * @brief	Update LCD with splashscreen image
 * @param	shadow_addr: address of the image
 * @return	0 if no error, not 0 otherwise
 */
int splash_screen_update(void *shadow_addr)
{
	struct framebuffer fb;
	int ret = -1;
	uint32_t screen_w = 0;
	uint32_t screen_h = 0;
	uint32_t pos_x = 0;
	uint32_t pos_y = 0;

	if (!shadow_addr)
		return -1;

	/* define framebuffer information */
	fb.width = 800;
	fb.height = 480;
	fb.x = 0;
	fb.y = 0;
	fb.paddr = (uint32_t)shadow_addr;
	fb.offsets[0] = 0;
	fb.pixel_format = PF_RGB565;
	fb.pitches[0] = 2 * fb.width;

	lcd_get_display_resolution(&screen_w, &screen_h);

	if (screen_w > fb.width)
		pos_x = (screen_w - fb.width) >> 1;
	if (screen_h > fb.height)
		pos_y = (screen_h - fb.height) >> 1;

	ret = ltdc_layer_update(&fb, LTDC_DISPLAY_OVERLAY_SPLASH,
				pos_x, pos_y, BOTH);

	return ret;
}

static uint32_t *jpeg_sizes;
static uint32_t jpeg_max_size;
static uint32_t start_frame;
static uint32_t nb_frames;
static uint32_t frame_duration;

/* ===================================================================== */

static uint32_t anim_pos_x;
static uint32_t anim_pos_y;
static bool pos_known;

/* =====================================================================
 * The functions below ("find_atom" and "parse_mov") are a basic implementation
 * of a QuickTime file format (.mov).
 * It does not intend to parse any kind of mov file but only mov files
 * with no audio and one single M-JPEG track.
 *
 * See QTFF specification at following URL:
 * https://developer.apple.com/library/archive/documentation/QuickTime/QTFF
 * =====================================================================
 */

/**
 * @brief	Find a specific atom.
 *		An atom is a basic data unit in a QuickTime file
 * @param	ptr: address of the parent atom:
 *		     will be updated by child atom address if it is found
 * @param	type: type of atom to be found
 * @param	atom_size: size of parent atom
 *			   will be updated by child atom size if it is found
 * @return	0 if no error and child atom is found, not 0 otherwise
 */
int find_atom(unsigned char **ptr, uint32_t type, uint32_t *atom_size)
{
	uint32_t pos = 0;
	uint32_t subatom_size;
	uint32_t subatom_type;
	unsigned char *c_ptr = *ptr;

	if ((!ptr) || (!*ptr) || (!atom_size))
		return -1;

	while (pos < *atom_size) {
		subatom_size = READ_U32_BE((c_ptr + pos));
		subatom_type = READ_U32_BE((c_ptr + pos + 4));

		if (subatom_size > *atom_size)
			break;

		if (type == subatom_type) {
			*ptr = &c_ptr[pos + 8];
			*atom_size = subatom_size;
			return 0;
		}
		pos += subatom_size;
	}

	return -1;
}

/**
 * @brief	Parse QuickTime file (.mov), finds framerate and create table
 *		of JPEG files
 *		Updates the following global variables:
 *		 * jpeg_sizes: table of jpeg sizes
 *		 * jpeg_max_size: maximum jpeg size
 *		 * nb_frames: number of frames in the M-JPEG animation
 *		 * frame_duration: time duration of each frame in ms
 * @param	mov_buffer: address of QuickTime buffer
 * @param	buffer_size: size of QuickTime buffer
 * @param	mjpeg_ptr: M-JPEG start address
 * @return	0 if no error, not 0 otherwise
 */
static int parse_mov(u8 *mov_buffer, uint32_t buffer_size, uint8_t **mjpeg_ptr)
{
	uint8_t *ptr = mov_buffer;
	uint32_t atom_size = buffer_size;
	uint32_t pos = 0;
	uint32_t i = 0;
	uint8_t *tkhd_ptr;
	uint32_t tkhd_size;
	uint32_t duration;
	uint32_t jpeg_size;
	int ret = 0;

	/* Get the M-JPEG start address */
	*mjpeg_ptr = mov_buffer;
	ret = find_atom(mjpeg_ptr, 0x6d646174, &atom_size); // mdat
	CHECK_CONDITION(!ret, "Cannot find atom 'mdat'", err);

	/* Find the JPEG sizes table which is in 'stsz' atom
	 * and duration of stream which is in 'tkhd' atom
	 *
	 * Here is the location of these atoms:
	 * moov => trak => mdia => minf => stbl => stsz
	 *		=> tkhd
	 */
	atom_size = buffer_size;
	ret = find_atom(&ptr, 0x6D6F6F76, &atom_size); // moov
	CHECK_CONDITION(!ret, "Cannot find atom 'moov'", err);

	ret = find_atom(&ptr, 0x7472616b, &atom_size); // trak
	CHECK_CONDITION(!ret, "Cannot find atom 'trak'", err);

	tkhd_ptr = ptr; tkhd_size = atom_size;
	ret = find_atom(&tkhd_ptr, 0x746b6864, &tkhd_size); // tkhd
	CHECK_CONDITION(!ret, "Cannot find atom 'tkhd'", err);

	ret = find_atom(&ptr, 0x6d646961, &atom_size); // mdia
	CHECK_CONDITION(!ret, "Cannot find atom 'mdia'", err);

	ret = find_atom(&ptr, 0x6d696e66, &atom_size); // minf
	CHECK_CONDITION(!ret, "Cannot find atom 'minf'", err);

	ret = find_atom(&ptr, 0x7374626c, &atom_size); // stbl
	CHECK_CONDITION(!ret, "Cannot find atom 'stbl'", err);

	ret = find_atom(&ptr, 0x7374737a, &atom_size); // stsz
	CHECK_CONDITION(!ret, "Cannot find atom 'stsz'", err);

	duration = READ_U32_BE((tkhd_ptr + 20));
	TRACE_VERBOSE("Duration = %d\n", duration);

	ptr += 8; /* Jump to "number of entries" */
	nb_frames = READ_U32_BE(ptr);
	TRACE_VERBOSE("splash animation - nb_frames %d\n", nb_frames);
	ptr += 4; /* Jump to "sample size table" */

	if (atom_size < (4 * nb_frames))
		return -1;

	frame_duration = duration / nb_frames;

	jpeg_sizes = (uint32_t *)pvPortMalloc(nb_frames * sizeof(uint32_t));
	CHECK_CONDITION(jpeg_sizes, "Cannot allocates jpeg_sizes table", err);

	pos = 0;
	for (i = 0; i < nb_frames; i++) {
		jpeg_size = READ_U32_BE((ptr + pos));
		jpeg_sizes[i] = jpeg_size;
		jpeg_max_size = MAX(jpeg_max_size, jpeg_size);
		TRACE_VERBOSE("jpeg_size %u\n", jpeg_size);
		pos += 4;
	}
	return 0;
err:
	TRACE_ERR("ERROR: mov file parsing failed\n");
	return -1;
}

void display_image(JpegDecOutput * jpegOut, JpegDecImageInfo * pImageInfo)
{
	struct framebuffer fb;
	char *ptrcY = NULL;
	char *ptrcCbCr = NULL;

	/* define framebuffer information */
	fb.pixel_format = PF_FORMAT_NV12;
	fb.width = pImageInfo->outputWidth;
	fb.height = pImageInfo->outputHeight;
	fb.x = 0;
	fb.y = 0;

	if (!pos_known)	{
		uint32_t screen_w = 0;
		uint32_t screen_h = 0;

		anim_pos_x = 0;
		anim_pos_y = 0;
		lcd_get_display_resolution(&screen_w, &screen_h);
		if (screen_w > fb.width)
			anim_pos_x = (screen_w - fb.width) >> 1;
		if (screen_h > fb.height)
			anim_pos_y = (screen_h - fb.height) >> 1;
		pos_known = true;
	}

	/* Luma info */
	fb.paddr = (uint32_t)jpegOut->outputPictureY.pVirtualAddress;
	fb.offsets[0] = 0;
	fb.pitches[0] = fb.width;
	/* Chroma info */
	ptrcY = (char *)jpegOut->outputPictureY.pVirtualAddress;
	ptrcCbCr = (char *)jpegOut->outputPictureCbCr.pVirtualAddress;
	fb.offsets[1] = ptrcCbCr - ptrcY;
	fb.pitches[1] = fb.width;
	// Display this buffer
	ltdc_layer_update(&fb, LTDC_DISPLAY_OVERLAY_SPLASH,
			  anim_pos_x, anim_pos_y, BOTH);
}

int alloc_output_buf(JpegDecLinearMem *outY,
		     JpegDecLinearMem *outCbCr,
		     JpegDecImageInfo *pImageInfo)
{
	int buf_size;

	/* we only support YUV 4:2:0 */
	buf_size = (pImageInfo->outputWidth * pImageInfo->outputHeight * 3) / 2;

	outY->pVirtualAddress = malloc(buf_size);
	outY->busAddress = (u32)outY->pVirtualAddress;
	if (!outY->pVirtualAddress)
		return 2;
	outCbCr->pVirtualAddress = outY->pVirtualAddress +
				   (pImageInfo->outputWidth *
				    pImageInfo->outputHeight) / sizeof(u32);
	outCbCr->busAddress = (u32)outCbCr->pVirtualAddress;

	return 0;
}

int jpegdecode(u8 *jpegbuffer)
{
	JpegDecRet ret = 0;
	JpegDecInst decInst = NULL;
	JpegDecInput decIn;
	JpegDecOutput decOut;
	JpegDecLinearMem lOutBufY[2];
	JpegDecLinearMem lOutBufCbCr[2];

	JpegDecImageInfo imageInfo;
	uint32_t i = 0, count = 0, indexOutputBuffer = 0;
	u32 *input_buffer = NULL;
	u8 *jpeg_buf_ptr = jpegbuffer;
	const TickType_t xFrequency = pdMS_TO_TICKS(frame_duration);
	TickType_t xLastWakeTime;
	int reverse = 0;
	int status = -1;

	if (end_of_animation)
		return -1;

	if (!jpeg_buf_ptr)
		return -1;

	TRACE_INFO("Start animation\n");
	sta_mm_send_event(MM_EVT_ANIM_STARTED);

	/* Book a splash screen display layer to be
	 * able to display each image
	 */
	if (ltdc_layer_book(LTDC_DISPLAY_OVERLAY_SPLASH) < 0)
		goto on_error;

	if (!g_splash_g1_ctx)
		goto end;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	// We assume here that input buffer has always the same size
	input_buffer = malloc(jpeg_max_size);
	if (!input_buffer) {
		TRACE_ERR("ERROR: malloc returns NULL pointer\n");
		goto end;
	}

	memset(lOutBufY, 0, sizeof(JpegDecLinearMem) * 2);
	memset(lOutBufCbCr, 0, sizeof(JpegDecLinearMem) * 2);

	/* Jump to start_frame */
	for (i = 0; i < start_frame; i++)
		jpeg_buf_ptr += jpeg_sizes[i];

	while (!end_of_animation) {
		if (decInst) {
			JpegDecRelease(decInst);
			decInst = NULL;
		}

		if (jpeg_sizes[i] > jpeg_max_size) {
			TRACE_ERR("jpeg size (%d) > jpeg_max_size (%d)\n",
				  jpeg_sizes[i], jpeg_max_size);
			goto end;
		}
		memcpy(input_buffer, jpeg_buf_ptr, jpeg_sizes[i]);

		count = 0;
		do {
			// Init the decoder
			ret = JpegDecInit(&decInst, g_splash_g1_ctx);
			if (ret != JPEGDEC_OK) {
				TRACE_ERR("ERROR: JpegDecInit ret=%d\n", ret);
				goto end;
			}
			indexOutputBuffer = i % 2;
			memset(&decIn, 0, sizeof(decIn));
			/* Full resolution image to be decoded
			 * (not thumbnail)
			 */
			decIn.decImageType = 0;
			decIn.streamBuffer.pVirtualAddress = input_buffer;
			decIn.streamBuffer.busAddress = (u32)input_buffer;
			decIn.streamLength = jpeg_sizes[i];

			ret = JpegDecGetImageInfo(decInst, &decIn, &imageInfo);
			if (ret != JPEGDEC_OK) {
				TRACE_ERR("ERROR: JpegDecGetImageInfo ret=%d\n",
					  ret);
				count++;
				continue;
			}
			/* If not yet allocated, we allocate an output buffer */
			if (!lOutBufY[indexOutputBuffer].pVirtualAddress)
				ret = alloc_output_buf(
					&lOutBufY[indexOutputBuffer],
					&lOutBufCbCr[indexOutputBuffer],
					&imageInfo);
			/* Output buffer info are transferred to the decoder
			 * inside the input data (decIn)
			 */
			decIn.pictureBufferY.pVirtualAddress =
				lOutBufY[indexOutputBuffer].pVirtualAddress;
			decIn.pictureBufferY.busAddress =
				(u32)decIn.pictureBufferY.pVirtualAddress;
			decIn.pictureBufferCbCr.pVirtualAddress =
				lOutBufCbCr[indexOutputBuffer].pVirtualAddress;
			decIn.pictureBufferCbCr.busAddress =
				(u32)decIn.pictureBufferCbCr.pVirtualAddress;

			if (imageInfo.outputFormat !=
				JPEGDEC_YCbCr420_SEMIPLANAR) {
				TRACE_ERR("Bad format : %d\n",
					  imageInfo.outputFormat);
				goto end;
			}

			ret = JpegDecDecode(decInst, &decIn, &decOut);
			if (ret == JPEGDEC_FRAME_READY)
				display_image(&decOut, &imageInfo);
			count++;
		} while (ret != JPEGDEC_FRAME_READY && count < 2);

		if (ret != JPEGDEC_FRAME_READY && count == 2)
			TRACE_ERR("ERROR: JpegDecDecode %d ret=%d - retry\n",
				  i, ret);

		// Wait for frame_duration ms
		vTaskDelayUntil(&xLastWakeTime, xFrequency);

		/* Switch to next image */
		if (reverse)
			jpeg_buf_ptr -= jpeg_sizes[--i];
		else
			jpeg_buf_ptr += jpeg_sizes[i++];

		if (i == (nb_frames + start_frame)) {
			reverse = 1;
			jpeg_buf_ptr -= jpeg_sizes[--i];
		}

		if (i == start_frame)
			reverse = 0;
	}
	status = 0;
end:
	if (decInst)
		JpegDecRelease(decInst);

	if (input_buffer)
		free(input_buffer);

	for (i = 0; i < 2; i++) {
		if (lOutBufY[i].pVirtualAddress) {
			free(lOutBufY[i].pVirtualAddress);
			lOutBufY[i].pVirtualAddress = NULL;
		}
	}
	/* Release resources previously booked so that
	 * it becomes available on Linux side
	 */
	ltdc_layer_release(LTDC_DISPLAY_OVERLAY_SPLASH);

on_error:
	sta_mm_send_event(MM_EVT_ANIM_STOPPED);
	TRACE_INFO("End of animation\n");
	if (g_splash_decode_ended_sem)
		COMPLETE(g_splash_decode_ended_sem);
	return status;
}

void splash_init(void)
{
	taskENTER_CRITICAL();
	if (init_done) {
		taskEXIT_CRITICAL();
		return;
	}
	if (!g_splash_unfreeze_task_sem)
		g_splash_unfreeze_task_sem = xSemaphoreCreateBinary();
	if (!g_splash_task_ended_sem)
		g_splash_task_ended_sem = xSemaphoreCreateBinary();
	if (!g_splash_decode_ended_sem)
		g_splash_decode_ended_sem = xSemaphoreCreateBinary();
	init_done = true;
	taskEXIT_CRITICAL();
}

int splash_stop_anim(bool stop_task)
{
	if (!init_done)
		return 0;
	TRACE_INFO("[SPLASH] stop animation request\n");
	end_of_animation = true;
	end_of_anim_task = stop_task;
	/* Unblocked the task */
	if (g_splash_unfreeze_task_sem)
		COMPLETE(g_splash_unfreeze_task_sem);

	if (g_splash_decode_ended_sem) {
		TRACE_INFO("[SPLASH] wait end of decode if any\n");
		if (WAIT_FOR_COMPLETION_TIMEOUT(g_splash_decode_ended_sem,
						pdMS_TO_TICKS(1000)) ==
						pdFALSE) {
			TRACE_ERR("Unable to wait end of decode\n");
		}
	}

	TRACE_INFO("[SPLASH] wait end of task\n");
	if (stop_task && g_splash_task_ended_sem) {
		if (WAIT_FOR_COMPLETION_TIMEOUT(g_splash_task_ended_sem,
						pdMS_TO_TICKS(1000)) ==
						pdFALSE) {
			TRACE_ERR("Unable to wait end of task\n");
		}
		vSemaphoreDelete(g_splash_task_ended_sem);
		g_splash_task_ended_sem = NULL;
	}
	return 0;
}

int splash_start_anim(bool start_allowed)
{
	if (!start_allowed)
		return -1;
	TRACE_INFO("[SPLASH] start animation request\n");
	splash_init();
	end_of_animation = false;
	/* Unblocked the task */
	COMPLETE(g_splash_unfreeze_task_sem);
	return 0;
}

/**
 * @brief	start splash animation
 * @param	none
 * @return	none
 */
void splash_animation_task(void *p)
{
	uint8_t *mjpeg_ptr;
	struct sta_mm_ctx *sta_mm = (struct sta_mm_ctx *)p;
	int ret = 0;

	splash_init();
	G1ResInit();

	ret = parse_mov(sta_mm->anim_shadow_addr,
			sta_mm->anim_size,
			&mjpeg_ptr);
	if (ret) {
		TRACE_ERR("Error parsing mov file\n");
		goto end_task;
	}

	TRACE_INFO("[SPLASH] Task started\n");

	/* Reserve G1 resource */
	g_splash_g1_ctx = G1ResBookResource();
	if (!g_splash_g1_ctx) {
		TRACE_ERR("[SPLASH] Unable to book jpeg decoder\n");
		return;
	}

	do {
		TRACE_INFO("[SPLASH] Wait for start request\n");
		WAIT_FOR_COMPLETION(g_splash_unfreeze_task_sem);
		if (jpegdecode(mjpeg_ptr))
			TRACE_INFO("[SPLASH] abort animation\n");
	} while (!end_of_anim_task);

	vSemaphoreDelete(g_splash_unfreeze_task_sem);
	TRACE_INFO("ANIM TASK ====> FREEING G1 RESOURCES\n");
	G1ResFreeResource(g_splash_g1_ctx);
	g_splash_g1_ctx = NULL;

	TRACE_INFO("[SPLASH] Task stopped\n");
	COMPLETE(g_splash_task_ended_sem);

end_task:
	vTaskDelete(NULL);
}
