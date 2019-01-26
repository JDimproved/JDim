// ライセンス: GPL2

// ログ一覧ビュー

#ifndef _BOARDVIEWLOG_H
#define _BOARDVIEWLOG_H

#include "boardviewbase.h"

#include <string>
#include <unordered_set>

namespace BOARD
{
    class BoardViewLog : public BOARD::BoardViewBase
    {
        std::unordered_set< std::string > m_set_thread;

      public:

        BoardViewLog( const std::string& url );
        ~BoardViewLog();

        void stop() override;
        void reload() override;
        void show_view() override;
        void update_boardname() override;

        void update_item( const std::string& url, const std::string& id ) override;

      protected:

        // デフォルトのソート状態
        int get_default_sort_column() override;
        int get_default_view_sort_mode() override;
        int get_default_view_sort_pre_column() override;
        int get_default_view_sort_pre_mode() override;

      private:

        void slot_search_fin( const std::string& id );

        void slot_abone_thread() override;

        // ソート列やソートモードの保存
        void save_sort_columns() override {} // 保存しない

        // 列幅の保存
        void save_column_width() override {} // 保存しない
    };

}

#endif
