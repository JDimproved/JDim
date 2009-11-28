// ライセンス: GPL2

// ログ一覧ビュー

#ifndef _BOARDVIEWLOG_H
#define _BOARDVIEWLOG_H

#include "boardviewbase.h"

namespace BOARD
{
    class BoardViewLog : public BOARD::BoardViewBase
    {

      public:

        BoardViewLog( const std::string& url );
        virtual ~BoardViewLog();

        virtual void stop();
        virtual void reload();
        virtual void show_view();
        virtual void update_boardname();

        virtual void update_item( const std::string& url, const std::string& id );

      private:

        void slot_search_fin( const std::string& id );

        virtual void slot_abone_thread();

        // ソート列やソートモードの保存
        virtual void save_sort_columns(){} // 保存しない

        // 列幅の保存
        virtual void save_column_width(){} // 保存しない
    };

};

#endif
