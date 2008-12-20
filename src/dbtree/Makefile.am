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
\
	settingloader.cpp \
	ruleloader.cpp \
	boardfactory.cpp \
\
	articlebase.cpp \
	article2ch.cpp \
	article2chcompati.cpp \
	articlejbbs.cpp \
	articlemachi.cpp \
\
	nodetreebase.cpp \
	nodetree2ch.cpp \
	nodetree2chcompati.cpp \
	nodetreejbbs.cpp \
	nodetreemachi.cpp

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
	boardjbbs.h \
	boardmachi.h \
\
	settingloader.h \
	ruleloader.h \
	boardfactory.h \
\
	articlebase.h \
	article2ch.h \
	article2chcompati.h \
	articlejbbs.h \
	articlemachi.h \
\
	node.h \
	nodetreebase.h \
	nodetree2ch.h \
	nodetree2chcompati.h \
	nodetreejbbs.h \
	nodetreemachi.h

AM_CXXFLAGS = @GTKMM_CFLAGS@ @ONIG_CFLAGS@
INCLUDES = -I$(top_srcdir)/src
