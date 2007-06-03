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
        Gtk::Label m_label;
        Gtk::Entry m_entry;

        Gdk::Color m_color_bg;
        Gdk::Color m_color_bg_org;

      public:

        void slot_realize()
        {
            Gtk::HBox::on_realize();

            // 背景色変更
            m_color_bg = get_style()->get_bg( Gtk::STATE_NORMAL );
            if( ! m_editable ) m_entry.modify_base( m_entry.get_state(), m_color_bg );
        }

        void setup(){
            m_entry.set_editable( m_editable );
            m_entry.set_activates_default( m_editable );
            m_entry.set_has_frame( m_editable );
            m_entry.property_can_focus() = m_editable;
        }

        LabelEntry( const bool editable, const std::string& label, const std::string& text = std::string() )
        : m_editable( editable )
        {
            m_label.set_text( label );
            m_color_bg_org = m_entry.get_style()->get_base( Gtk::STATE_NORMAL );

            setup();

            m_entry.set_text( text );
            m_entry.signal_realize().connect( sigc::mem_fun( *this, &LabelEntry::slot_realize ) );

            pack_start( m_label, Gtk::PACK_SHRINK );
            pack_start( m_entry );
        }

        void set_editable( bool editable ){

            m_editable = editable;

            setup();

            if( ! editable ) m_entry.modify_base( m_entry.get_state(), m_color_bg );
            else m_entry.modify_base( m_entry.get_state(), m_color_bg_org );
        }

        void set_visibility( bool visibility ){ m_entry.set_visibility( visibility ); }
        void set_text( const std::string& text ){ m_entry.set_text( text ); }
        Glib::ustring get_text(){ return m_entry.get_text(); }

        void grab_focus(){
            if( m_editable ) m_entry.grab_focus();
        }

        bool has_grab(){
            if( m_editable ) return m_entry.has_grab();
            return false;
        }
    };
}

#endif
