SUBDIRS = dbtree dbimg bbslist board article image message jdlib skeleton config icons xml

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
	./config/libconfig.a \
	./jdlib/libjdlib.a  \
	./icons/libicon.a \
	./xml/libxml.a

jd_SOURCES = \
	main.cpp \
	winmain.cpp \
	core.cpp \
	cache.cpp \
	command.cpp \
	sharedbuffer.cpp \
	dndmanager.cpp \
	usrcmdmanager.cpp \
	compmanager.cpp \
	searchmanager.cpp \
	searchloader.cpp \
	aamanager.cpp \
	dispatchmanager.cpp \
	cssmanager.cpp \
	updatemanager.cpp \
	historymenu.cpp \
	historysubmenu.cpp \
	control.cpp \
	controlutil.cpp \
	browsers.cpp \
	setupwizard.cpp \
\
	session.cpp \
	login2ch.cpp \
	loginbe.cpp \
\
	viewfactory.cpp \
\
	prefdiagfactory.cpp \
	boarditempref.cpp \
	articleitempref.cpp \
	sidebaritempref.cpp \
	fontcolorpref.cpp


noinst_HEADERS = \
	winmain.h \
	httpcode.h \
	core.h \
	cache.h \
	jddebug.h \
	global.h \
	jdversion.h \
	command.h \
	sharedbuffer.h \
	dndmanager.h \
	usrcmdmanager.h \
	compmanager.h \
	searchmanager.h \
	searchloader.h \
	aamanager.h \
	dispatchmanager.h \
	cssmanager.h \
	updatemanager.h \
	historymenu.h \
	historysubmenu.h \
	colorid.h \
	fontid.h \
	controlid.h \
	controllabel.h \
	control.h \
	controlutil.h \
	command_args.h \
	browsers.h \
	setupwizard.h \
\
	session.h \
	login2ch.h \
	loginbe.h \
\
	viewfactory.h \
\
	prefdiagfactory.h \
	proxypref.h \
	browserpref.h \
	globalabonepref.h \
	globalabonethreadpref.h \
	passwdpref.h \
	privacypref.h \
	boarditempref.h \
	articleitempref.h \
	sidebaritempref.h \
	fontcolorpref.h


AM_CXXFLAGS = @GTKMM_CFLAGS@ @GTHREAD_CFLAGS@ @OPENSSL_CFLAGS@ @GNOMEUI_CFLAGS@ @LIBSM_CFLAGS@
