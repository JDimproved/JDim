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
//#define JDVERSION_SVN

// gitのリポジトリを使ってビルドしているときはリビジョンから日付を取得する
// リビジョンが参照できない場合はJDDATE_FALLBACKを使う
// SEE ALSO: ENVIRONMENT::get_jdversion()

#define MAJORVERSION 2
#define MINORVERSION 8
#define MICROVERSION 9
#define JDDATE_FALLBACK    "180424"
#define JDTAG     ""

//---------------------------------

#define JDVERSION ( MAJORVERSION * 100 + MINORVERSION * 10 + MICROVERSION )
#define JDVERSION_FULL ( JDVERSION * 1000000 + atoi( JDDATE_FALLBACK ) )

//---------------------------------

#define JDCOMMENT "JD は gtkmm/GTK+2 を用いた2chブラウザです。"
#define JDCOPYRIGHT "(c) 2006-2015 JD project"
#define JDBBS CONFIG::get_url_jdhp()+"cgi-bin/bbs/support/"
#define JD2CHLOG CONFIG::get_url_jdhp()+"old2ch/"
#define JDHELP CONFIG::get_url_jdhp()+"manual/"+MISC::itostr( JDVERSION )+"/"
#define JDHELPCMD CONFIG::get_url_jdhp()+"manual/"+MISC::itostr( JDVERSION )+"/usrcmd.html"

// [ ライセンス表記 ]
//
// 以下の文章は和訳を元にバージョン及び住所を訂正した物です。
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
// http://www.opensource.jp/gpl/gpl.ja.html#SEC4 (和訳)
#define JDLICENSE JDCOMMENT "\n" \
    "\n" \
    JDCOPYRIGHT "\n" \
    "\n" \
    "このプログラムはフリーソフトウェアです。あなたはこれを、フリーソフトウェ" \
    "ア財団によって発行された GNU 一般公衆利用許諾契約書(バージョン2)の定める" \
    "条件の下で再頒布または改変することができます。\n" \
    "\n" \
    "このプログラムは有用であることを願って頒布されますが、*全くの無保証* " \
    "です。商業可能性の保証や特定の目的への適合性は、言外に示されたものも含" \
    "め全く存在しません。詳しくはGNU 一般公衆利用許諾契約書をご覧ください。\n" \
    "\n" \
    "あなたはこのプログラムと共に、GNU 一般公衆利用許諾契約書の複製物を一部" \
    "受け取ったはずです。もし受け取っていなければ、フリーソフトウェア財団ま" \
    "で請求してください(宛先は the Free Software Foundation, Inc., 51 " \
    "Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA)。\n"

#endif

//---------------------------------
// for windres.rc
#ifndef DEBUG
#define JDVER_DEBUG             0
#else
#define JDVER_DEBUG             VS_FF_DEBUG
#endif

#define JDVER_FILEVERSION       MAJORVERSION,MINORVERSION,MICROVERSION,0
#define JDVER_PRODUCTVERSION    JDVER_FILEVERSION

// Two macros expansion is gcc preprocessor technic
#define JDRC_VERSION_EXP(a,b,c,d,e) JDRC_VERSION_FMT(a,b,c,d,e)
#ifdef JDVERSION_SVN
#define JDRC_VERSION_FMT(a,b,c,d,e) #a "." #b "." #c "-svnversion"
#else
#define JDRC_VERSION_FMT(a,b,c,d,e) #a "." #b "." #c "-" d e
#endif
#define JDRC_VERSION_STR JDRC_VERSION_EXP( \
            MAJORVERSION, MINORVERSION, MICROVERSION, JDTAG, JDDATE_FALLBACK)

#define JDRC_FILEVERSION        JDRC_VERSION_STR
#define JDRC_PRODUCTVERSION     JDRC_FILEVERSION

#define JDRC_PRODUCTNAME        "JD for Linux"
#define JDRC_INTERNALNAME       "JD"
#define JDRC_ORIGINALFILENAME   "jd.exe"
#define JDRC_COMPANYNAME        "JD project"
#define JDRC_FILEDESCRIPTION    JDRC_PRODUCTNAME
#define JDRC_COMMENTS           JDRC_PRODUCTNAME
#define JDRC_LEGALCOPYRIGHT     JDCOPYRIGHT

