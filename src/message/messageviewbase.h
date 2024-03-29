// ライセンス: GPL2

#ifndef _MESSAGEVIEWBASE_H
#define _MESSAGEVIEWBASE_H

#include "gtkmmversion.h"

#include "skeleton/view.h"
#include "skeleton/compentry.h"
#include "skeleton/jdtoolbar.h"

#include <memory>
#include <string_view>


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
        std::unique_ptr<Post> m_post;

        Gtk::Notebook m_notebook;
        std::unique_ptr<SKELETON::View> m_preview;
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

        SKELETON::EditView* m_text_message{};

        bool m_enable_focus;

        // 文字数計算用
        std::unique_ptr<JDLIB::Iconv> m_iconv;
        int m_max_line;
        int m_max_str;
        int m_lng_str_enc{};
        int m_lng_encoded_subject{}; ///< @brief エンコードしたスレタイトルのバイト数
        int m_lng_iconv;

        // 経過時間表示用
        int m_counter{};
        std::string m_str_pass;

        bool m_text_changed{};
        bool m_new_subject_changed{}; ///< @brief true なら新スレ作成でスレタイトルが編集された

        bool m_over_lines{};
        bool m_over_lng{};
        bool m_over_subject{}; ///< @brief true ならスレタイトルの最大バイト数を超過した

      public:

        explicit MessageViewBase( const std::string& url );
        ~MessageViewBase() override;

        //
        // SKELETON::View の関数のオーバロード
        //

        void save_session() override {}

        // 親ウィンドウを取得
        Gtk::Window* get_parent_win() override;

        // コピー用のURL
        std::string url_for_copy() const override;

        // コマンド
        bool set_command( const std::string& command,
                          const std::string& arg1 = {},
                          const std::string& arg2 = {} ) override;

        // ロード中
        bool is_loading() const override;

        // 規制中や行数や文字列がオーバーして書き込めない
        bool is_broken() const override
        {
            return ( ! m_str_pass.empty() || m_over_lines || m_over_lng || m_over_subject );
        }

        // キーを押した        
        bool slot_key_press( GdkEventKey* event ) override;

        void slot_new_subject_changed();

        void clock_in() override;
        void write() override;
        void reload() override {}
        void relayout( const bool completely = false ) override;
        void close_view() override;
        void redraw_view() override;
        void focus_view() override;
        bool operate_view( const int control ) override;

        // 特殊文字で増加する文字数を計算する
        static int count_diffs_for_special_char( std::string_view source );

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
        void slot_switch_page( Gtk::Widget*, guint page );
        void slot_text_changed();

        virtual std::string create_message( const bool utf8_post ) = 0;

        void show_status();

        /// @brief スレタイトルの最大バイト数
        virtual int get_max_subject() const noexcept { return 0; }

      protected:

        // Viewが所属するAdminクラス
        SKELETON::Admin* get_admin() override;

        void set_message( const std::string& msg );
        Glib::ustring get_message() const;

        const SKELETON::CompletionEntry& get_entry_name() const { return m_entry_name; }
        const SKELETON::CompletionEntry& get_entry_mail() const { return m_entry_mail; }
        SKELETON::EditView* get_text_message() { return m_text_message; }

        void post_msg( const std::string& msg, bool new_article );
        void post_fin();

        void save_name();
        void setup_view();
        void pack_widget();

        // テキストの折り返し
        void set_wrap();

        /// @brief エンコードしたスレタイトルのバイト数
        int get_lng_encoded_subject() const noexcept { return m_lng_encoded_subject; }
    };
}

#endif
