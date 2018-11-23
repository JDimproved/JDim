// ライセンス: GPL2

// ログ一覧ビュー

#ifndef _BOARDVIEWLOG_H
#define _BOARDVIEWLOG_H

#include "boardviewbase.h"

#include "jdlib/hash_set.h"

namespace BOARD
{
    class BoardViewLog : public BOARD::BoardViewBase
    {
        JDLIB::hash_set_thread m_set_thread;

      public:

        BoardViewLog( const std::string& url );
        virtual ~BoardViewLog();

        virtual void stop();
        virtual void reload();
        virtual void show_view();
        virtual void update_boardname();

        virtual void update_item( const std::string& url, const std::string& id );

      protected:

        // デフォルトのソート状態
        virtual const int get_default_sort_column();
        virtual const int get_default_view_sort_mode();
        virtual const int get_default_view_sort_pre_column();
        virtual const int get_default_view_sort_pre_mode();

      private:

        void slot_search_fin( const std::string& id );

        virtual void slot_abone_thread();

        // ソート列やソートモードの保存
        virtual void save_sort_columns(){} // 保存しない

        // 列幅の保存
        virtual void save_column_width(){} // 保存しない
    };

}

#endif
