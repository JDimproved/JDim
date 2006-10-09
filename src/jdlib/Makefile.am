noinst_LIBRARIES = libjdlib.a

libjdlib_a_SOURCES = \
	miscutil.cpp \
	miscmsg.cpp \
	misctime.cpp \
	misctrip.cpp \
        heap.cpp \
	jdiconv.cpp \
	loader.cpp \
	confloader.cpp \
	jdregex.cpp \
	jdmigemo.cpp

noinst_HEADERS = \
	miscutil.h \
	miscmsg.h \
	misctime.h \
	misctrip.h \
	constptr.h \
	refptr_lock.h \
	heap.h \
	jdiconv.h \
	loader.h \
	loaderdata.h \
	confloader.h \
	jdregex.h \
	jdmigemo.h

AM_CXXFLAGS = @GTKMM_CFLAGS@
INCLUDES = -I$(top_srcdir)/src
