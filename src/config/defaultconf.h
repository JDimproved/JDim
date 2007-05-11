// ライセンス: GPL2
//
// 設定のデフォルト値
//

#ifndef _DEFAULTCONF_H
#define _DEFAULTCONF_H

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

#define FONTSIZE_THREAD "12"
#define FONTSIZE_POPUP  "10"
#define FONTSIZE_TREE   "10"


// 色
#define CONF_COLOR_CHAR    "#000000000000"  // スレの文字

#define CONF_COLOR_CHAR_LINK "#00000000ffff" //リンク(通常)の文字色

#define CONF_COLOR_BACK           "#fde8fde8f618" // スレ背景色
#define CONF_COLOR_BACK_POPUP     CONF_COLOR_BACK // ポップアップ背景色
#define CONF_COLOR_BACK_SELECTION CONF_COLOR_CHAR_LINK // 選択範囲の背景色

#define CONF_COLOR_HL      "#ffffffff0000"  // ハイライト色
#define CONF_COLOR_HL_TREE CONF_COLOR_HL    // ツリーのハイライト色

#define CONF_COLOR_BACK_MESSAGE CONF_COLOR_BACK // メッセージビューの背景色
#define CONF_COLOR_BACK_MESSAGE_SELECTION  CONF_COLOR_BACK_SELECTION // メッセージビューの選択色

#define CONF_COLOR_SEPARATOR_NEW "#7d007d007d00" // セパレータ

#define CONF_COLOR_CHAR_BBS   CONF_COLOR_CHAR // 板一覧の文字
#define CONF_COLOR_CHAR_BOARD CONF_COLOR_CHAR // スレ一覧の文字

#define CONF_COLOR_BACK_BBS   CONF_COLOR_BACK // 板一覧の背景色
#define CONF_COLOR_BACK_BBS_EVEN CONF_COLOR_BACK // 板一覧の背景色(偶数行)
#define CONF_COLOR_BACK_BOARD CONF_COLOR_BACK // スレ一覧の背景色
#define CONF_COLOR_BACK_BOARD_EVEN CONF_COLOR_BACK // スレ一覧の背景色(偶数行)


// インライン画像を表示する
#define INLINE_IMG 0

}

#endif
