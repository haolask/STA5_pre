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

/**
 * SECTION:element-glvideomixer
 *
 * glmixer sub element. N gl sink pads to 1 source pad.
 * N + 1 OpenGL contexts shared together.
 *
 * <refsect2>
 * <title>Examples</title>
 * |[
 * gst-launch-1.0 filesrc location=/home/root/720p_Pharrell_Williams-Happy_Official_Music_Video.mp4 ! qtdemux ! h264parse ! queue ! v4l2dec ! queue ! v4l2trans ! video/x-raw,format=BGRA ! queue ! stmglvideomixer name=m  sink_1::alpha=0.5 sink_1::xpos=200 ! waylandsink filesrc location=/home/root/Moments_of_Everyday_Life.mp4 ! qtdemux ! h264parse ! queue ! v4l2dec ! queue ! v4l2trans ! video/x-raw, format=BGRA, width=640, height=480 ! queue ! m.
 * ]| A pipeline mixing 2 BGRA videos with an alpha applied on the second video
 * FBO (Frame Buffer Object) is required.
 * |[
 * gst-launch-1.0 filesrc location=/home/root/720p_Pharrell_Williams-Happy_Official_Music_Video.mp4 ! qtdemux ! h264parse ! queue ! v4l2dec ! queue ! stmglvideomixer name=m  sink_1::alpha=0.5 sink_1::xpos=200 ! waylandsink filesrc location=/home/root/Moments_of_Everyday_Life.mp4 ! qtdemux ! h264parse ! queue ! v4l2dec ! queue ! v4l2trans ! video/x-raw, format=BGRA, width=640, height=480 ! queue ! m.
 * ]| A pipeline mixing 1 BGRA and 1 NV12 video with an alpha applied on the second video
 * FBO (Frame Buffer Object) is required.
 * |[
 * gst-launch-1.0 filesrc location=/home/root/720p_Pharrell_Williams-Happy_Official_Music_Video.mp4 ! qtdemux ! h264parse ! queue ! v4l2dec ! queue ! stmglvideomixer name=m  sink_1::alpha=0.5 sink_1::xpos=200 ! waylandsink filesrc location=/home/root/Moments_of_Everyday_Life.mp4 ! qtdemux ! h264parse ! queue ! v4l2dec ! queue ! m.
 * ]| A pipeline mixing 2 NV12 videos with an alpha applied on the second video
 * FBO (Frame Buffer Object) is required.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstglvideomixer.h"

#define GST_CAT_DEFAULT gst_gl_video_mix_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

enum
{
  PROP_0,
};

#define DEBUG_INIT \
    GST_DEBUG_CATEGORY_INIT (gst_gl_video_mix_debug, "glvideomixer", 0, "glvideomixer element");

G_DEFINE_TYPE_WITH_CODE (GstGLVideoMix, gst_gl_video_mix, GST_TYPE_GL_MIXER,
    DEBUG_INIT);

static void gst_gl_video_mix_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_gl_video_mix_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean _update_info (GstVideoAggregator * vagg, GstVideoInfo * info);
static void gst_gl_video_mix_reset (GstGLMixer * mixer);
static gboolean gst_gl_video_mix_init_shader (GstGLMixer * mixer,
    GstCaps * outcaps);

static gboolean gst_gl_video_mix_process_textures (GstGLMixer * mixer,
    GPtrArray * in_frames, guint out_tex);
static void gst_gl_video_mix_callback (gpointer stuff);

/* vertex source */
static const gchar *video_mix_v_src =
    "attribute vec4 a_position;                                   \n"
    "attribute vec2 a_texCoord;                                   \n"
    "varying vec2 v_texCoord;                                     \n"
    "void main()                                                  \n"
    "{                                                            \n"
    "   gl_Position = a_position;                                 \n"
    "   v_texCoord = a_texCoord;                                  \n"
    "}";

