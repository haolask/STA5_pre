plugin_LTLIBRARIES = libgstfbsink.la

libgstfbsink_la_SOURCES = \
	fbpool.c \
	fbsink.c


noinst_HEADERS = \
	fbpool.h \
	fbsink.h


libgstfbsink_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) $(GST_BASE_CFLAGS) $(GST_CFLAGS)
libgstfbsink_la_LIBADD = \
	$(GST_PLUGINS_BASE_LIBS) \
	$(GST_BASE_LIBS) \
	$(GST_LIBS) \
	-lgstvideo-$(GST_API_VERSION)

libgstfbsink_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstfbsink_la_LIBTOOLFLAGS =$(GST_PLUGIN_LIBTOOLFLAGS)
