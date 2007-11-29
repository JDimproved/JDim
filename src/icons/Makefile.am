noinst_LIBRARIES = libicon.a

libicon_a_SOURCES = \
	iconmanager.cpp

icon_headers = jd16.h jd32.h jd48.h jd96.h \
	dir.h board.h board_update.h board_updated.h thread.h thread_update.h thread_updated.h thread_old.h \
	image.h link.h loading.h loading_stop.h check.h down.h \
	update.h newthread.h bkmark.h bkmark_update.h favorite.h write.h

noinst_HEADERS = \
	iconmanager.h $(icon_headers)

AM_CXXFLAGS = @GTKMM_CFLAGS@
INCLUDES = -I$(top_srcdir)/src

clean_icon :
	rm -rf $(icon_headers)

iconmanager.cpp : $(icon_headers)

SUFFIXES = .png .h

.png.h:
	gdk-pixbuf-csource --raw --name=icon_$* $< > $@
