#ifndef __GST_FBSINK_H__
#define __GST_FBSINK_H__


#include <gst/video/gstvideosink.h>

#include <string.h>
#include <math.h>

/* Helper functions */
#include <gst/video/video.h>

#include <gst/gst.h>
#include <gst/base/gstbasesink.h>
#include <glib-object.h>
#include <linux/fb.h>

G_BEGIN_DECLS
#define GST_TYPE_FB_SINK \
  (gst_fbsink_get_type())
#define GST_FB_SINK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_FB_SINK, GstFbSink))
#define GST_FB_SINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_FB_SINK, GstFbSinkClass))
#define GST_IS_FB_SINK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_FB_SINK))
#define GST_IS_FB_SINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_FB_SINK))
typedef struct _GstFbSink GstFbSink;
typedef struct _GstFbSinkClass GstFbSinkClass;


struct _GstFbSink
{
  /* Our element stuff */
  GstVideoSink videosink;

  gchar *device_name;

  GstVideoInfo info;
  GstBuffer *framebuffer;

  GstCaps *caps;
  gpointer *data;
  gint fd;
  gint num_buffers, max_buffers;

  /* size of  all buffers */
  gint mapsize;
  /* size of one buffer */
  gint size;

  /* the buffer pool */
  GstBufferPool *pool;

  struct fb_var_screeninfo fb_varinfo;
  struct fb_fix_screeninfo fb_fixinfo;
};

struct _GstFbSinkClass
{
  GstVideoSinkClass parent_class;
};

GType gst_fbsink_get_type (void);

G_END_DECLS
#endif /* __GST_FBSINK_H__ */
