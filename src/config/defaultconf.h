// ライセンス: GPL2
//
// 設定のデフォルト値
//

#ifndef _DEFAULTCONF_H
#define _DEFAULTCONF_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace CONFIG
{
    enum{
        USE_IPV6 = 1,         // ipv6使用
        BROWSER_NO = 2,       // browsers.h のラベル番号
        TREE_YPAD = 1,        // ツリービューの行間スペース
        SCROLL_SIZE = 3,      // スレビューのスクロール量
        MARGIN_POPUP = 30,    // レスアンカーとポップアップの間のマージン
        LOOSE_URL = 1         // datのパース時にURL判定を甘くする(^なども含める)
    };

// フォント
#ifdef MONAFONT
 #define DEFAULT_FONT "Mona"
#else
 #define DEFAULT_FONT "IPA モナー Pゴシック"
#endif

#define FONTSIZE_THREAD "12"
#define FONTSIZE_POPUP  "10"
#define FONTSIZE_TREE   "10"


// 色
#define CONF_COLOR_HL      "#ffffffff0000"  // ハイライト色
#define CONF_COLOR_HL_TREE CONF_COLOR_HL    // ツリーのハイライト色

}

#endif
