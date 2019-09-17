## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = gconf

gconf_SOURCES = \
	gconf.c \
	kconfig_load.c  \
	zconf.tab.c \
	support.c support.h \
	interface.c interface.h 

gconf_LDADD = @PACKAGE_LIBS@ $(INTLLIBS) -ldl

CFLAGS = -DLKC_DIRECT_LINK -g

