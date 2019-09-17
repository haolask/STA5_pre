/* 
 * GStreamer
 * Copyright (C) 2008 Julien Isorce <julien.isorce@gmail.com>
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

#ifndef _GST_GL_CUBE_H_
#define _GST_GL_CUBE_H_

#include <gst/gl/gstglfilter.h>

G_BEGIN_DECLS

#define GST_TYPE_GL_CUBE            (gst_gl_cube_get_type())
#define GST_GL_CUBE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_GL_CUBE,GstGLCube))
#define GST_IS_GL_CUBE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_GL_CUBE))
#define GST_GL_CUBE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass) ,GST_TYPE_GL_CUBE,GstGLCubeClass))
#define GST_IS_GL_CUBE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass) ,GST_TYPE_GL_CUBE))
#define GST_GL_CUBE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj) ,GST_TYPE_GL_CUBE,GstGLCubeClass))

typedef struct _GstGLCube GstGLCube;
typedef struct _GstGLCubeClass GstGLCubeClass;

struct _GstGLCube
{
    GstGLFilter filter;

    GstGLShader *shader;
    GstGLShader *shader_ext;

    /* background color */
    gfloat red;
    gfloat green;
    gfloat blue;

    /* perspective */
    gdouble fovy;
    gdouble aspect;
    gdouble znear;
    gdouble zfar;
};

struct _GstGLCubeClass
{
    GstGLFilterClass filter_class;
};

GType gst_gl_cube_get_type (void);

G_END_DECLS

#endif /* _GST_GL_CUBE_H_ */
