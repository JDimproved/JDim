dnl
dnl JDim用 configure.ac
dnl
AC_PREREQ([2.69])
AC_INIT([jdim],[1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS(config.h)

AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
LT_INIT
AC_PROG_MKDIR_P

AC_C_BIGENDIAN

AC_LANG([C++])

AX_CXX_COMPILE_STDCXX([17], [noext], [mandatory])

dnl
dnl buildinfo.h
dnl
AC_DEFINE(HAVE_BUILDINFO_H, 1, Define to 1 if you have the 'buildinfo.h' header file.)
AC_PATH_PROG(GIT, git)
AC_PATH_PROG(XSUM, md5sum, [cksum])
AC_SUBST(ac_configure_args)


dnl ---------------------------------------------------
dnl ---------------------------------------------------
dnl
dnl ユーザー設定
dnl

dnl 追加コンパイルオプション
dnl -Wextraで有効になる-Wunused-parameterは修正方法の検討が必要なので暫定的に無効
CXXFLAGS="-ggdb -Wall -Wextra -Wno-unused-parameter -pedantic $CXXFLAGS"
CPPFLAGS="-DGTK_DOMAIN='\"gtk30\"' $CPPFLAGS"

dnl ---------------------------------------------------
dnl ---------------------------------------------------

LIBSM_CFLAGS=""
LIBSM_LIBS=""

dnl
dnl パッケージのチェック
dnl


dnl
dnl gtkmm
dnl
PKG_CHECK_MODULES(GTKMM, [gtkmm-3.0 >= 3.22.0])
AC_SUBST(GTKMM_CFLAGS)
AC_SUBST(GTKMM_LIBS)


dnl
dnl crypt
dnl
AC_CHECK_LIB(crypt,crypt)
AC_CHECK_LIB(crypt, crypt_r,
  [AC_DEFINE(HAVE_CRYPT_R, [1], [Define to 1 if you have the 'crypt_r' function])],
)


dnl
dnl zlib
dnl
AC_CHECK_HEADERS([zlib.h])
AC_CHECK_LIB(z,inflate)


dnl
dnl packages dependent on platform 
dnl

dnl
dnl any other POSIX systems
dnl

AC_CHECK_HEADERS([socket.h])
AC_CHECK_LIB(socket,socket)

AC_CHECK_FUNC([timegm], [],
[AC_DEFINE(NO_TIMEGM, 1, [Define to 1 if you do not have the 'timegm' function])])


dnl
dnl X関連ライブラリ
dnl
PKG_CHECK_MODULES(X11, x11)
AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)


dnl
dnl セッション管理
dnl
AC_MSG_CHECKING(for --with-sessionlib)
AC_ARG_WITH(sessionlib,
AS_HELP_STRING([--with-sessionlib=xsmp|no],[use session control library @<:@default=xsmp@:>@]),
[with_sessionlib="$withval"], [with_sessionlib=xsmp])
AC_MSG_RESULT($with_sessionlib)

AS_IF(
  [test "x$with_sessionlib" = xno],
  [],

  dnl XSMPを使ってセッション管理をする。libSMとlibICEが必要。無ければXSMPは無効になる
  [test "x$with_sessionlib" = xxsmp],
  [PKG_CHECK_MODULES(LIBSM, [sm >= 1.2], [], [ac_sm_ice_found=no])
   PKG_CHECK_MODULES(LIBICE, [ice >= 1.0], [], [ac_sm_ice_found=no])
   AS_IF(
     [test "x$ac_sm_ice_found" = xno],
     [AC_MSG_NOTICE([Disable XSMP due to LIBSM or LIBICE not found])],

     [LIBSM_CFLAGS="$LIBSM_CFLAGS $LIBICE_CFLAGS"
      LIBSM_LIBS="$LIBSM_LIBS $LIBICE_LIBS"
      AC_DEFINE(USE_XSMP, , [use xsmp])
      AC_SUBST(LIBSM_CFLAGS)
      AC_SUBST(LIBSM_LIBS)]
   )],

  [AC_MSG_ERROR([session control library not found])]
)


dnl
dnl 正規表現ライブラリのオプションは廃止された
dnl
AC_ARG_WITH(regex,
AS_HELP_STRING([--with-regex], [NO LONGER AVAILABLE]),
[AC_MSG_ERROR([regex library options have been removed. see https://github.com/JDimproved/JDim/issues/697])])


dnl
dnl TLS
dnl
AC_MSG_CHECKING(for --with-tls)
AC_ARG_WITH(tls,
AS_HELP_STRING([--with-tls=@<:@gnutls|openssl@:>@],[use TLS library @<:@default=gnutls@:>@]),
[with_tls="$withval"], [with_tls=gnutls])
AC_MSG_RESULT($with_tls)

AS_IF(
  [test "x$with_tls" = xgnutls],
  [PKG_CHECK_MODULES(GNUTLS, [gnutls >= 3.5.18])
   AC_DEFINE(USE_GNUTLS, , [use gnutls])
   AC_SUBST(GNUTLS_CFLAGS)
   AC_SUBST(GNUTLS_LIBS)],

  [test "x$with_tls" = xopenssl],
  [PKG_CHECK_MODULES(OPENSSL, [openssl >= 1.1.0])
   AC_DEFINE(USE_OPENSSL, , [use openssl])
   AC_SUBST(OPENSSL_CFLAGS)
   AC_SUBST(OPENSSL_LIBS)],

  [AC_MSG_ERROR([TLS library not found])]
)


dnl
dnl enable gprof
dnl
AC_MSG_CHECKING(for --enable-gprof)
AC_ARG_ENABLE(gprof,
AS_HELP_STRING([--enable-gprof],[enable gprof]),
[enable_gprof="$enableval"], [enable_gprof=no])
AC_MSG_RESULT($enable_gprof)

AS_IF(
  [test "x$enable_gprof" = xyes],
  [CXXFLAGS="$CXXFLAGS -pg"]
)


dnl
dnl checking migemo
dnl
AC_MSG_CHECKING(for --with-migemo)
AC_ARG_WITH(migemo,
AS_HELP_STRING([--with-migemo],[enable migemo search]),
[with_migemo="$withval"], [with_migemo=no])
AC_MSG_RESULT($with_migemo)

AS_IF(
  [test "x$with_migemo" = xyes],
  [AC_CHECK_HEADERS([migemo.h])
   AC_CHECK_LIB(migemo, migemo_open)]
)


AC_MSG_CHECKING(for --with-migemodict)
AC_ARG_WITH(migemodict,
AS_HELP_STRING([--with-migemodict=PATH],[specifiy the path of migemo dictionary]),
[with_migemodict="$withval"], [with_migemodict=])
AC_MSG_RESULT($with_migemodict)

AS_IF(
  [test -n "$with_migemodict"],
  [AC_DEFINE_UNQUOTED(MIGEMODICT, "$with_migemodict", [migemodict])]
)


dnl
dnl checking alsa
dnl
case "${host_os}" in
  linux*|*linux)
  AC_MSG_CHECKING(for --with-alsa)
  AC_ARG_WITH(alsa,
  AS_HELP_STRING([--with-alsa],[enable alsa]),
  [with_alsa="$withval"], [with_alsa=no])
  AC_MSG_RESULT($with_alsa)

  AS_IF(
    [test "x$with_alsa" = xyes],
    [PKG_CHECK_MODULES(ALSA, [alsa >= 1.0])
     AC_DEFINE(USE_ALSA, , [use alsa])
     AC_SUBST(ALSA_CFLAGS)
     AC_SUBST(ALSA_LIBS)]
  )
 ;;
esac


dnl
dnl checking googletest
dnl
dnl テストプログラムは明示的にコマンド(make test)を実行しなければビルドされない
dnl 1. 環境変数 GTEST_SRCDIR が設定されていればgoogletestのソースコードをコンパイルして使う
dnl 2. インストールされたパッケージがあればライブラリを使う
dnl 3. それ以外のときはmake testが失敗する
AM_CONDITIONAL([USE_GTEST_SOURCES], [test -d "$GTEST_SRCDIR"])
dnl PKG_CHECK_MODULES()の4番目の引数(action-if-not-found)を空欄にすると
dnl パッケージが見つからないときエラーになるためダミーの処理を入れる
AM_COND_IF([USE_GTEST_SOURCES], [],
           [PKG_CHECK_MODULES(GTEST, [gtest_main >= 1.10.0], [], [ac_gtest_main_found=no])])
AC_SUBST(GTEST_CFLAGS)
AC_SUBST(GTEST_LIBS)


dnl
dnl checking pangolayout
dnl
AC_MSG_CHECKING(for --with-pangolayout)
AC_ARG_WITH(pangolayout,
AS_HELP_STRING([--with-pangolayout],[use pangolayout]),
[with_pangolayout="$withval"], [with_pangolayout=no])
AC_MSG_RESULT($with_pangolayout)

AS_IF(
  [test "x$with_pangolayout" = xyes],
  [AC_DEFINE(USE_PANGOLAYOUT, , [use pangolayout])]
)


dnl
dnl checking compatible cache directory
dnl
AC_MSG_CHECKING(for --disable-compat-cache-dir)
AC_ARG_ENABLE(compat-cache-dir,
AS_HELP_STRING([--disable-compat-cache-dir],[disable compatible cache directory]),
[enable_compat_cache_dir="$enableval"], [enable_compat_cache_dir=yes])

AS_IF(
  [test "x$enable_compat_cache_dir" = xyes],
  [AC_MSG_RESULT(no)
   AC_DEFINE(ENABLE_COMPAT_CACHE_DIR, 1, [Support for compatible cache directory.])],
  [AC_MSG_RESULT(yes)]
)


dnl
dnl CPUの最適化オプション
dnl
AS_IF(
  dnl gprofを利用する場合最適化オプションは無視される
  [test "x$enable_gprof" = xno],
  [AC_MSG_CHECKING(for --with-native)
   AC_ARG_WITH(native, AS_HELP_STRING([--with-native],[produce code optimized for the local machine]),
               [with_native="$withval"], [with_native=no])
   AC_MSG_RESULT($with_native)
   AS_IF([test "x$with_native" != xno],
         [CXXFLAGS="$CXXFLAGS -march=native"])]
)


AC_ARG_VAR(GTEST_SRCDIR, [path for googletest framework source directory])


AC_CONFIG_FILES([Makefile src/Makefile src/skeleton/Makefile src/jdlib/Makefile src/dbtree/Makefile src/dbimg/Makefile  src/bbslist/Makefile src/board/Makefile src/article/Makefile src/image/Makefile src/message/Makefile src/history/Makefile src/config/Makefile src/icons/Makefile src/sound/Makefile src/xml/Makefile src/control/Makefile test/Makefile])
AC_OUTPUT
