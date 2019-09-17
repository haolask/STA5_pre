/*
 * gst-launch-interactive.c
 *
 * Copyright (C) STMicroelectronics SA 2013
 * Author: Christophe Priouzeau <christophe.priouzeau@st.com> for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */
#include <gst/gst.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//Uncomment the following line if you like to see the bus message on terminal
//#define GST_INTERACTIVE_PRINT_MESSAGE 1

#define fb_print(s,args...) g_print("INTERACTION> " s "\n", ##args)

#define SEEK_POSITION_NONE (gint64)-1

typedef struct
{
  GMainLoop *loop;
  GstElement *pipeline;
  gint64 duration;              /* stream duration in milliseconds */
  gboolean is_playing;          /* stream state (PLAYING or PAUSED) */
  gdouble rate;                 /* playback rate */
  gboolean playback_in_loop;
  GstElement *elt_Contrast;
  GstElement *elt_Brightness;
  GstElement *elt_Saturation;
  GstElement *elt_Alpha;
} AppData;

#ifdef GST_INTERACTIVE_PRINT_MESSAGE
static void
print_message (GstMessage * msg)
{
  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_UNKNOWN:
      fb_print ("MSG: GST_MESSAGE_UNKNOWN");
      break;
    case GST_MESSAGE_EOS:
      fb_print ("MSG: GST_MESSAGE_EOS");
      break;
    case GST_MESSAGE_ERROR:
      fb_print ("MSG: GST_MESSAGE_ERROR");
      break;
    case GST_MESSAGE_WARNING:
      fb_print ("MSG: GST_MESSAGE_WARNING");
      break;
    case GST_MESSAGE_INFO:
      fb_print ("MSG: GST_MESSAGE_INFO");
      break;
    case GST_MESSAGE_TAG:
      fb_print ("MSG: GST_MESSAGE_TAG");
      break;
    case GST_MESSAGE_BUFFERING:
      fb_print ("MSG: GST_MESSAGE_BUFFERING");
      break;
    case GST_MESSAGE_STATE_CHANGED:
      fb_print ("MSG: GST_MESSAGE_STATE_CHANGED");
      break;
    case GST_MESSAGE_STATE_DIRTY:
      fb_print ("MSG: GST_MESSAGE_STATE_DIRTY");
      break;
    case GST_MESSAGE_STEP_DONE:
      fb_print ("MSG: GST_MESSAGE_STEP_DONE");
      break;
    case GST_MESSAGE_CLOCK_PROVIDE:
      fb_print ("MSG: GST_MESSAGE_CLOCK_PROVIDE");
      break;
    case GST_MESSAGE_CLOCK_LOST:
      fb_print ("MSG: GST_MESSAGE_CLOCK_LOST");
      break;
    case GST_MESSAGE_NEW_CLOCK:
      fb_print ("MSG: GST_MESSAGE_NEW_CLOCK");
      break;
    case GST_MESSAGE_STRUCTURE_CHANGE:
      fb_print ("MSG: GST_MESSAGE_STRUCTURE_CHANGE");
      break;
    case GST_MESSAGE_STREAM_STATUS:
      fb_print ("MSG: GST_MESSAGE_STREAM_STATUS");
      break;
    case GST_MESSAGE_APPLICATION:
      fb_print ("MSG: GST_MESSAGE_APPLICATION");
      break;
    case GST_MESSAGE_ELEMENT:
      fb_print ("MSG: GST_MESSAGE_ELEMENT");
      break;
    case GST_MESSAGE_SEGMENT_START:
      fb_print ("MSG: GST_MESSAGE_SEGMENT_START");
      break;
    case GST_MESSAGE_SEGMENT_DONE:
      fb_print ("MSG: GST_MESSAGE_SEGMENT_DONE");
      break;
    case GST_MESSAGE_DURATION_CHANGED:
      fb_print ("MSG: GST_MESSAGE_DURATION_CHANGED");
      break;
    case GST_MESSAGE_LATENCY:
      fb_print ("MSG: GST_MESSAGE_LATENCY");
      break;
    case GST_MESSAGE_ASYNC_START:
      fb_print ("MSG: GST_MESSAGE_ASYNC_START");
      break;
    case GST_MESSAGE_ASYNC_DONE:
      fb_print ("MSG: GST_MESSAGE_ASYNC_DONE");
      break;
    case GST_MESSAGE_REQUEST_STATE:
      fb_print ("MSG: GST_MESSAGE_REQUEST_STATE");
      break;
    case GST_MESSAGE_STEP_START:
      fb_print ("MSG: GST_MESSAGE_STEP_START");
      break;
    case GST_MESSAGE_QOS:
      fb_print ("MSG: GST_MESSAGE_QOS");
      break;
    case GST_MESSAGE_PROGRESS:
      fb_print ("MSG: GST_MESSAGE_PROGRESS");
      break;
    case GST_MESSAGE_TOC:
      fb_print ("MSG: GST_MESSAGE_TOC");
      break;
    case GST_MESSAGE_RESET_TIME:
      fb_print ("MSG: GST_MESSAGE_RESET_TIME");
      break;
    case GST_MESSAGE_STREAM_START:
      fb_print ("MSG: GST_MESSAGE_STREAM_START");
      break;
    case GST_MESSAGE_NEED_CONTEXT:
      fb_print ("MSG: GST_MESSAGE_NEED_CONTEXT");
      break;
    case GST_MESSAGE_HAVE_CONTEXT:
      fb_print ("MSG: GST_MESSAGE_HAVE_CONTEXT");
      break;
    case GST_MESSAGE_ANY:
      fb_print ("MSG: GST_MESSAGE_ANY");
      break;
  }
}
#endif
static gboolean
bus_call (GstBus * bus, GstMessage * msg, gpointer data)
{
  AppData *app_data = (AppData *) data;
#ifdef GST_INTERACTIVE_PRINT_MESSAGE
  print_message (msg);
#endif
  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      fb_print ("End of stream");
      g_print ("EOS\n");
      if (app_data->playback_in_loop) {
        fb_print ("Loop");
        gst_element_seek_simple (app_data->pipeline, GST_FORMAT_TIME,
            GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, 0);
      } else
        g_main_loop_quit (app_data->loop);
      break;

    case GST_MESSAGE_ERROR:
    {
      gchar *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);

      g_printerr ("Error: %s\n\t%s\n",
          GST_STR_NULL (error->message), GST_STR_NULL (debug));
      g_error_free (error);
      g_free (debug);

      g_main_loop_quit (app_data->loop);
      break;
    }

    default:
      break;
  }

  return TRUE;
}

