// ライセンス: GPL2

// スレ一覧ビュー

#ifndef _BOARDVIEW_H
#define _BOARDVIEW_H

#include "boardviewbase.h"

namespace BOARD
{

    // メインビュー
    class BoardView : public BOARD::BoardViewBase
    {
      public:

        BoardView( const std::string& url );
        virtual ~BoardView();

        virtual void reload();
        virtual void update_view();
        virtual void update_boardname();
    };


    // 次スレ検索ビュー

    typedef struct
    {

        DBTREE::ArticleBase* article;
        int value;
        time_t since;

    } NEXT_ITEM;

    class BoardViewNext : public BOARD::BoardViewBase
    {
        std::string m_url_pre_article;

      public:

        BoardViewNext( const std::string& url, const std::string& url_pre_article );
        virtual ~BoardViewNext();

        virtual void reload();
        virtual void update_view();
        virtual void update_boardname();

      private:

        // TFIDFで次スレ検索
        void update_by_tfidf( std::list< NEXT_ITEM >& next_items );

        virtual const std::string get_url_pre_article(){ return m_url_pre_article; }

        // ソート列やソートモードの保存
        virtual void save_sort_columns(){} // 保存しない

        // 列幅の保存
        virtual void save_column_width(){} // 保存しない
    };

};

#endif
