/*
 * Copyright (C) STMicroelectronic SA 2015
 * Author: Cyrille Simo Djokam <cyrille.simo@gmail.com> for ST
 *         Vincent Abriou <vincent.abriou@st.com> for ST
 */

/**
 * SECTION:element-gl360view2d
 *
 * glmixer sub element. N gl sink pads to 1 source pad.
 * N + 1 OpenGL contexts shared together.
 *
 * <refsect2>
 * <title>Examples</title>
 * |[
 * gst-launch-1.0 udpsrc caps=\"application/x-rtp, clock-rate=90000, encoding-name=H264, payload=96\" port=5003 ! queue ! rtph264depay ! h264parse ! queue ! v4l2dec ! queue ! v4l2trans ! video/x-raw,format=BGRA,width=1280,height=360 !  queue ! videorate ! video/x-raw,framerate=20/1 ! stmgl360view2d name=360view2d ! waylandsink sync=FALSE udpsrc caps=\"application/x-rtp, clock-rate=90000, encoding-name=H264, payload=96\" port=5004 ! queue ! rtph264depay ! h264parse ! queue ! v4l2dec ! queue ! v4l2trans ! video/x-raw,format=BGRA,width=1280,height=360 ! queue ! videorate ! video/x-raw,framerate=20/1 ! 360view2d. udpsrc caps=\"application/x-rtp, clock-rate=90000, encoding-name=H264, payload=96\" port=5002 ! queue ! rtph264depay ! h264parse ! queue ! v4l2dec ! queue ! v4l2trans ! video/x-raw,format=BGRA,width=320,height=720 ! queue ! videorate ! video/x-raw,framerate=20/1 ! 360view2d. udpsrc caps=\"application/x-rtp, clock-rate=90000, encoding-name=H264, payload=96\" port=5005 ! queue ! rtph264depay ! h264parse ! queue ! v4l2dec ! queue ! v4l2trans ! video/x-raw,format=BGRA,width=320,height=720 ! queue ! videorate ! video/x-raw,framerate=20/1 ! 360view2d.
 * ]| A pipeline mixing 4 BGRA videos
 * FBO (Frame Buffer Object) is required.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstgl360view2d.h"
#include <math.h>

#define GST_CAT_DEFAULT gst_gl_360_view2d_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

enum
{
  PROP_0,
};

#define DEBUG_INIT \
    GST_DEBUG_CATEGORY_INIT (gst_gl_360_view2d_debug, "stmgl360view2d", 0, "stmgl360view2d element");

G_DEFINE_TYPE_WITH_CODE (GstGL360view2d, gst_gl_360_view2d, GST_TYPE_GL_MIXER,
    DEBUG_INIT);

static void gst_gl_360_view2d_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_gl_360_view2d_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean _update_info (GstVideoAggregator * vagg, GstVideoInfo * info);
static void gst_gl_360_view2d_reset (GstGLMixer * mixer);
static gboolean gst_gl_360_view2d_init_shader (GstGLMixer * mixer,
    GstCaps * outcaps);

static gboolean gst_gl_360_view2d_process_textures (GstGLMixer * mixer,
    GPtrArray * in_frames, guint out_tex);
static void gst_gl_360_view2d_callback (gpointer stuff);

/* vertex source */
static const gchar *view2d_360_v_src =
    "attribute vec4 a_position;                                   \n"
    "attribute vec2 a_texCoord;                                   \n"
    "varying vec2 v_texCoord;                                     \n"
    "void main()                                                  \n"
    "{                                                            \n"
    "   gl_Position = a_position;                                 \n"
    "   v_texCoord = a_texCoord;                                  \n"
    "}";

