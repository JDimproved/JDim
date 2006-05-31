// ライセンス: 最新のGPL

#ifndef _MESSAGEVIEWBASE_H
#define _MESSAGEVIEWBASE_H

#include "skeleton/view.h"
#include "skeleton/editview.h"
#include "skeleton/imgbutton.h"

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
        Gtk::Label m_label_board;
        SKELETON::ImgButton m_button_write;
        SKELETON::ImgButton m_button_cancel;
        Gtk::Tooltips m_tooltip;

        Gtk::Entry m_entry_subject;
        Gtk::Entry m_entry_name;
        Gtk::Entry m_entry_mail;
        SKELETON::EditView m_text_message;

      protected:
        
        Gtk::Entry& get_entry_subject() { return m_entry_subject; }
        Gtk::Entry& get_entry_name(){ return m_entry_name; }
        Gtk::Entry& get_entry_mail(){ return m_entry_mail; }
        SKELETON::EditView& get_text_message() { return m_text_message; }

      public:

        MessageViewBase( const std::string& url );
        ~MessageViewBase();

        // コマンド
        virtual bool set_command( const std::string& command, const std::string& arg = std::string() );

        // SKELETON::View の関数のオーバロード
        virtual void reload(){}
        virtual void close_view();
        virtual void focus_view();
        virtual void operate_view( const int& control );

        void post_fin();

      private:

        virtual void write(){};

        void tab_left();
        void tab_right();

        void slot_write_clicked();
        void slot_cancel_clicked();
        bool slot_key_release( GdkEventKey* event );
        virtual void slot_switch_page( GtkNotebookPage*, guint page );

        virtual std::string create_message(){ return std::string() ;}

      protected:

        void set_message( const std::string& msg ){ m_text_message.set_text( msg ); }
        Glib::ustring get_message(){ return m_text_message.get_text(); }

        void post_msg( const std::string& msg, bool new_article );

        void save_name();
        virtual void setup_view();
        virtual void pack_widget();
    };
}

#endif
