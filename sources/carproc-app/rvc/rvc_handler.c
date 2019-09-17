/*
 * Manage Rear View Camera application. This user application is able to catch
 * Rear Gear Event coming from remote processor. Depending on the return value
 * application builds or deletes GStreamer graph to perform V4l2 Stream from
 * RVCAM module.
 *
 * Copyright (C) 2016, STMicroelectronics - All Rights Reserved
 * Author: Pierre-Yves MORDRET <pierre-yves.mordret@st.con> for STMicroelectronics.
 *
 * License type: Apache
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <getopt.h> /* getopt_long() */

#include <sys/signalfd.h>
#include <signal.h>

#include <assert.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/syscall.h>
#include <stdbool.h>
#include <gst/gst.h>
#include <glib.h>
#include <linux/videodev2.h>

#ifdef HAVE_LIBV4L2
#include <libv4l2.h>
#else
#define v4l2_fd_open(fd, flags) (fd)
#define v4l2_close    close
#define v4l2_dup      dup
#define v4l2_ioctl    ioctl
#define v4l2_read     read
#define v4l2_mmap     mmap
#define v4l2_munmap   munmap
#endif

/*#define SUBDEV_MODULE "/kernel/drivers/media/i2c/adv7180.ko"*/
#define SUBDEV_MODULE "/kernel/drivers/media/platform/sta_tvdec.ko"
#define VIP_MODULE    "/kernel/drivers/media/platform/sta_vip.ko"

#define SUBDEV_MODULE_NAME "sta_tvdec"
#define VIP_MODULE_NAME    "sta_vip"

#define RPMSG_MM_DEVICE_NAME    "/dev/rpmsg_mm0"
#define RPMSG_MM_IOC_MAGIC      'L'

#define RPMSG_MM_IOC_GET_RG_STATUS     _IOR(RPMSG_MM_IOC_MAGIC, 1, int)
#define RPMSG_MM_IOC_RVC_Ax_TAKEOVER   _IOW(RPMSG_MM_IOC_MAGIC, 6, int)
/*!< IOCTL used to request A7 takeover of RVC resources */
#define RPMSG_MM_REARGEAR_STATUS_ACK    14
#define RPMSG_MM_PRIVATE_MESSAGE        15

#define MAX_DEVICES 20          /* Max V4L2 device instances tried */

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define GST_RVC_PIPELINE_NAME  "rear-view-camera"
#define GST_RVC_CAMERA         "v4l2src"
#define GST_RVC_SCALER         "v4l2trans"
#define GST_RVC_COLOR_CONV     "v4l2trans"
#define GST_RVC_RENDER         "waylandsink"

#define SCREEN_WIDTH   800
#define SCREEN_HEIGHT  480

#define DEFAULT_STANDARD 0 /* 0 for autodetect */

#define VERSION "1.1.0"
#define MAX_NAME_LENGTH 15

#ifdef DEBUG
#define DEBUG_PRINTF printf
#else
#define DEBUG_PRINTF
#endif

char *info2str[] = {
	"UNUSED",
	"REGISTRATION_REQ",
	"REGISTRATION_ACK",
	"UNREGISTRATION_REQ",
	"UNREGISTRATION_ACK",
	"SHARED_RES_STATUS_REQ",
	"SHARED_RES_STATUS_ACK",
	"SHARED_RES_LOCKED",
	"SHARED_RES_LOCKED_ACK",
	"SHARED_RES_UNLOCKED",
	"COMM_UNAVAILABLE",
	"COMM_AVAILABLE",
	"USER_SERVICE",
	/* RVC dedicated messages */
	"REARGEAR_STATUS_REQ",
	"REARGEAR_STATUS_ACK",
	"PRIVATE_MESSAGE"
};

enum rvc_rpsmg_private_msg {
	RVC_BUFFER_FILLED,
	RVC_FILL_BUFFER,
	RVC_START_STREAMING_REQ,
	RVC_STOP_STREAMING_REQ,
	RVC_GET_CONFIGURATION_REQ,
	RVC_GET_CONFIGURATION_ACK,
	RVC_GET_TAKEOVER_STATUS_ACK,
};

struct rpmsg_mm_msg_hdr {
    int info;
    int len;
    char data[0];
};

struct gst_context {
    GstElement *pipeline;
    GMainLoop *loop;
    guint bus_watch_id;
    char vid_device[100];
    guint io_mode;
    bool rescale;
    bool deinterlace;
    unsigned int scale_width;
    unsigned int scale_height;
    v4l2_std_id norm;
};

struct fd_context {
    int fd_rvc;
    int fd_vip;
    char libpath[255];
    char init_done;
};

struct glob_context {
    struct gst_context gst_ctx;
    struct fd_context fd_ctx;
    struct pollfd pfd[3];
    int sig_fd;
    bool autodetect;
};

struct resolution {
    short width;
    short height;
};

