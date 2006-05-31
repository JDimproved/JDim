// ライセンス: 最新のGPL
//
// コントロール系ユーティリティ関数
//

#ifndef _CONTROLUTIL_H
#define _CONTROLUTIL_H

#include <gtkmm.h>

namespace CONTROL
{
    // メニューにショートカットキーやマウスジェスチャを表示
    void set_menu_motion( Gtk::Menu* menu );

    // ラベルからID取得
    int get_id( const std::string& label );

    // IDからラベル取得
    const std::string get_label( int id );

    // IDから操作取得
    const std::string get_motion( int id );

    // IDからラベルと操作の両方を取得
    const std::string get_label_motion( int id );
}

#endif
