noinst_LIBRARIES = libcontrol.a

libcontrol_a_SOURCES = \
	control.cpp \
	controlutil.cpp \
	mousekeyconf.cpp keyconfig.cpp mouseconfig.cpp buttonconfig.cpp \
	keypref.cpp

noinst_HEADERS = \
	controlid.h \
	controllabel.h \
	keysyms.h \
	control.h \
	controlutil.h \
	mousekeyconf.h mousekeyitem.h keyconfig.h mouseconfig.h buttonconfig.h \
	keypref.h \
	defaultconf.h

AM_CXXFLAGS = @GTKMM_CFLAGS@
INCLUDES = -I$(top_srcdir)/src