/* fragment source */

 /* for each view 0,1,2,3, define the space region (angle value)
  * where the alpha will be applied
  * texCoord.x or texCoord.y = 1 => max size of the widht or the height of
  * image; = 1.0 / 2.0 => the half and so on...
  * */
  static const gchar *view2d_360_f_src =
    "#ifdef GL_ES                                                                                                                                                 \n"
    "precision mediump float;                                                                                                                                     \n"
    "#endif                                                                                                                                                       \n"
    "uniform sampler2D texture0;                                                                                                                                  \n"
    "uniform sampler2D texture1;                                                                                                                                  \n"
    "uniform float alpha;                                                                                                                                         \n"
    "uniform float angle;                                                                                                                                         \n"
    "uniform float blend_area;                                                                                                                                    \n"
    "varying vec2 v_texCoord;                                                                                                                                     \n"
    "void main()                                                                                                                                                  \n"
    "{                                                                                                                                                            \n"
    " float bl_area = blend_area;                                                                                                                                 \n"
    " float set_alpha = 1.0;                                                                                                                                      \n"
    " float PI = 3.14159265;                                                                                                                                      \n"
    " float angle_radian = angle * PI / 180.0;                                                                                                                    \n"
    " float theta = tan(angle_radian);                                                                                                                            \n"
    " if(angle == 0.0)                                                                                                                                            \n"
    "  set_alpha = 0.0;                                                                                                                                           \n"
    " else                                                                                                                                                        \n"
    " {                                                                                                                                                           \n"
    "  set_alpha = 1.0;                                                                                                                                           \n"
    " }                                                                                                                                                           \n"
    "  vec4 rgba = (texture2D( texture0, v_texCoord.xy) + texture2D(texture1,v_texCoord.xy) ) * 0.5;                                                              \n"
    " if(angle == -1.0)                                                                                                                                           \n"
    " {                                                                                                                                                           \n"
    "  if ( (bl_area == 0.0) && ( ( (v_texCoord.x <= 1.0 / 4.0) && (v_texCoord.y >= (4.0)*v_texCoord.x) && (v_texCoord.y <= 1.0)) ||                              \
            ( (v_texCoord.x >= 3.0 / 4.0) && (v_texCoord.y >= (-4.0)*v_texCoord.x + 4.0) && (v_texCoord.y <= 1.0 )) ) )                                           \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha / 8.0);                                                                                                      \n"
    " else if ( (bl_area == 1.0) && ( ( (v_texCoord.x <= 1.0 / 4.0) && (v_texCoord.y <= (-2.0)*v_texCoord.x + 1.0) ) ||                                           \
            ( (v_texCoord.x >= 3.0 / 4.0) && (v_texCoord.y <= (2.0) * v_texCoord.x - 1.0 ) ) ) )                                                                  \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha / 8.0);                                                                                                      \n"
    " else  if ( (bl_area == 2.0) && ( ( (v_texCoord.x <= 1.0) && (v_texCoord.y <= 1.0 / 4.0) && (v_texCoord.y <= (1.0 / 4.0)*v_texCoord.x)) ||                   \
            ( (v_texCoord.x <= 1.0) && (v_texCoord.y >= (-1.0 / 4.0) * v_texCoord.x + 1.0) ) ) )                                                                  \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha / 8.0);                                                                                                      \n"
    " else if ( (bl_area == 3.0) && ( ( (v_texCoord.x >= 0.0) && (v_texCoord.y <= (-1.0 / 4.0)*v_texCoord.x + 1.0 / 4.0)) ||                                      \
            ( (v_texCoord.x >= 0.0) && (v_texCoord.y >= (1.0 / 4.0) * v_texCoord.x + 3.0 / 4.0) && (v_texCoord.y >= 3.0 / 4.0) ) ) )                              \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha / 8.0);                                                                                                      \n"
    " else                                                                                                                                                        \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha);                                                                                                            \n"
    " }                                                                                                                                                           \n"
    " else                                                                                                                                                        \n"
    " {                                                                                                                                                           \n"
    "  if ( (set_alpha == 1.0) && (bl_area == 0.0) && ( ( (v_texCoord.x <= 1.0 / 4.0) && (v_texCoord.y >= (theta)*v_texCoord.x) && (v_texCoord.y <= 1.0)) ||      \
            ( (v_texCoord.x >= 3.0 / 4.0) && (v_texCoord.y >= (-theta)*v_texCoord.x + theta) && (v_texCoord.y <= 1.0 )) ) )                                       \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha / 8.0);                                                                                                      \n"
    " else if ( (set_alpha == 1.0) && (bl_area == 1.0) && ( ( (v_texCoord.x <= 1.0 / 4.0) && (v_texCoord.y <= (-theta)*v_texCoord.x + theta / 2.0) ) ||           \
            ( (v_texCoord.x >= 3.0 / 4.0) && (v_texCoord.y <= (theta) * v_texCoord.x - theta / 2.0 ) ) ) )                                                        \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha / 8.0);                                                                                                      \n"
    " else  if ( (set_alpha == 1.0) && (bl_area == 2.0) && ( ( (v_texCoord.x <= 1.0) && (v_texCoord.y <= 1.0 / 4.0) && (v_texCoord.y <= theta * v_texCoord.x)) || \
            ( (v_texCoord.x <= 1.0) && (v_texCoord.y >= (-theta) * v_texCoord.x + 4.0 * theta) ) ) )                                                              \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha / 8.0);                                                                                                      \n"
    " else if ( (set_alpha == 1.0) && (bl_area == 3.0) && ( ( (v_texCoord.x >= 0.0) && (v_texCoord.y <= (-theta) * v_texCoord.x + theta)) ||                      \
            ( (v_texCoord.x >= 0.0) && (v_texCoord.y >= theta * v_texCoord.x + 3.0 * theta) && (v_texCoord.y >= 3.0 / 4.0) ) ) )                                  \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha / 8.0);                                                                                                      \n"
    " else                                                                                                                                                        \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha);                                                                                                            \n"
    " }                                                                                                                                                           \n"
    "}\n";


