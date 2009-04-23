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
        DragableNoteBook* m_parent;

        Gtk::Button m_button;
        Gtk::Arrow m_arrow;

      public:

        TabSwitchButton( DragableNoteBook* parent );
        virtual ~TabSwitchButton();

        Gtk::Button& get_button(){ return m_button; }

      protected:

        virtual bool on_expose_event( GdkEventExpose* event );
    };
}

#endif
