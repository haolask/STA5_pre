/*
 * Copyright (C) STMicroelectronic SA 2015
 * Author: Cyrille Simo Djokam <cyrille.simo@gmail.com> for ST
 *         Vincent Abriou <vincent.abriou@st.com> for ST
 */

#ifndef _GST_GL_360_VIEW2D_H_
#define _GST_GL_360_VIEW2D_H_

#include <gst/gl/gstglmixer.h>
#include <gst/gl/gstglmixerpad.h>

G_BEGIN_DECLS


#define GST_TYPE_GL_360_VIEW2D            (gst_gl_360_view2d_get_type())
#define GST_GL_360_VIEW2D(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_GL_360_VIEW2D,GstGL360view2d))
#define GST_IS_GL_360_VIEW2D(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_GL_360_VIEW2D))
#define GST_GL_360_VIEW2D_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass) ,GST_TYPE_GL_360_VIEW2D,GstGL360view2dClass))
#define GST_IS_GL_360_VIEW2D_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass) ,GST_TYPE_GL_360_VIEW2D))
#define GST_GL_360_VIEW2D_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj) ,GST_TYPE_GL_360_VIEW2D,GstGL360view2dClass))

typedef struct _GstGL360view2d GstGL360view2d;
typedef struct _GstGL360view2dClass GstGL360view2dClass;

struct _GstGL360view2d
{
    GstGLMixer mixer;

    GstGLShader *shader;
    GstGLShader *shader_ext;
    GPtrArray *input_frames;
};

struct _GstGL360view2dClass
{
    GstGLMixerClass mixer_class;
};

GType gst_gl_360_view2d_get_type (void);

G_END_DECLS

#endif /* _GST_GL_360_VIEW2D_H_ */

