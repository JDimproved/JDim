// ライセンス: GPL2

// スクロール情報

#ifndef _SCROLLINFO_H
#define _SCROLLINFO_H

namespace ARTICLE
{
    // スクロールモード
    enum
    {
        SCROLL_NOT = 0,
        SCROLL_NORMAL,  // dy の量だけ 1 回だけスクロール
        SCROLL_LOCKED,  // 常に dy の量だけスクロール
        SCROLL_TO_NUM,  // レス番号 res にジャンプ
        SCROLL_TO_TOP,  // 先頭にジャンプ
        SCROLL_TO_BOTTOM, // 最後にジャンプ
        SCROLL_AUTO     // マーカを中心にしてオートスクロール
    };
    
    class SCROLLINFO
    {
      public:

        int mode;
        int dy; // スクロール量
        int res; // レス番号
        
        // オートスクロールモード(マウスの中ボタン押し)用の変数
        int x; // 中心のx座標
        int y; // 中心のy座標
        bool show_marker;  // true ならマーカを出す
        bool enable_up;    // true なら上方向にスクロール可
        bool enable_down;  // true なら下方向にスクロール可
        bool just_finished; // true ならオートスクロールが丁度終わったところ( slot_button_release_drawarea() で使う )

        // 実況モード用変数
        bool live;
        double live_speed;
        int live_counter;

        SCROLLINFO()
        : live( false )
        {
            reset();
        }

        void reset()
        {
            mode = SCROLL_NOT;
            dy = 0;
            res = 0;

            x = 0;
            y = 0;
            show_marker = false;
            enable_up = false;
            enable_down = false;
            just_finished = false;
        }
    };
}

#endif
