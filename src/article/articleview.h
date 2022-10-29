// ライセンス: GPL2

//
// メインビュー
//

#ifndef _ARTICLEVIEW_H
#define _ARTICLEVIEW_H

#include "articleviewbase.h"

namespace ARTICLE
{
    class ArticleViewMain : public ArticleViewBase
    {
        // ジャンプ予約, goto_num() のコメント参照
        int m_gotonum_reserve_to{};
        int m_gotonum_reserve_from{};

        int m_gotonum_seen{}; // 前回見ていた場所へのジャンプ用

        bool m_set_history{}; // update_finish() で履歴を登録する

        bool m_show_instdialog{};

        bool m_playsound{};

        bool m_show_closedialog{};

        bool m_reload_reserve{};

        // 連続リロード防止用
        int m_cancel_reload_counter{};

      public:
        explicit ArticleViewMain( const std::string& url );
        ~ArticleViewMain();

        void clock_in() override;
        void clock_in_always() override;

        void goto_num( const int num_to, const int num_from ) override;

        // SKELETON::View の関数のオーバロード

        void save_session() override;

        bool is_loading() const override;
        bool is_updated() const override;
        bool is_check_update() const override;
        bool is_old() const override;
        bool is_broken() const override;
        bool is_overflow() const noexcept override;

        void show_view() override;
        void update_view() override;
        void update_finish() override;
        void relayout( const bool completely = false ) override;

      protected:

        // 実況
        void live_start() override;
        void live_stop() override;

      private:

        void exec_reload() override;

        // ステータスに表示する文字列作成
        void create_status_message();

        void show_instruct_diag();
    };
}



#endif
