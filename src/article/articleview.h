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
        int m_gotonum_reserve; 

        int m_gotonum_seen; // 前回見ていた場所へのジャンプ用

        bool m_set_history; // update_finish() で履歴を登録する

        bool m_show_instdialog;

        bool m_playsound;

        bool m_show_closedialog;

        bool m_reload_reserve;

      public:
        ArticleViewMain( const std::string& url );
        ~ArticleViewMain();

        virtual void clock_in();

        virtual void goto_num( int num );

        // SKELETON::View の関数のオーバロード
        virtual const bool is_loading();
        virtual const bool is_updated();
        virtual const bool is_check_update();
        virtual const bool is_old();
        virtual const bool is_broken();

        virtual void show_view();
        virtual void update_view();
        virtual void update_finish();
        virtual void relayout();

      protected:

        // 実況
        virtual void live_start();
        virtual void live_stop();

      private:

        virtual void exec_reload();

        // ステータスに表示する文字列作成
        void create_status_message();

        void show_instruct_diag();
    };
}



#endif
