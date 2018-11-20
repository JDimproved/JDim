// ライセンス: GPL2

#ifndef _MESSAGEVIEWBASE_H
#define _MESSAGEVIEWBASE_H

#include "gtkmmversion.h"

#include "skeleton/view.h"
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
    class EditView;
}


namespace MESSAGE
{
    class Post;

    class MessageViewBase : public SKELETON::View
    {
        Post* m_post;

#if !GTKMM_CHECK_VERSION(2,12,0)
        Gtk::Tooltips m_tooltip;
#endif

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
        SKELETON::CompletionEntry m_entry_name;
        SKELETON::CompletionEntry m_entry_mail;

        SKELETON::EditView* m_text_message;

        bool m_enable_focus;

        // 文字数計算用
        JDLIB::Iconv* m_iconv;
        int m_max_line;
        int m_max_str;
        int m_lng_str_enc;
        int m_lng_iconv;

        // 経過時間表示用
        int m_counter;
        std::string m_str_pass;

        bool m_text_changed;

        bool m_over_lines;
        bool m_over_lng;

      public:

        MessageViewBase( const std::string& url );
        virtual ~MessageViewBase();

        //
        // SKELETON::View の関数のオーバロード
        //

        virtual void save_session(){}

        // 親ウィンドウを取得
        virtual Gtk::Window* get_parent_win();

        // コピー用のURL
        virtual const std::string url_for_copy();

        // コマンド
        virtual const bool set_command( const std::string& command,
                                        const std::string& arg1 = std::string(),
                                        const std::string& arg2 = std::string()
            );

        // ロード中
        virtual const bool is_loading() const;

        // 規制中や行数や文字列がオーバーして書き込めない
        virtual const bool is_broken(){ return ( ! m_str_pass.empty() || m_over_lines || m_over_lng ); }

        // キーを押した        
        virtual const bool slot_key_press( GdkEventKey* event );

        virtual void clock_in();
        virtual void write();
        virtual void reload(){}
        virtual void relayout();
        virtual void close_view();
        virtual void redraw_view();
        virtual void focus_view();
        virtual const bool operate_view( const int control );

      private:

        // フォント初期化
        void init_font( const std::string& fontname );

        // 色初期化
        void init_color();

        // 名前欄に名前をセット
        void set_name();

        // メール欄にアドレスをセット
        void set_mail();

        // 自分の書き込みの判定用データの保存
        void push_logitem();

        // 書き込みログ保存
        void save_postlog();

        // 実際の書き込み処理を行う関数(子クラス別に実装)
        virtual void write_impl( const std::string& msg ) = 0;  

        // プレビュー切り替え
        void toggle_preview();

        void tab_left();
        void tab_right();

        // 下書きファイル挿入
        void insert_draft();

        bool slot_button_press( GdkEventButton* event );
        void slot_switch_page( GtkNotebookPage*, guint page );
        void slot_text_changed();

        virtual const std::string create_message() = 0;

        void show_status();

      protected:

        // Viewが所属するAdminクラス
        virtual SKELETON::Admin* get_admin();

        void set_message( const std::string& msg );
        const Glib::ustring get_message();

        SKELETON::CompletionEntry& get_entry_name(){ return m_entry_name; }
        SKELETON::CompletionEntry& get_entry_mail(){ return m_entry_mail; }
        SKELETON::EditView* get_text_message() { return m_text_message; }

        void post_msg( const std::string& msg, bool new_article );
        void post_fin();

        void save_name();
        void setup_view();
        void pack_widget();

        // テキストの折り返し
        void set_wrap();
    };
}

#endif
