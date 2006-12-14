// ライセンス: GPL2

#ifndef _EDITVIEW_H
#define _EDITVIEW_H

#include <gtkmm.h>

namespace SKELETON
{
    typedef sigc::signal< bool, GdkEventKey* > SIG_KEY_PRESS;
    typedef sigc::signal< bool, GdkEventKey* > SIG_KEY_RELEASE;

    // キーのプレスとリリースをフックする
    class EditTextView : public Gtk::TextView
    {
        SIG_KEY_PRESS m_sig_key_press;
        SIG_KEY_RELEASE m_sig_key_release;

      public:

        SIG_KEY_PRESS sig_key_press(){ return m_sig_key_press; }
        SIG_KEY_RELEASE sig_key_release(){ return m_sig_key_release; }

        EditTextView(){}

      protected:

        virtual bool on_key_press_event( GdkEventKey* event )
        {
            m_sig_key_press.emit( event );

            if( event->state & GDK_MOD1_MASK && event->keyval == 'w' ) return true;

            return Gtk::TextView::on_key_press_event( event );
        }

        virtual bool on_key_release_event( GdkEventKey* event )
        {
            m_sig_key_release.emit( event );

            if(  event->state & GDK_MOD1_MASK && event->keyval == 'w' ) return true;

            return Gtk::TextView::on_key_release_event( event );
        }
    };


    class EditView : public Gtk::ScrolledWindow
    {
        EditTextView m_textview;

    public:

        EditView(){
            set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );

            add( m_textview );

            show_all_children();
        }
        virtual ~EditView(){}

        SIG_KEY_PRESS sig_key_press(){ return m_textview.sig_key_press(); }
        SIG_KEY_RELEASE sig_key_release(){ return m_textview.sig_key_release(); }

        Glib::RefPtr< Gtk::TextBuffer > get_buffer(){ return m_textview.get_buffer(); }

        void set_text( const Glib::ustring& text ){ m_textview.get_buffer()->set_text( text ); }
        Glib::ustring get_text(){ return m_textview.get_buffer()->get_text(); }

        void set_editable( bool editable ){ m_textview.set_editable( editable ); }

        void modify_font( const Pango::FontDescription& font_desc ){ m_textview.modify_font( font_desc ); }

        void focus_view(){ m_textview.grab_focus(); }
    };
}

#endif
