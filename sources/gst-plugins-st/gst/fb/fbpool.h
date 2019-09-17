#ifndef __GST_FB_BUFFER_POOL_H__
#define __GST_FB_BUFFER_POOL_H__

#include <gst/gst.h>

#include "fbsink.h"
#include <gst/video/gstvideometa.h>

typedef struct _GstFbBufferPool GstFbBufferPool;
typedef struct _GstFbBufferPoolClass GstFbBufferPoolClass;
typedef struct _GstFbMeta GstFbMeta;

G_BEGIN_DECLS
#define GST_TYPE_FB_BUFFER_POOL      (gst_fb_buffer_pool_get_type())
#define GST_IS_FB_BUFFER_POOL(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_FB_BUFFER_POOL))
#define GST_FB_BUFFER_POOL(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_FB_BUFFER_POOL, GstFbBufferPool))
#define GST_FB_BUFFER_POOL_CAST(obj) ((GstFbBufferPool*)(obj))

struct _GstFbBufferPool
{
  GstBufferPool parent;
  GstFbSink *fbsink;

  GstAllocator *allocator;
  GstAllocationParams params;
  guint size, min_buffers, max_buffers;

  guint num_buffers;            /* number of buffers we use */
  guint num_allocated;          /* number of buffers allocated by the driver */
};

struct _GstFbBufferPoolClass
{
  GstBufferPoolClass parent_class;
};

struct _GstFbMeta
{
  GstMeta meta;
  guint yres;
};

GType gst_fb_meta_api_get_type (void);
const GstMetaInfo *gst_fb_meta_get_info (void);

#define GST_FB_META_GET(buf) ((GstFbMeta *)gst_buffer_get_meta(buf,gst_fb_meta_api_get_type()))
#define GST_FB_META_ADD(buf) ((GstFbMeta *)gst_buffer_add_meta(buf,gst_fb_meta_get_info(),NULL))

GType gst_fb_buffer_pool_get_type (void);

GstBufferPool *gst_fb_buffer_pool_new (GstCaps * caps, GstFbSink * fbsink,
    guint size);

G_END_DECLS
#endif /*__GST_FB_BUFFER_POOL_H__ */
