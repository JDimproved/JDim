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
    class ArticleViewSearch : public ArticleViewBase
    {
        std::string m_url_title;
        std::string m_url_board;
        std::string m_time_str;
        int m_searchmode; // 上のenumで定義した検索モード
        bool m_mode_or;
        bool m_enable_bm;
        bool m_bm;
        std::list< CORE::SEARCHDATA > m_list_searchdata;
        bool m_loading;
        bool m_search_executed;

      public:

        // exec_search == true ならviewを開いてすぐに検索開始
        // mode_or == true なら OR 検索する
        ArticleViewSearch( const std::string& url_board, // searchmode == SEARCHMODE_LOG の場合はboardのurl
                           const std::string& query,
                           const int searchmode,  // searchmanager.hで定義した検索モード
                           const bool exec_search, const bool mode_or, const bool bm );
        ~ArticleViewSearch();

        // SKELETON::View の関数のオーバロード

        virtual const std::string url_for_copy(); // コピーやURLバー表示用のURL

        virtual const bool is_loading(){ return m_loading; }

        virtual void focus_view();
        virtual void show_view();
        virtual void relayout();
        virtual void reload();
        virtual void stop();

        // 検索
        virtual void exec_search();
        virtual void operate_search( const std::string& controlid );
        const bool get_enable_bm() const{ return m_enable_bm; }
        const bool get_bm() const { return m_bm; }
        void set_bm( const bool set ){ m_bm = set; }

      protected:

        virtual void slot_push_write(){} // 書き込みキャンセル

      private:

        //viewのURL更新
        void update_url_query( const bool update_history );

        void slot_search_fin( const std::string& id );

        virtual void exec_reload();
    };
}

#endif
