dnl
dnl JD用 configure.ac
dnl
AC_PREREQ(2.50)
AC_INIT(jd, 1.0)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS(config.h)

AC_ISC_POSIX
AC_PROG_CC
AC_HEADER_STDC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_LIBTOOL
AC_LANG_CPLUSPLUS

AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])

dnl
dnl buildinfo.h
dnl
AC_DEFINE(HAVE_BUILDINFO_H, 1, Define to 1 if you have the 'buildinfo.h' header file.)
AC_PATH_PROG(SVN, svn)
AC_PATH_PROG(SVNVERSION, svnversion)
AC_PATH_PROG(GIT, git)
AC_PATH_PROG(XSUM, md5sum, [cksum])
AC_SUBST(ac_configure_args)

dnl
dnl OSを判定してOS別の設定
dnl 
case "${host_os}" in 
 freebsd*) 
   echo "os = freebsd"
   AC_DEFINE(ICONV_CONST, const, "iconv_const")
   AC_DEFINE(USE_MKTIME, , "use mktime")
   ;;
 solaris*)
   echo "os = solaris"
   AC_DEFINE(ICONV_CONST, const, "iconv_const")
   AC_DEFINE(NO_TIMEGM, , "no timegm")
   ;;
 mingw*)
   echo "os = ${host_os}"
   AC_DEFINE(ICONV_CONST, , "iconv_const")
   AC_DEFINE(USE_MKTIME, , "use mktime")
   ;;
 darwin*)
   echo "os = ${host_os}"
   AC_DEFUN([AM_ICONV], [])
   AC_DEFINE(ICONV_CONST, , "iconv_const")
   AC_DEFINE(USE_MKTIME, , "use mktime")
   ;;
dnl linux*|gnu*|*-gnu
 *)
   echo "os = ${host_os}"
   AC_DEFUN([AM_ICONV], [])
   AC_DEFINE(ICONV_CONST, , "iconv_const")
   ;;
esac

AM_ICONV


dnl ---------------------------------------------------
dnl ---------------------------------------------------
dnl
dnl ユーザー設定
dnl

dnl 追加コンパイルオプション
dnl -Wextraで有効になる-Wunused-parameterは修正方法の検討が必要なので暫定的に無効
CXXFLAGS="-ggdb -Wall -Wextra -Wno-unused-parameter -pedantic $CXXFLAGS"

dnl ---------------------------------------------------
dnl ---------------------------------------------------

LIBSM_CFLAGS=""
LIBSM_LIBS=""
GNOMEUI_CFLAGS=""
GNOMEUI_LIBS=""

dnl
dnl パッケージのチェック
dnl
PKG_CHECK_MODULES(GTKMM,[gtkmm-2.4 >= 2.4.0], [], [])
PKG_CHECK_MODULES(GTHREAD, [gthread-2.0 >= 2.0] )

AC_SUBST(GTKMM_CFLAGS)
AC_SUBST(GTKMM_LIBS)
AC_SUBST(GTHREAD_CFLAGS)
AC_SUBST(GTHREAD_LIBS)


dnl 
dnl crypt
dnl

echo "use crypt"
AC_CHECK_HEADERS([crypt.h])
AC_CHECK_LIB(crypt,crypt)


dnl
dnl zlib
dnl

echo "use zlib"
AC_CHECK_HEADERS([zlib.h])
AC_CHECK_LIB(z,inflate)


dnl
dnl packages dependent on platform 
dnl
use_windres=""

case "${host_os}" in 
 mingw*)
   dnl
   dnl winsock2 using model of socket
   dnl not available uname on windows
   dnl
   
   echo "use winsock2"
   AC_CHECK_HEADERS([winsock2.h])
   AC_CHECK_LIB(ws2_32,_head_libws2_32_a)
   
   echo "use regex2"
   AC_CHECK_HEADERS([regex.h])
   AC_CHECK_LIB(regex,regexec)
   
   echo "use iconv"
   AC_CHECK_HEADERS([iconv.h])
   AC_CHECK_LIB(iconv,_head_libiconv_a)
   
   echo "use windows resources"
   AC_CHECK_TOOL([WINDRES], [windres], [windres])
   AC_SUBST(WINDRES)
   AC_DEFINE(USE_WINDRES, , "use windres")
   use_windres="yes"
   ;;
 *)
   dnl
   dnl any other POSIX systems
   dnl

   echo "use uname"
   AC_CHECK_HEADERS([sys/utsname.h])

   echo "use socket"
   AC_CHECK_HEADERS([socket.h])
   AC_CHECK_LIB(socket,socket)
   ;;
