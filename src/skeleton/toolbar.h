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

        // ツールバー表示
        void show_toolbar();

        // ツールバー非表示
        void hide_toolbar();

        // タブのロック
        void lock();

        // タブのアンロック
        void unlock();

        // ボタンのパッキング
        virtual void pack_buttons()=0;
        virtual void unpack_buttons()=0;

      protected:

        Gtk::HBox& get_buttonbar(){ return m_buttonbar; }
        Gtk::Button& get_close_button(){ return m_button_close; }
        void set_tooltip( Gtk::Widget& widget, const std::string& tip ){ m_tooltip.set_tip( widget, tip ); }

      private:

        // vboxがrealizeしたときに呼び出される
        virtual void slot_vbox_realize(){}
    };
}


#endif
