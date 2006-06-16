#!/bin/sh

if test ! -f install-sh ; then touch install-sh ; fi

MAKE=`which gnumake`
if test ! -x "$MAKE" ; then MAKE=`which gmake` ; fi
if test ! -x "$MAKE" ; then MAKE=`which make` ; fi
HAVE_GNU_MAKE=`$MAKE --version|grep -c "Free Software Foundation"`

if test "$HAVE_GNU_MAKE" != "1"; then 
echo Only non-GNU make found: $MAKE
else
echo `$MAKE --version | head -1` found
fi

if which autoconf2.50 >/dev/null 2>&1
then AC_POSTFIX=2.50
elif which autoconf259 >/dev/null 2>&1
then AC_POSTFIX=259
elif which autoconf >/dev/null 2>&1
then AC_POSTFIX=""
else 
  echo 'you need autoconfig (2.58+ recommended) to generate the Makefile'
  exit 1
fi
echo `autoconf$AC_POSTFIX --version | head -1` found

if which automake-1.9 >/dev/null 2>&1
then AM_POSTFIX=-1.9
elif which automake-1.8 >/dev/null 2>&1
then AM_POSTFIX=-1.8
elif which automake-1.7 >/dev/null 2>&1
then AM_POSTFIX=-1.7
elif which automake-1.6 >/dev/null 2>&1
then AM_POSTFIX=-1.6
elif which automake19 >/dev/null 2>&1
then AM_POSTFIX=19
elif which automake >/dev/null 2>&1
then AM_POSTFIX=""
else
  echo 'you need automake (1.8.3+ recommended) to generate the Makefile'
  exit 1
fi

if which libtoolize15 >/dev/null 2>&1
then LB_POSTFIX=15
elif which libtoolize >/dev/null 2>&1
then LB_POSTFIX=""
else
  echo 'you need libtoolize to generate the Makefile'
  exit 1
fi

echo `libtoolize$LB_POSTFIX --version | head -1` found

if test -d /usr/local/share/aclocal ; then
  ACLOCAL_INCLUDE="-I /usr/local/share/aclocal"
fi

echo `automake$AM_POSTFIX --version | head -1` found

echo This script runs configure and make...
echo You did remember necessary arguments for configure, right?

# autoreconf$AC_POSTFIX -fim _might_ do the trick, too.
#  chose to your taste
aclocal$AM_POSTFIX $ACLOCAL_INCLUDE
libtoolize$LB_POSTFIX --force --copy
autoheader$AC_POSTFIX
automake$AM_POSTFIX --add-missing --copy --gnu
autoconf$AC_POSTFIX
#./configure $* && $MAKE
