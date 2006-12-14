// ライセンス: GPL2

//
// メインビューなどのタブに張り付けるビュー
//

#ifndef _ARTICLEVIEW_H
#define _ARTICLEVIEW_H

#include "articleviewbase.h"

namespace ARTICLE
{
    // メインビュー
    class ArticleViewMain : public ArticleViewBase
    {
        // ジャンプ予約, goto_num() のコメント参照
        int m_gotonum_reserve; 

      public:
        ArticleViewMain( const std::string& url );
        ~ArticleViewMain();

        virtual void goto_num( int num );

        // SKELETON::View の関数のオーバロード
        virtual void reload();
        virtual void show_view();
        virtual void update_view();
        virtual void update_finish();
        virtual void relayout();
    };


    /////////////////////////////////////////////////////////////////////////

    // レス抽出ビュー
    class ArticleViewRes : public ArticleViewBase
    {
        std::string m_str_num;
        std::string m_str_center;
        bool m_show_title;

      public:
        ArticleViewRes( const std::string& url, const std::string& num, bool show_title, const std::string& center );
        ~ArticleViewRes();

        // SKELETON::View の関数のオーバロード
        virtual void show_view();
        virtual void relayout();
    };


    /////////////////////////////////////////////////////////////////////////


    // 名前抽出ビュー
    class ArticleViewName : public ArticleViewBase
    {
        std::string m_str_name;

      public:
        ArticleViewName( const std::string& url, const std::string& name );
        ~ArticleViewName();

        // SKELETON::View の関数のオーバロード
        virtual void show_view();
        virtual void relayout();
    };


    /////////////////////////////////////////////////////////////////////////


    // ID 抽出ビュー
    class ArticleViewID : public ArticleViewBase
    {
        std::string m_str_id;

      public:
        ArticleViewID( const std::string& url, const std::string& id );
        ~ArticleViewID();

        // SKELETON::View の関数のオーバロード
        virtual void show_view();
        virtual void relayout();
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
        virtual void show_view();
        virtual void relayout();
    };



    /////////////////////////////////////////////////////////////////////////


    // URL抽出ビュー
    class ArticleViewURL : public ArticleViewBase
    {
      public:
        ArticleViewURL( const std::string& url );
        ~ArticleViewURL();

        // SKELETON::View の関数のオーバロード
        virtual void show_view();
        virtual void relayout();
    };


    /////////////////////////////////////////////////////////////////////////


    // 参照抽出ビュー
    class ArticleViewRefer : public ArticleViewBase
    {
        std::string m_str_num;

      public:
        ArticleViewRefer( const std::string& url, const std::string& num );
        ~ArticleViewRefer();

        // SKELETON::View の関数のオーバロード
        virtual void show_view();
        virtual void relayout();
    };



    /////////////////////////////////////////////////////////////////////////


    // キーワード抽出ビュー
    class ArticleViewDrawout : public ArticleViewBase
    {
        std::string m_query;
        bool m_mode_or;

      public:
        ArticleViewDrawout( const std::string& url, const std::string& query, bool mode_or );
        ~ArticleViewDrawout();

        // SKELETON::View の関数のオーバロード
        virtual void show_view();
        virtual void relayout();
    };

}



#endif
