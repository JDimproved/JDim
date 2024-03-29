// ライセンス: GPL2

// サイドバー一覧ビュー

#ifndef _BOARDVIEWSIDEBAR_H
#define _BOARDVIEWSIDEBAR_H

#include "boardviewbase.h"

#include <string>
#include <unordered_set>

namespace BOARD
{
    class BoardViewSidebar : public BOARD::BoardViewBase
    {
        std::string m_sidebar_url;
        size_t m_dirid;

        bool m_set_history;

        std::unordered_set< std::string > m_set_thread;

      public:

        BoardViewSidebar( const std::string& url, const bool set_history );
        ~BoardViewSidebar() override;

        void stop() override {}
        void reload() override;
        void show_view() override;
        void update_boardname() override;

        void update_item( const std::string& url, const std::string& id ) override;

      protected:

        // デフォルトのソート状態
        int get_default_sort_column() const override;
        int get_default_view_sort_mode() const override;
        int get_default_view_sort_pre_column() const override;
        int get_default_view_sort_pre_mode() const override;

      private:

        void slot_abone_thread() override;

        // ソート列やソートモードの保存
        void save_sort_columns() override {} // 保存しない

        // 列幅の保存
        void save_column_width() override {} // 保存しない
    };

}

#endif
