// ライセンス: 最新のGPL
//
// 自前のツールチップクラス
//

#ifndef _TOOLTIP_H
#define _TOOLTIP_H

#include <gtkmm.h>

namespace SKELETON
{
    class Tooltip : public Gtk::Window
    {
        Glib::RefPtr< Gdk::GC > m_gc;
        Gtk::Label m_label;

        int m_counter;

        // 横幅が m_min_width より大きければ表示
        int m_min_width;

      public:

        Tooltip();
        void clock_in();

        void set_text( const std::string& text );
        void set_min_width( const int& min_width ){ m_min_width = min_width; }

        void show_tooltip();
        void hide_tooltip();

      protected:
        virtual void on_realize();
        virtual bool on_expose_event( GdkEventExpose* event );
    };
}


#endif
