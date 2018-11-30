// ライセンス: GPL2

//
// ポップアップ系ビュー
//

#ifndef _ARTICLEVIEWPOPUP_H
#define _ARTICLEVIEWPOPUP_H

#include "articleviewbase.h"

namespace ARTICLE
{

    // ポップアップビューのベース
    class ArticleViewPopup : public ArticleViewBase
    {
        bool m_show_abone;

      public:
        ArticleViewPopup( const std::string& url, bool show_abone );
        ~ArticleViewPopup();

        void stop() override {}

      protected:
        void show_instruct_popup();
        const bool show_abone() const { return m_show_abone; }

      private:
        DrawAreaBase* create_drawarea() override;
    };

    /////////////////////////////////////////////////////////////////////////


    // HTMLコメントポップアップ
    class ArticleViewPopupHTML : public ArticleViewPopup
    {
        std::string m_html;

      public:
        ArticleViewPopupHTML( const std::string& url, const std::string& html ): ArticleViewPopup( url, false ), m_html( html ){}
        ~ArticleViewPopupHTML() noexcept {}

        void show_view() override { append_html( m_html ); }
    };


    /////////////////////////////////////////////////////////////////////////


    // レス抽出ポップアップ
    class ArticleViewPopupRes : public ArticleViewPopup
    {
        std::string m_str_num;
        bool m_show_title;

      public:
        ArticleViewPopupRes( const std::string& url, const std::string& num, bool show_title, bool show_abone )
        : ArticleViewPopup( url, show_abone ), m_str_num( num ), m_show_title( show_title ){}
        ~ArticleViewPopupRes() noexcept {}

        void show_view() override
        {
            show_instruct_popup();
            show_res( m_str_num, m_show_title );
        }
    };


    /////////////////////////////////////////////////////////////////////////


    // 名前抽出ポップアップ
    class ArticleViewPopupName : public ArticleViewPopup
    {
        std::string m_str_name;

      public:
        ArticleViewPopupName( const std::string& url, const std::string& name ): ArticleViewPopup( url, false ), m_str_name( name ){}
        ~ArticleViewPopupName() noexcept {}

        void show_view() override
        {
            show_instruct_popup();
            show_name( m_str_name, false );
        }
    };


    /////////////////////////////////////////////////////////////////////////


    // ID 抽出ポップアップ
    class ArticleViewPopupID : public ArticleViewPopup
    {
        std::string m_str_id;

      public:
        ArticleViewPopupID( const std::string& url, const std::string& id ): ArticleViewPopup( url, false ), m_str_id( id ) {}
        ~ArticleViewPopupID() noexcept {}

        void show_view() override
        {
            show_instruct_popup();
            show_id( m_str_id, false );
        }
    };


    /////////////////////////////////////////////////////////////////////////


    // 参照抽出ポップアップ
    class ArticleViewPopupRefer : public ArticleViewPopup
    {
        std::string m_str_num;

      public:
        ArticleViewPopupRefer( const std::string& url, const std::string& num ): ArticleViewPopup( url, false ), m_str_num( num ){}
        ~ArticleViewPopupRefer() noexcept {}

        void show_view() override
        {
            show_instruct_popup();
            show_refer( atol( m_str_num.c_str() ) );
        }
    };

    /////////////////////////////////////////////////////////////////////////

    // キーワード抽出ビュー
    class ArticleViewPopupDrawout : public ArticleViewPopup
    {
        std::string m_query;
        bool m_mode_or;

      public:
        ArticleViewPopupDrawout( const std::string& url, const std::string& query, bool mode_or )
        : ArticleViewPopup( url, false ), m_query( query ), m_mode_or( mode_or ){}
        ~ArticleViewPopupDrawout() noexcept {}

        void show_view() override
        {
            show_instruct_popup();
            drawout_keywords( m_query, m_mode_or, false );
        }
    };

    /////////////////////////////////////////////////////////////////////////


    // しおり抽出ポップアップ
    class ArticleViewPopupBM : public ArticleViewPopup
    {
      public:
      ArticleViewPopupBM( const std::string& url ) : ArticleViewPopup( url, false ){}
        ~ArticleViewPopupBM() noexcept {}

        void show_view() override
        {
            show_instruct_popup();
            show_bm();
        }
    };
}

#endif