esac

AM_CONDITIONAL(MAKE_WINDRES, test -n "$use_windres")


dnl
dnl X関連ライブラリ
dnl
X11_CFLAGS=""
X11_LIBS=""

case "${host_os}" in 
 mingw*)
   ;;
 *)
   PKG_CHECK_MODULES(X11, x11)
   AC_SUBST(X11_CFLAGS)
   AC_SUBST(X11_LIBS)
   ;;
esac


dnl
dnl セッション管理
dnl
dnl
use_xsmp=no
use_gnomeui=no

AC_ARG_WITH(sessionlib,
[
  --with-sessionlib[[=xsmp/gnomeui/no]]
                          use session control library [[default=xsmp]]
],
[case "${withval}" in
  xsmp)
    use_xsmp=yes
    ;;
  gnomeui)
    use_gnomeui=yes
    ;;
  no)
    use_xsmp=no
    use_gnomeui=no
    ;;
  *)
    use_xsmp=yes
    ;;
esac],use_xsmp=yes)

dnl
dnl XSMPを使ってセッション管理をする。libSMとlibICEが必要。無ければXSMPは無効になる
dnl
dnl dirsの並びは Tk の configure に書いてあったもの + Fedora 向けにディレクトリを追加
dnl
if test x"$use_xsmp" = "xyes" ; then
  
  AC_MSG_CHECKING(for SMlib.h and ICElib.h)
  LIBSM_CFLAGS=""
  dirs="/usr/unsupported/include /usr/local/include /usr/X386/include /usr/X11R6/include /usr/X11R5/include /usr/include/X11R5 /usr/include/X11R4 /usr/openwin/include /usr/X11/include /usr/sww/include /usr/include"
  for i in $dirs ; do
    if test -r $i/X11/ICE/ICElib.h -a -r $i/X11/SM/SMlib.h ; then
      AC_MSG_RESULT($i)
      LIBSM_CFLAGS="-I$i"
      break
    fi
  done
  if test -z "$LIBSM_CFLAGS" ; then
    AC_MSG_RESULT(could not find SMlib.h and ICElib.h)
  fi

  AC_MSG_CHECKING(for libSM and libICE)
  LIBSM_LIBS="" 
  dirs="/usr/unsupported/lib /usr/local/lib /usr/X386/lib /usr/X11R6/lib /usr/X11R5/lib /usr/lib/X11R5 /usr/lib/X11R4 /usr/openwin/lib /usr/X11/lib /usr/sww/X11/lib /usr/lib /usr/X11R6/lib64 /usr/lib64"
  for i in $dirs ; do
    if test -r $i/libICE.so -a -r $i/libSM.so ; then
      AC_MSG_RESULT($i)
      LIBSM_LIBS="-L$i -lICE -lSM"
      break
    fi
  done
  if test -z "$LIBSM_LIBS" ; then
    AC_MSG_RESULT(could not find libSM and libICE.)
  fi

  if test -n "$LIBSM_CFLAGS" -a -n "$LIBSM_LIBS" ; then
    echo "use XSMP"
    AC_SUBST(LIBSM_CFLAGS)
    AC_SUBST(LIBSM_LIBS)
    CXXFLAGS="$CXXFLAGS -DUSE_XSMP"
  fi
fi

dnl
dnl GNOMEUIを使ってセッション管理をする。libgnomeui-develが必要。
dnl
if test x"$use_gnomeui" = "xyes" ; then

  PKG_CHECK_MODULES(GNOMEUI, [libgnomeui-2.0 >= 2.0] )

  echo "use GNOMEUI"
  AC_SUBST(GNOMEUI_CFLAGS)
  AC_SUBST(GNOMEUI_LIBS)
  CXXFLAGS="$CXXFLAGS -DUSE_GNOMEUI"
fi


dnl
dnl SSL
dnl

use_gnutls=yes

AC_ARG_WITH(openssl,[ --with-openssl    (use openssl)],
       [ if test "$withval" != "no" ;then
         use_gnutls=no
       fi ])

