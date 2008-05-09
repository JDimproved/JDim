// ライセンス: GPL2

// ツールバーに表示する SKELETON::MenuButton、および SKELETON::BackForwardButton

// メニューがオーバーフローしたときにアイコン、または label を表示して
// 選択したら MenuButton::on_clicked() を呼び出す

#ifndef _TOOLMENUBUTTON_H
#define _TOOLMENUBUTTON_H

#include <gtkmm.h>

namespace SKELETON
{
    class MenuButton;
    class BackForwardButton;

    class ToolMenuButton : public Gtk::ToolItem
    {
        SKELETON::MenuButton* m_button;

      public:

        ToolMenuButton() : m_button( NULL ){}

        ToolMenuButton( const std::string& label, const bool expand,
                        const bool show_arrow,
                        Gtk::Widget& widget );

        ToolMenuButton( const std::string& label, const bool expand,
                        const bool show_arrow ,
                        const Gtk::StockID& stock_id,
                        const Gtk::BuiltinIconSize icon_size = Gtk::ICON_SIZE_MENU );

        SKELETON::MenuButton* get_button(){ return m_button; }

      protected:

        void setup( SKELETON::MenuButton* button, const std::string& label, const bool expand );
    };

    //////////////////////////

    class ToolBackForwardButton : public SKELETON::ToolMenuButton
    {
      public:

        ToolBackForwardButton( const std::string& label, const bool expand,
                               const std::string& url, const bool back );

        SKELETON::BackForwardButton* get_backforward_button();
    };
}

#endif
