noinst_LIBRARIES = libicon.a

libicon_a_SOURCES = \
	iconmanager.cpp

icon_headers = jd16.h jd32.h jd48.h jd96.h \
	dir.h board.h board_update.h board_updated.h thread.h thread_update.h thread_updated.h thread_old.h \
	image.h link.h loading.h loading_stop.h check.h down.h \
	update.h newthread.h newthread_hour.h broken_subject.h bkmark.h bkmark_broken_subject.h bkmark_update.h bkmark_thread.h favorite.h write.h post.h post_refer.h \
	play.h hist.h hist_board.h hist_close.h hist_closeboard.h hist_closeimg.h info.h

noinst_HEADERS = \
	iconmanager.h $(icon_headers)

AM_CXXFLAGS = @GTKMM_CFLAGS@
INCLUDES = -I$(top_srcdir)/src

DISTCLEANFILES = $(icon_headers)

clean_icons :
	rm -rf $(icon_headers)

iconmanager.cpp : $(icon_headers)

SUFFIXES = .png .h

.png.h:
	gdk-pixbuf-csource --raw --name=icon_$* $< > $@
