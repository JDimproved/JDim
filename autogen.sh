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

unset AM_POSTFIX
for num in `seq 10 -1 7`; do
if which automake-1.$num &> /dev/null ; then
	AM_POSTFIX=-1.$num
	break
fi
done

if test -z "$AM_POSTFIX" ; then
for num in `seq 19 -1 17` ; do
if which automake$num &> /dev/null ; then
	AM_POSTFIX=$num
	break
fi
done
fi

if test -z "$AM_POSTFIX" ; then
if ! which automake &> /dev/null ; then
	echo 'you need automake (1.8.3+ recommended) to generate the Makefile'
	exit 1
fi
fi
echo `automake$AM_POSTFIX --version | head -1` found

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
