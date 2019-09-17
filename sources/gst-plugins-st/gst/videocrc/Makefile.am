plugin_LTLIBRARIES = libgstvideocrc.la

libgstvideocrc_la_SOURCES = \
	gstvideocrc.c \
	gstvideocrc.h

noinst_HEADERS = \
	gstvideocrc.h

libgstvideocrc_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) \
			  $(GST_BASE_CFLAGS) \
			  $(GST_CFLAGS) \
			  $(GST_VIDEOCRC_CFLAGS)

libgstvideocrc_la_LIBADD = $(GST_PLUGINS_BASE_LIBS) \
			  $(GST_BASE_LIBS) \
			  $(GST_LIBS) \
			  -lgstvideo-$(GST_API_VERSION) \
			  $(GST_VIDEOCRC_LIBS)

libgstvideocrc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstvideocrc_la_LIBTOOLFLAGS =$(GST_PLUGIN_LIBTOOLFLAGS)