/* fragment source */
static const gchar *video_mix_f_src =
    "#ifdef GL_ES                                                 \n"
    "precision mediump float;                                     \n"
    "#endif                                                       \n"
    "uniform sampler2D texture;                                   \n"
    "uniform float alpha;                                         \n"
    "varying vec2 v_texCoord;                                     \n"
    "void main()                                                  \n"
    "{                                                            \n"
    "  vec4 rgba = texture2D( texture, v_texCoord );              \n"
    "  gl_FragColor = vec4(rgba.rgb, rgba.a * alpha);             \n"
    "}                                                            \n";

static const gchar *video_mix_f_ext_src =
    "#ifdef GL_ES                                                 \n"
    "#extension GL_OES_EGL_image_external : require               \n"
    "precision mediump float;                                     \n"
    "#endif                                                       \n"
    "uniform samplerExternalOES texture;                          \n"
    "uniform float alpha;                                         \n"
    "varying vec2 v_texCoord;                                     \n"
    "void main()                                                  \n"
    "{                                                            \n"
    "  vec4 rgba = texture2D( texture, v_texCoord );              \n"
    "  gl_FragColor = vec4(rgba.rgb, rgba.a * alpha);             \n"
    "}                                                            \n";


#define GST_TYPE_GL_VIDEO_MIX_PAD (gst_gl_video_mix_pad_get_type())
#define GST_GL_VIDEO_MIX_PAD(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_GL_VIDEO_MIX_PAD, GstGLVideoMixPad))
#define GST_GL_VIDEO_MIX_PAD_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_GL_VIDEO_MIX_PAD, GstGLVideoMixPadClass))
#define GST_IS_GL_VIDEO_MIX_PAD(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_GL_VIDEO_MIX_PAD))
#define GST_IS_GL_VIDEO_MIX_PAD_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_GL_VIDEO_MIX_PAD))

typedef struct _GstGLVideoMixerPad GstGLVideoMixPad;
typedef struct _GstGLVideoMixerPadClass GstGLVideoMixPadClass;
typedef struct _GstGLVideoMixerCollect GstGLVideoMixerCollect;

/**
 * GstGLVideoMixPad:
 *
 * The opaque #GstGLVideoMixPad structure.
 */
struct _GstGLVideoMixerPad
{
  GstGLMixerPad parent;

  /* < private > */
  /* properties */
  gint xpos, ypos;
  gint width, height;
  gdouble alpha;
};

struct _GstGLVideoMixerPadClass
{
  GstGLMixerPadClass parent_class;
};

GType gst_gl_video_mix_pad_get_type (void);
G_DEFINE_TYPE (GstGLVideoMixPad, gst_gl_video_mix_pad,
    GST_TYPE_GL_MIXER_PAD);

static void gst_gl_video_mix_pad_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_gl_video_mix_pad_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

#define DEFAULT_PAD_XPOS   0
#define DEFAULT_PAD_YPOS   0
#define DEFAULT_PAD_WIDTH  0
#define DEFAULT_PAD_HEIGHT 0
#define DEFAULT_PAD_ALPHA  1.0
enum
{
  PROP_PAD_0,
  PROP_PAD_XPOS,
  PROP_PAD_YPOS,
  PROP_PAD_WIDTH,
  PROP_PAD_HEIGHT,
  PROP_PAD_ALPHA
};

static void
gst_gl_video_mix_pad_init (GstGLVideoMixPad * pad)
{
  pad->alpha = 1.0;
}

