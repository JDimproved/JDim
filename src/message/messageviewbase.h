// ライセンス: GPL2

#ifndef _MESSAGEVIEWBASE_H
#define _MESSAGEVIEWBASE_H

#include "skeleton/view.h"
#include "skeleton/editview.h"
#include "skeleton/label_entry.h"
#include "skeleton/imgbutton.h"

namespace JDLIB
{
    class Iconv;
}


namespace MESSAGE
{
    class Post;

    class MessageViewBase : public SKELETON::View
    {
        Post* m_post;

        Gtk::Notebook m_notebook;
        SKELETON::View* m_preview;
        Gtk::VBox m_msgview;
        
        Gtk::HBox m_hbox_name_mail;
        Gtk::Label m_label_name;
        Gtk::Label m_label_mail;
        Gtk::CheckButton m_check_fixname;
        Gtk::CheckButton m_check_fixmail;

        Gtk::HBox m_toolbar;
        SKELETON::ImgButton m_button_write;
        SKELETON::ImgButton m_button_cancel;
        SKELETON::ImgButton m_button_undo;
        Gtk::Tooltips m_tooltip;

        SKELETON::LabelEntry m_entry_subject;
        Gtk::Entry m_entry_name;
        Gtk::Entry m_entry_mail;
        SKELETON::EditView m_text_message;

        // ステータスバー
#if GTKMMVER <= 240
        Gtk::Statusbar m_statbar;
#else
        Gtk::HBox m_statbar;
        Gtk::Label m_label_stat;
#endif

        // 文字数計算用
        JDLIB::Iconv* m_iconv;
        char* m_str_iconv;
        int m_max_line;
        int m_max_str;
        int m_lng_str_enc;
        int m_lng_iconv;

      protected:
        
        SKELETON::LabelEntry& get_entry_subject() { return m_entry_subject; }
        Gtk::Entry& get_entry_name(){ return m_entry_name; }
        Gtk::Entry& get_entry_mail(){ return m_entry_mail; }
        SKELETON::EditView& get_text_message() { return m_text_message; }

      public:

        MessageViewBase( const std::string& url );
        ~MessageViewBase();

        // フォント初期化
        void init_font( const std::string& fontname );

        // コマンド
        virtual bool set_command( const std::string& command, const std::string& arg = std::string() );

        // SKELETON::View の関数のオーバロード
        virtual void clock_in();
        virtual void reload(){}
        virtual void relayout();
        virtual void close_view();
        virtual void redraw_view();
        virtual void focus_view();
        virtual void operate_view( const int& control );

        void post_fin();

        // 書き込みログ保存
        void save_postlog();

      private:

        virtual void write(){};

        void tab_left();
        void tab_right();

        void slot_write_clicked();
        void slot_cancel_clicked();
        void slot_undo_clicked();
        bool slot_key_release( GdkEventKey* event );
        void slot_switch_page( GtkNotebookPage*, guint page );

        virtual std::string create_message(){ return std::string() ;}

        void show_status();

      protected:

        void set_message( const std::string& msg ){ m_text_message.set_text( msg ); }
        Glib::ustring get_message(){ return m_text_message.get_text(); }

        void post_msg( const std::string& msg, bool new_article );

        void save_name();
        void setup_view();
        void pack_widget();
    };
}

#endif
