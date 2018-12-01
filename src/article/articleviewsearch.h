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
        int m_searchmode; // searchmanager.hで定義した検索モード
        bool m_mode_or;
        bool m_enable_bm;
        bool m_bm;
        std::list< CORE::SEARCHDATA > m_list_searchdata;
        bool m_loading;
        bool m_search_executed;
        bool m_escaped;
        bool m_cancel_reload;

      public:

        // exec_search == true ならviewを開いてすぐに検索開始
        ArticleViewSearch( const std::string& url, const bool exec_search );
        ~ArticleViewSearch();

        // SKELETON::View の関数のオーバロード

        virtual const std::string url_for_copy(); // コピーやURLバー表示用のURL
        virtual const bool set_command( const std::string& command,
                                        const std::string& arg1 = std::string(),
                                        const std::string& arg2 = std::string()
            );

        virtual const bool is_loading() const { return m_loading; }

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

        // url から query などを取得してツールバーの状態をセット
        void set_toolbar_from_url();

        // queryなどを変更した時の新しいURL
        const std::string get_new_url();

        // ラベルやタブを更新
        void update_label();

        // 正規表現メタ文字をエスケープ
        void regex_escape();

        void slot_search_fin( const std::string& id );

        virtual void exec_reload();
    };
}

#endif
