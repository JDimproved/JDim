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

        TabLabel(){
            pack_start( m_label );
            show_all_children();
        }

        Gtk::Label& get_label() { return m_label; }

        void set_fulltext( const std::string& label ){ m_fulltext = label; }
        const std::string& get_fulltext() const { return m_fulltext; }
        void set_text( const std::string& label ){ m_label.set_text( label ); }
    }; 
}

#endif