static const gchar *view2d_360_f_ext_src =
    "#ifdef GL_ES                                                                                                                                                 \n"
    "precision mediump float;                                                                                                                                     \n"
    "#extension GL_OES_EGL_image_external : require                                                                                                               \n"
    "#endif                                                                                                                                                       \n"
    "uniform samplerExternalOES texture0;                                                                                                                         \n"
    "uniform samplerExternalOES texture1;                                                                                                                         \n"
    "uniform float alpha;                                                                                                                                         \n"
    "uniform float angle;                                                                                                                                         \n"
    "uniform float blend_area;                                                                                                                                    \n"
    "varying vec2 v_texCoord;                                                                                                                                     \n"
    "void main()                                                                                                                                                  \n"
    "{                                                                                                                                                            \n"
    " float bl_area = blend_area;                                                                                                                                 \n"
    " float set_alpha = 1.0;                                                                                                                                      \n"
    " float PI = 3.14159265;                                                                                                                                      \n"
    " float angle_radian = angle * PI / 180.0;                                                                                                                    \n"
    " float theta = tan(angle_radian);                                                                                                                            \n"
    " if(angle == 0.0)                                                                                                                                            \n"
    "  set_alpha = 0.0;                                                                                                                                           \n"
    " else                                                                                                                                                        \n"
    " {                                                                                                                                                           \n"
    "  set_alpha = 1.0;                                                                                                                                           \n"
    " }                                                                                                                                                           \n"
    "  vec4 rgba = (texture2D( texture0, v_texCoord.xy) + texture2D(texture1,v_texCoord.xy) ) * 0.5;                                                              \n"
    " if(angle == -1.0)                                                                                                                                           \n"
    " {                                                                                                                                                           \n"
    "  if ( (bl_area == 0.0) && ( ( (v_texCoord.x <= 1.0 / 4.0) && (v_texCoord.y >= (4.0)*v_texCoord.x) && (v_texCoord.y <= 1.0)) ||                              \
            ( (v_texCoord.x >= 3.0 / 4.0) && (v_texCoord.y >= (-4.0)*v_texCoord.x + 4.0) && (v_texCoord.y <= 1.0 )) ) )                                           \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha / 8.0);                                                                                                      \n"
    " else if ( (bl_area == 1.0) && ( ( (v_texCoord.x <= 1.0 / 4.0) && (v_texCoord.y <= (-2.0)*v_texCoord.x + 1.0) ) ||                                           \
            ( (v_texCoord.x >= 3.0 / 4.0) && (v_texCoord.y <= (2.0) * v_texCoord.x - 1.0 ) ) ) )                                                                  \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha / 8.0);                                                                                                      \n"
    " else  if ( (bl_area == 2.0) && ( ( (v_texCoord.x <= 1.0) && (v_texCoord.y <= 1.0 / 4.0) && (v_texCoord.y <= (1.0 / 4.0)*v_texCoord.x)) ||                   \
            ( (v_texCoord.x <= 1.0) && (v_texCoord.y >= (-1.0 / 4.0) * v_texCoord.x + 1.0) ) ) )                                                                  \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha / 8.0);                                                                                                      \n"
    " else if ( (bl_area == 3.0) && ( ( (v_texCoord.x >= 0.0) && (v_texCoord.y <= (-1.0 / 4.0)*v_texCoord.x + 1.0 / 4.0)) ||                                      \
            ( (v_texCoord.x >= 0.0) && (v_texCoord.y >= (1.0 / 4.0) * v_texCoord.x + 3.0 / 4.0) && (v_texCoord.y >= 3.0 / 4.0) ) ) )                              \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha / 8.0);                                                                                                      \n"
    " else                                                                                                                                                        \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha);                                                                                                            \n"
    " }                                                                                                                                                           \n"
    " else                                                                                                                                                        \n"
    " {                                                                                                                                                           \n"
    "  if ( (set_alpha == 1.0) && (bl_area == 0.0) && ( ( (v_texCoord.x <= 1.0 / 4.0) && (v_texCoord.y >= (theta)*v_texCoord.x) && (v_texCoord.y <= 1.0)) ||      \
            ( (v_texCoord.x >= 3.0 / 4.0) && (v_texCoord.y >= (-theta)*v_texCoord.x + theta) && (v_texCoord.y <= 1.0 )) ) )                                       \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha / 8.0);                                                                                                      \n"
    " else if ( (set_alpha == 1.0) && (bl_area == 1.0) && ( ( (v_texCoord.x <= 1.0 / 4.0) && (v_texCoord.y <= (-theta)*v_texCoord.x + theta / 2.0) ) ||           \
            ( (v_texCoord.x >= 3.0 / 4.0) && (v_texCoord.y <= (theta) * v_texCoord.x - theta / 2.0 ) ) ) )                                                        \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha / 8.0);                                                                                                      \n"
    " else  if ( (set_alpha == 1.0) && (bl_area == 2.0) && ( ( (v_texCoord.x <= 1.0) && (v_texCoord.y <= 1.0 / 4.0) && (v_texCoord.y <= theta * v_texCoord.x)) || \
            ( (v_texCoord.x <= 1.0) && (v_texCoord.y >= (-theta) * v_texCoord.x + 4.0 * theta) ) ) )                                                              \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha / 8.0);                                                                                                      \n"
    " else if ( (set_alpha == 1.0) && (bl_area == 3.0) && ( ( (v_texCoord.x >= 0.0) && (v_texCoord.y <= (-theta) * v_texCoord.x + theta)) ||                      \
            ( (v_texCoord.x >= 0.0) && (v_texCoord.y >= theta * v_texCoord.x + 3.0 * theta) && (v_texCoord.y >= 3.0 / 4.0) ) ) )                                  \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha / 8.0);                                                                                                      \n"
    " else                                                                                                                                                        \n"
    "   gl_FragColor = vec4(rgba.rgb, rgba.a * alpha);                                                                                                            \n"
    " }                                                                                                                                                           \n"
    "}\n";