static void
gst_gl_video_mix_pad_class_init (GstGLVideoMixPadClass * klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;

  gobject_class->set_property = gst_gl_video_mix_pad_set_property;
  gobject_class->get_property = gst_gl_video_mix_pad_get_property;

  g_object_class_install_property (gobject_class, PROP_PAD_XPOS,
      g_param_spec_int ("xpos", "X Position", "X Position of the picture",
          G_MININT, G_MAXINT, DEFAULT_PAD_XPOS,
          G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_PAD_YPOS,
      g_param_spec_int ("ypos", "Y Position", "Y Position of the picture",
          G_MININT, G_MAXINT, DEFAULT_PAD_YPOS,
          G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_PAD_WIDTH,
      g_param_spec_int ("width", "Width", "Width of the picture",
          G_MININT, G_MAXINT, DEFAULT_PAD_WIDTH,
          G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_PAD_HEIGHT,
      g_param_spec_int ("height", "Height", "Height of the picture",
          G_MININT, G_MAXINT, DEFAULT_PAD_HEIGHT,
          G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_PAD_ALPHA,
      g_param_spec_double ("alpha", "Alpha", "Alpha of the picture", 0.0, 1.0,
          DEFAULT_PAD_ALPHA,
          G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS));
}

static void
gst_gl_video_mix_pad_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstGLVideoMixPad *pad = GST_GL_VIDEO_MIX_PAD (object);

  switch (prop_id) {
    case PROP_PAD_XPOS:
      g_value_set_int (value, pad->xpos);
      break;
    case PROP_PAD_YPOS:
      g_value_set_int (value, pad->ypos);
      break;
    case PROP_PAD_WIDTH:
      g_value_set_int (value, pad->width);
      break;
    case PROP_PAD_HEIGHT:
      g_value_set_int (value, pad->height);
      break;
    case PROP_PAD_ALPHA:
      g_value_set_double (value, pad->alpha);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_gl_video_mix_pad_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstGLVideoMixPad *pad = GST_GL_VIDEO_MIX_PAD (object);
  GstGLMixer *mix = GST_GL_MIXER (gst_pad_get_parent (GST_PAD (pad)));

  switch (prop_id) {
    case PROP_PAD_XPOS:
      pad->xpos = g_value_get_int (value);
      break;
    case PROP_PAD_YPOS:
      pad->ypos = g_value_get_int (value);
      break;
    case PROP_PAD_WIDTH:
      pad->width = g_value_get_int (value);
      break;
    case PROP_PAD_HEIGHT:
      pad->height = g_value_get_int (value);
      break;
    case PROP_PAD_ALPHA:
      pad->alpha = g_value_get_double (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }

  gst_object_unref (mix);
}

static void
gst_gl_video_mix_class_init (GstGLVideoMixClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *element_class;
  GstAggregatorClass *agg_class = (GstAggregatorClass *) klass;
  GstVideoAggregatorClass *vagg_class = (GstVideoAggregatorClass *) klass;

  gobject_class = (GObjectClass *) klass;
  element_class = GST_ELEMENT_CLASS (klass);

  gobject_class->set_property = gst_gl_video_mix_set_property;
  gobject_class->get_property = gst_gl_video_mix_get_property;

  gst_element_class_set_metadata (element_class, "OpenGL video_mixer",
      "Filter/Effect/Video/Compositor", "OpenGL video_mixer",
      "Julien Isorce <julien.isorce@gmail.com>");

  GST_GL_MIXER_CLASS (klass)->set_caps = gst_gl_video_mix_init_shader;
  GST_GL_MIXER_CLASS (klass)->reset = gst_gl_video_mix_reset;
  GST_GL_MIXER_CLASS (klass)->process_textures =
      gst_gl_video_mix_process_textures;

  vagg_class->update_info = _update_info;

  agg_class->sinkpads_type = GST_TYPE_GL_VIDEO_MIX_PAD;
}

static void
gst_gl_video_mix_init (GstGLVideoMix * video_mixer)
{
  video_mixer->shader = NULL;
  video_mixer->shader_ext = NULL;
  video_mixer->input_frames = NULL;
}

static void
gst_gl_video_mix_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_gl_video_mix_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
_update_info (GstVideoAggregator * vagg, GstVideoInfo * info)
{
  GList *l;
  gint best_width = -1, best_height = -1;
  gboolean ret = FALSE;

  GST_OBJECT_LOCK (vagg);
  for (l = GST_ELEMENT (vagg)->sinkpads; l; l = l->next) {
    GstVideoAggregatorPad *vaggpad = l->data;
    GstGLVideoMixPad *mixer_pad = GST_GL_VIDEO_MIX_PAD (vaggpad);
    gint this_width, this_height;
    gint width, height;

    if (mixer_pad->width > 0)
      width = mixer_pad->width;
    else
      width = GST_VIDEO_INFO_WIDTH (&vaggpad->info);

    if (mixer_pad->height > 0)
      height = mixer_pad->height;
    else
      height = GST_VIDEO_INFO_HEIGHT (&vaggpad->info);

    if (width == 0 || height == 0)
      continue;

    this_width = width + MAX (mixer_pad->xpos, 0);
    this_height = height + MAX (mixer_pad->ypos, 0);

    if (best_width < this_width)
      best_width = this_width;
    if (best_height < this_height)
      best_height = this_height;
  }
  GST_OBJECT_UNLOCK (vagg);

  if (best_width > 0 && best_height > 0) {
    info->width = best_width;
    info->height = best_height;
    ret = TRUE;
  }

  return ret;
}

static void
gst_gl_video_mix_reset (GstGLMixer * mixer)
{
  GstGLVideoMix *video_mixer = GST_GL_VIDEO_MIX (mixer);

  video_mixer->input_frames = NULL;

  if (video_mixer->shader)
    gst_gl_context_del_shader (mixer->context, video_mixer->shader);
  video_mixer->shader = NULL;

  if (video_mixer->shader_ext)
    gst_gl_context_del_shader (mixer->context, video_mixer->shader_ext);
  video_mixer->shader_ext = NULL;
}

static gboolean
gst_gl_video_mix_init_shader (GstGLMixer * mixer, GstCaps * outcaps)
{
  GstGLVideoMix *video_mixer = GST_GL_VIDEO_MIX (mixer);

  if (video_mixer->shader)
    gst_gl_context_del_shader (mixer->context, video_mixer->shader);

  if (video_mixer->shader_ext)
    gst_gl_context_del_shader (mixer->context, video_mixer->shader_ext);

  if (!gst_gl_context_gen_shader (mixer->context, video_mix_v_src,
        video_mix_f_src, &video_mixer->shader))
    return FALSE;

  if (!gst_gl_context_gen_shader (mixer->context, video_mix_v_src,
        video_mix_f_ext_src, &video_mixer->shader_ext))
    return FALSE;

  return TRUE;
}

static gboolean
gst_gl_video_mix_process_textures (GstGLMixer * mix, GPtrArray * frames,
    guint out_tex)
{
  GstGLVideoMix *video_mixer = GST_GL_VIDEO_MIX (mix);

  video_mixer->input_frames = frames;

  gst_gl_context_use_fbo_v2 (mix->context,
      GST_VIDEO_INFO_WIDTH (&GST_VIDEO_AGGREGATOR (mix)->info),
      GST_VIDEO_INFO_HEIGHT (&GST_VIDEO_AGGREGATOR (mix)->info),
      mix->fbo, mix->depthbuffer,
      out_tex, gst_gl_video_mix_callback, (gpointer) video_mixer);

  return TRUE;
}

/* opengl scene, params: input texture (not the output mixer->texture) */
static void
gst_gl_video_mix_callback (gpointer stuff)
{
  GstGLVideoMix *video_mixer = GST_GL_VIDEO_MIX (stuff);
  GstGLMixer *mixer = GST_GL_MIXER (video_mixer);
  GstGLFuncs *gl = mixer->context->gl_vtable;

  GLint attr_position_loc = 0;
  GLint attr_texture_loc = 0;
  guint out_width, out_height;

  const GLushort indices[] = {
    0, 1, 2,
    0, 2, 3
  };

  guint count = 0;

  out_width = GST_VIDEO_INFO_WIDTH (&GST_VIDEO_AGGREGATOR (stuff)->info);
  out_height = GST_VIDEO_INFO_HEIGHT (&GST_VIDEO_AGGREGATOR (stuff)->info);

  gl->Disable (GL_DEPTH_TEST);
  gl->Disable (GL_CULL_FACE);

  gl->ClearColor (0.0, 0.0, 0.0, 0.0);
  gl->Clear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  gl->Enable (GL_BLEND);

  while (count < video_mixer->input_frames->len) {
    GstGLMixerFrameData *frame;
    GstGLVideoMixPad *pad;
    /* *INDENT-OFF* */
    gfloat v_vertices[] = {
      /* front face */
      -1.0,-1.0,-1.0f, 0.0f, 0.0f,
       1.0,-1.0,-1.0f, 1.0f, 0.0f,
       1.0, 1.0,-1.0f, 1.0f, 1.0f,
      -1.0, 1.0,-1.0f, 0.0f, 1.0f,
    };
    /* *INDENT-ON* */
    guint in_tex;
    guint in_width, in_height;
    GstVideoFormat in_format;
    guint pad_width, pad_height;
    gfloat w, h;
    GLenum target = GL_TEXTURE_2D;
    GstGLShader *shader = video_mixer->shader;

    frame = g_ptr_array_index (video_mixer->input_frames, count);
    if (!frame) {
      GST_DEBUG ("skipping texture, null frame");
      count++;
      continue;
    }
    pad = (GstGLVideoMixPad *) frame->pad;
    in_width = GST_VIDEO_INFO_WIDTH (&GST_VIDEO_AGGREGATOR_PAD (pad)->info);
    in_height = GST_VIDEO_INFO_HEIGHT (&GST_VIDEO_AGGREGATOR_PAD (pad)->info);
    in_format = GST_VIDEO_INFO_FORMAT(&GST_VIDEO_AGGREGATOR_PAD (pad)->info);

    if (in_format == GST_VIDEO_FORMAT_NV12) {
      shader = video_mixer->shader_ext;
      target = GL_TEXTURE_EXTERNAL_OES;
    }

    gst_gl_context_clear_shader (mixer->context);

    gst_gl_shader_use (shader);

    attr_position_loc =
        gst_gl_shader_get_attribute_location (shader, "a_position");
    attr_texture_loc =
        gst_gl_shader_get_attribute_location (shader, "a_texCoord");

    if (!frame->texture || in_width <= 0 || in_height <= 0
        || pad->alpha == 0.0f) {
      GST_DEBUG ("skipping texture:%u frame:%p width:%u height:%u alpha:%f",
          frame->texture, frame, in_width, in_height, pad->alpha);
      count++;
      continue;
    }

    in_tex = frame->texture;
    pad_width = pad->width <= 0 ? in_width : pad->width;
    pad_height = pad->height <= 0 ? in_height : pad->height;

    w = ((gfloat) pad_width / (gfloat) out_width);
    h = ((gfloat) pad_height / (gfloat) out_height);

    /* top-left */
    v_vertices[0] = v_vertices[15] =
        2.0f * (gfloat) pad->xpos / (gfloat) out_width - 1.0f;
    /* bottom-left */
    v_vertices[1] = v_vertices[6] =
        2.0f * (gfloat) pad->ypos / (gfloat) out_height - 1.0f;
    /* top-right */
    v_vertices[5] = v_vertices[10] = v_vertices[0] + 2.0f * w;
    /* bottom-right */
    v_vertices[11] = v_vertices[16] = v_vertices[1] + 2.0f * h;
    GST_TRACE ("processing texture:%u dimensions:%ux%u, at %f,%f %fx%f with "
        "alpha:%f", in_tex, in_width, in_height, v_vertices[0], v_vertices[1],
        v_vertices[5], v_vertices[11], pad->alpha);

    gl->VertexAttribPointer (attr_position_loc, 3, GL_FLOAT,
        GL_FALSE, 5 * sizeof (GLfloat), &v_vertices[0]);

    gl->VertexAttribPointer (attr_texture_loc, 2, GL_FLOAT,
        GL_FALSE, 5 * sizeof (GLfloat), &v_vertices[3]);

    gl->EnableVertexAttribArray (attr_position_loc);
    gl->EnableVertexAttribArray (attr_texture_loc);

    gl->BlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl->BlendEquation (GL_FUNC_ADD);

    gl->ActiveTexture (GL_TEXTURE0);
    gl->BindTexture (target, in_tex);
    gst_gl_shader_set_uniform_1i (shader, "texture", 0);
    gst_gl_shader_set_uniform_1f (shader, "alpha", pad->alpha);

    gl->DrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    ++count;

    gl->DisableVertexAttribArray (attr_position_loc);
    gl->DisableVertexAttribArray (attr_texture_loc);

    gl->BindTexture (target, 0);
  }

  gl->Disable (GL_BLEND);

  gst_gl_context_clear_shader (mixer->context);
}
