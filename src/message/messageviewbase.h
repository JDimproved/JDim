// ライセンス: GPL2

#ifndef _MESSAGEVIEWBASE_H
#define _MESSAGEVIEWBASE_H

#include "skeleton/view.h"
#include "skeleton/editview.h"
#include "skeleton/label_entry.h"
#include "skeleton/imgbutton.h"
#include "skeleton/compentry.h"

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

        bool m_enable_menuslot;
        bool m_enable_focus;

        Gtk::HBox m_toolbar;
        SKELETON::ImgButton m_button_write;
        SKELETON::ImgButton m_button_cancel;
        SKELETON::ImgButton m_button_open;
        SKELETON::ImgButton m_button_undo;
        SKELETON::ImgToggleButton m_button_not_close;
        SKELETON::ImgToggleButton m_button_preview;
        Gtk::Tooltips m_tooltip;

        SKELETON::LabelEntry m_entry_subject;
        SKELETON::NameEntry m_entry_name;
        SKELETON::MailEntry m_entry_mail;
        SKELETON::EditView m_text_message;

        // 文字数計算用
        JDLIB::Iconv* m_iconv;
        char* m_str_iconv;
        int m_max_line;
        int m_max_str;
        int m_lng_str_enc;
        int m_lng_iconv;

        // 経過時間表示用
        int m_counter;
        std::string m_str_pass;

      protected:
        
        SKELETON::LabelEntry& get_entry_subject() { return m_entry_subject; }
        SKELETON::CompletionEntry& get_entry_name(){ return m_entry_name; }
        SKELETON::CompletionEntry& get_entry_mail(){ return m_entry_mail; }
        SKELETON::EditView& get_text_message() { return m_text_message; }

      public:

        MessageViewBase( const std::string& url );
        virtual ~MessageViewBase();

        // コピー用のURL
        virtual const std::string url_for_copy();

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

      private:

        // フォント初期化
        void init_font( const std::string& fontname );

        // 色初期化
        void init_color();

        // 書き込みログ保存
        void save_postlog();

        virtual void write() = 0;

        void tab_left();
        void tab_right();

        void focus_writebutton();

        void slot_write_clicked();
        void slot_close_clicked();
        void slot_draft_open();
        void slot_undo_clicked();
        void slot_not_close_clicked();
        void slot_preview_clicked();
        bool slot_key_press( GdkEventKey* event );
        bool slot_button_press( GdkEventButton* event );
        void slot_switch_page( GtkNotebookPage*, guint page );

        virtual std::string create_message() = 0;

        void show_status();

      protected:

        void set_message( const std::string& msg ){ m_text_message.set_text( msg ); }
        Glib::ustring get_message(){ return m_text_message.get_text(); }

        void post_msg( const std::string& msg, bool new_article );
        void post_fin();

        void save_name();
        void setup_view();
        void pack_widget();
    };
}

#endif