#define GST_TYPE_GL_360_VIEW2D_PAD (gst_gl_360_view2d_pad_get_type())
#define GST_GL_360_VIEW2D_PAD(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_GL_360_VIEW2D_PAD, GstGL360view2dPad))
#define GST_GL_360_VIEW2D_PAD_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_GL_360_VIEW2D_PAD, GstGL360view2dPadClass))
#define GST_IS_GL_360_VIEW2D_PAD(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_GL_360_VIEW2D_PAD))
#define GST_IS_GL_360_VIEW2D_PAD_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_GL_360_VIEW2D_PAD))

typedef struct _GstGL360view2dPad GstGL360view2dPad;
typedef struct _GstGL360view2dPadClass GstGL360view2dPadClass;
typedef struct _GstGL360view2dCollect GstGL360view2dCollect;

/**
 * GstGL360view2dPad:
 *
 * The opaque #GstGL360view2dPad structure.
 */
struct _GstGL360view2dPad
{
  GstGLMixerPad parent;

  /* < private > */
  /* properties */
  gint xpos, ypos;
  gint width, height;
  gdouble alpha;
  gdouble angle;
};

struct _GstGL360view2dPadClass
{
  GstGLMixerPadClass parent_class;
};

GType gst_gl_360_view2d_pad_get_type (void);
G_DEFINE_TYPE (GstGL360view2dPad, gst_gl_360_view2d_pad,
    GST_TYPE_GL_MIXER_PAD);

