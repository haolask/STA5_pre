plugin_LTLIBRARIES = libgstglstm.la

libgstglstm_la_SOURCES = \
	gstglstm.c \
	gstglcube.c \
	gstglvideomixer.c \
	gstgl360view2d.c

noinst_HEADERS = \
	gstglcube.h \
	gstglvideomixer.h \
	gstgl360view2d.h

libgstglstm_la_CFLAGS = \
	$(GST_PLUGINS_BASE_CFLAGS) \
	$(GST_BASE_CFLAGS) \
	$(GST_CFLAGS) \
	$(EGL_CFLAGS)

libgstglstm_la_LIBADD = \
	$(GST_PLUGINS_BASE_LIBS) \
	$(GST_BASE_LIBS) \
	$(GST_LIBS) \
	-lgstvideo-$(GST_API_VERSION) \
	-lgstbadvideo-$(GST_API_VERSION) \
	-lgstallocators-$(GST_API_VERSION) \
	-lgstgl-$(GST_API_VERSION) \
	$(EGL_LIBS)

libgstglstm_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

libgstglstm_la_LIBTOOLFLAGS = \
	$(GST_PLUGIN_LIBTOOLFLAGS) \
	--tag=disable-static
