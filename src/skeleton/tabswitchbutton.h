// ライセンス: GPL2
//
// タブの切り替えボタン
//
// TODO: 使われなくなったコンストラクタの引数を整理する

#ifndef TABSWITCHBUTON_H
#define TABSWITCHBUTON_H

#include <gtkmm.h>

namespace SKELETON
{
    class DragableNoteBook;

    class TabSwitchButton: public Gtk::Notebook
    {
        Gtk::VBox m_vbox;
        Gtk::Button m_button;
        Gtk::Image m_arrow;

        bool m_shown = false;

      public:

        explicit TabSwitchButton( DragableNoteBook* );
        ~TabSwitchButton() noexcept override;

        Gtk::Button& get_button(){ return m_button; }
        void show_button();
        void hide_button();
    };
}

#endif
