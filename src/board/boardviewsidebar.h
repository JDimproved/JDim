// ライセンス: GPL2

// サイドバー一覧ビュー

#ifndef _BOARDVIEWSIDEBAR_H
#define _BOARDVIEWSIDEBAR_H

#include "boardviewbase.h"

#include "jdlib/hash_set.h"

namespace BOARD
{
    class BoardViewSidebar : public BOARD::BoardViewBase
    {
        std::string m_sidebar_url;
        size_t m_dirid;

        bool m_set_history;

        JDLIB::hash_set_thread m_set_thread;

      public:

        BoardViewSidebar( const std::string& url, const bool set_history );
        virtual ~BoardViewSidebar();

        virtual void stop(){}
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

        virtual void slot_abone_thread();

        // ソート列やソートモードの保存
        virtual void save_sort_columns(){} // 保存しない

        // 列幅の保存
        virtual void save_column_width(){} // 保存しない
    };

}

#endif
