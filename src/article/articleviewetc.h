// ライセンス: GPL2

//
// その他のarticle系view
//

#ifndef _ARTICLEVIEWETC_H
#define _ARTICLEVIEWETC_H

#include "articleviewbase.h"

namespace ARTICLE
{
    // レス抽出ビュー
    class ArticleViewRes : public ArticleViewBase
    {
        std::string m_str_num;
        std::string m_str_center;

      public:
        ArticleViewRes( const std::string& url );
        ~ArticleViewRes();

        // SKELETON::View の関数のオーバロード
        void relayout() override;

        void show_view() override;
        void reload() override;

      private:

        void exec_reload() override;
    };


    /////////////////////////////////////////////////////////////////////////


    // 名前抽出ビュー
    class ArticleViewName : public ArticleViewBase
    {
        std::string m_str_name;

      public:
        ArticleViewName( const std::string& url );
        ~ArticleViewName();

        // SKELETON::View の関数のオーバロード
        void relayout() override;

        void show_view() override;
        void reload() override;

      private:

        void exec_reload() override;
    };


    /////////////////////////////////////////////////////////////////////////


    // ID 抽出ビュー
    class ArticleViewID : public ArticleViewBase
    {
        std::string m_str_id;

      public:
        ArticleViewID( const std::string& url );
        ~ArticleViewID();

        // SKELETON::View の関数のオーバロード
        void relayout() override;

        void show_view() override;
        void reload() override;

      private:

        void exec_reload() override;
    };


    /////////////////////////////////////////////////////////////////////////


    // ブックマーク抽出ビュー
    class ArticleViewBM : public ArticleViewBase
    {
        std::string m_str_id;

      public:
        ArticleViewBM( const std::string& url );
        ~ArticleViewBM();

        // SKELETON::View の関数のオーバロード
        void relayout() override;

        void show_view() override;
        void reload() override;

      private:

        void exec_reload() override;
    };



    /////////////////////////////////////////////////////////////////////////


    // 書き込み抽出ビュー
    class ArticleViewPost : public ArticleViewBase
    {
        std::string m_str_id;

      public:
        ArticleViewPost( const std::string& url );
        ~ArticleViewPost();

        // SKELETON::View の関数のオーバロード
        void relayout() override;

        void show_view() override;
        void reload() override;

      private:

        void exec_reload() override;
    };



    /////////////////////////////////////////////////////////////////////////


    // URL抽出ビュー
    class ArticleViewURL : public ArticleViewBase
    {
      public:
        ArticleViewURL( const std::string& url );
        ~ArticleViewURL();

        // SKELETON::View の関数のオーバロード
        void relayout() override;

        void show_view() override;
        void reload() override;

      private:

        void exec_reload() override;
    };


    /////////////////////////////////////////////////////////////////////////


    // 参照抽出ビュー
    class ArticleViewRefer : public ArticleViewBase
    {
        std::string m_str_num;

      public:
        ArticleViewRefer( const std::string& url );
        ~ArticleViewRefer();

        // SKELETON::View の関数のオーバロード
        void relayout() override;

        void show_view() override;
        void reload() override;

      private:

        void exec_reload() override;
    };



    /////////////////////////////////////////////////////////////////////////


    // キーワード抽出ビュー
    class ArticleViewDrawout : public ArticleViewBase
    {
        std::string m_query;
        bool m_mode_or;

      public:
        ArticleViewDrawout( const std::string& url );
        ~ArticleViewDrawout();

        // SKELETON::View の関数のオーバロード
        void relayout() override;

        void show_view() override;
        void reload() override;

      private:

        void exec_reload() override;
    };

    /////////////////////////////////////////////////////////////////////////


    // 書き込みログ表示ビュー
    class ArticleViewPostlog : public ArticleViewBase
    {
        int m_num;

      public:
        ArticleViewPostlog( const std::string& url );
        ~ArticleViewPostlog();

        // SKELETON::View の関数のオーバロード
        void relayout() override;
        void stop() override {} // キャンセル

        // 検索
        void operate_search( const std::string& controlid ) override;

        void show_view() override;
        void reload() override;

      private:
        void exec_reload() override;
    };
}



#endif
