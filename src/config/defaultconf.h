// ライセンス: GPL2
//
// 設定のデフォルト値
//

#ifndef _DEFAULTCONF_H
#define _DEFAULTCONF_H

namespace CONFIG
{
    enum{
        MARGIN_POPUP = 30,    // レスアンカーとポップアップの間のマージン
        SCROLL_SIZE = 3,      // スレビューのスクロール量
        LOOSE_URL = 1         // datのパース時にURL判定を甘くする(^なども含める)
    };
}

#endif
