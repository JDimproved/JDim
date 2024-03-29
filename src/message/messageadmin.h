// ライセンス: GPL2

//
// 書き込みビューの管理クラス
//

#ifndef _MESSAGEADMIN_H
#define _MESSAGEADMIN_H

#include "skeleton/admin.h"

#include <memory>


namespace SKELETON
{
    class EditView;
}


namespace MESSAGE
{

#define ID_OF_NEWTHREAD "0000000000"

    enum
    {
        TOOLBAR_MESSAGE = 0,
        TOOLBAR_PREVIEW = 1
    };

    class MessageToolBar;
    class MessageToolBarPreview;

    class MessageAdmin : public SKELETON::Admin
    {
        std::unique_ptr<MessageToolBar> m_toolbar;
        std::unique_ptr<MessageToolBarPreview> m_toolbar_preview;

        // 書き込み用のメッセージ欄
        // インスタンスを破棄しないで、前回書き込みビューを閉じた時の
        // 日本語のON/OFF状態を次回開いたときに継続させる
        std::unique_ptr<SKELETON::EditView> m_text_message;
        sigc::signal<void> m_sig_new_subject_changed;

      public:

        explicit MessageAdmin( const std::string& url );
        ~MessageAdmin() override = default;

        void save_session() override {}

        void show_entry_new_subject( bool show );
        std::string get_new_subject() const;
        /// @brief ツールバーにあるスレタイトル入力欄のmap, changedシグナルにつなげたいハンドラをここに接続する
        sigc::signal<void> sig_new_subject_changed() { return m_sig_new_subject_changed; }
        /// @brief ツールバーのスレタイトル入力欄に接続して橋渡しするシグナルハンドラ
        void slot_new_subject_changed() { m_sig_new_subject_changed.emit(); }

        SKELETON::EditView* get_text_message();

      protected:

        void set_status( const std::string& url, const std::string& stat, const bool force ) override;

        // ツールバー
        void show_toolbar() override;

        void command_local( const COMMAND_ARGS& command ) override;

      private:

        bool delete_message( SKELETON::View * view );

        // 復元をしない
        void restore( const bool only_locked ) override {}
        COMMAND_ARGS url_to_openarg( const std::string& url, const bool tab, const bool lock ) override
        {
            COMMAND_ARGS ret;
            return ret;
        }

        void open_view( const COMMAND_ARGS& command ) override;
        void switch_admin() override;
        void tab_left( const TabMove mode ) override;
        void tab_right( const TabMove mode ) override;
        void close_view( const std::string& url ) override;
        void open_window() override;
        void close_window() override;

        // タブの D&D 処理をしない
        void slot_drag_data_get( Gtk::SelectionData& selection_data, const int page ) override {}
    };
    
    MESSAGE::MessageAdmin* get_admin();
    void delete_admin();
}

#endif
