// バージョン情報

#ifndef _JDVER_H
#define _JDVER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_BUILDINFO_H
#include "buildinfo.h"
#endif

// gitのリポジトリを使ってビルドしているときはリビジョンから日付を取得する
// リビジョンが参照できない場合はJDDATE_FALLBACKを使う
// SEE ALSO: ENVIRONMENT::get_jdversion()

#define MAJORVERSION 0
#define MINORVERSION 14
#define MICROVERSION 0
#define JDDATE_FALLBACK    "20250614"
#define JDTAG     "beta"

//---------------------------------

#define JDCOMMENT "JDim (JD improved) は gtkmm/GTK+ を用いた2chブラウザです。"
#define JDCOPYRIGHT "(c) 2006-2015 JD project" "\n" \
                    "(c) 2017-2019 yama-natuki" "\n" \
                    "(c) 2019-2025 JDimproved project"
#define JDBBS CONFIG::get_url_jdhp()+"cgi-bin/bbs/support/"
#define JD2CHLOG CONFIG::get_url_jdhp()+"old2ch/"
#define JDHELP "https://jdimproved.github.io/JDim/"
#define JDHELPCMD JDHELP "usrcmd/"
#define JDHELPREPLSTR JDHELP "replacestr/"
#define JDHELPIMGHASH JDHELP "imghash/"

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

/// パッケージ作成者の情報
#ifdef PACKAGE_PACKAGER
#define JDIM_PACKAGER PACKAGE_PACKAGER
#else
#define JDIM_PACKAGER "バイナリ/ソース( <配布元> )"
#endif

#endif
