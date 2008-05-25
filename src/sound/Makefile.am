noinst_LIBRARIES = libsound.a

libsound_a_SOURCES = \
	playsound.cpp \
	soundmanager.cpp

noinst_HEADERS = \
	playsound.h \
	soundmanager.h

AM_CXXFLAGS = @GTKMM_CFLAGS@ @ALSA_CFLAGS@
INCLUDES = -I$(top_srcdir)/src
