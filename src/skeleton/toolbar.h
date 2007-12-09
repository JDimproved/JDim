// ライセンス: GPL2
//
// ツールバーの基底クラス
//

#ifndef _TOOLBAR_H
#define _TOOLBAR_H

#include <gtkmm.h>

#include "imgbutton.h"

namespace SKELETON
{
    class ToolBar : public Gtk::VBox
    {
        // ツールバー表示状態
        bool m_toolbar_shown;

        Gtk::Tooltips m_tooltip;
        Gtk::ScrolledWindow m_scrwin;
        Gtk::HBox m_buttonbar;

        // 閉じるボタン
        SKELETON::ImgButton m_button_close;

      public:

        ToolBar();
        virtual ~ToolBar(){}

        bool is_empty();

        // ツールバー表示
        void show_toolbar();

        // ツールバー非表示
        void hide_toolbar();

        // タブのロック
        void lock();

        // タブのアンロック
        void unlock();

        // 更新
        void update();

      protected:

        // ボタンのパッキング
        virtual void pack_buttons()=0;
        void unpack_buttons();

        Gtk::HBox& get_buttonbar(){ return m_buttonbar; }
        Gtk::Button& get_close_button(){ return m_button_close; }
        void pack_separator();
        void set_tooltip( Gtk::Widget& widget, const std::string& tip ){ m_tooltip.set_tip( widget, tip ); }

      private:

        // vboxがrealizeしたときに呼び出される
        virtual void slot_vbox_realize(){}

        // テーマが変わったときなど、vboxの背景色が変わったときに呼び出される
        virtual void slot_vbox_style_changed( Glib::RefPtr< Gtk::Style > style ){}
    };
}


#endif
