plugin_LTLIBRARIES = libgstv4l2enc.la

libgstv4l2enc_la_SOURCES = \
	gstv4l2enc.c \
	gstv4l2encbufferpool.c

noinst_HEADERS = \
	gstv4l2enc.h \
	gstv4l2encbufferpool.h

libgstv4l2enc_la_CFLAGS =\
	$(GST_PLUGINS_BASE_CFLAGS) \
	$(GST_BASE_CFLAGS)\
	$(GST_CFLAGS)\
	$(LIBV4L2_CFLAGS)

libgstv4l2enc_la_LIBADD = \
	$(GST_PLUGINS_BASE_LIBS) \
	$(GST_BASE_LIBS) \
	$(GST_LIBS) \
	-lgstvideo-$(GST_API_VERSION) \
	-lgstallocators-$(GST_API_VERSION) \
	$(LIBV4L2_LIBS)

libgstv4l2enc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstv4l2enc_la_LIBTOOLFLAGS =$(GST_PLUGIN_LIBTOOLFLAGS)
