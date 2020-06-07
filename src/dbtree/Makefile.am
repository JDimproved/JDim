noinst_LIBRARIES = libdbtree.a

libdbtree_a_SOURCES = \
	interface.cpp \
	spchar_decoder.cpp \
	root.cpp \
\
	boardbase.cpp \
	board2ch.cpp \
	board2chcompati.cpp \
	boardjbbs.cpp \
	boardmachi.cpp \
	boardlocal.cpp \
\
	frontloader.cpp \
	settingloader.cpp \
	ruleloader.cpp \
	boardfactory.cpp \
	articlehash.cpp \
\
	articlebase.cpp \
	article2ch.cpp \
	article2chcompati.cpp \
	articlejbbs.cpp \
	articlemachi.cpp \
	articlelocal.cpp \
\
	nodetreebase.cpp \
	nodetree2ch.cpp \
	nodetree2chcompati.cpp \
	nodetreejbbs.cpp \
	nodetreemachi.cpp \
	nodetreelocal.cpp \
	nodetreedummy.cpp

noinst_HEADERS = \
	interface.h \
	spchar_decoder.h \
	spchar_tbl.h \
	etcboardinfo.h \
	root.h \
\
	boardbase.h \
	board2ch.h \
	board2chcompati.h \
	boardlocal.h \
	boardjbbs.h \
	boardmachi.h \
\
	frontloader.h \
	settingloader.h \
	ruleloader.h \
	boardfactory.h \
	articlehash.h \
\
	articlebase.h \
	article2ch.h \
	article2chcompati.h \
	articlelocal.h \
	articlejbbs.h \
	articlemachi.h \
\
	node.h \
	nodetreebase.h \
	nodetree2ch.h \
	nodetree2chcompati.h \
	nodetreejbbs.h \
	nodetreemachi.h \
	nodetreelocal.h \
	nodetreedummy.h

AM_CXXFLAGS = @GTKMM_CFLAGS@
AM_CPPFLAGS = -I$(top_srcdir)/src
