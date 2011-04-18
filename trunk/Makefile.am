SUBDIRS = src include/xal
EXTRA_DIST = autogen.sh

lib_LTLIBRARIES = libxal.la
libxal_la_SOURCES = 
libxal_la_LIBADD =  $(LIBOBJS) \
	src/libxal_src.la
libxal_la_LDFLAGS = -version-info @LT_VERSION@

