// ライセンス: GPL2

//
// 記事の管理クラス
//
#ifndef _ARTICLEADMIN_H
#define _ARTICLEADMIN_H

#include "skeleton/admin.h"

#include "sign.h"

#include <memory>
#include <set>
#include <string>


namespace JDLIB
{
    class Timeout;
}


namespace ARTICLE
{
    /// @brief ツールバーの削除ボタンを紐づけるID
    static constexpr std::size_t kToolbarWidgetDelete = 0;

    class ArticleToolBar;
    class ArticleToolBarSimple;
    class SearchToolBar;

    enum
    {
        TOOLBAR_ARTICLE = 0,
        TOOLBAR_SIMPLE,
        TOOLBAR_SEARCH
    };

    class ArticleAdmin : public SKELETON::Admin
    {
        std::unique_ptr<ArticleToolBar> m_toolbar;
        std::unique_ptr<ArticleToolBarSimple> m_toolbarsimple;
        std::unique_ptr<SearchToolBar> m_search_toolbar;

        std::unique_ptr<JDLIB::Timeout> m_conn_timer;

      public:
        explicit ArticleAdmin( const std::string& url );
        ~ArticleAdmin() override;

        void save_session() override;
       
      protected:
        COMMAND_ARGS get_open_list_args( const std::string& url, const COMMAND_ARGS& command_list ) override;
        SKELETON::View* create_view( const COMMAND_ARGS& command ) override;

        // ツールバー
        void show_toolbar() override;
        void toggle_toolbar() override;
        void open_searchbar() override;
        void close_searchbar() override;

        void command_local( const COMMAND_ARGS& command ) override;

        void restore( const bool only_locked ) override;
        COMMAND_ARGS url_to_openarg( const std::string& url, const bool tab, const bool lock ) override;
        std::string command_to_url( const COMMAND_ARGS& command ) override;

        void switch_admin() override;

        void restore_lasttab() override;

      private:

        bool clock_in_smooth_scroll( int timer_number );

        void delete_popup();
        void delete_all_popups();

        // タブをお気に入りにドロップした時にお気に入りがデータ送信を要求してきた
        void slot_drag_data_get( Gtk::SelectionData& selection_data, const int page ) override;
        void append_favorite_impl( const std::string& url ) override;
    };
    
    ARTICLE::ArticleAdmin* get_admin();
    void delete_admin();
}

#endif
