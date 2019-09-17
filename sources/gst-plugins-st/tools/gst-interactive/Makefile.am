bin_PROGRAMS = \
	gst-interactive-@GST_API_VERSION@

gst_interactive_@GST_API_VERSION@_SOURCES = gst-launch-interactive.c
gst_interactive_@GST_API_VERSION@_CFLAGS = $(GST_CFLAGS)
gst_interactive_@GST_API_VERSION@_LDADD = $(GST_LIBS)

Android.mk: Makefile.am
	androgenizer -:PROJECT gstreamer \
	 -:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
	 -:EXECUTABLE gst-interactive-@GST_API_VERSION@ -:TAGS eng debug \
	 -:SOURCES $(gst_interactive_@GST_API_VERSION@_SOURCES) \
	 -:CFLAGS $(DEFS) $(gst_interactive_@GST_API_VERSION@_CFLAGS) \
	 -:LDFLAGS $(gst_interactive_@GST_API_VERSION@_LDADD) \
	 -:PASSTHROUGH LOCAL_ARM_MODE:=arm \
	> $@

manpages = \
	gst-interactive-@GST_API_VERSION@.1

CLEANFILES = $(manpages) *.gcno *.gcda
man_MANS = $(manpages)

EXTRA_DIST = \
	gst-interactive.1.in

%-@GST_API_VERSION@.1: %.1.in
	$(AM_V_GEN)sed \
		-e s,gst-interactive,gst-interactive-@GST_API_VERSION@,g \
		-e s,GST_API_VERSION,@GST_API_VERSION@,g \
		$< >$@

