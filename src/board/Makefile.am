noinst_LIBRARIES = libboard.a

libboard_a_SOURCES = \
	boardadmin.cpp \
	boardviewbase.cpp \
	boardview.cpp \
	toolbar.cpp \
	preference.cpp

noinst_HEADERS = \
	boardadmin.h \
	boardviewbase.h \
	boardview.h \
	toolbar.h \
	columns.h \
	preference.h

AM_CXXFLAGS = @GTKMM_CFLAGS@ @ONIG_CFLAGS@
INCLUDES = -I$(top_srcdir)/src
