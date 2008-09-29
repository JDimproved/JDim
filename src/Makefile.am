SUBDIRS = dbtree dbimg bbslist board article image message jdlib skeleton history config icons sound xml control

bin_PROGRAMS = jd

jd_LDADD = \
	./dbtree/libdbtree.a \
	./dbimg/libdbimg.a \
	./bbslist/libbbslist.a \
	./board/libboard.a \
	./article/libarticle.a \
	./image/libimage.a \
	./message/libmessage.a \
	./skeleton/libskeleton.a \
	./history/libhistory.a \
	./config/libconfig.a \
	./jdlib/libjdlib.a  \
	./icons/libicon.a \
	./sound/libsound.a \
	./xml/libxml.a \
	./control/libcontrol.a \
	@LIBS@ @GTKMM_LIBS@ @GTHREAD_LIBS@ @GNUTLS_LIBS@ @OPENSSL_LIBS@ @GNOMEUI_LIBS@ @LIBSM_LIBS@ @ALSA_LIBS@

jd_SOURCES = \
	main.cpp \
	winmain.cpp \
	core.cpp \
	maintoolbar.cpp \
	cache.cpp \
	command.cpp \
	sharedbuffer.cpp \
	dndmanager.cpp \
	usrcmdmanager.cpp \
	linkfiltermanager.cpp \
	compmanager.cpp \
	searchmanager.cpp \
	searchloader.cpp \
	aamanager.cpp \
	dispatchmanager.cpp \
	cssmanager.cpp \
	updatemanager.cpp \
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
	mainitempref.cpp \
	boarditempref.cpp \
	articleitempref.cpp \
	msgitempref.cpp \
	sidebaritempref.cpp \
	fontcolorpref.cpp \
	usrcmdpref.cpp \
	linkfilterpref.cpp \
	iomonitor.cpp


noinst_HEADERS = \
	winmain.h \
	httpcode.h \
	core.h \
	maintoolbar.h \
	cache.h \
	jddebug.h \
	global.h \
	jdversion.h \
	command.h \
	sharedbuffer.h \
	dndmanager.h \
	usrcmdmanager.h \
	linkfiltermanager.h \
	compmanager.h \
	searchmanager.h \
	searchloader.h \
	aamanager.h \
	dispatchmanager.h \
	cssmanager.h \
	updatemanager.h \
	colorid.h \
	fontid.h \
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
	mainitempref.h \
	boarditempref.h \
	articleitempref.h \
	msgitempref.h \
	sidebaritempref.h \
	fontcolorpref.h \
	usrcmdpref.h \
	linkfilterpref.h \
	livepref.h \
	iomonitor.h


AM_CXXFLAGS = @GTKMM_CFLAGS@ @GTHREAD_CFLAGS@ @GNUTLS_CFLAGS@ @OPENSSL_CFLAGS@ @GNOMEUI_CFLAGS@ @LIBSM_CFLAGS@
