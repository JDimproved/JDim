// ライセンス: GPL2

#ifndef _MESSAGEVIEWBASE_H
#define _MESSAGEVIEWBASE_H

#include "skeleton/view.h"
#include "skeleton/editview.h"
#include "skeleton/imgbutton.h"
#include "skeleton/compentry.h"
#include "skeleton/jdtoolbar.h"

namespace JDLIB
{
    class Iconv;
}


namespace SKELETON
{
    class Admin;
    class LabelEntry;
}


namespace MESSAGE
{
    class Post;

    class MessageViewBase : public SKELETON::View
    {
        Post* m_post;

        Gtk::Tooltips m_tooltip;

        Gtk::Notebook m_notebook;
        SKELETON::View* m_preview;
        Gtk::VBox m_msgview;

        SKELETON::JDToolbar m_toolbar_name_mail;

        Gtk::ToolItem m_tool_name;
        Gtk::ToolItem m_tool_mail;
        Gtk::ToolItem m_tool_fixname;
        Gtk::ToolItem m_tool_fixmail;
        Gtk::ToolItem m_tool_entry_name;
        Gtk::ToolItem m_tool_entry_mail;

        Gtk::Label m_label_name;
        Gtk::Label m_label_mail;
        Gtk::CheckButton m_check_fixname;
        Gtk::CheckButton m_check_fixmail;
        SKELETON::NameEntry m_entry_name;
        SKELETON::MailEntry m_entry_mail;

        SKELETON::EditView m_text_message;

        bool m_enable_focus;

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

      public:

        MessageViewBase( const std::string& url );
        virtual ~MessageViewBase();

        // コピー用のURL
        virtual const std::string url_for_copy();

        // コマンド
        virtual bool set_command( const std::string& command, const std::string& arg = std::string() );

        // ロード中
        virtual const bool is_loading();

        // SKELETON::View の関数のオーバロード
        virtual void clock_in();
        virtual void write();
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

        // 実際の書き込み処理を行う関数(子クラス別に実装)
        virtual void write_impl() = 0;  

        // プレビュー切り替え
        void toggle_preview();

        void tab_left();
        void tab_right();

        // 下書きファイル挿入
        void insert_draft();

        bool slot_key_press( GdkEventKey* event );
        bool slot_button_press( GdkEventButton* event );
        void slot_switch_page( GtkNotebookPage*, guint page );

        virtual std::string create_message() = 0;

        void show_status();

      protected:

        // Viewが所属するAdminクラス
        virtual SKELETON::Admin* get_admin();

        void set_message( const std::string& msg ){ m_text_message.set_text( msg ); }
        Glib::ustring get_message(){ return m_text_message.get_text(); }

        SKELETON::CompletionEntry& get_entry_name(){ return m_entry_name; }
        SKELETON::CompletionEntry& get_entry_mail(){ return m_entry_mail; }
        SKELETON::EditView& get_text_message() { return m_text_message; }

        void post_msg( const std::string& msg, bool new_article );
        void post_fin();

        void save_name();
        void setup_view();
        void pack_widget();
    };
}

#endif
