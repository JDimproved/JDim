SUBDIRS = dbtree dbimg bbslist board article image message jdlib skeleton config icons

bin_PROGRAMS = jd

jd_LDADD = @LIBS@ @GTKMM_LIBS@ @GTHREAD_LIBS@ @OPENSSL_LIBS@ @GNOMEUI_LIBS@ @LIBSM_LIBS@ \
	./dbtree/libdbtree.a \
	./dbimg/libdbimg.a \
	./bbslist/libbbslist.a \
	./board/libboard.a \
	./article/libarticle.a \
	./image/libimage.a \
	./message/libmessage.a \
	./skeleton/libskeleton.a \
	./jdlib/libjdlib.a  \
	./config/libconfig.a \
	./icons/libicon.a

jd_SOURCES = \
	main.cpp \
	winmain.cpp \
	core.cpp \
	cache.cpp \
	xml.cpp \
	command.cpp \
	sharedbuffer.cpp \
	dndmanager.cpp \
	historymenu.cpp \
	historysubmenu.cpp \
	control.cpp \
	controlutil.cpp \
\
	session.cpp \
	login2ch.cpp \
\
	viewfactory.cpp \
\
	prefdiagfactory.cpp


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
	sharedbuffer.h \
	dndmanager.h \
	historymenu.h \
	historysubmenu.h \
	colorid.h \
	controlid.h \
	controllabel.h \
	control.h \
	controlutil.h \
	command_args.h \
\
	session.h \
	login2ch.h \
\
	viewfactory.h \
\
	prefdiagfactory.h \
	proxypref.h \
	browserpref.h \
	globalabonepref.h \
	globalabonethreadpref.h \
	passwdpref.h


AM_CXXFLAGS = @GTKMM_CFLAGS@ @GTHREAD_CFLAGS@ @OPENSSL_CFLAGS@ @GNOMEUI_CFLAGS@ @LIBSM_CFLAGS@