struct resolution resolution = {720, 480};

struct pixel_format
{
    guint32 pixel_fmt_nb;
    gchar *pixel_fmt_str;
};

#define RVCAM_COLOR_FORMAT V4L2_PIX_FMT_UYVY

struct pixel_format px_formats[] = {
    {V4L2_PIX_FMT_NV12,        (gchar *) "V4L2_PIX_FMT_NV12"},
    {V4L2_PIX_FMT_RGB565,      (gchar *) "V4L2_PIX_FMT_RGB565"},
    {V4L2_PIX_FMT_UYVY,        (gchar *) "V4L2_PIX_FMT_UYVY"},
    {V4L2_PIX_FMT_XBGR32,      (gchar *) "V4L2_PIX_FMT_XBGR32"},
    {V4L2_PIX_FMT_H264,        (gchar *) "V4L2_PIX_FMT_H264"},
#ifdef V4L2_PIX_FMT_HEVC
    {V4L2_PIX_FMT_HEVC,        (gchar *) "V4L2_PIX_FMT_HEVC"},
#endif
    {V4L2_PIX_FMT_MPEG1,       (gchar *) "V4L2_PIX_FMT_MPEG1"},
    {V4L2_PIX_FMT_MPEG2,       (gchar *) "V4L2_PIX_FMT_MPEG2"},
    {V4L2_PIX_FMT_MPEG4,       (gchar *) "V4L2_PIX_FMT_MPEG4"},
    {V4L2_PIX_FMT_XVID,        (gchar *) "V4L2_PIX_FMT_XVID"},
    {V4L2_PIX_FMT_VP8,         (gchar *) "V4L2_PIX_FMT_VP8"},
    {V4L2_PIX_FMT_VC1_ANNEX_G, (gchar *) "V4L2_PIX_FMT_VC1_ANNEX_G"},
    {V4L2_PIX_FMT_VC1_ANNEX_L, (gchar *) "V4L2_PIX_FMT_VC1_ANNEX_L"},
    {V4L2_PIX_FMT_MJPEG,       (gchar *) "V4L2_PIX_FMT_MJPEG"},
#ifdef V4L2_PIX_FMT_CAVS
    {V4L2_PIX_FMT_CAVS,        (gchar *) "V4L2_PIX_FMT_CAVS"},
#endif
};

/* used for debug pixelformat (v4l2 object) */
static char * v4l2_fmt_str (guint32 fmt)
{
    int i = 0;
    for (i = 0; i < ARRAY_SIZE (px_formats); i++) {
        if (px_formats[i].pixel_fmt_nb == fmt)
            return px_formats[i].pixel_fmt_str;
    }
    return NULL;
}

#define init_module(mod, len, opts) syscall(__NR_init_module, mod, len, opts)
#define delete_module(name, flags) syscall(__NR_delete_module, name, flags)

static gboolean gst_bus_call (GstBus     *bus,
                              GstMessage *msg,
                              gpointer    data)
{
    GMainLoop *loop = (GMainLoop *) data;

    switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
        g_print ("End of stream\n");
        g_main_loop_quit (loop);
        break;

    case GST_MESSAGE_ERROR: {
        gchar  *debug;
        GError *error;

        gst_message_parse_error (msg, &error, &debug);
        g_free (debug);

        g_printerr ("Error: %s\n", error->message);
        g_error_free (error);

        g_main_loop_quit (loop);
        break;
    }
    default:
        g_print ("Got message 0x%X\n", GST_MESSAGE_TYPE (msg));
        break;
    }

    return TRUE;
}

