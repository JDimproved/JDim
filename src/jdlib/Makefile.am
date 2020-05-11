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
	imgloader.cpp \
	ssl.cpp \
	loaderdata.cpp \
	cookiemanager.cpp \
	confloader.cpp \
	jdregex.cpp \
	jdmigemo.cpp \
	tfidf.cpp \
	timeout.cpp

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
	imgloader.h \
	ssl.h \
	loaderdata.h \
	cookiemanager.h \
	confloader.h \
	jdregex.h \
	jdmigemo.h \
	tfidf.h \
	timeout.h \
	hkana.h

AM_CXXFLAGS = @GTKMM_CFLAGS@ @GNUTLS_CFLAGS@ @OPENSSL_CFLAGS@ @X11_CFLAGS@
AM_CPPFLAGS = -I$(top_srcdir)/src
