// ライセンス: 最新のGPL

#ifndef _TABLABEL_H
#define _TABLABEL_H

#include <gtkmm.h>
#include <string>

#include "control.h"

namespace SKELETON
{
    class TabLabel : public Gtk::HBox
    {
        Gtk::Label m_label;

        // ラベルに表示する文字列の全体
        std::string m_fulltext;

      public:

        TabLabel();

        // タブの幅(ピクセル)
        const int get_tabwidth();

        // カットしていない全体の文字列
        const std::string& get_fulltext() const { return m_fulltext; }
        void set_fulltext( const std::string& label );

        // 実際にラベルに表示している文字列
        const Glib::ustring get_text() const { return m_label.get_text(); }

        // 伸縮
        bool dec();
        bool inc();

      private:

        // タブの文字列の文字数がlngになるようにリサイズする
        void resize_tab( int lng );
    }; 
}

#endif
