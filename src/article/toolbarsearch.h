// ライセンス: GPL2

// ログ検索などのツールバーのクラス
//
// ARTICLE::ArticleViewSearch 以外では使わない
//

#ifndef _ARTICLE_TOOLBARSEARCH_H
#define _ARTICLE_TOOLBARSEARCH_H

#include "skeleton/toolbar.h"

namespace ARTICLE
{
    class ArticleViewSearch;

    class SearchToolBar : public SKELETON::ToolBar
    {
        ArticleViewSearch* m_searchview;

        bool m_enable_slot;

        Gtk::ToolItem m_tool_bm;
        Gtk::CheckButton m_check_bm;

      public:

        SearchToolBar();
        ~SearchToolBar() noexcept {}

        // タブが切り替わった時に呼び出される( Viewの情報を取得する )
        void set_view( SKELETON::View * view ) override;

      protected:

        void pack_buttons() override;

      private:

        void slot_toggle_bm();
    };
}


#endif
