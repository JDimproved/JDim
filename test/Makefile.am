AUTOMAKE_OPTIONS = subdir-objects

GTEST_SRCDIR = @GTEST_SRCDIR@


.PHONY: test

# no-op
all: ;

test: check


if USE_GTEST_SOURCES

GTEST_DIR = $(GTEST_SRCDIR)/googletest
GTEST_INCS = -isystem $(GTEST_DIR)/include
GTEST_MAIN_LIB = gtest_main.a

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o: $(GTEST_DIR)/src/gtest-all.cc
	$(CXX) $(CPPFLAGS) $(GTEST_INCS) -I$(GTEST_DIR) $(CXXFLAGS) -c -o $@ $<

gtest_main.o: $(GTEST_DIR)/src/gtest_main.cc
	$(CXX) $(CPPFLAGS) $(GTEST_INCS) -I$(GTEST_DIR) $(CXXFLAGS) -c -o $@ $<

$(GTEST_MAIN_LIB): gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

endif


check_PROGRAMS = gtest_jdim
TESTS = gtest_jdim


AM_CXXFLAGS = @GTKMM_CFLAGS@ @GNUTLS_CFLAGS@ @OPENSSL_CFLAGS@ @LIBSM_CFLAGS@ @GTEST_CFLAGS@ \
	$(GTEST_INCS) -I$(top_srcdir)/src


gtest_jdim_LDADD = \
	$(top_builddir)/src/article/libarticle.a \
	$(top_builddir)/src/bbslist/libbbslist.a \
	$(top_builddir)/src/board/libboard.a \
	$(top_builddir)/src/config/libconfig.a \
	$(top_builddir)/src/control/libcontrol.a \
	$(top_builddir)/src/dbimg/libdbimg.a \
	$(top_builddir)/src/dbtree/libdbtree.a \
	$(top_builddir)/src/history/libhistory.a \
	$(top_builddir)/src/icons/libicon.a \
	$(top_builddir)/src/image/libimage.a \
	$(top_builddir)/src/jdlib/libjdlib.a  \
	$(top_builddir)/src/message/libmessage.a \
	$(top_builddir)/src/skeleton/libskeleton.a \
	$(top_builddir)/src/sound/libsound.a \
	$(top_builddir)/src/xml/libxml.a \
\
	$(top_builddir)/src/aamanager.o \
	$(top_builddir)/src/articleitemmenupref.o \
	$(top_builddir)/src/articleitempref.o \
	$(top_builddir)/src/boarditemmenupref.o \
	$(top_builddir)/src/boarditempref.o \
	$(top_builddir)/src/browsers.o \
	$(top_builddir)/src/cache.o \
	$(top_builddir)/src/command.o \
	$(top_builddir)/src/compmanager.o \
	$(top_builddir)/src/core.o \
	$(top_builddir)/src/cssmanager.o \
	$(top_builddir)/src/dispatchmanager.o \
	$(top_builddir)/src/dndmanager.o \
	$(top_builddir)/src/environment.o \
	$(top_builddir)/src/fontcolorpref.o \
	$(top_builddir)/src/iomonitor.o \
	$(top_builddir)/src/linkfiltermanager.o \
	$(top_builddir)/src/linkfilterpref.o \
	$(top_builddir)/src/livepref.o \
	$(top_builddir)/src/login2ch.o \
	$(top_builddir)/src/loginbe.o \
	$(top_builddir)/src/mainitempref.o \
	$(top_builddir)/src/maintoolbar.o \
	$(top_builddir)/src/menuslots.o \
	$(top_builddir)/src/msgitempref.o \
	$(top_builddir)/src/openurldiag.o \
	$(top_builddir)/src/prefdiagfactory.o \
	$(top_builddir)/src/replacestrmanager.o \
	$(top_builddir)/src/replacestrpref.o \
	$(top_builddir)/src/searchitempref.o \
	$(top_builddir)/src/searchloader.o \
	$(top_builddir)/src/searchmanager.o \
	$(top_builddir)/src/session.o \
	$(top_builddir)/src/setupwizard.o \
	$(top_builddir)/src/sharedbuffer.o \
	$(top_builddir)/src/sidebaritempref.o \
	$(top_builddir)/src/updatemanager.o \
	$(top_builddir)/src/urlreplacemanager.o \
	$(top_builddir)/src/usrcmdmanager.o \
	$(top_builddir)/src/usrcmdpref.o \
	$(top_builddir)/src/viewfactory.o \
	$(top_builddir)/src/winmain.o \
\
	$(GTEST_MAIN_LIB) \
	@LIBS@ @GTKMM_LIBS@ @GNUTLS_LIBS@ @OPENSSL_LIBS@ @LIBSM_LIBS@ @ALSA_LIBS@ @X11_LIBS@ @GTEST_LIBS@


# テストコードのファイルを追加したらここにリストします
# 行末のバックスラッシュに注意
gtest_jdim_SOURCES = \
	gtest_jdlib_cookiemanager.cpp \
	gtest_jdlib_jdiconv.cpp \
	gtest_jdlib_misctime.cpp \
	gtest_jdlib_misctrip.cpp \
	gtest_jdlib_miscutil.cpp \
	gtest_xml_dom.cpp
