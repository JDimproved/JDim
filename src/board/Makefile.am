noinst_LIBRARIES = libboard.a

libboard_a_SOURCES = \
	boardadmin.cpp \
	boardviewbase.cpp \
	boardview.cpp \
	boardviewnext.cpp \
	boardviewlog.cpp \
	boardviewsidebar.cpp \
	toolbar.cpp \
	preference.cpp

noinst_HEADERS = \
	boardadmin.h \
	boardviewbase.h \
	boardview.h \
	boardviewnext.h \
	boardviewlog.h \
	boardviewsidebar.h \
	toolbar.h \
	columns.h \
	preference.h

AM_CXXFLAGS = @GTKMM_CFLAGS@
AM_CPPFLAGS = -I$(top_srcdir)/src
