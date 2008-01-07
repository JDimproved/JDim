noinst_LIBRARIES = libjdlib.a

libjdlib_a_SOURCES = \
	miscutil.cpp \
	miscmsg.cpp \
	misctime.cpp \
	misctrip.cpp \
	miscx.cpp \
	miscgtk.cpp \
	jdthread.cpp \
	misccharcode.cpp \
        heap.cpp \
	jdiconv.cpp \
	loader.cpp \
	ssl.cpp \
	loaderdata.cpp \
	confloader.cpp \
	jdregex.cpp \
	jdmigemo.cpp

noinst_HEADERS = \
	miscutil.h \
	miscmsg.h \
	misctime.h \
	misctrip.h \
	miscx.h \
	miscgtk.h \
	jdthread.h \
	misccharcode.h \
	constptr.h \
	refptr_lock.h \
	heap.h \
	jdiconv.h \
	loader.h \
	ssl.h \
	loaderdata.h \
	confloader.h \
	jdregex.h \
	jdmigemo.h

AM_CXXFLAGS = @GTKMM_CFLAGS@
INCLUDES = -I$(top_srcdir)/src
