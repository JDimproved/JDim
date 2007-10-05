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
#define JDDATE    "071005"
#define JDTAG     "beta"

//---------------------------------

#define JDVERSION ( MAJORVERSION * 100 + MINORVERSION * 10 + MICROVERSION )
#define JDVERSION_FULL ( JDVERSION * 1000000 + atoi( JDDATE ) )

//--------------------------------
#ifdef JDVERSION_SVN // SVN版

#ifdef SVN_REPOSITORY // リポジトリ
#define REPOSITORY_URL SVN_REPOSITORY
#endif // SVN_REPOSITORY

#ifdef SVN_REVISION // リビジョン
#define JDVERSIONSTR "SVN Rev." + std::string( SVN_REVISION )
#else  // SVN_REVISION
#define JDVERSIONSTR ( "svn." + std::string( __DATE__ ) + "-" + std::string( __TIME__ ) )
#endif // SVN_REVISION

#else  // JDVERSION_SVN

// 通常版のバージョン
#define JDVERSIONSTR ( MISC::itostr( MAJORVERSION ) + "." + MISC::itostr( MINORVERSION ) + "." + MISC::itostr( MICROVERSION ) + "-" + std::string( JDTAG ) + std::string( JDDATE ) )

#endif // JDVERSION_SVN
//--------------------------------

#define JDCOPYRIGHT "(c) 2006-2007 JD project"
#define JDBBS CONFIG::get_url_jdhp()+"cgi-bin/bbs/support/"
#define JD2CHLOG CONFIG::get_url_jdhp()+"old2ch/"
#define JDHELP CONFIG::get_url_jdhp()+"manual/"+MISC::itostr( JDVERSION )+"/"

#define GTKMM_VERSION ( MISC::itostr( GTKMM_MAJOR_VERSION ) + "." + MISC::itostr( GTKMM_MINOR_VERSION ) + "." + MISC::itostr( GTKMM_MICRO_VERSION ) )
#define GLIBMM_VERSION ( MISC::itostr( GLIBMM_MAJOR_VERSION ) + "." + MISC::itostr( GLIBMM_MINOR_VERSION ) + "." + MISC::itostr( GLIBMM_MICRO_VERSION ) )

#endif
