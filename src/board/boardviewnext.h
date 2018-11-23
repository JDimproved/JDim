// ライセンス: GPL2

// 次スレ検索ビュー

#ifndef _BOARDVIEWNEXT_H
#define _BOARDVIEWNEXT_H

#include "boardviewbase.h"

#include <ctime>

namespace BOARD
{

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

      protected:

        // デフォルトのソート状態
        virtual const int get_default_sort_column();
        virtual const int get_default_view_sort_mode();
        virtual const int get_default_view_sort_pre_column();
        virtual const int get_default_view_sort_pre_mode();

      private:

        // TFIDFで次スレ検索
        void update_by_tfidf( std::vector< NEXT_ITEM >& next_items );

        // 次スレ移行処理に使用する前スレのアドレス
        // 次スレ移行処理に使用する。BoardViewBase::open_row()を参照せよ
        virtual const std::string get_url_pre_article(){ return m_url_pre_article; }

        virtual void slot_abone_thread();

        // ソート列やソートモードの保存
        virtual void save_sort_columns(){} // 保存しない

        // 列幅の保存
        virtual void save_column_width(){} // 保存しない
    };

}

#endif
