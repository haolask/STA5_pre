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

/**
 * SECTION:element-stmglcube
 *
 * The resize and redraw callbacks can be set from a client code.
 *
 * <refsect2>
 * <title>Examples</title>
 * |[
 * gst-launch-1.0 filesrc location=/home/root/Irma.mp4 ! qtdemux ! h264parse ! queue ! v4l2dec ! queue ! stmglcube ! waylandsink
 * gst-launch -v videotestsrc ! stmglcube ! waylandsink
 * ]| A pipeline to map a NV12 video textures  on the 6 cube faces..
 * FBO is required.
 * |[
 * gst-launch-1.0 filesrc location=/home/root/Irma.mp4 ! qtdemux ! h264parse ! queue ! v4l2dec ! queue ! v4l2trans ! video/x-raw, format=BGRA ! stmglcube ! waylandsink
 * gst-launch -v videotestsrc ! stmglcube ! waylandsink
 * ]| A pipeline to map a BGRA video textures  on the 6 cube faces..
 * FBO is required.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gl/gstglapi.h>
#include "gstglcube.h"

#define GST_CAT_DEFAULT gst_gl_cube_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

enum
{
  PROP_0,
  PROP_RED,
  PROP_GREEN,
  PROP_BLUE,
  PROP_FOVY,
  PROP_ASPECT,
  PROP_ZNEAR,
  PROP_ZFAR
};

#define DEBUG_INIT \
    GST_DEBUG_CATEGORY_INIT (gst_gl_cube_debug, "glcube", 0, "stm glcube element");

G_DEFINE_TYPE_WITH_CODE (GstGLCube, gst_gl_cube,
    GST_TYPE_GL_FILTER, DEBUG_INIT);

static void gst_gl_filter_cube_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_gl_filter_cube_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_gl_filter_cube_set_caps (GstGLFilter * filter,
    GstCaps * incaps, GstCaps * outcaps);
static void gst_gl_filter_cube_reset (GstGLFilter * filter);
static gboolean gst_gl_filter_cube_init_shader (GstGLFilter * filter);
static void _callback_gles2 (gint width, gint height, guint texture,
    gpointer stuff);
static gboolean gst_gl_filter_cube_filter_texture (GstGLFilter * filter,
    guint in_tex, guint out_tex);

/* vertex source */
static const gchar *cube_v_src =
    "attribute vec4 a_position;                                   \n"
    "attribute vec2 a_texCoord;                                   \n"
    "uniform mat4 u_matrix;                                       \n"
    "uniform float xrot_degree, yrot_degree, zrot_degree;         \n"
    "varying vec2 v_texCoord;                                     \n"
    "void main()                                                  \n"
    "{                                                            \n"
    "   float PI = 3.14159265;                                    \n"
    "   float xrot = xrot_degree*2.0*PI/360.0;                    \n"
    "   float yrot = yrot_degree*2.0*PI/360.0;                    \n"
    "   float zrot = zrot_degree*2.0*PI/360.0;                    \n"
    "   mat4 matX = mat4 (                                        \n"
    "            1.0,        0.0,        0.0, 0.0,                \n"
    "            0.0,  cos(xrot),  sin(xrot), 0.0,                \n"
    "            0.0, -sin(xrot),  cos(xrot), 0.0,                \n"
    "            0.0,        0.0,        0.0, 1.0 );              \n"
    "   mat4 matY = mat4 (                                        \n"
    "      cos(yrot),        0.0, -sin(yrot), 0.0,                \n"
    "            0.0,        1.0,        0.0, 0.0,                \n"
    "      sin(yrot),        0.0,  cos(yrot), 0.0,                \n"
    "            0.0,        0.0,       0.0,  1.0 );              \n"
    "   mat4 matZ = mat4 (                                        \n"
    "      cos(zrot),  sin(zrot),        0.0, 0.0,                \n"
    "     -sin(zrot),  cos(zrot),        0.0, 0.0,                \n"
    "            0.0,        0.0,        1.0, 0.0,                \n"
    "            0.0,        0.0,        0.0, 1.0 );              \n"
    "   gl_Position = u_matrix * matZ * matY * matX * a_position; \n"
    "   v_texCoord = a_texCoord;                                  \n"
    "}                                                            \n";

