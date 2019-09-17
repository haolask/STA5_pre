/* GStreamer DRM/KMS video sink plugin
 *
 * Copyright (C) 2015 STMicroelectronics SA
 *
 * Author: Benjamin Gaignard <benjamin.gaignard@st.com> for STMicroelectronics.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __KMS_POOL_H__
#define __KMS_POOL_H__

#include "kmssink.h"
#include "drm_fourcc.h"
#include <gst/video/gstvideometa.h>

G_BEGIN_DECLS
#define GST_TYPE_KMS_POOL      (gst_kms_pool_get_type())
#define GST_IS_KMS_POOL(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_KMS_POOL))
#define GST_KMS_POOL(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_KMS_POOL, GstKMSPool))
#define GST_KMS_POOL_CAST(obj) ((GstKMSPool*)(obj))
typedef struct _GstKMSPool GstKMSPool;
typedef struct _GstKMSPoolClass GstKMSPoolClass;

struct _GstKMSPool
{
  GstBufferPool parent;
  GstKMSSink *kmssink;

  GstAllocator *allocator;
  GstAllocationParams params;
};

struct _GstKMSPoolClass
{
  GstBufferPoolClass parent_class;
};

GType gst_kms_meta_api_get_type (void);
const GstMetaInfo *gst_kms_meta_get_info (void);
#define GST_KMS_META_GET(buf) ((GstKMSMeta *)gst_buffer_get_meta(buf,gst_kms_meta_api_get_type()))
#define GST_KMS_META_ADD(buf) ((GstKMSMeta *)gst_buffer_add_meta(buf,gst_kms_meta_get_info(),NULL))

GType gst_kms_pool_get_type (void);

guint format_gst_to_fourcc (GstVideoFormat gst_format);

GstBufferPool *gst_kms_pool_new (GstCaps * caps, GstKMSSink * kmssink,
    guint max);

G_END_DECLS
#endif /* __KMS_POOL_H__ */
