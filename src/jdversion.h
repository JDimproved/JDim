// バージョン情報

#ifndef _JDVER_H
#define _JDVER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// svn 版の時は JDVERSION_SVN をdefineする
#define JDVERSION_SVN

#define MAJORVERSION 1
#define MINORVERSION 9
#define MICROVERSION 6
#define JDDATE    "070804"
#define JDTAG     "beta"

//---------------------------------

#define JDVERSION ( MAJORVERSION * 100 + MINORVERSION * 10 + MICROVERSION )
#define JDVERSION_FULL ( JDVERSION * 1000000 + atoi( JDDATE ) )

#ifdef JDVERSION_SVN
#ifdef SVN_REVISION
// リビジョンが取得できた場合のSVN版のバージョン
#define JDVERSIONSTR ( "SVN Rev." + MISC::itostr( SVN_REVISION ) )
#else
// リビジョンが取得できなかった場合のSVN版のバージョン
#define JDVERSIONSTR ( "svn." + std::string( __DATE__ ) + "-" + std::string( __TIME__ ) )
#endif
#else
// 通常版のバージョン
#define JDVERSIONSTR ( MISC::itostr( MAJORVERSION ) + "." + MISC::itostr( MINORVERSION ) + "." + MISC::itostr( MICROVERSION ) + "-" + std::string( JDTAG ) + std::string( JDDATE ) )
#endif

#define JDCOPYRIGHT "(c) 2006-2007 JD project"
#define JDBBS CONFIG::get_url_jdhp()+"cgi-bin/bbs/support/"
#define JD2CHLOG CONFIG::get_url_jdhp()+"old2ch/"
#define JDHELP CONFIG::get_url_jdhp()+"manual/"+MISC::itostr( JDVERSION )+"/"

#define GTKMM_VERSION ( MISC::itostr( GTKMM_MAJOR_VERSION ) + "." + MISC::itostr( GTKMM_MINOR_VERSION ) + "." + MISC::itostr( GTKMM_MICRO_VERSION ) )
#define GLIBMM_VERSION ( MISC::itostr( GLIBMM_MAJOR_VERSION ) + "." + MISC::itostr( GLIBMM_MINOR_VERSION ) + "." + MISC::itostr( GLIBMM_MICRO_VERSION ) )

#endif