/* get stream duration in milliseconds */
static gboolean
getDuration (AppData * data, gint64 * duration)
{
  gboolean ret;

  ret = gst_element_query_duration (data->pipeline, GST_FORMAT_TIME, duration);
  if (ret == FALSE) {
    fb_print ("  failed to get stream duration");
    *duration = 0;
  } else {
    *duration /= GST_MSECOND;
  }

  return ret;
}

/* get stream current position in nanoseconds */
static gboolean
getPosition (AppData * data, gint64 * current_position)
{
  gboolean ret;

  ret = gst_element_query_position (data->pipeline, GST_FORMAT_TIME,
      current_position);
  if (ret == FALSE) {
    fb_print ("  failed to get stream current position");
    *current_position = SEEK_POSITION_NONE;
  }

  return ret;
}

/* modify the stream playback:
 * set the current position to seek_position nanoseconds (if different from
 * SEEK_POSITION_NONE)
 * increase or decrease the playback rate
 */
static gboolean
send_seek_event (AppData * data, gint64 seek_position)
{
  gboolean ret;
  gint64 seek_from;
  gint64 seek_to;
  GstEvent *seek_event = NULL;

  ret = getPosition (data, &seek_from);
  if (ret == TRUE) {
    seek_to = (seek_position == SEEK_POSITION_NONE) ? seek_from : seek_position;

    fb_print ("  seek from %" G_GINT64_FORMAT " ms to %" G_GINT64_FORMAT
        " ms at rate %g", seek_from / GST_MSECOND, seek_to / GST_MSECOND,
        data->rate);

    /* create the seek event */
    if (data->rate > 0) {
      seek_event = gst_event_new_seek (data->rate, GST_FORMAT_TIME,
          GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE,
          GST_SEEK_TYPE_SET, seek_to, GST_SEEK_TYPE_NONE, 0);
    } else {
      seek_event = gst_event_new_seek (data->rate, GST_FORMAT_TIME,
          GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE,
          GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, seek_to);
    }

    if (seek_event == NULL) {
      fb_print ("  failed to create a seek event");
      ret = FALSE;
    } else {
      /* send the event */
      ret = gst_element_send_event (data->pipeline, seek_event);
      if (ret == FALSE) {
        fb_print ("  failed to send the seek event");
      } else {
        GstState state;
        GstState expected_state;

        /* check the state */
        expected_state = data->is_playing ?
            GST_STATE_PLAYING : GST_STATE_PAUSED;
        do {
          gst_element_get_state (data->pipeline, &state, NULL,
              GST_CLOCK_TIME_NONE);
          if (state != expected_state)
            sleep (1);
        } while (state != expected_state);

        ret = getPosition (data, &seek_from);
        if (ret == TRUE) {
          fb_print ("  stream position after seek %" G_GINT64_FORMAT " ms",
              seek_from / GST_MSECOND);
        }
      }
    }
  }

  return ret;
}

