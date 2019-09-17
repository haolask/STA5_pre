/*
 * GStreamer
 * Copyright (C) 2009 Julien Isorce <julien.isorce@gmail.com>
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

/*
 * Copyright (C) STMicroelectronic SA 2015
 * Update current implementation to add support of texture zero-copy
 * with BGRA and NV12 pixel formats.
 * Author: Vincent Abriou <vincent.abriou@st.com> for ST
 */

#ifndef _GST_GL_VIDEO_MIXER_H_
#define _GST_GL_VIDEO_MIXER_H_

#include <gst/gl/gstglmixer.h>
#include <gst/gl/gstglmixerpad.h>

G_BEGIN_DECLS

#define GST_TYPE_GL_VIDEO_MIX            (gst_gl_video_mix_get_type())
#define GST_GL_VIDEO_MIX(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_GL_VIDEO_MIXER,GstGLVideoMix))
#define GST_IS_GL_VIDEO_MIX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_GL_VIDEO_MIXER))
#define GST_GL_VIDEO_MIX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass) ,GST_TYPE_GL_VIDEO_MIXER,GstGLVideoMixClass))
#define GST_IS_GL_VIDEO_MIX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass) ,GST_TYPE_GL_VIDEO_MIXER))
#define GST_GL_VIDEO_MIX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj) ,GST_TYPE_GL_VIDEO_MIXER,GstGLVideoMixClass))

typedef struct _GstGLVideoMix GstGLVideoMix;
typedef struct _GstGLVideoMixClass GstGLVideoMixClass;

struct _GstGLVideoMix
{
    GstGLMixer mixer;

    GstGLShader *shader;
    GstGLShader *shader_ext;
    GPtrArray *input_frames;
};

struct _GstGLVideoMixClass
{
    GstGLMixerClass mixer_class;
};

GType gst_gl_video_mix_get_type (void);

G_END_DECLS

#endif /* _GST_GLFILTERCUBE_H_ */
