// バージョン情報

#ifndef _JDVER_H
#define _JDVER_H

// ベータ版の場合は define する
#define JDVERSION_BETA

#ifdef JDVERSION_BETA
// svn 版の時は JDVERSION_SVN をdefineする
#define JDVERSION_SVN
#define JDHELP "http://jd4linux.sourceforge.jp/manual/develop/"
#else
#define JDHELP "http://jd4linux.sourceforge.jp/manual/"
#endif

#define JDCOPYRIGHT "(c) 2006-2007 JD project"
#define JDVERSIONSTR "1.9.5-beta070516"
#define JDVERSION 195070516
#define JDURL "http://jd4linux.sourceforge.jp/"
#define JDBBS "http://jd4linux.sourceforge.jp/cgi-bin/bbs/support/"
#define JD2CHLOG "http://jd4linux.sourceforge.jp/old2ch/"

#endif
