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
        ~BoardViewNext();

        void reload() override;
        void update_view() override;
        void update_boardname() override;

      protected:

        // デフォルトのソート状態
        int get_default_sort_column() override;
        int get_default_view_sort_mode() override;
        int get_default_view_sort_pre_column() override;
        int get_default_view_sort_pre_mode() override;

      private:

        // TFIDFで次スレ検索
        void update_by_tfidf( std::vector< NEXT_ITEM >& next_items );

        // 次スレ移行処理に使用する前スレのアドレス
        // 次スレ移行処理に使用する。BoardViewBase::open_row()を参照せよ
        std::string get_url_pre_article() override { return m_url_pre_article; }

        void slot_abone_thread() override;

        // ソート列やソートモードの保存
        void save_sort_columns() override {} // 保存しない

        // 列幅の保存
        void save_column_width() override {} // 保存しない
    };

}

#endif