case "${use_gnutls}" in 
 yes)
   echo "use gnutls"

   PKG_CHECK_MODULES(GNUTLS, [gnutls >= 1.2], [echo "gnutls >= 1.2"])
   AC_DEFINE(USE_GNUTLS, , "use gnutls")
   AC_CHECK_HEADERS([gcrypt.h])
   AC_CHECK_LIB(gcrypt, gcry_md_hash_buffer, [], [AC_MSG_ERROR([gcrypt not found])])
   AC_SUBST(GNUTLS_CFLAGS)
   AC_SUBST(GNUTLS_LIBS)
   ;;
 *)
   echo "use openssl"
   PKG_CHECK_MODULES(OPENSSL, [openssl >= 0.9] )
   AC_DEFINE(USE_OPENSSL, , "use openssl")
   AC_SUBST(OPENSSL_CFLAGS)
   AC_SUBST(OPENSSL_LIBS)
   ;;
esac


dnl
dnl enable gprof
dnl

use_gprof=no

AC_ARG_ENABLE(gprof,[ --enable-gprof  (enable gprof)],
       [ if test "$enable_gprof" = "yes"; then
               echo "use gprof"
               CXXFLAGS="$CXXFLAGS -pg"
               use_gprof=yes
       fi ])

dnl
dnl checking migemo
dnl
AC_ARG_WITH(migemo,[ --with-migemo    (enable migemo search)],
       [ if test "$withval" != "no" ;then
               echo "use migemo"
               AC_CHECK_HEADERS([migemo.h])
               AC_CHECK_LIB(migemo,migemo_open)
       fi ])


AC_ARG_WITH(migemodict,[ --with-migemodict    (specifiy the path of migemo dictionary)],
       [ if test "$withval" ;then
               echo "migemodict = $withval"
               AC_DEFINE_UNQUOTED(MIGEMODICT, "$withval" , "migemodict")
       fi ])


dnl
dnl checking xdg-open
dnl
AC_ARG_WITH(xdgopen,[ --with-xdgopen    (use xdg-open as default browser)],
       [ if test "$withval" ;then
               echo "use xdg-open"
               AC_DEFINE(XDGOPEN, , "use xdg-open")
       fi ])


dnl
dnl checking alsa
dnl
case "${host_os}" in
 linux*|*linux)
 AC_ARG_WITH(alsa,[ --with-alsa (enable alsa)],
       [ if test "$withval" != "no" ;then
               echo "use alsa"
	       PKG_CHECK_MODULES(ALSA, [alsa >= 1.0] )
	       AC_DEFINE(USE_ALSA, , "use alsa")
	       AC_SUBST(ALSA_CFLAGS)
	       AC_SUBST(ALSA_LIBS)
       fi ])
 ;;
esac


dnl
dnl checking oniguruma
dnl
AC_ARG_WITH(oniguruma,[ --with-oniguruma    (enable oniguruma)],
       [ if test "$withval" != "no" ;then
               echo "use oniguruma"
               AC_CHECK_PROG(ONIG_CONFIG, onig-config, onig-config)
               if test "x${ONIG_CONFIG}" = "x" ; then
                 AC_MSG_ERROR([onig-config not found])
               fi
               ONIG_CFLAGS=`onig-config --cflags`
               CPPFLAGS="${CPPFLAGS} ${ONIG_CFLAGS}"
               ONIG_LIBS=`onig-config --libs`
               LDFLAGS_ORIGINAL="${LDFLAGS}"
               LDFLAGS="${LDFLAGS} ${ONIG_LIBS}"
               AC_CHECK_HEADER([onigposix.h], ,[AC_MSG_ERROR([onigposix.h not found])])
               AC_CHECK_LIB([onig],[regexec], ,[AC_MSG_ERROR([libonig.a not found])])
               AC_DEFINE(USE_ONIG, , "use oniguruma")
               AC_SUBST(ONIG_LIBS)
               LDFLAGS="${LDFLAGS_ORIGINAL}"
       fi ])


