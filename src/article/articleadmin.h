// ライセンス: GPL2

//
// 記事の管理クラス
//
#ifndef _ARTICLEADMIN_H
#define _ARTICLEADMIN_H

#include "skeleton/admin.h"

#include <list>
#include <string>
#include <set>

#define ARTICLE_SIGN "_ARTICLE_"
#define BOARD_SIGN "_BOARD_"
#define RES_SIGN     "_RES_"
#define NAME_SIGN    "_NAME_"
#define ID_SIGN      "_ID_"
#define BOOKMK_SIGN  "_BM_"
#define WROTE_SIGN  "_WROTE_"
#define URL_SIGN     "_URL_"
#define REFER_SIGN   "_REF_"
#define KEYWORD_SIGN "_KW_"
#define ORMODE_SIGN      "_OR_"
#define CENTER_SIGN  "_CENTER_"
#define TIME_SIGN    "_TIME_"
#define TITLE_SIGN   "_TITLE_"

namespace ARTICLE
{
    class ArticleToolBar;
    class SearchToolBar;

    enum
    {
        TOOLBAR_ARTICLE = 0,
        TOOLBAR_SEARCH = 1
    };

    class ArticleAdmin : public SKELETON::Admin
    {
        ArticleToolBar* m_toolbar;
        SearchToolBar* m_search_toolbar;

      public:
        ArticleAdmin( const std::string& url );
        ~ArticleAdmin();

      protected:
        virtual COMMAND_ARGS get_open_list_args( const std::string& url );
        virtual SKELETON::View* create_view( const COMMAND_ARGS& command );

        // ツールバー
        virtual void show_toolbar();
        virtual void toggle_toolbar();
        virtual void open_searchbar();
        virtual void close_searchbar();

        virtual void command_local( const COMMAND_ARGS& command );

        virtual void restore( const bool only_locked );
        virtual COMMAND_ARGS url_to_openarg( const std::string& url, const bool tab, const bool lock );

        virtual void switch_admin();

      private:

        bool clock_in_smooth_scroll( int timer_number );

        void delete_popup();
        void delete_all_popups();

        // タブの D&D 処理
        virtual void slot_drag_begin( int page );
        virtual void slot_drag_end();
    };
    
    ARTICLE::ArticleAdmin* get_admin();
    void delete_admin();
}

#endif
