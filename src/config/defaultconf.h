// ライセンス: GPL2
//
// 設定のデフォルト値
//

#ifndef _DEFAULTCONF_H
#define _DEFAULTCONF_H

namespace CONFIG
{
    enum{
        TREE_YPAD = 1,        // ツリービューの行間スペース
        SCROLL_SIZE = 3,      // スレビューのスクロール量
        MARGIN_POPUP = 30,    // レスアンカーとポップアップの間のマージン
        LOOSE_URL = 1         // datのパース時にURL判定を甘くする(^なども含める)
    };
}

#endif