dnl
dnl checking PCRE
dnl
AC_ARG_WITH(pcre,[ --with-pcre    (enable PCRE)],
       [ if test "$withval" != "no" ;then
               echo "use PCRE"
               PKG_CHECK_MODULES(PCRE, [libpcre >= 6.5] )
               AC_DEFINE(USE_PCRE, , "use PCRE")
               AC_SUBST(PCRE_CFLAGS)
               AC_SUBST(PCRE_LIBS)
               AC_CHECK_HEADERS([pcreposix.h])
               AC_CHECK_LIB(pcreposix,regexec)
       fi ])


dnl
dnl checking gthread
dnl
AC_MSG_CHECKING(for --with-gthread)
AC_ARG_WITH(gthread,
  AS_HELP_STRING([--with-gthread],
                 [use gthread instead of pthread [default=no]]),
  [with_gthread="$withval"],
  [with_gthread=no])
AC_MSG_RESULT($with_gthread)


dnl
dnl checking std::thread
dnl
AC_MSG_CHECKING(for --with-stdthread)
AC_ARG_WITH(stdthread,
  AS_HELP_STRING([--with-stdthread],
                 [use std::thread instead of pthread [default=no]]),
  [with_stdthread="$withval"],
  [with_stdthread=no])
AC_MSG_RESULT($with_stdthread)


AS_IF(
  [test "x$with_gthread" = xyes -a "x$with_stdthread" = xyes],
  [AC_MSG_ERROR([cannot configure both gthread and stdthread.])],
  [test "x$with_gthread" = xyes],
  [AC_DEFINE(USE_GTHREAD, , [use gthread instead of pthread])],
  [test "x$with_stdthread" = xyes],
  [AC_DEFINE(WITH_STD_THREAD, 1, [Use std::thread instead of pthread])]
)


dnl
dnl CPU別の最適化オプション
dnl
if test "$use_gprof" = "no"; then

    dnl
    dnl checking native (gcc >= 4.2 x86 & x86_64)
    dnl
	AC_ARG_WITH(native,[ --with-native    (use native)],
        [ if test "$withval" != "no"; then
            echo "use native"
            CXXFLAGS="$CXXFLAGS -march=native"
        fi ])

    dnl
    dnl checking core2duo
    dnl
    AC_ARG_WITH(core2duo,[ --with-core2duo    (use core2duo)],
        [ if test "$withval" != "no"; then
            echo "use core2duo"
            CXXFLAGS="$CXXFLAGS -march=pentium-m -msse3"
        fi ])

    dnl
    dnl checking athlon64
    dnl
    AC_ARG_WITH(athlon64,[ --with-athlon64    (use athlon64)],
        [ if test "$withval" != "no"; then
            echo "use athlon64"
            CXXFLAGS="$CXXFLAGS -march=athlon64"
        fi ])

    dnl
    dnl checking atom
    dnl
    AC_ARG_WITH(atom,[ --with-atom    (use atom)],
        [ if test "$withval" != "no"; then
            echo "use atom"
            CXXFLAGS="$CXXFLAGS -march=prescott"
        fi ])

    dnl
    dnl checking ppc7400
    dnl
    AC_ARG_WITH(ppc7400,[ --with-ppc7400    (use PowerPC7400)],
        [ if test "$withval" != "no"; then
            echo "use ppc7400"
            CXXFLAGS="$CXXFLAGS -mcpu=7400 -maltivec -mabi=altivec"
        fi ])

    dnl
    dnl checking ppc7450
    dnl
    AC_ARG_WITH(ppc7450,[ --with-ppc7450    (use PowerPC7450)],
        [ if test "$withval" != "no"; then
            echo "use ppc7450"
            CXXFLAGS="$CXXFLAGS -mcpu=7450 -maltivec -mabi=altivec"
        fi ])
fi


dnl
dnl checking pangolayout
dnl
AC_ARG_WITH(pangolayout,[ --with-pangolayout    (use pangolayout)],
       [ if test "$withval" != "no" ;then
               echo "use pango_layout for drawing"
	       CXXFLAGS="$CXXFLAGS -DUSE_PANGOLAYOUT"
       fi ])


AC_OUTPUT(Makefile src/Makefile src/skeleton/Makefile src/jdlib/Makefile src/dbtree/Makefile src/dbimg/Makefile  src/bbslist/Makefile src/board/Makefile src/article/Makefile src/image/Makefile src/message/Makefile src/history/Makefile src/config/Makefile src/icons/Makefile src/sound/Makefile src/xml/Makefile src/control/Makefile )