static void
seek_test (AppData * data)
{
  gint64 seek_position;
  int i = 0;

  fb_print ("Start automatic seek tests");

  /* set PLAYING state if needed */
  if (!data->is_playing) {
    gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
    data->is_playing = TRUE;
  }

  getDuration (data, &data->duration);
  if (data->duration > 60000) {
    /* play 10 seconds */
    fb_print ("  play 10 seconds");
    sleep (10);

    /* seek to (EOS - 30 seconds) */
    seek_position = data->duration - 30 * (GST_SECOND / GST_MSECOND);
    fb_print ("  seek to %" G_GINT64_FORMAT " ms", seek_position);
    send_seek_event (data, seek_position * GST_MSECOND);

    /* play 20 seconds */
    fb_print ("  play 20 seconds");
    sleep (20);

    /* seek to half of the stream duration */
    seek_position = data->duration / 2;
    fb_print ("  seek to %" G_GINT64_FORMAT " ms", seek_position);
    send_seek_event (data, seek_position * GST_MSECOND);

    /* play 30 seconds */
    fb_print ("  play 30 seconds");
    sleep (30);

    fb_print ("  run 5 times the same test sequence");
    while (i < 5) {
      /* seek to 20% of the stream duration */
      seek_position = data->duration * 2 / 10;
      fb_print ("  [%d] seek to %" G_GINT64_FORMAT " ms", i, seek_position);
      send_seek_event (data, seek_position * GST_MSECOND);

      /* play 10 seconds */
      fb_print ("  [%d] play 10 seconds", i);
      sleep (10);

      /* seek to 60% of the stream duration */
      seek_position = data->duration * 6 / 10;
      fb_print ("  [%d] seek to %" G_GINT64_FORMAT " ms", i, seek_position);
      send_seek_event (data, seek_position * GST_MSECOND);

      /* play 20 seconds */
      fb_print ("  [%d] play 20 seconds", i);
      sleep (20);
      i++;
    }
  }

  fb_print ("End of automatic seek tests");
}

static void
usage (void)
{
  fb_print ("Usage: choose one of the following commands and press enter:");
  fb_print ("  'h' or 'H' to display this help");
  fb_print ("  'p' or 'P' to toogle the stream between PAUSE and PLAYING"
      " states");
  fb_print ("  's <pos>' or 'S <pos>' to set the stream current position"
      " to <pos> milliseconds");
  fb_print ("  'n <nb>' or 'N <nb>' to move to the next <nb> frame (<nb> is"
      " optional and its default value is 1). Note that the stream is"
      " paused before this action");
  fb_print ("  'R' to increase the playback rate by 2");
  fb_print ("  'r' to decrease the playback rate by 2");
  fb_print ("  'd' or 'D' to toogle the playback direction");
  fb_print ("  'i' or 'I' to get the stream state, duration, current"
      " position and rate");
  fb_print ("  'l' or 'L' to enable/disable loopback");
  fb_print ("  'q' or 'Q' to quit gst-interactive");
  fb_print
      ("  'c <contrast>' or 'C <contrast>' to update the contrast of the video content range [-64;64]");
  fb_print
      ("  'b <brightness>' or 'B <brightness>' to update the brightness of the video content range [-128;127]");
  fb_print
      ("  'z <saturation>' or 'Z <saturation>' to update the saturation of the video content range [-64;128]");
  fb_print
      ("  'a <alpha>' or 'A <alpha>' to update the alpha channel of the video content range [0;255]");
  fb_print ("  'auto' to launch automatic seek tests");
}

