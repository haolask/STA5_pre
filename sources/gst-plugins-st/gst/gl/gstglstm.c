/*
 * Copyright (C) STMicroelectronic SA 2015
 * Author: Vincent Abriou <vincent.abriou@st.com> for ST
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstglcube.h"
#include "gstglvideomixer.h"
#include "gstgl360view2d.h"

#define GST_CAT_DEFAULT gst_debug_glstm
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

/* Register filters that make up the gstgl plugin */
static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (gst_debug_glstm, "glstm", 0, "glstm");

  if (!gst_element_register (plugin, "stmglcube",
          GST_RANK_NONE, GST_TYPE_GL_CUBE)) {
    return FALSE;
  }

  if (!gst_element_register (plugin, "stmglvideomixer",
          GST_RANK_NONE, GST_TYPE_GL_VIDEO_MIX)) {
    return FALSE;
  }

  if (!gst_element_register (plugin, "stmgl360view2d",
          GST_RANK_NONE, GST_TYPE_GL_360_VIEW2D)) {
    return FALSE;
  }

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    glstm,
    "STM gl plugins, shall only be used for internal ST purposes",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
