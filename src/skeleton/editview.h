// ライセンス: GPL2

#ifndef _EDITVIEW_H
#define _EDITVIEW_H

#include <gtkmm.h>

#include "control/control.h"
#include "jdlib/miscutil.h"

namespace SKELETON
{
    class AAMenu;

    typedef sigc::signal< bool, GdkEventKey* > SIG_KEY_PRESS;
    typedef sigc::signal< bool, GdkEventKey* > SIG_KEY_RELEASE;
    typedef sigc::signal< bool, GdkEventButton* > SIG_BUTTON_PRESS;

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
        SIG_BUTTON_PRESS m_sig_button_press;

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

        // コンテキストメニュー
        Gtk::Menu* m_context_menu;

        // AAポップアップ
        AAMenu* m_aapopupmenu;

      public:

        SIG_KEY_PRESS sig_key_press(){ return m_sig_key_press; }
        SIG_KEY_RELEASE sig_key_release(){ return m_sig_key_release; }
        SIG_BUTTON_PRESS sig_button_press() { return m_sig_button_press; }

        EditTextView();
        ~EditTextView();

        void insert_str( const std::string& str, bool use_br );

        void cursor_up();
        void cursor_down();        
        void cursor_left();
        void cursor_right();
        void cursor_home();
        void cursor_end();

        void delete_char();
        void backsp_char();

        void undo();
        void clear_undo();

        // カーソルの画面上の座標
        Gdk::Rectangle get_cursor_root_origin();

      protected:

        bool on_button_press_event( GdkEventButton* event ) override;

        bool on_key_press_event( GdkEventKey* event ) override;
        bool on_key_release_event( GdkEventKey* event ) override;

        void on_populate_popup( Gtk::Menu* menu ) override;

        void slot_buffer_changed();

      private:

        void cursor_up_down( bool up );

        void slot_select_aamenu();
        void slot_map_popupmenu();
        void slot_hide_popupmenu();

        // クリップボードから引用
        void slot_quote_clipboard();

        // 変換(スペース⇔&nbsp;)
        void slot_convert_space();
 
        // JDの動作環境を記入
        void slot_write_jdinfo();

        // AA ポップアップ
        void slot_popup_aamenu_pos( int& x, int& y, bool& push_in );
        void show_aalist_popup();
        void slot_aamenu_selected( const std::string& aa );
        void slot_map_aamenu();
        void slot_hide_aamenu();
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
        ~EditView() noexcept {}

        SIG_BUTTON_PRESS sig_button_press(){ return m_textview.sig_button_press(); }
        SIG_KEY_PRESS sig_key_press(){ return m_textview.sig_key_press(); }
        SIG_KEY_RELEASE sig_key_release(){ return m_textview.sig_key_release(); }

        Glib::RefPtr< Gtk::TextBuffer > get_buffer(){ return m_textview.get_buffer(); }

        void set_text( const Glib::ustring& text ){ m_textview.get_buffer()->set_text( text ); }
        Glib::ustring get_text(){
#ifdef _WIN32
            return MISC::utf8_fix_wavedash( m_textview.get_buffer()->get_text(), MISC::WINtoUNIX );
#else
            return m_textview.get_buffer()->get_text();
#endif
        }

        void set_wrap_mode( Gtk::WrapMode wrap_mode ){ m_textview.set_wrap_mode( wrap_mode ); }

        void modify_text( Gtk::StateType state, const Gdk::Color& color ){ m_textview.modify_text( state, color ); }
        void modify_base( Gtk::StateType state, const Gdk::Color& color ){ m_textview.modify_base( state, color ); }

        void insert_str( const std::string& str, bool use_br ){ m_textview.insert_str( str, use_br ); }

        void set_editable( bool editable ){ m_textview.set_editable( editable ); }
        void set_accepts_tab( bool accept ){ m_textview.set_accepts_tab( accept ); }

        void modify_font( const Pango::FontDescription& font_desc ){ m_textview.modify_font( font_desc ); }

        void focus_view(){ m_textview.grab_focus(); }

        void undo(){ m_textview.undo(); }
        void clear_undo(){ m_textview.clear_undo(); }

        // カーソルの画面上の座標
        Gdk::Rectangle get_cursor_root_origin(){ return m_textview.get_cursor_root_origin(); }
    };
}

#endif
