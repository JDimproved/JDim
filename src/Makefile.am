SUBDIRS = dbtree dbimg bbslist board article image message jdlib skeleton config

bin_PROGRAMS = jd

jd_LDADD = @LIBS@ @GTKMM_LIBS@ @GTHREAD_LIBS@ @OPENSSL_LIBS@ @GNOMEUI_LIBS@ @LIBSM_LIBS@\
	./dbtree/libdbtree.a \
	./dbimg/libdbimg.a \
	./bbslist/libbbslist.a \
	./board/libboard.a \
	./article/libarticle.a \
	./image/libimage.a \
	./message/libmessage.a \
	./skeleton/libskeleton.a \
	./jdlib/libjdlib.a  \
	./config/libconfig.a

jd_SOURCES = \
	main.cpp \
	winmain.cpp \
	core.cpp \
	cache.cpp \
	xml.cpp \
	command.cpp \
	viewfactory.cpp \
	sharedbuffer.cpp \
	dndmanager.cpp \
	historymenu.cpp \
	historysubmenu.cpp \
	control.cpp \
	controlutil.cpp \
\
	session.cpp \
	login2ch.cpp

noinst_HEADERS = \
	winmain.h \
	httpcode.h \
	core.h \
	cache.h \
	xml.h \
	jddebug.h \
	global.h \
	jdversion.h \
	command.h \
	viewfactory.h \
	sharedbuffer.h \
	dndmanager.h \
	proxypref.h \
	browserpref.h \
	passwdpref.h \
	historymenu.h \
	historysubmenu.h \
	colorid.h \
	controlid.h \
	controllabel.h \
	control.h \
	controlutil.h \
\
	session.h \
	login2ch.h

AM_CXXFLAGS = @GTKMM_CFLAGS@ @GTHREAD_CFLAGS@ @OPENSSL_CFLAGS@ @GNOMEUI_CFLAGS@ @LIBSM_CFLAGS@
