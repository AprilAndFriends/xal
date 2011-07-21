SUBDIRS = src include/xalutil
EXTRA_DIST = autogen.sh

lib_LTLIBRARIES = libxalutil.la
libxalutil_la_SOURCES = 
libxalutil_la_LIBADD =  $(LIBOBJS) \
	src/libxalutil_src.la
libxalutil_la_LDFLAGS = -version-info @LT_VERSION@

