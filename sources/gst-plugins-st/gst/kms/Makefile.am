plugin_LTLIBRARIES = libgstkmssink.la

libgstkmssink_la_SOURCES = \
	kmssink.c \
	kmspool.c

noinst_HEADERS = \
	kmssink.h \
	kmspool.h

libgstkmssink_la_CFLAGS =  $(GST_PLUGINS_BASE_CFLAGS) $(GST_BASE_CFLAGS) $(GST_CFLAGS) $(KMS_CFLAGS) $(DRM_CFLAGS) $(LIBUDEV_CFLAGS)
libgstkmssink_la_LIBADD = \
	$(GST_PLUGINS_BASE_LIBS) \
	$(GST_BASE_LIBS) \
	$(GST_LIBS) \
	-lgstallocators-$(GST_API_VERSION) \
	-lgstvideo-$(GST_API_VERSION) \
	$(KMS_LIBS) $(DRM_LIBS) $(LIBUDEV_LIBS)

libgstkmssink_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstkmssink_la_LIBTOOLFLAGS =$(GST_PLUGIN_LIBTOOLFLAGS)