static void gst_gl_360_view2d_pad_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_gl_360_view2d_pad_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

#define DEFAULT_PAD_XPOS   0
#define DEFAULT_PAD_YPOS   0
#define DEFAULT_PAD_WIDTH  0
#define DEFAULT_PAD_HEIGHT 0
#define DEFAULT_PAD_ALPHA  1.0
#define DEFAULT_PAD_ANGLE -1.0
enum
{
  PROP_PAD_0,
  PROP_PAD_XPOS,
  PROP_PAD_YPOS,
  PROP_PAD_WIDTH,
  PROP_PAD_HEIGHT,
  PROP_PAD_ALPHA,
  PROP_PAD_ANGLE
};

static void
gst_gl_360_view2d_pad_init (GstGL360view2dPad * pad)
{
  pad->alpha = 1.0;
  pad->angle = -1.0;
}

static void
gst_gl_360_view2d_pad_class_init (GstGL360view2dPadClass * klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;

  gobject_class->set_property = gst_gl_360_view2d_pad_set_property;
  gobject_class->get_property = gst_gl_360_view2d_pad_get_property;

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

 g_object_class_install_property (gobject_class, PROP_PAD_ANGLE,
      g_param_spec_double ("angle", "Angle", "Angle to apply alpha on picture", -2.0, 89.0,
          DEFAULT_PAD_ANGLE,
          G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS));

}

static void
gst_gl_360_view2d_pad_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstGL360view2dPad *pad = GST_GL_360_VIEW2D_PAD (object);

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
    case PROP_PAD_ANGLE:
      g_value_set_double (value, pad->angle);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_gl_360_view2d_pad_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstGL360view2dPad *pad = GST_GL_360_VIEW2D_PAD (object);
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
    case PROP_PAD_ANGLE:
      pad->angle = g_value_get_double (value);
     break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }

  gst_object_unref (mix);
}

static void
gst_gl_360_view2d_class_init (GstGL360view2dClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *element_class;
  GstAggregatorClass *agg_class = (GstAggregatorClass *) klass;
  GstVideoAggregatorClass *vagg_class = (GstVideoAggregatorClass *) klass;

  gobject_class = (GObjectClass *) klass;
  element_class = GST_ELEMENT_CLASS (klass);

  gobject_class->set_property = gst_gl_360_view2d_set_property;
  gobject_class->get_property = gst_gl_360_view2d_get_property;

  gst_element_class_set_metadata (element_class, "OpenGL 360 2D view",
      "Filter/Effect/Video/Compositor", "OpenGL 360 2D view",
      "Cyrille Simo Djokam <cyrille.simo@gmail.com> & "
      "Vincent Abriou <vincent.abriou@st.com> based on "
      "Julien Isorce <julien.isorce@gmail.com> glvideomixer work");

  GST_GL_MIXER_CLASS (klass)->set_caps = gst_gl_360_view2d_init_shader;
  GST_GL_MIXER_CLASS (klass)->reset = gst_gl_360_view2d_reset;
  GST_GL_MIXER_CLASS (klass)->process_textures =
      gst_gl_360_view2d_process_textures;

  vagg_class->update_info = _update_info;

  agg_class->sinkpads_type = GST_TYPE_GL_360_VIEW2D_PAD;
}

static void
gst_gl_360_view2d_init (GstGL360view2d * view2d_360)
{
  view2d_360->shader = NULL;
  view2d_360->shader_ext = NULL;
  view2d_360->input_frames = NULL;
}

