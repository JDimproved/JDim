// ライセンス: GPL2

//
// ログやスレタイ検索抽出ビュー
//

#ifndef _ARTICLEVIEWSEARCH_H
#define _ARTICLEVIEWSEARCH_H

#include "articleviewbase.h"

#include "searchmanager.h"

namespace ARTICLE
{
    // 検索モード
    // コンストラクタの searchmode で指定する
    enum{
        SEARCHMODE_LOG = 0,
        SEARCHMODE_ALLLOG,
        SEARCHMODE_TITLE
    };

    class ArticleViewSearch : public ArticleViewBase
    {
        std::string m_url_board;
        std::string m_time_str;
        int m_searchmode;
        bool m_mode_or;
        std::list< CORE::SEARCHDATA > m_list_searchdata;
        bool m_loading;
        bool m_search_executed;

      public:

        // exec_search == true ならviewを開いてすぐに検索開始
        // mode_or == true なら OR 検索する
        ArticleViewSearch( const std::string& url_board, // searchmode == SEARCHMODE_LOG の場合はboardのurl
                           const std::string& query, const int searchmode,
                           const bool exec_search, const bool mode_or );
        ~ArticleViewSearch();

        // SKELETON::View の関数のオーバロード
        virtual const bool is_loading(){ return m_loading; }

        virtual void focus_view();
        virtual void show_view();
        virtual void relayout();
        virtual void reload();
        virtual void stop();

        // 検索
        virtual void exec_search();
        virtual void operate_search( const std::string& controlid );

      protected:

        virtual void slot_push_write(){} // 書き込みキャンセル

      private:

        //viewのURL更新
        void update_url_query( const bool update_history );

        void slot_search_fin();
    };
}

#endif
