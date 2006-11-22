// ライセンス: GPL2

//
// 入力コントロール
//
// キー入力やマウスジェスチャ管理
//

#ifndef _CONTROL_H
#define _CONTROL_H

#include <gtkmm.h>

namespace CONTROL
{
    class Control
    {
        int m_mode;

        // マウスジェスチャ用変数
        bool m_mg; // true ならマウスジェスチャのモードになっている
        int m_mg_lng;
        int m_mg_x;
        int m_mg_y;
        int m_mg_value;
        std::string m_mg_direction;

      public:

        Control();

        // コントロールモード設定
        void set_mode( int mode ){ m_mode = mode; }

        // キー入力
        // 戻り値はコントロールID
        int key_press( GdkEventKey* event );  // 戻り値はコントロールID

        // マウスボタン
        bool button_alloted( GdkEventButton* event, int id );  // eventがidに割り当てられていたらtrue
        bool get_eventbutton( int id, GdkEventButton& event ); // ID からevent取得

        // マウスジェスチャ
        void MG_reset();
        bool MG_start( GdkEventButton* event );
        bool MG_motion( GdkEventMotion* event );
        int MG_end( GdkEventButton* event );  // 戻り値はコントロールID

        // ホイールマウスジェスチャ
        void MG_wheel_reset();
        bool MG_wheel_start( GdkEventButton* event );
        int MG_wheel_scroll( GdkEventScroll* event );  // 戻り値はコントロールID
        bool MG_wheel_end( GdkEventButton* event ); // ジェスチャを実行したらtrueを返す
    };
}


#endif
