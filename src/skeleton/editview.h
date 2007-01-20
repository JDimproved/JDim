// ライセンス: GPL2

#ifndef _EDITVIEW_H
#define _EDITVIEW_H

#include <gtkmm.h>

#include "control.h"

namespace SKELETON
{
    typedef sigc::signal< bool, GdkEventKey* > SIG_KEY_PRESS;
    typedef sigc::signal< bool, GdkEventKey* > SIG_KEY_RELEASE;

    // undo 用のバッファ
    struct UNDO_DATA
    {
        Glib::ustring str_diff;
        unsigned int pos;
        unsigned int pos_cursor;
        bool append;
    };


    // キーのプレスとリリースをフックする
    class EditTextView : public Gtk::TextView
    {
        SIG_KEY_PRESS m_sig_key_press;
        SIG_KEY_RELEASE m_sig_key_release;

        // 入力コントローラ
        CONTROL::Control m_control;

        // undo 用
        std::vector< UNDO_DATA > m_undo_tree;
        int m_undo_pos;
        Glib::ustring m_pre_text;
        bool m_cancel_change;
        bool m_delete_pushed;

        // カーソル移動用
        int m_pre_offset;
        int m_pre_line;
        int m_line_offset;

      public:

        SIG_KEY_PRESS sig_key_press(){ return m_sig_key_press; }
        SIG_KEY_RELEASE sig_key_release(){ return m_sig_key_release; }

        EditTextView();


        void cursor_up();
        void cursor_down();        
        void cursor_left();
        void cursor_right();
        void cursor_home();
        void cursor_end();

        void delete_char();

        void undo();

      protected:

        virtual bool on_key_press_event( GdkEventKey* event );
        virtual bool on_key_release_event( GdkEventKey* event );

        void slot_buffer_changed();

      private:
        void cursor_up_down( bool up );
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
        void set_accepts_tab( bool accept ){ m_textview.set_accepts_tab( accept ); }

        void modify_font( const Pango::FontDescription& font_desc ){ m_textview.modify_font( font_desc ); }

        void focus_view(){ m_textview.grab_focus(); }

        void undo(){ m_textview.undo(); }
    };
}

#endif