static void gst_start_pipeline(struct glob_context *ctx)
{
    struct gst_context *gst_ctx = &ctx->gst_ctx;
    GstElement *source, *color_converter, *scaler, *sink;
    GstBus *bus;
    GstCaps *caps;

    gst_ctx->loop = g_main_loop_new (NULL, FALSE);

    /* Create GStreamer elements */
    gst_ctx->pipeline = gst_pipeline_new (GST_RVC_PIPELINE_NAME);
    source   = gst_element_factory_make (GST_RVC_CAMERA, "vip-source");
    if(gst_ctx->rescale || gst_ctx->deinterlace)
        scaler   = gst_element_factory_make (GST_RVC_SCALER, "scaler");
    if(gst_ctx->deinterlace)
        color_converter   = gst_element_factory_make (GST_RVC_COLOR_CONV, "color_conv");
    sink  = gst_element_factory_make (GST_RVC_RENDER, "renderer");

    if (!gst_ctx->pipeline || !source || !sink) {
        g_printerr ("One element could not be created. Exiting.\n");
    }

    if(gst_ctx->rescale && !scaler)
        g_printerr ("scaler could not be created. Exiting.\n");

    if (gst_ctx->norm) {
        g_object_set (G_OBJECT (source),
                      "device", gst_ctx->vid_device,
                      "io-mode", gst_ctx->io_mode,
                      "norm", gst_ctx->norm,
                      NULL);
    } else {
        g_object_set (G_OBJECT (source),
                      "device", gst_ctx->vid_device,
                      "io-mode", gst_ctx->io_mode,
                      NULL);
    }

    /* We add a message handler */
    bus = gst_pipeline_get_bus (GST_PIPELINE (gst_ctx->pipeline));
    gst_ctx->bus_watch_id = gst_bus_add_watch (bus, gst_bus_call,
                                               gst_ctx->loop);
    gst_object_unref (bus);

    /* We add all elements into the pipeline */
    if(gst_ctx->deinterlace) {
        if (!gst_ctx->scale_width)
            gst_ctx->scale_width = resolution.width;

        if (!gst_ctx->scale_height)
            gst_ctx->scale_height = resolution.height;

        gst_bin_add_many (GST_BIN (gst_ctx->pipeline), source, color_converter, scaler, sink, NULL);

        caps = gst_caps_new_simple ("video/x-raw",
                                    "format", G_TYPE_STRING , "UYVY",
                                    "width", G_TYPE_INT, resolution.width,
                                    "height", G_TYPE_INT, resolution.height,
                                    NULL);

        if (!gst_element_link_filtered(source, color_converter, caps))
            g_warning("failed to link source and color converter\n");

        gst_caps_unref(caps);

        caps = gst_caps_new_simple ("video/x-raw",
                                    "format", G_TYPE_STRING , "NV12",
                                    "width", G_TYPE_INT, resolution.width,
                                    "height", G_TYPE_INT, resolution.height,
                                    NULL);

        if (!gst_element_link_filtered(color_converter, scaler, caps))
            g_warning("failed to link source and scaler\n");

        gst_caps_unref(caps);

        caps = gst_caps_new_simple ("video/x-raw",
                                    "width",  G_TYPE_INT, gst_ctx->scale_width,
                                    "height", G_TYPE_INT, gst_ctx->scale_height,
                                    "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                                    NULL);

        if (!gst_element_link_filtered(scaler, sink, caps))
            g_warning("failed to link scaler and sink\n");

        gst_caps_unref(caps);
    }
    else if(gst_ctx->rescale) {
        if (!gst_ctx->scale_width)
            gst_ctx->scale_width = resolution.width;

        if (!gst_ctx->scale_height)
            gst_ctx->scale_height = resolution.height;

        gst_bin_add_many (GST_BIN (gst_ctx->pipeline), source, scaler, sink, NULL);

        caps = gst_caps_new_simple ("video/x-raw",
                                    "format", G_TYPE_STRING , "UYVY",
                                    "width", G_TYPE_INT, resolution.width,
                                    "height", G_TYPE_INT, resolution.height,
                                    NULL);

        if (!gst_element_link_filtered(source, scaler, caps))
            g_warning("failed to link source and scaler\n");

        gst_caps_unref(caps);

        caps = gst_caps_new_simple ("video/x-raw",
                                    "width",  G_TYPE_INT, gst_ctx->scale_width,
                                    "height", G_TYPE_INT, gst_ctx->scale_height,
                                    "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                                    NULL);

        if (!gst_element_link_filtered(scaler, sink, caps))
            g_warning("failed to link scaler and sink\n");

        gst_caps_unref(caps);

    } else {
        gst_bin_add_many (GST_BIN (gst_ctx->pipeline), source, sink, NULL);

        caps = gst_caps_new_simple ("video/x-raw",
                                    "format", G_TYPE_STRING , "UYVY",
                                    "width", G_TYPE_INT, resolution.width,
                                    "height", G_TYPE_INT, resolution.height,
                                    NULL);

        if (!gst_element_link_filtered(source, sink, caps))
            g_warning("failed to link source and renderer\n");
        gst_caps_unref(caps);
    }

    /* Set the pipeline to "playing" state*/
    g_print ("Now streaming with I/O Mode(%d)\n", gst_ctx->io_mode);
    gst_element_set_state (gst_ctx->pipeline, GST_STATE_PLAYING);

    /* wait until it's up and running or failed */
    if (gst_element_get_state (gst_ctx->pipeline, NULL, NULL, GST_SECOND) ==
        GST_STATE_CHANGE_FAILURE)
        g_error ("Failed to go into PLAYING state");

    DEBUG_PRINTF("Running ...\n");
}

static void gst_stop_pipeline(struct glob_context *ctx)
{
    struct gst_context *gst_ctx = &ctx->gst_ctx;

    /* Out of the main loop, clean up nicely */
    DEBUG_PRINTF("Stopping playback\n");

    if (gst_ctx->pipeline) {
        gst_element_set_state (gst_ctx->pipeline, GST_STATE_NULL);
        gst_object_unref (GST_OBJECT (gst_ctx->pipeline));
    }

    if (gst_ctx->bus_watch_id)
        g_source_remove (gst_ctx->bus_watch_id);

    if (gst_ctx->loop)
        g_main_loop_unref (gst_ctx->loop);

    gst_ctx->pipeline = NULL;
    gst_ctx->loop = NULL;
    gst_ctx->bus_watch_id = 0;
    g_print ("Stopped playback\n");
}

static int find_device(char *out, int *vid_fd)
{
    char path[100];
    int libv4l2_fd;
    struct v4l2_format try_fmt, req_fmt, s_fmt;
    struct v4l2_capability cap;
    int i = 0, fd = -1, ret;
    char found = FALSE;

    memset (&req_fmt, 0, sizeof(req_fmt));
    req_fmt.type = V4L2_CAP_VIDEO_CAPTURE;
    req_fmt.fmt.pix.pixelformat = RVCAM_COLOR_FORMAT;

    for (i = 0; i < MAX_DEVICES; i++) {
        snprintf (path, sizeof (path), "/dev/video%d", i);
        fd = open (path, O_RDWR, 0);
        if (fd < 0)
            continue;

        libv4l2_fd = v4l2_fd_open (fd, V4L2_DISABLE_CONVERSION);
        if (libv4l2_fd != -1)
            fd = libv4l2_fd;

        try_fmt = req_fmt;
        ret = v4l2_ioctl (fd, VIDIOC_TRY_FMT, &try_fmt);
        if (ret < 0) {
            v4l2_close (fd);
            continue;
        }

        s_fmt = req_fmt;
        ret = v4l2_ioctl (fd, VIDIOC_S_FMT, &s_fmt);
        if (ret < 0) {
            v4l2_close (fd);
            continue;
        }

        ret = v4l2_ioctl (fd, VIDIOC_QUERYCAP, &cap);
        if (ret != 0) {
            v4l2_close (fd);
            continue;
        }

        /* select first driver which is not M2M compatible */
        if ((cap.capabilities & V4L2_CAP_VIDEO_M2M) != 0) {
            v4l2_close (fd);
            continue;
        }

        /* device can silently accepts (no error returned) any format
         * and change s_fmt values according to its capabilities, so
         * verify that device comply with requested format
         */
        if (!(s_fmt.fmt.pix.pixelformat == req_fmt.fmt.pix.pixelformat)) {
            v4l2_close (fd);
            continue;
        }
/*
        resolution.width = s_fmt.fmt.pix.width;
        resolution.height = s_fmt.fmt.pix.height;
*/
        found = TRUE;
        break;
    }

    if (!found) {
        fprintf (stderr,
                 "No device found matching format %s(0x%x) and resolution %dx%d\n",
                 v4l2_fmt_str(RVCAM_COLOR_FORMAT), RVCAM_COLOR_FORMAT,
                 resolution.width,
                 resolution.height);
        return -1;
    }

    fprintf (stderr, "Device is %s for format %s and %dx%d resolution\n",
             path, v4l2_fmt_str(RVCAM_COLOR_FORMAT),
             resolution.width,
             resolution.height);
    memcpy (out, path, strlen(path));
    *vid_fd = fd;

    return 0;
}

static int load_mod(const char name[255])
{
    int mod_fd = open(name, O_RDONLY);
    struct stat st;
    size_t image_size;
    void *image;

    fstat(mod_fd, &st);
    image_size = st.st_size;

    image = malloc(image_size);

    fprintf(stderr, "Insmoding \"%s\"(%i), size=%i\n",
            name, mod_fd, image_size);

    if (mod_fd < 0) {
        perror("init_module: open");
        return -1;
    }

    read(mod_fd, image, image_size);
    close(mod_fd);
    if (init_module(image, image_size, "") != 0) {
        if ( errno != EEXIST) {
            perror("init_module fail");
            return EXIT_FAILURE;
        }
    }
    free(image);

    return 0;
}

static int send_takeover_request(struct fd_context *fd_ctx)
{
    int ret;
    int retval;

    printf(" <<<<        send_takeover_request       >>>> \n");

    ret = ioctl(fd_ctx->fd_rvc, RPMSG_MM_IOC_RVC_Ax_TAKEOVER, &retval);
    if (ret < 0) {
        printf(" Unable to request Ax takeover ret = %d\n", ret);
        return ret;
    }

    return 0;
}

static int load_framework(struct glob_context *ctx)
{
    struct fd_context *fd_ctx   = &ctx->fd_ctx;
    struct gst_context *gst_ctx = &ctx->gst_ctx;

    char lib_name[255];

    struct v4l2_event_subscription event_subcription;
    int ret=0;

    if (!fd_ctx->init_done) {
        memset( lib_name, 0, 255);
        memcpy(lib_name, fd_ctx->libpath ,strlen(fd_ctx->libpath));
        strcat(lib_name, SUBDEV_MODULE);
        if (load_mod(lib_name))
            return -1;

        memset( lib_name, 0, 255);
        memcpy(lib_name, fd_ctx->libpath, strlen(fd_ctx->libpath));
        strcat(lib_name, VIP_MODULE);
        if (load_mod(lib_name))
            return -1;

        if (find_device((char *)gst_ctx->vid_device, &fd_ctx->fd_vip)) {
            return -1;
        }

        if (ctx->autodetect)
        {
            memset(&event_subcription, 0, sizeof(event_subcription));
            event_subcription.type = V4L2_EVENT_SOURCE_CHANGE;
            event_subcription.id = 0;
            event_subcription.flags = 0;
            ret = v4l2_ioctl (fd_ctx->fd_vip, VIDIOC_SUBSCRIBE_EVENT, &event_subcription);
            if (ret < 0) {
                v4l2_close (fd_ctx->fd_vip);
                fprintf(stderr, "Impossible to ioctl (VIDIOC_SUBSCRIBE_EVENT).\n");
                return -1;
            }
        }
        else
        {
            /* If there is no autodetect, there is no need to keep a handle
             * on VIP module as we don't wait for events
             */
            v4l2_close (fd_ctx->fd_vip);
        }

        fd_ctx->init_done = true;
    }

    return 0;
}

static int unload_framework(struct glob_context *ctx)
{
    struct fd_context *fd_ctx   = &ctx->fd_ctx;
    struct gst_context *gst_ctx = &ctx->gst_ctx;

    int ret=0;

    if (fd_ctx->init_done) {
        if (ctx->autodetect)
        {
            v4l2_close (fd_ctx->fd_vip);
        }

        ret = delete_module(VIP_MODULE_NAME, O_NONBLOCK);
        if (ret)
            perror("fail to delete module " VIP_MODULE_NAME);

        ret = delete_module(SUBDEV_MODULE_NAME, O_NONBLOCK);
        if (ret)
            perror("fail to delete module " SUBDEV_MODULE_NAME);

        fd_ctx->init_done = false;
    }

    return 0;
}

static int get_rear_gear_status_ioctl(bool *status, struct fd_context *fd_ctx)
{
    int ret;
    int rear_gear_val;

    ret = ioctl(fd_ctx->fd_rvc, RPMSG_MM_IOC_GET_RG_STATUS, &rear_gear_val);
    if (ret < 0) {
        printf(" Unable to get RG status ret = %d\n", ret);
        return ret;
    }
    DEBUG_PRINTF("rear_gear_val: %d\n", rear_gear_val);

    switch(rear_gear_val) {
    case 1:
        printf ("-> Rear Gear Engaged <-\n");
        *status=true;
        break;
    case 0:
        printf ("<- Rear Gear Disengaged ->\n");
        *status=false;
        break;
    default:
        fprintf(stderr, "Unknown rear gear status 0x%08X\n", rear_gear_val);
        return -1;
    }

    return 0;

}

static int get_takeover_status_read(bool *status, struct fd_context *fd_ctx)
{
    int bytes;
    uint32_t rear_gear_val=0;
    struct rpmsg_mm_msg_hdr * pdata;
    char buffer[128];
    int i = 0;
    uint32_t *pstatus = NULL;

    bytes = read(fd_ctx->fd_rvc, &buffer, 128);

    if (bytes > sizeof(struct rpmsg_mm_msg_hdr)) {
        pdata = (struct rpmsg_mm_msg_hdr *)buffer;
        DEBUG_PRINTF("info : %s\n", info2str[pdata->info]);
    }

    if (pdata->info != RPMSG_MM_PRIVATE_MESSAGE)
        return -1;

    pstatus = (uint32_t *)&pdata->data[0];
    if (*pstatus == RVC_GET_TAKEOVER_STATUS_ACK)
    {
        printf("RVC_GET_TAKEOVER_STATUS_ACK\n");
        return 0;
    }

    return -1;
}

static int get_rear_gear_status_read(bool *status, struct fd_context *fd_ctx)
{
    int bytes;
    uint32_t rear_gear_val=0;
    struct rpmsg_mm_msg_hdr * pdata;
    char buffer[128];
    int i = 0;
    uint32_t *pstatus = NULL;

    bytes = read(fd_ctx->fd_rvc, &buffer, 128);

    if (bytes > sizeof(struct rpmsg_mm_msg_hdr)) {
        pdata = (struct rpmsg_mm_msg_hdr *)buffer;
        DEBUG_PRINTF("info : %s\n", info2str[pdata->info]);
    }

    if (pdata->info != RPMSG_MM_REARGEAR_STATUS_ACK)
        return -1;

    pstatus = (uint32_t *)&pdata->data[0];
    rear_gear_val = *pstatus;

    DEBUG_PRINTF("rear_gear_val: 0x%08X\n", rear_gear_val);

    switch(rear_gear_val) {
    case 1:
        printf ("-> Rear Gear Engaged <-\n");
        *status=true;
        break;
    case 0:
        printf ("<- Rear Gear Disengaged ->\n");
        *status=false;
        break;
    default:
        fprintf(stderr, "Unknown rear gear status 0x%08X\n", rear_gear_val);
        return -1;
    }

    return 0;
}

static int dequeue_events(bool *src_change_event, struct glob_context *ctx)
{
    int ret;
    struct v4l2_event vip_ev;
    struct fd_context *fd_ctx   = &ctx->fd_ctx;

    *src_change_event = false;
    memset(&vip_ev, 0, sizeof(vip_ev));

    do {
        ret = v4l2_ioctl(fd_ctx->fd_vip, VIDIOC_DQEVENT, &vip_ev);
        if (ret < 0) {
            perror("ioctl VIDIOC_DQEVENT failed");
            return ret;
        }

        if ((vip_ev.type==V4L2_EVENT_SOURCE_CHANGE) &&
            (vip_ev.u.src_change.changes ==
             V4L2_EVENT_SRC_CH_RESOLUTION) ) {

            *src_change_event = true;
            printf("DQEVENT - V4L2_EVENT_SRC_CH_RESOLUTION\n");
        }
    } while (vip_ev.pending != 0);

    return 0;
}

static int update_video_standard(bool *std_change, struct glob_context *ctx)
{
    int ret;
    v4l2_std_id std_id;

    struct fd_context  *fd_ctx  = &ctx->fd_ctx;
    struct gst_context *gst_ctx = &ctx->gst_ctx;

    *std_change = false;

    ret = v4l2_ioctl(fd_ctx->fd_vip, VIDIOC_QUERYSTD, &std_id);
    if (ret < 0) {
        perror("ioctl VIDIOC_QUERYSTD failed");
        return ret;
    }

    if(std_id==0)
        std_id = DEFAULT_STANDARD;

    printf("new std = 0x%X - old_std = 0x%X\n",
           (unsigned int)std_id,
           (unsigned int)gst_ctx->norm );

    if(gst_ctx->norm != std_id)
    {
        *std_change = true;
        gst_ctx->norm = std_id;
    }

    return 0;
}

static int wait_for_ax_takeover(struct glob_context *ctx)
{
    struct fd_context *fd_ctx = &ctx->fd_ctx;
    struct pollfd *pfd = ctx->pfd;
    int ret;
    bool status;
    unsigned sig;
    struct signalfd_siginfo info;
    ssize_t bytes;
    bool got_sig_ev=0, got_rvc_event=0;

    while (1) {
        printf("** Wait Ax takeover\n");
        ret = poll(pfd, 2, -1);

        if (ret == -1) {
            fprintf(stderr, "%s: Poll error\n", __FILE__);
            break;
        }

        if (ret == 0) {
            fprintf(stderr, "%s: Poll timeout\n", __FILE__);
            break;
        }

        got_sig_ev=pfd[0].revents & POLLIN;
        got_rvc_event=pfd[1].revents & POLLIN;

        sig = 0;

        if (got_sig_ev) {
            pfd[0].revents = 0;

            bytes = read(pfd[0].fd, &info, sizeof(info));
            assert(bytes == sizeof(info));

            sig = info.ssi_signo;

            if ((sig == SIGINT) ||(sig == SIGTERM))
            {
                break;
            }
        }

        if (got_rvc_event) {
            DEBUG_PRINTF ("<- GOT RVC EVENT 0x%X->\n", pfd[1].revents);
            pfd[1].revents = 0;

            if (get_takeover_status_read(&status, fd_ctx))
                continue;
            else
                return 0;
        }
    }
    return -1;
}

static int set_poll_fd(struct glob_context *ctx)
{
    struct fd_context *fd_ctx   = &ctx->fd_ctx;
    sigset_t sigset;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGTERM);

    /* Block SIGUSR1 SIGINT and SIGTERM to avoid terminating processus */
    if (sigprocmask(SIG_BLOCK, &sigset, NULL) == -1) {
        perror("Cannot block signal SIGUSR1");
        return -1;
    }

    ctx->sig_fd = signalfd(-1, &sigset, 0);

    memset(ctx->pfd, 0, sizeof(ctx->pfd));

    ctx->pfd[0].fd = ctx->sig_fd;
    ctx->pfd[0].events = POLLIN;

    ctx->pfd[1].fd = fd_ctx->fd_rvc;
    ctx->pfd[1].events = POLLIN;

    ctx->pfd[2].fd = 0;
    ctx->pfd[2].events = POLLPRI;

    return 0;
}

static void mainloop(struct glob_context *ctx)
{
    struct fd_context *fd_ctx   = &ctx->fd_ctx;
    struct gst_context *gst_ctx = &ctx->gst_ctx;
    struct pollfd *pfd = ctx->pfd;

    bool rear_gear=false;
    bool rear_gear_prev=false;
    bool change=false;
    int num_poll=2;
    bool got_sig_ev=0, got_rvc_event=0, got_vip_ev=0;
    struct signalfd_siginfo info;
    unsigned sig;
    ssize_t bytes;
    int ret;

    get_rear_gear_status_ioctl(&rear_gear, fd_ctx);
    if (rear_gear) {
        if(ctx->autodetect)
        {
            pfd[2].fd = fd_ctx->fd_vip;
            num_poll=3;
        }
        gst_start_pipeline(ctx);
    }

    rear_gear_prev = rear_gear;

    while (1) {
        DEBUG_PRINTF("** Wait next event - num_poll=%d\n", num_poll);
        ret = poll(pfd, num_poll, -1);

        if (ret == -1) {
            fprintf(stderr, "%s: Poll error\n", __FILE__);
            break;
        }

        if (ret == 0) {
            fprintf(stderr, "%s: Poll timeout\n", __FILE__);
            break;
        }

        got_sig_ev=pfd[0].revents & POLLIN;
        got_rvc_event=pfd[1].revents & POLLIN;
        got_vip_ev=(num_poll == 3) && (pfd[2].revents & POLLPRI);

        sig = 0;

        if (got_sig_ev) {
            pfd[0].revents = 0;

            bytes = read(ctx->sig_fd, &info, sizeof(info));
            assert(bytes == sizeof(info));

            sig = info.ssi_signo;

            if (sig == SIGUSR1)
            {
                rear_gear = !rear_gear;
                DEBUG_PRINTF ("<- GOT SIGUSR1 SIGNAL - rear_gear %d->\n", rear_gear);

                if (rear_gear)
                {
                    if(ctx->autodetect)
                    {
                        pfd[2].fd = fd_ctx->fd_vip;
                        num_poll=3;
                    }

                    gst_start_pipeline(ctx);
                }
                else
                {
                    gst_stop_pipeline(ctx);
                    num_poll=2;
                }

                rear_gear_prev = rear_gear;
            }

            if ((sig == SIGINT) ||(sig == SIGTERM))
            {
                break;
            }
        }

        if (got_rvc_event) {
            DEBUG_PRINTF ("<- GOT RVC EVENT 0x%X->\n", pfd[1].revents);
            pfd[1].revents = 0;

            if (get_rear_gear_status_read(&rear_gear, fd_ctx)) {
                perror("Unable to get rear gear status");
                continue;
            }

            if(rear_gear_prev == rear_gear)
                continue;

            if (!fd_ctx->init_done) {
                if (!rear_gear) {
                    rear_gear_prev = rear_gear;
                    continue;
                }
            }

            if (rear_gear)
            {
                if(ctx->autodetect)
                {
                    pfd[2].fd = fd_ctx->fd_vip;
                    num_poll=3;
                }

                gst_start_pipeline(ctx);
            }
            else
            {
                gst_stop_pipeline(ctx);
                DEBUG_PRINTF("pipeline stopped\n");
                num_poll=2;
            }

            rear_gear_prev = rear_gear;
        }

        if (got_vip_ev) {
            DEBUG_PRINTF ("<- GOT VIP EVENT 0x%X->\n", pfd[2].revents);
            pfd[2].revents = 0;

            dequeue_events(&change, ctx);

            if(!rear_gear)
                continue;

            if(!change)
                continue;

            ret=update_video_standard(&change, ctx);
            if(ret) {
                fprintf(stderr, "Impossible to update video standard).\n");
            }

            if(!change)
                continue;

            /* Restart pipeline with new standard */
            gst_stop_pipeline(ctx);
            gst_start_pipeline(ctx);
        }
    }
}

static void usage(FILE * fp, int argc, char **argv)
{
    fprintf(fp,
            "Version: %s\n\n"
            "Usage: %s [options]\n\n"
            "Options:\n"
            "     -h | --help         Print this message\n"
            "     -m | --iomode       Select which io-mode should be use\n"
            "                            0(default) or 2 for Shared Memory\n"
            "                            4 for dmabuf\n"
            "                            5 for dmabuf-import\n"
            "                            Other values are forbidden\n"
            "     -s | --standard     Select which video standard should be use\n"
            "                            0(default) autodetect\n"
            "                            1 for PAL\n"
            "                            2 for NTSC\n"
            "                            Other values are forbidden\n"
            "     -r | --rescale      Use v4l2trans for rescale and/or color conversion\n"
            "     -d | --deinterlace  Use v4l2trans for deinterlace\n"
            "     -W <width>          Rescale width\n"
            "     -H <height>         Rescale height\n"
            "     -p | --libpath      Kernel Library path\n" ""
            ,VERSION, argv[0]);
}

static const char short_options[] = "hrdfW:H:p:m:s:";

static const struct option long_options[] = {
    {"help",        no_argument,       NULL, 'h'},
    {"fullscreen",  no_argument,       NULL, 'f'},
    {"rescale",     no_argument,       NULL, 'r'},
    {"deinterlace", no_argument,       NULL, 'd'},
    {"scale-width", required_argument, NULL, 'W'},
    {"scale-height",required_argument, NULL, 'H'},
    {"libpath",     required_argument, NULL, 'p'},
    {"iomode",      required_argument, NULL, 'm'},
    {"standard",    required_argument, NULL, 's'},
    {0, 0, 0, 0}
};

int main(int argc, char * argv[])
{
    struct glob_context context;
    struct fd_context *fd_ctx   = &context.fd_ctx;
    struct gst_context *gst_ctx = &context.gst_ctx;
    int std = 0;

    gst_ctx->io_mode = 0;
    gst_ctx->rescale = false;
    gst_ctx->deinterlace = false;
    gst_ctx->scale_width  = 0;
    gst_ctx->scale_height = 0;
    memset(gst_ctx->vid_device, 0, sizeof(gst_ctx->vid_device));

    for (;;) {
        int index;
        int c;

        c = getopt_long(argc, argv,
                        short_options, long_options, &index);

        if (-1 == c)
            break;

        switch (c) {
        case 0: /* getopt_long() flag */
            break;

        case 'h':
            usage(stdout, argc, argv);
            exit(EXIT_SUCCESS);

        case 'r':
            gst_ctx->rescale = true;
            break;

        case 'd':
            gst_ctx->deinterlace = true;
            break;

        case 'f':
            gst_ctx->rescale = true;
            gst_ctx->scale_width = SCREEN_WIDTH;
            gst_ctx->scale_height = SCREEN_HEIGHT;
            break;

        case 'W':
            gst_ctx->scale_width = atoi(optarg);
            break;

        case 'H':
            gst_ctx->scale_height = atoi(optarg);
            break;

        case 'm':
            if ((atoi(optarg) != 0) && (atoi(optarg) != 2)  &&
                (atoi(optarg) != 4) && (atoi(optarg) != 5)) {
                printf("Wrong IO-Mode: %d\n", atoi(optarg));
                exit(EXIT_FAILURE);
            }
            gst_ctx->io_mode = atoi(optarg);
            break;

        case 's':
            if ((atoi(optarg) != 0) && (atoi(optarg) != 1)  &&
                (atoi(optarg) != 2)) {
                printf("Wrong Video standard: %d\n", atoi(optarg));
                exit(EXIT_FAILURE);
            }

            std = atoi(optarg);
            break;

        case 'p':
            if (strlen(optarg) >= 255) {
                printf("Library path too long (max 254 characters)\n");
                exit(EXIT_FAILURE);
            }
            strcpy(fd_ctx->libpath, optarg);
            break;

        default:
            usage(stderr, argc, argv);
            exit(EXIT_FAILURE);
        }
    }

    switch(std)
    {
        case 0:
            gst_ctx->norm = DEFAULT_STANDARD;
            context.autodetect=true;
            break;

        case 1:
            gst_ctx->norm = V4L2_STD_PAL;
            context.autodetect=false;
            break;

        case 2:
            gst_ctx->norm = V4L2_STD_NTSC;
            context.autodetect=false;
            break;
    }

    if (strlen(fd_ctx->libpath) == 0) {
        fprintf(stderr, "No Library path has been specified\n");
        usage(stderr, argc, argv);
        return -1;
    }

    fd_ctx->fd_rvc = open (RPMSG_MM_DEVICE_NAME, O_RDWR);
    if (!fd_ctx->fd_rvc) {
        fprintf(stderr, "%s: Impossible to open.\n", RPMSG_MM_DEVICE_NAME);
        return -1;
    }

    fd_ctx->init_done = false;

    if (send_takeover_request(fd_ctx)) {
        fprintf(stderr, "Impossible to request RVC takeover to Ax\n");
        return -1;
    }

    if (set_poll_fd(&context)) {
        fprintf(stderr, "Failed to set poll fd\n");
        return -1;
    }

    if(wait_for_ax_takeover(&context))
    {
        fprintf(stderr, "Failed to wait for takeover event\n");
        return -1;
    }

    if(load_framework(&context)) {
        fprintf(stderr, "Impossible to load framewaork\n");
        return -1;
    }

    /* Initialisation */
    if (gst_init_check(&argc, &argv, NULL) == FALSE) {
        fprintf(stderr, "Impossible to initialize gstreamer.\n");
        return -1;
    }

    gst_ctx->pipeline = NULL;
    gst_ctx->loop = NULL;
    gst_ctx->bus_watch_id = 0;

    mainloop(&context);

    fprintf(stderr, "Exiting\n");

    if(fd_ctx->init_done) {
        gst_stop_pipeline(&context);
        v4l2_close(fd_ctx->fd_vip);
        unload_framework(&context);
        close(fd_ctx->fd_rvc);
    }

    gst_deinit ();
    return 0;
}

