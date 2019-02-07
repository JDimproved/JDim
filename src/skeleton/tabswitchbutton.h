// ライセンス: GPL2
//
// タブの切り替えボタン
//
// 枠の描画時に get_style()->paint_box_gap() で warning が出るので
// 直接 Gtk::Button を継承するのではなくて Notebook の中にボタンを入れる
// DragableNoteBook::draw_box() 参照
//

#ifndef TABSWITCHBUTON_H
#define TABSWITCHBUTON_H

#include <gtkmm.h>

namespace SKELETON
{
    class DragableNoteBook;

    class TabSwitchButton: public Gtk::Notebook
    {
#if !GTKMM_CHECK_VERSION(3,0,0)
        DragableNoteBook* m_parent;
#endif

        Gtk::VBox m_vbox;
        Gtk::Button m_button;
        Gtk::Arrow m_arrow{ Gtk::ARROW_DOWN, Gtk::SHADOW_NONE };

        bool m_shown = false;

      public:

        TabSwitchButton( DragableNoteBook* parent );
        ~TabSwitchButton() noexcept;

        Gtk::Button& get_button(){ return m_button; }
        void show_button();
        void hide_button();

#if !GTKMM_CHECK_VERSION(3,0,0)
      protected:

        bool on_expose_event( GdkEventExpose* event ) override;
#endif
    };
}

#endif
