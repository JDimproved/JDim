// ライセンス: GPL2

//
// 入力コントロール
//
// キー入力やマウスジェスチャ管理
//

#ifndef _CONTROL_H
#define _CONTROL_H

#include <gtkmm.h>
#include <vector>

namespace CONTROL
{
    class Control
    {
        std::vector< int > m_mode;

        // マウスジェスチャ用変数
        bool m_mg; // true ならマウスジェスチャのモードになっている
        bool m_send_mg_info; // core にマウスジェスチャを送るか
        int m_mg_lng;
        int m_mg_x;
        int m_mg_y;
        int m_mg_value;
        std::string m_mg_direction;
        guint32 m_wheel_scroll_time; // 前回ホイールを回した時刻

      public:

        Control();

        // コントロールモード設定
        void add_mode( const int mode );
        void clear_mode();

        // キー入力
        // 戻り値はコントロールID
        int key_press( const GdkEventKey* event );

        // マウスボタン
        int button_press( const GdkEventButton* event ); // 戻り値はコントロールID
        bool button_alloted( const GdkEventButton* event, const int id );  // eventがidに割り当てられていたらtrue
        bool get_eventbutton( const int id, GdkEventButton& event ); // ID からevent取得

        // マウスジェスチャ
        void MG_reset();
        bool is_mg_mode() const { return m_mg; }
        void set_send_mg_info( const bool send ){ m_send_mg_info = send; }
        const std::string& get_mg_direction() const{ return m_mg_direction; }
        bool MG_start( const GdkEventButton* event );
        bool MG_motion( const GdkEventMotion* event );
        int MG_end( const GdkEventButton* event );  // 戻り値はコントロールID

        // ホイールマウスジェスチャ
        void MG_wheel_reset();
        bool MG_wheel_start( const GdkEventButton* event );
        int MG_wheel_scroll( const GdkEventScroll* event );  // 戻り値はコントロールID
        bool MG_wheel_end( const GdkEventButton* event ); // ジェスチャを実行したらtrueを返す
    };
}


#endif
