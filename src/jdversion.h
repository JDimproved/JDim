// バージョン情報

#ifndef _JDVER_H
#define _JDVER_H

#define JDCOPYRIGHT "(c) 2006-2007 JD project"
#define JDVERSIONSTR "1.9.5-beta070528"
#define JDVERSION 195070528
#define JDBBS CONFIG::get_url_jdhp()+"cgi-bin/bbs/support/"
#define JD2CHLOG CONFIG::get_url_jdhp()+"old2ch/"

// svn 版の時は JDVERSION_SVN をdefineする
#define JDVERSION_SVN

// ベータ版の場合は define する
#define JDVERSION_BETA

#ifdef JDVERSION_BETA
#define JDHELP CONFIG::get_url_jdhp()+"manual/develop/"
#else
#define JDHELP CONFIG::get_url_jdhp()+"manual/"+MISC::itostr( JDVERSION )+"/"
#endif


#endif