/* fragment source */
static const gchar *cube_f_src =
    "precision mediump float;                                    \n"
    "varying vec2 v_texCoord;                                    \n"
    "uniform sampler2D s_texture;                                \n"
    "void main()                                                 \n"
    "{                                                           \n"
    "  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
    "}                                                           \n";

/* fragment source */
static const gchar *cube_f_ext_src =
    "#extension GL_OES_EGL_image_external : require              \n"
    "precision mediump float;                                    \n"
    "varying vec2 v_texCoord;                                    \n"
    "uniform samplerExternalOES s_texture;                       \n"
    "void main()                                                 \n"
    "{                                                           \n"
    "  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
    "}                                                           \n";

static void
gst_gl_cube_class_init (GstGLCubeClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *element_class;

  gobject_class = (GObjectClass *) klass;
  element_class = GST_ELEMENT_CLASS (klass);

  gobject_class->set_property = gst_gl_filter_cube_set_property;
  gobject_class->get_property = gst_gl_filter_cube_get_property;

  GST_GL_FILTER_CLASS (klass)->onInitFBO = gst_gl_filter_cube_init_shader;
  GST_GL_FILTER_CLASS (klass)->onReset = gst_gl_filter_cube_reset;
  GST_GL_FILTER_CLASS (klass)->set_caps = gst_gl_filter_cube_set_caps;
  GST_GL_FILTER_CLASS (klass)->filter_texture =
      gst_gl_filter_cube_filter_texture;

  g_object_class_install_property (gobject_class, PROP_RED,
      g_param_spec_float ("red", "Red", "Background red color",
          0.0f, 1.0f, 0.0f, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_GREEN,
      g_param_spec_float ("green", "Green", "Background reen color",
          0.0f, 1.0f, 0.0f, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_BLUE,
      g_param_spec_float ("blue", "Blue", "Background blue color",
          0.0f, 1.0f, 0.0f, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_FOVY,
      g_param_spec_double ("fovy", "Fovy", "Field of view angle in degrees",
          0.0, 180.0, 45.0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_ASPECT,
      g_param_spec_double ("aspect", "Aspect",
          "Field of view in the x direction", 0.0, 100, 0.0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_ZNEAR,
      g_param_spec_double ("znear", "Znear",
          "Specifies the distance from the viewer to the near clipping plane",
          0.0, 100.0, 0.1, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_ZFAR,
      g_param_spec_double ("zfar", "Zfar",
          "Specifies the distance from the viewer to the far clipping plane",
          0.0, 1000.0, 100.0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_metadata (element_class, "OpenGL cube texture",
      "Filter/Effect/Video", "Map input texture on the 6 cube faces. It supports DMABUF zero copy texture in NV12 or BGRA format",
      "Vincent Abriou <vincent.abriou@st.com> mostly inspired by Julien Isorce <julien.isorce@gmail.com> glfiltercube plugin");
}

static void
gst_gl_cube_init (GstGLCube * filter)
{
  filter->shader = NULL;
  filter->shader_ext = NULL;
  filter->fovy = 45;
  filter->aspect = 0;
  filter->znear = 0.1;
  filter->zfar = 100;
}

static void
gst_gl_filter_cube_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstGLCube *filter = GST_GL_CUBE (object);

  switch (prop_id) {
    case PROP_RED:
      filter->red = g_value_get_float (value);
      break;
    case PROP_GREEN:
      filter->green = g_value_get_float (value);
      break;
    case PROP_BLUE:
      filter->blue = g_value_get_float (value);
      break;
    case PROP_FOVY:
      filter->fovy = g_value_get_double (value);
      break;
    case PROP_ASPECT:
      filter->aspect = g_value_get_double (value);
      break;
    case PROP_ZNEAR:
      filter->znear = g_value_get_double (value);
      break;
    case PROP_ZFAR:
      filter->zfar = g_value_get_double (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_gl_filter_cube_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstGLCube *filter = GST_GL_CUBE (object);

  switch (prop_id) {
    case PROP_RED:
      g_value_set_float (value, filter->red);
      break;
    case PROP_GREEN:
      g_value_set_float (value, filter->green);
      break;
    case PROP_BLUE:
      g_value_set_float (value, filter->blue);
      break;
    case PROP_FOVY:
      g_value_set_double (value, filter->fovy);
      break;
    case PROP_ASPECT:
      g_value_set_double (value, filter->aspect);
      break;
    case PROP_ZNEAR:
      g_value_set_double (value, filter->znear);
      break;
    case PROP_ZFAR:
      g_value_set_double (value, filter->zfar);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_gl_filter_cube_set_caps (GstGLFilter * filter, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstGLCube *cube_filter = GST_GL_CUBE (filter);

  if (cube_filter->aspect == 0)
    cube_filter->aspect = (gdouble) GST_VIDEO_INFO_WIDTH (&filter->out_info) /
        (gdouble) GST_VIDEO_INFO_HEIGHT (&filter->out_info);

  return TRUE;
}

static void
gst_gl_filter_cube_reset (GstGLFilter * filter)
{
  GstGLCube *cube_filter = GST_GL_CUBE (filter);

  /* blocking call, wait the opengl thread has destroyed the shaders */
  if (cube_filter->shader)
    gst_gl_context_del_shader (filter->context, cube_filter->shader);
  cube_filter->shader = NULL;

  if (cube_filter->shader_ext)
    gst_gl_context_del_shader (filter->context, cube_filter->shader_ext);
  cube_filter->shader_ext = NULL;
}

static gboolean
gst_gl_filter_cube_init_shader (GstGLFilter * filter)
{
  GstGLCube *cube_filter = GST_GL_CUBE (filter);

  if (gst_gl_context_get_gl_api (filter->context) & GST_GL_API_GLES2) {
    if (!gst_gl_context_gen_shader (filter->context,
          cube_v_src, cube_f_src,
          &cube_filter->shader))
      return FALSE;

    if (!gst_gl_context_gen_shader (filter->context,
          cube_v_src, cube_f_ext_src,
          &cube_filter->shader_ext))
      return FALSE;
  }
  return TRUE;
}

static gboolean
gst_gl_filter_cube_filter_texture (GstGLFilter * filter, guint in_tex,
    guint out_tex)
{
  GstGLCube *cube_filter = GST_GL_CUBE (filter);
  GLCB cb = NULL;
  GstGLAPI api;

  api = gst_gl_context_get_gl_api (GST_GL_FILTER (cube_filter)->context);

  if (api & GST_GL_API_GLES2)
    cb = _callback_gles2;

  /* blocking call, use a FBO */
  gst_gl_context_use_fbo (filter->context,
      GST_VIDEO_INFO_WIDTH (&filter->out_info),
      GST_VIDEO_INFO_HEIGHT (&filter->out_info),
      filter->fbo, filter->depthbuffer, out_tex,
      cb,
      GST_VIDEO_INFO_WIDTH (&filter->in_info),
      GST_VIDEO_INFO_HEIGHT (&filter->in_info),
      in_tex, cube_filter->fovy, cube_filter->aspect,
      cube_filter->znear, cube_filter->zfar,
      GST_GL_DISPLAY_PROJECTION_PERSPECTIVE, (gpointer) cube_filter);

  return TRUE;
}

/* opengl scene, params: input texture (not the output filter->texture) */
static void
_callback_gles2 (gint width, gint height, guint texture, gpointer stuff)
{
  GstGLFilter *filter = GST_GL_FILTER (stuff);
  GstGLCube *cube_filter = GST_GL_CUBE (filter);
  GstGLFuncs *gl = filter->context->gl_vtable;

  static GLfloat xrot = 0;
  static GLfloat yrot = 0;
  static GLfloat zrot = 0;

/* *INDENT-OFF* */
  const GLfloat v_vertices[] = {
 /*|     Vertex     | TexCoord |*/ 
    /* front face */
     1.0,  1.0, -1.0, 1.0, 0.0,
     1.0, -1.0, -1.0, 1.0, 1.0,
    -1.0, -1.0, -1.0, 0.0, 1.0,
    -1.0,  1.0, -1.0, 0.0, 0.0,
    /* back face */
     1.0,  1.0,  1.0, 1.0, 0.0,
    -1.0,  1.0,  1.0, 0.0, 0.0,
    -1.0, -1.0,  1.0, 0.0, 1.0,
     1.0, -1.0,  1.0, 1.0, 1.0,
    /* right face */
     1.0,  1.0,  1.0, 1.0, 0.0,
     1.0, -1.0,  1.0, 0.0, 0.0,
     1.0, -1.0, -1.0, 0.0, 1.0,
     1.0,  1.0, -1.0, 1.0, 1.0,
    /* left face */
    -1.0,  1.0,  1.0, 1.0, 0.0,
    -1.0,  1.0, -1.0, 1.0, 1.0,
    -1.0, -1.0, -1.0, 0.0, 1.0,
    -1.0, -1.0,  1.0, 0.0, 0.0,
    /* top face */
     1.0, -1.0,  1.0, 1.0, 0.0,
    -1.0, -1.0,  1.0, 0.0, 0.0,
    -1.0, -1.0, -1.0, 0.0, 1.0,
     1.0, -1.0, -1.0, 1.0, 1.0,
    /* bottom face */
     1.0,  1.0,  1.0, 1.0, 0.0,
     1.0,  1.0, -1.0, 1.0, 1.0,
    -1.0,  1.0, -1.0, 0.0, 1.0,
    -1.0,  1.0,  1.0, 0.0, 0.0
  };
/* *INDENT-ON* */

  GLushort indices[] = {
    0, 1, 2,
    0, 2, 3,
    4, 5, 6,
    4, 6, 7,
    8, 9, 10,
    8, 10, 11,
    12, 13, 14,
    12, 14, 15,
    16, 17, 18,
    16, 18, 19,
    20, 21, 22,
    20, 22, 23
  };

  GLint attr_position_loc = 0;
  GLint attr_texture_loc = 0;

  const GLfloat matrix[] = {
    0.5f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.5f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.5f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };

  GLenum target = GL_TEXTURE_2D;
  GstGLShader *shader = cube_filter->shader;

  if (GST_VIDEO_INFO_FORMAT(&filter->in_info) == GST_VIDEO_FORMAT_NV12) {
    target = GL_TEXTURE_EXTERNAL_OES;
    shader = cube_filter->shader_ext;
  }

  gl->Enable (GL_DEPTH_TEST);

  gl->ClearColor (cube_filter->red, cube_filter->green, cube_filter->blue, 0.0);
  gl->Clear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  gst_gl_shader_use (shader);

  attr_position_loc =
      gst_gl_shader_get_attribute_location (shader, "a_position");
  attr_texture_loc =
      gst_gl_shader_get_attribute_location (shader, "a_texCoord");

  /* Load the vertex position */
  gl->VertexAttribPointer (attr_position_loc, 3, GL_FLOAT,
      GL_FALSE, 5 * sizeof (GLfloat), v_vertices);

  /* Load the texture coordinate */
  gl->VertexAttribPointer (attr_texture_loc, 2, GL_FLOAT,
      GL_FALSE, 5 * sizeof (GLfloat), &v_vertices[3]);

  gl->EnableVertexAttribArray (attr_position_loc);
  gl->EnableVertexAttribArray (attr_texture_loc);

  gl->ActiveTexture (GL_TEXTURE0);
  gl->BindTexture (target, texture);
  gst_gl_shader_set_uniform_1i (shader, "s_texture", 0);
  gst_gl_shader_set_uniform_1f (shader, "xrot_degree", xrot);
  gst_gl_shader_set_uniform_1f (shader, "yrot_degree", yrot);
  gst_gl_shader_set_uniform_1f (shader, "zrot_degree", zrot);
  gst_gl_shader_set_uniform_matrix_4fv (shader, "u_matrix", 1,
      GL_FALSE, matrix);

  gl->DrawElements (GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, indices);

  gl->DisableVertexAttribArray (attr_position_loc);
  gl->DisableVertexAttribArray (attr_texture_loc);

  gl->Disable (GL_DEPTH_TEST);

  xrot += 0.5f;
  yrot += 0.6f;
  zrot += 0.7f;
}