static gboolean
pipeline_stuff_string (AppData * data, gchar * cmd)
{
  GstState state, pending;
  //g_message ("Pipeline stuff. Received command: %s", cmd);
  if (!strncasecmp (cmd, "h", 1)) {
    usage ();
  } else if (!strncasecmp (cmd, "p", 1)) {
    data->is_playing = !data->is_playing;
    gst_element_set_state (data->pipeline, data->is_playing ?
        GST_STATE_PLAYING : GST_STATE_PAUSED);
    fb_print ("  set stream state to %s\n", data->is_playing ?
        "PLAYING" : "PAUSED");
  } else if (!strncasecmp (cmd, "s", 1)) {
    gint64 seek_position;

    if (1 == sscanf (cmd + 2, "%" G_GINT64_FORMAT "", &seek_position)) {
      send_seek_event (data, seek_position * GST_MSECOND);
    }
  } else if (!strncasecmp (cmd, "n", 1)) {
    gint64 nb = 1;

    sscanf (cmd + 2, "%" G_GINT64_FORMAT "", &nb);
    fb_print ("  JCT %" G_GINT64_FORMAT "\n", nb);

    if (data->is_playing) {
      gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
      data->is_playing = FALSE;
    }
    gst_element_send_event (data->pipeline,
        gst_event_new_step (GST_FORMAT_BUFFERS, nb, data->rate, TRUE, FALSE));
  } else if (!strncmp (cmd, "R", 1)) {
    data->rate *= 2;
    send_seek_event (data, SEEK_POSITION_NONE);
  } else if (!strncmp (cmd, "r", 1)) {
    data->rate /= 2;
    send_seek_event (data, SEEK_POSITION_NONE);
  } else if (!strncasecmp (cmd, "d", 1)) {
    data->rate *= -1;
    send_seek_event (data, SEEK_POSITION_NONE);
  } else if (!strncasecmp (cmd, "i", 1)) {
    gint64 seek_position;

    getDuration (data, &data->duration);
    getPosition (data, &seek_position);
    fb_print ("  stream information:");
    fb_print ("  - state %s", data->is_playing ? "PLAYING" : "PAUSED");
    fb_print ("  - duration %" G_GINT64_FORMAT " ms", data->duration);
    fb_print ("  - current position %" G_GINT64_FORMAT " ms",
        seek_position / GST_MSECOND);
    fb_print ("  - rate %g", data->rate);
  } else if (!strncasecmp (cmd, "q", 1)) {
    gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
    gst_element_set_state (data->pipeline, GST_STATE_NULL);
    do {
      state = GST_STATE_NULL;
      sleep (1);
      gst_element_get_state (data->pipeline, &state, &pending,
          GST_CLOCK_TIME_NONE);
    } while (state != GST_STATE_NULL);
    g_main_loop_quit (data->loop);
    return FALSE;
  } else if (!strncasecmp (cmd, "auto", 4)) {
    seek_test (data);
  } else if (!strncmp (cmd, "L", 1) || !strncmp (cmd, "l", 1)) {
    data->playback_in_loop = !data->playback_in_loop;
    fb_print ("  Loopback %s", data->playback_in_loop ? "enabled" : "disabled");
  } else if (!strncasecmp (cmd, "c", 1)) {
    gint32 param;
    if (1 == sscanf (cmd + 2, "%" G_GINT32_FORMAT "", &param)) {
      if (data->elt_Contrast) {
        g_object_set (data->elt_Contrast, "contrast", param, NULL);
      }
    }
  } else if (!strncasecmp (cmd, "b", 1)) {
    gint32 param;
    if (1 == sscanf (cmd + 2, "%" G_GINT32_FORMAT "", &param)) {
      if (data->elt_Brightness) {
        g_object_set (data->elt_Brightness, "brightness", param, NULL);
      }
    }
  } else if (!strncasecmp (cmd, "z", 1)) {
    gint32 param;
    if (1 == sscanf (cmd + 2, "%" G_GINT32_FORMAT "", &param)) {
      if (data->elt_Saturation) {
        g_object_set (data->elt_Saturation, "saturation", param, NULL);
      }
    }
  } else if (!strncasecmp (cmd, "a", 1)) {
    gint32 param;
    if (1 == sscanf (cmd + 2, "%" G_GINT32_FORMAT "", &param)) {
      if (data->elt_Alpha) {
        g_object_set (data->elt_Alpha, "alpha", param, NULL);
      }
    }
  } else {
    if (strlen (cmd) > 2) {
      fb_print ("Unknown command: %s", cmd);
      usage ();
    }
  }
  return TRUE;
}

