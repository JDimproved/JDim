// ライセンス: 最新のGPL

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

      protected:
        void show_instruct_popup();

      private:
        virtual void pack_widget();
        virtual DrawAreaBase* create_drawarea();
    };

    /////////////////////////////////////////////////////////////////////////


    // HTMLコメントポップアップ
    class ArticleViewPopupHTML : public ArticleViewPopup
    {
        std::string m_html;

      public:
        ArticleViewPopupHTML( const std::string& url, const std::string& html ): ArticleViewPopup( url, false ), m_html( html ){}
        ~ArticleViewPopupHTML(){}

        virtual void show_view(){ append_html( m_html ); }
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
        ~ArticleViewPopupRes(){}

        virtual void show_view(){
            show_instruct_popup();
            show_res( m_str_num, m_show_title );
        }
    };


    /////////////////////////////////////////////////////////////////////////


    // ID 抽出ポップアップ
    class ArticleViewPopupID : public ArticleViewPopup
    {
        std::string m_str_id;

      public:
        ArticleViewPopupID( const std::string& url, const std::string& id ): ArticleViewPopup( url, false ), m_str_id( id ) {}
        ~ArticleViewPopupID(){}

        virtual void show_view(){
            show_instruct_popup();
            show_id( m_str_id );
        }
    };


    /////////////////////////////////////////////////////////////////////////


    // 参照抽出ポップアップ
    class ArticleViewPopupRefer : public ArticleViewPopup
    {
        std::string m_str_num;

      public:
        ArticleViewPopupRefer( const std::string& url, const std::string& num ): ArticleViewPopup( url, false ), m_str_num( num ){}
        ~ArticleViewPopupRefer(){}

        virtual void show_view(){
            show_instruct_popup();
            show_refer( atol( m_str_num.c_str() ) );
        }
    };

}

#endif
