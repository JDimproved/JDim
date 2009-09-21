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

#define MAJORVERSION 2
#define MINORVERSION 4
#define MICROVERSION 2
#define JDDATE    "090921"
#define JDTAG     "rc"

//---------------------------------

#define JDVERSION ( MAJORVERSION * 100 + MINORVERSION * 10 + MICROVERSION )
#define JDVERSION_FULL ( JDVERSION * 1000000 + atoi( JDDATE ) )

//---------------------------------

#define JDCOMMENT "JD は gtkmm/GTK+2 を用いた2chブラウザです。"
#define JDCOPYRIGHT "(c) 2006-2009 JD project"
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