static gboolean
io_callback (GIOChannel * io, GIOCondition condition, gpointer data)
{
  GString *buffer = g_string_new (NULL);
  gsize tpos;
  AppData *app_data = (AppData *) data;
  GError *error = NULL;

  switch (g_io_channel_read_line_string (io, buffer, &tpos, &error)) {
    case G_IO_STATUS_NORMAL:
      return pipeline_stuff_string (app_data, buffer->str);

    case G_IO_STATUS_ERROR:
      g_printerr ("IO error: %s\n", error->message);
      g_error_free (error);

      return FALSE;

    case G_IO_STATUS_EOF:
      g_warning ("No input data available");
      return TRUE;

    case G_IO_STATUS_AGAIN:
      return TRUE;

    default:
      g_return_val_if_reached (FALSE);
      break;
  }

  return FALSE;
}


static gboolean
element_has_property (GstElement * element, const gchar * pname, GType type)
{
  GParamSpec *pspec;

  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (element), pname);

  if (pspec == NULL) {
    return FALSE;
  }

  if (type == G_TYPE_INVALID || type == pspec->value_type ||
      g_type_is_a (pspec->value_type, type)) {
    return TRUE;
  }
  return FALSE;
}

typedef struct
{
  const gchar *prop_name;
  GType prop_type;
} FindPropertyHelper;

static gint
find_property (const GValue * item, FindPropertyHelper * helper)
{
  GstElement *element = g_value_get_object (item);

  if (!element_has_property (element, helper->prop_name, helper->prop_type)) {
    return 1;
  }
  return 0;
}


/* find an object in the hierarchy with a property named @name */
static GstElement *
gst_play_sink_find_property (GstElement * obj,
    const gchar * name, GType expected_type)
{
  GstElement *result = NULL;
  GstIterator *it;

  if (GST_IS_BIN (obj)) {
    gboolean found;
    GValue item = { 0, };
    FindPropertyHelper helper = { name, expected_type };

    it = gst_bin_iterate_recurse (GST_BIN_CAST (obj));
    found = gst_iterator_find_custom (it,
        (GCompareFunc) find_property, &item, &helper);
    gst_iterator_free (it);
    if (found) {
      result = g_value_dup_object (&item);
      g_value_unset (&item);
    }
  } else {
    if (element_has_property (obj, name, expected_type)) {
      result = obj;
      gst_object_ref (obj);
    }
  }
  return result;
}

static void
for_each_pipeline_element (const GValue * value, gpointer data)
{
  GstElement *element = g_value_get_object (value);
  AppData *app_data = (AppData *) data;
  GstElement *result = NULL;

  if (!element || !app_data)
    return;

  result = gst_play_sink_find_property (element, "contrast", G_TYPE_INT);
  if (result && !app_data->elt_Contrast) {
    app_data->elt_Contrast = element;
  }
  result = gst_play_sink_find_property (element, "brightness", G_TYPE_INT);
  if (result && !app_data->elt_Brightness) {
    app_data->elt_Brightness = element;
  }
  result = gst_play_sink_find_property (element, "saturation", G_TYPE_INT);
  if (result && !app_data->elt_Saturation) {
    app_data->elt_Saturation = element;
  }
  result = gst_play_sink_find_property (element, "alpha", G_TYPE_INT);
  if (result && !app_data->elt_Alpha) {
    app_data->elt_Alpha = element;
  }
}

