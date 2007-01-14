// ライセンス: GPL2

//
// ラベル + エントリー
//
// プロパティの表示用
//

#ifndef _LABEL_ENTRY_H
#define _LABEL_ENTRY_H

#include <gtkmm.h>

namespace SKELETON
{
    class LabelEntry : public Gtk::HBox
    {
        bool m_editable;
        Gtk::HBox m_hbox;
        Gtk::Label m_label;
        Gtk::Entry m_entry;

      public:

        LabelEntry( const bool editable, const std::string& label, const std::string& text = std::string() )
        : m_editable( editable )
        {
            m_label.set_text( label );

            if( ! m_editable ){
                m_entry.set_editable( false );
                m_entry.set_activates_default( false );
                m_entry.set_has_frame( false );
            }

            m_entry.set_text( text );

            pack_start( m_label, Gtk::PACK_SHRINK );
            pack_start( m_entry );
        }

        void set_text( const std::string& text ){ m_entry.set_text( text ); }
        Glib::ustring get_text(){ return m_entry.get_text(); }

        virtual void on_realize()
        {
            Gtk::HBox::on_realize();

            if( ! m_editable ){
            
                // entryの背景色を変える
                Gdk::Color color_bg = get_style()->get_bg( Gtk::STATE_NORMAL );
                m_entry.modify_base( m_entry.get_state(), color_bg );
            }
        }

    };
}

#endif
