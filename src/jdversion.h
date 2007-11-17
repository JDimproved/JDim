// バージョン情報

#ifndef _JDVER_H
#define _JDVER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_BUILDINFO_H
#include "buildinfo.h"
#endif

// svn 版の時は JDVERSION_SVN をdefineする
#define JDVERSION_SVN

#define MAJORVERSION 1
#define MINORVERSION 9
#define MICROVERSION 7
#define JDDATE    "071115"
#define JDTAG     "rc"

//---------------------------------

#define JDVERSION ( MAJORVERSION * 100 + MINORVERSION * 10 + MICROVERSION )
#define JDVERSION_FULL ( JDVERSION * 1000000 + atoi( JDDATE ) )

//--------------------------------
#ifdef JDVERSION_SVN // SVN版

#ifdef SVN_REPOSITORY // リポジトリ
#define REPOSITORY_URL SVN_REPOSITORY
#endif // SVN_REPOSITORY

#ifdef SVN_REVISION
#define JDVERSIONSTR MISC::get_svn_revision( SVN_REVISION )
#else
#define JDVERSIONSTR MISC::get_svn_revision()
#endif // SVN_REVISION

#else  // JDVERSION_SVN

// 通常版のバージョン
#define JDVERSIONSTR std::string( MISC::itostr( MAJORVERSION ) + "." + MISC::itostr( MINORVERSION ) + "." + MISC::itostr( MICROVERSION ) + "-" + std::string( JDTAG ) + std::string( JDDATE ) )

#endif // JDVERSION_SVN
//--------------------------------

#define JDCOPYRIGHT "(c) 2006-2007 JD project"
#define JDBBS CONFIG::get_url_jdhp()+"cgi-bin/bbs/support/"
#define JD2CHLOG CONFIG::get_url_jdhp()+"old2ch/"
#define JDHELP CONFIG::get_url_jdhp()+"manual/"+MISC::itostr( JDVERSION )+"/"

#define GTKMM_VERSION ( MISC::itostr( GTKMM_MAJOR_VERSION ) + "." + MISC::itostr( GTKMM_MINOR_VERSION ) + "." + MISC::itostr( GTKMM_MICRO_VERSION ) )
#define GLIBMM_VERSION ( MISC::itostr( GLIBMM_MAJOR_VERSION ) + "." + MISC::itostr( GLIBMM_MINOR_VERSION ) + "." + MISC::itostr( GLIBMM_MICRO_VERSION ) )

#endif