int
main (int argc, char *argv[])
{
  GError *error = NULL;

  AppData data = { NULL, NULL };

  GIOChannel *io = NULL;
  GstIterator *iterator = NULL;
  GstBus *bus = NULL;
  guint bus_watch_id = 0;

  int ret = EXIT_FAILURE;

  gchar **argvn = NULL;
  int i = 0;

  if (1 == argc) {
    fprintf (stderr, "Error: Missing argument\n"
        "USAGE: %s [-loop] <pipeline description>\n", argv[0]);
    return EXIT_FAILURE;
  }

  /* Extract additional parameter not part of gstreamer pipeline :
   *   -> "-loop" :to enable loopback (by default disabled)
   */
  data.playback_in_loop = FALSE;
  if (strcmp (argv[1], "-loop") == 0) {
    data.playback_in_loop = TRUE;
    for (i = 1; i < (argc - 1); i++)
      argv[i] = argv[i + 1];
    argc--;
  }

  if (1 == argc) {
    fprintf (stderr, "Error: Missing argument\n"
        "USAGE: %s [-loop] <pipeline description>\n", argv[0]);
    return EXIT_FAILURE;
  }

  gst_init (&argc, &argv);

  /* make a null-terminated version of argv */
  argvn = g_new0 (char *, argc);
  memcpy (argvn, argv + 1, sizeof (char *) * (argc - 1));
  {
    data.pipeline =
        (GstElement *) gst_parse_launchv ((const gchar **) argvn, &error);
  }
  g_free (argvn);

  /* handling pipeline creation failure */
  if (!data.pipeline) {
    g_printerr ("ERROR: pipeline could not be constructed: %s\n",
        error ? GST_STR_NULL (error->message) : "(unknown error)");
    goto untergang;
  } else if (error) {
    g_printerr ("Erroneous pipeline: %s\n", GST_STR_NULL (error->message));
    goto untergang;
  }
  data.loop = g_main_loop_new (NULL, FALSE);

  iterator = gst_bin_iterate_recurse (GST_BIN (data.pipeline));
  gst_iterator_foreach (iterator, for_each_pipeline_element, &data);
  gst_iterator_free (iterator);

  /* bus callback */
  bus = gst_pipeline_get_bus (GST_PIPELINE (data.pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, &data);
  gst_object_unref (GST_OBJECT (bus));

  /* standard input callback */
  io = g_io_channel_unix_new (STDIN_FILENO);
  g_io_add_watch (io, G_IO_IN, io_callback, &data);
  g_io_channel_unref (io);
  g_message ("Running...");
  g_message ("Loopback %s", data.playback_in_loop ? "enabled" : "disabled");
  if (GST_STATE_CHANGE_FAILURE == gst_element_set_state (data.pipeline,
          GST_STATE_PLAYING)) {
    g_printerr ("Failed to play the pipeline\n");
    goto untergang;
  }

  /* initialize stream information */
  data.duration = 0;
  data.is_playing = TRUE;
  data.rate = 1;

  g_main_loop_run (data.loop);

  g_message ("Returned, stopping playback");
  if (GST_STATE_CHANGE_FAILURE == gst_element_set_state (data.pipeline,
          GST_STATE_NULL)) {
    g_printerr ("Failed to stop the pipeline\n");
    goto untergang;
  }

  ret = EXIT_SUCCESS;

untergang:
  if (0 != bus_watch_id)
    g_source_remove (bus_watch_id);
  if (data.pipeline)
    gst_object_unref (GST_OBJECT (data.pipeline));
  if (error)
    g_error_free (error);
  if (data.loop)
    g_main_loop_unref (data.loop);

  return ret;
}