static void
gst_gl_360_view2d_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_gl_360_view2d_get_property (GObject * object, guint prop_id,
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
    GstGL360view2dPad *mixer_pad = GST_GL_360_VIEW2D_PAD (vaggpad);
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
gst_gl_360_view2d_reset (GstGLMixer * mixer)
{
  GstGL360view2d *view2d_360 = GST_GL_360_VIEW2D (mixer);

  view2d_360->input_frames = NULL;

  if (view2d_360->shader)
    gst_gl_context_del_shader (mixer->context, view2d_360->shader);
  view2d_360->shader = NULL;

  if (view2d_360->shader_ext)
    gst_gl_context_del_shader (mixer->context, view2d_360->shader_ext);
  view2d_360->shader_ext = NULL;
}

static gboolean
gst_gl_360_view2d_init_shader (GstGLMixer * mixer, GstCaps * outcaps)
{
  GstGL360view2d *view2d_360 = GST_GL_360_VIEW2D (mixer);

  if (view2d_360->shader)
    gst_gl_context_del_shader (mixer->context, view2d_360->shader);

  if (view2d_360->shader_ext)
    gst_gl_context_del_shader (mixer->context, view2d_360->shader_ext);

  if (!gst_gl_context_gen_shader (mixer->context, view2d_360_v_src,
        view2d_360_f_src, &view2d_360->shader))
    return FALSE;

  if (!gst_gl_context_gen_shader (mixer->context, view2d_360_v_src,
        view2d_360_f_ext_src, &view2d_360->shader_ext))
    return FALSE;

  return TRUE;
}

static gboolean
gst_gl_360_view2d_process_textures (GstGLMixer * mix, GPtrArray * frames,
    guint out_tex)
{
  GstGL360view2d *view2d_360 = GST_GL_360_VIEW2D (mix);

  view2d_360->input_frames = frames;

  gst_gl_context_use_fbo_v2 (mix->context,
      GST_VIDEO_INFO_WIDTH (&GST_VIDEO_AGGREGATOR (mix)->info),
      GST_VIDEO_INFO_HEIGHT (&GST_VIDEO_AGGREGATOR (mix)->info),
      mix->fbo, mix->depthbuffer,
      out_tex, gst_gl_360_view2d_callback, (gpointer) view2d_360);

  return TRUE;
}

/* opengl scene, params: input texture (not the output mixer->texture) */
static void
gst_gl_360_view2d_callback (gpointer stuff)
{
  GstGL360view2d *view2d_360 = GST_GL_360_VIEW2D (stuff);
  GstGLMixer *mixer = GST_GL_MIXER (view2d_360);
  GstGLFuncs *gl = mixer->context->gl_vtable;

  GLint attr_position_loc = 0;
  GLint attr_texture_loc = 0;

  static GLfloat bl_area = 0;

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

  while (count < view2d_360->input_frames->len) {
    GstGLMixerFrameData *frame;
    GstGL360view2dPad *pad;
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
    GstGLShader *shader = view2d_360->shader;

    frame = g_ptr_array_index (view2d_360->input_frames, count);
    if (!frame) {
      GST_DEBUG ("skipping texture, null frame");
      count++;
      continue;
    }
    pad = (GstGL360view2dPad *) frame->pad;
    in_width = GST_VIDEO_INFO_WIDTH (&GST_VIDEO_AGGREGATOR_PAD (pad)->info);
    in_height = GST_VIDEO_INFO_HEIGHT (&GST_VIDEO_AGGREGATOR_PAD (pad)->info);
    in_format = GST_VIDEO_INFO_FORMAT(&GST_VIDEO_AGGREGATOR_PAD (pad)->info);

    if (in_format == GST_VIDEO_FORMAT_NV12) {
      shader = view2d_360->shader_ext;
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

      /*** layout   ***/

   /* Front */
   if(count == 0)
    {
       pad->xpos = 0;
       pad->ypos = 0;
       bl_area = 0.0;
    }

   /* Rear */
   if(count == 1)
    {
       pad->xpos = 0;
       pad->ypos = (out_height / 2);
       bl_area = 1.0f;
    }

   /* Left */
   if(count == 2)
    {
       pad->xpos = 0;
       pad->ypos = 0;
       bl_area = 2.0f;
    }

   /* Right */
   if(count == 3)
    {
       pad->xpos = (3 * out_width / 4);
       pad->ypos = 0;
       bl_area = 3.0f;
    }

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
    gst_gl_shader_set_uniform_1f (shader, "angle", pad->angle);
    gst_gl_shader_set_uniform_1f (view2d_360->shader, "blend_area",bl_area);
    gl->DrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    ++count;

    gl->DisableVertexAttribArray (attr_position_loc);
    gl->DisableVertexAttribArray (attr_texture_loc);

    gl->BindTexture (target, 0);
  }

  gl->Disable (GL_BLEND);

  gst_gl_context_clear_shader (mixer->context);
}

