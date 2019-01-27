noinst_LIBRARIES = libcontrol.a

libcontrol_a_SOURCES = \
	control.cpp \
	controlutil.cpp \
	mousekeyconf.cpp keyconfig.cpp mouseconfig.cpp buttonconfig.cpp \
	mousekeypref.cpp keypref.cpp mousepref.cpp buttonpref.cpp

noinst_HEADERS = \
	controlid.h \
	controllabel.h \
	keysyms.h \
	control.h \
	controlutil.h \
	mousekeyconf.h mousekeyitem.h keyconfig.h mouseconfig.h buttonconfig.h \
	mousekeypref.h keypref.h mousepref.h buttonpref.h \
	defaultconf.h \
	get_config.h

AM_CXXFLAGS = @GTKMM_CFLAGS@
AM_CPPFLAGS = -I$(top_srcdir)/src
